// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/cxfa_ffbarcode.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CXFA_FFBarcode, GetBarcodeTypeByName) {
  EXPECT_EQ(BC_TYPE::kUnknown, CXFA_FFBarcode::GetBarcodeTypeByName(L""));
  EXPECT_EQ(BC_TYPE::kUnknown,
            CXFA_FFBarcode::GetBarcodeTypeByName(L"not_found"));

  EXPECT_EQ(BC_TYPE::kEAN13, CXFA_FFBarcode::GetBarcodeTypeByName(L"ean13"));
  EXPECT_EQ(BC_TYPE::kPDF417, CXFA_FFBarcode::GetBarcodeTypeByName(L"pdf417"));
  EXPECT_EQ(BC_TYPE::kCode39,
            CXFA_FFBarcode::GetBarcodeTypeByName(L"code3Of9"));
}
