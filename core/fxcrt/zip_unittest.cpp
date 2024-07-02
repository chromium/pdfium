// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "core/fxcrt/span.h"
#include "core/fxcrt/zip.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::ElementsAreArray;

namespace fxcrt {

TEST(Zip, EmptyZip) {
  pdfium::span<const int> nothing;
  int stuff[] = {1, 2, 3};

  auto zip_nothing_nothing = Zip(nothing, nothing);
  EXPECT_EQ(zip_nothing_nothing.begin(), zip_nothing_nothing.end());

  auto zip_nothing_stuff = Zip(nothing, stuff);
  EXPECT_EQ(zip_nothing_stuff.begin(), zip_nothing_stuff.end());
}

TEST(Zip, ActualZip) {
  const int stuff[] = {1, 2, 3};
  const int expected[] = {1, 2, 3, 0};
  int output[4] = {};

  for (auto [in, out] : Zip(stuff, output)) {
    out = in;
  }
  EXPECT_THAT(output, ElementsAreArray(expected));
}

TEST(Zip, BadArgumentsZip) {
  pdfium::span<const int> nothing;
  int stuff[] = {1, 2, 3};

  EXPECT_DEATH(Zip(stuff, nothing), ".*");
}

}  // namespace fxcrt
