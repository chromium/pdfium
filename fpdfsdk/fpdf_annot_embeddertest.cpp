// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_annot.h"

#include <limits.h>

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include "build/build_config.h"
#include "constants/annotation_common.h"
#include "core/fpdfapi/page/cpdf_annotcontext.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/span.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_attachment.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_formfill.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/hash.h"

using pdfium::AnnotationStampWithApChecksum;

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
  EXPECT_EQ(expected_uri, buffer.data());
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
  const CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);
  RetainPtr<const CPDF_Dictionary> ap_dict =
      annot_dict->GetDictFor(pdfium::annotation::kAP);
  ASSERT_TRUE(ap_dict);
  RetainPtr<const CPDF_Dictionary> stream_dict = ap_dict->GetDictFor("N");
  ASSERT_TRUE(stream_dict);
  // Check for non-existence of resources dictionary in case of opaque color
  RetainPtr<const CPDF_Dictionary> resources_dict =
      stream_dict->GetDictFor("Resources");
  ASSERT_FALSE(resources_dict);
  ByteString type = stream_dict->GetByteStringFor(pdfium::annotation::kType);
  EXPECT_EQ("XObject", type);
  ByteString sub_type =
      stream_dict->GetByteStringFor(pdfium::annotation::kSubtype);
  EXPECT_EQ("Form", sub_type);

  // Check that the appearance stream is same as we just set.
  const uint32_t kStreamDataSize = std::size(kStreamData) * sizeof(FPDF_WCHAR);
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
  const CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);
  RetainPtr<const CPDF_Dictionary> ap_dict =
      annot_dict->GetDictFor(pdfium::annotation::kAP);
  ASSERT_TRUE(ap_dict);
  RetainPtr<const CPDF_Dictionary> stream_dict = ap_dict->GetDictFor("N");
  ASSERT_TRUE(stream_dict);
  RetainPtr<const CPDF_Dictionary> resources_dict =
      stream_dict->GetDictFor("Resources");
  ASSERT_TRUE(stream_dict);
  RetainPtr<const CPDF_Dictionary> extGState_dict =
      resources_dict->GetDictFor("ExtGState");
  ASSERT_TRUE(extGState_dict);
  RetainPtr<const CPDF_Dictionary> gs_dict = extGState_dict->GetDictFor("GS");
  ASSERT_TRUE(gs_dict);
  ByteString type = gs_dict->GetByteStringFor(pdfium::annotation::kType);
  EXPECT_EQ("ExtGState", type);
  float opacity = gs_dict->GetFloatFor("CA");
  // Opacity value of 102 is represented as 0.4f (=104/255) in /CA entry.
  EXPECT_FLOAT_EQ(0.4f, opacity);
  ByteString blend_mode = gs_dict->GetByteStringFor("BM");
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
  const CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);

  static constexpr FS_POINTF kFirstInkStroke[] = {
      {80.0f, 90.0f}, {81.0f, 91.0f}, {82.0f, 92.0f},
      {83.0f, 93.0f}, {84.0f, 94.0f}, {85.0f, 95.0f}};
  static constexpr size_t kFirstStrokePointCount = std::size(kFirstInkStroke);

  static constexpr FS_POINTF kSecondInkStroke[] = {
      {70.0f, 90.0f}, {71.0f, 91.0f}, {72.0f, 92.0f}};
  static constexpr size_t kSecondStrokePointCount = std::size(kSecondInkStroke);

  static constexpr FS_POINTF kThirdInkStroke[] = {{60.0f, 90.0f},
                                                  {61.0f, 91.0f},
                                                  {62.0f, 92.0f},
                                                  {63.0f, 93.0f},
                                                  {64.0f, 94.0f}};
  static constexpr size_t kThirdStrokePointCount = std::size(kThirdInkStroke);

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

  RetainPtr<const CPDF_Array> inklist = annot_dict->GetArrayFor("InkList");
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
  const CPDF_Dictionary* annot_dict = context->GetAnnotDict();
  ASSERT_TRUE(annot_dict);

  static constexpr FS_POINTF kInkStroke[] = {{80.0f, 90.0f}, {81.0f, 91.0f},
                                             {82.0f, 92.0f}, {83.0f, 93.0f},
                                             {84.0f, 94.0f}, {85.0f, 95.0f}};
  static constexpr size_t kPointCount = std::size(kInkStroke);

  // InkStroke should get added to ink annotation. Also inklist should get
  // created.
  EXPECT_EQ(0,
            FPDFAnnot_AddInkStroke(ink_annot.get(), kInkStroke, kPointCount));

  RetainPtr<const CPDF_Array> inklist = annot_dict->GetArrayFor("InkList");
  ASSERT_TRUE(inklist);
  ASSERT_EQ(1u, inklist->size());
  EXPECT_EQ(kPointCount * 2, inklist->GetArrayAt(0)->size());

  // Remove inklist.
  EXPECT_TRUE(FPDFAnnot_RemoveInkList(ink_annot.get()));
  EXPECT_FALSE(annot_dict->KeyExist("InkList"));
}

TEST_F(FPDFAnnotEmbedderTest, BadParams) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(0, FPDFPage_GetAnnotCount(nullptr));

  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, 0));
  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, -1));
  EXPECT_FALSE(FPDFPage_GetAnnot(nullptr, 1));
  EXPECT_FALSE(FPDFPage_GetAnnot(page.get(), -1));
  EXPECT_FALSE(FPDFPage_GetAnnot(page.get(), 1));

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
}

TEST_F(FPDFAnnotEmbedderTest, BadAnnotsEntry) {
  ASSERT_TRUE(OpenDocument("bad_annots_entry.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));
  EXPECT_FALSE(FPDFPage_GetAnnot(page.get(), 0));
}

TEST_F(FPDFAnnotEmbedderTest, RenderAnnotWithOnlyRolloverAP) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // This annotation has a malformed appearance stream, which does not have its
  // normal appearance defined, only its rollover appearance. In this case, its
  // normal appearance should be generated, allowing the highlight annotation to
  // still display.
  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
}

TEST_F(FPDFAnnotEmbedderTest, RenderMultilineMarkupAnnotWithoutAP) {
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "ec1f4ccbd0aecfdea6d53893387a0101";
    }
    return "76512832d88017668d9acc7aacd13dae";
  }();

  // Open a file with multiline markup annotations.
  ASSERT_TRUE(OpenDocument("annotation_markup_multiline_no_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
  CompareBitmap(bitmap.get(), 595, 842, checksum);
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
    EXPECT_EQ(157.211166f, quadpoints.x4);
    EXPECT_EQ(706.264404f, quadpoints.y4);
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
    EXPECT_EQ(351.820435f, rect.left);
    EXPECT_EQ(583.830750f, rect.bottom);
    EXPECT_EQ(475.336121f, rect.right);
    EXPECT_EQ(681.535034f, rect.top);
  }
  {
    const char* expected_hash = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "42e03089ef2392579d73bf9065896488";
#elif BUILDFLAG(IS_APPLE)
        return "5891aa346046c3f5b8c3cd8607f0d256";
#else
        return "d02d468d9022ddf35014daa50191a895";
#endif
      }
      return "738a3881cb2cfb3e69e2a3adbc7c1d5b";
    }();
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, expected_hash);
  }
  UnloadPageNoEvents(page);
}

TEST_F(FPDFAnnotEmbedderTest, AddIllegalSubtypeAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Add an annotation with an illegal subtype.
  ASSERT_FALSE(FPDFPage_CreateAnnot(page.get(), -1));
}

