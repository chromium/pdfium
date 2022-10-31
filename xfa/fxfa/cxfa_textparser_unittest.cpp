// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/cxfa_textparser.h"

#include "fxjs/gc/heap.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/cppgc/heap.h"

class CXFA_TestTextParser final : public CXFA_TextParser {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;

 private:
  CXFA_TestTextParser() = default;

  // Add test cases as friends to access protected member functions.
  FRIEND_TEST(CXFATextParserTest, TagValidate);
};

class CXFATextParserTest : public FXGCUnitTest {};

TEST_F(CXFATextParserTest, TagValidate) {
  auto* parser = cppgc::MakeGarbageCollected<CXFA_TestTextParser>(
      heap()->GetAllocationHandle());
  EXPECT_TRUE(parser->TagValidate(L"br"));
  EXPECT_TRUE(parser->TagValidate(L"Br"));
  EXPECT_TRUE(parser->TagValidate(L"BR"));
  EXPECT_TRUE(parser->TagValidate(L"a"));
  EXPECT_TRUE(parser->TagValidate(L"b"));
  EXPECT_TRUE(parser->TagValidate(L"i"));
  EXPECT_TRUE(parser->TagValidate(L"p"));
  EXPECT_TRUE(parser->TagValidate(L"li"));
  EXPECT_TRUE(parser->TagValidate(L"ol"));
  EXPECT_TRUE(parser->TagValidate(L"ul"));
  EXPECT_TRUE(parser->TagValidate(L"sub"));
  EXPECT_TRUE(parser->TagValidate(L"sup"));
  EXPECT_TRUE(parser->TagValidate(L"span"));
  EXPECT_TRUE(parser->TagValidate(L"body"));
  EXPECT_TRUE(parser->TagValidate(L"html"));

  EXPECT_FALSE(parser->TagValidate(L""));
  EXPECT_FALSE(parser->TagValidate(L"tml"));
  EXPECT_FALSE(parser->TagValidate(L"xhtml"));
  EXPECT_FALSE(parser->TagValidate(L"htmlx"));
}
