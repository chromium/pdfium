// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Document.h"

#include <vector>

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/fpdf_parser_decode.h"
#include "core/fpdfdoc/include/cpdf_interform.h"
#include "core/fpdfdoc/include/cpdf_nametree.h"
#include "fpdfsdk/include/cpdfsdk_annotiterator.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/include/cpdfsdk_widget.h"
#include "fpdfsdk/javascript/Annot.h"
#include "fpdfsdk/javascript/Field.h"
#include "fpdfsdk/javascript/Icon.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/app.h"
#include "fpdfsdk/javascript/cjs_context.h"
#include "fpdfsdk/javascript/cjs_runtime.h"
#include "fpdfsdk/javascript/resource.h"
#include "third_party/base/numerics/safe_math.h"

BEGIN_JS_STATIC_CONST(CJS_PrintParamsObj)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_PrintParamsObj)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_PrintParamsObj)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_PrintParamsObj, PrintParamsObj)

PrintParamsObj::PrintParamsObj(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject) {
  bUI = TRUE;
  nStart = 0;
  nEnd = 0;
  bSilent = FALSE;
  bShrinkToFit = FALSE;
  bPrintAsImage = FALSE;
  bReverse = FALSE;
  bAnnotations = TRUE;
}

#define MINWIDTH 5.0f
#define MINHEIGHT 5.0f

BEGIN_JS_STATIC_CONST(CJS_Document)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Document)
JS_STATIC_PROP_ENTRY(ADBE)
JS_STATIC_PROP_ENTRY(author)
JS_STATIC_PROP_ENTRY(baseURL)
JS_STATIC_PROP_ENTRY(bookmarkRoot)
JS_STATIC_PROP_ENTRY(calculate)
JS_STATIC_PROP_ENTRY(Collab)
JS_STATIC_PROP_ENTRY(creationDate)
JS_STATIC_PROP_ENTRY(creator)
JS_STATIC_PROP_ENTRY(delay)
JS_STATIC_PROP_ENTRY(dirty)
JS_STATIC_PROP_ENTRY(documentFileName)
JS_STATIC_PROP_ENTRY(external)
JS_STATIC_PROP_ENTRY(filesize)
JS_STATIC_PROP_ENTRY(icons)
JS_STATIC_PROP_ENTRY(info)
JS_STATIC_PROP_ENTRY(keywords)
JS_STATIC_PROP_ENTRY(layout)
JS_STATIC_PROP_ENTRY(media)
JS_STATIC_PROP_ENTRY(modDate)
JS_STATIC_PROP_ENTRY(mouseX)
JS_STATIC_PROP_ENTRY(mouseY)
JS_STATIC_PROP_ENTRY(numFields)
JS_STATIC_PROP_ENTRY(numPages)
JS_STATIC_PROP_ENTRY(pageNum)
JS_STATIC_PROP_ENTRY(pageWindowRect)
JS_STATIC_PROP_ENTRY(path)
JS_STATIC_PROP_ENTRY(producer)
JS_STATIC_PROP_ENTRY(subject)
JS_STATIC_PROP_ENTRY(title)
JS_STATIC_PROP_ENTRY(URL)
JS_STATIC_PROP_ENTRY(zoom)
JS_STATIC_PROP_ENTRY(zoomType)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Document)
JS_STATIC_METHOD_ENTRY(addAnnot)
JS_STATIC_METHOD_ENTRY(addField)
JS_STATIC_METHOD_ENTRY(addLink)
JS_STATIC_METHOD_ENTRY(addIcon)
JS_STATIC_METHOD_ENTRY(calculateNow)
JS_STATIC_METHOD_ENTRY(closeDoc)
JS_STATIC_METHOD_ENTRY(createDataObject)
JS_STATIC_METHOD_ENTRY(deletePages)
JS_STATIC_METHOD_ENTRY(exportAsText)
JS_STATIC_METHOD_ENTRY(exportAsFDF)
JS_STATIC_METHOD_ENTRY(exportAsXFDF)
JS_STATIC_METHOD_ENTRY(extractPages)
JS_STATIC_METHOD_ENTRY(getAnnot)
JS_STATIC_METHOD_ENTRY(getAnnots)
JS_STATIC_METHOD_ENTRY(getAnnot3D)
JS_STATIC_METHOD_ENTRY(getAnnots3D)
JS_STATIC_METHOD_ENTRY(getField)
JS_STATIC_METHOD_ENTRY(getIcon)
JS_STATIC_METHOD_ENTRY(getLinks)
JS_STATIC_METHOD_ENTRY(getNthFieldName)
JS_STATIC_METHOD_ENTRY(getOCGs)
JS_STATIC_METHOD_ENTRY(getPageBox)
JS_STATIC_METHOD_ENTRY(getPageNthWord)
JS_STATIC_METHOD_ENTRY(getPageNthWordQuads)
JS_STATIC_METHOD_ENTRY(getPageNumWords)
JS_STATIC_METHOD_ENTRY(getPrintParams)
JS_STATIC_METHOD_ENTRY(getURL)
JS_STATIC_METHOD_ENTRY(gotoNamedDest)
JS_STATIC_METHOD_ENTRY(importAnFDF)
JS_STATIC_METHOD_ENTRY(importAnXFDF)
JS_STATIC_METHOD_ENTRY(importTextData)
JS_STATIC_METHOD_ENTRY(insertPages)
JS_STATIC_METHOD_ENTRY(mailForm)
JS_STATIC_METHOD_ENTRY(print)
JS_STATIC_METHOD_ENTRY(removeField)
JS_STATIC_METHOD_ENTRY(replacePages)
JS_STATIC_METHOD_ENTRY(resetForm)
JS_STATIC_METHOD_ENTRY(removeIcon)
JS_STATIC_METHOD_ENTRY(saveAs)
JS_STATIC_METHOD_ENTRY(submitForm)
JS_STATIC_METHOD_ENTRY(syncAnnotScan)
JS_STATIC_METHOD_ENTRY(mailDoc)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Document, Document)

