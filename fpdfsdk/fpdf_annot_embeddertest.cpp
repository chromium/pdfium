// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_annot.h"

#include <limits.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "build/build_config.h"
#include "constants/annotation_common.h"
#include "core/fpdfapi/page/cpdf_annotcontext.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/fx_system.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_formfill.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/hash.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/cxx17_backports.h"
#include "third_party/base/span.h"

using pdfium::kAnnotationStampWithApChecksum;

namespace {

const wchar_t kStreamData[] =
    L"/GS gs 0.0 0.0 0.0 RG 4 w 211.8 747.6 m 211.8 744.8 "
    L"212.6 743.0 214.2 740.8 "
    L"c 215.4 739.0 216.8 737.1 218.9 736.1 c 220.8 735.1 221.4 733.0 "
    L"223.7 732.4 c 232.6 729.9 242.0 730.8 251.2 730.8 c 257.5 730.8 "
    L"263.0 732.9 269.0 734.4 c S";

void VerifyFocusableAnnotSubtypes(
    FPDF_FORMHANDLE form_handle,
    pdfium::span<const FPDF_ANNOTATION_SUBTYPE> expected_subtypes) {
  ASSERT_EQ(static_cast<int>(expected_subtypes.size()),
            FPDFAnnot_GetFocusableSubtypesCount(form_handle));

  std::vector<FPDF_ANNOTATION_SUBTYPE> actual_subtypes(
      expected_subtypes.size());
  ASSERT_TRUE(FPDFAnnot_GetFocusableSubtypes(
      form_handle, actual_subtypes.data(), actual_subtypes.size()));
  for (size_t i = 0; i < expected_subtypes.size(); ++i)
    ASSERT_EQ(expected_subtypes[i], actual_subtypes[i]);
}

void SetAndVerifyFocusableAnnotSubtypes(
    FPDF_FORMHANDLE form_handle,
    pdfium::span<const FPDF_ANNOTATION_SUBTYPE> subtypes) {
  ASSERT_TRUE(FPDFAnnot_SetFocusableSubtypes(form_handle, subtypes.data(),
                                             subtypes.size()));
  VerifyFocusableAnnotSubtypes(form_handle, subtypes);
}

void VerifyAnnotationSubtypesAndFocusability(
    FPDF_FORMHANDLE form_handle,
    FPDF_PAGE page,
    pdfium::span<const FPDF_ANNOTATION_SUBTYPE> expected_subtypes,
    pdfium::span<const FPDF_ANNOTATION_SUBTYPE> expected_focusable_subtypes) {
  ASSERT_EQ(static_cast<int>(expected_subtypes.size()),
            FPDFPage_GetAnnotCount(page));
  for (size_t i = 0; i < expected_subtypes.size(); ++i) {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, i));
    ASSERT_TRUE(annot);
    EXPECT_EQ(expected_subtypes[i], FPDFAnnot_GetSubtype(annot.get()));

    bool expected_focusable =
        pdfium::Contains(expected_focusable_subtypes, expected_subtypes[i]);
    EXPECT_EQ(expected_focusable,
              FORM_SetFocusedAnnot(form_handle, annot.get()));

    // Kill the focus so the next test starts in an unfocused state.
    FORM_ForceToKillFocus(form_handle);
  }
}

void VerifyUriActionInLink(FPDF_DOCUMENT doc,
                           FPDF_LINK link,
                           const std::string& expected_uri) {
  ASSERT_TRUE(link);

  FPDF_ACTION action = FPDFLink_GetAction(link);
  ASSERT_TRUE(action);
  EXPECT_EQ(static_cast<unsigned long>(PDFACTION_URI),
            FPDFAction_GetType(action));

  unsigned long bufsize = FPDFAction_GetURIPath(doc, action, nullptr, 0);
  ASSERT_EQ(expected_uri.size() + 1, bufsize);

  std::vector<char> buffer(bufsize);
  EXPECT_EQ(bufsize,
            FPDFAction_GetURIPath(doc, action, buffer.data(), bufsize));
  EXPECT_STREQ(expected_uri.c_str(), buffer.data());
}

}  // namespace

class FPDFAnnotEmbedderTest : public EmbedderTest {};

TEST_F(FPDFAnnotEmbedderTest, SetAP) {
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
      pdfium::size(kStreamData) * sizeof(FPDF_WCHAR);
  unsigned long normal_length_bytes = FPDFAnnot_GetAP(
      annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0);
  ASSERT_EQ(kStreamDataSize, normal_length_bytes);
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(normal_length_bytes);
  EXPECT_EQ(kStreamDataSize,
            FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                            buf.data(), normal_length_bytes));
  EXPECT_EQ(kStreamData, GetPlatformWString(buf.data()));
}

TEST_F(FPDFAnnotEmbedderTest, SetAPWithOpacity) {
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

TEST_F(FPDFAnnotEmbedderTest, InkListAPIValidations) {
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
      pdfium::size(kFirstInkStroke);

  static constexpr FS_POINTF kSecondInkStroke[] = {
      {70.0f, 90.0f}, {71.0f, 91.0f}, {72.0f, 92.0f}};
  static constexpr size_t kSecondStrokePointCount =
      pdfium::size(kSecondInkStroke);

  static constexpr FS_POINTF kThirdInkStroke[] = {{60.0f, 90.0f},
                                                  {61.0f, 91.0f},
                                                  {62.0f, 92.0f},
                                                  {63.0f, 93.0f},
                                                  {64.0f, 94.0f}};
  static constexpr size_t kThirdStrokePointCount =
      pdfium::size(kThirdInkStroke);

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

TEST_F(FPDFAnnotEmbedderTest, RemoveInkList) {
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
  static constexpr size_t kPointCount = pdfium::size(kInkStroke);

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

TEST_F(FPDFAnnotEmbedderTest, BadParams) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(0, FPDFPage_GetAnnotCount(nullptr));

  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, 0));
  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, -1));
  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, 1));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, -1));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, 1));

  EXPECT_EQ(FPDF_ANNOT_UNKNOWN, FPDFAnnot_GetSubtype(nullptr));

  EXPECT_EQ(0, FPDFAnnot_GetObjectCount(nullptr));

  EXPECT_FALSE(FPDFAnnot_GetObject(nullptr, 0));
  EXPECT_FALSE(FPDFAnnot_GetObject(nullptr, -1));
  EXPECT_FALSE(FPDFAnnot_GetObject(nullptr, 1));

  EXPECT_FALSE(FPDFAnnot_HasKey(nullptr, "foo"));

  static const wchar_t kContents[] = L"Bar";
  ScopedFPDFWideString text = GetFPDFWideString(kContents);
  EXPECT_FALSE(FPDFAnnot_SetStringValue(nullptr, "foo", text.get()));

  FPDF_WCHAR buffer[64];
  EXPECT_EQ(0u, FPDFAnnot_GetStringValue(nullptr, "foo", nullptr, 0));
  EXPECT_EQ(0u, FPDFAnnot_GetStringValue(nullptr, "foo", buffer, 0));
  EXPECT_EQ(0u,
            FPDFAnnot_GetStringValue(nullptr, "foo", buffer, sizeof(buffer)));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, BadAnnotsEntry) {
  ASSERT_TRUE(OpenDocument("bad_annots_entry.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, 0));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, RenderAnnotWithOnlyRolloverAP) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // This annotation has a malformed appearance stream, which does not have its
  // normal appearance defined, only its rollover appearance. In this case, its
  // normal appearance should be generated, allowing the highlight annotation to
  // still display.
  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, RenderMultilineMarkupAnnotWithoutAP) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static const char kChecksum[] = "ec1f4ccbd0aecfdea6d53893387a0101";
#else
  static const char kChecksum[] = "76512832d88017668d9acc7aacd13dae";
#endif
  // Open a file with multiline markup annotations.
  ASSERT_TRUE(OpenDocument("annotation_markup_multiline_no_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 595, 842, kChecksum);

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, ExtractHighlightLongContent) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = LoadPageNoEvents(0);
  ASSERT_TRUE(page);

  // Check that there is a total of 1 annotation on its first page.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Check that the annotation is of type "highlight".
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));

    // Check that the annotation color is yellow.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(255u, R);
    EXPECT_EQ(255u, G);
    EXPECT_EQ(0u, B);
    EXPECT_EQ(255u, A);

    // Check that the author is correct.
    static const char kAuthorKey[] = "T";
    EXPECT_EQ(FPDF_OBJECT_STRING,
              FPDFAnnot_GetValueType(annot.get(), kAuthorKey));
    unsigned long length_bytes =
        FPDFAnnot_GetStringValue(annot.get(), kAuthorKey, nullptr, 0);
    ASSERT_EQ(28u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(28u, FPDFAnnot_GetStringValue(annot.get(), kAuthorKey, buf.data(),
                                            length_bytes));
    EXPECT_EQ(L"Jae Hyun Park", GetPlatformWString(buf.data()));

    // Check that the content is correct.
    EXPECT_EQ(
        FPDF_OBJECT_STRING,
        FPDFAnnot_GetValueType(annot.get(), pdfium::annotation::kContents));
    length_bytes = FPDFAnnot_GetStringValue(
        annot.get(), pdfium::annotation::kContents, nullptr, 0);
    ASSERT_EQ(2690u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(2690u, FPDFAnnot_GetStringValue(annot.get(),
                                              pdfium::annotation::kContents,
                                              buf.data(), length_bytes));
    static const wchar_t kContents[] =
        L"This is a note for that highlight annotation. Very long highlight "
        "annotation. Long long long Long long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long longLong long longLong long longLong long longLong long "
        "longLong long long. END";
    EXPECT_EQ(kContents, GetPlatformWString(buf.data()));

    // Check that the quadpoints are correct.
    FS_QUADPOINTSF quadpoints;
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &quadpoints));
    EXPECT_EQ(115.802643f, quadpoints.x1);
    EXPECT_EQ(718.913940f, quadpoints.y1);
    EXPECT_EQ(157.211182f, quadpoints.x4);
    EXPECT_EQ(706.264465f, quadpoints.y4);
  }
  UnloadPageNoEvents(page);
}

