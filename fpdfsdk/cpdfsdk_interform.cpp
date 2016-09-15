// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_interform.h"

#include <algorithm>
#include <memory>

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cfdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfdoc/include/cpdf_actionfields.h"
#include "core/fpdfdoc/include/cpdf_interform.h"
#include "core/fxge/include/cfx_graphstatedata.h"
#include "core/fxge/include/cfx_pathdata.h"
#include "core/fxge/include/cfx_renderdevice.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/fxedit/include/fxet_edit.h"
#include "fpdfsdk/include/cba_annotiterator.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/include/cpdfsdk_widget.h"
#include "fpdfsdk/include/fsdk_actionhandler.h"
#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/ipdfsdk_annothandler.h"
#include "fpdfsdk/javascript/ijs_context.h"
#include "fpdfsdk/javascript/ijs_runtime.h"
#include "fpdfsdk/pdfwindow/PWL_Utils.h"
#include "third_party/base/stl_util.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_util.h"
#include "fpdfsdk/include/cpdfsdk_xfawidget.h"
#include "xfa/fxfa/include/cxfa_eventparam.h"
#include "xfa/fxfa/include/xfa_ffdocview.h"
#include "xfa/fxfa/include/xfa_ffwidget.h"
#include "xfa/fxfa/include/xfa_ffwidgethandler.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_InterForm::CPDFSDK_InterForm(CPDFSDK_Document* pDocument)
    : m_pDocument(pDocument),
      m_pInterForm(new CPDF_InterForm(m_pDocument->GetPDFDocument())),
#ifdef PDF_ENABLE_XFA
      m_bXfaCalculate(TRUE),
      m_bXfaValidationsEnabled(TRUE),
#endif  // PDF_ENABLE_XFA
      m_bCalculate(TRUE),
      m_bBusy(FALSE),
      m_iHighlightAlpha(0) {
  m_pInterForm->SetFormNotify(this);
  for (int i = 0; i < kNumFieldTypes; ++i)
    m_bNeedHightlight[i] = FALSE;
}

CPDFSDK_InterForm::~CPDFSDK_InterForm() {
  m_Map.clear();
#ifdef PDF_ENABLE_XFA
  m_XFAMap.clear();
#endif  // PDF_ENABLE_XFA
}

FX_BOOL CPDFSDK_InterForm::HighlightWidgets() {
  return FALSE;
}

CPDFSDK_Widget* CPDFSDK_InterForm::GetSibling(CPDFSDK_Widget* pWidget,
                                              FX_BOOL bNext) const {
  std::unique_ptr<CBA_AnnotIterator> pIterator(new CBA_AnnotIterator(
      pWidget->GetPageView(), CPDF_Annot::Subtype::WIDGET));

  if (bNext)
    return static_cast<CPDFSDK_Widget*>(pIterator->GetNextAnnot(pWidget));

  return static_cast<CPDFSDK_Widget*>(pIterator->GetPrevAnnot(pWidget));
}

CPDFSDK_Widget* CPDFSDK_InterForm::GetWidget(CPDF_FormControl* pControl,
                                             bool createIfNeeded) const {
  if (!pControl || !m_pInterForm)
    return nullptr;

  CPDFSDK_Widget* pWidget = nullptr;
  const auto it = m_Map.find(pControl);
  if (it != m_Map.end())
    pWidget = it->second;
  if (pWidget)
    return pWidget;
  if (!createIfNeeded)
    return nullptr;

  CPDF_Dictionary* pControlDict = pControl->GetWidget();
  CPDF_Document* pDocument = m_pDocument->GetPDFDocument();
  CPDFSDK_PageView* pPage = nullptr;

  if (CPDF_Dictionary* pPageDict = pControlDict->GetDictFor("P")) {
    int nPageIndex = pDocument->GetPageIndex(pPageDict->GetObjNum());
    if (nPageIndex >= 0)
      pPage = m_pDocument->GetPageView(nPageIndex);
  }

  if (!pPage) {
    int nPageIndex = GetPageIndexByAnnotDict(pDocument, pControlDict);
    if (nPageIndex >= 0)
      pPage = m_pDocument->GetPageView(nPageIndex);
  }

  if (!pPage)
    return nullptr;

  return static_cast<CPDFSDK_Widget*>(pPage->GetAnnotByDict(pControlDict));
}

