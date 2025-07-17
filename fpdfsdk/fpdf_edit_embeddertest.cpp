// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <array>
#include <limits>
#include <memory>
#include <ostream>
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
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
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
#include "testing/utils/compare_coordinates.h"
#include "testing/utils/file_util.h"
#include "testing/utils/hash.h"
#include "testing/utils/path_service.h"

using pdfium::HelloWorldChecksum;
using pdfium::kBlankPage200By200Checksum;
using testing::HasSubstr;
using testing::Not;
using testing::UnorderedElementsAreArray;

namespace {

const wchar_t kBottomText[] = L"I'm at the bottom of the page";

const char* BottomTextChecksum() {
  if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
    return "5d8f2b613a2f9591a52373c72d6b88ee";
#elif BUILDFLAG(IS_APPLE)
    return "8ca7dc6269ee68507389aa40eebcb9f8";
#else
    return "c62d315856a558d2666b80d474831efe";
#endif
  }
#if BUILDFLAG(IS_APPLE)
  return "81636489006a31fcb00cf29efcdf7909";
#else
  return "891dcb6e914c8360998055f1f47c9727";
#endif
}

const char* FirstRemovedChecksum() {
  if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
    return "251007e902e512d0359240ad957ee2dc";
#elif BUILDFLAG(IS_APPLE)
    return "dcb929fae86d5b935888ce7f9f1ab71b";
#else
    return "3006ab2b12d27246eae4faad509ac575";
#endif
  }
#if BUILDFLAG(IS_APPLE)
  return "a1dc2812692fcc7ee4f01ca77435df9d";
#else
  return "e1477dc3b5b3b9c560814c4d1135a02b";
#endif
}

const wchar_t kLoadedFontText[] = L"I am testing my loaded font, WEE.";

const char* LoadedFontTextChecksum() {
  if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
    return "b0efd562e84958f06bb006ba27d5f4bd";
#elif BUILDFLAG(IS_APPLE)
    return "23e7874d160692b0ef3e0c8780f73dab";
#else
    return "fc2334c350cbd0d2ae6076689da09741";
#endif
  }
#if BUILDFLAG(IS_APPLE)
  return "0f3e4a7d71f9e7eb8a1a0d69403b9848";
#else
  return "d58570cc045dfb818b92cbabbd1a364c";
#endif
}

const char kRedRectangleChecksum[] = "66d02eaa6181e2c069ce2ea99beda497";

// In embedded_images.pdf.
const char kEmbeddedImage33Checksum[] = "cb3637934bb3b95a6e4ae1ea9eb9e56e";

const char* NotoSansSCChecksum() {
  if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
    return "a1bc9e4007dc2155e9f56bf16234573e";
#elif BUILDFLAG(IS_APPLE)
    return "9a31fb87d1c6d2346bba22d1196041cd";
#else
    return "5bb65e15fc0a685934cd5006dec08a76";
#endif
  }
  return "9a31fb87d1c6d2346bba22d1196041cd";
}

struct FPDFEditMoveEmbedderTestCase {
  std::vector<int> page_indices;
  int page_indices_len;
  int dest_page_index;
  // whether FPDF_MovePages() will succeed or fail
  bool expected_result;
  // expected order of pages if `expected_result` is true
  std::vector<int> expected_order;
  const char* const name;
};

std::ostream& operator<<(std::ostream& os,
                         const FPDFEditMoveEmbedderTestCase& t) {
  os << t.name << ": Indices are {";
  for (size_t i = 0; i < t.page_indices.size(); ++i) {
    os << t.page_indices[i];
    if (i != t.page_indices.size() - 1) {
      os << ", ";
    }
  }
  os << "}, page order len is " << t.page_indices_len << ", dest page index is "
     << t.dest_page_index << ", expected result is " << t.expected_result;
  return os;
}

RetainPtr<const CPDF_Array> GetWidthsArrayForCidFont(const CPDF_Font* font) {
  if (!font || !font->IsCIDFont()) {
    ADD_FAILURE();
    return nullptr;
  }

  RetainPtr<const CPDF_Dictionary> font_dict = font->GetFontDict();
  if (!font_dict) {
    ADD_FAILURE();
    return nullptr;
  }

  RetainPtr<const CPDF_Array> descendant_array =
      font_dict->GetArrayFor("DescendantFonts");
  if (!descendant_array || descendant_array->size() != 1) {
    ADD_FAILURE();
    return nullptr;
  }

  RetainPtr<const CPDF_Dictionary> cidfont_dict =
      descendant_array->GetDictAt(0);
  if (!cidfont_dict) {
    ADD_FAILURE();
    return nullptr;
  }

  return cidfont_dict->GetArrayFor("W");
}

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
    RetainPtr<const CPDF_Dictionary> font_desc =
        font_dict->GetDictFor("FontDescriptor");
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

    RetainPtr<const CPDF_Array> fontBBox = font_desc->GetArrayFor("FontBBox");
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
    ByteStringView present("FontFile");
    ByteStringView absent("FontFile2");
    if (font_type == FPDF_FONT_TRUETYPE) {
      std::swap(present, absent);
    }
    EXPECT_TRUE(font_desc->KeyExist(present));
    EXPECT_FALSE(font_desc->KeyExist(absent));

    auto streamAcc =
        pdfium::MakeRetain<CPDF_StreamAcc>(font_desc->GetStreamFor(present));
    streamAcc->LoadAllDataRaw();

    // Check that the font stream is the one that was provided
    ASSERT_EQ(span.size(), streamAcc->GetSize());
    if (font_type == FPDF_FONT_TRUETYPE) {
      ASSERT_EQ(static_cast<int>(span.size()), streamAcc->GetLength1ForTest());
    }

    pdfium::span<const uint8_t> stream_data = streamAcc->GetSpan();
    for (size_t j = 0; j < span.size(); j++) {
      EXPECT_EQ(span[j], stream_data[j]) << " at byte " << j;
    }
  }

  void CheckCompositeFontWidths(const CPDF_Array* widths_array,
                                CPDF_Font* typed_font,
                                testing::Matcher<int> matcher) {
    // Check that W array is in a format that conforms to PDF spec 1.7 section
    // "Glyph Metrics in CIDFonts" (these checks are not
    // implementation-specific).
    EXPECT_GT(widths_array->size(), 1u);
    int num_cids_checked = 0;
    int cur_cid = 0;
    for (size_t idx = 0; idx < widths_array->size(); idx++) {
      int cid = widths_array->GetFloatAt(idx);
      EXPECT_GE(cid, cur_cid);
      ASSERT_FALSE(++idx == widths_array->size());
      RetainPtr<const CPDF_Object> next = widths_array->GetObjectAt(idx);
      if (next->IsArray()) {
        // We are in the c [w1 w2 ...] case
        const CPDF_Array* arr = next->AsArray();
        int cnt = static_cast<int>(arr->size());
        size_t inner_idx = 0;
        for (cur_cid = cid; cur_cid < cid + cnt; cur_cid++) {
          int width = arr->GetFloatAt(inner_idx++);
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
      int width = widths_array->GetFloatAt(idx);
      for (cur_cid = cid; cur_cid <= last_cid; cur_cid++) {
        EXPECT_EQ(width, typed_font->GetCharWidthF(cur_cid))
            << " at cid " << cur_cid;
      }
      num_cids_checked += last_cid - cid + 1;
    }
    // Check the CID count.
    EXPECT_THAT(num_cids_checked, matcher);
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
    "/Resources<<>>"
    "/Rotate 0/Type/Page"
    ">>\r\n"
    "endobj\r\n"
    "xref\r\n"
    "0 5\r\n"
    "0000000000 65535 f\r\n"
    "0000000017 00000 n\r\n"
    "0000000066 00000 n\r\n"
    "0000000122 00000 n\r\n"
    "0000000192 00000 n\r\n"
    "trailer\r\n"
    "<<\r\n"
    "/Root 1 0 R\r\n"
    "/Info 3 0 R\r\n"
    "/Size 5/ID\\[<.*><.*>\\]>>\r\n"
    "startxref\r\n"
    "285\r\n"
    "%%EOF\r\n";

}  // namespace

TEST_F(FPDFEditEmbedderTest, EmbedNotoSansSCFont) {
  CreateEmptyDocument();
  ScopedFPDFPage page(FPDFPage_New(document(), 0, 400, 400));
  std::string font_path;
  ASSERT_TRUE(PathService::GetThirdPartyFilePath(
      "NotoSansCJK/NotoSansSC-Regular.subset.otf", &font_path));

  std::vector<uint8_t> font_data = GetFileContents(font_path.c_str());
  ASSERT_FALSE(font_data.empty());

  ScopedFPDFFont font(FPDFText_LoadFont(document(), font_data.data(),
                                        font_data.size(), FPDF_FONT_TRUETYPE,
                                        /*cid=*/true));
  ASSERT_TRUE(font);
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_CreateTextObj(document(), font.get(), 20.0f);
  EXPECT_TRUE(text_object);

  // Test the characters which are either mapped to one single unicode or
  // multiple unicodes in the embedded font.
  ScopedFPDFWideString text = GetFPDFWideString(L"这是第一句。 这是第二行。");
  EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));

  const FS_MATRIX matrix{1, 0, 0, 1, 50, 200};
  ASSERT_TRUE(FPDFPageObj_TransformF(text_object, &matrix));
  FPDFPage_InsertObject(page.get(), text_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
  CompareBitmap(page_bitmap.get(), 400, 400, NotoSansSCChecksum());

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(400, 400, NotoSansSCChecksum());
}

TEST_F(FPDFEditEmbedderTest, EmbedNotoSansSCFontWithCharcodes) {
  CreateEmptyDocument();
  ScopedFPDFPage page(FPDFPage_New(document(), 0, 400, 400));
  std::string font_path;
  ASSERT_TRUE(PathService::GetThirdPartyFilePath(
      "NotoSansCJK/NotoSansSC-Regular.subset.otf", &font_path));

  std::vector<uint8_t> font_data = GetFileContents(font_path.c_str());
  ASSERT_FALSE(font_data.empty());

  ScopedFPDFFont font(FPDFText_LoadFont(document(), font_data.data(),
                                        font_data.size(), FPDF_FONT_TRUETYPE,
                                        /*cid=*/true));
  ASSERT_TRUE(font);
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_CreateTextObj(document(), font.get(), 20.0f);
  EXPECT_TRUE(text_object);

  // Same as `text` in the EmbedNotoSansSCFont test case above.
  const std::vector<uint32_t> charcodes = {9, 6, 7, 3, 5, 2, 1,
                                           9, 6, 7, 4, 8, 2};
  EXPECT_TRUE(
      FPDFText_SetCharcodes(text_object, charcodes.data(), charcodes.size()));

  const FS_MATRIX matrix{1, 0, 0, 1, 50, 200};
  ASSERT_TRUE(FPDFPageObj_TransformF(text_object, &matrix));
  FPDFPage_InsertObject(page.get(), text_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
  CompareBitmap(page_bitmap.get(), 400, 400, NotoSansSCChecksum());

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(400, 400, NotoSansSCChecksum());
}

TEST_F(FPDFEditEmbedderTest, Bug2094) {
  CreateEmptyDocument();
  ScopedFPDFPage page(FPDFPage_New(document(), 0, 400, 400));
  std::string font_path = PathService::GetTestFilePath("fonts/bug_2094.ttf");
  ASSERT_FALSE(font_path.empty());

  std::vector<uint8_t> font_data = GetFileContents(font_path.c_str());
  ASSERT_FALSE(font_data.empty());

  ScopedFPDFFont font(FPDFText_LoadFont(document(), font_data.data(),
                                        font_data.size(), FPDF_FONT_TRUETYPE,
                                        /*cid=*/true));
  EXPECT_TRUE(font);
}

TEST_F(FPDFEditEmbedderTest, EmptyCreation) {
  CreateEmptyDocument();
  FPDF_PAGE page = FPDFPage_New(document(), 0, 640.0, 480.0);
  EXPECT_TRUE(page);
  // The FPDFPage_GenerateContent call should do nothing.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  EXPECT_THAT(GetString(), testing::MatchesRegex(std::string(
                               kExpectedPDF, sizeof(kExpectedPDF))));
  FPDF_ClosePage(page);
}

// Regression test for https://crbug.com/667012
TEST_F(FPDFEditEmbedderTest, RasterizePDF) {
  const char kAllBlackChecksum[] = "5708fc5c4a8bd0abde99c8e8f0390615";

  // Get the bitmap for the original document.
  ScopedFPDFBitmap orig_bitmap;
  {
    ASSERT_TRUE(OpenDocument("black.pdf"));
    ScopedPage orig_page = LoadScopedPage(0);
    ASSERT_TRUE(orig_page);
    orig_bitmap = RenderLoadedPage(orig_page.get());
    CompareBitmap(orig_bitmap.get(), 612, 792, kAllBlackChecksum);
  }

  // Create a new document from |orig_bitmap| and save it.
  {
    ScopedFPDFDocument temp_doc(FPDF_CreateNewDocument());
    ScopedFPDFPage temp_page(FPDFPage_New(temp_doc.get(), 0, 612, 792));

    // Add the bitmap to an image object and add the image object to the output
    // page.
    ScopedFPDFPageObject temp_img(FPDFPageObj_NewImageObj(temp_doc.get()));
    FPDF_PAGE pages_array[] = {temp_page.get()};
    EXPECT_TRUE(FPDFImageObj_SetBitmap(pages_array, 1, temp_img.get(),
                                       orig_bitmap.get()));
    static constexpr FS_MATRIX kLetterScaleMatrix{612, 0, 0, 792, 0, 0};
    EXPECT_TRUE(FPDFPageObj_SetMatrix(temp_img.get(), &kLetterScaleMatrix));
    FPDFPage_InsertObject(temp_page.get(), temp_img.release());
    EXPECT_TRUE(FPDFPage_GenerateContent(temp_page.get()));
    EXPECT_TRUE(FPDF_SaveAsCopy(temp_doc.get(), this, 0));
  }

  // Get the generated content. Make sure it is at least as big as the original
  // PDF.
  EXPECT_GT(GetString().size(), 923u);
  VerifySavedDocument(612, 792, kAllBlackChecksum);
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
  CompareFS_MATRIX(kMatrix, matrix);

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
  EXPECT_FALSE(FPDFPath_GetPathSegment(black_path, 3));

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
  {
    const char* blue_path_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
        return "ed14c60702b1489c597c7d46ece7f86d";
      }
      return "9823e1a21bd9b72b6a442ba4f12af946";
    }();
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, blue_path_checksum);
  }

  // Now save the result, closing the page and document.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  // Render the saved result. The checksum will change due to floating point
  // precision error.
  {
    const char* last_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
        return "ed14c60702b1489c597c7d46ece7f86d";
      }
      return "9823e1a21bd9b72b6a442ba4f12af946";
    }();
    VerifySavedDocument(612, 792, last_checksum);
  }
}

TEST_F(FPDFEditEmbedderTest, ClipPath) {
  // Load document with a clipped rectangle.
  ASSERT_TRUE(OpenDocument("clip_path.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT triangle = FPDFPage_GetObject(page.get(), 0);
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
}

TEST_F(FPDFEditEmbedderTest, Bug1399) {
  // Load document with a clipped rectangle.
  ASSERT_TRUE(OpenDocument("bug_1399.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(7, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 0);
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
}

TEST_F(FPDFEditEmbedderTest, Bug1549) {
  static const char kOriginalChecksum[] = "126366fb95e6caf8ea196780075b22b2";
  static const char kRemovedChecksum[] = "6ec2f27531927882624b37bc7d8e12f4";

  ASSERT_TRUE(OpenDocument("bug_1549.pdf"));
  ScopedPage page = LoadScopedPage(0);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 100, 150, kOriginalChecksum);

    ScopedFPDFPageObject obj(FPDFPage_GetObject(page.get(), 0));
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj.get()));
    ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), obj.get()));
  }

  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 100, 150, kRemovedChecksum);
  }

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // TODO(crbug.com/pdfium/1549): Should be `kRemovedChecksum`.
  VerifySavedDocument(100, 150, "4f9889cd5993db20f1ab37d677ac8d26");
}

TEST_F(FPDFEditEmbedderTest, SetText) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Get the "Hello, world!" text object and change it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  ScopedFPDFWideString text1 = GetFPDFWideString(L"Changed for SetText test");
  EXPECT_TRUE(FPDFText_SetText(page_object, text1.get()));

  // Verify the "Hello, world!" text is gone and "Changed for SetText test" is
  // now displayed.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

  const char* changed_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "e1c530ca0705424f19a1b7ff0bffdbaa";
#elif BUILDFLAG(IS_APPLE)
      return "c65881cb16125c23e5513a16dc68f3a2";
#else
      return "4a8345a139507932729e07d4831cbe2b";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "b720e83476fd6819d47c533f1f43c728";
#else
    return "9a85b9354a69c61772ed24151c140f46";
#endif
  }();
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, changed_checksum);
  }

  // Now save the result.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and check the changes were kept in the saved .pdf.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(2, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, changed_checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, SetCharcodesBadParams) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);

  const uint32_t kDummyValue = 42;
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, nullptr, 0));
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, nullptr, 1));
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, &kDummyValue, 0));
  EXPECT_FALSE(FPDFText_SetCharcodes(nullptr, &kDummyValue, 1));
  EXPECT_FALSE(FPDFText_SetCharcodes(page_object, nullptr, 1));
}