TEST_F(FPDFAnnotEmbedderTest, ExtractInkMultiple) {
  // Open a file with three annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPageNoEvents(0);
  ASSERT_TRUE(page);

  // Check that there is a total of 3 annotation on its first page.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  {
    // Check that the third annotation is of type "ink".
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_INK, FPDFAnnot_GetSubtype(annot.get()));

    // Check that the annotation color is blue with opacity.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(0u, R);
    EXPECT_EQ(0u, G);
    EXPECT_EQ(255u, B);
    EXPECT_EQ(76u, A);

    // Check that there is no content.
    EXPECT_EQ(2u, FPDFAnnot_GetStringValue(
                      annot.get(), pdfium::annotation::kContents, nullptr, 0));

    // Check that the rectangle coordinates are correct.
    // Note that upon rendering, the rectangle coordinates will be adjusted.
    FS_RECTF rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_EQ(351.820404f, rect.left);
    EXPECT_EQ(583.830688f, rect.bottom);
    EXPECT_EQ(475.336090f, rect.right);
    EXPECT_EQ(681.535034f, rect.top);
  }
  {
#if defined(_SKIA_SUPPORT_) && defined(OS_APPLE)
    static constexpr char kExpectedHash[] = "fad91b9c968fe8019a774f5e2419b8fc";
#elif defined(_SKIA_SUPPORT_PATHS_) && defined(OS_APPLE)
    static constexpr char kExpectedHash[] = "acddfe688a117ead56af7b249a2cf8a1";
#elif defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static constexpr char kExpectedHash[] = "1fb0dd8dd5f0b9bb8d076e48eb59296d";
#elif defined(OS_WIN)
    static constexpr char kExpectedHash[] = "49d0a81c636531a337429325273d0508";
#else
    static constexpr char kExpectedHash[] = "354002e1c4386d38fdde29ef8d61074a";
#endif
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kExpectedHash);
  }
  UnloadPageNoEvents(page);
}

TEST_F(FPDFAnnotEmbedderTest, AddIllegalSubtypeAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Add an annotation with an illegal subtype.
  ASSERT_FALSE(FPDFPage_CreateAnnot(page, -1));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, AddFirstTextAnnotation) {
  // Open a file with no annotation and load its first page.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page));

  {
    // Add a text annotation to the page.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_TEXT));
    ASSERT_TRUE(annot);

    // Check that there is now 1 annotations on this page.
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

    // Check that the subtype of the annotation is correct.
    EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

    // Set the color of the annotation.
    ASSERT_TRUE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, 51,
                                   102, 153, 204));
    // Check that the color has been set correctly.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int A;
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(51u, R);
    EXPECT_EQ(102u, G);
    EXPECT_EQ(153u, B);
    EXPECT_EQ(204u, A);

    // Change the color of the annotation.
    ASSERT_TRUE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, 204,
                                   153, 102, 51));
    // Check that the color has been set correctly.
    ASSERT_TRUE(FPDFAnnot_GetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, &R,
                                   &G, &B, &A));
    EXPECT_EQ(204u, R);
    EXPECT_EQ(153u, G);
    EXPECT_EQ(102u, B);
    EXPECT_EQ(51u, A);

    // Set the annotation rectangle.
    FS_RECTF rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_EQ(0.f, rect.left);
    EXPECT_EQ(0.f, rect.right);
    rect.left = 35;
    rect.bottom = 150;
    rect.right = 53;
    rect.top = 165;
    ASSERT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));
    // Check that the annotation rectangle has been set correctly.
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_EQ(35.f, rect.left);
    EXPECT_EQ(150.f, rect.bottom);
    EXPECT_EQ(53.f, rect.right);
    EXPECT_EQ(165.f, rect.top);

    // Set the content of the annotation.
    static const wchar_t kContents[] = L"Hello! This is a customized content.";
    ScopedFPDFWideString text = GetFPDFWideString(kContents);
    ASSERT_TRUE(FPDFAnnot_SetStringValue(
        annot.get(), pdfium::annotation::kContents, text.get()));
    // Check that the content has been set correctly.
    unsigned long length_bytes = FPDFAnnot_GetStringValue(
        annot.get(), pdfium::annotation::kContents, nullptr, 0);
    ASSERT_EQ(74u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(74u, FPDFAnnot_GetStringValue(annot.get(),
                                            pdfium::annotation::kContents,
                                            buf.data(), length_bytes));
    EXPECT_EQ(kContents, GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, AddAndSaveLinkAnnotation) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 200, 200, pdfium::kHelloWorldChecksum);
  }
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page));

  constexpr char kUri[] = "https://pdfium.org/";

  {
    // Add a link annotation to the page and set its URI.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_LINK));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
    EXPECT_EQ(FPDF_ANNOT_LINK, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_TRUE(FPDFAnnot_SetURI(annot.get(), kUri));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()), kUri);

    // Negative tests:
    EXPECT_FALSE(FPDFAnnot_SetURI(nullptr, nullptr));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()), kUri);
    EXPECT_FALSE(FPDFAnnot_SetURI(annot.get(), nullptr));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()), kUri);
    EXPECT_FALSE(FPDFAnnot_SetURI(nullptr, kUri));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()), kUri);

    // Position the link on top of "Hello, world!" without a border.
    const FS_RECTF kRect = {19.0f, 48.0f, 85.0f, 60.0f};
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &kRect));
    EXPECT_TRUE(FPDFAnnot_SetBorder(annot.get(), /*horizontal_radius=*/0.0f,
                                    /*vertical_radius=*/0.0f,
                                    /*border_width=*/0.0f));

    VerifyUriActionInLink(document(), FPDFLink_GetLinkAtPoint(page, 40.0, 50.0),
                          kUri);
  }

  {
    // Add an ink annotation to the page. Trying to add a link to it fails.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_INK));
    ASSERT_TRUE(annot);
    EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));
    EXPECT_EQ(FPDF_ANNOT_INK, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_FALSE(FPDFAnnot_SetURI(annot.get(), kUri));
  }

  // Remove the ink annotation added above for negative testing.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page, 1));
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  // Save the document, closing the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Reopen the document and make sure it still renders the same. Since the link
  // does not have a border, it does not affect the rendering.
  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  ASSERT_TRUE(page);
  VerifySavedRendering(page, 200, 200, pdfium::kHelloWorldChecksum);
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_LINK, FPDFAnnot_GetSubtype(annot.get()));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()), kUri);
    VerifyUriActionInLink(document(), FPDFLink_GetLinkAtPoint(page, 40.0, 50.0),
                          kUri);
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, AddAndSaveUnderlineAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Check that there is a total of one annotation on its first page, and verify
  // its quadpoints.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
  FS_QUADPOINTSF quadpoints;
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &quadpoints));
    EXPECT_EQ(115.802643f, quadpoints.x1);
    EXPECT_EQ(718.913940f, quadpoints.y1);
    EXPECT_EQ(157.211182f, quadpoints.x4);
    EXPECT_EQ(706.264465f, quadpoints.y4);
  }

  // Add an underline annotation to the page and set its quadpoints.
  {
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page, FPDF_ANNOT_UNDERLINE));
    ASSERT_TRUE(annot);
    quadpoints.x1 = 140.802643f;
    quadpoints.x3 = 140.802643f;
    ASSERT_TRUE(FPDFAnnot_AppendAttachmentPoints(annot.get(), &quadpoints));
  }

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Open the saved document.
#if defined(_SKIA_SUPPORT_) && defined(OS_APPLE)
  static const char kChecksum[] = "899387ae792390cd0d83cf7e2bbebfb5";
#elif defined(_SKIA_SUPPORT_PATHS_) && defined(OS_APPLE)
  static const char kChecksum[] = "e40e235ee35f47ff28dda009aaaf36df";
#elif defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static const char kChecksum[] = "798fa41303381c9ba6d99092f5cd4d2b";
#else
  static const char kChecksum[] = "dba153419f67b7c0c0e3d22d3e8910d5";
#endif

  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  ASSERT_TRUE(page);
  VerifySavedRendering(page, 612, 792, kChecksum);

  // Check that the saved document has 2 annotations on the first page
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  {
    // Check that the second annotation is an underline annotation and verify
    // its quadpoints.
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(new_annot);
    EXPECT_EQ(FPDF_ANNOT_UNDERLINE, FPDFAnnot_GetSubtype(new_annot.get()));
    FS_QUADPOINTSF new_quadpoints;
    ASSERT_TRUE(
        FPDFAnnot_GetAttachmentPoints(new_annot.get(), 0, &new_quadpoints));
    EXPECT_NEAR(quadpoints.x1, new_quadpoints.x1, 0.001f);
    EXPECT_NEAR(quadpoints.y1, new_quadpoints.y1, 0.001f);
    EXPECT_NEAR(quadpoints.x4, new_quadpoints.x4, 0.001f);
    EXPECT_NEAR(quadpoints.y4, new_quadpoints.y4, 0.001f);
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, GetAndSetQuadPoints) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page));

  // Retrieve the highlight annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, 0);
  ASSERT_TRUE(annot);
  ASSERT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot));

  FS_QUADPOINTSF quadpoints;
  ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, 0, &quadpoints));

  {
    // Verify the current one set of quadpoints.
    ASSERT_EQ(1u, FPDFAnnot_CountAttachmentPoints(annot));

    EXPECT_NEAR(72.0000f, quadpoints.x1, 0.001f);
    EXPECT_NEAR(720.792f, quadpoints.y1, 0.001f);
    EXPECT_NEAR(132.055f, quadpoints.x4, 0.001f);
    EXPECT_NEAR(704.796f, quadpoints.y4, 0.001f);
  }

  {
    // Update the quadpoints.
    FS_QUADPOINTSF new_quadpoints = quadpoints;
    new_quadpoints.y1 -= 20.f;
    new_quadpoints.y2 -= 20.f;
    new_quadpoints.y3 -= 20.f;
    new_quadpoints.y4 -= 20.f;
    ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot, 0, &new_quadpoints));

    // Verify added quadpoint set
    ASSERT_EQ(1u, FPDFAnnot_CountAttachmentPoints(annot));
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, 0, &quadpoints));
    EXPECT_NEAR(new_quadpoints.x1, quadpoints.x1, 0.001f);
    EXPECT_NEAR(new_quadpoints.y1, quadpoints.y1, 0.001f);
    EXPECT_NEAR(new_quadpoints.x4, quadpoints.x4, 0.001f);
    EXPECT_NEAR(new_quadpoints.y4, quadpoints.y4, 0.001f);
  }

  {
    // Append a new set of quadpoints.
    FS_QUADPOINTSF new_quadpoints = quadpoints;
    new_quadpoints.y1 += 20.f;
    new_quadpoints.y2 += 20.f;
    new_quadpoints.y3 += 20.f;
    new_quadpoints.y4 += 20.f;
    ASSERT_TRUE(FPDFAnnot_AppendAttachmentPoints(annot, &new_quadpoints));

    // Verify added quadpoint set
    ASSERT_EQ(2u, FPDFAnnot_CountAttachmentPoints(annot));
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot, 1, &quadpoints));
    EXPECT_NEAR(new_quadpoints.x1, quadpoints.x1, 0.001f);
    EXPECT_NEAR(new_quadpoints.y1, quadpoints.y1, 0.001f);
    EXPECT_NEAR(new_quadpoints.x4, quadpoints.x4, 0.001f);
    EXPECT_NEAR(new_quadpoints.y4, quadpoints.y4, 0.001f);
  }

  {
    // Setting and getting quadpoints at out-of-bound index should fail
    EXPECT_FALSE(FPDFAnnot_SetAttachmentPoints(annot, 300000, &quadpoints));
    EXPECT_FALSE(FPDFAnnot_GetAttachmentPoints(annot, 300000, &quadpoints));
  }

  FPDFPage_CloseAnnot(annot);

  // Retrieve the square annotation
  FPDF_ANNOTATION squareAnnot = FPDFPage_GetAnnot(page, 2);

  {
    // Check that attempting to set its quadpoints would fail
    ASSERT_TRUE(squareAnnot);
    EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(squareAnnot));
    EXPECT_EQ(0u, FPDFAnnot_CountAttachmentPoints(squareAnnot));
    EXPECT_FALSE(FPDFAnnot_SetAttachmentPoints(squareAnnot, 0, &quadpoints));
  }

  FPDFPage_CloseAnnot(squareAnnot);
  UnloadPage(page);
}

