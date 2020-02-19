// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffwidgethandler.h"

#include <vector>

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_checkbutton.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_ui.h"
#include "xfa/fxfa/parser/cxfa_validate.h"

CXFA_FFWidgetHandler::CXFA_FFWidgetHandler(CXFA_FFDocView* pDocView)
    : m_pDocView(pDocView) {}

CXFA_FFWidgetHandler::~CXFA_FFWidgetHandler() {}

bool CXFA_FFWidgetHandler::OnMouseEnter(CXFA_FFWidget* hWidget) {
  m_pDocView->LockUpdate();
  bool bRet = hWidget->OnMouseEnter();
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}

bool CXFA_FFWidgetHandler::OnMouseExit(CXFA_FFWidget* hWidget) {
  m_pDocView->LockUpdate();
  bool bRet = hWidget->OnMouseExit();
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}

bool CXFA_FFWidgetHandler::OnLButtonDown(CXFA_FFWidget* hWidget,
                                         uint32_t dwFlags,
                                         const CFX_PointF& point) {
  m_pDocView->LockUpdate();
  bool bRet = hWidget->AcceptsFocusOnButtonDown(
      dwFlags, hWidget->Rotate2Normal(point), FWL_MouseCommand::LeftButtonDown);
  if (bRet) {
    if (m_pDocView->SetFocus(hWidget)) {
      m_pDocView->GetDoc()->GetDocEnvironment()->SetFocusWidget(
          m_pDocView->GetDoc(), hWidget);
    }
    bRet = hWidget->OnLButtonDown(dwFlags, hWidget->Rotate2Normal(point));
  }
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}

bool CXFA_FFWidgetHandler::OnLButtonUp(CXFA_FFWidget* hWidget,
                                       uint32_t dwFlags,
                                       const CFX_PointF& point) {
  m_pDocView->LockUpdate();
  m_pDocView->m_bLayoutEvent = true;
  bool bRet = hWidget->OnLButtonUp(dwFlags, hWidget->Rotate2Normal(point));
  m_pDocView->UnlockUpdate();
  m_pDocView->UpdateDocView();
  return bRet;
}

bool CXFA_FFWidgetHandler::OnLButtonDblClk(CXFA_FFWidget* hWidget,
                                           uint32_t dwFlags,
                                           const CFX_PointF& point) {
  bool bRet = hWidget->OnLButtonDblClk(dwFlags, hWidget->Rotate2Normal(point));
  return bRet;
}

bool CXFA_FFWidgetHandler::OnMouseMove(CXFA_FFWidget* hWidget,
                                       uint32_t dwFlags,
                                       const CFX_PointF& point) {
  bool bRet = hWidget->OnMouseMove(dwFlags, hWidget->Rotate2Normal(point));
  return bRet;
}

bool CXFA_FFWidgetHandler::OnMouseWheel(CXFA_FFWidget* hWidget,
                                        uint32_t dwFlags,
                                        int16_t zDelta,
                                        const CFX_PointF& point) {
  bool bRet =
      hWidget->OnMouseWheel(dwFlags, zDelta, hWidget->Rotate2Normal(point));
  return bRet;
}

bool CXFA_FFWidgetHandler::OnRButtonDown(CXFA_FFWidget* hWidget,
                                         uint32_t dwFlags,
                                         const CFX_PointF& point) {
  bool bRet =
      hWidget->AcceptsFocusOnButtonDown(dwFlags, hWidget->Rotate2Normal(point),
                                        FWL_MouseCommand::RightButtonDown);
  if (bRet) {
    if (m_pDocView->SetFocus(hWidget)) {
      m_pDocView->GetDoc()->GetDocEnvironment()->SetFocusWidget(
          m_pDocView->GetDoc(), hWidget);
    }
    bRet = hWidget->OnRButtonDown(dwFlags, hWidget->Rotate2Normal(point));
  }
  return bRet;
}

bool CXFA_FFWidgetHandler::OnRButtonUp(CXFA_FFWidget* hWidget,
                                       uint32_t dwFlags,
                                       const CFX_PointF& point) {
  bool bRet = hWidget->OnRButtonUp(dwFlags, hWidget->Rotate2Normal(point));
  return bRet;
}

bool CXFA_FFWidgetHandler::OnRButtonDblClk(CXFA_FFWidget* hWidget,
                                           uint32_t dwFlags,
                                           const CFX_PointF& point) {
  bool bRet = hWidget->OnRButtonDblClk(dwFlags, hWidget->Rotate2Normal(point));
  return bRet;
}

bool CXFA_FFWidgetHandler::OnKeyDown(CXFA_FFWidget* hWidget,
                                     uint32_t dwKeyCode,
                                     uint32_t dwFlags) {
  bool bRet = hWidget->OnKeyDown(dwKeyCode, dwFlags);
  m_pDocView->UpdateDocView();
  return bRet;
}

