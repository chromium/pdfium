// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_cmyk_to_srgb.h"

#include "testing/gtest/include/gtest/gtest.h"

union Float_t {
  Float_t(float num = 0.0f) : f(num) {}

  int32_t i;
  float f;
};

TEST(fxge, CMYK_Rounding) {
  // Testing all floats from 0.0 to 1.0 takes about 35 seconds in release
  // builds and much longer in debug builds, so just test the known-dangerous
  // range.
  constexpr float kStartValue = 0.001f;
  constexpr float kEndValue = 0.003f;
  float R = 0.0f;
  float G = 0.0f;
  float B = 0.0f;
  // Iterate through floats by incrementing the representation, as discussed in
  // https://randomascii.wordpress.com/2012/01/23/stupid-float-tricks-2/
  for (Float_t f = kStartValue; f.f < kEndValue; f.i++) {
    std::tie(R, G, B) = AdobeCMYK_to_sRGB(f.f, f.f, f.f, f.f);
  }
  // Check various other 'special' numbers.
  std::tie(R, G, B) = AdobeCMYK_to_sRGB(0.0f, 0.25f, 0.5f, 1.0f);
}
