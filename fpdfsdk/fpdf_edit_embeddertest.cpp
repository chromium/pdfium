// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_font.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_edit.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gmock/include/gmock/gmock-matchers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/file_util.h"
#include "testing/utils/hash.h"
#include "testing/utils/path_service.h"
#include "third_party/base/check.h"

using pdfium::kHelloWorldChecksum;

namespace {

const char kAllRemovedChecksum[] = "eee4600ac08b458ac7ac2320e225674c";

const wchar_t kBottomText[] = L"I'm at the bottom of the page";

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
static constexpr char kBottomTextChecksum[] =
    "84461cd5d952b6ae3d57a5070da84e19";
#elif BUILDFLAG(IS_APPLE)
static constexpr char kBottomTextChecksum[] =
    "81636489006a31fcb00cf29efcdf7909";
#else
static constexpr char kBottomTextChecksum[] =
    "891dcb6e914c8360998055f1f47c9727";
#endif

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kFirstRemovedChecksum[] = "f46cbf12eb4e9bbdc3a5d8e1f2103446";
#elif BUILDFLAG(IS_APPLE)
const char kFirstRemovedChecksum[] = "a1dc2812692fcc7ee4f01ca77435df9d";
#else
const char kFirstRemovedChecksum[] = "e1477dc3b5b3b9c560814c4d1135a02b";
#endif

const wchar_t kLoadedFontText[] = L"I am testing my loaded font, WEE.";

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
const char kLoadedFontTextChecksum[] = "fc2334c350cbd0d2ae6076689da09741";
#elif BUILDFLAG(IS_APPLE)
const char kLoadedFontTextChecksum[] = "0f3e4a7d71f9e7eb8a1a0d69403b9848";
#else
const char kLoadedFontTextChecksum[] = "d58570cc045dfb818b92cbabbd1a364c";
#endif

const char kRedRectangleChecksum[] = "66d02eaa6181e2c069ce2ea99beda497";

// In embedded_images.pdf.
const char kEmbeddedImage33Checksum[] = "cb3637934bb3b95a6e4ae1ea9eb9e56e";

}  // namespace

class FPDFEditEmbedderTest : public EmbedderTest {
 protected:
  FPDF_DOCUMENT CreateNewDocument() {
    CreateEmptyDocumentWithoutFormFillEnvironment();
    cpdf_doc_ = CPDFDocumentFromFPDFDocument(document());
    return document();
  }

  void CheckFontDescriptor(const CPDF_Dictionary* font_dict,
                           int font_type,
                           bool bold,
                           bool italic,
                           pdfium::span<const uint8_t> span) {
    const CPDF_Dictionary* font_desc = font_dict->GetDictFor("FontDescriptor");
    ASSERT_TRUE(font_desc);
    EXPECT_EQ("FontDescriptor", font_desc->GetNameFor("Type"));
    ByteString font_name = font_desc->GetNameFor("FontName");
    EXPECT_FALSE(font_name.IsEmpty());
    EXPECT_EQ(font_dict->GetNameFor("BaseFont"), font_name);

    // Check that the font descriptor has the required keys according to spec
    // 1.7 Table 5.19
    ASSERT_TRUE(font_desc->KeyExist("Flags"));

    int font_flags = font_desc->GetIntegerFor("Flags");
    EXPECT_EQ(bold, FontStyleIsForceBold(font_flags));
    EXPECT_EQ(italic, FontStyleIsItalic(font_flags));
    EXPECT_TRUE(FontStyleIsNonSymbolic(font_flags));
    ASSERT_TRUE(font_desc->KeyExist("FontBBox"));

    const CPDF_Array* fontBBox = font_desc->GetArrayFor("FontBBox");
    ASSERT_TRUE(fontBBox);
    EXPECT_EQ(4u, fontBBox->size());
    // Check that the coordinates are in the preferred order according to spec
    // 1.7 Section 3.8.4
    EXPECT_TRUE(fontBBox->GetIntegerAt(0) < fontBBox->GetIntegerAt(2));
    EXPECT_TRUE(fontBBox->GetIntegerAt(1) < fontBBox->GetIntegerAt(3));

    EXPECT_TRUE(font_desc->KeyExist("ItalicAngle"));
    EXPECT_TRUE(font_desc->KeyExist("Ascent"));
    EXPECT_TRUE(font_desc->KeyExist("Descent"));
    EXPECT_TRUE(font_desc->KeyExist("CapHeight"));
    EXPECT_TRUE(font_desc->KeyExist("StemV"));
    ByteString present("FontFile");
    ByteString absent("FontFile2");
    if (font_type == FPDF_FONT_TRUETYPE)
      std::swap(present, absent);
    EXPECT_TRUE(font_desc->KeyExist(present));
    EXPECT_FALSE(font_desc->KeyExist(absent));

    auto streamAcc =
        pdfium::MakeRetain<CPDF_StreamAcc>(font_desc->GetStreamFor(present));
    streamAcc->LoadAllDataRaw();

    // Check that the font stream is the one that was provided
    ASSERT_EQ(span.size(), streamAcc->GetSize());
    if (font_type == FPDF_FONT_TRUETYPE) {
      ASSERT_EQ(static_cast<int>(span.size()),
                streamAcc->GetDict()->GetIntegerFor("Length1"));
    }

    const uint8_t* stream_data = streamAcc->GetData();
    for (size_t j = 0; j < span.size(); j++)
      EXPECT_EQ(span[j], stream_data[j]) << " at byte " << j;
  }

  void CheckCompositeFontWidths(const CPDF_Array* widths_array,
                                CPDF_Font* typed_font) {
    // Check that W array is in a format that conforms to PDF spec 1.7 section
    // "Glyph Metrics in CIDFonts" (these checks are not
    // implementation-specific).
    EXPECT_GT(widths_array->size(), 1u);
    int num_cids_checked = 0;
    int cur_cid = 0;
    for (size_t idx = 0; idx < widths_array->size(); idx++) {
      int cid = widths_array->GetNumberAt(idx);
      EXPECT_GE(cid, cur_cid);
      ASSERT_FALSE(++idx == widths_array->size());
      const CPDF_Object* next = widths_array->GetObjectAt(idx);
      if (next->IsArray()) {
        // We are in the c [w1 w2 ...] case
        const CPDF_Array* arr = next->AsArray();
        int cnt = static_cast<int>(arr->size());
        size_t inner_idx = 0;
        for (cur_cid = cid; cur_cid < cid + cnt; cur_cid++) {
          int width = arr->GetNumberAt(inner_idx++);
          EXPECT_EQ(width, typed_font->GetCharWidthF(cur_cid))
              << " at cid " << cur_cid;
        }
        num_cids_checked += cnt;
        continue;
      }
      // Otherwise, are in the c_first c_last w case.
      ASSERT_TRUE(next->IsNumber());
      int last_cid = next->AsNumber()->GetInteger();
      ASSERT_FALSE(++idx == widths_array->size());
      int width = widths_array->GetNumberAt(idx);
      for (cur_cid = cid; cur_cid <= last_cid; cur_cid++) {
        EXPECT_EQ(width, typed_font->GetCharWidthF(cur_cid))
            << " at cid " << cur_cid;
      }
      num_cids_checked += last_cid - cid + 1;
    }
    // Make sure we have a good amount of cids described
    EXPECT_GT(num_cids_checked, 200);
  }
  CPDF_Document* cpdf_doc() { return cpdf_doc_; }

 private:
  CPDF_Document* cpdf_doc_;
};

namespace {

const char kExpectedPDF[] =
    "%PDF-1.7\r\n"
    "%\xA1\xB3\xC5\xD7\r\n"
    "1 0 obj\r\n"
    "<</Pages 2 0 R /Type/Catalog>>\r\n"
    "endobj\r\n"
    "2 0 obj\r\n"
    "<</Count 1/Kids\\[ 4 0 R \\]/Type/Pages>>\r\n"
    "endobj\r\n"
    "3 0 obj\r\n"
    "<</CreationDate\\(D:.*\\)/Creator\\(PDFium\\)>>\r\n"
    "endobj\r\n"
    "4 0 obj\r\n"
    "<</MediaBox\\[ 0 0 640 480\\]/Parent 2 0 R "
    "/Resources<</ExtGState<</FXE1 5 0 R >>>>"
    "/Rotate 0/Type/Page"
    ">>\r\n"
    "endobj\r\n"
    "5 0 obj\r\n"
    "<</BM/Normal/CA 1/ca 1>>\r\n"
    "endobj\r\n"
    "xref\r\n"
    "0 6\r\n"
    "0000000000 65535 f\r\n"
    "0000000017 00000 n\r\n"
    "0000000066 00000 n\r\n"
    "0000000122 00000 n\r\n"
    "0000000192 00000 n\r\n"
    "0000000311 00000 n\r\n"
    "trailer\r\n"
    "<<\r\n"
    "/Root 1 0 R\r\n"
    "/Info 3 0 R\r\n"
    "/Size 6/ID\\[<.*><.*>\\]>>\r\n"
    "startxref\r\n"
    "354\r\n"
    "%%EOF\r\n";

}  // namespace

TEST_F(FPDFEditEmbedderTest, EmbedNotoSansSCFont) {
  CreateEmptyDocument();
  ScopedFPDFPage page(FPDFPage_New(document(), 0, 400, 400));
  std::string font_path;
  ASSERT_TRUE(PathService::GetThirdPartyFilePath(
      "NotoSansCJK/NotoSansSC-Regular.subset.otf", &font_path));

  size_t file_length = 0;
  std::unique_ptr<char, pdfium::FreeDeleter> font_data =
      GetFileContents(font_path.c_str(), &file_length);
  ASSERT_TRUE(font_data);

  ScopedFPDFFont font(FPDFText_LoadFont(
      document(), reinterpret_cast<const uint8_t*>(font_data.get()),
      file_length, FPDF_FONT_TRUETYPE, /*cid=*/true));
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_CreateTextObj(document(), font.get(), 20.0f);
  EXPECT_TRUE(text_object);

  // Test the characters which are either mapped to one single unicode or
  // multiple unicodes in the embedded font.
  ScopedFPDFWideString text = GetFPDFWideString(L"这是第一句。 这是第二行。");
  EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));

  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 50, 200);
  FPDFPage_InsertObject(page.get(), text_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#if BUILDFLAG(IS_APPLE)
  const char kChecksum[] = "9a31fb87d1c6d2346bba22d1196041cd";
#else   // BUILDFLAG(IS_APPLE)
  const char kChecksum[] = "5bb65e15fc0a685934cd5006dec08a76";
#endif  // BUILDFLAG(IS_APPLE)
#else   // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kChecksum[] = "9a31fb87d1c6d2346bba22d1196041cd";
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
  CompareBitmap(page_bitmap.get(), 400, 400, kChecksum);

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(400, 400, kChecksum);
}

TEST_F(FPDFEditEmbedderTest, EmbedNotoSansSCFontWithCharcodes) {
  CreateEmptyDocument();
  ScopedFPDFPage page(FPDFPage_New(document(), 0, 400, 400));
  std::string font_path;
  ASSERT_TRUE(PathService::GetThirdPartyFilePath(
      "NotoSansCJK/NotoSansSC-Regular.subset.otf", &font_path));

  size_t file_length = 0;
  std::unique_ptr<char, pdfium::FreeDeleter> font_data =
      GetFileContents(font_path.c_str(), &file_length);
  ASSERT_TRUE(font_data);

  ScopedFPDFFont font(FPDFText_LoadFont(
      document(), reinterpret_cast<const uint8_t*>(font_data.get()),
      file_length, FPDF_FONT_TRUETYPE, /*cid=*/true));
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_CreateTextObj(document(), font.get(), 20.0f);
  EXPECT_TRUE(text_object);

  // Same as `text` in the EmbedNotoSansSCFont test case above.
  const std::vector<uint32_t> charcodes = {9, 6, 7, 3, 5, 2, 1,
                                           9, 6, 7, 4, 8, 2};
  EXPECT_TRUE(
      FPDFText_SetCharcodes(text_object, charcodes.data(), charcodes.size()));

  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 50, 200);
  FPDFPage_InsertObject(page.get(), text_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#if BUILDFLAG(IS_APPLE)
  const char kChecksum[] = "9a31fb87d1c6d2346bba22d1196041cd";
#else   // BUILDFLAG(IS_APPLE)
  const char kChecksum[] = "5bb65e15fc0a685934cd5006dec08a76";
#endif  // BUILDFLAG(IS_APPLE)
#else   // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kChecksum[] = "9a31fb87d1c6d2346bba22d1196041cd";
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
  CompareBitmap(page_bitmap.get(), 400, 400, kChecksum);

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(400, 400, kChecksum);
}

TEST_F(FPDFEditEmbedderTest, EmptyCreation) {
  CreateEmptyDocument();
  FPDF_PAGE page = FPDFPage_New(document(), 0, 640.0, 480.0);
  EXPECT_NE(nullptr, page);
  // The FPDFPage_GenerateContent call should do nothing.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  EXPECT_THAT(GetString(), testing::MatchesRegex(std::string(
                               kExpectedPDF, sizeof(kExpectedPDF))));
  FPDF_ClosePage(page);
}

// Regression test for https://crbug.com/667012
TEST_F(FPDFEditEmbedderTest, RasterizePDF) {
  const char kAllBlackMd5sum[] = "5708fc5c4a8bd0abde99c8e8f0390615";

  // Get the bitmap for the original document.
  ScopedFPDFBitmap orig_bitmap;
  {
    ASSERT_TRUE(OpenDocument("black.pdf"));
    FPDF_PAGE orig_page = LoadPage(0);
    ASSERT_TRUE(orig_page);
    orig_bitmap = RenderLoadedPage(orig_page);
    CompareBitmap(orig_bitmap.get(), 612, 792, kAllBlackMd5sum);
    UnloadPage(orig_page);
  }

  // Create a new document from |orig_bitmap| and save it.
  {
    FPDF_DOCUMENT temp_doc = FPDF_CreateNewDocument();
    FPDF_PAGE temp_page = FPDFPage_New(temp_doc, 0, 612, 792);

    // Add the bitmap to an image object and add the image object to the output
    // page.
    FPDF_PAGEOBJECT temp_img = FPDFPageObj_NewImageObj(temp_doc);
    EXPECT_TRUE(
        FPDFImageObj_SetBitmap(&temp_page, 1, temp_img, orig_bitmap.get()));
    static constexpr FS_MATRIX kLetterScaleMatrix{612, 0, 0, 792, 0, 0};
    EXPECT_TRUE(FPDFPageObj_SetMatrix(temp_img, &kLetterScaleMatrix));
    FPDFPage_InsertObject(temp_page, temp_img);
    EXPECT_TRUE(FPDFPage_GenerateContent(temp_page));
    EXPECT_TRUE(FPDF_SaveAsCopy(temp_doc, this, 0));
    FPDF_ClosePage(temp_page);
    FPDF_CloseDocument(temp_doc);
  }

  // Get the generated content. Make sure it is at least as big as the original
  // PDF.
  EXPECT_GT(GetString().size(), 923u);
  VerifySavedDocument(612, 792, kAllBlackMd5sum);
}

