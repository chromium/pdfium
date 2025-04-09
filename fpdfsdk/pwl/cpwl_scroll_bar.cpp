// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_scroll_bar.h"

#include <math.h>

#include <algorithm>
#include <sstream>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"

namespace {

constexpr float kButtonWidth = 9.0f;
constexpr float kPosButtonMinWidth = 2.0f;

}  // namespace

void PWL_FLOATRANGE::Reset() {
  fMin = 0.0f;
  fMax = 0.0f;
}

void PWL_FLOATRANGE::Set(float min, float max) {
  fMin = std::min(min, max);
  fMax = std::max(min, max);
}

bool PWL_FLOATRANGE::In(float x) const {
  return (FXSYS_IsFloatBigger(x, fMin) || FXSYS_IsFloatEqual(x, fMin)) &&
         (FXSYS_IsFloatSmaller(x, fMax) || FXSYS_IsFloatEqual(x, fMax));
}

float PWL_FLOATRANGE::GetWidth() const {
  return fMax - fMin;
}

PWL_SCROLL_PRIVATEDATA::PWL_SCROLL_PRIVATEDATA() {
  Default();
}

void PWL_SCROLL_PRIVATEDATA::Default() {
  ScrollRange.Reset();
  fScrollPos = ScrollRange.fMin;
  fClientWidth = 0;
  fBigStep = 10;
  fSmallStep = 1;
}

void PWL_SCROLL_PRIVATEDATA::SetScrollRange(float min, float max) {
  ScrollRange.Set(min, max);

  if (FXSYS_IsFloatSmaller(fScrollPos, ScrollRange.fMin)) {
    fScrollPos = ScrollRange.fMin;
  }
  if (FXSYS_IsFloatBigger(fScrollPos, ScrollRange.fMax)) {
    fScrollPos = ScrollRange.fMax;
  }
}

void PWL_SCROLL_PRIVATEDATA::SetClientWidth(float width) {
  fClientWidth = width;
}

void PWL_SCROLL_PRIVATEDATA::SetSmallStep(float step) {
  fSmallStep = step;
}

void PWL_SCROLL_PRIVATEDATA::SetBigStep(float step) {
  fBigStep = step;
}

bool PWL_SCROLL_PRIVATEDATA::SetPos(float pos) {
  if (ScrollRange.In(pos)) {
    fScrollPos = pos;
    return true;
  }
  return false;
}

void PWL_SCROLL_PRIVATEDATA::AddSmall() {
  if (!SetPos(fScrollPos + fSmallStep)) {
    SetPos(ScrollRange.fMax);
  }
}

void PWL_SCROLL_PRIVATEDATA::SubSmall() {
  if (!SetPos(fScrollPos - fSmallStep)) {
    SetPos(ScrollRange.fMin);
  }
}

void PWL_SCROLL_PRIVATEDATA::AddBig() {
  if (!SetPos(fScrollPos + fBigStep)) {
    SetPos(ScrollRange.fMax);
  }
}

void PWL_SCROLL_PRIVATEDATA::SubBig() {
  if (!SetPos(fScrollPos - fBigStep)) {
    SetPos(ScrollRange.fMin);
  }
}

CPWL_ScrollBar::CPWL_ScrollBar(
    const CreateParams& cp,
    std::unique_ptr<IPWL_FillerNotify::PerWindowData> pAttachedData)
    : CPWL_Wnd(cp, std::move(pAttachedData)) {
  GetCreationParams()->eCursorType = IPWL_FillerNotify::CursorStyle::kArrow;
}

CPWL_ScrollBar::~CPWL_ScrollBar() = default;

void CPWL_ScrollBar::OnDestroy() {
  // Until cleanup takes place in the virtual destructor for CPWL_Wnd
  // subclasses, implement the virtual OnDestroy method that does the
  // cleanup first, then invokes the superclass OnDestroy ... gee,
  // like a dtor would.
  min_button_.ExtractAsDangling();
  max_button_.ExtractAsDangling();
  pos_button_.ExtractAsDangling();
  CPWL_Wnd::OnDestroy();
}

