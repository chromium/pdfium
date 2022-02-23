// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_string_wrappers.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(STLUtil, string) {
  fxcrt::string str;
  str += '2';
  str += '2';
  str += "C is ";
  str += '7';
  str += '1';
  str += '.';
  str += '6';
  str += "F";
  EXPECT_STREQ("22C is 71.6F", str.c_str());
}

TEST(STLUtil, OStringStream) {
  fxcrt::ostringstream str;
  str << 22 << "C is " << 71.6f << "F";
  EXPECT_STREQ("22C is 71.6F", str.str().c_str());
}
