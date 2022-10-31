// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/widetext_buffer.h"

#include <utility>

#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {

TEST(WideTextBuffer, EmptyBuf) {
  WideTextBuffer wtb;
  EXPECT_TRUE(wtb.GetWideSpan().empty());
  EXPECT_TRUE(wtb.AsStringView().IsEmpty());
  EXPECT_TRUE(wtb.MakeString().IsEmpty());
}

TEST(WideTextBuffer, OperatorLtLt) {
  WideTextBuffer wtb;
  wtb << "clams" << L"\u208c\u208e";
  EXPECT_EQ(wtb.MakeString(), L"clams\u208c\u208e");
}

TEST(WideTextBuffer, Deletion) {
  WideTextBuffer wtb;
  wtb << L"ABCDEFG";
  EXPECT_TRUE(wtb.AsStringView().EqualsASCII("ABCDEFG"));

  wtb.Delete(1, 3);
  EXPECT_TRUE(wtb.AsStringView().EqualsASCII("AEFG"));

  wtb.Delete(1, 0);
  EXPECT_TRUE(wtb.AsStringView().EqualsASCII("AEFG"));

  wtb.Delete(0, 2);
  EXPECT_TRUE(wtb.AsStringView().EqualsASCII("FG"));

  wtb.Delete(0, 2);
  EXPECT_TRUE(wtb.AsStringView().EqualsASCII(""));

  wtb.Delete(0, 0);
  EXPECT_TRUE(wtb.AsStringView().EqualsASCII(""));
}

TEST(WideTextBuffer, Move) {
  WideTextBuffer wtb;
  wtb << "clams";
  EXPECT_EQ(wtb.MakeString(), L"clams");

  WideTextBuffer wtb2(std::move(wtb));
  EXPECT_EQ(wtb.MakeString(), L"");
  EXPECT_EQ(wtb2.MakeString(), L"clams");

  WideTextBuffer wtb3;
  wtb3 = std::move(wtb2);
  EXPECT_EQ(wtb2.MakeString(), L"");
  EXPECT_EQ(wtb3.MakeString(), L"clams");
}

}  // namespace fxcrt
