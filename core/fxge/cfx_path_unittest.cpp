// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_path.h"

#include "core/fxcrt/fx_coordinates.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_Path, BasicTest) {
  CFX_Path path;
  path.AppendRect(/*left=*/1, /*bottom=*/2, /*right=*/3, /*top=*/5);
  EXPECT_EQ(5u, path.GetPoints().size());
  EXPECT_TRUE(path.IsRect());
  absl::optional<CFX_FloatRect> rect = path.GetRect(nullptr);
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
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({1, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({1, 0}, CFX_Path::Point::Type::kLine);
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

TEST(CFX_Path, ShearTransform) {
  CFX_Path path;
  path.AppendRect(/*left=*/1, /*bottom=*/2, /*right=*/3, /*top=*/5);

  const CFX_Matrix kShearMatrix(1, 2, 0, 1, 0, 0);
  EXPECT_TRUE(path.IsRect());
  absl::optional<CFX_FloatRect> rect = path.GetRect(&kShearMatrix);
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

TEST(CFX_Path, Hexagon) {
  CFX_Path path;
  path.AppendPoint({1, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({3, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 2}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({1, 2}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  ASSERT_EQ(6u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(5));
  EXPECT_FALSE(path.IsClosingFigure(5));
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 2), path.GetBoundingBox());

  path.ClosePath();
  ASSERT_EQ(6u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(5));
  EXPECT_TRUE(path.IsClosingFigure(5));
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());

  // Calling ClosePath() repeatedly makes no difference.
  path.ClosePath();
  ASSERT_EQ(6u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(5));
  EXPECT_TRUE(path.IsClosingFigure(5));
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());

  // A hexagon with the same start/end point is still not a rectangle.
  path.Clear();
  path.AppendPoint({1, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({3, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 2}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({1, 2}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({1, 0}, CFX_Path::Point::Type::kLine);
  EXPECT_FALSE(path.IsRect());
  EXPECT_FALSE(path.GetRect(nullptr).has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 2), path.GetBoundingBox());
}

TEST(CFX_Path, ClosePath) {
  CFX_Path path;
  path.AppendLine({0, 0}, {0, 1});
  path.AppendLine({0, 1}, {1, 1});
  path.AppendLine({1, 1}, {1, 0});
  ASSERT_EQ(4u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(3));
  EXPECT_FALSE(path.IsClosingFigure(3));
  EXPECT_TRUE(path.IsRect());
  absl::optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  const CFX_Matrix kIdentityMatrix;
  ASSERT_TRUE(kIdentityMatrix.IsIdentity());
  rect = path.GetRect(&kIdentityMatrix);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  path.ClosePath();
  ASSERT_EQ(4u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(3));
  EXPECT_TRUE(path.IsClosingFigure(3));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  // Calling ClosePath() repeatedly makes no difference.
  path.ClosePath();
  ASSERT_EQ(4u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(3));
  EXPECT_TRUE(path.IsClosingFigure(3));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());

  path.AppendPointAndClose({0, 0}, CFX_Path::Point::Type::kLine);
  ASSERT_EQ(5u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(3));
  EXPECT_TRUE(path.IsClosingFigure(3));
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(4));
  EXPECT_TRUE(path.IsClosingFigure(4));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 1, 1), rect.value());
}

TEST(CFX_Path, FivePointRect) {
  CFX_Path path;
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  ASSERT_EQ(5u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(4));
  EXPECT_FALSE(path.IsClosingFigure(4));
  EXPECT_TRUE(path.IsRect());
  absl::optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());

  path.ClosePath();
  ASSERT_EQ(5u, path.GetPoints().size());
  EXPECT_EQ(CFX_Path::Point::Type::kLine, path.GetType(4));
  EXPECT_TRUE(path.IsClosingFigure(4));
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());
}

TEST(CFX_Path, SixPlusPointRect) {
  CFX_Path path;
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  EXPECT_TRUE(path.IsRect());
  absl::optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  EXPECT_TRUE(path.IsRect());
  rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());
}

