// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/fading_control.h"

#include <math.h>

#include "base/logging.h"
#include "pdf/draw_utils.h"
#include "pdf/resource_consts.h"

namespace chrome_pdf {

FadingControl::FadingControl()
    : alpha_shift_(0), timer_id_(0) {
}

FadingControl::~FadingControl() {
}

void FadingControl::OnTimerFired(uint32 timer_id) {
  if (timer_id == timer_id_) {
    int32 new_alpha = transparency() + alpha_shift_;
    if (new_alpha <= kTransparentAlpha) {
      Show(false, true);
      OnFadeOutComplete();
      return;
    }
    if (new_alpha >= kOpaqueAlpha) {
      AdjustTransparency(kOpaqueAlpha, true);
      OnFadeInComplete();
      return;
    }

    AdjustTransparency(static_cast<uint8>(new_alpha), true);
    timer_id_ = owner()->ScheduleTimer(id(), kFadingTimeoutMs);
  }
}

// Fade In/Out control depending on visible flag over the time of time_ms.
void FadingControl::Fade(bool show, uint32 time_ms) {
  DCHECK(time_ms != 0);
  // Check if we already in the same state.
  if (!visible() && !show)
    return;
  if (!visible() && show) {
    Show(show, false);
    AdjustTransparency(kTransparentAlpha, false);
    OnFadeOutComplete();
  }
  if (transparency() == kOpaqueAlpha && show) {
    OnFadeInComplete();
    return;
  }

  int delta = show ? kOpaqueAlpha - transparency() : transparency();
  double shift =
      static_cast<double>(delta) * kFadingTimeoutMs / time_ms;
  if (shift > delta)
    alpha_shift_ = delta;
  else
    alpha_shift_ = static_cast<int>(ceil(shift));

  if (alpha_shift_ == 0)
    alpha_shift_ = 1;

  // If disabling, make alpha shift negative.
  if (!show)
    alpha_shift_ = -alpha_shift_;

  timer_id_ = owner()->ScheduleTimer(id(), kFadingTimeoutMs);
}

}  // namespace chrome_pdf
