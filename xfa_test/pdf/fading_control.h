// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_FADING_CONTROL_H_
#define PDF_FADING_CONTROL_H_

#include "pdf/control.h"

namespace chrome_pdf {

class FadingControl : public Control {
 public:
  FadingControl();
  virtual ~FadingControl();

  virtual void OnTimerFired(uint32 timer_id);

  // Fade In/Out control depending on visible flag over the time of time_ms.
  virtual void Fade(bool visible, uint32 time_ms);

  virtual void OnFadeInComplete() {}
  virtual void OnFadeOutComplete() {}

 private:
  int alpha_shift_;
  uint32 timer_id_;
};

}  // namespace chrome_pdf

#endif  // PDF_FADING_CONTROL_H_