void CPDFSDK_InterForm::GetWidgets(
    const CFX_WideString& sFieldName,
    std::vector<CPDFSDK_Widget*>* widgets) const {
  for (int i = 0, sz = m_pInterForm->CountFields(sFieldName); i < sz; ++i) {
    CPDF_FormField* pFormField = m_pInterForm->GetField(i, sFieldName);
    ASSERT(pFormField);
    GetWidgets(pFormField, widgets);
  }
}

void CPDFSDK_InterForm::GetWidgets(
    CPDF_FormField* pField,
    std::vector<CPDFSDK_Widget*>* widgets) const {
  for (int i = 0, sz = pField->CountControls(); i < sz; ++i) {
    CPDF_FormControl* pFormCtrl = pField->GetControl(i);
    ASSERT(pFormCtrl);
    CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl, true);
    if (pWidget)
      widgets->push_back(pWidget);
  }
}

int CPDFSDK_InterForm::GetPageIndexByAnnotDict(
    CPDF_Document* pDocument,
    CPDF_Dictionary* pAnnotDict) const {
  ASSERT(pAnnotDict);

  for (int i = 0, sz = pDocument->GetPageCount(); i < sz; i++) {
    if (CPDF_Dictionary* pPageDict = pDocument->GetPage(i)) {
      if (CPDF_Array* pAnnots = pPageDict->GetArrayFor("Annots")) {
        for (int j = 0, jsz = pAnnots->GetCount(); j < jsz; j++) {
          CPDF_Object* pDict = pAnnots->GetDirectObjectAt(j);
          if (pAnnotDict == pDict)
            return i;
        }
      }
    }
  }

  return -1;
}

void CPDFSDK_InterForm::AddMap(CPDF_FormControl* pControl,
                               CPDFSDK_Widget* pWidget) {
  m_Map[pControl] = pWidget;
}

void CPDFSDK_InterForm::RemoveMap(CPDF_FormControl* pControl) {
  m_Map.erase(pControl);
}

void CPDFSDK_InterForm::EnableCalculate(FX_BOOL bEnabled) {
  m_bCalculate = bEnabled;
}

FX_BOOL CPDFSDK_InterForm::IsCalculateEnabled() const {
  return m_bCalculate;
}

#ifdef PDF_ENABLE_XFA
void CPDFSDK_InterForm::AddXFAMap(CXFA_FFWidget* hWidget,
                                  CPDFSDK_XFAWidget* pWidget) {
  ASSERT(hWidget);
  m_XFAMap[hWidget] = pWidget;
}

void CPDFSDK_InterForm::RemoveXFAMap(CXFA_FFWidget* hWidget) {
  ASSERT(hWidget);
  m_XFAMap.erase(hWidget);
}

CPDFSDK_XFAWidget* CPDFSDK_InterForm::GetXFAWidget(CXFA_FFWidget* hWidget) {
  ASSERT(hWidget);
  auto it = m_XFAMap.find(hWidget);
  return it != m_XFAMap.end() ? it->second : nullptr;
}

void CPDFSDK_InterForm::XfaEnableCalculate(FX_BOOL bEnabled) {
  m_bXfaCalculate = bEnabled;
}
FX_BOOL CPDFSDK_InterForm::IsXfaCalculateEnabled() const {
  return m_bXfaCalculate;
}

FX_BOOL CPDFSDK_InterForm::IsXfaValidationsEnabled() {
  return m_bXfaValidationsEnabled;
}
void CPDFSDK_InterForm::XfaSetValidationsEnabled(FX_BOOL bEnabled) {
  m_bXfaValidationsEnabled = bEnabled;
}