void CJS_Document::InitInstance(IJS_Runtime* pIRuntime) {
  CJS_Runtime* pRuntime = static_cast<CJS_Runtime*>(pIRuntime);
  Document* pDoc = static_cast<Document*>(GetEmbedObject());
  pDoc->AttachDoc(pRuntime->GetReaderDocument());
}

Document::Document(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject),
      m_pDocument(nullptr),
      m_cwBaseURL(L""),
      m_bDelay(FALSE) {}

Document::~Document() {
}

// the total number of fileds in document.
FX_BOOL Document::numFields(IJS_Context* cc,
                            CJS_PropValue& vp,
                            CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  vp << (int)pPDFForm->CountFields();
  return TRUE;
}

FX_BOOL Document::dirty(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (vp.IsGetting()) {
    vp << !!m_pDocument->GetChangeMark();
  } else {
    bool bChanged = false;
    vp >> bChanged;

    if (bChanged)
      m_pDocument->SetChangeMark();
    else
      m_pDocument->ClearChangeMark();
  }
  return TRUE;
}

FX_BOOL Document::ADBE(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (vp.IsGetting())
    vp.GetJSValue()->SetNull(CJS_Runtime::FromContext(cc));

  return TRUE;
}

FX_BOOL Document::pageNum(IJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (vp.IsGetting()) {
    if (CPDFSDK_PageView* pPageView = m_pDocument->GetCurrentView()) {
      vp << pPageView->GetPageIndex();
    }
  } else {
    int iPageCount = m_pDocument->GetPageCount();
    int iPageNum = 0;
    vp >> iPageNum;

    CPDFSDK_Environment* pEnv = m_pDocument->GetEnv();
    if (iPageNum >= 0 && iPageNum < iPageCount) {
      pEnv->JS_docgotoPage(iPageNum);
    } else if (iPageNum >= iPageCount) {
      pEnv->JS_docgotoPage(iPageCount - 1);
    } else if (iPageNum < 0) {
      pEnv->JS_docgotoPage(0);
    }
  }

  return TRUE;
}

FX_BOOL Document::addAnnot(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  // Not supported.
  return TRUE;
}

FX_BOOL Document::addField(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  // Not supported.
  return TRUE;
}

