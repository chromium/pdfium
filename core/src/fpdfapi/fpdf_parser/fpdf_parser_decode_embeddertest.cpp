// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <string>

#include "../../../../public/fpdfview.h"
#include "../../../../testing/fx_string_testhelpers.h"
#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fxcrt/fx_basic.h"
#include "../../../testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFParserDecodeEmbeddertest : public EmbedderTest {
};

// NOTE: python's zlib.compress() and zlib.decompress() may be useful for
// external validation of the FlateEncode/FlateDecode test cases.

#define TEST_CASE(input_literal, expected_literal) \
  { (const unsigned char*)input_literal, sizeof(input_literal) - 1, \
    (const unsigned char*)expected_literal, sizeof(expected_literal) - 1 }

TEST_F(FPDFParserDecodeEmbeddertest, FlateEncode) {
  struct FlateEncodeCase {
    const unsigned char* input;
    unsigned int input_size;
    const unsigned char* expected;
    unsigned int expected_size;
  } flate_encode_cases[] = {
    TEST_CASE("", "\x78\x9c\x03\x00\x00\x00\x00\x01"),
    TEST_CASE(" ", "\x78\x9c\x53\x00\x00\x00\x21\x00\x21"),
    TEST_CASE("123", "\x78\x9c\x33\x34\x32\x06\x00\01\x2d\x00\x97"),
    TEST_CASE("\x00\xff", "\x78\x9c\x63\xf8\x0f\x00\x01\x01\x01\x00"),
    TEST_CASE("1 0 0 -1 29 763 cm\n0 0 555 735 re\nW n\nq\n0 0 555 734.394 re\n"
              "W n\nq\n0.8009 0 0 0.8009 0 0 cm\n1 1 1 RG 1 1 1 rg\n/G0 gs\n"
              "0 0 693 917 re\nf\nQ\nQ\n"
              ,
              "\x78\x9c\x33\x54\x30\x00\x42\x5d\x43\x05\x23\x4b\x05\x73\x33\x63"
              "\x85\xe4\x5c\x2e\x90\x80\xa9\xa9\xa9\x82\xb9\xb1\xa9\x42\x51\x2a"
              "\x57\xb8\x42\x1e\x57\x21\x92\xa0\x89\x9e\xb1\xa5\x09\x92\x84\x9e"
              "\x85\x81\x81\x25\xd8\x14\x24\x26\xd0\x18\x43\x05\x10\x0c\x72\x57"
              "\x80\x30\x8a\xd2\xb9\xf4\xdd\x0d\x14\xd2\x8b\xc1\x46\x99\x59\x1a"
              "\x2b\x58\x1a\x9a\x83\x8c\x49\xe3\x0a\x04\x42\x00\x37\x4c\x1b\x42"
              ),
  };

  for (size_t i = 0; i < FX_ArraySize(flate_encode_cases); ++i) {
    FlateEncodeCase* ptr = &flate_encode_cases[i];
    unsigned char* result;
    unsigned int result_size;
    FlateEncode(ptr->input, ptr->input_size, result, result_size);
    ASSERT_TRUE(result);
    EXPECT_EQ(std::string((const char*)ptr->expected, ptr->expected_size),
              std::string((const char*)result, result_size))
        << " for case " << i;
    FX_Free(result);
  }
}

TEST_F(FPDFParserDecodeEmbeddertest, FlateDecode) {
  struct FlateDecodeCase {
    const unsigned char* input;
    unsigned int input_size;
    const unsigned char* expected;
    unsigned int expected_size;
  } flate_decode_cases[] = {
    TEST_CASE("", ""),
    TEST_CASE("preposterous nonsense", ""),
    TEST_CASE("\x78\x9c\x03\x00\x00\x00\x00\x01", ""),
    TEST_CASE("\x78\x9c\x53\x00\x00\x00\x21\x00\x21", " "),
    TEST_CASE("\x78\x9c\x33\x34\x32\x06\x00\01\x2d\x00\x97", "123"),
    TEST_CASE("\x78\x9c\x63\xf8\x0f\x00\x01\x01\x01\x00", "\x00\xff"),
    TEST_CASE("\x78\x9c\x33\x54\x30\x00\x42\x5d\x43\x05\x23\x4b\x05\x73\x33\x63"
              "\x85\xe4\x5c\x2e\x90\x80\xa9\xa9\xa9\x82\xb9\xb1\xa9\x42\x51\x2a"
              "\x57\xb8\x42\x1e\x57\x21\x92\xa0\x89\x9e\xb1\xa5\x09\x92\x84\x9e"
              "\x85\x81\x81\x25\xd8\x14\x24\x26\xd0\x18\x43\x05\x10\x0c\x72\x57"
              "\x80\x30\x8a\xd2\xb9\xf4\xdd\x0d\x14\xd2\x8b\xc1\x46\x99\x59\x1a"
              "\x2b\x58\x1a\x9a\x83\x8c\x49\xe3\x0a\x04\x42\x00\x37\x4c\x1b\x42"
              ,
              "1 0 0 -1 29 763 cm\n0 0 555 735 re\nW n\nq\n0 0 555 734.394 re\n"
              "W n\nq\n0.8009 0 0 0.8009 0 0 cm\n1 1 1 RG 1 1 1 rg\n/G0 gs\n"
              "0 0 693 917 re\nf\nQ\nQ\n"
              ),
  };

  for (size_t i = 0; i < FX_ArraySize(flate_decode_cases); ++i) {
    FlateDecodeCase* ptr = &flate_decode_cases[i];
    unsigned char* result;
    unsigned int result_size;
    FlateDecode(ptr->input, ptr->input_size, result, result_size);
    ASSERT_TRUE(result);
    EXPECT_EQ(std::string((const char*)ptr->expected, ptr->expected_size),
              std::string((const char*)result, result_size))
        << " for case " << i;
    FX_Free(result);
  }
}


#undef TEST_CASE
