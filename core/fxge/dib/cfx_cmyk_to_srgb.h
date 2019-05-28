// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_CMYK_TO_SRGB_H_
#define CORE_FXGE_DIB_CFX_CMYK_TO_SRGB_H_

#include <stdint.h>

#include <tuple>

namespace fxge {

std::tuple<float, float, float> AdobeCMYK_to_sRGB(float c,
                                                  float m,
                                                  float y,
                                                  float k);
std::tuple<uint8_t, uint8_t, uint8_t> AdobeCMYK_to_sRGB1(uint8_t c,
                                                         uint8_t m,
                                                         uint8_t y,
                                                         uint8_t k);

}  // namespace fxge

using fxge::AdobeCMYK_to_sRGB;
using fxge::AdobeCMYK_to_sRGB1;

#endif  // CORE_FXGE_DIB_CFX_CMYK_TO_SRGB_H_