// TODO(crbug.com/pdfium/1569): Fix this issue and enable the test for Skia.
#if defined(_SKIA_SUPPORT_)
#define MAYBE_ModifyRectQuadpointsWithAP DISABLED_ModifyRectQuadpointsWithAP
#else
#define MAYBE_ModifyRectQuadpointsWithAP ModifyRectQuadpointsWithAP
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_ModifyRectQuadpointsWithAP) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static const char kMd5Original[] = "00e70eb543c2a6e8f8aafb4ee951d9bf";
  static const char kMd5ModifiedHighlight[] =
      "7638c4a8fe4aabbf8704e198f49b3198";
  static const char kMd5ModifiedSquare[] = "54f507af6af63de877b9cafdab1bbdaa";
#else
#if defined(OS_WIN)
  static const char kMd5Original[] = "0e27376094f11490f74c65f3dc3a42c5";
  static const char kMd5ModifiedHighlight[] =
      "66f3caef3a7d488a4fa1ad37fc06310e";
  static const char kMd5ModifiedSquare[] = "a456dad0bc6801ee2d6408a4394af563";
#elif defined(OS_APPLE)
  static const char kMd5Original[] = "fc59468d154f397fd298c69f47ef565a";
  static const char kMd5ModifiedHighlight[] =
      "e64bf648f6e9354d1f3eedb47a2c9498";
  static const char kMd5ModifiedSquare[] = "a66591662c8e7ad3c6059952e234bebf";
#else
  static const char kMd5Original[] = "0e27376094f11490f74c65f3dc3a42c5";
  static const char kMd5ModifiedHighlight[] =
      "66f3caef3a7d488a4fa1ad37fc06310e";
  static const char kMd5ModifiedSquare[] = "a456dad0bc6801ee2d6408a4394af563";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page));

  // Check that the original file renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5Original);
  }

  FS_RECTF rect;
  FS_RECTF new_rect;

  // Retrieve the highlight annotation which has its AP stream already defined.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));

    // Check that color cannot be set when an AP stream is defined already.
    EXPECT_FALSE(FPDFAnnot_SetColor(annot.get(), FPDFANNOT_COLORTYPE_Color, 51,
                                    102, 153, 204));

    // Verify its attachment points.
    FS_QUADPOINTSF quadpoints;
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &quadpoints));
    EXPECT_NEAR(72.0000f, quadpoints.x1, 0.001f);
    EXPECT_NEAR(720.792f, quadpoints.y1, 0.001f);
    EXPECT_NEAR(132.055f, quadpoints.x4, 0.001f);
    EXPECT_NEAR(704.796f, quadpoints.y4, 0.001f);

    // Check that updating the attachment points would succeed.
    quadpoints.x1 -= 50.f;
    quadpoints.x2 -= 50.f;
    quadpoints.x3 -= 50.f;
    quadpoints.x4 -= 50.f;
    ASSERT_TRUE(FPDFAnnot_SetAttachmentPoints(annot.get(), 0, &quadpoints));
    FS_QUADPOINTSF new_quadpoints;
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &new_quadpoints));
    EXPECT_EQ(quadpoints.x1, new_quadpoints.x1);
    EXPECT_EQ(quadpoints.y1, new_quadpoints.y1);
    EXPECT_EQ(quadpoints.x4, new_quadpoints.x4);
    EXPECT_EQ(quadpoints.y4, new_quadpoints.y4);

    // Check that updating quadpoints does not change the annotation's position.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, kMd5Original);
    }

    // Verify its annotation rectangle.
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(67.7299f, rect.left, 0.001f);
    EXPECT_NEAR(704.296f, rect.bottom, 0.001f);
    EXPECT_NEAR(136.325f, rect.right, 0.001f);
    EXPECT_NEAR(721.292f, rect.top, 0.001f);

    // Check that updating the rectangle would succeed.
    rect.left -= 60.f;
    rect.right -= 60.f;
    ASSERT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.right, new_rect.right);
  }

  // Check that updating the rectangle changes the annotation's position.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5ModifiedHighlight);
  }

  {
    // Retrieve the square annotation which has its AP stream already defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(annot.get()));

    // Check that updating the rectangle would succeed.
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    rect.left += 70.f;
    rect.right += 70.f;
    ASSERT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.right, new_rect.right);

    // Check that updating the rectangle changes the square annotation's
    // position.
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5ModifiedSquare);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, CountAttachmentPoints) {
  // Open a file with multiline markup annotations.
  ASSERT_TRUE(OpenDocument("annotation_markup_multiline_no_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // This is a three line annotation.
    EXPECT_EQ(3u, FPDFAnnot_CountAttachmentPoints(annot.get()));
  }
  UnloadPage(page);

  // null annotation should return 0
  EXPECT_EQ(0u, FPDFAnnot_CountAttachmentPoints(nullptr));
}

TEST_F(FPDFAnnotEmbedderTest, RemoveAnnotation) {
  // Open a file with 3 annotations on its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPageNoEvents(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  FS_RECTF rect;

  // Check that the annotations have the expected rectangle coordinates.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(149.8127f, rect.left, 0.001f);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  }

  // Check that nothing happens when attempting to remove an annotation with an
  // out-of-bound index.
  EXPECT_FALSE(FPDFPage_RemoveAnnot(page, 4));
  EXPECT_FALSE(FPDFPage_RemoveAnnot(page, -1));
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  // Remove the second annotation.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page, 1));
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));
  EXPECT_FALSE(FPDFPage_GetAnnot(page, 2));

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPageNoEvents(page);

  // TODO(npm): VerifySavedRendering changes annot rect dimensions by 1??
  // Open the saved document.
  std::string new_file = GetString();
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = new_file.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &new_file;
  FPDF_DOCUMENT new_doc = FPDF_LoadCustomDocument(&file_access, nullptr);
  ASSERT_TRUE(new_doc);
  FPDF_PAGE new_page = FPDF_LoadPage(new_doc, 0);
  ASSERT_TRUE(new_page);

  // Check that the saved document has 2 annotations on the first page.
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(new_page));

  // Check that the remaining 2 annotations are the original 1st and 3rd ones
  // by verifying their rectangle coordinates.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(new_page, 0));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(new_page, 1));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  }
  FPDF_ClosePage(new_page);
  FPDF_CloseDocument(new_doc);
}

TEST_F(FPDFAnnotEmbedderTest, AddAndModifyPath) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  static const char kMd5ModifiedPath[] = "d76382fd57fafad0233f7549f871dafa";
  static const char kMd5TwoPaths[] = "323151317b8cb62130546c7b8d70f622";
  static const char kMd5NewAnnot[] = "9a3b02d876620d19787549ee1100b63c";
#else
  static const char kMd5ModifiedPath[] = "c9ba60887a312370d9a32198aa53aca4";
  static const char kMd5TwoPaths[] = "0768d56373094fcdf4ddf3f3364c006f";
  static const char kMd5NewAnnot[] = "6f7e1c189bcfac90ffccf2a527857006";
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)
#else
#if defined(OS_WIN)
  static const char kMd5ModifiedPath[] = "a7a8d675a6ddbcbdfecee65a33ba19e1";
  static const char kMd5TwoPaths[] = "7c0bdd4552329704c47a7cce47edbbd6";
  static const char kMd5NewAnnot[] = "3c48d492b4f62941fed0fb62f729f31e";
#elif defined(OS_APPLE)
  static const char kMd5ModifiedPath[] = "8cfae6d547fc5d6702f5f1ac631beb5e";
  static const char kMd5TwoPaths[] = "9677e4892bb02950d3e4dbe74470578f";
  static const char kMd5NewAnnot[] = "e8ebddac4db8c0a4b556ddf79aa1a26d";
#else
  static const char kMd5ModifiedPath[] = "6ff77d6d1fec4ea571fabe0c7a19b517";
  static const char kMd5TwoPaths[] = "ca37ad549e74ac5b359a055708f3e7b6";
  static const char kMd5NewAnnot[] = "0d7a0e33fbf41ff7fa5d732ab2c5edff";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kAnnotationStampWithApChecksum);
  }

  {
    // Retrieve the stamp annotation which has its AP stream already defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that this annotation has one path object and retrieve it.
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    ASSERT_EQ(32, FPDFPage_CountObjects(page));
    FPDF_PAGEOBJECT path = FPDFAnnot_GetObject(annot.get(), 1);
    EXPECT_FALSE(path);
    path = FPDFAnnot_GetObject(annot.get(), 0);
    EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(path));
    EXPECT_TRUE(path);

    // Modify the color of the path object.
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(path, 0, 0, 0, 255));
    EXPECT_TRUE(FPDFAnnot_UpdateObject(annot.get(), path));

    // Check that the page with the modified annotation renders correctly.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 595, 842, kMd5ModifiedPath);
    }

    // Add a second path object to the same annotation.
    FPDF_PAGEOBJECT dot = FPDFPageObj_CreateNewPath(7, 84);
    EXPECT_TRUE(FPDFPath_BezierTo(dot, 9, 86, 10, 87, 11, 88));
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(dot, 255, 0, 0, 100));
    EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(dot, 14));
    EXPECT_TRUE(FPDFPath_SetDrawMode(dot, 0, 1));
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), dot));
    EXPECT_EQ(2, FPDFAnnot_GetObjectCount(annot.get()));

    // The object is in the annontation, not in the page, so the page object
    // array should not change.
    ASSERT_EQ(32, FPDFPage_CountObjects(page));

    // Check that the page with an annotation with two paths renders correctly.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 595, 842, kMd5TwoPaths);
    }

    // Delete the newly added path object.
    EXPECT_TRUE(FPDFAnnot_RemoveObject(annot.get(), 1));
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    ASSERT_EQ(32, FPDFPage_CountObjects(page));
  }

  // Check that the page renders the same as before.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5ModifiedPath);
  }

  FS_RECTF rect;

  {
    // Create another stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    rect.left = 200.f;
    rect.bottom = 400.f;
    rect.right = 500.f;
    rect.top = 600.f;
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));

    // Add a new path to the annotation.
    FPDF_PAGEOBJECT check = FPDFPageObj_CreateNewPath(200, 500);
    EXPECT_TRUE(FPDFPath_LineTo(check, 300, 400));
    EXPECT_TRUE(FPDFPath_LineTo(check, 500, 600));
    EXPECT_TRUE(FPDFPath_MoveTo(check, 350, 550));
    EXPECT_TRUE(FPDFPath_LineTo(check, 450, 450));
    EXPECT_TRUE(FPDFPageObj_SetStrokeColor(check, 0, 255, 255, 180));
    EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(check, 8.35f));
    EXPECT_TRUE(FPDFPath_SetDrawMode(check, 0, 1));
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), check));
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));

    // Check that the annotation's bounding box came from its rectangle.
    FS_RECTF new_rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.left, new_rect.left);
    EXPECT_EQ(rect.bottom, new_rect.bottom);
    EXPECT_EQ(rect.right, new_rect.right);
    EXPECT_EQ(rect.top, new_rect.top);
  }

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Open the saved document.
  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  ASSERT_TRUE(page);
  VerifySavedRendering(page, 595, 842, kMd5NewAnnot);

  // Check that the document has a correct count of annotations and objects.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));

    // Check that the new annotation's rectangle is as defined.
    FS_RECTF new_rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &new_rect));
    EXPECT_EQ(rect.left, new_rect.left);
    EXPECT_EQ(rect.bottom, new_rect.bottom);
    EXPECT_EQ(rect.right, new_rect.right);
    EXPECT_EQ(rect.top, new_rect.top);
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, ModifyAnnotationFlags) {
  // Open a file with an annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
  }

  {
    // Retrieve the annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the original flag values are as expected.
    int flags = FPDFAnnot_GetFlags(annot.get());
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_INVISIBLE);
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_HIDDEN);
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_NOZOOM);
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_NOROTATE);
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_NOVIEW);
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_LOCKED);
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_TOGGLENOVIEW);

    // Set the HIDDEN flag.
    flags |= FPDF_ANNOT_FLAG_HIDDEN;
    EXPECT_TRUE(FPDFAnnot_SetFlags(annot.get(), flags));
    flags = FPDFAnnot_GetFlags(annot.get());
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_HIDDEN);
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

    // Check that the page renders correctly without rendering the annotation.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, pdfium::kBlankPage612By792Checksum);
    }

    // Unset the HIDDEN flag.
    EXPECT_TRUE(FPDFAnnot_SetFlags(annot.get(), FPDF_ANNOT_FLAG_NONE));
    EXPECT_FALSE(FPDFAnnot_GetFlags(annot.get()));
    flags &= ~FPDF_ANNOT_FLAG_HIDDEN;
    EXPECT_TRUE(FPDFAnnot_SetFlags(annot.get(), flags));
    flags = FPDFAnnot_GetFlags(annot.get());
    EXPECT_FALSE(flags & FPDF_ANNOT_FLAG_HIDDEN);
    EXPECT_TRUE(flags & FPDF_ANNOT_FLAG_PRINT);

    // Check that the page renders correctly as before.
    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
    }
  }

  UnloadPage(page);
}

