// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fsdk_mgr.h"
#include "../include/formfiller/FFL_FormFiller.h"
#include "../include/fsdk_annothandler.h"

CPDFSDK_AnnotHandlerMgr::CPDFSDK_AnnotHandlerMgr(CPDFDoc_Environment* pApp) {
  m_pApp = pApp;

  CPDFSDK_AnnotHandler* pHandler = new CPDFSDK_AnnotHandler(m_pApp);
  pHandler->SetFormFiller(m_pApp->GetIFormFiller());
  RegisterAnnotHandler(pHandler);
}

CPDFSDK_AnnotHandlerMgr::~CPDFSDK_AnnotHandlerMgr() {
  for (int i = 0; i < m_Handlers.GetSize(); i++) {
    CPDFSDK_AnnotHandler* pHandler = m_Handlers.GetAt(i);
    delete pHandler;
  }
  m_Handlers.RemoveAll();
  m_mapType2Handler.clear();
}

void CPDFSDK_AnnotHandlerMgr::RegisterAnnotHandler(
    CPDFSDK_AnnotHandler* pAnnotHandler) {
  ASSERT(!GetAnnotHandler(pAnnotHandler->GetType()));

  m_Handlers.Add(pAnnotHandler);
  m_mapType2Handler[pAnnotHandler->GetType()] = pAnnotHandler;
}

void CPDFSDK_AnnotHandlerMgr::UnRegisterAnnotHandler(
    CPDFSDK_AnnotHandler* pAnnotHandler) {
  m_mapType2Handler.erase(pAnnotHandler->GetType());
  for (int i = 0, sz = m_Handlers.GetSize(); i < sz; i++) {
    if (m_Handlers.GetAt(i) == pAnnotHandler) {
      m_Handlers.RemoveAt(i);
      break;
    }
  }
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::NewAnnot(CPDF_Annot* pAnnot,
                                                 CPDFSDK_PageView* pPageView) {
  ASSERT(pAnnot != NULL);
  ASSERT(pPageView != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler =
          GetAnnotHandler(pAnnot->GetSubType())) {
    return pAnnotHandler->NewAnnot(pAnnot, pPageView);
  }

  return new CPDFSDK_Annot(pAnnot, pPageView);
}

void CPDFSDK_AnnotHandlerMgr::ReleaseAnnot(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot != NULL);

  pAnnot->GetPDFPage();

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnRelease(pAnnot);
    pAnnotHandler->ReleaseAnnot(pAnnot);
  } else {
    delete (CPDFSDK_Annot*)pAnnot;
  }
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnCreate(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot != NULL);

  CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();

  CPDFSDK_DateTime curTime;
  pPDFAnnot->GetAnnotDict()->SetAtString("M", curTime.ToPDFDateTimeString());
  pPDFAnnot->GetAnnotDict()->SetAtNumber("F", 0);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnCreate(pAnnot);
  }
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnLoad(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnLoad(pAnnot);
  }
}

CPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    CPDFSDK_Annot* pAnnot) const {
  ASSERT(pAnnot != NULL);

  CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();
  ASSERT(pPDFAnnot != NULL);

  return GetAnnotHandler(pPDFAnnot->GetSubType());
}

CPDFSDK_AnnotHandler* CPDFSDK_AnnotHandlerMgr::GetAnnotHandler(
    const CFX_ByteString& sType) const {
  auto it = m_mapType2Handler.find(sType);
  return it != m_mapType2Handler.end() ? it->second : nullptr;
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnDraw(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           CFX_RenderDevice* pDevice,
                                           CPDF_Matrix* pUser2Device,
                                           FX_DWORD dwFlags) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnDraw(pPageView, pAnnot, pDevice, pUser2Device, dwFlags);
  } else {
    pAnnot->DrawAppearance(pDevice, pUser2Device, CPDF_Annot::Normal, NULL);
  }
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDown(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    FX_DWORD nFlags,
    const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnLButtonDown(pPageView, pAnnot, nFlags, point);
  }
  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonUp(CPDFSDK_PageView* pPageView,
                                                   CPDFSDK_Annot* pAnnot,
                                                   FX_DWORD nFlags,
                                                   const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnLButtonUp(pPageView, pAnnot, nFlags, point);
  }
  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnLButtonDblClk(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    FX_DWORD nFlags,
    const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnLButtonDblClk(pPageView, pAnnot, nFlags, point);
  }
  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnMouseMove(CPDFSDK_PageView* pPageView,
                                                   CPDFSDK_Annot* pAnnot,
                                                   FX_DWORD nFlags,
                                                   const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnMouseMove(pPageView, pAnnot, nFlags, point);
  }
  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnMouseWheel(CPDFSDK_PageView* pPageView,
                                                    CPDFSDK_Annot* pAnnot,
                                                    FX_DWORD nFlags,
                                                    short zDelta,
                                                    const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnMouseWheel(pPageView, pAnnot, nFlags, zDelta,
                                       point);
  }
  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonDown(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot,
    FX_DWORD nFlags,
    const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnRButtonDown(pPageView, pAnnot, nFlags, point);
  }
  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnRButtonUp(CPDFSDK_PageView* pPageView,
                                                   CPDFSDK_Annot* pAnnot,
                                                   FX_DWORD nFlags,
                                                   const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnRButtonUp(pPageView, pAnnot, nFlags, point);
  }
  return FALSE;
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseEnter(CPDFSDK_PageView* pPageView,
                                                 CPDFSDK_Annot* pAnnot,
                                                 FX_DWORD nFlag) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnMouseEnter(pPageView, pAnnot, nFlag);
  }
  return;
}

