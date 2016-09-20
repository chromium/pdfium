// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/include/cpdfxfa_docenvironment.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_string.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream_acc.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_page.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/javascript/ijs_runtime.h"
#include "xfa/fxfa/include/xfa_ffdocview.h"
#include "xfa/fxfa/include/xfa_ffwidget.h"
#include "xfa/fxfa/include/xfa_ffwidgethandler.h"

#define IDS_XFA_Validate_Input                                          \
  "At least one required field was empty. Please fill in the required " \
  "fields\r\n(highlighted) before continuing."

// submit
#define FXFA_CONFIG 0x00000001
#define FXFA_TEMPLATE 0x00000010
#define FXFA_LOCALESET 0x00000100
#define FXFA_DATASETS 0x00001000
#define FXFA_XMPMETA 0x00010000
#define FXFA_XFDF 0x00100000
#define FXFA_FORM 0x01000000
#define FXFA_PDF 0x10000000
#define FXFA_XFA_ALL 0x01111111

CPDFXFA_DocEnvironment::CPDFXFA_DocEnvironment(CPDFXFA_Document* doc)
    : m_pDocument(doc), m_pJSContext(nullptr) {
  ASSERT(m_pDocument);
}

CPDFXFA_DocEnvironment::~CPDFXFA_DocEnvironment() {
  if (m_pJSContext && m_pDocument->GetSDKDoc() &&
      m_pDocument->GetSDKDoc()->GetEnv())
    m_pDocument->GetSDKDoc()->GetEnv()->GetJSRuntime()->ReleaseContext(
        m_pJSContext);
}

void CPDFXFA_DocEnvironment::SetChangeMark(CXFA_FFDoc* hDoc) {
  if (hDoc == m_pDocument->GetXFADoc() && m_pDocument->GetSDKDoc())
    m_pDocument->GetSDKDoc()->SetChangeMark();
}

void CPDFXFA_DocEnvironment::InvalidateRect(CXFA_FFPageView* pPageView,
                                            const CFX_RectF& rt,
                                            uint32_t dwFlags /* = 0 */) {
  if (!m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc())
    return;

  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA)
    return;

  CPDFXFA_Page* pPage = m_pDocument->GetXFAPage(pPageView);
  if (!pPage)
    return;

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return;

  CFX_FloatRect rcPage = CFX_FloatRect::FromCFXRectF(rt);
  pEnv->Invalidate((FPDF_PAGE)pPage, rcPage.left, rcPage.bottom, rcPage.right,
                   rcPage.top);
}

void CPDFXFA_DocEnvironment::DisplayCaret(CXFA_FFWidget* hWidget,
                                          FX_BOOL bVisible,
                                          const CFX_RectF* pRtAnchor) {
  if (!hWidget || !pRtAnchor || !m_pDocument->GetXFADoc() ||
      !m_pDocument->GetSDKDoc() || !m_pDocument->GetXFADocView())
    return;

  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA)
    return;

  CXFA_FFWidgetHandler* pWidgetHandler =
      m_pDocument->GetXFADocView()->GetWidgetHandler();
  if (!pWidgetHandler)
    return;

  CXFA_FFPageView* pPageView = hWidget->GetPageView();
  if (!pPageView)
    return;

  CPDFXFA_Page* pPage = m_pDocument->GetXFAPage(pPageView);
  if (!pPage)
    return;

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return;

  CFX_FloatRect rcCaret = CFX_FloatRect::FromCFXRectF(*pRtAnchor);
  pEnv->DisplayCaret((FPDF_PAGE)pPage, bVisible, rcCaret.left, rcCaret.top,
                     rcCaret.right, rcCaret.bottom);
}

