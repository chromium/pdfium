// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_stream_acc.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/fx_stream.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/invalid_seekable_read_stream.h"

TEST(CPDF_StreamAccTest, ReadRawDataFailed) {
  auto stream = pdfium::MakeRetain<CPDF_Stream>();
  stream->InitStreamFromFile(
      pdfium::MakeRetain<InvalidSeekableReadStream>(1024),
      pdfium::MakeRetain<CPDF_Dictionary>());
  auto stream_acc = pdfium::MakeRetain<CPDF_StreamAcc>(stream.Get());
  stream_acc->LoadAllDataRaw();
  EXPECT_EQ(0u, stream_acc->GetSize());
  EXPECT_FALSE(stream_acc->GetData());
}
