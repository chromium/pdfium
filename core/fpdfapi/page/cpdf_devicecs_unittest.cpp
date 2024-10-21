// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_devicecs.h"

#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDFDeviceCSTest, GetRGBFromGray) {
  auto device_gray =
      pdfium::MakeRetain<CPDF_DeviceCS>(CPDF_ColorSpace::Family::kDeviceGray);

  // Test normal values. For gray, only first value from buf should be used.
  float buf[3] = {0.43f, 0.11f, 0.34f};
  auto maybe_rgb = device_gray->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.43f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.43f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.43f, maybe_rgb.value().blue);

  buf[0] = 0.872f;
  maybe_rgb = device_gray->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.872f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.872f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.872f, maybe_rgb.value().blue);

  // Test boundary values
  buf[0] = {0.0f};
  maybe_rgb = device_gray->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().blue);

  buf[0] = 1.0f;
  maybe_rgb = device_gray->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().blue);

  // Test out of range values
  buf[0] = -0.01f;
  maybe_rgb = device_gray->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().blue);

  buf[0] = 12.5f;
  maybe_rgb = device_gray->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().blue);
}

TEST(CPDFDeviceCSTest, GetRGBFromRGB) {
  auto device_rgb =
      pdfium::MakeRetain<CPDF_DeviceCS>(CPDF_ColorSpace::Family::kDeviceRGB);

  // Test normal values
  float buf[3] = {0.13f, 1.0f, 0.652f};
  auto maybe_rgb = device_rgb->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.13f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.652f, maybe_rgb.value().blue);

  buf[0] = 0.0f;
  buf[1] = 0.52f;
  buf[2] = 0.78f;
  maybe_rgb = device_rgb->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.52f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.78f, maybe_rgb.value().blue);

  // Test out of range values
  buf[0] = -10.5f;
  buf[1] = 100.0f;
  maybe_rgb = device_rgb->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.0f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(1.0f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.78f, maybe_rgb.value().blue);
}

TEST(CPDFDeviceCSTest, GetRGBFromCMYK) {
  auto device_cmyk =
      pdfium::MakeRetain<CPDF_DeviceCS>(CPDF_ColorSpace::Family::kDeviceCMYK);

  // Test normal values
  float buf[4] = {0.6f, 0.5f, 0.3f, 0.9f};
  auto maybe_rgb = device_cmyk->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.0627451f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.0627451f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.10588236f, maybe_rgb.value().blue);

  buf[0] = 0.15f;
  buf[2] = 0.0f;
  maybe_rgb = device_cmyk->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.2f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.0862745f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.16470589f, maybe_rgb.value().blue);

  buf[2] = 1.0f;
  buf[3] = 0.0f;
  maybe_rgb = device_cmyk->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.85098046f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.552941f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.15686275f, maybe_rgb.value().blue);

  // Test out of range values
  buf[2] = 1.5f;
  buf[3] = -0.6f;
  maybe_rgb = device_cmyk->GetRGB(buf);
  ASSERT_TRUE(maybe_rgb.has_value());
  EXPECT_FLOAT_EQ(0.85098046f, maybe_rgb.value().red);
  EXPECT_FLOAT_EQ(0.552941f, maybe_rgb.value().green);
  EXPECT_FLOAT_EQ(0.15686275f, maybe_rgb.value().blue);
}
