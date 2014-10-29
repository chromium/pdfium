// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/number_image_generator.h"

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "pdf/draw_utils.h"
#include "pdf/instance.h"

namespace chrome_pdf {

const int kPageNumberSeparator = 0;
const int kPageNumberOriginX = 8;
const int kPageNumberOriginY = 4;

NumberImageGenerator::NumberImageGenerator(Instance* instance)
    : instance_(instance),
      device_scale_(1.0f) {
}

NumberImageGenerator::~NumberImageGenerator() {
}

void NumberImageGenerator::Configure(const pp::ImageData& number_background,
    const std::vector<pp::ImageData>& number_images, float device_scale) {
  number_background_ = number_background;
  number_images_ = number_images;
  device_scale_ = device_scale;
}

void NumberImageGenerator::GenerateImage(
    int page_number, pp::ImageData* image) {
  char buffer[12];
  base::snprintf(buffer, sizeof(buffer), "%u", page_number);
  int extra_width = 0;
  DCHECK(number_images_.size() >= 10);
  DCHECK(!number_background_.is_null());
  for (size_t i = 1; i < strlen(buffer); ++i) {
    int index = buffer[i] - '0';
    extra_width += number_images_[index].size().width();
    extra_width += static_cast<int>(kPageNumberSeparator * device_scale_);
  }

  *image = pp::ImageData(
      instance_,
      PP_IMAGEDATAFORMAT_BGRA_PREMUL,
      pp::Size(number_background_.size().width() + extra_width,
               number_background_.size().height()),
      false);

  int stretch_point = number_background_.size().width() / 2;

  pp::Rect src_rc(0, 0, stretch_point, number_background_.size().height());
  pp::Rect dest_rc(src_rc);
  CopyImage(number_background_, src_rc, image, dest_rc, false);
  src_rc.Offset(number_background_.size().width() - stretch_point, 0);
  dest_rc.Offset(image->size().width() - stretch_point, 0);
  CopyImage(number_background_, src_rc, image, dest_rc, false);
  src_rc = pp::Rect(stretch_point, 0, 1, number_background_.size().height());
  dest_rc = src_rc;
  dest_rc.set_width(extra_width + 1);
  CopyImage(number_background_, src_rc, image, dest_rc, true);

  pp::Point origin(static_cast<int>(kPageNumberOriginX * device_scale_),
                   static_cast<int>(kPageNumberOriginY * device_scale_));
  for (size_t i = 0; i < strlen(buffer); ++i) {
    int index = buffer[i] - '0';
    CopyImage(
        number_images_[index],
        pp::Rect(pp::Point(), number_images_[index].size()),
        image, pp::Rect(origin, number_images_[index].size()), false);
    origin += pp::Point(
        number_images_[index].size().width() +
        static_cast<int>(kPageNumberSeparator * device_scale_), 0);
  }
}

}  // namespace chrome_pdf

