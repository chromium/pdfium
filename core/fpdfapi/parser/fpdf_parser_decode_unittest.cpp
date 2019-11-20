// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/fpdf_parser_decode.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

TEST(fpdf_parser_decode, ValidateDecoderPipeline) {
  {
    // Empty decoder list is always valid.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    EXPECT_TRUE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // 1 decoder is almost always valid.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("FlateEncode");
    EXPECT_TRUE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // 1 decoder is almost always valid, even with an unknown decoder.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("FooBar");
    EXPECT_TRUE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Valid 2 decoder pipeline.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("AHx");
    decoders->AddNew<CPDF_Name>("LZWDecode");
    EXPECT_TRUE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Valid 2 decoder pipeline.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("ASCII85Decode");
    decoders->AddNew<CPDF_Name>("ASCII85Decode");
    EXPECT_TRUE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Valid 5 decoder pipeline.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("ASCII85Decode");
    decoders->AddNew<CPDF_Name>("A85");
    decoders->AddNew<CPDF_Name>("RunLengthDecode");
    decoders->AddNew<CPDF_Name>("FlateDecode");
    decoders->AddNew<CPDF_Name>("RL");
    EXPECT_TRUE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Valid 5 decoder pipeline, with an image decoder at the end.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("RunLengthDecode");
    decoders->AddNew<CPDF_Name>("ASCII85Decode");
    decoders->AddNew<CPDF_Name>("FlateDecode");
    decoders->AddNew<CPDF_Name>("LZW");
    decoders->AddNew<CPDF_Name>("DCTDecode");
    EXPECT_TRUE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Invalid 1 decoder pipeline due to wrong type.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_String>("FlateEncode", false);
    EXPECT_FALSE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Invalid 2 decoder pipeline, with 2 image decoders.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("DCTDecode");
    decoders->AddNew<CPDF_Name>("CCITTFaxDecode");
    EXPECT_FALSE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Invalid 2 decoder pipeline, with 1 image decoder at the start.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("DCTDecode");
    decoders->AddNew<CPDF_Name>("FlateDecode");
    EXPECT_FALSE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Invalid 2 decoder pipeline due to wrong type.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_String>("AHx", false);
    decoders->AddNew<CPDF_Name>("LZWDecode");
    EXPECT_FALSE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Invalid 5 decoder pipeline.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("FlateDecode");
    decoders->AddNew<CPDF_Name>("FlateDecode");
    decoders->AddNew<CPDF_Name>("DCTDecode");
    decoders->AddNew<CPDF_Name>("FlateDecode");
    decoders->AddNew<CPDF_Name>("FlateDecode");
    EXPECT_FALSE(ValidateDecoderPipeline(decoders.Get()));
  }
  {
    // Invalid 5 decoder pipeline due to wrong type.
    auto decoders = pdfium::MakeRetain<CPDF_Array>();
    decoders->AddNew<CPDF_Name>("ASCII85Decode");
    decoders->AddNew<CPDF_Name>("A85");
    decoders->AddNew<CPDF_Name>("RunLengthDecode");
    decoders->AddNew<CPDF_Name>("FlateDecode");
    decoders->AddNew<CPDF_String>("RL", false);
    EXPECT_FALSE(ValidateDecoderPipeline(decoders.Get()));
  }
}

TEST(fpdf_parser_decode, A85Decode) {
  const pdfium::DecodeTestData kTestData[] = {
      // Empty src string.
      STR_IN_OUT_CASE("", "", 0),
      // Empty content in src string.
      STR_IN_OUT_CASE("~>", "", 0),
      // Regular conversion.
      STR_IN_OUT_CASE("FCfN8~>", "test", 7),
      // End at the ending mark.
      STR_IN_OUT_CASE("FCfN8~>FCfN8", "test", 7),
      // Skip whitespaces.
      STR_IN_OUT_CASE("\t F C\r\n \tf N 8 ~>", "test", 17),
      // No ending mark.
      STR_IN_OUT_CASE("@3B0)DJj_BF*)>@Gp#-s", "a funny story :)", 20),
      // Non-multiple length.
      STR_IN_OUT_CASE("12A", "2k", 3),
      // Stop at unknown characters.
      STR_IN_OUT_CASE("FCfN8FCfN8vw", "testtest", 11),
  };
  for (const auto& test_case : kTestData) {
    std::unique_ptr<uint8_t, FxFreeDeleter> result;
    uint32_t result_size = 0;
    EXPECT_EQ(test_case.processed_size,
              A85Decode({test_case.input, test_case.input_size}, &result,
                        &result_size))
        << "for case " << test_case.input;
    ASSERT_EQ(test_case.expected_size, result_size);
    const uint8_t* result_ptr = result.get();
    for (size_t j = 0; j < result_size; ++j) {
      EXPECT_EQ(test_case.expected[j], result_ptr[j])
          << "for case " << test_case.input << " char " << j;
    }
  }
}

