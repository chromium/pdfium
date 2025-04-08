// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_pushbutton.h"

#include "xfa/fde/cfde_textout.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_eventmouse.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

CFWL_PushButton::CFWL_PushButton(CFWL_App* app)
    : CFWL_Widget(app, Properties(), nullptr) {}

CFWL_PushButton::~CFWL_PushButton() = default;

FWL_Type CFWL_PushButton::GetClassID() const {
  return FWL_Type::PushButton;
}

void CFWL_PushButton::SetStates(uint32_t dwStates) {
  if (dwStates & FWL_STATE_WGT_Disabled) {
    properties_.states_ = FWL_STATE_WGT_Disabled;
    return;
  }
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_PushButton::Update() {
  if (IsLocked()) {
    return;
  }

  client_rect_ = GetClientRect();
  caption_rect_ = client_rect_;
}

void CFWL_PushButton::DrawWidget(CFGAS_GEGraphics* pGraphics,
                                 const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);
  }

  DrawBkground(pGraphics, matrix);
}

void CFWL_PushButton::DrawBkground(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackground, this,
                             pGraphics);
  param.states_ = GetPartStates();
  param.matrix_ = matrix;
  param.part_rect_ = client_rect_;
  if (properties_.states_ & FWL_STATE_WGT_Focused) {
    param.data_rect_ = &caption_rect_;
  }
  GetThemeProvider()->DrawBackground(param);
}

Mask<CFWL_PartState> CFWL_PushButton::GetPartStates() {
  Mask<CFWL_PartState> dwStates = CFWL_PartState::kNormal;
  if (properties_.states_ & FWL_STATE_WGT_Focused) {
    dwStates |= CFWL_PartState::kFocused;
  }
  if (properties_.states_ & FWL_STATE_WGT_Disabled) {
    dwStates = CFWL_PartState::kDisabled;
  } else if (properties_.states_ & FWL_STATE_PSB_Pressed) {
    dwStates |= CFWL_PartState::kPressed;
  } else if (properties_.states_ & FWL_STATE_PSB_Hovered) {
    dwStates |= CFWL_PartState::kHovered;
  }
  return dwStates;
}

void CFWL_PushButton::OnProcessMessage(CFWL_Message* pMessage) {
  if (!IsEnabled()) {
    return;
  }

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
      OnFocusGained();
      break;
    case CFWL_Message::Type::kKillFocus:
      OnFocusLost();
      break;
    case CFWL_Message::Type::kMouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->cmd_) {
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kMove:
          OnMouseMove(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeave:
          OnMouseLeave(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::kKey: {
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->cmd_ == CFWL_MessageKey::KeyCommand::kKeyDown) {
        OnKeyDown(pKey);
      }
      break;
    }
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget()) {
    CFWL_Widget::OnProcessMessage(pMessage);
  }
}

void CFWL_PushButton::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_PushButton::OnFocusGained() {
  properties_.states_ |= FWL_STATE_WGT_Focused;
  RepaintRect(client_rect_);
}

void CFWL_PushButton::OnFocusLost() {
  properties_.states_ &= ~FWL_STATE_WGT_Focused;
  RepaintRect(client_rect_);
}

void CFWL_PushButton::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  btn_down_ = true;
  properties_.states_ |= FWL_STATE_PSB_Hovered;
  properties_.states_ |= FWL_STATE_PSB_Pressed;
  RepaintRect(client_rect_);
}

void CFWL_PushButton::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  btn_down_ = false;
  if (client_rect_.Contains(pMsg->pos_)) {
    properties_.states_ &= ~FWL_STATE_PSB_Pressed;
    properties_.states_ |= FWL_STATE_PSB_Hovered;
  } else {
    properties_.states_ &= ~FWL_STATE_PSB_Hovered;
    properties_.states_ &= ~FWL_STATE_PSB_Pressed;
  }
  if (client_rect_.Contains(pMsg->pos_)) {
    CFWL_Event wmClick(CFWL_Event::Type::Click, this);
    DispatchEvent(&wmClick);
  }
  RepaintRect(client_rect_);
}

void CFWL_PushButton::OnMouseMove(CFWL_MessageMouse* pMsg) {
  bool bRepaint = false;
  if (btn_down_) {
    if (client_rect_.Contains(pMsg->pos_)) {
      if ((properties_.states_ & FWL_STATE_PSB_Pressed) == 0) {
        properties_.states_ |= FWL_STATE_PSB_Pressed;
        bRepaint = true;
      }
      if (properties_.states_ & FWL_STATE_PSB_Hovered) {
        properties_.states_ &= ~FWL_STATE_PSB_Hovered;
        bRepaint = true;
      }
    } else {
      if (properties_.states_ & FWL_STATE_PSB_Pressed) {
        properties_.states_ &= ~FWL_STATE_PSB_Pressed;
        bRepaint = true;
      }
      if ((properties_.states_ & FWL_STATE_PSB_Hovered) == 0) {
        properties_.states_ |= FWL_STATE_PSB_Hovered;
        bRepaint = true;
      }
    }
  } else {
    if (!client_rect_.Contains(pMsg->pos_)) {
      return;
    }
    if ((properties_.states_ & FWL_STATE_PSB_Hovered) == 0) {
      properties_.states_ |= FWL_STATE_PSB_Hovered;
      bRepaint = true;
    }
  }
  if (bRepaint) {
    RepaintRect(client_rect_);
  }
}

void CFWL_PushButton::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  btn_down_ = false;
  properties_.states_ &= ~FWL_STATE_PSB_Hovered;
  properties_.states_ &= ~FWL_STATE_PSB_Pressed;
  RepaintRect(client_rect_);
}

void CFWL_PushButton::OnKeyDown(CFWL_MessageKey* pMsg) {
  if (pMsg->key_code_or_char_ != XFA_FWL_VKEY_Return) {
    return;
  }

  CFWL_EventMouse wmMouse(this, nullptr,
                          CFWL_MessageMouse::MouseCommand::kLeftButtonUp);
  DispatchEvent(&wmMouse);
  if (!wmMouse.GetSrcTarget()) {
    return;
  }

  CFWL_Event wmClick(CFWL_Event::Type::Click, this);
  DispatchEvent(&wmClick);
}

}  // namespace pdfium