// TODO(crbug.com/pdfium/1541): Fix this test and enable.
#if defined(_SKIA_SUPPORT_)
#define MAYBE_AddAndModifyImage DISABLED_AddAndModifyImage
#else
#define MAYBE_AddAndModifyImage AddAndModifyImage
#endif
TEST_F(FPDFAnnotEmbedderTest, MAYBE_AddAndModifyImage) {
#if defined(_SKIA_SUPPORT_PATHS_)
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  static const char kMd5NewImage[] = "26a8eb30937226a677839379e0d7ae1a";
  static const char kMd5ModifiedImage[] = "2985114b32ba1a96be78ee643fe31aa5";
#else
  static const char kMd5NewImage[] = "14012ab500b4671fa73dd760129a8a93";
  static const char kMd5ModifiedImage[] = "5f97f98f58ed04dc393f31460485f1a2";
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)
#else
#if defined(OS_APPLE)
  static const char kMd5NewImage[] = "dd18709d90c245a12ce0b8c4d092bea9";
  static const char kMd5ModifiedImage[] = "8d6f478ff8c7e67d49b253f1af587a99";
#elif defined(OS_WIN)
  static const char kMd5NewImage[] = "3d77d06a971bcb9fb54db082f1082c8b";
  static const char kMd5ModifiedImage[] = "dc4f4afc26c345418330d31c065020e1";
#else
  static const char kMd5NewImage[] = "528e6243dc29d54f36b61e0d3287d935";
  static const char kMd5ModifiedImage[] = "6d9e59f3e57a1ff82fb258356b7eb731";
#endif
#endif  // defined(_SKIA_SUPPORT_PATHS_)

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kAnnotationStampWithApChecksum);
  }

  constexpr int kBitmapSize = 200;
  FPDF_BITMAP image_bitmap;

  {
    // Create a stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    FS_RECTF rect;
    rect.left = 200.f;
    rect.bottom = 600.f;
    rect.right = 400.f;
    rect.top = 800.f;
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));

    // Add a solid-color translucent image object to the new annotation.
    image_bitmap = FPDFBitmap_Create(kBitmapSize, kBitmapSize, 1);
    FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize, kBitmapSize,
                        0xeeeecccc);
    EXPECT_EQ(kBitmapSize, FPDFBitmap_GetWidth(image_bitmap));
    EXPECT_EQ(kBitmapSize, FPDFBitmap_GetHeight(image_bitmap));
    FPDF_PAGEOBJECT image_object = FPDFPageObj_NewImageObj(document());
    ASSERT_TRUE(FPDFImageObj_SetBitmap(&page, 0, image_object, image_bitmap));
    static constexpr FS_MATRIX kBitmapScaleMatrix{kBitmapSize, 0, 0,
                                                  kBitmapSize, 0, 0};
    ASSERT_TRUE(FPDFPageObj_SetMatrix(image_object, &kBitmapScaleMatrix));
    FPDFPageObj_Transform(image_object, 1, 0, 0, 1, 200, 600);
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), image_object));
  }

  // Check that the page renders correctly with the new image object.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5NewImage);
  }

  {
    // Retrieve the newly added stamp annotation and its image object.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    FPDF_PAGEOBJECT image_object = FPDFAnnot_GetObject(annot.get(), 0);
    EXPECT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image_object));

    // Modify the image in the new annotation.
    FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize, kBitmapSize,
                        0xff000000);
    ASSERT_TRUE(FPDFImageObj_SetBitmap(&page, 0, image_object, image_bitmap));
    EXPECT_TRUE(FPDFAnnot_UpdateObject(annot.get(), image_object));
  }

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);
  FPDFBitmap_Destroy(image_bitmap);

  // Test that the saved document renders the modified image object correctly.
  VerifySavedDocument(595, 842, kMd5ModifiedImage);
}

TEST_F(FPDFAnnotEmbedderTest, AddAndModifyText) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  static const char kMd5NewText[] = "c9d853a5fb6bca31e9696ccc4462c74a";
  static const char kMd5ModifiedText[] = "bc681fa9174223983c5e4357e919d36c";
#else
  static const char kMd5NewText[] = "4aaa34e9df2e41d621dbd81b1d535c48";
  static const char kMd5ModifiedText[] = "d6ea20beb7834ef4b6d370581ce425fc";
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)
#else
#if defined(OS_WIN)
  static const char kMd5NewText[] = "204cc01749a70b8afc246a4ca33c7eb6";
  static const char kMd5ModifiedText[] = "641261a45e8dfd68c89b80bfd237660d";
#elif defined(OS_APPLE)
  static const char kMd5NewText[] = "e657266260b88c964938efe6c9b292da";
  static const char kMd5ModifiedText[] = "7accdf2bac64463101783221f53d3188";
