// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PAGE_INDICATOR_H_
#define PDF_PAGE_INDICATOR_H_

#include <string>
#include <vector>

#include "pdf/control.h"
#include "pdf/fading_control.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/point.h"
#include "ppapi/cpp/rect.h"

namespace chrome_pdf {

class NumberImageGenerator;

const uint32 kPageIndicatorScrollFadeTimeoutMs = 240;
const uint32 kPageIndicatorInitialFadeTimeoutMs = 960;
const uint32 kPageIndicatorSplashTimeoutMs = 2000;

class PageIndicator : public FadingControl {
 public:
  PageIndicator();
  virtual ~PageIndicator();
  virtual bool CreatePageIndicator(
      uint32 id,
      bool visible,
      Control::Owner* delegate,
      NumberImageGenerator* number_image_generator,
      bool always_visible);

  void Configure(const pp::Point& origin, const pp::ImageData& background);

  int current_page() const { return current_page_; }
  void set_current_page(int current_page);

  virtual void Splash();
  void Splash(uint32 splash_timeout, uint32 page_timeout);

  // Returns the y position where the page indicator should be drawn given the
  // position of the scrollbar and the total document height and the plugin
  // height.
  int GetYPosition(
      int vertical_scrollbar_y, int document_height, int plugin_height);

  // Control interface.
  virtual void Paint(pp::ImageData* image_data, const pp::Rect& rc);
  virtual void OnTimerFired(uint32 timer_id);

  // FadingControl interface.
  virtual void OnFadeInComplete();

 private:
  void ResetFadeOutTimer();

  int current_page_;
  pp::ImageData background_;
  NumberImageGenerator* number_image_generator_;
  uint32 fade_out_timer_id_;
  uint32 splash_timeout_;
  uint32 fade_timeout_;

  bool always_visible_;
};

}  // namespace chrome_pdf

#endif  // PDF_PAGE_INDICATOR_H_