TEST_F(FPDFEditEmbedderTest, SetTextKeepClippingPath) {
  // Load document with some text, with parts clipped.
  ASSERT_TRUE(OpenDocument("bug_1558.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  const char* original_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "0822ec5d476e8371544ef4bb7a0596e1";
#elif BUILDFLAG(IS_APPLE)
      return "721dae4e2258a52a000af88d09ec75ca";
#else
      return "3c04e3acc732faaf39fb0c19efd056ac";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "ae7a25c85e0e2dd0c5cb9dd5cd37f6df";
#else
    return "7af7fe5b281298261eb66ac2d22f5054";
#endif
  }();
  {
    // When opened before any editing and saving, the clipping path is rendered.
    ScopedFPDFBitmap original_bitmap = RenderPage(page.get());
    CompareBitmap(original_bitmap.get(), 200, 200, original_checksum);
  }

  // "Change" the text in the objects to their current values to force them to
  // regenerate when saving.
  {
    ScopedFPDFTextPage text_page(FPDFText_LoadPage(page.get()));
    ASSERT_TRUE(text_page);
    const int obj_count = FPDFPage_CountObjects(page.get());
    ASSERT_EQ(2, obj_count);
    for (int i = 0; i < obj_count; ++i) {
      FPDF_PAGEOBJECT text_obj = FPDFPage_GetObject(page.get(), i);
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
    ScopedFPDFBitmap edited_bitmap = RenderPage(page.get());
    CompareBitmap(edited_bitmap.get(), 200, 200, original_checksum);
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Open the saved copy and render it.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  {
    ScopedFPDFBitmap saved_bitmap = RenderSavedPage(saved_page);
    CompareBitmap(saved_bitmap.get(), 200, 200, original_checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, Bug1574) {
  // Load document with some text within a clipping path.
  ASSERT_TRUE(OpenDocument("bug_1574.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  const char* original_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "6f22adb3ba2a2c60a940bfb52e14dd58";
#elif BUILDFLAG(IS_APPLE)
      return "afa2260cbe84be78867940d72420d0b4";
#else
      return "d76a31d931a350f0809226a41029a9a4";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "1226bc2b8072622eb28f52321876e814";
#else
    return "c5241eef60b9eac68ed1f2a5fd002703";
#endif
  }();
  {
    // When opened before any editing and saving, the text object is rendered.
    ScopedFPDFBitmap original_bitmap = RenderPage(page.get());
    CompareBitmap(original_bitmap.get(), 200, 300, original_checksum);
  }

  // "Change" the text in the objects to their current values to force them to
  // regenerate when saving.
  {
    ScopedFPDFTextPage text_page(FPDFText_LoadPage(page.get()));
    ASSERT_TRUE(text_page);

    ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
    FPDF_PAGEOBJECT text_obj = FPDFPage_GetObject(page.get(), 1);
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
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Open the saved copy and render it.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  {
    ScopedFPDFBitmap saved_bitmap = RenderSavedPage(saved_page);
    CompareBitmap(saved_bitmap.get(), 200, 300, original_checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, Bug1893) {
  ASSERT_TRUE(OpenDocument("bug_1893.pdf"));
  ScopedPage page = LoadScopedPage(0);
  {
    const char* original_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "10c8257bb54b4431196d963d68d45f12";
#elif BUILDFLAG(IS_APPLE)
        return "c42eef2028cb86a0a8601d61707b126f";
#else
        return "d8be4379e729242785945458924318a3";
#endif
      }
#if BUILDFLAG(IS_APPLE)
      return "0964322399241618539b474dbf9d40c6";
#else
      return "c3672f206e47d98677401f1617ad56eb";
#endif
    }();

    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 200, 300, original_checksum);
  }

  EXPECT_EQ(3, FPDFPage_CountObjects(page.get()));

  const char* removed_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "95484d03b9da898617f297b1429f7f84";
#elif BUILDFLAG(IS_APPLE)
      return "7222709eca0e8f72a66ce06283f7c10f";
#else
      return "4a02191e033dddeb2110d55af3f14544";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "d0837f2b8809a5902d3c4219441fbafe";
#else
    return "e9c0cbd6adcb2151b4e36a61ab26a20a";
#endif
  }();

  // Remove the underline and regenerate the page content.
  {
    ScopedFPDFPageObject object(FPDFPage_GetObject(page.get(), 0));
    ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), object.get()));
    ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));

    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 200, 300, removed_checksum);
  }

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ASSERT_TRUE(OpenSavedDocument());
    FPDF_PAGE saved_page = LoadSavedPage(0);
    ScopedFPDFBitmap bitmap = RenderSavedPageWithFlags(saved_page, FPDF_ANNOT);
    CompareBitmap(bitmap.get(), 200, 300, removed_checksum);
    CloseSavedPage(saved_page);
    CloseSavedDocument();
  }
}

TEST_F(FPDFEditEmbedderTest, RemoveTextObject) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Show what the original file looks like.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Check the initial state of the text page as well. `text_page` must be freed
  // before calling FPDFPage_RemoveObject() below, so ASAN does not report
  // dangling pointers.
  {
    ScopedFPDFTextPage text_page(FPDFText_LoadPage(page.get()));
    ASSERT_TRUE(text_page);
    EXPECT_EQ(30, FPDFText_CountChars(text_page.get()));
    EXPECT_EQ(0, FPDFText_GetFontWeight(text_page.get(), 0));
  }

  // Get the "Hello, world!" text object and remove it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  {
    ScopedFPDFPageObject page_object(FPDFPage_GetObject(page.get(), 0));
    ASSERT_TRUE(page_object);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(page_object.get()));
    EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object.get()));
  }
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));

  // Verify the "Hello, world!" text is gone.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, FirstRemovedChecksum());
  }

  // Create a new text page, which has updated results.
  {
    ScopedFPDFTextPage text_page(FPDFText_LoadPage(page.get()));
    ASSERT_TRUE(text_page);
    EXPECT_EQ(15, FPDFText_CountChars(text_page.get()));
    EXPECT_EQ(0, FPDFText_GetFontWeight(text_page.get(), 0));
  }

  // Verify the rendering again after calling FPDFPage_GenerateContent().
  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, FirstRemovedChecksum());
  }

  // Save the document and verify it after reloading.
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(200, 200, FirstRemovedChecksum());

  // Verify removed/renamed resources are no longer there.
  EXPECT_THAT(GetString(), Not(HasSubstr("/F1")));
  EXPECT_THAT(GetString(), Not(HasSubstr("/F2")));
  EXPECT_THAT(GetString(), Not(HasSubstr("/Times-Roman")));
}

TEST_F(FPDFEditEmbedderTest,
       RemoveTextObjectWithTwoPagesSharingContentStreamAndResources) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_2_pages.pdf"));
  ScopedPage page1 = LoadScopedPage(0);
  ASSERT_TRUE(page1);
  ScopedPage page2 = LoadScopedPage(1);
  ASSERT_TRUE(page2);

  // Show what the original file looks like.
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, HelloWorldChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Get the "Hello, world!" text object from page 1 and remove it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page1.get()));
  {
    ScopedFPDFPageObject page_object(FPDFPage_GetObject(page1.get(), 0));
    ASSERT_TRUE(page_object);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(page_object.get()));
    EXPECT_TRUE(FPDFPage_RemoveObject(page1.get(), page_object.get()));
  }
  ASSERT_EQ(1, FPDFPage_CountObjects(page1.get()));

  // Verify the "Hello, world!" text is gone from page 1.
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, FirstRemovedChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Verify the rendering again after calling FPDFPage_GenerateContent().
  ASSERT_TRUE(FPDFPage_GenerateContent(page1.get()));
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, FirstRemovedChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Save the document and verify it after reloading.
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page1 = LoadSavedPage(0);
  VerifySavedRendering(saved_page1, 200, 200, FirstRemovedChecksum());
  CloseSavedPage(saved_page1);
  FPDF_PAGE saved_page2 = LoadSavedPage(1);
  VerifySavedRendering(saved_page2, 200, 200, HelloWorldChecksum());
  CloseSavedPage(saved_page2);
  CloseSavedDocument();

  std::vector<std::string> split_saved_data = StringSplit(GetString(), '\n');
  // Verify removed/renamed resources are in the save PDF the correct number of
  // times.
  EXPECT_THAT(split_saved_data, Contains(HasSubstr("/F1")).Times(1));
  EXPECT_THAT(split_saved_data, Contains(HasSubstr("/F2")).Times(1));
  EXPECT_THAT(split_saved_data, Contains(HasSubstr("/Times-Roman")).Times(1));
}

TEST_F(FPDFEditEmbedderTest,
       RemoveTextObjectWithTwoPagesSharingContentArrayAndResources) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_2_pages_split_streams.pdf"));
  ScopedPage page1 = LoadScopedPage(0);
  ASSERT_TRUE(page1);
  ScopedPage page2 = LoadScopedPage(1);
  ASSERT_TRUE(page2);

  // Show what the original file looks like.
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, HelloWorldChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Get the "Hello, world!" text object from page 1 and remove it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page1.get()));
  {
    ScopedFPDFPageObject page_object(FPDFPage_GetObject(page1.get(), 0));
    ASSERT_TRUE(page_object);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(page_object.get()));
    EXPECT_TRUE(FPDFPage_RemoveObject(page1.get(), page_object.get()));
  }
  ASSERT_EQ(1, FPDFPage_CountObjects(page1.get()));

  // Verify the "Hello, world!" text is gone from page 1.
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, FirstRemovedChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Verify the rendering again after calling FPDFPage_GenerateContent().
  ASSERT_TRUE(FPDFPage_GenerateContent(page1.get()));
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, FirstRemovedChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Save the document and verify it after reloading.
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page1 = LoadSavedPage(0);
  VerifySavedRendering(saved_page1, 200, 200, FirstRemovedChecksum());
  CloseSavedPage(saved_page1);
  FPDF_PAGE saved_page2 = LoadSavedPage(1);
  VerifySavedRendering(saved_page2, 200, 200, HelloWorldChecksum());
  CloseSavedPage(saved_page2);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveTextObjectWithTwoPagesSharingResourcesDict) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_2_pages_shared_resources_dict.pdf"));
  ScopedPage page1 = LoadScopedPage(0);
  ASSERT_TRUE(page1);
  ScopedPage page2 = LoadScopedPage(1);
  ASSERT_TRUE(page2);

  // Show what the original file looks like.
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, HelloWorldChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Get the "Hello, world!" text object from page 1 and remove it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page1.get()));
  {
    ScopedFPDFPageObject page_object(FPDFPage_GetObject(page1.get(), 0));
    ASSERT_TRUE(page_object);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(page_object.get()));
    EXPECT_TRUE(FPDFPage_RemoveObject(page1.get(), page_object.get()));
  }
  ASSERT_EQ(1, FPDFPage_CountObjects(page1.get()));

  // Verify the "Hello, world!" text is gone from page 1
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, FirstRemovedChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Verify the rendering again after calling FPDFPage_GenerateContent().
  ASSERT_TRUE(FPDFPage_GenerateContent(page1.get()));
  {
    ScopedFPDFBitmap page1_bitmap = RenderPage(page1.get());
    CompareBitmap(page1_bitmap.get(), 200, 200, FirstRemovedChecksum());
    ScopedFPDFBitmap page2_bitmap = RenderPage(page2.get());
    CompareBitmap(page2_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Save the document and verify it after reloading.
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page1 = LoadSavedPage(0);
  VerifySavedRendering(saved_page1, 200, 200, FirstRemovedChecksum());
  CloseSavedPage(saved_page1);
  FPDF_PAGE saved_page2 = LoadSavedPage(1);
  VerifySavedRendering(saved_page2, 200, 200, HelloWorldChecksum());
  CloseSavedPage(saved_page2);
  CloseSavedDocument();
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

      FPDF_WCHAR buffer[128];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name = GetPlatformWString(buffer);
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
        std::wstring key = GetPlatformWString(buffer);
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
        std::wstring key = GetPlatformWString(buffer);
        EXPECT_EQ(L"Position", key);

        EXPECT_EQ(FPDF_OBJECT_STRING,
                  FPDFPageObjMark_GetParamValueType(mark, "Position"));
        unsigned long length;
        EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(
            mark, "Position", buffer, sizeof(buffer), &length));
        ASSERT_GT(length, 0u);
        std::wstring value = GetPlatformWString(buffer);

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

TEST_F(FPDFEditEmbedderTest, GetMarkedContentId) {
  ASSERT_TRUE(OpenDocument("tagged_marked_content.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static constexpr int kExpectedObjectCount = 4;
  ASSERT_EQ(kExpectedObjectCount, FPDFPage_CountObjects(page.get()));
  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);
    ASSERT_TRUE(page_object);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(page_object));
    EXPECT_EQ(i, FPDFPageObj_GetMarkedContentID(page_object));
  }

  // Negative testing.
  EXPECT_EQ(-1, FPDFPageObj_GetMarkedContentID(nullptr));
}

TEST_F(FPDFEditEmbedderTest, ReadMarkedObjectsIndirectDict) {
  // Load document with some text marked with an indirect property.
  ASSERT_TRUE(OpenDocument("text_in_page_marked_indirect.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  CheckMarkCounts(page.get(), 1, 19, 8, 4, 9, 1);
}

TEST_F(FPDFEditEmbedderTest, RemoveMarkedObjectsPrime) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Show what the original file looks like.
  {
    const char* original_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "cefa45d13f92fb761251661a2c889157";
#elif BUILDFLAG(IS_APPLE)
        return "b2044dc5b49fdca723d74bd6277df689";
#else
        return "efc2206b313fff03be8e701907322b06";
#endif
      }
#if BUILDFLAG(IS_APPLE)
#ifdef ARCH_CPU_ARM64
      return "401858d37db450bfd3f9458ac490eb08";
#else
      return "7c898d207b5f9bc7843d4ef93349bf71";
#endif  // ARCH_CPU_ARM64
#else
      return "3d5a3de53d5866044c2b6bf339742c97";
#endif  // BUILDFLAG(IS_APPLE)
    }();
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, original_checksum);
  }

  static constexpr int expected_object_count = 19;
  CheckMarkCounts(page.get(), 1, expected_object_count, 8, 4, 9, 1);

  // Get all objects marked with "Prime"
  std::vector<FPDF_PAGEOBJECT> primes;
  for (int i = 0; i < expected_object_count; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = 0; j < mark_count; ++j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      FPDF_WCHAR buffer[128];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name = GetPlatformWString(buffer);
      if (name == L"Prime") {
        primes.push_back(page_object);
      }
    }
  }

  // Remove all objects marked with "Prime".
  for (FPDF_PAGEOBJECT page_object : primes) {
    EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
    FPDFPageObj_Destroy(page_object);
  }

  EXPECT_EQ(11, FPDFPage_CountObjects(page.get()));
  const char* non_primes_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "690c7d4c7850fbe726c2299208425f4f";
#elif BUILDFLAG(IS_APPLE)
      return "427228e73125ede1050a641cd9b9c8ec";
#else
      return "10a6558c9e40ea837922e6f2882a2d57";
#endif
    }
#if BUILDFLAG(IS_APPLE)
#ifdef ARCH_CPU_ARM64
    return "6a1e31ffe451997946e449250b97d5b2";
#else
    return "727b1ea388b2374270f21d35d1fae70e";
#endif  // ARCH_CPU_ARM64
#else
    return "bc8623c052f12376c3d8dd09a6cd27df";
#endif  // BUILDFLAG(IS_APPLE)
  }();
  // TODO(thestig): Should `non_primes_checksum` and
  // `non_primes_after_save_checksum` be merged together?
  const char* non_primes_after_save_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "690c7d4c7850fbe726c2299208425f4f";
#elif BUILDFLAG(IS_APPLE)
      return "427228e73125ede1050a641cd9b9c8ec";
#else
      return "10a6558c9e40ea837922e6f2882a2d57";
#endif
    }
#if BUILDFLAG(IS_APPLE)
#ifdef ARCH_CPU_ARM64
    return "d250bee3658c74e5d74729a09cbd80cd";
#else
    return "727b1ea388b2374270f21d35d1fae70e";
#endif  // ARCH_CPU_ARM64
#else
    return "bc8623c052f12376c3d8dd09a6cd27df";