bool CPWL_ScrollBar::RepositionChildWnd() {
  ObservedPtr<CPWL_ScrollBar> this_observed(this);
  CFX_FloatRect rcClient = this_observed->GetClientRect();
  CFX_FloatRect rcMinButton;
  CFX_FloatRect rcMaxButton;
  if (FXSYS_IsFloatBigger(rcClient.top - rcClient.bottom,
                          kButtonWidth * 2 + kPosButtonMinWidth + 2)) {
    rcMinButton = CFX_FloatRect(rcClient.left, rcClient.top - kButtonWidth,
                                rcClient.right, rcClient.top);
    rcMaxButton = CFX_FloatRect(rcClient.left, rcClient.bottom, rcClient.right,
                                rcClient.bottom + kButtonWidth);
  } else {
    float fBWidth =
        (rcClient.top - rcClient.bottom - kPosButtonMinWidth - 2) / 2;
    if (FXSYS_IsFloatBigger(fBWidth, 0)) {
      rcMinButton = CFX_FloatRect(rcClient.left, rcClient.top - fBWidth,
                                  rcClient.right, rcClient.top);
      rcMaxButton = CFX_FloatRect(rcClient.left, rcClient.bottom,
                                  rcClient.right, rcClient.bottom + fBWidth);
    } else {
      if (!this_observed->SetVisible(false)) {
        return false;
      }
    }
  }
  if (this_observed->min_button_) {
    this_observed->min_button_->Move(rcMinButton, true, false);
    if (!this_observed) {
      return false;
    }
  }
  if (this_observed->max_button_) {
    this_observed->max_button_->Move(rcMaxButton, true, false);
    if (!this_observed) {
      return false;
    }
  }
  return this_observed->MovePosButton(false);
}

void CPWL_ScrollBar::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                        const CFX_Matrix& mtUser2Device) {
  CFX_FloatRect rectWnd = GetWindowRect();

  if (IsVisible() && !rectWnd.IsEmpty()) {
    pDevice->DrawFillRect(&mtUser2Device, rectWnd, GetBackgroundColor(),
                          GetTransparency());

    pDevice->DrawStrokeLine(
        &mtUser2Device, CFX_PointF(rectWnd.left + 2.0f, rectWnd.top - 2.0f),
        CFX_PointF(rectWnd.left + 2.0f, rectWnd.bottom + 2.0f),
        ArgbEncode(GetTransparency(), 100, 100, 100), 1.0f);

    pDevice->DrawStrokeLine(
        &mtUser2Device, CFX_PointF(rectWnd.right - 2.0f, rectWnd.top - 2.0f),
        CFX_PointF(rectWnd.right - 2.0f, rectWnd.bottom + 2.0f),
        ArgbEncode(GetTransparency(), 100, 100, 100), 1.0f);
  }
}

bool CPWL_ScrollBar::OnLButtonDown(Mask<FWL_EVENTFLAG> nFlag,
                                   const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDown(nFlag, point);

  if (HasFlag(PWS_AUTOTRANSPARENT)) {
    if (GetTransparency() != 255) {
      SetTransparency(255);
      if (!InvalidateRect(nullptr)) {
        return true;
      }
    }
  }

  if (pos_button_ && pos_button_->IsVisible()) {
    CFX_FloatRect rcClient = GetClientRect();
    CFX_FloatRect rcPosButton = pos_button_->GetWindowRect();
    CFX_FloatRect rcMinArea =
        CFX_FloatRect(rcClient.left, rcPosButton.top, rcClient.right,
                      rcClient.top - kButtonWidth);
    CFX_FloatRect rcMaxArea =
        CFX_FloatRect(rcClient.left, rcClient.bottom + kButtonWidth,
                      rcClient.right, rcPosButton.bottom);

    rcMinArea.Normalize();
    rcMaxArea.Normalize();

    if (rcMinArea.Contains(point)) {
      private_data_.SubBig();
      if (!MovePosButton(true)) {
        return true;
      }
      NotifyScrollWindow();
    }

    if (rcMaxArea.Contains(point)) {
      private_data_.AddBig();
      if (!MovePosButton(true)) {
        return true;
      }
      NotifyScrollWindow();
    }
  }

  return true;
}