TEST_F(FPDFAnnotEmbedderTest, AddFirstTextAnnotation) {
  // Open a file with no annotation and load its first page.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page.get()));

  {
    // Add a text annotation to the page.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_TEXT));
    ASSERT_TRUE(annot);

    // Check that there is now 1 annotations on this page.
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));

    // Check that the subtype of the annotation is correct.
    EXPECT_EQ(FPDF_ANNOT_TEXT, FPDFAnnot_GetSubtype(annot.get()));
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
}

TEST_F(FPDFAnnotEmbedderTest, AddAndSaveLinkAnnotation) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 200, 200, pdfium::HelloWorldChecksum());
  }
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page.get()));

  constexpr char kUri[] = "https://pdfium.org/";

  {
    // Add a link annotation to the page and set its URI.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_LINK));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));
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

    VerifyUriActionInLink(
        document(), FPDFLink_GetLinkAtPoint(page.get(), 40.0, 50.0), kUri);
  }

  {
    // Add an ink annotation to the page. Trying to add a link to it fails.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
    ASSERT_TRUE(annot);
    EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));
    EXPECT_EQ(FPDF_ANNOT_INK, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_FALSE(FPDFAnnot_SetURI(annot.get(), kUri));
  }

  // Remove the ink annotation added above for negative testing.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page.get(), 1));
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Reopen the document and make sure it still renders the same. Since the link
  // does not have a border, it does not affect the rendering.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  VerifySavedRendering(saved_page, 200, 200, pdfium::HelloWorldChecksum());
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(saved_page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(saved_page, 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_LINK, FPDFAnnot_GetSubtype(annot.get()));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()), kUri);
    VerifyUriActionInLink(
        document(), FPDFLink_GetLinkAtPoint(saved_page, 40.0, 50.0), kUri);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, AddAndSaveUnderlineAnnotation) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Check that there is a total of one annotation on its first page, and verify
  // its quadpoints.
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));
  FS_QUADPOINTSF quadpoints;
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    ASSERT_TRUE(FPDFAnnot_GetAttachmentPoints(annot.get(), 0, &quadpoints));
    EXPECT_EQ(115.802643f, quadpoints.x1);
    EXPECT_EQ(718.913940f, quadpoints.y1);
    EXPECT_EQ(157.211166f, quadpoints.x4);
    EXPECT_EQ(706.264404f, quadpoints.y4);
  }

  // Add an underline annotation to the page and set its quadpoints.
  {
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_UNDERLINE));
    ASSERT_TRUE(annot);
    quadpoints.x1 = 140.802643f;
    quadpoints.x3 = 140.802643f;
    ASSERT_TRUE(FPDFAnnot_AppendAttachmentPoints(annot.get(), &quadpoints));
  }

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Open the saved document.
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "c50012ab122cd3706d39f371ca7462ee";
#elif BUILDFLAG(IS_APPLE)
      return "24994ad69aa612a66d183eaf9a92aa06";
#else
      return "798fa41303381c9ba6d99092f5cd4d2b";
#endif
    }
    return "dba153419f67b7c0c0e3d22d3e8910d5";
  }();

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  VerifySavedRendering(saved_page, 612, 792, checksum);

  // Check that the saved document has 2 annotations on the first page
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(saved_page));

  {
    // Check that the second annotation is an underline annotation and verify
    // its quadpoints.
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(saved_page, 1));
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

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, GetAndSetQuadPoints) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page.get()));

  // Retrieve the highlight annotation.
  FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page.get(), 0);
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
  FPDF_ANNOTATION squareAnnot = FPDFPage_GetAnnot(page.get(), 2);

  {
    // Check that attempting to set its quadpoints would fail
    ASSERT_TRUE(squareAnnot);
    EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(squareAnnot));
    EXPECT_EQ(0u, FPDFAnnot_CountAttachmentPoints(squareAnnot));
    EXPECT_FALSE(FPDFAnnot_SetAttachmentPoints(squareAnnot, 0, &quadpoints));
  }

  FPDFPage_CloseAnnot(squareAnnot);
}

TEST_F(FPDFAnnotEmbedderTest, ModifyRectQuadpointsWithAP) {
  const char* md5_original = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "3867f6e34e801abad4e98811f6d7b887";
#elif BUILDFLAG(IS_APPLE)
      return "32cd26430a31752e612475bf881cc597";
#else
      return "2a9d1df839d5ec81a49f982347d9656c";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "fc59468d154f397fd298c69f47ef565a";
#else
    return "0e27376094f11490f74c65f3dc3a42c5";
#endif
  }();
  const char* md5_modified_highlight = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "a6f6df562dcf96b3670d40fa2999a582";
#elif BUILDFLAG(IS_APPLE)
      return "9a969b7089f49c029b10cf8c208b40dd";
#else
      return "0fb1653db0e8e8f7ce5d726bb0074bb5";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "e64bf648f6e9354d1f3eedb47a2c9498";
#else
    return "66f3caef3a7d488a4fa1ad37fc06310e";
#endif
  }();
  const char* md5_modified_square = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "25bb74224a3519a397a8b987069085a5";
#elif BUILDFLAG(IS_APPLE)
      return "8b9df3824179d3757932bda23b95c8ce";
#else
      return "4f66924b2d246f9e8a1b926e956b615b";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "a66591662c8e7ad3c6059952e234bebf";
#else
    return "a456dad0bc6801ee2d6408a4394af563";
#endif
  }();

  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(4, FPDFPage_GetAnnotCount(page.get()));

  // Check that the original file renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, md5_original);
  }

  FS_RECTF rect;
  FS_RECTF new_rect;

  // Retrieve the highlight annotation which has its AP stream already defined.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
      ScopedFPDFBitmap bitmap =
          RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, md5_original);
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
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, md5_modified_highlight);
  }

  {
    // Retrieve the square annotation which has its AP stream already defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
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
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, md5_modified_square);
  }
}

TEST_F(FPDFAnnotEmbedderTest, CountAttachmentPoints) {
  // Open a file with multiline markup annotations.
  ASSERT_TRUE(OpenDocument("annotation_markup_multiline_no_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // This is a three line annotation.
    EXPECT_EQ(3u, FPDFAnnot_CountAttachmentPoints(annot.get()));
  }

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
  FPDF_FILEACCESS file_access = {};  // Aggregate initialization
  static_assert(std::is_aggregate_v<decltype(file_access)>);
  file_access.m_FileLen = new_file.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &new_file;
  ScopedFPDFDocument new_doc(FPDF_LoadCustomDocument(&file_access, nullptr));
  ASSERT_TRUE(new_doc);
  ScopedFPDFPage new_page(FPDF_LoadPage(new_doc.get(), 0));
  ASSERT_TRUE(new_page);

  // Check that the saved document has 2 annotations on the first page.
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(new_page.get()));

  // Check that the remaining 2 annotations are the original 1st and 3rd ones
  // by verifying their rectangle coordinates.
  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(new_page.get(), 0));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(86.1971f, rect.left, 0.001f);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(new_page.get(), 1));
    ASSERT_TRUE(FPDFAnnot_GetRect(annot.get(), &rect));
    EXPECT_NEAR(351.8204f, rect.left, 0.001f);
  }
}

