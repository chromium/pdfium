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
  tto_styles_.single_line_ = true;
}

CFWL_CheckBox::~CFWL_CheckBox() = default;

FWL_Type CFWL_CheckBox::GetClassID() const {
  return FWL_Type::CheckBox;
}

void CFWL_CheckBox::SetBoxSize(float fHeight) {
  box_height_ = fHeight;
}

void CFWL_CheckBox::Update() {
  if (IsLocked()) {
    return;
  }

  UpdateTextOutStyles();
  Layout();
}

void CFWL_CheckBox::DrawWidget(CFGAS_GEGraphics* pGraphics,
                               const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);
  }

  Mask<CFWL_PartState> dwStates = GetPartStates();
  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackground, this,
                             pGraphics);
  param.states_ = dwStates;
  param.matrix_ = matrix;
  param.part_rect_ = client_rect_;
  if (properties_.states_ & FWL_STATE_WGT_Focused) {
    param.data_rect_ = &focus_rect_;
  }
  pTheme->DrawBackground(param);

  CFWL_ThemeBackground checkParam(CFWL_ThemePart::Part::kCheckBox, this,
                                  pGraphics);
  checkParam.states_ = dwStates;
  checkParam.matrix_ = matrix;
  checkParam.part_rect_ = box_rect_;
  if (properties_.states_ & FWL_STATE_WGT_Focused) {
    checkParam.data_rect_ = &focus_rect_;
  }
  pTheme->DrawBackground(checkParam);

  CFWL_ThemeText textParam(CFWL_ThemePart::Part::kCaption, this, pGraphics);
  textParam.states_ = dwStates;
  textParam.matrix_ = matrix;
  textParam.part_rect_ = caption_rect_;
  textParam.text_ = WideString::FromASCII("Check box");
  textParam.tto_styles_ = tto_styles_;
  textParam.tto_align_ = tto_align_;
  pTheme->DrawText(textParam);
}

void CFWL_CheckBox::SetCheckState(int32_t iCheck) {
  properties_.states_ &= ~FWL_STATE_CKB_CheckMask;
  switch (iCheck) {
    case 1:
      properties_.states_ |= FWL_STATE_CKB_Checked;
      break;
    case 2:
      if (properties_.style_exts_ & FWL_STYLEEXT_CKB_3State) {
        properties_.states_ |= FWL_STATE_CKB_Neutral;
      }
      break;
    default:
      break;
  }
  RepaintRect(client_rect_);
}

void CFWL_CheckBox::Layout() {
  widget_rect_.width = FXSYS_roundf(widget_rect_.width);
  widget_rect_.height = FXSYS_roundf(widget_rect_.height);
  client_rect_ = GetClientRect();

  float fTextLeft = client_rect_.left + box_height_;
  box_rect_ = CFX_RectF(client_rect_.TopLeft(), box_height_, box_height_);
  caption_rect_ =
      CFX_RectF(fTextLeft, client_rect_.top, client_rect_.right() - fTextLeft,
                client_rect_.height);
  caption_rect_.Inflate(-kCaptionMargin, -kCaptionMargin);

  CFX_RectF rtFocus = caption_rect_;
  CalcTextRect(WideString::FromASCII("Check box"), tto_styles_, tto_align_,
               &rtFocus);
  focus_rect_ = CFX_RectF(caption_rect_.TopLeft(),
                          std::max(caption_rect_.width, rtFocus.width),
                          std::min(caption_rect_.height, rtFocus.height));
  focus_rect_.Inflate(1, 1);
}

Mask<CFWL_PartState> CFWL_CheckBox::GetPartStates() const {
  Mask<CFWL_PartState> dwStates = CFWL_PartState::kNormal;
  if ((properties_.states_ & FWL_STATE_CKB_CheckMask) ==
      FWL_STATE_CKB_Neutral) {
    dwStates = CFWL_PartState::kNeutral;
  } else if ((properties_.states_ & FWL_STATE_CKB_CheckMask) ==
             FWL_STATE_CKB_Checked) {
    dwStates = CFWL_PartState::kChecked;
  }
  if (properties_.states_ & FWL_STATE_WGT_Disabled) {
    dwStates |= CFWL_PartState::kDisabled;
  } else if (properties_.states_ & FWL_STATE_CKB_Hovered) {
    dwStates |= CFWL_PartState::kHovered;
  } else if (properties_.states_ & FWL_STATE_CKB_Pressed) {
    dwStates |= CFWL_PartState::kPressed;
  } else {
    dwStates |= CFWL_PartState::kNormal;
  }
  if (properties_.states_ & FWL_STATE_WGT_Focused) {
    dwStates |= CFWL_PartState::kFocused;
  }
  return dwStates;
}

