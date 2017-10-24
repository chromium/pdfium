// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Field.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfdoc/cpdf_interform.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/javascript/Document.h"
#include "fpdfsdk/javascript/Icon.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/PublicMethods.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_runtime.h"
#include "fpdfsdk/javascript/color.h"

namespace {

bool SetWidgetDisplayStatus(CPDFSDK_Widget* pWidget, int value) {
  if (!pWidget)
    return false;

  uint32_t dwFlag = pWidget->GetFlags();
  switch (value) {
    case 0:
      dwFlag &= ~ANNOTFLAG_INVISIBLE;
      dwFlag &= ~ANNOTFLAG_HIDDEN;
      dwFlag &= ~ANNOTFLAG_NOVIEW;
      dwFlag |= ANNOTFLAG_PRINT;
      break;
    case 1:
      dwFlag &= ~ANNOTFLAG_INVISIBLE;
      dwFlag &= ~ANNOTFLAG_NOVIEW;
      dwFlag |= (ANNOTFLAG_HIDDEN | ANNOTFLAG_PRINT);
      break;
    case 2:
      dwFlag &= ~ANNOTFLAG_INVISIBLE;
      dwFlag &= ~ANNOTFLAG_PRINT;
      dwFlag &= ~ANNOTFLAG_HIDDEN;
      dwFlag &= ~ANNOTFLAG_NOVIEW;
      break;
    case 3:
      dwFlag |= ANNOTFLAG_NOVIEW;
      dwFlag |= ANNOTFLAG_PRINT;
      dwFlag &= ~ANNOTFLAG_HIDDEN;
      break;
  }

  if (dwFlag != pWidget->GetFlags()) {
    pWidget->SetFlags(dwFlag);
    return true;
  }

  return false;
}

}  // namespace

JSConstSpec CJS_Field::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Field::PropertySpecs[] = {
    {"alignment", get_alignment_static, set_alignment_static},
    {"borderStyle", get_border_style_static, set_border_style_static},
    {"buttonAlignX", get_button_align_x_static, set_button_align_x_static},
    {"buttonAlignY", get_button_align_y_static, set_button_align_y_static},
    {"buttonFitBounds", get_button_fit_bounds_static,
     set_button_fit_bounds_static},
    {"buttonPosition", get_button_position_static, set_button_position_static},
    {"buttonScaleHow", get_button_scale_how_static,
     set_button_scale_how_static},
    {"buttonScaleWhen", get_button_scale_when_static,
     set_button_scale_when_static},
    {"calcOrderIndex", get_calc_order_index_static,
     set_calc_order_index_static},
    {"charLimit", get_char_limit_static, set_char_limit_static},
    {"comb", get_comb_static, set_comb_static},
    {"commitOnSelChange", get_commit_on_sel_change_static,
     set_commit_on_sel_change_static},
    {"currentValueIndices", get_current_value_indices_static,
     set_current_value_indices_static},
    {"defaultStyle", get_default_style_static, set_default_style_static},
    {"defaultValue", get_default_value_static, set_default_value_static},
    {"doNotScroll", get_do_not_scroll_static, set_do_not_scroll_static},
    {"doNotSpellCheck", get_do_not_spell_check_static,
     set_do_not_spell_check_static},
    {"delay", get_delay_static, set_delay_static},
    {"display", get_display_static, set_display_static},
    {"doc", get_doc_static, set_doc_static},
    {"editable", get_editable_static, set_editable_static},
    {"exportValues", get_export_values_static, set_export_values_static},
    {"hidden", get_hidden_static, set_hidden_static},
    {"fileSelect", get_file_select_static, set_file_select_static},
    {"fillColor", get_fill_color_static, set_fill_color_static},
    {"lineWidth", get_line_width_static, set_line_width_static},
    {"highlight", get_highlight_static, set_highlight_static},
    {"multiline", get_multiline_static, set_multiline_static},
    {"multipleSelection", get_multiple_selection_static,
     set_multiple_selection_static},
    {"name", get_name_static, set_name_static},
    {"numItems", get_num_items_static, set_num_items_static},
    {"page", get_page_static, set_page_static},
    {"password", get_password_static, set_password_static},
    {"print", get_print_static, set_print_static},
    {"radiosInUnison", get_radios_in_unison_static,
     set_radios_in_unison_static},
    {"readonly", get_readonly_static, set_readonly_static},
    {"rect", get_rect_static, set_rect_static},
    {"required", get_required_static, set_required_static},
    {"richText", get_rich_text_static, set_rich_text_static},
    {"richValue", get_rich_value_static, set_rich_value_static},
    {"rotation", get_rotation_static, set_rotation_static},
    {"strokeColor", get_stroke_color_static, set_stroke_color_static},
    {"style", get_style_static, set_style_static},
    {"submitName", get_submit_name_static, set_submit_name_static},
    {"textColor", get_text_color_static, set_text_color_static},
    {"textFont", get_text_font_static, set_text_font_static},
    {"textSize", get_text_size_static, set_text_size_static},
    {"type", get_type_static, set_type_static},
    {"userName", get_user_name_static, set_user_name_static},
    {"value", get_value_static, set_value_static},
    {"valueAsString", get_value_as_string_static, set_value_as_string_static},
    {"source", get_source_static, set_source_static},
    {0, 0, 0}};

JSMethodSpec CJS_Field::MethodSpecs[] = {
    {"browseForFileToSubmit", browseForFileToSubmit_static},
    {"buttonGetCaption", buttonGetCaption_static},
    {"buttonGetIcon", buttonGetIcon_static},
    {"buttonImportIcon", buttonImportIcon_static},
    {"buttonSetCaption", buttonSetCaption_static},
    {"buttonSetIcon", buttonSetIcon_static},
    {"checkThisBox", checkThisBox_static},
    {"clearItems", clearItems_static},
    {"defaultIsChecked", defaultIsChecked_static},
    {"deleteItemAt", deleteItemAt_static},
    {"getArray", getArray_static},
    {"getItemAt", getItemAt_static},
    {"getLock", getLock_static},
    {"insertItemAt", insertItemAt_static},
    {"isBoxChecked", isBoxChecked_static},
    {"isDefaultChecked", isDefaultChecked_static},
    {"setAction", setAction_static},
    {"setFocus", setFocus_static},
    {"setItems", setItems_static},
    {"setLock", setLock_static},
    {"signatureGetModifications", signatureGetModifications_static},
    {"signatureGetSeedValue", signatureGetSeedValue_static},
    {"signatureInfo", signatureInfo_static},
    {"signatureSetSeedValue", signatureSetSeedValue_static},
    {"signatureSign", signatureSign_static},
    {"signatureValidate", signatureValidate_static},
    {0, 0}};

IMPLEMENT_JS_CLASS(CJS_Field, Field, Field)

CJS_DelayData::CJS_DelayData(FIELD_PROP prop, int idx, const WideString& name)
    : eProp(prop), nControlIndex(idx), sFieldName(name) {}

CJS_DelayData::~CJS_DelayData() {}

void CJS_Field::InitInstance(IJS_Runtime* pIRuntime) {}

Field::Field(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject),
      m_pJSDoc(nullptr),
      m_pFormFillEnv(nullptr),
      m_nFormControlIndex(-1),
      m_bCanSet(false),
      m_bDelay(false) {}

Field::~Field() {}

// note: iControlNo = -1, means not a widget.
void Field::ParseFieldName(const std::wstring& strFieldNameParsed,
                           std::wstring& strFieldName,
                           int& iControlNo) {
  int iStart = strFieldNameParsed.find_last_of(L'.');
  if (iStart == -1) {
    strFieldName = strFieldNameParsed;
    iControlNo = -1;
    return;
  }
  std::wstring suffixal = strFieldNameParsed.substr(iStart + 1);
  iControlNo = FXSYS_wtoi(suffixal.c_str());
  if (iControlNo == 0) {
    int iSpaceStart;
    while ((iSpaceStart = suffixal.find_last_of(L" ")) != -1) {
      suffixal.erase(iSpaceStart, 1);
    }

    if (suffixal.compare(L"0") != 0) {
      strFieldName = strFieldNameParsed;
      iControlNo = -1;
      return;
    }
  }
  strFieldName = strFieldNameParsed.substr(0, iStart);
}

bool Field::AttachField(Document* pDocument, const WideString& csFieldName) {
  m_pJSDoc = pDocument;
  m_pFormFillEnv.Reset(pDocument->GetFormFillEnv());
  m_bCanSet = m_pFormFillEnv->GetPermissions(FPDFPERM_FILL_FORM) ||
              m_pFormFillEnv->GetPermissions(FPDFPERM_ANNOT_FORM) ||
              m_pFormFillEnv->GetPermissions(FPDFPERM_MODIFY);

  CPDFSDK_InterForm* pRDInterForm = m_pFormFillEnv->GetInterForm();
  CPDF_InterForm* pInterForm = pRDInterForm->GetInterForm();
  WideString swFieldNameTemp = csFieldName;
  swFieldNameTemp.Replace(L"..", L".");

  if (pInterForm->CountFields(swFieldNameTemp) <= 0) {
    std::wstring strFieldName;
    int iControlNo = -1;
    ParseFieldName(swFieldNameTemp.c_str(), strFieldName, iControlNo);
    if (iControlNo == -1)
      return false;

    m_FieldName = strFieldName.c_str();
    m_nFormControlIndex = iControlNo;
    return true;
  }

  m_FieldName = swFieldNameTemp;
  m_nFormControlIndex = -1;

  return true;
}