FX_BOOL CPDFXFA_DocEnvironment::GetPopupPos(CXFA_FFWidget* hWidget,
                                            FX_FLOAT fMinPopup,
                                            FX_FLOAT fMaxPopup,
                                            const CFX_RectF& rtAnchor,
                                            CFX_RectF& rtPopup) {
  if (!hWidget)
    return FALSE;

  CXFA_FFPageView* pXFAPageView = hWidget->GetPageView();
  if (!pXFAPageView)
    return FALSE;

  CPDFXFA_Page* pPage = m_pDocument->GetXFAPage(pXFAPageView);
  if (!pPage)
    return FALSE;

  CXFA_WidgetAcc* pWidgetAcc = hWidget->GetDataAcc();
  int nRotate = pWidgetAcc->GetRotate();
  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return FALSE;

  FS_RECTF pageViewRect = {0.0f, 0.0f, 0.0f, 0.0f};
  pEnv->GetPageViewRect(pPage, pageViewRect);

  int t1;
  int t2;
  CFX_FloatRect rcAnchor = CFX_FloatRect::FromCFXRectF(rtAnchor);
  switch (nRotate) {
    case 90: {
      t1 = (int)(pageViewRect.right - rcAnchor.right);
      t2 = (int)(rcAnchor.left - pageViewRect.left);
      if (rcAnchor.bottom < pageViewRect.bottom)
        rtPopup.left += rcAnchor.bottom - pageViewRect.bottom;
      break;
    }
    case 180: {
      t2 = (int)(pageViewRect.top - rcAnchor.top);
      t1 = (int)(rcAnchor.bottom - pageViewRect.bottom);
      if (rcAnchor.left < pageViewRect.left)
        rtPopup.left += rcAnchor.left - pageViewRect.left;
      break;
    }
    case 270: {
      t1 = (int)(rcAnchor.left - pageViewRect.left);
      t2 = (int)(pageViewRect.right - rcAnchor.right);
      if (rcAnchor.top > pageViewRect.top)
        rtPopup.left -= rcAnchor.top - pageViewRect.top;
      break;
    }
    case 0:
    default: {
      t1 = (int)(pageViewRect.top - rcAnchor.top);
      t2 = (int)(rcAnchor.bottom - pageViewRect.bottom);
      if (rcAnchor.right > pageViewRect.right)
        rtPopup.left -= rcAnchor.right - pageViewRect.right;
      break;
    }
  }

  int t;
  uint32_t dwPos;
  if (t1 <= 0 && t2 <= 0)
    return FALSE;
  if (t1 <= 0) {
    t = t2;
    dwPos = 1;
  } else if (t2 <= 0) {
    t = t1;
    dwPos = 0;
  } else if (t1 > t2) {
    t = t1;
    dwPos = 0;
  } else {
    t = t2;
    dwPos = 1;
  }

  FX_FLOAT fPopupHeight;
  if (t < fMinPopup)
    fPopupHeight = fMinPopup;
  else if (t > fMaxPopup)
    fPopupHeight = fMaxPopup;
  else
    fPopupHeight = static_cast<FX_FLOAT>(t);

  switch (nRotate) {
    case 0:
    case 180: {
      if (dwPos == 0) {
        rtPopup.top = rtAnchor.height;
        rtPopup.height = fPopupHeight;
      } else {
        rtPopup.top = -fPopupHeight;
        rtPopup.height = fPopupHeight;
      }
      break;
    }
    case 90:
    case 270: {
      if (dwPos == 0) {
        rtPopup.top = rtAnchor.width;
        rtPopup.height = fPopupHeight;
      } else {
        rtPopup.top = -fPopupHeight;
        rtPopup.height = fPopupHeight;
      }
      break;
    }
    default:
      break;
  }

  return TRUE;
}

FX_BOOL CPDFXFA_DocEnvironment::PopupMenu(CXFA_FFWidget* hWidget,
                                          CFX_PointF ptPopup) {
  if (!hWidget)
    return FALSE;

  CXFA_FFPageView* pXFAPageView = hWidget->GetPageView();
  if (!pXFAPageView)
    return FALSE;

  CPDFXFA_Page* pPage = m_pDocument->GetXFAPage(pXFAPageView);
  if (!pPage)
    return FALSE;

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return FALSE;

  int menuFlag = 0;
  if (hWidget->CanUndo())
    menuFlag |= FXFA_MENU_UNDO;
  if (hWidget->CanRedo())
    menuFlag |= FXFA_MENU_REDO;
  if (hWidget->CanPaste())
    menuFlag |= FXFA_MENU_PASTE;
  if (hWidget->CanCopy())
    menuFlag |= FXFA_MENU_COPY;
  if (hWidget->CanCut())
    menuFlag |= FXFA_MENU_CUT;
  if (hWidget->CanSelectAll())
    menuFlag |= FXFA_MENU_SELECTALL;

  return pEnv->PopupMenu(pPage, hWidget, menuFlag, ptPopup, nullptr);
}

void CPDFXFA_DocEnvironment::PageViewEvent(CXFA_FFPageView* pPageView,
                                           uint32_t dwFlags) {
  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return;

  if (m_pDocument->GetLoadStatus() == FXFA_LOADSTATUS_LOADING ||
      m_pDocument->GetLoadStatus() == FXFA_LOADSTATUS_CLOSING ||
      XFA_PAGEVIEWEVENT_StopLayout != dwFlags)
    return;

  int nNewCount = m_pDocument->GetPageCount();
  if (nNewCount == m_pDocument->GetOriginalPageCount())
    return;

  CXFA_FFDocView* pXFADocView = m_pDocument->GetXFADocView();
  if (!pXFADocView)
    return;

  for (int iPageIter = 0; iPageIter < m_pDocument->GetOriginalPageCount();
       iPageIter++) {
    CPDFXFA_Page* pPage = m_pDocument->GetXFAPageList()->GetAt(iPageIter);
    if (!pPage)
      continue;

    m_pDocument->GetSDKDoc()->RemovePageView(pPage);
    pPage->SetXFAPageView(pXFADocView->GetPageView(iPageIter));
  }

  int flag = (nNewCount < m_pDocument->GetOriginalPageCount())
                 ? FXFA_PAGEVIEWEVENT_POSTREMOVED
                 : FXFA_PAGEVIEWEVENT_POSTADDED;
  int count = FXSYS_abs(nNewCount - m_pDocument->GetOriginalPageCount());
  m_pDocument->SetOriginalPageCount(nNewCount);
  pEnv->PageEvent(count, flag);
}

