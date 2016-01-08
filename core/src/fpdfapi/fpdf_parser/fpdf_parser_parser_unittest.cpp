// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fpdfapi/fpdf_parser.h"
#include "core/include/fxcrt/fx_stream.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/path_service.h"

// Functions to help test an array's content against expected results.
template <class TYPE>
bool CompareArray(const CFX_ArrayTemplate<TYPE>& array1,
                  const TYPE* array2,
                  size_t size) {
  if (array1.GetSize() != size)
    return false;

  for (int i = 0; i < size; ++i)
    if (array1.GetAt(i) != array2[i])
      return false;
  return true;
}

// Provide a way to read test data from a buffer instead of a file.
class CFX_TestBufferRead : public IFX_FileRead {
 public:
  CFX_TestBufferRead(const unsigned char* buffer_in, size_t buf_size)
      : buffer_(buffer_in), total_size_(buf_size) {}

  // IFX_Stream
  void Release() override { delete this; }

  // IFX_FileRead
  FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override {
    if (offset < 0 || offset + size > total_size_) {
      return FALSE;
    }

    memcpy(buffer, buffer_ + offset, size);
    return TRUE;
  }
  FX_FILESIZE GetSize() override { return (FX_FILESIZE)total_size_; };

 protected:
  const unsigned char* buffer_;
  size_t total_size_;
};

// A wrapper class to help test member functions of CPDF_Parser.
class CPDF_TestParser : public CPDF_Parser {
 public:
  CPDF_TestParser() {}
  ~CPDF_TestParser() {}

  // Setup reading from a file and initial states.
  bool InitTestFromFile(const FX_CHAR* path) {
    IFX_FileRead* pFileAccess = FX_CreateFileRead(path);
    if (!pFileAccess)
      return false;

    // For the test file, the header is set at the beginning.
    m_Syntax.InitParser(pFileAccess, 0);
    return true;
  }

  // Setup reading from a buffer and initial states.
  bool InitTestFromBuffer(const unsigned char* buffer, size_t len) {
    CFX_TestBufferRead* buffer_reader = new CFX_TestBufferRead(buffer, len);

    // For the test file, the header is set at the beginning.
    m_Syntax.InitParser(buffer_reader, 0);
    return true;
  }

 private:
  // Add test cases here as private friend so that protected members in
  // CPDF_Parser can be accessed by test cases.
  // Need to access RebuildCrossRef.
  FRIEND_TEST(fpdf_parser_parser, RebuildCrossRefCorrectly);
  FRIEND_TEST(fpdf_parser_parser, RebuildCrossRefFailed);
  // Need to access LoadCrossRefV4.
  FRIEND_TEST(fpdf_parser_parser, LoadCrossRefV4);
};

// TODO(thestig) Using unique_ptr with ReleaseDeleter is still not ideal.
// Come up or wait for something better.
using ScopedFileStream =
    std::unique_ptr<IFX_FileStream, ReleaseDeleter<IFX_FileStream>>;

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
  std::string test_file;
  ASSERT_TRUE(PathService::GetTestFilePath("parser_rebuildxref_correct.pdf",
                                           &test_file));
  ASSERT_TRUE(parser.InitTestFromFile(test_file.c_str())) << test_file;

  ASSERT_TRUE(parser.RebuildCrossRef());
  const FX_FILESIZE offsets[] = {0, 15, 61, 154, 296, 374, 450};
  const FX_WORD versions[] = {0, 0, 2, 4, 6, 8, 0};
  for (size_t i = 0; i < FX_ArraySize(offsets); ++i)
    EXPECT_EQ(offsets[i], parser.m_ObjectInfo[i].pos);
  ASSERT_TRUE(
      CompareArray(parser.m_ObjVersion, versions, FX_ArraySize(versions)));
}

TEST(fpdf_parser_parser, RebuildCrossRefFailed) {
  CPDF_TestParser parser;
  std::string test_file;
  ASSERT_TRUE(PathService::GetTestFilePath(
      "parser_rebuildxref_error_notrailer.pdf", &test_file));
  ASSERT_TRUE(parser.InitTestFromFile(test_file.c_str())) << test_file;

  ASSERT_FALSE(parser.RebuildCrossRef());
}

