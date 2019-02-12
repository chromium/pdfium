// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fgas/crt/cfgas_decimal.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"

TEST(CFGAS_Decimal, Empty) {
  CFGAS_Decimal empty;
  EXPECT_EQ(L"0", empty.ToWideString());
  EXPECT_EQ(0.0f, empty.ToFloat());
  EXPECT_EQ(0.0, empty.ToDouble());
}