#endif  // BUILDFLAG(IS_APPLE)
  }();
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, non_primes_checksum);
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and check the prime marks are not there anymore.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(11, FPDFPage_CountObjects(saved_page));

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, non_primes_after_save_checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveMarks) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static constexpr int kExpectedObjectCount = 19;
  CheckMarkCounts(page.get(), 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Remove all "Prime" content marks.
  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = mark_count - 1; j >= 0; --j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      FPDF_WCHAR buffer[128];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name = GetPlatformWString(buffer);
      if (name == L"Prime") {
        // Remove mark.
        EXPECT_TRUE(FPDFPageObj_RemoveMark(page_object, mark));

        // Verify there is now one fewer mark in the page object.
        EXPECT_EQ(mark_count - 1, FPDFPageObj_CountMarks(page_object));
      }
    }
  }

  // Verify there are 0 "Prime" content marks now.
  CheckMarkCounts(page.get(), 1, kExpectedObjectCount, 0, 4, 9, 1);

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static constexpr int kExpectedObjectCount = 19;
  CheckMarkCounts(page.get(), 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Remove all "Square" content marks parameters.
  for (int i = 0; i < kExpectedObjectCount; ++i) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);

    int mark_count = FPDFPageObj_CountMarks(page_object);
    for (int j = 0; j < mark_count; ++j) {
      FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, j);

      FPDF_WCHAR buffer[128];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name = GetPlatformWString(buffer);
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
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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

      FPDF_WCHAR buffer[128];
      unsigned long name_len = 999u;
      ASSERT_TRUE(
          FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
      EXPECT_GT(name_len, 0u);
      EXPECT_NE(999u, name_len);
      std::wstring name = GetPlatformWString(buffer);
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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Iterate over all objects, counting the number of times each content mark
  // name appears.
  CheckMarkCounts(page.get(), 1, 19, 8, 4, 9, 1);

  // Remove first page object.
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
  FPDFPageObj_Destroy(page_object);

  CheckMarkCounts(page.get(), 2, 18, 8, 3, 9, 1);

  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Iterate over all objects, counting the number of times each content mark
  // name appears.
  CheckMarkCounts(page.get(), 1, 19, 8, 4, 9, 1);

  // Remove first page object.
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
  FPDFPageObj_Destroy(page_object);

  CheckMarkCounts(page.get(), 2, 18, 8, 3, 9, 1);

  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Get the "Hello, world!" text object and remove it.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));

  // Verify the "Hello, world!" text is gone.
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Get the "Hello, world!" text object and remove it. There is another object
  // in the same stream that says "Goodbye, world!"
  ASSERT_EQ(3, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));

  // Verify the "Hello, world!" text is gone.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  const char* hello_removed_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "48b5524e20e942d2a8f7e15611968cc7";
#elif BUILDFLAG(IS_APPLE)
      return "5b9d1dee233eb9d51e23a36c6c631443";
#else
      return "204c11472f5b93719487de7b9c1b1c93";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "5508c2f06d104050f74f655693e38c2c";
#else
    return "a8cd82499cf744e0862ca468c9d4ceb8";
#endif
  }();
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, hello_removed_checksum);
  }

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDFPageObj_Destroy(page_object);

  // Re-open the file and check the page object count is still 2.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(2, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, hello_removed_checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveExistingPageObjectSplitStreamsLonely) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_split_streams.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Get the "Greetings, world!" text object and remove it. This is the only
  // object in the stream.
  ASSERT_EQ(3, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 2);
  ASSERT_TRUE(page_object);
  EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));

  // Verify the "Greetings, world!" text is gone.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDFPageObj_Destroy(page_object);

  // Re-open the file and check the page object count is still 2.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(2, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldChecksum());
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, GetContentStream) {
  // Load document with some text split across streams.
  ASSERT_TRUE(OpenDocument("split_streams.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-14.
  // Content stream 1: page objects 15-17.
  // Content stream 2: page object 18.
  ASSERT_EQ(19, FPDFPage_CountObjects(page.get()));
  for (int i = 0; i < 19; i++) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    if (i < 15) {
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    } else if (i < 18) {
      EXPECT_EQ(1, cpdf_page_object->GetContentStream()) << i;
    } else {
      EXPECT_EQ(2, cpdf_page_object->GetContentStream()) << i;
    }
  }
}

TEST_F(FPDFEditEmbedderTest, RemoveAllFromStream) {
  // Load document with some text split across streams.
  ASSERT_TRUE(OpenDocument("split_streams.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-14.
  // Content stream 1: page objects 15-17.
  // Content stream 2: page object 18.
  ASSERT_EQ(19, FPDFPage_CountObjects(page.get()));

  // Loop backwards because objects will being removed, which shifts the indexes
  // after the removed position.
  for (int i = 18; i >= 0; i--) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);

    // Empty content stream 1.
    if (cpdf_page_object->GetContentStream() == 1) {
      EXPECT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
      FPDFPageObj_Destroy(page_object);
    }
  }

  // Content stream 0: page objects 0-14.
  // Content stream 2: page object 15.
  ASSERT_EQ(16, FPDFPage_CountObjects(page.get()));
  for (int i = 0; i < 16; i++) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    if (i < 15) {
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    } else {
      EXPECT_EQ(2, cpdf_page_object->GetContentStream()) << i;
    }
  }

  // Generate contents should remove the empty stream and update the page
  // objects' contents stream indexes.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  // Content stream 0: page objects 0-14.
  // Content stream 1: page object 15.
  ASSERT_EQ(16, FPDFPage_CountObjects(page.get()));
  for (int i = 0; i < 16; i++) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    if (i < 15) {
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    } else {
      EXPECT_EQ(1, cpdf_page_object->GetContentStream()) << i;
    }
  }

  const char* stream1_removed_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "d7e6debf2dc02de449860ee8012a18d2";
#elif BUILDFLAG(IS_APPLE)
      return "b26ac6d9bef9c756a19a9aafc60709bd";
#else
      return "0b3ef335b8d86a3f9d609368b9d075e0";
#endif
    }
#if BUILDFLAG(IS_APPLE)
#if ARCH_CPU_ARM64
    return "a47297bbcfa01e27891eeb52375b6f9e";
#else
    return "1c1d478b59e3e63813f0f56124564f48";
#endif  // ARCH_CPU_ARM64
#else
    return "b474826df1acedb05c7b82e1e49e64a6";
#endif  // BUILDFLAG(IS_APPLE)
  }();
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, stream1_removed_checksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
    if (i < 15) {
      EXPECT_EQ(0, cpdf_page_object->GetContentStream()) << i;
    } else {
      EXPECT_EQ(1, cpdf_page_object->GetContentStream()) << i;
    }
  }

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, stream1_removed_checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveAllFromSingleStream) {
  // Load document with a single stream.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

  // Loop backwards because objects will being removed, which shifts the indexes
  // after the removed position.
  for (int i = 1; i >= 0; i--) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);
    ASSERT_TRUE(page_object);
    CPDF_PageObject* cpdf_page_object =
        CPDFPageObjectFromFPDFPageObject(page_object);
    ASSERT_EQ(0, cpdf_page_object->GetContentStream());
    ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
    FPDFPageObj_Destroy(page_object);
  }

  // No more objects in the stream
  ASSERT_EQ(0, FPDFPage_CountObjects(page.get()));

  // Generate contents should remove the empty stream and update the page
  // objects' contents stream indexes.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  ASSERT_EQ(0, FPDFPage_CountObjects(page.get()));

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, kBlankPage200By200Checksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and check the page object count is still 0.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(0, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kBlankPage200By200Checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveFirstFromSingleStream) {
  // Load document with a single stream.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

  // Remove first object.
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  CPDF_PageObject* cpdf_page_object =
      CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());
  ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
  FPDFPageObj_Destroy(page_object);

  // One object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));
  page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  // Still one object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));
  page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, FirstRemovedChecksum());
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
    CompareBitmap(page_bitmap.get(), 200, 200, FirstRemovedChecksum());
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveLastFromSingleStream) {
  // Load document with a single stream.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

  // Remove last object
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 1);
  ASSERT_TRUE(page_object);
  CPDF_PageObject* cpdf_page_object =
      CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());
  ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
  FPDFPageObj_Destroy(page_object);

  // One object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));
  page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  // Still one object left in the stream.
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));
  page_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(page_object);
  cpdf_page_object = CPDFPageObjectFromFPDFPageObject(page_object);
  ASSERT_EQ(0, cpdf_page_object->GetContentStream());

  using pdfium::HelloWorldRemovedChecksum;
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldRemovedChecksum());
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldRemovedChecksum());
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, RemoveAllFromMultipleStreams) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world_split_streams.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Content stream 0: page objects 0-1.
  // Content stream 1: page object 2.
  ASSERT_EQ(3, FPDFPage_CountObjects(page.get()));

  // Loop backwards because objects will being removed, which shifts the indexes
  // after the removed position.
  for (int i = 2; i >= 0; i--) {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), i);
    ASSERT_TRUE(page_object);
    ASSERT_TRUE(FPDFPage_RemoveObject(page.get(), page_object));
    FPDFPageObj_Destroy(page_object);
  }

  // No more objects in the page.
  ASSERT_EQ(0, FPDFPage_CountObjects(page.get()));

  // Generate contents should remove the empty streams and update the page
  // objects' contents stream indexes.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  ASSERT_EQ(0, FPDFPage_CountObjects(page.get()));

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, kBlankPage200By200Checksum);
  }

  // Save the file
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and check the page object count is still 0.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  EXPECT_EQ(0, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, kBlankPage200By200Checksum);
  }

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, InsertPageObjectAndSave) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Add a red rectangle.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page.get(), red_rect);

  // Verify the red rectangle was added.
  ASSERT_EQ(3, FPDFPage_CountObjects(page.get()));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Add a red rectangle.
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 100, 100, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page.get(), red_rect);

  // Verify the red rectangle was added.
  ASSERT_EQ(3, FPDFPage_CountObjects(page.get()));

  // Generate content but change it again
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and check the page object count is still 3.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(3, FPDFPage_CountObjects(saved_page));
  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, InsertObjectAtIndex) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ASSERT_TRUE(doc);
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  ASSERT_TRUE(page);

  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT img1 = FPDFPageObj_NewImageObj(doc.get());
  FPDF_PAGEOBJECT img2 = FPDFPageObj_NewImageObj(doc.get());
  FPDF_PAGEOBJECT img3 = FPDFPageObj_NewImageObj(doc.get());
  ASSERT_TRUE(img1);
  ASSERT_TRUE(img2);
  ASSERT_TRUE(img3);

  EXPECT_TRUE(FPDFPage_InsertObjectAtIndex(page.get(), img1, 0));
  EXPECT_TRUE(FPDFPage_InsertObjectAtIndex(page.get(), img2, 0));
  EXPECT_TRUE(FPDFPage_InsertObjectAtIndex(page.get(), img3, 1));

  EXPECT_EQ(3, FPDFPage_CountObjects(page.get()));

  EXPECT_EQ(img2, FPDFPage_GetObject(page.get(), 0));
  EXPECT_EQ(img3, FPDFPage_GetObject(page.get(), 1));
  EXPECT_EQ(img1, FPDFPage_GetObject(page.get(), 2));

  // Test invalid index
  FPDF_PAGEOBJECT img4 = FPDFPageObj_NewImageObj(doc.get());
  ASSERT_TRUE(img4);
  EXPECT_FALSE(FPDFPage_InsertObjectAtIndex(page.get(), img4, 4));

  EXPECT_EQ(3, FPDFPage_CountObjects(page.get()));
  EXPECT_EQ(img1, FPDFPage_GetObject(page.get(), 2));

  // inserting at the end
  FPDF_PAGEOBJECT img5 = FPDFPageObj_NewImageObj(doc.get());
  ASSERT_TRUE(img5);
  EXPECT_TRUE(FPDFPage_InsertObjectAtIndex(page.get(), img5, 3));
  EXPECT_EQ(4, FPDFPage_CountObjects(page.get()));
  EXPECT_EQ(img5, FPDFPage_GetObject(page.get(), 3));

  FPDF_PAGEOBJECT img6 = FPDFPageObj_NewImageObj(doc.get());
  ASSERT_TRUE(img6);
  EXPECT_FALSE(FPDFPage_InsertObjectAtIndex(nullptr, img6, 0));
}

TEST_F(FPDFEditEmbedderTest, InsertAndRemoveLargeFile) {
  const int kOriginalObjectCount = 600;

  // Load document with many objects.
  ASSERT_TRUE(OpenDocument("many_rectangles.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  using pdfium::ManyRectanglesChecksum;
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 300, ManyRectanglesChecksum());
  }

  // Add a black rectangle.
  ASSERT_EQ(kOriginalObjectCount, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT black_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(black_rect, 0, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(black_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page.get(), black_rect);

  // Verify the black rectangle was added.
  ASSERT_EQ(kOriginalObjectCount + 1, FPDFPage_CountObjects(page.get()));
  const char* plus_rectangle_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "0d3715fcfb9bd0dd25dcce60800bff47";
    }
    return "6b9396ab570754b32b04ca629e902f77";
  }();
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 300, plus_rectangle_checksum);
  }

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and check the rectangle added is still there.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  EXPECT_EQ(kOriginalObjectCount + 1, FPDFPage_CountObjects(saved_page));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 300, plus_rectangle_checksum);
  }

  // Remove the added rectangle.
  FPDF_PAGEOBJECT added_object =
      FPDFPage_GetObject(saved_page, kOriginalObjectCount);
  EXPECT_TRUE(FPDFPage_RemoveObject(saved_page, added_object));
  FPDFPageObj_Destroy(added_object);
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 300, ManyRectanglesChecksum());
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
    CompareBitmap(page_bitmap.get(), 200, 300, ManyRectanglesChecksum());
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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Add an opaque rectangle on top of some of the text.
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(20, 100, 50, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page.get(), red_rect);

  // Add a transparent triangle on top of other part of the text.
  FPDF_PAGEOBJECT black_path = FPDFPageObj_CreateNewPath(20, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(black_path, 0, 0, 0, 100));
  EXPECT_TRUE(FPDFPath_SetDrawMode(black_path, FPDF_FILLMODE_ALTERNATE, 0));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 30, 80));
  EXPECT_TRUE(FPDFPath_LineTo(black_path, 40, 10));
  EXPECT_TRUE(FPDFPath_Close(black_path));
  FPDFPage_InsertObject(page.get(), black_path);

  // Render and check the result.
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "a78103bc6e22f52d17da01057370c14c";
#elif BUILDFLAG(IS_APPLE)
      return "c2d3b92224759e4c3f0fc5ab2907b974";
#else
      return "c8813a6cd0cbf1e776f11c4b4d6314bf";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "279693baca9f48da2d75a8e289aed58e";
#else
    return "fe415d47945c10b9cc8e9ca08887369e";
#endif
  }();
  CompareBitmap(bitmap.get(), 200, 200, checksum);
}

TEST_F(FPDFEditEmbedderTest, EditOverExistingContent) {
  // Load document with existing content
  ASSERT_TRUE(OpenDocument("bug_717.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Add a transparent rectangle on top of the existing content
  FPDF_PAGEOBJECT red_rect2 = FPDFPageObj_CreateNewRect(90, 700, 25, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect2, 255, 0, 0, 100));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect2, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page.get(), red_rect2);

  // Add an opaque rectangle on top of the existing content
  FPDF_PAGEOBJECT red_rect = FPDFPageObj_CreateNewRect(115, 700, 25, 50);
  EXPECT_TRUE(FPDFPageObj_SetFillColor(red_rect, 255, 0, 0, 255));
  EXPECT_TRUE(FPDFPath_SetDrawMode(red_rect, FPDF_FILLMODE_ALTERNATE, 0));
  FPDFPage_InsertObject(page.get(), red_rect);

  const char* original_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "1982180b50c0c4dac503de293bd5eeb4";
    }
    return "ad04e5bd0f471a9a564fb034bd0fb073";
  }();
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
  CompareBitmap(bitmap.get(), 612, 792, original_checksum);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  // Now save the result, closing the page and document
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);
  VerifySavedRendering(saved_page, 612, 792, original_checksum);

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
  const char* last_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "0556aa9cc5c95087f010699cd1aff87a";
    }
    return "4b5b00f824620f8c9b8801ebb98e1cdd";
  }();
  {
    ScopedFPDFBitmap new_bitmap = RenderSavedPage(saved_page);
    CompareBitmap(new_bitmap.get(), 612, 792, last_checksum);
  }
  EXPECT_TRUE(FPDFPage_GenerateContent(saved_page));

  // Now save the result, closing the page and document
  EXPECT_TRUE(FPDF_SaveAsCopy(saved_document(), this, 0));

  CloseSavedPage(saved_page);
  CloseSavedDocument();

  // Render the saved result
  VerifySavedDocument(612, 792, last_checksum);
}

TEST_F(FPDFEditEmbedderTest, AddStrokedPaths) {
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
    const char* checksum_1 = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
        return "1469acf60e7647ebeb8e1fb08c5d6c7a";
      }
      return "64bd31f862a89e0a9e505a5af6efd506";
    }();
    CompareBitmap(page_bitmap.get(), 612, 792, checksum_1);
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
    const char* checksum_2 = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
        return "c4b2314ce2da802fbb390ea3bb2adae9";
      }
      return "4b6f3b9d25c4e194821217d5016c3724";
    }();
    CompareBitmap(page_bitmap.get(), 612, 792, checksum_2);
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
    const char* checksum_3 = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
        return "a5de6ddefcbae60924bebc99347e460b";
      }
      return "ff3e6a22326754944cc6e56609acd73b";
    }();
    CompareBitmap(page_bitmap.get(), 612, 792, checksum_3);
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
    CompareBitmap(page_bitmap.get(), 612, 792, BottomTextChecksum());

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    VerifySavedDocument(612, 792, BottomTextChecksum());
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
    const char* checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "667f74c7cbf72c75bce303ca2de975a3";
#elif BUILDFLAG(IS_APPLE)
        return "86d51a764615b843465695786e92fec5";
#else
        return "3fa05f8935a43a38a8923e9d5fb94365";
#endif
      }
#if BUILDFLAG(IS_APPLE)
      return "983baaa1f688eff7a14b1bf91c171a1a";
