// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_parser.h"

#include <array>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_linearized_header.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_syntax_parser.h"
#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/path_service.h"

using testing::ElementsAre;
using testing::Pair;
using testing::Return;

namespace {

struct OffsetAndType {
  FX_FILESIZE offset;
  CPDF_CrossRefTable::ObjectType type;
};

CPDF_CrossRefTable::ObjectInfo GetObjInfo(const CPDF_Parser& parser,
                                          uint32_t obj_num) {
  const auto* info =
      parser.GetCrossRefTableForTesting()->GetObjectInfo(obj_num);
  return info ? *info : CPDF_CrossRefTable::ObjectInfo();
}

class TestObjectsHolder final : public CPDF_Parser::ParsedObjectsHolder {
 public:
  TestObjectsHolder() = default;
  ~TestObjectsHolder() override = default;

  // CPDF_Parser::ParsedObjectsHolder:
  bool TryInit() override { return true; }
  MOCK_METHOD(RetainPtr<CPDF_Object>, ParseIndirectObject, (uint32_t objnum));
};

}  // namespace

// Test-only helper to support Gmock. Cannot be in an anonymous namespace.
bool operator==(const CPDF_CrossRefTable::ObjectInfo& lhs,
                const CPDF_CrossRefTable::ObjectInfo& rhs) {
  if (lhs.type != rhs.type) {
    return false;
  }

  if (lhs.gennum != rhs.gennum) {
    return false;
  }

  switch (lhs.type) {
    case CPDF_CrossRefTable::ObjectType::kFree:
      return true;
    case CPDF_CrossRefTable::ObjectType::kNormal:
      return lhs.pos == rhs.pos;
    case CPDF_CrossRefTable::ObjectType::kCompressed:
      return lhs.archive.obj_num == rhs.archive.obj_num &&
             lhs.archive.obj_index == rhs.archive.obj_index;
  }
}

// Test-only helper to let Gmock pretty-print `info`. Cannot be in an anonymous
// namespace.
std::ostream& operator<<(std::ostream& os,
                         const CPDF_CrossRefTable::ObjectInfo& info) {
  os << "(";
  switch (info.type) {
    case CPDF_CrossRefTable::ObjectType::kFree:
      os << "Free object";
      break;
    case CPDF_CrossRefTable::ObjectType::kNormal:
      os << "Normal object, pos: " << info.pos
         << ", obj_stream=" << info.is_object_stream_flag;
      break;
    case CPDF_CrossRefTable::ObjectType::kCompressed:
      os << "Compressed object, archive obj_num: " << info.archive.obj_num
         << ", archive obj_index: " << info.archive.obj_index;
      break;
  }
  os << ", gennum: " << info.gennum << ")";
  return os;
}

// A wrapper class to help test member functions of CPDF_Parser.
class CPDF_TestParser final : public CPDF_Parser {
 public:
  CPDF_TestParser() : CPDF_Parser(&object_holder_) {}
  ~CPDF_TestParser() = default;

  // Setup reading from a file and initial states.
  bool InitTestFromFile(const char* path) {
    RetainPtr<IFX_SeekableReadStream> pFileAccess =
        IFX_SeekableReadStream::CreateFromFilename(path);
    if (!pFileAccess)
      return false;

    // For the test file, the header is set at the beginning.
    SetSyntaxParserForTesting(
        std::make_unique<CPDF_SyntaxParser>(std::move(pFileAccess)));
    return true;
  }

  // Setup reading from a buffer and initial states.
  bool InitTestFromBufferWithOffset(pdfium::span<const uint8_t> buffer,
                                    FX_FILESIZE header_offset) {
    SetSyntaxParserForTesting(CPDF_SyntaxParser::CreateForTesting(
        pdfium::MakeRetain<CFX_ReadOnlySpanStream>(buffer), header_offset));
    return true;
  }

  bool InitTestFromBuffer(pdfium::span<const uint8_t> buffer) {
    return InitTestFromBufferWithOffset(buffer, 0 /*header_offset*/);
  }

