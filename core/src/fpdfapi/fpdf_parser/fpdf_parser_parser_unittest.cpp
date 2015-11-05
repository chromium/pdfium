// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"

#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fxcrt/fx_stream.h"

class CPDF_TestParser : public CPDF_Parser {
 public:
  CPDF_TestParser() {}
  ~CPDF_TestParser() {}

  bool InitTest(const FX_CHAR* path) {
    IFX_FileRead* pFileAccess = FX_CreateFileRead(path);
    if (!pFileAccess)
      return false;

    // For the test file, the header is set at the beginning.
    m_Syntax.InitParser(pFileAccess, 0);
    return true;
  }

 private:
  // Add test case as private friend so that RebuildCrossRef in CPDF_Parser
  // can be accessed.
  FRIEND_TEST(fpdf_parser_parser, RebuildCrossRefCorrectly);
};

// TODO(thestig) Using unique_ptr with ReleaseDeleter is still not ideal.
// Come up or wait for something better.
using ScopedFileStream =
    nonstd::unique_ptr<IFX_FileStream, ReleaseDeleter<IFX_FileStream>>;

TEST(fpdf_parser_parser, ReadHexString) {
  {
    // Empty string.
    uint8_t data[] = "";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 0, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("", parser.ReadHexString());
    EXPECT_EQ(0, parser.SavePos());
  }

  {
    // Blank string.
    uint8_t data[] = "  ";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 2, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("", parser.ReadHexString());
    EXPECT_EQ(2, parser.SavePos());
  }

  {
    // Skips unknown characters.
    uint8_t data[] = "z12b";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 4, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\x12\xb0", parser.ReadHexString());
    EXPECT_EQ(4, parser.SavePos());
  }

  {
    // Skips unknown characters.
    uint8_t data[] = "*<&*#$^&@1";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 10, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\x10", parser.ReadHexString());
    EXPECT_EQ(10, parser.SavePos());
  }

  {
    // Skips unknown characters.
    uint8_t data[] = "\x80zab";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 4, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\xab", parser.ReadHexString());
    EXPECT_EQ(4, parser.SavePos());
  }

  {
    // Skips unknown characters.
    uint8_t data[] = "\xffzab";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 4, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\xab", parser.ReadHexString());
    EXPECT_EQ(4, parser.SavePos());
  }

  {
    // Regular conversion.
    uint8_t data[] = "1A2b>abcd";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 9, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\x1a\x2b", parser.ReadHexString());
    EXPECT_EQ(5, parser.SavePos());
  }

  {
    // Position out of bounds.
    uint8_t data[] = "12ab>";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 5, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    parser.RestorePos(5);
    EXPECT_EQ("", parser.ReadHexString());

    parser.RestorePos(6);
    EXPECT_EQ("", parser.ReadHexString());

    parser.RestorePos(-1);
    EXPECT_EQ("", parser.ReadHexString());

    parser.RestorePos(std::numeric_limits<FX_FILESIZE>::max());
    EXPECT_EQ("", parser.ReadHexString());

    // Check string still parses when set to 0.
    parser.RestorePos(0);
    EXPECT_EQ("\x12\xab", parser.ReadHexString());
  }

  {
    // Missing ending >.
    uint8_t data[] = "1A2b";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 4, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\x1a\x2b", parser.ReadHexString());
    EXPECT_EQ(4, parser.SavePos());
  }

  {
    // Missing ending >.
    uint8_t data[] = "12abz";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 5, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\x12\xab", parser.ReadHexString());
    EXPECT_EQ(5, parser.SavePos());
  }

  {
    // Uneven number of bytes.
    uint8_t data[] = "1A2>asdf";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 8, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\x1a\x20", parser.ReadHexString());
    EXPECT_EQ(4, parser.SavePos());
  }

  {
    // Uneven number of bytes.
    uint8_t data[] = "1A2zasdf";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 8, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("\x1a\x2a\xdf", parser.ReadHexString());
    EXPECT_EQ(8, parser.SavePos());
  }

  {
    // Just ending character.
    uint8_t data[] = ">";
    ScopedFileStream stream(FX_CreateMemoryStream(data, 1, FALSE));

    CPDF_SyntaxParser parser;
    parser.InitParser(stream.get(), 0);
    EXPECT_EQ("", parser.ReadHexString());
    EXPECT_EQ(1, parser.SavePos());
  }
}

TEST(fpdf_parser_parser, RebuildCrossRefCorrectly) {
  CPDF_TestParser parser;
  ASSERT_TRUE(
      parser.InitTest("testing/resources/parser_rebuildxref_correct.pdf"));

  ASSERT_TRUE(parser.RebuildCrossRef());
  const FX_FILESIZE offsets[] = {0, 15, 61, 154, 296, 374, 450};
  const FX_WORD versions[] = {0, 0, 2, 4, 6, 8, 0};
  static_assert(FX_ArraySize(offsets) == FX_ArraySize(versions),
                "numbers of offsets and versions should be same.");
  ASSERT_EQ(FX_ArraySize(offsets), parser.m_CrossRef.GetSize());
  ASSERT_EQ(FX_ArraySize(versions), parser.m_ObjVersion.GetSize());

  for (int i = 0; i < FX_ArraySize(offsets); ++i) {
    EXPECT_EQ(offsets[i], parser.m_CrossRef.GetAt(i));
    EXPECT_EQ(versions[i], parser.m_ObjVersion.GetAt(i));
  }
}