#else
      return "161523e196eb5341604cd73e12c97922";
#endif
    }();
    CompareBitmap(page_bitmap.get(), 612, 792, checksum);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    VerifySavedDocument(612, 792, checksum);
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
    const char* checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "4695b3de213d6795a591f27cd8d86e26";
#elif BUILDFLAG(IS_APPLE)
        return "422f1384c13e95c218498a8f18b9e5a7";
#else
        return "63385a217934d9ee9e17ef4d7f7b2128";
#endif
      }
#if BUILDFLAG(IS_APPLE)
      return "e0b3493c5c16e41d0d892ffb48e63fba";
#else
      return "1fbf772dca8d82b960631e6683934964";
#endif
    }();
    CompareBitmap(page_bitmap.get(), 612, 792, checksum);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
    VerifySavedDocument(612, 792, checksum);
  }

  FS_MATRIX matrix;

  EXPECT_TRUE(FPDFPageObj_GetMatrix(text_object3, &matrix));
  CompareFS_MATRIX({1.0f, 1.5f, 2.0f, 0.5f, 200.0f, 200.0f}, matrix);

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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

  EXPECT_EQ(FPDF_TEXTRENDERMODE_UNKNOWN,
            FPDFTextObj_GetTextRenderMode(nullptr));

  FPDF_PAGEOBJECT fill = FPDFPage_GetObject(page.get(), 0);
  EXPECT_EQ(FPDF_TEXTRENDERMODE_FILL, FPDFTextObj_GetTextRenderMode(fill));

  FPDF_PAGEOBJECT stroke = FPDFPage_GetObject(page.get(), 1);
  EXPECT_EQ(FPDF_TEXTRENDERMODE_STROKE, FPDFTextObj_GetTextRenderMode(stroke));
}

TEST_F(FPDFEditEmbedderTest, SetTextRenderMode) {
  const char* original_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "e17a6453cb48a600f180c5907c4ea02e";
#elif BUILDFLAG(IS_APPLE)
      return "e2d5c32499173c0ff939ad2e7fc01fd6";
#else
      return "48c7f21b2a1a1bbeab24cccccc131e47";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "c488514ce0fc949069ff560407edacd2";
#else
    return "97a4fcf3c9581e19917895631af31d41";
#endif
  }();
  const char* stroke_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "d16eb1bb4748eeb5fb801594da70d519";
    }
    return "e06ee84aeebe926e8c980b7822027e8a";
  }();

  {
    ASSERT_TRUE(OpenDocument("text_render_mode.pdf"));
    ScopedPage page = LoadScopedPage(0);
    ASSERT_TRUE(page);
    ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

    // Check the bitmap
    {
      ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
      CompareBitmap(page_bitmap.get(), 612, 446, original_checksum);
    }

    // Cannot set on a null object.
    EXPECT_FALSE(
        FPDFTextObj_SetTextRenderMode(nullptr, FPDF_TEXTRENDERMODE_UNKNOWN));
    EXPECT_FALSE(
        FPDFTextObj_SetTextRenderMode(nullptr, FPDF_TEXTRENDERMODE_INVISIBLE));

    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
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
      ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
      CompareBitmap(page_bitmap.get(), 612, 446, stroke_checksum);
    }

    // Save a copy.
    EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
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
    CompareBitmap(bitmap.get(), 612, 446, stroke_checksum);

    CloseSavedPage(saved_page);
    CloseSavedDocument();
  }
}

TEST_F(FPDFEditEmbedderTest, TextFontProperties) {
  // bad object tests
  EXPECT_FALSE(FPDFTextObj_GetFont(nullptr));
  EXPECT_EQ(0U, FPDFFont_GetBaseFontName(nullptr, nullptr, 5));
  EXPECT_EQ(0U, FPDFFont_GetFamilyName(nullptr, nullptr, 5));
  EXPECT_EQ(-1, FPDFFont_GetFlags(nullptr));
  EXPECT_EQ(-1, FPDFFont_GetWeight(nullptr));
  EXPECT_FALSE(FPDFFont_GetItalicAngle(nullptr, nullptr));
  EXPECT_FALSE(FPDFFont_GetAscent(nullptr, 12.f, nullptr));
  EXPECT_FALSE(FPDFFont_GetDescent(nullptr, 12.f, nullptr));
  EXPECT_FALSE(FPDFFont_GetGlyphWidth(nullptr, 's', 12.f, nullptr));
  EXPECT_FALSE(FPDFFont_GetGlyphPath(nullptr, 's', 12.f));

  // good object tests
  ASSERT_TRUE(OpenDocument("text_font.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT text = FPDFPage_GetObject(page.get(), 0);
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
    // FPDFFont_GetBaseFontName() positive testing.
    size_t size = FPDFFont_GetBaseFontName(font, nullptr, 0);
    const char kExpectedFontName[] = "LiberationSerif";
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    std::vector<char> font_name(size);
    ASSERT_EQ(size, FPDFFont_GetBaseFontName(font, font_name.data(), size));
    ASSERT_STREQ(kExpectedFontName, font_name.data());

    // FPDFFont_GetBaseFontName() negative testing.
    ASSERT_EQ(0U, FPDFFont_GetBaseFontName(nullptr, nullptr, 0));

    font_name.resize(2);
    font_name[0] = 'x';
    font_name[1] = '\0';
    size = FPDFFont_GetBaseFontName(font, font_name.data(), font_name.size());
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    ASSERT_STREQ("x", font_name.data());
  }

  {
    // FPDFFont_GetFamilyName() positive testing.
    unsigned long size = FPDFFont_GetFamilyName(font, nullptr, 0);
    const char kExpectedFontName[] = "Liberation Serif";
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    std::vector<char> font_name(size);
    ASSERT_EQ(size, FPDFFont_GetFamilyName(font, font_name.data(), size));
    ASSERT_STREQ(kExpectedFontName, font_name.data());

    // FPDFFont_GetFamilyName() negative testing.
    ASSERT_EQ(0U, FPDFFont_GetFamilyName(nullptr, nullptr, 0));

    font_name.resize(2);
    font_name[0] = 'x';
    font_name[1] = '\0';
    size = FPDFFont_GetFamilyName(font, font_name.data(), font_name.size());
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    ASSERT_STREQ("x", font_name.data());
  }

  {
    // FPDFFont_GetFontData() positive testing.
    static constexpr size_t kExpectedSize = 8268;
    std::vector<uint8_t> buf;
    size_t buf_bytes_required = 123;
    ASSERT_TRUE(FPDFFont_GetFontData(font, nullptr, 0, &buf_bytes_required));
    ASSERT_EQ(kExpectedSize, buf_bytes_required);

    buf.resize(kExpectedSize);
    EXPECT_EQ("495800b8e56e2d37f3bc48a1b52db952", GenerateMD5Base16(buf));
    buf_bytes_required = 234;
    // Test with buffer that is too small. Make sure `buf` is unchanged.
    EXPECT_TRUE(FPDFFont_GetFontData(font, buf.data(), buf.size() - 1,
                                     &buf_bytes_required));
    EXPECT_EQ("495800b8e56e2d37f3bc48a1b52db952", GenerateMD5Base16(buf));
    EXPECT_EQ(kExpectedSize, buf_bytes_required);

    // Test with buffer of the correct size.
    buf_bytes_required = 234;
    EXPECT_TRUE(FPDFFont_GetFontData(font, buf.data(), buf.size(),
                                     &buf_bytes_required));
    EXPECT_EQ("1a67be75f719b6c476804d85bb9e4844", GenerateMD5Base16(buf));
    EXPECT_EQ(kExpectedSize, buf_bytes_required);

    // FPDFFont_GetFontData() negative testing.
    EXPECT_FALSE(FPDFFont_GetFontData(nullptr, nullptr, 0, nullptr));
    EXPECT_FALSE(FPDFFont_GetFontData(font, nullptr, 0, nullptr));

    buf_bytes_required = 345;
    EXPECT_FALSE(
        FPDFFont_GetFontData(nullptr, nullptr, 0, &buf_bytes_required));
    EXPECT_EQ(345u, buf_bytes_required);

    EXPECT_FALSE(
        FPDFFont_GetFontData(nullptr, buf.data(), buf.size(), nullptr));
    EXPECT_FALSE(FPDFFont_GetFontData(font, buf.data(), buf.size(), nullptr));

    buf_bytes_required = 345;
    EXPECT_FALSE(FPDFFont_GetFontData(nullptr, buf.data(), buf.size(),
                                      &buf_bytes_required));
    EXPECT_EQ(345u, buf_bytes_required);
  }
  {
    ASSERT_EQ(1, FPDFFont_GetIsEmbedded(font));
    ASSERT_EQ(-1, FPDFFont_GetIsEmbedded(nullptr));
  }
}

TEST_F(FPDFEditEmbedderTest, NoEmbeddedFontData) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

  // Since hello_world.pdf does not embed any font data, FPDFFont_GetFontData()
  // will return the substitution font data. Since pdfium_embeddertest is
  // hermetic, this first object consistently maps to Tinos-Regular.ttf.
  static constexpr size_t kTinosRegularSize = 469968;
  FPDF_PAGEOBJECT text = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(text);
  FPDF_FONT font = FPDFTextObj_GetFont(text);
  ASSERT_TRUE(font);
  std::vector<uint8_t> buf;
  buf.resize(kTinosRegularSize);
  size_t buf_bytes_required;
  ASSERT_TRUE(
      FPDFFont_GetFontData(font, buf.data(), buf.size(), &buf_bytes_required));
  EXPECT_EQ(kTinosRegularSize, buf_bytes_required);
  EXPECT_EQ("2b019558f2c2de0b7cbc0a6e64b20599", GenerateMD5Base16(buf));
  EXPECT_EQ(0, FPDFFont_GetIsEmbedded(font));

  // Similarly, the second object consistently maps to Arimo-Regular.ttf.
  static constexpr size_t kArimoRegularSize = 436180;
  text = FPDFPage_GetObject(page.get(), 1);
  ASSERT_TRUE(text);
  font = FPDFTextObj_GetFont(text);
  ASSERT_TRUE(font);
  buf.resize(kArimoRegularSize);
  ASSERT_TRUE(
      FPDFFont_GetFontData(font, buf.data(), buf.size(), &buf_bytes_required));
  EXPECT_EQ(kArimoRegularSize, buf_bytes_required);
  EXPECT_EQ("7ac02a544211773d9636e056e9da6c35", GenerateMD5Base16(buf));
  EXPECT_EQ(0, FPDFFont_GetIsEmbedded(font));
}

TEST_F(FPDFEditEmbedderTest, Type1BaseFontName) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));

  {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
    ASSERT_TRUE(page_object);
    FPDF_FONT font = FPDFTextObj_GetFont(page_object);
    ASSERT_TRUE(font);
    size_t size = FPDFFont_GetBaseFontName(font, nullptr, 0);
    const char kExpectedFontName[] = "Times-Roman";
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    std::vector<char> font_name(size);
    ASSERT_EQ(size, FPDFFont_GetBaseFontName(font, font_name.data(), size));
    EXPECT_STREQ(kExpectedFontName, font_name.data());
  }
  {
    FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 1);
    ASSERT_TRUE(page_object);
    FPDF_FONT font = FPDFTextObj_GetFont(page_object);
    ASSERT_TRUE(font);
    size_t size = FPDFFont_GetBaseFontName(font, nullptr, 0);
    const char kExpectedFontName[] = "Helvetica";
    ASSERT_EQ(sizeof(kExpectedFontName), size);
    std::vector<char> font_name(size);
    ASSERT_EQ(size, FPDFFont_GetBaseFontName(font, font_name.data(), size));
    ASSERT_STREQ(kExpectedFontName, font_name.data());
  }
}

TEST_F(FPDFEditEmbedderTest, GlyphPaths) {
  // bad glyphpath
  EXPECT_EQ(-1, FPDFGlyphPath_CountGlyphSegments(nullptr));
  EXPECT_FALSE(FPDFGlyphPath_GetGlyphPathSegment(nullptr, 1));

  ASSERT_TRUE(OpenDocument("text_font.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT text = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(text);
  FPDF_FONT font = FPDFTextObj_GetFont(text);
  ASSERT_TRUE(font);

  // bad glyph argument.
  ASSERT_FALSE(FPDFFont_GetGlyphPath(font, 1, 12.0f));

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
}

TEST_F(FPDFEditEmbedderTest, FormGetObjects) {
  ASSERT_TRUE(OpenDocument("form_object.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT form = FPDFPage_GetObject(page.get(), 0);
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
  CompareFS_MATRIX(kMatrix, matrix);

  // FPDFPageObj_GetMatrix() negative testing for forms.
  EXPECT_FALSE(FPDFPageObj_GetMatrix(form, nullptr));

  // Show that FPDFPage_RemoveObject() cannot remove page objects from within
  // `form`. This is working as intended, as FPDFPage_RemoveObject() only works
  // for page object within `page`.
  EXPECT_FALSE(FPDFPage_RemoveObject(page.get(), text1));
  EXPECT_FALSE(FPDFPage_RemoveObject(page.get(), text2));
}

TEST_F(FPDFEditEmbedderTest, ModifyFormObject) {
  const char* orig_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "9d0ca0d471efc12950f337a867ab1694";
#elif BUILDFLAG(IS_APPLE)
      return "4cfff1919007a7faf099be5cc2cea00a";
#else
      return "1c6dae4b04fea7430a791135721eaba5";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "a637057185f50aac1aa5490f726aef95";
#else
    return "34a9ec0a9581a7970e073c0bcc4ca676";
#endif
  }();
  const char* new_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "dbebf244eb706dfebfd0594c23e993a9";
#elif BUILDFLAG(IS_APPLE)
      return "eb88a6842f5e12f5180385261db1f81d";
#else
      return "7282fe98693c0a7ad2c1b3f3f9563977";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "8ad9d79b02b609ff734e2a2195c96e2d";
#else
    return "609b5632a21c886fa93182dbc290bf7a";
#endif
  }();

  ASSERT_TRUE(OpenDocument("form_object.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 62, 69, orig_checksum);
  }

  FPDF_PAGEOBJECT form = FPDFPage_GetObject(page.get(), 0);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(form));

  FPDFPageObj_Transform(form, 0.5, 0, 0, 0.5, 0, 0);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), 62, 69, new_checksum);
  }

  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(62, 69, new_checksum);
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
  CompareBitmap(page_bitmap.get(), 612, 792, BottomTextChecksum());
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
  RetainPtr<const CPDF_Dictionary> graphics_dict =
      cpage->GetResources()->GetDictFor("ExtGState");
  ASSERT_TRUE(graphics_dict);
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE2"}));

  // Add a text object causing no change to the graphics dictionary
  FPDF_PAGEOBJECT text1 = FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
  // Only alpha, the last component, matters for the graphics dictionary. And
  // the default value is 255.
  EXPECT_TRUE(FPDFPageObj_SetFillColor(text1, 100, 100, 100, 255));
  FPDFPage_InsertObject(page.get(), text1);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE2"}));

  // Add a text object increasing the size of the graphics dictionary
  FPDF_PAGEOBJECT text2 =
      FPDFPageObj_NewTextObj(document(), "Times-Roman", 12.0f);
  FPDFPage_InsertObject(page.get(), text2);
  FPDFPageObj_SetBlendMode(text2, "Darken");
  EXPECT_TRUE(FPDFPageObj_SetFillColor(text2, 0, 0, 255, 150));
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE2", "FXE3"}));

  // Add a path that should reuse graphics
  FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(400, 100);
  FPDFPageObj_SetBlendMode(path, "Darken");
  EXPECT_TRUE(FPDFPageObj_SetFillColor(path, 200, 200, 100, 150));
  FPDFPage_InsertObject(page.get(), path);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE2", "FXE3"}));

  // Add a rect increasing the size of the graphics dictionary
  FPDF_PAGEOBJECT rect2 = FPDFPageObj_CreateNewRect(10, 10, 100, 100);
  FPDFPageObj_SetBlendMode(rect2, "Darken");
  EXPECT_TRUE(FPDFPageObj_SetFillColor(rect2, 0, 0, 255, 150));
  EXPECT_TRUE(FPDFPageObj_SetStrokeColor(rect2, 0, 0, 0, 200));
  FPDFPage_InsertObject(page.get(), rect2);
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE2", "FXE3", "FXE4"}));
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
  RetainPtr<const CPDF_Dictionary> graphics_dict =
      cpage->GetResources()->GetDictFor("ExtGState");
  ASSERT_TRUE(graphics_dict);
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE2"}));

  // Check the bitmap
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "5384da3406d62360ffb5cac4476fff1c");
  }

  // Never mind, my new favorite color is blue, increase alpha.
  // The red graphics state goes away.
  EXPECT_TRUE(FPDFPageObj_SetFillColor(rect, 0, 0, 255, 180));
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE3"}));

  // Check that bitmap displays changed content
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "2e51656f5073b0bee611d9cd086aa09c");
  }

  // And now generate, without changes
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE3"}));
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792,
                  "2e51656f5073b0bee611d9cd086aa09c");
  }

  // Add some text to the page, which starts out with no fonts.
  RetainPtr<const CPDF_Dictionary> font_dict =
      cpage->GetResources()->GetDictFor("Font");
  EXPECT_FALSE(font_dict);
  FPDF_PAGEOBJECT text_object =
      FPDFPageObj_NewTextObj(document(), "Arial", 12.0f);
  ScopedFPDFWideString text =
      GetFPDFWideString(L"Something something #text# something");
  EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
  FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 300, 300);
  FPDFPage_InsertObject(page, text_object);
  EXPECT_TRUE(FPDFPage_GenerateContent(page));

  // After generating the content, there should now be a font resource.
  font_dict = cpage->GetResources()->GetDictFor("Font");
  ASSERT_TRUE(font_dict);
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE3"}));
  EXPECT_THAT(font_dict->GetKeys(), UnorderedElementsAreArray({"FXF1"}));

  // Generate yet again, check dicts are reasonably sized
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_THAT(graphics_dict->GetKeys(),
              UnorderedElementsAreArray({"FXE1", "FXE3"}));
  EXPECT_THAT(font_dict->GetKeys(), UnorderedElementsAreArray({"FXF1"}));
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
  ASSERT_TRUE(font);
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsType1Font());

  RetainPtr<const CPDF_Dictionary> font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("Type1", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Tinos-Bold", font_dict->GetNameFor("BaseFont"));
  ASSERT_TRUE(font_dict->KeyExist("FirstChar"));
  ASSERT_TRUE(font_dict->KeyExist("LastChar"));
  EXPECT_EQ(32, font_dict->GetIntegerFor("FirstChar"));
  EXPECT_EQ(255, font_dict->GetIntegerFor("LastChar"));

  RetainPtr<const CPDF_Array> widths_array = font_dict->GetArrayFor("Widths");
  ASSERT_TRUE(widths_array);
  ASSERT_EQ(224u, widths_array->size());
  EXPECT_EQ(250, widths_array->GetFloatAt(0));
  EXPECT_EQ(569, widths_array->GetFloatAt(11));
  EXPECT_EQ(500, widths_array->GetFloatAt(223));
  CheckFontDescriptor(font_dict, FPDF_FONT_TYPE1, true, false, span);
}

