// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_widgethandler.h"

#include <memory>
#include <vector>

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfdoc/include/cpdf_interform.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/include/cpdfsdk_widget.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_WidgetHandler::CPDFSDK_WidgetHandler(CPDFSDK_Environment* pApp)
    : m_pApp(pApp), m_pFormFiller(nullptr) {}

CPDFSDK_WidgetHandler::~CPDFSDK_WidgetHandler() {}

FX_BOOL CPDFSDK_WidgetHandler::CanAnswer(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::WIDGET);
  if (pAnnot->IsSignatureWidget())
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

CPDFSDK_Annot* CPDFSDK_WidgetHandler::NewAnnot(CPDF_Annot* pAnnot,
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
CPDFSDK_Annot* CPDFSDK_WidgetHandler::NewAnnot(CXFA_FFWidget* hWidget,
                                               CPDFSDK_PageView* pPage) {
  return nullptr;
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_WidgetHandler::ReleaseAnnot(CPDFSDK_Annot* pAnnot) {
  ASSERT(pAnnot);

  if (m_pFormFiller)
    m_pFormFiller->OnDelete(pAnnot);

  std::unique_ptr<CPDFSDK_Widget> pWidget(static_cast<CPDFSDK_Widget*>(pAnnot));
  CPDFSDK_InterForm* pInterForm = pWidget->GetInterForm();
  CPDF_FormControl* pControl = pWidget->GetFormControl();
  pInterForm->RemoveMap(pControl);
}

void CPDFSDK_WidgetHandler::DeleteAnnot(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_WidgetHandler::OnDraw(CPDFSDK_PageView* pPageView,
                                   CPDFSDK_Annot* pAnnot,
                                   CFX_RenderDevice* pDevice,
                                   CFX_Matrix* pUser2Device,
                                   bool bDrawAnnots) {
  if (pAnnot->IsSignatureWidget()) {
    static_cast<CPDFSDK_BAAnnot*>(pAnnot)->DrawAppearance(
        pDevice, pUser2Device, CPDF_Annot::Normal, nullptr);
  } else {
    if (m_pFormFiller)
      m_pFormFiller->OnDraw(pPageView, pAnnot, pDevice, pUser2Device);
  }
}

void CPDFSDK_WidgetHandler::OnDelete(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_WidgetHandler::OnRelease(CPDFSDK_Annot* pAnnot) {}

void CPDFSDK_WidgetHandler::OnMouseEnter(CPDFSDK_PageView* pPageView,
                                         CPDFSDK_Annot* pAnnot,
                                         uint32_t nFlag) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    m_pFormFiller->OnMouseEnter(pPageView, pAnnot, nFlag);
}

void CPDFSDK_WidgetHandler::OnMouseExit(CPDFSDK_PageView* pPageView,
                                        CPDFSDK_Annot* pAnnot,
                                        uint32_t nFlag) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    m_pFormFiller->OnMouseExit(pPageView, pAnnot, nFlag);
}

FX_BOOL CPDFSDK_WidgetHandler::OnLButtonDown(CPDFSDK_PageView* pPageView,
                                             CPDFSDK_Annot* pAnnot,
                                             uint32_t nFlags,
                                             const CFX_FloatPoint& point) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnLButtonDown(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnLButtonUp(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           uint32_t nFlags,
                                           const CFX_FloatPoint& point) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnLButtonUp(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                                               CPDFSDK_Annot* pAnnot,
                                               uint32_t nFlags,
                                               const CFX_FloatPoint& point) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnLButtonDblClk(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnMouseMove(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           uint32_t nFlags,
                                           const CFX_FloatPoint& point) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnMouseMove(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnMouseWheel(CPDFSDK_PageView* pPageView,
                                            CPDFSDK_Annot* pAnnot,
                                            uint32_t nFlags,
                                            short zDelta,
                                            const CFX_FloatPoint& point) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnMouseWheel(pPageView, pAnnot, nFlags, zDelta,
                                       point);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnRButtonDown(CPDFSDK_PageView* pPageView,
                                             CPDFSDK_Annot* pAnnot,
                                             uint32_t nFlags,
                                             const CFX_FloatPoint& point) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnRButtonDown(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnRButtonUp(CPDFSDK_PageView* pPageView,
                                           CPDFSDK_Annot* pAnnot,
                                           uint32_t nFlags,
                                           const CFX_FloatPoint& point) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnRButtonUp(pPageView, pAnnot, nFlags, point);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnRButtonDblClk(CPDFSDK_PageView* pPageView,
                                               CPDFSDK_Annot* pAnnot,
                                               uint32_t nFlags,
                                               const CFX_FloatPoint& point) {
  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnChar(CPDFSDK_Annot* pAnnot,
                                      uint32_t nChar,
                                      uint32_t nFlags) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnChar(pAnnot, nChar, nFlags);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnKeyDown(CPDFSDK_Annot* pAnnot,
                                         int nKeyCode,
                                         int nFlag) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnKeyDown(pAnnot, nKeyCode, nFlag);

  return FALSE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnKeyUp(CPDFSDK_Annot* pAnnot,
                                       int nKeyCode,
                                       int nFlag) {
  return FALSE;
}

void CPDFSDK_WidgetHandler::OnCreate(CPDFSDK_Annot* pAnnot) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    m_pFormFiller->OnCreate(pAnnot);
}

void CPDFSDK_WidgetHandler::OnLoad(CPDFSDK_Annot* pAnnot) {
  if (pAnnot->IsSignatureWidget())
    return;

  CPDFSDK_Widget* pWidget = static_cast<CPDFSDK_Widget*>(pAnnot);
  if (!pWidget->IsAppearanceValid())
    pWidget->ResetAppearance(nullptr, FALSE);

  int nFieldType = pWidget->GetFieldType();
  if (nFieldType == FIELDTYPE_TEXTFIELD || nFieldType == FIELDTYPE_COMBOBOX) {
    FX_BOOL bFormatted = FALSE;
    CFX_WideString sValue = pWidget->OnFormat(bFormatted);
    if (bFormatted && nFieldType == FIELDTYPE_COMBOBOX)
      pWidget->ResetAppearance(&sValue, FALSE);
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

FX_BOOL CPDFSDK_WidgetHandler::OnSetFocus(CPDFSDK_Annot* pAnnot,
                                          uint32_t nFlag) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnSetFocus(pAnnot, nFlag);

  return TRUE;
}

FX_BOOL CPDFSDK_WidgetHandler::OnKillFocus(CPDFSDK_Annot* pAnnot,
                                           uint32_t nFlag) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return m_pFormFiller->OnKillFocus(pAnnot, nFlag);

  return TRUE;
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CPDFSDK_WidgetHandler::OnXFAChangedFocus(CPDFSDK_Annot* pOldAnnot,
                                                 CPDFSDK_Annot* pNewAnnot) {
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

CFX_FloatRect CPDFSDK_WidgetHandler::GetViewBBox(CPDFSDK_PageView* pPageView,
                                                 CPDFSDK_Annot* pAnnot) {
  if (!pAnnot->IsSignatureWidget() && m_pFormFiller)
    return CFX_FloatRect(m_pFormFiller->GetViewBBox(pPageView, pAnnot));

  return CFX_FloatRect(0, 0, 0, 0);
}

FX_BOOL CPDFSDK_WidgetHandler::HitTest(CPDFSDK_PageView* pPageView,
                                       CPDFSDK_Annot* pAnnot,
                                       const CFX_FloatPoint& point) {
  ASSERT(pPageView);
  ASSERT(pAnnot);

  CFX_FloatRect rect = GetViewBBox(pPageView, pAnnot);
  return rect.Contains(point.x, point.y);
}
