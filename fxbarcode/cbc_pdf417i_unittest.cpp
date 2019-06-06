// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/cbc_pdf417i.h"

#include <vector>

#include "testing/gtest/include/gtest/gtest.h"

TEST(CBC_PDF417ITest, Normal) {
  CBC_PDF417I encoder;
  EXPECT_TRUE(encoder.Encode(L"Foo"));
}

TEST(CBC_PDF417ITest, MaxLength) {
  std::vector<wchar_t> input(2711, L'1');
  CBC_PDF417I encoder;
  EXPECT_FALSE(encoder.Encode(WideStringView(input.data(), input.size())));
}