void CPDFSDK_AnnotHandlerMgr::Annot_OnMouseExit(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot,
                                                FX_DWORD nFlag) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    pAnnotHandler->OnMouseExit(pPageView, pAnnot, nFlag);
  }
  return;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnChar(CPDFSDK_Annot* pAnnot,
                                              FX_DWORD nChar,
                                              FX_DWORD nFlags) {
  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnChar(pAnnot, nChar, nFlags);
  }
  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKeyDown(CPDFSDK_Annot* pAnnot,
                                                 int nKeyCode,
                                                 int nFlag) {
  if (!m_pApp->FFI_IsCTRLKeyDown(nFlag) && !m_pApp->FFI_IsALTKeyDown(nFlag)) {
    CPDFSDK_PageView* pPage = pAnnot->GetPageView();
    CPDFSDK_Annot* pFocusAnnot = pPage->GetFocusAnnot();
    if (pFocusAnnot && (nKeyCode == FWL_VKEY_Tab)) {
      CPDFSDK_Annot* pNext =
          GetNextAnnot(pFocusAnnot, !m_pApp->FFI_IsSHIFTKeyDown(nFlag));

      if (pNext && pNext != pFocusAnnot) {
        CPDFSDK_Document* pDocument = pPage->GetSDKDocument();
        pDocument->SetFocusAnnot(pNext);
        return TRUE;
      }
    }
  }

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    return pAnnotHandler->OnKeyDown(pAnnot, nKeyCode, nFlag);
  }
  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKeyUp(CPDFSDK_Annot* pAnnot,
                                               int nKeyCode,
                                               int nFlag) {
  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnSetFocus(CPDFSDK_Annot* pAnnot,
                                                  FX_DWORD nFlag) {
  ASSERT(pAnnot != NULL);

  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    if (pAnnotHandler->OnSetFocus(pAnnot, nFlag)) {
      CPDFSDK_PageView* pPage = pAnnot->GetPageView();
      pPage->GetSDKDocument();
      return TRUE;
    }
  }
  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnKillFocus(CPDFSDK_Annot* pAnnot,
                                                   FX_DWORD nFlag) {
  ASSERT(pAnnot);
  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->OnKillFocus(pAnnot, nFlag);

  return FALSE;
}

CPDF_Rect CPDFSDK_AnnotHandlerMgr::Annot_OnGetViewBBox(
    CPDFSDK_PageView* pPageView,
    CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);
  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot))
    return pAnnotHandler->GetViewBBox(pPageView, pAnnot);

  return pAnnot->GetRect();
}

FX_BOOL CPDFSDK_AnnotHandlerMgr::Annot_OnHitTest(CPDFSDK_PageView* pPageView,
                                                 CPDFSDK_Annot* pAnnot,
                                                 const CPDF_Point& point) {
  ASSERT(pAnnot);
  if (CPDFSDK_AnnotHandler* pAnnotHandler = GetAnnotHandler(pAnnot)) {
    if (pAnnotHandler->CanAnswer(pAnnot))
      return pAnnotHandler->HitTest(pPageView, pAnnot, point);
  }
  return FALSE;
}

CPDFSDK_Annot* CPDFSDK_AnnotHandlerMgr::GetNextAnnot(CPDFSDK_Annot* pSDKAnnot,
                                                     FX_BOOL bNext) {
  CBA_AnnotIterator ai(pSDKAnnot->GetPageView(), "Widget", "");
  return bNext ? ai.GetNextAnnot(pSDKAnnot) : ai.GetPrevAnnot(pSDKAnnot);
}

