// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "public/fpdf_attachment.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/utils/hash.h"

static constexpr char kDateKey[] = "CreationDate";
static constexpr char kChecksumKey[] = "CheckSum";

class FPDFAttachmentEmbedderTest : public EmbedderTest {};

TEST_F(FPDFAttachmentEmbedderTest, ExtractAttachments) {
  // Open a file with two attachments.
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Try to retrieve attachments at bad indices.
  EXPECT_FALSE(FPDFDoc_GetAttachment(document(), -1));
  EXPECT_FALSE(FPDFDoc_GetAttachment(document(), 2));

  // Retrieve the first attachment.
  FPDF_ATTACHMENT attachment = FPDFDoc_GetAttachment(document(), 0);
  ASSERT_TRUE(attachment);

  // Check that the name of the first attachment is correct.
  unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(12u, length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"1.txt", GetPlatformWString(buf.data()));

  // Check that the content of the first attachment is correct.
  length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
  std::vector<char> content_buf(length_bytes);
  ASSERT_EQ(
      4u, FPDFAttachment_GetFile(attachment, content_buf.data(), length_bytes));
  EXPECT_EQ(std::string("test"), std::string(content_buf.data(), 4));

  // Check that a non-existent key does not exist.
  EXPECT_FALSE(FPDFAttachment_HasKey(attachment, "none"));

  // Check that the string value of a non-string dictionary entry is empty.
  static constexpr char kSizeKey[] = "Size";
  EXPECT_EQ(FPDF_OBJECT_NUMBER,
            FPDFAttachment_GetValueType(attachment, kSizeKey));
  EXPECT_EQ(2u,
            FPDFAttachment_GetStringValue(attachment, kSizeKey, nullptr, 0));

  // Check that the creation date of the first attachment is correct.
  length_bytes =
      FPDFAttachment_GetStringValue(attachment, kDateKey, nullptr, 0);
  ASSERT_EQ(48u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(48u, FPDFAttachment_GetStringValue(attachment, kDateKey, buf.data(),
                                               length_bytes));
  EXPECT_EQ(L"D:20170712214438-07'00'", GetPlatformWString(buf.data()));

  // Retrieve the second attachment.
  attachment = FPDFDoc_GetAttachment(document(), 1);
  ASSERT_TRUE(attachment);

  // Retrieve the second attachment file.
  length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
  content_buf.clear();
  content_buf.resize(length_bytes);
  ASSERT_EQ(5869u, FPDFAttachment_GetFile(attachment, content_buf.data(),
                                          length_bytes));

  // Check that the calculated checksum of the file data matches expectation.
  const char kCheckSum[] = "72afcddedf554dda63c0c88e06f1ce18";
  const wchar_t kCheckSumW[] = L"<72AFCDDEDF554DDA63C0C88E06F1CE18>";
  const std::string generated_checksum = GenerateMD5Base16(
      reinterpret_cast<uint8_t*>(content_buf.data()), length_bytes);
  EXPECT_EQ(kCheckSum, generated_checksum);

  // Check that the stored checksum matches expectation.
  length_bytes =
      FPDFAttachment_GetStringValue(attachment, kChecksumKey, nullptr, 0);
  ASSERT_EQ(70u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(70u, FPDFAttachment_GetStringValue(attachment, kChecksumKey,
                                               buf.data(), length_bytes));
  EXPECT_EQ(kCheckSumW, GetPlatformWString(buf.data()));
}

TEST_F(FPDFAttachmentEmbedderTest, NoAttachmentToExtract) {
  // Open a file with no attachments.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(0, FPDFDoc_GetAttachmentCount(document()));

  // Try to retrieve attachments at bad indices.
  EXPECT_FALSE(FPDFDoc_GetAttachment(document(), -1));
  EXPECT_FALSE(FPDFDoc_GetAttachment(document(), 0));
}

TEST_F(FPDFAttachmentEmbedderTest, AddAttachments) {
  // Open a file with two attachments.
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Check that adding an attachment with an empty name would fail.
  EXPECT_FALSE(FPDFDoc_AddAttachment(document(), nullptr));

  // Add an attachment to the beginning of the embedded file list.
  ScopedFPDFWideString file_name = GetFPDFWideString(L"0.txt");
  FPDF_ATTACHMENT attachment =
      FPDFDoc_AddAttachment(document(), file_name.get());
  ASSERT_TRUE(attachment);

  // Check that writing to a file with nullptr but non-zero bytes would fail.
  EXPECT_FALSE(FPDFAttachment_SetFile(attachment, document(), nullptr, 10));

  // Set the new attachment's file.
  constexpr char kContents1[] = "Hello!";
  EXPECT_TRUE(FPDFAttachment_SetFile(attachment, document(), kContents1,
                                     strlen(kContents1)));
  EXPECT_EQ(3, FPDFDoc_GetAttachmentCount(document()));

  // Verify the name of the new attachment (i.e. the first attachment).
  attachment = FPDFDoc_GetAttachment(document(), 0);
  ASSERT_TRUE(attachment);
  unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(12u, length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"0.txt", GetPlatformWString(buf.data()));

  // Verify the content of the new attachment (i.e. the first attachment).
  length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
  std::vector<char> content_buf(length_bytes);
  ASSERT_EQ(
      6u, FPDFAttachment_GetFile(attachment, content_buf.data(), length_bytes));
  EXPECT_EQ(std::string(kContents1), std::string(content_buf.data(), 6));

  // Add an attachment to the end of the embedded file list and set its file.
  file_name = GetFPDFWideString(L"z.txt");
  attachment = FPDFDoc_AddAttachment(document(), file_name.get());
  ASSERT_TRUE(attachment);
  constexpr char kContents2[] = "World!";
  EXPECT_TRUE(FPDFAttachment_SetFile(attachment, document(), kContents2,
                                     strlen(kContents2)));
  EXPECT_EQ(4, FPDFDoc_GetAttachmentCount(document()));

  // Verify the name of the new attachment (i.e. the fourth attachment).
  attachment = FPDFDoc_GetAttachment(document(), 3);
  ASSERT_TRUE(attachment);
  length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(12u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"z.txt", GetPlatformWString(buf.data()));

  // Verify the content of the new attachment (i.e. the fourth attachment).
  length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
  content_buf.clear();
  content_buf.resize(length_bytes);
  ASSERT_EQ(
      6u, FPDFAttachment_GetFile(attachment, content_buf.data(), length_bytes));
  EXPECT_EQ(std::string(kContents2), std::string(content_buf.data(), 6));
}

TEST_F(FPDFAttachmentEmbedderTest, AddAttachmentsWithParams) {
  // Open a file with two attachments.
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Add an attachment to the embedded file list.
  ScopedFPDFWideString file_name = GetFPDFWideString(L"5.txt");
  FPDF_ATTACHMENT attachment =
      FPDFDoc_AddAttachment(document(), file_name.get());
  ASSERT_TRUE(attachment);
  constexpr char kContents[] = "Hello World!";
  EXPECT_TRUE(FPDFAttachment_SetFile(attachment, document(), kContents,
                                     strlen(kContents)));

  // Set the date to be an arbitrary value.
  constexpr wchar_t kDateW[] = L"D:20170720161527-04'00'";
  ScopedFPDFWideString ws_date = GetFPDFWideString(kDateW);
  EXPECT_TRUE(
      FPDFAttachment_SetStringValue(attachment, kDateKey, ws_date.get()));

  // Set the checksum to be an arbitrary value.
  constexpr wchar_t kCheckSumW[] = L"<ABCDEF01234567899876543210FEDCBA>";
  ScopedFPDFWideString ws_checksum = GetFPDFWideString(kCheckSumW);
  EXPECT_TRUE(FPDFAttachment_SetStringValue(attachment, kChecksumKey,
                                            ws_checksum.get()));

  // Verify the name of the new attachment (i.e. the second attachment).
  attachment = FPDFDoc_GetAttachment(document(), 1);
  ASSERT_TRUE(attachment);
  unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(12u, length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"5.txt", GetPlatformWString(buf.data()));

  // Verify the content of the new attachment.
  length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
  std::vector<char> content_buf(length_bytes);
  ASSERT_EQ(12u, FPDFAttachment_GetFile(attachment, content_buf.data(),
                                        length_bytes));
  EXPECT_EQ(std::string(kContents), std::string(content_buf.data(), 12));

  // Verify the creation date of the new attachment.
  length_bytes =
      FPDFAttachment_GetStringValue(attachment, kDateKey, nullptr, 0);
  ASSERT_EQ(48u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(48u, FPDFAttachment_GetStringValue(attachment, kDateKey, buf.data(),
                                               length_bytes));
  EXPECT_EQ(kDateW, GetPlatformWString(buf.data()));

  // Verify the checksum of the new attachment.
  length_bytes =
      FPDFAttachment_GetStringValue(attachment, kChecksumKey, nullptr, 0);
  ASSERT_EQ(70u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(70u, FPDFAttachment_GetStringValue(attachment, kChecksumKey,
                                               buf.data(), length_bytes));
  EXPECT_EQ(kCheckSumW, GetPlatformWString(buf.data()));

  // Overwrite the existing file with empty content, and check that the checksum
  // gets updated to the correct value.
  EXPECT_TRUE(FPDFAttachment_SetFile(attachment, document(), nullptr, 0));
  EXPECT_EQ(0u, FPDFAttachment_GetFile(attachment, nullptr, 0));
  length_bytes =
      FPDFAttachment_GetStringValue(attachment, kChecksumKey, nullptr, 0);
  ASSERT_EQ(70u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(70u, FPDFAttachment_GetStringValue(attachment, kChecksumKey,
                                               buf.data(), length_bytes));
  EXPECT_EQ(L"<D41D8CD98F00B204E9800998ECF8427E>",
            GetPlatformWString(buf.data()));
}

TEST_F(FPDFAttachmentEmbedderTest, AddAttachmentsToFileWithNoAttachments) {
  // Open a file with no attachments.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(0, FPDFDoc_GetAttachmentCount(document()));

  // Add an attachment to the beginning of the embedded file list.
  ScopedFPDFWideString file_name = GetFPDFWideString(L"0.txt");
  FPDF_ATTACHMENT attachment =
      FPDFDoc_AddAttachment(document(), file_name.get());
  ASSERT_TRUE(attachment);

  // Set the new attachment's file.
  constexpr char kContents1[] = "Hello!";
  EXPECT_TRUE(FPDFAttachment_SetFile(attachment, document(), kContents1,
                                     strlen(kContents1)));
  EXPECT_EQ(1, FPDFDoc_GetAttachmentCount(document()));

  // Verify the name of the new attachment (i.e. the first attachment).
  attachment = FPDFDoc_GetAttachment(document(), 0);
  ASSERT_TRUE(attachment);
  unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(12u, length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"0.txt", GetPlatformWString(buf.data()));

  // Verify the content of the new attachment (i.e. the first attachment).
  length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
  std::vector<char> content_buf(length_bytes);
  ASSERT_EQ(
      6u, FPDFAttachment_GetFile(attachment, content_buf.data(), length_bytes));
  EXPECT_EQ(std::string(kContents1), std::string(content_buf.data(), 6));

  // Add an attachment to the end of the embedded file list and set its file.
  file_name = GetFPDFWideString(L"z.txt");
  attachment = FPDFDoc_AddAttachment(document(), file_name.get());
  ASSERT_TRUE(attachment);
  constexpr char kContents2[] = "World!";
  EXPECT_TRUE(FPDFAttachment_SetFile(attachment, document(), kContents2,
                                     strlen(kContents2)));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Verify the name of the new attachment (i.e. the second attachment).
  attachment = FPDFDoc_GetAttachment(document(), 1);
  ASSERT_TRUE(attachment);
  length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(12u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"z.txt", GetPlatformWString(buf.data()));

  // Verify the content of the new attachment (i.e. the second attachment).
  length_bytes = FPDFAttachment_GetFile(attachment, nullptr, 0);
  content_buf.clear();
  content_buf.resize(length_bytes);
  ASSERT_EQ(
      6u, FPDFAttachment_GetFile(attachment, content_buf.data(), length_bytes));
  EXPECT_EQ(std::string(kContents2), std::string(content_buf.data(), 6));
}

TEST_F(FPDFAttachmentEmbedderTest, DeleteAttachment) {
  // Open a file with two attachments.
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Verify the name of the first attachment.
  FPDF_ATTACHMENT attachment = FPDFDoc_GetAttachment(document(), 0);
  ASSERT_TRUE(attachment);
  unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(12u, length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"1.txt", GetPlatformWString(buf.data()));

  // Delete the first attachment.
  EXPECT_TRUE(FPDFDoc_DeleteAttachment(document(), 0));
  EXPECT_EQ(1, FPDFDoc_GetAttachmentCount(document()));

  // Verify the name of the new first attachment.
  attachment = FPDFDoc_GetAttachment(document(), 0);
  ASSERT_TRUE(attachment);
  length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
  ASSERT_EQ(26u, length_bytes);
  buf = GetFPDFWideStringBuffer(length_bytes);
  EXPECT_EQ(26u, FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
  EXPECT_EQ(L"attached.pdf", GetPlatformWString(buf.data()));
}