std::vector<CPDF_FormField*> Field::GetFormFields(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const WideString& csFieldName) {
  std::vector<CPDF_FormField*> fields;
  CPDFSDK_InterForm* pReaderInterForm = pFormFillEnv->GetInterForm();
  CPDF_InterForm* pInterForm = pReaderInterForm->GetInterForm();
  for (int i = 0, sz = pInterForm->CountFields(csFieldName); i < sz; ++i) {
    if (CPDF_FormField* pFormField = pInterForm->GetField(i, csFieldName))
      fields.push_back(pFormField);
  }
  return fields;
}

std::vector<CPDF_FormField*> Field::GetFormFields(
    const WideString& csFieldName) const {
  return Field::GetFormFields(m_pFormFillEnv.Get(), csFieldName);
}

void Field::UpdateFormField(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                            CPDF_FormField* pFormField,
                            bool bChangeMark,
                            bool bResetAP,
                            bool bRefresh) {
  CPDFSDK_InterForm* pInterForm = pFormFillEnv->GetInterForm();

  if (bResetAP) {
    std::vector<CPDFSDK_Annot::ObservedPtr> widgets;
    pInterForm->GetWidgets(pFormField, &widgets);

    int nFieldType = pFormField->GetFieldType();
    if (nFieldType == FIELDTYPE_COMBOBOX || nFieldType == FIELDTYPE_TEXTFIELD) {
      for (auto& pObserved : widgets) {
        if (pObserved) {
          bool bFormatted = false;
          WideString sValue = static_cast<CPDFSDK_Widget*>(pObserved.Get())
                                  ->OnFormat(bFormatted);
          if (pObserved) {  // Not redundant, may be clobbered by OnFormat.
            static_cast<CPDFSDK_Widget*>(pObserved.Get())
                ->ResetAppearance(bFormatted ? &sValue : nullptr, false);
          }
        }
      }
    } else {
      for (auto& pObserved : widgets) {
        if (pObserved) {
          static_cast<CPDFSDK_Widget*>(pObserved.Get())
              ->ResetAppearance(nullptr, false);
        }
      }
    }
  }

  if (bRefresh) {
    // Refresh the widget list. The calls in |bResetAP| may have caused widgets
    // to be removed from the list. We need to call |GetWidgets| again to be
    // sure none of the widgets have been deleted.
    std::vector<CPDFSDK_Annot::ObservedPtr> widgets;
    pInterForm->GetWidgets(pFormField, &widgets);

    // TODO(dsinclair): Determine if all widgets share the same
    // CPDFSDK_InterForm. If that's the case, we can move the code to
    // |GetFormFillEnv| out of the loop.
    for (auto& pObserved : widgets) {
      if (pObserved) {
        CPDFSDK_Widget* pWidget = static_cast<CPDFSDK_Widget*>(pObserved.Get());
        pWidget->GetInterForm()->GetFormFillEnv()->UpdateAllViews(nullptr,
                                                                  pWidget);
      }
    }
  }

  if (bChangeMark)
    pFormFillEnv->SetChangeMark();
}

void Field::UpdateFormControl(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                              CPDF_FormControl* pFormControl,
                              bool bChangeMark,
                              bool bResetAP,
                              bool bRefresh) {
  ASSERT(pFormControl);

  CPDFSDK_InterForm* pForm = pFormFillEnv->GetInterForm();
  CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl);

  if (pWidget) {
    CPDFSDK_Widget::ObservedPtr observed_widget(pWidget);
    if (bResetAP) {
      int nFieldType = pWidget->GetFieldType();
      if (nFieldType == FIELDTYPE_COMBOBOX ||
          nFieldType == FIELDTYPE_TEXTFIELD) {
        bool bFormatted = false;
        WideString sValue = pWidget->OnFormat(bFormatted);
        if (!observed_widget)
          return;
        pWidget->ResetAppearance(bFormatted ? &sValue : nullptr, false);
      } else {
        pWidget->ResetAppearance(nullptr, false);
      }
      if (!observed_widget)
        return;
    }

    if (bRefresh) {
      CPDFSDK_InterForm* pInterForm = pWidget->GetInterForm();
      pInterForm->GetFormFillEnv()->UpdateAllViews(nullptr, pWidget);
    }
  }

  if (bChangeMark)
    pFormFillEnv->SetChangeMark();
}

CPDFSDK_Widget* Field::GetWidget(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                 CPDF_FormControl* pFormControl) {
  CPDFSDK_InterForm* pInterForm =
      static_cast<CPDFSDK_InterForm*>(pFormFillEnv->GetInterForm());
  return pInterForm ? pInterForm->GetWidget(pFormControl) : nullptr;
}

bool Field::ValueIsOccur(CPDF_FormField* pFormField, WideString csOptLabel) {
  for (int i = 0, sz = pFormField->CountOptions(); i < sz; i++) {
    if (csOptLabel.Compare(pFormField->GetOptionLabel(i)) == 0)
      return true;
  }

  return false;
}

CPDF_FormControl* Field::GetSmartFieldControl(CPDF_FormField* pFormField) {
  if (!pFormField->CountControls() ||
      m_nFormControlIndex >= pFormField->CountControls())
    return nullptr;
  if (m_nFormControlIndex < 0)
    return pFormField->GetControl(0);
  return pFormField->GetControl(m_nFormControlIndex);
}

bool Field::get_alignment(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  switch (pFormControl->GetControlAlignment()) {
    case 1:
      vp->Set(pRuntime->NewString(L"center"));
      break;
    case 0:
      vp->Set(pRuntime->NewString(L"left"));
      break;
    case 2:
      vp->Set(pRuntime->NewString(L"right"));
      break;
    default:
      vp->Set(pRuntime->NewString(L""));
  }

  return true;
}

bool Field::set_alignment(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_border_style(CJS_Runtime* pRuntime,
                             CJS_Value* vp,
                             WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (!pFormField)
    return false;

  CPDFSDK_Widget* pWidget =
      GetWidget(m_pFormFillEnv.Get(), GetSmartFieldControl(pFormField));
  if (!pWidget)
    return false;

  switch (pWidget->GetBorderStyle()) {
    case BorderStyle::SOLID:
      vp->Set(pRuntime->NewString(L"solid"));
      break;
    case BorderStyle::DASH:
      vp->Set(pRuntime->NewString(L"dashed"));
      break;
    case BorderStyle::BEVELED:
      vp->Set(pRuntime->NewString(L"beveled"));
      break;
    case BorderStyle::INSET:
      vp->Set(pRuntime->NewString(L"inset"));
      break;
    case BorderStyle::UNDERLINE:
      vp->Set(pRuntime->NewString(L"underline"));
      break;
    default:
      vp->Set(pRuntime->NewString(L""));
      break;
  }
  return true;
}

bool Field::set_border_style(CJS_Runtime* pRuntime,
                             const CJS_Value& vp,
                             WideString* sError) {
  ASSERT(m_pFormFillEnv);

  if (!m_bCanSet)
    return false;

  if (m_bDelay) {
    AddDelay_String(FP_BORDERSTYLE, vp.ToByteString(pRuntime));
  } else {
    Field::SetBorderStyle(m_pFormFillEnv.Get(), m_FieldName,
                          m_nFormControlIndex, vp.ToByteString(pRuntime));
  }
  return true;
}

void Field::SetBorderStyle(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                           const WideString& swFieldName,
                           int nControlIndex,
                           const ByteString& string) {
  ASSERT(pFormFillEnv);

  BorderStyle nBorderStyle = BorderStyle::SOLID;
  if (string == "solid")
    nBorderStyle = BorderStyle::SOLID;
  else if (string == "beveled")
    nBorderStyle = BorderStyle::BEVELED;
  else if (string == "dashed")
    nBorderStyle = BorderStyle::DASH;
  else if (string == "inset")
    nBorderStyle = BorderStyle::INSET;
  else if (string == "underline")
    nBorderStyle = BorderStyle::UNDERLINE;
  else
    return;

  std::vector<CPDF_FormField*> FieldArray =
      GetFormFields(pFormFillEnv, swFieldName);
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bSet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        if (CPDFSDK_Widget* pWidget =
                GetWidget(pFormFillEnv, pFormField->GetControl(i))) {
          if (pWidget->GetBorderStyle() != nBorderStyle) {
            pWidget->SetBorderStyle(nBorderStyle);
            bSet = true;
          }
        }
      }
      if (bSet)
        UpdateFormField(pFormFillEnv, pFormField, true, true, true);
    } else {
      if (nControlIndex >= pFormField->CountControls())
        return;
      if (CPDF_FormControl* pFormControl =
              pFormField->GetControl(nControlIndex)) {
        if (CPDFSDK_Widget* pWidget = GetWidget(pFormFillEnv, pFormControl)) {
          if (pWidget->GetBorderStyle() != nBorderStyle) {
            pWidget->SetBorderStyle(nBorderStyle);
            UpdateFormControl(pFormFillEnv, pFormControl, true, true, true);
          }
        }
      }
    }
  }
}