TEST_F(FPDFAnnotEmbedderTest, AddAndModifyPath) {
  const char* md5_modified_path = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "02c0628d5e1da4d0f2d481272c7e5a9b";
#elif BUILDFLAG(IS_APPLE)
      return "c81ee833f81ae0fa674e0c16872156d2";
#else
      return "4a3a53f2b1b0cb40e9a32305eef9197a";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "ac6a1526c71bb0cebe3c3c0b6bf59aab";
#else
    return "fb9e6f707986e5daf2f7bb75b4ed28ec";
#endif
  }();
  const char* md5_two_paths = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "cf7119f478a5daf7fa7c707514f39e13";
#elif BUILDFLAG(IS_APPLE)
      return "c3ce00fda0bcaad52536b49991a24aba";
#else
      return "db4c609adaa0dcfba213450bbd62264e";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "48a03185099a335f60b158ade04ff22c";
#else
    return "1abd24eb0ec89cd429ea7080d9762713";
#endif
  }();
  const char* md5_new_annot = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "df816a3ea0b92e597fa04e8a0a80e5a7";
#elif BUILDFLAG(IS_APPLE)
      return "75f8d0866cda9bc7545a9c1162841aad";
#else
      return "d4d1b116d5f23dfec894e25db4c5ebdb";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "eae62b0038343c77141eedea842f01eb";
#else
    return "897415260f33a8b553c85466ffcf4a56";
#endif
  }();

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, AnnotationStampWithApChecksum());
  }

  {
    // Retrieve the stamp annotation which has its AP stream already defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // Check that this annotation has one path object and retrieve it.
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    ASSERT_EQ(32, FPDFPage_CountObjects(page.get()));
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
      ScopedFPDFBitmap bitmap =
          RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 595, 842, md5_modified_path);
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
    ASSERT_EQ(32, FPDFPage_CountObjects(page.get()));

    // Check that the page with an annotation with two paths renders correctly.
    {
      ScopedFPDFBitmap bitmap =
          RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 595, 842, md5_two_paths);
    }

    // Delete the newly added path object.
    EXPECT_TRUE(FPDFAnnot_RemoveObject(annot.get(), 1));
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    ASSERT_EQ(32, FPDFPage_CountObjects(page.get()));
  }

  // Check that the page renders the same as before.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, md5_modified_path);
  }

  FS_RECTF rect;

  {
    // Create another stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_STAMP));
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

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Open the saved document.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  VerifySavedRendering(saved_page, 595, 842, md5_new_annot);

  // Check that the document has a correct count of annotations and objects.
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(saved_page));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(saved_page, 2));
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

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, ModifyAnnotationFlags) {
  // Open a file with an annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_rollover_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
  }

  {
    // Retrieve the annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
      ScopedFPDFBitmap bitmap =
          RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
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
      ScopedFPDFBitmap bitmap =
          RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, "dc98f06da047bd8aabfa99562d2cbd1e");
    }
  }
}

TEST_F(FPDFAnnotEmbedderTest, AddAndModifyImage) {
  const char* md5_new_image = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "795cf0ce02f6f2d3a23ed06a148af82f";
#elif BUILDFLAG(IS_APPLE)
      return "617f9f6f00167f83e9ffb31751624a9e";
#else
      return "5526f05092a0253d56a0e79e0e104f71";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "4c63afd81a7835579485f056e2d5f951";
#else
    return "727380057983d1ce10c7f51cbcd0917f";
#endif
  }();
  const char* md5_modified_image = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "7c33121197cf13a83b96e913d70fe12d";
#elif BUILDFLAG(IS_APPLE)
      return "0bd1b3bbefa3bae4199ce9c8b5bf840b";
#else
      return "e3f115b37181ce0ffd02c7b5288ea99a";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "5d242652769254acd9a1f91311ff6b65";
#else
    return "f0d4f77fee46e77f79edae2d93d574a3";
#endif
  }();

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, AnnotationStampWithApChecksum());
  }

  constexpr int kBitmapSize = 200;
  FPDF_BITMAP image_bitmap;

  {
    // Create a stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_STAMP));
    FPDF_PAGE page0 = page.get();
    ASSERT_TRUE(annot);
    FS_RECTF rect;
    rect.left = 200.f;
    rect.bottom = 600.f;
    rect.right = 400.f;
    rect.top = 800.f;
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &rect));

    // Add a solid-color translucent image object to the new annotation.
    image_bitmap = FPDFBitmap_Create(kBitmapSize, kBitmapSize, 1);
    ASSERT_TRUE(FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize,
                                    kBitmapSize, 0xeeeecccc));
    EXPECT_EQ(kBitmapSize, FPDFBitmap_GetWidth(image_bitmap));
    EXPECT_EQ(kBitmapSize, FPDFBitmap_GetHeight(image_bitmap));
    FPDF_PAGEOBJECT image_object = FPDFPageObj_NewImageObj(document());
    ASSERT_TRUE(FPDFImageObj_SetBitmap(&page0, 0, image_object, image_bitmap));
    static constexpr FS_MATRIX kBitmapScaleMatrix{kBitmapSize, 0, 0,
                                                  kBitmapSize, 0, 0};
    ASSERT_TRUE(FPDFPageObj_SetMatrix(image_object, &kBitmapScaleMatrix));
    FPDFPageObj_Transform(image_object, 1, 0, 0, 1, 200, 600);
    EXPECT_TRUE(FPDFAnnot_AppendObject(annot.get(), image_object));
  }

  // Check that the page renders correctly with the new image object.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, md5_new_image);
  }

  {
    // Retrieve the newly added stamp annotation and its image object.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetObjectCount(annot.get()));
    FPDF_PAGEOBJECT image_object = FPDFAnnot_GetObject(annot.get(), 0);
    EXPECT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image_object));

    // Modify the image in the new annotation.
    ASSERT_TRUE(FPDFBitmap_FillRect(image_bitmap, 0, 0, kBitmapSize,
                                    kBitmapSize, 0xff000000));
    FPDF_PAGE page_ptr = page.get();
    ASSERT_TRUE(
        FPDFImageObj_SetBitmap(&page_ptr, 0, image_object, image_bitmap));
    EXPECT_TRUE(FPDFAnnot_UpdateObject(annot.get(), image_object));
  }

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDFBitmap_Destroy(image_bitmap);

  // Test that the saved document renders the modified image object correctly.
  VerifySavedDocument(595, 842, md5_modified_image);
}

TEST_F(FPDFAnnotEmbedderTest, AddAndModifyText) {
  const char* md5_new_text = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "d9b4f92933bbde80a123048ae2ef7296";
#elif BUILDFLAG(IS_APPLE)
      return "f63c9d35c6c19f88ffb92471e97b7e22";
#else
      return "6b1cfec7e9c89da93dc14172afffc5db";
#endif
    }
#if BUILDFLAG(IS_APPLE) && defined(ARCH_CPU_ARM64)
    return "8eabf79dcdcfc6474c593bc60d996def";
#elif BUILDFLAG(IS_APPLE) && !defined(ARCH_CPU_ARM64)
    return "8cfa6f61f5a03b3f2306d0924ef6c000";
#else
    return "c9b2be9f9bcb8998ee181f149a979cb8";
#endif
  }();
  const char* md5_modified_text = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "a393718c0bc2144131e3c60b51cd985c";
#elif BUILDFLAG(IS_APPLE)
      return "76b7f0cb6f0e52aa25a2c1e11c9abd62";
#else
      return "007960dd422d0999f71a9de0faa29b07";
#endif
    }
