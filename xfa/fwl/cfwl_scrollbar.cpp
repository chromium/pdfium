// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_scrollbar.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagemousewheel.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

namespace {

constexpr int kScrollbarElapsedMsecs = 500;
constexpr float kMinThumbSize = 5.0f;

}  // namespace

CFWL_ScrollBar::CFWL_ScrollBar(CFWL_App* app,
                               const Properties& properties,
                               CFWL_Widget* pOuter)
    : CFWL_Widget(app, properties, pOuter) {}

CFWL_ScrollBar::~CFWL_ScrollBar() = default;

FWL_Type CFWL_ScrollBar::GetClassID() const {
  return FWL_Type::ScrollBar;
}

void CFWL_ScrollBar::Update() {
  if (IsLocked()) {
    return;
  }

  Layout();
}

void CFWL_ScrollBar::DrawWidget(CFGAS_GEGraphics* pGraphics,
                                const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);
  }

  DrawLowerTrack(pGraphics, matrix);
  DrawUpperTrack(pGraphics, matrix);
  DrawMinArrowBtn(pGraphics, matrix);
  DrawMaxArrowBtn(pGraphics, matrix);
  DrawThumb(pGraphics, matrix);
}

void CFWL_ScrollBar::SetTrackPos(float fTrackPos) {
  track_pos_ = fTrackPos;
  thumb_rect_ = CalcThumbButtonRect(thumb_rect_);
  min_track_rect_ = CalcMinTrackRect(min_track_rect_);
  max_track_rect_ = CalcMaxTrackRect(max_track_rect_);
}

bool CFWL_ScrollBar::DoScroll(CFWL_EventScroll::Code dwCode, float fPos) {
  if (dwCode == CFWL_EventScroll::Code::None) {
    return false;
  }
  return OnScroll(dwCode, fPos);
}

void CFWL_ScrollBar::DrawUpperTrack(CFGAS_GEGraphics* pGraphics,
                                    const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kUpperTrack, this,
                             pGraphics);
  param.states_ = (properties_.states_ & FWL_STATE_WGT_Disabled)
                      ? CFWL_PartState::kDisabled
                      : max_track_state_;
  param.matrix_ = mtMatrix;
  param.part_rect_ = max_track_rect_;
  GetThemeProvider()->DrawBackground(param);
}

void CFWL_ScrollBar::DrawLowerTrack(CFGAS_GEGraphics* pGraphics,
                                    const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kLowerTrack, this,
                             pGraphics);
  param.states_ = (properties_.states_ & FWL_STATE_WGT_Disabled)
                      ? CFWL_PartState::kDisabled
                      : min_track_state_;
  param.matrix_ = mtMatrix;
  param.part_rect_ = min_track_rect_;
  GetThemeProvider()->DrawBackground(param);
}

void CFWL_ScrollBar::DrawMaxArrowBtn(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackArrow, this, pGraphics);
  param.states_ = (properties_.states_ & FWL_STATE_WGT_Disabled)
                      ? CFWL_PartState::kDisabled
                      : max_button_state_;
  param.matrix_ = mtMatrix;
  param.part_rect_ = max_btn_rect_;
  if (param.part_rect_.height > 0 && param.part_rect_.width > 0) {
    GetThemeProvider()->DrawBackground(param);
  }
}

void CFWL_ScrollBar::DrawMinArrowBtn(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kForeArrow, this, pGraphics);
  param.states_ = (properties_.states_ & FWL_STATE_WGT_Disabled)
                      ? CFWL_PartState::kDisabled
                      : min_button_state_;
  param.matrix_ = mtMatrix;
  param.part_rect_ = min_btn_rect_;
  if (param.part_rect_.height > 0 && param.part_rect_.width > 0) {
    GetThemeProvider()->DrawBackground(param);
  }
}

void CFWL_ScrollBar::DrawThumb(CFGAS_GEGraphics* pGraphics,
                               const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kThumb, this, pGraphics);
  param.states_ = (properties_.states_ & FWL_STATE_WGT_Disabled)
                      ? CFWL_PartState::kDisabled
                      : thumb_button_state_;
  param.matrix_ = mtMatrix;
  param.part_rect_ = thumb_rect_;
  GetThemeProvider()->DrawBackground(param);
}

