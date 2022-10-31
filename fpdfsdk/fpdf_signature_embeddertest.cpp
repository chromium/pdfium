// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "public/fpdf_signature.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"

class FPDFSignatureEmbedderTest : public EmbedderTest {};

TEST_F(FPDFSignatureEmbedderTest, GetSignatureCount) {
  ASSERT_TRUE(OpenDocument("two_signatures.pdf"));
  EXPECT_EQ(2, FPDF_GetSignatureCount(document()));
}

TEST_F(FPDFSignatureEmbedderTest, GetSignatureCountZero) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(0, FPDF_GetSignatureCount(document()));

  // Provide no document.
  EXPECT_EQ(-1, FPDF_GetSignatureCount(nullptr));
}

TEST_F(FPDFSignatureEmbedderTest, GetSignatureObject) {
  ASSERT_TRUE(OpenDocument("two_signatures.pdf"));
  // Different, non-null signature objects are returned.
  FPDF_SIGNATURE signature1 = FPDF_GetSignatureObject(document(), 0);
  EXPECT_TRUE(signature1);
  FPDF_SIGNATURE signature2 = FPDF_GetSignatureObject(document(), 1);
  EXPECT_TRUE(signature2);
  EXPECT_NE(signature1, signature2);

  // Out of bounds.
  EXPECT_FALSE(FPDF_GetSignatureObject(document(), -1));
  EXPECT_FALSE(FPDF_GetSignatureObject(document(), 2));

  // Provide no document.
  EXPECT_FALSE(FPDF_GetSignatureObject(nullptr, 0));
}

TEST_F(FPDFSignatureEmbedderTest, GetContents) {
  ASSERT_TRUE(OpenDocument("two_signatures.pdf"));
  FPDF_SIGNATURE signature = FPDF_GetSignatureObject(document(), 0);
  EXPECT_TRUE(signature);

  // FPDFSignatureObj_GetContents() positive testing.
  unsigned long size = FPDFSignatureObj_GetContents(signature, nullptr, 0);
  const uint8_t kExpectedContents[] = {0x30, 0x80, 0x06, 0x09, 0x2A, 0x86, 0x48,
                                       0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02, 0xA0,
                                       0x80, 0x30, 0x80, 0x02, 0x01, 0x01};
  ASSERT_EQ(sizeof(kExpectedContents), size);
  std::vector<char> contents(size);
  ASSERT_EQ(size,
            FPDFSignatureObj_GetContents(signature, contents.data(), size));
  ASSERT_EQ(0, memcmp(kExpectedContents, contents.data(), size));

  // FPDFSignatureObj_GetContents() negative testing.
  ASSERT_EQ(0U, FPDFSignatureObj_GetContents(nullptr, nullptr, 0));

  contents.resize(2);
  contents[0] = 'x';
  contents[1] = '\0';
  size =
      FPDFSignatureObj_GetContents(signature, contents.data(), contents.size());
  ASSERT_EQ(sizeof(kExpectedContents), size);
  EXPECT_EQ('x', contents[0]);
  EXPECT_EQ('\0', contents[1]);
}

TEST_F(FPDFSignatureEmbedderTest, GetByteRange) {
  ASSERT_TRUE(OpenDocument("two_signatures.pdf"));
  FPDF_SIGNATURE signature = FPDF_GetSignatureObject(document(), 0);
  EXPECT_TRUE(signature);

  // FPDFSignatureObj_GetByteRange() positive testing.
  unsigned long size = FPDFSignatureObj_GetByteRange(signature, nullptr, 0);
  const std::vector<int> kExpectedByteRange{0, 10, 30, 10};
  ASSERT_EQ(kExpectedByteRange.size(), size);
  std::vector<int> byte_range(size);
  ASSERT_EQ(size,
            FPDFSignatureObj_GetByteRange(signature, byte_range.data(), size));
  ASSERT_EQ(kExpectedByteRange, byte_range);

  // FPDFSignatureObj_GetByteRange() negative testing.
  ASSERT_EQ(0U, FPDFSignatureObj_GetByteRange(nullptr, nullptr, 0));

  byte_range.resize(2);
  byte_range[0] = 0;
  byte_range[1] = 1;
  size = FPDFSignatureObj_GetByteRange(signature, byte_range.data(),
                                       byte_range.size());
  ASSERT_EQ(kExpectedByteRange.size(), size);
  EXPECT_EQ(0, byte_range[0]);
  EXPECT_EQ(1, byte_range[1]);
}