#if BUILDFLAG(IS_APPLE) && defined(ARCH_CPU_ARM64)
    return "704f3eb56f82377753a816a43de250ea";
#elif BUILDFLAG(IS_APPLE) && !defined(ARCH_CPU_ARM64)
    return "c39124df1815dd6fce3b2f113169c8c9";
#else
    return "c08913614721f4337ff893c3cf53026d";
#endif
  }();

  // Open a file with two annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

  // Check that the page renders correctly.
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, AnnotationStampWithApChecksum());
  }

  {
    // Create a stamp annotation and set its annotation rectangle.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_STAMP));
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
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, md5_new_text);
  }

  {
    // Retrieve the newly added stamp annotation and its text object.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
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
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, md5_modified_text);
  }

  // Remove the new annotation, and check that the page renders as before.
  EXPECT_TRUE(FPDFPage_RemoveAnnot(page.get(), 2));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 595, 842, AnnotationStampWithApChecksum());
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetSetStringValue) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static const wchar_t kNewDate[] = L"D:201706282359Z00'00'";

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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

  const char* md5 = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "d0dfc003b3e08160e698355b599f7eb8";
#elif BUILDFLAG(IS_APPLE)
      return "8774ac1779ea66056860290ae7df8f44";
#else
      return "8b8618de537ec6aee1f3fc53fedfbcfc";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "587311ad93447614cbe5887df14caa78";
#else
    return "2908fd6166f795dfd73c607ec12c5356";
#endif
  }();

  // Open the saved annotation.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  VerifySavedRendering(saved_page, 595, 842, md5);
  {
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(saved_page, 0));

    // Check that the string value of the modified date is the newly-set
    // value.
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

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, GetNumberValue) {
  // Open a file with four text annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  {
    // First two annotations do not have "MaxLen" attribute.
    for (int i = 0; i < 2; i++) {
      ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), i));
      ASSERT_TRUE(annot);

      // Verify that no "MaxLen" key present.
      EXPECT_FALSE(FPDFAnnot_HasKey(annot.get(), "MaxLen"));

      float value;
      EXPECT_FALSE(FPDFAnnot_GetNumberValue(annot.get(), "MaxLen", &value));
    }

    // Annotation in index 2 has "MaxLen" of 10.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetSetAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    static const char kMd5NormalAP[] = "be903df0343fd774fadab9c8900cdf4a";
    static constexpr size_t kExpectNormalAPLength = 73970;

    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // Check that the string value of an AP returns the expected length.
    unsigned long normal_length_bytes = FPDFAnnot_GetAP(
        annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL, nullptr, 0);
    ASSERT_EQ(kExpectNormalAPLength, normal_length_bytes);

    // Check that the string value of an AP is not returned if the buffer is too
    // small. The result buffer should be overwritten with an empty string.
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(normal_length_bytes);
    // Write in the buffer to verify it's not overwritten.
    UNSAFE_TODO(FXSYS_memcpy(buf.data(), "abcdefgh", 8));
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes - 1));
    UNSAFE_TODO(EXPECT_EQ(0, memcmp(buf.data(), "abcdefgh", 8)));

    // Check that the string value of an AP is returned through a buffer that is
    // the right size.
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes));
    EXPECT_EQ(kMd5NormalAP, GenerateMD5Base16(pdfium::as_byte_span(buf).first(
                                normal_length_bytes)));

    // Check that the string value of an AP is returned through a buffer that is
    // larger than necessary.
    buf = GetFPDFWideStringBuffer(normal_length_bytes + 2);
    EXPECT_EQ(kExpectNormalAPLength,
              FPDFAnnot_GetAP(annot.get(), FPDF_ANNOT_APPEARANCEMODE_NORMAL,
                              buf.data(), normal_length_bytes + 2));
    EXPECT_EQ(kMd5NormalAP, GenerateMD5Base16(pdfium::as_byte_span(buf).first(
                                normal_length_bytes)));

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
    EXPECT_EQ(kMd5NormalAP, GenerateMD5Base16(pdfium::as_byte_span(buf).first(
                                normal_length_bytes)));
  }

  // Save the modified document, then reopen it.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(page);
  {
    ScopedFPDFAnnotation new_annot(FPDFPage_GetAnnot(saved_page, 0));

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
  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, RemoveOptionalAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
}

TEST_F(FPDFAnnotEmbedderTest, RemoveRequiredAP) {
  // Open a file with four annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
}

TEST_F(FPDFAnnotEmbedderTest, ExtractLinkedAnnotations) {
  // Open a file with annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(-1, FPDFPage_GetAnnotIndex(page.get(), nullptr));

  {
    // Retrieve the highlight annotation which has its popup defined.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_EQ(0, FPDFPage_GetAnnotIndex(page.get(), annot.get()));
    static const char kPopupKey[] = "Popup";
    ASSERT_TRUE(FPDFAnnot_HasKey(annot.get(), kPopupKey));
    ASSERT_EQ(FPDF_OBJECT_REFERENCE,
              FPDFAnnot_GetValueType(annot.get(), kPopupKey));

    // Retrieve and verify the popup of the highlight annotation.
    ScopedFPDFAnnotation popup(
        FPDFAnnot_GetLinkedAnnot(annot.get(), kPopupKey));
    ASSERT_TRUE(popup);
    EXPECT_EQ(FPDF_ANNOT_POPUP, FPDFAnnot_GetSubtype(popup.get()));
    EXPECT_EQ(1, FPDFPage_GetAnnotIndex(page.get(), popup.get()));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldFlagsTextField) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation: user-editable text field.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 3));
    ASSERT_TRUE(annot);

    // Check that the flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_FALSE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_TEXT_MULTILINE);
    EXPECT_TRUE(flags & FPDF_FORMFLAG_TEXT_PASSWORD);
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldFlagsComboBox) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve the first annotation: user-editable combobox.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotNull) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Attempt to get an annotation where no annotation exists on page.
  static const FS_POINTF kOriginPoint = {0.0f, 0.0f};
  EXPECT_FALSE(
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), page.get(), &kOriginPoint));

  static const FS_POINTF kValidPoint = {120.0f, 120.0f};
  {
    // Verify there is an annotation.
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page.get(), &kValidPoint));
    EXPECT_TRUE(annot);
  }

  // Try other bad inputs at a valid location.
  EXPECT_FALSE(FPDFAnnot_GetFormFieldAtPoint(nullptr, nullptr, &kValidPoint));
  EXPECT_FALSE(
      FPDFAnnot_GetFormFieldAtPoint(nullptr, page.get(), &kValidPoint));
  EXPECT_FALSE(
      FPDFAnnot_GetFormFieldAtPoint(form_handle(), nullptr, &kValidPoint));
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotAndCheckFlagsTextField) {
  // Open file with form text fields.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve user-editable text field annotation.
    static const FS_POINTF kPoint = {105.0f, 118.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page.get(), &kPoint));
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
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page.get(), &kPoint));
    ASSERT_TRUE(annot);

    // Check that interactive form annotation flag values are as expected.
    int flags = FPDFAnnot_GetFormFieldFlags(form_handle(), annot.get());
    EXPECT_TRUE(flags & FPDF_FORMFLAG_READONLY);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_REQUIRED);
    EXPECT_FALSE(flags & FPDF_FORMFLAG_NOEXPORT);
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormAnnotAndCheckFlagsComboBox) {
  // Open file with form comboboxes.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Retrieve user-editable combobox annotation.
    static const FS_POINTF kPoint = {102.0f, 363.0f};
    ScopedFPDFAnnotation annot(
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page.get(), &kPoint));
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
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page.get(), &kPoint));
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
        FPDFAnnot_GetFormFieldAtPoint(form_handle(), page.get(), &kPoint));
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
}

