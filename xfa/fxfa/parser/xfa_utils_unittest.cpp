// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/xfa_utils.h"

#include <iterator>

#include "testing/gtest/include/gtest/gtest.h"

namespace {

struct TestCase {
  int input;
  int expected_output;
};

}  // namespace

TEST(XfaUtilsImpTest, XFAMapRotation) {
  static const TestCase kTestCases[] = {
      {-1000000, 80}, {-361, 359},  {-360, 0}, {-359, 1},  {-91, 269},
      {-90, 270},     {-89, 271},   {-1, 359}, {0, 0},     {1, 1},
      {89, 89},       {90, 90},     {91, 91},  {359, 359}, {360, 0},
      {361, 1},       {100000, 280}};
  for (const TestCase& item : kTestCases) {
    EXPECT_EQ(item.expected_output, XFA_MapRotation(item.input));
  }
}