FX_BOOL CPDFSDK_AnnotHandler::CanAnswer(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot->GetType() == "Widget");
  if (pAnnot->GetSubType() == BFFT_SIGNATURE)
    return FALSE;

  CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
  if (!pWidget->IsVisible())
    return FALSE;

  int nFieldFlags = pWidget->GetFieldFlags();
  if ((nFieldFlags & FIELDFLAG_READONLY) == FIELDFLAG_READONLY)
    return FALSE;

  if (pWidget->GetFieldType() == FIELDTYPE_PUSHBUTTON)
    return TRUE;

  CPDF_Page* pPage = pWidget->GetPDFPage();
  CPDF_Document* pDocument = pPage->m_pDocument;
  FX_DWORD dwPermissions = pDocument->GetUserPermissions();
  return (dwPermissions & FPDFPERM_FILL_FORM) ||
         (dwPermissions & FPDFPERM_ANNOT_FORM);
}

CPDFSDK_Annot* CPDFSDK_AnnotHandler::NewAnnot(CPDF_Annot* pAnnot,
                                              CPDFSDK_PageView* pPage) {
  CPDFSDK_Document* pSDKDoc = m_pApp->GetSDKDocument();
  CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pSDKDoc->GetInterForm();
  CPDF_FormControl* pCtrl = CPDFSDK_Widget::GetFormControl(
      pInterForm->GetInterForm(), pAnnot->GetAnnotDict());
  if (!pCtrl)
    return nullptr;

  CPDFSDK_Widget* pWidget = new CPDFSDK_Widget(pAnnot, pPage, pInterForm);
  pInterForm->AddMap(pCtrl, pWidget);
  CPDF_InterForm* pPDFInterForm = pInterForm->GetInterForm();
  if (pPDFInterForm && pPDFInterForm->NeedConstructAP())
    pWidget->ResetAppearance(nullptr, FALSE);

  return pWidget;
}

void CPDFSDK_AnnotHandler::ReleaseAnnot(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot != NULL);

  if (m_pFormFiller)
    m_pFormFiller->OnDelete(pAnnot);

  CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
  CPDFSDK_InterForm* pInterForm = pWidget->GetInterForm();
  ASSERT(pInterForm != NULL);

  CPDF_FormControl* pCtrol = pWidget->GetFormControl();
  pInterForm->RemoveMap(pCtrol);

  delete pWidget;
}

void CPDFSDK_AnnotHandler::OnDraw(CPDFSDK_PageView* pPageView,
                                  CPDFSDK_Annot* pAnnot,
                                  CFX_RenderDevice* pDevice,
                                  CPDF_Matrix* pUser2Device,
                                  FX_DWORD dwFlags) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
    pAnnot->DrawAppearance(pDevice, pUser2Device, CPDF_Annot::Normal, NULL);
  } else {
    if (m_pFormFiller) {
      m_pFormFiller->OnDraw(pPageView, pAnnot, pDevice, pUser2Device, dwFlags);
    }
  }
}