bool Field::get_button_align_x(CJS_Runtime* pRuntime,
                               CJS_Value* vp,
                               WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  CPDF_IconFit IconFit = pFormControl->GetIconFit();

  float fLeft;
  float fBottom;
  IconFit.GetIconPosition(fLeft, fBottom);

  vp->Set(pRuntime->NewNumber(static_cast<int32_t>(fLeft)));
  return true;
}

bool Field::set_button_align_x(CJS_Runtime* pRuntime,
                               const CJS_Value& vp,
                               WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_button_align_y(CJS_Runtime* pRuntime,
                               CJS_Value* vp,
                               WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  CPDF_IconFit IconFit = pFormControl->GetIconFit();

  float fLeft;
  float fBottom;
  IconFit.GetIconPosition(fLeft, fBottom);

  vp->Set(pRuntime->NewNumber(static_cast<int32_t>(fBottom)));
  return true;
}

bool Field::set_button_align_y(CJS_Runtime* pRuntime,
                               const CJS_Value& vp,
                               WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_button_fit_bounds(CJS_Runtime* pRuntime,
                                  CJS_Value* vp,
                                  WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  vp->Set(pRuntime->NewBoolean(pFormControl->GetIconFit().GetFittingBounds()));
  return true;
}

bool Field::set_button_fit_bounds(CJS_Runtime* pRuntime,
                                  const CJS_Value& vp,
                                  WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_button_position(CJS_Runtime* pRuntime,
                                CJS_Value* vp,
                                WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  vp->Set(pRuntime->NewNumber(pFormControl->GetTextPosition()));
  return true;
}

bool Field::set_button_position(CJS_Runtime* pRuntime,
                                const CJS_Value& vp,
                                WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_button_scale_how(CJS_Runtime* pRuntime,
                                 CJS_Value* vp,
                                 WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  vp->Set(pRuntime->NewBoolean(
      pFormControl->GetIconFit().IsProportionalScale() ? 0 : 1));
  return true;
}

bool Field::set_button_scale_how(CJS_Runtime* pRuntime,
                                 const CJS_Value& vp,
                                 WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_button_scale_when(CJS_Runtime* pRuntime,
                                  CJS_Value* vp,
                                  WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  CPDF_IconFit IconFit = pFormControl->GetIconFit();
  int ScaleM = IconFit.GetScaleMethod();
  switch (ScaleM) {
    case CPDF_IconFit::Always:
      vp->Set(pRuntime->NewNumber(static_cast<int32_t>(CPDF_IconFit::Always)));
      break;
    case CPDF_IconFit::Bigger:
      vp->Set(pRuntime->NewNumber(static_cast<int32_t>(CPDF_IconFit::Bigger)));
      break;
    case CPDF_IconFit::Never:
      vp->Set(pRuntime->NewNumber(static_cast<int32_t>(CPDF_IconFit::Never)));
      break;
    case CPDF_IconFit::Smaller:
      vp->Set(pRuntime->NewNumber(static_cast<int32_t>(CPDF_IconFit::Smaller)));
      break;
  }
  return true;
}

bool Field::set_button_scale_when(CJS_Runtime* pRuntime,
                                  const CJS_Value& vp,
                                  WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_calc_order_index(CJS_Runtime* pRuntime,
                                 CJS_Value* vp,
                                 WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX &&
      pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD) {
    return false;
  }

  CPDFSDK_InterForm* pRDInterForm = m_pFormFillEnv->GetInterForm();
  CPDF_InterForm* pInterForm = pRDInterForm->GetInterForm();
  vp->Set(pRuntime->NewNumber(static_cast<int32_t>(
      pInterForm->FindFieldInCalculationOrder(pFormField))));
  return true;
}

bool Field::set_calc_order_index(CJS_Runtime* pRuntime,
                                 const CJS_Value& vp,
                                 WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_char_limit(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  vp->Set(pRuntime->NewNumber(static_cast<int32_t>(pFormField->GetMaxLen())));
  return true;
}

bool Field::set_char_limit(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_comb(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  vp->Set(
      pRuntime->NewBoolean(!!(pFormField->GetFieldFlags() & FIELDFLAG_COMB)));
  return true;
}

bool Field::set_comb(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_commit_on_sel_change(CJS_Runtime* pRuntime,
                                     CJS_Value* vp,
                                     WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX &&
      pFormField->GetFieldType() != FIELDTYPE_LISTBOX) {
    return false;
  }

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_COMMITONSELCHANGE)));
  return true;
}

bool Field::set_commit_on_sel_change(CJS_Runtime* pRuntime,
                                     const CJS_Value& vp,
                                     WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_current_value_indices(CJS_Runtime* pRuntime,
                                      CJS_Value* vp,
                                      WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX &&
      pFormField->GetFieldType() != FIELDTYPE_LISTBOX) {
    return false;
  }

  int count = pFormField->CountSelectedItems();
  if (count <= 0) {
    vp->Set(pRuntime->NewNumber(-1));
    return true;
  }
  if (count == 1) {
    vp->Set(pRuntime->NewNumber(pFormField->GetSelectedIndex(0)));
    return true;
  }

  CJS_Array SelArray;
  for (int i = 0, sz = pFormField->CountSelectedItems(); i < sz; i++) {
    SelArray.SetElement(
        pRuntime, i,
        CJS_Value(pRuntime->NewNumber(pFormField->GetSelectedIndex(i))));
  }
  vp->Set(SelArray.ToV8Array(pRuntime));

  return true;
}

bool Field::set_current_value_indices(CJS_Runtime* pRuntime,
                                      const CJS_Value& vp,
                                      WideString* sError) {
  if (!m_bCanSet)
    return false;

  std::vector<uint32_t> array;
  if (vp.GetType() == CJS_Value::VT_number) {
    array.push_back(vp.ToInt(pRuntime));
  } else if (vp.IsArrayObject()) {
    CJS_Array SelArray = vp.ToArray(pRuntime);
    for (int i = 0, sz = SelArray.GetLength(pRuntime); i < sz; i++)
      array.push_back(SelArray.GetElement(pRuntime, i).ToInt(pRuntime));
  }

  if (m_bDelay) {
    AddDelay_WordArray(FP_CURRENTVALUEINDICES, array);
  } else {
    Field::SetCurrentValueIndices(m_pFormFillEnv.Get(), m_FieldName,
                                  m_nFormControlIndex, array);
  }
  return true;
}

void Field::SetCurrentValueIndices(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                   const WideString& swFieldName,
                                   int nControlIndex,
                                   const std::vector<uint32_t>& array) {
  ASSERT(pFormFillEnv);
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFields(pFormFillEnv, swFieldName);

  for (CPDF_FormField* pFormField : FieldArray) {
    int nFieldType = pFormField->GetFieldType();
    if (nFieldType == FIELDTYPE_COMBOBOX || nFieldType == FIELDTYPE_LISTBOX) {
      uint32_t dwFieldFlags = pFormField->GetFieldFlags();
      pFormField->ClearSelection(true);
      for (size_t i = 0; i < array.size(); ++i) {
        if (i != 0 && !(dwFieldFlags & (1 << 21)))
          break;
        if (array[i] < static_cast<uint32_t>(pFormField->CountOptions()) &&
            !pFormField->IsItemSelected(array[i])) {
          pFormField->SetItemSelection(array[i], true);
        }
      }
      UpdateFormField(pFormFillEnv, pFormField, true, true, true);
    }
  }
}

bool Field::get_default_style(CJS_Runtime* pRuntime,
                              CJS_Value* vp,
                              WideString* sError) {
  return false;
}

bool Field::set_default_style(CJS_Runtime* pRuntime,
                              const CJS_Value& vp,
                              WideString* sError) {
  return false;
}

bool Field::get_default_value(CJS_Runtime* pRuntime,
                              CJS_Value* vp,
                              WideString* sError) {
  ASSERT(m_pFormFillEnv);

    std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
    if (FieldArray.empty())
      return false;

    CPDF_FormField* pFormField = FieldArray[0];
    if (pFormField->GetFieldType() == FIELDTYPE_PUSHBUTTON ||
        pFormField->GetFieldType() == FIELDTYPE_SIGNATURE) {
      return false;
    }

    vp->Set(pRuntime->NewString(pFormField->GetDefaultValue().c_str()));
    return true;
}

bool Field::set_default_value(CJS_Runtime* pRuntime,
                              const CJS_Value& vp,
                              WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_do_not_scroll(CJS_Runtime* pRuntime,
                              CJS_Value* vp,
                              WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_DONOTSCROLL)));
  return true;
}

bool Field::set_do_not_scroll(CJS_Runtime* pRuntime,
                              const CJS_Value& vp,
                              WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_do_not_spell_check(CJS_Runtime* pRuntime,
                                   CJS_Value* vp,
                                   WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD &&
      pFormField->GetFieldType() != FIELDTYPE_COMBOBOX) {
    return false;
  }

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_DONOTSPELLCHECK)));
  return true;
}