  // Expose protected CPDF_Parser methods for testing.
  using CPDF_Parser::LoadCrossRefTable;
  using CPDF_Parser::ParseLinearizedHeader;
  using CPDF_Parser::ParseStartXRef;
  using CPDF_Parser::RebuildCrossRef;
  using CPDF_Parser::StartParseInternal;

  TestObjectsHolder& object_holder() { return object_holder_; }

 private:
  TestObjectsHolder object_holder_;
};

TEST(ParserTest, RebuildCrossRefCorrectly) {
  CPDF_TestParser parser;
  std::string test_file =
      PathService::GetTestFilePath("parser_rebuildxref_correct.pdf");
  ASSERT_FALSE(test_file.empty());
  ASSERT_TRUE(parser.InitTestFromFile(test_file.c_str())) << test_file;

  ASSERT_TRUE(parser.RebuildCrossRef());
  constexpr std::array<FX_FILESIZE, 7> offsets = {
      {0, 15, 61, 154, 296, 374, 450}};
  constexpr std::array<uint16_t, 7> versions = {{0, 0, 2, 4, 6, 8, 0}};
  for (size_t i = 0; i < std::size(offsets); ++i) {
    EXPECT_EQ(offsets[i], GetObjInfo(parser, i).pos);
  }
  for (size_t i = 0; i < std::size(versions); ++i) {
    EXPECT_EQ(versions[i], GetObjInfo(parser, i).gennum);
  }

  const CPDF_CrossRefTable* cross_ref_table =
      parser.GetCrossRefTableForTesting();
  ASSERT_TRUE(cross_ref_table);
  EXPECT_EQ(0u, cross_ref_table->trailer_object_number());
}

TEST(ParserTest, RebuildCrossRefFailed) {
  CPDF_TestParser parser;
  std::string test_file =
      PathService::GetTestFilePath("parser_rebuildxref_error_notrailer.pdf");
  ASSERT_FALSE(test_file.empty());
  ASSERT_TRUE(parser.InitTestFromFile(test_file.c_str())) << test_file;

  ASSERT_FALSE(parser.RebuildCrossRef());
}

