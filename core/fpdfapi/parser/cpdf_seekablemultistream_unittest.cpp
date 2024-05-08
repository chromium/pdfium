// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_seekablemultistream.h"

#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDFSeekableMultiStreamTest, NoStreams) {
  std::vector<RetainPtr<const CPDF_Stream>> streams;
  auto fileread =
      pdfium::MakeRetain<CPDF_SeekableMultiStream>(std::move(streams));

  uint8_t output_buffer[16];
  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_FALSE(fileread->ReadBlockAtOffset(
      pdfium::make_span(output_buffer).first(0u), 0));
  EXPECT_EQ(0xbd, output_buffer[0]);
}

TEST(CXFAFileReadTest, EmptyStreams) {
  std::vector<RetainPtr<const CPDF_Stream>> streams;
  streams.push_back(
      pdfium::MakeRetain<CPDF_Stream>(pdfium::MakeRetain<CPDF_Dictionary>()));

  auto fileread =
      pdfium::MakeRetain<CPDF_SeekableMultiStream>(std::move(streams));

  uint8_t output_buffer[16];
  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_FALSE(fileread->ReadBlockAtOffset(
      pdfium::make_span(output_buffer).first(0u), 0));
  EXPECT_EQ(0xbd, output_buffer[0]);
}

TEST(CXFAFileReadTest, NormalStreams) {
  // 16 chars total.
  static const char kOne[] = "one t";
  static const char kTwo[] = "wo ";
  static const char kThree[] = "three!!!";

  ByteStringView one_view(kOne);
  ByteStringView two_view(kTwo);
  ByteStringView three_view(kThree);
  auto stream1 = pdfium::MakeRetain<CPDF_Stream>(one_view.unsigned_span());
  auto stream2 = pdfium::MakeRetain<CPDF_Stream>(two_view.unsigned_span());
  auto stream3 = pdfium::MakeRetain<CPDF_Stream>(three_view.unsigned_span());

  std::vector<RetainPtr<const CPDF_Stream>> streams;
  streams.push_back(std::move(stream1));
  streams.push_back(std::move(stream2));
  streams.push_back(std::move(stream3));
  auto fileread =
      pdfium::MakeRetain<CPDF_SeekableMultiStream>(std::move(streams));

  uint8_t output_buffer[16];
  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_TRUE(fileread->ReadBlockAtOffset(
      pdfium::make_span(output_buffer).first(0u), 0));
  EXPECT_EQ(0xbd, output_buffer[0]);

  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_TRUE(fileread->ReadBlockAtOffset(
      pdfium::make_span(output_buffer).first(0u), 1));
  EXPECT_EQ(0xbd, output_buffer[0]);

  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_TRUE(fileread->ReadBlockAtOffset(
      pdfium::make_span(output_buffer).first(1u), 0));
  EXPECT_EQ(0, memcmp(output_buffer, "o", 1));
  EXPECT_EQ(0xbd, output_buffer[1]);

  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_TRUE(fileread->ReadBlockAtOffset(output_buffer, 0));
  EXPECT_EQ(0, memcmp(output_buffer, "one two three!!!", 16));

  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_TRUE(fileread->ReadBlockAtOffset(
      pdfium::make_span(output_buffer).first(10u), 2));
  EXPECT_EQ(0, memcmp(output_buffer, "e two thre", 10));
  EXPECT_EQ(0xbd, output_buffer[11]);

  fxcrt::Fill(output_buffer, 0xbd);
  EXPECT_FALSE(fileread->ReadBlockAtOffset(output_buffer, 1));
  EXPECT_EQ(0, memcmp(output_buffer, "ne two three!!!", 15));
  EXPECT_EQ(0xbd, output_buffer[15]);
}