bool Field::set_do_not_spell_check(CJS_Runtime* pRuntime,
                                   const CJS_Value& vp,
                                   WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

void Field::SetDelay(bool bDelay) {
  m_bDelay = bDelay;

  if (m_bDelay)
    return;
  if (m_pJSDoc)
    m_pJSDoc->DoFieldDelay(m_FieldName, m_nFormControlIndex);
}

bool Field::get_delay(CJS_Runtime* pRuntime,
                      CJS_Value* vp,
                      WideString* sError) {
  vp->Set(pRuntime->NewBoolean(m_bDelay));
  return true;
}

bool Field::set_delay(CJS_Runtime* pRuntime,
                      const CJS_Value& vp,
                      WideString* sError) {
  if (!m_bCanSet)
    return false;

  SetDelay(vp.ToBool(pRuntime));
  return true;
}

bool Field::get_display(CJS_Runtime* pRuntime,
                        CJS_Value* vp,
                        WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  ASSERT(pFormField);

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDFSDK_Widget* pWidget =
      pInterForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget)
    return false;

  uint32_t dwFlag = pWidget->GetFlags();
  if (ANNOTFLAG_INVISIBLE & dwFlag || ANNOTFLAG_HIDDEN & dwFlag) {
    vp->Set(pRuntime->NewNumber(1));
    return true;
  }
  if (ANNOTFLAG_PRINT & dwFlag) {
    if (ANNOTFLAG_NOVIEW & dwFlag)
      vp->Set(pRuntime->NewNumber(3));
    else
      vp->Set(pRuntime->NewNumber(0));
  } else {
    vp->Set(pRuntime->NewNumber(2));
  }
  return true;
}

bool Field::set_display(CJS_Runtime* pRuntime,
                        const CJS_Value& vp,
                        WideString* sError) {
  if (!m_bCanSet)
    return false;

  if (m_bDelay) {
    AddDelay_Int(FP_DISPLAY, vp.ToInt(pRuntime));
  } else {
    Field::SetDisplay(m_pFormFillEnv.Get(), m_FieldName, m_nFormControlIndex,
                      vp.ToInt(pRuntime));
  }
  return true;
}

void Field::SetDisplay(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                       const WideString& swFieldName,
                       int nControlIndex,
                       int number) {
  CPDFSDK_InterForm* pInterForm = pFormFillEnv->GetInterForm();
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFields(pFormFillEnv, swFieldName);
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bAnySet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        CPDF_FormControl* pFormControl = pFormField->GetControl(i);
        ASSERT(pFormControl);

        CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl);
        if (SetWidgetDisplayStatus(pWidget, number))
          bAnySet = true;
      }

      if (bAnySet)
        UpdateFormField(pFormFillEnv, pFormField, true, false, true);
    } else {
      if (nControlIndex >= pFormField->CountControls())
        return;

      CPDF_FormControl* pFormControl = pFormField->GetControl(nControlIndex);
      if (!pFormControl)
        return;

      CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl);
      if (SetWidgetDisplayStatus(pWidget, number))
        UpdateFormControl(pFormFillEnv, pFormControl, true, false, true);
    }
  }
}

bool Field::get_doc(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  vp->Set(m_pJSDoc->GetCJSDoc()->ToV8Object());
  return true;
}

bool Field::set_doc(CJS_Runtime* pRuntime,
                    const CJS_Value& vp,
                    WideString* sError) {
  return false;
}

bool Field::get_editable(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX)
    return false;

  vp->Set(
      pRuntime->NewBoolean(!!(pFormField->GetFieldFlags() & FIELDFLAG_EDIT)));
  return true;
}

bool Field::set_editable(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  return m_bCanSet;
}

bool Field::get_export_values(CJS_Runtime* pRuntime,
                              CJS_Value* vp,
                              WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_CHECKBOX &&
      pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON) {
    return false;
  }

  CJS_Array ExportValusArray;
  if (m_nFormControlIndex < 0) {
    for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
      CPDF_FormControl* pFormControl = pFormField->GetControl(i);
      ExportValusArray.SetElement(pRuntime, i,
                                  CJS_Value(pRuntime->NewString(
                                      pFormControl->GetExportValue().c_str())));
    }
  } else {
    if (m_nFormControlIndex >= pFormField->CountControls())
      return false;

    CPDF_FormControl* pFormControl =
        pFormField->GetControl(m_nFormControlIndex);
    if (!pFormControl)
      return false;

    ExportValusArray.SetElement(
        pRuntime, 0,
        CJS_Value(pRuntime->NewString(pFormControl->GetExportValue().c_str())));
  }

  vp->Set(ExportValusArray.ToV8Array(pRuntime));
  return true;
}

bool Field::set_export_values(CJS_Runtime* pRuntime,
                              const CJS_Value& vp,
                              WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_CHECKBOX &&
      pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON) {
    return false;
  }

  return m_bCanSet && vp.IsArrayObject();
}

bool Field::get_file_select(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_FILESELECT)));
  return true;
}

bool Field::set_file_select(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  return m_bCanSet;
}

bool Field::get_fill_color(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  ASSERT(pFormField);
  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  int iColorType;
  pFormControl->GetBackgroundColor(iColorType);

  CFX_Color color;
  if (iColorType == CFX_Color::kTransparent) {
    color = CFX_Color(CFX_Color::kTransparent);
  } else if (iColorType == CFX_Color::kGray) {
    color = CFX_Color(CFX_Color::kGray,
                      pFormControl->GetOriginalBackgroundColor(0));
  } else if (iColorType == CFX_Color::kRGB) {
    color =
        CFX_Color(CFX_Color::kRGB, pFormControl->GetOriginalBackgroundColor(0),
                  pFormControl->GetOriginalBackgroundColor(1),
                  pFormControl->GetOriginalBackgroundColor(2));
  } else if (iColorType == CFX_Color::kCMYK) {
    color =
        CFX_Color(CFX_Color::kCMYK, pFormControl->GetOriginalBackgroundColor(0),
                  pFormControl->GetOriginalBackgroundColor(1),
                  pFormControl->GetOriginalBackgroundColor(2),
                  pFormControl->GetOriginalBackgroundColor(3));
  } else {
    return false;
  }

  vp->Set(color::ConvertPWLColorToArray(pRuntime, color).ToV8Array(pRuntime));
  return true;
}

bool Field::set_fill_color(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;
  if (!m_bCanSet)
    return false;
  if (!vp.IsArrayObject())
    return false;
  return true;
}

bool Field::get_hidden(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  ASSERT(pFormField);

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDFSDK_Widget* pWidget =
      pInterForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget)
    return false;

  uint32_t dwFlags = pWidget->GetFlags();
  vp->Set(pRuntime->NewBoolean(ANNOTFLAG_INVISIBLE & dwFlags ||
                               ANNOTFLAG_HIDDEN & dwFlags));
  return true;
}

bool Field::set_hidden(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError) {
  if (!m_bCanSet)
    return false;

  if (m_bDelay) {
    AddDelay_Bool(FP_HIDDEN, vp.ToBool(pRuntime));
  } else {
    Field::SetHidden(m_pFormFillEnv.Get(), m_FieldName, m_nFormControlIndex,
                     vp.ToBool(pRuntime));
  }
  return true;
}

void Field::SetHidden(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                      const WideString& swFieldName,
                      int nControlIndex,
                      bool b) {
  int display = b ? 1 /*Hidden*/ : 0 /*Visible*/;
  SetDisplay(pFormFillEnv, swFieldName, nControlIndex, display);
}

bool Field::get_highlight(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  int eHM = pFormControl->GetHighlightingMode();
  switch (eHM) {
    case CPDF_FormControl::None:
      vp->Set(pRuntime->NewString(L"none"));
      break;
    case CPDF_FormControl::Push:
      vp->Set(pRuntime->NewString(L"push"));
      break;
    case CPDF_FormControl::Invert:
      vp->Set(pRuntime->NewString(L"invert"));
      break;
    case CPDF_FormControl::Outline:
      vp->Set(pRuntime->NewString(L"outline"));
      break;
    case CPDF_FormControl::Toggle:
      vp->Set(pRuntime->NewString(L"toggle"));
      break;
  }
  return true;
}