TEST(ParserTest, LoadCrossRefTable) {
  {
    static const unsigned char kXrefTable[] =
        "xref \n"
        "0 6 \n"
        "0000000003 65535 f \n"
        "0000000017 00000 n \n"
        "0000000081 00000 n \n"
        "0000000000 00007 f \n"
        "0000000331 00000 n \n"
        "0000000409 00000 n \n"
        "trail";  // Needed to end cross ref table reading.
    static constexpr auto kExpected = fxcrt::ToArray<OffsetAndType>({
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {17, CPDF_CrossRefTable::ObjectType::kNormal},
        {81, CPDF_CrossRefTable::ObjectType::kNormal},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {331, CPDF_CrossRefTable::ObjectType::kNormal},
        {409, CPDF_CrossRefTable::ObjectType::kNormal},
    });
    CPDF_TestParser parser;
    ASSERT_TRUE(parser.InitTestFromBuffer(kXrefTable));
    ASSERT_TRUE(parser.LoadCrossRefTable(/*pos=*/0, /*skip=*/false));
    for (size_t i = 0; i < std::size(kExpected); ++i) {
      EXPECT_EQ(kExpected[i].offset, GetObjInfo(parser, i).pos);
      EXPECT_EQ(kExpected[i].type, GetObjInfo(parser, i).type);
    }
  }
  {
    static const unsigned char kXrefTable[] =
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
    static constexpr auto kExpected = fxcrt::ToArray<OffsetAndType>({
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {25325, CPDF_CrossRefTable::ObjectType::kNormal},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {25518, CPDF_CrossRefTable::ObjectType::kNormal},
        {25635, CPDF_CrossRefTable::ObjectType::kNormal},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {25777, CPDF_CrossRefTable::ObjectType::kNormal},
    });
    CPDF_TestParser parser;
    ASSERT_TRUE(parser.InitTestFromBuffer(kXrefTable));
    ASSERT_TRUE(parser.LoadCrossRefTable(/*pos=*/0, /*skip=*/false));
    for (size_t i = 0; i < std::size(kExpected); ++i) {
      EXPECT_EQ(kExpected[i].offset, GetObjInfo(parser, i).pos);
      EXPECT_EQ(kExpected[i].type, GetObjInfo(parser, i).type);
    }
  }
  {
    static const unsigned char kXrefTable[] =
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
    static constexpr auto kExpected = fxcrt::ToArray<OffsetAndType>({
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {25325, CPDF_CrossRefTable::ObjectType::kNormal},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {25635, CPDF_CrossRefTable::ObjectType::kNormal},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {25777, CPDF_CrossRefTable::ObjectType::kNormal},
    });
    CPDF_TestParser parser;
    ASSERT_TRUE(parser.InitTestFromBuffer(kXrefTable));
    ASSERT_TRUE(parser.LoadCrossRefTable(/*pos=*/0, /*skip=*/false));
    for (size_t i = 0; i < std::size(kExpected); ++i) {
      EXPECT_EQ(kExpected[i].offset, GetObjInfo(parser, i).pos);
      EXPECT_EQ(kExpected[i].type, GetObjInfo(parser, i).type);
    }
  }
  {
    static const unsigned char kXrefTable[] =
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
    static constexpr auto kExpected = fxcrt::ToArray<OffsetAndType>({
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {23, CPDF_CrossRefTable::ObjectType::kNormal},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {0, CPDF_CrossRefTable::ObjectType::kFree},
        {45, CPDF_CrossRefTable::ObjectType::kNormal},
        {179, CPDF_CrossRefTable::ObjectType::kNormal},
    });
    CPDF_TestParser parser;
    ASSERT_TRUE(parser.InitTestFromBuffer(kXrefTable));
    ASSERT_TRUE(parser.LoadCrossRefTable(/*pos=*/0, /*skip=*/false));
    for (size_t i = 0; i < std::size(kExpected); ++i) {
      EXPECT_EQ(kExpected[i].offset, GetObjInfo(parser, i).pos);
      EXPECT_EQ(kExpected[i].type, GetObjInfo(parser, i).type);
    }
  }
  {
    // Regression test for https://crbug.com/945624 - Make sure the parser
    // can correctly handle table sizes that are multiples of the read size,
    // which is 1024.
    std::string xref_table = "xref \n 0 2048 \n";
    xref_table.reserve(41000);
    for (int i = 0; i < 2048; ++i) {
      char buffer[21];
      snprintf(buffer, sizeof(buffer), "%010d 00000 n \n", i + 1);
      xref_table += buffer;
    }
    xref_table += "trail";  // Needed to end cross ref table reading.
    CPDF_TestParser parser;
    ASSERT_TRUE(parser.InitTestFromBuffer(
        ByteStringView(xref_table.c_str()).unsigned_span()));
    ASSERT_TRUE(parser.LoadCrossRefTable(/*pos=*/0, /*skip=*/false));
    for (size_t i = 0; i < 2048; ++i) {
      EXPECT_EQ(static_cast<int>(i) + 1, GetObjInfo(parser, i).pos);
      EXPECT_EQ(CPDF_CrossRefTable::ObjectType::kNormal,
                GetObjInfo(parser, i).type);
    }
  }
}

TEST(ParserTest, ParseStartXRef) {
  CPDF_TestParser parser;
  std::string test_file =
      PathService::GetTestFilePath("annotation_stamp_with_ap.pdf");
  ASSERT_FALSE(test_file.empty());
  ASSERT_TRUE(parser.InitTestFromFile(test_file.c_str())) << test_file;

  EXPECT_EQ(100940, parser.ParseStartXRef());
  RetainPtr<CPDF_Object> cross_ref_stream_obj =
      parser.ParseIndirectObjectAtForTesting(100940);
  ASSERT_TRUE(cross_ref_stream_obj);
  EXPECT_EQ(75u, cross_ref_stream_obj->GetObjNum());
}

