// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_memorystream.h"

#include "core/fxcrt/retain_ptr.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const char kSomeText[] = "Lets make holes in streams";
const size_t kSomeTextLen = sizeof(kSomeText) - 1;

}  // namespace

TEST(CFXMemoryStreamTest, SparseBlockWrites) {
  auto stream = pdfium::MakeRetain<CFX_MemoryStream>();
  for (FX_FILESIZE offset = 0; offset <= 200000; offset += 100000)
    stream->WriteBlockAtOffset(kSomeText, offset, kSomeTextLen);

  EXPECT_EQ(200000 + kSomeTextLen, static_cast<size_t>(stream->GetSize()));
}

TEST(CFXMemoryStreamTest, OverlappingBlockWrites) {
  auto stream = pdfium::MakeRetain<CFX_MemoryStream>();
  for (FX_FILESIZE offset = 0; offset <= 100; ++offset)
    stream->WriteBlockAtOffset(kSomeText, offset, kSomeTextLen);

  EXPECT_EQ(100 + kSomeTextLen, static_cast<size_t>(stream->GetSize()));
}

TEST(CFXMemoryStreamTest, ReadWriteBlockAtOffset) {
  auto stream = pdfium::MakeRetain<CFX_MemoryStream>();
  const uint8_t kData1[] = {'a', 'b', 'c'};
  ASSERT_TRUE(stream->WriteBlock(kData1, sizeof(kData1)));
  ASSERT_THAT(stream->GetSpan(), testing::ElementsAre('a', 'b', 'c'));

  ASSERT_TRUE(stream->WriteBlockAtOffset(kData1, 5, sizeof(kData1)));
  ASSERT_THAT(stream->GetSpan(),
              testing::ElementsAre('a', 'b', 'c', '\0', '\0', 'a', 'b', 'c'));

  uint8_t buffer[4];
  ASSERT_TRUE(stream->ReadBlockAtOffset(buffer, 2, sizeof(buffer)));
  ASSERT_THAT(buffer, testing::ElementsAre('c', '\0', '\0', 'a'));
}

TEST(CFXMemoryStreamTest, WriteZeroBytes) {
  auto stream = pdfium::MakeRetain<CFX_MemoryStream>();
  const uint8_t kData1[] = {'a', 'b', 'c'};
  ASSERT_TRUE(stream->WriteBlock(kData1, sizeof(kData1)));
  ASSERT_THAT(stream->GetSpan(), testing::ElementsAre('a', 'b', 'c'));

  ASSERT_TRUE(stream->WriteBlock(kData1, 0));
  ASSERT_THAT(stream->GetSpan(), testing::ElementsAre('a', 'b', 'c'));

  ASSERT_TRUE(stream->WriteBlock(nullptr, 0));
  ASSERT_THAT(stream->GetSpan(), testing::ElementsAre('a', 'b', 'c'));
}