TEST(CFX_Path, NotRect) {
  CFX_Path path;
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0.1f}, CFX_Path::Point::Type::kLine);
  EXPECT_FALSE(path.IsRect());
  absl::optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.ClosePath();
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({3, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPointAndClose({0, 1}, CFX_Path::Point::Type::kLine);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPointAndClose({0, 1}, CFX_Path::Point::Type::kMove);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({3, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPointAndClose({0, 1}, CFX_Path::Point::Type::kLine);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 3, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 1), path.GetBoundingBox());

  path.Clear();
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({2, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({2, 2}, CFX_Path::Point::Type::kLine);
  EXPECT_FALSE(path.IsRect());
  rect = path.GetRect(nullptr);
  EXPECT_FALSE(rect.has_value());
  const CFX_Matrix kScaleMatrix(1, 0, 0, 2, 60, 70);
  rect = path.GetRect(&kScaleMatrix);
  EXPECT_FALSE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 2, 2), path.GetBoundingBox());
}

TEST(CFX_Path, EmptyRect) {
  // Document existing behavior where an empty rect is still considered a rect.
  CFX_Path path;
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kMove);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
  path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
  EXPECT_TRUE(path.IsRect());
  absl::optional<CFX_FloatRect> rect = path.GetRect(nullptr);
  ASSERT_TRUE(rect.has_value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 0, 1), rect.value());
  EXPECT_EQ(CFX_FloatRect(0, 0, 0, 1), path.GetBoundingBox());
}

TEST(CFX_Path, Append) {
  CFX_Path path;
  path.AppendPoint({5, 6}, CFX_Path::Point::Type::kMove);
  ASSERT_EQ(1u, path.GetPoints().size());
  EXPECT_EQ(CFX_PointF(5, 6), path.GetPoint(0));

  CFX_Path empty_path;
  path.Append(empty_path, nullptr);
  ASSERT_EQ(1u, path.GetPoints().size());
  EXPECT_EQ(CFX_PointF(5, 6), path.GetPoint(0));

  path.Append(path, nullptr);
  ASSERT_EQ(2u, path.GetPoints().size());
  EXPECT_EQ(CFX_PointF(5, 6), path.GetPoint(0));
  EXPECT_EQ(CFX_PointF(5, 6), path.GetPoint(1));

  const CFX_Matrix kScaleMatrix(1, 0, 0, 2, 60, 70);
  path.Append(path, &kScaleMatrix);
  ASSERT_EQ(4u, path.GetPoints().size());
  EXPECT_EQ(CFX_PointF(5, 6), path.GetPoint(0));
  EXPECT_EQ(CFX_PointF(5, 6), path.GetPoint(1));
  EXPECT_EQ(CFX_PointF(65, 82), path.GetPoint(2));
  EXPECT_EQ(CFX_PointF(65, 82), path.GetPoint(3));
}

TEST(CFX_Path, GetBoundingBoxForStrokePath) {
  static constexpr float kLineWidth = 1.0f;
  static constexpr float kMiterLimit = 1.0f;

  {
    // Test the case that the first/last point is "move" and it closes the
    // paths.
    CFX_Path path;
    path.AppendPoint({2, 0}, CFX_Path::Point::Type::kMove);
    path.ClosePath();
    EXPECT_EQ(CFX_FloatRect(2, 0, 2, 0),
              path.GetBoundingBoxForStrokePath(kLineWidth, kMiterLimit));
  }

  {
    // Test on a regular rect path.
    CFX_Path path;
    path.AppendPoint({2, 0}, CFX_Path::Point::Type::kMove);
    path.AppendPoint({2, 1}, CFX_Path::Point::Type::kLine);
    path.AppendPoint({0, 1}, CFX_Path::Point::Type::kLine);
    path.AppendPoint({0, 0}, CFX_Path::Point::Type::kLine);
    path.ClosePath();
    EXPECT_EQ(CFX_FloatRect(-1, -1, 3, 2),
              path.GetBoundingBoxForStrokePath(kLineWidth, kMiterLimit));

    // If the final point is "move" and the path remains open, it should not
    // affect the bounding rect.
    path.AppendPoint({20, 20}, CFX_Path::Point::Type::kMove);
    EXPECT_EQ(CFX_FloatRect(-1, -1, 3, 2),
              path.GetBoundingBoxForStrokePath(kLineWidth, kMiterLimit));
  }
}