TEST(ParserTest, ParseStartXRefWithHeaderOffset) {
  static constexpr FX_FILESIZE kTestHeaderOffset = 765;
  std::string test_file =
      PathService::GetTestFilePath("annotation_stamp_with_ap.pdf");
  ASSERT_FALSE(test_file.empty());
  RetainPtr<IFX_SeekableReadStream> pFileAccess =
      IFX_SeekableReadStream::CreateFromFilename(test_file.c_str());
  ASSERT_TRUE(pFileAccess);

  std::vector<unsigned char> data(pFileAccess->GetSize() + kTestHeaderOffset);
  ASSERT_TRUE(pFileAccess->ReadBlockAtOffset(
      pdfium::make_span(data).subspan(kTestHeaderOffset), 0));
  CPDF_TestParser parser;
  parser.InitTestFromBufferWithOffset(data, kTestHeaderOffset);

  EXPECT_EQ(100940, parser.ParseStartXRef());
  RetainPtr<CPDF_Object> cross_ref_stream_obj =
      parser.ParseIndirectObjectAtForTesting(100940);
  ASSERT_TRUE(cross_ref_stream_obj);
  EXPECT_EQ(75u, cross_ref_stream_obj->GetObjNum());
}

TEST(ParserTest, ParseLinearizedWithHeaderOffset) {
  static constexpr FX_FILESIZE kTestHeaderOffset = 765;
  std::string test_file = PathService::GetTestFilePath("linearized.pdf");
  ASSERT_FALSE(test_file.empty());
  RetainPtr<IFX_SeekableReadStream> pFileAccess =
      IFX_SeekableReadStream::CreateFromFilename(test_file.c_str());
  ASSERT_TRUE(pFileAccess);

  std::vector<unsigned char> data(pFileAccess->GetSize() + kTestHeaderOffset);
  ASSERT_TRUE(pFileAccess->ReadBlockAtOffset(
      pdfium::make_span(data).subspan(kTestHeaderOffset), 0));

  CPDF_TestParser parser;
  parser.InitTestFromBufferWithOffset(data, kTestHeaderOffset);
  EXPECT_TRUE(parser.ParseLinearizedHeader());

  const CPDF_CrossRefTable* cross_ref_table =
      parser.GetCrossRefTableForTesting();
  ASSERT_TRUE(cross_ref_table);
  EXPECT_EQ(0u, cross_ref_table->trailer_object_number());
}

TEST(ParserTest, BadStartXrefShouldNotBuildCrossRefTable) {
  const unsigned char kData[] =
      "%PDF1-7 0 obj <</Size 2 /W [0 0 0]\n>>\n"
      "stream\n"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "6\n"
      "%%EOF\n";
  CPDF_TestParser parser;
  ASSERT_TRUE(parser.InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::FORMAT_ERROR, parser.StartParseInternal());
  ASSERT_TRUE(parser.GetCrossRefTableForTesting());
  EXPECT_EQ(0u, parser.GetCrossRefTableForTesting()->objects_info().size());
}

class ParserXRefTest : public testing::Test {
 public:
  ParserXRefTest() = default;
  ~ParserXRefTest() override = default;

  // testing::Test:
  void SetUp() override {
    // Satisfy CPDF_Parser's checks, so the test data below can concentrate on
    // the /XRef stream and avoid also providing other valid dictionaries.
    dummy_root_ = pdfium::MakeRetain<CPDF_Dictionary>();
    EXPECT_CALL(parser().object_holder(), ParseIndirectObject)
        .WillRepeatedly(Return(dummy_root_));
  }

  CPDF_TestParser& parser() { return parser_; }

 private:
  RetainPtr<CPDF_Dictionary> dummy_root_;
  CPDF_TestParser parser_;
};

TEST_F(ParserXRefTest, XrefObjectIndicesTooBig) {
  // Since /Index starts at 4194303, the object number will go past
  // `kMaxObjectNumber`.
  static_assert(CPDF_Parser::kMaxObjectNumber == 4194304,
                "Unexpected kMaxObjectNumber");
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Index [4194303 3]\n"
      "  /Root 1 0 R\n"
      "  /Size 4194306\n"
      "  /W [1 1 1]\n"
      ">>\n"
      "stream\n"
      "01 00 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";
  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_FALSE(parser().xref_table_rebuilt());
  ASSERT_TRUE(parser().GetCrossRefTableForTesting());
  const auto& objects_info =
      parser().GetCrossRefTableForTesting()->objects_info();

  // This should be the only object from table. Subsequent objects have object
  // numbers that are too big.
  CPDF_CrossRefTable::ObjectInfo only_valid_object = {
      .type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 0};

  // TODO(thestig): Should the xref table contain object 4194305?
  // Consider reworking CPDF_Parser's object representation to avoid having to
  // store this placeholder object.
  CPDF_CrossRefTable::ObjectInfo placeholder_object = {
      .type = CPDF_CrossRefTable::ObjectType::kFree, .pos = 0};

  EXPECT_THAT(objects_info, ElementsAre(Pair(4194303, only_valid_object),
                                        Pair(4194305, placeholder_object)));
}

