// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_formfill.h"

#include <memory>

#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/fsdk_mgr.h"
#include "public/fpdfview.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_app.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_doc.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_page.h"
#endif  // PDF_ENABLE_XFA

namespace {

CPDFSDK_Document* FormHandleToSDKDoc(FPDF_FORMHANDLE hHandle) {
  CPDFDoc_Environment* pEnv = (CPDFDoc_Environment*)hHandle;
  return pEnv ? pEnv->GetSDKDocument() : nullptr;
}

CPDFSDK_InterForm* FormHandleToInterForm(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_Document* pSDKDoc = FormHandleToSDKDoc(hHandle);
  return pSDKDoc ? pSDKDoc->GetInterForm() : nullptr;
}

CPDFSDK_PageView* FormHandleToPageView(FPDF_FORMHANDLE hHandle,
                                       FPDF_PAGE page) {
  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  if (!pPage)
    return nullptr;

  CPDFSDK_Document* pSDKDoc = FormHandleToSDKDoc(hHandle);
  return pSDKDoc ? pSDKDoc->GetPageView(pPage, TRUE) : nullptr;
}

}  // namespace

DLLEXPORT int STDCALL FPDFPage_HasFormFieldAtPoint(FPDF_FORMHANDLE hHandle,
                                                   FPDF_PAGE page,
                                                   double page_x,
                                                   double page_y) {
  if (!hHandle)
    return -1;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
#ifdef PDF_ENABLE_XFA
  if (pPage) {
    CPDF_InterForm interform(pPage->m_pDocument, FALSE);
    CPDF_FormControl* pFormCtrl = interform.GetControlAtPoint(
        pPage, (FX_FLOAT)page_x, (FX_FLOAT)page_y, nullptr);
    if (!pFormCtrl)
      return -1;

    CPDF_FormField* pFormField = pFormCtrl->GetField();
    if (!pFormField)
      return -1;

    int nType = pFormField->GetFieldType();
    return nType;
  }

  IXFA_PageView* pPageView = ((CPDFXFA_Page*)page)->GetXFAPageView();
  if (pPageView) {
    IXFA_WidgetHandler* pWidgetHandler = NULL;
    IXFA_DocView* pDocView = pPageView->GetDocView();
    if (!pDocView)
      return -1;

    pWidgetHandler = pDocView->GetWidgetHandler();
    if (!pWidgetHandler)
      return -1;

    IXFA_Widget* pXFAAnnot = NULL;
    IXFA_WidgetIterator* pWidgetIterator = pPageView->CreateWidgetIterator(
        XFA_TRAVERSEWAY_Form,
        XFA_WIDGETFILTER_Viewable | XFA_WIDGETFILTER_AllType);
    if (!pWidgetIterator)
      return -1;
    pXFAAnnot = pWidgetIterator->MoveToNext();
    while (pXFAAnnot) {
      CFX_RectF rcBBox;
      pWidgetHandler->GetBBox(pXFAAnnot, rcBBox, 0);
      CFX_FloatRect rcWidget(rcBBox.left, rcBBox.top,
                             rcBBox.left + rcBBox.width,
                             rcBBox.top + rcBBox.height);
      rcWidget.left -= 1.0f;
      rcWidget.right += 1.0f;
      rcWidget.bottom -= 1.0f;
      rcWidget.top += 1.0f;

      if (rcWidget.Contains(static_cast<FX_FLOAT>(page_x),
                            static_cast<FX_FLOAT>(page_y))) {
        pWidgetIterator->Release();
        return FPDF_FORMFIELD_XFA;
      }
      pXFAAnnot = pWidgetIterator->MoveToNext();
    }

    pWidgetIterator->Release();
  }
  return -1;
#else   // PDF_ENABLE_XFA
  if (!pPage)
    return -1;
  CPDF_InterForm interform(pPage->m_pDocument, FALSE);
  CPDF_FormControl* pFormCtrl = interform.GetControlAtPoint(
      pPage, (FX_FLOAT)page_x, (FX_FLOAT)page_y, nullptr);
  if (!pFormCtrl)
    return -1;
  CPDF_FormField* pFormField = pFormCtrl->GetField();
  return pFormField ? pFormField->GetFieldType() : -1;
#endif  // PDF_ENABLE_XFA
}

