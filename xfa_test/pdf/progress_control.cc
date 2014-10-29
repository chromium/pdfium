// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/progress_control.h"

#include <algorithm>

#include "base/logging.h"
#include "pdf/draw_utils.h"
#include "pdf/resource_consts.h"
#include "ppapi/cpp/dev/font_dev.h"

namespace chrome_pdf {

const double ProgressControl::kCompleted = 100.0;

// There is a bug outputting text with alpha 0xFF (opaque) to an intermediate
// image. It outputs alpha channgel of the text pixels to 0xFF (transparent).
// And it breaks next alpha blending.
// For now, let's use alpha 0xFE to work around this bug.
// TODO(gene): investigate this bug.
const uint32 kProgressTextColor = 0xFEDDE6FC;
const uint32 kProgressTextSize = 16;
const uint32 kImageTextSpacing = 8;
const uint32 kTopPadding = 8;
const uint32 kBottomPadding = 12;
const uint32 kLeftPadding = 10;
const uint32 kRightPadding = 10;

int ScaleInt(int val, float scale) {
  return static_cast<int>(val * scale);
}

ProgressControl::ProgressControl()
    : progress_(0.0),
      device_scale_(1.0) {
}

ProgressControl::~ProgressControl() {
}

bool ProgressControl::CreateProgressControl(
    uint32 id,
    bool visible,
    Control::Owner* delegate,
    double progress,
    float device_scale,
    const std::vector<pp::ImageData>& images,
    const pp::ImageData& background,
    const std::string& text) {
  progress_ = progress;
  text_ = text;
  bool res = Control::Create(id, pp::Rect(), visible, delegate);
  if (res)
    Reconfigure(background, images, device_scale);
  return res;
}

void ProgressControl::Reconfigure(const pp::ImageData& background,
                                  const std::vector<pp::ImageData>& images,
                                  float device_scale) {
  DCHECK(images.size() != 0);
  images_ = images;
  background_ = background;
  device_scale_ = device_scale;
  pp::Size ctrl_size;
  CalculateLayout(owner()->GetInstance(), images_, background_, text_,
      device_scale_, &ctrl_size, &image_rc_, &text_rc_);
  pp::Rect rc(pp::Point(), ctrl_size);
  Control::SetRect(rc, false);
  PrepareBackground();
}

// static
void ProgressControl::CalculateLayout(pp::Instance* instance,
                                      const std::vector<pp::ImageData>& images,
                                      const pp::ImageData& background,
                                      const std::string& text,
                                      float device_scale,
                                      pp::Size* ctrl_size,
                                      pp::Rect* image_rc,
                                      pp::Rect* text_rc) {
  DCHECK(images.size() != 0);
  int image_width = 0;
  int image_height = 0;
  for (size_t i = 0; i < images.size(); i++) {
    image_width = std::max(image_width, images[i].size().width());
    image_height = std::max(image_height, images[i].size().height());
  }

  pp::FontDescription_Dev description;
  description.set_family(PP_FONTFAMILY_SANSSERIF);
  description.set_size(ScaleInt(kProgressTextSize, device_scale));
  description.set_weight(PP_FONTWEIGHT_BOLD);
  pp::Font_Dev font(instance, description);
  int text_length = font.MeasureSimpleText(text);

  pp::FontDescription_Dev desc;
  PP_FontMetrics_Dev metrics;
  font.Describe(&desc, &metrics);
  int text_height = metrics.height;

  *ctrl_size = pp::Size(
      image_width + text_length +
      ScaleInt(kImageTextSpacing + kLeftPadding + kRightPadding, device_scale),
      std::max(image_height, text_height) +
      ScaleInt(kTopPadding + kBottomPadding, device_scale));

  int offset_x = 0;
  int offset_y = 0;
  if (ctrl_size->width() < background.size().width()) {
    offset_x += (background.size().width() - ctrl_size->width()) / 2;
    ctrl_size->set_width(background.size().width());
  }
  if (ctrl_size->height() < background.size().height()) {
    offset_y += (background.size().height() - ctrl_size->height()) / 2;
    ctrl_size->set_height(background.size().height());
  }

  *image_rc = pp::Rect(ScaleInt(kLeftPadding, device_scale) + offset_x,
                       ScaleInt(kTopPadding, device_scale) + offset_y,
                       image_width,
                       image_height);

  *text_rc = pp::Rect(
      ctrl_size->width() - text_length -
      ScaleInt(kRightPadding, device_scale) - offset_x,
      (ctrl_size->height() - text_height) / 2,
      text_length,
      text_height);
}

size_t ProgressControl::GetImageIngex() const {
  return static_cast<size_t>((progress_ / 100.0) * images_.size());
}

void ProgressControl::Paint(pp::ImageData* image_data, const pp::Rect& rc) {
  if (!visible())
    return;

  pp::Rect draw_rc = rect().Intersect(rc);
  if (draw_rc.IsEmpty())
    return;

  pp::ImageData buffer(owner()->GetInstance(), ctrl_background_.format(),
                       ctrl_background_.size(), false);
  CopyImage(ctrl_background_, pp::Rect(ctrl_background_.size()),
            &buffer, pp::Rect(ctrl_background_.size()), false);

  size_t index = GetImageIngex();
  if (index >= images_.size())
    index = images_.size() - 1;

  AlphaBlend(images_[index],
             pp::Rect(images_[index].size()),
             &buffer,
             image_rc_.point(),
             kOpaqueAlpha);

  pp::Rect image_draw_rc(draw_rc);
  image_draw_rc.Offset(-rect().x(), -rect().y());
  AlphaBlend(buffer,
             image_draw_rc,
             image_data,
             draw_rc.point(),
             transparency());
}

void ProgressControl::SetProgress(double progress) {
  size_t old_index = GetImageIngex();
  progress_ = progress;
  size_t new_index = GetImageIngex();
  if (progress_ >= kCompleted) {
    progress_ = kCompleted;
    owner()->OnEvent(id(), EVENT_ID_PROGRESS_COMPLETED, NULL);
  }
  if (visible() && old_index != new_index)
    owner()->Invalidate(id(), rect());
}

void ProgressControl::PrepareBackground() {
  AdjustBackground();

  pp::FontDescription_Dev description;
  description.set_family(PP_FONTFAMILY_SANSSERIF);
  description.set_size(ScaleInt(kProgressTextSize,  device_scale_));
  description.set_weight(PP_FONTWEIGHT_BOLD);
  pp::Font_Dev font(owner()->GetInstance(), description);

  pp::FontDescription_Dev desc;
  PP_FontMetrics_Dev metrics;
  font.Describe(&desc, &metrics);

  pp::Point text_origin = pp::Point(text_rc_.x(),
      (text_rc_.y() + text_rc_.bottom() + metrics.x_height) / 2);
  font.DrawTextAt(&ctrl_background_, pp::TextRun_Dev(text_), text_origin,
      kProgressTextColor, pp::Rect(ctrl_background_.size()), false);
}

void ProgressControl::AdjustBackground() {
  ctrl_background_ = pp::ImageData(owner()->GetInstance(),
                                   PP_IMAGEDATAFORMAT_BGRA_PREMUL,
                                   rect().size(),
                                   false);

  if (rect().size() == background_.size()) {
    CopyImage(background_, pp::Rect(background_.size()),
        &ctrl_background_, pp::Rect(ctrl_background_.size()), false);
    return;
  }

  // We need to stretch background to new dimentions. To do so, we split
  // background into 9 different parts. We copy corner rects (1,3,7,9) as is,
  // stretch rectangles between corners (2,4,6,8) in 1 dimention, and
  // stretch center rect (5) in 2 dimentions.
  //    |---|---|---|
  //    | 1 | 2 | 3 |
  //    |---|---|---|
  //    | 4 | 5 | 6 |
  //    |---|---|---|
  //    | 7 | 8 | 9 |
  //    |---|---|---|
  int slice_x = background_.size().width() / 3;
  int slice_y = background_.size().height() / 3;

  // Copy rect 1
  pp::Rect src_rc(0, 0, slice_x, slice_y);
  pp::Rect dest_rc(0, 0, slice_x, slice_y);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, false);