TEST_F(FPDFSignatureEmbedderTest, GetSubFilter) {
  ASSERT_TRUE(OpenDocument("two_signatures.pdf"));
  FPDF_SIGNATURE signature = FPDF_GetSignatureObject(document(), 0);
  EXPECT_TRUE(signature);

  // FPDFSignatureObj_GetSubFilter() positive testing.
  unsigned long size = FPDFSignatureObj_GetSubFilter(signature, nullptr, 0);
  const char kExpectedSubFilter[] = "ETSI.CAdES.detached";
  ASSERT_EQ(sizeof(kExpectedSubFilter), size);
  std::vector<char> sub_filter(size);
  ASSERT_EQ(size,
            FPDFSignatureObj_GetSubFilter(signature, sub_filter.data(), size));
  ASSERT_EQ(0, memcmp(kExpectedSubFilter, sub_filter.data(), size));

  // FPDFSignatureObj_GetSubFilter() negative testing.
  ASSERT_EQ(0U, FPDFSignatureObj_GetSubFilter(nullptr, nullptr, 0));

  sub_filter.resize(2);
  sub_filter[0] = 'x';
  sub_filter[1] = '\0';
  size = FPDFSignatureObj_GetSubFilter(signature, sub_filter.data(),
                                       sub_filter.size());
  ASSERT_EQ(sizeof(kExpectedSubFilter), size);
  EXPECT_EQ('x', sub_filter[0]);
  EXPECT_EQ('\0', sub_filter[1]);
}

TEST_F(FPDFSignatureEmbedderTest, GetSubFilterNoKeyExists) {
  ASSERT_TRUE(OpenDocument("signature_no_sub_filter.pdf"));
  FPDF_SIGNATURE signature = FPDF_GetSignatureObject(document(), 0);
  EXPECT_TRUE(signature);

  // FPDFSignatureObj_GetSubFilter() negative testing: no SubFilter
  ASSERT_EQ(0U, FPDFSignatureObj_GetSubFilter(signature, nullptr, 0));
}

TEST_F(FPDFSignatureEmbedderTest, GetReason) {
  ASSERT_TRUE(OpenDocument("signature_reason.pdf"));
  FPDF_SIGNATURE signature = FPDF_GetSignatureObject(document(), 0);
  EXPECT_TRUE(signature);

  // FPDFSignatureObj_GetReason() positive testing.
  constexpr char kReason[] = "test reason";
  // Return value includes the terminating NUL that is provided.
  constexpr unsigned long kReasonUTF16Size = std::size(kReason) * 2;
  constexpr wchar_t kReasonWide[] = L"test reason";
  unsigned long size = FPDFSignatureObj_GetReason(signature, nullptr, 0);
  ASSERT_EQ(kReasonUTF16Size, size);

  std::vector<unsigned short> buffer(size);
  ASSERT_EQ(size, FPDFSignatureObj_GetReason(signature, buffer.data(), size));
  ASSERT_EQ(kReasonWide, GetPlatformWString(buffer.data()));

  // FPDFSignatureObj_GetReason() negative testing.
  ASSERT_EQ(0U, FPDFSignatureObj_GetReason(nullptr, nullptr, 0));

  // Buffer is too small, ensure it's not modified.
  buffer.resize(2);
  buffer[0] = 'x';
  buffer[1] = '\0';
  size = FPDFSignatureObj_GetReason(signature, buffer.data(), buffer.size());
  ASSERT_EQ(kReasonUTF16Size, size);
  EXPECT_EQ('x', buffer[0]);
  EXPECT_EQ('\0', buffer[1]);
}

TEST_F(FPDFSignatureEmbedderTest, GetTime) {
  ASSERT_TRUE(OpenDocument("two_signatures.pdf"));
  FPDF_SIGNATURE signature = FPDF_GetSignatureObject(document(), 0);
  EXPECT_TRUE(signature);

  // FPDFSignatureObj_GetTime() positive testing.
  unsigned long size = FPDFSignatureObj_GetTime(signature, nullptr, 0);
  const char kExpectedTime[] = "D:20200624093114+02'00'";
  ASSERT_EQ(sizeof(kExpectedTime), size);
  std::vector<char> time_buffer(size);
  ASSERT_EQ(size,
            FPDFSignatureObj_GetTime(signature, time_buffer.data(), size));
  ASSERT_EQ(0, memcmp(kExpectedTime, time_buffer.data(), size));

  // FPDFSignatureObj_GetTime() negative testing.
  ASSERT_EQ(0U, FPDFSignatureObj_GetTime(nullptr, nullptr, 0));

  time_buffer.resize(2);
  time_buffer[0] = 'x';
  time_buffer[1] = '\0';
  size = FPDFSignatureObj_GetTime(signature, time_buffer.data(),
                                  time_buffer.size());
  ASSERT_EQ(sizeof(kExpectedTime), size);
  EXPECT_EQ('x', time_buffer[0]);
  EXPECT_EQ('\0', time_buffer[1]);
}

TEST_F(FPDFSignatureEmbedderTest, GetDocMDPPermission) {
  ASSERT_TRUE(OpenDocument("docmdp.pdf"));
  FPDF_SIGNATURE signature = FPDF_GetSignatureObject(document(), 0);
  ASSERT_NE(nullptr, signature);

  // FPDFSignatureObj_GetDocMDPPermission() positive testing.
  unsigned int permission = FPDFSignatureObj_GetDocMDPPermission(signature);
  EXPECT_EQ(1U, permission);

  // FPDFSignatureObj_GetDocMDPPermission() negative testing.
  EXPECT_EQ(0U, FPDFSignatureObj_GetDocMDPPermission(nullptr));
}