TEST_F(ParserXRefTest, XrefHasInvalidArchiveObjectNumber) {
  // 0xFF in the first object in the xref object stream is invalid.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 3\n"
      "  /W [1 1 1]\n"
      ">>\n"
      "stream\n"
      "02 FF 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";
  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_FALSE(parser().xref_table_rebuilt());

  const CPDF_CrossRefTable* cross_ref_table =
      parser().GetCrossRefTableForTesting();
  ASSERT_TRUE(cross_ref_table);
  EXPECT_EQ(7u, cross_ref_table->trailer_object_number());
  const auto& objects_info = cross_ref_table->objects_info();

  // The expectation is for the parser to skip over the first object, and
  // continue parsing the remaining objects. So these are the second and third
  // objects.
  CPDF_CrossRefTable::ObjectInfo expected_objects[2] = {
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 15},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 18}};

  EXPECT_THAT(objects_info, ElementsAre(Pair(1, expected_objects[0]),
                                        Pair(2, expected_objects[1])));
}

TEST_F(ParserXRefTest, XrefHasInvalidObjectType) {
  // The XRef object is a dictionary and not a stream.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 3\n"
      "  /W [1 1 1]\n"
      ">>\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::FORMAT_ERROR, parser().StartParseInternal());
}

TEST_F(ParserXRefTest, XrefHasInvalidPrevValue) {
  // The /Prev value is an absolute offset, so it should never be negative.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 3\n"
      "  /W [1 1 1]\n"
      "  /Prev -1\n"
      ">>\n"
      "stream\n"
      "02 FF 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::FORMAT_ERROR, parser().StartParseInternal());
}

TEST_F(ParserXRefTest, XrefHasInvalidSizeValue) {
  // The /Size value should never be negative.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 3\n"
      "  /W [1 1 1]\n"
      "  /Size -1\n"
      ">>\n"
      "stream\n"
      "02 FF 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::FORMAT_ERROR, parser().StartParseInternal());
}

TEST_F(ParserXRefTest, XrefHasInvalidWidth) {
  // The /W array needs to have at least 3 values.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 3\n"
      "  /W [1 1]\n"
      ">>\n"
      "stream\n"
      "02 FF 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));

  // StartParseInternal() succeeded not because XRef parsing succeeded, but
  // because RebuildCrossRef() got lucky with the data stream. Therefore, don't
  // bother checking the garbage output.
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_TRUE(parser().xref_table_rebuilt());
}

TEST_F(ParserXRefTest, XrefFirstWidthEntryIsZero) {
  // When the first /W array entry is 0, it implies the objects are all of the
  // normal type.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 2\n"
      "  /W [0 1 1]\n"
      ">>\n"
      "stream\n"
      "0F 00\n"
      "12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_FALSE(parser().xref_table_rebuilt());
  ASSERT_TRUE(parser().GetCrossRefTableForTesting());
  const auto& objects_info =
      parser().GetCrossRefTableForTesting()->objects_info();

  CPDF_CrossRefTable::ObjectInfo expected_result[2] = {
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 15},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 18}};

  EXPECT_THAT(objects_info, ElementsAre(Pair(0, expected_result[0]),
                                        Pair(1, expected_result[1])));
}