TEST_F(FPDFAnnotEmbedderTest, Bug1206) {
  const char* expected_bitmap = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "a1ea1ceebb26922fae576cb79ce63af0";
    }
    return "0d9fc05c6762fd788bd23fd87a4967bc";
  }();
  static constexpr size_t kExpectedMinimumOriginalSize = 1601;

  ASSERT_TRUE(OpenDocument("bug_1206.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  const size_t original_size = GetString().size();
  EXPECT_LE(kExpectedMinimumOriginalSize, original_size);  // Sanity check.
  ClearString();

  for (size_t i = 0; i < 10; ++i) {
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, expected_bitmap);

    ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    // TODO(https://crbug.com/pdfium/1206): This is wrong. The size should be
    // equal, not bigger.
    EXPECT_GT(GetString().size(), original_size);
    ClearString();
  }
}

TEST_F(FPDFAnnotEmbedderTest, Bug1212) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(0, FPDFPage_GetAnnotCount(page.get()));

  static const char kTestKey[] = "test";
  static const wchar_t kData[] = L"\xf6\xe4";
  static const size_t kBufSize = 12;
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(kBufSize);

  {
    // Add a text annotation to the page.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_TEXT));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));
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
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_STAMP));
    ASSERT_TRUE(annot);
    const FS_RECTF bounding_rect{206.0f, 753.0f, 339.0f, 709.0f};
    EXPECT_TRUE(FPDFAnnot_SetRect(annot.get(), &bounding_rect));
    EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));
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
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(3, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(26, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    // Check bad form handle / annot.
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(nullptr, nullptr));
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), nullptr));
    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(nullptr, annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(3, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(26, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountInvalidAnnotations) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // annotations do not have "Opt" array and will return -1
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionCountWrongAnnotationType) {
  // Open a file with annotations and load its first page.
  ASSERT_TRUE(OpenDocument("multiple_form_types.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // This annotation is a form field, but does not support Options.
  ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 3));
  ASSERT_TRUE(annot);

  // Check types.
  EXPECT_EQ(FPDF_ANNOT_WIDGET, FPDFAnnot_GetSubtype(annot.get()));
  EXPECT_EQ(FPDF_FORMFIELD_TEXTFIELD,
            FPDFAnnot_GetFormFieldType(form_handle(), annot.get()));

  // Option APIs do not support this annot type. They gracefully return an error
  // and do not crash.
  EXPECT_EQ(-1, FPDFAnnot_GetOptionCount(form_handle(), annot.get()));
  EXPECT_EQ(
      0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 0, nullptr, 0));
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelCombobox) {
  // Open a file with combobox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    int index = 0;
    unsigned long length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(8u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(8u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                           buf.data(), length_bytes));
    EXPECT_EQ(L"Foo", GetPlatformWString(buf.data()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    int index = 0;
    unsigned long length_bytes =
        FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index, nullptr, 0);
    ASSERT_EQ(8u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(8u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), index,
                                           buf.data(), length_bytes));
    EXPECT_EQ(L"Foo", GetPlatformWString(buf.data()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetOptionLabelInvalidAnnotations) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // annotations do not have "Opt" array and will return 0
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 0,
                                           nullptr, 0));

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    EXPECT_EQ(0u, FPDFAnnot_GetOptionLabel(form_handle(), annot.get(), 0,
                                           nullptr, 0));
  }
}

TEST_F(FPDFAnnotEmbedderTest, IsOptionSelectedCombobox) {
  // Open a file with combobox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // Checks for Combobox with no Values (/V) or Selected Indices (/I) objects.
    int count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(3, count);
    for (int i = 0; i < count; i++) {
      EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
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
}

TEST_F(FPDFAnnotEmbedderTest, IsOptionSelectedListbox) {
  // Open a file with listbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // Checks for Listbox with no Values (/V) or Selected Indices (/I) objects.
    int count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(3, count);
    for (int i = 0; i < count; i++) {
      EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    // Checks for Listbox with Values (/V) object which is just a string.
    count = FPDFAnnot_GetOptionCount(form_handle(), annot.get());
    ASSERT_EQ(26, count);
    for (int i = 0; i < count; i++) {
      EXPECT_EQ(i == 1,
                FPDFAnnot_IsOptionSelected(form_handle(), annot.get(), i));
    }

    annot.reset(FPDFPage_GetAnnot(page.get(), 3));
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

    annot.reset(FPDFPage_GetAnnot(page.get(), 4));
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

    annot.reset(FPDFPage_GetAnnot(page.get(), 5));
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
}

TEST_F(FPDFAnnotEmbedderTest, IsOptionSelectedInvalidAnnotations) {
  // Open a file with multiple form field annotations and load its first page.
  ASSERT_TRUE(OpenDocument("multiple_form_types.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // Checks for link annotation.
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(),
                                            /*index=*/0));

    annot.reset(FPDFPage_GetAnnot(page.get(), 3));
    ASSERT_TRUE(annot);

    // Checks for text field annotation.
    EXPECT_FALSE(FPDFAnnot_IsOptionSelected(form_handle(), annot.get(),
                                            /*index=*/0));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeCombobox) {
  // Open a file with combobox annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // All 3 widgets have Tf font size 12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(12.0, value);

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    float value_two;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_two));
    EXPECT_EQ(12.0, value_two);

    annot.reset(FPDFPage_GetAnnot(page.get(), 2));
    ASSERT_TRUE(annot);

    float value_three;
    ASSERT_TRUE(
        FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_three));
    EXPECT_EQ(12.0, value_three);
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeTextField) {
  // Open a file with textfield annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // All 4 widgets have Tf font size 12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(12.0, value);

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    float value_two;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_two));
    EXPECT_EQ(12.0, value_two);

    annot.reset(FPDFPage_GetAnnot(page.get(), 2));
    ASSERT_TRUE(annot);

    float value_three;
    ASSERT_TRUE(
        FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_three));
    EXPECT_EQ(12.0, value_three);

    float value_four;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value_four));
    EXPECT_EQ(12.0, value_four);
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeInvalidAnnotationTypes) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Annotations that do not have variable text and will return -1.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));

    annot.reset(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeInvalidArguments) {
  // Open a file with combobox annotations and load its first page.
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // Check bad form handle / annot.
    float value;
    ASSERT_FALSE(FPDFAnnot_GetFontSize(nullptr, annot.get(), &value));
    ASSERT_FALSE(FPDFAnnot_GetFontSize(form_handle(), nullptr, &value));
    ASSERT_FALSE(FPDFAnnot_GetFontSize(nullptr, nullptr, &value));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFontSizeNegative) {
  // Open a file with textfield annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_negative_fontsize.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Obtain the first annotation, a text field with negative font size, -12.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    float value;
    ASSERT_TRUE(FPDFAnnot_GetFontSize(form_handle(), annot.get(), &value));
    EXPECT_EQ(-12.0, value);
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFontColor) {
  // Open a file with textfield annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form_color.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Obtain the first annotation, a text field with orange color.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // Negative testing.
    unsigned int R;
    unsigned int G;
    unsigned int B;
    ASSERT_FALSE(
        FPDFAnnot_GetFontColor(nullptr, nullptr, nullptr, nullptr, nullptr));
    ASSERT_FALSE(FPDFAnnot_GetFontColor(form_handle(), nullptr, nullptr,
                                        nullptr, nullptr));
    ASSERT_FALSE(FPDFAnnot_GetFontColor(form_handle(), annot.get(), nullptr,
                                        nullptr, nullptr));
    ASSERT_FALSE(FPDFAnnot_GetFontColor(form_handle(), annot.get(), &R, nullptr,
                                        nullptr));
    ASSERT_FALSE(
        FPDFAnnot_GetFontColor(form_handle(), annot.get(), &R, &G, nullptr));

    // Positive testing.
    ASSERT_TRUE(FPDFAnnot_GetFontColor(form_handle(), annot.get(), &R, &G, &B));
    // Make sure it's #ff8000, i.e. orange.
    EXPECT_EQ(0xffU, R);
    EXPECT_EQ(0x80U, G);
    EXPECT_EQ(0x00U, B);
  }
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedCheckbox) {
  // Open a file with checkbox and radiobuttons widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedCheckboxReadOnly) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedRadioButton) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 5));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 6));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 7));
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedRadioButtonReadOnly) {
  // Open a file with checkbox and radiobutton widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 3));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));

    annot.reset(FPDFPage_GetAnnot(page.get(), 4));
    ASSERT_TRUE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedInvalidArguments) {
  // Open a file with checkbox and radiobuttons widget annotations and load its
  // first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(nullptr, annot.get()));
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), nullptr));
    ASSERT_FALSE(FPDFAnnot_IsChecked(nullptr, nullptr));
  }
}