TEST_F(FPDFEditEmbedderTest, LoadSimpleTrueTypeFont) {
  CreateNewDocument();
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Courier");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TRUETYPE, false));
  ASSERT_TRUE(font);
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsTrueTypeFont());

  RetainPtr<const CPDF_Dictionary> font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("TrueType", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Cousine-Regular", font_dict->GetNameFor("BaseFont"));
  ASSERT_TRUE(font_dict->KeyExist("FirstChar"));
  ASSERT_TRUE(font_dict->KeyExist("LastChar"));
  EXPECT_EQ(32, font_dict->GetIntegerFor("FirstChar"));
  EXPECT_EQ(255, font_dict->GetIntegerFor("LastChar"));

  RetainPtr<const CPDF_Array> widths_array = font_dict->GetArrayFor("Widths");
  ASSERT_TRUE(widths_array);
  ASSERT_EQ(224u, widths_array->size());
  EXPECT_EQ(600, widths_array->GetFloatAt(33));
  EXPECT_EQ(600, widths_array->GetFloatAt(74));
  EXPECT_EQ(600, widths_array->GetFloatAt(223));
  CheckFontDescriptor(font_dict, FPDF_FONT_TRUETYPE, false, false, span);
}

TEST_F(FPDFEditEmbedderTest, LoadCIDType0Font) {
  CreateNewDocument();
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Times-Roman");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TYPE1, 1));
  ASSERT_TRUE(font);
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsCIDFont());

  // Check font dictionary entries
  RetainPtr<const CPDF_Dictionary> font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("Type0", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Tinos-Regular-Identity-H", font_dict->GetNameFor("BaseFont"));
  EXPECT_EQ("Identity-H", font_dict->GetNameFor("Encoding"));
  RetainPtr<const CPDF_Array> descendant_array =
      font_dict->GetArrayFor("DescendantFonts");
  ASSERT_TRUE(descendant_array);
  EXPECT_EQ(1u, descendant_array->size());

  // Check the CIDFontDict
  RetainPtr<const CPDF_Dictionary> cidfont_dict =
      descendant_array->GetDictAt(0);
  EXPECT_EQ("Font", cidfont_dict->GetNameFor("Type"));
  EXPECT_EQ("CIDFontType0", cidfont_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Tinos-Regular", cidfont_dict->GetNameFor("BaseFont"));
  RetainPtr<const CPDF_Dictionary> cidinfo_dict =
      cidfont_dict->GetDictFor("CIDSystemInfo");
  ASSERT_TRUE(cidinfo_dict);
  RetainPtr<const CPDF_Object> registry =
      cidinfo_dict->GetObjectFor("Registry");
  ASSERT_TRUE(registry);
  EXPECT_EQ(CPDF_Object::kString, registry->GetType());
  EXPECT_EQ("Adobe", registry->GetString());
  RetainPtr<const CPDF_Object> ordering =
      cidinfo_dict->GetObjectFor("Ordering");
  ASSERT_TRUE(ordering);
  EXPECT_EQ(CPDF_Object::kString, ordering->GetType());
  EXPECT_EQ("Identity", ordering->GetString());
  EXPECT_EQ(0, cidinfo_dict->GetFloatFor("Supplement"));
  CheckFontDescriptor(cidfont_dict.Get(), FPDF_FONT_TYPE1, false, false, span);

  // Check widths
  RetainPtr<const CPDF_Array> widths_array = cidfont_dict->GetArrayFor("W");
  ASSERT_TRUE(widths_array);
  EXPECT_GT(widths_array->size(), 1u);
  CheckCompositeFontWidths(widths_array, typed_font, testing::Ge(201));
}

TEST_F(FPDFEditEmbedderTest, LoadCIDType2Font) {
  CreateNewDocument();
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(cpdf_doc(), "Helvetica-Oblique");
  pdfium::span<const uint8_t> span = stock_font->GetFont()->GetFontSpan();
  ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                        FPDF_FONT_TRUETYPE, 1));
  ASSERT_TRUE(font);
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  EXPECT_TRUE(typed_font->IsCIDFont());

  // Check font dictionary entries
  RetainPtr<const CPDF_Dictionary> font_dict = typed_font->GetFontDict();
  EXPECT_EQ("Font", font_dict->GetNameFor("Type"));
  EXPECT_EQ("Type0", font_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Arimo-Italic", font_dict->GetNameFor("BaseFont"));
  EXPECT_EQ("Identity-H", font_dict->GetNameFor("Encoding"));
  RetainPtr<const CPDF_Array> descendant_array =
      font_dict->GetArrayFor("DescendantFonts");
  ASSERT_TRUE(descendant_array);
  EXPECT_EQ(1u, descendant_array->size());

  // Check the CIDFontDict
  RetainPtr<const CPDF_Dictionary> cidfont_dict =
      descendant_array->GetDictAt(0);
  EXPECT_EQ("Font", cidfont_dict->GetNameFor("Type"));
  EXPECT_EQ("CIDFontType2", cidfont_dict->GetNameFor("Subtype"));
  EXPECT_EQ("Arimo-Italic", cidfont_dict->GetNameFor("BaseFont"));
  RetainPtr<const CPDF_Dictionary> cidinfo_dict =
      cidfont_dict->GetDictFor("CIDSystemInfo");
  ASSERT_TRUE(cidinfo_dict);
  EXPECT_EQ("Adobe", cidinfo_dict->GetByteStringFor("Registry"));
  EXPECT_EQ("Identity", cidinfo_dict->GetByteStringFor("Ordering"));
  EXPECT_EQ(0, cidinfo_dict->GetFloatFor("Supplement"));
  CheckFontDescriptor(cidfont_dict.Get(), FPDF_FONT_TRUETYPE, false, true,
                      span);

  // Check widths
  RetainPtr<const CPDF_Array> widths_array = cidfont_dict->GetArrayFor("W");
  ASSERT_TRUE(widths_array);
  CheckCompositeFontWidths(widths_array, typed_font, testing::Ge(201));
}

TEST_F(FPDFEditEmbedderTest, NormalizeNegativeRotation) {
  // Load document with a -90 degree rotation
  ASSERT_TRUE(OpenDocument("bug_713197.pdf"));
  ScopedPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  EXPECT_EQ(3, FPDFPage_GetRotation(page.get()));
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
    ASSERT_TRUE(font);

    // Add some text to the page
    FPDF_PAGEOBJECT text_object =
        FPDFPageObj_CreateTextObj(document(), font.get(), 12.0f);
    EXPECT_TRUE(text_object);
    ScopedFPDFWideString text = GetFPDFWideString(kLoadedFontText);
    EXPECT_TRUE(FPDFText_SetText(text_object, text.get()));
    FPDFPageObj_Transform(text_object, 1, 0, 0, 1, 400, 400);
    FPDFPage_InsertObject(page, text_object);
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, LoadedFontTextChecksum());

    // Add some more text, same font
    FPDF_PAGEOBJECT text_object2 =
        FPDFPageObj_CreateTextObj(document(), font.get(), 15.0f);
    ScopedFPDFWideString text2 = GetFPDFWideString(L"Bigger font size");
    EXPECT_TRUE(FPDFText_SetText(text_object2, text2.get()));
    FPDFPageObj_Transform(text_object2, 1, 0, 0, 1, 200, 200);
    FPDFPage_InsertObject(page, text_object2);
  }
  ScopedFPDFBitmap page_bitmap2 = RenderPage(page);
  const char* insert_true_type_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "7e44d135666d8bfcef5cdb4c8161fd4b";
#elif BUILDFLAG(IS_APPLE)
      return "6bab7f663daca1aab63d787a0f5cb33b";
#else
      return "4f9a6c7752ac7d4e4c731260fdb5af15";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "c7e2271a7f30e5b919a13ead47cea105";
#else
    return "683f4a385a891494100192cb338b11f0";
#endif
  }();
  CompareBitmap(page_bitmap2.get(), 612, 792, insert_true_type_checksum);

  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  VerifySavedDocument(612, 792, insert_true_type_checksum);
}

TEST_F(FPDFEditEmbedderTest, TransformAnnot) {
  // Open a file with one annotation and load its first page.
  ASSERT_TRUE(OpenDocument("annotation_highlight_long_content.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    // Add an underline annotation to the page without specifying its rectangle.
    ScopedFPDFAnnotation annot(
        FPDFPage_CreateAnnot(page.get(), FPDF_ANNOT_UNDERLINE));
    ASSERT_TRUE(annot);

    // FPDFPage_TransformAnnots() should run without errors when modifying
    // annotation rectangles.
    FPDFPage_TransformAnnots(page.get(), 1, 2, 3, 4, 5, 6);
  }
}

// TODO(npm): Add tests using Japanese fonts in other OS.
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
TEST_F(FPDFEditEmbedderTest, AddCIDFontText) {
  // Start with a blank page
  FPDF_PAGE page = FPDFPage_New(CreateNewDocument(), 0, 612, 792);
  CFX_Font cid_font;
  {
    // First, get the data from the font
    cid_font.LoadSubst("Noto Sans CJK JP", true, 0, 400, 0,
                       FX_CodePage::kShiftJIS, false);
    EXPECT_EQ("Noto Sans CJK JP", cid_font.GetFamilyName());
    pdfium::span<const uint8_t> span = cid_font.GetFontSpan();

    // Load the data into a FPDF_Font.
    ScopedFPDFFont font(FPDFText_LoadFont(document(), span.data(), span.size(),
                                          FPDF_FONT_TRUETYPE, 1));
    ASSERT_TRUE(font);

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
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "2e174d17de96a760d42ca3a06acbf36a";
    }
    return "84d31d11b76845423a2cfc1879c0fbb9";
  }();

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, checksum);
  }

  // Save the document, close the page.
  EXPECT_TRUE(FPDFPage_GenerateContent(page));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  FPDF_ClosePage(page);

  VerifySavedDocument(612, 792, checksum);
}
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)

TEST_F(FPDFEditEmbedderTest, LoadCidType2FontCustom) {
  // This is the same test as FPDFEditEmbedderTest.EmbedNotoSansSCFont, but some
  // of the font data is provided by the caller, instead of being generated.
  CreateEmptyDocument();
  ScopedFPDFPage page(FPDFPage_New(document(), 0, 400, 400));
  std::string font_path;
  ASSERT_TRUE(PathService::GetThirdPartyFilePath(
      "NotoSansCJK/NotoSansSC-Regular.subset.otf", &font_path));

  std::vector<uint8_t> font_data = GetFileContents(font_path.c_str());
  ASSERT_FALSE(font_data.empty());

  static const char kToUnicodeCMap[] = R"(
/CIDInit /ProcSet findresource begin
12 dict begin
begincmap
/CIDSystemInfo <<
  /Registry (Adobe)
  /Ordering (Identity)
  /Supplement 0
>> def
/CMapName /Adobe-Identity-H def
/CMapType 2 def
1 begincodespacerange
<0000> <FFFF>
endcodespacerange
5 beginbfrange
<0001> <0003> [<0020> <3002> <2F00>]
<0003> <0004> [<4E00> <2F06>]
<0004> <0005> [<4E8C> <53E5>]
<0005> <0008> [<F906> <662F> <7B2C> <884C>]
<0008> <0009> [<FA08> <8FD9>]
endbfrange
endcmap
CMapName currentdict /CMap defineresource pop
end
end
)";

  static constexpr auto kCidToGidMap = std::to_array<const uint8_t>(
      {0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9});

  ScopedFPDFFont font(FPDFText_LoadCidType2Font(
      document(), font_data.data(), font_data.size(), kToUnicodeCMap,
      kCidToGidMap.data(), kCidToGidMap.size()));
  ASSERT_TRUE(font);
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  RetainPtr<const CPDF_Array> widths_array =
      GetWidthsArrayForCidFont(typed_font);
  ASSERT_TRUE(widths_array);
  CheckCompositeFontWidths(widths_array, typed_font, testing::Eq(10));

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

  ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
  CompareBitmap(page_bitmap.get(), 400, 400, NotoSansSCChecksum());

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(400, 400, NotoSansSCChecksum());
}

TEST_F(FPDFEditEmbedderTest, LoadCidType2FontCustomGeneratedWidths) {
  CreateEmptyDocument();
  std::string font_path;
  ASSERT_TRUE(PathService::GetThirdPartyFilePath(
      "NotoSansCJK/NotoSansSC-Regular.subset.otf", &font_path));

  std::vector<uint8_t> font_data = GetFileContents(font_path.c_str());
  ASSERT_FALSE(font_data.empty());

  static const char kToUnicodeCMap[] = R"(
/CIDInit /ProcSet findresource begin
12 dict begin
begincmap
/CIDSystemInfo <<
  /Registry (Adobe)
  /Ordering (Identity)
  /Supplement 0
>> def
/CMapName /Adobe-Identity-H def
/CMapType 2 def
1 begincodespacerange
<0000> <FFFF>
endcodespacerange
3 beginbfrange
<0002> <0003> [<3002> <2F00>]
<0003> <0004> [<4E00> <2F06>]
<0004> <0005> [<4E8C> <53E5>]
endbfrange
endcmap
CMapName currentdict /CMap defineresource pop
end
end
)";

  static constexpr auto kCidToGidMap =
      std::to_array<const uint8_t>({0, 0, 0, 1, 0, 2, 0, 3, 0, 4});

  ScopedFPDFFont font(FPDFText_LoadCidType2Font(
      document(), font_data.data(), font_data.size(), kToUnicodeCMap,
      kCidToGidMap.data(), kCidToGidMap.size()));
  ASSERT_TRUE(font);
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  RetainPtr<const CPDF_Array> widths_array =
      GetWidthsArrayForCidFont(typed_font);
  ASSERT_TRUE(widths_array);
  CheckCompositeFontWidths(widths_array, typed_font, testing::Eq(5));
}

TEST_F(FPDFEditEmbedderTest, LoadCidType2FontWithBadParameters) {
  ASSERT_TRUE(CreateNewDocument());

  const std::vector<uint8_t> dummy_vec(3);
  const char kDummyString[] = "dummy";
  EXPECT_FALSE(FPDFText_LoadCidType2Font(nullptr, dummy_vec.data(),
                                         dummy_vec.size(), kDummyString,
                                         dummy_vec.data(), dummy_vec.size()));
  EXPECT_FALSE(FPDFText_LoadCidType2Font(document(), nullptr, dummy_vec.size(),
                                         kDummyString, dummy_vec.data(),
                                         dummy_vec.size()));
  EXPECT_FALSE(FPDFText_LoadCidType2Font(document(), dummy_vec.data(), 0,
                                         kDummyString, dummy_vec.data(),
                                         dummy_vec.size()));
  EXPECT_FALSE(FPDFText_LoadCidType2Font(document(), dummy_vec.data(),
                                         dummy_vec.size(), nullptr,
                                         dummy_vec.data(), dummy_vec.size()));
  EXPECT_FALSE(FPDFText_LoadCidType2Font(document(), dummy_vec.data(),
                                         dummy_vec.size(), "", dummy_vec.data(),
                                         dummy_vec.size()));
  EXPECT_FALSE(FPDFText_LoadCidType2Font(document(), dummy_vec.data(),
                                         dummy_vec.size(), kDummyString,
                                         nullptr, dummy_vec.size()));
  EXPECT_FALSE(FPDFText_LoadCidType2Font(document(), dummy_vec.data(),
                                         dummy_vec.size(), kDummyString,
                                         dummy_vec.data(), 0));
}

