// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_checkbox.h"

#include <algorithm>
#include <utility>

#include "xfa/fde/cfde_textout.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

namespace {

const int kCaptionMargin = 5;

}  // namespace

CFWL_CheckBox::CFWL_CheckBox(CFWL_App* app)
    : CFWL_Widget(app, Properties(), nullptr) {
  m_TTOStyles.single_line_ = true;
}

CFWL_CheckBox::~CFWL_CheckBox() = default;

FWL_Type CFWL_CheckBox::GetClassID() const {
  return FWL_Type::CheckBox;
}

void CFWL_CheckBox::SetBoxSize(float fHeight) {
  m_fBoxHeight = fHeight;
}

void CFWL_CheckBox::Update() {
  if (IsLocked())
    return;

  UpdateTextOutStyles();
  Layout();
}

void CFWL_CheckBox::DrawWidget(CFGAS_GEGraphics* pGraphics,
                               const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);

  Mask<CFWL_PartState> dwStates = GetPartStates();
  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackground, this,
                             pGraphics);
  param.m_dwStates = dwStates;
  param.m_matrix = matrix;
  param.m_PartRect = m_ClientRect;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)
    param.m_pRtData = &m_FocusRect;
  pTheme->DrawBackground(param);

  CFWL_ThemeBackground checkParam(CFWL_ThemePart::Part::kCheckBox, this,
                                  pGraphics);
  checkParam.m_dwStates = dwStates;
  checkParam.m_matrix = matrix;
  checkParam.m_PartRect = m_BoxRect;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)
    checkParam.m_pRtData = &m_FocusRect;
  pTheme->DrawBackground(checkParam);

  CFWL_ThemeText textParam(CFWL_ThemePart::Part::kCaption, this, pGraphics);
  textParam.m_dwStates = dwStates;
  textParam.m_matrix = matrix;
  textParam.m_PartRect = m_CaptionRect;
  textParam.m_wsText = WideString::FromASCII("Check box");
  textParam.m_dwTTOStyles = m_TTOStyles;
  textParam.m_iTTOAlign = m_iTTOAlign;
  pTheme->DrawText(textParam);
}

void CFWL_CheckBox::SetCheckState(int32_t iCheck) {
  m_Properties.m_dwStates &= ~FWL_STATE_CKB_CheckMask;
  switch (iCheck) {
    case 1:
      m_Properties.m_dwStates |= FWL_STATE_CKB_Checked;
      break;
    case 2:
      if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CKB_3State)
        m_Properties.m_dwStates |= FWL_STATE_CKB_Neutral;
      break;
    default:
      break;
  }
  RepaintRect(m_ClientRect);
}

void CFWL_CheckBox::Layout() {
  m_WidgetRect.width = FXSYS_roundf(m_WidgetRect.width);
  m_WidgetRect.height = FXSYS_roundf(m_WidgetRect.height);
  m_ClientRect = GetClientRect();

  float fTextLeft = m_ClientRect.left + m_fBoxHeight;
  m_BoxRect = CFX_RectF(m_ClientRect.TopLeft(), m_fBoxHeight, m_fBoxHeight);
  m_CaptionRect =
      CFX_RectF(fTextLeft, m_ClientRect.top, m_ClientRect.right() - fTextLeft,
                m_ClientRect.height);
  m_CaptionRect.Inflate(-kCaptionMargin, -kCaptionMargin);

  CFX_RectF rtFocus = m_CaptionRect;
  CalcTextRect(WideString::FromASCII("Check box"), m_TTOStyles, m_iTTOAlign,
               &rtFocus);
  m_FocusRect = CFX_RectF(m_CaptionRect.TopLeft(),
                          std::max(m_CaptionRect.width, rtFocus.width),
                          std::min(m_CaptionRect.height, rtFocus.height));
  m_FocusRect.Inflate(1, 1);
}

Mask<CFWL_PartState> CFWL_CheckBox::GetPartStates() const {
  Mask<CFWL_PartState> dwStates = CFWL_PartState::kNormal;
  if ((m_Properties.m_dwStates & FWL_STATE_CKB_CheckMask) ==
      FWL_STATE_CKB_Neutral) {
    dwStates = CFWL_PartState::kNeutral;
  } else if ((m_Properties.m_dwStates & FWL_STATE_CKB_CheckMask) ==
             FWL_STATE_CKB_Checked) {
    dwStates = CFWL_PartState::kChecked;
  }
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)
    dwStates |= CFWL_PartState::kDisabled;
  else if (m_Properties.m_dwStates & FWL_STATE_CKB_Hovered)
    dwStates |= CFWL_PartState::kHovered;
  else if (m_Properties.m_dwStates & FWL_STATE_CKB_Pressed)
    dwStates |= CFWL_PartState::kPressed;
  else
    dwStates |= CFWL_PartState::kNormal;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)
    dwStates |= CFWL_PartState::kFocused;
  return dwStates;
}

void CFWL_CheckBox::UpdateTextOutStyles() {
  m_iTTOAlign = FDE_TextAlignment::kTopLeft;
  m_TTOStyles.Reset();
  m_TTOStyles.single_line_ = true;
}