void CFWL_ScrollBar::Layout() {
  client_rect_ = GetClientRect();

  CalcButtonLen();
  min_btn_rect_ = CalcMinButtonRect();
  max_btn_rect_ = CalcMaxButtonRect();
  thumb_rect_ = CalcThumbButtonRect(thumb_rect_);
  min_track_rect_ = CalcMinTrackRect(min_track_rect_);
  max_track_rect_ = CalcMaxTrackRect(max_track_rect_);
}

void CFWL_ScrollBar::CalcButtonLen() {
  button_len_ = IsVertical() ? client_rect_.width : client_rect_.height;
  float fLength = IsVertical() ? client_rect_.height : client_rect_.width;
  if (fLength < button_len_ * 2) {
    button_len_ = fLength / 2;
    min_size_ = true;
  } else {
    min_size_ = false;
  }
}

CFX_RectF CFWL_ScrollBar::CalcMinButtonRect() {
  if (IsVertical()) {
    return CFX_RectF(client_rect_.TopLeft(), client_rect_.width, button_len_);
  }
  return CFX_RectF(client_rect_.TopLeft(), button_len_, client_rect_.height);
}

CFX_RectF CFWL_ScrollBar::CalcMaxButtonRect() {
  if (IsVertical()) {
    return CFX_RectF(client_rect_.left, client_rect_.bottom() - button_len_,
                     client_rect_.width, button_len_);
  }
  return CFX_RectF(client_rect_.right() - button_len_, client_rect_.top,
                   button_len_, client_rect_.height);
}

CFX_RectF CFWL_ScrollBar::CalcThumbButtonRect(const CFX_RectF& rtThumb) {
  CFX_RectF rect;
  if (!IsEnabled()) {
    return rect;
  }

  if (min_size_) {
    rect.left = rtThumb.left;
    rect.top = rtThumb.top;
    return rect;
  }

  float fRange = range_max_ - range_min_;
  if (fRange < 0) {
    if (IsVertical()) {
      return CFX_RectF(client_rect_.left, max_btn_rect_.bottom(),
                       client_rect_.width, 0);
    }
    return CFX_RectF(max_btn_rect_.right(), client_rect_.top, 0,
                     client_rect_.height);
  }

  CFX_RectF rtClient = client_rect_;
  float fLength = IsVertical() ? rtClient.height : rtClient.width;
  float fSize = button_len_;
  fLength -= fSize * 2.0f;
  if (fLength < fSize) {
    fLength = 0.0f;
  }

  float fThumbSize = fLength * fLength / (fRange + fLength);
  fThumbSize = std::max(fThumbSize, kMinThumbSize);

  float fDiff = std::max(fLength - fThumbSize, 0.0f);
  float fTrackPos = std::clamp(track_pos_, range_min_, range_max_);
  if (!fRange) {
    return rect;
  }

  float iPos = fSize + fDiff * (fTrackPos - range_min_) / fRange;
  rect.left = rtClient.left;
  rect.top = rtClient.top;
  if (IsVertical()) {
    rect.top += iPos;
    rect.width = rtClient.width;
    rect.height = fThumbSize;
  } else {
    rect.left += iPos;
    rect.width = fThumbSize;
    rect.height = rtClient.height;
  }
  return rect;
}

CFX_RectF CFWL_ScrollBar::CalcMinTrackRect(const CFX_RectF& rtMinRect) {
  CFX_RectF rect;
  if (min_size_) {
    rect.left = rtMinRect.left;
    rect.top = rtMinRect.top;
    return rect;
  }

  rect.left = client_rect_.left;
  rect.top = client_rect_.top;
  if (IsVertical()) {
    rect.width = client_rect_.width;
    rect.height = (thumb_rect_.top + thumb_rect_.bottom()) / 2;
  } else {
    rect.width = (thumb_rect_.left + thumb_rect_.right()) / 2;
    rect.height = client_rect_.height;
  }
  return rect;
}

CFX_RectF CFWL_ScrollBar::CalcMaxTrackRect(const CFX_RectF& rtMaxRect) {
  if (min_size_) {
    return CFX_RectF(rtMaxRect.TopLeft(), 0, 0);
  }

  if (IsVertical()) {
    float iy = (thumb_rect_.top + thumb_rect_.bottom()) / 2;
    return CFX_RectF(client_rect_.left, iy, client_rect_.width,
                     client_rect_.bottom() - iy);
  }

  float ix = (thumb_rect_.left + thumb_rect_.right()) / 2;
  return CFX_RectF(ix, client_rect_.top, client_rect_.height - ix,
                   client_rect_.height);
}

