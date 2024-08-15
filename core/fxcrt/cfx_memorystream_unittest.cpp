// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_memorystream.h"

#include "core/fxcrt/retain_ptr.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CFXMemoryStreamTest, ReadBlockAtOffset) {
  auto stream = pdfium::MakeRetain<CFX_MemoryStream>();
  const uint8_t kData1[] = {'a', 'b', 'c', 'd', 'e', 'f'};
  ASSERT_TRUE(stream->WriteBlock(kData1));
  ASSERT_THAT(stream->GetSpan(),
              testing::ElementsAre('a', 'b', 'c', 'd', 'e', 'f'));

  uint8_t buffer[3];
  ASSERT_TRUE(stream->ReadBlockAtOffset(buffer, 2));
  ASSERT_THAT(buffer, testing::ElementsAre('c', 'd', 'e'));

  ASSERT_TRUE(stream->ReadBlockAtOffset(buffer, 0));
  ASSERT_THAT(buffer, testing::ElementsAre('a', 'b', 'c'));

  ASSERT_TRUE(stream->ReadBlockAtOffset(buffer, 3));
  ASSERT_THAT(buffer, testing::ElementsAre('d', 'e', 'f'));

  ASSERT_FALSE(stream->ReadBlockAtOffset(buffer, 4));
  ASSERT_THAT(buffer, testing::ElementsAre('d', 'e', 'f'));
}

TEST(CFXMemoryStreamTest, WriteBlock) {
  auto stream = pdfium::MakeRetain<CFX_MemoryStream>();
  ASSERT_TRUE(stream->WriteBlock({}));
  ASSERT_TRUE(stream->GetSpan().empty());

  const uint8_t kData1[] = {'a', 'b', 'c'};
  ASSERT_TRUE(stream->WriteBlock(kData1));
  ASSERT_THAT(stream->GetSpan(), testing::ElementsAre('a', 'b', 'c'));

  ASSERT_TRUE(stream->WriteBlock(kData1));
  ASSERT_THAT(stream->GetSpan(),
              testing::ElementsAre('a', 'b', 'c', 'a', 'b', 'c'));

  ASSERT_TRUE(stream->WriteBlock({}));
  ASSERT_THAT(stream->GetSpan(),
              testing::ElementsAre('a', 'b', 'c', 'a', 'b', 'c'));
}

TEST(CFXMemoryStreamTest, WriteZeroBytes) {
  auto stream = pdfium::MakeRetain<CFX_MemoryStream>();
  const uint8_t kData1[] = {'a', 'b', 'c'};
  ASSERT_TRUE(stream->WriteBlock(kData1));
  ASSERT_THAT(stream->GetSpan(), testing::ElementsAre('a', 'b', 'c'));

  ASSERT_TRUE(stream->WriteBlock({}));
  ASSERT_THAT(stream->GetSpan(), testing::ElementsAre('a', 'b', 'c'));
}