bool Field::set_highlight(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_line_width(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  ASSERT(pFormField);

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  if (!pFormField->CountControls())
    return false;

  CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormField->GetControl(0));
  if (!pWidget)
    return false;

  vp->Set(pRuntime->NewNumber(pWidget->GetBorderWidth()));
  return true;
}

bool Field::set_line_width(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  if (!m_bCanSet)
    return false;

  if (m_bDelay) {
    AddDelay_Int(FP_LINEWIDTH, vp.ToInt(pRuntime));
  } else {
    Field::SetLineWidth(m_pFormFillEnv.Get(), m_FieldName, m_nFormControlIndex,
                        vp.ToInt(pRuntime));
  }
  return true;
}

void Field::SetLineWidth(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                         const WideString& swFieldName,
                         int nControlIndex,
                         int number) {
  CPDFSDK_InterForm* pInterForm = pFormFillEnv->GetInterForm();
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFields(pFormFillEnv, swFieldName);
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bSet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        CPDF_FormControl* pFormControl = pFormField->GetControl(i);
        ASSERT(pFormControl);

        if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl)) {
          if (number != pWidget->GetBorderWidth()) {
            pWidget->SetBorderWidth(number);
            bSet = true;
          }
        }
      }
      if (bSet)
        UpdateFormField(pFormFillEnv, pFormField, true, true, true);
    } else {
      if (nControlIndex >= pFormField->CountControls())
        return;
      if (CPDF_FormControl* pFormControl =
              pFormField->GetControl(nControlIndex)) {
        if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl)) {
          if (number != pWidget->GetBorderWidth()) {
            pWidget->SetBorderWidth(number);
            UpdateFormControl(pFormFillEnv, pFormControl, true, true, true);
          }
        }
      }
    }
  }
}

bool Field::get_multiline(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_MULTILINE)));
  return true;
}

bool Field::set_multiline(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_multiple_selection(CJS_Runtime* pRuntime,
                                   CJS_Value* vp,
                                   WideString* sError) {
  ASSERT(m_pFormFillEnv);
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_LISTBOX)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_MULTISELECT)));
  return true;
}

bool Field::set_multiple_selection(CJS_Runtime* pRuntime,
                                   const CJS_Value& vp,
                                   WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_name(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  vp->Set(pRuntime->NewString(m_FieldName.c_str()));
  return true;
}

bool Field::set_name(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  return false;
}

bool Field::get_num_items(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX &&
      pFormField->GetFieldType() != FIELDTYPE_LISTBOX) {
    return false;
  }

  vp->Set(pRuntime->NewNumber(pFormField->CountOptions()));
  return true;
}

bool Field::set_num_items(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  return false;
}

bool Field::get_page(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (!pFormField)
    return false;

  std::vector<CPDFSDK_Annot::ObservedPtr> widgets;
  m_pFormFillEnv->GetInterForm()->GetWidgets(pFormField, &widgets);
  if (widgets.empty()) {
    vp->Set(pRuntime->NewNumber(-1));
    return true;
  }

  CJS_Array PageArray;
  int i = 0;
  for (const auto& pObserved : widgets) {
    if (!pObserved) {
      *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
      return false;
    }

    auto* pWidget = static_cast<CPDFSDK_Widget*>(pObserved.Get());
    CPDFSDK_PageView* pPageView = pWidget->GetPageView();
    if (!pPageView)
      return false;

    PageArray.SetElement(pRuntime, i,
                         CJS_Value(pRuntime->NewNumber(
                             static_cast<int32_t>(pPageView->GetPageIndex()))));
    ++i;
  }

  vp->Set(PageArray.ToV8Array(pRuntime));
  return true;
}

bool Field::set_page(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

bool Field::get_password(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_PASSWORD)));
  return true;
}

bool Field::set_password(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_print(CJS_Runtime* pRuntime,
                      CJS_Value* vp,
                      WideString* sError) {
  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  CPDFSDK_Widget* pWidget =
      pInterForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget)
    return false;

  vp->Set(pRuntime->NewBoolean(!!(pWidget->GetFlags() & ANNOTFLAG_PRINT)));
  return true;
}

bool Field::set_print(CJS_Runtime* pRuntime,
                      const CJS_Value& vp,
                      WideString* sError) {
  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

    if (!m_bCanSet)
      return false;

    for (CPDF_FormField* pFormField : FieldArray) {
      if (m_nFormControlIndex < 0) {
        bool bSet = false;
        for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
          if (CPDFSDK_Widget* pWidget =
                  pInterForm->GetWidget(pFormField->GetControl(i))) {
            uint32_t dwFlags = pWidget->GetFlags();
            if (vp.ToBool(pRuntime))
              dwFlags |= ANNOTFLAG_PRINT;
            else
              dwFlags &= ~ANNOTFLAG_PRINT;

            if (dwFlags != pWidget->GetFlags()) {
              pWidget->SetFlags(dwFlags);
              bSet = true;
            }
          }
        }

        if (bSet)
          UpdateFormField(m_pFormFillEnv.Get(), pFormField, true, false, true);

        continue;
      }

      if (m_nFormControlIndex >= pFormField->CountControls())
        return false;
      if (CPDF_FormControl* pFormControl =
              pFormField->GetControl(m_nFormControlIndex)) {
        if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl)) {
          uint32_t dwFlags = pWidget->GetFlags();
          if (vp.ToBool(pRuntime))
            dwFlags |= ANNOTFLAG_PRINT;
          else
            dwFlags &= ~ANNOTFLAG_PRINT;

          if (dwFlags != pWidget->GetFlags()) {
            pWidget->SetFlags(dwFlags);
            UpdateFormControl(m_pFormFillEnv.Get(),
                              pFormField->GetControl(m_nFormControlIndex), true,
                              false, true);
          }
        }
      }
    }
    return true;
}

bool Field::get_radios_in_unison(CJS_Runtime* pRuntime,
                                 CJS_Value* vp,
                                 WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_RADIOSINUNISON)));
  return true;
}

bool Field::set_radios_in_unison(CJS_Runtime* pRuntime,
                                 const CJS_Value& vp,
                                 WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;
  return m_bCanSet;
}

bool Field::get_readonly(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(FieldArray[0]->GetFieldFlags() & FIELDFLAG_READONLY)));
  return true;
}

bool Field::set_readonly(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;
  return m_bCanSet;
}

bool Field::get_rect(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDFSDK_Widget* pWidget =
      pInterForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget)
    return false;

  CFX_FloatRect crRect = pWidget->GetRect();
  CJS_Array rcArray;
  rcArray.SetElement(
      pRuntime, 0,
      CJS_Value(pRuntime->NewNumber(static_cast<int32_t>(crRect.left))));
  rcArray.SetElement(
      pRuntime, 1,
      CJS_Value(pRuntime->NewNumber(static_cast<int32_t>(crRect.top))));
  rcArray.SetElement(
      pRuntime, 2,
      CJS_Value(pRuntime->NewNumber(static_cast<int32_t>(crRect.right))));
  rcArray.SetElement(
      pRuntime, 3,
      CJS_Value(pRuntime->NewNumber(static_cast<int32_t>(crRect.bottom))));
  vp->Set(rcArray.ToV8Array(pRuntime));
  return true;
}

bool Field::set_rect(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  if (!m_bCanSet)
    return false;
  if (!vp.IsArrayObject())
    return false;

  CJS_Array rcArray = vp.ToArray(pRuntime);
  float pArray[4];
  pArray[0] =
      static_cast<float>(rcArray.GetElement(pRuntime, 0).ToInt(pRuntime));
  pArray[1] =
      static_cast<float>(rcArray.GetElement(pRuntime, 1).ToInt(pRuntime));
  pArray[2] =
      static_cast<float>(rcArray.GetElement(pRuntime, 2).ToInt(pRuntime));
  pArray[3] =
      static_cast<float>(rcArray.GetElement(pRuntime, 3).ToInt(pRuntime));

  CFX_FloatRect crRect(pArray);
  if (m_bDelay) {
    AddDelay_Rect(FP_RECT, crRect);
  } else {
    Field::SetRect(m_pFormFillEnv.Get(), m_FieldName, m_nFormControlIndex,
                   crRect);
  }
  return true;
}