float CFWL_ScrollBar::GetTrackPointPos(const CFX_PointF& point) {
  CFX_PointF diff = point - track_point_;
  float fRange = range_max_ - range_min_;
  float fPos;

  if (IsVertical()) {
    fPos = fRange * diff.y /
           (max_btn_rect_.top - min_btn_rect_.bottom() - thumb_rect_.height);
  } else {
    fPos = fRange * diff.x /
           (max_btn_rect_.left - min_btn_rect_.right() - thumb_rect_.width);
  }

  fPos += last_track_pos_;
  return std::clamp(fPos, range_min_, range_max_);
}

bool CFWL_ScrollBar::SendEvent() {
  if (min_button_state_ == CFWL_PartState::kPressed) {
    DoScroll(CFWL_EventScroll::Code::StepBackward, track_pos_);
    return false;
  }
  if (max_button_state_ == CFWL_PartState::kPressed) {
    DoScroll(CFWL_EventScroll::Code::StepForward, track_pos_);
    return false;
  }
  if (min_track_state_ == CFWL_PartState::kPressed) {
    DoScroll(CFWL_EventScroll::Code::PageBackward, track_pos_);
    return thumb_rect_.Contains(track_point_);
  }
  if (max_track_state_ == CFWL_PartState::kPressed) {
    DoScroll(CFWL_EventScroll::Code::PageForward, track_pos_);
    return thumb_rect_.Contains(track_point_);
  }
  if (mouse_wheel_) {
    CFWL_EventScroll::Code dwCode = mouse_wheel_ < 0
                                        ? CFWL_EventScroll::Code::StepForward
                                        : CFWL_EventScroll::Code::StepBackward;
    DoScroll(dwCode, track_pos_);
  }
  return true;
}

bool CFWL_ScrollBar::OnScroll(CFWL_EventScroll::Code dwCode, float fPos) {
  CFWL_EventScroll ev(this, dwCode, fPos);
  DispatchEvent(&ev);
  return true;
}

void CFWL_ScrollBar::OnProcessMessage(CFWL_Message* pMessage) {
  CFWL_Message::Type type = pMessage->GetType();
  if (type == CFWL_Message::Type::kMouse) {
    CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
    switch (pMsg->cmd_) {
      case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
        OnLButtonDown(pMsg->pos_);
        break;
      case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
        OnLButtonUp(pMsg->pos_);
        break;
      case CFWL_MessageMouse::MouseCommand::kMove:
        OnMouseMove(pMsg->pos_);
        break;
      case CFWL_MessageMouse::MouseCommand::kLeave:
        OnMouseLeave();
        break;
      default:
        break;
    }
  } else if (type == CFWL_Message::Type::kMouseWheel) {
    auto* pMsg = static_cast<CFWL_MessageMouseWheel*>(pMessage);
    OnMouseWheel(pMsg->delta());
  }
}

void CFWL_ScrollBar::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                  const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_ScrollBar::OnLButtonDown(const CFX_PointF& point) {
  if (!IsEnabled()) {
    return;
  }

  mouse_down_ = true;
  SetGrab(true);

  track_point_ = point;
  last_track_pos_ = track_pos_;
  if (min_btn_rect_.Contains(point)) {
    DoMouseDown(0, min_btn_rect_, &min_button_state_, point);
  } else if (thumb_rect_.Contains(point)) {
    DoMouseDown(1, thumb_rect_, &thumb_button_state_, point);
  } else if (max_btn_rect_.Contains(point)) {
    DoMouseDown(2, max_btn_rect_, &max_button_state_, point);
  } else if (min_track_rect_.Contains(point)) {
    DoMouseDown(3, min_track_rect_, &min_track_state_, point);
  } else {
    DoMouseDown(4, max_track_rect_, &max_track_state_, point);
  }

  if (!SendEvent()) {
    timer_ = std::make_unique<CFX_Timer>(GetFWLApp()->GetTimerHandler(), this,
                                         kScrollbarElapsedMsecs);
  }
}