TEST_F(FPDFAnnotEmbedderTest, IsCheckedInvalidWidgetType) {
  // Open a file with text widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    ASSERT_FALSE(FPDFAnnot_IsChecked(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldType) {
  ASSERT_TRUE(OpenDocument("multiple_form_types.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(-1, FPDFAnnot_GetFormFieldType(form_handle(), nullptr));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);
    EXPECT_EQ(-1, FPDFAnnot_GetFormFieldType(nullptr, annot.get()));
  }

  static const struct {
    int input;
    int output;
  } kTests[] = {{0, -1},
                {1, FPDF_FORMFIELD_COMBOBOX},
                {2, FPDF_FORMFIELD_LISTBOX},
                {3, FPDF_FORMFIELD_TEXTFIELD},
                {4, FPDF_FORMFIELD_CHECKBOX},
                {5, FPDF_FORMFIELD_RADIOBUTTON}};
  for (const auto& test : kTests) {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), test.input));
    ASSERT_TRUE(annot);
    EXPECT_EQ(test.output,
              FPDFAnnot_GetFormFieldType(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldValueTextField) {
  ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldValue(form_handle(), nullptr, nullptr, 0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                               buf.data(), length_bytes));
    EXPECT_EQ(L"Elephant", GetPlatformWString(buf.data()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldValueComboBox) {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(14u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(14u, FPDFAnnot_GetFormFieldValue(form_handle(), annot.get(),
                                               buf.data(), length_bytes));
    EXPECT_EQ(L"Banana", GetPlatformWString(buf.data()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldNameTextField) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    EXPECT_EQ(0u,
              FPDFAnnot_GetFormFieldName(form_handle(), nullptr, nullptr, 0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldNameComboBox) {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    unsigned long length_bytes =
        FPDFAnnot_GetFormFieldName(form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(30u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(30u, FPDFAnnot_GetFormFieldName(form_handle(), annot.get(),
                                              buf.data(), length_bytes));
    EXPECT_EQ(L"Combo_Editable", GetPlatformWString(buf.data()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, FocusableAnnotSubtypes) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
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
  VerifyAnnotationSubtypesAndFocusability(form_handle(), page.get(),
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

  VerifyAnnotationSubtypesAndFocusability(form_handle(), page.get(),
                                          kExpectedAnnotSubtypes, {});

  // Now make links focusable.
  const FPDF_ANNOTATION_SUBTYPE kLinkSubtypes[] = {FPDF_ANNOT_LINK};
  SetAndVerifyFocusableAnnotSubtypes(form_handle(), kLinkSubtypes);

  const FPDF_ANNOTATION_SUBTYPE kExpectedLinkocusableSubtypes[] = {
      FPDF_ANNOT_LINK};
  VerifyAnnotationSubtypesAndFocusability(form_handle(), page.get(),
                                          kExpectedAnnotSubtypes,
                                          kExpectedLinkocusableSubtypes);

  // Test invalid parameters.
  EXPECT_FALSE(FPDFAnnot_SetFocusableSubtypes(nullptr, kDefaultSubtypes,
                                              std::size(kDefaultSubtypes)));
  EXPECT_FALSE(FPDFAnnot_SetFocusableSubtypes(form_handle(), nullptr,
                                              std::size(kDefaultSubtypes)));
  EXPECT_EQ(-1, FPDFAnnot_GetFocusableSubtypesCount(nullptr));

  std::vector<FPDF_ANNOTATION_SUBTYPE> subtypes(1);
  EXPECT_FALSE(FPDFAnnot_GetFocusableSubtypes(nullptr, subtypes.data(),
                                              subtypes.size()));
  EXPECT_FALSE(
      FPDFAnnot_GetFocusableSubtypes(form_handle(), nullptr, subtypes.size()));
  EXPECT_FALSE(
      FPDFAnnot_GetFocusableSubtypes(form_handle(), subtypes.data(), 0));
}

TEST_F(FPDFAnnotEmbedderTest, FocusableAnnotRendering) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    const char* md5_sum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "f8c17a0b11d5e152d9a90d6469c6be96";
#elif BUILDFLAG(IS_APPLE)
        return "cd02e06aeb6555ca7d03136cb8f2e336";
#else
        return "a08901d205e54530e76f5fc81846eb6a";
#endif
      }
#if BUILDFLAG(IS_APPLE)
      return "108a46c517c4eaace9982ee83e8e3296";
#else
      return "5550d8dcb4d1af1f50e8b4bcaef2ee60";
#endif
    }();
    // Check the initial rendering.
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, md5_sum);
  }

  // Make links and highlights focusable.
  static constexpr FPDF_ANNOTATION_SUBTYPE kSubTypes[] = {FPDF_ANNOT_LINK,
                                                          FPDF_ANNOT_HIGHLIGHT};
  constexpr int kSubTypesCount = std::size(kSubTypes);
  ASSERT_TRUE(
      FPDFAnnot_SetFocusableSubtypes(form_handle(), kSubTypes, kSubTypesCount));
  ASSERT_EQ(kSubTypesCount, FPDFAnnot_GetFocusableSubtypesCount(form_handle()));
  std::vector<FPDF_ANNOTATION_SUBTYPE> subtypes(kSubTypesCount);
  ASSERT_TRUE(FPDFAnnot_GetFocusableSubtypes(form_handle(), subtypes.data(),
                                             subtypes.size()));
  ASSERT_EQ(FPDF_ANNOT_LINK, subtypes[0]);
  ASSERT_EQ(FPDF_ANNOT_HIGHLIGHT, subtypes[1]);

  {
    const char* md5_sum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "29886792e8942117a291dee7acdbf39f";
#elif BUILDFLAG(IS_APPLE)
        return "7e85e4675adccb100fc2cf1037f65f4a";
#else
        return "de2186f2f36169d0002257a810435648";
#endif
      }
#if BUILDFLAG(IS_APPLE)
      return "eb3869335e7a219e1b5f25c1c6037b97";
#else
      return "805fe7bb751ac4ed2b82bb66efe6db40";
#endif
    }();
    // Focus the first link and check the rendering.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_LINK, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_TRUE(FORM_SetFocusedAnnot(form_handle(), annot.get()));
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, md5_sum);
  }

  {
    const char* md5_sum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "651517d1a58558c6eae18ebb3ff90784";
#elif BUILDFLAG(IS_APPLE)
        return "96f271ee3f1520d174f887f33989bbcb";
#else
        return "27bb036f3a507fce66a74a00daf558ec";
#endif
      }
#if BUILDFLAG(IS_APPLE)
      return "d20b1978da2362d3942ea0fc6d230997";
#else
      return "c5c5dcb462af3ef5f43b298ec048feef";
#endif
    }();
    // Focus the first highlight and check the rendering.
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 4));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_TRUE(FORM_SetFocusedAnnot(form_handle(), annot.get()));
    ScopedFPDFBitmap bitmap = RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 612, 792, md5_sum);
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetLinkFromAnnotation) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  {
    constexpr char kExpectedResult[] =
        "https://cs.chromium.org/chromium/src/third_party/pdfium/public/"
        "fpdf_text.h";

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 3));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_LINK, FPDFAnnot_GetSubtype(annot.get()));
    VerifyUriActionInLink(document(), FPDFAnnot_GetLink(annot.get()),
                          kExpectedResult);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 4));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_HIGHLIGHT, FPDFAnnot_GetSubtype(annot.get()));
    EXPECT_FALSE(FPDFAnnot_GetLink(annot.get()));
  }

  EXPECT_FALSE(FPDFAnnot_GetLink(nullptr));
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlCountRadioButton) {
  // Open a file with radio button widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Checks for bad annot.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlCount(form_handle(), /*annot=*/nullptr));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 3));
    ASSERT_TRUE(annot);

    // Checks for bad form handle.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlCount(/*hHandle=*/nullptr, annot.get()));

    EXPECT_EQ(3, FPDFAnnot_GetFormControlCount(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlCountCheckBox) {
  // Open a file with checkbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(1, FPDFAnnot_GetFormControlCount(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlCountInvalidAnnotation) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(-1, FPDFAnnot_GetFormControlCount(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlIndexRadioButton) {
  // Open a file with radio button widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Checks for bad annot.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlIndex(form_handle(), /*annot=*/nullptr));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 3));
    ASSERT_TRUE(annot);

    // Checks for bad form handle.
    EXPECT_EQ(-1,
              FPDFAnnot_GetFormControlIndex(/*hHandle=*/nullptr, annot.get()));

    EXPECT_EQ(1, FPDFAnnot_GetFormControlIndex(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlIndexCheckBox) {
  // Open a file with checkbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(0, FPDFAnnot_GetFormControlIndex(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormControlIndexInvalidAnnotation) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(-1, FPDFAnnot_GetFormControlIndex(form_handle(), annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldExportValueRadioButton) {
  // Open a file with radio button widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Checks for bad annot.
    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldExportValue(
                      form_handle(), /*annot=*/nullptr,
                      /*buffer=*/nullptr, /*buflen=*/0));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 6));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldExportValueCheckBox) {
  // Open a file with checkbox widget annotations and load its first page.
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
}

TEST_F(FPDFAnnotEmbedderTest, GetFormFieldExportValueInvalidAnnotation) {
  // Open a file with ink annotations and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_ink_multiple.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldExportValue(form_handle(), annot.get(),
                                                    /*buffer=*/nullptr,
                                                    /*buflen=*/0));
  }
}