void Field::SetRect(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                    const WideString& swFieldName,
                    int nControlIndex,
                    const CFX_FloatRect& rect) {
  CPDFSDK_InterForm* pInterForm = pFormFillEnv->GetInterForm();
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFields(pFormFillEnv, swFieldName);
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bSet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        CPDF_FormControl* pFormControl = pFormField->GetControl(i);
        ASSERT(pFormControl);

        if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl)) {
          CFX_FloatRect crRect = rect;

          CPDF_Page* pPDFPage = pWidget->GetPDFPage();
          crRect.Intersect(pPDFPage->GetPageBBox());

          if (!crRect.IsEmpty()) {
            CFX_FloatRect rcOld = pWidget->GetRect();
            if (crRect.left != rcOld.left || crRect.right != rcOld.right ||
                crRect.top != rcOld.top || crRect.bottom != rcOld.bottom) {
              pWidget->SetRect(crRect);
              bSet = true;
            }
          }
        }
      }

      if (bSet)
        UpdateFormField(pFormFillEnv, pFormField, true, true, true);

      continue;
    }

    if (nControlIndex >= pFormField->CountControls())
      return;
    if (CPDF_FormControl* pFormControl =
            pFormField->GetControl(nControlIndex)) {
      if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl)) {
        CFX_FloatRect crRect = rect;

        CPDF_Page* pPDFPage = pWidget->GetPDFPage();
        crRect.Intersect(pPDFPage->GetPageBBox());

        if (!crRect.IsEmpty()) {
          CFX_FloatRect rcOld = pWidget->GetRect();
          if (crRect.left != rcOld.left || crRect.right != rcOld.right ||
              crRect.top != rcOld.top || crRect.bottom != rcOld.bottom) {
            pWidget->SetRect(crRect);
            UpdateFormControl(pFormFillEnv, pFormControl, true, true, true);
          }
        }
      }
    }
  }
}

bool Field::get_required(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() == FIELDTYPE_PUSHBUTTON)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_REQUIRED)));
  return true;
}

bool Field::set_required(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  return m_bCanSet;
}

bool Field::get_rich_text(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
    return false;

  vp->Set(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & FIELDFLAG_RICHTEXT)));
  return true;
}

bool Field::set_rich_text(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_rich_value(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  return true;
}

bool Field::set_rich_value(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return true;
}

bool Field::get_rotation(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  vp->Set(pRuntime->NewNumber(pFormControl->GetRotation()));
  return true;
}

bool Field::set_rotation(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_stroke_color(CJS_Runtime* pRuntime,
                             CJS_Value* vp,
                             WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  int iColorType;
  pFormControl->GetBorderColor(iColorType);

  CFX_Color color;
  if (iColorType == CFX_Color::kTransparent) {
    color = CFX_Color(CFX_Color::kTransparent);
  } else if (iColorType == CFX_Color::kGray) {
    color =
        CFX_Color(CFX_Color::kGray, pFormControl->GetOriginalBorderColor(0));
  } else if (iColorType == CFX_Color::kRGB) {
    color = CFX_Color(CFX_Color::kRGB, pFormControl->GetOriginalBorderColor(0),
                      pFormControl->GetOriginalBorderColor(1),
                      pFormControl->GetOriginalBorderColor(2));
  } else if (iColorType == CFX_Color::kCMYK) {
    color = CFX_Color(CFX_Color::kCMYK, pFormControl->GetOriginalBorderColor(0),
                      pFormControl->GetOriginalBorderColor(1),
                      pFormControl->GetOriginalBorderColor(2),
                      pFormControl->GetOriginalBorderColor(3));
  } else {
    return false;
  }

  vp->Set(color::ConvertPWLColorToArray(pRuntime, color).ToV8Array(pRuntime));
  return true;
}

bool Field::set_stroke_color(CJS_Runtime* pRuntime,
                             const CJS_Value& vp,
                             WideString* sError) {
  if (!m_bCanSet)
    return false;
  if (!vp.IsArrayObject())
    return false;
  return true;
}

bool Field::get_style(CJS_Runtime* pRuntime,
                      CJS_Value* vp,
                      WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON &&
      pFormField->GetFieldType() != FIELDTYPE_CHECKBOX) {
    return false;
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  WideString csWCaption = pFormControl->GetNormalCaption();
  ByteString csBCaption;

  switch (csWCaption[0]) {
    case L'l':
      csBCaption = "circle";
      break;
    case L'8':
      csBCaption = "cross";
      break;
    case L'u':
      csBCaption = "diamond";
      break;
    case L'n':
      csBCaption = "square";
      break;
    case L'H':
      csBCaption = "star";
      break;
    default:  // L'4'
      csBCaption = "check";
      break;
  }
  vp->Set(
      pRuntime->NewString(WideString::FromLocal(csBCaption.c_str()).c_str()));
  return true;
}

bool Field::set_style(CJS_Runtime* pRuntime,
                      const CJS_Value& vp,
                      WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_submit_name(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  return true;
}

bool Field::set_submit_name(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return true;
}

bool Field::get_text_color(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  int iColorType;
  FX_ARGB color;
  CPDF_DefaultAppearance FieldAppearance = pFormControl->GetDefaultAppearance();
  FieldAppearance.GetColor(color, iColorType);

  int32_t a;
  int32_t r;
  int32_t g;
  int32_t b;
  std::tie(a, r, g, b) = ArgbDecode(color);

  CFX_Color crRet =
      CFX_Color(CFX_Color::kRGB, r / 255.0f, g / 255.0f, b / 255.0f);

  if (iColorType == CFX_Color::kTransparent)
    crRet = CFX_Color(CFX_Color::kTransparent);

  vp->Set(color::ConvertPWLColorToArray(pRuntime, crRet).ToV8Array(pRuntime));
  return true;
}

bool Field::set_text_color(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  if (!m_bCanSet)
    return false;
  if (!vp.IsArrayObject())
    return false;
  return true;
}

bool Field::get_text_font(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  ASSERT(pFormField);
  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  int nFieldType = pFormField->GetFieldType();
  if (nFieldType != FIELDTYPE_PUSHBUTTON && nFieldType != FIELDTYPE_COMBOBOX &&
      nFieldType != FIELDTYPE_LISTBOX && nFieldType != FIELDTYPE_TEXTFIELD) {
    return false;
  }
  CPDF_Font* pFont = pFormControl->GetDefaultControlFont();
  if (!pFont)
    return false;

  vp->Set(pRuntime->NewString(
      WideString::FromLocal(pFont->GetBaseFont().c_str()).c_str()));
  return true;
}

bool Field::set_text_font(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  if (!m_bCanSet)
    return false;

  ByteString fontName = vp.ToByteString(pRuntime);
  return !fontName.IsEmpty();
}

bool Field::get_text_size(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  ASSERT(pFormField);
  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  float fFontSize;
  CPDF_DefaultAppearance FieldAppearance = pFormControl->GetDefaultAppearance();
  FieldAppearance.GetFont(&fFontSize);
  vp->Set(pRuntime->NewNumber(static_cast<int>(fFontSize)));
  return true;
}

bool Field::set_text_size(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_type(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  switch (pFormField->GetFieldType()) {
    case FIELDTYPE_UNKNOWN:
      vp->Set(pRuntime->NewString(L"unknown"));
      break;
    case FIELDTYPE_PUSHBUTTON:
      vp->Set(pRuntime->NewString(L"button"));
      break;
    case FIELDTYPE_CHECKBOX:
      vp->Set(pRuntime->NewString(L"checkbox"));
      break;
    case FIELDTYPE_RADIOBUTTON:
      vp->Set(pRuntime->NewString(L"radiobutton"));
      break;
    case FIELDTYPE_COMBOBOX:
      vp->Set(pRuntime->NewString(L"combobox"));
      break;
    case FIELDTYPE_LISTBOX:
      vp->Set(pRuntime->NewString(L"listbox"));
      break;
    case FIELDTYPE_TEXTFIELD:
      vp->Set(pRuntime->NewString(L"text"));
      break;
    case FIELDTYPE_SIGNATURE:
      vp->Set(pRuntime->NewString(L"signature"));
      break;
    default:
      vp->Set(pRuntime->NewString(L"unknown"));
      break;
  }
  return true;
}

bool Field::set_type(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  return false;
}

bool Field::get_user_name(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  vp->Set(pRuntime->NewString(FieldArray[0]->GetAlternateName().c_str()));
  return true;
}

bool Field::set_user_name(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  ASSERT(m_pFormFillEnv);
  return m_bCanSet;
}

bool Field::get_value(CJS_Runtime* pRuntime,
                      CJS_Value* vp,
                      WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  switch (pFormField->GetFieldType()) {
    case FIELDTYPE_PUSHBUTTON:
      return false;
    case FIELDTYPE_COMBOBOX:
    case FIELDTYPE_TEXTFIELD:
      vp->Set(pRuntime->NewString(pFormField->GetValue().c_str()));
      break;
    case FIELDTYPE_LISTBOX: {
      if (pFormField->CountSelectedItems() > 1) {
        CJS_Array ValueArray;
        CJS_Value ElementValue;
        int iIndex;
        for (int i = 0, sz = pFormField->CountSelectedItems(); i < sz; i++) {
          iIndex = pFormField->GetSelectedIndex(i);
          ElementValue = CJS_Value(
              pRuntime->NewString(pFormField->GetOptionValue(iIndex).c_str()));
          if (wcslen(ElementValue.ToWideString(pRuntime).c_str()) == 0) {
            ElementValue = CJS_Value(pRuntime->NewString(
                pFormField->GetOptionLabel(iIndex).c_str()));
          }
          ValueArray.SetElement(pRuntime, i, ElementValue);
        }
        vp->Set(ValueArray.ToV8Array(pRuntime));
      } else {
        vp->Set(pRuntime->NewString(pFormField->GetValue().c_str()));
      }
      break;
    }
    case FIELDTYPE_CHECKBOX:
    case FIELDTYPE_RADIOBUTTON: {
      bool bFind = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
        if (pFormField->GetControl(i)->IsChecked()) {
          vp->Set(pRuntime->NewString(
              pFormField->GetControl(i)->GetExportValue().c_str()));
          bFind = true;
          break;
        }
      }
      if (!bFind)
        vp->Set(pRuntime->NewString(L"Off"));

      break;
    }
    default:
      vp->Set(pRuntime->NewString(pFormField->GetValue().c_str()));
      break;
  }
  vp->MaybeCoerceToNumber(pRuntime);
  return true;
}

bool Field::set_value(CJS_Runtime* pRuntime,
                      const CJS_Value& vp,
                      WideString* sError) {
  if (!m_bCanSet)
    return false;

  std::vector<WideString> strArray;
  if (vp.IsArrayObject()) {
    CJS_Array ValueArray(vp.ToArray(pRuntime));
    for (int i = 0, sz = ValueArray.GetLength(pRuntime); i < sz; i++) {
      CJS_Value ElementValue(ValueArray.GetElement(pRuntime, i));
      strArray.push_back(ElementValue.ToWideString(pRuntime));
    }
  } else {
    strArray.push_back(vp.ToWideString(pRuntime));
  }

  if (m_bDelay) {
    AddDelay_WideStringArray(FP_VALUE, strArray);
  } else {
    Field::SetValue(m_pFormFillEnv.Get(), m_FieldName, m_nFormControlIndex,
                    strArray);
  }
  return true;
}

void Field::SetValue(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                     const WideString& swFieldName,
                     int nControlIndex,
                     const std::vector<WideString>& strArray) {
  ASSERT(pFormFillEnv);
  if (strArray.empty())
    return;

  std::vector<CPDF_FormField*> FieldArray =
      GetFormFields(pFormFillEnv, swFieldName);

  for (CPDF_FormField* pFormField : FieldArray) {
    if (pFormField->GetFullName().Compare(swFieldName) != 0)
      continue;

    switch (pFormField->GetFieldType()) {
      case FIELDTYPE_TEXTFIELD:
      case FIELDTYPE_COMBOBOX:
        if (pFormField->GetValue() != strArray[0]) {
          pFormField->SetValue(strArray[0], true);
          UpdateFormField(pFormFillEnv, pFormField, true, false, true);
        }
        break;
      case FIELDTYPE_CHECKBOX:
      case FIELDTYPE_RADIOBUTTON: {
        if (pFormField->GetValue() != strArray[0]) {
          pFormField->SetValue(strArray[0], true);
          UpdateFormField(pFormFillEnv, pFormField, true, false, true);
        }
      } break;
      case FIELDTYPE_LISTBOX: {
        bool bModified = false;
        for (const auto& str : strArray) {
          if (!pFormField->IsItemSelected(pFormField->FindOption(str))) {
            bModified = true;
            break;
          }
        }
        if (bModified) {
          pFormField->ClearSelection(true);
          for (const auto& str : strArray) {
            int index = pFormField->FindOption(str);
            if (!pFormField->IsItemSelected(index))
              pFormField->SetItemSelection(index, true, true);
          }
          UpdateFormField(pFormFillEnv, pFormField, true, false, true);
        }
      } break;
      default:
        break;
    }
  }
}

bool Field::get_value_as_string(CJS_Runtime* pRuntime,
                                CJS_Value* vp,
                                WideString* sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() == FIELDTYPE_PUSHBUTTON)
    return false;

  if (pFormField->GetFieldType() == FIELDTYPE_CHECKBOX) {
    if (!pFormField->CountControls())
      return false;

    vp->Set(pRuntime->NewString(
        pFormField->GetControl(0)->IsChecked() ? L"Yes" : L"Off"));
    return true;
  }

  if (pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON &&
      !(pFormField->GetFieldFlags() & FIELDFLAG_RADIOSINUNISON)) {
    for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
      if (pFormField->GetControl(i)->IsChecked()) {
        vp->Set(pRuntime->NewString(
            pFormField->GetControl(i)->GetExportValue().c_str()));
        break;
      } else {
        vp->Set(pRuntime->NewString(L"Off"));
      }
    }
    return true;
  }

  if (pFormField->GetFieldType() == FIELDTYPE_LISTBOX &&
      (pFormField->CountSelectedItems() > 1)) {
    vp->Set(pRuntime->NewString(L""));
  } else {
    vp->Set(pRuntime->NewString(pFormField->GetValue().c_str()));
  }

  return true;
}

bool Field::set_value_as_string(CJS_Runtime* pRuntime,
                                const CJS_Value& vp,
                                WideString* sError) {
  return false;
}

bool Field::browseForFileToSubmit(CJS_Runtime* pRuntime,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  WideString& sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if ((pFormField->GetFieldFlags() & FIELDFLAG_FILESELECT) &&
      (pFormField->GetFieldType() == FIELDTYPE_TEXTFIELD)) {
    WideString wsFileName = m_pFormFillEnv->JS_fieldBrowse();
    if (!wsFileName.IsEmpty()) {
      pFormField->SetValue(wsFileName);
      UpdateFormField(m_pFormFillEnv.Get(), pFormField, true, true, true);
    }
    return true;
  }
  return false;
}

bool Field::buttonGetCaption(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError) {
  int nface = 0;
  int iSize = params.size();
  if (iSize >= 1)
    nface = params[0].ToInt(pRuntime);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  if (nface == 0)
    vRet = CJS_Value(
        pRuntime->NewString(pFormControl->GetNormalCaption().c_str()));
  else if (nface == 1)
    vRet =
        CJS_Value(pRuntime->NewString(pFormControl->GetDownCaption().c_str()));
  else if (nface == 2)
    vRet = CJS_Value(
        pRuntime->NewString(pFormControl->GetRolloverCaption().c_str()));
  else
    return false;

  return true;
}

bool Field::buttonGetIcon(CJS_Runtime* pRuntime,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          WideString& sError) {
  if (params.size() >= 1) {
    int nFace = params[0].ToInt(pRuntime);
    if (nFace < 0 || nFace > 2)
      return false;
  }

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
    return false;

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl)
    return false;

  v8::Local<v8::Object> pObj =
      pRuntime->NewFxDynamicObj(CJS_Icon::g_nObjDefnID);
  if (pObj.IsEmpty())
    return false;

  CJS_Icon* pJS_Icon = static_cast<CJS_Icon*>(pRuntime->GetObjectPrivate(pObj));
  if (pJS_Icon)
    vRet = CJS_Value(pJS_Icon->ToV8Object());

  return true;
}

