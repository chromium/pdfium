// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/skia/fx_skia_device.h"

#include <memory>
#include <set>
#include <utility>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/render/cpdf_pagerendercontext.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/text_char_pos.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_renderpage.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkSize.h"
#include "third_party/skia/include/utils/SkNoDrawCanvas.h"

namespace {

using ::testing::NiceMock;
using ::testing::SizeIs;
using ::testing::WithArg;

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
  charPos[0].m_GlyphIndex = 0;
  charPos[0].m_FontCharWidth = 4;

  CFX_Font font;
  font.LoadSubst("Courier", /*bTrueType=*/true, /*flags=*/0,
                 /*weight=*/400, /*italic_angle=*/0, FX_CodePage::kShiftJIS,
                 /*bVertical=*/false);
  float fontSize = 20;
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
  // Turn off anti-aliasing so that pixels with transitional colors can be
  // avoided.
  static constexpr CFX_TextRenderOptions kTextOptions(
      CFX_TextRenderOptions::kAliasing);
  if (state.m_save == State::Save::kYes)
    driver->SaveState();
  if (state.m_clip != State::Clip::kNo)
    driver->SetClip_PathFill(clipPath, &clipMatrix, CFX_FillRenderOptions());
  if (state.m_graphic == State::Graphic::kPath) {
    driver->DrawPath(path1, &matrix, &graphState, 0xFF112233, 0,
                     {.fill_type = CFX_FillRenderOptions::FillType::kWinding},
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
                     {.fill_type = CFX_FillRenderOptions::FillType::kWinding},
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
  uint32_t pixel = pBitmap->GetPixel(0, 0);
  EXPECT_EQ(state.m_pixel, pixel);
}

void RenderPageToSkCanvas(FPDF_PAGE page,
                          int start_x,
                          int start_y,
                          int size_x,
                          int size_y,
                          SkCanvas* canvas) {
  CPDF_Page* cpdf_page = CPDFPageFromFPDFPage(page);

  auto context = std::make_unique<CPDF_PageRenderContext>();
  CPDF_PageRenderContext* unowned_context = context.get();

  CPDF_Page::RenderContextClearer clearer(cpdf_page);
  cpdf_page->SetRenderContext(std::move(context));

  auto default_device = std::make_unique<CFX_DefaultRenderDevice>();
  default_device->AttachCanvas(canvas);
  unowned_context->m_pDevice = std::move(default_device);

  CPDFSDK_RenderPageWithContext(unowned_context, cpdf_page, start_x, start_y,
                                size_x, size_y, /*rotate=*/0, /*flags=*/0,
                                /*color_scheme=*/nullptr,
                                /*need_to_restore=*/true, /*pause=*/nullptr);
}

class MockCanvas : public SkNoDrawCanvas {
 public:
  MockCanvas(int width, int height) : SkNoDrawCanvas(width, height) {}

  MOCK_METHOD(void,
              onDrawImageRect2,
              (const SkImage*,
               const SkRect&,
               const SkRect&,
               const SkSamplingOptions&,
               const SkPaint*,
               SrcRectConstraint),
              (override));
};

using FxgeSkiaEmbedderTest = EmbedderTest;

}  // namespace

TEST(fxge, SkiaStateEmpty) {
  if (!CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    return;
  }
  Harness(&EmptyTest, {});
}

TEST(fxge, SkiaStatePath) {
  if (!CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    return;
  }
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

TEST(fxge, SkiaStateText) {
  if (!CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    return;
  }

  Harness(&CommonTest,
          {State::Change::kNo, State::Save::kYes, State::Clip::kDifferentMatrix,
           State::Graphic::kText, 0xFF445566});
  Harness(&CommonTest, {State::Change::kNo, State::Save::kYes,
                        State::Clip::kSame, State::Graphic::kText, 0xFF445566});
}

TEST(fxge, SkiaStateOOSClip) {
  if (!CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    return;
  }
  Harness(&OutOfSequenceClipTest, {});
}

TEST_F(FxgeSkiaEmbedderTest, RenderBigImageTwice) {
  static constexpr int kImageWidth = 5100;
  static constexpr int kImageHeight = 6600;

  // Page size that renders 20 image pixels per output pixel. This value evenly
  // divides both the image width and half the image height.
  static constexpr int kPageToImageFactor = 20;
  static constexpr int kPageWidth = kImageWidth / kPageToImageFactor;
  static constexpr int kPageHeight = kImageHeight / kPageToImageFactor;

  if (!CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    GTEST_SKIP() << "Skia is not the default renderer";
  }

  ASSERT_TRUE(OpenDocument("bug_2034.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  std::set<int> image_ids;
  NiceMock<MockCanvas> canvas(kPageWidth, kPageHeight / 2);
  EXPECT_CALL(canvas, onDrawImageRect2)
      .WillRepeatedly(WithArg<0>([&image_ids](const SkImage* image) {
        ASSERT_TRUE(image);
        image_ids.insert(image->uniqueID());

        // TODO(crbug.com/pdfium/2026): Image dimensions should be clipped to
        // 5100x3320. The extra `kPageToImageFactor` accounts for anti-aliasing.
        EXPECT_EQ(SkISize::Make(kImageWidth, kImageHeight), image->dimensions())
            << "Actual image dimensions: " << image->width() << "x"
            << image->height();
      }));

  // Render top half.
  RenderPageToSkCanvas(page, /*start_x=*/0, /*start_y=*/0,
                       /*size_x=*/kPageWidth, /*size_y=*/kPageHeight, &canvas);

  // Render bottom half.
  RenderPageToSkCanvas(page, /*start_x=*/0, /*start_y=*/-kPageHeight / 2,
                       /*size_x=*/kPageWidth, /*size_y=*/kPageHeight, &canvas);

  EXPECT_THAT(image_ids, SizeIs(1));

  UnloadPage(page);
}