DLLEXPORT int STDCALL FPDPage_HasFormFieldAtPoint(FPDF_FORMHANDLE hHandle,
                                                  FPDF_PAGE page,
                                                  double page_x,
                                                  double page_y) {
  return FPDFPage_HasFormFieldAtPoint(hHandle, page, page_x, page_y);
}

DLLEXPORT int STDCALL FPDFPage_FormFieldZOrderAtPoint(FPDF_FORMHANDLE hHandle,
                                                      FPDF_PAGE page,
                                                      double page_x,
                                                      double page_y) {
  if (!hHandle)
    return -1;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return -1;
  CPDF_InterForm interform(pPage->m_pDocument, FALSE);
  int z_order = -1;
  (void)interform.GetControlAtPoint(pPage, (FX_FLOAT)page_x, (FX_FLOAT)page_y,
                                    &z_order);
  return z_order;
}

DLLEXPORT FPDF_FORMHANDLE STDCALL
FPDFDOC_InitFormFillEnvironment(FPDF_DOCUMENT document,
                                FPDF_FORMFILLINFO* formInfo) {
#ifdef PDF_ENABLE_XFA
  const int kRequiredVersion = 2;
#else   // PDF_ENABLE_XFA
  const int kRequiredVersion = 1;
#endif  // PDF_ENABLE_XFA
  if (!formInfo || formInfo->version != kRequiredVersion)
    return nullptr;

  UnderlyingDocumentType* pDocument = UnderlyingFromFPDFDocument(document);
  if (!pDocument)
    return nullptr;

  CPDFDoc_Environment* pEnv = new CPDFDoc_Environment(pDocument, formInfo);
#ifdef PDF_ENABLE_XFA
  pEnv->SetSDKDocument(pDocument->GetSDKDocument(pEnv));
  CPDFXFA_App* pApp = CPDFXFA_App::GetInstance();
  pApp->AddFormFillEnv(pEnv);
#else  // PDF_ENABLE_XFA
  pEnv->SetSDKDocument(new CPDFSDK_Document(pDocument, pEnv));
#endif  // PDF_ENABLE_XFA
  return pEnv;
}