  // Copy rect 3
  src_rc.set_x(background_.size().width() - slice_x);
  dest_rc.set_x(ctrl_background_.size().width() - slice_x);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, false);

  // Copy rect 9
  src_rc.set_y(background_.size().height() - slice_y);
  dest_rc.set_y(ctrl_background_.size().height() - slice_y);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, false);

  // Copy rect 7
  src_rc.set_x(0);
  dest_rc.set_x(0);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, false);

  // Stretch rect 2
  src_rc = pp::Rect(
      slice_x, 0, background_.size().width() - 2 * slice_x, slice_y);
  dest_rc = pp::Rect(
      slice_x, 0, ctrl_background_.size().width() - 2 * slice_x, slice_y);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, true);

  // Copy rect 8
  src_rc.set_y(background_.size().height() - slice_y);
  dest_rc.set_y(ctrl_background_.size().height() - slice_y);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, true);

  // Stretch rect 4
  src_rc = pp::Rect(
      0, slice_y, slice_x, background_.size().height() - 2 * slice_y);
  dest_rc = pp::Rect(
      0, slice_y, slice_x, ctrl_background_.size().height() - 2 * slice_y);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, true);

  // Copy rect 6
  src_rc.set_x(background_.size().width() - slice_x);
  dest_rc.set_x(ctrl_background_.size().width() - slice_x);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, true);

  // Stretch rect 5
  src_rc = pp::Rect(slice_x,
                    slice_y,
                    background_.size().width() - 2 * slice_x,
                    background_.size().height() - 2 * slice_y);
  dest_rc = pp::Rect(slice_x,
                     slice_y,
                     ctrl_background_.size().width() - 2 * slice_x,
                     ctrl_background_.size().height() - 2 * slice_y);
  CopyImage(background_, src_rc, &ctrl_background_, dest_rc, true);
}

}  // namespace chrome_pdf