TEST(fpdf_parser_decode, HexDecode) {
  const pdfium::DecodeTestData kTestData[] = {
      // Empty src string.
      STR_IN_OUT_CASE("", "", 0),
      // Empty content in src string.
      STR_IN_OUT_CASE(">", "", 1),
      // Only whitespaces in src string.
      STR_IN_OUT_CASE("\t   \r\n>", "", 7),
      // Regular conversion.
      STR_IN_OUT_CASE("12Ac>zzz", "\x12\xac", 5),
      // Skip whitespaces.
      STR_IN_OUT_CASE("12 Ac\t02\r\nBF>zzz>", "\x12\xac\x02\xbf", 13),
      // Non-multiple length.
      STR_IN_OUT_CASE("12A>zzz", "\x12\xa0", 4),
      // Skips unknown characters.
      STR_IN_OUT_CASE("12tk  \tAc>zzz", "\x12\xac", 10),
      // No ending mark.
      STR_IN_OUT_CASE("12AcED3c3456", "\x12\xac\xed\x3c\x34\x56", 12),
  };
  for (const auto& test_case : kTestData) {
    std::unique_ptr<uint8_t, FxFreeDeleter> result;
    uint32_t result_size = 0;
    EXPECT_EQ(test_case.processed_size,
              HexDecode({test_case.input, test_case.input_size}, &result,
                        &result_size))
        << "for case " << test_case.input;
    ASSERT_EQ(test_case.expected_size, result_size);
    const uint8_t* result_ptr = result.get();
    for (size_t j = 0; j < result_size; ++j) {
      EXPECT_EQ(test_case.expected[j], result_ptr[j])
          << "for case " << test_case.input << " char " << j;
    }
  }
}

TEST(fpdf_parser_decode, DecodeText) {
  const struct DecodeTestData {
    const char* input;
    size_t input_length;
    const wchar_t* expected_output;
    size_t expected_length;
  } kTestData[] = {
      // Empty src string.
      {"", 0, L"", 0},
      // ASCII text.
      {"the quick\tfox", 13, L"the quick\tfox", 13},
      // Unicode text.
      {"\xFE\xFF\x03\x30\x03\x31", 6, L"\x0330\x0331", 2},
      // More Unicode text.
      {"\xFE\xFF\x7F\x51\x98\x75\x00\x20\x56\xFE\x72\x47\x00"
       "\x20\x8D\x44\x8B\xAF\x66\xF4\x59\x1A\x00\x20\x00\xBB",
       26,
       L"\x7F51\x9875\x0020\x56FE\x7247\x0020"
       L"\x8D44\x8BAF\x66F4\x591A\x0020\x00BB",
       12},
      // Unicode escape sequence. https://crbug.com/pdfium/182
      {"\xFE\xFF\x00\x1B\x6A\x61\x00\x1B\x00\x20\x53\x70\x52\x37", 14,
       L"\x0020\x5370\x5237", 3},
      {"\xFE\xFF\x00\x1B\x6A\x61\x00\x1B\x00\x20\x53\x70\x52\x37\x29", 15,
       L"\x0020\x5370\x5237", 3},
      {"\xFE\xFF\x00\x1B\x6A\x61\x4A\x50\x00\x1B\x00\x20\x53\x70\x52\x37", 16,
       L"\x0020\x5370\x5237", 3},
      {"\xFE\xFF\x00\x20\x00\x1B\x6A\x61\x4A\x50\x00\x1B\x52\x37", 14,
       L"\x0020\x5237", 2},
      // https://crbug.com/1001159
      {"\xFE\xFF\x00\x1B\x00\x1B", 6, L"", 0},
      {"\xFE\xFF\x00\x1B\x00\x1B\x20", 7, L"", 0},
      {"\xFE\xFF\x00\x1B\x00\x1B\x00\x20", 8, L"\x0020", 1},
  };

  for (const auto& test_case : kTestData) {
    WideString output = PDF_DecodeText(
        pdfium::make_span(reinterpret_cast<const uint8_t*>(test_case.input),
                          test_case.input_length));
    ASSERT_EQ(test_case.expected_length, output.GetLength())
        << "for case " << test_case.input;
    const wchar_t* str_ptr = output.c_str();
    for (size_t i = 0; i < test_case.expected_length; ++i) {
      EXPECT_EQ(test_case.expected_output[i], str_ptr[i])
          << "for case " << test_case.input << " char " << i;
    }
  }
}

TEST(fpdf_parser_decode, EncodeText) {
  const struct EncodeTestData {
    const wchar_t* input;
    const char* expected_output;
    size_t expected_length;
  } kTestData[] = {
      // Empty src string.
      {L"", "", 0},
      // ASCII text.
      {L"the quick\tfox", "the quick\tfox", 13},
      // Unicode text.
      {L"\x0330\x0331", "\xFE\xFF\x03\x30\x03\x31", 6},
      // More Unicode text.
      {L"\x7F51\x9875\x0020\x56FE\x7247\x0020"
       L"\x8D44\x8BAF\x66F4\x591A\x0020\x00BB",
       "\xFE\xFF\x7F\x51\x98\x75\x00\x20\x56\xFE\x72\x47\x00"
       "\x20\x8D\x44\x8B\xAF\x66\xF4\x59\x1A\x00\x20\x00\xBB",
       26},
  };

  for (const auto& test_case : kTestData) {
    ByteString output = PDF_EncodeText(test_case.input);
    ASSERT_EQ(test_case.expected_length, output.GetLength())
        << "for case " << test_case.input;
    const char* str_ptr = output.c_str();
    for (size_t j = 0; j < test_case.expected_length; ++j) {
      EXPECT_EQ(test_case.expected_output[j], str_ptr[j])
          << "for case " << test_case.input << " char " << j;
    }
  }
}