TEST_F(FPDFEditEmbedderTest, AddPaths) {
  // Start with a blank page
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);
  ASSERT_TRUE(page);

  // We will first add a red rectangle
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(10, 10, 20, 20);
  ASSERT_TRUE(red_rect);
  // Expect false when trying to set colors out of range
  EXPECT_FALSE(FPDFPageObj_SetStrokeColor(red_rect, 100, 100, 100, 300));
  EXPECT_FALSE(FPDFPageObj_SetFillColor(red_rect, 200, 256, 200, 0));

  // Fill rectangle with red and insert to the page
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));

  int fillmode = FPDF_FILLMODE_NONE;
  FPDF_BOOL stroke = true;
  EXPECT_TRUE(FPDFPath_GetDrawMode(red_rect, &fillmode, &stroke));
  EXPECT_EQ(FPDF_FILLMODE_ALTERNATE, fillmode);
  EXPECT_FALSE(stroke);

  static constexpr FS_MATRIX kMatrix = {1, 2, 3, 4, 5, 6};
  EXPECT_TRUE(FPDFPageObj_SetMatrix(red_rect, &kMatrix));

  FS_MATRIX matrix;
  EXPECT_TRUE(FPDFPageObj_GetMatrix(red_rect, &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(2.0f, matrix.b);
  EXPECT_FLOAT_EQ(3.0f, matrix.c);
  EXPECT_FLOAT_EQ(4.0f, matrix.d);
  EXPECT_FLOAT_EQ(5.0f, matrix.e);
  EXPECT_FLOAT_EQ(6.0f, matrix.f);

  // Set back the identity matrix.
  matrix = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
  EXPECT_TRUE(FPDFPageObj_SetMatrix(red_rect, &matrix));

  FPDFPage_InsertObject(page, red_rect);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kRedRectangleChecksum);
  }

  // Now add to that a green rectangle with some medium alpha
  FPDF_PAGEOBJECT green_rect = FPDFPageObj_CreateNewRect(100, 100, 40, 40);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(green_rect, 0, 255, 0, 128));

  // Make sure the type of the rectangle is a path.
  EXPECT_EQ(FPDF_PAGEOBJ_PATH, FPDFPageObj_GetType(green_rect));

  // Make sure we get back the same color we set previously.
  unsigned int R;
  unsigned int G;
  unsigned int B;
  unsigned int A;
  EXPECT_TRUE(FPDFPageObj_GetFillColor(green_rect, &R, &G, &B, &A));
  EXPECT_EQ(0u, R);
  EXPECT_EQ(255u, G);
  EXPECT_EQ(0u, B);
  EXPECT_EQ(128u, A);

  // Make sure the path has 5 points (1 CFX_Path::Point::Type::kMove and 4
  // CFX_Path::Point::Type::kLine).
  ASSERT_EQ(5, FPDFPath_CountSegments(green_rect));
  // Verify actual coordinates.
  FPDF_PATHSEGMENT segment = FPDFPath_GetPathSegment(green_rect, 0);
  float x;
  float y;
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(100, x);
  EXPECT_EQ(100, y);
  EXPECT_EQ(FPDF_SEGMENT_MOVETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));
  segment = FPDFPath_GetPathSegment(green_rect, 1);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(100, x);
  EXPECT_EQ(140, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));
  segment = FPDFPath_GetPathSegment(green_rect, 2);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(140, x);
  EXPECT_EQ(140, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));
  segment = FPDFPath_GetPathSegment(green_rect, 3);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(140, x);
  EXPECT_EQ(100, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));
  segment = FPDFPath_GetPathSegment(green_rect, 4);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(100, x);
  EXPECT_EQ(100, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_TRUE(FPDFPathSegment_GetClose(segment));

  EXPECT_TRUE(FPDFPath_SetDrawMode(green_rect, FPDF_FILLMODE_WINDING, 0));
  FPDFPage_InsertObject(page, green_rect);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "7b0b87604594e773add528fae567a558");
  }

  // Add a black triangle.
  FPDF_PAGEOBJECT black_path = FPDFPageObj_CreateNewPath(400, 100);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(black_path, 0, 0, 0, 200));
  EXPECT_TRUE(FPDFPath_SetDrawMode(black_path, FPDF_FILLMODE_ALTERNATE, 0));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 400, 200));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 300, 100));
  EXPECT_TRUE(FPDFPath_Close(black_path));

  // Make sure the path has 3 points (1 CFX_Path::Point::Type::kMove and 2
  // CFX_Path::Point::Type::kLine).
  ASSERT_EQ(3, FPDFPath_CountSegments(black_path));
  // Verify actual coordinates.
  segment = FPDFPath_GetPathSegment(black_path, 0);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(400, x);
  EXPECT_EQ(100, y);
  EXPECT_EQ(FPDF_SEGMENT_MOVETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));
  segment = FPDFPath_GetPathSegment(black_path, 1);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(400, x);
  EXPECT_EQ(200, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));
  segment = FPDFPath_GetPathSegment(black_path, 2);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(300, x);
  EXPECT_EQ(100, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_TRUE(FPDFPathSegment_GetClose(segment));
  // Make sure out of bounds index access fails properly.
  EXPECT_EQ(nullptr, FPDFPath_GetPathSegment(black_path, 3));

  FPDFPage_InsertObject(page, black_path);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "eadc8020a14dfcf091da2688733d8806");
  }

  // Now add a more complex blue path.
  FPDF_PAGEOBJECT blue_path = FPDFPageObj_CreateNewPath(200, 200);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(blue_path, 0, 0, 255, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(blue_path, FPDF_FILLMODE_WINDING, 0));
  EXPECT_TRUE(FPDFPath_LineTo(blue_path, 230, 230));
  EXPECT_TRUE(FPDFPath_BezierTo(blue_path, 250, 250, 280, 280, 300, 300));
  EXPECT_TRUE(FPDFPath_LineTo(blue_path, 325, 325));
  EXPECT_TRUE(FPDFPath_LineTo(blue_path, 350, 325));
  EXPECT_TRUE(FPDFPath_BezierTo(blue_path, 375, 330, 390, 360, 400, 400));
  EXPECT_TRUE(FPDFPath_Close(blue_path));
  FPDFPage_InsertObject(page, blue_path);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kLastChecksum[] = "ed14c60702b1489c597c7d46ece7f86d";
#else
  static constexpr char kLastChecksum[] = "9823e1a21bd9b72b6a442ba4f12af946";
#endif
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kLastChecksum);
  }

  // Now save the result, closing the page and document
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Render the saved result
  VerifySavedDocument(612, 792, kLastChecksum);
}

TEST_F(FPDFEditEmbedderTest, ClipPath) {
  // Load document with a clipped rectangle.
  ASSERT_TRUE(OpenDocument("clip_path.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(1, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT triangle = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(triangle);

  // Test that we got the expected triangle.
  ASSERT_EQ(4, FPDFPath_CountSegments(triangle));

  FPDF_PATHSEGMENT segment = FPDFPath_GetPathSegment(triangle, 0);
  float x;
  float y;
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(10, x);
  EXPECT_EQ(10, y);
  EXPECT_EQ(FPDF_SEGMENT_MOVETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  segment = FPDFPath_GetPathSegment(triangle, 1);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(25, x);
  EXPECT_EQ(40, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  segment = FPDFPath_GetPathSegment(triangle, 2);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(40, x);
  EXPECT_EQ(10, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  segment = FPDFPath_GetPathSegment(triangle, 3);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_TRUE(FPDFPathSegment_GetClose(segment));

  // Test FPDFPageObj_GetClipPath().
  ASSERT_EQ(nullptr, FPDFPageObj_GetClipPath(nullptr));

  FPDF_CLIPPATH clip_path = FPDFPageObj_GetClipPath(triangle);
  ASSERT_TRUE(clip_path);

  // Test FPDFClipPath_CountPaths().
  ASSERT_EQ(-1, FPDFClipPath_CountPaths(nullptr));
  ASSERT_EQ(1, FPDFClipPath_CountPaths(clip_path));

  // Test FPDFClipPath_CountPathSegments().
  ASSERT_EQ(-1, FPDFClipPath_CountPathSegments(nullptr, 0));
  ASSERT_EQ(-1, FPDFClipPath_CountPathSegments(clip_path, -1));
  ASSERT_EQ(-1, FPDFClipPath_CountPathSegments(clip_path, 1));
  ASSERT_EQ(4, FPDFClipPath_CountPathSegments(clip_path, 0));

  // FPDFClipPath_GetPathSegment() negative testing.
  ASSERT_EQ(nullptr, FPDFClipPath_GetPathSegment(nullptr, 0, 0));
  ASSERT_EQ(nullptr, FPDFClipPath_GetPathSegment(clip_path, -1, 0));
  ASSERT_EQ(nullptr, FPDFClipPath_GetPathSegment(clip_path, 1, 0));
  ASSERT_EQ(nullptr, FPDFClipPath_GetPathSegment(clip_path, 0, -1));
  ASSERT_EQ(nullptr, FPDFClipPath_GetPathSegment(clip_path, 0, 4));

  // FPDFClipPath_GetPathSegment() positive testing.
  segment = FPDFClipPath_GetPathSegment(clip_path, 0, 0);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(10, x);
  EXPECT_EQ(15, y);
  EXPECT_EQ(FPDF_SEGMENT_MOVETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  segment = FPDFClipPath_GetPathSegment(clip_path, 0, 1);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(40, x);
  EXPECT_EQ(15, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  segment = FPDFClipPath_GetPathSegment(clip_path, 0, 2);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(40, x);
  EXPECT_EQ(35, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  segment = FPDFClipPath_GetPathSegment(clip_path, 0, 3);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_EQ(10, x);
  EXPECT_EQ(35, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, BUG_1399) {
  // Load document with a clipped rectangle.
  ASSERT_TRUE(OpenDocument("bug_1399.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(7, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(obj);

  ASSERT_EQ(2, FPDFPath_CountSegments(obj));

  FPDF_PATHSEGMENT segment = FPDFPath_GetPathSegment(obj, 0);
  float x;
  float y;
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_FLOAT_EQ(107.718f, x);
  EXPECT_FLOAT_EQ(719.922f, y);
  EXPECT_EQ(FPDF_SEGMENT_MOVETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  segment = FPDFPath_GetPathSegment(obj, 1);
  EXPECT_TRUE(FPDFPathSegment_GetPoint(segment, &x, &y));
  EXPECT_FLOAT_EQ(394.718f, x);
  EXPECT_FLOAT_EQ(719.922f, y);
  EXPECT_EQ(FPDF_SEGMENT_LINETO, FPDFPathSegment_GetType(segment));
  EXPECT_FALSE(FPDFPathSegment_GetClose(segment));

  FPDF_CLIPPATH clip_path = FPDFPageObj_GetClipPath(obj);
  ASSERT_TRUE(clip_path);

  EXPECT_EQ(-1, FPDFClipPath_CountPaths(clip_path));
  EXPECT_EQ(-1, FPDFClipPath_CountPathSegments(clip_path, 0));
  EXPECT_FALSE(FPDFClipPath_GetPathSegment(clip_path, 0, 0));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, BUG_1549) {
  static const char kOriginalChecksum[] = "126366fb95e6caf8ea196780075b22b2";
  static const char kRemovedChecksum[] = "6ec2f27531927882624b37bc7d8e12f4";

  ASSERT_TRUE(OpenDocument("bug_1549.pdf"));
  FPDF_PAGE page = LoadPage(0);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
    CompareBitmap(bitmap.get(), 100, 150, kOriginalChecksum);

    ScopedFPDFPageObject obj(FPDFPage_GetObject(page, 0));
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj.get()));
    ASSERT_TRUE(FPDFPage_RemoveObject(page, obj.get()));
  }

  ASSERT_TRUE(FPDFPage_GenerateContent(page));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
    CompareBitmap(bitmap.get(), 100, 150, kRemovedChecksum);
  }

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // TODO(crbug.com/pdfium/1549): Should be `kRemovedChecksum`.
  VerifySavedDocument(100, 150, "4f9889cd5993db20f1ab37d677ac8d26");
}

TEST_F(FPDFEditEmbedderTest, SetText) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Get the "Hello, world!" text object and change it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  ScopedFPDFWideString text1 = GetFPDFWideString(L"Changed for SetText test");
  EXPECT_TRUE(FPDFText_SetText(page_object, text1.get()));

  // Verify the "Hello, world!" text is gone and "Changed for SetText test" is
  // now displayed.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kChangedChecksum[] = "49c8602cb60508009a34d0caaac63bb4";
#elif BUILDFLAG(IS_APPLE)
  const char kChangedChecksum[] = "b720e83476fd6819d47c533f1f43c728";
#else
  const char kChangedChecksum[] = "9a85b9354a69c61772ed24151c140f46";
#endif
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kChangedChecksum);
  }

  // Now save the result.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  UnloadPage(page);

  // Re-open the file and check the changes were kept in the saved .pdf.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(2, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kChangedChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, SetCharcodesBadParams) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(2, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);

  const uint32_t kDummyValue = 42;
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, nullptr, 0));
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, nullptr, 1));
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, &kDummyValue, 0));
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, &kDummyValue, 1));
  EXPECT_FALSE(FPDFText_SetCharcodes(page_object, nullptr, 1));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, SetTextKeepClippingPath) {
  // Load document with some text, with parts clipped.
  ASSERT_TRUE(OpenDocument("bug_1558.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kOriginalChecksum[] =
      "92ff84385a0f986eacfa4bbecf8d7a7a";
#elif BUILDFLAG(IS_APPLE)
  static constexpr char kOriginalChecksum[] =
      "ae7a25c85e0e2dd0c5cb9dd5cd37f6df";
#else
  static constexpr char kOriginalChecksum[] =
      "7af7fe5b281298261eb66ac2d22f5054";
#endif
  {
    // When opened before any editing and saving, the clipping path is rendered.
    ScopedFPDFBitmap original_bitmap = RenderPage(page);
    CompareBitmap(original_bitmap.get(), 200, 200, kOriginalChecksum);
  }

  // "Change" the text in the objects to their current values to force them to
  // regenerate when saving.
  {
    ScopedFPDFTextPage text_page(FPDFText_LoadPage(page));
    ASSERT_TRUE(text_page);
    const int obj_count = FPDFPage_CountObjects(page);
    ASSERT_EQ(2, obj_count);
    for (int i = 0; i < obj_count; ++i) {
      FPDF_PAGEOBJECT text_obj = FPDFPage_GetObject(page, i);
      ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_obj));
      unsigned long size =
          FPDFTextObj_GetText(text_obj, text_page.get(),
                              /*buffer=*/nullptr, /*length=*/0);
      ASSERT_GT(size, 0u);
      std::vector<FPDF_WCHAR> buffer = GetFPDFWideStringBuffer(size);
      ASSERT_EQ(size, FPDFTextObj_GetText(text_obj, text_page.get(),
                                          buffer.data(), size));
      EXPECT_TRUE(FPDFText_SetText(text_obj, buffer.data()));
    }
  }

  {
    // After editing but before saving, the clipping path is retained.
    ScopedFPDFBitmap edited_bitmap = RenderPage(page);
    CompareBitmap(edited_bitmap.get(), 200, 200, kOriginalChecksum);
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Open the saved copy and render it.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  {
    ScopedFPDFBitmap saved_bitmap = RenderSavedPage(saved_page);
    CompareBitmap(saved_bitmap.get(), 200, 200, kOriginalChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, BUG_1574) {
  // Load document with some text within a clipping path.
  ASSERT_TRUE(OpenDocument("bug_1574.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kOriginalChecksum[] =
      "1e022a0360f053ecb41cc431a36834a6";
#elif BUILDFLAG(IS_APPLE)
  static constexpr char kOriginalChecksum[] =
      "1226bc2b8072622eb28f52321876e814";
#else
  static constexpr char kOriginalChecksum[] =
      "c5241eef60b9eac68ed1f2a5fd002703";
#endif
  {
    // When opened before any editing and saving, the text object is rendered.
    ScopedFPDFBitmap original_bitmap = RenderPage(page);
    CompareBitmap(original_bitmap.get(), 200, 300, kOriginalChecksum);
  }

  // "Change" the text in the objects to their current values to force them to
  // regenerate when saving.
  {
    ScopedFPDFTextPage text_page(FPDFText_LoadPage(page));
    ASSERT_TRUE(text_page);

    ASSERT_EQ(2, FPDFPage_CountObjects(page));
    FPDF_PAGEOBJECT text_obj = FPDFPage_GetObject(page, 1);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_obj));

    unsigned long size = FPDFTextObj_GetText(text_obj, text_page.get(),
                                             /*buffer=*/nullptr, /*length=*/0);
    ASSERT_GT(size, 0u);
    std::vector<FPDF_WCHAR> buffer = GetFPDFWideStringBuffer(size);
    ASSERT_EQ(size, FPDFTextObj_GetText(text_obj, text_page.get(),
                                        buffer.data(), size));
    EXPECT_TRUE(FPDFText_SetText(text_obj, buffer.data()));
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Open the saved copy and render it.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  {
    ScopedFPDFBitmap saved_bitmap = RenderSavedPage(saved_page);
    CompareBitmap(saved_bitmap.get(), 200, 300, kOriginalChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemovePageObject) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Show what the original file looks like.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldChecksum);
  }

  // Get the "Hello, world!" text object and remove it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));

  // Verify the "Hello, world!" text is gone.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kFirstRemovedChecksum);
  }
  ASSERT_EQ(1, FPDFPage_CountObjects(page));

  UnloadPage(page);
  FPDFPageObj_Destroy(page_object);
}

void CheckMarkCounts(FPDF_PAGE page,
                     int start_from,
                     int expected_object_count,
                     size_t expected_prime_count,
                     size_t expected_square_count,
                     size_t expected_greater_than_ten_count,
                     size_t expected_bounds_count) {
  int object_count = FPDFPage_CountObjects(page);
  ASSERT_EQ(expected_object_count, object_count);

  size_t prime_count = 0;
  size_t square_count = 0;
  size_t greater_than_ten_count = 0;
  size_t bounds_count = 0;
  for (int i = 0; i < object_count; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = 0; j < mark_count; ++j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      char buffer[256];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name =
          GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
      if (name == L"Prime") {
        prime_count++;
      } else if (name == L"Square") {
        square_count++;
        int expected_square = start_from + i;
        EXPECT_EQ(1, FPDFPageObjMark_CountParams(mark));

        unsigned long get_param_key_return = 999u;
        ASSERT_TRUE(FPDFPageObjMark_GetParamKey(mark, 0, buffer, sizeof(buffer),
                                                &get_param_key_return));
        EXPECT_EQ((6u + 1u) * 2u, get_param_key_return);
        std::wstring key =
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
        EXPECT_EQ(L"Factor", key);

        EXPECT_EQ(FPDF_OBJECT_NUMBER,
                  FPDFPageObjMark_GetParamValueType(mark, "Factor"));
        int square_root;
        EXPECT_TRUE(
            FPDFPageObjMark_GetParamIntValue(mark, "Factor", &square_root));
        EXPECT_EQ(expected_square, square_root * square_root);
      } else if (name == L"GreaterThanTen") {
        greater_than_ten_count++;
      } else if (name == L"Bounds") {
        bounds_count++;
        EXPECT_EQ(1, FPDFPageObjMark_CountParams(mark));

        unsigned long get_param_key_return = 999u;
        ASSERT_TRUE(FPDFPageObjMark_GetParamKey(mark, 0, buffer, sizeof(buffer),
                                                &get_param_key_return));
        EXPECT_EQ((8u + 1u) * 2u, get_param_key_return);
        std::wstring key =
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
        EXPECT_EQ(L"Position", key);

        EXPECT_EQ(FPDF_OBJECT_STRING,
                  FPDFPageObjMark_GetParamValueType(mark, "Position"));
        unsigned long length;
        EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(
            mark, "Position", buffer, sizeof(buffer), &length));
        ASSERT_GT(length, 0u);
        std::wstring value =
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));

        // "Position" can be "First", "Last", or "End".
        if (i == 0) {
          EXPECT_EQ((5u + 1u) * 2u, length);
          EXPECT_EQ(L"First", value);
        } else if (i == object_count - 1) {
          if (length == (4u + 1u) * 2u) {
            EXPECT_EQ(L"Last", value);
          } else if (length == (3u + 1u) * 2u) {
            EXPECT_EQ(L"End", value);
          } else {
            FAIL();
          }
        } else {
          FAIL();
        }
      } else {
        FAIL();
      }
    }
  }

  // Expect certain number of tagged objects. The test file contains strings
  // from 1 to 19.
  EXPECT_EQ(expected_prime_count, prime_count);
  EXPECT_EQ(expected_square_count, square_count);
  EXPECT_EQ(expected_greater_than_ten_count, greater_than_ten_count);
  EXPECT_EQ(expected_bounds_count, bounds_count);
}