void CPDFSDK_InterForm::SynchronizeField(CPDF_FormField* pFormField,
                                         FX_BOOL bSynchronizeElse) {
  for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
    CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
    if (CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl, false))
      pWidget->Synchronize(bSynchronizeElse);
  }
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_InterForm::OnCalculate(CPDF_FormField* pFormField) {
  CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();
  ASSERT(pEnv);
  if (!pEnv->IsJSInitiated())
    return;

  if (m_bBusy)
    return;

  m_bBusy = TRUE;

  if (!IsCalculateEnabled()) {
    m_bBusy = FALSE;
    return;
  }

  IJS_Runtime* pRuntime = m_pDocument->GetJsRuntime();
  pRuntime->SetReaderDocument(m_pDocument);

  int nSize = m_pInterForm->CountFieldsInCalculationOrder();
  for (int i = 0; i < nSize; i++) {
    CPDF_FormField* pField = m_pInterForm->GetFieldInCalculationOrder(i);
    if (!pField)
      continue;

    int nType = pField->GetFieldType();
    if (nType != FIELDTYPE_COMBOBOX && nType != FIELDTYPE_TEXTFIELD)
      continue;

    CPDF_AAction aAction = pField->GetAdditionalAction();
    if (!aAction.GetDict() || !aAction.ActionExist(CPDF_AAction::Calculate))
      continue;

    CPDF_Action action = aAction.GetAction(CPDF_AAction::Calculate);
    if (!action.GetDict())
      continue;

    CFX_WideString csJS = action.GetJavaScript();
    if (csJS.IsEmpty())
      continue;

    IJS_Context* pContext = pRuntime->NewContext();
    CFX_WideString sOldValue = pField->GetValue();
    CFX_WideString sValue = sOldValue;
    FX_BOOL bRC = TRUE;
    pContext->OnField_Calculate(pFormField, pField, sValue, bRC);

    CFX_WideString sInfo;
    FX_BOOL bRet = pContext->RunScript(csJS, &sInfo);
    pRuntime->ReleaseContext(pContext);

    if (bRet && bRC && sValue.Compare(sOldValue) != 0)
      pField->SetValue(sValue, TRUE);
  }

  m_bBusy = FALSE;
}

CFX_WideString CPDFSDK_InterForm::OnFormat(CPDF_FormField* pFormField,
                                           FX_BOOL& bFormatted) {
  CFX_WideString sValue = pFormField->GetValue();
  CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();
  ASSERT(pEnv);
  if (!pEnv->IsJSInitiated()) {
    bFormatted = FALSE;
    return sValue;
  }

  IJS_Runtime* pRuntime = m_pDocument->GetJsRuntime();
  pRuntime->SetReaderDocument(m_pDocument);

  if (pFormField->GetFieldType() == FIELDTYPE_COMBOBOX &&
      pFormField->CountSelectedItems() > 0) {
    int index = pFormField->GetSelectedIndex(0);
    if (index >= 0)
      sValue = pFormField->GetOptionLabel(index);
  }

  bFormatted = FALSE;

  CPDF_AAction aAction = pFormField->GetAdditionalAction();
  if (aAction.GetDict() && aAction.ActionExist(CPDF_AAction::Format)) {
    CPDF_Action action = aAction.GetAction(CPDF_AAction::Format);
    if (action.GetDict()) {
      CFX_WideString script = action.GetJavaScript();
      if (!script.IsEmpty()) {
        CFX_WideString Value = sValue;

        IJS_Context* pContext = pRuntime->NewContext();
        pContext->OnField_Format(pFormField, Value, TRUE);
        CFX_WideString sInfo;
        FX_BOOL bRet = pContext->RunScript(script, &sInfo);
        pRuntime->ReleaseContext(pContext);

        if (bRet) {
          sValue = Value;
          bFormatted = TRUE;
        }
      }
    }
  }

  return sValue;
}