void CPDFXFA_DocEnvironment::WidgetPostAdd(CXFA_FFWidget* hWidget,
                                           CXFA_WidgetAcc* pWidgetData) {
  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA || !hWidget)
    return;

  CXFA_FFPageView* pPageView = hWidget->GetPageView();
  if (!pPageView)
    return;

  CPDFXFA_Page* pXFAPage = m_pDocument->GetXFAPage(pPageView);
  if (!pXFAPage)
    return;

  m_pDocument->GetSDKDoc()->GetPageView(pXFAPage, true)->AddAnnot(hWidget);
}

void CPDFXFA_DocEnvironment::WidgetPreRemove(CXFA_FFWidget* hWidget,
                                             CXFA_WidgetAcc* pWidgetData) {
  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA || !hWidget)
    return;

  CXFA_FFPageView* pPageView = hWidget->GetPageView();
  if (!pPageView)
    return;

  CPDFXFA_Page* pXFAPage = m_pDocument->GetXFAPage(pPageView);
  if (!pXFAPage)
    return;

  CPDFSDK_PageView* pSdkPageView =
      m_pDocument->GetSDKDoc()->GetPageView(pXFAPage, true);
  if (CPDFSDK_Annot* pAnnot = pSdkPageView->GetAnnotByXFAWidget(hWidget))
    pSdkPageView->DeleteAnnot(pAnnot);
}

int32_t CPDFXFA_DocEnvironment::CountPages(CXFA_FFDoc* hDoc) {
  if (hDoc == m_pDocument->GetXFADoc() && m_pDocument->GetSDKDoc())
    return m_pDocument->GetPageCount();
  return 0;
}

int32_t CPDFXFA_DocEnvironment::GetCurrentPage(CXFA_FFDoc* hDoc) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc())
    return -1;
  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA)
    return -1;

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return -1;

  return pEnv->GetCurrentPageIndex(this);
}

void CPDFXFA_DocEnvironment::SetCurrentPage(CXFA_FFDoc* hDoc,
                                            int32_t iCurPage) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc() ||
      m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA || iCurPage < 0 ||
      iCurPage >= m_pDocument->GetSDKDoc()->GetPageCount()) {
    return;
  }

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return;
  pEnv->SetCurrentPage(this, iCurPage);
}

FX_BOOL CPDFXFA_DocEnvironment::IsCalculationsEnabled(CXFA_FFDoc* hDoc) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc())
    return FALSE;
  if (m_pDocument->GetSDKDoc()->GetInterForm())
    return m_pDocument->GetSDKDoc()->GetInterForm()->IsXfaCalculateEnabled();
  return FALSE;
}

void CPDFXFA_DocEnvironment::SetCalculationsEnabled(CXFA_FFDoc* hDoc,
                                                    FX_BOOL bEnabled) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc())
    return;
  if (m_pDocument->GetSDKDoc()->GetInterForm())
    m_pDocument->GetSDKDoc()->GetInterForm()->XfaEnableCalculate(bEnabled);
}

void CPDFXFA_DocEnvironment::GetTitle(CXFA_FFDoc* hDoc,
                                      CFX_WideString& wsTitle) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetPDFDoc())
    return;

  CPDF_Dictionary* pInfoDict = m_pDocument->GetPDFDoc()->GetInfo();
  if (!pInfoDict)
    return;

  CFX_ByteString csTitle = pInfoDict->GetStringFor("Title");
  wsTitle = wsTitle.FromLocal(csTitle.GetBuffer(csTitle.GetLength()));
  csTitle.ReleaseBuffer(csTitle.GetLength());
}

void CPDFXFA_DocEnvironment::SetTitle(CXFA_FFDoc* hDoc,
                                      const CFX_WideString& wsTitle) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetPDFDoc())
    return;

  if (CPDF_Dictionary* pInfoDict = m_pDocument->GetPDFDoc()->GetInfo())
    pInfoDict->SetFor("Title", new CPDF_String(wsTitle));
}