bool CPWL_ScrollBar::OnLButtonUp(Mask<FWL_EVENTFLAG> nFlag,
                                 const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonUp(nFlag, point);

  if (HasFlag(PWS_AUTOTRANSPARENT)) {
    if (GetTransparency() != kTransparency) {
      SetTransparency(kTransparency);
      if (!InvalidateRect(nullptr)) {
        return true;
      }
    }
  }

  timer_.reset();
  mouse_down_ = false;
  return true;
}

void CPWL_ScrollBar::SetScrollInfo(const PWL_SCROLL_INFO& info) {
  if (info == origin_info_) {
    return;
  }

  origin_info_ = info;
  float fMax =
      std::max(0.0f, info.fContentMax - info.fContentMin - info.fPlateWidth);
  SetScrollRange(0, fMax, info.fPlateWidth);
  SetScrollStep(info.fBigStep, info.fSmallStep);
}

void CPWL_ScrollBar::SetScrollPosition(float pos) {
  pos = origin_info_.fContentMax - pos;
  SetScrollPos(pos);
}

void CPWL_ScrollBar::NotifyLButtonDown(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == min_button_) {
    OnMinButtonLBDown(pos);
  } else if (child == max_button_) {
    OnMaxButtonLBDown(pos);
  } else if (child == pos_button_) {
    OnPosButtonLBDown(pos);
  }
}

void CPWL_ScrollBar::NotifyLButtonUp(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == min_button_) {
    OnMinButtonLBUp(pos);
  } else if (child == max_button_) {
    OnMaxButtonLBUp(pos);
  } else if (child == pos_button_) {
    OnPosButtonLBUp(pos);
  }
}

void CPWL_ScrollBar::NotifyMouseMove(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == min_button_) {
    OnMinButtonMouseMove(pos);
  } else if (child == max_button_) {
    OnMaxButtonMouseMove(pos);
  } else if (child == pos_button_) {
    OnPosButtonMouseMove(pos);
  }
}

void CPWL_ScrollBar::CreateButtons(const CreateParams& cp) {
  ObservedPtr<CPWL_ScrollBar> this_observed(this);

  CreateParams scp = cp;
  scp.dwBorderWidth = 2;
  scp.nBorderStyle = BorderStyle::kBeveled;
  scp.dwFlags = PWS_VISIBLE | PWS_BORDER | PWS_BACKGROUND | PWS_NOREFRESHCLIP;

  if (!this_observed->min_button_) {
    auto pButton =
        std::make_unique<CPWL_SBButton>(scp, this_observed->CloneAttachedData(),
                                        CPWL_SBButton::Type::kMinButton);
    this_observed->min_button_ = pButton.get();
    this_observed->AddChild(std::move(pButton));
    this_observed->min_button_->Realize();
  }
  if (!this_observed->max_button_) {
    auto pButton =
        std::make_unique<CPWL_SBButton>(scp, this_observed->CloneAttachedData(),
                                        CPWL_SBButton::Type::kMaxButton);
    this_observed->max_button_ = pButton.get();
    this_observed->AddChild(std::move(pButton));
    this_observed->max_button_->Realize();
  }
  if (!this_observed->pos_button_) {
    auto pButton =
        std::make_unique<CPWL_SBButton>(scp, this_observed->CloneAttachedData(),
                                        CPWL_SBButton::Type::kPosButton);
    this_observed->pos_button_ = pButton.get();
    if (this_observed->pos_button_->SetVisible(false) && this_observed) {
      this_observed->AddChild(std::move(pButton));
      this_observed->pos_button_->Realize();
    }
  }
}

float CPWL_ScrollBar::GetScrollBarWidth() const {
  return IsVisible() ? kWidth : 0.0f;
}

