// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_caret.h"

#include <sstream>
#include <utility>

#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"

CPWL_Caret::CPWL_Caret(
    const CreateParams& cp,
    std::unique_ptr<IPWL_FillerNotify::PerWindowData> pAttachedData)
    : CPWL_Wnd(cp, std::move(pAttachedData)) {}

CPWL_Caret::~CPWL_Caret() = default;

void CPWL_Caret::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                    const CFX_Matrix& mtUser2Device) {
  if (!IsVisible() || !flash_) {
    return;
  }

  CFX_FloatRect rcRect = GetCaretRect();
  CFX_FloatRect rcClip = GetClipRect();

  float fCaretX = rcRect.left + width_ * 0.5f;
  float fCaretTop = rcRect.top;
  float fCaretBottom = rcRect.bottom;
  if (!rcClip.IsEmpty()) {
    rcRect.Intersect(rcClip);
    if (rcRect.IsEmpty()) {
      return;
    }

    fCaretTop = rcRect.top;
    fCaretBottom = rcRect.bottom;
  }

  CFX_Path path;
  path.AppendPoint(CFX_PointF(fCaretX, fCaretBottom),
                   CFX_Path::Point::Type::kMove);
  path.AppendPoint(CFX_PointF(fCaretX, fCaretTop),
                   CFX_Path::Point::Type::kLine);

  CFX_GraphStateData gsd;
  gsd.set_line_width(width_);
  pDevice->DrawPath(path, &mtUser2Device, &gsd, 0, ArgbEncode(255, 0, 0, 0),
                    CFX_FillRenderOptions::EvenOddOptions());
}

void CPWL_Caret::OnTimerFired() {
  flash_ = !flash_;
  InvalidateRect(nullptr);
  // Note, |this| may no longer be viable at this point. If more work needs
  // to be done, add an observer.
}

CFX_FloatRect CPWL_Caret::GetCaretRect() const {
  return CFX_FloatRect(foot_point_.x, foot_point_.y, head_point_.x + width_,
                       head_point_.y);
}

void CPWL_Caret::SetCaret(bool bVisible,
                          const CFX_PointF& ptHead,
                          const CFX_PointF& ptFoot) {
  if (!bVisible) {
    head_point_ = CFX_PointF();
    foot_point_ = CFX_PointF();
    flash_ = false;
    if (!IsVisible()) {
      return;
    }

    timer_.reset();
    (void)CPWL_Wnd::SetVisible(false);
    // Note, |this| may no longer be viable at this point. If more work needs
    // to be done, check the return value of SetVisible().
    return;
  }

  if (!IsVisible()) {
    static constexpr int32_t kCaretFlashIntervalMs = 500;

    head_point_ = ptHead;
    foot_point_ = ptFoot;
    timer_ = std::make_unique<CFX_Timer>(GetTimerHandler(), this,
                                         kCaretFlashIntervalMs);

    if (!CPWL_Wnd::SetVisible(true)) {
      return;
    }

    flash_ = true;
    Move(invalid_rect_, false, true);
    // Note, |this| may no longer be viable at this point. If more work needs
    // to be done, check the return value of Move().
    return;
  }

  if (head_point_ == ptHead && foot_point_ == ptFoot) {
    return;
  }

  head_point_ = ptHead;
  foot_point_ = ptFoot;
  flash_ = true;
  Move(invalid_rect_, false, true);
  // Note, |this| may no longer be viable at this point. If more work
  // needs to be done, check the return value of Move().
}

bool CPWL_Caret::InvalidateRect(const CFX_FloatRect* pRect) {
  if (!pRect) {
    return CPWL_Wnd::InvalidateRect(nullptr);
  }

  CFX_FloatRect rcRefresh = *pRect;
  if (!rcRefresh.IsEmpty()) {
    rcRefresh.Inflate(0.5f, 0.5f);
    rcRefresh.Normalize();
  }
  rcRefresh.top += 1;
  rcRefresh.bottom -= 1;
  return CPWL_Wnd::InvalidateRect(&rcRefresh);
}

bool CPWL_Caret::SetVisible(bool bVisible) {
  return true;
}
