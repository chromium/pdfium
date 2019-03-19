// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_seekablestreamproxy.h"

#include <memory>
#include <vector>

#include "core/fxcrt/cfx_readonlymemorystream.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/span.h"

TEST(SeekableStreamProxyTest, NullStream) {
  auto proxy_stream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(
      pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(
          pdfium::make_span<const uint8_t>(nullptr, 0)));

  wchar_t buffer[16];
  EXPECT_EQ(0u, proxy_stream->ReadBlock(buffer, FX_ArraySize(buffer)));
}

TEST(SeekableStreamProxyTest, DefaultStreamBOMNotRecognized) {
  const char data[] = "abcd";
  auto proxy_stream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(
      pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(pdfium::make_span(
          reinterpret_cast<const uint8_t*>(data), sizeof(data) - 1)));

  wchar_t buffer[16];
  EXPECT_EQ(0u, proxy_stream->ReadBlock(buffer, FX_ArraySize(buffer)));
}

TEST(SeekableStreamProxyTest, UTF8Stream) {
  const char data[] = "\xEF\xBB\xBF*\xC2\xA2*";
  auto proxy_stream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(
      pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(pdfium::make_span(
          reinterpret_cast<const uint8_t*>(data), sizeof(data) - 1)));

  wchar_t buffer[16];
  EXPECT_EQ(3u, proxy_stream->ReadBlock(buffer, FX_ArraySize(buffer)));
  EXPECT_EQ(L'*', buffer[0]);
  EXPECT_EQ(L'\u00A2', buffer[1]);
  EXPECT_EQ(L'*', buffer[2]);
}

TEST(SeekableStreamProxyTest, UTF16LEStream) {
  const char data[] = "\xFF\xFE\x41\x00\x42\x01";
  auto proxy_stream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(
      pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(pdfium::make_span(
          reinterpret_cast<const uint8_t*>(data), sizeof(data) - 1)));

  wchar_t buffer[16];
  EXPECT_EQ(2u, proxy_stream->ReadBlock(buffer, FX_ArraySize(buffer)));
  EXPECT_EQ(L'A', buffer[0]);
  EXPECT_EQ(L'\u0142', buffer[1]);
}

TEST(SeekableStreamProxyTest, UTF16BEStream) {
  const char data[] = "\xFE\xFF\x00\x41\x01\x42";
  auto proxy_stream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(
      pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(pdfium::make_span(
          reinterpret_cast<const uint8_t*>(data), sizeof(data) - 1)));

  wchar_t buffer[16];
  EXPECT_EQ(2u, proxy_stream->ReadBlock(buffer, FX_ArraySize(buffer)));
  EXPECT_EQ(L'A', buffer[0]);
  EXPECT_EQ(L'\u0142', buffer[1]);
}