void CPDFXFA_DocEnvironment::ExportData(CXFA_FFDoc* hDoc,
                                        const CFX_WideString& wsFilePath,
                                        FX_BOOL bXDP) {
  if (hDoc != m_pDocument->GetXFADoc())
    return;

  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA &&
      m_pDocument->GetDocType() != DOCTYPE_STATIC_XFA)
    return;

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return;

  int fileType = bXDP ? FXFA_SAVEAS_XDP : FXFA_SAVEAS_XML;
  CFX_ByteString bs = wsFilePath.UTF16LE_Encode();
  if (wsFilePath.IsEmpty()) {
    if (!pEnv->GetFormFillInfo() || !pEnv->GetFormFillInfo()->m_pJsPlatform)
      return;

    CFX_WideString filepath = pEnv->JS_fieldBrowse();
    bs = filepath.UTF16LE_Encode();
  }
  int len = bs.GetLength();
  FPDF_FILEHANDLER* pFileHandler =
      pEnv->OpenFile(bXDP ? FXFA_SAVEAS_XDP : FXFA_SAVEAS_XML,
                     (FPDF_WIDESTRING)bs.GetBuffer(len), "wb");
  bs.ReleaseBuffer(len);
  if (!pFileHandler)
    return;

  CFPDF_FileStream fileWrite(pFileHandler);
  CFX_ByteString content;
  if (fileType == FXFA_SAVEAS_XML) {
    content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
    fileWrite.WriteBlock(content.c_str(), fileWrite.GetSize(),
                         content.GetLength());
    m_pDocument->GetXFADocView()->GetDoc()->SavePackage(XFA_HASHCODE_Data,
                                                        &fileWrite, nullptr);
  } else if (fileType == FXFA_SAVEAS_XDP) {
    if (!m_pDocument->GetPDFDoc())
      return;

    CPDF_Dictionary* pRoot = m_pDocument->GetPDFDoc()->GetRoot();
    if (!pRoot)
      return;

    CPDF_Dictionary* pAcroForm = pRoot->GetDictFor("AcroForm");
    if (!pAcroForm)
      return;

    CPDF_Array* pArray = ToArray(pAcroForm->GetObjectFor("XFA"));
    if (!pArray)
      return;

    int size = pArray->GetCount();
    for (int i = 1; i < size; i += 2) {
      CPDF_Object* pPDFObj = pArray->GetObjectAt(i);
      CPDF_Object* pPrePDFObj = pArray->GetObjectAt(i - 1);
      if (!pPrePDFObj->IsString())
        continue;
      if (!pPDFObj->IsReference())
        continue;

      CPDF_Stream* pStream = ToStream(pPDFObj->GetDirect());
      if (!pStream)
        continue;
      if (pPrePDFObj->GetString() == "form") {
        m_pDocument->GetXFADocView()->GetDoc()->SavePackage(
            XFA_HASHCODE_Form, &fileWrite, nullptr);
        continue;
      }
      if (pPrePDFObj->GetString() == "datasets") {
        m_pDocument->GetXFADocView()->GetDoc()->SavePackage(
            XFA_HASHCODE_Datasets, &fileWrite, nullptr);
        continue;
      }
      if (i == size - 1) {
        CFX_WideString wPath = CFX_WideString::FromUTF16LE(
            reinterpret_cast<const unsigned short*>(bs.c_str()),
            bs.GetLength() / sizeof(unsigned short));
        CFX_ByteString bPath = wPath.UTF8Encode();
        const char* szFormat =
            "\n<pdf href=\"%s\" xmlns=\"http://ns.adobe.com/xdp/pdf/\"/>";
        content.Format(szFormat, bPath.c_str());
        fileWrite.WriteBlock(content.c_str(), fileWrite.GetSize(),
                             content.GetLength());
      }
      std::unique_ptr<CPDF_StreamAcc> pAcc(new CPDF_StreamAcc);
      pAcc->LoadAllData(pStream);
      fileWrite.WriteBlock(pAcc->GetData(), fileWrite.GetSize(),
                           pAcc->GetSize());
    }
  }
  if (!fileWrite.Flush()) {
    // Ignoring flush error.
  }
}

void CPDFXFA_DocEnvironment::GotoURL(CXFA_FFDoc* hDoc,
                                     const CFX_WideString& bsURL,
                                     FX_BOOL bAppend) {
  if (hDoc != m_pDocument->GetXFADoc())
    return;

  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA)
    return;

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return;

  CFX_WideStringC str(bsURL.c_str());

  pEnv->GotoURL(this, str, bAppend);
}

FX_BOOL CPDFXFA_DocEnvironment::IsValidationsEnabled(CXFA_FFDoc* hDoc) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc())
    return FALSE;
  if (m_pDocument->GetSDKDoc()->GetInterForm())
    return m_pDocument->GetSDKDoc()->GetInterForm()->IsXfaValidationsEnabled();
  return TRUE;
}

void CPDFXFA_DocEnvironment::SetValidationsEnabled(CXFA_FFDoc* hDoc,
                                                   FX_BOOL bEnabled) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc())
    return;
  if (m_pDocument->GetSDKDoc()->GetInterForm())
    m_pDocument->GetSDKDoc()->GetInterForm()->XfaSetValidationsEnabled(
        bEnabled);
}

void CPDFXFA_DocEnvironment::SetFocusWidget(CXFA_FFDoc* hDoc,
                                            CXFA_FFWidget* hWidget) {
  if (hDoc != m_pDocument->GetXFADoc())
    return;

  if (!hWidget) {
    m_pDocument->GetSDKDoc()->SetFocusAnnot(nullptr);
    return;
  }

  int pageViewCount = m_pDocument->GetSDKDoc()->GetPageViewCount();
  for (int i = 0; i < pageViewCount; i++) {
    CPDFSDK_PageView* pPageView = m_pDocument->GetSDKDoc()->GetPageView(i);
    if (!pPageView)
      continue;

    CPDFSDK_Annot* pAnnot = pPageView->GetAnnotByXFAWidget(hWidget);
    if (pAnnot) {
      m_pDocument->GetSDKDoc()->SetFocusAnnot(pAnnot);
      break;
    }
  }
}

