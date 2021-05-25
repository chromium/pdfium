// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_pathdata.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_PathData, BasicTest) {
  CFX_PathData path;
  path.AppendRect(/*left=*/1, /*bottom=*/2, /*right=*/3, /*top=*/5);
  EXPECT_EQ(5u, path.GetPoints().size());
  EXPECT_TRUE(path.IsRect());
  Optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), rect.value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), path.GetBoundingBox());

  const CFX_Matrix kScaleMatrix(1, 0, 0, 2, 60, 70);
  rect = path.GetRect(&kScaleMatrix);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(61, 74, 63, 80), rect.value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), path.GetBoundingBox());

  path.Clear();
  EXPECT_EQ(0u, path.GetPoints().size());
  EXPECT_FALSE(path.IsRect());
  EXPECT_EQ(CFX_FloatRect(), path.GetBoundingBox());

  // 4 points without a closed path makes a rect.
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({1, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({1, 0}, FXPT_TYPE::LineTo);
  EXPECT_EQ(4u, path.GetPoints().size());
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), path.GetBoundingBox());

  // 4 points with a closed path also makes a rect.
  path.ClosePath();
  EXPECT_EQ(4u, path.GetPoints().size());
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), path.GetBoundingBox());

  path.Transform(kScaleMatrix);
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(60, 70, 61, 72), rect.value());
  EXPECT_EQ(CFX_FloatRect(60, 70, 61, 72), path.GetBoundingBox());

  path.Clear();
  path.AppendFloatRect({1, 2, 3, 5});
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), rect.value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), path.GetBoundingBox());
}

TEST(CFX_PathData, ShearTransform) {
  CFX_PathData path;
  path.AppendRect(/*left=*/1, /*bottom=*/2, /*right=*/3, /*top=*/5);

  const CFX_Matrix kShearMatrix(1, 2, 0, 1, 0, 0);
  EXPECT_TRUE(path.IsRect());
  Optional<CFX_FloatRect> rect = path.GetRect(&kShearMatrix);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), path.GetBoundingBox());

  path.Transform(kShearMatrix);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(1, 4, 3, 11), path.GetBoundingBox());

  const CFX_Matrix shear_inverse_matrix = kShearMatrix.GetInverse();
  rect = path.GetRect(&shear_inverse_matrix);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), rect.value());
  EXPECT_EQ(CFX_FloatRect(1, 4, 3, 11), path.GetBoundingBox());

  path.Transform(shear_inverse_matrix);
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), rect.value());
  EXPECT_EQ(CFX_FloatRect(1, 2, 3, 5), path.GetBoundingBox());
}

TEST(CFX_PathData, Hexagon) {
  CFX_PathData path;
  path.AppendPoint({1, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({3, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 2}, FXPT_TYPE::LineTo);
  path.AppendPoint({1, 2}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  ASSERT_EQ(6u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(5));
  EXPECT_FALSE(path.IsClosingFigure(5));
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 2), path.GetBoundingBox());

  path.ClosePath();
  ASSERT_EQ(6u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(5));
  EXPECT_TRUE(path.IsClosingFigure(5));
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());

  // Calling ClosePath() repeatedly makes no difference.
  path.ClosePath();
  ASSERT_EQ(6u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(5));
  EXPECT_TRUE(path.IsClosingFigure(5));
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());

  // A hexagon with the same start/end point is still not a rectangle.
  path.Clear();
  path.AppendPoint({1, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({3, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 2}, FXPT_TYPE::LineTo);
  path.AppendPoint({1, 2}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({1, 0}, FXPT_TYPE::LineTo);
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 2), path.GetBoundingBox());
}

TEST(CFX_PathData, ClosePath) {
  CFX_PathData path;
  path.AppendLine({0, 0}, {0, 1});
  path.AppendLine({0, 1}, {1, 1});
  path.AppendLine({1, 1}, {1, 0});
  ASSERT_EQ(4u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(3));
  EXPECT_FALSE(path.IsClosingFigure(3));
  EXPECT_TRUE(path.IsRect());
  Optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  const CFX_Matrix kIdentityMatrix;
  ASSERT_TRUE(kIdentityMatrix.IsIdentity());
  rect = path.GetRect(&kIdentityMatrix);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  path.ClosePath();
  ASSERT_EQ(4u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(3));
  EXPECT_TRUE(path.IsClosingFigure(3));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  // Calling ClosePath() repeatedly makes no difference.
  path.ClosePath();
  ASSERT_EQ(4u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(3));
  EXPECT_TRUE(path.IsClosingFigure(3));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  path.AppendPointAndClose({0, 0}, FXPT_TYPE::LineTo);
  ASSERT_EQ(5u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(3));
  EXPECT_TRUE(path.IsClosingFigure(3));
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(4));
  EXPECT_TRUE(path.IsClosingFigure(4));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());
}

TEST(CFX_PathData, FivePointRect) {
  CFX_PathData path;
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  ASSERT_EQ(5u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(4));
  EXPECT_FALSE(path.IsClosingFigure(4));
  EXPECT_TRUE(path.IsRect());
  Optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());

  path.ClosePath();
  ASSERT_EQ(5u, path.GetPoints().size());
  EXPECT_EQ(FXPT_TYPE::LineTo, path.GetType(4));
  EXPECT_TRUE(path.IsClosingFigure(4));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());
}

TEST(CFX_PathData, SixPlusPointRect) {
  CFX_PathData path;
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  EXPECT_TRUE(path.IsRect());
  Optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());
}

TEST(CFX_PathData, NotRect) {
  CFX_PathData path;
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0.1f}, FXPT_TYPE::LineTo);
  EXPECT_FALSE(path.IsRect());
  Optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.ClosePath();
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({3, 1}, FXPT_TYPE::LineTo);
  path.AppendPointAndClose({0, 1}, FXPT_TYPE::LineTo);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 1}, FXPT_TYPE::LineTo);
  path.AppendPointAndClose({0, 1}, FXPT_TYPE::MoveTo);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({3, 0}, FXPT_TYPE::LineTo);
  path.AppendPointAndClose({0, 1}, FXPT_TYPE::LineTo);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({2, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());
}

TEST(CFX_PathData, EmptyRect) {
  // Document existing behavior where an empty rect is still considered a rect.
  CFX_PathData path;
  path.AppendPoint({0, 0}, FXPT_TYPE::MoveTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 1}, FXPT_TYPE::LineTo);
  path.AppendPoint({0, 0}, FXPT_TYPE::LineTo);
  EXPECT_TRUE(path.IsRect());
  Optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 0, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 0, 1), path.GetBoundingBox());
}
