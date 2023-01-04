// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/skia/fx_skia_device.h"

#include <memory>

#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/text_char_pos.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdfview.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"

namespace {

struct State {
  enum class Change { kNo, kYes };
  enum class Save { kNo, kYes };
  enum class Clip { kNo, kSame, kDifferentPath, kDifferentMatrix };
  enum class Graphic { kNone, kPath, kText };

  Change m_change;
  Save m_save;
  Clip m_clip;
  Graphic m_graphic;
  uint32_t m_pixel;
};

void EmptyTest(CFX_SkiaDeviceDriver* driver, const State&) {
  driver->SaveState();
  driver->RestoreState(true);
  driver->RestoreState(false);
}

void CommonTest(CFX_SkiaDeviceDriver* driver, const State& state) {
  TextCharPos charPos[1];
  charPos[0].m_Origin = CFX_PointF(0, 1);
  charPos[0].m_GlyphIndex = 1;
  charPos[0].m_FontCharWidth = 4;

  CFX_Font font;
  float fontSize = 1;
  CFX_Path clipPath;
  CFX_Path clipPath2;
  clipPath.AppendRect(0, 0, 3, 1);
  clipPath2.AppendRect(0, 0, 2, 1);
  CFX_Matrix clipMatrix;
  CFX_Matrix clipMatrix2(1, 0, 0, 1, 0, 1);
  driver->SaveState();
  CFX_Path path1;
  path1.AppendRect(0, 0, 1, 2);

  CFX_Matrix matrix;
  CFX_Matrix matrix2;
  matrix2.Translate(1, 0);
  CFX_GraphStateData graphState;
  static constexpr CFX_TextRenderOptions kTextOptions;
  if (state.m_save == State::Save::kYes)
    driver->SaveState();
  if (state.m_clip != State::Clip::kNo)
    driver->SetClip_PathFill(clipPath, &clipMatrix, CFX_FillRenderOptions());
  if (state.m_graphic == State::Graphic::kPath) {
    driver->DrawPath(path1, &matrix, &graphState, 0xFF112233, 0,
                     CFX_FillRenderOptions::WindingOptions(),
                     BlendMode::kNormal);
  } else if (state.m_graphic == State::Graphic::kText) {
    driver->DrawDeviceText(charPos, &font, matrix, fontSize, 0xFF445566,
                           kTextOptions);
  }
  if (state.m_save == State::Save::kYes)
    driver->RestoreState(true);
  CFX_Path path2;
  path2.AppendRect(0, 0, 2, 2);
  if (state.m_change == State::Change::kYes) {
    if (state.m_graphic == State::Graphic::kPath)
      graphState.m_LineCap = CFX_GraphStateData::LineCap::kRound;
    else if (state.m_graphic == State::Graphic::kText)
      fontSize = 2;
  }
  if (state.m_clip == State::Clip::kSame)
    driver->SetClip_PathFill(clipPath, &clipMatrix, CFX_FillRenderOptions());
  else if (state.m_clip == State::Clip::kDifferentPath)
    driver->SetClip_PathFill(clipPath2, &clipMatrix, CFX_FillRenderOptions());
  else if (state.m_clip == State::Clip::kDifferentMatrix)
    driver->SetClip_PathFill(clipPath, &clipMatrix2, CFX_FillRenderOptions());
  if (state.m_graphic == State::Graphic::kPath) {
    driver->DrawPath(path2, &matrix2, &graphState, 0xFF112233, 0,
                     CFX_FillRenderOptions::WindingOptions(),
                     BlendMode::kNormal);
  } else if (state.m_graphic == State::Graphic::kText) {
    driver->DrawDeviceText(charPos, &font, matrix2, fontSize, 0xFF445566,
                           kTextOptions);
  }
  if (state.m_save == State::Save::kYes)
    driver->RestoreState(false);
  driver->RestoreState(false);
}

void OutOfSequenceClipTest(CFX_SkiaDeviceDriver* driver, const State&) {
  CFX_Path clipPath;
  clipPath.AppendRect(1, 0, 3, 1);
  CFX_Matrix clipMatrix;
  driver->SaveState();
  driver->SetClip_PathFill(clipPath, &clipMatrix, CFX_FillRenderOptions());
  driver->RestoreState(true);
  driver->SaveState();
  driver->SetClip_PathFill(clipPath, &clipMatrix, CFX_FillRenderOptions());
  driver->RestoreState(false);
  driver->RestoreState(false);

  driver->SaveState();
  driver->SaveState();
  driver->SetClip_PathFill(clipPath, &clipMatrix, CFX_FillRenderOptions());
  driver->RestoreState(true);
  driver->SetClip_PathFill(clipPath, &clipMatrix, CFX_FillRenderOptions());
  driver->RestoreState(false);
  driver->RestoreState(false);
}

void Harness(void (*Test)(CFX_SkiaDeviceDriver*, const State&),
             const State& state) {
  constexpr int kWidth = 4;
  constexpr int kHeight = 1;
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(kWidth, kHeight, 1));
  ASSERT_TRUE(bitmap);
  FPDFBitmap_FillRect(bitmap.get(), 0, 0, kWidth, kHeight, 0x00000000);
  RetainPtr<CFX_DIBitmap> pBitmap(CFXDIBitmapFromFPDFBitmap(bitmap.get()));
  auto driver = CFX_SkiaDeviceDriver::Create(pBitmap, false, nullptr, false);
  ASSERT_TRUE(driver);
  (*Test)(driver.get(), state);
  driver->Flush();
  uint32_t pixel = pBitmap->GetPixel(0, 0);
  EXPECT_EQ(state.m_pixel, pixel);
}

}  // namespace

TEST(fxge, SkiaStateEmpty) {
  if (!CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;
  Harness(&EmptyTest, {});
}

TEST(fxge, SkiaStatePath) {
  if (!CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;
  Harness(&CommonTest, {State::Change::kNo, State::Save::kYes,
                        State::Clip::kSame, State::Graphic::kPath, 0xFF112233});
  Harness(&CommonTest,
          {State::Change::kNo, State::Save::kYes, State::Clip::kDifferentPath,
           State::Graphic::kPath, 0xFF112233});
  Harness(&CommonTest, {State::Change::kNo, State::Save::kYes, State::Clip::kNo,
                        State::Graphic::kPath, 0xFF112233});
  Harness(&CommonTest, {State::Change::kYes, State::Save::kNo, State::Clip::kNo,
                        State::Graphic::kPath, 0xFF112233});
  Harness(&CommonTest, {State::Change::kNo, State::Save::kNo, State::Clip::kNo,
                        State::Graphic::kPath, 0xFF112233});
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
TEST(fxge, DISABLED_SkiaStateText) {
  if (!CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;

  Harness(&CommonTest,
          {State::Change::kNo, State::Save::kYes, State::Clip::kDifferentMatrix,
           State::Graphic::kText, 0xFF445566});
  Harness(&CommonTest, {State::Change::kNo, State::Save::kYes,
                        State::Clip::kSame, State::Graphic::kText, 0xFF445566});
}

TEST(fxge, SkiaStateOOSClip) {
  if (!CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;
  Harness(&OutOfSequenceClipTest, {});
}
