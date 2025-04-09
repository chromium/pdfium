// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_windowsrenderdevice.h"

#include <windows.h>

#include <memory>

#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/win32/cfx_psfonttracker.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

constexpr CFX_Matrix kIdentityMatrix;

}  // namespace

class CFXWindowsRenderDeviceTest : public EmbedderTest {
 public:
  void SetUp() override {
    EmbedderTest::SetUp();

    // Get a device context with Windows GDI.
    dc_handle_ = CreateCompatibleDC(nullptr);
    ASSERT_TRUE(dc_handle_);
    driver_ = std::make_unique<CFX_WindowsRenderDevice>(
        dc_handle_, &psfont_tracker_, /*encoder_iface=*/nullptr);
    driver_->SaveState();
  }

  void TearDown() override {
    driver_->RestoreState(false);
    driver_.reset();
    DeleteDC(dc_handle_);
    EmbedderTest::TearDown();
  }

 protected:
  HDC dc_handle_;
  CFX_PSFontTracker psfont_tracker_;
  std::unique_ptr<CFX_WindowsRenderDevice> driver_;
};

TEST_F(CFXWindowsRenderDeviceTest, SimpleClipTriangle) {
  CFX_Path path_data;
  CFX_PointF p1(0.0f, 0.0f);
  CFX_PointF p2(0.0f, 100.0f);
  CFX_PointF p3(100.0f, 100.0f);

  path_data.AppendLine(p1, p2);
  path_data.AppendLine(p2, p3);
  path_data.AppendLine(p3, p1);
  path_data.ClosePath();
  EXPECT_TRUE(driver_->SetClip_PathFill(
      path_data, &kIdentityMatrix, CFX_FillRenderOptions::WindingOptions()));
}

TEST_F(CFXWindowsRenderDeviceTest, SimpleClipRect) {
  CFX_Path path_data;

  path_data.AppendRect(0.0f, 100.0f, 200.0f, 0.0f);
  path_data.ClosePath();
  EXPECT_TRUE(driver_->SetClip_PathFill(
      path_data, &kIdentityMatrix, CFX_FillRenderOptions::WindingOptions()));
}

TEST_F(CFXWindowsRenderDeviceTest, GargantuanClipRect) {
  CFX_Path path_data;

  path_data.AppendRect(-257698020.0f, -257697252.0f, 257698044.0f,
                       257698812.0f);
  path_data.ClosePath();
  // These coordinates for a clip path are valid, just very large. Using these
  // for a clip path should allow IntersectClipRect() to return success;
  // however they do not because the GDI API IntersectClipRect() errors out and
  // affect subsequent imaging.  crbug.com/1019026
  EXPECT_FALSE(driver_->SetClip_PathFill(
      path_data, &kIdentityMatrix, CFX_FillRenderOptions::WindingOptions()));
}

TEST_F(CFXWindowsRenderDeviceTest, GargantuanClipRectWithBaseClip) {
  CFX_Path path_data;
  const FX_RECT kBaseClip(0, 0, 5100, 6600);

  driver_->SetBaseClip(kBaseClip);
  path_data.AppendRect(-257698020.0f, -257697252.0f, 257698044.0f,
                       257698812.0f);
  path_data.ClosePath();
  // Use of a reasonable base clip ensures that we avoid getting an error back
  // from GDI API IntersectClipRect().
  EXPECT_TRUE(driver_->SetClip_PathFill(
      path_data, &kIdentityMatrix, CFX_FillRenderOptions::WindingOptions()));
}