bool Field::buttonImportIcon(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError) {
  return true;
}

bool Field::buttonSetCaption(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError) {
  return false;
}

bool Field::buttonSetIcon(CJS_Runtime* pRuntime,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          WideString& sError) {
  return false;
}

bool Field::checkThisBox(CJS_Runtime* pRuntime,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         WideString& sError) {
  int iSize = params.size();
  if (iSize < 1)
    return false;

  if (!m_bCanSet)
    return false;

  int nWidget = params[0].ToInt(pRuntime);
  bool bCheckit = true;
  if (iSize >= 2)
    bCheckit = params[1].ToBool(pRuntime);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (pFormField->GetFieldType() != FIELDTYPE_CHECKBOX &&
      pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON)
    return false;
  if (nWidget < 0 || nWidget >= pFormField->CountControls())
    return false;
  // TODO(weili): Check whether anything special needed for radio button,
  // otherwise merge these branches.
  if (pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON)
    pFormField->CheckControl(nWidget, bCheckit, true);
  else
    pFormField->CheckControl(nWidget, bCheckit, true);

  UpdateFormField(m_pFormFillEnv.Get(), pFormField, true, true, true);
  return true;
}

bool Field::clearItems(CJS_Runtime* pRuntime,
                       const std::vector<CJS_Value>& params,
                       CJS_Value& vRet,
                       WideString& sError) {
  return true;
}

bool Field::defaultIsChecked(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError) {
  if (!m_bCanSet)
    return false;

  int iSize = params.size();
  if (iSize < 1)
    return false;

  int nWidget = params[0].ToInt(pRuntime);
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (nWidget < 0 || nWidget >= pFormField->CountControls())
    return false;

  vRet = CJS_Value(pRuntime->NewBoolean(
      pFormField->GetFieldType() == FIELDTYPE_CHECKBOX ||
      pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON));

  return true;
}

bool Field::deleteItemAt(CJS_Runtime* pRuntime,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         WideString& sError) {
  return true;
}