TEST_F(FPDFEditEmbedderTest, ReadMarkedObjectsIndirectDict) {
  // Load document with some text marked with an indirect property.
  ASSERT_TRUE(OpenDocument("text_in_page_marked_indirect.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  CheckMarkCounts(page, 1, 19, 8, 4, 9, 1);

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, RemoveMarkedObjectsPrime) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Show what the original file looks like.
  {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static constexpr char kOriginalChecksum[] =
        "8a8bed7820764522955f422e27f4292f";
#elif BUILDFLAG(IS_APPLE)
    static constexpr char kOriginalChecksum[] =
        "966579fb98206858ce2f0a1f94a74d05";
#else
    static constexpr char kOriginalChecksum[] =
        "3d5a3de53d5866044c2b6bf339742c97";
#endif
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kOriginalChecksum);
  }

  constexpr int expected_object_count = 19;
  CheckMarkCounts(page, 1, expected_object_count, 8, 4, 9, 1);

  // Get all objects marked with "Prime"
  std::vector<FPDF_PAGEOBJECT> primes;
  for (int i = 0; i < expected_object_count; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = 0; j < mark_count; ++j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      char buffer[256];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name =
          GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
      if (name == L"Prime") {
        primes.push_back(page_object);
      }
    }
  }

  // Remove all objects marked with "Prime".
  for (FPDF_PAGEOBJECT page_object : primes) {
    EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));
    FPDFPageObj_Destroy(page_object);
  }

  EXPECT_EQ(11, FPDFPage_CountObjects(page));
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kNonPrimesChecksum[] =
      "1573b85dd9a2c59401c1c30abbee3b25";
  static constexpr char kNonPrimesAfterSaveChecksum[] =
      "1573b85dd9a2c59401c1c30abbee3b25";
#elif BUILDFLAG(IS_APPLE)
  static constexpr char kNonPrimesChecksum[] =
      "6e19a4dd674b522cd39cf41956559bd6";
  static constexpr char kNonPrimesAfterSaveChecksum[] =
      "3cb35c681f8fb5a43a49146ac7caa818";
#else
  static constexpr char kNonPrimesChecksum[] =
      "bc8623c052f12376c3d8dd09a6cd27df";
  static constexpr char kNonPrimesAfterSaveChecksum[] =
      "bc8623c052f12376c3d8dd09a6cd27df";
#endif
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kNonPrimesChecksum);
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the prime marks are not there anymore.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(11, FPDFPage_CountObjects(saved_page));

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kNonPrimesAfterSaveChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveMarks) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  constexpr int kExpectedObjectCount = 19;
  CheckMarkCounts(page, 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Remove all "Prime" content marks.
  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = mark_count - 1; j >= 0; --j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      char buffer[256];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name =
          GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
      if (name == L"Prime") {
        // Remove mark.
        EXPECT_TRUE(FPDFPageObj_RemoveMark(page_object, mark));

        // Verify there is now one fewer mark in the page object.
        EXPECT_EQ(mark_count - 1, FPDFPageObj_CountMarks(page_object));
      }
    }
  }

  // Verify there are 0 "Prime" content marks now.
  CheckMarkCounts(page, 1, kExpectedObjectCount, 0, 4, 9, 1);

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the prime marks are not there anymore.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  CheckMarkCounts(saved_page, 1, kExpectedObjectCount, 0, 4, 9, 1);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveMarkParam) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  constexpr int kExpectedObjectCount = 19;
  CheckMarkCounts(page, 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Remove all "Square" content marks parameters.
  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = 0; j < mark_count; ++j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      char buffer[256];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name =
          GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
      if (name == L"Square") {
        // Show the mark has a "Factor" parameter.
        int out_value;
        EXPECT_TRUE(
            FPDFPageObjMark_GetParamIntValue(mark, "Factor", &out_value));

        // Remove parameter.
        EXPECT_TRUE(FPDFPageObjMark_RemoveParam(page_object, mark, "Factor"));

        // Verify the "Factor" parameter is gone.
        EXPECT_FALSE(
            FPDFPageObjMark_GetParamIntValue(mark, "Factor", &out_value));
      }
    }
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the "Factor" parameters are still gone.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  size_t square_count = 0;
  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(saved_page, i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = 0; j < mark_count; ++j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      char buffer[256];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name =
          GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
      if (name == L"Square") {
        // Verify the "Factor" parameter is still gone.
        int out_value;
        EXPECT_FALSE(
            FPDFPageObjMark_GetParamIntValue(mark, "Factor", &out_value));

        ++square_count;
      }
    }
  }

  // Verify the parameters are gone, but the marks are not.
  EXPECT_EQ(4u, square_count);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, MaintainMarkedObjects) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Iterate over all objects, counting the number of times each content mark
  // name appears.
  CheckMarkCounts(page, 1, 19, 8, 4, 9, 1);

  // Remove first page object.
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));
  FPDFPageObj_Destroy(page_object);

  CheckMarkCounts(page, 2, 18, 8, 3, 9, 1);

  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  UnloadPage(page);

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  CheckMarkCounts(saved_page, 2, 18, 8, 3, 9, 1);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, MaintainIndirectMarkedObjects) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked_indirect.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Iterate over all objects, counting the number of times each content mark
  // name appears.
  CheckMarkCounts(page, 1, 19, 8, 4, 9, 1);

  // Remove first page object.
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));
  FPDFPageObj_Destroy(page_object);

  CheckMarkCounts(page, 2, 18, 8, 3, 9, 1);

  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  UnloadPage(page);

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  CheckMarkCounts(saved_page, 2, 18, 8, 3, 9, 1);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveExistingPageObject) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Get the "Hello, world!" text object and remove it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));

  // Verify the "Hello, world!" text is gone.
  ASSERT_EQ(1, FPDFPage_CountObjects(page));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);
  FPDFPageObj_Destroy(page_object);

  // Re-open the file and check the page object count is still 1.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(1, FPDFPage_CountObjects(saved_page));
  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveExistingPageObjectSplitStreamsNotLonely) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_split_streams.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Get the "Hello, world!" text object and remove it. There is another object
  // in the same stream that says "Goodbye, world!"
  ASSERT_EQ(3, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));

  // Verify the "Hello, world!" text is gone.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kHelloRemovedChecksum[] = "fc2a40a3d1edfe6e972be104b5ae87ad";
#elif BUILDFLAG(IS_APPLE)
  const char kHelloRemovedChecksum[] = "5508c2f06d104050f74f655693e38c2c";
#else
  const char kHelloRemovedChecksum[] = "a8cd82499cf744e0862ca468c9d4ceb8";
#endif
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloRemovedChecksum);
  }

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);
  FPDFPageObj_Destroy(page_object);

  // Re-open the file and check the page object count is still 2.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(2, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloRemovedChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveExistingPageObjectSplitStreamsLonely) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_split_streams.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Get the "Greetings, world!" text object and remove it. This is the only
  // object in the stream.
  ASSERT_EQ(3, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 2);
  ASSERT_TRUE(page_object);
  EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));

  // Verify the "Greetings, world!" text is gone.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldChecksum);
  }

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);
  FPDFPageObj_Destroy(page_object);

  // Re-open the file and check the page object count is still 2.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(2, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, GetContentStream) {
  // Load document with some text split across streams.
  ASSERT_TRUE(OpenDocument("split_streams.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-14.
  // Content stream 1: page objects 15-17.
  // Content stream 2: page object 18.
  ASSERT_EQ(19, FPDFPage_CountObjects(page));
  for (int i = 0; i < 19; i++) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    if (i < 15)
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    else if (i < 18)
      EXPECT_EQ(1, cpdf_page_object->GetContentStream()) << i;
    else
      EXPECT_EQ(2, cpdf_page_object->GetContentStream()) << i;
  }

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, RemoveAllFromStream) {
  // Load document with some text split across streams.
  ASSERT_TRUE(OpenDocument("split_streams.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-14.
  // Content stream 1: page objects 15-17.
  // Content stream 2: page object 18.
  ASSERT_EQ(19, FPDFPage_CountObjects(page));

  // Loop backwards because objects will being removed, which shifts the indexes
  // after the removed position.
  for (int i = 18; i >= 0; i--) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);

    // Empty content stream 1.
    if (cpdf_page_object->GetContentStream() == 1) {
      EXPECT_TRUE(FPDFPage_RemoveObject(page, page_object));
      FPDFPageObj_Destroy(page_object);
    }
  }

  // Content stream 0: page objects 0-14.
  // Content stream 2: page object 15.
  ASSERT_EQ(16, FPDFPage_CountObjects(page));
  for (int i = 0; i < 16; i++) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    if (i < 15)
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    else
      EXPECT_EQ(2, cpdf_page_object->GetContentStream()) << i;
  }

  // Generate contents should remove the empty stream and update the page
  // objects' contents stream indexes.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // Content stream 0: page objects 0-14.
  // Content stream 1: page object 15.
  ASSERT_EQ(16, FPDFPage_CountObjects(page));
  for (int i = 0; i < 16; i++) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    if (i < 15)
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    else
      EXPECT_EQ(1, cpdf_page_object->GetContentStream()) << i;
  }

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kStream1RemovedChecksum[] = "7fe07f182b37d40afc6ae36a4e89fe73";
#elif BUILDFLAG(IS_APPLE)
  const char kStream1RemovedChecksum[] = "3cdc75af44c15bed80998facd6e674c9";
#else
  const char kStream1RemovedChecksum[] = "b474826df1acedb05c7b82e1e49e64a6";
