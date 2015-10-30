// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../public/fpdf_formfill.h"
#include "../../public/fpdfview.h"
#include "../../third_party/base/nonstd_unique_ptr.h"
#include "../include/fsdk_define.h"
#include "../include/fsdk_mgr.h"

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
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
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
  if (!pPage)
    return -1;
  CPDF_InterForm interform(pPage->m_pDocument, FALSE);
  CPDF_FormControl* pFormCtrl = interform.GetControlAtPoint(
      pPage, (FX_FLOAT)page_x, (FX_FLOAT)page_y, nullptr);
  if (!pFormCtrl)
    return -1;

  CPDF_FormField* pFormField = pFormCtrl->GetField();
  return pFormField ? pFormField->GetFieldType() : -1;
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
  if (!formInfo || formInfo->version != 1)
    return nullptr;

  CPDF_Document* pDocument = CPDFDocumentFromFPDFDocument(document);
  if (!pDocument)
    return nullptr;
  CPDFDoc_Environment* pEnv = new CPDFDoc_Environment(pDocument, formInfo);
  pEnv->SetSDKDocument(new CPDFSDK_Document(pDocument, pEnv));
  return pEnv;
}

DLLEXPORT void STDCALL
FPDFDOC_ExitFormFillEnvironment(FPDF_FORMHANDLE hHandle) {
  if (!hHandle)
    return;

  CPDFDoc_Environment* pEnv = (CPDFDoc_Environment*)hHandle;
  if (CPDFSDK_Document* pSDKDoc = pEnv->GetSDKDocument()) {
    pEnv->SetSDKDocument(NULL);
    delete pSDKDoc;
  }
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

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

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

  CFX_AffineMatrix matrix;
  pPage->GetDisplayMatrix(matrix, start_x, start_y, size_x, size_y, rotate);

  FX_RECT clip;
  clip.left = start_x;
  clip.right = start_x + size_x;
  clip.top = start_y;
  clip.bottom = start_y + size_y;

#ifdef _SKIA_SUPPORT_
  nonstd::unique_ptr<CFX_SkiaDevice> pDevice(new CFX_SkiaDevice);
#else
  nonstd::unique_ptr<CFX_FxgeDevice> pDevice(new CFX_FxgeDevice);
#endif
  pDevice->Attach((CFX_DIBitmap*)bitmap);
  pDevice->SaveState();
  pDevice->SetClip_Rect(&clip);

  if (CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, pPage))
    pPageView->PageView_OnDraw(pDevice.get(), &matrix, &options);

  pDevice->RestoreState();
  delete options.m_pOCContext;
}

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

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

  CPDFSDK_PageView* pPageView = pSDKDoc->GetPageView(pPage, FALSE);
  if (pPageView) {
    pPageView->SetValid(FALSE);
    // ReMovePageView() takes care of the delete for us.
    pSDKDoc->ReMovePageView(pPage);
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

  CPDF_Document* pDoc = pSDKDoc->GetDocument();
  CPDF_Dictionary* pDic = pDoc->GetRoot();
  if (!pDic)
    return;

  CPDF_AAction aa = pDic->GetDict(FX_BSTRC("AA"));
  if (aa.ActionExist((CPDF_AAction::AActionType)aaType)) {
    CPDF_Action action = aa.GetAction((CPDF_AAction::AActionType)aaType);
    CPDFSDK_ActionHandler* pActionHandler =
        ((CPDFDoc_Environment*)hHandle)->GetActionHander();
    ASSERT(pActionHandler != NULL);
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
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;
  if (pSDKDoc->GetPageView(pPage, FALSE)) {
    CPDFDoc_Environment* pEnv = pSDKDoc->GetEnv();
    CPDFSDK_ActionHandler* pActionHandler = pEnv->GetActionHander();
    CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
    CPDF_AAction aa = pPageDict->GetDict(FX_BSTRC("AA"));
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
