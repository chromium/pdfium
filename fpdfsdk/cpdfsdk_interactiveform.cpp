// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_interactiveform.h"

#include <stdint.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "constants/annotation_flags.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cfdf_document.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_formcontrol.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_string_wrappers.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "fxjs/ijs_event_context.h"
#include "fxjs/ijs_runtime.h"

namespace {

constexpr uint32_t kWhiteBGR = FXSYS_BGR(255, 255, 255);

bool IsFormFieldTypeComboOrText(FormFieldType fieldType) {
  switch (fieldType) {
    case FormFieldType::kComboBox:
    case FormFieldType::kTextField:
      return true;
    default:
      return false;
  }
}

#ifdef PDF_ENABLE_XFA
bool IsFormFieldTypeXFA(FormFieldType fieldType) {
  switch (fieldType) {
    case FormFieldType::kXFA:
    case FormFieldType::kXFA_CheckBox:
    case FormFieldType::kXFA_ComboBox:
    case FormFieldType::kXFA_ImageField:
    case FormFieldType::kXFA_ListBox:
    case FormFieldType::kXFA_PushButton:
    case FormFieldType::kXFA_Signature:
    case FormFieldType::kXFA_TextField:
      return true;
    default:
      return false;
  }
}
#endif  // PDF_ENABLE_XFA

ByteString FDFToURLEncodedData(ByteString buffer) {
  std::unique_ptr<CFDF_Document> pFDF =
      CFDF_Document::ParseMemory(buffer.unsigned_span());
  if (!pFDF) {
    return buffer;
  }

  RetainPtr<const CPDF_Dictionary> pMainDict =
      pFDF->GetRoot()->GetDictFor("FDF");
  if (!pMainDict) {
    return ByteString();
  }

  RetainPtr<const CPDF_Array> pFields = pMainDict->GetArrayFor("Fields");
  if (!pFields) {
    return ByteString();
  }

  fxcrt::ostringstream encoded_data;
  for (uint32_t i = 0; i < pFields->size(); i++) {
    RetainPtr<const CPDF_Dictionary> pField = pFields->GetDictAt(i);
    if (!pField) {
      continue;
    }
    WideString name = pField->GetUnicodeTextFor("T");
    ByteString name_b = name.ToDefANSI();
    ByteString csBValue = pField->GetByteStringFor("V");
    WideString csWValue = PDF_DecodeText(csBValue.unsigned_span());
    ByteString csValue_b = csWValue.ToDefANSI();
    encoded_data << name_b << "=" << csValue_b;
    if (i != pFields->size() - 1) {
      encoded_data << "&";
    }
  }

  return ByteString(encoded_data);
}

}  // namespace

CPDFSDK_InteractiveForm::CPDFSDK_InteractiveForm(
    CPDFSDK_FormFillEnvironment* pFormFillEnv)
    : form_fill_env_(pFormFillEnv),
      interactive_form_(std::make_unique<CPDF_InteractiveForm>(
          form_fill_env_->GetPDFDocument())) {
  interactive_form_->SetNotifierIface(this);
  RemoveAllHighLights();
}

CPDFSDK_InteractiveForm::~CPDFSDK_InteractiveForm() = default;

CPDFSDK_Widget* CPDFSDK_InteractiveForm::GetWidget(
    CPDF_FormControl* pControl) const {
  if (!pControl) {
    return nullptr;
  }

  CPDFSDK_Widget* pWidget = nullptr;
  const auto it = map_.find(pControl);
  if (it != map_.end()) {
    pWidget = it->second;
  }
  if (pWidget) {
    return pWidget;
  }

  CPDF_Document* pDocument = form_fill_env_->GetPDFDocument();
  CPDFSDK_PageView* pPage = nullptr;
  RetainPtr<const CPDF_Dictionary> pControlDict = pControl->GetWidgetDict();
  RetainPtr<const CPDF_Dictionary> pPageDict = pControlDict->GetDictFor("P");
  if (pPageDict) {
    int nPageIndex = pDocument->GetPageIndex(pPageDict->GetObjNum());
    if (nPageIndex >= 0) {
      pPage = form_fill_env_->GetPageViewAtIndex(nPageIndex);
    }
  }

  if (!pPage) {
    int nPageIndex = GetPageIndexByAnnotDict(pDocument, pControlDict);
    if (nPageIndex >= 0) {
      pPage = form_fill_env_->GetPageViewAtIndex(nPageIndex);
    }
  }

  return pPage ? ToCPDFSDKWidget(pPage->GetAnnotByDict(pControlDict)) : nullptr;
}

