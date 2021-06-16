// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cstretchengine.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_dib.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxge/dib/fx_dib.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

uint32_t PixelWeightSum(const CStretchEngine::PixelWeight* weights) {
  uint32_t sum = 0;
  for (int i = weights->m_SrcStart; i <= weights->m_SrcEnd; ++i) {
    sum += weights->GetWeightForPosition(i);
  }
  return sum;
}

void ExecuteOneStretchTest(uint32_t dest_width,
                           uint32_t src_width,
                           const FXDIB_ResampleOptions& options) {
  constexpr uint32_t kExpectedSum = CStretchEngine::kFixedPointOne;
  CStretchEngine::WeightTable table;
  table.CalculateWeights(dest_width, 0, dest_width, src_width, 0, src_width,
                         options);
  for (uint32_t i = 0; i < dest_width; ++i) {
    EXPECT_EQ(kExpectedSum, PixelWeightSum(table.GetPixelWeight(i)))
        << "for { " << src_width << ", " << dest_width << " } at " << i;
  }
}

void ExecuteStretchTests(const FXDIB_ResampleOptions& options) {
  // Can't test everything, few random values chosen.
  constexpr uint32_t kDestWidths[] = {1, 2, 337, 512, 808, 2550};
  constexpr uint32_t kSrcWidths[] = {1, 2, 187, 256, 809, 1110};
  for (uint32_t src_width : kSrcWidths) {
    for (uint32_t dest_width : kDestWidths) {
      ExecuteOneStretchTest(dest_width, src_width, options);
    }
  }
}

}  // namespace

TEST(CStretchEngine, OverflowInCtor) {
  FX_RECT clip_rect;
  RetainPtr<CPDF_Dictionary> dict_obj = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_obj->SetNewFor<CPDF_Number>("Width", 71000);
  dict_obj->SetNewFor<CPDF_Number>("Height", 12500);
  RetainPtr<CPDF_Stream> stream =
      pdfium::MakeRetain<CPDF_Stream>(nullptr, 0, std::move(dict_obj));
  auto dib_source = pdfium::MakeRetain<CPDF_DIB>();
  dib_source->Load(nullptr, stream.Get());
  CStretchEngine engine(nullptr, FXDIB_Format::k8bppRgb, 500, 500, clip_rect,
                        dib_source, FXDIB_ResampleOptions());
  EXPECT_TRUE(engine.GetResampleOptionsForTest().bInterpolateBilinear);
  EXPECT_FALSE(engine.GetResampleOptionsForTest().bHalftone);
  EXPECT_FALSE(engine.GetResampleOptionsForTest().bNoSmoothing);
  EXPECT_FALSE(engine.GetResampleOptionsForTest().bLossy);
}

// See https://crbug.com/pdfium/1688
TEST(CStretchEngine, DISABLED_WeightRounding) {
  FXDIB_ResampleOptions options;
  ExecuteStretchTests(options);
}

TEST(CStretchEngine, WeightRoundingNoSmoothing) {
  FXDIB_ResampleOptions options;
  options.bNoSmoothing = true;
  ExecuteStretchTests(options);
}

// See https://crbug.com/pdfium/1688
TEST(CStretchEngine, DISABLED_WeightRoundingBilinear) {
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  ExecuteStretchTests(options);
}

TEST(CStretchEngine, WeightRoundingNoSmoothingBilinear) {
  FXDIB_ResampleOptions options;
  options.bNoSmoothing = true;
  options.bInterpolateBilinear = true;
  ExecuteStretchTests(options);
}