bool Field::getArray(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  std::vector<std::unique_ptr<WideString>> swSort;
  for (CPDF_FormField* pFormField : FieldArray) {
    swSort.push_back(
        std::unique_ptr<WideString>(new WideString(pFormField->GetFullName())));
  }

  std::sort(swSort.begin(), swSort.end(),
            [](const std::unique_ptr<WideString>& p1,
               const std::unique_ptr<WideString>& p2) { return *p1 < *p2; });

  CJS_Array FormFieldArray;

  int j = 0;
  for (const auto& pStr : swSort) {
    v8::Local<v8::Object> pObj =
        pRuntime->NewFxDynamicObj(CJS_Field::g_nObjDefnID);
    if (pObj.IsEmpty())
      return false;

    CJS_Field* pJSField =
        static_cast<CJS_Field*>(pRuntime->GetObjectPrivate(pObj));
    Field* pField = static_cast<Field*>(pJSField->GetEmbedObject());
    pField->AttachField(m_pJSDoc, *pStr);
    FormFieldArray.SetElement(
        pRuntime, j++,
        pJSField ? CJS_Value(pJSField->ToV8Object()) : CJS_Value());
  }

  vRet = CJS_Value(FormFieldArray.ToV8Array(pRuntime));
  return true;
}

bool Field::getItemAt(CJS_Runtime* pRuntime,
                      const std::vector<CJS_Value>& params,
                      CJS_Value& vRet,
                      WideString& sError) {
  int iSize = params.size();
  int nIdx = -1;
  if (iSize >= 1)
    nIdx = params[0].ToInt(pRuntime);

  bool bExport = true;
  if (iSize >= 2)
    bExport = params[1].ToBool(pRuntime);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if ((pFormField->GetFieldType() == FIELDTYPE_LISTBOX) ||
      (pFormField->GetFieldType() == FIELDTYPE_COMBOBOX)) {
    if (nIdx == -1 || nIdx > pFormField->CountOptions())
      nIdx = pFormField->CountOptions() - 1;
    if (bExport) {
      WideString strval = pFormField->GetOptionValue(nIdx);
      if (strval.IsEmpty())
        vRet = CJS_Value(
            pRuntime->NewString(pFormField->GetOptionLabel(nIdx).c_str()));
      else
        vRet = CJS_Value(pRuntime->NewString(strval.c_str()));
    } else {
      vRet = CJS_Value(
          pRuntime->NewString(pFormField->GetOptionLabel(nIdx).c_str()));
    }
  } else {
    return false;
  }

  return true;
}

bool Field::getLock(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError) {
  return false;
}

bool Field::insertItemAt(CJS_Runtime* pRuntime,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         WideString& sError) {
  return true;
}

bool Field::isBoxChecked(CJS_Runtime* pRuntime,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         WideString& sError) {
  int nIndex = -1;
  if (params.size() >= 1)
    nIndex = params[0].ToInt(pRuntime);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (nIndex < 0 || nIndex >= pFormField->CountControls())
    return false;

  vRet = CJS_Value(pRuntime->NewBoolean(
      ((pFormField->GetFieldType() == FIELDTYPE_CHECKBOX ||
        pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON) &&
       pFormField->GetControl(nIndex)->IsChecked() != 0)));
  return true;
}

bool Field::isDefaultChecked(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError) {
  int nIndex = -1;
  if (params.size() >= 1)
    nIndex = params[0].ToInt(pRuntime);

  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  if (nIndex < 0 || nIndex >= pFormField->CountControls())
    return false;

  vRet = CJS_Value(pRuntime->NewBoolean(
      ((pFormField->GetFieldType() == FIELDTYPE_CHECKBOX ||
        pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON) &&
       pFormField->GetControl(nIndex)->IsDefaultChecked() != 0)));
  return true;
}

bool Field::setAction(CJS_Runtime* pRuntime,
                      const std::vector<CJS_Value>& params,
                      CJS_Value& vRet,
                      WideString& sError) {
  return true;
}

bool Field::setFocus(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields(m_FieldName);
  if (FieldArray.empty())
    return false;

  CPDF_FormField* pFormField = FieldArray[0];
  int32_t nCount = pFormField->CountControls();
  if (nCount < 1)
    return false;

  CPDFSDK_InterForm* pInterForm = m_pFormFillEnv->GetInterForm();
  CPDFSDK_Widget* pWidget = nullptr;
  if (nCount == 1) {
    pWidget = pInterForm->GetWidget(pFormField->GetControl(0));
  } else {
    UnderlyingPageType* pPage =
        UnderlyingFromFPDFPage(m_pFormFillEnv->GetCurrentPage(
            m_pFormFillEnv->GetUnderlyingDocument()));
    if (!pPage)
      return false;
    if (CPDFSDK_PageView* pCurPageView =
            m_pFormFillEnv->GetPageView(pPage, true)) {
      for (int32_t i = 0; i < nCount; i++) {
        if (CPDFSDK_Widget* pTempWidget =
                pInterForm->GetWidget(pFormField->GetControl(i))) {
          if (pTempWidget->GetPDFPage() == pCurPageView->GetPDFPage()) {
            pWidget = pTempWidget;
            break;
          }
        }
      }
    }
  }

  if (pWidget) {
    CPDFSDK_Annot::ObservedPtr pObserved(pWidget);
    m_pFormFillEnv->SetFocusAnnot(&pObserved);
  }

  return true;
}

bool Field::setItems(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError) {
  return true;
}

bool Field::setLock(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError) {
  return false;
}

bool Field::signatureGetModifications(CJS_Runtime* pRuntime,
                                      const std::vector<CJS_Value>& params,
                                      CJS_Value& vRet,
                                      WideString& sError) {
  return false;
}

bool Field::signatureGetSeedValue(CJS_Runtime* pRuntime,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  WideString& sError) {
  return false;
}

bool Field::signatureInfo(CJS_Runtime* pRuntime,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          WideString& sError) {
  return false;
}

bool Field::signatureSetSeedValue(CJS_Runtime* pRuntime,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  WideString& sError) {
  return false;
}

bool Field::signatureSign(CJS_Runtime* pRuntime,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          WideString& sError) {
  return false;
}

bool Field::signatureValidate(CJS_Runtime* pRuntime,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              WideString& sError) {
  return false;
}

bool Field::get_source(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
  vp->Set(v8::Local<v8::Value>());
  return true;
}

bool Field::set_source(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError) {
  return true;
}

void Field::AddDelay_Int(FIELD_PROP prop, int32_t n) {
  CJS_DelayData* pNewData =
      new CJS_DelayData(prop, m_nFormControlIndex, m_FieldName);
  pNewData->num = n;
  m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_Bool(FIELD_PROP prop, bool b) {
  CJS_DelayData* pNewData =
      new CJS_DelayData(prop, m_nFormControlIndex, m_FieldName);
  pNewData->b = b;
  m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_String(FIELD_PROP prop, const ByteString& string) {
  CJS_DelayData* pNewData =
      new CJS_DelayData(prop, m_nFormControlIndex, m_FieldName);
  pNewData->string = string;
  m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_Rect(FIELD_PROP prop, const CFX_FloatRect& rect) {
  CJS_DelayData* pNewData =
      new CJS_DelayData(prop, m_nFormControlIndex, m_FieldName);
  pNewData->rect = rect;
  m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_WordArray(FIELD_PROP prop,
                               const std::vector<uint32_t>& array) {
  CJS_DelayData* pNewData =
      new CJS_DelayData(prop, m_nFormControlIndex, m_FieldName);
  pNewData->wordarray = array;
  m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_WideStringArray(FIELD_PROP prop,
                                     const std::vector<WideString>& array) {
  CJS_DelayData* pNewData =
      new CJS_DelayData(prop, m_nFormControlIndex, m_FieldName);
  pNewData->widestringarray = array;
  m_pJSDoc->AddDelayData(pNewData);
}

void Field::DoDelay(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                    CJS_DelayData* pData) {
  ASSERT(pFormFillEnv);
  switch (pData->eProp) {
    case FP_BORDERSTYLE:
      Field::SetBorderStyle(pFormFillEnv, pData->sFieldName,
                            pData->nControlIndex, pData->string);
      break;
    case FP_CURRENTVALUEINDICES:
      Field::SetCurrentValueIndices(pFormFillEnv, pData->sFieldName,
                                    pData->nControlIndex, pData->wordarray);
      break;
    case FP_DISPLAY:
      Field::SetDisplay(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                        pData->num);
      break;
    case FP_HIDDEN:
      Field::SetHidden(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                       pData->b);
      break;
    case FP_LINEWIDTH:
      Field::SetLineWidth(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                          pData->num);
      break;
    case FP_RECT:
      Field::SetRect(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                     pData->rect);
      break;
    case FP_VALUE:
      Field::SetValue(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                      pData->widestringarray);
      break;
    default:
      NOTREACHED();
  }
}