void CPDFSDK_AnnotHandler::OnMouseEnter(CPDFSDK_PageView* pPageView,
                                        CPDFSDK_Annot* pAnnot,
                                        FX_DWORD nFlag) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      m_pFormFiller->OnMouseEnter(pPageView, pAnnot, nFlag);
  }
}
void CPDFSDK_AnnotHandler::OnMouseExit(CPDFSDK_PageView* pPageView,
                                       CPDFSDK_Annot* pAnnot,
                                       FX_DWORD nFlag) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      m_pFormFiller->OnMouseExit(pPageView, pAnnot, nFlag);
  }
}
FX_BOOL CPDFSDK_AnnotHandler::OnLButtonDown(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot,
                                            FX_DWORD nFlags,
                                            const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnLButtonDown(pPageView, pAnnot, nFlags, point);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnLButtonUp(CPDFSDK_PageView* pPageView,
                                          CPDFSDK_Annot* pAnnot,
                                          FX_DWORD nFlags,
                                          const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnLButtonUp(pPageView, pAnnot, nFlags, point);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              FX_DWORD nFlags,
                                              const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnLButtonDblClk(pPageView, pAnnot, nFlags, point);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnMouseMove(CPDFSDK_PageView* pPageView,
                                          CPDFSDK_Annot* pAnnot,
                                          FX_DWORD nFlags,
                                          const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnMouseMove(pPageView, pAnnot, nFlags, point);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnMouseWheel(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           FX_DWORD nFlags,
                                           short zDelta,
                                           const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnMouseWheel(pPageView, pAnnot, nFlags, zDelta,
                                         point);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnRButtonDown(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot,
                                            FX_DWORD nFlags,
                                            const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnRButtonDown(pPageView, pAnnot, nFlags, point);
  }

  return FALSE;
}
FX_BOOL CPDFSDK_AnnotHandler::OnRButtonUp(CPDFSDK_PageView* pPageView,
                                          CPDFSDK_Annot* pAnnot,
                                          FX_DWORD nFlags,
                                          const CPDF_Point& point) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnRButtonUp(pPageView, pAnnot, nFlags, point);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnChar(CPDFSDK_Annot* pAnnot,
                                     FX_DWORD nChar,
                                     FX_DWORD nFlags) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnChar(pAnnot, nChar, nFlags);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                        int nKeyCode,
                                        int nFlag) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnKeyDown(pAnnot, nKeyCode, nFlag);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_AnnotHandler::OnKeyUp(CPDFSDK_Annot* pAnnot,
                                      int nKeyCode,
                                      int nFlag) {
  return FALSE;
}
void CPDFSDK_AnnotHandler::OnCreate(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      m_pFormFiller->OnCreate(pAnnot);
  }
}

void CPDFSDK_AnnotHandler::OnLoad(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot != NULL);

  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
    if (!pWidget->IsAppearanceValid())
      pWidget->ResetAppearance(NULL, FALSE);

    int nFieldType = pWidget->GetFieldType();
    if (nFieldType == FIELDTYPE_TEXTFIELD || nFieldType == FIELDTYPE_COMBOBOX) {
      FX_BOOL bFormated = FALSE;
      CFX_WideString sValue = pWidget->OnFormat(bFormated);
      if (bFormated && nFieldType == FIELDTYPE_COMBOBOX) {
        pWidget->ResetAppearance(sValue.c_str(), FALSE);
      }
    }

    if (m_pFormFiller)
      m_pFormFiller->OnLoad(pAnnot);
  }
}

FX_BOOL CPDFSDK_AnnotHandler::OnSetFocus(CPDFSDK_Annot* pAnnot,
                                         FX_DWORD nFlag) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnSetFocus(pAnnot, nFlag);
  }

  return TRUE;
}
FX_BOOL CPDFSDK_AnnotHandler::OnKillFocus(CPDFSDK_Annot* pAnnot,
                                          FX_DWORD nFlag) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->OnKillFocus(pAnnot, nFlag);
  }

  return TRUE;
}

CPDF_Rect CPDFSDK_AnnotHandler::GetViewBBox(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot != NULL);
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
  } else {
    if (m_pFormFiller)
      return m_pFormFiller->GetViewBBox(pPageView, pAnnot);
  }

  return CPDF_Rect(0, 0, 0, 0);
}

FX_BOOL CPDFSDK_AnnotHandler::HitTest(CPDFSDK_PageView* pPageView,
                                      CPDFSDK_Annot* pAnnot,
                                      const CPDF_Point& point) {
  ASSERT(pPageView);
  ASSERT(pAnnot);

  CPDF_Rect rect = GetViewBBox(pPageView, pAnnot);
  return rect.Contains(point.x, point.y);
}

// CReader_AnnotIteratorEx