void CPDFSDK_InteractiveForm::GetWidgets(
    const WideString& sFieldName,
    std::vector<ObservedPtr<CPDFSDK_Widget>>* widgets) const {
  for (size_t i = 0, sz = interactive_form_->CountFields(sFieldName); i < sz;
       ++i) {
    CPDF_FormField* pFormField = interactive_form_->GetField(i, sFieldName);
    DCHECK(pFormField);
    GetWidgets(pFormField, widgets);
  }
}

void CPDFSDK_InteractiveForm::GetWidgets(
    CPDF_FormField* pField,
    std::vector<ObservedPtr<CPDFSDK_Widget>>* widgets) const {
  for (int i = 0, sz = pField->CountControls(); i < sz; ++i) {
    CPDF_FormControl* pFormCtrl = pField->GetControl(i);
    DCHECK(pFormCtrl);
    CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl);
    if (pWidget) {
      widgets->emplace_back(pWidget);
    }
  }
}

int CPDFSDK_InteractiveForm::GetPageIndexByAnnotDict(
    CPDF_Document* pDocument,
    const CPDF_Dictionary* pAnnotDict) const {
  DCHECK(pAnnotDict);

  for (int i = 0, sz = pDocument->GetPageCount(); i < sz; i++) {
    RetainPtr<const CPDF_Dictionary> pPageDict =
        pDocument->GetPageDictionary(i);
    if (!pPageDict) {
      continue;
    }

    RetainPtr<const CPDF_Array> pAnnots = pPageDict->GetArrayFor("Annots");
    if (!pAnnots) {
      continue;
    }

    for (size_t j = 0, jsz = pAnnots->size(); j < jsz; j++) {
      RetainPtr<const CPDF_Object> pDict = pAnnots->GetDirectObjectAt(j);
      if (pAnnotDict == pDict) {
        return i;
      }
    }
  }
  return -1;
}

void CPDFSDK_InteractiveForm::AddMap(CPDF_FormControl* pControl,
                                     CPDFSDK_Widget* pWidget) {
  map_[pdfium::WrapUnowned(pControl)] = pWidget;
}

void CPDFSDK_InteractiveForm::RemoveMap(CPDF_FormControl* pControl) {
  auto it = map_.find(pControl);
  if (it != map_.end()) {
    map_.erase(it);
  }
}

void CPDFSDK_InteractiveForm::EnableCalculate(bool bEnabled) {
  calculate_ = bEnabled;
}

bool CPDFSDK_InteractiveForm::IsCalculateEnabled() const {
  return calculate_;
}

#ifdef PDF_ENABLE_XFA
void CPDFSDK_InteractiveForm::XfaEnableCalculate(bool bEnabled) {
  xfa_calculate_ = bEnabled;
}

bool CPDFSDK_InteractiveForm::IsXfaCalculateEnabled() const {
  return xfa_calculate_;
}

bool CPDFSDK_InteractiveForm::IsXfaValidationsEnabled() {
  return xfa_validations_enabled_;
}
void CPDFSDK_InteractiveForm::XfaSetValidationsEnabled(bool bEnabled) {
  xfa_validations_enabled_ = bEnabled;
}

void CPDFSDK_InteractiveForm::SynchronizeField(CPDF_FormField* pFormField) {
  for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
    CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
    if (CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl)) {
      pWidget->Synchronize(false);
    }
  }
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_InteractiveForm::OnCalculate(CPDF_FormField* pFormField) {
  if (!form_fill_env_->IsJSPlatformPresent()) {
    return;
  }

  if (busy_) {
    return;
  }

  AutoRestorer<bool> restorer(&busy_);
  busy_ = true;

  if (!IsCalculateEnabled()) {
    return;
  }

  IJS_Runtime* pRuntime = form_fill_env_->GetIJSRuntime();
  int nSize = interactive_form_->CountFieldsInCalculationOrder();
  for (int i = 0; i < nSize; i++) {
    CPDF_FormField* pField = interactive_form_->GetFieldInCalculationOrder(i);
    if (!pField) {
      continue;
    }

    FormFieldType fieldType = pField->GetFieldType();
    if (!IsFormFieldTypeComboOrText(fieldType)) {
      continue;
    }

    CPDF_AAction aAction = pField->GetAdditionalAction();
    if (!aAction.ActionExist(CPDF_AAction::kCalculate)) {
      continue;
    }

    CPDF_Action action = aAction.GetAction(CPDF_AAction::kCalculate);
    if (!action.HasDict()) {
      continue;
    }

    WideString csJS = action.GetJavaScript();
    if (csJS.IsEmpty()) {
      continue;
    }

    WideString sOldValue = pField->GetValue();
    WideString sValue = sOldValue;
    bool bRC = true;
    IJS_Runtime::ScopedEventContext pContext(pRuntime);
    pContext->OnField_Calculate(pFormField, pField, &sValue, &bRC);

    std::optional<IJS_Runtime::JS_Error> err = pContext->RunScript(csJS);
    if (!err.has_value() && bRC && sValue != sOldValue) {
      pField->SetValue(sValue, NotificationOption::kNotify);
    }
  }
}

