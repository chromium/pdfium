// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPDF_CreatorEmbedderTest : public EmbedderTest {};

TEST_F(CPDF_CreatorEmbedderTest, SavedDocsAreEqualAfterParse) {
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  // Save without additional data reading.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  const std::string saved_doc_1 = GetString();
  ClearString();

  {
    // Do some read only operations.
    ASSERT_GE(1, FPDF_GetPageCount(document()));
    FPDF_PAGE page = FPDF_LoadPage(document(), 0);
    ASSERT_TRUE(page);
    FPDF_BITMAP new_bitmap =
        RenderPageWithFlags(page, form_handle(), FPDF_ANNOT);
    FPDFBitmap_Destroy(new_bitmap);
    UnloadPage(page);
  }

  // Save when we have additional loaded data.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  const std::string saved_doc_2 = GetString();
  ClearString();

  // The sizes of saved docs should be equal.
  EXPECT_EQ(saved_doc_1.size(), saved_doc_2.size());
}
