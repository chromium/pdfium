// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_stream_acc.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/fx_stream.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/invalid_seekable_read_stream.h"

TEST(StreamAccTest, ReadRawDataFailed) {
  auto stream = pdfium::MakeRetain<CPDF_Stream>();
  stream->InitStreamFromFile(
      pdfium::MakeRetain<InvalidSeekableReadStream>(1024),
      pdfium::MakeRetain<CPDF_Dictionary>());
  auto stream_acc = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(stream));
  stream_acc->LoadAllDataRaw();
  EXPECT_TRUE(stream_acc->GetSpan().empty());
}

// Regression test for crbug.com/1361849. Should not trigger dangling pointer
// failure with UnownedPtr.
TEST(StreamAccTest, DataStreamLifeTime) {
  constexpr uint8_t kData[] = {'a', 'b', 'c'};
  auto stream = pdfium::MakeRetain<CPDF_Stream>();
  stream->SetData(kData);
  auto stream_acc = pdfium::MakeRetain<CPDF_StreamAcc>(stream);
  stream_acc->LoadAllDataRaw();
  stream.Reset();
  auto span = stream_acc->GetSpan();
  EXPECT_TRUE(
      std::equal(std::begin(kData), std::end(kData), span.begin(), span.end()));
}