std::optional<WideString> CPDFSDK_InteractiveForm::OnFormat(
    CPDF_FormField* pFormField) {
  if (!form_fill_env_->IsJSPlatformPresent()) {
    return std::nullopt;
  }

  WideString sValue = pFormField->GetValue();
  IJS_Runtime* pRuntime = form_fill_env_->GetIJSRuntime();
  if (pFormField->GetFieldType() == FormFieldType::kComboBox &&
      pFormField->CountSelectedItems() > 0) {
    int index = pFormField->GetSelectedIndex(0);
    if (index >= 0) {
      sValue = pFormField->GetOptionLabel(index);
    }
  }

  CPDF_AAction aAction = pFormField->GetAdditionalAction();
  if (aAction.ActionExist(CPDF_AAction::kFormat)) {
    CPDF_Action action = aAction.GetAction(CPDF_AAction::kFormat);
    if (action.HasDict()) {
      WideString script = action.GetJavaScript();
      if (!script.IsEmpty()) {
        IJS_Runtime::ScopedEventContext pContext(pRuntime);
        pContext->OnField_Format(pFormField, &sValue);
        std::optional<IJS_Runtime::JS_Error> err = pContext->RunScript(script);
        if (!err.has_value()) {
          return sValue;
        }
      }
    }
  }
  return std::nullopt;
}

void CPDFSDK_InteractiveForm::ResetFieldAppearance(
    CPDF_FormField* pFormField,
    std::optional<WideString> sValue) {
  for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
    CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
    DCHECK(pFormCtrl);
    if (CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl)) {
      pWidget->ResetAppearance(sValue, CPDFSDK_Widget::kValueChanged);
    }
  }
}

void CPDFSDK_InteractiveForm::UpdateField(CPDF_FormField* pFormField) {
  auto* formfiller = form_fill_env_->GetInteractiveFormFiller();
  for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
    CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
    DCHECK(pFormCtrl);

    CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl);
    if (!pWidget) {
      continue;
    }

    IPDF_Page* pPage = pWidget->GetPage();
    FX_RECT rect =
        formfiller->GetViewBBox(form_fill_env_->GetPageView(pPage), pWidget);
    form_fill_env_->Invalidate(pPage, rect);
  }
}

bool CPDFSDK_InteractiveForm::OnKeyStrokeCommit(CPDF_FormField* pFormField,
                                                const WideString& csValue) {
  CPDF_AAction aAction = pFormField->GetAdditionalAction();
  if (!aAction.ActionExist(CPDF_AAction::kKeyStroke)) {
    return true;
  }

  CPDF_Action action = aAction.GetAction(CPDF_AAction::kKeyStroke);
  if (!action.HasDict()) {
    return true;
  }

  CFFL_FieldAction fa;
  fa.bModifier = false;
  fa.bShift = false;
  fa.sValue = csValue;
  form_fill_env_->DoActionFieldJavaScript(action, CPDF_AAction::kKeyStroke,
                                          pFormField, &fa);
  return fa.bRC;
}

bool CPDFSDK_InteractiveForm::OnValidate(CPDF_FormField* pFormField,
                                         const WideString& csValue) {
  CPDF_AAction aAction = pFormField->GetAdditionalAction();
  if (!aAction.ActionExist(CPDF_AAction::kValidate)) {
    return true;
  }

  CPDF_Action action = aAction.GetAction(CPDF_AAction::kValidate);
  if (!action.HasDict()) {
    return true;
  }

  CFFL_FieldAction fa;
  fa.bModifier = false;
  fa.bShift = false;
  fa.sValue = csValue;
  form_fill_env_->DoActionFieldJavaScript(action, CPDF_AAction::kValidate,
                                          pFormField, &fa);
  return fa.bRC;
}

