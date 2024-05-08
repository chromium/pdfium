// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(UNSAFE_BUFFERS_BUILD)
// TODO(crbug.com/pdfium/2153): resolve buffer safety issues.
#pragma allow_unsafe_buffers
#endif

#include "core/fpdfapi/page/cpdf_colorspace.h"

#include <stdint.h>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDF_CalGray, TranslateImageLine) {
  const uint8_t kSrc[12] = {255, 0, 0, 0, 255, 0, 0, 0, 255, 128, 128, 128};
  const uint8_t kExpect[12] = {255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  RetainPtr<CPDF_ColorSpace> pCal = CPDF_ColorSpace::AllocateColorSpace("CalG");
  ASSERT_TRUE(pCal);

  uint8_t dst[12];
  fxcrt::Fill(dst, 0xbd);
  // `bTransMask` only applies to CYMK colorspaces.
  pCal->TranslateImageLine(dst, kSrc, 4, 4, 1, /*bTransMask=*/false);
  for (size_t i = 0; i < 12; ++i)
    EXPECT_EQ(dst[i], kExpect[i]) << " at " << i;
}

TEST(CPDF_CalRGB, TranslateImageLine) {
  const uint8_t kSrc[12] = {255, 0, 0, 0, 255, 0, 0, 0, 255, 128, 128, 128};
  const uint8_t kExpectNomask[12] = {0,   0, 255, 0,   255, 0,
                                     255, 0, 0,   128, 128, 128};

  RetainPtr<CPDF_ColorSpace> pCal = CPDF_ColorSpace::AllocateColorSpace("CalR");
  ASSERT_TRUE(pCal);

  uint8_t dst[12];
  fxcrt::Fill(dst, 0xbd);
  // `bTransMask` only applies to CYMK colorspaces.
  pCal->TranslateImageLine(dst, kSrc, 4, 4, 1, /*bTransMask=*/false);
  for (size_t i = 0; i < 12; ++i)
    EXPECT_EQ(dst[i], kExpectNomask[i]) << " at " << i;
}
