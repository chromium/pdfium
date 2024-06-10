// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/flate/flatemodule.h"

#include "core/fxcodec/data_and_bytes_consumed.h"
#include "core/fxcrt/compiler_specific.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

using testing::ElementsAreArray;

// NOTE: python's zlib.compress() and zlib.decompress() may be useful for
// external validation of the FlateDncode/FlateEecode test cases.
TEST(FlateModule, Decode) {
  static const pdfium::DecodeTestData flate_decode_cases[] = {
      STR_IN_OUT_CASE("", "", 0),
      STR_IN_OUT_CASE("preposterous nonsense", "", 2),
      STR_IN_OUT_CASE("\x78\x9c\x03\x00\x00\x00\x00\x01", "", 8),
      STR_IN_OUT_CASE("\x78\x9c\x53\x00\x00\x00\x21\x00\x21", " ", 9),
      STR_IN_OUT_CASE("\x78\x9c\x33\x34\x32\x06\x00\01\x2d\x00\x97", "123", 11),
      STR_IN_OUT_CASE("\x78\x9c\x63\xf8\x0f\x00\x01\x01\x01\x00", "\x00\xff",
                      10),
      STR_IN_OUT_CASE(
          "\x78\x9c\x33\x54\x30\x00\x42\x5d\x43\x05\x23\x4b\x05\x73\x33\x63"
          "\x85\xe4\x5c\x2e\x90\x80\xa9\xa9\xa9\x82\xb9\xb1\xa9\x42\x51\x2a"
          "\x57\xb8\x42\x1e\x57\x21\x92\xa0\x89\x9e\xb1\xa5\x09\x92\x84\x9e"
          "\x85\x81\x81\x25\xd8\x14\x24\x26\xd0\x18\x43\x05\x10\x0c\x72\x57"
          "\x80\x30\x8a\xd2\xb9\xf4\xdd\x0d\x14\xd2\x8b\xc1\x46\x99\x59\x1a"
          "\x2b\x58\x1a\x9a\x83\x8c\x49\xe3\x0a\x04\x42\x00\x37\x4c\x1b\x42",
          "1 0 0 -1 29 763 cm\n0 0 555 735 re\nW n\nq\n0 0 555 734.394 re\n"
          "W n\nq\n0.8009 0 0 0.8009 0 0 cm\n1 1 1 RG 1 1 1 rg\n/G0 gs\n"
          "0 0 693 917 re\nf\nQ\nQ\n",
          96),
  };
  size_t i = 0;
  for (const pdfium::DecodeTestData& data : flate_decode_cases) {
    DataAndBytesConsumed result = FlateModule::FlateOrLZWDecode(
        false, UNSAFE_TODO(pdfium::make_span(data.input, data.input_size)),
        false, 0, 0, 0, 0, 0);
    EXPECT_EQ(data.processed_size, result.bytes_consumed) << " for case " << i;
    EXPECT_THAT(result.data, ElementsAreArray(data.expected_span()))
        << " for case " << i;
    ++i;
  }
}

TEST(FlateModule, Encode) {
  static const pdfium::StrFuncTestData flate_encode_cases[] = {
      STR_IN_OUT_CASE("", "\x78\x9c\x03\x00\x00\x00\x00\x01"),
      STR_IN_OUT_CASE(" ", "\x78\x9c\x53\x00\x00\x00\x21\x00\x21"),
      STR_IN_OUT_CASE("123", "\x78\x9c\x33\x34\x32\x06\x00\01\x2d\x00\x97"),
      STR_IN_OUT_CASE("\x00\xff", "\x78\x9c\x63\xf8\x0f\x00\x01\x01\x01\x00"),
      STR_IN_OUT_CASE(
          "1 0 0 -1 29 763 cm\n0 0 555 735 re\nW n\nq\n0 0 555 734.394 re\n"
          "W n\nq\n0.8009 0 0 0.8009 0 0 cm\n1 1 1 RG 1 1 1 rg\n/G0 gs\n"
          "0 0 693 917 re\nf\nQ\nQ\n",
          "\x78\x9c\x4d\x8c\x3b\x0e\x80\x20\x10\x05\xfb\x3d\xc5\xbb\x80\xb8"
          "\xc8\xcf\x3d\x01\x35\x36\x1e\x80\x28\x15\x26\xe2\xfd\x13\x83\x36"
          "\xe4\x35\x93\x79\xc9\x68\x30\x18\x93\xc6\x22\x08\xde\x20\x57\xea"
          "\xc2\x39\x87\x60\x1c\xda\x41\x3b\x2e\xba\x07\x69\x95\x11\x3b\x1c"
          "\x6a\x65\x96\xaf\x32\x60\xae\xa4\xd1\xb7\x45\xfc\xd0\x0a\xcd\x91"
          "\x51\x9e\x2f\xe5\xc5\x40\x74\xe8\x99\x93\x12\x25\x7a\x01\x37\x4c"
          "\x1b\x42"),
  };

  size_t i = 0;
  for (const pdfium::StrFuncTestData& data : flate_encode_cases) {
    DataVector<uint8_t> result = FlateModule::Encode(data.input_span());
    EXPECT_THAT(result, ElementsAreArray(data.expected_span()))
        << " for case " << i;
    ++i;
  }
}