bool CPDFSDK_InteractiveForm::DoAction_Hide(const CPDF_Action& action) {
  DCHECK(action.GetDict());
  std::vector<CPDF_FormField*> fields =
      GetFieldFromObjects(action.GetAllFields());
  bool bHide = action.GetHideStatus();
  bool bChanged = false;

  for (CPDF_FormField* pField : fields) {
    for (int i = 0, sz = pField->CountControls(); i < sz; ++i) {
      CPDF_FormControl* pControl = pField->GetControl(i);
      DCHECK(pControl);

      if (CPDFSDK_Widget* pWidget = GetWidget(pControl)) {
        uint32_t nFlags = pWidget->GetFlags();
        nFlags &= ~pdfium::annotation_flags::kInvisible;
        nFlags &= ~pdfium::annotation_flags::kNoView;
        if (bHide) {
          nFlags |= pdfium::annotation_flags::kHidden;
        } else {
          nFlags &= ~pdfium::annotation_flags::kHidden;
        }
        pWidget->SetFlags(nFlags);
        pWidget->GetPageView()->UpdateView(pWidget);
        bChanged = true;
      }
    }
  }

  return bChanged;
}

bool CPDFSDK_InteractiveForm::DoAction_SubmitForm(const CPDF_Action& action) {
  WideString sDestination = action.GetFilePath();
  if (sDestination.IsEmpty()) {
    return false;
  }

  if (action.HasFields()) {
    uint32_t dwFlags = action.GetFlags();
    std::vector<CPDF_FormField*> fields =
        GetFieldFromObjects(action.GetAllFields());
    if (!fields.empty()) {
      bool bIncludeOrExclude = !(dwFlags & 0x01);
      if (!interactive_form_->CheckRequiredFields(&fields, bIncludeOrExclude)) {
        return false;
      }

      return SubmitFields(sDestination, fields, bIncludeOrExclude, false);
    }
  }
  if (!interactive_form_->CheckRequiredFields(nullptr, true)) {
    return false;
  }

  return SubmitForm(sDestination);
}

bool CPDFSDK_InteractiveForm::SubmitFields(
    const WideString& csDestination,
    const std::vector<CPDF_FormField*>& fields,
    bool bIncludeOrExclude,
    bool bUrlEncoded) {
  ByteString text_buf = ExportFieldsToFDFTextBuf(fields, bIncludeOrExclude);
  if (text_buf.IsEmpty()) {
    return false;
  }

  if (bUrlEncoded) {
    text_buf = FDFToURLEncodedData(text_buf);
    if (text_buf.IsEmpty()) {
      return false;
    }
  }

  form_fill_env_->SubmitForm(text_buf.unsigned_span(), csDestination);
  return true;
}

ByteString CPDFSDK_InteractiveForm::ExportFieldsToFDFTextBuf(
    const std::vector<CPDF_FormField*>& fields,
    bool bIncludeOrExclude) {
  std::unique_ptr<CFDF_Document> pFDF = interactive_form_->ExportToFDF(
      form_fill_env_->GetFilePath(), fields, bIncludeOrExclude);

  return pFDF ? pFDF->WriteToString() : ByteString();
}

bool CPDFSDK_InteractiveForm::SubmitForm(const WideString& sDestination) {
  if (sDestination.IsEmpty()) {
    return false;
  }

  std::unique_ptr<CFDF_Document> pFDFDoc =
      interactive_form_->ExportToFDF(form_fill_env_->GetFilePath());
  if (!pFDFDoc) {
    return false;
  }

  ByteString fdf_buffer = pFDFDoc->WriteToString();
  if (fdf_buffer.IsEmpty()) {
    return false;
  }

  form_fill_env_->SubmitForm(fdf_buffer.unsigned_span(), sDestination);
  return true;
}

ByteString CPDFSDK_InteractiveForm::ExportFormToFDFTextBuf() {
  std::unique_ptr<CFDF_Document> pFDF =
      interactive_form_->ExportToFDF(form_fill_env_->GetFilePath());

  return pFDF ? pFDF->WriteToString() : ByteString();
}

void CPDFSDK_InteractiveForm::DoAction_ResetForm(const CPDF_Action& action) {
  DCHECK(action.GetDict());
  if (!action.HasFields()) {
    interactive_form_->ResetForm();
    return;
  }
  uint32_t dwFlags = action.GetFlags();
  std::vector<CPDF_FormField*> fields =
      GetFieldFromObjects(action.GetAllFields());
  interactive_form_->ResetForm(fields, !(dwFlags & 0x01));
}

std::vector<CPDF_FormField*> CPDFSDK_InteractiveForm::GetFieldFromObjects(
    const std::vector<RetainPtr<const CPDF_Object>>& objects) const {
  std::vector<CPDF_FormField*> fields;
  for (const CPDF_Object* pObject : objects) {
    if (!pObject || !pObject->IsString()) {
      continue;
    }

    WideString csName = pObject->GetUnicodeText();
    CPDF_FormField* pField = interactive_form_->GetField(0, csName);
    if (pField) {
      fields.push_back(pField);
    }
  }
  return fields;
}

