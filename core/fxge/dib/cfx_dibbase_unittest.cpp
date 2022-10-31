// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_dibbase.h"

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

struct Input {
  CFX_Point src_top_left;
  CFX_Size src_size;
  CFX_Point dest_top_left;
  CFX_Size overlap_size;
};

struct Output {
  CFX_Point src_top_left;
  CFX_Point dest_top_left;
  CFX_Size overlap_size;
};

void RunOverlapRectTest(const CFX_DIBitmap* bitmap,
                        const Input& input,
                        const Output* expected_output) {
  // Initialize in-out parameters.
  int src_left = input.src_top_left.x;
  int src_top = input.src_top_left.y;
  int dest_left = input.dest_top_left.x;
  int dest_top = input.dest_top_left.y;
  int overlap_width = input.overlap_size.width;
  int overlap_height = input.overlap_size.height;

  bool success = bitmap->GetOverlapRect(
      dest_left, dest_top, overlap_width, overlap_height, input.src_size.width,
      input.src_size.height, src_left, src_top,
      /*pClipRgn=*/nullptr);
  if (success == !expected_output) {
    ADD_FAILURE();
    return;
  }

  if (expected_output) {
    EXPECT_EQ(expected_output->src_top_left.x, src_left);
    EXPECT_EQ(expected_output->src_top_left.y, src_top);
    EXPECT_EQ(expected_output->dest_top_left.x, dest_left);
    EXPECT_EQ(expected_output->dest_top_left.y, dest_top);
    EXPECT_EQ(expected_output->overlap_size.width, overlap_width);
    EXPECT_EQ(expected_output->overlap_size.height, overlap_height);
  }
}

}  // namespace

TEST(CFX_DIBBaseTest, GetOverlapRectTrivialOverlap) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_TRUE(bitmap->Create(400, 300, FXDIB_Format::k1bppRgb));

  const Input kInput = {/*src_top_left=*/{0, 0}, /*src_size=*/{400, 300},
                        /*dest_top_left=*/{0, 0},
                        /*overlap_size=*/{400, 300}};
  const Output kExpectedOutput = {/*src_top_left=*/{0, 0},
                                  /*dest_top_left=*/{0, 0},
                                  /*overlap_size=*/{400, 300}};
  RunOverlapRectTest(bitmap.Get(), kInput, &kExpectedOutput);
}

TEST(CFX_DIBBaseTest, GetOverlapRectOverlapNoLimit) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_TRUE(bitmap->Create(400, 300, FXDIB_Format::k1bppRgb));

  const Input kInput = {/*src_top_left=*/{35, 41}, /*src_size=*/{400, 300},
                        /*dest_top_left=*/{123, 137},
                        /*overlap_size=*/{200, 100}};
  const Output kExpectedOutput = {/*src_top_left=*/{35, 41},
                                  /*dest_top_left=*/{123, 137},
                                  /*overlap_size=*/{200, 100}};
  RunOverlapRectTest(bitmap.Get(), kInput, &kExpectedOutput);
}

TEST(CFX_DIBBaseTest, GetOverlapRectOverlapLimitedBySource) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_TRUE(bitmap->Create(400, 300, FXDIB_Format::k1bppRgb));

  const Input kInput = {/*src_top_left=*/{141, 154}, /*src_size=*/{400, 300},
                        /*dest_top_left=*/{35, 41},
                        /*overlap_size=*/{270, 160}};
  const Output kExpectedOutput = {/*src_top_left=*/{141, 154},
                                  /*dest_top_left=*/{35, 41},
                                  /*overlap_size=*/{259, 146}};
  RunOverlapRectTest(bitmap.Get(), kInput, &kExpectedOutput);
}

TEST(CFX_DIBBaseTest, GetOverlapRectOverlapLimitedByDestination) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_TRUE(bitmap->Create(400, 300, FXDIB_Format::k1bppRgb));

  const Input kInput = {/*src_top_left=*/{35, 41}, /*src_size=*/{400, 300},
                        /*dest_top_left=*/{123, 137},
                        /*overlap_size=*/{280, 170}};
  const Output kExpectedOutput = {/*src_top_left=*/{35, 41},
                                  /*dest_top_left=*/{123, 137},
                                  /*overlap_size=*/{277, 163}};
  RunOverlapRectTest(bitmap.Get(), kInput, &kExpectedOutput);
}

TEST(CFX_DIBBaseTest, GetOverlapRectBadInputs) {
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  EXPECT_TRUE(bitmap->Create(400, 300, FXDIB_Format::k1bppRgb));

  const Input kEmptyInputs[] = {
      // Empty source rect.
      {/*src_top_left=*/{0, 0}, /*src_size=*/{0, 0},
       /*dest_top_left=*/{0, 0},
       /*overlap_size=*/{400, 300}},
      // Empty overlap size.
      {/*src_top_left=*/{0, 0}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{0, 0},
       /*overlap_size=*/{0, 0}},
      // Source out of bounds on x-axis.
      {/*src_top_left=*/{-400, 0}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{0, 0},
       /*overlap_size=*/{400, 300}},
  };
  for (const Input& input : kEmptyInputs)
    RunOverlapRectTest(bitmap.Get(), input, /*expected_output=*/nullptr);

  const Input kOutOfBoundInputs[] = {
      // Source out of bounds on x-axis.
      {/*src_top_left=*/{400, 0}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{0, 0},
       /*overlap_size=*/{400, 300}},
      // Source out of bounds on y-axis.
      {/*src_top_left=*/{0, 300}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{0, 0},
       /*overlap_size=*/{400, 300}},
      // Source out of bounds on y-axis.
      {/*src_top_left=*/{0, -300}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{0, 0},
       /*overlap_size=*/{400, 300}},
      // Destination out of bounds on x-axis.
      {/*src_top_left=*/{0, 0}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{-400, 0},
       /*overlap_size=*/{400, 300}},
      // Destination out of bounds on x-axis.
      {/*src_top_left=*/{0, 0}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{400, 0},
       /*overlap_size=*/{400, 300}},
      // Destination out of bounds on y-axis.
      {/*src_top_left=*/{0, 0}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{0, -300},
       /*overlap_size=*/{400, 300}},
      // Destination out of bounds on y-axis.
      {/*src_top_left=*/{0, 0}, /*src_size=*/{400, 300},
       /*dest_top_left=*/{0, 300},
       /*overlap_size=*/{400, 300}},
  };
  for (const Input& input : kOutOfBoundInputs)
    RunOverlapRectTest(bitmap.Get(), input, /*expected_output=*/nullptr);
}