TEST(fpdf_parser_parser, LoadCrossRefV4) {
  {
    const unsigned char xref_table[] =
        "xref \n"
        "0 6 \n"
        "0000000003 65535 f \n"
        "0000000017 00000 n \n"
        "0000000081 00000 n \n"
        "0000000000 00007 f \n"
        "0000000331 00000 n \n"
        "0000000409 00000 n \n"
        "trail";  // Needed to end cross ref table reading.
    CPDF_TestParser parser;
    ASSERT_TRUE(
        parser.InitTestFromBuffer(xref_table, FX_ArraySize(xref_table)));

    ASSERT_TRUE(parser.LoadCrossRefV4(0, 0, FALSE));
    const FX_FILESIZE offsets[] = {0, 17, 81, 0, 331, 409};
    const uint8_t types[] = {0, 1, 1, 0, 1, 1};
    for (size_t i = 0; i < FX_ArraySize(offsets); ++i)
      EXPECT_EQ(offsets[i], parser.m_ObjectInfo[i].pos);
    ASSERT_TRUE(CompareArray(parser.m_V5Type, types, FX_ArraySize(types)));
  }
  {
    const unsigned char xref_table[] =
        "xref \n"
        "0 1 \n"
        "0000000000 65535 f \n"
        "3 1 \n"
        "0000025325 00000 n \n"
        "8 2 \n"
        "0000025518 00002 n \n"
        "0000025635 00000 n \n"
        "12 1 \n"
        "0000025777 00000 n \n"
        "trail";  // Needed to end cross ref table reading.
    CPDF_TestParser parser;
    ASSERT_TRUE(
        parser.InitTestFromBuffer(xref_table, FX_ArraySize(xref_table)));

    ASSERT_TRUE(parser.LoadCrossRefV4(0, 0, FALSE));
    const FX_FILESIZE offsets[] = {0, 0,     0,     25325, 0, 0,    0,
                                   0, 25518, 25635, 0,     0, 25777};
    const uint8_t types[] = {0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1};
    for (size_t i = 0; i < FX_ArraySize(offsets); ++i)
      EXPECT_EQ(offsets[i], parser.m_ObjectInfo[i].pos);
    ASSERT_TRUE(CompareArray(parser.m_V5Type, types, FX_ArraySize(types)));
  }
  {
    const unsigned char xref_table[] =
        "xref \n"
        "0 1 \n"
        "0000000000 65535 f \n"
        "3 1 \n"
        "0000025325 00000 n \n"
        "8 2 \n"
        "0000000000 65535 f \n"
        "0000025635 00000 n \n"
        "12 1 \n"
        "0000025777 00000 n \n"
        "trail";  // Needed to end cross ref table reading.
    CPDF_TestParser parser;
    ASSERT_TRUE(
        parser.InitTestFromBuffer(xref_table, FX_ArraySize(xref_table)));

    ASSERT_TRUE(parser.LoadCrossRefV4(0, 0, FALSE));
    const FX_FILESIZE offsets[] = {0, 0, 0,     25325, 0, 0,    0,
                                   0, 0, 25635, 0,     0, 25777};
    const uint8_t types[] = {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1};
    for (size_t i = 0; i < FX_ArraySize(offsets); ++i)
      EXPECT_EQ(offsets[i], parser.m_ObjectInfo[i].pos);
    ASSERT_TRUE(CompareArray(parser.m_V5Type, types, FX_ArraySize(types)));
  }
  {
    const unsigned char xref_table[] =
        "xref \n"
        "0 7 \n"
        "0000000002 65535 f \n"
        "0000000023 00000 n \n"
        "0000000003 65535 f \n"
        "0000000004 65535 f \n"
        "0000000000 65535 f \n"
        "0000000045 00000 n \n"
        "0000000179 00000 n \n"
        "trail";  // Needed to end cross ref table reading.
    CPDF_TestParser parser;
    ASSERT_TRUE(
        parser.InitTestFromBuffer(xref_table, FX_ArraySize(xref_table)));

    ASSERT_TRUE(parser.LoadCrossRefV4(0, 0, FALSE));
    const FX_FILESIZE offsets[] = {0, 23, 0, 0, 0, 45, 179};
    const uint8_t types[] = {0, 1, 0, 0, 0, 1, 1};
    for (size_t i = 0; i < FX_ArraySize(offsets); ++i)
      EXPECT_EQ(offsets[i], parser.m_ObjectInfo[i].pos);
    ASSERT_TRUE(CompareArray(parser.m_V5Type, types, FX_ArraySize(types)));
  }
}