TEST_F(FPDFAnnotEmbedderTest, Redactannotation) {
  ASSERT_TRUE(OpenDocument("redact_annot.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_REDACT, FPDFAnnot_GetSubtype(annot.get()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, PolygonAnnotation) {
  ASSERT_TRUE(OpenDocument("polygon_annot.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
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
    ScopedFPDFAnnotation ink_annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
    EXPECT_EQ(0U, FPDFAnnot_GetVertices(ink_annot.get(), nullptr, 0));
  }
}

TEST_F(FPDFAnnotEmbedderTest, InkAnnotation) {
  ASSERT_TRUE(OpenDocument("ink_annot.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    EXPECT_EQ(kExpectedPathSize, path_size);
    EXPECT_FLOAT_EQ(42, path_buffer[0].x);
    EXPECT_FLOAT_EQ(43, path_buffer[0].y);
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
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
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_POLYGON));
    EXPECT_EQ(0U, FPDFAnnot_GetInkListCount(polygon_annot.get()));
    const unsigned long kPathIndex = 0;
    EXPECT_EQ(0U, FPDFAnnot_GetInkListPath(polygon_annot.get(), kPathIndex,
                                           nullptr, 0));
  }
}

TEST_F(FPDFAnnotEmbedderTest, LineAnnotation) {
  ASSERT_TRUE(OpenDocument("line_annot.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);

    // Too few elements in the line array.
    FS_POINTF start;
    FS_POINTF end;
    EXPECT_FALSE(FPDFAnnot_GetLine(annot.get(), &start, &end));
  }

  {
    // Wrong annotation type.
    ScopedFPDFAnnotation ink_annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_INK));
    FS_POINTF start;
    FS_POINTF end;
    EXPECT_FALSE(FPDFAnnot_GetLine(ink_annot.get(), &start, &end));
  }
}

TEST_F(FPDFAnnotEmbedderTest, AnnotationBorder) {
  ASSERT_TRUE(OpenDocument("line_annot.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
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
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
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
}

TEST_F(FPDFAnnotEmbedderTest, AnnotationJavaScript) {
  ASSERT_TRUE(OpenDocument("annot_javascript.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // FPDFAnnot_GetFormAdditionalActionJavaScript() positive testing.
    unsigned long length_bytes = FPDFAnnot_GetFormAdditionalActionJavaScript(
        form_handle(), annot.get(), FPDF_ANNOT_AACTION_FORMAT, nullptr, 0);
    ASSERT_EQ(62u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(62u, FPDFAnnot_GetFormAdditionalActionJavaScript(
                       form_handle(), annot.get(), FPDF_ANNOT_AACTION_FORMAT,
                       buf.data(), length_bytes));
    EXPECT_EQ(L"AFDate_FormatEx(\"yyyy-mm-dd\");",
              GetPlatformWString(buf.data()));

    // FPDFAnnot_GetFormAdditionalActionJavaScript() negative testing.
    EXPECT_EQ(0u, FPDFAnnot_GetFormAdditionalActionJavaScript(
                      form_handle(), nullptr, 0, nullptr, 0));
    EXPECT_EQ(0u, FPDFAnnot_GetFormAdditionalActionJavaScript(
                      nullptr, annot.get(), 0, nullptr, 0));
    EXPECT_EQ(0u, FPDFAnnot_GetFormAdditionalActionJavaScript(
                      form_handle(), annot.get(), 0, nullptr, 0));
    EXPECT_EQ(2u, FPDFAnnot_GetFormAdditionalActionJavaScript(
                      form_handle(), annot.get(), FPDF_ANNOT_AACTION_KEY_STROKE,
                      nullptr, 0));
  }
}

TEST_F(FPDFAnnotEmbedderTest, FormFieldAlternateName) {
  ASSERT_TRUE(OpenDocument("click_form.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(8, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    // FPDFAnnot_GetFormFieldAlternateName() positive testing.
    unsigned long length_bytes = FPDFAnnot_GetFormFieldAlternateName(
        form_handle(), annot.get(), nullptr, 0);
    ASSERT_EQ(34u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(34u, FPDFAnnot_GetFormFieldAlternateName(
                       form_handle(), annot.get(), buf.data(), length_bytes));
    EXPECT_EQ(L"readOnlyCheckbox", GetPlatformWString(buf.data()));

    // FPDFAnnot_GetFormFieldAlternateName() negative testing.
    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldAlternateName(form_handle(), nullptr,
                                                      nullptr, 0));
    EXPECT_EQ(0u, FPDFAnnot_GetFormFieldAlternateName(nullptr, annot.get(),
                                                      nullptr, 0));
  }
}

// Due to https://crbug.com/pdfium/570, the AnnotationBorder test above cannot
// actually render the line annotations inside line_annot.pdf. For now, use a
// square annotation in annots.pdf for testing.
TEST_F(FPDFAnnotEmbedderTest, AnnotationBorderRendering) {
  ASSERT_TRUE(OpenDocument("annots.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(1);
  ASSERT_TRUE(page);
  EXPECT_EQ(3, FPDFPage_GetAnnotCount(page.get()));

  const char* original_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "36ab186e78c0b88eeb8f7aceea93b72c";
#elif BUILDFLAG(IS_APPLE)
      return "953b14259560aeca886ea44c9529892b";
#else
      return "238dccc7df0ac61ac580c28e1109da3c";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "522a4a6b6c7eab5bf95ded1f21ea372e";
#else
    return "12127303aecd80c6288460f7c0d79f3f";
#endif
  }();
  const char* modified_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "ece1ab24a0d9425ef3b06747c95d75ce";
#elif BUILDFLAG(IS_APPLE)
      return "bfc344e98798298bf7bb0953db75c686";
#else
      return "0f326acb3eb583125ca584d703ccb13b";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "6844019e07b83cc01723415f58218d06";
#else
    return "73d06ff4c665fe85029acef30240dcca";
#endif
  }();

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 2));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_SQUARE, FPDFAnnot_GetSubtype(annot.get()));

    {
      ScopedFPDFBitmap bitmap =
          RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, original_checksum);
    }

    EXPECT_TRUE(FPDFAnnot_SetBorder(annot.get(), /*horizontal_radius=*/2.0f,
                                    /*vertical_radius=*/3.5f,
                                    /*border_width=*/4.0f));

    {
      ScopedFPDFBitmap bitmap =
          RenderLoadedPageWithFlags(page.get(), FPDF_ANNOT);
      CompareBitmap(bitmap.get(), 612, 792, modified_checksum);
    }
  }

  // Save the document and close the page.
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(1);
  ASSERT_TRUE(saved_page);
  VerifySavedRendering(saved_page, 612, 792, modified_checksum);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFAnnotEmbedderTest, GetAndAddFileAttachmentAnnotation) {
  ASSERT_TRUE(OpenDocument("annotation_fileattachment.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_FILEATTACHMENT, FPDFAnnot_GetSubtype(annot.get()));

    FPDF_ATTACHMENT attachment = FPDFAnnot_GetFileAttachment(annot.get());
    ASSERT_TRUE(attachment);

    // Check that the name of the attachment is correct.
    unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
    ASSERT_EQ(18u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(18u,
              FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
    EXPECT_EQ(L"test.txt", GetPlatformWString(buf.data()));

    // Check that the content of the attachment is correct.
    ASSERT_TRUE(FPDFAttachment_GetFile(attachment, nullptr, 0, &length_bytes));
    std::vector<uint8_t> content_buf(length_bytes);
    unsigned long actual_length_bytes;
    ASSERT_TRUE(FPDFAttachment_GetFile(attachment, content_buf.data(),
                                       length_bytes, &actual_length_bytes));
    ASSERT_THAT(content_buf, testing::ElementsAre('t', 'e', 's', 't', ' ', 't',
                                                  'e', 'x', 't'));
  }

  {
    // Add a file attachment annotation to the page.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_FILEATTACHMENT));
    ASSERT_TRUE(annot);

    // Check that there is now 2 annotations on this page.
    EXPECT_EQ(2, FPDFPage_GetAnnotCount(page.get()));

    ScopedFPDFWideString file_name = GetFPDFWideString(L"0.txt");
    FPDF_ATTACHMENT attachment =
        FPDFAnnot_AddFileAttachment(annot.get(), file_name.get());
    ASSERT_TRUE(attachment);

    // The filling of the FPDF_ATTACHMENT has been tested in
    // fpdf_attachment_embeddertest.cpp
  }

  {
    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 1));
    ASSERT_TRUE(annot);
    EXPECT_EQ(FPDF_ANNOT_FILEATTACHMENT, FPDFAnnot_GetSubtype(annot.get()));

    // Check that we can read newly created file spec
    FPDF_ATTACHMENT attachment = FPDFAnnot_GetFileAttachment(annot.get());
    ASSERT_TRUE(attachment);

    // Verify the name of the new attachment.
    unsigned long length_bytes = FPDFAttachment_GetName(attachment, nullptr, 0);
    ASSERT_EQ(12u, length_bytes);
    std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(length_bytes);
    EXPECT_EQ(12u,
              FPDFAttachment_GetName(attachment, buf.data(), length_bytes));
    EXPECT_EQ(L"0.txt", GetPlatformWString(buf.data()));
  }
}

TEST_F(FPDFAnnotEmbedderTest, BadCasesFileAttachmentAnnotation) {
  ASSERT_TRUE(OpenDocument("annotation_fileattachment.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(1, FPDFPage_GetAnnotCount(page.get()));

  {
    ASSERT_FALSE(FPDFAnnot_GetFileAttachment(nullptr));

    ScopedFPDFAnnotation text_annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_TEXT));
    ASSERT_TRUE(text_annot);
    ASSERT_FALSE(FPDFAnnot_GetFileAttachment(text_annot.get()));

    ScopedFPDFAnnotation newly_file_annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_FILEATTACHMENT));
    ASSERT_TRUE(newly_file_annot);
    ASSERT_FALSE(FPDFAnnot_GetFileAttachment(newly_file_annot.get()));
  }

  {
    ScopedFPDFWideString empty_name = GetFPDFWideString(L"");
    ScopedFPDFWideString not_empty_name = GetFPDFWideString(L"0.txt");

    ASSERT_FALSE(FPDFAnnot_AddFileAttachment(nullptr, nullptr));
    ASSERT_FALSE(FPDFAnnot_AddFileAttachment(nullptr, empty_name.get()));
    ASSERT_FALSE(FPDFAnnot_AddFileAttachment(nullptr, not_empty_name.get()));

    ScopedFPDFAnnotation annot(FPDFPage_GetAnnot(page.get(), 0));
    ASSERT_TRUE(annot);

    ASSERT_FALSE(FPDFAnnot_AddFileAttachment(annot.get(), nullptr));
    ASSERT_FALSE(FPDFAnnot_AddFileAttachment(annot.get(), empty_name.get()));

    FPDF_ATTACHMENT old_attachment = FPDFAnnot_GetFileAttachment(annot.get());
    EXPECT_NE(old_attachment,
              FPDFAnnot_AddFileAttachment(annot.get(), not_empty_name.get()));
  }
}
