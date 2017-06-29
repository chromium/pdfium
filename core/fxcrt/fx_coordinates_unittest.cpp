// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_coordinates.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_Matrix, ReverseIdentity) {
  CFX_Matrix m;
  m.SetIdentity();
  CFX_Matrix rev = m.GetInverse();

  EXPECT_FLOAT_EQ(1.0, rev.a);
  EXPECT_FLOAT_EQ(0.0, rev.b);
  EXPECT_FLOAT_EQ(0.0, rev.c);
  EXPECT_FLOAT_EQ(1.0, rev.d);
  EXPECT_FLOAT_EQ(0.0, rev.e);
  EXPECT_FLOAT_EQ(0.0, rev.f);

  CFX_PointF expected(2, 3);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}

TEST(CFX_Matrix, Reverse) {
  float data[6] = {3, 0, 2, 3, 1, 4};
  CFX_Matrix m(data);
  CFX_Matrix rev = m.GetInverse();

  EXPECT_FLOAT_EQ(0.33333334f, rev.a);
  EXPECT_FLOAT_EQ(0.0f, rev.b);
  EXPECT_FLOAT_EQ(-0.22222222f, rev.c);
  EXPECT_FLOAT_EQ(0.33333334f, rev.d);
  EXPECT_FLOAT_EQ(0.55555556f, rev.e);
  EXPECT_FLOAT_EQ(-1.3333334f, rev.f);

  CFX_PointF expected(2, 3);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}

// Note, I think these are a bug and the matrix should be the identity.
TEST(CFX_Matrix, ReverseCR702041) {
  // The determinate is < std::numeric_limits<float>::epsilon()
  float data[6] = {0.947368443f, -0.108947366f, -0.923076928f,
                   0.106153846f, 18.0f,         787.929993f};
  CFX_Matrix m(data);
  CFX_Matrix rev = m.GetInverse();

  EXPECT_FLOAT_EQ(14247728.0f, rev.a);
  EXPECT_FLOAT_EQ(14622668.0f, rev.b);
  EXPECT_FLOAT_EQ(1.2389329e+08f, rev.c);
  EXPECT_FLOAT_EQ(1.2715364e+08f, rev.d);
  EXPECT_FLOAT_EQ(-9.7875698e+10f, rev.e);
  EXPECT_FLOAT_EQ(-1.0045138e+11f, rev.f);

  // Should be 2, 3
  CFX_PointF expected(0, 0);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}

TEST(CFX_Matrix, ReverseCR714187) {
  // The determinate is < std::numeric_limits<float>::epsilon()
  float data[6] = {0.000037f, 0.0f, 0.0f, -0.000037f, 182.413101f, 136.977646f};
  CFX_Matrix m(data);
  CFX_Matrix rev = m.GetInverse();

  EXPECT_FLOAT_EQ(27027.025f, rev.a);
  EXPECT_FLOAT_EQ(0.0f, rev.b);
  EXPECT_FLOAT_EQ(0.0f, rev.c);
  EXPECT_FLOAT_EQ(-27027.025f, rev.d);
  EXPECT_FLOAT_EQ(-4930083.5f, rev.e);
  EXPECT_FLOAT_EQ(3702098.2f, rev.f);

  // Should be 3 ....
  CFX_PointF expected(2, 2.75);
  CFX_PointF result = rev.Transform(m.Transform(CFX_PointF(2, 3)));
  EXPECT_FLOAT_EQ(expected.x, result.x);
  EXPECT_FLOAT_EQ(expected.y, result.y);
}