TEST_F(FPDFEditEmbedderTest, SaveAndRender) {
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "edd4aed776c0eaf8c79dd24d9654af95";
    }
    return "3c20472b0552c0c22b88ab1ed8c6202b";
  }();
  {
    ASSERT_TRUE(OpenDocument("bug_779.pdf"));
    ScopedPage page = LoadScopedPage(0);
    ASSERT_NE(nullptr, page.get());

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
    FPDFPage_InsertObject(page.get(), green_path);
    ScopedFPDFBitmap page_bitmap = RenderLoadedPage(page.get());
    CompareBitmap(page_bitmap.get(), 612, 792, checksum);

    // Now save the result, closing the page and document
    EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  }

  VerifySavedDocument(612, 792, checksum);
}

TEST_F(FPDFEditEmbedderTest, AddMark) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  CheckMarkCounts(page.get(), 1, 19, 8, 4, 9, 1);

  // Add to the first page object a "Bounds" mark with "Position": "First".
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_AddMark(page_object, "Bounds");
  EXPECT_TRUE(mark);
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), page_object, mark,
                                             "Position", "First"));

  CheckMarkCounts(page.get(), 1, 19, 8, 4, 9, 2);

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

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
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Render and check there are no marks.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldChecksum());
  }
  CheckMarkCounts(page.get(), 0, 2, 0, 0, 0, 0);

  // Add to the first page object a "Bounds" mark with "Position": "First".
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_AddMark(page_object, "Bounds");
  EXPECT_TRUE(mark);
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), page_object, mark,
                                             "Position", "First"));

  // Render and check there is 1 mark.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page.get());
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldChecksum());
  }
  CheckMarkCounts(page.get(), 0, 2, 0, 0, 0, 1);

  // Save the file.
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and check the new mark is present.
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  {
    ScopedFPDFBitmap page_bitmap = RenderPage(saved_page);
    CompareBitmap(page_bitmap.get(), 200, 200, HelloWorldChecksum());
  }
  CheckMarkCounts(saved_page, 0, 2, 0, 0, 0, 1);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, SetMarkParam) {
  // Load document with some text.
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static constexpr int kExpectedObjectCount = 19;
  CheckMarkCounts(page.get(), 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Check the "Bounds" mark's "Position" param is "Last".
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);
  FPDF_WCHAR buffer[128];
  unsigned long name_len = 999u;
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
  EXPECT_EQ((6u + 1u) * 2u, name_len);
  ASSERT_EQ(L"Bounds", GetPlatformWString(buffer));
  unsigned long out_buffer_len;
  ASSERT_TRUE(FPDFPageObjMark_GetParamStringValue(
      mark, "Position", buffer, sizeof(buffer), &out_buffer_len));
  ASSERT_EQ(L"Last", GetPlatformWString(buffer));

  // Set is to "End".
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), page_object, mark,
                                             "Position", "End"));

  // Verify the object passed must correspond to the mark passed.
  FPDF_PAGEOBJECT another_page_object = FPDFPage_GetObject(page.get(), 17);
  EXPECT_FALSE(FPDFPageObjMark_SetStringParam(document(), another_page_object,
                                              mark, "Position", "End"));

  // Verify nothing else changed.
  CheckMarkCounts(page.get(), 1, kExpectedObjectCount, 8, 4, 9, 1);

  // Verify "Position" now maps to "End".
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(
      mark, "Position", buffer, sizeof(buffer), &out_buffer_len));
  EXPECT_EQ(L"End", GetPlatformWString(buffer));

  // Save the file
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Re-open the file and cerify "Position" still maps to "End".
  ASSERT_TRUE(OpenSavedDocument());
  FPDF_PAGE saved_page = LoadSavedPage(0);
  ASSERT_TRUE(saved_page);

  CheckMarkCounts(saved_page, 1, kExpectedObjectCount, 8, 4, 9, 1);
  page_object = FPDFPage_GetObject(saved_page, 18);
  mark = FPDFPageObj_GetMark(page_object, 1);
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(
      mark, "Position", buffer, sizeof(buffer), &out_buffer_len));
  EXPECT_EQ(L"End", GetPlatformWString(buffer));

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
  FPDF_WCHAR buffer[128];
  unsigned long name_len = 999u;
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &name_len));
  EXPECT_EQ((14u + 1u) * 2, name_len);
  std::wstring name = GetPlatformWString(buffer);
  EXPECT_EQ(L"Test Mark Name", name);

  // Add parameters:
  // - int "IntKey" : 42
  // - string "StringKey": "StringValue"
  // - blob "BlobKey": "\x01\x02\x03\0BlobValue1\0\0\0BlobValue2\0"
  //
  // Note that the trailing NUL is in `kBlobData` implicitly.
  static constexpr uint8_t kBlobData[] =
      "\x01\x02\x03\0BlobValue1\0\0\0BlobValue2";
  EXPECT_EQ(0, FPDFPageObjMark_CountParams(mark));
  EXPECT_TRUE(
      FPDFPageObjMark_SetIntParam(document(), text_object, mark, "IntKey", 42));
  EXPECT_TRUE(FPDFPageObjMark_SetStringParam(document(), text_object, mark,
                                             "StringKey", "StringValue"));
  EXPECT_TRUE(FPDFPageObjMark_SetBlobParam(
      document(), text_object, mark, "BlobKey", kBlobData, sizeof(kBlobData)));
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
  name = GetPlatformWString(buffer);
  EXPECT_EQ(L"StringValue", name);

  EXPECT_EQ(FPDF_OBJECT_STRING,
            FPDFPageObjMark_GetParamValueType(mark, "BlobKey"));
  out_buffer_len = 0;
  static constexpr size_t kBlobLen = 28;
  unsigned char blob_buffer[kBlobLen];
  EXPECT_TRUE(FPDFPageObjMark_GetParamBlobValue(
      mark, "BlobKey", blob_buffer, sizeof(blob_buffer), &out_buffer_len));
  EXPECT_EQ(kBlobLen, out_buffer_len);
  EXPECT_EQ(pdfium::span(kBlobData), blob_buffer);

  // Render and check the bitmap is the expected one.
  {
    ScopedFPDFBitmap page_bitmap = RenderPage(page);
    CompareBitmap(page_bitmap.get(), 612, 792, LoadedFontTextChecksum());
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
  name = GetPlatformWString(buffer);
  EXPECT_EQ(L"Test Mark Name", name);

  CloseSavedPage(saved_page);
  CloseSavedDocument();
}

TEST_F(FPDFEditEmbedderTest, MarkGetName) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);

  FPDF_WCHAR buffer[128];
  unsigned long out_len;

  // Show the positive cases of FPDFPageObjMark_GetName.
  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetName(mark, nullptr, 0, &out_len));
  EXPECT_EQ((6u + 1u) * 2u, out_len);

  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(L"Bounds", GetPlatformWString(buffer));
  EXPECT_EQ((6u + 1u) * 2u, out_len);

  // Show the negative cases of FPDFPageObjMark_GetName.
  out_len = 999u;
  EXPECT_FALSE(
      FPDFPageObjMark_GetName(nullptr, buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(999u, out_len);

  EXPECT_FALSE(FPDFPageObjMark_GetName(mark, buffer, sizeof(buffer), nullptr));
}

TEST_F(FPDFEditEmbedderTest, MarkGetParamKey) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);

  FPDF_WCHAR buffer[128];
  unsigned long out_len;

  // Show the positive cases of FPDFPageObjMark_GetParamKey.
  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetParamKey(mark, 0, nullptr, 0, &out_len));
  EXPECT_EQ((8u + 1u) * 2u, out_len);

  out_len = 999u;
  EXPECT_TRUE(
      FPDFPageObjMark_GetParamKey(mark, 0, buffer, sizeof(buffer), &out_len));
  EXPECT_EQ(L"Position", GetPlatformWString(buffer));
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
}

TEST_F(FPDFEditEmbedderTest, MarkGetIntParam) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 8);
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

  page_object = FPDFPage_GetObject(page.get(), 18);
  mark = FPDFPageObj_GetMark(page_object, 1);
  out_value = 999;
  EXPECT_FALSE(FPDFPageObjMark_GetParamIntValue(mark, "Position", &out_value));
  EXPECT_EQ(999, out_value);
}

TEST_F(FPDFEditEmbedderTest, MarkGetStringParam) {
  ASSERT_TRUE(OpenDocument("text_in_page_marked.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 18);
  FPDF_PAGEOBJECTMARK mark = FPDFPageObj_GetMark(page_object, 1);
  ASSERT_TRUE(mark);

  FPDF_WCHAR buffer[128];
  unsigned long out_len;

  // Show the positive cases of FPDFPageObjMark_GetParamStringValue.
  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(mark, "Position", nullptr, 0,
                                                  &out_len));
  EXPECT_EQ((4u + 1u) * 2u, out_len);

  out_len = 999u;
  EXPECT_TRUE(FPDFPageObjMark_GetParamStringValue(mark, "Position", buffer,
                                                  sizeof(buffer), &out_len));
  EXPECT_EQ(L"Last", GetPlatformWString(buffer));
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

  page_object = FPDFPage_GetObject(page.get(), 8);
  mark = FPDFPageObj_GetMark(page_object, 0);
  out_len = 999u;
  EXPECT_FALSE(FPDFPageObjMark_GetParamStringValue(mark, "Factor", buffer,
                                                   sizeof(buffer), &out_len));
  EXPECT_EQ(999u, out_len);
}

// See also FPDFStructTreeEmbedderTest.GetMarkedContentID, which traverses the
// marked contents using FPDF_StructTree_GetForPage() and related API.
TEST_F(FPDFEditEmbedderTest, TraverseMarkedContentID) {
  ASSERT_TRUE(OpenDocument("marked_content_id.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  ASSERT_EQ(2, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT object1 = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(object1);
  ASSERT_EQ(1, FPDFPageObj_CountMarks(object1));

  FPDF_PAGEOBJECTMARK mark11 = FPDFPageObj_GetMark(object1, 0);
  ASSERT_TRUE(mark11);
  unsigned long len = 0;
  unsigned short buf[40];
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark11, buf, sizeof(buf), &len));
  EXPECT_EQ(18u, len);
  EXPECT_EQ(L"Artifact", GetPlatformWString(buf));
  ASSERT_EQ(2, FPDFPageObjMark_CountParams(mark11));
  ASSERT_TRUE(FPDFPageObjMark_GetParamKey(mark11, 0, buf, sizeof(buf), &len));
  EXPECT_EQ(10u, len);
  EXPECT_EQ(L"BBox", GetPlatformWString(buf));
  EXPECT_EQ(FPDF_OBJECT_ARRAY,
            FPDFPageObjMark_GetParamValueType(mark11, "BBox"));
  ASSERT_TRUE(FPDFPageObjMark_GetParamKey(mark11, 1, buf, sizeof(buf), &len));
  EXPECT_EQ(10u, len);
  EXPECT_EQ(L"Type", GetPlatformWString(buf));
  EXPECT_EQ(FPDF_OBJECT_NAME,
            FPDFPageObjMark_GetParamValueType(mark11, "Type"));

  FPDF_PAGEOBJECT object2 = FPDFPage_GetObject(page.get(), 1);
  ASSERT_TRUE(object2);
  ASSERT_EQ(2, FPDFPageObj_CountMarks(object2));
  EXPECT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(object2));

  FPDF_PAGEOBJECTMARK mark21 = FPDFPageObj_GetMark(object2, 0);
  ASSERT_TRUE(mark21);
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark21, buf, sizeof(buf), &len));
  EXPECT_EQ(14u, len);
  EXPECT_EQ(L"Figure", GetPlatformWString(buf));
  ASSERT_EQ(1, FPDFPageObjMark_CountParams(mark21));
  ASSERT_TRUE(FPDFPageObjMark_GetParamKey(mark21, 0, buf, sizeof(buf), &len));
  EXPECT_EQ(10u, len);
  EXPECT_EQ(L"MCID", GetPlatformWString(buf));
  ASSERT_EQ(FPDF_OBJECT_NUMBER,
            FPDFPageObjMark_GetParamValueType(mark21, "MCID"));
  int mcid = -1;
  ASSERT_TRUE(FPDFPageObjMark_GetParamIntValue(mark21, "MCID", &mcid));
  EXPECT_EQ(0, mcid);

  FPDF_PAGEOBJECTMARK mark22 = FPDFPageObj_GetMark(object2, 1);
  ASSERT_TRUE(mark22);
  ASSERT_TRUE(FPDFPageObjMark_GetName(mark22, buf, sizeof(buf), &len));
  EXPECT_EQ(18u, len);
  EXPECT_EQ(L"ClipSpan", GetPlatformWString(buf));
  EXPECT_EQ(0, FPDFPageObjMark_CountParams(mark22));
}

TEST_F(FPDFEditEmbedderTest, GetBitmap) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 32);
  EXPECT_NE(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_FALSE(FPDFImageObj_GetBitmap(obj));

  {
    obj = FPDFPage_GetObject(page.get(), 33);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 109, 88, kEmbeddedImage33Checksum);
  }

  {
    obj = FPDFPage_GetObject(page.get(), 34);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 103, 75, "c8d51fa6821ceb2a67f08446ff236c40");
  }

  {
    obj = FPDFPage_GetObject(page.get(), 35);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 92, 68, "7e34551035943e30a9f353db17de62ab");
  }

  {
    obj = FPDFPage_GetObject(page.get(), 36);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 79, 60, "f4e72fb783a01c7b4614cdc25eaa98ac");
  }

  {
    obj = FPDFPage_GetObject(page.get(), 37);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 126, 106, "2cf9e66414c72461f4ccbf9cdebdfa1b");
  }

  {
    obj = FPDFPage_GetObject(page.get(), 38);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 194, 119, "a8f3a126cec274dab8242fd2ccdc1b8b");
  }
}

TEST_F(FPDFEditEmbedderTest, GetBitmapIgnoresSetMatrix) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 33);
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
  CompareFS_MATRIX({53.0f, 0.0f, 0.0f, 43.0f, 72.0f, 646.510009765625f},
                   matrix);

  // Modify the matrix for |obj|.
  matrix.a = 120.0;
  EXPECT_TRUE(FPDFPageObj_SetMatrix(obj, &matrix));

  // Make sure the matrix modification took place.
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({120.0f, 0.0f, 0.0f, 43.0f, 72.0f, 646.510009765625f},
                   matrix);

  {
    // Render |obj| again. Note that the FPDFPageObj_SetMatrix() call has no
    // effect.
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 109, 88, kEmbeddedImage33Checksum);
  }
}

TEST_F(FPDFEditEmbedderTest, GetBitmapForJBigImage) {
  ASSERT_TRUE(OpenDocument("bug_631912.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(1, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 0);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  {
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    ASSERT_TRUE(bitmap);
    EXPECT_EQ(FPDFBitmap_Gray, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 1152, 720, "3f6a48e2b3e91b799bf34567f55cb4de");
  }
}

TEST_F(FPDFEditEmbedderTest, GetBitmapIgnoresSMask) {
  ASSERT_TRUE(OpenDocument("matte.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static constexpr int kExpectedObjects = 4;
  ASSERT_EQ(kExpectedObjects, FPDFPage_CountObjects(page.get()));

  for (int i = 0; i < kExpectedObjects; ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), i);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
    ASSERT_TRUE(bitmap);
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
    CompareBitmap(bitmap.get(), 50, 50, "46c9a1dbe0b44765ce46017ad629a2fe");
  }
}

TEST_F(FPDFEditEmbedderTest, GetBitmapWithArgbImageWithPalette) {
  ASSERT_TRUE(OpenDocument("bug_343075986.pdf"));

  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static constexpr int kExpectedObjects = 2;
  ASSERT_EQ(kExpectedObjects, FPDFPage_CountObjects(page.get()));
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 1);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  ScopedFPDFBitmap bitmap(FPDFImageObj_GetBitmap(obj));
  ASSERT_TRUE(bitmap);
  EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
  CompareBitmap(bitmap.get(), 4, 4, "49b4d39d3fd81c9853b493b615e475d1");
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapHandlesSetMatrix) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  {
    // Render `obj` as is.
    ScopedFPDFBitmap bitmap(
        FPDFImageObj_GetRenderedBitmap(document(), page.get(), obj));
    EXPECT_EQ(FPDFBitmap_BGRA, FPDFBitmap_GetFormat(bitmap.get()));
    const char* checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
        return "3b51fc066ee18efbf70bab0501763603";
      }
      return "582ca300e003f512d7b552c7b5b45d2e";
    }();
    CompareBitmap(bitmap.get(), 53, 43, checksum);
  }

  // Check the matrix for `obj`.
  FS_MATRIX matrix;

  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({53.0f, 0.0f, 0.0f, 43.0f, 72.0f, 646.510009765625f},
                   matrix);

  // Modify the matrix for `obj`.
  matrix.a = 120.0;
  EXPECT_TRUE(FPDFPageObj_SetMatrix(obj, &matrix));

  // Make sure the matrix modification took place.
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({120.0f, 0.0f, 0.0f, 43.0f, 72.0f, 646.510009765625f},
                   matrix);

  {
    // Render `obj` again. Note that the FPDFPageObj_SetMatrix() call has an
    // effect.
    ScopedFPDFBitmap bitmap(
        FPDFImageObj_GetRenderedBitmap(document(), page.get(), obj));
    EXPECT_EQ(FPDFBitmap_BGRA, FPDFBitmap_GetFormat(bitmap.get()));
    const char* checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
        return "1003585870ad0fe37baf1c5bb3f5fd76";
      }
      return "0824c16dcf2dfcef44b45d88db1fddce";
    }();
    CompareBitmap(bitmap.get(), 120, 43, checksum);
  }
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapHandlesSMask) {
  ASSERT_TRUE(OpenDocument("matte.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  static constexpr int kExpectedObjects = 4;
  ASSERT_EQ(kExpectedObjects, FPDFPage_CountObjects(page.get()));

  const char* smask_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "a85ca0183ac6aee8851c30c5bdc2f594";
    }
    return "5a3ae4a660ce919e29c42ec2258142f1";
  }();
  const char* no_smask_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "712f832dcbfb6cefc74f39bef459bea4";
    }
    return "67504e83f5d78214ea00efc19082c5c1";
  }();

  for (int i = 0; i < kExpectedObjects; ++i) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), i);
    ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
    ScopedFPDFBitmap bitmap(
        FPDFImageObj_GetRenderedBitmap(document(), page.get(), obj));
    ASSERT_TRUE(bitmap);
    EXPECT_EQ(FPDFBitmap_BGRA, FPDFBitmap_GetFormat(bitmap.get()));
    if (i == 0) {
      CompareBitmap(bitmap.get(), 40, 60, smask_checksum);
    } else {
      CompareBitmap(bitmap.get(), 40, 60, no_smask_checksum);
    }
  }
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapBadParams) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  // Test various null parameters.
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, nullptr, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(document(), nullptr, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, page.get(), nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, nullptr, obj));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(document(), page.get(), nullptr));
  EXPECT_FALSE(FPDFImageObj_GetRenderedBitmap(nullptr, page.get(), obj));

  // Test mismatch between document and page parameters.
  ScopedFPDFDocument new_document(FPDF_CreateNewDocument());
  EXPECT_FALSE(
      FPDFImageObj_GetRenderedBitmap(new_document.get(), page.get(), obj));
}

