// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_bfannothandler.h"

#include <memory>
#include <vector>

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfdoc/include/cpdf_interform.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"
#include "fpdfsdk/include/cpdfsdk_widget.h"
#include "fpdfsdk/include/fsdk_mgr.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_BFAnnotHandler::CPDFSDK_BFAnnotHandler(CPDFDoc_Environment* pApp)
    : m_pApp(pApp), m_pFormFiller(nullptr) {}

CPDFSDK_BFAnnotHandler::~CPDFSDK_BFAnnotHandler() {}

CFX_ByteString CPDFSDK_BFAnnotHandler::GetType() {
  return CFX_ByteString("Widget");
}

CFX_ByteString CPDFSDK_BFAnnotHandler::GetName() {
  return CFX_ByteString("WidgetHandler");
}

FX_BOOL CPDFSDK_BFAnnotHandler::CanAnswer(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot->GetType() == "Widget");
  if (pAnnot->GetSubType() == BFFT_SIGNATURE)
    return FALSE;

  CPDFSDK_Widget* pWidget = static_cast<CPDFSDK_Widget*>(pAnnot);
  if (!pWidget->IsVisible())
    return FALSE;

  int nFieldFlags = pWidget->GetFieldFlags();
  if ((nFieldFlags & FIELDFLAG_READONLY) == FIELDFLAG_READONLY)
    return FALSE;

  if (pWidget->GetFieldType() == FIELDTYPE_PUSHBUTTON)
    return TRUE;

  CPDF_Page* pPage = pWidget->GetPDFPage();
  CPDF_Document* pDocument = pPage->m_pDocument;
  uint32_t dwPermissions = pDocument->GetUserPermissions();
  return (dwPermissions & FPDFPERM_FILL_FORM) ||
         (dwPermissions & FPDFPERM_ANNOT_FORM);
}

