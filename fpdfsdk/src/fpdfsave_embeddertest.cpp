// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "core/include/fxcrt/fx_string.h"
#include "public/fpdf_save.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

class FPDFSaveEmbedderTest : public EmbedderTest, public FPDF_FILEWRITE {
 public:
  FPDFSaveEmbedderTest() {
    FPDF_FILEWRITE::version = 1;
    FPDF_FILEWRITE::WriteBlock = WriteBlockCallback;
  }
  bool SaveDocumentToString() {
    m_String.clear();
    return FPDF_SaveAsCopy(document(), this, 0);
  }
  bool SaveDocumentWithVersionToString(int version) {
    m_String.clear();
    return FPDF_SaveWithVersion(document(), this, 0, version);
  }
  const std::string& GetString() const { return m_String; }

 private:
  static int WriteBlockCallback(FPDF_FILEWRITE* pFileWrite,
                                const void* data,
                                unsigned long size) {
    FPDFSaveEmbedderTest* pThis =
        static_cast<FPDFSaveEmbedderTest*>(pFileWrite);
    pThis->m_String.append(static_cast<const char*>(data), size);
    return 1;
  }

  std::string m_String;
};

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDoc) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(SaveDocumentToString());
  EXPECT_EQ("%PDF-1.7\r\n", GetString().substr(0, 10));
  EXPECT_EQ(843, GetString().length());
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocWithVersion) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(SaveDocumentWithVersionToString(14));
  EXPECT_EQ("%PDF-1.4\r\n", GetString().substr(0, 10));
  EXPECT_EQ(843, GetString().length());
}

TEST_F(FPDFSaveEmbedderTest, SaveSimpleDocWithBadVersion) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(SaveDocumentWithVersionToString(-1));
  EXPECT_EQ("%PDF-1.7\r\n", GetString().substr(0, 10));
  EXPECT_TRUE(SaveDocumentWithVersionToString(0));
  EXPECT_EQ("%PDF-1.7\r\n", GetString().substr(0, 10));
  EXPECT_TRUE(SaveDocumentWithVersionToString(18));
  EXPECT_EQ("%PDF-1.7\r\n", GetString().substr(0, 10));
}

TEST_F(FPDFSaveEmbedderTest, BUG_342) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(SaveDocumentToString());
  EXPECT_EQ(std::string::npos, GetString().find("0000000000 65536 f\r\n"));
  EXPECT_NE(std::string::npos, GetString().find("0000000000 65535 f\r\n"));
}