TEST_F(FPDFEditEmbedderTest, GetImageData) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page.get()));

  // Retrieve an image object with flate-encoded data stream.
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  // Check that the raw image data has the correct length and hash value.
  unsigned long len = FPDFImageObj_GetImageDataRaw(obj, nullptr, 0);
  std::vector<uint8_t> buf(len);
  EXPECT_EQ(4091u, FPDFImageObj_GetImageDataRaw(obj, buf.data(), len));
  EXPECT_EQ("f73802327d2e88e890f653961bcda81a", GenerateMD5Base16(buf));

  // Check that the decoded image data has the correct length and hash value.
  len = FPDFImageObj_GetImageDataDecoded(obj, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(28776u, FPDFImageObj_GetImageDataDecoded(obj, buf.data(), len));
  EXPECT_EQ(kEmbeddedImage33Checksum, GenerateMD5Base16(buf));

  // Retrieve an image object with DCTDecode-encoded data stream.
  obj = FPDFPage_GetObject(page.get(), 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  // Check that the raw image data has the correct length and hash value.
  len = FPDFImageObj_GetImageDataRaw(obj, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(4370u, FPDFImageObj_GetImageDataRaw(obj, buf.data(), len));
  EXPECT_EQ("6aae1f3710335023a9e12191be66b64b", GenerateMD5Base16(buf));

  // Check that the decoded image data has the correct length and hash value,
  // which should be the same as those of the raw data, since this image is
  // encoded by a single DCTDecode filter and decoding is a noop.
  len = FPDFImageObj_GetImageDataDecoded(obj, nullptr, 0);
  buf.clear();
  buf.resize(len);
  EXPECT_EQ(4370u, FPDFImageObj_GetImageDataDecoded(obj, buf.data(), len));
  EXPECT_EQ("6aae1f3710335023a9e12191be66b64b", GenerateMD5Base16(buf));
}

TEST_F(FPDFEditEmbedderTest, GetImageMatrix) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  ASSERT_EQ(39, FPDFPage_CountObjects(page.get()));

  FPDF_PAGEOBJECT obj;
  FS_MATRIX matrix;

  obj = FPDFPage_GetObject(page.get(), 33);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({53.0f, 0.0f, 0.0f, 43.0f, 72.0f, 646.510009765625f},
                   matrix);

  obj = FPDFPage_GetObject(page.get(), 34);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({70.0f, 0.0f, 0.0f, 51.0f, 216.0f, 646.510009765625f},
                   matrix);

  obj = FPDFPage_GetObject(page.get(), 35);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({69.0f, 0.0f, 0.0f, 51.0f, 360.0f, 646.510009765625f},
                   matrix);

  obj = FPDFPage_GetObject(page.get(), 36);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({59.0f, 0.0f, 0.0f, 45.0f, 72.0f, 553.510009765625f},
                   matrix);

  obj = FPDFPage_GetObject(page.get(), 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({55.94000244140625f, 0.0f, 0.0f, 46.950000762939453f, 216.0f,
                    552.510009765625f},
                   matrix);

  obj = FPDFPage_GetObject(page.get(), 38);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_TRUE(FPDFPageObj_GetMatrix(obj, &matrix));
  CompareFS_MATRIX({70.528999328613281f, 0.0f, 0.0f, 43.149997711181641f,
                    360.0f, 553.3599853515625f},
                   matrix);
}

TEST_F(FPDFEditEmbedderTest, DestroyPageObject) {
  FPDF_PAGEOBJECT rect = FPDFPageObj_CreateNewRect(10, 10, 20, 20);
  ASSERT_TRUE(rect);

  // There should be no memory leaks with a call to FPDFPageObj_Destroy().
  FPDFPageObj_Destroy(rect);
}

TEST_F(FPDFEditEmbedderTest, GetImageFilters) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Verify that retrieving the filter of a non-image object would fail.
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 32);
  ASSERT_NE(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_EQ(0, FPDFImageObj_GetImageFilterCount(obj));
  EXPECT_EQ(0u, FPDFImageObj_GetImageFilter(obj, 0, nullptr, 0));

  // Verify the returned filter string for an image object with a single filter.
  obj = FPDFPage_GetObject(page.get(), 33);
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
  obj = FPDFPage_GetObject(page.get(), 38);
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
}

TEST_F(FPDFEditEmbedderTest, GetImageMetadata) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Check that getting the metadata of a null object would fail.
  FPDF_IMAGEOBJ_METADATA metadata;
  EXPECT_FALSE(FPDFImageObj_GetImageMetadata(nullptr, page.get(), &metadata));

  // Check that receiving the metadata with a null metadata object would fail.
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 35);
  EXPECT_FALSE(FPDFImageObj_GetImageMetadata(obj, page.get(), nullptr));

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
  ASSERT_TRUE(FPDFImageObj_GetImageMetadata(obj, page.get(), &metadata));
  EXPECT_EQ(7, metadata.marked_content_id);
  EXPECT_EQ(92u, metadata.width);
  EXPECT_EQ(68u, metadata.height);
  EXPECT_FLOAT_EQ(96.0f, metadata.horizontal_dpi);
  EXPECT_FLOAT_EQ(96.0f, metadata.vertical_dpi);
  EXPECT_EQ(1u, metadata.bits_per_pixel);
  EXPECT_EQ(FPDF_COLORSPACE_INDEXED, metadata.colorspace);

  // Verify the metadata of an image with RGB colorspace.
  obj = FPDFPage_GetObject(page.get(), 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_TRUE(FPDFImageObj_GetImageMetadata(obj, page.get(), &metadata));
  EXPECT_EQ(9, metadata.marked_content_id);
  EXPECT_EQ(126u, metadata.width);
  EXPECT_EQ(106u, metadata.height);
  EXPECT_FLOAT_EQ(162.173752f, metadata.horizontal_dpi);
  EXPECT_FLOAT_EQ(162.555878f, metadata.vertical_dpi);
  EXPECT_EQ(24u, metadata.bits_per_pixel);
  EXPECT_EQ(FPDF_COLORSPACE_DEVICERGB, metadata.colorspace);
}

TEST_F(FPDFEditEmbedderTest, GetImageMetadataJpxLzw) {
  ASSERT_TRUE(OpenDocument("jpx_lzw.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 0);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));

  FPDF_IMAGEOBJ_METADATA metadata;
  ASSERT_TRUE(FPDFImageObj_GetImageMetadata(obj, page.get(), &metadata));
  EXPECT_EQ(-1, metadata.marked_content_id);
  EXPECT_EQ(612u, metadata.width);
  EXPECT_EQ(792u, metadata.height);
  EXPECT_FLOAT_EQ(72.0f, metadata.horizontal_dpi);
  EXPECT_FLOAT_EQ(72.0f, metadata.vertical_dpi);
  EXPECT_EQ(24u, metadata.bits_per_pixel);
  EXPECT_EQ(FPDF_COLORSPACE_UNKNOWN, metadata.colorspace);
}

TEST_F(FPDFEditEmbedderTest, GetImagePixelSize) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Check that getting the size of a null object would fail.
  unsigned int width = 0;
  unsigned int height = 0;
  EXPECT_FALSE(FPDFImageObj_GetImagePixelSize(nullptr, &width, &height));

  // Check that receiving the size with a null width and height pointers would
  // fail.
  FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page.get(), 35);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  EXPECT_FALSE(FPDFImageObj_GetImagePixelSize(obj, nullptr, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetImagePixelSize(obj, nullptr, &height));
  EXPECT_FALSE(FPDFImageObj_GetImagePixelSize(obj, &width, nullptr));

  // Verify the pixel size of image.
  ASSERT_TRUE(FPDFImageObj_GetImagePixelSize(obj, &width, &height));
  EXPECT_EQ(92u, width);
  EXPECT_EQ(68u, height);

  obj = FPDFPage_GetObject(page.get(), 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(obj));
  ASSERT_TRUE(FPDFImageObj_GetImagePixelSize(obj, &width, &height));
  EXPECT_EQ(126u, width);
  EXPECT_EQ(106u, height);
}

TEST_F(FPDFEditEmbedderTest, GetIccProfileDataDecoded) {
  ASSERT_TRUE(OpenDocument("bug_42270471.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(1, FPDFPage_CountObjects(page.get()));

  // Retrieve the image object and validate its type.
  FPDF_PAGEOBJECT image_obj = FPDFPage_GetObject(page.get(), 0);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image_obj));

  // Validate failure cases for null parameters.
  // 12345 is an arbitrary non-zero value to verify that the length is not
  // modified.
  size_t icc_length = 12345;
  EXPECT_FALSE(FPDFImageObj_GetIccProfileDataDecoded(nullptr, page.get(),
                                                     nullptr, 0, &icc_length));
  EXPECT_EQ(12345u, icc_length);

  EXPECT_FALSE(FPDFImageObj_GetIccProfileDataDecoded(image_obj, nullptr,
                                                     nullptr, 0, &icc_length));
  EXPECT_EQ(12345u, icc_length);

  EXPECT_FALSE(FPDFImageObj_GetIccProfileDataDecoded(image_obj, page.get(),
                                                     nullptr, 0, nullptr));
  EXPECT_FALSE(FPDFImageObj_GetIccProfileDataDecoded(nullptr, nullptr, nullptr,
                                                     0, nullptr));
  // Retrieve the raw ICC profile data length
  icc_length = 0;
  EXPECT_TRUE(FPDFImageObj_GetIccProfileDataDecoded(image_obj, page.get(),
                                                    nullptr, 0, &icc_length));
  EXPECT_EQ(525u, icc_length);

  // Check that the raw ICC profile data has the correct length and hash value.
  std::vector<uint8_t> icc_data(icc_length);
  EXPECT_TRUE(FPDFImageObj_GetIccProfileDataDecoded(
      image_obj, page.get(), icc_data.data(), icc_data.size(), &icc_length));
  EXPECT_EQ(525u, icc_length);
  EXPECT_EQ(icc_data.size(), icc_length);
  EXPECT_EQ("6f10cf8865bf3ae7e49aa766f78bfba8", GenerateMD5Base16(icc_data));
}

TEST_F(FPDFEditEmbedderTest, GetIccProfileDataDecodedNoIccProfile) {
  ASSERT_TRUE(OpenDocument("embedded_images.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Retrieve the image object without an ICC profile.
  FPDF_PAGEOBJECT image_obj = FPDFPage_GetObject(page.get(), 37);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image_obj));

  // Validate failure cases for image object without an ICC profile.
  // 12345 is an arbitrary non-zero value to verify that the length is not
  // modified.
  size_t icc_length = 12345;
  EXPECT_FALSE(FPDFImageObj_GetIccProfileDataDecoded(image_obj, page.get(),
                                                     nullptr, 0, &icc_length));
  EXPECT_EQ(12345u, icc_length);
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapForHelloWorldText) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    FPDF_PAGEOBJECT text_object = FPDFPage_GetObject(page.get(), 0);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object));

    ScopedFPDFBitmap bitmap(
        FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 1));
    ASSERT_TRUE(bitmap);
    const char* checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "764e3503960ef0b176796faa3543b9c7";
#elif BUILDFLAG(IS_APPLE)
        return "9e7774173acee966fcaa72e599eb9a93";
#else
        return "b17801afe8a36d6aad6c2239b88f2a73";
#endif
      }
      return "bb0abe1accca1cfeaaf78afa35762350";
    }();
    CompareBitmap(bitmap.get(), 64, 11, checksum);

    ScopedFPDFBitmap x2_bitmap(FPDFTextObj_GetRenderedBitmap(
        document(), page.get(), text_object, 2.4f));
    ASSERT_TRUE(x2_bitmap);
    const char* x2_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "3cea4255285df04659e3c7477287bdb1";
#elif BUILDFLAG(IS_APPLE)
        return "2b34bddd2a1471e245cf72603c6799b4";
#else
        return "33af8b151ab26ebce5a71b39eedea6b1";
#endif
      }
      return "80db528ec7146d92247f2339a8f10ba5";
    }();
    CompareBitmap(x2_bitmap.get(), 153, 25, x2_checksum);

    ScopedFPDFBitmap x10_bitmap(
        FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 10));
    ASSERT_TRUE(x10_bitmap);
    const char* x10_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "1cc617da9ed5922eeac2414108509ef5";
#elif BUILDFLAG(IS_APPLE)
        return "0450d576560274a7df31cb93d040e721";
#else
        return "93dd7ad07bdaaba9ecd268350cb91596";
#endif
      }
      return "149f63de758ab01d3b75605cdfd4c176";
    }();
    CompareBitmap(x10_bitmap.get(), 631, 103, x10_checksum);
  }

  {
    FPDF_PAGEOBJECT text_object = FPDFPage_GetObject(page.get(), 1);
    ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object));

    ScopedFPDFBitmap bitmap(
        FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 1));
    ASSERT_TRUE(bitmap);
    const char* checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "4cdba7492317bcae2643bd4090e18812";
#elif BUILDFLAG(IS_APPLE)
        return "0b9efedcb8f5aa9246c52e90811cb046";
#else
        return "63fd059d984a5bea10f27ba026420202";
#endif
      }
      return "3fc1101b2408c5484adc24ba0a11ff3d";
    }();
    CompareBitmap(bitmap.get(), 116, 16, checksum);

    ScopedFPDFBitmap x2_bitmap(FPDFTextObj_GetRenderedBitmap(
        document(), page.get(), text_object, 2.4f));
    ASSERT_TRUE(x2_bitmap);
    const char* x2_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "c5cecc5553843a4dd4fff3ceb4855a82";
#elif BUILDFLAG(IS_APPLE)
        return "10f4d9528a5471ab0b235984f0354dd4";
#else
        return "fc45021e3ea3ebd406fe6ffaa8c5c5b7";
#endif
      }
      return "429960ae7b822f0c630432535e637465";
    }();
    CompareBitmap(x2_bitmap.get(), 276, 36, x2_checksum);

    ScopedFPDFBitmap x10_bitmap(
        FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 10));
    ASSERT_TRUE(x10_bitmap);
    const char* x10_checksum = []() {
      if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
        return "cff29dcbe77092ec7f73e46766a289c7";
#elif BUILDFLAG(IS_APPLE)
        return "9e87791ffdf4cca0a0f118be245970c8";
#else
        return "61476636eaa0da0b93d8b1937cf22b75";
#endif
      }
      return "f5f93bf64de579b59e775d7076ca0a5a";
    }();
    CompareBitmap(x10_bitmap.get(), 1143, 150, x10_checksum);
  }
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapForRotatedText) {
  ASSERT_TRUE(OpenDocument("rotated_text.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT text_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object));

  ScopedFPDFBitmap bitmap(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 1));
  ASSERT_TRUE(bitmap);
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "ba5322a4e6b0f79dca42be88f3007708";
#elif BUILDFLAG(IS_APPLE)
      return "22cf71716a7059f481a63e32b6088c8c";
#else
      return "f515a7209d7892065d3716ec462f5c10";
#endif
    }
    return "08ada0802f780d3fefb161dc6fb45977";
  }();
  CompareBitmap(bitmap.get(), 29, 28, checksum);

  ScopedFPDFBitmap x2_bitmap(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 2.4f));
  ASSERT_TRUE(x2_bitmap);
  const char* x2_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "e8fb0a707b2924726757a2ed32d6f28d";