bool CPDFSDK_InteractiveForm::BeforeValueChange(CPDF_FormField* pField,
                                                const WideString& csValue) {
  FormFieldType fieldType = pField->GetFieldType();
  if (!IsFormFieldTypeComboOrText(fieldType)) {
    return true;
  }
  if (!OnKeyStrokeCommit(pField, csValue)) {
    return false;
  }
  return OnValidate(pField, csValue);
}

void CPDFSDK_InteractiveForm::AfterValueChange(CPDF_FormField* pField) {
#ifdef PDF_ENABLE_XFA
  SynchronizeField(pField);
#endif  // PDF_ENABLE_XFA

  FormFieldType fieldType = pField->GetFieldType();
  if (!IsFormFieldTypeComboOrText(fieldType)) {
    return;
  }

  OnCalculate(pField);
  ResetFieldAppearance(pField, OnFormat(pField));
  UpdateField(pField);
}

bool CPDFSDK_InteractiveForm::BeforeSelectionChange(CPDF_FormField* pField,
                                                    const WideString& csValue) {
  if (pField->GetFieldType() != FormFieldType::kListBox) {
    return true;
  }
  if (!OnKeyStrokeCommit(pField, csValue)) {
    return false;
  }
  return OnValidate(pField, csValue);
}

void CPDFSDK_InteractiveForm::AfterSelectionChange(CPDF_FormField* pField) {
  if (pField->GetFieldType() != FormFieldType::kListBox) {
    return;
  }

  OnCalculate(pField);
  ResetFieldAppearance(pField, std::nullopt);
  UpdateField(pField);
}

void CPDFSDK_InteractiveForm::AfterCheckedStatusChange(CPDF_FormField* pField) {
  FormFieldType fieldType = pField->GetFieldType();
  if (fieldType != FormFieldType::kCheckBox &&
      fieldType != FormFieldType::kRadioButton) {
    return;
  }

  OnCalculate(pField);
  UpdateField(pField);
}

void CPDFSDK_InteractiveForm::AfterFormReset(CPDF_InteractiveForm* pForm) {
  OnCalculate(nullptr);
}

bool CPDFSDK_InteractiveForm::IsNeedHighLight(FormFieldType fieldType) const {
  if (fieldType == FormFieldType::kUnknown) {
    return false;
  }

#ifdef PDF_ENABLE_XFA
  // For the XFA fields, we need to return if the specific field type has
  // highlight enabled or if the general XFA field type has it enabled.
  if (IsFormFieldTypeXFA(fieldType)) {
    if (!needs_highlight_[static_cast<size_t>(fieldType)]) {
      return needs_highlight_[static_cast<size_t>(FormFieldType::kXFA)];
    }
  }
#endif  // PDF_ENABLE_XFA
  return needs_highlight_[static_cast<size_t>(fieldType)];
}

void CPDFSDK_InteractiveForm::RemoveAllHighLights() {
  std::fill(std::begin(highlight_color_), std::end(highlight_color_),
            kWhiteBGR);
  std::fill(std::begin(needs_highlight_), std::end(needs_highlight_), false);
}

void CPDFSDK_InteractiveForm::SetHighlightColor(FX_COLORREF clr,
                                                FormFieldType fieldType) {
  if (fieldType == FormFieldType::kUnknown) {
    return;
  }

  highlight_color_[static_cast<size_t>(fieldType)] = clr;
  needs_highlight_[static_cast<size_t>(fieldType)] = true;
}

void CPDFSDK_InteractiveForm::SetAllHighlightColors(FX_COLORREF clr) {
  for (size_t i = 0; i < kFormFieldTypeCount; ++i) {
    highlight_color_[i] = clr;
    needs_highlight_[i] = true;
  }
}

FX_COLORREF CPDFSDK_InteractiveForm::GetHighlightColor(
    FormFieldType fieldType) {
  if (fieldType == FormFieldType::kUnknown) {
    return kWhiteBGR;
  }

#ifdef PDF_ENABLE_XFA
  // For the XFA fields, we need to return the specific field type highlight
  // colour or the general XFA field type colour if present.
  if (IsFormFieldTypeXFA(fieldType)) {
    if (!needs_highlight_[static_cast<size_t>(fieldType)] &&
        needs_highlight_[static_cast<size_t>(FormFieldType::kXFA)]) {
      return highlight_color_[static_cast<size_t>(FormFieldType::kXFA)];
    }
  }
#endif  // PDF_ENABLE_XFA
  return highlight_color_[static_cast<size_t>(fieldType)];
}
