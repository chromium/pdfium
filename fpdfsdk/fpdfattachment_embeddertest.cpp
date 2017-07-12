// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_attachment.h"
#include "testing/embedder_test.h"

class FPDFAttachmentEmbeddertest : public EmbedderTest {};

TEST_F(FPDFAttachmentEmbeddertest, ExtractAttachments) {
  // Open a file with two attachments.
  ASSERT_TRUE(OpenDocument("embedded_attachments.pdf"));
  EXPECT_EQ(2, FPDFDoc_GetAttachmentCount(document()));

  // Check that the name of the first attachment is correct.
  unsigned long len = FPDFDoc_GetAttachmentName(document(), 0, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(12u, FPDFDoc_GetAttachmentName(document(), 0, buf.data(), len));
  EXPECT_STREQ(L"1.txt",
               GetPlatformWString(reinterpret_cast<unsigned short*>(buf.data()))
                   .c_str());
}
