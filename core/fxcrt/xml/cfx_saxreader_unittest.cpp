// Copyright 2017 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/xml/cfx_saxreader.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

using testing::_;
using testing::Eq;
using testing::Return;

namespace {

class MockHandler : public CFX_SAXReader::HandlerIface {
 public:
  MOCK_METHOD3(OnTagEnter,
               CFX_SAXContext*(const ByteStringView& bsTagName,
                               CFX_SAXItem::Type eType,
                               uint32_t dwStartPos));
  MOCK_METHOD3(OnTagAttribute,
               void(CFX_SAXContext* pTag,
                    const ByteStringView& bsAttri,
                    const ByteStringView& bsValue));
  MOCK_METHOD1(OnTagBreak, void(CFX_SAXContext* pTag));
  MOCK_METHOD4(OnTagData,
               void(CFX_SAXContext* pTag,
                    CFX_SAXItem::Type eType,
                    const ByteStringView& bsData,
                    uint32_t dwStartPos));
  MOCK_METHOD2(OnTagClose, void(CFX_SAXContext* pTag, uint32_t dwEndPos));
  MOCK_METHOD3(OnTagEnd,
               void(CFX_SAXContext* pTag,
                    const ByteStringView& bsTagName,
                    uint32_t dwEndPos));
  MOCK_METHOD4(OnTargetData,
               void(CFX_SAXContext* pTag,
                    CFX_SAXItem::Type eType,
                    const ByteStringView& bsData,
                    uint32_t dwStartPos));
};

}  // namespace

class CFX_SAXReaderTest : public testing::Test {
 public:
  void SetHandler(CFX_SAXReader::HandlerIface* handler) {
    reader_.SetHandler(handler);
  }

  bool StartParse(char* str) {
    return reader_.StartParse(
               pdfium::MakeRetain<CFX_MemoryStream>(
                   reinterpret_cast<uint8_t*>(str), strlen(str), false),
               0, static_cast<uint32_t>(-1),
               CFX_SaxParseMode_NotSkipSpace) >= 0;
  }

  int32_t ContinueParse() {
    int32_t ret;
    do {
      ret = reader_.ContinueParse();
    } while (ret >= 0 && ret < 100);
    return ret;
  }

 private:
  CFX_SAXReader reader_;
};

TEST_F(CFX_SAXReaderTest, Null) {
  char data[] = "";
  ASSERT_FALSE(StartParse(data));
}

TEST_F(CFX_SAXReaderTest, SimpleText) {
  MockHandler mock;
  SetHandler(&mock);

  char data[] = "clams";
  ASSERT_TRUE(StartParse(data));
  EXPECT_EQ(100, ContinueParse());
}

TEST_F(CFX_SAXReaderTest, SimpleTag) {
  MockHandler mock;
  EXPECT_CALL(mock, OnTagEnter(Eq("clams"), _, _));
  EXPECT_CALL(mock, OnTagBreak(_));
  SetHandler(&mock);

  char data[] = "<clams>";
  ASSERT_TRUE(StartParse(data));
  EXPECT_EQ(100, ContinueParse());
}

TEST_F(CFX_SAXReaderTest, AttributeTag) {
  MockHandler mock;
  EXPECT_CALL(mock, OnTagEnter(Eq("clams"), _, _));
  EXPECT_CALL(mock, OnTagAttribute(_, Eq("size"), Eq("small")));
  EXPECT_CALL(mock, OnTagAttribute(_, Eq("color"), Eq("red")));
  EXPECT_CALL(mock, OnTagBreak(_));
  SetHandler(&mock);

  char data[] = "<clams size='small' color='red'>";
  ASSERT_TRUE(StartParse(data));
  EXPECT_EQ(100, ContinueParse());
}

TEST_F(CFX_SAXReaderTest, AttributeEntityTag) {
  MockHandler mock;
  EXPECT_CALL(mock, OnTagEnter(Eq("clams"), _, _));
  EXPECT_CALL(mock, OnTagAttribute(_, Eq("predicate"), Eq("1 < 2")));
  EXPECT_CALL(mock, OnTagBreak(_));
  SetHandler(&mock);

  char data[] = "<clams predicate='1 &lt; 2'>";
  ASSERT_TRUE(StartParse(data));
  EXPECT_EQ(100, ContinueParse());
}

TEST_F(CFX_SAXReaderTest, TextWithinTag) {
  MockHandler mock;
  EXPECT_CALL(mock, OnTagEnter(Eq("b"), _, _));
  EXPECT_CALL(mock, OnTagBreak(_));
  EXPECT_CALL(mock, OnTagData(_, _, Eq("biff"), _));
  EXPECT_CALL(mock, OnTagEnd(_, Eq("b"), _));
  SetHandler(&mock);

  char data[] = "<b>biff</b>";
  ASSERT_TRUE(StartParse(data));
  EXPECT_EQ(100, ContinueParse());
}

TEST_F(CFX_SAXReaderTest, bug_711459) {
  char data[] =
      "&a<tag "
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      ">x;";
  ASSERT_TRUE(StartParse(data));
  EXPECT_EQ(100, ContinueParse());
}
