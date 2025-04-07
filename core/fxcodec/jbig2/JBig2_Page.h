// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_PAGE_H_
#define CORE_FXCODEC_JBIG2_JBIG2_PAGE_H_

#include <stdint.h>

struct JBig2PageInfo {
  uint32_t width_;
  uint32_t height_;
  uint32_t resolution_x_;
  uint32_t resolution_y_;
  // Page segment flags, bit 2.
  bool default_pixel_value_;
  bool is_striped_;
  uint16_t max_stripe_size_;
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_PAGE_H_