#endif
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kStream1RemovedChecksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the page object count is still 16, and that
  // content stream 1 was removed.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  // Content stream 0: page objects 0-14.
  // Content stream 1: page object 15.
  EXPECT_EQ(16, FPDFPage_CountObjects(saved_page));
  for (int i = 0; i < 16; i++) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(saved_page, i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    if (i < 15)
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    else
      EXPECT_EQ(1, cpdf_page_object->GetContentStream()) << i;
  }

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kStream1RemovedChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveAllFromSingleStream) {
  // Load document with a single stream.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));

  // Loop backwards because objects will being removed, which shifts the indexes
  // after the removed position.
  for (int i = 1; i >= 0; i--) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    ASSERT_EQ(0, cpdf_page_object->GetContentStream());
    ASSERT_TRUE(FPDFPage_RemoveObject(page, page_object));
    FPDFPageObj_Destroy(page_object);
  }

  // No more objects in the stream
  ASSERT_EQ(0, FPDFPage_CountObjects(page));

  // Generate contents should remove the empty stream and update the page
  // objects' contents stream indexes.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  ASSERT_EQ(0, FPDFPage_CountObjects(page));

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kAllRemovedChecksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the page object count is still 0.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(0, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kAllRemovedChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveFirstFromSingleStream) {
  // Load document with a single stream.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));

  // Remove first object.
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  CPDF_PageObject* cpdf_page_object =
      CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());
  ASSERT_TRUE(FPDFPage_RemoveObject(page, page_object));
  FPDFPageObj_Destroy(page_object);

  // One object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page));
  page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // Still one object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page));
  page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kFirstRemovedChecksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the page object count is still 0.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  ASSERT_EQ(1, FPDFPage_CountObjects(saved_page));
  page_object = FPDFPage_GetObject(saved_page, 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kFirstRemovedChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveLastFromSingleStream) {
  // Load document with a single stream.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));

  // Remove last object
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 1);
  ASSERT_TRUE(page_object);
  CPDF_PageObject* cpdf_page_object =
      CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());
  ASSERT_TRUE(FPDFPage_RemoveObject(page, page_object));
  FPDFPageObj_Destroy(page_object);

  // One object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page));
  page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // Still one object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page));
  page_object = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  using pdfium::kHelloWorldRemovedChecksum;
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldRemovedChecksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the page object count is still 0.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  ASSERT_EQ(1, FPDFPage_CountObjects(saved_page));
  page_object = FPDFPage_GetObject(saved_page, 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldRemovedChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveAllFromMultipleStreams) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_split_streams.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  // Content stream 1: page object 2.
  ASSERT_EQ(3, FPDFPage_CountObjects(page));

  // Loop backwards because objects will being removed, which shifts the indexes
  // after the removed position.
  for (int i = 2; i >= 0; i--) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, i);
    ASSERT_TRUE(page_object);
    ASSERT_TRUE(FPDFPage_RemoveObject(page, page_object));
    FPDFPageObj_Destroy(page_object);
  }

  // No more objects in the page.
  ASSERT_EQ(0, FPDFPage_CountObjects(page));

  // Generate contents should remove the empty streams and update the page
  // objects' contents stream indexes.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  ASSERT_EQ(0, FPDFPage_CountObjects(page));

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kAllRemovedChecksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the page object count is still 0.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(0, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kAllRemovedChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, InsertPageObjectAndSave) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Add a red rectangle.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect);

  // Verify the red rectangle was added.
  ASSERT_EQ(3, FPDFPage_CountObjects(page));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the page object count is still 3.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(3, FPDFPage_CountObjects(saved_page));
  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, InsertPageObjectEditAndSave) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Add a red rectangle.
  ASSERT_EQ(2, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 100, 100, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect);

  // Verify the red rectangle was added.
  ASSERT_EQ(3, FPDFPage_CountObjects(page));

  // Generate content but change it again
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the page object count is still 3.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(3, FPDFPage_CountObjects(saved_page));
  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, InsertAndRemoveLargeFile) {
  const int kOriginalObjectCount = 600;

  // Load document with many objects.
  ASSERT_TRUE(OpenDocument("many_rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  using pdfium::kManyRectanglesChecksum;
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 300, kManyRectanglesChecksum);
  }

  // Add a black rectangle.
  ASSERT_EQ(kOriginalObjectCount, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT black_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(black_rect, 0, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(black_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, black_rect);

  // Verify the black rectangle was added.
  ASSERT_EQ(kOriginalObjectCount + 1, FPDFPage_CountObjects(page));
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kPlusRectangleMD5[] = "0d3715fcfb9bd0dd25dcce60800bff47";
#else
  const char kPlusRectangleMD5[] = "6b9396ab570754b32b04ca629e902f77";
#endif
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 300, kPlusRectangleMD5);
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the rectangle added is still there.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(kOriginalObjectCount + 1, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 300, kPlusRectangleMD5);
  }

  // Remove the added rectangle.
  FPDF_PAGEOBJECT added_object =
      FPDFPage_GetObject(saved_page, kOriginalObjectCount);
  EXPECT_TRUE(FPDFPage_RemoveObject(saved_page, added_object));
  FPDFPageObj_Destroy(added_object);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 300, kManyRectanglesChecksum);
  }
  EXPECT_EQ(kOriginalObjectCount, FPDFPage_CountObjects(saved_page));

  // Save the file again.
  EXPECT_TRUE(FPDFPage_GenerateContent(saved_page));
  EXPECT_TRUE(FPDF_SaveAsCopy(saved_document(), this, 0));

  CloseSavedPage(saved_page);
  CloseSavedDocument();

  // Re-open the file (again) and check the black rectangle was removed and the
  // rest is intact.
  ASSERT_TRUE(OpenSavedDocument());
  saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(kOriginalObjectCount, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 300, kManyRectanglesChecksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, AddAndRemovePaths) {
  // Start with a blank page.
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);
  ASSERT_TRUE(page);

  // Render the blank page and verify it's a blank bitmap.
  using pdfium::kBlankPage612By792Checksum;
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kBlankPage612By792Checksum);
  }
  ASSERT_EQ(0, FPDFPage_CountObjects(page));

  // Add a red rectangle.
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(10, 10, 20, 20);
  ASSERT_TRUE(red_rect);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kRedRectangleChecksum);
  }
  EXPECT_EQ(1, FPDFPage_CountObjects(page));

  // Remove rectangle and verify it does not render anymore and the bitmap is
  // back to a blank one.
  EXPECT_TRUE(FPDFPage_RemoveObject(page, red_rect));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kBlankPage612By792Checksum);
  }
  EXPECT_EQ(0, FPDFPage_CountObjects(page));

  // Trying to remove an object not in the page should return false.
  EXPECT_FALSE(FPDFPage_RemoveObject(page, red_rect));

  FPDF_ClosePage(page);
  FPDFPageObj_Destroy(red_rect);
}

TEST_F(FPDFEditEmbedderTest, PathsPoints) {
  CreateNewDocument();
  FPDF_PAGEOBJECT img = FPDFPageObj_NewImageObj(document());
  // This should fail gracefully, even if img is not a path.
  ASSERT_EQ(-1, FPDFPath_CountSegments(img));

  // This should fail gracefully, even if path is NULL.
  ASSERT_EQ(-1, FPDFPath_CountSegments(nullptr));

  // FPDFPath_GetPathSegment() with a non-path.
  ASSERT_EQ(nullptr, FPDFPath_GetPathSegment(img, 0));
  // FPDFPath_GetPathSegment() with a NULL path.
  ASSERT_EQ(nullptr, FPDFPath_GetPathSegment(nullptr, 0));
  float x;
  float y;
  // FPDFPathSegment_GetPoint() with a NULL segment.
  EXPECT_FALSE(FPDFPathSegment_GetPoint(nullptr, &x, &y));

  // FPDFPathSegment_GetType() with a NULL segment.
  ASSERT_EQ(FPDF_SEGMENT_UNKNOWN, FPDFPathSegment_GetType(nullptr));

  // FPDFPathSegment_GetClose() with a NULL segment.
  EXPECT_FALSE(FPDFPathSegment_GetClose(nullptr));

  FPDFPageObj_Destroy(img);
}

TEST_F(FPDFEditEmbedderTest, PathOnTopOfText) {
  // Load document with some text
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Add an opaque rectangle on top of some of the text.
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect);

  // Add a transparent triangle on top of other part of the text.
  FPDF_PAGEOBJECT black_path = FPDFPageObj_CreateNewPath(20, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(black_path, 0, 0, 0, 100));
  EXPECT_TRUE(FPDFPath_SetDrawMode(black_path, FPDF_FILLMODE_ALTERNATE, 0));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 30, 80));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 40, 10));
  EXPECT_TRUE(FPDFPath_Close(black_path));
  FPDFPage_InsertObject(page, black_path);

  // Render and check the result.
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kChecksum[] = "3490f699d894351a554d79e1fcdf7981";
#elif BUILDFLAG(IS_APPLE)
  const char kChecksum[] = "279693baca9f48da2d75a8e289aed58e";
#else
  const char kChecksum[] = "fe415d47945c10b9cc8e9ca08887369e";
#endif
  CompareBitmap(bitmap.get(), 200, 200, kChecksum);
  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, EditOverExistingContent) {
  // Load document with existing content
  ASSERT_TRUE(OpenDocument("bug_717.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Add a transparent rectangle on top of the existing content
  FPDF_PAGEOBJECT red_rect2 = FPDFPageObj_CreateNewRect(90, 700, 25, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect2, 255, 0, 0, 100));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect2, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect2);

  // Add an opaque rectangle on top of the existing content
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(115, 700, 25, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page, red_rect);

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kOriginalChecksum[] = "1e82fbdd21490cee9d3479fe6125af67";
#else
  const char kOriginalChecksum[] = "ad04e5bd0f471a9a564fb034bd0fb073";
#endif
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  CompareBitmap(bitmap.get(), 612, 792, kOriginalChecksum);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // Now save the result, closing the page and document
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  VerifySavedRendering(saved_page, 612, 792, kOriginalChecksum);

  ClearString();
  // Add another opaque rectangle on top of the existing content
  FPDF_PAGEOBJECT green_rect = FPDFPageObj_CreateNewRect(150, 700, 25, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(green_rect, 0, 255, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(green_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(saved_page, green_rect);

  // Add another transparent rectangle on top of existing content
  FPDF_PAGEOBJECT green_rect2 = FPDFPageObj_CreateNewRect(175, 700, 25, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(green_rect2, 0, 255, 0, 100));
  EXPECT_TRUE(FPDFPath_SetDrawMode(green_rect2, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(saved_page, green_rect2);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kLastChecksum[] = "8705d023e5fec3499d1e30cf2bcc5dc1";
#else
  const char kLastChecksum[] = "4b5b00f824620f8c9b8801ebb98e1cdd";
#endif
  {
    ScopedFPDFBitmap new_bitmap = RenderSavedPage(saved_page);
    CompareBitmap(new_bitmap.get(), 612, 792, kLastChecksum);
  }
  EXPECT_TRUE(FPDFPage_GenerateContent(saved_page));

  // Now save the result, closing the page and document
  EXPECT_TRUE(FPDF_SaveAsCopy(saved_document(), this, 0));

  CloseSavedPage(saved_page);
  CloseSavedDocument();

  // Render the saved result
  VerifySavedDocument(612, 792, kLastChecksum);
}

// TODO(crbug.com/pdfium/1651): Fix this issue and enable the test for Skia.
#if defined(_SKIA_SUPPORT_)
#define MAYBE_AddStrokedPaths DISABLED_AddStrokedPaths
#else
#define MAYBE_AddStrokedPaths AddStrokedPaths
#endif
TEST_F(FPDFEditEmbedderTest, MAYBE_AddStrokedPaths) {
  // Start with a blank page
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);

  // Add a large stroked rectangle (fill color should not affect it).
  FPDF_PAGEOBJECT rect = FPDFPageObj_CreateNewRect(20, 20, 200, 400);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPageObj_SetStrokeColor(rect, 0, 255, 0, 255));
  EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(rect, 15.0f));

  float width = 0;
  EXPECT_TRUE(FPDFPageObj_GetStrokeWidth(rect, &width));
  EXPECT_EQ(15.0f, width);

  EXPECT_TRUE(FPDFPath_SetDrawMode(rect, 0, 1));
  FPDFPage_InsertObject(page, rect);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static constexpr char kChecksum_1[] = "1469acf60e7647ebeb8e1fb08c5d6c7a";
#else
    static constexpr char kChecksum_1[] = "64bd31f862a89e0a9e505a5af6efd506";
#endif
    CompareBitmap(page_bitmap.get(), 612, 792, kChecksum_1);
  }

  // Add crossed-checkmark
  FPDF_PAGEOBJECT check = FPDFPageObj_CreateNewPath(300, 500);
  EXPECT_TRUE(FPDFPath_LineTo(check, 400, 400));
  EXPECT_TRUE(FPDFPath_LineTo(check, 600, 600));
  EXPECT_TRUE(FPDFPath_MoveTo(check, 400, 600));
  EXPECT_TRUE(FPDFPath_LineTo(check, 600, 400));
  EXPECT_TRUE(FPDFPageObj_SetStrokeColor(check, 128, 128, 128, 180));
  EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(check, 8.35f));
  EXPECT_TRUE(FPDFPath_SetDrawMode(check, 0, 1));
  FPDFPage_InsertObject(page, check);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static constexpr char kChecksum_2[] = "68b3194f74abd9d471695ce1415be43f";
#else
    static constexpr char kChecksum_2[] = "4b6f3b9d25c4e194821217d5016c3724";
#endif
    CompareBitmap(page_bitmap.get(), 612, 792, kChecksum_2);
  }

  // Add stroked and filled oval-ish path.
  FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(250, 100);
  EXPECT_TRUE(FPDFPath_BezierTo(path, 180, 166, 180, 233, 250, 300));
  EXPECT_TRUE(FPDFPath_LineTo(path, 255, 305));
  EXPECT_TRUE(FPDFPath_BezierTo(path, 325, 233, 325, 166, 255, 105));
  EXPECT_TRUE(FPDFPath_Close(path));
  EXPECT_TRUE(FPDFPageObj_SetFillColor(path, 200, 128, 128, 100));
  EXPECT_TRUE(FPDFPageObj_SetStrokeColor(path, 128, 200, 128, 150));
  EXPECT_TRUE(FPDFPageObj_SetStrokeWidth(path, 10.5f));
  EXPECT_TRUE(FPDFPath_SetDrawMode(path, FPDF_FILLMODE_ALTERNATE, 1));
  FPDFPage_InsertObject(page, path);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static constexpr char kChecksum_3[] = "ea784068651df2b9ba132ce9215e6780";
#else
    static constexpr char kChecksum_3[] = "ff3e6a22326754944cc6e56609acd73b";
#endif
    CompareBitmap(page_bitmap.get(), 612, 792, kChecksum_3);
  }
  FPDF_ClosePage(page);
}

// Tests adding text from standard font using FPDFPageObj_NewTextObj.
TEST_F(FPDFEditEmbedderTest, AddStandardFontText) {
  // Start with a blank page
  ScopedFPDFPage page(FPDFPage_New(CreateNewDocument(), 0, 612, 792));

  // Add some text to the page
  FPDF_PAGEOBJECT text_object1 =
      FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
  EXPECT_TRUE(text_object1);
  ScopedFPDFWideString text1 = GetFPDFWideString(kBottomText);
  EXPECT_TRUE(FPDFText_SetText(text_object1, text1.get()));
  static constexpr FS_MATRIX kMatrix1{1, 0, 0, 1, 20, 20};
  EXPECT_TRUE(FPDFPageObj_SetMatrix(text_object1, &kMatrix1));
  FPDFPage_InsertObject(page.get(), text_object1);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 612, 792, kBottomTextChecksum);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    VerifySavedDocument(612, 792, kBottomTextChecksum);
  }

  // Try another font
  FPDF_PAGEOBJECT text_object2 =
      FPDFPageObj_NewTextObj(document(), "TimesNewRomanBold", 15.0f);
  EXPECT_TRUE(text_object2);
  ScopedFPDFWideString text2 =
      GetFPDFWideString(L"Hi, I'm Bold. Times New Roman Bold.");
  EXPECT_TRUE(FPDFText_SetText(text_object2, text2.get()));
  FPDFPageObj_Transform(text_object2, 1, 0, 0, 1, 100, 600);
  FPDFPage_InsertObject(page.get(), text_object2);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static constexpr char md5[] = "fa64eb3808b541342496281277fad5f2";
#else
#if BUILDFLAG(IS_APPLE)
    static constexpr char md5[] = "983baaa1f688eff7a14b1bf91c171a1a";
#else
    static constexpr char md5[] = "161523e196eb5341604cd73e12c97922";