#elif BUILDFLAG(IS_APPLE)
      return "5d4be6808bdcec3f6ee7352122dd986d";
#else
      return "c69bbe5318ec149f63228e276e708612";
#endif
    }
    return "09d7ddb647b8653cb59aede349a0c3e1";
  }();
  CompareBitmap(x2_bitmap.get(), 67, 67, x2_checksum);

  ScopedFPDFBitmap x10_bitmap(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 10));
  ASSERT_TRUE(x10_bitmap);
  const char* x10_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "eb0cbf56707d1c39ce0ab89a9b43d6a8";
#elif BUILDFLAG(IS_APPLE)
      return "98757f865abde60c7f7f60c74435cb85";
#else
      return "bb7c2ec575f27cf882dcd38f2563c00f";
#endif
    }
    return "bbd3842a4b50dbfcbce4eee2b067a297";
  }();
  CompareBitmap(x10_bitmap.get(), 275, 275, x10_checksum);
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapForColorText) {
  ASSERT_TRUE(OpenDocument("text_color.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT text_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object));

  ScopedFPDFBitmap bitmap(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 7.3f));
  ASSERT_TRUE(bitmap);
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "9199f0c27c8a61a57189b1b044941e5e";
    }
    return "e8154fa8ededf4d9b8b35b5260897b6c";
  }();
  CompareBitmap(bitmap.get(), 120, 186, checksum);
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapForNewlyCreatedText) {
  // Start with a blank document.
  ASSERT_TRUE(CreateNewDocument());

  // Create a new text object.
  ScopedFPDFPageObject text_object(
      FPDFPageObj_NewTextObj(document(), "Arial", 12.0f));
  ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text_object.get()));
  ScopedFPDFWideString text = GetFPDFWideString(kBottomText);
  EXPECT_TRUE(FPDFText_SetText(text_object.get(), text.get()));

  ScopedFPDFBitmap bitmap(
      FPDFTextObj_GetRenderedBitmap(document(), nullptr, text_object.get(), 1));
  ASSERT_TRUE(bitmap);
  const char* checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "6d88537a49fa2dccfa0f58ac325c5b75";
#elif BUILDFLAG(IS_APPLE)
      return "a637d62f2e8ae10c3267b2ff5fcc2246";
#else
      return "574ae982d02e653ab6a8f23a6cdf4085";
#endif
    }
    return "fa947759dab76d68a07ccf6f97b2d9c2";
  }();
  CompareBitmap(bitmap.get(), 151, 12, checksum);
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapForTextWithBadParameters) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  FPDF_PAGEOBJECT text_object = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(text_object);

  // Simple bad parameters testing.
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(nullptr, nullptr, nullptr, 0));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(document(), nullptr, nullptr, 0));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(nullptr, page.get(), nullptr, 0));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(nullptr, nullptr, text_object, 0));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(nullptr, nullptr, nullptr, 1));
  EXPECT_FALSE(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), nullptr, 0));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(document(), nullptr, nullptr, 1));
  EXPECT_FALSE(
      FPDFTextObj_GetRenderedBitmap(nullptr, page.get(), text_object, 0));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(nullptr, page.get(), nullptr, 1));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(nullptr, nullptr, text_object, 1));
  EXPECT_FALSE(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), nullptr, 1));
  EXPECT_FALSE(
      FPDFTextObj_GetRenderedBitmap(nullptr, page.get(), text_object, 1));

  // Test bad scale values.
  EXPECT_FALSE(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, 0));
  EXPECT_FALSE(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object, -1));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(document(), page.get(),
                                             text_object, 10000));
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(
      document(), page.get(), text_object, std::numeric_limits<float>::max()));
  EXPECT_FALSE(
      FPDFTextObj_GetRenderedBitmap(document(), page.get(), text_object,
                                    std::numeric_limits<float>::infinity()));

  {
    // `text_object` will render without `page`, but may not render correctly
    // without the resources from `page`. Although it does in this simple case.
    ScopedFPDFBitmap bitmap(
        FPDFTextObj_GetRenderedBitmap(document(), nullptr, text_object, 1));
    EXPECT_TRUE(bitmap);
  }

  // Mismatch between the document and the page fails too.
  ScopedFPDFDocument empty_document(FPDF_CreateNewDocument());
  EXPECT_FALSE(FPDFTextObj_GetRenderedBitmap(empty_document.get(), page.get(),
                                             text_object, 1));
}

TEST_F(FPDFEditEmbedderTest, GetRenderedBitmapForRotatedImage) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPage page(FPDFPage_New(doc.get(), 0, 100, 100));
  EXPECT_EQ(0, FPDFPage_CountObjects(page.get()));

  static constexpr int kBitmapWidth = 50;
  static constexpr int kBitmapHeight = 100;
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(kBitmapWidth, kBitmapHeight, 0));
  ASSERT_TRUE(FPDFBitmap_FillRect(bitmap.get(), 0, 0, kBitmapWidth,
                                  kBitmapHeight, 0x00000000));
  ScopedFPDFPageObject page_image(FPDFPageObj_NewImageObj(doc.get()));
  ASSERT_TRUE(
      FPDFImageObj_SetBitmap(nullptr, 0, page_image.get(), bitmap.get()));

  // Set bitmap matrix with scaling and 90 degrees clockwise rotation.
  static constexpr int kScaleX = 2;
  static constexpr int kScaleY = 3;
  static constexpr FS_MATRIX kBitmapMatrix{
      0, -kScaleX * kBitmapWidth, kScaleY * kBitmapHeight, 0, 0, 0};
  ASSERT_TRUE(FPDFPageObj_SetMatrix(page_image.get(), &kBitmapMatrix));
  FPDFPage_InsertObject(page.get(), page_image.release());
  EXPECT_EQ(1, FPDFPage_CountObjects(page.get()));
  EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));

  FPDF_PAGEOBJECT page_object = FPDFPage_GetObject(page.get(), 0);
  ScopedFPDFBitmap extracted_bitmap(
      FPDFImageObj_GetRenderedBitmap(doc.get(), page.get(), page_object));
  ASSERT_TRUE(extracted_bitmap);

  ASSERT_EQ(FPDFBitmap_GetWidth(extracted_bitmap.get()),
            kScaleY * kBitmapHeight);
  ASSERT_EQ(FPDFBitmap_GetHeight(extracted_bitmap.get()),
            kScaleX * kBitmapWidth);
}

TEST_F(FPDFEditEmbedderTest, MultipleGraphicsStates) {
  ASSERT_TRUE(OpenDocument("multiple_graphics_states.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFPageObject path(FPDFPageObj_CreateNewPath(400, 100));
    EXPECT_TRUE(FPDFPageObj_SetFillColor(path.get(), 255, 0, 0, 255));
    EXPECT_TRUE(FPDFPath_SetDrawMode(path.get(), FPDF_FILLMODE_ALTERNATE, 0));
    EXPECT_TRUE(FPDFPath_MoveTo(path.get(), 100, 100));
    EXPECT_TRUE(FPDFPath_LineTo(path.get(), 100, 125));
    EXPECT_TRUE(FPDFPath_Close(path.get()));

    FPDFPage_InsertObject(page.get(), path.release());
    EXPECT_TRUE(FPDFPage_GenerateContent(page.get()));
  }

  const char* checksum = CFX_DefaultRenderDevice::UseSkiaRenderer()
                             ? "7ebec75d95c64b522999a710de76c52c"
                             : "f4b36616a7fea81a4f06cc7b01a55ac1";

  ScopedFPDFBitmap bitmap = RenderPage(page.get());
  CompareBitmap(bitmap.get(), 200, 300, checksum);

  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));
  VerifySavedDocument(200, 300, checksum);
}

TEST_F(FPDFEditEmbedderTest, GetAndSetMatrixForFormWithText) {
  static constexpr int kExpectedWidth = 200;
  static constexpr int kExpectedHeight = 200;

  ASSERT_TRUE(OpenDocument("form_object_with_text.pdf"));
  ScopedPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  HelloWorldChecksum());
  }

  FPDF_PAGEOBJECT form = FPDFPage_GetObject(page.get(), 0);
  ASSERT_TRUE(form);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(form));

  FS_MATRIX matrix;

  ASSERT_TRUE(FPDFPageObj_GetMatrix(form, &matrix));
  CompareFS_MATRIX({2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 200.0f}, matrix);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(form, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  HelloWorldChecksum());
  }

  FPDF_PAGEOBJECT text = FPDFFormObj_GetObject(form, 0);
  ASSERT_TRUE(text);
  ASSERT_EQ(FPDF_PAGEOBJ_TEXT, FPDFPageObj_GetType(text));

  ASSERT_TRUE(FPDFPageObj_GetMatrix(text, &matrix));
  CompareFS_MATRIX({0.5f, 0.0f, 0.0f, -1.0f, 10.0f, 150.0f}, matrix);

  ASSERT_TRUE(FPDFPageObj_SetMatrix(text, &matrix));
  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  HelloWorldChecksum());
  }

  ASSERT_TRUE(FPDFPage_GenerateContent(page.get()));
  ASSERT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  {
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    CompareBitmap(bitmap.get(), kExpectedWidth, kExpectedHeight,
                  HelloWorldChecksum());
  }

  VerifySavedDocument(kExpectedWidth, kExpectedHeight, HelloWorldChecksum());
}

TEST_F(FPDFEditEmbedderTest, PageObjTransformFWithBadParameters) {
  ScopedFPDFDocument doc(FPDF_CreateNewDocument());
  ScopedFPDFPageObject image(FPDFPageObj_NewImageObj(doc.get()));
  ASSERT_TRUE(image);

  const FS_MATRIX matrix{1, 2, 3, 4, 5, 6};
  EXPECT_FALSE(FPDFPageObj_TransformF(nullptr, nullptr));
  EXPECT_FALSE(FPDFPageObj_TransformF(image.get(), nullptr));
  EXPECT_FALSE(FPDFPageObj_TransformF(nullptr, &matrix));
}

class FPDFEditMoveEmbedderTest : public EmbedderTest {
 protected:
  std::vector<std::string> HashesForDocument(int page_count) {
    std::vector<std::string> hashes;
    hashes.reserve(page_count);
    for (int i = 0; i < page_count; ++i) {
      hashes.push_back(HashForPage(i));
    }
    return hashes;
  }

 private:
  std::string HashForPage(int page_index) {
    ScopedPage page = LoadScopedPage(page_index);
    EXPECT_TRUE(page);
    ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
    std::string hash = HashBitmap(bitmap.get());
    return hash;
  }
};

TEST_F(FPDFEditMoveEmbedderTest, MovePagesTest) {
  static const FPDFEditMoveEmbedderTestCase kTestCases[] = {
      {{0, 1, 2, 3, 4}, 5, 0, true, {0, 1, 2, 3, 4}, "no change"},
      {{0, 4, 2, 1, 3}, 5, 0, true, {0, 4, 2, 1, 3}, "reorder all pages"},
      {{0, 2, 4, 3}, 4, 1, true, {1, 0, 2, 4, 3}, "reorder 4 pages"},
      {{1, 4, 2}, 3, 2, true, {0, 3, 1, 4, 2}, "reorder 3 pages"},
      {{3, 2}, 2, 3, true, {0, 1, 4, 3, 2}, "reorder 2 pages"},
      {{3}, 1, 4, true, {0, 1, 2, 4, 3}, "reorder 1 page"},
      {{1, 1}, 2, 2, false, {}, "duplicate index"},
      {{5, 3, 2}, 3, 0, false, {}, "out of range index"},
      {{3}, 0, 0, false, {}, "page_indices_len needs to be in range [1, 5]"},
      {{4, 3, 2, 1, 0}, 6, 0, false, {}, "page_indices_len is too big"},
      {{3}, 0, 5, false, {}, "dest_page_index is out of range"},
      {{3, 1, 4}, 0, -1, false, {}, "dest_page_index is out of range"},
  };

  // Try all test cases with a freshly opened document that has 5 pages.
  for (const FPDFEditMoveEmbedderTestCase& tc : kTestCases) {
    ASSERT_TRUE(OpenDocument("rectangles_multi_pages.pdf"));
    const int page_count = GetPageCount();
    ASSERT_EQ(page_count, 5);
    // Check that the test case has correctly formed expected result.
    if (tc.expected_result) {
      ASSERT_THAT(tc.expected_order, testing::SizeIs(page_count));
    } else {
      ASSERT_THAT(tc.expected_order, testing::SizeIs(0));
    }

    // Cache the original pages' hashes.
    std::vector<std::string> orig_hashes = HashesForDocument(page_count);
    ASSERT_THAT(orig_hashes, testing::SizeIs(page_count));

    EXPECT_EQ(FPDF_MovePages(document(), &tc.page_indices[0],
                             tc.page_indices_len, tc.dest_page_index),
              tc.expected_result)
        << tc;

    if (tc.expected_result) {
      // Check for updated page order.
      std::vector<std::string> new_hashes = HashesForDocument(page_count);
      std::vector<std::string> expected_hashes;
      expected_hashes.reserve(page_count);
      for (int i = 0; i < page_count; ++i) {
        expected_hashes.push_back(orig_hashes[tc.expected_order[i]]);
      }
      EXPECT_THAT(new_hashes, testing::ContainerEq(expected_hashes)) << tc;
    } else {
      // Check that pages are unchanged.
      EXPECT_THAT(HashesForDocument(page_count),
                  testing::ContainerEq(orig_hashes))
          << tc;
    }

    CloseDocument();
  }
}

// Regression test for https://issues.chromium.org/377948405
// fonts/bug_377948405.ttf was made with harfbuzz* subset utility:
// > hb-subset NotoSans-Regular.ttf -u 41,C0,C4-C6,C8 -o bug_377948405.ttf
// Font W array state before bug fix: [1 [639 639 639 639 881 556]]
// Font W array state after bug fix: [1 [639] 5 7 639 8 [881 556]]
// *At the moment of the bug detection harfbuzz version was 10.1.0
TEST_F(FPDFEditEmbedderTest, Bug377948405) {
  CreateEmptyDocument();
  ScopedFPDFPage page(FPDFPage_New(document(), 0, 400, 400));
  std::string font_path =
      PathService::GetTestFilePath("fonts/bug_377948405.ttf");
  ASSERT_FALSE(font_path.empty());

  std::vector<uint8_t> font_data = GetFileContents(font_path.c_str());
  ASSERT_FALSE(font_data.empty());

  ScopedFPDFFont font(FPDFText_LoadFont(
      document(), font_data.data(), font_data.size(), FPDF_FONT_TRUETYPE, 1));
  ASSERT_TRUE(font);
  CPDF_Font* typed_font = CPDFFontFromFPDFFont(font.get());
  RetainPtr<const CPDF_Array> widths_array =
      GetWidthsArrayForCidFont(typed_font);
  ASSERT_TRUE(widths_array);
  EXPECT_EQ(widths_array->GetIntegerAt(0), 1);
  EXPECT_EQ(widths_array->GetIntegerAt(2), 5);
}

TEST_F(FPDFEditEmbedderTest, FormModifyObject) {
  ASSERT_TRUE(OpenDocument("form_object_with_image.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Since we know the document structure, assert the exact object count
  constexpr int kExpectedPageObjectCount = 1;
  ASSERT_EQ(kExpectedPageObjectCount, FPDFPage_CountObjects(page));

  FPDF_PAGEOBJECT form_obj = FPDFPage_GetObject(page, 0);
  ASSERT_TRUE(form_obj);
  ASSERT_EQ(FPDF_PAGEOBJ_FORM, FPDFPageObj_GetType(form_obj));

  // Get the count of objects in the form
  constexpr int kExpectedFormObjectCount = 1;
  ASSERT_EQ(kExpectedFormObjectCount, FPDFFormObj_CountObjects(form_obj));

  constexpr int kImageObjectIndex = 0;
  FPDF_PAGEOBJECT image_obj =
      FPDFFormObj_GetObject(form_obj, kImageObjectIndex);
  ASSERT_TRUE(image_obj);
  ASSERT_EQ(FPDF_PAGEOBJ_IMAGE, FPDFPageObj_GetType(image_obj));

  ASSERT_FALSE(FPDFFormObj_RemoveObject(nullptr, image_obj));
  ASSERT_FALSE(FPDFFormObj_RemoveObject(form_obj, nullptr));
  ASSERT_FALSE(FPDFFormObj_RemoveObject(nullptr, nullptr));
  ASSERT_TRUE(FPDFFormObj_RemoveObject(form_obj, image_obj));

  // After removing the image, the form should have no objects left
  ASSERT_EQ(0, FPDFFormObj_CountObjects(form_obj));

  FPDFPageObj_Destroy(image_obj);

  ASSERT_TRUE(FPDFPage_GenerateContent(page));

  // Save the document to the internal buffer
  EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

  // Unload the original page and close the document
  UnloadPage(page);
  CloseDocument();

  const char kExpectedChecksumAfterRemoval[] =
      "847febf1c7c38c1d2a1673cbea0bbe6b";
  constexpr int kPageWidth = 200;
  constexpr int kPageHeight = 300;

  VerifySavedDocument(kPageWidth, kPageHeight, kExpectedChecksumAfterRemoval);
}
