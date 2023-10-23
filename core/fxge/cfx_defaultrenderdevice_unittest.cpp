// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_defaultrenderdevice.h"

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_DefaultRenderDeviceTest, GetClipBox_Default) {
  CFX_DefaultRenderDevice device;
  ASSERT_TRUE(device.Create(/*width=*/16, /*height=*/16, FXDIB_Format::kArgb,
                            /*pBackdropBitmap=*/nullptr));

  EXPECT_EQ(FX_RECT(0, 0, 16, 16), device.GetClipBox());
}

TEST(CFX_DefaultRenderDeviceTest, GetClipBox_PathFill) {
  // Matrix that transposes and translates by 1 unit on each axis.
  const CFX_Matrix object_to_device(0, 1, 1, 0, 1, -1);

  CFX_DefaultRenderDevice device;
  ASSERT_TRUE(device.Create(/*width=*/16, /*height=*/16, FXDIB_Format::kArgb,
                            /*pBackdropBitmap=*/nullptr));

  CFX_Path path;
  path.AppendRect(2, 4, 14, 12);
  EXPECT_TRUE(device.SetClip_PathFill(
      path, &object_to_device,
      {.fill_type = CFX_FillRenderOptions::FillType::kEvenOdd}));

  EXPECT_EQ(FX_RECT(5, 1, 13, 13), device.GetClipBox());
}

TEST(CFX_DefaultRenderDeviceTest, GetClipBox_PathStroke) {
  // Matrix that transposes and translates by 1 unit on each axis.
  const CFX_Matrix object_to_device(0, 1, 1, 0, 1, -1);

  // Default line width is 1.
  const CFX_GraphStateData graphics_state;

  CFX_DefaultRenderDevice device;
  ASSERT_TRUE(device.Create(/*width=*/16, /*height=*/16, FXDIB_Format::kArgb,
                            /*pBackdropBitmap=*/nullptr));

  CFX_Path path;
  path.AppendRect(2, 4, 14, 12);
  EXPECT_TRUE(
      device.SetClip_PathStroke(path, &object_to_device, &graphics_state));

  EXPECT_EQ(FX_RECT(4, 0, 14, 14), device.GetClipBox());
}

TEST(CFX_DefaultRenderDeviceTest, GetClipBox_Rect) {
  CFX_DefaultRenderDevice device;
  ASSERT_TRUE(device.Create(/*width=*/16, /*height=*/16, FXDIB_Format::kArgb,
                            /*pBackdropBitmap=*/nullptr));

  EXPECT_TRUE(device.SetClip_Rect({2, 4, 14, 12}));

  EXPECT_EQ(FX_RECT(2, 4, 14, 12), device.GetClipBox());
}

TEST(CFX_DefaultRenderDeviceTest, GetClipBox_Empty) {
  CFX_DefaultRenderDevice device;
  ASSERT_TRUE(device.Create(/*width=*/16, /*height=*/16, FXDIB_Format::kArgb,
                            /*pBackdropBitmap=*/nullptr));

  EXPECT_TRUE(device.SetClip_Rect({2, 8, 14, 8}));

  EXPECT_TRUE(device.GetClipBox().IsEmpty());
}