#endif
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    CompareBitmap(page_bitmap.get(), 612, 792, md5);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    VerifySavedDocument(612, 792, md5);
  }

  // And some randomly transformed text
  FPDF_PAGEOBJECT text_object3 =
      FPDFPageObj_NewTextObj(document(), "Courier-Bold", 20.0f);
  EXPECT_TRUE(text_object3);
  ScopedFPDFWideString text3 = GetFPDFWideString(L"Can you read me? <:)>");
  EXPECT_TRUE(FPDFText_SetText(text_object3, text3.get()));
  FPDFPageObj_Transform(text_object3, 1, 1.5, 2, 0.5, 200, 200);
  FPDFPage_InsertObject(page.get(), text_object3);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
    static constexpr char md5[] = "abc65660389911aab95550ca8cd97a2b";
#elif BUILDFLAG(IS_APPLE)
    static constexpr char md5[] = "e0b3493c5c16e41d0d892ffb48e63fba";
#else
    static constexpr char md5[] = "1fbf772dca8d82b960631e6683934964";
#endif
    CompareBitmap(page_bitmap.get(), 612, 792, md5);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    VerifySavedDocument(612, 792, md5);
  }

  FS_MATRIX matrix;
  EXPECT_TRUE(FPDFPageObj_GetMatrix(text_object3, &matrix));
  EXPECT_FLOAT_EQ(1.0f, matrix.a);
  EXPECT_FLOAT_EQ(1.5f, matrix.b);
  EXPECT_FLOAT_EQ(2.0f, matrix.c);
  EXPECT_FLOAT_EQ(0.5f, matrix.d);
  EXPECT_FLOAT_EQ(200.0f, matrix.e);
  EXPECT_FLOAT_EQ(200.0f, matrix.f);

  EXPECT_FALSE(FPDFTextObj_GetFontSize(nullptr, nullptr));
  float size = 55;
  EXPECT_FALSE(FPDFTextObj_GetFontSize(nullptr, &size));
  EXPECT_EQ(55, size);
  EXPECT_TRUE(FPDFTextObj_GetFontSize(text_object3, &size));
  EXPECT_EQ(20, size);

  // TODO(npm): Why are there issues with text rotated by 90 degrees?
  // TODO(npm): FPDF_SaveAsCopy not giving the desired result after this.
}

TEST_F(FPDFEditEmbedderTest, AddStandardFontTextOfSizeZero) {
  // Start with a blank page
  ScopedFPDFPage page(FPDFPage_New(CreateNewDocument(), 0, 612, 792));

  // Add some text of size 0 to the page.
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_NewTextObj(document(), "Arial", 0.0f);
  EXPECT_TRUE(text_object);
  ScopedFPDFWideString text = GetFPDFWideString(kBottomText);
  EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 20, 20);

  float size = -1;  // Make sure 'size' gets changed.
  EXPECT_TRUE(FPDFTextObj_GetFontSize(text_object, &size));
  EXPECT_EQ(0.0f, size);

  FPDFPage_InsertObject(page.get(), text_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 612, 792,
                  pdfium::kBlankPage612By792Checksum);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    VerifySavedDocument(612, 792, pdfium::kBlankPage612By792Checksum);
  }
}

TEST_F(FPDFEditEmbedderTest, GetTextRenderMode) {
  ASSERT_TRUE(OpenDocument("text_render_mode.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(2, FPDFPage_CountObjects(page));

  EXPECT_EQ(FPDF_TEXTRENDERMODE_UNKNOWN,
            FPDFTextObj_GetTextRenderMode(nullptr));

  FPDF_PAGEOBJECT fill = FPDFPage_GetObject(page, 0);
  EXPECT_EQ(FPDF_TEXTRENDERMODE_FILL, FPDFTextObj_GetTextRenderMode(fill));

  FPDF_PAGEOBJECT stroke = FPDFPage_GetObject(page, 1);
  EXPECT_EQ(FPDF_TEXTRENDERMODE_STROKE, FPDFTextObj_GetTextRenderMode(stroke));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, SetTextRenderMode) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kOriginalChecksum[] = "bf87e8b36380ebd96ca429213fa23a09";
  const char kStrokeChecksum[] = "d16eb1bb4748eeb5fb801594da70d519";
#elif BUILDFLAG(IS_APPLE)
  const char kOriginalChecksum[] = "c488514ce0fc949069ff560407edacd2";
  const char kStrokeChecksum[] = "e06ee84aeebe926e8c980b7822027e8a";
#else
  const char kOriginalChecksum[] = "97a4fcf3c9581e19917895631af31d41";
  const char kStrokeChecksum[] = "e06ee84aeebe926e8c980b7822027e8a";
#endif

  {
    ASSERT_TRUE(OpenDocument("text_render_mode.pdf"));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);
    ASSERT_EQ(2, FPDFPage_CountObjects(page));

    // Check the bitmap
    {
      ScopedFPDFBitmap page_bitmap = RenderPage(page);
      CompareBitmap(page_bitmap.get(), 612, 446, kOriginalChecksum);
    }

    // Cannot set on a null object.
    EXPECT_FALSE(
        FPDFTextObj_SetTextRenderMode(nullptr, FPDF_TEXTRENDERMODE_UNKNOWN));
    EXPECT_FALSE(
        FPDFTextObj_SetTextRenderMode(nullptr, FPDF_TEXTRENDERMODE_INVISIBLE));

    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
    ASSERT_TRUE(page_object);
    EXPECT_EQ(FPDF_TEXTRENDERMODE_FILL,
              FPDFTextObj_GetTextRenderMode(page_object));

    // Cannot set UNKNOWN as a render mode.
    EXPECT_FALSE(FPDFTextObj_SetTextRenderMode(page_object,
                                               FPDF_TEXTRENDERMODE_UNKNOWN));

    EXPECT_TRUE(
        FPDFTextObj_SetTextRenderMode(page_object, FPDF_TEXTRENDERMODE_STROKE));
    EXPECT_EQ(FPDF_TEXTRENDERMODE_STROKE,
              FPDFTextObj_GetTextRenderMode(page_object));

    // Check that bitmap displays changed content
    {
      ScopedFPDFBitmap page_bitmap = RenderPage(page);
      CompareBitmap(page_bitmap.get(), 612, 446, kStrokeChecksum);
    }

    // Save a copy.
    EXPECT_TRUE(FPDFPage_GenerateContent(page));
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

    UnloadPage(page);
  }

  {
    // Open the saved copy and render it. Check that the changed text render
    // mode is kept in the saved copy.
    ASSERT_TRUE(OpenSavedDocument());
    FPDF_PAGE saved_page = LoadSavedPage(0);
    ASSERT_TRUE(saved_page);

    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(saved_page, 0);
    EXPECT_TRUE(page_object);
    EXPECT_EQ(FPDF_TEXTRENDERMODE_STROKE,
              FPDFTextObj_GetTextRenderMode(page_object));

    ScopedFPDFBitmap bitmap = RenderSavedPage(saved_page);
    CompareBitmap(bitmap.get(), 612, 446, kStrokeChecksum);

    CloseSavedPage(saved_page);
    CloseSavedDocument();
  }
}

TEST_F(FPDFEditEmbedderTest, TextFontProperties) {
  // bad object tests
  EXPECT_FALSE(FPDFTextObj_GetFont(nullptr));
  EXPECT_EQ(0U, FPDFFont_GetFontName(nullptr, nullptr, 5));
  EXPECT_EQ(-1, FPDFFont_GetFlags(nullptr));
  EXPECT_EQ(-1, FPDFFont_GetWeight(nullptr));
  EXPECT_FALSE(FPDFFont_GetItalicAngle(nullptr, nullptr));
  EXPECT_FALSE(FPDFFont_GetAscent(nullptr, 12.f, nullptr));
  EXPECT_FALSE(FPDFFont_GetDescent(nullptr, 12.f, nullptr));
  EXPECT_FALSE(FPDFFont_GetGlyphWidth(nullptr, 's', 12.f, nullptr));
  EXPECT_FALSE(FPDFFont_GetGlyphPath(nullptr, 's', 12.f));

  // good object tests
  ASSERT_TRUE(OpenDocument("text_font.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT text = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(text);
  float font_size;
  ASSERT_TRUE(FPDFTextObj_GetFontSize(text, &font_size));
  FPDF_FONT font = FPDFTextObj_GetFont(text);
  ASSERT_TRUE(font);

  // null return pointer tests
  EXPECT_FALSE(FPDFFont_GetItalicAngle(font, nullptr));
  EXPECT_FALSE(FPDFFont_GetAscent(font, font_size, nullptr));
  EXPECT_FALSE(FPDFFont_GetDescent(font, font_size, nullptr));
  EXPECT_FALSE(FPDFFont_GetGlyphWidth(font, 's', font_size, nullptr));

  // correct property tests
  {
    EXPECT_EQ(4, FPDFFont_GetFlags(font));
    EXPECT_EQ(400, FPDFFont_GetWeight(font));

    int angle;
    EXPECT_TRUE(FPDFFont_GetItalicAngle(font, &angle));
    EXPECT_EQ(0, angle);

    float ascent;
    EXPECT_TRUE(FPDFFont_GetAscent(font, font_size, &ascent));
    EXPECT_FLOAT_EQ(891 * font_size / 1000.0f, ascent);

    float descent;
    EXPECT_TRUE(FPDFFont_GetDescent(font, font_size, &descent));
    EXPECT_FLOAT_EQ(-216 * font_size / 1000.0f, descent);

    float a12;
    float a24;
    EXPECT_TRUE(FPDFFont_GetGlyphWidth(font, 'a', 12.0f, &a12));
    EXPECT_FLOAT_EQ(a12, 5.316f);
    EXPECT_TRUE(FPDFFont_GetGlyphWidth(font, 'a', 24.0f, &a24));
    EXPECT_FLOAT_EQ(a24, 10.632f);
  }

  {
    // FPDFFont_GetFontName() positive testing.
    unsigned long size = FPDFFont_GetFontName(font, nullptr, 0);
    const char kExpectedFontName[] = "Liberation Serif";
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    std::vector<char> font_name(size);
    ASSERT_EQ(size, FPDFFont_GetFontName(font, font_name.data(), size));
    ASSERT_STREQ(kExpectedFontName, font_name.data());

    // FPDFFont_GetFontName() negative testing.
    ASSERT_EQ(0U, FPDFFont_GetFontName(nullptr, nullptr, 0));

    font_name.resize(2);
    font_name[0] = 'x';
    font_name[1] = '\0';
    size = FPDFFont_GetFontName(font, font_name.data(), font_name.size());
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    ASSERT_STREQ("x", font_name.data());
  }

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GlyphPaths) {
  // bad glyphpath
  EXPECT_EQ(-1, FPDFGlyphPath_CountGlyphSegments(nullptr));
  EXPECT_FALSE(FPDFGlyphPath_GetGlyphPathSegment(nullptr, 1));

  ASSERT_TRUE(OpenDocument("text_font.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page));
  FPDF_PAGEOBJECT text = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(text);
  FPDF_FONT font = FPDFTextObj_GetFont(text);
  ASSERT_TRUE(font);

  // good glyphpath
  FPDF_GLYPHPATH gpath = FPDFFont_GetGlyphPath(font, 's', 12.0f);
  ASSERT_TRUE(gpath);

  int count = FPDFGlyphPath_CountGlyphSegments(gpath);
  ASSERT_GT(count, 0);
  EXPECT_FALSE(FPDFGlyphPath_GetGlyphPathSegment(gpath, -1));
  EXPECT_FALSE(FPDFGlyphPath_GetGlyphPathSegment(gpath, count));

  FPDF_PATHSEGMENT segment = FPDFGlyphPath_GetGlyphPathSegment(gpath, 1);
  ASSERT_TRUE(segment);
  EXPECT_EQ(FPDF_SEGMENT_BEZIERTO, FPDFPathSegment_GetType(segment));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, FormGetObjects) {
  ASSERT_TRUE(OpenDocument("form_object.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT form = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(form));
  ASSERT_EQ(-1, FPDFFormObj_CountObjects(nullptr));
  ASSERT_EQ(2, FPDFFormObj_CountObjects(form));

  // FPDFFormObj_GetObject() positive testing.
  FPDF_PAGEOBJECT text1 = FPDFFormObj_GetObject(form, 0);
  ASSERT_TRUE(text1);
  float left = 0;
  float bottom = 0;
  float right = 0;
  float top = 0;
  ASSERT_TRUE(FPDFPageObj_GetBounds(text1, &left, &bottom, &right, &top));
  ASSERT_EQ(271, static_cast<int>(top));

  FPDF_PAGEOBJECT text2 = FPDFFormObj_GetObject(form, 1);
  ASSERT_TRUE(text2);
  ASSERT_TRUE(FPDFPageObj_GetBounds(text2, &left, &bottom, &right, &top));
  ASSERT_EQ(221, static_cast<int>(top));

  // FPDFFormObj_GetObject() negative testing.
  ASSERT_EQ(nullptr, FPDFFormObj_GetObject(nullptr, 0));
  ASSERT_EQ(nullptr, FPDFFormObj_GetObject(form, -1));
  ASSERT_EQ(nullptr, FPDFFormObj_GetObject(form, 2));

  // FPDFPageObj_GetMatrix() positive testing for forms.
  static constexpr FS_MATRIX kMatrix = {1.0f, 1.5f, 2.0f, 2.5f, 100.0f, 200.0f};
  EXPECT_TRUE(FPDFPageObj_SetMatrix(form, &kMatrix));

  FS_MATRIX matrix;
  EXPECT_TRUE(FPDFPageObj_GetMatrix(form, &matrix));
  EXPECT_FLOAT_EQ(kMatrix.a, matrix.a);
  EXPECT_FLOAT_EQ(kMatrix.b, matrix.b);
  EXPECT_FLOAT_EQ(kMatrix.c, matrix.c);
  EXPECT_FLOAT_EQ(kMatrix.d, matrix.d);
  EXPECT_FLOAT_EQ(kMatrix.e, matrix.e);
  EXPECT_FLOAT_EQ(kMatrix.f, matrix.f);

  // FPDFPageObj_GetMatrix() negative testing for forms.
  EXPECT_FALSE(FPDFPageObj_GetMatrix(form, nullptr));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, ModifyFormObject) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kOrigChecksum[] = "e15086e54078e4d22fa3fb12105c579e";
  const char kNewChecksum[] = "7282fe98693c0a7ad2c1b3f3f9563977";
#elif BUILDFLAG(IS_APPLE)
  const char kOrigChecksum[] = "a637057185f50aac1aa5490f726aef95";
  const char kNewChecksum[] = "8ad9d79b02b609ff734e2a2195c96e2d";
#else
  const char kOrigChecksum[] = "34a9ec0a9581a7970e073c0bcc4ca676";
  const char kNewChecksum[] = "609b5632a21c886fa93182dbc290bf7a";
#endif

  ASSERT_TRUE(OpenDocument("form_object.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
    CompareBitmap(bitmap.get(), 62, 69, kOrigChecksum);
  }

  FPDF_PAGEOBJECT form = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(form));

  FPDFPageObj_Transform(form, 0.5, 0, 0, 0.5, 0, 0);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
    CompareBitmap(bitmap.get(), 62, 69, kNewChecksum);
  }

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(62, 69, kNewChecksum);

  UnloadPage(page);
}

// Tests adding text from standard font using FPDFText_LoadStandardFont.
TEST_F(FPDFEditEmbedderTest, AddStandardFontText2) {
  // Start with a blank page
  ScopedFPDFPage page(FPDFPage_New(CreateNewDocument(), 0, 612, 792));

  // Load a standard font.
  ScopedFPDFFont font(FPDFText_LoadStandardFont(document(), "Helvetica"));
  ASSERT_TRUE(font);

  // Add some text to the page.
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_CreateTextObj(document(), font.get(), 12.0f);
  EXPECT_TRUE(text_object);
  ScopedFPDFWideString text = GetFPDFWideString(kBottomText);
  EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 20, 20);
  FPDFPage_InsertObject(page.get(), text_object);
  ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
  CompareBitmap(page_bitmap.get(), 612, 792, kBottomTextChecksum);
}

TEST_F(FPDFEditEmbedderTest, LoadStandardFonts) {
  CreateNewDocument();
  static constexpr const char* kStandardFontNames[] = {
      "Arial",
      "Arial-Bold",
      "Arial-BoldItalic",
      "Arial-Italic",
      "Courier",
      "Courier-BoldOblique",
      "Courier-Oblique",
      "Courier-Bold",
      "CourierNew",
      "CourierNew-Bold",
      "CourierNew-BoldItalic",
      "CourierNew-Italic",
      "Helvetica",
      "Helvetica-Bold",
      "Helvetica-BoldOblique",
      "Helvetica-Oblique",
      "Symbol",
      "TimesNewRoman",
      "TimesNewRoman-Bold",
      "TimesNewRoman-BoldItalic",
      "TimesNewRoman-Italic",
      "ZapfDingbats"};
  for (const char* font_name : kStandardFontNames) {
    ScopedFPDFFont font(FPDFText_LoadStandardFont(document(), font_name));
    EXPECT_TRUE(font) << font_name << " should be considered a standard font.";
  }
  static constexpr const char* kNotStandardFontNames[] = {
      "Abcdefg",      "ArialB",    "Arial-Style",
      "Font Name",    "FontArial", "NotAStandardFontName",
      "TestFontName", "Quack",     "Symbol-Italic",
      "Zapf"};
  for (const char* font_name : kNotStandardFontNames) {
    ScopedFPDFFont font(FPDFText_LoadStandardFont(document(), font_name));
    EXPECT_FALSE(font) << font_name
                       << " should not be considered a standard font.";
  }
}

TEST_F(FPDFEditEmbedderTest, GraphicsData) {
  // New page
  ScopedFPDFPage page(FPDFPage_New(CreateNewDocument(), 0, 612, 792));

  // Create a rect with nontrivial graphics
  FPDF_PAGEOBJECT rect1 = FPDFPageObj_CreateNewRect(10, 10, 100, 100);
  FPDFPageObj_SetBlendMode(rect1, "Color");
  FPDFPage_InsertObject(page.get(), rect1);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  // Check that the ExtGState was created
  CPDF_Page* cpage = CPDFPageFromFPDFPage(page.get());
  const CPDF_Dictionary* graphics_dict =
      cpage->GetResources()->GetDictFor("ExtGState");
  ASSERT_TRUE(graphics_dict);
  EXPECT_EQ(2u, graphics_dict->size());

  // Add a text object causing no change to the graphics dictionary
  FPDF_PAGEOBJECT text1 = FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
  // Only alpha, the last component, matters for the graphics dictionary. And
  // the default value is 255.
  EXPECT_TRUE(FPDFPageObj_SetFillColor(text1, 100, 100, 100, 255));
  FPDFPage_InsertObject(page.get(), text1);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_EQ(2u, graphics_dict->size());

  // Add a text object increasing the size of the graphics dictionary
  FPDF_PAGEOBJECT text2 =
      FPDFPageObj_NewTextObj(document(), "Times-Roman", 12.0f);
  FPDFPage_InsertObject(page.get(), text2);
  FPDFPageObj_SetBlendMode(text2, "Darken");
  EXPECT_TRUE(FPDFPageObj_SetFillColor(text2, 0, 0, 255, 150));
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_EQ(3u, graphics_dict->size());

  // Add a path that should reuse graphics
  FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(400, 100);
  FPDFPageObj_SetBlendMode(path, "Darken");
  EXPECT_TRUE(FPDFPageObj_SetFillColor(path, 200, 200, 100, 150));
  FPDFPage_InsertObject(page.get(), path);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_EQ(3u, graphics_dict->size());

  // Add a rect increasing the size of the graphics dictionary
  FPDF_PAGEOBJECT rect2 = FPDFPageObj_CreateNewRect(10, 10, 100, 100);
  FPDFPageObj_SetBlendMode(rect2, "Darken");
  EXPECT_TRUE(FPDFPageObj_SetFillColor(rect2, 0, 0, 255, 150));
  EXPECT_TRUE(FPDFPageObj_SetStrokeColor(rect2, 0, 0, 0, 200));
  FPDFPage_InsertObject(page.get(), rect2);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_EQ(4u, graphics_dict->size());
}

TEST_F(FPDFEditEmbedderTest, DoubleGenerating) {
  // Start with a blank page
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);

  // Add a red rectangle with some non-default alpha
  FPDF_PAGEOBJECT rect = FPDFPageObj_CreateNewRect(10, 10, 100, 100);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(rect, 255, 0, 0, 128));
  EXPECT_TRUE(FPDFPath_SetDrawMode(rect, FPDF_FILLMODE_WINDING, 0));
  FPDFPage_InsertObject(page, rect);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // Check the ExtGState
  CPDF_Page* cpage = CPDFPageFromFPDFPage(page);
  const CPDF_Dictionary* graphics_dict =
      cpage->GetResources()->GetDictFor("ExtGState");
  ASSERT_TRUE(graphics_dict);
  EXPECT_EQ(2u, graphics_dict->size());

  // Check the bitmap
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "5384da3406d62360ffb5cac4476fff1c");
  }

  // Never mind, my new favorite color is blue, increase alpha
  EXPECT_TRUE(FPDFPageObj_SetFillColor(rect, 0, 0, 255, 180));
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_EQ(3u, graphics_dict->size());

  // Check that bitmap displays changed content
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "2e51656f5073b0bee611d9cd086aa09c");
  }

  // And now generate, without changes
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_EQ(3u, graphics_dict->size());
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "2e51656f5073b0bee611d9cd086aa09c");
  }

  // Add some text to the page
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
  ScopedFPDFWideString text =
      GetFPDFWideString(L"Something something #text# something");
  EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 300, 300);
  FPDFPage_InsertObject(page, text_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  const CPDF_Dictionary* font_dict = cpage->GetResources()->GetDictFor("Font");
  ASSERT_TRUE(font_dict);
  EXPECT_EQ(1u, font_dict->size());

  // Generate yet again, check dicts are reasonably sized
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_EQ(3u, graphics_dict->size());
  EXPECT_EQ(1u, font_dict->size());
  FPDF_ClosePage(page);
}

