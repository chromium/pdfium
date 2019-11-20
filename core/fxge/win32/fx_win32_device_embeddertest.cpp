// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/win32/win32_int.h"

#include <windows.h>

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

namespace {

constexpr CFX_Matrix kIdentityMatrix;

}  // namespace

class CFX_WindowsRenderDeviceTest : public testing::Test {
 public:
  void SetUp() override {
    // Get a device context with Windows GDI.
    m_hDC = CreateCompatibleDC(nullptr);
    ASSERT_TRUE(m_hDC);
    CFX_GEModule::Create(nullptr);
    m_driver = pdfium::MakeUnique<CFX_WindowsRenderDevice>(m_hDC, nullptr);
    m_driver->SaveState();
  }

  void TearDown() override {
    m_driver->RestoreState(false);
    m_driver.reset();
    CFX_GEModule::Destroy();
    DeleteDC(m_hDC);
  }

 protected:
  HDC m_hDC;
  std::unique_ptr<CFX_WindowsRenderDevice> m_driver;
};

TEST_F(CFX_WindowsRenderDeviceTest, SimpleClipTriangle) {
  CFX_PathData path_data;
  CFX_PointF p1(0.0f, 0.0f);
  CFX_PointF p2(0.0f, 100.0f);
  CFX_PointF p3(100.0f, 100.0f);

  path_data.AppendLine(p1, p2);
  path_data.AppendLine(p2, p3);
  path_data.AppendLine(p3, p1);
  path_data.ClosePath();
  EXPECT_TRUE(
      m_driver->SetClip_PathFill(&path_data, &kIdentityMatrix, FXFILL_WINDING));
}

TEST_F(CFX_WindowsRenderDeviceTest, SimpleClipRect) {
  CFX_PathData path_data;

  path_data.AppendRect(0.0f, 100.0f, 200.0f, 0.0f);
  path_data.ClosePath();
  EXPECT_TRUE(
      m_driver->SetClip_PathFill(&path_data, &kIdentityMatrix, FXFILL_WINDING));
}

TEST_F(CFX_WindowsRenderDeviceTest, GargantuanClipRect) {
  CFX_PathData path_data;

  path_data.AppendRect(-257698020.0f, -257697252.0f, 257698044.0f,
                       257698812.0f);
  path_data.ClosePath();
  // These coordinates for a clip path are valid, just very large. Using these
  // for a clip path should allow IntersectClipRect() to return success;
  // however they do not because the GDI API IntersectClipRect() errors out and
  // affect subsequent imaging.  crbug.com/1019026
  EXPECT_FALSE(
      m_driver->SetClip_PathFill(&path_data, &kIdentityMatrix, FXFILL_WINDING));
}

TEST_F(CFX_WindowsRenderDeviceTest, GargantuanClipRectWithBaseClip) {
  CFX_PathData path_data;
  const FX_RECT kBaseClip(0, 0, 5100, 6600);

  m_driver->SetBaseClip(kBaseClip);
  path_data.AppendRect(-257698020.0f, -257697252.0f, 257698044.0f,
                       257698812.0f);
  path_data.ClosePath();
  // Use of a reasonable base clip ensures that we avoid getting an error back
  // from GDI API IntersectClipRect().
  EXPECT_TRUE(
      m_driver->SetClip_PathFill(&path_data, &kIdentityMatrix, FXFILL_WINDING));
}