DLLEXPORT void STDCALL
FPDFDOC_ExitFormFillEnvironment(FPDF_FORMHANDLE hHandle) {
  if (!hHandle)
    return;
  CPDFDoc_Environment* pEnv = (CPDFDoc_Environment*)hHandle;
#ifdef PDF_ENABLE_XFA
  CPDFXFA_App* pApp = CPDFXFA_App::GetInstance();
  pApp->RemoveFormFillEnv(pEnv);
#else   // PDF_ENABLE_XFA
  if (CPDFSDK_Document* pSDKDoc = pEnv->GetSDKDocument()) {
    pEnv->SetSDKDocument(NULL);
    delete pSDKDoc;
  }
#endif  // PDF_ENABLE_XFA
  delete pEnv;
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnMouseMove(FPDF_FORMHANDLE hHandle,
                                             FPDF_PAGE page,
                                             int modifier,
                                             double page_x,
                                             double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
  return pPageView->OnMouseMove(pt, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnLButtonDown(FPDF_FORMHANDLE hHandle,
                                               FPDF_PAGE page,
                                               int modifier,
                                               double page_x,
                                               double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
  return pPageView->OnLButtonDown(pt, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnLButtonUp(FPDF_FORMHANDLE hHandle,
                                             FPDF_PAGE page,
                                             int modifier,
                                             double page_x,
                                             double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
  return pPageView->OnLButtonUp(pt, modifier);
}

#ifdef PDF_ENABLE_XFA
DLLEXPORT FPDF_BOOL STDCALL FORM_OnRButtonDown(FPDF_FORMHANDLE hHandle,
                                               FPDF_PAGE page,
                                               int modifier,
                                               double page_x,
                                               double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
  return pPageView->OnRButtonDown(pt, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnRButtonUp(FPDF_FORMHANDLE hHandle,
                                             FPDF_PAGE page,
                                             int modifier,
                                             double page_x,
                                             double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
  return pPageView->OnRButtonUp(pt, modifier);
}
#endif  // PDF_ENABLE_XFA

DLLEXPORT FPDF_BOOL STDCALL FORM_OnKeyDown(FPDF_FORMHANDLE hHandle,
                                           FPDF_PAGE page,
                                           int nKeyCode,
                                           int modifier) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  return pPageView->OnKeyDown(nKeyCode, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnKeyUp(FPDF_FORMHANDLE hHandle,
                                         FPDF_PAGE page,
                                         int nKeyCode,
                                         int modifier) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  return pPageView->OnKeyUp(nKeyCode, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnChar(FPDF_FORMHANDLE hHandle,
                                        FPDF_PAGE page,
                                        int nChar,
                                        int modifier) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return FALSE;

  return pPageView->OnChar(nChar, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_ForceToKillFocus(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_Document* pSDKDoc = FormHandleToSDKDoc(hHandle);
  if (!pSDKDoc)
    return FALSE;

  return pSDKDoc->KillFocusAnnot(0);
}

DLLEXPORT void STDCALL FPDF_FFLDraw(FPDF_FORMHANDLE hHandle,
                                    FPDF_BITMAP bitmap,
                                    FPDF_PAGE page,
                                    int start_x,
                                    int start_y,
                                    int size_x,
                                    int size_y,
                                    int rotate,
                                    int flags) {
  if (!hHandle)
    return;

  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  if (!pPage)
    return;

#ifndef PDF_ENABLE_XFA
  CPDF_RenderOptions options;
  if (flags & FPDF_LCD_TEXT)
    options.m_Flags |= RENDER_CLEARTYPE;
  else
    options.m_Flags &= ~RENDER_CLEARTYPE;
  // Grayscale output
  if (flags & FPDF_GRAYSCALE) {
    options.m_ColorMode = RENDER_COLOR_GRAY;
    options.m_ForeColor = 0;
    options.m_BackColor = 0xffffff;
  }
  options.m_AddFlags = flags >> 8;
  options.m_pOCContext = new CPDF_OCContext(pPage->m_pDocument);
#else   // PDF_ENABLE_XFA
  CPDFXFA_Document* pDocument = pPage->GetDocument();
  if (!pDocument)
    return;
  CPDF_Document* pPDFDoc = pDocument->GetPDFDoc();
  if (!pPDFDoc)
    return;
  CPDFDoc_Environment* pEnv = (CPDFDoc_Environment*)hHandle;
  CPDFSDK_Document* pFXDoc = pEnv->GetSDKDocument();
  if (!pFXDoc)
    return;
#endif  // PDF_ENABLE_XFA

  CFX_Matrix matrix;
  pPage->GetDisplayMatrix(matrix, start_x, start_y, size_x, size_y, rotate);

  FX_RECT clip;
  clip.left = start_x;
  clip.right = start_x + size_x;
  clip.top = start_y;
  clip.bottom = start_y + size_y;

#ifdef _SKIA_SUPPORT_
  std::unique_ptr<CFX_SkiaDevice> pDevice(new CFX_SkiaDevice);
#else
  std::unique_ptr<CFX_FxgeDevice> pDevice(new CFX_FxgeDevice);
#endif
  pDevice->Attach((CFX_DIBitmap*)bitmap);
  pDevice->SaveState();
  pDevice->SetClip_Rect(&clip);

#ifndef PDF_ENABLE_XFA
  if (CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, pPage))
    pPageView->PageView_OnDraw(pDevice.get(), &matrix, &options);
#else   // PDF_ENABLE_XFA
  CPDF_RenderOptions options;
  if (flags & FPDF_LCD_TEXT)
    options.m_Flags |= RENDER_CLEARTYPE;
  else
    options.m_Flags &= ~RENDER_CLEARTYPE;

  // Grayscale output
  if (flags & FPDF_GRAYSCALE) {
    options.m_ColorMode = RENDER_COLOR_GRAY;
    options.m_ForeColor = 0;
    options.m_BackColor = 0xffffff;
  }
  options.m_AddFlags = flags >> 8;
  options.m_pOCContext = new CPDF_OCContext(pPDFDoc);

  if (CPDFSDK_PageView* pPageView = pFXDoc->GetPageView(pPage))
    pPageView->PageView_OnDraw(pDevice.get(), &matrix, &options, clip);
#endif  // PDF_ENABLE_XFA

  pDevice->RestoreState();
  delete options.m_pOCContext;
#ifdef PDF_ENABLE_XFA
  options.m_pOCContext = NULL;
#endif  // PDF_ENABLE_XFA
}

#ifdef PDF_ENABLE_XFA
DLLEXPORT void STDCALL FPDF_Widget_Undo(FPDF_DOCUMENT document,
                                        FPDF_WIDGET hWidget) {
  if (NULL == hWidget || NULL == document)
    return;

  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  pXFAMenuHander->Undo((IXFA_Widget*)hWidget);
}
DLLEXPORT void STDCALL FPDF_Widget_Redo(FPDF_DOCUMENT document,
                                        FPDF_WIDGET hWidget) {
  if (NULL == hWidget || NULL == document)
    return;

  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  pXFAMenuHander->Redo((IXFA_Widget*)hWidget);
}

DLLEXPORT void STDCALL FPDF_Widget_SelectAll(FPDF_DOCUMENT document,
                                             FPDF_WIDGET hWidget) {
  if (NULL == hWidget || NULL == document)
    return;

  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  pXFAMenuHander->SelectAll((IXFA_Widget*)hWidget);
}
DLLEXPORT void STDCALL FPDF_Widget_Copy(FPDF_DOCUMENT document,
                                        FPDF_WIDGET hWidget,
                                        FPDF_WIDESTRING wsText,
                                        FPDF_DWORD* size) {
  if (NULL == hWidget || NULL == document)
    return;

  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  CFX_WideString wsCpText;
  pXFAMenuHander->Copy((IXFA_Widget*)hWidget, wsCpText);

  CFX_ByteString bsCpText = wsCpText.UTF16LE_Encode();
  int len = bsCpText.GetLength() / sizeof(unsigned short);
  if (wsText == NULL) {
    *size = len;
    return;
  }

  int real_size = len < *size ? len : *size;
  if (real_size > 0) {
    FXSYS_memcpy((void*)wsText,
                 bsCpText.GetBuffer(real_size * sizeof(unsigned short)),
                 real_size * sizeof(unsigned short));
    bsCpText.ReleaseBuffer(real_size * sizeof(unsigned short));
  }
  *size = real_size;
}
DLLEXPORT void STDCALL FPDF_Widget_Cut(FPDF_DOCUMENT document,
                                       FPDF_WIDGET hWidget,
                                       FPDF_WIDESTRING wsText,
                                       FPDF_DWORD* size) {
  if (NULL == hWidget || NULL == document)
    return;
  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  CFX_WideString wsCpText;
  pXFAMenuHander->Cut((IXFA_Widget*)hWidget, wsCpText);

  CFX_ByteString bsCpText = wsCpText.UTF16LE_Encode();
  int len = bsCpText.GetLength() / sizeof(unsigned short);
  if (wsText == NULL) {
    *size = len;
    return;
  }

  int real_size = len < *size ? len : *size;
  if (real_size > 0) {
    FXSYS_memcpy((void*)wsText,
                 bsCpText.GetBuffer(real_size * sizeof(unsigned short)),
                 real_size * sizeof(unsigned short));
    bsCpText.ReleaseBuffer(real_size * sizeof(unsigned short));
  }
  *size = real_size;
}
DLLEXPORT void STDCALL FPDF_Widget_Paste(FPDF_DOCUMENT document,
                                         FPDF_WIDGET hWidget,
                                         FPDF_WIDESTRING wsText,
                                         FPDF_DWORD size) {
  if (NULL == hWidget || NULL == document)
    return;

  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  CFX_WideString wstr = CFX_WideString::FromUTF16LE(wsText, size);
  pXFAMenuHander->Paste((IXFA_Widget*)hWidget, wstr);
}
DLLEXPORT void STDCALL
FPDF_Widget_ReplaceSpellCheckWord(FPDF_DOCUMENT document,
                                  FPDF_WIDGET hWidget,
                                  float x,
                                  float y,
                                  FPDF_BYTESTRING bsText) {
  if (NULL == hWidget || NULL == document)
    return;

  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  CFX_PointF ptPopup;
  ptPopup.x = x;
  ptPopup.y = y;
  CFX_ByteStringC bs(bsText);
  pXFAMenuHander->ReplaceSpellCheckWord((IXFA_Widget*)hWidget, ptPopup, bs);
}
DLLEXPORT void STDCALL
FPDF_Widget_GetSpellCheckWords(FPDF_DOCUMENT document,
                               FPDF_WIDGET hWidget,
                               float x,
                               float y,
                               FPDF_STRINGHANDLE* stringHandle) {
  if (NULL == hWidget || NULL == document)
    return;

  CPDFXFA_Document* pDocument = (CPDFXFA_Document*)document;
  if (pDocument->GetDocType() != XFA_DOCTYPE_Dynamic &&
      pDocument->GetDocType() != XFA_DOCTYPE_Static)
    return;

  IXFA_MenuHandler* pXFAMenuHander =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetMenuHandler();
  if (pXFAMenuHander == NULL)
    return;

  CFX_ByteStringArray* sSuggestWords = new CFX_ByteStringArray;
  CFX_PointF ptPopup;
  ptPopup.x = x;
  ptPopup.y = y;
  pXFAMenuHander->GetSuggestWords((IXFA_Widget*)hWidget, ptPopup,
                                  *sSuggestWords);
  *stringHandle = (FPDF_STRINGHANDLE)sSuggestWords;
}
DLLEXPORT int STDCALL FPDF_StringHandleCounts(FPDF_STRINGHANDLE stringHandle) {
  if (stringHandle == NULL)
    return -1;
  CFX_ByteStringArray* sSuggestWords = (CFX_ByteStringArray*)stringHandle;
  return sSuggestWords->GetSize();
}
DLLEXPORT FPDF_BOOL STDCALL
FPDF_StringHandleGetStringByIndex(FPDF_STRINGHANDLE stringHandle,
                                  int index,
                                  FPDF_BYTESTRING bsText,
                                  FPDF_DWORD* size) {
  if (stringHandle == NULL || size == NULL)
    return FALSE;
  int count = FPDF_StringHandleCounts(stringHandle);
  if (index < 0 || index >= count)
    return FALSE;

  CFX_ByteStringArray sSuggestWords = *(CFX_ByteStringArray*)stringHandle;
  int len = sSuggestWords[index].GetLength();

  if (bsText == NULL) {
    *size = len;
    return TRUE;
  }

  int real_size = len < *size ? len : *size;
  if (real_size > 0)
    FXSYS_memcpy((void*)bsText, (const FX_CHAR*)(sSuggestWords[index]),
                 real_size);
  *size = real_size;

  return TRUE;
}
DLLEXPORT void STDCALL
FPDF_StringHandleRelease(FPDF_STRINGHANDLE stringHandle) {
  if (stringHandle == NULL)
    return;
  CFX_ByteStringArray* sSuggestWords = (CFX_ByteStringArray*)stringHandle;
  delete sSuggestWords;
}

DLLEXPORT FPDF_BOOL STDCALL
FPDF_StringHandleAddString(FPDF_STRINGHANDLE stringHandle,
                           FPDF_BYTESTRING bsText,
                           FPDF_DWORD size) {
  if (stringHandle == NULL || bsText == NULL || size <= 0)
    return FALSE;

  CFX_ByteStringArray* stringArr = (CFX_ByteStringArray*)stringHandle;
  CFX_ByteString bsStr(bsText, size);

  stringArr->Add(bsStr);
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

DLLEXPORT void STDCALL FPDF_SetFormFieldHighlightColor(FPDF_FORMHANDLE hHandle,
                                                       int fieldType,
                                                       unsigned long color) {
  if (CPDFSDK_InterForm* pInterForm = FormHandleToInterForm(hHandle))
    pInterForm->SetHighlightColor(color, fieldType);
}

DLLEXPORT void STDCALL FPDF_SetFormFieldHighlightAlpha(FPDF_FORMHANDLE hHandle,
                                                       unsigned char alpha) {
  if (CPDFSDK_InterForm* pInterForm = FormHandleToInterForm(hHandle))
    pInterForm->SetHighlightAlpha(alpha);
}

DLLEXPORT void STDCALL FPDF_RemoveFormFieldHighlight(FPDF_FORMHANDLE hHandle) {
  if (CPDFSDK_InterForm* pInterForm = FormHandleToInterForm(hHandle))
    pInterForm->RemoveAllHighLight();
}

DLLEXPORT void STDCALL FORM_OnAfterLoadPage(FPDF_PAGE page,
                                            FPDF_FORMHANDLE hHandle) {
  if (CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page))
    pPageView->SetValid(TRUE);
}

DLLEXPORT void STDCALL FORM_OnBeforeClosePage(FPDF_PAGE page,
                                              FPDF_FORMHANDLE hHandle) {
  if (!hHandle)
    return;

  CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetSDKDocument();
  if (!pSDKDoc)
    return;

  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  if (!pPage)
    return;

  CPDFSDK_PageView* pPageView = pSDKDoc->GetPageView(pPage, FALSE);
  if (pPageView) {
    pPageView->SetValid(FALSE);
    // RemovePageView() takes care of the delete for us.
    pSDKDoc->RemovePageView(pPage);
  }
}

DLLEXPORT void STDCALL FORM_DoDocumentJSAction(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_Document* pSDKDoc = FormHandleToSDKDoc(hHandle);
  if (pSDKDoc && ((CPDFDoc_Environment*)hHandle)->IsJSInitiated())
    pSDKDoc->ProcJavascriptFun();
}

DLLEXPORT void STDCALL FORM_DoDocumentOpenAction(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_Document* pSDKDoc = FormHandleToSDKDoc(hHandle);
  if (pSDKDoc && ((CPDFDoc_Environment*)hHandle)->IsJSInitiated())
    pSDKDoc->ProcOpenAction();
}

DLLEXPORT void STDCALL FORM_DoDocumentAAction(FPDF_FORMHANDLE hHandle,
                                              int aaType) {
  CPDFSDK_Document* pSDKDoc = FormHandleToSDKDoc(hHandle);
  if (!pSDKDoc)
    return;

  CPDF_Document* pDoc = pSDKDoc->GetPDFDocument();
  CPDF_Dictionary* pDic = pDoc->GetRoot();
  if (!pDic)
    return;

  CPDF_AAction aa = pDic->GetDict("AA");
  if (aa.ActionExist((CPDF_AAction::AActionType)aaType)) {
    CPDF_Action action = aa.GetAction((CPDF_AAction::AActionType)aaType);
    CPDFSDK_ActionHandler* pActionHandler =
        ((CPDFDoc_Environment*)hHandle)->GetActionHander();
    pActionHandler->DoAction_Document(action, (CPDF_AAction::AActionType)aaType,
                                      pSDKDoc);
  }
}

DLLEXPORT void STDCALL FORM_DoPageAAction(FPDF_PAGE page,
                                          FPDF_FORMHANDLE hHandle,
                                          int aaType) {
  if (!hHandle)
    return;
  CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetSDKDocument();
  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  CPDF_Page* pPDFPage = CPDFPageFromFPDFPage(page);
  if (!pPDFPage)
    return;
  if (pSDKDoc->GetPageView(pPage, FALSE)) {
    CPDFDoc_Environment* pEnv = pSDKDoc->GetEnv();
    CPDFSDK_ActionHandler* pActionHandler = pEnv->GetActionHander();
    CPDF_Dictionary* pPageDict = pPDFPage->m_pFormDict;
    CPDF_AAction aa = pPageDict->GetDict("AA");
    if (FPDFPAGE_AACTION_OPEN == aaType) {
      if (aa.ActionExist(CPDF_AAction::OpenPage)) {
        CPDF_Action action = aa.GetAction(CPDF_AAction::OpenPage);
        pActionHandler->DoAction_Page(action, CPDF_AAction::OpenPage, pSDKDoc);
      }
    } else {
      if (aa.ActionExist(CPDF_AAction::ClosePage)) {
        CPDF_Action action = aa.GetAction(CPDF_AAction::ClosePage);
        pActionHandler->DoAction_Page(action, CPDF_AAction::ClosePage, pSDKDoc);
      }
    }
  }
}
