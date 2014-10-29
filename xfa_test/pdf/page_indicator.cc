// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/page_indicator.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "pdf/draw_utils.h"
#include "pdf/number_image_generator.h"
#include "pdf/resource_consts.h"

namespace chrome_pdf {


PageIndicator::PageIndicator()
    : current_page_(0),
      fade_out_timer_id_(0),
      splash_timeout_(kPageIndicatorSplashTimeoutMs),
      fade_timeout_(kPageIndicatorScrollFadeTimeoutMs),
      always_visible_(false) {
}

PageIndicator::~PageIndicator() {
}

bool PageIndicator::CreatePageIndicator(
    uint32 id,
    bool visible,
    Control::Owner* delegate,
    NumberImageGenerator* number_image_generator,
    bool always_visible) {
  number_image_generator_ = number_image_generator;
  always_visible_ = always_visible;

  pp::Rect rc;
  bool res = Control::Create(id, rc, visible, delegate);
  return res;
}

void PageIndicator::Configure(const pp::Point& origin,
                              const pp::ImageData& background) {
  background_ = background;
  pp::Rect rc(origin, background_.size());
  Control::SetRect(rc, false);
}

void PageIndicator::set_current_page(int current_page) {
  if (current_page_ < 0)
    return;

  current_page_ = current_page;
}

void PageIndicator::Paint(pp::ImageData* image_data, const pp::Rect& rc) {
  if (!visible())
    return;

  pp::Rect draw_rc = rc.Intersect(rect());
  if (draw_rc.IsEmpty())
    return;

  // Copying the background image to a temporary buffer.
  pp::ImageData buffer(owner()->GetInstance(), background_.format(),
                       background_.size(), false);
  CopyImage(background_, pp::Rect(background_.size()),
            &buffer, pp::Rect(background_.size()), false);

  // Creating the page number image.
  pp::ImageData page_number_image;
  number_image_generator_->GenerateImage(current_page_, &page_number_image);

  pp::Point origin2(
      (buffer.size().width() - page_number_image.size().width()) / 2.5,
      (buffer.size().height() - page_number_image.size().height()) / 2);

  // Drawing page number image on the buffer.
  if (origin2.x() > 0 && origin2.y() > 0) {
    CopyImage(page_number_image,
              pp::Rect(pp::Point(), page_number_image.size()),
              &buffer,
              pp::Rect(origin2, page_number_image.size()),
              false);
  }

  // Drawing the buffer.
  pp::Point origin = draw_rc.point();
  draw_rc.Offset(-rect().x(), -rect().y());
  AlphaBlend(buffer, draw_rc, image_data, origin, transparency());
}

void PageIndicator::OnTimerFired(uint32 timer_id) {
  FadingControl::OnTimerFired(timer_id);
  if (timer_id == fade_out_timer_id_) {
    Fade(false, fade_timeout_);
  }
}

void PageIndicator::ResetFadeOutTimer() {
  fade_out_timer_id_ =
      owner()->ScheduleTimer(id(), splash_timeout_);
}

void PageIndicator::OnFadeInComplete() {
  if (!always_visible_)
    ResetFadeOutTimer();
}

void PageIndicator::Splash() {
  Splash(kPageIndicatorSplashTimeoutMs, kPageIndicatorScrollFadeTimeoutMs);
}

void PageIndicator::Splash(uint32 splash_timeout, uint32 fade_timeout) {
  splash_timeout_ = splash_timeout;
  fade_timeout_ = fade_timeout;
  if (!always_visible_)
    fade_out_timer_id_ = 0;
  Fade(true, fade_timeout_);
}

int PageIndicator::GetYPosition(
    int vertical_scrollbar_y, int document_height, int plugin_height) {
  double percent = static_cast<double>(vertical_scrollbar_y) / document_height;
  return (plugin_height - rect().height()) * percent;
}

}  // namespace chrome_pdf