#else
  static const char kMd5NewText[] = "00197ad6206f763febad5719e5935306";
  static const char kMd5ModifiedText[] = "85853bc0aaa5a4e3af04e58b9cbfff23";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kAnnotationStampWithApChecksum);
  }

  {
    // Create a stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    FS_RECTF rect;
    rect.left = 200.f;
    rect.bottom = 550.f;
    rect.right = 450.f;
    rect.top = 650.f;
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));

    // Add a translucent text object to the new annotation.
    FPDF_PAGEOBJECT text_object =
        FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
    EXPECT_TRUE(text_object);
    ScopedFPDFWideString text =
        GetFPDFWideString(L"I'm a translucent text laying on other text.");
    EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
    EXPECT_TRUE(FPDFPageObj_SetFillColor(text_object, 0, 0, 255, 150));
    FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 200, 600);
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), text_object));
  }

  // Check that the page renders correctly with the new text object.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5NewText);
  }

  {
    // Retrieve the newly added stamp annotation and its text object.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    FPDF_PAGEOBJECT text_object = FPDFAnnot_GetObject(annot.get(), 0);
    EXPECT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object));

    // Modify the text in the new annotation.
    ScopedFPDFWideString new_text = GetFPDFWideString(L"New text!");
    EXPECT_TRUE(FPDFText_SetText(text_object, new_text.get()));
    EXPECT_TRUE(FPDFAnnot_UpdateObject(annot.get(), text_object));
  }

  // Check that the page renders correctly with the modified text object.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kMd5ModifiedText);
  }

  // Remove the new annotation, and check that the page renders as before.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page, 2));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, kAnnotationStampWithApChecksum);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetSetStringValue) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  static const wchar_t kNewDate[] = L"D:201706282359Z00'00'";

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that a non-existent key does not exist.
    EXPECT_FALSE(FPDFAnnot_HasKey(annot.get(), "none"));

    // Check that the string value of a non-string dictionary entry is empty.
    EXPECT_TRUE(FPDFAnnot_HasKey(annot.get(), pdfium::annotation::kAP));
    EXPECT_EQ(FPDF_OBJECT_REFERENCE,
              FPDFAnnot_GetValueType(annot.get(), pdfium::annotation::kAP));
    EXPECT_EQ(2u, FPDFAnnot_GetStringValue(annot.get(), pdfium::annotation::kAP,
                                           nullptr, 0));

    // Check that the string value of the hash is correct.
    static const char kHashKey[] = "AAPL:Hash";
    EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot.get(), kHashKey));
    unsigned long length_bytes =
        FPDFAnnot_GetStringValue(annot.get(), kHashKey, nullptr, 0);
    ASSERT_EQ(66u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(66u, FPDFAnnot_GetStringValue(annot.get(), kHashKey, buf.data(),
                                            length_bytes));
    EXPECT_EQ(L"395fbcb98d558681742f30683a62a2ad",
              GetPlatformWString(buf.data()));

    // Check that the string value of the modified date is correct.
    EXPECT_EQ(FPDF_OBJECT_NAME, FPDFAnnot_GetValueType(annot.get(), kHashKey));
    length_bytes = FPDFAnnot_GetStringValue(annot.get(), pdfium::annotation::kM,
                                            nullptr, 0);
    ASSERT_EQ(44u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(44u, FPDFAnnot_GetStringValue(annot.get(), pdfium::annotation::kM,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"D:201706071721Z00'00'", GetPlatformWString(buf.data()));

    // Update the date entry for the annotation.
    ScopedFPDFWideString text = GetFPDFWideString(kNewDate);
    EXPECT_TRUE(FPDFAnnot_SetStringValue(annot.get(), pdfium::annotation::kM,
                                         text.get()));
  }

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
  static const char kMd5[] = "7a2b712ca88d7b71f125ea3f9c88e57a";
#else
  static const char kMd5[] = "626d25c5aa5baf67d22d9a0e1c23f6aa";
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)
#else
#if defined(OS_APPLE)
  static const char kMd5[] = "5e7e185b386ad21ca83b0287268c50fb";
#elif defined(OS_WIN)
  static const char kMd5[] = "20b612ebd46babcb44c48c903e2c5a48";
#else
  static const char kMd5[] = "1d7bea2042c6fea0558ff2aef05811b5";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

  // Open the saved annotation.
  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  ASSERT_TRUE(page);
  VerifySavedRendering(page, 595, 842, kMd5);
  {
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(page, 0));

    // Check that the string value of the modified date is the newly-set value.
    EXPECT_EQ(FPDF_OBJECT_STRING,
              FPDFAnnot_GetValueType(new_annot.get(), pdfium::annotation::kM));
    unsigned long length_bytes = FPDFAnnot_GetStringValue(
        new_annot.get(), pdfium::annotation::kM, nullptr, 0);
    ASSERT_EQ(44u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(44u,
              FPDFAnnot_GetStringValue(new_annot.get(), pdfium::annotation::kM,
                                       buf.data(), length_bytes));
    EXPECT_EQ(kNewDate, GetPlatformWString(buf.data()));
  }

  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, GetNumberValue) {
  // Open a file with four text annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  {
    // First two annotations do not have "MaxLen" attribute.
    for (int i = 0; i < 2; i++) {
      ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, i));
      ASSERT_TRUE(annot);

      // Verify that no "MaxLen" key present.
      EXPECT_FALSE(FPDFAnnot_HasKey(annot.get(), "MaxLen"));

      float value;
      EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), "MaxLen", &value));
    }

    // Annotation in index 2 has "MaxLen" of 10.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    // Verify that "MaxLen" key present.
    EXPECT_TRUE(FPDFAnnot_HasKey(annot.get(), "MaxLen"));

    float value;
    EXPECT_TRUE(FPDFAnnot_GetNumberValue(annot.get(), "MaxLen", &value));
    EXPECT_FLOAT_EQ(10.0f, value);

    // Check bad inputs.
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(nullptr, "MaxLen", &value));
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), nullptr, &value));
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), "MaxLen", nullptr));
    // Ask for key that exists but is not a number.
    EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), "V", &value));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetSetAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    static const char kMd5NormalAP[] = "be903df0343fd774fadab9c8900cdf4a";
    static constexpr size_t kExpectNormalAPLength = 73970;

    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the string value of an AP returns the expected length.
    unsigned long normal_length_bytes = FPDFAnnot_GetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0);
    ASSERT_EQ(kExpectNormalAPLength, normal_length_bytes);

    // Check that the string value of an AP is not returned if the buffer is too
    // small. The result buffer should be overwritten with an empty string.
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(normal_length_bytes);
    // Write in the buffer to verify it's not overwritten.
    memcpy(buf.data(), "abcdefgh", 8);
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes - 1));
    EXPECT_EQ(0, memcmp(buf.data(), "abcdefgh", 8));

    // Check that the string value of an AP is returned through a buffer that is
    // the right size.
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes));
    EXPECT_EQ(kMd5NormalAP,
              GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()),
                                normal_length_bytes));

    // Check that the string value of an AP is returned through a buffer that is
    // larger than necessary.
    buf = GetFPDFWideStringBuffer(normal_length_bytes + 2);
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes + 2));
    EXPECT_EQ(kMd5NormalAP,
              GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()),
                                normal_length_bytes));

    // Check that getting an AP for a mode that does not have an AP returns an
    // empty string.
    unsigned long rollover_length_bytes = FPDFAnnot_GetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
    ASSERT_EQ(2u, rollover_length_bytes);

    buf = GetFPDFWideStringBuffer(1000);
    EXPECT_EQ(2u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), 1000));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));

    // Check that setting the AP for an invalid appearance mode fails.
    ScopedFPDFWideString ap_text = GetFPDFWideString(L"new test ap");
    EXPECT_FALSE(FPDFAnnot_SetAP(annot.get(), -1, ap_text.get()));
    EXPECT_FALSE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_COUNT,
                                 ap_text.get()));
    EXPECT_FALSE(FPDFAnnot_SetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_COUNT + 1, ap_text.get()));

    // Set the AP correctly now.
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                ap_text.get()));

    // Check that the new annotation value is equal to the value we just set.
    rollover_length_bytes = FPDFAnnot_GetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
    ASSERT_EQ(24u, rollover_length_bytes);
    buf = GetFPDFWideStringBuffer(rollover_length_bytes);
    EXPECT_EQ(24u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), rollover_length_bytes));
    EXPECT_EQ(L"new test ap", GetPlatformWString(buf.data()));

    // Check that the Normal AP was not touched when the Rollover AP was set.
    buf = GetFPDFWideStringBuffer(normal_length_bytes);
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes));
    EXPECT_EQ(kMd5NormalAP,
              GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()),
                                normal_length_bytes));
  }

  // Save the modified document, then reopen it.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(0);
  ASSERT_TRUE(page);
  {
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(page, 0));

    // Check that the new annotation value is equal to the value we set before
    // saving.
    unsigned long rollover_length_bytes = FPDFAnnot_GetAP(
        new_annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER, nullptr, 0);
    ASSERT_EQ(24u, rollover_length_bytes);
    std::vector<FPDF_WCHAR> buf =
        GetFPDFWideStringBuffer(rollover_length_bytes);
    EXPECT_EQ(24u, FPDFAnnot_GetAP(new_annot.get(),
                                   FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                   buf.data(), rollover_length_bytes));
    EXPECT_EQ(L"new test ap", GetPlatformWString(buf.data()));
  }

  // Close saved document.
  CloseSavedPage(page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, RemoveOptionalAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Set Down AP. Normal AP is already set.
    ScopedFPDFWideString ap_text = GetFPDFWideString(L"new test ap");
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                ap_text.get()));
    EXPECT_EQ(73970u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              nullptr, 0));
    EXPECT_EQ(24u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                   nullptr, 0));

    // Check that setting the Down AP to null removes the Down entry but keeps
    // Normal intact.
    EXPECT_TRUE(
        FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN, nullptr));
    EXPECT_EQ(73970u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              nullptr, 0));
    EXPECT_EQ(2u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                  nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, RemoveRequiredAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Set Down AP. Normal AP is already set.
    ScopedFPDFWideString ap_text = GetFPDFWideString(L"new test ap");
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                ap_text.get()));
    EXPECT_EQ(73970u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              nullptr, 0));
    EXPECT_EQ(24u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                   nullptr, 0));

    // Check that setting the Normal AP to null removes the whole AP dictionary.
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                nullptr));
    EXPECT_EQ(2u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                                  nullptr, 0));
    EXPECT_EQ(2u, FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_DOWN,
                                  nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, ExtractLinkedAnnotations) {
  // Open a file with annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(-1, FPDFPage_GetAnnotIndex(page, nullptr));

  {
    // Retrieve the highlight annotation which has its popup defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_EQ(0, FPDFPage_GetAnnotIndex(page, annot.get()));
    static const char kPopupKey[] = "Popup";
    ASSERT_TRUE(FPDFAnnot_HasKey(annot.get(), kPopupKey));
    ASSERT_EQ(FPDF_OBJECT_REFERENCE,
              FPDFAnnot_GetValueType(annot.get(), kPopupKey));

    // Retrieve and verify the popup of the highlight annotation.
    ScopedFPDFAnnotation popup(
        FPDFAnnot_GetLinkedAnnot(annot.get(), kPopupKey));
    ASSERT_TRUE(popup);
    EXPECT_EQ(FPDF_ANNOT_POPUP, FPDFAnnot_GetSubtype(popup.get()));
    EXPECT_EQ(1, FPDFPage_GetAnnotIndex(page, popup.get()));
    FS_RECTF rect;
    ASSERT_TRUE(FPDFAnnot_GetRect(popup.get(), &rect));
    EXPECT_NEAR(612.0f, rect.left, 0.001f);
    EXPECT_NEAR(578.792, rect.bottom, 0.001f);

    // Attempting to retrieve |annot|'s "IRT"-linked annotation would fail,
    // since "IRT" is not a key in |annot|'s dictionary.
    static const char kIRTKey[] = "IRT";
    ASSERT_FALSE(FPDFAnnot_HasKey(annot.get(), kIRTKey));
    EXPECT_FALSE(FPDFAnnot_GetLinkedAnnot(annot.get(), kIRTKey));

    // Attempting to retrieve |annot|'s parent dictionary as an annotation
    // would fail, since its parent is not an annotation.
    ASSERT_TRUE(FPDFAnnot_HasKey(annot.get(), pdfium::annotation::kP));
    EXPECT_EQ(FPDF_OBJECT_REFERENCE,
              FPDFAnnot_GetValueType(annot.get(), pdfium::annotation::kP));
    EXPECT_FALSE(FPDFAnnot_GetLinkedAnnot(annot.get(), pdfium::annotation::kP));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldFlagsTextField) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation: user-editable text field.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_MULTILINE);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_PASSWORD);
  }

  {
    // Retrieve the second annotation: read-only text field.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_MULTILINE);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_PASSWORD);
  }

  {
    // Retrieve the fourth annotation: user-editable password text field.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 3));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_MULTILINE);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_TEXT_PASSWORD);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldFlagsComboBox) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation: user-editable combobox.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_MULTI_SELECT);
  }

  {
    // Retrieve the second annotation: regular combobox.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_MULTI_SELECT);
  }

  {
    // Retrieve the third annotation: read-only combobox.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_MULTI_SELECT);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotNull) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Attempt to get an annotation where no annotation exists on page.
  static const FS_POINTF kOriginPoint = {0.0f, 0.0f};
  EXPECT_FALSE(
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kOriginPoint));

  static const FS_POINTF kValidPoint = {120.0f, 120.0f};
  {
    // Verify there is an annotation.
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kValidPoint));
    EXPECT_TRUE(annot);
  }

  // Try other bad inputs at a valid location.
  EXPECT_FALSE(FPDFAnnot_GetFormFieldAtPoint(nullptr, nullptr, &kValidPoint));
  EXPECT_FALSE(FPDFAnnot_GetFormFieldAtPoint(nullptr, page, &kValidPoint));
  EXPECT_FALSE(
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), nullptr, &kValidPoint));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotAndCheckFlagsTextField) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve user-editable text field annotation.
    static const FS_POINTF kPoint = {105.0f, 118.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
  }

  {
    // Retrieve read-only text field annotation.
    static const FS_POINTF kPoint = {105.0f, 202.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotAndCheckFlagsComboBox) {
  // Open file with form comboboxes.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve user-editable combobox annotation.
    static const FS_POINTF kPoint = {102.0f, 363.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_MULTI_SELECT);
  }

  {
    // Retrieve regular combobox annotation.
    static const FS_POINTF kPoint = {102.0f, 413.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_MULTI_SELECT);
  }

  {
    // Retrieve read-only combobox annotation.
    static const FS_POINTF kPoint = {102.0f, 513.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page, &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_CHOICE_COMBO);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_EDIT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_CHOICE_MULTI_SELECT);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, BUG_1206) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static const char kExpectedBitmap[] = "a1ea1ceebb26922fae576cb79ce63af0";