void CPDFSDK_InterForm::ResetFieldAppearance(CPDF_FormField* pFormField,
                                             const CFX_WideString* sValue,
                                             FX_BOOL bValueChanged) {
  for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
    CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
    ASSERT(pFormCtrl);
    if (CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl, false))
      pWidget->ResetAppearance(sValue, bValueChanged);
  }
}

void CPDFSDK_InterForm::UpdateField(CPDF_FormField* pFormField) {
  for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
    CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
    ASSERT(pFormCtrl);

    if (CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl, false)) {
      CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();
      CFFL_IFormFiller* pIFormFiller = pEnv->GetIFormFiller();
      UnderlyingPageType* pPage = pWidget->GetUnderlyingPage();
      CPDFSDK_PageView* pPageView = m_pDocument->GetPageView(pPage, false);
      FX_RECT rcBBox = pIFormFiller->GetViewBBox(pPageView, pWidget);

      pEnv->Invalidate(pPage, rcBBox.left, rcBBox.top, rcBBox.right,
                       rcBBox.bottom);
    }
  }
}

FX_BOOL CPDFSDK_InterForm::OnKeyStrokeCommit(CPDF_FormField* pFormField,
                                             const CFX_WideString& csValue) {
  CPDF_AAction aAction = pFormField->GetAdditionalAction();
  if (!aAction.GetDict() || !aAction.ActionExist(CPDF_AAction::KeyStroke))
    return TRUE;

  CPDF_Action action = aAction.GetAction(CPDF_AAction::KeyStroke);
  if (!action.GetDict())
    return TRUE;

  CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();
  CPDFSDK_ActionHandler* pActionHandler = pEnv->GetActionHander();
  PDFSDK_FieldAction fa;
  fa.bModifier = pEnv->IsCTRLKeyDown(0);
  fa.bShift = pEnv->IsSHIFTKeyDown(0);
  fa.sValue = csValue;
  pActionHandler->DoAction_FieldJavaScript(action, CPDF_AAction::KeyStroke,
                                           m_pDocument, pFormField, fa);
  return fa.bRC;
}

FX_BOOL CPDFSDK_InterForm::OnValidate(CPDF_FormField* pFormField,
                                      const CFX_WideString& csValue) {
  CPDF_AAction aAction = pFormField->GetAdditionalAction();
  if (!aAction.GetDict() || !aAction.ActionExist(CPDF_AAction::Validate))
    return TRUE;

  CPDF_Action action = aAction.GetAction(CPDF_AAction::Validate);
  if (!action.GetDict())
    return TRUE;

  CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();
  CPDFSDK_ActionHandler* pActionHandler = pEnv->GetActionHander();
  PDFSDK_FieldAction fa;
  fa.bModifier = pEnv->IsCTRLKeyDown(0);
  fa.bShift = pEnv->IsSHIFTKeyDown(0);
  fa.sValue = csValue;
  pActionHandler->DoAction_FieldJavaScript(action, CPDF_AAction::Validate,
                                           m_pDocument, pFormField, fa);
  return fa.bRC;
}

FX_BOOL CPDFSDK_InterForm::DoAction_Hide(const CPDF_Action& action) {
  ASSERT(action.GetDict());

  CPDF_ActionFields af(&action);
  std::vector<CPDF_Object*> fieldObjects = af.GetAllFields();
  std::vector<CPDF_FormField*> fields = GetFieldFromObjects(fieldObjects);

  bool bHide = action.GetHideStatus();
  FX_BOOL bChanged = FALSE;

  for (CPDF_FormField* pField : fields) {
    for (int i = 0, sz = pField->CountControls(); i < sz; ++i) {
      CPDF_FormControl* pControl = pField->GetControl(i);
      ASSERT(pControl);

      if (CPDFSDK_Widget* pWidget = GetWidget(pControl, false)) {
        uint32_t nFlags = pWidget->GetFlags();
        nFlags &= ~ANNOTFLAG_INVISIBLE;
        nFlags &= ~ANNOTFLAG_NOVIEW;
        if (bHide)
          nFlags |= ANNOTFLAG_HIDDEN;
        else
          nFlags &= ~ANNOTFLAG_HIDDEN;
        pWidget->SetFlags(nFlags);
        pWidget->GetPageView()->UpdateView(pWidget);
        bChanged = TRUE;
      }
    }
  }

  return bChanged;
}