void CPWL_ScrollBar::SetScrollRange(float fMin,
                                    float fMax,
                                    float fClientWidth) {
  ObservedPtr<CPWL_ScrollBar> this_observed(this);
  if (!this_observed->pos_button_) {
    return;
  }
  this_observed->private_data_.SetScrollRange(fMin, fMax);
  this_observed->private_data_.SetClientWidth(fClientWidth);

  if (FXSYS_IsFloatSmaller(this_observed->private_data_.ScrollRange.GetWidth(),
                           0.0f)) {
    (void)this_observed->pos_button_->SetVisible(false);
    // Note, |this| may no longer be viable at this point. If more work needs
    // to be done, check this_observed.
    return;
  }

  if (!this_observed->pos_button_->SetVisible(true) || !this_observed) {
    return;
  }

  (void)this_observed->MovePosButton(true);
  // Note, |this| may no longer be viable at this point. If more work needs
  // to be done, check the return value of MovePosButton().
}

void CPWL_ScrollBar::SetScrollPos(float fPos) {
  float fOldPos = private_data_.fScrollPos;
  private_data_.SetPos(fPos);
  if (!FXSYS_IsFloatEqual(private_data_.fScrollPos, fOldPos)) {
    (void)MovePosButton(true);
    // Note, |this| may no longer be viable at this point. If more work needs
    // to be done, check the return value of MovePosButton().
  }
}

void CPWL_ScrollBar::SetScrollStep(float fBigStep, float fSmallStep) {
  private_data_.SetBigStep(fBigStep);
  private_data_.SetSmallStep(fSmallStep);
}

bool CPWL_ScrollBar::MovePosButton(bool bRefresh) {
  ObservedPtr<CPWL_ScrollBar> this_observed(this);

  DCHECK(min_button_);
  DCHECK(max_button_);

  if (this_observed->pos_button_->IsVisible()) {
    CFX_FloatRect rcPosArea = this_observed->GetScrollArea();
    float fTop = this_observed->TrueToFace(private_data_.fScrollPos);
    float fBottom = this_observed->TrueToFace(private_data_.fScrollPos +
                                              private_data_.fClientWidth);
    if (FXSYS_IsFloatSmaller(fTop - fBottom, kPosButtonMinWidth)) {
      fBottom = fTop - kPosButtonMinWidth;
    }
    if (FXSYS_IsFloatSmaller(fBottom, rcPosArea.bottom)) {
      fBottom = rcPosArea.bottom;
      fTop = fBottom + kPosButtonMinWidth;
    }

    CFX_FloatRect rcPosButton =
        CFX_FloatRect(rcPosArea.left, fBottom, rcPosArea.right, fTop);

    this_observed->pos_button_->Move(rcPosButton, true, bRefresh);
    if (!this_observed) {
      return false;
    }
  }
  return true;
}

void CPWL_ScrollBar::OnMinButtonLBDown(const CFX_PointF& point) {
  private_data_.SubSmall();
  if (!MovePosButton(true)) {
    return;
  }

  NotifyScrollWindow();
  min_or_max_ = true;
  timer_ = std::make_unique<CFX_Timer>(GetTimerHandler(), this, 100);
}