void CPDFXFA_DocEnvironment::Print(CXFA_FFDoc* hDoc,
                                   int32_t nStartPage,
                                   int32_t nEndPage,
                                   uint32_t dwOptions) {
  if (hDoc != m_pDocument->GetXFADoc())
    return;

  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv || !pEnv->GetFormFillInfo() ||
      !pEnv->GetFormFillInfo()->m_pJsPlatform ||
      !pEnv->GetFormFillInfo()->m_pJsPlatform->Doc_print) {
    return;
  }

  pEnv->GetFormFillInfo()->m_pJsPlatform->Doc_print(
      pEnv->GetFormFillInfo()->m_pJsPlatform,
      dwOptions & XFA_PRINTOPT_ShowDialog, nStartPage, nEndPage,
      dwOptions & XFA_PRINTOPT_CanCancel, dwOptions & XFA_PRINTOPT_ShrinkPage,
      dwOptions & XFA_PRINTOPT_AsImage, dwOptions & XFA_PRINTOPT_ReverseOrder,
      dwOptions & XFA_PRINTOPT_PrintAnnot);
}

FX_ARGB CPDFXFA_DocEnvironment::GetHighlightColor(CXFA_FFDoc* hDoc) {
  if (hDoc != m_pDocument->GetXFADoc() || !m_pDocument->GetSDKDoc())
    return 0;

  CPDFSDK_InterForm* pInterForm = m_pDocument->GetSDKDoc()->GetInterForm();
  if (!pInterForm)
    return 0;

  return ArgbEncode(pInterForm->GetHighlightAlpha(),
                    pInterForm->GetHighlightColor(FPDF_FORMFIELD_XFA));
}

FX_BOOL CPDFXFA_DocEnvironment::NotifySubmit(FX_BOOL bPrevOrPost) {
  if (bPrevOrPost)
    return OnBeforeNotifySubmit();

  OnAfterNotifySubmit();
  return TRUE;
}

FX_BOOL CPDFXFA_DocEnvironment::OnBeforeNotifySubmit() {
  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA &&
      m_pDocument->GetDocType() != DOCTYPE_STATIC_XFA)
    return TRUE;

  if (!m_pDocument->GetXFADocView())
    return TRUE;

  CXFA_FFWidgetHandler* pWidgetHandler =
      m_pDocument->GetXFADocView()->GetWidgetHandler();
  if (!pWidgetHandler)
    return TRUE;

  std::unique_ptr<CXFA_WidgetAccIterator> pWidgetAccIterator(
      m_pDocument->GetXFADocView()->CreateWidgetAccIterator());
  if (pWidgetAccIterator) {
    CXFA_EventParam Param;
    Param.m_eType = XFA_EVENT_PreSubmit;
    while (CXFA_WidgetAcc* pWidgetAcc = pWidgetAccIterator->MoveToNext())
      pWidgetHandler->ProcessEvent(pWidgetAcc, &Param);
  }

  pWidgetAccIterator.reset(
      m_pDocument->GetXFADocView()->CreateWidgetAccIterator());
  if (!pWidgetAccIterator)
    return TRUE;

  CXFA_WidgetAcc* pWidgetAcc = pWidgetAccIterator->MoveToNext();
  pWidgetAcc = pWidgetAccIterator->MoveToNext();
  while (pWidgetAcc) {
    int fRet = pWidgetAcc->ProcessValidate(-1);
    if (fRet == XFA_EVENTERROR_Error) {
      CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
      if (!pEnv)
        return FALSE;

      CFX_WideString ws;
      ws.FromLocal(IDS_XFA_Validate_Input);
      CFX_ByteString bs = ws.UTF16LE_Encode();
      int len = bs.GetLength();
      pEnv->Alert((FPDF_WIDESTRING)bs.GetBuffer(len), (FPDF_WIDESTRING)L"", 0,
                  1);
      bs.ReleaseBuffer(len);
      return FALSE;
    }
    pWidgetAcc = pWidgetAccIterator->MoveToNext();
  }
  m_pDocument->GetXFADocView()->UpdateDocView();

  return TRUE;
}

void CPDFXFA_DocEnvironment::OnAfterNotifySubmit() {
  if (m_pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA &&
      m_pDocument->GetDocType() != DOCTYPE_STATIC_XFA)
    return;

  if (!m_pDocument->GetXFADocView())
    return;

  CXFA_FFWidgetHandler* pWidgetHandler =
      m_pDocument->GetXFADocView()->GetWidgetHandler();
  if (!pWidgetHandler)
    return;

  std::unique_ptr<CXFA_WidgetAccIterator> pWidgetAccIterator(
      m_pDocument->GetXFADocView()->CreateWidgetAccIterator());
  if (!pWidgetAccIterator)
    return;

  CXFA_EventParam Param;
  Param.m_eType = XFA_EVENT_PostSubmit;
  CXFA_WidgetAcc* pWidgetAcc = pWidgetAccIterator->MoveToNext();
  while (pWidgetAcc) {
    pWidgetHandler->ProcessEvent(pWidgetAcc, &Param);
    pWidgetAcc = pWidgetAccIterator->MoveToNext();
  }
  m_pDocument->GetXFADocView()->UpdateDocView();
}