void CFWL_CheckBox::UpdateTextOutStyles() {
  tto_align_ = FDE_TextAlignment::kTopLeft;
  tto_styles_.Reset();
  tto_styles_.single_line_ = true;
}

void CFWL_CheckBox::NextStates() {
  uint32_t dwFirststate = properties_.states_;
  if (properties_.style_exts_ & FWL_STYLEEXT_CKB_RadioButton) {
    if ((properties_.states_ & FWL_STATE_CKB_CheckMask) ==
        FWL_STATE_CKB_Unchecked) {
      properties_.states_ |= FWL_STATE_CKB_Checked;
    }
  } else {
    if ((properties_.states_ & FWL_STATE_CKB_CheckMask) ==
        FWL_STATE_CKB_Neutral) {
      properties_.states_ &= ~FWL_STATE_CKB_CheckMask;
      if (properties_.style_exts_ & FWL_STYLEEXT_CKB_3State) {
        properties_.states_ |= FWL_STATE_CKB_Checked;
      }
    } else if ((properties_.states_ & FWL_STATE_CKB_CheckMask) ==
               FWL_STATE_CKB_Checked) {
      properties_.states_ &= ~FWL_STATE_CKB_CheckMask;
    } else {
      if (properties_.style_exts_ & FWL_STYLEEXT_CKB_3State) {
        properties_.states_ |= FWL_STATE_CKB_Neutral;
      } else {
        properties_.states_ |= FWL_STATE_CKB_Checked;
      }
    }
  }

  RepaintRect(client_rect_);
  if (dwFirststate == properties_.states_) {
    return;
  }

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
      switch (pMsg->cmd_) {
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

void CFWL_CheckBox::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                 const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_CheckBox::OnFocusGained() {
  properties_.states_ |= FWL_STATE_WGT_Focused;
  RepaintRect(client_rect_);
}

void CFWL_CheckBox::OnFocusLost() {
  properties_.states_ &= ~FWL_STATE_WGT_Focused;
  RepaintRect(client_rect_);
}

void CFWL_CheckBox::OnLButtonDown() {
  if (properties_.states_ & FWL_STATE_WGT_Disabled) {
    return;
  }

  btn_down_ = true;
  properties_.states_ &= ~FWL_STATE_CKB_Hovered;
  properties_.states_ |= FWL_STATE_CKB_Pressed;
  RepaintRect(client_rect_);
}

void CFWL_CheckBox::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (!btn_down_) {
    return;
  }

  btn_down_ = false;
  if (!client_rect_.Contains(pMsg->pos_)) {
    return;
  }

  properties_.states_ |= FWL_STATE_CKB_Hovered;
  properties_.states_ &= ~FWL_STATE_CKB_Pressed;
  NextStates();
}

void CFWL_CheckBox::OnMouseMove(CFWL_MessageMouse* pMsg) {
  if (properties_.states_ & FWL_STATE_WGT_Disabled) {
    return;
  }

  bool bRepaint = false;
  if (btn_down_) {
    if (client_rect_.Contains(pMsg->pos_)) {
      if ((properties_.states_ & FWL_STATE_CKB_Pressed) == 0) {
        bRepaint = true;
        properties_.states_ |= FWL_STATE_CKB_Pressed;
      }
      if ((properties_.states_ & FWL_STATE_CKB_Hovered)) {
        bRepaint = true;
        properties_.states_ &= ~FWL_STATE_CKB_Hovered;
      }
    } else {
      if (properties_.states_ & FWL_STATE_CKB_Pressed) {
        bRepaint = true;
        properties_.states_ &= ~FWL_STATE_CKB_Pressed;
      }
      if ((properties_.states_ & FWL_STATE_CKB_Hovered) == 0) {
        bRepaint = true;
        properties_.states_ |= FWL_STATE_CKB_Hovered;
      }
    }
  } else {
    if (client_rect_.Contains(pMsg->pos_)) {
      if ((properties_.states_ & FWL_STATE_CKB_Hovered) == 0) {
        bRepaint = true;
        properties_.states_ |= FWL_STATE_CKB_Hovered;
      }
    }
  }
  if (bRepaint) {
    RepaintRect(box_rect_);
  }
}

void CFWL_CheckBox::OnMouseLeave() {
  if (btn_down_) {
    properties_.states_ |= FWL_STATE_CKB_Hovered;
  } else {
    properties_.states_ &= ~FWL_STATE_CKB_Hovered;
  }

  RepaintRect(box_rect_);
}

void CFWL_CheckBox::OnKeyDown(CFWL_MessageKey* pMsg) {
  if (pMsg->key_code_or_char_ == XFA_FWL_VKEY_Tab) {
    return;
  }
  if (pMsg->key_code_or_char_ == XFA_FWL_VKEY_Return ||
      pMsg->key_code_or_char_ == XFA_FWL_VKEY_Space) {
    NextStates();
  }
}

}  // namespace pdfium