TEST_F(FPDFEditEmbedderTest, LoadSimpleType1Font) {
  CreateNewDocument();
  // TODO(npm): use other fonts after disallowing loading any font as any type
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Times-Bold");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TYPE1, false));
  ASSERT_TRUE(font.get());
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsType1Font());

  const CPDF_Dictionary* font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("Type1", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Tinos-Bold", font_dict->GetNameFor("BaseFont"));
  ASSERT_TRUE(font_dict->KeyExist("FirstChar"));
  ASSERT_TRUE(font_dict->KeyExist("LastChar"));
  EXPECT_EQ(32, font_dict->GetIntegerFor("FirstChar"));
  EXPECT_EQ(255, font_dict->GetIntegerFor("LastChar"));

  const CPDF_Array* widths_array = font_dict->GetArrayFor("Widths");
  ASSERT_TRUE(widths_array);
  ASSERT_EQ(224u, widths_array->size());
  EXPECT_EQ(250, widths_array->GetNumberAt(0));
  EXPECT_EQ(569, widths_array->GetNumberAt(11));
  EXPECT_EQ(500, widths_array->GetNumberAt(223));
  CheckFontDescriptor(font_dict, FPDF_FONT_TYPE1, true, false, span);
}

TEST_F(FPDFEditEmbedderTest, LoadSimpleTrueTypeFont) {
  CreateNewDocument();
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Courier");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TRUETYPE, false));
  ASSERT_TRUE(font.get());
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsTrueTypeFont());

  const CPDF_Dictionary* font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("TrueType", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Cousine-Regular", font_dict->GetNameFor("BaseFont"));
  ASSERT_TRUE(font_dict->KeyExist("FirstChar"));
  ASSERT_TRUE(font_dict->KeyExist("LastChar"));
  EXPECT_EQ(32, font_dict->GetIntegerFor("FirstChar"));
  EXPECT_EQ(255, font_dict->GetIntegerFor("LastChar"));

  const CPDF_Array* widths_array = font_dict->GetArrayFor("Widths");
  ASSERT_TRUE(widths_array);
  ASSERT_EQ(224u, widths_array->size());
  EXPECT_EQ(600, widths_array->GetNumberAt(33));
  EXPECT_EQ(600, widths_array->GetNumberAt(74));
  EXPECT_EQ(600, widths_array->GetNumberAt(223));
  CheckFontDescriptor(font_dict, FPDF_FONT_TRUETYPE, false, false, span);
}

TEST_F(FPDFEditEmbedderTest, LoadCIDType0Font) {
  CreateNewDocument();
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Times-Roman");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TYPE1, 1));
  ASSERT_TRUE(font.get());
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsCIDFont());

  // Check font dictionary entries
  const CPDF_Dictionary* font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("Type0", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Tinos-Regular-Identity-H", font_dict->GetNameFor("BaseFont"));
  EXPECT_EQ("Identity-H", font_dict->GetNameFor("Encoding"));
  const CPDF_Array* descendant_array =
      font_dict->GetArrayFor("DescendantFonts");
  ASSERT_TRUE(descendant_array);
  EXPECT_EQ(1u, descendant_array->size());

  // Check the CIDFontDict
  const CPDF_Dictionary* cidfont_dict = descendant_array->GetDictAt(0);
  EXPECT_EQ("Font", cidfont_dict->GetNameFor("Type"));
  EXPECT_EQ("CIDFontType0", cidfont_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Tinos-Regular", cidfont_dict->GetNameFor("BaseFont"));
  const CPDF_Dictionary* cidinfo_dict =
      cidfont_dict->GetDictFor("CIDSystemInfo");
  ASSERT_TRUE(cidinfo_dict);
  const CPDF_Object* registry = cidinfo_dict->GetObjectFor("Registry");
  ASSERT_TRUE(registry);
  EXPECT_EQ(CPDF_Object::kString, registry->GetType());
  EXPECT_EQ("Adobe", registry->GetString());
  const CPDF_Object* ordering = cidinfo_dict->GetObjectFor("Ordering");
  ASSERT_TRUE(ordering);
  EXPECT_EQ(CPDF_Object::kString, ordering->GetType());
  EXPECT_EQ("Identity", ordering->GetString());
  EXPECT_EQ(0, cidinfo_dict->GetNumberFor("Supplement"));
  CheckFontDescriptor(cidfont_dict, FPDF_FONT_TYPE1, false, false, span);

  // Check widths
  const CPDF_Array* widths_array = cidfont_dict->GetArrayFor("W");
  ASSERT_TRUE(widths_array);
  EXPECT_GT(widths_array->size(), 1u);
  CheckCompositeFontWidths(widths_array, typed_font);
}

TEST_F(FPDFEditEmbedderTest, LoadCIDType2Font) {
  CreateNewDocument();
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Helvetica-Oblique");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TRUETYPE, 1));
  ASSERT_TRUE(font.get());
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsCIDFont());

  // Check font dictionary entries
  const CPDF_Dictionary* font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("Type0", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Arimo-Italic", font_dict->GetNameFor("BaseFont"));
  EXPECT_EQ("Identity-H", font_dict->GetNameFor("Encoding"));
  const CPDF_Array* descendant_array =
      font_dict->GetArrayFor("DescendantFonts");
  ASSERT_TRUE(descendant_array);
  EXPECT_EQ(1u, descendant_array->size());

  // Check the CIDFontDict
  const CPDF_Dictionary* cidfont_dict = descendant_array->GetDictAt(0);
  EXPECT_EQ("Font", cidfont_dict->GetNameFor("Type"));
  EXPECT_EQ("CIDFontType2", cidfont_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Arimo-Italic", cidfont_dict->GetNameFor("BaseFont"));
  const CPDF_Dictionary* cidinfo_dict =
      cidfont_dict->GetDictFor("CIDSystemInfo");
  ASSERT_TRUE(cidinfo_dict);
  EXPECT_EQ("Adobe", cidinfo_dict->GetStringFor("Registry"));
  EXPECT_EQ("Identity", cidinfo_dict->GetStringFor("Ordering"));
  EXPECT_EQ(0, cidinfo_dict->GetNumberFor("Supplement"));
  CheckFontDescriptor(cidfont_dict, FPDF_FONT_TRUETYPE, false, true, span);

  // Check widths
  const CPDF_Array* widths_array = cidfont_dict->GetArrayFor("W");
  ASSERT_TRUE(widths_array);
  CheckCompositeFontWidths(widths_array, typed_font);
}

TEST_F(FPDFEditEmbedderTest, NormalizeNegativeRotation) {
  // Load document with a -90 degree rotation
  ASSERT_TRUE(OpenDocument("bug_713197.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);

  EXPECT_EQ(3, FPDFPage_GetRotation(page));
  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, AddTrueTypeFontText) {
  // Start with a blank page
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);
  {
    RetainPtr<CPDF_Font> stock_font =
        CPDF_Font::GetStockFont(cpdf_doc(), "Arial");
    pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
    ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                          FPDF_FONT_TRUETYPE, 0));
    ASSERT_TRUE(font.get());

    // Add some text to the page
    FPDF_PAGEOBJECT text_object =
        FPDFPageObj_CreateTextObj(document(), font.get(), 12.0f);
    EXPECT_TRUE(text_object);
    ScopedFPDFWideString text = GetFPDFWideString(kLoadedFontText);
    EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
    FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 400, 400);
    FPDFPage_InsertObject(page, text_object);
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kLoadedFontTextChecksum);

    // Add some more text, same font
    FPDF_PAGEOBJECT text_object2 =
        FPDFPageObj_CreateTextObj(document(), font.get(), 15.0f);
    ScopedFPDFWideString text2 = GetFPDFWideString(L"Bigger font size");
    EXPECT_TRUE(FPDFText_SetText(text_object2, text2.get()));
    FPDFPageObj_Transform(text_object2, 1, 0, 0, 1, 200, 200);
    FPDFPage_InsertObject(page, text_object2);
  }
  ScopedFPDFBitmap page_bitmap2 = RenderPage(page);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  const char kInsertTrueTypeChecksum[] = "4f9a6c7752ac7d4e4c731260fdb5af15";
#elif BUILDFLAG(IS_APPLE)
  const char kInsertTrueTypeChecksum[] = "c7e2271a7f30e5b919a13ead47cea105";
#else
  const char kInsertTrueTypeChecksum[] = "683f4a385a891494100192cb338b11f0";
#endif
  CompareBitmap(page_bitmap2.get(), 612, 792, kInsertTrueTypeChecksum);

  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  VerifySavedDocument(612, 792, kInsertTrueTypeChecksum);
}

TEST_F(FPDFEditEmbedderTest, TransformAnnot) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  {
    // Add an underline annotation to the page without specifying its rectangle.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page, FPDF_ANNOT_UNDERLINE));
    ASSERT_TRUE(annot);

    // FPDFPage_TransformAnnots() should run without errors when modifying
    // annotation rectangles.
    FPDFPage_TransformAnnots(page, 1, 2, 3, 4, 5, 6);
  }
  UnloadPage(page);
}

