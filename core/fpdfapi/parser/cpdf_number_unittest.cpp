// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_number.h"

#include <limits>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class ByteStringArchiveStream : public IFX_ArchiveStream {
 public:
  ByteStringArchiveStream() = default;
  ~ByteStringArchiveStream() override = default;

  // IFX_ArchiveStream:
  bool WriteBlock(pdfium::span<const uint8_t> buffer) override {
    str_ += ByteStringView(buffer);
    return true;
  }
  FX_FILESIZE CurrentOffset() const override { NOTREACHED_NORETURN(); }

  const ByteString& str() const { return str_; }

 private:
  ByteString str_;
};

}  // namespace

TEST(CPDFNumber, WriteToFloat) {
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(0.0f);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" 0", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(1.0f);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" 1", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(-7.5f);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" -7.5", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(38.895285f);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    // TODO(crbug.com/352496170): Can the output have more precision?
    EXPECT_EQ(" 38.8953", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(-77.037232f);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    // TODO(crbug.com/352496170): Can the output have more precision?
    EXPECT_EQ(" -77.0372", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number =
        pdfium::MakeRetain<CPDF_Number>(std::numeric_limits<float>::max());
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    // TODO(crbug.com/352496170): This should be much bigger than INT_MAX.
    EXPECT_EQ(" 2147483647", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number =
        pdfium::MakeRetain<CPDF_Number>(std::numeric_limits<float>::min());
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    // TODO(crbug.com/352496170): This should not be 0.
    EXPECT_EQ(" 0", output_stream.str());
  }
}

TEST(CPDFNumber, WriteToInt) {
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(0);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" 0", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(1);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" 1", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(-99);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" -99", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(1234);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" 1234", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number = pdfium::MakeRetain<CPDF_Number>(-54321);
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" -54321", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number =
        pdfium::MakeRetain<CPDF_Number>(std::numeric_limits<int>::max());
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" 2147483647", output_stream.str());
  }
  {
    ByteStringArchiveStream output_stream;
    auto number =
        pdfium::MakeRetain<CPDF_Number>(std::numeric_limits<int>::min());
    ASSERT_TRUE(number->WriteTo(&output_stream, /*encryptor=*/nullptr));
    EXPECT_EQ(" -2147483648", output_stream.str());
  }
}