void CFWL_CheckBox::NextStates() {
  uint32_t dwFirststate = m_Properties.m_dwStates;
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CKB_RadioButton) {
    if ((m_Properties.m_dwStates & FWL_STATE_CKB_CheckMask) ==
        FWL_STATE_CKB_Unchecked) {
      m_Properties.m_dwStates |= FWL_STATE_CKB_Checked;
    }
  } else {
    if ((m_Properties.m_dwStates & FWL_STATE_CKB_CheckMask) ==
        FWL_STATE_CKB_Neutral) {
      m_Properties.m_dwStates &= ~FWL_STATE_CKB_CheckMask;
      if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CKB_3State)
        m_Properties.m_dwStates |= FWL_STATE_CKB_Checked;
    } else if ((m_Properties.m_dwStates & FWL_STATE_CKB_CheckMask) ==
               FWL_STATE_CKB_Checked) {
      m_Properties.m_dwStates &= ~FWL_STATE_CKB_CheckMask;
    } else {
      if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CKB_3State)
        m_Properties.m_dwStates |= FWL_STATE_CKB_Neutral;
      else
        m_Properties.m_dwStates |= FWL_STATE_CKB_Checked;
    }
  }

  RepaintRect(m_ClientRect);
  if (dwFirststate == m_Properties.m_dwStates)
    return;

  CFWL_Event wmCheckBoxState(CFWL_Event::Type::CheckStateChanged, this);
  DispatchEvent(&wmCheckBoxState);
}

void CFWL_CheckBox::OnProcessMessage(CFWL_Message* pMessage) {
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
      OnFocusGained();
      break;
    case CFWL_Message::Type::kKillFocus:
      OnFocusLost();
      break;
    case CFWL_Message::Type::kMouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
          OnLButtonDown();
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kMove:
          OnMouseMove(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeave:
          OnMouseLeave();
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::kKey: {
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->m_dwCmd == CFWL_MessageKey::KeyCommand::kKeyDown)
        OnKeyDown(pKey);
      break;
    }
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget())
    CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_CheckBox::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                 const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_CheckBox::OnFocusGained() {
  m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
  RepaintRect(m_ClientRect);
}

void CFWL_CheckBox::OnFocusLost() {
  m_Properties.m_dwStates &= ~FWL_STATE_WGT_Focused;
  RepaintRect(m_ClientRect);
}

void CFWL_CheckBox::OnLButtonDown() {
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)
    return;

  m_bBtnDown = true;
  m_Properties.m_dwStates &= ~FWL_STATE_CKB_Hovered;
  m_Properties.m_dwStates |= FWL_STATE_CKB_Pressed;
  RepaintRect(m_ClientRect);
}

void CFWL_CheckBox::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (!m_bBtnDown)
    return;

  m_bBtnDown = false;
  if (!m_ClientRect.Contains(pMsg->m_pos))
    return;

  m_Properties.m_dwStates |= FWL_STATE_CKB_Hovered;
  m_Properties.m_dwStates &= ~FWL_STATE_CKB_Pressed;
  NextStates();
}

void CFWL_CheckBox::OnMouseMove(CFWL_MessageMouse* pMsg) {
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)
    return;

  bool bRepaint = false;
  if (m_bBtnDown) {
    if (m_ClientRect.Contains(pMsg->m_pos)) {
      if ((m_Properties.m_dwStates & FWL_STATE_CKB_Pressed) == 0) {
        bRepaint = true;
        m_Properties.m_dwStates |= FWL_STATE_CKB_Pressed;
      }
      if ((m_Properties.m_dwStates & FWL_STATE_CKB_Hovered)) {
        bRepaint = true;
        m_Properties.m_dwStates &= ~FWL_STATE_CKB_Hovered;
      }
    } else {
      if (m_Properties.m_dwStates & FWL_STATE_CKB_Pressed) {
        bRepaint = true;
        m_Properties.m_dwStates &= ~FWL_STATE_CKB_Pressed;
      }
      if ((m_Properties.m_dwStates & FWL_STATE_CKB_Hovered) == 0) {
        bRepaint = true;
        m_Properties.m_dwStates |= FWL_STATE_CKB_Hovered;
      }
    }
  } else {
    if (m_ClientRect.Contains(pMsg->m_pos)) {
      if ((m_Properties.m_dwStates & FWL_STATE_CKB_Hovered) == 0) {
        bRepaint = true;
        m_Properties.m_dwStates |= FWL_STATE_CKB_Hovered;
      }
    }
  }
  if (bRepaint)
    RepaintRect(m_BoxRect);
}

void CFWL_CheckBox::OnMouseLeave() {
  if (m_bBtnDown)
    m_Properties.m_dwStates |= FWL_STATE_CKB_Hovered;
  else
    m_Properties.m_dwStates &= ~FWL_STATE_CKB_Hovered;

  RepaintRect(m_BoxRect);
}

void CFWL_CheckBox::OnKeyDown(CFWL_MessageKey* pMsg) {
  if (pMsg->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Tab)
    return;
  if (pMsg->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Return ||
      pMsg->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Space) {
    NextStates();
  }
}

}  // namespace pdfium