void CPWL_ScrollBar::OnMinButtonLBUp(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnMinButtonMouseMove(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnMaxButtonLBDown(const CFX_PointF& point) {
  private_data_.AddSmall();
  if (!MovePosButton(true)) {
    return;
  }

  NotifyScrollWindow();
  min_or_max_ = false;
  timer_ = std::make_unique<CFX_Timer>(GetTimerHandler(), this, 100);
}

void CPWL_ScrollBar::OnMaxButtonLBUp(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnMaxButtonMouseMove(const CFX_PointF& point) {}

void CPWL_ScrollBar::OnPosButtonLBDown(const CFX_PointF& point) {
  mouse_down_ = true;

  if (pos_button_) {
    CFX_FloatRect rcPosButton = pos_button_->GetWindowRect();
    old_pos_ = point.y;
    old_pos_button_ = rcPosButton.top;
  }
}

void CPWL_ScrollBar::OnPosButtonLBUp(const CFX_PointF& point) {
  mouse_down_ = false;
}

void CPWL_ScrollBar::OnPosButtonMouseMove(const CFX_PointF& point) {
  if (fabs(point.y - old_pos_) < 1) {
    return;
  }

  float fOldScrollPos = private_data_.fScrollPos;
  float fNewPos = FaceToTrue(old_pos_button_ + point.y - old_pos_);
  if (mouse_down_) {
    if (FXSYS_IsFloatSmaller(fNewPos, private_data_.ScrollRange.fMin)) {
      fNewPos = private_data_.ScrollRange.fMin;
    }

    if (FXSYS_IsFloatBigger(fNewPos, private_data_.ScrollRange.fMax)) {
      fNewPos = private_data_.ScrollRange.fMax;
    }

    private_data_.SetPos(fNewPos);

    if (!FXSYS_IsFloatEqual(fOldScrollPos, private_data_.fScrollPos)) {
      if (!MovePosButton(true)) {
        return;
      }

      NotifyScrollWindow();
    }
  }
}

void CPWL_ScrollBar::NotifyScrollWindow() {
  CPWL_Wnd* pParent = GetParentWindow();
  if (!pParent) {
    return;
  }

  pParent->ScrollWindowVertically(origin_info_.fContentMax -
                                  private_data_.fScrollPos);
}

CFX_FloatRect CPWL_ScrollBar::GetScrollArea() const {
  CFX_FloatRect rcClient = GetClientRect();
  if (!min_button_ || !max_button_) {
    return rcClient;
  }

  CFX_FloatRect rcMin = min_button_->GetWindowRect();
  CFX_FloatRect rcMax = max_button_->GetWindowRect();
  float fMinHeight = rcMin.Height();
  float fMaxHeight = rcMax.Height();

  CFX_FloatRect rcArea;
  if (rcClient.top - rcClient.bottom > fMinHeight + fMaxHeight + 2) {
    rcArea = CFX_FloatRect(rcClient.left, rcClient.bottom + fMinHeight + 1,
                           rcClient.right, rcClient.top - fMaxHeight - 1);
  } else {
    rcArea = CFX_FloatRect(rcClient.left, rcClient.bottom + fMinHeight + 1,
                           rcClient.right, rcClient.bottom + fMinHeight + 1);
  }

  rcArea.Normalize();
  return rcArea;
}

float CPWL_ScrollBar::TrueToFace(float fTrue) {
  CFX_FloatRect rcPosArea = GetScrollArea();
  float fFactWidth =
      private_data_.ScrollRange.GetWidth() + private_data_.fClientWidth;
  fFactWidth = fFactWidth == 0 ? 1 : fFactWidth;
  return rcPosArea.top -
         fTrue * (rcPosArea.top - rcPosArea.bottom) / fFactWidth;
}

float CPWL_ScrollBar::FaceToTrue(float fFace) {
  CFX_FloatRect rcPosArea = GetScrollArea();
  float fFactWidth =
      private_data_.ScrollRange.GetWidth() + private_data_.fClientWidth;
  fFactWidth = fFactWidth == 0 ? 1 : fFactWidth;
  return (rcPosArea.top - fFace) * fFactWidth /
         (rcPosArea.top - rcPosArea.bottom);
}

void CPWL_ScrollBar::CreateChildWnd(const CreateParams& cp) {
  CreateButtons(cp);
}

void CPWL_ScrollBar::OnTimerFired() {
  PWL_SCROLL_PRIVATEDATA sTemp = private_data_;
  if (min_or_max_) {
    private_data_.SubSmall();
  } else {
    private_data_.AddSmall();
  }

  if (sTemp == private_data_) {
    return;
  }

  if (!MovePosButton(true)) {
    return;
  }

  NotifyScrollWindow();
}