// TODO(npm): Add tests using Japanese fonts in other OS.
#if defined(OS_LINUX) || defined(OS_CHROMEOS)
TEST_F(FPDFEditEmbedderTest, AddCIDFontText) {
  // Start with a blank page
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);
  CFX_Font CIDfont;
  {
    // First, get the data from the font
    CIDfont.LoadSubst("Noto Sans CJK JP", 1, 0, 400, 0, FX_CodePage::kShiftJIS,
                      0);
    EXPECT_EQ("Noto Sans CJK JP", CIDfont.GetFaceName());
    pdfium::span<const uint8_t> span = CIDfont.GetFontSpan();

    // Load the data into a FPDF_Font.
    ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                          FPDF_FONT_TRUETYPE, 1));
    ASSERT_TRUE(font.get());

    // Add some text to the page
    FPDF_PAGEOBJECT text_object =
        FPDFPageObj_CreateTextObj(document(), font.get(), 12.0f);
    ASSERT_TRUE(text_object);
    std::wstring wstr = L"ABCDEFGhijklmnop.";
    ScopedFPDFWideString text = GetFPDFWideString(wstr);
    EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
    FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 200, 200);
    FPDFPage_InsertObject(page, text_object);

    // And add some Japanese characters
    FPDF_PAGEOBJECT text_object2 =
        FPDFPageObj_CreateTextObj(document(), font.get(), 18.0f);
    ASSERT_TRUE(text_object2);
    std::wstring wstr2 =
        L"\u3053\u3093\u306B\u3061\u306f\u4e16\u754C\u3002\u3053\u3053\u306B1"
        L"\u756A";
    ScopedFPDFWideString text2 = GetFPDFWideString(wstr2);
    EXPECT_TRUE(FPDFText_SetText(text_object2, text2.get()));
    FPDFPageObj_Transform(text_object2, 1, 0, 0, 1, 100, 500);
    FPDFPage_InsertObject(page, text_object2);
  }

  // Check that the text renders properly.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char md5[] = "2e174d17de96a760d42ca3a06acbf36a";
#else
  static constexpr char md5[] = "84d31d11b76845423a2cfc1879c0fbb9";
#endif
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, md5);
  }

  // Save the document, close the page.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  VerifySavedDocument(612, 792, md5);
}
#endif  // defined(OS_LINUX) || defined(OS_CHROMEOS)

// TODO(crbug.com/pdfium/1651): Fix this issue and enable the test for Skia.
#if defined(_SKIA_SUPPORT_)
#define MAYBE_SaveAndRender DISABLED_SaveAndRender
#else
#define MAYBE_SaveAndRender SaveAndRender
#endif
TEST_F(FPDFEditEmbedderTest, MAYBE_SaveAndRender) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static constexpr char kChecksum[] = "0e8b079e349e34f64211c495845a3529";
#else
  static constexpr char kChecksum[] = "3c20472b0552c0c22b88ab1ed8c6202b";
#endif
  {
    ASSERT_TRUE(OpenDocument("bug_779.pdf"));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_NE(nullptr, page);

    // Now add a more complex green path.
    FPDF_PAGEOBJECT green_path = FPDFPageObj_CreateNewPath(20, 20);
    EXPECT_TRUE(FPDFPageObj_SetFillColor(green_path, 0, 255, 0, 200));
    // TODO(npm): stroking will cause the checksums to differ.
    EXPECT_TRUE(FPDFPath_SetDrawMode(green_path, FPDF_FILLMODE_WINDING, 0));
    EXPECT_TRUE(FPDFPath_LineTo(green_path, 20, 63));
    EXPECT_TRUE(FPDFPath_BezierTo(green_path, 55, 55, 78, 78, 90, 90));
    EXPECT_TRUE(FPDFPath_LineTo(green_path, 133, 133));
    EXPECT_TRUE(FPDFPath_LineTo(green_path, 133, 33));
    EXPECT_TRUE(FPDFPath_BezierTo(green_path, 38, 33, 39, 36, 40, 40));
    EXPECT_TRUE(FPDFPath_Close(green_path));
    FPDFPage_InsertObject(page, green_path);
    ScopedFPDFBitmap page_bitmap = RenderLoadedPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kChecksum);

    // Now save the result, closing the page and document
    EXPECT_TRUE(FPDFPage_GenerateContent(page));
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    UnloadPage(page);
  }

  VerifySavedDocument(612, 792, kChecksum);
}