FX_BOOL Document::exportAsText(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::exportAsFDF(IJS_Context* cc,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::exportAsXFDF(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

// Maps a field object in PDF document to a JavaScript variable
// comment:
// note: the paremter cName, this is clue how to treat if the cName is not a
// valiable filed name in this document

FX_BOOL Document::getField(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  if (params.size() < 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  CFX_WideString wideName = params[0].ToCFXWideString(pRuntime);
  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  if (pPDFForm->CountFields(wideName) <= 0) {
    vRet.SetNull(pRuntime);
    return TRUE;
  }

  v8::Local<v8::Object> pFieldObj =
      pRuntime->NewFxDynamicObj(CJS_Field::g_nObjDefnID);
  CJS_Field* pJSField =
      static_cast<CJS_Field*>(pRuntime->GetObjectPrivate(pFieldObj));
  Field* pField = static_cast<Field*>(pJSField->GetEmbedObject());
  pField->AttachField(this, wideName);

  vRet = CJS_Value(pRuntime, pJSField);
  return TRUE;
}

// Gets the name of the nth field in the document
FX_BOOL Document::getNthFieldName(IJS_Context* cc,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  CFX_WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  int nIndex = params[0].ToInt(pRuntime);
  if (nIndex < 0) {
    sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
    return FALSE;
  }
  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  CPDF_FormField* pField = pPDFForm->GetField(nIndex);
  if (!pField)
    return FALSE;

  vRet = CJS_Value(pRuntime, pField->GetFullName().c_str());
  return TRUE;
}

FX_BOOL Document::importAnFDF(IJS_Context* cc,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::importAnXFDF(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::importTextData(IJS_Context* cc,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

// exports the form data and mails the resulting fdf file as an attachment to
// all recipients.
// comment: need reader supports
// note:
// int CPDFSDK_Document::mailForm(FX_BOOL bUI,String cto,string ccc,string
// cbcc,string cSubject,string cms);

FX_BOOL Document::mailForm(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return FALSE;
  }

  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();

  int iLength = params.size();
  FX_BOOL bUI = iLength > 0 ? params[0].ToBool(pRuntime) : TRUE;
  CFX_WideString cTo = iLength > 1 ? params[1].ToCFXWideString(pRuntime) : L"";
  CFX_WideString cCc = iLength > 2 ? params[2].ToCFXWideString(pRuntime) : L"";
  CFX_WideString cBcc = iLength > 3 ? params[3].ToCFXWideString(pRuntime) : L"";
  CFX_WideString cSubject =
      iLength > 4 ? params[4].ToCFXWideString(pRuntime) : L"";
  CFX_WideString cMsg = iLength > 5 ? params[5].ToCFXWideString(pRuntime) : L"";

  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  CFX_ByteTextBuf textBuf;
  if (!pInterForm->ExportFormToFDFTextBuf(textBuf))
    return FALSE;

  pRuntime->BeginBlock();
  CPDFSDK_Environment* pEnv = pContext->GetReaderApp();
  pEnv->JS_docmailForm(textBuf.GetBuffer(), textBuf.GetLength(), bUI,
                       cTo.c_str(), cSubject.c_str(), cCc.c_str(), cBcc.c_str(),
                       cMsg.c_str());
  pRuntime->EndBlock();
  return TRUE;
}

FX_BOOL Document::print(IJS_Context* cc,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }

  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();

  FX_BOOL bUI = TRUE;
  int nStart = 0;
  int nEnd = 0;
  FX_BOOL bSilent = FALSE;
  FX_BOOL bShrinkToFit = FALSE;
  FX_BOOL bPrintAsImage = FALSE;
  FX_BOOL bReverse = FALSE;
  FX_BOOL bAnnotations = FALSE;

  int nlength = params.size();
  if (nlength == 9) {
    if (params[8].GetType() == CJS_Value::VT_object) {
      v8::Local<v8::Object> pObj = params[8].ToV8Object(pRuntime);
      if (CFXJS_Engine::GetObjDefnID(pObj) ==
          CJS_PrintParamsObj::g_nObjDefnID) {
        if (CJS_Object* pJSObj = params[8].ToCJSObject(pRuntime)) {
          if (PrintParamsObj* pprintparamsObj =
                  static_cast<PrintParamsObj*>(pJSObj->GetEmbedObject())) {
            bUI = pprintparamsObj->bUI;
            nStart = pprintparamsObj->nStart;
            nEnd = pprintparamsObj->nEnd;
            bSilent = pprintparamsObj->bSilent;
            bShrinkToFit = pprintparamsObj->bShrinkToFit;
            bPrintAsImage = pprintparamsObj->bPrintAsImage;
            bReverse = pprintparamsObj->bReverse;
            bAnnotations = pprintparamsObj->bAnnotations;
          }
        }
      }
    }
  } else {
    if (nlength >= 1)
      bUI = params[0].ToBool(pRuntime);
    if (nlength >= 2)
      nStart = params[1].ToInt(pRuntime);
    if (nlength >= 3)
      nEnd = params[2].ToInt(pRuntime);
    if (nlength >= 4)
      bSilent = params[3].ToBool(pRuntime);
    if (nlength >= 5)
      bShrinkToFit = params[4].ToBool(pRuntime);
    if (nlength >= 6)
      bPrintAsImage = params[5].ToBool(pRuntime);
    if (nlength >= 7)
      bReverse = params[6].ToBool(pRuntime);
    if (nlength >= 8)
      bAnnotations = params[7].ToBool(pRuntime);
  }

  if (CPDFSDK_Environment* pEnv = m_pDocument->GetEnv()) {
    pEnv->JS_docprint(bUI, nStart, nEnd, bSilent, bShrinkToFit, bPrintAsImage,
                      bReverse, bAnnotations);
    return TRUE;
  }
  return FALSE;
}

// removes the specified field from the document.
// comment:
// note: if the filed name is not rational, adobe is dumb for it.

FX_BOOL Document::removeField(IJS_Context* cc,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              CFX_WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (!(m_pDocument->GetPermissions(FPDFPERM_MODIFY) ||
        m_pDocument->GetPermissions(FPDFPERM_ANNOT_FORM))) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return FALSE;
  }
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  CFX_WideString sFieldName = params[0].ToCFXWideString(pRuntime);
  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  std::vector<CPDFSDK_Widget*> widgets;
  pInterForm->GetWidgets(sFieldName, &widgets);
  if (widgets.empty())
    return TRUE;

  for (CPDFSDK_Widget* pWidget : widgets) {
    CFX_FloatRect rcAnnot = pWidget->GetRect();
    --rcAnnot.left;
    --rcAnnot.bottom;
    ++rcAnnot.right;
    ++rcAnnot.top;

    std::vector<CFX_FloatRect> aRefresh(1, rcAnnot);
    UnderlyingPageType* pPage = pWidget->GetUnderlyingPage();
    ASSERT(pPage);

    // If there is currently no pageview associated with the page being used
    // do not create one. We may be in the process of tearing down the document
    // and creating a new pageview at this point will cause bad things.
    CPDFSDK_PageView* pPageView = m_pDocument->GetPageView(pPage, false);
    if (pPageView) {
      pPageView->DeleteAnnot(pWidget);
      pPageView->UpdateRects(aRefresh);
    }
  }
  m_pDocument->SetChangeMark();

  return TRUE;
}

// reset filed values within a document.
// comment:
// note: if the fields names r not rational, aodbe is dumb for it.

FX_BOOL Document::resetForm(IJS_Context* cc,
                            const std::vector<CJS_Value>& params,
                            CJS_Value& vRet,
                            CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (!(m_pDocument->GetPermissions(FPDFPERM_MODIFY) ||
        m_pDocument->GetPermissions(FPDFPERM_ANNOT_FORM) ||
        m_pDocument->GetPermissions(FPDFPERM_FILL_FORM))) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return FALSE;
  }

  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
  CJS_Array aName;

  if (params.empty()) {
    pPDFForm->ResetForm(TRUE);
    m_pDocument->SetChangeMark();
    return TRUE;
  }

  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();

  switch (params[0].GetType()) {
    default:
      aName.Attach(params[0].ToV8Array(pRuntime));
      break;
    case CJS_Value::VT_string:
      aName.SetElement(pRuntime, 0, params[0]);
      break;
  }

  std::vector<CPDF_FormField*> aFields;
  for (int i = 0, isz = aName.GetLength(pRuntime); i < isz; ++i) {
    CJS_Value valElement(pRuntime);
    aName.GetElement(pRuntime, i, valElement);
    CFX_WideString swVal = valElement.ToCFXWideString(pRuntime);
    for (int j = 0, jsz = pPDFForm->CountFields(swVal); j < jsz; ++j)
      aFields.push_back(pPDFForm->GetField(j, swVal));
  }

  if (!aFields.empty()) {
    pPDFForm->ResetForm(aFields, TRUE, TRUE);
    m_pDocument->SetChangeMark();
  }

  return TRUE;
}

FX_BOOL Document::saveAs(IJS_Context* cc,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::syncAnnotScan(IJS_Context* cc,
                                const std::vector<CJS_Value>& params,
                                CJS_Value& vRet,
                                CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::submitForm(IJS_Context* cc,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             CFX_WideString& sError) {

  int nSize = params.size();
  if (nSize < 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  CJS_Array aFields;
  CFX_WideString strURL;
  FX_BOOL bFDF = TRUE;
  FX_BOOL bEmpty = FALSE;
  CJS_Value v = params[0];
  if (v.GetType() == CJS_Value::VT_string) {
    strURL = params[0].ToCFXWideString(pRuntime);
    if (nSize > 1)
      bFDF = params[1].ToBool(pRuntime);
    if (nSize > 2)
      bEmpty = params[2].ToBool(pRuntime);
    if (nSize > 3)
      aFields.Attach(params[3].ToV8Array(pRuntime));
  } else if (v.GetType() == CJS_Value::VT_object) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object(pRuntime);
    v8::Local<v8::Value> pValue = pRuntime->GetObjectProperty(pObj, L"cURL");
    if (!pValue.IsEmpty())
      strURL = CJS_Value(pRuntime, pValue).ToCFXWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"bFDF");
    bFDF = CJS_Value(pRuntime, pValue).ToBool(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"bEmpty");
    bEmpty = CJS_Value(pRuntime, pValue).ToBool(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"aFields");
    aFields.Attach(CJS_Value(pRuntime, pValue).ToV8Array(pRuntime));
  }

  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  CPDF_InterForm* pPDFInterForm = pInterForm->GetInterForm();
  if (aFields.GetLength(pRuntime) == 0 && bEmpty) {
    if (pPDFInterForm->CheckRequiredFields(nullptr, true)) {
      pRuntime->BeginBlock();
      pInterForm->SubmitForm(strURL, FALSE);
      pRuntime->EndBlock();
    }
    return TRUE;
  }

  std::vector<CPDF_FormField*> fieldObjects;
  for (int i = 0, sz = aFields.GetLength(pRuntime); i < sz; ++i) {
    CJS_Value valName(pRuntime);
    aFields.GetElement(pRuntime, i, valName);

    CFX_WideString sName = valName.ToCFXWideString(pRuntime);
    CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
    for (int j = 0, jsz = pPDFForm->CountFields(sName); j < jsz; ++j) {
      CPDF_FormField* pField = pPDFForm->GetField(j, sName);
      if (!bEmpty && pField->GetValue().IsEmpty())
        continue;

      fieldObjects.push_back(pField);
    }
  }

  if (pPDFInterForm->CheckRequiredFields(&fieldObjects, true)) {
    pRuntime->BeginBlock();
    pInterForm->SubmitFields(strURL, fieldObjects, true, !bFDF);
    pRuntime->EndBlock();
  }
  return TRUE;
}

void Document::AttachDoc(CPDFSDK_Document* pDoc) {
  m_pDocument.Reset(pDoc);
}

FX_BOOL Document::bookmarkRoot(IJS_Context* cc,
                               CJS_PropValue& vp,
                               CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::mailDoc(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);

  // TODO(tsepez): Check maximum number of allowed params.

  FX_BOOL bUI = TRUE;
  CFX_WideString cTo = L"";
  CFX_WideString cCc = L"";
  CFX_WideString cBcc = L"";
  CFX_WideString cSubject = L"";
  CFX_WideString cMsg = L"";

  CJS_Runtime* pRuntime = pContext->GetJSRuntime();

  if (params.size() >= 1)
    bUI = params[0].ToBool(pRuntime);
  if (params.size() >= 2)
    cTo = params[1].ToCFXWideString(pRuntime);
  if (params.size() >= 3)
    cCc = params[2].ToCFXWideString(pRuntime);
  if (params.size() >= 4)
    cBcc = params[3].ToCFXWideString(pRuntime);
  if (params.size() >= 5)
    cSubject = params[4].ToCFXWideString(pRuntime);
  if (params.size() >= 6)
    cMsg = params[5].ToCFXWideString(pRuntime);

  if (params.size() >= 1 && params[0].GetType() == CJS_Value::VT_object) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object(pRuntime);

    v8::Local<v8::Value> pValue = pRuntime->GetObjectProperty(pObj, L"bUI");
    bUI = CJS_Value(pRuntime, pValue).ToInt(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cTo");
    cTo = CJS_Value(pRuntime, pValue).ToCFXWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cCc");
    cCc = CJS_Value(pRuntime, pValue).ToCFXWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cBcc");
    cBcc = CJS_Value(pRuntime, pValue).ToCFXWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cSubject");
    cSubject = CJS_Value(pRuntime, pValue).ToCFXWideString(pRuntime);

    pValue = pRuntime->GetObjectProperty(pObj, L"cMsg");
    cMsg = CJS_Value(pRuntime, pValue).ToCFXWideString(pRuntime);
  }

  pRuntime->BeginBlock();
  CPDFSDK_Environment* pEnv = pRuntime->GetReaderApp();
  pEnv->JS_docmailForm(nullptr, 0, bUI, cTo.c_str(), cSubject.c_str(),
                       cCc.c_str(), cBcc.c_str(), cMsg.c_str());
  pRuntime->EndBlock();

  return TRUE;
}

FX_BOOL Document::author(IJS_Context* cc,
                         CJS_PropValue& vp,
                         CFX_WideString& sError) {
  return getPropertyInternal(cc, vp, "Author", sError);
}

FX_BOOL Document::info(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CPDF_Dictionary* pDictionary = m_pDocument->GetPDFDocument()->GetInfo();
  if (!pDictionary)
    return FALSE;

  CFX_WideString cwAuthor = pDictionary->GetUnicodeTextFor("Author");
  CFX_WideString cwTitle = pDictionary->GetUnicodeTextFor("Title");
  CFX_WideString cwSubject = pDictionary->GetUnicodeTextFor("Subject");
  CFX_WideString cwKeywords = pDictionary->GetUnicodeTextFor("Keywords");
  CFX_WideString cwCreator = pDictionary->GetUnicodeTextFor("Creator");
  CFX_WideString cwProducer = pDictionary->GetUnicodeTextFor("Producer");
  CFX_WideString cwCreationDate =
      pDictionary->GetUnicodeTextFor("CreationDate");
  CFX_WideString cwModDate = pDictionary->GetUnicodeTextFor("ModDate");
  CFX_WideString cwTrapped = pDictionary->GetUnicodeTextFor("Trapped");

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();

  v8::Local<v8::Object> pObj = pRuntime->NewFxDynamicObj(-1);
  pRuntime->PutObjectString(pObj, L"Author", cwAuthor);
  pRuntime->PutObjectString(pObj, L"Title", cwTitle);
  pRuntime->PutObjectString(pObj, L"Subject", cwSubject);
  pRuntime->PutObjectString(pObj, L"Keywords", cwKeywords);
  pRuntime->PutObjectString(pObj, L"Creator", cwCreator);
  pRuntime->PutObjectString(pObj, L"Producer", cwProducer);
  pRuntime->PutObjectString(pObj, L"CreationDate", cwCreationDate);
  pRuntime->PutObjectString(pObj, L"ModDate", cwModDate);
  pRuntime->PutObjectString(pObj, L"Trapped", cwTrapped);

  // It's to be compatible to non-standard info dictionary.
  for (const auto& it : *pDictionary) {
    const CFX_ByteString& bsKey = it.first;
    CPDF_Object* pValueObj = it.second;
    CFX_WideString wsKey = CFX_WideString::FromUTF8(bsKey.AsStringC());
    if (pValueObj->IsString() || pValueObj->IsName()) {
      pRuntime->PutObjectString(pObj, wsKey, pValueObj->GetUnicodeText());
    } else if (pValueObj->IsNumber()) {
      pRuntime->PutObjectNumber(pObj, wsKey, (float)pValueObj->GetNumber());
    } else if (pValueObj->IsBoolean()) {
      pRuntime->PutObjectBoolean(pObj, wsKey, !!pValueObj->GetInteger());
    }
  }
  vp << pObj;
  return TRUE;
}

FX_BOOL Document::getPropertyInternal(IJS_Context* cc,
                                      CJS_PropValue& vp,
                                      const CFX_ByteString& propName,
                                      CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CPDF_Dictionary* pDictionary = m_pDocument->GetPDFDocument()->GetInfo();
  if (!pDictionary)
    return FALSE;

  if (vp.IsGetting()) {
    vp << pDictionary->GetUnicodeTextFor(propName);
  } else {
    if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) {
      sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
      return FALSE;
    }
    CFX_WideString csProperty;
    vp >> csProperty;
    pDictionary->SetStringFor(propName, PDF_EncodeText(csProperty));
    m_pDocument->SetChangeMark();
  }
  return TRUE;
}

FX_BOOL Document::creationDate(IJS_Context* cc,
                               CJS_PropValue& vp,
                               CFX_WideString& sError) {
  return getPropertyInternal(cc, vp, "CreationDate", sError);
}

FX_BOOL Document::creator(IJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError) {
  return getPropertyInternal(cc, vp, "Creator", sError);
}

FX_BOOL Document::delay(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (vp.IsGetting()) {
    vp << m_bDelay;
  } else {
    if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) {
      sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
      return FALSE;
    }
    vp >> m_bDelay;
    if (m_bDelay) {
      m_DelayData.clear();
    } else {
      std::list<std::unique_ptr<CJS_DelayData>> DelayDataToProcess;
      DelayDataToProcess.swap(m_DelayData);
      for (const auto& pData : DelayDataToProcess)
        Field::DoDelay(m_pDocument.Get(), pData.get());
    }
  }
  return TRUE;
}

FX_BOOL Document::keywords(IJS_Context* cc,
                           CJS_PropValue& vp,
                           CFX_WideString& sError) {
  return getPropertyInternal(cc, vp, "Keywords", sError);
}

FX_BOOL Document::modDate(IJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError) {
  return getPropertyInternal(cc, vp, "ModDate", sError);
}

FX_BOOL Document::producer(IJS_Context* cc,
                           CJS_PropValue& vp,
                           CFX_WideString& sError) {
  return getPropertyInternal(cc, vp, "Producer", sError);
}

FX_BOOL Document::subject(IJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError) {
  return getPropertyInternal(cc, vp, "Subject", sError);
}

FX_BOOL Document::title(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  if (!m_pDocument || !m_pDocument->GetUnderlyingDocument()) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  return getPropertyInternal(cc, vp, "Title", sError);
}

FX_BOOL Document::numPages(IJS_Context* cc,
                           CJS_PropValue& vp,
                           CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  vp << m_pDocument->GetPageCount();
  return TRUE;
}

FX_BOOL Document::external(IJS_Context* cc,
                           CJS_PropValue& vp,
                           CFX_WideString& sError) {
  // In Chrome case, should always return true.
  if (vp.IsGetting()) {
    vp << true;
  }
  return TRUE;
}

FX_BOOL Document::filesize(IJS_Context* cc,
                           CJS_PropValue& vp,
                           CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }
  vp << 0;
  return TRUE;
}

FX_BOOL Document::mouseX(IJS_Context* cc,
                         CJS_PropValue& vp,
                         CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::mouseY(IJS_Context* cc,
                         CJS_PropValue& vp,
                         CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::URL(IJS_Context* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  vp << m_pDocument->GetPath();
  return TRUE;
}

FX_BOOL Document::baseURL(IJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError) {
  if (vp.IsGetting()) {
    vp << m_cwBaseURL;
  } else {
    vp >> m_cwBaseURL;
  }
  return TRUE;
}

FX_BOOL Document::calculate(IJS_Context* cc,
                            CJS_PropValue& vp,
                            CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
  if (vp.IsGetting()) {
    vp << !!pInterForm->IsCalculateEnabled();
  } else {
    bool bCalculate;
    vp >> bCalculate;
    pInterForm->EnableCalculate(bCalculate);
  }
  return TRUE;
}

FX_BOOL Document::documentFileName(IJS_Context* cc,
                                   CJS_PropValue& vp,
                                   CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CFX_WideString wsFilePath = m_pDocument->GetPath();
  int32_t i = wsFilePath.GetLength() - 1;
  for (; i >= 0; i--) {
    if (wsFilePath.GetAt(i) == L'\\' || wsFilePath.GetAt(i) == L'/')
      break;
  }
  if (i >= 0 && i < wsFilePath.GetLength() - 1) {
    vp << (wsFilePath.GetBuffer(wsFilePath.GetLength()) + i + 1);
  } else {
    vp << L"";
  }
  return TRUE;
}

FX_BOOL Document::path(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  vp << app::SysPathToPDFPath(m_pDocument->GetPath());
  return TRUE;
}

FX_BOOL Document::pageWindowRect(IJS_Context* cc,
                                 CJS_PropValue& vp,
                                 CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::layout(IJS_Context* cc,
                         CJS_PropValue& vp,
                         CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::addLink(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::closeDoc(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::getPageBox(IJS_Context* cc,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::getAnnot(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  if (params.size() != 2) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  int nPageNo = params[0].ToInt(pRuntime);
  CFX_WideString swAnnotName = params[1].ToCFXWideString(pRuntime);
  CPDFSDK_PageView* pPageView = m_pDocument->GetPageView(nPageNo);
  if (!pPageView)
    return FALSE;

  CPDFSDK_AnnotIterator annotIterator(pPageView, false);
  CPDFSDK_BAAnnot* pSDKBAAnnot = nullptr;
  while (CPDFSDK_Annot* pSDKAnnotCur = annotIterator.Next()) {
    CPDFSDK_BAAnnot* pBAAnnot = static_cast<CPDFSDK_BAAnnot*>(pSDKAnnotCur);
    if (pBAAnnot && pBAAnnot->GetAnnotName() == swAnnotName) {
      pSDKBAAnnot = pBAAnnot;
      break;
    }
  }

  if (!pSDKBAAnnot)
    return FALSE;

  v8::Local<v8::Object> pObj =
      pRuntime->NewFxDynamicObj(CJS_Annot::g_nObjDefnID);
  if (pObj.IsEmpty())
    return FALSE;

  CJS_Annot* pJS_Annot =
      static_cast<CJS_Annot*>(pRuntime->GetObjectPrivate(pObj));
  if (!pJS_Annot)
    return FALSE;

  Annot* pAnnot = static_cast<Annot*>(pJS_Annot->GetEmbedObject());
  if (!pAnnot)
    return FALSE;

  pAnnot->SetSDKAnnot(pSDKBAAnnot);

  vRet = CJS_Value(pRuntime, pJS_Annot);
  return TRUE;
}

FX_BOOL Document::getAnnots(IJS_Context* cc,
                            const std::vector<CJS_Value>& params,
                            CJS_Value& vRet,
                            CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();

  // TODO(tonikitoo): Add support supported parameters as per
  // the PDF spec.

  int nPageNo = m_pDocument->GetPageCount();
  CJS_Array annots;

  for (int i = 0; i < nPageNo; ++i) {
    CPDFSDK_PageView* pPageView = m_pDocument->GetPageView(i);
    if (!pPageView)
      return FALSE;

    CPDFSDK_AnnotIterator annotIterator(pPageView, false);
    while (CPDFSDK_Annot* pSDKAnnotCur = annotIterator.Next()) {
      CPDFSDK_BAAnnot* pSDKBAAnnot =
          static_cast<CPDFSDK_BAAnnot*>(pSDKAnnotCur);
      if (!pSDKBAAnnot)
        return FALSE;

      v8::Local<v8::Object> pObj =
          pRuntime->NewFxDynamicObj(CJS_Annot::g_nObjDefnID);
      if (pObj.IsEmpty())
        return FALSE;

      CJS_Annot* pJS_Annot =
          static_cast<CJS_Annot*>(pRuntime->GetObjectPrivate(pObj));
      if (!pJS_Annot)
        return FALSE;

      Annot* pAnnot = static_cast<Annot*>(pJS_Annot->GetEmbedObject());
      if (!pAnnot)
        return FALSE;

      pAnnot->SetSDKAnnot(pSDKBAAnnot);
      annots.SetElement(pRuntime, i, CJS_Value(pRuntime, pJS_Annot));
    }
  }

  vRet = CJS_Value(pRuntime, annots);
  return TRUE;
}

FX_BOOL Document::getAnnot3D(IJS_Context* cc,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             CFX_WideString& sError) {
  vRet.SetNull(CJS_Runtime::FromContext(cc));
  return TRUE;
}

FX_BOOL Document::getAnnots3D(IJS_Context* cc,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::getOCGs(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::getLinks(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  return TRUE;
}

bool Document::IsEnclosedInRect(CFX_FloatRect rect, CFX_FloatRect LinkRect) {
  return (rect.left <= LinkRect.left && rect.top <= LinkRect.top &&
          rect.right >= LinkRect.right && rect.bottom >= LinkRect.bottom);
}

FX_BOOL Document::addIcon(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {

  if (params.size() != 2) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  CFX_WideString swIconName = params[0].ToCFXWideString(pRuntime);

  if (params[1].GetType() != CJS_Value::VT_object) {
    sError = JSGetStringFromID(IDS_STRING_JSTYPEERROR);
    return FALSE;
  }

  v8::Local<v8::Object> pJSIcon = params[1].ToV8Object(pRuntime);
  if (pRuntime->GetObjDefnID(pJSIcon) != CJS_Icon::g_nObjDefnID) {
    sError = JSGetStringFromID(IDS_STRING_JSTYPEERROR);
    return FALSE;
  }

  CJS_EmbedObj* pEmbedObj = params[1].ToCJSObject(pRuntime)->GetEmbedObject();
  if (!pEmbedObj) {
    sError = JSGetStringFromID(IDS_STRING_JSTYPEERROR);
    return FALSE;
  }

  m_IconList.push_back(std::unique_ptr<IconElement>(
      new IconElement(swIconName, (Icon*)pEmbedObj)));
  return TRUE;
}

FX_BOOL Document::icons(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return FALSE;
  }

  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  if (m_IconList.empty()) {
    vp.GetJSValue()->SetNull(pRuntime);
    return TRUE;
  }

  CJS_Array Icons;

  int i = 0;
  for (const auto& pIconElement : m_IconList) {
    v8::Local<v8::Object> pObj =
        pRuntime->NewFxDynamicObj(CJS_Icon::g_nObjDefnID);
    if (pObj.IsEmpty())
      return FALSE;

    CJS_Icon* pJS_Icon =
        static_cast<CJS_Icon*>(pRuntime->GetObjectPrivate(pObj));
    if (!pJS_Icon)
      return FALSE;

    Icon* pIcon = (Icon*)pJS_Icon->GetEmbedObject();
    if (!pIcon)
      return FALSE;

    pIcon->SetStream(pIconElement->IconStream->GetStream());
    pIcon->SetIconName(pIconElement->IconName);
    Icons.SetElement(pRuntime, i++, CJS_Value(pRuntime, pJS_Icon));
  }

  vp << Icons;
  return TRUE;
}

FX_BOOL Document::getIcon(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  if (m_IconList.empty())
    return FALSE;

  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  CFX_WideString swIconName = params[0].ToCFXWideString(pRuntime);

  for (const auto& pIconElement : m_IconList) {
    if (pIconElement->IconName == swIconName) {
      Icon* pRetIcon = pIconElement->IconStream;

      v8::Local<v8::Object> pObj =
          pRuntime->NewFxDynamicObj(CJS_Icon::g_nObjDefnID);
      if (pObj.IsEmpty())
        return FALSE;

      CJS_Icon* pJS_Icon =
          static_cast<CJS_Icon*>(pRuntime->GetObjectPrivate(pObj));
      if (!pJS_Icon)
        return FALSE;

      Icon* pIcon = (Icon*)pJS_Icon->GetEmbedObject();
      if (!pIcon)
        return FALSE;

      pIcon->SetIconName(swIconName);
      pIcon->SetStream(pRetIcon->GetStream());
      vRet = CJS_Value(pRuntime, pJS_Icon);
      return TRUE;
    }
  }

  return FALSE;
}

FX_BOOL Document::removeIcon(IJS_Context* cc,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             CFX_WideString& sError) {
  // Unsafe, no supported.
  return TRUE;
}

FX_BOOL Document::createDataObject(IJS_Context* cc,
                                   const std::vector<CJS_Value>& params,
                                   CJS_Value& vRet,
                                   CFX_WideString& sError) {
  // Unsafe, not implemented.
  return TRUE;
}

FX_BOOL Document::media(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::calculateNow(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (!(m_pDocument->GetPermissions(FPDFPERM_MODIFY) ||
        m_pDocument->GetPermissions(FPDFPERM_ANNOT_FORM) ||
        m_pDocument->GetPermissions(FPDFPERM_FILL_FORM))) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return FALSE;
  }
  m_pDocument->GetInterForm()->OnCalculate();
  return TRUE;
}

FX_BOOL Document::Collab(IJS_Context* cc,
                         CJS_PropValue& vp,
                         CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::getPageNthWord(IJS_Context* cc,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return FALSE;
  }
  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);

  // TODO(tsepez): check maximum allowable params.

  int nPageNo = params.size() > 0 ? params[0].ToInt(pRuntime) : 0;
  int nWordNo = params.size() > 1 ? params[1].ToInt(pRuntime) : 0;
  bool bStrip = params.size() > 2 ? params[2].ToBool(pRuntime) : true;

  CPDF_Document* pDocument = m_pDocument->GetPDFDocument();
  if (!pDocument)
    return FALSE;

  if (nPageNo < 0 || nPageNo >= pDocument->GetPageCount()) {
    sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
    return FALSE;
  }

  CPDF_Dictionary* pPageDict = pDocument->GetPage(nPageNo);
  if (!pPageDict)
    return FALSE;

  CPDF_Page page(pDocument, pPageDict, true);
  page.ParseContent();

  int nWords = 0;
  CFX_WideString swRet;
  for (auto& pPageObj : *page.GetPageObjectList()) {
    if (pPageObj->IsText()) {
      CPDF_TextObject* pTextObj = pPageObj->AsText();
      int nObjWords = CountWords(pTextObj);
      if (nWords + nObjWords >= nWordNo) {
        swRet = GetObjWordStr(pTextObj, nWordNo - nWords);
        break;
      }
      nWords += nObjWords;
    }
  }

  if (bStrip) {
    swRet.TrimLeft();
    swRet.TrimRight();
  }

  vRet = CJS_Value(pRuntime, swRet.c_str());
  return TRUE;
}

FX_BOOL Document::getPageNthWordQuads(IJS_Context* cc,
                                      const std::vector<CJS_Value>& params,
                                      CJS_Value& vRet,
                                      CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  return FALSE;
}

FX_BOOL Document::getPageNumWords(IJS_Context* cc,
                                  const std::vector<CJS_Value>& params,
                                  CJS_Value& vRet,
                                  CFX_WideString& sError) {
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) {
    sError = JSGetStringFromID(IDS_STRING_JSNOPERMISSION);
    return FALSE;
  }
  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  int nPageNo = params.size() > 0 ? params[0].ToInt(pRuntime) : 0;
  CPDF_Document* pDocument = m_pDocument->GetPDFDocument();
  if (nPageNo < 0 || nPageNo >= pDocument->GetPageCount()) {
    sError = JSGetStringFromID(IDS_STRING_JSVALUEERROR);
    return FALSE;
  }

  CPDF_Dictionary* pPageDict = pDocument->GetPage(nPageNo);
  if (!pPageDict)
    return FALSE;

  CPDF_Page page(pDocument, pPageDict, true);
  page.ParseContent();

  int nWords = 0;
  for (auto& pPageObj : *page.GetPageObjectList()) {
    if (pPageObj->IsText())
      nWords += CountWords(pPageObj->AsText());
  }

  vRet = CJS_Value(pRuntime, nWords);
  return TRUE;
}

FX_BOOL Document::getPrintParams(IJS_Context* cc,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  v8::Local<v8::Object> pRetObj =
      pRuntime->NewFxDynamicObj(CJS_PrintParamsObj::g_nObjDefnID);

  // Not implemented yet.

  vRet = CJS_Value(pRuntime, pRetObj);
  return TRUE;
}

#define ISLATINWORD(u) (u != 0x20 && u <= 0x28FF)

int Document::CountWords(CPDF_TextObject* pTextObj) {
  if (!pTextObj)
    return 0;

  int nWords = 0;

  CPDF_Font* pFont = pTextObj->GetFont();
  if (!pFont)
    return 0;

  FX_BOOL bIsLatin = FALSE;

  for (int i = 0, sz = pTextObj->CountChars(); i < sz; i++) {
    uint32_t charcode = CPDF_Font::kInvalidCharCode;
    FX_FLOAT kerning;

    pTextObj->GetCharInfo(i, charcode, kerning);
    CFX_WideString swUnicode = pFont->UnicodeFromCharCode(charcode);

    uint16_t unicode = 0;
    if (swUnicode.GetLength() > 0)
      unicode = swUnicode[0];

    if (ISLATINWORD(unicode) && bIsLatin)
      continue;

    bIsLatin = ISLATINWORD(unicode);
    if (unicode != 0x20)
      nWords++;
  }

  return nWords;
}

CFX_WideString Document::GetObjWordStr(CPDF_TextObject* pTextObj,
                                       int nWordIndex) {
  CFX_WideString swRet;

  CPDF_Font* pFont = pTextObj->GetFont();
  if (!pFont)
    return L"";

  int nWords = 0;
  FX_BOOL bIsLatin = FALSE;

  for (int i = 0, sz = pTextObj->CountChars(); i < sz; i++) {
    uint32_t charcode = CPDF_Font::kInvalidCharCode;
    FX_FLOAT kerning;

    pTextObj->GetCharInfo(i, charcode, kerning);
    CFX_WideString swUnicode = pFont->UnicodeFromCharCode(charcode);

    uint16_t unicode = 0;
    if (swUnicode.GetLength() > 0)
      unicode = swUnicode[0];

    if (ISLATINWORD(unicode) && bIsLatin) {
    } else {
      bIsLatin = ISLATINWORD(unicode);
      if (unicode != 0x20)
        nWords++;
    }

    if (nWords - 1 == nWordIndex)
      swRet += unicode;
  }

  return swRet;
}

FX_BOOL Document::zoom(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  return TRUE;
}

/**
(none,  NoVary)
(fitP,  FitPage)
(fitW,  FitWidth)
(fitH,  FitHeight)
(fitV,  FitVisibleWidth)
(pref,  Preferred)
(refW,  ReflowWidth)
*/

FX_BOOL Document::zoomType(IJS_Context* cc,
                           CJS_PropValue& vp,
                           CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL Document::deletePages(IJS_Context* cc,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              CFX_WideString& sError) {
  // Unsafe, no supported.
  return TRUE;
}

FX_BOOL Document::extractPages(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::insertPages(IJS_Context* cc,
                              const std::vector<CJS_Value>& params,
                              CJS_Value& vRet,
                              CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::replacePages(IJS_Context* cc,
                               const std::vector<CJS_Value>& params,
                               CJS_Value& vRet,
                               CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::getURL(IJS_Context* cc,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::gotoNamedDest(IJS_Context* cc,
                                const std::vector<CJS_Value>& params,
                                CJS_Value& vRet,
                                CFX_WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  if (!m_pDocument) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return FALSE;
  }
  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  CFX_WideString wideName = params[0].ToCFXWideString(pRuntime);
  CFX_ByteString utf8Name = wideName.UTF8Encode();
  CPDF_Document* pDocument = m_pDocument->GetPDFDocument();
  if (!pDocument)
    return FALSE;

  CPDF_NameTree nameTree(pDocument, "Dests");
  CPDF_Array* destArray = nameTree.LookupNamedDest(pDocument, utf8Name);
  if (!destArray)
    return FALSE;

  CPDF_Dest dest(destArray);
  const CPDF_Array* arrayObject = ToArray(dest.GetObject());

  std::unique_ptr<float[]> scrollPositionArray;
  int scrollPositionArraySize = 0;

  if (arrayObject) {
    scrollPositionArray.reset(new float[arrayObject->GetCount()]);
    int j = 0;
    for (size_t i = 2; i < arrayObject->GetCount(); i++)
      scrollPositionArray[j++] = arrayObject->GetFloatAt(i);
    scrollPositionArraySize = j;
  }

  pRuntime->BeginBlock();
  CPDFSDK_Environment* pApp = m_pDocument->GetEnv();
  pApp->DoGoToAction(dest.GetPageIndex(pDocument), dest.GetZoomMode(),
                     scrollPositionArray.get(), scrollPositionArraySize);
  pRuntime->EndBlock();

  return TRUE;
}

void Document::AddDelayData(CJS_DelayData* pData) {
  m_DelayData.push_back(std::unique_ptr<CJS_DelayData>(pData));
}

void Document::DoFieldDelay(const CFX_WideString& sFieldName,
                            int nControlIndex) {
  std::vector<std::unique_ptr<CJS_DelayData>> DelayDataForFieldAndControlIndex;
  auto iter = m_DelayData.begin();
  while (iter != m_DelayData.end()) {
    auto old = iter++;
    if ((*old)->sFieldName == sFieldName &&
        (*old)->nControlIndex == nControlIndex) {
      DelayDataForFieldAndControlIndex.push_back(std::move(*old));
      m_DelayData.erase(old);
    }
  }

  for (const auto& pData : DelayDataForFieldAndControlIndex)
    Field::DoDelay(m_pDocument.Get(), pData.get());
}

CJS_Document* Document::GetCJSDoc() const {
  return static_cast<CJS_Document*>(m_pJSObject);
}