CPDFSDK_AnnotIterator::CPDFSDK_AnnotIterator(CPDFSDK_PageView* pPageView,
                                             FX_BOOL bReverse,
                                             FX_BOOL bIgnoreTopmost /*=FALSE*/,
                                             FX_BOOL bCircle /*=FALSE*/,
                                             CFX_PtrArray* pList /*=NULL*/) {
  ASSERT(pPageView);
  m_bReverse = bReverse;
  m_bIgnoreTopmost = bIgnoreTopmost;
  m_bCircle = bCircle;
  m_pIteratorAnnotList.RemoveAll();
  InitIteratorAnnotList(pPageView, pList);
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::NextAnnot(const CPDFSDK_Annot* pCurrent) {
  int index = -1;
  int nCount = m_pIteratorAnnotList.GetSize();
  if (pCurrent) {
    for (int i = 0; i < nCount; i++) {
      CPDFSDK_Annot* pReaderAnnot =
          (CPDFSDK_Annot*)m_pIteratorAnnotList.GetAt(i);
      if (pReaderAnnot == pCurrent) {
        index = i;
        break;
      }
    }
  }
  return NextAnnot(index);
}
CPDFSDK_Annot* CPDFSDK_AnnotIterator::PrevAnnot(const CPDFSDK_Annot* pCurrent) {
  int index = -1;
  int nCount = m_pIteratorAnnotList.GetSize();
  if (pCurrent) {
    for (int i = 0; i < nCount; i++) {
      CPDFSDK_Annot* pReaderAnnot =
          (CPDFSDK_Annot*)m_pIteratorAnnotList.GetAt(i);
      if (pReaderAnnot == pCurrent) {
        index = i;
        break;
      }
    }
  }
  return PrevAnnot(index);
}
CPDFSDK_Annot* CPDFSDK_AnnotIterator::NextAnnot(int& index) {
  int nCount = m_pIteratorAnnotList.GetSize();
  if (nCount <= 0)
    index = -1;
  else {
    if (index < 0) {
      index = 0;
    } else {
      if (m_bCircle) {
        index = (index < nCount - 1) ? (index + 1) : 0;
      } else {
        index = (index < nCount - 1) ? (index + 1) : -1;
      }
    }
  }
  return (index < 0) ? NULL : (CPDFSDK_Annot*)m_pIteratorAnnotList.GetAt(index);
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::PrevAnnot(int& index) {
  int nCount = m_pIteratorAnnotList.GetSize();
  if (nCount <= 0)
    index = -1;
  else {
    if (index < 0) {
      index = nCount - 1;
    } else {
      if (m_bCircle) {
        index = (index > 0) ? (index - 1) : nCount - 1;
      } else {
        index = (index > 0) ? (index - 1) : -1;
      }
    }
  }
  return (index < 0) ? NULL : (CPDFSDK_Annot*)m_pIteratorAnnotList.GetAt(index);
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::Next(const CPDFSDK_Annot* pCurrent) {
  return (m_bReverse) ? PrevAnnot(pCurrent) : NextAnnot(pCurrent);
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::Prev(const CPDFSDK_Annot* pCurrent) {
  return (m_bReverse) ? NextAnnot(pCurrent) : PrevAnnot(pCurrent);
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::Next(int& index) {
  return (m_bReverse) ? PrevAnnot(index) : NextAnnot(index);
}

CPDFSDK_Annot* CPDFSDK_AnnotIterator::Prev(int& index) {
  return (m_bReverse) ? NextAnnot(index) : PrevAnnot(index);
}

void CPDFSDK_AnnotIterator::InsertSort(CFX_PtrArray& arrayList,
                                       AI_COMPARE pCompare) {
  for (int i = 1; i < arrayList.GetSize(); i++) {
    if (pCompare((CPDFSDK_Annot*)(arrayList[i]),
                 (CPDFSDK_Annot*)(arrayList[i - 1])) < 0) {
      int j = i - 1;
      CPDFSDK_Annot* pTemp = (CPDFSDK_Annot*)arrayList[i];

      do {
        arrayList[j + 1] = arrayList[j];
      } while (--j >= 0 && pCompare(pTemp, (CPDFSDK_Annot*)arrayList[j]) < 0);

      arrayList[j + 1] = pTemp;
    }
  }
}

int LyOrderCompare(CPDFSDK_Annot* p1, CPDFSDK_Annot* p2) {
  if (p1->GetLayoutOrder() < p2->GetLayoutOrder())
    return -1;
  if (p1->GetLayoutOrder() > p2->GetLayoutOrder())
    return 1;
  return 0;
}

FX_BOOL CPDFSDK_AnnotIterator::InitIteratorAnnotList(
    CPDFSDK_PageView* pPageView,
    CFX_PtrArray* pAnnotList) {
  ASSERT(pPageView);

  if (pAnnotList == NULL) {
    pAnnotList = pPageView->GetAnnotList();
  }

  m_pIteratorAnnotList.RemoveAll();
  if (!pAnnotList)
    return FALSE;

  CPDFSDK_Annot* pTopMostAnnot =
      (m_bIgnoreTopmost) ? NULL : pPageView->GetFocusAnnot();

  int nCount = pAnnotList->GetSize();

  for (int i = nCount - 1; i >= 0; i--) {
    CPDFSDK_Annot* pReaderAnnot = (CPDFSDK_Annot*)pAnnotList->GetAt(i);
    m_pIteratorAnnotList.Add(pReaderAnnot);
  }

  InsertSort(m_pIteratorAnnotList, &LyOrderCompare);

  if (pTopMostAnnot) {
    for (int i = 0; i < nCount; i++) {
      CPDFSDK_Annot* pReaderAnnot =
          (CPDFSDK_Annot*)m_pIteratorAnnotList.GetAt(i);
      if (pReaderAnnot == pTopMostAnnot) {
        m_pIteratorAnnotList.RemoveAt(i);
        m_pIteratorAnnotList.InsertAt(0, pReaderAnnot);
        break;
      }
    }
  }

  return TRUE;
}