FX_BOOL CPDFSDK_InterForm::DoAction_SubmitForm(const CPDF_Action& action) {
  CFX_WideString sDestination = action.GetFilePath();
  if (sDestination.IsEmpty())
    return FALSE;

  CPDF_Dictionary* pActionDict = action.GetDict();
  if (pActionDict->KeyExist("Fields")) {
    CPDF_ActionFields af(&action);
    uint32_t dwFlags = action.GetFlags();
    std::vector<CPDF_Object*> fieldObjects = af.GetAllFields();
    std::vector<CPDF_FormField*> fields = GetFieldFromObjects(fieldObjects);
    if (!fields.empty()) {
      bool bIncludeOrExclude = !(dwFlags & 0x01);
      if (m_pInterForm->CheckRequiredFields(&fields, bIncludeOrExclude))
        return FALSE;

      return SubmitFields(sDestination, fields, bIncludeOrExclude, false);
    }
  }
  if (m_pInterForm->CheckRequiredFields(nullptr, true))
    return FALSE;

  return SubmitForm(sDestination, FALSE);
}

FX_BOOL CPDFSDK_InterForm::SubmitFields(
    const CFX_WideString& csDestination,
    const std::vector<CPDF_FormField*>& fields,
    bool bIncludeOrExclude,
    bool bUrlEncoded) {
  CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();

  CFX_ByteTextBuf textBuf;
  ExportFieldsToFDFTextBuf(fields, bIncludeOrExclude, textBuf);

  uint8_t* pBuffer = textBuf.GetBuffer();
  FX_STRSIZE nBufSize = textBuf.GetLength();

  if (bUrlEncoded && !FDFToURLEncodedData(pBuffer, nBufSize))
    return FALSE;

  pEnv->JS_docSubmitForm(pBuffer, nBufSize, csDestination.c_str());
  return TRUE;
}

FX_BOOL CPDFSDK_InterForm::FDFToURLEncodedData(CFX_WideString csFDFFile,
                                               CFX_WideString csTxtFile) {
  return TRUE;
}

FX_BOOL CPDFSDK_InterForm::FDFToURLEncodedData(uint8_t*& pBuf,
                                               FX_STRSIZE& nBufSize) {
  CFDF_Document* pFDF = CFDF_Document::ParseMemory(pBuf, nBufSize);
  if (!pFDF)
    return TRUE;

  CPDF_Dictionary* pMainDict = pFDF->GetRoot()->GetDictFor("FDF");
  if (!pMainDict)
    return FALSE;

  CPDF_Array* pFields = pMainDict->GetArrayFor("Fields");
  if (!pFields)
    return FALSE;

  CFX_ByteTextBuf fdfEncodedData;
  for (uint32_t i = 0; i < pFields->GetCount(); i++) {
    CPDF_Dictionary* pField = pFields->GetDictAt(i);
    if (!pField)
      continue;
    CFX_WideString name;
    name = pField->GetUnicodeTextFor("T");
    CFX_ByteString name_b = CFX_ByteString::FromUnicode(name);
    CFX_ByteString csBValue = pField->GetStringFor("V");
    CFX_WideString csWValue = PDF_DecodeText(csBValue);
    CFX_ByteString csValue_b = CFX_ByteString::FromUnicode(csWValue);

    fdfEncodedData << name_b.GetBuffer(name_b.GetLength());
    name_b.ReleaseBuffer();
    fdfEncodedData << "=";
    fdfEncodedData << csValue_b.GetBuffer(csValue_b.GetLength());
    csValue_b.ReleaseBuffer();
    if (i != pFields->GetCount() - 1)
      fdfEncodedData << "&";
  }

  nBufSize = fdfEncodedData.GetLength();
  pBuf = FX_Alloc(uint8_t, nBufSize);
  FXSYS_memcpy(pBuf, fdfEncodedData.GetBuffer(), nBufSize);
  return TRUE;
}

