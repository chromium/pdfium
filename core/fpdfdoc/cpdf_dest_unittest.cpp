// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_dest.h"

#include <memory>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_null.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(cpdf_dest, GetXYZ) {
  bool hasX;
  bool hasY;
  bool hasZoom;
  float x;
  float y;
  float zoom;

  // |array| must outlive |dest|.
  auto array = pdfium::MakeRetain<CPDF_Array>();
  array->AppendNew<CPDF_Number>(0);  // Page Index.
  array->AppendNew<CPDF_Name>("XYZ");
  array->AppendNew<CPDF_Number>(4);  // X
  {
    CPDF_Dest dest(nullptr);
    EXPECT_FALSE(dest.GetXYZ(&hasX, &hasY, &hasZoom, &x, &y, &zoom));
  }
  {
    // Not enough entries.
    CPDF_Dest dest(array);
    EXPECT_FALSE(dest.GetXYZ(&hasX, &hasY, &hasZoom, &x, &y, &zoom));
  }
  array->AppendNew<CPDF_Number>(5);  // Y
  array->AppendNew<CPDF_Number>(6);  // Zoom.
  {
    CPDF_Dest dest(array);
    EXPECT_TRUE(dest.GetXYZ(&hasX, &hasY, &hasZoom, &x, &y, &zoom));
    EXPECT_TRUE(hasX);
    EXPECT_TRUE(hasY);
    EXPECT_TRUE(hasZoom);
    EXPECT_EQ(4, x);
    EXPECT_EQ(5, y);
    EXPECT_EQ(6, zoom);
  }
  // Set zoom to 0.
  array->SetNewAt<CPDF_Number>(4, 0);
  {
    CPDF_Dest dest(array);
    EXPECT_TRUE(dest.GetXYZ(&hasX, &hasY, &hasZoom, &x, &y, &zoom));
    EXPECT_FALSE(hasZoom);
  }
  // Set values to null.
  array->SetNewAt<CPDF_Null>(2);
  array->SetNewAt<CPDF_Null>(3);
  array->SetNewAt<CPDF_Null>(4);
  {
    CPDF_Dest dest(array);
    EXPECT_TRUE(dest.GetXYZ(&hasX, &hasY, &hasZoom, &x, &y, &zoom));
    EXPECT_FALSE(hasX);
    EXPECT_FALSE(hasY);
    EXPECT_FALSE(hasZoom);
  }
}