CPDFSDK_Annot* CPDFSDK_BFAnnotHandler::NewAnnot(CPDF_Annot* pAnnot,
                                                CPDFSDK_PageView* pPage) {
  CPDFSDK_Document* pSDKDoc = m_pApp->GetSDKDocument();
  CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm();
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

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_BFAnnotHandler::NewAnnot(CXFA_FFWidget* hWidget,
                                                CPDFSDK_PageView* pPage) {
  return nullptr;
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_BFAnnotHandler::ReleaseAnnot(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);

  if (m_pFormFiller)
    m_pFormFiller->OnDelete(pAnnot);

  std::unique_ptr<CPDFSDK_Widget> pWidget(static_cast<CPDFSDK_Widget*>(pAnnot));
  CPDFSDK_InterForm* pInterForm = pWidget->GetInterForm();
  CPDF_FormControl* pControl = pWidget->GetFormControl();
  pInterForm->RemoveMap(pControl);
}

void CPDFSDK_BFAnnotHandler::DeleteAnnot(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_BFAnnotHandler::OnDraw(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot,
                                    CFX_RenderDevice* pDevice,
                                    CFX_Matrix* pUser2Device,
                                    uint32_t dwFlags) {
  CFX_ByteString sSubType = pAnnot->GetSubType();

  if (sSubType == BFFT_SIGNATURE) {
    static_cast<CPDFSDK_BAAnnot*>(pAnnot)->DrawAppearance(
        pDevice, pUser2Device, CPDF_Annot::Normal, nullptr);
  } else {
    if (m_pFormFiller)
      m_pFormFiller->OnDraw(pPageView, pAnnot, pDevice, pUser2Device, dwFlags);
  }
}

void CPDFSDK_BFAnnotHandler::OnDrawSleep(CPDFSDK_PageView* pPageView,
                                         CPDFSDK_Annot* pAnnot,
                                         CFX_RenderDevice* pDevice,
                                         CFX_Matrix* pUser2Device,
                                         const CFX_FloatRect& rcWindow,
                                         uint32_t dwFlags) {}

void CPDFSDK_BFAnnotHandler::OnDelete(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_BFAnnotHandler::OnRelease(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_BFAnnotHandler::OnMouseEnter(CPDFSDK_PageView* pPageView,
                                          CPDFSDK_Annot* pAnnot,
                                          uint32_t nFlag) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    m_pFormFiller->OnMouseEnter(pPageView, pAnnot, nFlag);
}

void CPDFSDK_BFAnnotHandler::OnMouseExit(CPDFSDK_PageView* pPageView,
                                         CPDFSDK_Annot* pAnnot,
                                         uint32_t nFlag) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    m_pFormFiller->OnMouseExit(pPageView, pAnnot, nFlag);
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnLButtonDown(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              uint32_t nFlags,
                                              const CFX_FloatPoint& point) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnLButtonDown(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnLButtonUp(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot,
                                            uint32_t nFlags,
                                            const CFX_FloatPoint& point) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnLButtonUp(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot,
                                                uint32_t nFlags,
                                                const CFX_FloatPoint& point) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnLButtonDblClk(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnMouseMove(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot,
                                            uint32_t nFlags,
                                            const CFX_FloatPoint& point) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnMouseMove(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnMouseWheel(CPDFSDK_PageView* pPageView,
                                             CPDFSDK_Annot* pAnnot,
                                             uint32_t nFlags,
                                             short zDelta,
                                             const CFX_FloatPoint& point) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnMouseWheel(pPageView, pAnnot, nFlags, zDelta,
                                       point);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnRButtonDown(CPDFSDK_PageView* pPageView,
                                              CPDFSDK_Annot* pAnnot,
                                              uint32_t nFlags,
                                              const CFX_FloatPoint& point) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnRButtonDown(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnRButtonUp(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot,
                                            uint32_t nFlags,
                                            const CFX_FloatPoint& point) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnRButtonUp(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnRButtonDblClk(CPDFSDK_PageView* pPageView,
                                                CPDFSDK_Annot* pAnnot,
                                                uint32_t nFlags,
                                                const CFX_FloatPoint& point) {
  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnChar(CPDFSDK_Annot* pAnnot,
                                       uint32_t nChar,
                                       uint32_t nFlags) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnChar(pAnnot, nChar, nFlags);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                          int nKeyCode,
                                          int nFlag) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnKeyDown(pAnnot, nKeyCode, nFlag);

  return FALSE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnKeyUp(CPDFSDK_Annot* pAnnot,
                                        int nKeyCode,
                                        int nFlag) {
  return FALSE;
}

void CPDFSDK_BFAnnotHandler::OnCreate(CPDFSDK_Annot* pAnnot) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    m_pFormFiller->OnCreate(pAnnot);
}

void CPDFSDK_BFAnnotHandler::OnLoad(CPDFSDK_Annot* pAnnot) {
  if (pAnnot->GetSubType() == BFFT_SIGNATURE)
    return;

  CPDFSDK_Widget* pWidget = static_cast<CPDFSDK_Widget*>(pAnnot);
  if (!pWidget->IsAppearanceValid())
    pWidget->ResetAppearance(nullptr, FALSE);

  int nFieldType = pWidget->GetFieldType();
  if (nFieldType == FIELDTYPE_TEXTFIELD || nFieldType == FIELDTYPE_COMBOBOX) {
    FX_BOOL bFormated = FALSE;
    CFX_WideString sValue = pWidget->OnFormat(bFormated);
    if (bFormated && nFieldType == FIELDTYPE_COMBOBOX)
      pWidget->ResetAppearance(sValue.c_str(), FALSE);
  }

#ifdef PDF_ENABLE_XFA
  CPDFSDK_PageView* pPageView = pAnnot->GetPageView();
  CPDFSDK_Document* pSDKDoc = pPageView->GetSDKDocument();
  CPDFXFA_Document* pDoc = pSDKDoc->GetXFADocument();
  if (pDoc->GetDocType() == DOCTYPE_STATIC_XFA) {
    if (!pWidget->IsAppearanceValid() && !pWidget->GetValue().IsEmpty())
      pWidget->ResetAppearance(FALSE);
  }
#endif  // PDF_ENABLE_XFA
  if (m_pFormFiller)
    m_pFormFiller->OnLoad(pAnnot);
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnSetFocus(CPDFSDK_Annot* pAnnot,
                                           uint32_t nFlag) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnSetFocus(pAnnot, nFlag);

  return TRUE;
}

FX_BOOL CPDFSDK_BFAnnotHandler::OnKillFocus(CPDFSDK_Annot* pAnnot,
                                            uint32_t nFlag) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return m_pFormFiller->OnKillFocus(pAnnot, nFlag);

  return TRUE;
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CPDFSDK_BFAnnotHandler::OnXFAChangedFocus(CPDFSDK_Annot* pOldAnnot,
                                                  CPDFSDK_Annot* pNewAnnot) {
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

CFX_FloatRect CPDFSDK_BFAnnotHandler::GetViewBBox(CPDFSDK_PageView* pPageView,
                                                  CPDFSDK_Annot* pAnnot) {
  if (pAnnot->GetSubType() != BFFT_SIGNATURE && m_pFormFiller)
    return CFX_FloatRect(m_pFormFiller->GetViewBBox(pPageView, pAnnot));

  return CFX_FloatRect(0, 0, 0, 0);
}

FX_BOOL CPDFSDK_BFAnnotHandler::HitTest(CPDFSDK_PageView* pPageView,
                                        CPDFSDK_Annot* pAnnot,
                                        const CFX_FloatPoint& point) {
  ASSERT(pPageView);
  ASSERT(pAnnot);

  CFX_FloatRect rect = GetViewBBox(pPageView, pAnnot);
  return rect.Contains(point.x, point.y);
}