FX_BOOL CPDFSDK_InterForm::ExportFieldsToFDFTextBuf(
    const std::vector<CPDF_FormField*>& fields,
    bool bIncludeOrExclude,
    CFX_ByteTextBuf& textBuf) {
  std::unique_ptr<CFDF_Document> pFDF(m_pInterForm->ExportToFDF(
      m_pDocument->GetPath().AsStringC(), fields, bIncludeOrExclude));
  return pFDF ? pFDF->WriteBuf(textBuf) : FALSE;
}

CFX_WideString CPDFSDK_InterForm::GetTemporaryFileName(
    const CFX_WideString& sFileExt) {
  return L"";
}

FX_BOOL CPDFSDK_InterForm::SubmitForm(const CFX_WideString& sDestination,
                                      FX_BOOL bUrlEncoded) {
  if (sDestination.IsEmpty())
    return FALSE;

  if (!m_pDocument || !m_pInterForm)
    return FALSE;

  CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();
  CFX_WideString wsPDFFilePath = m_pDocument->GetPath();
  CFDF_Document* pFDFDoc = m_pInterForm->ExportToFDF(wsPDFFilePath.AsStringC());
  if (!pFDFDoc)
    return FALSE;

  CFX_ByteTextBuf FdfBuffer;
  FX_BOOL bRet = pFDFDoc->WriteBuf(FdfBuffer);
  delete pFDFDoc;
  if (!bRet)
    return FALSE;

  uint8_t* pBuffer = FdfBuffer.GetBuffer();
  FX_STRSIZE nBufSize = FdfBuffer.GetLength();

  if (bUrlEncoded && !FDFToURLEncodedData(pBuffer, nBufSize))
    return FALSE;

  pEnv->JS_docSubmitForm(pBuffer, nBufSize, sDestination.c_str());

  if (bUrlEncoded)
    FX_Free(pBuffer);

  return TRUE;
}

FX_BOOL CPDFSDK_InterForm::ExportFormToFDFTextBuf(CFX_ByteTextBuf& textBuf) {
  CFDF_Document* pFDF =
      m_pInterForm->ExportToFDF(m_pDocument->GetPath().AsStringC());
  if (!pFDF)
    return FALSE;

  FX_BOOL bRet = pFDF->WriteBuf(textBuf);
  delete pFDF;

  return bRet;
}

FX_BOOL CPDFSDK_InterForm::DoAction_ResetForm(const CPDF_Action& action) {
  ASSERT(action.GetDict());

  CPDF_Dictionary* pActionDict = action.GetDict();
  if (!pActionDict->KeyExist("Fields"))
    return m_pInterForm->ResetForm(true);

  CPDF_ActionFields af(&action);
  uint32_t dwFlags = action.GetFlags();

  std::vector<CPDF_Object*> fieldObjects = af.GetAllFields();
  std::vector<CPDF_FormField*> fields = GetFieldFromObjects(fieldObjects);
  return m_pInterForm->ResetForm(fields, !(dwFlags & 0x01), true);
}

FX_BOOL CPDFSDK_InterForm::DoAction_ImportData(const CPDF_Action& action) {
  return FALSE;
}

std::vector<CPDF_FormField*> CPDFSDK_InterForm::GetFieldFromObjects(
    const std::vector<CPDF_Object*>& objects) const {
  std::vector<CPDF_FormField*> fields;
  for (CPDF_Object* pObject : objects) {
    if (pObject && pObject->IsString()) {
      CFX_WideString csName = pObject->GetUnicodeText();
      CPDF_FormField* pField = m_pInterForm->GetField(0, csName);
      if (pField)
        fields.push_back(pField);
    }
  }
  return fields;
}