FX_BOOL CPDFXFA_DocEnvironment::SubmitData(CXFA_FFDoc* hDoc,
                                           CXFA_Submit submit) {
  if (!NotifySubmit(TRUE) || !m_pDocument->GetXFADocView())
    return FALSE;

  m_pDocument->GetXFADocView()->UpdateDocView();
  FX_BOOL ret = SubmitDataInternal(hDoc, submit);
  NotifySubmit(FALSE);
  return ret;
}

IFX_FileRead* CPDFXFA_DocEnvironment::OpenLinkedFile(
    CXFA_FFDoc* hDoc,
    const CFX_WideString& wsLink) {
  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return FALSE;

  CFX_ByteString bs = wsLink.UTF16LE_Encode();
  int len = bs.GetLength();
  FPDF_FILEHANDLER* pFileHandler =
      pEnv->OpenFile(0, (FPDF_WIDESTRING)bs.GetBuffer(len), "rb");
  bs.ReleaseBuffer(len);

  if (!pFileHandler)
    return nullptr;
  return new CFPDF_FileStream(pFileHandler);
}

FX_BOOL CPDFXFA_DocEnvironment::ExportSubmitFile(FPDF_FILEHANDLER* pFileHandler,
                                                 int fileType,
                                                 FPDF_DWORD encodeType,
                                                 FPDF_DWORD flag) {
  if (!m_pDocument->GetXFADocView())
    return FALSE;

  CFX_ByteString content;
  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return FALSE;

  CFPDF_FileStream fileStream(pFileHandler);
  if (fileType == FXFA_SAVEAS_XML) {
    const char kContent[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
    fileStream.WriteBlock(kContent, 0, strlen(kContent));
    m_pDocument->GetXFADoc()->SavePackage(XFA_HASHCODE_Data, &fileStream,
                                          nullptr);
    return TRUE;
  }

  if (fileType != FXFA_SAVEAS_XDP)
    return TRUE;

  if (!flag) {
    flag = FXFA_CONFIG | FXFA_TEMPLATE | FXFA_LOCALESET | FXFA_DATASETS |
           FXFA_XMPMETA | FXFA_XFDF | FXFA_FORM;
  }
  if (!m_pDocument->GetPDFDoc()) {
    fileStream.Flush();
    return FALSE;
  }

  CPDF_Dictionary* pRoot = m_pDocument->GetPDFDoc()->GetRoot();
  if (!pRoot) {
    fileStream.Flush();
    return FALSE;
  }

  CPDF_Dictionary* pAcroForm = pRoot->GetDictFor("AcroForm");
  if (!pAcroForm) {
    fileStream.Flush();
    return FALSE;
  }

  CPDF_Array* pArray = ToArray(pAcroForm->GetObjectFor("XFA"));
  if (!pArray) {
    fileStream.Flush();
    return FALSE;
  }

  int size = pArray->GetCount();
  for (int i = 1; i < size; i += 2) {
    CPDF_Object* pPDFObj = pArray->GetObjectAt(i);
    CPDF_Object* pPrePDFObj = pArray->GetObjectAt(i - 1);
    if (!pPrePDFObj->IsString())
      continue;
    if (!pPDFObj->IsReference())
      continue;

    CPDF_Object* pDirectObj = pPDFObj->GetDirect();
    if (!pDirectObj->IsStream())
      continue;
    if (pPrePDFObj->GetString() == "config" && !(flag & FXFA_CONFIG))
      continue;
    if (pPrePDFObj->GetString() == "template" && !(flag & FXFA_TEMPLATE))
      continue;
    if (pPrePDFObj->GetString() == "localeSet" && !(flag & FXFA_LOCALESET))
      continue;
    if (pPrePDFObj->GetString() == "datasets" && !(flag & FXFA_DATASETS))
      continue;
    if (pPrePDFObj->GetString() == "xmpmeta" && !(flag & FXFA_XMPMETA))
      continue;
    if (pPrePDFObj->GetString() == "xfdf" && !(flag & FXFA_XFDF))
      continue;
    if (pPrePDFObj->GetString() == "form" && !(flag & FXFA_FORM))
      continue;
    if (pPrePDFObj->GetString() == "form") {
      m_pDocument->GetXFADoc()->SavePackage(XFA_HASHCODE_Form, &fileStream,
                                            nullptr);
    } else if (pPrePDFObj->GetString() == "datasets") {
      m_pDocument->GetXFADoc()->SavePackage(XFA_HASHCODE_Datasets, &fileStream,
                                            nullptr);
    } else {
      // PDF,creator.
    }
  }
  return TRUE;
}

void CPDFXFA_DocEnvironment::ToXFAContentFlags(CFX_WideString csSrcContent,
                                               FPDF_DWORD& flag) {
  if (csSrcContent.Find(L" config ", 0) != -1)
    flag |= FXFA_CONFIG;
  if (csSrcContent.Find(L" template ", 0) != -1)
    flag |= FXFA_TEMPLATE;
  if (csSrcContent.Find(L" localeSet ", 0) != -1)
    flag |= FXFA_LOCALESET;
  if (csSrcContent.Find(L" datasets ", 0) != -1)
    flag |= FXFA_DATASETS;
  if (csSrcContent.Find(L" xmpmeta ", 0) != -1)
    flag |= FXFA_XMPMETA;
  if (csSrcContent.Find(L" xfdf ", 0) != -1)
    flag |= FXFA_XFDF;
  if (csSrcContent.Find(L" form ", 0) != -1)
    flag |= FXFA_FORM;
  if (flag == 0) {
    flag = FXFA_CONFIG | FXFA_TEMPLATE | FXFA_LOCALESET | FXFA_DATASETS |
           FXFA_XMPMETA | FXFA_XFDF | FXFA_FORM;
  }
}

FX_BOOL CPDFXFA_DocEnvironment::MailToInfo(CFX_WideString& csURL,
                                           CFX_WideString& csToAddress,
                                           CFX_WideString& csCCAddress,
                                           CFX_WideString& csBCCAddress,
                                           CFX_WideString& csSubject,
                                           CFX_WideString& csMsg) {
  CFX_WideString srcURL = csURL;
  srcURL.TrimLeft();
  if (srcURL.Left(7).CompareNoCase(L"mailto:") != 0)
    return FALSE;

  int pos = srcURL.Find(L'?', 0);
  CFX_WideString tmp;
  if (pos == -1) {
    pos = srcURL.Find(L'@', 0);
    if (pos == -1)
      return FALSE;

    tmp = srcURL.Right(csURL.GetLength() - 7);
  } else {
    tmp = srcURL.Left(pos);
    tmp = tmp.Right(tmp.GetLength() - 7);
  }
  tmp.TrimLeft();
  tmp.TrimRight();

  csToAddress = tmp;

  srcURL = srcURL.Right(srcURL.GetLength() - (pos + 1));
  while (!srcURL.IsEmpty()) {
    srcURL.TrimLeft();
    srcURL.TrimRight();
    pos = srcURL.Find(L'&', 0);

    tmp = (pos == -1) ? srcURL : srcURL.Left(pos);
    tmp.TrimLeft();
    tmp.TrimRight();
    if (tmp.GetLength() >= 3 && tmp.Left(3).CompareNoCase(L"cc=") == 0) {
      tmp = tmp.Right(tmp.GetLength() - 3);
      if (!csCCAddress.IsEmpty())
        csCCAddress += L';';
      csCCAddress += tmp;
    } else if (tmp.GetLength() >= 4 &&
               tmp.Left(4).CompareNoCase(L"bcc=") == 0) {
      tmp = tmp.Right(tmp.GetLength() - 4);
      if (!csBCCAddress.IsEmpty())
        csBCCAddress += L';';
      csBCCAddress += tmp;
    } else if (tmp.GetLength() >= 8 &&
               tmp.Left(8).CompareNoCase(L"subject=") == 0) {
      tmp = tmp.Right(tmp.GetLength() - 8);
      csSubject += tmp;
    } else if (tmp.GetLength() >= 5 &&
               tmp.Left(5).CompareNoCase(L"body=") == 0) {
      tmp = tmp.Right(tmp.GetLength() - 5);
      csMsg += tmp;
    }
    srcURL = (pos == -1) ? L"" : srcURL.Right(csURL.GetLength() - (pos + 1));
  }
  csToAddress.Replace(L",", L";");
  csCCAddress.Replace(L",", L";");
  csBCCAddress.Replace(L",", L";");
  return TRUE;
}

FX_BOOL CPDFXFA_DocEnvironment::SubmitDataInternal(CXFA_FFDoc* hDoc,
                                                   CXFA_Submit submit) {
  CPDFSDK_Environment* pEnv = m_pDocument->GetSDKDoc()->GetEnv();
  if (!pEnv)
    return FALSE;

  CFX_WideStringC csURLC;
  submit.GetSubmitTarget(csURLC);
  CFX_WideString csURL(csURLC);
  if (csURL.IsEmpty()) {
    CFX_WideString ws;
    ws.FromLocal("Submit cancelled.");
    CFX_ByteString bs = ws.UTF16LE_Encode();
    int len = bs.GetLength();
    pEnv->Alert((FPDF_WIDESTRING)bs.GetBuffer(len), (FPDF_WIDESTRING)L"", 0, 4);
    bs.ReleaseBuffer(len);
    return FALSE;
  }

  FPDF_BOOL bRet = TRUE;
  FPDF_FILEHANDLER* pFileHandler = nullptr;
  int fileFlag = -1;
  switch (submit.GetSubmitFormat()) {
    case XFA_ATTRIBUTEENUM_Xdp: {
      CFX_WideStringC csContentC;
      submit.GetSubmitXDPContent(csContentC);
      CFX_WideString csContent;
      csContent = csContentC;
      csContent.TrimLeft();
      csContent.TrimRight();
      CFX_WideString space;
      space.FromLocal(" ");
      csContent = space + csContent + space;
      FPDF_DWORD flag = 0;
      if (submit.IsSubmitEmbedPDF())
        flag |= FXFA_PDF;

      ToXFAContentFlags(csContent, flag);
      pFileHandler = pEnv->OpenFile(FXFA_SAVEAS_XDP, nullptr, "wb");
      fileFlag = FXFA_SAVEAS_XDP;
      ExportSubmitFile(pFileHandler, FXFA_SAVEAS_XDP, 0, flag);
      break;
    }
    case XFA_ATTRIBUTEENUM_Xml:
      pFileHandler = pEnv->OpenFile(FXFA_SAVEAS_XML, nullptr, "wb");
      fileFlag = FXFA_SAVEAS_XML;
      ExportSubmitFile(pFileHandler, FXFA_SAVEAS_XML, 0, FXFA_XFA_ALL);
      break;
    case XFA_ATTRIBUTEENUM_Pdf:
      break;
    case XFA_ATTRIBUTEENUM_Urlencoded:
      pFileHandler = pEnv->OpenFile(FXFA_SAVEAS_XML, nullptr, "wb");
      fileFlag = FXFA_SAVEAS_XML;
      ExportSubmitFile(pFileHandler, FXFA_SAVEAS_XML, 0, FXFA_XFA_ALL);
      break;
    default:
      return false;
  }
  if (!pFileHandler)
    return FALSE;
  if (csURL.Left(7).CompareNoCase(L"mailto:") == 0) {
    CFX_WideString csToAddress;
    CFX_WideString csCCAddress;
    CFX_WideString csBCCAddress;
    CFX_WideString csSubject;
    CFX_WideString csMsg;
    bRet = MailToInfo(csURL, csToAddress, csCCAddress, csBCCAddress, csSubject,
                      csMsg);
    if (!bRet)
      return FALSE;

    CFX_ByteString bsTo = CFX_WideString(csToAddress).UTF16LE_Encode();
    CFX_ByteString bsCC = CFX_WideString(csCCAddress).UTF16LE_Encode();
    CFX_ByteString bsBcc = CFX_WideString(csBCCAddress).UTF16LE_Encode();
    CFX_ByteString bsSubject = CFX_WideString(csSubject).UTF16LE_Encode();
    CFX_ByteString bsMsg = CFX_WideString(csMsg).UTF16LE_Encode();
    FPDF_WIDESTRING pTo = (FPDF_WIDESTRING)bsTo.GetBuffer(bsTo.GetLength());
    FPDF_WIDESTRING pCC = (FPDF_WIDESTRING)bsCC.GetBuffer(bsCC.GetLength());
    FPDF_WIDESTRING pBcc = (FPDF_WIDESTRING)bsBcc.GetBuffer(bsBcc.GetLength());
    FPDF_WIDESTRING pSubject =
        (FPDF_WIDESTRING)bsSubject.GetBuffer(bsSubject.GetLength());
    FPDF_WIDESTRING pMsg = (FPDF_WIDESTRING)bsMsg.GetBuffer(bsMsg.GetLength());
    pEnv->EmailTo(pFileHandler, pTo, pSubject, pCC, pBcc, pMsg);
    bsTo.ReleaseBuffer();
    bsCC.ReleaseBuffer();
    bsBcc.ReleaseBuffer();
    bsSubject.ReleaseBuffer();
    bsMsg.ReleaseBuffer();
  } else {
    // HTTP or FTP
    CFX_WideString ws;
    CFX_ByteString bs = csURL.UTF16LE_Encode();
    int len = bs.GetLength();
    pEnv->UploadTo(pFileHandler, fileFlag, (FPDF_WIDESTRING)bs.GetBuffer(len));
    bs.ReleaseBuffer(len);
  }
  return bRet;
}

FX_BOOL CPDFXFA_DocEnvironment::SetGlobalProperty(
    CXFA_FFDoc* hDoc,
    const CFX_ByteStringC& szPropName,
    CFXJSE_Value* pValue) {
  if (hDoc != m_pDocument->GetXFADoc())
    return FALSE;

  if (m_pDocument->GetSDKDoc() &&
      m_pDocument->GetSDKDoc()->GetEnv()->GetJSRuntime())
    return m_pDocument->GetSDKDoc()->GetEnv()->GetJSRuntime()->SetValueByName(
        szPropName, pValue);
  return FALSE;
}

FX_BOOL CPDFXFA_DocEnvironment::GetGlobalProperty(
    CXFA_FFDoc* hDoc,
    const CFX_ByteStringC& szPropName,
    CFXJSE_Value* pValue) {
  if (hDoc != m_pDocument->GetXFADoc())
    return FALSE;
  if (!m_pDocument->GetSDKDoc() ||
      !m_pDocument->GetSDKDoc()->GetEnv()->GetJSRuntime())
    return FALSE;

  if (!m_pJSContext) {
    m_pDocument->GetSDKDoc()->GetEnv()->GetJSRuntime()->SetReaderDocument(
        m_pDocument->GetSDKDoc());
    m_pJSContext =
        m_pDocument->GetSDKDoc()->GetEnv()->GetJSRuntime()->NewContext();
  }

  return m_pDocument->GetSDKDoc()->GetEnv()->GetJSRuntime()->GetValueByName(
      szPropName, pValue);
}