#else
  static const char kExpectedBitmap[] = "0d9fc05c6762fd788bd23fd87a4967bc";
#endif
  static constexpr size_t kExpectedSize = 1609;

  ASSERT_TRUE(OpenDocument("bug_1206.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  EXPECT_EQ(kExpectedSize, GetString().size());
  ClearString();

  for (size_t i = 0; i < 10; ++i) {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kExpectedBitmap);

    ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    // TODO(https://crbug.com/pdfium/1206): This is wrong. The size should be
    // equal, not bigger.
    EXPECT_LT(kExpectedSize, GetString().size());
    ClearString();
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, BUG_1212) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page));

  static const char kTestKey[] = "test";
  static const wchar_t kData[] = L"\xf6\xe4";
  static const size_t kBufSize = 12;
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(kBufSize);

  {
    // Add a text annotation to the page.
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_TEXT));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));
    EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

    // Make sure there is no test key, add set a value there, and read it back.
    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(2u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                           kBufSize));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));

    ScopedFPDFWideString text = GetFPDFWideString(kData);
    EXPECT_TRUE(FPDFAnnot_SetStringValue(annot.get(), kTestKey, text.get()));

    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(6u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                           kBufSize));
    EXPECT_EQ(kData, GetPlatformWString(buf.data()));
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    const FS_RECTF bounding_rect{206.0f, 753.0f, 339.0f, 709.0f};
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &bounding_rect));
    EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));
    EXPECT_EQ(FPDF_ANNOT_STAMP, FPDFAnnot_GetSubtype(annot.get()));
    // Also do the same test for its appearance string.
    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(2u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), kBufSize));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));

    ScopedFPDFWideString text = GetFPDFWideString(kData);
    EXPECT_TRUE(FPDFAnnot_SetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                                text.get()));

    std::fill(buf.begin(), buf.end(), 'x');
    ASSERT_EQ(6u,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_ROLLOVER,
                              buf.data(), kBufSize));
    EXPECT_EQ(kData, GetPlatformWString(buf.data()));
  }

  UnloadPage(page);

  {
    // Save a copy, open the copy, and check the annotation again.
    // Note that it renders the rotation.
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    ASSERT_TRUE(OpenSavedDocument());
    FPDF_PAGE saved_page = LoadSavedPage(0);
    ASSERT_TRUE(saved_page);

    EXPECT_EQ(2, FPDFPage_GetAnnotCount(saved_page));
    {
      ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(saved_page, 0));
      ASSERT_TRUE(annot);
      EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

      std::fill(buf.begin(), buf.end(), 'x');
      ASSERT_EQ(6u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                             kBufSize));
      EXPECT_EQ(kData, GetPlatformWString(buf.data()));
    }

    {
      ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(saved_page, 0));
      ASSERT_TRUE(annot);
      // TODO(thestig): This return FPDF_ANNOT_UNKNOWN for some reason.
      // EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));

      std::fill(buf.begin(), buf.end(), 'x');
      ASSERT_EQ(6u, FPDFAnnot_GetStringValue(annot.get(), kTestKey, buf.data(),
                                             kBufSize));
      EXPECT_EQ(kData, GetPlatformWString(buf.data()));
    }

    CloseSavedPage(saved_page);
    CloseSavedDocument();
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountCombobox) {
  // Open a file with combobox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(3, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(26, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    // Check bad form handle / annot.
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(nullptr, nullptr));
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), nullptr));
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(nullptr, annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(3, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(26, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountInvalidAnnotations) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // annotations do not have "Opt" array and will return -1
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelCombobox) {
  // Open a file with combobox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    int index = 0;
    unsigned long length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(8u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(8u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                           buf.data(), length_bytes));
    EXPECT_EQ(L"Foo", GetPlatformWString(buf.data()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    index = 0;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(12u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(12u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Apple", GetPlatformWString(buf.data()));

    index = 25;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Zucchini", GetPlatformWString(buf.data()));

    // Indices out of range
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), -1,
                                           nullptr, 0));
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 26,
                                           nullptr, 0));

    // Check bad form handle / annot.
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(nullptr, nullptr, 0, nullptr, 0));
    EXPECT_EQ(0u,
              FPDFAnnot_GetOptionLabel(nullptr, annot.get(), 0, nullptr, 0));
    EXPECT_EQ(0u,
              FPDFAnnot_GetOptionLabel(form_handle(), nullptr, 0, nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    int index = 0;
    unsigned long length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(8u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(8u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                           buf.data(), length_bytes));
    EXPECT_EQ(L"Foo", GetPlatformWString(buf.data()));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    index = 0;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(12u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(12u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Apple", GetPlatformWString(buf.data()));

    index = 25;
    length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                            buf.data(), length_bytes));
    EXPECT_EQ(L"Zucchini", GetPlatformWString(buf.data()));

    // indices out of range
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), -1,
                                           nullptr, 0));
    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 26,
                                           nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelInvalidAnnotations) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // annotations do not have "Opt" array and will return 0
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 0,
                                           nullptr, 0));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 0,
                                           nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsOptionSelectedCombobox) {
  // Open a file with combobox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Checks for Combobox with no Values (/V) or Selected Indices (/I) objects.
    int count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(3, count);
    for (int i = 0; i < count; i++) {
      EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Checks for Combobox with Values (/V) object which is just a string.
    count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(26, count);
    for (int i = 0; i < count; i++) {
      EXPECT_EQ(i == 1,
                FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    // Checks for index outside bound i.e. (index >= CountOption()).
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(),
                                            /*index=*/26));
    // Checks for negetive index.
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(),
                                            /*index=*/-1));

    // Checks for bad form handle/annot.
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(nullptr, nullptr, /*index=*/0));
    EXPECT_FALSE(
        FPDFAnnot_IsOptionSelected(form_handle(), nullptr, /*index=*/0));
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(nullptr, annot.get(), /*index=*/0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsOptionSelectedListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Checks for Listbox with no Values (/V) or Selected Indices (/I) objects.
    int count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(3, count);
    for (int i = 0; i < count; i++) {
      EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Checks for Listbox with Values (/V) object which is just a string.
    count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(26, count);
    for (int i = 0; i < count; i++) {
      EXPECT_EQ(i == 1,
                FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page, 3));
    ASSERT_TRUE(annot);

    // Checks for Listbox with only Selected indices (/I) object which is an
    // array with multiple objects.
    count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(5, count);
    for (int i = 0; i < count; i++) {
      bool expected = (i == 1 || i == 3);
      EXPECT_EQ(expected,
                FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page, 4));
    ASSERT_TRUE(annot);

    // Checks for Listbox with Values (/V) object which is an array with
    // multiple objects.
    count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(5, count);
    for (int i = 0; i < count; i++) {
      bool expected = (i == 2 || i == 4);
      EXPECT_EQ(expected,
                FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page, 5));
    ASSERT_TRUE(annot);

    // Checks for Listbox with both Values (/V) and Selected Indices (/I)
    // objects conflict with different lengths.
    count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(5, count);
    for (int i = 0; i < count; i++) {
      bool expected = (i == 0 || i == 2);
      EXPECT_EQ(expected,
                FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsOptionSelectedInvalidAnnotations) {
  // Open a file with multiple form field annotations and load its first page.
  ASSERT_TRUE(OpenDocument("multiple_form_types.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Checks for link annotation.
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(),
                                            /*index=*/0));

    annot.reset(FPDFPage_GetAnnot(page, 3));
    ASSERT_TRUE(annot);

    // Checks for text field annotation.
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(),
                                            /*index=*/0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeCombobox) {
  // Open a file with combobox annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // All 3 widgets have Tf font size 12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(12.0, value);

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    float value_two;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_two));
    EXPECT_EQ(12.0, value_two);

    annot.reset(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    float value_three;
    ASSERT_TRUE(
        FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_three));
    EXPECT_EQ(12.0, value_three);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeTextField) {
  // Open a file with textfield annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // All 4 widgets have Tf font size 12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(12.0, value);

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    float value_two;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_two));
    EXPECT_EQ(12.0, value_two);

    annot.reset(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    float value_three;
    ASSERT_TRUE(
        FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_three));
    EXPECT_EQ(12.0, value_three);

    float value_four;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_four));
    EXPECT_EQ(12.0, value_four);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeInvalidAnnotationTypes) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Annotations that do not have variable text and will return -1.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));

    annot.reset(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeInvalidArguments) {
  // Open a file with combobox annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // Check bad form handle / annot.
    float value;
    ASSERT_FALSE(FPDFAnnot_GetFontSize(nullptr, annot.get(), &value));
    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), nullptr, &value));
    ASSERT_FALSE(FPDFAnnot_GetFontSize(nullptr, nullptr, &value));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeNegative) {
  // Open a file with textfield annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_negative_fontsize.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Obtain the first annotation, a text field with negative font size, -12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(-12.0, value);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedCheckbox) {
  // Open a file with checkbox and radiobuttons widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedCheckboxReadOnly) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedRadioButton) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 5));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 6));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 7));
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedRadioButtonReadOnly) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 3));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page, 4));
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedInvalidArguments) {
  // Open a file with checkbox and radiobuttons widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(nullptr, annot.get()));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), nullptr));
    ASSERT_FALSE(FPDFAnnot_IsChecked(nullptr, nullptr));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedInvalidWidgetType) {
  // Open a file with text widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldType) {
  ASSERT_TRUE(OpenDocument("multiple_form_types.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(-1, FPDFAnnot_GetFormFieldType(form_handle(), nullptr));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);
    EXPECT_EQ(-1, FPDFAnnot_GetFormFieldType(nullptr, annot.get()));
  }

  constexpr int kExpectedAnnotTypes[] = {-1,
                                         FPDF_FORMFIELD_COMBOBOX,
                                         FPDF_FORMFIELD_LISTBOX,
                                         FPDF_FORMFIELD_TEXTFIELD,
                                         FPDF_FORMFIELD_CHECKBOX,
                                         FPDF_FORMFIELD_RADIOBUTTON};

  for (size_t i = 0; i < pdfium::size(kExpectedAnnotTypes); ++i) {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, i));
    ASSERT_TRUE(annot);
    EXPECT_EQ(kExpectedAnnotTypes[i],
              FPDFAnnot_GetFormFieldType(form_handle(), annot.get()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldValueTextField) {
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldValue(form_handle(), nullptr, nullptr, 0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldValue(nullptr, annot.get(), nullptr, 0));

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(2u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(2u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));
  }
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                               buf.data(), length_bytes));
    EXPECT_EQ(L"Elephant", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldValueComboBox) {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(2u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(2u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"", GetPlatformWString(buf.data()));
  }
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(14u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(14u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                               buf.data(), length_bytes));
    EXPECT_EQ(L"Banana", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldNameTextField) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldName(form_handle(), nullptr, nullptr, 0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldName(nullptr, annot.get(), nullptr, 0));

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldName(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetFormFieldName(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"Text Box", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldNameComboBox) {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldName(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(30u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(30u, FPDFAnnot_GetFormFieldName(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"Combo_Editable", GetPlatformWString(buf.data()));
  }
  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, FocusableAnnotSubtypes) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Verify widgets are by default focusable.
  const FPDF_ANNOTATION_SUBTYPE kDefaultSubtypes[] = {FPDF_ANNOT_WIDGET};
  VerifyFocusableAnnotSubtypes(form_handle(), kDefaultSubtypes);

  // Expected annot subtypes for page 0 of annots.pdf.
  const FPDF_ANNOTATION_SUBTYPE kExpectedAnnotSubtypes[] = {
      FPDF_ANNOT_LINK,  FPDF_ANNOT_LINK,      FPDF_ANNOT_LINK,
      FPDF_ANNOT_LINK,  FPDF_ANNOT_HIGHLIGHT, FPDF_ANNOT_HIGHLIGHT,
      FPDF_ANNOT_POPUP, FPDF_ANNOT_HIGHLIGHT, FPDF_ANNOT_WIDGET,
  };

  const FPDF_ANNOTATION_SUBTYPE kExpectedDefaultFocusableSubtypes[] = {
      FPDF_ANNOT_WIDGET};
  VerifyAnnotationSubtypesAndFocusability(form_handle(), page,
                                          kExpectedAnnotSubtypes,
                                          kExpectedDefaultFocusableSubtypes);

  // Make no annotation type focusable using the preferred method.
  ASSERT_TRUE(FPDFAnnot_SetFocusableSubtypes(form_handle(), nullptr, 0));
  ASSERT_EQ(0, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  // Restore the focusable type count back to 1, then set it back to 0 using a
  // different method.
  SetAndVerifyFocusableAnnotSubtypes(form_handle(), kDefaultSubtypes);
  ASSERT_TRUE(
      FPDFAnnot_SetFocusableSubtypes(form_handle(), kDefaultSubtypes, 0));
  ASSERT_EQ(0, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));

  VerifyAnnotationSubtypesAndFocusability(form_handle(), page,
                                          kExpectedAnnotSubtypes, {});

  // Now make links focusable.
  const FPDF_ANNOTATION_SUBTYPE kLinkSubtypes[] = {FPDF_ANNOT_LINK};
  SetAndVerifyFocusableAnnotSubtypes(form_handle(), kLinkSubtypes);

  const FPDF_ANNOTATION_SUBTYPE kExpectedLinkocusableSubtypes[] = {
      FPDF_ANNOT_LINK};
  VerifyAnnotationSubtypesAndFocusability(form_handle(), page,
                                          kExpectedAnnotSubtypes,
                                          kExpectedLinkocusableSubtypes);

  // Test invalid parameters.
  EXPECT_FALSE(FPDFAnnot_SetFocusableSubtypes(nullptr, kDefaultSubtypes,
                                              pdfium::size(kDefaultSubtypes)));
  EXPECT_FALSE(FPDFAnnot_SetFocusableSubtypes(form_handle(), nullptr,
                                              pdfium::size(kDefaultSubtypes)));
  EXPECT_EQ(-1, FPDFAnnot_GetFocusableSubtypesCount(nullptr));

  std::vector<FPDF_ANNOTATION_SUBTYPE> subtypes(1);
  EXPECT_FALSE(FPDFAnnot_GetFocusableSubtypes(nullptr, subtypes.data(),
                                              subtypes.size()));
  EXPECT_FALSE(
      FPDFAnnot_GetFocusableSubtypes(form_handle(), nullptr, subtypes.size()));
  EXPECT_FALSE(
      FPDFAnnot_GetFocusableSubtypes(form_handle(), subtypes.data(), 0));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, FocusableAnnotRendering) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static const char kMd5sum[] = "4ca14c670396711194b40ecc2514969b";
#else
#if defined(OS_WIN)
    static const char kMd5sum[] = "3877bec7cb3e3144eaa6d10f38bf7a30";
#elif defined(OS_APPLE)
    static const char kMd5sum[] = "04b16db5026b5490a50fb6ff0954c867";
#else
    static const char kMd5sum[] = "40a7354d1f653127bcdac10e15f81654";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    // Check the initial rendering.
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5sum);
  }

  // Make links and highlights focusable.
  static constexpr FPDF_ANNOTATION_SUBTYPE kSubTypes[] = {FPDF_ANNOT_LINK,
                                                          FPDF_ANNOT_HIGHLIGHT};
  constexpr int kSubTypesCount = pdfium::size(kSubTypes);
  ASSERT_TRUE(
      FPDFAnnot_SetFocusableSubtypes(form_handle(), kSubTypes, kSubTypesCount));
  ASSERT_EQ(kSubTypesCount, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));
  std::vector<FPDF_ANNOTATION_SUBTYPE> subtypes(kSubTypesCount);
  ASSERT_TRUE(FPDFAnnot_GetFocusableSubtypes(form_handle(), subtypes.data(),
                                             subtypes.size()));
  ASSERT_EQ(FPDF_ANNOT_LINK, subtypes[0]);
  ASSERT_EQ(FPDF_ANNOT_HIGHLIGHT, subtypes[1]);

  {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static const char kMd5sum[] = "48cb60b3f9bc364c73582aff3418451e";
#else
#if defined(OS_WIN)
    static const char kMd5sum[] = "a30f1bd1cac022d08ceb100df4940b5f";
#elif defined(OS_APPLE)
    static const char kMd5sum[] = "3f984a164f2f6d6e3d69f27fd430e346";
#else
    static const char kMd5sum[] = "e4c4de73addabf10672c308870e8a4ee";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    // Focus the first link and check the rendering.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_LINK, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_TRUE(FORM_SetFocusedAnnot(form_handle(), annot.get()));
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5sum);
  }

  {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static const char kMd5sum[] = "d09869ff0a209daf179da2d7a58142b1";
#else
#if defined(OS_WIN)
    static const char kMd5sum[] = "467f5a4db98fcadd5121807ff4e2eb10";
#elif defined(OS_APPLE)
    static const char kMd5sum[] = "c6d6f9dc7090e8eaf3867ba714023b1e";
#else
    static const char kMd5sum[] = "65e831885e16b7ecc977cce2e4a27110";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    // Focus the first highlight and check the rendering.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 4));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_TRUE(FORM_SetFocusedAnnot(form_handle(), annot.get()));
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, kMd5sum);
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetLinkFromAnnotation) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  {
    constexpr char kExpectedResult[] =
        "https://cs.chromium.org/chromium/src/third_party/pdfium/public/"
        "fpdf_text.h";

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 3));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_LINK, FPDFAnnot_GetSubtype(annot.get()));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()),
                          kExpectedResult);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 4));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_FALSE(FPDFAnnot_GetLink(annot.get()));
  }

  EXPECT_FALSE(FPDFAnnot_GetLink(nullptr));

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlCountRadioButton) {
  // Open a file with radio button widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Checks for bad annot.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlCount(form_handle(), /*annot=*/nullptr));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 3));
    ASSERT_TRUE(annot);

    // Checks for bad form handle.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlCount(/*hHandle=*/nullptr, annot.get()));

    EXPECT_EQ(3, FPDFAnnot_GetFormControlCount(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlCountCheckBox) {
  // Open a file with checkbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetFormControlCount(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlCountInvalidAnnotation) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(-1, FPDFAnnot_GetFormControlCount(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlIndexRadioButton) {
  // Open a file with radio button widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Checks for bad annot.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlIndex(form_handle(), /*annot=*/nullptr));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 3));
    ASSERT_TRUE(annot);

    // Checks for bad form handle.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlIndex(/*hHandle=*/nullptr, annot.get()));

    EXPECT_EQ(1, FPDFAnnot_GetFormControlIndex(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlIndexCheckBox) {
  // Open a file with checkbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(0, FPDFAnnot_GetFormControlIndex(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlIndexInvalidAnnotation) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(-1, FPDFAnnot_GetFormControlIndex(form_handle(), annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldExportValueRadioButton) {
  // Open a file with radio button widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Checks for bad annot.
    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldExportValue(
                      form_handle(), /*annot=*/nullptr,
                      /*buffer=*/nullptr, /*buflen=*/0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 6));
    ASSERT_TRUE(annot);

    // Checks for bad form handle.
    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldExportValue(
                      /*hHandle=*/nullptr, annot.get(),
                      /*buffer=*/nullptr, /*buflen=*/0));

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldExportValue(form_handle(), annot.get(),
                                          /*buffer=*/nullptr, /*buflen=*/0);
    ASSERT_EQ(14u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(14u, FPDFAnnot_GetFormFieldExportValue(form_handle(), annot.get(),
                                                     buf.data(), length_bytes));
    EXPECT_EQ(L"value2", GetPlatformWString(buf.data()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldExportValueCheckBox) {
  // Open a file with checkbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldExportValue(form_handle(), annot.get(),
                                          /*buffer=*/nullptr, /*buflen=*/0);
    ASSERT_EQ(8u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(8u, FPDFAnnot_GetFormFieldExportValue(form_handle(), annot.get(),
                                                    buf.data(), length_bytes));
    EXPECT_EQ(L"Yes", GetPlatformWString(buf.data()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldExportValueInvalidAnnotation) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldExportValue(form_handle(), annot.get(),
                                                    /*buffer=*/nullptr,
                                                    /*buflen=*/0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, Redactannotation) {
  ASSERT_TRUE(OpenDocument("redact_annot.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_REDACT, FPDFAnnot_GetSubtype(annot.get()));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, PolygonAnnotation) {
  ASSERT_TRUE(OpenDocument("polygon_annot.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // FPDFAnnot_GetVertices() positive testing.
    unsigned long size = FPDFAnnot_GetVertices(annot.get(), nullptr, 0);
    const size_t kExpectedSize = 3;
    ASSERT_EQ(kExpectedSize, size);
    std::vector<FS_POINTF> vertices_buffer(size);
    EXPECT_EQ(size,
              FPDFAnnot_GetVertices(annot.get(), vertices_buffer.data(), size));
    EXPECT_FLOAT_EQ(159.0f, vertices_buffer[0].x);
    EXPECT_FLOAT_EQ(296.0f, vertices_buffer[0].y);
    EXPECT_FLOAT_EQ(350.0f, vertices_buffer[1].x);
    EXPECT_FLOAT_EQ(411.0f, vertices_buffer[1].y);
    EXPECT_FLOAT_EQ(472.0f, vertices_buffer[2].x);
    EXPECT_FLOAT_EQ(243.42f, vertices_buffer[2].y);

    // FPDFAnnot_GetVertices() negative testing.
    EXPECT_EQ(0U, FPDFAnnot_GetVertices(nullptr, nullptr, 0));

    // vertices_buffer is not overwritten if it is too small.
    vertices_buffer.resize(1);
    vertices_buffer[0].x = 42;
    vertices_buffer[0].y = 43;
    size = FPDFAnnot_GetVertices(annot.get(), vertices_buffer.data(),
                                 vertices_buffer.size());
    EXPECT_EQ(kExpectedSize, size);
    EXPECT_FLOAT_EQ(42, vertices_buffer[0].x);
    EXPECT_FLOAT_EQ(43, vertices_buffer[0].y);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // This has an odd number of elements in the vertices array, ignore the last
    // element.
    unsigned long size = FPDFAnnot_GetVertices(annot.get(), nullptr, 0);
    const size_t kExpectedSize = 3;
    ASSERT_EQ(kExpectedSize, size);
    std::vector<FS_POINTF> vertices_buffer(size);
    EXPECT_EQ(size,
              FPDFAnnot_GetVertices(annot.get(), vertices_buffer.data(), size));
    EXPECT_FLOAT_EQ(259.0f, vertices_buffer[0].x);
    EXPECT_FLOAT_EQ(396.0f, vertices_buffer[0].y);
    EXPECT_FLOAT_EQ(450.0f, vertices_buffer[1].x);
    EXPECT_FLOAT_EQ(511.0f, vertices_buffer[1].y);
    EXPECT_FLOAT_EQ(572.0f, vertices_buffer[2].x);
    EXPECT_FLOAT_EQ(343.0f, vertices_buffer[2].y);
  }

  {
    // Wrong annotation type.
    ScopedFPDFAnnotation ink_annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_INK));
    EXPECT_EQ(0U, FPDFAnnot_GetVertices(ink_annot.get(), nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, InkAnnotation) {
  ASSERT_TRUE(OpenDocument("ink_annot.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // FPDFAnnot_GetInkListCount() and FPDFAnnot_GetInkListPath() positive
    // testing.
    unsigned long size = FPDFAnnot_GetInkListCount(annot.get());
    const size_t kExpectedSize = 1;
    ASSERT_EQ(kExpectedSize, size);
    const unsigned long kPathIndex = 0;
    unsigned long path_size =
        FPDFAnnot_GetInkListPath(annot.get(), kPathIndex, nullptr, 0);
    const size_t kExpectedPathSize = 3;
    ASSERT_EQ(kExpectedPathSize, path_size);
    std::vector<FS_POINTF> path_buffer(path_size);
    EXPECT_EQ(path_size,
              FPDFAnnot_GetInkListPath(annot.get(), kPathIndex,
                                       path_buffer.data(), path_size));
    EXPECT_FLOAT_EQ(159.0f, path_buffer[0].x);
    EXPECT_FLOAT_EQ(296.0f, path_buffer[0].y);
    EXPECT_FLOAT_EQ(350.0f, path_buffer[1].x);
    EXPECT_FLOAT_EQ(411.0f, path_buffer[1].y);
    EXPECT_FLOAT_EQ(472.0f, path_buffer[2].x);
    EXPECT_FLOAT_EQ(243.42f, path_buffer[2].y);

    // FPDFAnnot_GetInkListCount() and FPDFAnnot_GetInkListPath() negative
    // testing.
    EXPECT_EQ(0U, FPDFAnnot_GetInkListCount(nullptr));
    EXPECT_EQ(0U, FPDFAnnot_GetInkListPath(nullptr, 0, nullptr, 0));

    // out of bounds path_index.
    EXPECT_EQ(0U, FPDFAnnot_GetInkListPath(nullptr, 42, nullptr, 0));

    // path_buffer is not overwritten if it is too small.
    path_buffer.resize(1);
    path_buffer[0].x = 42;
    path_buffer[0].y = 43;
    path_size = FPDFAnnot_GetInkListPath(
        annot.get(), kPathIndex, path_buffer.data(), path_buffer.size());
    EXPECT_EQ(kExpectedSize, size);
    EXPECT_FLOAT_EQ(42, path_buffer[0].x);
    EXPECT_FLOAT_EQ(43, path_buffer[0].y);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // This has an odd number of elements in the path array, ignore the last
    // element.
    unsigned long size = FPDFAnnot_GetInkListCount(annot.get());
    const size_t kExpectedSize = 1;
    ASSERT_EQ(kExpectedSize, size);
    const unsigned long kPathIndex = 0;
    unsigned long path_size =
        FPDFAnnot_GetInkListPath(annot.get(), kPathIndex, nullptr, 0);
    const size_t kExpectedPathSize = 3;
    ASSERT_EQ(kExpectedPathSize, path_size);
    std::vector<FS_POINTF> path_buffer(path_size);
    EXPECT_EQ(path_size,
              FPDFAnnot_GetInkListPath(annot.get(), kPathIndex,
                                       path_buffer.data(), path_size));
    EXPECT_FLOAT_EQ(259.0f, path_buffer[0].x);
    EXPECT_FLOAT_EQ(396.0f, path_buffer[0].y);
    EXPECT_FLOAT_EQ(450.0f, path_buffer[1].x);
    EXPECT_FLOAT_EQ(511.0f, path_buffer[1].y);
    EXPECT_FLOAT_EQ(572.0f, path_buffer[2].x);
    EXPECT_FLOAT_EQ(343.0f, path_buffer[2].y);
  }

  {
    // Wrong annotation type.
    ScopedFPDFAnnotation polygon_annot(
        FPDFPage_CreateAnnot(page, FPDF_ANNOT_POLYGON));
    EXPECT_EQ(0U, FPDFAnnot_GetInkListCount(polygon_annot.get()));
    const unsigned long kPathIndex = 0;
    EXPECT_EQ(0U, FPDFAnnot_GetInkListPath(polygon_annot.get(), kPathIndex,
                                           nullptr, 0));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, LineAnnotation) {
  ASSERT_TRUE(OpenDocument("line_annot.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // FPDFAnnot_GetVertices() positive testing.
    FS_POINTF start;
    FS_POINTF end;
    ASSERT_TRUE(FPDFAnnot_GetLine(annot.get(), &start, &end));
    EXPECT_FLOAT_EQ(159.0f, start.x);
    EXPECT_FLOAT_EQ(296.0f, start.y);
    EXPECT_FLOAT_EQ(472.0f, end.x);
    EXPECT_FLOAT_EQ(243.42f, end.y);

    // FPDFAnnot_GetVertices() negative testing.
    EXPECT_FALSE(FPDFAnnot_GetLine(nullptr, nullptr, nullptr));
    EXPECT_FALSE(FPDFAnnot_GetLine(annot.get(), nullptr, nullptr));
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Too few elements in the line array.
    FS_POINTF start;
    FS_POINTF end;
    EXPECT_FALSE(FPDFAnnot_GetLine(annot.get(), &start, &end));
  }

  {
    // Wrong annotation type.
    ScopedFPDFAnnotation ink_annot(FPDFPage_CreateAnnot(page, FPDF_ANNOT_INK));
    FS_POINTF start;
    FS_POINTF end;
    EXPECT_FALSE(FPDFAnnot_GetLine(ink_annot.get(), &start, &end));
  }

  UnloadPage(page);
}

TEST_F(FPDFAnnotEmbedderTest, AnnotationBorder) {
  ASSERT_TRUE(OpenDocument("line_annot.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 0));
    ASSERT_TRUE(annot);

    // FPDFAnnot_GetBorder() positive testing.
    float horizontal_radius;
    float vertical_radius;
    float border_width;
    ASSERT_TRUE(FPDFAnnot_GetBorder(annot.get(), &horizontal_radius,
                                    &vertical_radius, &border_width));
    EXPECT_FLOAT_EQ(0.25f, horizontal_radius);
    EXPECT_FLOAT_EQ(0.5f, vertical_radius);
    EXPECT_FLOAT_EQ(2.0f, border_width);

    // FPDFAnnot_GetBorder() negative testing.
    EXPECT_FALSE(FPDFAnnot_GetBorder(nullptr, nullptr, nullptr, nullptr));
    EXPECT_FALSE(FPDFAnnot_GetBorder(annot.get(), nullptr, nullptr, nullptr));
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 1));
    ASSERT_TRUE(annot);

    // Too few elements in the border array.
    float horizontal_radius;
    float vertical_radius;
    float border_width;
    EXPECT_FALSE(FPDFAnnot_GetBorder(annot.get(), &horizontal_radius,
                                     &vertical_radius, &border_width));

    // FPDFAnnot_SetBorder() positive testing.
    EXPECT_TRUE(FPDFAnnot_SetBorder(annot.get(), /*horizontal_radius=*/2.0f,
                                    /*vertical_radius=*/3.5f,
                                    /*border_width=*/4.0f));

    EXPECT_TRUE(FPDFAnnot_GetBorder(annot.get(), &horizontal_radius,
                                    &vertical_radius, &border_width));
    EXPECT_FLOAT_EQ(2.0f, horizontal_radius);
    EXPECT_FLOAT_EQ(3.5f, vertical_radius);
    EXPECT_FLOAT_EQ(4.0f, border_width);

    // FPDFAnnot_SetBorder() negative testing.
    EXPECT_FALSE(FPDFAnnot_SetBorder(nullptr, /*horizontal_radius=*/1.0f,
                                     /*vertical_radius=*/2.5f,
                                     /*border_width=*/3.0f));
  }

  UnloadPage(page);
}

// Due to https://crbug.com/pdfium/570, the AnnotationBorder test above cannot
// actually render the line annotations inside line_annot.pdf. For now, use a
// square annotation in annots.pdf for testing.
TEST_F(FPDFAnnotEmbedderTest, AnnotationBorderRendering) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  FPDF_PAGE page = LoadPage(1);
  ASSERT_TRUE(page);
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page));

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  constexpr char kOriginalChecksum[] = "bc9ba381d6d46ff93ed0b5288b763b60";
  constexpr char kModifiedChecksum[] = "0f3e10336f67cb6a8731a06d26d01e50";
#elif defined(OS_WIN)
  constexpr char kOriginalChecksum[] = "f9fcab6ac610ee2347c4eba6be86a90c";
  constexpr char kModifiedChecksum[] = "e0e9ad5b67ef84288d446861265898b3";
#elif defined(OS_APPLE)
  constexpr char kOriginalChecksum[] = "1839f5df5fb4fae10cf3793568e73ede";
  constexpr char kModifiedChecksum[] = "abd4f5d1c3b8d8cfc572b389e589da5a";
#else
  constexpr char kOriginalChecksum[] = "ccf6667b34ec2452bea0b5f1a0194191";
  constexpr char kModifiedChecksum[] = "1bbdb473d0757843e82053b0bd3298bc";
#endif

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page, 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(annot.get()));

    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, kOriginalChecksum);
    }

    EXPECT_TRUE(FPDFAnnot_SetBorder(annot.get(), /*horizontal_radius=*/2.0f,
                                    /*vertical_radius=*/3.5f,
                                    /*border_width=*/4.0f));

    {
      ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, kModifiedChecksum);
    }
  }

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  ASSERT_TRUE(OpenSavedDocument());
  page = LoadSavedPage(1);
  ASSERT_TRUE(page);
  VerifySavedRendering(page, 612, 792, kModifiedChecksum);

  CloseSavedPage(page);
  CloseSavedDocument();
}
