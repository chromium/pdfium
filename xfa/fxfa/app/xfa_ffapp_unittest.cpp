// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/xfa_ffapp.h"

#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/fx_memory.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

using UniqueFileRead =
    std::unique_ptr<CXFA_FileRead, ReleaseDeleter<CXFA_FileRead>>;

TEST(CXFAFileRead, NoStreams) {
  std::vector<CPDF_Stream*> streams;
  UniqueFileRead fileread(new CXFA_FileRead(streams));

  uint8_t output_buffer[16];
  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_FALSE(fileread->ReadBlock(output_buffer, 0, 0));
  EXPECT_EQ(0xbd, output_buffer[0]);
}

TEST(CXFAFileRead, EmptyStreams) {
  std::vector<CPDF_Stream*> streams;
  std::unique_ptr<CPDF_Stream> stream1 = pdfium::MakeUnique<CPDF_Stream>();
  streams.push_back(stream1.get());
  UniqueFileRead fileread(new CXFA_FileRead(streams));

  uint8_t output_buffer[16];
  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_FALSE(fileread->ReadBlock(output_buffer, 0, 0));
  EXPECT_EQ(0xbd, output_buffer[0]);
}

TEST(CXFAFileRead, NormalStreams) {
  std::vector<CPDF_Stream*> streams;
  std::unique_ptr<CPDF_Stream> stream1 = pdfium::MakeUnique<CPDF_Stream>();
  std::unique_ptr<CPDF_Stream> stream2 = pdfium::MakeUnique<CPDF_Stream>();
  std::unique_ptr<CPDF_Stream> stream3 = pdfium::MakeUnique<CPDF_Stream>();

  // 16 chars total.
  stream1->InitStream(reinterpret_cast<const uint8_t*>("one t"), 5,
                      pdfium::MakeUnique<CPDF_Dictionary>());
  stream2->InitStream(reinterpret_cast<const uint8_t*>("wo "), 3,
                      pdfium::MakeUnique<CPDF_Dictionary>());
  stream3->InitStream(reinterpret_cast<const uint8_t*>("three!!!"), 8,
                      pdfium::MakeUnique<CPDF_Dictionary>());

  streams.push_back(stream1.get());
  streams.push_back(stream2.get());
  streams.push_back(stream3.get());
  UniqueFileRead fileread(new CXFA_FileRead(streams));

  uint8_t output_buffer[16];
  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_TRUE(fileread->ReadBlock(output_buffer, 0, 0));
  EXPECT_EQ(0xbd, output_buffer[0]);

  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_TRUE(fileread->ReadBlock(output_buffer, 1, 0));
  EXPECT_EQ(0xbd, output_buffer[0]);

  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_TRUE(fileread->ReadBlock(output_buffer, 0, 1));
  EXPECT_EQ(0, memcmp(output_buffer, "o", 1));
  EXPECT_EQ(0xbd, output_buffer[1]);

  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_TRUE(fileread->ReadBlock(output_buffer, 0, sizeof(output_buffer)));
  EXPECT_EQ(0, memcmp(output_buffer, "one two three!!!", 16));

  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_TRUE(fileread->ReadBlock(output_buffer, 2, 10));
  EXPECT_EQ(0, memcmp(output_buffer, "e two thre", 10));
  EXPECT_EQ(0xbd, output_buffer[11]);

  memset(output_buffer, 0xbd, sizeof(output_buffer));
  EXPECT_FALSE(fileread->ReadBlock(output_buffer, 1, sizeof(output_buffer)));
  EXPECT_EQ(0, memcmp(output_buffer, "ne two three!!!", 15));
  EXPECT_EQ(0xbd, output_buffer[15]);
}