int CPDFSDK_InterForm::BeforeValueChange(CPDF_FormField* pField,
                                         const CFX_WideString& csValue) {
  int nType = pField->GetFieldType();
  if (nType != FIELDTYPE_COMBOBOX && nType != FIELDTYPE_TEXTFIELD)
    return 0;

  if (!OnKeyStrokeCommit(pField, csValue))
    return -1;

  if (!OnValidate(pField, csValue))
    return -1;

  return 1;
}

void CPDFSDK_InterForm::AfterValueChange(CPDF_FormField* pField) {
#ifdef PDF_ENABLE_XFA
  SynchronizeField(pField, FALSE);
#endif  // PDF_ENABLE_XFA
  int nType = pField->GetFieldType();
  if (nType == FIELDTYPE_COMBOBOX || nType == FIELDTYPE_TEXTFIELD) {
    OnCalculate(pField);
    FX_BOOL bFormatted = FALSE;
    CFX_WideString sValue = OnFormat(pField, bFormatted);
    ResetFieldAppearance(pField, bFormatted ? &sValue : nullptr, TRUE);
    UpdateField(pField);
  }
}

int CPDFSDK_InterForm::BeforeSelectionChange(CPDF_FormField* pField,
                                             const CFX_WideString& csValue) {
  if (pField->GetFieldType() != FIELDTYPE_LISTBOX)
    return 0;

  if (!OnKeyStrokeCommit(pField, csValue))
    return -1;

  if (!OnValidate(pField, csValue))
    return -1;

  return 1;
}

void CPDFSDK_InterForm::AfterSelectionChange(CPDF_FormField* pField) {
  if (pField->GetFieldType() != FIELDTYPE_LISTBOX)
    return;

  OnCalculate(pField);
  ResetFieldAppearance(pField, nullptr, TRUE);
  UpdateField(pField);
}

void CPDFSDK_InterForm::AfterCheckedStatusChange(CPDF_FormField* pField) {
  int nType = pField->GetFieldType();
  if (nType != FIELDTYPE_CHECKBOX && nType != FIELDTYPE_RADIOBUTTON)
    return;

  OnCalculate(pField);
  UpdateField(pField);
}

int CPDFSDK_InterForm::BeforeFormReset(CPDF_InterForm* pForm) {
  return 0;
}

void CPDFSDK_InterForm::AfterFormReset(CPDF_InterForm* pForm) {
  OnCalculate(nullptr);
}

int CPDFSDK_InterForm::BeforeFormImportData(CPDF_InterForm* pForm) {
  return 0;
}

void CPDFSDK_InterForm::AfterFormImportData(CPDF_InterForm* pForm) {
  OnCalculate(nullptr);
}

FX_BOOL CPDFSDK_InterForm::IsNeedHighLight(int nFieldType) {
  if (nFieldType < 1 || nFieldType > kNumFieldTypes)
    return FALSE;
  return m_bNeedHightlight[nFieldType - 1];
}

void CPDFSDK_InterForm::RemoveAllHighLight() {
  for (int i = 0; i < kNumFieldTypes; ++i)
    m_bNeedHightlight[i] = FALSE;
}

void CPDFSDK_InterForm::SetHighlightColor(FX_COLORREF clr, int nFieldType) {
  if (nFieldType < 0 || nFieldType > kNumFieldTypes)
    return;
  switch (nFieldType) {
    case 0: {
      for (int i = 0; i < kNumFieldTypes; ++i) {
        m_aHighlightColor[i] = clr;
        m_bNeedHightlight[i] = TRUE;
      }
      break;
    }
    default: {
      m_aHighlightColor[nFieldType - 1] = clr;
      m_bNeedHightlight[nFieldType - 1] = TRUE;
      break;
    }
  }
}

FX_COLORREF CPDFSDK_InterForm::GetHighlightColor(int nFieldType) {
  if (nFieldType < 0 || nFieldType > kNumFieldTypes)
    return FXSYS_RGB(255, 255, 255);
  if (nFieldType == 0)
    return m_aHighlightColor[0];
  return m_aHighlightColor[nFieldType - 1];
}