TEST_F(ParserXRefTest, XrefWithValidIndex) {
  // The /Index specifies objects (2), (4, 5), (80, 81, 82).
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 83\n"
      "  /Index [2 1 4 2 80 3]\n"
      "  /W [1 1 1]\n"
      ">>\n"
      "stream\n"
      "01 00 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "01 20 00\n"
      "01 22 00\n"
      "01 25 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_FALSE(parser().xref_table_rebuilt());
  ASSERT_TRUE(parser().GetCrossRefTableForTesting());
  const auto& objects_info =
      parser().GetCrossRefTableForTesting()->objects_info();

  CPDF_CrossRefTable::ObjectInfo expected_result[6] = {
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 0},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 15},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 18},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 32},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 34},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 37}};

  EXPECT_THAT(
      objects_info,
      ElementsAre(Pair(2, expected_result[0]), Pair(4, expected_result[1]),
                  Pair(5, expected_result[2]), Pair(80, expected_result[3]),
                  Pair(81, expected_result[4]), Pair(82, expected_result[5])));
}

TEST_F(ParserXRefTest, XrefIndexWithRepeatedObject) {
  // The /Index specifies objects (2, 3), (3). AKA the sub-sections overlap.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 4\n"
      "  /Index [2 2 3 1]\n"
      "  /W [1 1 1]\n"
      ">>\n"
      "stream\n"
      "01 00 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_FALSE(parser().xref_table_rebuilt());
  ASSERT_TRUE(parser().GetCrossRefTableForTesting());
  const auto& objects_info =
      parser().GetCrossRefTableForTesting()->objects_info();

  CPDF_CrossRefTable::ObjectInfo expected_result[2] = {
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 0},
      // Since the /Index does not follow the spec, this is one of the 2
      // possible values that a parser can come up with.
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 18}};

  EXPECT_THAT(objects_info, ElementsAre(Pair(2, expected_result[0]),
                                        Pair(3, expected_result[1])));
}

TEST_F(ParserXRefTest, XrefIndexWithOutOfOrderObjects) {
  // The /Index specifies objects (3, 4), (2), which is not in ascending order.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 5\n"
      "  /Index [3 2 2 1]\n"
      "  /W [1 1 1]\n"
      ">>\n"
      "stream\n"
      "01 00 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_FALSE(parser().xref_table_rebuilt());
  ASSERT_TRUE(parser().GetCrossRefTableForTesting());
  const auto& objects_info =
      parser().GetCrossRefTableForTesting()->objects_info();

  // Although the /Index does not follow the spec, the parser tolerates it.
  CPDF_CrossRefTable::ObjectInfo expected_result[3] = {
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 18},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 0},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 15}};

  EXPECT_THAT(objects_info, ElementsAre(Pair(2, expected_result[0]),
                                        Pair(3, expected_result[1]),
                                        Pair(4, expected_result[2])));
}

TEST_F(ParserXRefTest, XrefWithIndexAndWrongSize) {
  // The /Index specifies objects (2), (80, 81), so the /Size should be 82,
  // but is actually 81.
  const unsigned char kData[] =
      "%PDF1-7\n%\xa0\xf2\xa4\xf4\n"
      "7 0 obj <<\n"
      "  /Filter /ASCIIHexDecode\n"
      "  /Root 1 0 R\n"
      "  /Size 81\n"
      "  /Index [2 1 80 2]\n"
      "  /W [1 1 1]\n"
      ">>\n"
      "stream\n"
      "01 00 00\n"
      "01 0F 00\n"
      "01 12 00\n"
      "endstream\n"
      "endobj\n"
      "startxref\n"
      "14\n"
      "%%EOF\n";

  ASSERT_TRUE(parser().InitTestFromBuffer(kData));
  EXPECT_EQ(CPDF_Parser::SUCCESS, parser().StartParseInternal());
  EXPECT_FALSE(parser().xref_table_rebuilt());
  ASSERT_TRUE(parser().GetCrossRefTableForTesting());
  const auto& objects_info =
      parser().GetCrossRefTableForTesting()->objects_info();

  const CPDF_CrossRefTable::ObjectInfo expected_result[3] = {
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 0},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 15},
      {.type = CPDF_CrossRefTable::ObjectType::kNormal, .pos = 18}};

  EXPECT_THAT(objects_info, ElementsAre(Pair(2, expected_result[0]),
                                        Pair(80, expected_result[1]),
                                        Pair(81, expected_result[2])));
}