bool CXFA_FFWidgetHandler::OnKeyUp(CXFA_FFWidget* hWidget,
                                   uint32_t dwKeyCode,
                                   uint32_t dwFlags) {
  bool bRet = hWidget->OnKeyUp(dwKeyCode, dwFlags);
  return bRet;
}

bool CXFA_FFWidgetHandler::OnChar(CXFA_FFWidget* hWidget,
                                  uint32_t dwChar,
                                  uint32_t dwFlags) {
  bool bRet = hWidget->OnChar(dwChar, dwFlags);
  return bRet;
}

WideString CXFA_FFWidgetHandler::GetText(CXFA_FFWidget* widget) {
  return widget->GetText();
}

WideString CXFA_FFWidgetHandler::GetSelectedText(CXFA_FFWidget* widget) {
  if (!widget->CanCopy())
    return WideString();

  return widget->Copy().value_or(WideString());
}

void CXFA_FFWidgetHandler::PasteText(CXFA_FFWidget* widget,
                                     const WideString& text) {
  if (!widget->CanPaste())
    return;

  widget->Paste(text);
}

bool CXFA_FFWidgetHandler::CanUndo(CXFA_FFWidget* widget) {
  return widget->CanUndo();
}

bool CXFA_FFWidgetHandler::CanRedo(CXFA_FFWidget* widget) {
  return widget->CanRedo();
}

bool CXFA_FFWidgetHandler::Undo(CXFA_FFWidget* widget) {
  return widget->Undo();
}

bool CXFA_FFWidgetHandler::Redo(CXFA_FFWidget* widget) {
  return widget->Redo();
}

FWL_WidgetHit CXFA_FFWidgetHandler::HitTest(CXFA_FFWidget* pWidget,
                                            const CFX_PointF& point) {
  if (!pWidget->GetLayoutItem()->TestStatusBits(XFA_WidgetStatus_Visible))
    return FWL_WidgetHit::Unknown;
  return pWidget->HitTest(pWidget->Rotate2Normal(point));
}

void CXFA_FFWidgetHandler::RenderWidget(CXFA_FFWidget* hWidget,
                                        CXFA_Graphics* pGS,
                                        const CFX_Matrix& matrix,
                                        bool bHighlight) {
  hWidget->RenderWidget(
      pGS, matrix,
      bHighlight ? CXFA_FFWidget::kHighlight : CXFA_FFWidget::kNoHighlight);
}

bool CXFA_FFWidgetHandler::HasEvent(CXFA_Node* pNode,
                                    XFA_EVENTTYPE eEventType) {
  if (eEventType == XFA_EVENT_Unknown)
    return false;
  if (!pNode || pNode->GetElementType() == XFA_Element::Draw)
    return false;

  switch (eEventType) {
    case XFA_EVENT_Calculate: {
      CXFA_Calculate* calc = pNode->GetCalculateIfExists();
      return calc && calc->GetScriptIfExists();
    }
    case XFA_EVENT_Validate: {
      CXFA_Validate* validate = pNode->GetValidateIfExists();
      return validate && validate->GetScriptIfExists();
    }
    default:
      break;
  }
  return !pNode->GetEventByActivity(gs_EventActivity[eEventType], false)
              .empty();
}

XFA_EventError CXFA_FFWidgetHandler::ProcessEvent(CXFA_Node* pNode,
                                                  CXFA_EventParam* pParam) {
  if (!pParam || pParam->m_eType == XFA_EVENT_Unknown)
    return XFA_EventError::kNotExist;
  if (!pNode || pNode->GetElementType() == XFA_Element::Draw)
    return XFA_EventError::kNotExist;

  switch (pParam->m_eType) {
    case XFA_EVENT_Calculate:
      return pNode->ProcessCalculate(m_pDocView.Get());
    case XFA_EVENT_Validate:
      if (m_pDocView->GetDoc()->GetDocEnvironment()->IsValidationsEnabled(
              m_pDocView->GetDoc())) {
        return pNode->ProcessValidate(m_pDocView.Get(), 0);
      }
      return XFA_EventError::kDisabled;
    case XFA_EVENT_InitCalculate: {
      CXFA_Calculate* calc = pNode->GetCalculateIfExists();
      if (!calc)
        return XFA_EventError::kNotExist;
      if (pNode->IsUserInteractive())
        return XFA_EventError::kDisabled;
      return pNode->ExecuteScript(m_pDocView.Get(), calc->GetScriptIfExists(),
                                  pParam);
    }
    default:
      break;
  }
  return pNode->ProcessEvent(m_pDocView.Get(),
                             gs_EventActivity[pParam->m_eType], pParam);
}
