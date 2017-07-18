// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_attachment.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"

class FPDFAttachmentEmbeddertest : public EmbedderTest {};

TEST_F(FPDFAttachmentEmbeddertest, ExtractAttachments) {
  // Open a file with two attachments.
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Retrieve the first attachment.
  FPDF_ATTACHMENT attachment = FPDFDoc_GetAttachment(document(), 0);
  ASSERT_TRUE(attachment);

  // Check that the name of the first attachment is correct.
  unsigned long len = FPDFAttachment_GetName(attachment, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(12u, FPDFAttachment_GetName(attachment, buf.data(), len));
  EXPECT_STREQ(L"1.txt",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Check that the content of the first attachment is correct.
  len = FPDFAttachment_GetFile(attachment, nullptr, 0);
  buf.clear();
  buf.resize(len);
  ASSERT_EQ(4u, FPDFAttachment_GetFile(attachment, buf.data(), len));
  EXPECT_EQ(std::string("test"), std::string(buf.data(), 4));

  // Check that a non-existent key does not exist.
  EXPECT_FALSE(
      FPDFAttachment_HasKey(attachment, GetFPDFWideString(L"none").get()));

  // Check that the string value of a non-string dictionary entry is empty.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> size_key =
      GetFPDFWideString(L"Size");
  EXPECT_EQ(FPDF_OBJECT_NUMBER,
            FPDFAttachment_GetValueType(attachment, size_key.get()));
  EXPECT_EQ(2u, FPDFAttachment_GetStringValue(attachment, size_key.get(),
                                              nullptr, 0));

  // Check that the creation date of the first attachment is correct.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> date_key =
      GetFPDFWideString(L"CreationDate");
  len = FPDFAttachment_GetStringValue(attachment, date_key.get(), nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(48u, FPDFAttachment_GetStringValue(attachment, date_key.get(),
                                               buf.data(), len));
  EXPECT_STREQ(L"D:20170712214438-07'00'",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());

  // Retrieve the second attachment.
  attachment = FPDFDoc_GetAttachment(document(), 1);
  ASSERT_TRUE(attachment);

  // Retrieve the second attachment file.
  len = FPDFAttachment_GetFile(attachment, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(5869u, FPDFAttachment_GetFile(attachment, buf.data(), len));

  // Check that the calculated checksum of the file data matches expectation.
  const char kCheckSum[] = "72afcddedf554dda63c0c88e06f1ce18";
  const wchar_t kCheckSumW[] = L"<72AFCDDEDF554DDA63C0C88E06F1CE18>";
  const std::string generated_checksum =
      GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()), len);
  EXPECT_EQ(kCheckSum, generated_checksum);

  // Check that the stored checksum matches expectation.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> checksum_key =
      GetFPDFWideString(L"CheckSum");
  len =
      FPDFAttachment_GetStringValue(attachment, checksum_key.get(), nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(70u, FPDFAttachment_GetStringValue(attachment, checksum_key.get(),
                                               buf.data(), len));
  EXPECT_EQ(kCheckSumW,
            GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data())));
}