TEST_F(FPDFEditEmbedderTest, AddMark) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  CheckMarkCounts(page, 1, 19, 8, 4, 9, 1);

  // Add to the first page object a "Bounds" mark with "Position": "First".
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_AddMark(page_object, "Bounds");
  EXPECT_TRUE(mark);
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), page_object, mark,
                                             "Position", "First"));

  CheckMarkCounts(page, 1, 19, 8, 4, 9, 2);

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the new mark is present.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  CheckMarkCounts(saved_page, 1, 19, 8, 4, 9, 2);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, AddMarkCompressedStream) {
  // Load document with some text in a compressed stream.
  ASSERT_TRUE(OpenDocument("hello_world_compressed_stream.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Render and check there are no marks.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldChecksum);
  }
  CheckMarkCounts(page, 0, 2, 0, 0, 0, 0);

  // Add to the first page object a "Bounds" mark with "Position": "First".
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 0);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_AddMark(page_object, "Bounds");
  EXPECT_TRUE(mark);
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), page_object, mark,
                                             "Position", "First"));

  // Render and check there is 1 mark.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldChecksum);
  }
  CheckMarkCounts(page, 0, 2, 0, 0, 0, 1);

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and check the new mark is present.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kHelloWorldChecksum);
  }
  CheckMarkCounts(saved_page, 0, 2, 0, 0, 0, 1);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, SetMarkParam) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  constexpr int kExpectedObjectCount = 19;
  CheckMarkCounts(page, 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Check the "Bounds" mark's "Position" param is "Last".
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);
  char buffer[256];
  unsigned long name_len = 999u;
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
  EXPECT_EQ((6u + 1u) * 2u, name_len);
  ASSERT_EQ(L"Bounds",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer)));
  unsigned long out_buffer_len;
  ASSERT_TRUE(FPDFPageObjMark_GetParamStringValue(
      mark, "Position", buffer, sizeof(buffer), &out_buffer_len));
  ASSERT_EQ(L"Last",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer)));

  // Set is to "End".
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), page_object, mark,
                                             "Position", "End"));

  // Verify the object passed must correspond to the mark passed.
  FPDF_PAGEOBJECT another_page_object = FPDFPage_GetObject(page, 17);
  EXPECT_FALSE(FPDFPageObjMark_SetStringParam(document(), another_page_object,
                                              mark, "Position", "End"));

  // Verify nothing else changed.
  CheckMarkCounts(page, 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Verify "Position" now maps to "End".
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(
      mark, "Position", buffer, sizeof(buffer), &out_buffer_len));
  EXPECT_EQ(L"End",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer)));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  UnloadPage(page);

  // Re-open the file and cerify "Position" still maps to "End".
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  CheckMarkCounts(saved_page, 1, kExpectedObjectCount, 8, 4, 9, 1);
  page_object = FPDFPage_GetObject(saved_page, 18);
  mark = FPDFPageObj_GetMark(page_object, 1);
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(
      mark, "Position", buffer, sizeof(buffer), &out_buffer_len));
  EXPECT_EQ(L"End",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer)));

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, AddMarkedText) {
  // Start with a blank page.
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);

  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Arial");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TRUETYPE, 0));
  ASSERT_TRUE(font.get());

  // Add some text to the page.
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_CreateTextObj(document(), font.get(), 12.0f);

  EXPECT_TRUE(text_object);
  ScopedFPDFWideString text1 = GetFPDFWideString(kLoadedFontText);
  EXPECT_TRUE(FPDFText_SetText(text_object, text1.get()));
  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 400, 400);
  FPDFPage_InsertObject(page, text_object);

  // Add a mark with the tag "TestMarkName" to that text.
  EXPECT_EQ(0, FPDFPageObj_CountMarks(text_object));
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_AddMark(text_object, "Test Mark Name");
  EXPECT_TRUE(mark);
  EXPECT_EQ(1, FPDFPageObj_CountMarks(text_object));
  EXPECT_EQ(mark, FPDFPageObj_GetMark(text_object, 0));
  char buffer[256];
  unsigned long name_len = 999u;
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
  EXPECT_EQ((14u + 1u) * 2, name_len);
  std::wstring name =
      GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
  EXPECT_EQ(L"Test Mark Name", name);

  // Add parameters:
  // - int "IntKey" : 42
  // - string "StringKey": "StringValue"
  // - blob "BlobKey": "\x01\x02\x03\0BlobValue1\0\0\0BlobValue2\0"
  constexpr size_t kBlobLen = 28;
  char block_value[kBlobLen];
  memcpy(block_value, "\x01\x02\x03\0BlobValue1\0\0\0BlobValue2\0", kBlobLen);
  EXPECT_EQ(0, FPDFPageObjMark_CountParams(mark));
  EXPECT_TRUE(
      FPDFPageObjMark_SetIntParam(document(), text_object, mark, "IntKey", 42));
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), text_object, mark,
                                             "StringKey", "StringValue"));
  EXPECT_TRUE(FPDFPageObjMark_SetBlobParam(document(), text_object, mark,
                                           "BlobKey", block_value, kBlobLen));
  EXPECT_EQ(3, FPDFPageObjMark_CountParams(mark));

  // Check the two parameters can be retrieved.
  EXPECT_EQ(FPDF_OBJECT_NUMBER,
            FPDFPageObjMark_GetParamValueType(mark, "IntKey"));
  int int_value;
  EXPECT_TRUE(FPDFPageObjMark_GetParamIntValue(mark, "IntKey", &int_value));
  EXPECT_EQ(42, int_value);

  EXPECT_EQ(FPDF_OBJECT_STRING,
            FPDFPageObjMark_GetParamValueType(mark, "StringKey"));
  unsigned long out_buffer_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(
      mark, "StringKey", buffer, sizeof(buffer), &out_buffer_len));
  EXPECT_GT(out_buffer_len, 0u);
  EXPECT_NE(999u, out_buffer_len);
  name = GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
  EXPECT_EQ(L"StringValue", name);

  EXPECT_EQ(FPDF_OBJECT_STRING,
            FPDFPageObjMark_GetParamValueType(mark, "BlobKey"));
  out_buffer_len = 0;
  EXPECT_TRUE(FPDFPageObjMark_GetParamBlobValue(
      mark, "BlobKey", buffer, sizeof(buffer), &out_buffer_len));
  EXPECT_EQ(kBlobLen, out_buffer_len);
  EXPECT_EQ(0, memcmp(block_value, buffer, kBlobLen));

  // Render and check the bitmap is the expected one.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, kLoadedFontTextChecksum);
  }

  // Now save the result.
  EXPECT_EQ(1, FPDFPage_CountObjects(page));
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  FPDF_ClosePage(page);

  // Re-open the file and check the changes were kept in the saved .pdf.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(1, FPDFPage_CountObjects(saved_page));

  text_object = FPDFPage_GetObject(saved_page, 0);
  EXPECT_TRUE(text_object);
  EXPECT_EQ(1, FPDFPageObj_CountMarks(text_object));
  mark = FPDFPageObj_GetMark(text_object, 0);
  EXPECT_TRUE(mark);

  name_len = 999u;
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
  EXPECT_EQ((14u + 1u) * 2, name_len);
  name = GetPlatformWString(reinterpret_cast<unsigned short*>(buffer));
  EXPECT_EQ(L"Test Mark Name", name);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, MarkGetName) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);

  char buffer[256];
  unsigned long out_len;

  // Show the positive cases of FPDFPageObjMark_GetName.
  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetName(mark, nullptr, 0, &out_len));
  EXPECT_EQ((6u + 1u) * 2u, out_len);

  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(L"Bounds",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer)));
  EXPECT_EQ((6u + 1u) * 2u, out_len);

  // Show the negative cases of FPDFPageObjMark_GetName.
  out_len = 999u;
  EXPECT_FALSE(
      FPDFPageObjMark_GetName(nullptr, buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(999u, out_len);

  EXPECT_FALSE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), nullptr));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, MarkGetParamKey) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);

  char buffer[256];
  unsigned long out_len;

  // Show the positive cases of FPDFPageObjMark_GetParamKey.
  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetParamKey(mark, 0, nullptr, 0, &out_len));
  EXPECT_EQ((8u + 1u) * 2u, out_len);

  out_len = 999u;
  EXPECT_TRUE(
      FPDFPageObjMark_GetParamKey(mark, 0, buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(L"Position",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer)));
  EXPECT_EQ((8u + 1u) * 2u, out_len);

  // Show the negative cases of FPDFPageObjMark_GetParamKey.
  out_len = 999u;
  EXPECT_FALSE(FPDFPageObjMark_GetParamKey(nullptr, 0, buffer, sizeof(buffer),
                                           &out_len));
  EXPECT_EQ(999u, out_len);

  out_len = 999u;
  EXPECT_FALSE(
      FPDFPageObjMark_GetParamKey(mark, 1, buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(999u, out_len);

  EXPECT_FALSE(
      FPDFPageObjMark_GetParamKey(mark, 0, buffer, sizeof(buffer), nullptr));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, MarkGetIntParam) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 8);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 0);
  ASSERT_TRUE(mark);

  int out_value;

  // Show the positive cases of FPDFPageObjMark_GetParamIntValue.
  out_value = 999;
  EXPECT_TRUE(FPDFPageObjMark_GetParamIntValue(mark, "Factor", &out_value));
  EXPECT_EQ(3, out_value);

  // Show the negative cases of FPDFPageObjMark_GetParamIntValue.
  out_value = 999;
  EXPECT_FALSE(FPDFPageObjMark_GetParamIntValue(nullptr, "Factor", &out_value));
  EXPECT_EQ(999, out_value);

  out_value = 999;
  EXPECT_FALSE(FPDFPageObjMark_GetParamIntValue(mark, "ParamThatDoesNotExist",
                                                &out_value));
  EXPECT_EQ(999, out_value);

  EXPECT_FALSE(FPDFPageObjMark_GetParamIntValue(mark, "Factor", nullptr));

  page_object = FPDFPage_GetObject(page, 18);
  mark = FPDFPageObj_GetMark(page_object, 1);
  out_value = 999;
  EXPECT_FALSE(FPDFPageObjMark_GetParamIntValue(mark, "Position", &out_value));
  EXPECT_EQ(999, out_value);

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, MarkGetStringParam) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page, 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);

  char buffer[256];
  unsigned long out_len;

  // Show the positive cases of FPDFPageObjMark_GetParamStringValue.
  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(mark, "Position", nullptr, 0,
                                                  &out_len));
  EXPECT_EQ((4u + 1u) * 2u, out_len);

  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(mark, "Position", buffer,
                                                  sizeof(buffer), &out_len));
  EXPECT_EQ(L"Last",
            GetPlatformWString(reinterpret_cast<unsigned short*>(buffer)));
  EXPECT_EQ((4u + 1u) * 2u, out_len);

  // Show the negative cases of FPDFPageObjMark_GetParamStringValue.
  out_len = 999u;
  EXPECT_FALSE(FPDFPageObjMark_GetParamStringValue(nullptr, "Position", buffer,
                                                   sizeof(buffer), &out_len));
  EXPECT_EQ(999u, out_len);

  out_len = 999u;
  EXPECT_FALSE(FPDFPageObjMark_GetParamStringValue(
      mark, "ParamThatDoesNotExist", buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(999u, out_len);

  EXPECT_FALSE(FPDFPageObjMark_GetParamStringValue(mark, "Position", buffer,
                                                   sizeof(buffer), nullptr));

  page_object = FPDFPage_GetObject(page, 8);
  mark = FPDFPageObj_GetMark(page_object, 0);
  out_len = 999u;
  EXPECT_FALSE(FPDFPageObjMark_GetParamStringValue(mark, "Factor", buffer,
                                                   sizeof(buffer), &out_len));
  EXPECT_EQ(999u, out_len);

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetBitmap) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 32);
  EXPECT_NE(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_FALSE(FPDFImageObj_GetBitmap(obj));

  {
    obj = FPDFPage_GetObject(page, 33);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 109, 88, kEmbeddedImage33Checksum);
  }

  {
    obj = FPDFPage_GetObject(page, 34);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 103, 75, "c8d51fa6821ceb2a67f08446ff236c40");
  }

  {
    obj = FPDFPage_GetObject(page, 35);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_Gray, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 92, 68, "9c6d76cb1e37ef8514f9455d759391f3");
  }

  {
    obj = FPDFPage_GetObject(page, 36);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 79, 60, "f4e72fb783a01c7b4614cdc25eaa98ac");
  }

  {
    obj = FPDFPage_GetObject(page, 37);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 126, 106, "2cf9e66414c72461f4ccbf9cdebdfa1b");
  }

  {
    obj = FPDFPage_GetObject(page, 38);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 194, 119, "a8f3a126cec274dab8242fd2ccdc1b8b");
  }

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetBitmapIgnoresSetMatrix) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  {
    // Render |obj| as is.
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 109, 88, kEmbeddedImage33Checksum);
  }

  // Check the matrix for |obj|.
  FS_MATRIX matrix;
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(53.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(43.0f, matrix.d);
  EXPECT_FLOAT_EQ(72.0f, matrix.e);
  EXPECT_FLOAT_EQ(646.510009765625f, matrix.f);

  // Modify the matrix for |obj|.
  matrix.a = 120.0;
  EXPECT_TRUE(FPDFPageObj_SetMatrix(obj, &matrix));

  // Make sure the matrix modification took place.
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(120.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(43.0f, matrix.d);
  EXPECT_FLOAT_EQ(72.0f, matrix.e);
  EXPECT_FLOAT_EQ(646.510009765625f, matrix.f);

  {
    // Render |obj| again. Note that the FPDFPageObj_SetMatrix() call has no
    // effect.
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 109, 88, kEmbeddedImage33Checksum);
  }

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetBitmapForJBigImage) {
  ASSERT_TRUE(OpenDocument("bug_631912.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  {
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    ASSERT_TRUE(bitmap);
    EXPECT_EQ(FPDFBitmap_Gray, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 1152, 720, "3f6a48e2b3e91b799bf34567f55cb4de");
  }

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetBitmapIgnoresSMask) {
  ASSERT_TRUE(OpenDocument("matte.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  constexpr int kExpectedObjects = 4;
  ASSERT_EQ(kExpectedObjects, FPDFPage_CountObjects(page));

  for (int i = 0; i < kExpectedObjects; ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    ASSERT_TRUE(bitmap);
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 50, 50, "46c9a1dbe0b44765ce46017ad629a2fe");
  }

  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_)
#define MAYBE_GetRenderedBitmapHandlesSetMatrix \
  DISABLED_GetRenderedBitmapHandlesSetMatrix
#else
#define MAYBE_GetRenderedBitmapHandlesSetMatrix \
  GetRenderedBitmapHandlesSetMatrix
#endif
TEST_F(FPDFEditEmbedderTest, MAYBE_GetRenderedBitmapHandlesSetMatrix) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  {
    // Render |obj| as is.
    ScopedFPDFBitmap bitmap(
        FPDFImageObj_GetRenderedBitmap(document(), page, obj));
    EXPECT_EQ(FPDFBitmap_BGRA, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 53, 43, "582ca300e003f512d7b552c7b5b45d2e");
  }

  // Check the matrix for |obj|.
  FS_MATRIX matrix;
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(53.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(43.0f, matrix.d);
  EXPECT_FLOAT_EQ(72.0f, matrix.e);
  EXPECT_FLOAT_EQ(646.510009765625f, matrix.f);

  // Modify the matrix for |obj|.
  matrix.a = 120.0;
  EXPECT_TRUE(FPDFPageObj_SetMatrix(obj, &matrix));

  // Make sure the matrix modification took place.
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(120.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(43.0f, matrix.d);
  EXPECT_FLOAT_EQ(72.0f, matrix.e);
  EXPECT_FLOAT_EQ(646.510009765625f, matrix.f);

  {
    // Render |obj| again. Note that the FPDFPageObj_SetMatrix() call has an
    // effect.
    ScopedFPDFBitmap bitmap(
        FPDFImageObj_GetRenderedBitmap(document(), page, obj));
    EXPECT_EQ(FPDFBitmap_BGRA, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 120, 43, "0824c16dcf2dfcef44b45d88db1fddce");
  }

  UnloadPage(page);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_)
#define MAYBE_GetRenderedBitmapHandlesSMask \
  DISABLED_GetRenderedBitmapHandlesSMask
#else
#define MAYBE_GetRenderedBitmapHandlesSMask GetRenderedBitmapHandlesSMask
#endif
TEST_F(FPDFEditEmbedderTest, MAYBE_GetRenderedBitmapHandlesSMask) {
  ASSERT_TRUE(OpenDocument("matte.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  constexpr int kExpectedObjects = 4;
  ASSERT_EQ(kExpectedObjects, FPDFPage_CountObjects(page));

  for (int i = 0; i < kExpectedObjects; ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(
        FPDFImageObj_GetRenderedBitmap(document(), page, obj));
    ASSERT_TRUE(bitmap);
    EXPECT_EQ(FPDFBitmap_BGRA, FPDFBitmap_GetFormat(bitmap.get()));
    if (i == 0)
      CompareBitmap(bitmap.get(), 40, 60, "5a3ae4a660ce919e29c42ec2258142f1");
    else
      CompareBitmap(bitmap.get(), 40, 60, "67504e83f5d78214ea00efc19082c5c1");
  }

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapBadParams) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  // Test various null parameters.
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, nullptr, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(document(), nullptr, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, page, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, nullptr, obj));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(document(), page, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, page, obj));

  // Test mismatch between document and page parameters.
  ScopedFPDFDocument new_document(FPDF_CreateNewDocument());
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(new_document.get(), page, obj));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetImageData) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page));

  // Retrieve an image object with flate-encoded data stream.
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  // Check that the raw image data has the correct length and hash value.
  unsigned long len = FPDFImageObj_GetImageDataRaw(obj, nullptr, 0);
  std::vector<char> buf(len);
  EXPECT_EQ(4091u, FPDFImageObj_GetImageDataRaw(obj, buf.data(), len));
  EXPECT_EQ("f73802327d2e88e890f653961bcda81a",
            GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()), len));

  // Check that the decoded image data has the correct length and hash value.
  len = FPDFImageObj_GetImageDataDecoded(obj, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(28776u, FPDFImageObj_GetImageDataDecoded(obj, buf.data(), len));
  EXPECT_EQ(kEmbeddedImage33Checksum,
            GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()), len));

  // Retrieve an image object with DCTDecode-encoded data stream.
  obj = FPDFPage_GetObject(page, 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  // Check that the raw image data has the correct length and hash value.
  len = FPDFImageObj_GetImageDataRaw(obj, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(4370u, FPDFImageObj_GetImageDataRaw(obj, buf.data(), len));
  EXPECT_EQ("6aae1f3710335023a9e12191be66b64b",
            GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()), len));

  // Check that the decoded image data has the correct length and hash value,
  // which should be the same as those of the raw data, since this image is
  // encoded by a single DCTDecode filter and decoding is a noop.
  len = FPDFImageObj_GetImageDataDecoded(obj, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(4370u, FPDFImageObj_GetImageDataDecoded(obj, buf.data(), len));
  EXPECT_EQ("6aae1f3710335023a9e12191be66b64b",
            GenerateMD5Base16(reinterpret_cast<uint8_t*>(buf.data()), len));

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetImageMatrix) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT obj;
  FS_MATRIX matrix;

  obj = FPDFPage_GetObject(page, 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(53.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(43.0f, matrix.d);
  EXPECT_FLOAT_EQ(72.0f, matrix.e);
  EXPECT_FLOAT_EQ(646.510009765625f, matrix.f);

  obj = FPDFPage_GetObject(page, 34);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(70.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(51.0f, matrix.d);
  EXPECT_FLOAT_EQ(216.0f, matrix.e);
  EXPECT_FLOAT_EQ(646.510009765625f, matrix.f);

  obj = FPDFPage_GetObject(page, 35);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(69.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(51.0f, matrix.d);
  EXPECT_FLOAT_EQ(360.0f, matrix.e);
  EXPECT_FLOAT_EQ(646.510009765625f, matrix.f);

  obj = FPDFPage_GetObject(page, 36);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(59.0f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(45.0f, matrix.d);
  EXPECT_FLOAT_EQ(72.0f, matrix.e);
  EXPECT_FLOAT_EQ(553.510009765625f, matrix.f);

  obj = FPDFPage_GetObject(page, 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(55.94000244140625f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(46.950000762939453f, matrix.d);
  EXPECT_FLOAT_EQ(216.0f, matrix.e);
  EXPECT_FLOAT_EQ(552.510009765625f, matrix.f);

  obj = FPDFPage_GetObject(page, 38);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  EXPECT_FLOAT_EQ(70.528999328613281f, matrix.a);
  EXPECT_FLOAT_EQ(0.0f, matrix.b);
  EXPECT_FLOAT_EQ(0.0f, matrix.c);
  EXPECT_FLOAT_EQ(43.149997711181641f, matrix.d);
  EXPECT_FLOAT_EQ(360.0f, matrix.e);
  EXPECT_FLOAT_EQ(553.3599853515625f, matrix.f);

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, DestroyPageObject) {
  FPDF_PAGEOBJECT rect = FPDFPageObj_CreateNewRect(10, 10, 20, 20);
  ASSERT_TRUE(rect);

  // There should be no memory leaks with a call to FPDFPageObj_Destroy().
  FPDFPageObj_Destroy(rect);
}

TEST_F(FPDFEditEmbedderTest, GetImageFilters) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Verify that retrieving the filter of a non-image object would fail.
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 32);
  ASSERT_NE(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_EQ(0, FPDFImageObj_GetImageFilterCount(obj));
  EXPECT_EQ(0u, FPDFImageObj_GetImageFilter(obj, 0, nullptr, 0));

  // Verify the returned filter string for an image object with a single filter.
  obj = FPDFPage_GetObject(page, 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_EQ(1, FPDFImageObj_GetImageFilterCount(obj));
  unsigned long len = FPDFImageObj_GetImageFilter(obj, 0, nullptr, 0);
  std::vector<char> buf(len);
  static constexpr char kFlateDecode[] = "FlateDecode";
  EXPECT_EQ(sizeof(kFlateDecode),
            FPDFImageObj_GetImageFilter(obj, 0, buf.data(), len));
  EXPECT_STREQ(kFlateDecode, buf.data());
  EXPECT_EQ(0u, FPDFImageObj_GetImageFilter(obj, 1, nullptr, 0));

  // Verify all the filters for an image object with a list of filters.
  obj = FPDFPage_GetObject(page, 38);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_EQ(2, FPDFImageObj_GetImageFilterCount(obj));
  len = FPDFImageObj_GetImageFilter(obj, 0, nullptr, 0);
  buf.clear();
  buf.resize(len);
  static constexpr char kASCIIHexDecode[] = "ASCIIHexDecode";
  EXPECT_EQ(sizeof(kASCIIHexDecode),
            FPDFImageObj_GetImageFilter(obj, 0, buf.data(), len));
  EXPECT_STREQ(kASCIIHexDecode, buf.data());

  len = FPDFImageObj_GetImageFilter(obj, 1, nullptr, 0);
  buf.clear();
  buf.resize(len);
  static constexpr char kDCTDecode[] = "DCTDecode";
  EXPECT_EQ(sizeof(kDCTDecode),
            FPDFImageObj_GetImageFilter(obj, 1, buf.data(), len));
  EXPECT_STREQ(kDCTDecode, buf.data());

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetImageMetadata) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Check that getting the metadata of a null object would fail.
  FPDF_IMAGEOBJ_METADATA metadata;
  EXPECT_FALSE(FPDFImageObj_GetImageMetadata(nullptr, page, &metadata));

  // Check that receiving the metadata with a null metadata object would fail.
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 35);
  EXPECT_FALSE(FPDFImageObj_GetImageMetadata(obj, page, nullptr));

  // Check that when retrieving an image object's metadata without passing in
  // |page|, all values are correct, with the last two being default values.
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_TRUE(FPDFImageObj_GetImageMetadata(obj, nullptr, &metadata));
  EXPECT_EQ(7, metadata.marked_content_id);
  EXPECT_EQ(92u, metadata.width);
  EXPECT_EQ(68u, metadata.height);
  EXPECT_FLOAT_EQ(96.0f, metadata.horizontal_dpi);
  EXPECT_FLOAT_EQ(96.0f, metadata.vertical_dpi);
  EXPECT_EQ(0u, metadata.bits_per_pixel);
  EXPECT_EQ(FPDF_COLORSPACE_UNKNOWN, metadata.colorspace);

  // Verify the metadata of a bitmap image with indexed colorspace.
  ASSERT_TRUE(FPDFImageObj_GetImageMetadata(obj, page, &metadata));
  EXPECT_EQ(7, metadata.marked_content_id);
  EXPECT_EQ(92u, metadata.width);
  EXPECT_EQ(68u, metadata.height);
  EXPECT_FLOAT_EQ(96.0f, metadata.horizontal_dpi);
  EXPECT_FLOAT_EQ(96.0f, metadata.vertical_dpi);
  EXPECT_EQ(1u, metadata.bits_per_pixel);
  EXPECT_EQ(FPDF_COLORSPACE_INDEXED, metadata.colorspace);

  // Verify the metadata of an image with RGB colorspace.
  obj = FPDFPage_GetObject(page, 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_TRUE(FPDFImageObj_GetImageMetadata(obj, page, &metadata));
  EXPECT_EQ(9, metadata.marked_content_id);
  EXPECT_EQ(126u, metadata.width);
  EXPECT_EQ(106u, metadata.height);
  EXPECT_FLOAT_EQ(162.173752f, metadata.horizontal_dpi);
  EXPECT_FLOAT_EQ(162.555878f, metadata.vertical_dpi);
  EXPECT_EQ(24u, metadata.bits_per_pixel);
  EXPECT_EQ(FPDF_COLORSPACE_DEVICERGB, metadata.colorspace);

  UnloadPage(page);
}

TEST_F(FPDFEditEmbedderTest, GetImageMetadataJpxLzw) {
  ASSERT_TRUE(OpenDocument("jpx_lzw.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, 0);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  FPDF_IMAGEOBJ_METADATA metadata;
  ASSERT_TRUE(FPDFImageObj_GetImageMetadata(obj, page, &metadata));
  EXPECT_EQ(-1, metadata.marked_content_id);
  EXPECT_EQ(612u, metadata.width);
  EXPECT_EQ(792u, metadata.height);
  EXPECT_FLOAT_EQ(72.0f, metadata.horizontal_dpi);
  EXPECT_FLOAT_EQ(72.0f, metadata.vertical_dpi);
  EXPECT_EQ(24u, metadata.bits_per_pixel);
  EXPECT_EQ(FPDF_COLORSPACE_UNKNOWN, metadata.colorspace);

  UnloadPage(page);
}
