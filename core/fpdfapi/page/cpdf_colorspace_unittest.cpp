// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/cpdf_colorspace.h"

#include <stdint.h>
#include <string.h>

#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDF_CalGray, TranslateImageLine) {
  const uint8_t kSrc[12] = {255, 0, 0, 0, 255, 0, 0, 0, 255, 128, 128, 128};
  const uint8_t kExpect[12] = {255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  RetainPtr<CPDF_ColorSpace> pCal = CPDF_ColorSpace::AllocateColorSpace("CalG");
  ASSERT_TRUE(pCal);

  uint8_t dst[12];
  memset(dst, 0xbd, sizeof(dst));
  pCal->TranslateImageLine(dst, kSrc, 4, 4, 1, true);
  for (size_t i = 0; i < 12; ++i)
    EXPECT_EQ(dst[i], kExpect[i]) << " at " << i;

  memset(dst, 0xbd, sizeof(dst));
  pCal->TranslateImageLine(dst, kSrc, 4, 4, 1, false);
  for (size_t i = 0; i < 12; ++i)
    EXPECT_EQ(dst[i], kExpect[i]) << " at " << i;
}

TEST(CPDF_CalRGB, TranslateImageLine) {
  const uint8_t kSrc[12] = {255, 0, 0, 0, 255, 0, 0, 0, 255, 128, 128, 128};
  const uint8_t kExpectMask[12] = {255, 58, 0,   0,   255, 0,
                                   70,  0,  255, 188, 188, 188};
  const uint8_t kExpectNomask[12] = {0,   0, 255, 0,   255, 0,
                                     255, 0, 0,   128, 128, 128};

  RetainPtr<CPDF_ColorSpace> pCal = CPDF_ColorSpace::AllocateColorSpace("CalR");
  ASSERT_TRUE(pCal);

  uint8_t dst[12];
  memset(dst, 0xbd, sizeof(dst));
  pCal->TranslateImageLine(dst, kSrc, 4, 4, 1, true);
  for (size_t i = 0; i < 12; ++i)
    EXPECT_EQ(dst[i], kExpectMask[i]) << " at " << i;

  memset(dst, 0xbd, sizeof(dst));
  pCal->TranslateImageLine(dst, kSrc, 4, 4, 1, false);
  for (size_t i = 0; i < 12; ++i)
    EXPECT_EQ(dst[i], kExpectNomask[i]) << " at " << i;
}
