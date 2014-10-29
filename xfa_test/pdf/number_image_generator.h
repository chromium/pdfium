// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_NUMBER_IMAGE_GENERATOR_H
#define PDF_NUMBER_IMAGE_GENERATOR_H

#include <vector>

#include "ppapi/cpp/image_data.h"

namespace chrome_pdf {

class Instance;

class NumberImageGenerator {
 public:
  explicit NumberImageGenerator(Instance* instance);
  virtual ~NumberImageGenerator();

  void Configure(const pp::ImageData& number_background,
                 const std::vector<pp::ImageData>& number_images,
                 float device_scale);

  void GenerateImage(int page_number, pp::ImageData* image);

 private:
  Instance* instance_;
  pp::ImageData number_background_;
  std::vector<pp::ImageData> number_images_;
  float device_scale_;
};

}  // namespace chrome_pdf

#endif  // PDF_NUMBER_IMAGE_GENERATOR_H

