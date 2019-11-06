// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_annot.h"

#include <vector>

#include "constants/annotation_common.h"
#include "core/fpdfapi/page/cpdf_annotcontext.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_edit.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

class PDFAnnotTest : public testing::Test {
 protected:
  PDFAnnotTest() = default;
  ~PDFAnnotTest() override = default;

  void SetUp() override { CPDF_PageModule::Create(); }
  void TearDown() override { CPDF_PageModule::Destroy(); }
};

TEST_F(PDFAnnotTest, SetAP) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  const std::wstring kStreamData =
      L"0.0 0.0 0.0 RG 4 w 211.8 747.6 m 211.8 744.8 "
      L"212.6 743.0 214.2 740.8 "
      L"c 215.4 739.0 216.8 737.1 218.9 736.1 c 220.8 735.1 221.4 733.0 "
      L"223.7 732.4 c 232.6 729.9 242.0 730.8 251.2 730.8 c 257.5 730.8 "
      L"263.0 732.9 269.0 734.4 c S";
  ScopedFPDFWideString ap_stream = GetFPDFWideString(kStreamData);

  ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
  ASSERT_TRUE(annot);

  // Negative case: FPDFAnnot_SetAP() should fail if bounding rect is not yet
  // set on the annotation.
  EXPECT_FALSE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                               ap_stream.get()));

  FS_RECTF bounding_rect{206.f, 753.f, 339.f, 709.f};
  EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &bounding_rect));

  EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              ap_stream.get()));

  // Verify that appearance stream is created as form XObject
  CPDF_AnnotContext* context = CPDFAnnotContextFromFPDFAnnotation(annot.get());
  ASSERT_TRUE(context);
  CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);
  CPDF_Dictionary* ap_dict = annot_dict->GetDictFor(pdfium::annotation::kAP);
  ASSERT_TRUE(ap_dict);
  CPDF_Dictionary* stream_dict = ap_dict->GetDictFor("N");
  ASSERT_TRUE(stream_dict);
  ByteString type = stream_dict->GetStringFor("Type");
  EXPECT_EQ("XObject", type);
  ByteString sub_type = stream_dict->GetStringFor("Subtype");
  EXPECT_EQ("Form", sub_type);

  // Check that the appearance stream is same as we just set.
  const uint32_t kStreamDataSize =
      (kStreamData.size() + 1) * sizeof(FPDF_WCHAR);
  unsigned long normal_length_bytes = FPDFAnnot_GetAP(
      annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0);
  ASSERT_EQ(kStreamDataSize, normal_length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(normal_length_bytes);
  EXPECT_EQ(kStreamDataSize,
            FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                            buf.data(), normal_length_bytes));
  EXPECT_EQ(kStreamData, GetPlatformWString(buf.data()));
}
