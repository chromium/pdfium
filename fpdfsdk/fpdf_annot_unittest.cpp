// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_annot.h"

#include <limits.h>

#include <vector>

#include "constants/annotation_common.h"
#include "core/fpdfapi/page/cpdf_annotcontext.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_edit.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const wchar_t kStreamData[] =
    L"/GS gs 0.0 0.0 0.0 RG 4 w 211.8 747.6 m 211.8 744.8 "
    L"212.6 743.0 214.2 740.8 "
    L"c 215.4 739.0 216.8 737.1 218.9 736.1 c 220.8 735.1 221.4 733.0 "
    L"223.7 732.4 c 232.6 729.9 242.0 730.8 251.2 730.8 c 257.5 730.8 "
    L"263.0 732.9 269.0 734.4 c S";

}  // namespace

class PDFAnnotTest : public testing::Test {
 protected:
  PDFAnnotTest() = default;
  ~PDFAnnotTest() override = default;

  void SetUp() override { CPDF_PageModule::Create(); }
  void TearDown() override { CPDF_PageModule::Destroy(); }
};

TEST_F(PDFAnnotTest, SetAP) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(doc);
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  ASSERT_TRUE(page);
  ScopedFPDFWideString ap_stream = GetFPDFWideString(kStreamData);
  ASSERT_TRUE(ap_stream);

  ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
  ASSERT_TRUE(annot);

  // Negative case: FPDFAnnot_SetAP() should fail if bounding rect is not yet
  // set on the annotation.
  EXPECT_FALSE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                               ap_stream.get()));

  const FS_RECTF bounding_rect{206.0f, 753.0f, 339.0f, 709.0f};
  EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &bounding_rect));

  ASSERT_TRUE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color,
                                 /*R=*/255, /*G=*/0, /*B=*/0, /*A=*/255));

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
  // Check for non-existence of resources dictionary in case of opaque color
  CPDF_Dictionary* resources_dict = stream_dict->GetDictFor("Resources");
  ASSERT_FALSE(resources_dict);
  ByteString type = stream_dict->GetStringFor(pdfium::annotation::kType);
  EXPECT_EQ("XObject", type);
  ByteString sub_type = stream_dict->GetStringFor(pdfium::annotation::kSubtype);
  EXPECT_EQ("Form", sub_type);

  // Check that the appearance stream is same as we just set.
  const uint32_t kStreamDataSize =
      FX_ArraySize(kStreamData) * sizeof(FPDF_WCHAR);
  unsigned long normal_length_bytes = FPDFAnnot_GetAP(
      annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0);
  ASSERT_EQ(kStreamDataSize, normal_length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(normal_length_bytes);
  EXPECT_EQ(kStreamDataSize,
            FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                            buf.data(), normal_length_bytes));
  EXPECT_EQ(kStreamData, GetPlatformWString(buf.data()));
}

TEST_F(PDFAnnotTest, SetAPWithOpacity) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(doc);
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  ASSERT_TRUE(page);
  ScopedFPDFWideString ap_stream = GetFPDFWideString(kStreamData);
  ASSERT_TRUE(ap_stream);

  ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
  ASSERT_TRUE(annot);

  ASSERT_TRUE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color,
                                 /*R=*/255, /*G=*/0, /*B=*/0, /*A=*/102));

  const FS_RECTF bounding_rect{206.0f, 753.0f, 339.0f, 709.0f};
  EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &bounding_rect));

  EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              ap_stream.get()));

  CPDF_AnnotContext* context = CPDFAnnotContextFromFPDFAnnotation(annot.get());
  ASSERT_TRUE(context);
  CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);
  CPDF_Dictionary* ap_dict = annot_dict->GetDictFor(pdfium::annotation::kAP);
  ASSERT_TRUE(ap_dict);
  CPDF_Dictionary* stream_dict = ap_dict->GetDictFor("N");
  ASSERT_TRUE(stream_dict);
  CPDF_Dictionary* resources_dict = stream_dict->GetDictFor("Resources");
  ASSERT_TRUE(stream_dict);
  CPDF_Dictionary* extGState_dict = resources_dict->GetDictFor("ExtGState");
  ASSERT_TRUE(extGState_dict);
  CPDF_Dictionary* gs_dict = extGState_dict->GetDictFor("GS");
  ASSERT_TRUE(gs_dict);
  ByteString type = gs_dict->GetStringFor(pdfium::annotation::kType);
  EXPECT_EQ("ExtGState", type);
  float opacity = gs_dict->GetNumberFor("CA");
  // Opacity value of 102 is represented as 0.4f (=104/255) in /CA entry.
  EXPECT_FLOAT_EQ(0.4f, opacity);
  ByteString blend_mode = gs_dict->GetStringFor("BM");
  EXPECT_EQ("Normal", blend_mode);
  bool alpha_source_flag = gs_dict->GetBooleanFor("AIS", true);
  EXPECT_FALSE(alpha_source_flag);
}