void CFWL_ScrollBar::OnLButtonUp(const CFX_PointF& point) {
  timer_.reset();
  mouse_down_ = false;
  DoMouseUp(0, min_btn_rect_, &min_button_state_, point);
  DoMouseUp(1, thumb_rect_, &thumb_button_state_, point);
  DoMouseUp(2, max_btn_rect_, &max_button_state_, point);
  DoMouseUp(3, min_track_rect_, &min_track_state_, point);
  DoMouseUp(4, max_track_rect_, &max_track_state_, point);
  SetGrab(false);
}

void CFWL_ScrollBar::OnMouseMove(const CFX_PointF& point) {
  DoMouseMove(0, min_btn_rect_, &min_button_state_, point);
  DoMouseMove(1, thumb_rect_, &thumb_button_state_, point);
  DoMouseMove(2, max_btn_rect_, &max_button_state_, point);
  DoMouseMove(3, min_track_rect_, &min_track_state_, point);
  DoMouseMove(4, max_track_rect_, &max_track_state_, point);
}

void CFWL_ScrollBar::OnMouseLeave() {
  DoMouseLeave(0, min_btn_rect_, &min_button_state_);
  DoMouseLeave(1, thumb_rect_, &thumb_button_state_);
  DoMouseLeave(2, max_btn_rect_, &max_button_state_);
  DoMouseLeave(3, min_track_rect_, &min_track_state_);
  DoMouseLeave(4, max_track_rect_, &max_track_state_);
}

void CFWL_ScrollBar::OnMouseWheel(const CFX_Vector& delta) {
  mouse_wheel_ = delta.y;
  SendEvent();
  mouse_wheel_ = 0;
}

void CFWL_ScrollBar::DoMouseDown(int32_t iItem,
                                 const CFX_RectF& rtItem,
                                 CFWL_PartState* pState,
                                 const CFX_PointF& point) {
  if (!rtItem.Contains(point)) {
    return;
  }
  if (*pState == CFWL_PartState::kPressed) {
    return;
  }

  *pState = CFWL_PartState::kPressed;
  RepaintRect(rtItem);
}

void CFWL_ScrollBar::DoMouseUp(int32_t iItem,
                               const CFX_RectF& rtItem,
                               CFWL_PartState* pState,
                               const CFX_PointF& point) {
  CFWL_PartState iNewState = rtItem.Contains(point) ? CFWL_PartState::kHovered
                                                    : CFWL_PartState::kNormal;
  if (*pState == iNewState) {
    return;
  }

  *pState = iNewState;
  RepaintRect(rtItem);
  OnScroll(CFWL_EventScroll::Code::EndScroll, track_pos_);
}

void CFWL_ScrollBar::DoMouseMove(int32_t iItem,
                                 const CFX_RectF& rtItem,
                                 CFWL_PartState* pState,
                                 const CFX_PointF& point) {
  if (!mouse_down_) {
    CFWL_PartState iNewState = rtItem.Contains(point) ? CFWL_PartState::kHovered
                                                      : CFWL_PartState::kNormal;
    if (*pState == iNewState) {
      return;
    }

    *pState = iNewState;
    RepaintRect(rtItem);
  } else if ((2 == iItem) &&
             (thumb_button_state_ == CFWL_PartState::kPressed)) {
    track_pos_ = GetTrackPointPos(point);
    OnScroll(CFWL_EventScroll::Code::TrackPos, track_pos_);
  }
}

void CFWL_ScrollBar::DoMouseLeave(int32_t iItem,
                                  const CFX_RectF& rtItem,
                                  CFWL_PartState* pState) {
  if (*pState == CFWL_PartState::kNormal) {
    return;
  }

  *pState = CFWL_PartState::kNormal;
  RepaintRect(rtItem);
}

void CFWL_ScrollBar::DoMouseHover(int32_t iItem,
                                  const CFX_RectF& rtItem,
                                  CFWL_PartState* pState) {
  if (*pState == CFWL_PartState::kHovered) {
    return;
  }

  *pState = CFWL_PartState::kHovered;
  RepaintRect(rtItem);
}

void CFWL_ScrollBar::OnTimerFired() {
  timer_.reset();
  if (!SendEvent()) {
    timer_ =
        std::make_unique<CFX_Timer>(GetFWLApp()->GetTimerHandler(), this, 0);
  }
}

}  // namespace pdfium
