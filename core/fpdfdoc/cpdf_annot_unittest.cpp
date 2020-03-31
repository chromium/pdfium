// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_annot.h"

#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

RetainPtr<CPDF_Array> CreateQuadPointArrayFromVector(
    const std::vector<int>& points) {
  auto array = pdfium::MakeRetain<CPDF_Array>();
  for (float point : points)
    array->AppendNew<CPDF_Number>(point);
  return array;
}

}  // namespace

TEST(CPDFAnnotTest, RectFromQuadPointsArray) {
  RetainPtr<CPDF_Array> array = CreateQuadPointArrayFromVector(
      {0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1});
  CFX_FloatRect rect = CPDF_Annot::RectFromQuadPointsArray(array.Get(), 0);
  EXPECT_EQ(4.0f, rect.left);
  EXPECT_EQ(5.0f, rect.bottom);
  EXPECT_EQ(2.0f, rect.right);
  EXPECT_EQ(3.0f, rect.top);

  rect = CPDF_Annot::RectFromQuadPointsArray(array.Get(), 1);
  EXPECT_EQ(4.0f, rect.left);
  EXPECT_EQ(3.0f, rect.bottom);
  EXPECT_EQ(6.0f, rect.right);
  EXPECT_EQ(5.0f, rect.top);
}

TEST(CPDFAnnotTest, BoundingRectFromQuadPoints) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  CFX_FloatRect rect = CPDF_Annot::BoundingRectFromQuadPoints(dict.Get());
  EXPECT_EQ(0.0f, rect.left);
  EXPECT_EQ(0.0f, rect.bottom);
  EXPECT_EQ(0.0f, rect.right);
  EXPECT_EQ(0.0f, rect.top);

  dict->SetFor("QuadPoints", CreateQuadPointArrayFromVector({0, 1, 2}));
  rect = CPDF_Annot::BoundingRectFromQuadPoints(dict.Get());
  EXPECT_EQ(0.0f, rect.left);
  EXPECT_EQ(0.0f, rect.bottom);
  EXPECT_EQ(0.0f, rect.right);
  EXPECT_EQ(0.0f, rect.top);

  dict->SetFor("QuadPoints",
               CreateQuadPointArrayFromVector({0, 1, 2, 3, 4, 5, 6, 7}));
  rect = CPDF_Annot::BoundingRectFromQuadPoints(dict.Get());
  EXPECT_EQ(4.0f, rect.left);
  EXPECT_EQ(5.0f, rect.bottom);
  EXPECT_EQ(2.0f, rect.right);
  EXPECT_EQ(3.0f, rect.top);

  dict->SetFor("QuadPoints", CreateQuadPointArrayFromVector(
                                 {0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5,
                                  4, 3, 2, 1, 9, 2, 5, 7, 3, 6, 4, 1}));
  rect = CPDF_Annot::BoundingRectFromQuadPoints(dict.Get());
  EXPECT_EQ(2.0f, rect.left);
  EXPECT_EQ(3.0f, rect.bottom);
  EXPECT_EQ(6.0f, rect.right);
  EXPECT_EQ(7.0f, rect.top);
}

TEST(CPDFAnnotTest, RectFromQuadPoints) {
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(dict.Get(), 0);
  EXPECT_EQ(0.0f, rect.left);
  EXPECT_EQ(0.0f, rect.bottom);
  EXPECT_EQ(0.0f, rect.right);
  EXPECT_EQ(0.0f, rect.top);
  rect = CPDF_Annot::RectFromQuadPoints(dict.Get(), 5);
  EXPECT_EQ(0.0f, rect.left);
  EXPECT_EQ(0.0f, rect.bottom);
  EXPECT_EQ(0.0f, rect.right);
  EXPECT_EQ(0.0f, rect.top);

  dict->SetFor("QuadPoints",
               CreateQuadPointArrayFromVector({0, 1, 2, 3, 4, 5, 6, 7}));
  rect = CPDF_Annot::RectFromQuadPoints(dict.Get(), 0);
  EXPECT_EQ(4.0f, rect.left);
  EXPECT_EQ(5.0f, rect.bottom);
  EXPECT_EQ(2.0f, rect.right);
  EXPECT_EQ(3.0f, rect.top);
  rect = CPDF_Annot::RectFromQuadPoints(dict.Get(), 5);
  EXPECT_EQ(0.0f, rect.left);
  EXPECT_EQ(0.0f, rect.bottom);
  EXPECT_EQ(0.0f, rect.right);
  EXPECT_EQ(0.0f, rect.top);

  dict->SetFor("QuadPoints", CreateQuadPointArrayFromVector(
                                 {0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5,
                                  4, 3, 2, 1, 9, 2, 5, 7, 3, 6, 4, 1}));
  rect = CPDF_Annot::RectFromQuadPoints(dict.Get(), 0);
  EXPECT_EQ(4.0f, rect.left);
  EXPECT_EQ(5.0f, rect.bottom);
  EXPECT_EQ(2.0f, rect.right);
  EXPECT_EQ(3.0f, rect.top);
  rect = CPDF_Annot::RectFromQuadPoints(dict.Get(), 1);
  EXPECT_EQ(4.0f, rect.left);
  EXPECT_EQ(3.0f, rect.bottom);
  EXPECT_EQ(6.0f, rect.right);
  EXPECT_EQ(5.0f, rect.top);
  rect = CPDF_Annot::RectFromQuadPoints(dict.Get(), 2);
  EXPECT_EQ(3.0f, rect.left);
  EXPECT_EQ(6.0f, rect.bottom);
  EXPECT_EQ(5.0f, rect.right);
  EXPECT_EQ(7.0f, rect.top);
}

TEST(CPDFAnnotTest, QuadPointCount) {
  RetainPtr<CPDF_Array> array = CreateQuadPointArrayFromVector({});
  EXPECT_EQ(0u, CPDF_Annot::QuadPointCount(array.Get()));

  for (int i = 0; i < 7; ++i) {
    array->AppendNew<CPDF_Number>(0);
    EXPECT_EQ(0u, CPDF_Annot::QuadPointCount(array.Get()));
  }
  for (int i = 0; i < 8; ++i) {
    array->AppendNew<CPDF_Number>(0);
    EXPECT_EQ(1u, CPDF_Annot::QuadPointCount(array.Get()));
  }
  for (int i = 0; i < 50; ++i)
    array->AppendNew<CPDF_Number>(0);
  EXPECT_EQ(8u, CPDF_Annot::QuadPointCount(array.Get()));
}