TEST_F(PDFAnnotTest, InkListAPIValidations) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(doc);
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  ASSERT_TRUE(page);

  // Create a new ink annotation.
  ScopedFPDFAnnotation ink_annot(
      FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
  ASSERT_TRUE(ink_annot);
  CPDF_AnnotContext* context =
      CPDFAnnotContextFromFPDFAnnotation(ink_annot.get());
  ASSERT_TRUE(context);
  CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);

  static constexpr FS_POINTF kFirstInkStroke[] = {
      {80.0f, 90.0f}, {81.0f, 91.0f}, {82.0f, 92.0f},
      {83.0f, 93.0f}, {84.0f, 94.0f}, {85.0f, 95.0f}};
  static constexpr size_t kFirstStrokePointCount =
      FX_ArraySize(kFirstInkStroke);

  static constexpr FS_POINTF kSecondInkStroke[] = {
      {70.0f, 90.0f}, {71.0f, 91.0f}, {72.0f, 92.0f}};
  static constexpr size_t kSecondStrokePointCount =
      FX_ArraySize(kSecondInkStroke);

  static constexpr FS_POINTF kThirdInkStroke[] = {{60.0f, 90.0f},
                                                  {61.0f, 91.0f},
                                                  {62.0f, 92.0f},
                                                  {63.0f, 93.0f},
                                                  {64.0f, 94.0f}};
  static constexpr size_t kThirdStrokePointCount =
      FX_ArraySize(kThirdInkStroke);

  // Negative test: |annot| is passed as nullptr.
  EXPECT_EQ(-1, FPDFAnnot_AddInkStroke(nullptr, kFirstInkStroke,
                                       kFirstStrokePointCount));

  // Negative test: |annot| is not ink annotation.
  // Create a new highlight annotation.
  ScopedFPDFAnnotation highlight_annot(
      FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_HIGHLIGHT));
  ASSERT_TRUE(highlight_annot);
  EXPECT_EQ(-1, FPDFAnnot_AddInkStroke(highlight_annot.get(), kFirstInkStroke,
                                       kFirstStrokePointCount));

  // Negative test: passing |point_count| as  0.
  EXPECT_EQ(-1, FPDFAnnot_AddInkStroke(ink_annot.get(), kFirstInkStroke, 0));

  // Negative test: passing |points| array as nullptr.
  EXPECT_EQ(-1, FPDFAnnot_AddInkStroke(ink_annot.get(), nullptr,
                                       kFirstStrokePointCount));

  // Negative test: passing |point_count| more than ULONG_MAX/2.
  EXPECT_EQ(-1, FPDFAnnot_AddInkStroke(ink_annot.get(), kSecondInkStroke,
                                       ULONG_MAX / 2 + 1));

  // InkStroke should get added to ink annotation. Also inklist should get
  // created.
  EXPECT_EQ(0, FPDFAnnot_AddInkStroke(ink_annot.get(), kFirstInkStroke,
                                      kFirstStrokePointCount));

  CPDF_Array* inklist = annot_dict->GetArrayFor("InkList");
  ASSERT_TRUE(inklist);
  EXPECT_EQ(1u, inklist->size());
  EXPECT_EQ(kFirstStrokePointCount * 2, inklist->GetArrayAt(0)->size());

  // Adding another inkStroke to ink annotation with all valid paremeters.
  // InkList already exists in ink_annot.
  EXPECT_EQ(1, FPDFAnnot_AddInkStroke(ink_annot.get(), kSecondInkStroke,
                                      kSecondStrokePointCount));
  EXPECT_EQ(2u, inklist->size());
  EXPECT_EQ(kSecondStrokePointCount * 2, inklist->GetArrayAt(1)->size());

  // Adding one more InkStroke to the ink annotation. |point_count| passed is
  // less than the data available in |buffer|.
  EXPECT_EQ(2, FPDFAnnot_AddInkStroke(ink_annot.get(), kThirdInkStroke,
                                      kThirdStrokePointCount - 1));
  EXPECT_EQ(3u, inklist->size());
  EXPECT_EQ((kThirdStrokePointCount - 1) * 2, inklist->GetArrayAt(2)->size());
}

TEST_F(PDFAnnotTest, RemoveInkList) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(doc);
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  ASSERT_TRUE(page);

  // Negative test: |annot| is passed as nullptr.
  EXPECT_FALSE(FPDFAnnot_RemoveInkList(nullptr));

  // Negative test: |annot| is not ink annotation.
  // Create a new highlight annotation.
  ScopedFPDFAnnotation highlight_annot(
      FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_HIGHLIGHT));
  ASSERT_TRUE(highlight_annot);
  EXPECT_FALSE(FPDFAnnot_RemoveInkList(highlight_annot.get()));

  // Create a new ink annotation.
  ScopedFPDFAnnotation ink_annot(
      FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
  ASSERT_TRUE(ink_annot);
  CPDF_AnnotContext* context =
      CPDFAnnotContextFromFPDFAnnotation(ink_annot.get());
  ASSERT_TRUE(context);
  CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);

  static constexpr FS_POINTF kInkStroke[] = {{80.0f, 90.0f}, {81.0f, 91.0f},
                                             {82.0f, 92.0f}, {83.0f, 93.0f},
                                             {84.0f, 94.0f}, {85.0f, 95.0f}};
  static constexpr size_t kPointCount = FX_ArraySize(kInkStroke);

  // InkStroke should get added to ink annotation. Also inklist should get
  // created.
  EXPECT_EQ(0,
            FPDFAnnot_AddInkStroke(ink_annot.get(), kInkStroke, kPointCount));

  CPDF_Array* inklist = annot_dict->GetArrayFor("InkList");
  ASSERT_TRUE(inklist);
  ASSERT_EQ(1u, inklist->size());
  EXPECT_EQ(kPointCount * 2, inklist->GetArrayAt(0)->size());

  // Remove inklist.
  EXPECT_TRUE(FPDFAnnot_RemoveInkList(ink_annot.get()));
  EXPECT_FALSE(annot_dict->KeyExist("InkList"));
}
