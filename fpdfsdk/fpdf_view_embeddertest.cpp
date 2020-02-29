// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/fpdf_view_c_api_test.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/file_util.h"
#include "testing/utils/path_service.h"

namespace {

#if defined(OS_WIN)
const char kExpectedRectanglePostScript[] = R"(
save
/im/initmatrix load def
/n/newpath load def/m/moveto load def/l/lineto load def/c/curveto load def/h/closepath load def
/f/fill load def/F/eofill load def/s/stroke load def/W/clip load def/W*/eoclip load def
/rg/setrgbcolor load def/k/setcmykcolor load def
/J/setlinecap load def/j/setlinejoin load def/w/setlinewidth load def/M/setmiterlimit load def/d/setdash load def
/q/gsave load def/Q/grestore load def/iM/imagemask load def
/Tj/show load def/Ff/findfont load def/Fs/scalefont load def/Sf/setfont load def
/cm/concat load def/Cm/currentmatrix load def/mx/matrix load def/sm/setmatrix load def
0 300 m 0 0 l 200 0 l 200 300 l 0 300 l h W n
q
0 300 m 0 0 l 200 0 l 200 300 l 0 300 l h W n
q
0 J
[]0 d
0 j
1 w
10 M
mx Cm [1 0 0 -1 0 300]cm 0 290 m 10 290 l 10 300 l 0 300 l 0 290 l h 0 0 0 rg
q F Q s sm
mx Cm [1 0 0 -1 0 300]cm 10 150 m 60 150 l 60 180 l 10 180 l 10 150 l h q F Q s sm
mx Cm [1 0 0 -1 0 300]cm 190 290 m 200 290 l 200 300 l 190 300 l 190 290 l h 0 0 1 rg
q F Q 0 0 0 rg
s sm
mx Cm [1 0 0 -1 0 300]cm 70 232 m 120 232 l 120 262 l 70 262 l 70 232 l h 0 0 1 rg
q F Q 0 0 0 rg
s sm
mx Cm [1 0 0 -1 0 300]cm 190 0 m 200 0 l 200 10 l 190 10 l 190 0 l h 0 1 0 rg
q F Q 0 0 0 rg
s sm
mx Cm [1 0 0 -1 0 300]cm 130 150 m 180 150 l 180 180 l 130 180 l 130 150 l h 0 1 0 rg
q F Q 0 0 0 rg
s sm
mx Cm [1 0 0 -1 0 300]cm 0 0 m 10 0 l 10 10 l 0 10 l 0 0 l h 1 0 0 rg
q F Q 0 0 0 rg
s sm
mx Cm [1 0 0 -1 0 300]cm 70 67 m 120 67 l 120 97 l 70 97 l 70 67 l h 1 0 0 rg
q F Q 0 0 0 rg
s sm
Q
Q
Q

restore
)";
#endif  // defined(OS_WIN)

class MockDownloadHints final : public FX_DOWNLOADHINTS {
 public:
  static void SAddSegment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
  }

  MockDownloadHints() {
    FX_DOWNLOADHINTS::version = 1;
    FX_DOWNLOADHINTS::AddSegment = SAddSegment;
  }

  ~MockDownloadHints() {}
};

}  // namespace

TEST(fpdf, CApiTest) {
  EXPECT_TRUE(CheckPDFiumCApi());
}

class FPDFViewEmbedderTest : public EmbedderTest {
 protected:
  void TestRenderPageBitmapWithMatrix(FPDF_PAGE page,
                                      int bitmap_width,
                                      int bitmap_height,
                                      const FS_MATRIX& matrix,
                                      const FS_RECTF& rect,
                                      const char* expected_md5) {
    ScopedFPDFBitmap bitmap(FPDFBitmap_Create(bitmap_width, bitmap_height, 0));
    FPDFBitmap_FillRect(bitmap.get(), 0, 0, bitmap_width, bitmap_height,
                        0xFFFFFFFF);
    FPDF_RenderPageBitmapWithMatrix(bitmap.get(), page, &matrix, &rect, 0);
    CompareBitmap(bitmap.get(), bitmap_width, bitmap_height, expected_md5);
  }

  void TestRenderPageBitmapWithFlags(FPDF_PAGE page,
                                     int flags,
                                     const char* expected_md5) {
    int bitmap_width = static_cast<int>(FPDF_GetPageWidth(page));
    int bitmap_height = static_cast<int>(FPDF_GetPageHeight(page));
    ScopedFPDFBitmap bitmap(FPDFBitmap_Create(bitmap_width, bitmap_height, 0));
    FPDFBitmap_FillRect(bitmap.get(), 0, 0, bitmap_width, bitmap_height,
                        0xFFFFFFFF);
    FPDF_RenderPageBitmap(bitmap.get(), page, 0, 0, bitmap_width, bitmap_height,
                          0, flags);
    CompareBitmap(bitmap.get(), bitmap_width, bitmap_height, expected_md5);
  }

  void TestRenderPageBitmapWithExternalMemory(FPDF_PAGE page,
                                              int format,
                                              const char* expected_md5) {
    int bitmap_width = static_cast<int>(FPDF_GetPageWidth(page));
    int bitmap_height = static_cast<int>(FPDF_GetPageHeight(page));
    int bytes_per_pixel = BytesPerPixelForFormat(format);
    ASSERT_NE(0, bytes_per_pixel);

    int bitmap_stride = bytes_per_pixel * bitmap_width;
    std::vector<uint8_t> external_memory(bitmap_stride * bitmap_height);
    ScopedFPDFBitmap bitmap(FPDFBitmap_CreateEx(bitmap_width, bitmap_height,
                                                format, external_memory.data(),
                                                bitmap_stride));
    FPDFBitmap_FillRect(bitmap.get(), 0, 0, bitmap_width, bitmap_height,
                        0xFFFFFFFF);
    FPDF_RenderPageBitmap(bitmap.get(), page, 0, 0, bitmap_width, bitmap_height,
                          0, 0);
    CompareBitmap(bitmap.get(), bitmap_width, bitmap_height, expected_md5);
  }
};

// Test for conversion of a point in device coordinates to page coordinates
TEST_F(FPDFViewEmbedderTest, DeviceCoordinatesToPageCoordinates) {
  EXPECT_TRUE(OpenDocument("about_blank.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);

  // Error tolerance for floating point comparison
  const double kTolerance = 0.0001;

  // Display bounds in device coordinates
  int start_x = 0;
  int start_y = 0;
  int size_x = 640;
  int size_y = 480;

  // Page Orientation normal
  int rotate = 0;

  // Device coordinate to be converted
  int device_x = 10;
  int device_y = 10;

  double page_x = 0.0;
  double page_y = 0.0;
  EXPECT_TRUE(FPDF_DeviceToPage(page, start_x, start_y, size_x, size_y, rotate,
                                device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(9.5625, page_x, kTolerance);
  EXPECT_NEAR(775.5, page_y, kTolerance);

  // Rotate 90 degrees clockwise
  rotate = 1;
  EXPECT_TRUE(FPDF_DeviceToPage(page, start_x, start_y, size_x, size_y, rotate,
                                device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(12.75, page_x, kTolerance);
  EXPECT_NEAR(12.375, page_y, kTolerance);

  // Rotate 180 degrees
  rotate = 2;
  EXPECT_TRUE(FPDF_DeviceToPage(page, start_x, start_y, size_x, size_y, rotate,
                                device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(602.4374, page_x, kTolerance);
  EXPECT_NEAR(16.5, page_y, kTolerance);

  // Rotate 90 degrees counter-clockwise
  rotate = 3;
  EXPECT_TRUE(FPDF_DeviceToPage(page, start_x, start_y, size_x, size_y, rotate,
                                device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(599.25, page_x, kTolerance);
  EXPECT_NEAR(779.625, page_y, kTolerance);

  // FPDF_DeviceToPage() converts |rotate| into legal rotation by taking
  // modulo by 4. A value of 4 is expected to be converted into 0 (normal
  // rotation)
  rotate = 4;
  EXPECT_TRUE(FPDF_DeviceToPage(page, start_x, start_y, size_x, size_y, rotate,
                                device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(9.5625, page_x, kTolerance);
  EXPECT_NEAR(775.5, page_y, kTolerance);

  // FPDF_DeviceToPage returns untransformed coordinates if |rotate| % 4 is
  // negative.
  rotate = -1;
  EXPECT_TRUE(FPDF_DeviceToPage(page, start_x, start_y, size_x, size_y, rotate,
                                device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(device_x, page_x, kTolerance);
  EXPECT_NEAR(device_y, page_y, kTolerance);

  // Negative case - invalid page
  page_x = 1234.0;
  page_y = 5678.0;
  EXPECT_FALSE(FPDF_DeviceToPage(nullptr, start_x, start_y, size_x, size_y,
                                 rotate, device_x, device_y, &page_x, &page_y));
  // Out parameters are expected to remain unchanged
  EXPECT_NEAR(1234.0, page_x, kTolerance);
  EXPECT_NEAR(5678.0, page_y, kTolerance);

  // Negative case - invalid output parameters
  EXPECT_FALSE(FPDF_DeviceToPage(page, start_x, start_y, size_x, size_y, rotate,
                                 device_x, device_y, nullptr, nullptr));

  UnloadPage(page);
}

// Test for conversion of a point in page coordinates to device coordinates.
TEST_F(FPDFViewEmbedderTest, PageCoordinatesToDeviceCoordinates) {
  EXPECT_TRUE(OpenDocument("about_blank.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);

  // Display bounds in device coordinates
  int start_x = 0;
  int start_y = 0;
  int size_x = 640;
  int size_y = 480;

  // Page Orientation normal
  int rotate = 0;

  // Page coordinate to be converted
  double page_x = 9.0;
  double page_y = 775.0;

  int device_x = 0;
  int device_y = 0;
  EXPECT_TRUE(FPDF_PageToDevice(page, start_x, start_y, size_x, size_y, rotate,
                                page_x, page_y, &device_x, &device_y));

  EXPECT_EQ(9, device_x);
  EXPECT_EQ(10, device_y);

  // Rotate 90 degrees clockwise
  rotate = 1;
  EXPECT_TRUE(FPDF_PageToDevice(page, start_x, start_y, size_x, size_y, rotate,
                                page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(626, device_x);
  EXPECT_EQ(7, device_y);

  // Rotate 180 degrees
  rotate = 2;
  EXPECT_TRUE(FPDF_PageToDevice(page, start_x, start_y, size_x, size_y, rotate,
                                page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(631, device_x);
  EXPECT_EQ(470, device_y);

  // Rotate 90 degrees counter-clockwise
  rotate = 3;
  EXPECT_TRUE(FPDF_PageToDevice(page, start_x, start_y, size_x, size_y, rotate,
                                page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(14, device_x);
  EXPECT_EQ(473, device_y);

  // FPDF_PageToDevice() converts |rotate| into legal rotation by taking
  // modulo by 4. A value of 4 is expected to be converted into 0 (normal
  // rotation)
  rotate = 4;
  EXPECT_TRUE(FPDF_PageToDevice(page, start_x, start_y, size_x, size_y, rotate,
                                page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(9, device_x);
  EXPECT_EQ(10, device_y);

  // FPDF_PageToDevice() returns untransformed coordinates if |rotate| % 4 is
  // negative.
  rotate = -1;
  EXPECT_TRUE(FPDF_PageToDevice(page, start_x, start_y, size_x, size_y, rotate,
                                page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(start_x, device_x);
  EXPECT_EQ(start_y, device_y);

  // Negative case - invalid page
  device_x = 1234;
  device_y = 5678;
  EXPECT_FALSE(FPDF_PageToDevice(nullptr, start_x, start_y, size_x, size_y,
                                 rotate, page_x, page_y, &device_x, &device_y));
  // Out parameters are expected to remain unchanged
  EXPECT_EQ(1234, device_x);
  EXPECT_EQ(5678, device_y);

  // Negative case - invalid output parameters
  EXPECT_FALSE(FPDF_PageToDevice(page, start_x, start_y, size_x, size_y, rotate,
                                 page_x, page_y, nullptr, nullptr));

  UnloadPage(page);
}

TEST_F(FPDFViewEmbedderTest, MultipleInitDestroy) {
  FPDF_InitLibrary();  // Redundant given call in SetUp(), but safe.
  FPDF_InitLibrary();  // Doubly-redundant even, but safe.

  EXPECT_TRUE(OpenDocument("about_blank.pdf"));
  CloseDocument();
  CloseDocument();  // Redundant given above, but safe.
  CloseDocument();  // Doubly-redundant even, but safe.

  FPDF_DestroyLibrary();  // Doubly-redundant even, but safe.
  FPDF_DestroyLibrary();  // Redundant given call in TearDown(), but safe.
}

TEST_F(FPDFViewEmbedderTest, Document) {
  EXPECT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_EQ(1, GetPageCount());
  EXPECT_EQ(0, GetFirstPageNum());

  int version;
  EXPECT_TRUE(FPDF_GetFileVersion(document(), &version));
  EXPECT_EQ(14, version);

  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
  EXPECT_EQ(-1, FPDF_GetSecurityHandlerRevision(document()));
}

TEST_F(FPDFViewEmbedderTest, LoadNonexistentDocument) {
  FPDF_DOCUMENT doc = FPDF_LoadDocument("nonexistent_document.pdf", "");
  ASSERT_FALSE(doc);
  EXPECT_EQ(static_cast<int>(FPDF_GetLastError()), FPDF_ERR_FILE);
}

// See https://crbug.com/pdfium/465
TEST_F(FPDFViewEmbedderTest, EmptyDocument) {
  EXPECT_TRUE(CreateEmptyDocument());
  {
    int version = 42;
    EXPECT_FALSE(FPDF_GetFileVersion(document(), &version));
    EXPECT_EQ(0, version);
  }
  {
#ifdef PDF_ENABLE_XFA
    const unsigned long kExpected = static_cast<uint32_t>(-1);
#else   // PDF_ENABLE_XFA
    const unsigned long kExpected = 0;
#endif  // PDF_ENABLE_XFA
    EXPECT_EQ(kExpected, FPDF_GetDocPermissions(document()));
  }
  EXPECT_EQ(-1, FPDF_GetSecurityHandlerRevision(document()));
  EXPECT_EQ(0, FPDF_GetPageCount(document()));
  EXPECT_TRUE(FPDF_VIEWERREF_GetPrintScaling(document()));
  EXPECT_EQ(1, FPDF_VIEWERREF_GetNumCopies(document()));
  EXPECT_EQ(DuplexUndefined, FPDF_VIEWERREF_GetDuplex(document()));

  char buf[100];
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetName(document(), "foo", nullptr, 0));
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetName(document(), "foo", buf, sizeof(buf)));
  EXPECT_EQ(0u, FPDF_CountNamedDests(document()));
}

TEST_F(FPDFViewEmbedderTest, SandboxDocument) {
  uint16_t buf[200];
  unsigned long len;

  ASSERT_TRUE(CreateEmptyDocument());
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_GT(len, 2u);  // Not just "double NUL" end-of-string indicator.
  EXPECT_NE(0u, buf[0]);
  CloseDocument();

  FPDF_SetSandBoxPolicy(FPDF_POLICY_MACHINETIME_ACCESS, false);
  ASSERT_TRUE(CreateEmptyDocument());
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_EQ(2u, len);  // Only a "double NUL" end-of-string indicator.
  EXPECT_EQ(0u, buf[0]);
  CloseDocument();

  constexpr unsigned long kNoSuchPolicy = 102;
  FPDF_SetSandBoxPolicy(kNoSuchPolicy, true);
  ASSERT_TRUE(CreateEmptyDocument());
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_EQ(2u, len);  // Only a "double NUL" end-of-string indicator.
  EXPECT_EQ(0u, buf[0]);
  CloseDocument();

  FPDF_SetSandBoxPolicy(FPDF_POLICY_MACHINETIME_ACCESS, true);
  ASSERT_TRUE(CreateEmptyDocument());
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_GT(len, 2u);  // Not just "double NUL" end-of-string indicator.
  EXPECT_NE(0u, buf[0]);
  CloseDocument();
}

TEST_F(FPDFViewEmbedderTest, LinearizedDocument) {
  EXPECT_TRUE(OpenDocumentLinearized("feature_linearized_loading.pdf"));
  int version;
  EXPECT_TRUE(FPDF_GetFileVersion(document(), &version));
  EXPECT_EQ(16, version);
}

TEST_F(FPDFViewEmbedderTest, LoadCustomDocumentWithoutFileAccess) {
  EXPECT_FALSE(FPDF_LoadCustomDocument(nullptr, ""));
}

// See https://crbug.com/pdfium/1261
TEST_F(FPDFViewEmbedderTest, LoadCustomDocumentWithShortLivedFileAccess) {
  std::string file_contents_string;  // Must outlive |doc|.
  ScopedFPDFDocument doc;
  {
    // Read a PDF, and copy it into |file_contents_string|.
    std::string pdf_path;
    size_t pdf_length;
    ASSERT_TRUE(PathService::GetTestFilePath("rectangles.pdf", &pdf_path));
    auto file_contents = GetFileContents(pdf_path.c_str(), &pdf_length);
    ASSERT_TRUE(file_contents);
    for (size_t i = 0; i < pdf_length; ++i)
      file_contents_string.push_back(file_contents.get()[i]);

    // Define a FPDF_FILEACCESS object that will go out of scope, while the
    // loaded document in |doc| remains valid.
    FPDF_FILEACCESS file_access = {};
    file_access.m_FileLen = pdf_length;
    file_access.m_GetBlock = GetBlockFromString;
    file_access.m_Param = &file_contents_string;
    doc.reset(FPDF_LoadCustomDocument(&file_access, nullptr));
    ASSERT_TRUE(doc);
  }

  // Now try to access |doc| and make sure it still works.
  ScopedFPDFPage page(FPDF_LoadPage(doc.get(), 0));
  ASSERT_TRUE(page);
  EXPECT_FLOAT_EQ(200.0f, FPDF_GetPageWidthF(page.get()));
  EXPECT_FLOAT_EQ(300.0f, FPDF_GetPageHeightF(page.get()));
}

TEST_F(FPDFViewEmbedderTest, Page) {
  EXPECT_TRUE(OpenDocument("about_blank.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  EXPECT_FLOAT_EQ(612.0f, FPDF_GetPageWidthF(page));
  EXPECT_FLOAT_EQ(792.0f, FPDF_GetPageHeightF(page));

  FS_RECTF rect;
  EXPECT_TRUE(FPDF_GetPageBoundingBox(page, &rect));
  EXPECT_EQ(0.0, rect.left);
  EXPECT_EQ(0.0, rect.bottom);
  EXPECT_EQ(612.0, rect.right);
  EXPECT_EQ(792.0, rect.top);

  // Null arguments return errors rather than crashing,
  EXPECT_EQ(0.0, FPDF_GetPageWidth(nullptr));
  EXPECT_EQ(0.0, FPDF_GetPageHeight(nullptr));
  EXPECT_FALSE(FPDF_GetPageBoundingBox(nullptr, &rect));
  EXPECT_FALSE(FPDF_GetPageBoundingBox(page, nullptr));

  UnloadPage(page);
  EXPECT_FALSE(LoadPage(1));
}

TEST_F(FPDFViewEmbedderTest, ViewerRefDummy) {
  EXPECT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_TRUE(FPDF_VIEWERREF_GetPrintScaling(document()));
  EXPECT_EQ(1, FPDF_VIEWERREF_GetNumCopies(document()));
  EXPECT_EQ(DuplexUndefined, FPDF_VIEWERREF_GetDuplex(document()));

  char buf[100];
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetName(document(), "foo", nullptr, 0));
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetName(document(), "foo", buf, sizeof(buf)));

  FPDF_PAGERANGE page_range = FPDF_VIEWERREF_GetPrintPageRange(document());
  EXPECT_FALSE(page_range);
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetPrintPageRangeCount(page_range));
  EXPECT_EQ(-1, FPDF_VIEWERREF_GetPrintPageRangeElement(page_range, 0));
  EXPECT_EQ(-1, FPDF_VIEWERREF_GetPrintPageRangeElement(page_range, 1));
}

TEST_F(FPDFViewEmbedderTest, ViewerRef) {
  EXPECT_TRUE(OpenDocument("viewer_ref.pdf"));
  EXPECT_TRUE(FPDF_VIEWERREF_GetPrintScaling(document()));
  EXPECT_EQ(5, FPDF_VIEWERREF_GetNumCopies(document()));
  EXPECT_EQ(DuplexUndefined, FPDF_VIEWERREF_GetDuplex(document()));

  // Test some corner cases.
  char buf[100];
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetName(document(), "", buf, sizeof(buf)));
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetName(document(), "foo", nullptr, 0));
  EXPECT_EQ(0U, FPDF_VIEWERREF_GetName(document(), "foo", buf, sizeof(buf)));

  // Make sure |buf| does not get written into when it appears to be too small.
  // NOLINTNEXTLINE(runtime/printf)
  strcpy(buf, "ABCD");
  EXPECT_EQ(4U, FPDF_VIEWERREF_GetName(document(), "Foo", buf, 1));
  EXPECT_STREQ("ABCD", buf);

  // Note "Foo" is a different key from "foo".
  EXPECT_EQ(4U,
            FPDF_VIEWERREF_GetName(document(), "Foo", nullptr, sizeof(buf)));
  ASSERT_EQ(4U, FPDF_VIEWERREF_GetName(document(), "Foo", buf, sizeof(buf)));
  EXPECT_STREQ("foo", buf);

  // Try to retrieve a boolean and an integer.
  EXPECT_EQ(
      0U, FPDF_VIEWERREF_GetName(document(), "HideToolbar", buf, sizeof(buf)));
  EXPECT_EQ(0U,
            FPDF_VIEWERREF_GetName(document(), "NumCopies", buf, sizeof(buf)));

  // Try more valid cases.
  ASSERT_EQ(4U,
            FPDF_VIEWERREF_GetName(document(), "Direction", buf, sizeof(buf)));
  EXPECT_STREQ("R2L", buf);
  ASSERT_EQ(8U,
            FPDF_VIEWERREF_GetName(document(), "ViewArea", buf, sizeof(buf)));
  EXPECT_STREQ("CropBox", buf);

  FPDF_PAGERANGE page_range = FPDF_VIEWERREF_GetPrintPageRange(document());
  EXPECT_TRUE(page_range);
  EXPECT_EQ(4U, FPDF_VIEWERREF_GetPrintPageRangeCount(page_range));
  EXPECT_EQ(0, FPDF_VIEWERREF_GetPrintPageRangeElement(page_range, 0));
  EXPECT_EQ(2, FPDF_VIEWERREF_GetPrintPageRangeElement(page_range, 1));
  EXPECT_EQ(4, FPDF_VIEWERREF_GetPrintPageRangeElement(page_range, 2));
  EXPECT_EQ(4, FPDF_VIEWERREF_GetPrintPageRangeElement(page_range, 3));
  EXPECT_EQ(-1, FPDF_VIEWERREF_GetPrintPageRangeElement(page_range, 4));
}

TEST_F(FPDFViewEmbedderTest, NamedDests) {
  EXPECT_TRUE(OpenDocument("named_dests.pdf"));
  long buffer_size;
  char fixed_buffer[512];
  FPDF_DEST dest;

  // Query the size of the first item.
  buffer_size = 2000000;  // Absurdly large, check not used for this case.
  dest = FPDF_GetNamedDest(document(), 0, nullptr, &buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(12, buffer_size);

  // Try to retrieve the first item with too small a buffer.
  buffer_size = 10;
  dest = FPDF_GetNamedDest(document(), 0, fixed_buffer, &buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(-1, buffer_size);

  // Try to retrieve the first item with correctly sized buffer. Item is
  // taken from Dests NameTree in named_dests.pdf.
  buffer_size = 12;
  dest = FPDF_GetNamedDest(document(), 0, fixed_buffer, &buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(12, buffer_size);
  EXPECT_EQ(std::string("F\0i\0r\0s\0t\0\0\0", 12),
            std::string(fixed_buffer, buffer_size));

  // Try to retrieve the second item with ample buffer. Item is taken
  // from Dests NameTree but has a sub-dictionary in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 1, fixed_buffer, &buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(10, buffer_size);
  EXPECT_EQ(std::string("N\0e\0x\0t\0\0\0", 10),
            std::string(fixed_buffer, buffer_size));

  // Try to retrieve third item with ample buffer. Item is taken
  // from Dests NameTree but has a bad sub-dictionary in named_dests.pdf.
  // in named_dests.pdf).
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 2, fixed_buffer, &buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  // Try to retrieve the forth item with ample buffer. Item is taken
  // from Dests NameTree but has a vale of the wrong type in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 3, fixed_buffer, &buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  // Try to retrieve fifth item with ample buffer. Item taken from the
  // old-style Dests dictionary object in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 4, fixed_buffer, &buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(30, buffer_size);
  EXPECT_EQ(std::string("F\0i\0r\0s\0t\0A\0l\0t\0e\0r\0n\0a\0t\0e\0\0\0", 30),
            std::string(fixed_buffer, buffer_size));

  // Try to retrieve sixth item with ample buffer. Item istaken from the
  // old-style Dests dictionary object but has a sub-dictionary in
  // named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 5, fixed_buffer, &buffer_size);
  EXPECT_NE(nullptr, dest);
  EXPECT_EQ(28, buffer_size);
  EXPECT_EQ(std::string("L\0a\0s\0t\0A\0l\0t\0e\0r\0n\0a\0t\0e\0\0\0", 28),
            std::string(fixed_buffer, buffer_size));

  // Try to retrieve non-existent item with ample buffer.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 6, fixed_buffer, &buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  // Try to underflow/overflow the integer index.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), std::numeric_limits<int>::max(),
                           fixed_buffer, &buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), std::numeric_limits<int>::min(),
                           fixed_buffer, &buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), -1, fixed_buffer, &buffer_size);
  EXPECT_EQ(nullptr, dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.
}

TEST_F(FPDFViewEmbedderTest, NamedDestsByName) {
  EXPECT_TRUE(OpenDocument("named_dests.pdf"));

  // Null pointer returns nullptr.
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), nullptr);
  EXPECT_EQ(nullptr, dest);

  // Empty string returns nullptr.
  dest = FPDF_GetNamedDestByName(document(), "");
  EXPECT_EQ(nullptr, dest);

  // Item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_NE(nullptr, dest);

  long ignore_len = 0;
  FPDF_DEST dest_by_index =
      FPDF_GetNamedDest(document(), 0, nullptr, &ignore_len);
  EXPECT_EQ(dest_by_index, dest);

  // Item from Dests dictionary.
  dest = FPDF_GetNamedDestByName(document(), "FirstAlternate");
  EXPECT_NE(nullptr, dest);

  ignore_len = 0;
  dest_by_index = FPDF_GetNamedDest(document(), 4, nullptr, &ignore_len);
  EXPECT_EQ(dest_by_index, dest);

  // Bad value type for item from Dests NameTree array.
  dest = FPDF_GetNamedDestByName(document(), "WrongType");
  EXPECT_EQ(nullptr, dest);

  // No such destination in either Dest NameTree or dictionary.
  dest = FPDF_GetNamedDestByName(document(), "Bogus");
  EXPECT_EQ(nullptr, dest);
}

// The following tests pass if the document opens without crashing.
TEST_F(FPDFViewEmbedderTest, Crasher_113) {
  EXPECT_TRUE(OpenDocument("bug_113.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher_451830) {
  // Document is damaged and can't be opened.
  EXPECT_FALSE(OpenDocument("bug_451830.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher_452455) {
  EXPECT_TRUE(OpenDocument("bug_452455.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  UnloadPage(page);
}

TEST_F(FPDFViewEmbedderTest, Crasher_454695) {
  // Document is damaged and can't be opened.
  EXPECT_FALSE(OpenDocument("bug_454695.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher_572871) {
  EXPECT_TRUE(OpenDocument("bug_572871.pdf"));
}

// It tests that document can still be loaded even the trailer has no 'Size'
// field if other information is right.
TEST_F(FPDFViewEmbedderTest, Failed_213) {
  EXPECT_TRUE(OpenDocument("bug_213.pdf"));
}

// The following tests pass if the document opens without infinite looping.
TEST_F(FPDFViewEmbedderTest, Hang_298) {
  EXPECT_FALSE(OpenDocument("bug_298.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher_773229) {
  EXPECT_TRUE(OpenDocument("bug_773229.pdf"));
}

// Test if the document opens without infinite looping.
// Previously this test will hang in a loop inside LoadAllCrossRefV4. After
// the fix, LoadAllCrossRefV4 will return false after detecting a cross
// reference loop. Cross references will be rebuilt successfully.
TEST_F(FPDFViewEmbedderTest, CrossRefV4Loop) {
  EXPECT_TRUE(OpenDocument("bug_xrefv4_loop.pdf"));
  MockDownloadHints hints;

  // Make sure calling FPDFAvail_IsDocAvail() on this file does not infinite
  // loop either. See bug 875.
  int ret = PDF_DATA_NOTAVAIL;
  while (ret == PDF_DATA_NOTAVAIL)
    ret = FPDFAvail_IsDocAvail(avail_, &hints);
  EXPECT_EQ(PDF_DATA_AVAIL, ret);
}

// The test should pass when circular references to ParseIndirectObject will not
// cause infinite loop.
TEST_F(FPDFViewEmbedderTest, Hang_343) {
  EXPECT_FALSE(OpenDocument("bug_343.pdf"));
}

// The test should pass when the absence of 'Contents' field in a signature
// dictionary will not cause an infinite loop in CPDF_SyntaxParser::GetObject().
TEST_F(FPDFViewEmbedderTest, Hang_344) {
  EXPECT_FALSE(OpenDocument("bug_344.pdf"));
}

// The test should pass when there is no infinite recursion in
// CPDF_SyntaxParser::GetString().
TEST_F(FPDFViewEmbedderTest, Hang_355) {
  EXPECT_FALSE(OpenDocument("bug_355.pdf"));
}
// The test should pass even when the file has circular references to pages.
TEST_F(FPDFViewEmbedderTest, Hang_360) {
  EXPECT_FALSE(OpenDocument("bug_360.pdf"));
}

// Deliberately damaged version of linearized.pdf with bad data in the shared
// object hint table.
TEST_F(FPDFViewEmbedderTest, Hang_1055) {
  EXPECT_TRUE(OpenDocumentLinearized("linearized_bug_1055.pdf"));
  int version;
  EXPECT_TRUE(FPDF_GetFileVersion(document(), &version));
  EXPECT_EQ(16, version);
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_FPDF_RenderPageBitmapWithMatrix \
  DISABLED_FPDF_RenderPageBitmapWithMatrix
#else
#define MAYBE_FPDF_RenderPageBitmapWithMatrix FPDF_RenderPageBitmapWithMatrix
#endif
TEST_F(FPDFViewEmbedderTest, MAYBE_FPDF_RenderPageBitmapWithMatrix) {
  const char kOriginalMD5[] = "0a90de37f52127619c3dfb642b5fa2fe";
  const char kClippedMD5[] = "a84cab93c102b9b9290fba3047ba702c";
  const char kTopLeftQuarterMD5[] = "f11a11137c8834389e31cf555a4a6979";
  const char kHoriStretchedMD5[] = "48ef9205941ed19691ccfa00d717187e";
  const char kRotated90ClockwiseMD5[] = "d8da2c7bf77521550d0f2752b9cf3482";
  const char kRotated180ClockwiseMD5[] = "0113386bb0bd45125bacc6dee78bfe78";
  const char kRotated270ClockwiseMD5[] = "a287e0f74ce203699cda89f9cc97a240";
  const char kMirrorHoriMD5[] = "6e8d7a6fde39d8e720fb9e620102918c";
  const char kMirrorVertMD5[] = "8f3a555ef9c0d5031831ae3715273707";
  const char kLargerTopLeftQuarterMD5[] = "172a2f4adafbadbe98017b1c025b9e27";
  const char kLargerMD5[] = "c806145641c3e6fc4e022c7065343749";
  const char kLargerClippedMD5[] = "091d3b1c7933c8f6945eb2cb41e588e9";
  const char kLargerRotatedMD5[] = "115f13353ebfc82ddb392d1f0059eb12";
  const char kLargerRotatedLandscapeMD5[] = "c901239d17d84ac84cb6f2124da71b0d";
  const char kLargerRotatedDiagonalMD5[] = "3d62417468bdaff0eb14391a0c30a3b1";
  const char kTileMD5[] = "0a190003c97220bf8877684c8d7e89cf";

  EXPECT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  const int page_width = static_cast<int>(FPDF_GetPageWidthF(page));
  const int page_height = static_cast<int>(FPDF_GetPageHeightF(page));
  EXPECT_EQ(200, page_width);
  EXPECT_EQ(300, page_height);

  ScopedFPDFBitmap bitmap = RenderLoadedPage(page);
  CompareBitmap(bitmap.get(), page_width, page_height, kOriginalMD5);

  FS_RECTF page_rect{0, 0, page_width, page_height};

  // Try rendering with an identity matrix. The output should be the same as
  // the RenderLoadedPage() output.
  FS_MATRIX identity_matrix{1, 0, 0, 1, 0, 0};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height, identity_matrix,
                                 page_rect, kOriginalMD5);

  // Again render with an identity matrix but with a smaller clipping rect.
  FS_RECTF middle_of_page_rect{page_width / 4, page_height / 4,
                               page_width * 3 / 4, page_height * 3 / 4};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height, identity_matrix,
                                 middle_of_page_rect, kClippedMD5);

  // Now render again with the image scaled smaller.
  FS_MATRIX half_scale_matrix{0.5, 0, 0, 0.5, 0, 0};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height,
                                 half_scale_matrix, page_rect,
                                 kTopLeftQuarterMD5);

  // Now render again with the image scaled larger horizontally (the right half
  // will be clipped).
  FS_MATRIX stretch_x_matrix{2, 0, 0, 1, 0, 0};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height,
                                 stretch_x_matrix, page_rect,
                                 kHoriStretchedMD5);

  // Try a 90 degree rotation clockwise but with the same bitmap size, so part
  // will be clipped.
  FS_MATRIX rotate_90_matrix{0, 1, -1, 0, page_width, 0};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height,
                                 rotate_90_matrix, page_rect,
                                 kRotated90ClockwiseMD5);

  // 180 degree rotation clockwise.
  FS_MATRIX rotate_180_matrix{-1, 0, 0, -1, page_width, page_height};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height,
                                 rotate_180_matrix, page_rect,
                                 kRotated180ClockwiseMD5);

  // 270 degree rotation clockwise.
  FS_MATRIX rotate_270_matrix{0, -1, 1, 0, 0, page_width};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height,
                                 rotate_270_matrix, page_rect,
                                 kRotated270ClockwiseMD5);

  // Mirror horizontally.
  FS_MATRIX mirror_hori_matrix{-1, 0, 0, 1, page_width, 0};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height,
                                 mirror_hori_matrix, page_rect, kMirrorHoriMD5);

  // Mirror vertically.
  FS_MATRIX mirror_vert_matrix{1, 0, 0, -1, 0, page_height};
  TestRenderPageBitmapWithMatrix(page, page_width, page_height,
                                 mirror_vert_matrix, page_rect, kMirrorVertMD5);

  // Tests rendering to a larger bitmap
  const int bitmap_width = page_width * 2;
  const int bitmap_height = page_height * 2;

  // Render using an identity matrix and the whole bitmap area as clipping rect.
  FS_RECTF bitmap_rect{0, 0, bitmap_width, bitmap_height};
  TestRenderPageBitmapWithMatrix(page, bitmap_width, bitmap_height,
                                 identity_matrix, bitmap_rect,
                                 kLargerTopLeftQuarterMD5);

  // Render using a scaling matrix to fill the larger bitmap.
  FS_MATRIX double_scale_matrix{2, 0, 0, 2, 0, 0};
  TestRenderPageBitmapWithMatrix(page, bitmap_width, bitmap_height,
                                 double_scale_matrix, bitmap_rect, kLargerMD5);

  // Render the larger image again but with clipping.
  FS_RECTF middle_of_bitmap_rect{bitmap_width / 4, bitmap_height / 4,
                                 bitmap_width * 3 / 4, bitmap_height * 3 / 4};
  TestRenderPageBitmapWithMatrix(page, bitmap_width, bitmap_height,
                                 double_scale_matrix, middle_of_bitmap_rect,
                                 kLargerClippedMD5);

  // On the larger bitmap, try a 90 degree rotation but with the same bitmap
  // size, so part will be clipped.
  FS_MATRIX rotate_90_scale_2_matrix{0, 2, -2, 0, bitmap_width, 0};
  TestRenderPageBitmapWithMatrix(page, bitmap_width, bitmap_height,
                                 rotate_90_scale_2_matrix, bitmap_rect,
                                 kLargerRotatedMD5);

  // On the larger bitmap, apply 90 degree rotation to a bitmap with the
  // appropriate dimensions.
  const int landscape_bitmap_width = bitmap_height;
  const int landscape_bitmap_height = bitmap_width;
  FS_RECTF landscape_bitmap_rect{0, 0, landscape_bitmap_width,
                                 landscape_bitmap_height};
  FS_MATRIX landscape_rotate_90_scale_2_matrix{
      0, 2, -2, 0, landscape_bitmap_width, 0};
  TestRenderPageBitmapWithMatrix(
      page, landscape_bitmap_width, landscape_bitmap_height,
      landscape_rotate_90_scale_2_matrix, landscape_bitmap_rect,
      kLargerRotatedLandscapeMD5);

  // On the larger bitmap, apply 45 degree rotation to a bitmap with the
  // appropriate dimensions.
  const float sqrt2 = 1.41421356f;
  const int diagonal_bitmap_size = ceil((bitmap_width + bitmap_height) / sqrt2);
  FS_RECTF diagonal_bitmap_rect{0, 0, diagonal_bitmap_size,
                                diagonal_bitmap_size};
  FS_MATRIX rotate_45_scale_2_matrix{
      sqrt2, sqrt2, -sqrt2, sqrt2, bitmap_height / sqrt2, 0};
  TestRenderPageBitmapWithMatrix(page, diagonal_bitmap_size,
                                 diagonal_bitmap_size, rotate_45_scale_2_matrix,
                                 diagonal_bitmap_rect,
                                 kLargerRotatedDiagonalMD5);

  // Render the (2, 1) tile of the page (third column, second row) when the page
  // is divided in 50x50 pixel tiles. The tile is scaled by a factor of 7.
  const float scale = 7.0;
  const int tile_size = 50;
  const int tile_x = 2;
  const int tile_y = 1;
  int tile_bitmap_size = scale * tile_size;
  FS_RECTF tile_bitmap_rect{0, 0, tile_bitmap_size, tile_bitmap_size};
  FS_MATRIX tile_2_1_matrix{scale,
                            0,
                            0,
                            scale,
                            -tile_x * tile_bitmap_size,
                            -tile_y * tile_bitmap_size};
  TestRenderPageBitmapWithMatrix(page, tile_bitmap_size, tile_bitmap_size,
                                 tile_2_1_matrix, tile_bitmap_rect, kTileMD5);

  UnloadPage(page);
}

TEST_F(FPDFViewEmbedderTest, FPDF_GetPageSizeByIndexF) {
  EXPECT_TRUE(OpenDocument("rectangles.pdf"));

  FS_SIZEF size;
  EXPECT_FALSE(FPDF_GetPageSizeByIndexF(nullptr, 0, &size));
  EXPECT_FALSE(FPDF_GetPageSizeByIndexF(document(), 0, nullptr));

  // Page -1 doesn't exist.
  EXPECT_FALSE(FPDF_GetPageSizeByIndexF(document(), -1, &size));

  // Page 1 doesn't exist.
  EXPECT_FALSE(FPDF_GetPageSizeByIndexF(document(), 1, &size));

  // Page 0 exists.
  EXPECT_TRUE(FPDF_GetPageSizeByIndexF(document(), 0, &size));
  EXPECT_FLOAT_EQ(200.0f, size.width);
  EXPECT_FLOAT_EQ(300.0f, size.height);

  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document());
#ifdef PDF_ENABLE_XFA
  // TODO(tsepez): XFA must obtain this size without parsing.
  EXPECT_EQ(1u, pDoc->GetParsedPageCountForTesting());
#else   // PDF_ENABLE_XFA
  EXPECT_EQ(0u, pDoc->GetParsedPageCountForTesting());
#endif  // PDF_ENABLE_XFA

  // Double-check against values from when page is actually parsed.
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_FLOAT_EQ(size.width, FPDF_GetPageWidthF(page));
  EXPECT_FLOAT_EQ(size.height, FPDF_GetPageHeightF(page));
  EXPECT_EQ(1u, pDoc->GetParsedPageCountForTesting());
  UnloadPage(page);
}

TEST_F(FPDFViewEmbedderTest, FPDF_GetPageSizeByIndex) {
  EXPECT_TRUE(OpenDocument("rectangles.pdf"));

  double width = 0;
  double height = 0;

  EXPECT_FALSE(FPDF_GetPageSizeByIndex(nullptr, 0, &width, &height));
  EXPECT_FALSE(FPDF_GetPageSizeByIndex(document(), 0, nullptr, &height));
  EXPECT_FALSE(FPDF_GetPageSizeByIndex(document(), 0, &width, nullptr));

  // Page -1 doesn't exist.
  EXPECT_FALSE(FPDF_GetPageSizeByIndex(document(), -1, &width, &height));

  // Page 1 doesn't exist.
  EXPECT_FALSE(FPDF_GetPageSizeByIndex(document(), 1, &width, &height));

  // Page 0 exists.
  EXPECT_TRUE(FPDF_GetPageSizeByIndex(document(), 0, &width, &height));
  EXPECT_EQ(200.0, width);
  EXPECT_EQ(300.0, height);

  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document());
#ifdef PDF_ENABLE_XFA
  // TODO(tsepez): XFA must obtain this size without parsing.
  EXPECT_EQ(1u, pDoc->GetParsedPageCountForTesting());
#else   // PDF_ENABLE_XFA
  EXPECT_EQ(0u, pDoc->GetParsedPageCountForTesting());
#endif  // PDF_ENABLE_XFA

  // Double-check against values from when page is actually parsed.
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(width, FPDF_GetPageWidth(page));
  EXPECT_EQ(height, FPDF_GetPageHeight(page));
  EXPECT_EQ(1u, pDoc->GetParsedPageCountForTesting());
  UnloadPage(page);
}

class RecordUnsupportedErrorDelegate final : public EmbedderTest::Delegate {
 public:
  RecordUnsupportedErrorDelegate() = default;
  ~RecordUnsupportedErrorDelegate() override = default;

  void UnsupportedHandler(int type) override { type_ = type; }

  int type_ = -1;
};

TEST_F(FPDFViewEmbedderTest, UnSupportedOperations_NotFound) {
  RecordUnsupportedErrorDelegate delegate;
  SetDelegate(&delegate);
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(delegate.type_, -1);
  SetDelegate(nullptr);
}

TEST_F(FPDFViewEmbedderTest, UnSupportedOperations_LoadCustomDocument) {
  RecordUnsupportedErrorDelegate delegate;
  SetDelegate(&delegate);
  ASSERT_TRUE(OpenDocument("unsupported_feature.pdf"));
  EXPECT_EQ(FPDF_UNSP_DOC_PORTABLECOLLECTION, delegate.type_);
  SetDelegate(nullptr);
}

TEST_F(FPDFViewEmbedderTest, UnSupportedOperations_LoadDocument) {
  std::string file_path;
  ASSERT_TRUE(
      PathService::GetTestFilePath("unsupported_feature.pdf", &file_path));

  RecordUnsupportedErrorDelegate delegate;
  SetDelegate(&delegate);
  FPDF_DOCUMENT doc = FPDF_LoadDocument(file_path.c_str(), "");
  EXPECT_TRUE(doc != nullptr);
  EXPECT_EQ(FPDF_UNSP_DOC_PORTABLECOLLECTION, delegate.type_);
  FPDF_CloseDocument(doc);
  SetDelegate(nullptr);
}

TEST_F(FPDFViewEmbedderTest, DocumentHasValidCrossReferenceTable) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_TRUE(FPDF_DocumentHasValidCrossReferenceTable(document()));
}

TEST_F(FPDFViewEmbedderTest, DocumentHasInvalidCrossReferenceTable) {
  EXPECT_FALSE(FPDF_DocumentHasValidCrossReferenceTable(nullptr));

  ASSERT_TRUE(OpenDocument("bug_664284.pdf"));
  EXPECT_FALSE(FPDF_DocumentHasValidCrossReferenceTable(document()));
}

// Related to https://crbug.com/pdfium/1197
TEST_F(FPDFViewEmbedderTest, LoadDocumentWithEmptyXRefConsistently) {
  ASSERT_TRUE(OpenDocument("empty_xref.pdf"));
  EXPECT_TRUE(FPDF_DocumentHasValidCrossReferenceTable(document()));

  std::string file_path;
  ASSERT_TRUE(PathService::GetTestFilePath("empty_xref.pdf", &file_path));
  {
    ScopedFPDFDocument doc(FPDF_LoadDocument(file_path.c_str(), ""));
    ASSERT_TRUE(doc);
    EXPECT_TRUE(FPDF_DocumentHasValidCrossReferenceTable(doc.get()));
  }
  {
    size_t file_length = 0;
    std::unique_ptr<char, pdfium::FreeDeleter> file_contents =
        GetFileContents(file_path.c_str(), &file_length);
    ASSERT(file_contents);
    ScopedFPDFDocument doc(
        FPDF_LoadMemDocument(file_contents.get(), file_length, ""));
    ASSERT_TRUE(doc);
    EXPECT_TRUE(FPDF_DocumentHasValidCrossReferenceTable(doc.get()));
  }
}

TEST_F(FPDFViewEmbedderTest, RenderManyRectanglesWithFlags) {
  static const char kNormalMD5[] = "b0170c575b65ecb93ebafada0ff0f038";
  static const char kGrayscaleMD5[] = "7b553f1052069a9c61237a05db0955d6";
  static const char kNoSmoothpathMD5[] = "ff6e5c509d1f6984bcdfd18b26a4203a";

  ASSERT_TRUE(OpenDocument("many_rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  TestRenderPageBitmapWithFlags(page, 0, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_ANNOT, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_LCD_TEXT, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_NO_NATIVETEXT, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_GRAYSCALE, kGrayscaleMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_LIMITEDIMAGECACHE,
                                kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_FORCEHALFTONE, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_PRINTING, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_NO_SMOOTHTEXT, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_NO_SMOOTHIMAGE, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_NO_SMOOTHPATH,
                                kNoSmoothpathMD5);

  UnloadPage(page);
}

TEST_F(FPDFViewEmbedderTest, RenderManyRectanglesWithExternalMemory) {
  ASSERT_TRUE(OpenDocument("many_rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  static const char kGrayMD5[] = "3dfe1fc3889123d68e1748fefac65e72";
  static const char kNormalMD5[] = "4e7e280c1597222afcb0ee3bb90ec119";

  // TODO(crbug.com/pdfium/1489): Add a test for FPDFBitmap_BGR in
  // Skia/SkiaPaths modes once Skia provides support for BGR24 format.
#else
  static const char kGrayMD5[] = "b561c11edc44dc3972125a9b8744fa2f";
  static const char kBgrMD5[] = "ab6312e04c0d3f4e46fb302a45173d05";
  static const char kNormalMD5[] = "b0170c575b65ecb93ebafada0ff0f038";

  TestRenderPageBitmapWithExternalMemory(page, FPDFBitmap_BGR, kBgrMD5);
#endif
  TestRenderPageBitmapWithExternalMemory(page, FPDFBitmap_Gray, kGrayMD5);
  TestRenderPageBitmapWithExternalMemory(page, FPDFBitmap_BGRx, kNormalMD5);
  TestRenderPageBitmapWithExternalMemory(page, FPDFBitmap_BGRA, kNormalMD5);

  UnloadPage(page);
}

#if defined(OS_LINUX)
TEST_F(FPDFViewEmbedderTest, RenderHelloWorldWithFlags) {
  static const char kNormalMD5[] = "2baa4c0e1758deba1b9c908e1fbd04ed";
  static const char kLcdTextMD5[] = "825e881f39e48254e64e2808987a6b8c";
  static const char kNoSmoothtextMD5[] = "3d01e234120b783a3fffb27273ea1ea8";

  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  TestRenderPageBitmapWithFlags(page, 0, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_ANNOT, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_LCD_TEXT, kLcdTextMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_NO_NATIVETEXT, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_GRAYSCALE, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_LIMITEDIMAGECACHE,
                                kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_FORCEHALFTONE, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_PRINTING, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_NO_SMOOTHTEXT,
                                kNoSmoothtextMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_NO_SMOOTHIMAGE, kNormalMD5);
  TestRenderPageBitmapWithFlags(page, FPDF_RENDER_NO_SMOOTHPATH, kNormalMD5);

  UnloadPage(page);
}
#endif  // defined(OS_LINUX)

#if defined(OS_WIN)
TEST_F(FPDFViewEmbedderTest, FPDFRenderPageEmf) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf_normal = RenderPageWithFlagsToEmf(page, 0);
  EXPECT_EQ(3772u, emf_normal.size());

  // FPDF_REVERSE_BYTE_ORDER is ignored since EMFs are always BGR.
  std::vector<uint8_t> emf_reverse_byte_order =
      RenderPageWithFlagsToEmf(page, FPDF_REVERSE_BYTE_ORDER);
  EXPECT_EQ(emf_normal, emf_reverse_byte_order);

  UnloadPage(page);
}

class PostScriptRenderEmbedderTestBase : public FPDFViewEmbedderTest {
 protected:
  ~PostScriptRenderEmbedderTestBase() override = default;

  // FPDFViewEmbedderTest:
  void TearDown() override {
    FPDF_SetPrintMode(FPDF_PRINTMODE_EMF);
    FPDFViewEmbedderTest::TearDown();
  }
};

class PostScriptLevel2EmbedderTest : public PostScriptRenderEmbedderTestBase {
 public:
  PostScriptLevel2EmbedderTest() = default;
  ~PostScriptLevel2EmbedderTest() override = default;

 protected:
  // FPDFViewEmbedderTest:
  void SetUp() override {
    FPDFViewEmbedderTest::SetUp();
    FPDF_SetPrintMode(FPDF_PRINTMODE_POSTSCRIPT2);
  }
};

class PostScriptLevel3EmbedderTest : public PostScriptRenderEmbedderTestBase {
 public:
  PostScriptLevel3EmbedderTest() = default;
  ~PostScriptLevel3EmbedderTest() override = default;

 protected:
  // FPDFViewEmbedderTest:
  void SetUp() override {
    FPDFViewEmbedderTest::SetUp();
    FPDF_SetPrintMode(FPDF_PRINTMODE_POSTSCRIPT3);
  }
};

TEST_F(PostScriptLevel2EmbedderTest, Rectangles) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf_normal = RenderPageWithFlagsToEmf(page, 0);
  std::string ps_data = GetPostScriptFromEmf(emf_normal);
  EXPECT_STREQ(kExpectedRectanglePostScript, ps_data.c_str());

  // FPDF_REVERSE_BYTE_ORDER is ignored since PostScript is not bitmap-based.
  std::vector<uint8_t> emf_reverse_byte_order =
      RenderPageWithFlagsToEmf(page, FPDF_REVERSE_BYTE_ORDER);
  EXPECT_EQ(emf_normal, emf_reverse_byte_order);

  UnloadPage(page);
}

TEST_F(PostScriptLevel3EmbedderTest, Rectangles) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf_normal = RenderPageWithFlagsToEmf(page, 0);
  std::string ps_data = GetPostScriptFromEmf(emf_normal);
  EXPECT_STREQ(kExpectedRectanglePostScript, ps_data.c_str());

  // FPDF_REVERSE_BYTE_ORDER is ignored since PostScript is not bitmap-based.
  std::vector<uint8_t> emf_reverse_byte_order =
      RenderPageWithFlagsToEmf(page, FPDF_REVERSE_BYTE_ORDER);
  EXPECT_EQ(emf_normal, emf_reverse_byte_order);

  UnloadPage(page);
}

TEST_F(PostScriptLevel2EmbedderTest, Image) {
  const char kExpected[] =
      "\n"
      "save\n"
      "/im/initmatrix load def\n"
      "/n/newpath load def/m/moveto load def/l/lineto load def/c/curveto load "
      "def/h/closepath load def\n"
      "/f/fill load def/F/eofill load def/s/stroke load def/W/clip load "
      "def/W*/eoclip load def\n"
      "/rg/setrgbcolor load def/k/setcmykcolor load def\n"
      "/J/setlinecap load def/j/setlinejoin load def/w/setlinewidth load "
      "def/M/setmiterlimit load def/d/setdash load def\n"
      "/q/gsave load def/Q/grestore load def/iM/imagemask load def\n"
      "/Tj/show load def/Ff/findfont load def/Fs/scalefont load def/Sf/setfont "
      "load def\n"
      "/cm/concat load def/Cm/currentmatrix load def/mx/matrix load "
      "def/sm/setmatrix load def\n"
      "0 792 m 0 0 l 612 0 l 612 792 l 0 792 l h W n\n"
      "q\n"
      "0 792 m 0 0 l 612 0 l 612 792 l 0 792 l h W n\n"
      "q\n"
      "Q\n"
      "q\n"
      "281 106.7 m 331 106.7 l 331 56.7 l 281 56.7 l 281 106.7 l h W* n\n"
      "q\n"
      "[49.9 0 0 -50 281.1 106.6]cm 50 50 8[50 0 0 -50 0 "
      "50]currentfile/ASCII85Decode filter /DCTDecode filter false 3 "
      "colorimage\n"
      "s4IA0!\"_al8O`[\\!<<*#!!*'\"s4[N@!!ic5#6k>;#6tJ?#m^kH'FbHY$Odmc'+Yct)"
      "BU\"@)B9_>\r\n"
      ",VCGe+tOrY*%3`p/2/e81c-:%3B]>W4>&EH1B6)/"
      "6NIK\"#n.1M(_$ok1*IV\\1,:U?1,:U?1,:U?\r\n"
      "1,:U?1,:U?1,:U?1,:U?1,:U?1,:U?1,:U?1,:U?1,:U?1,AmF!\"fJ:1&s'3!?qLF&HMtG!"
      "WU(<\r\n"
      "*rl9A\"T\\W)!<E3$z!!!!\"!WrQ/\"pYD?$4HmP!4<@<!W`B*!X&T/"
      "\"U\"r.!!.KK!WrE*&Hrdj0gQ!W\r\n"
      ";.0\\RE>10ZOeE%*6F\"?A;UOtZ1LbBV#mqFa(`=5<-7:2j.Ps\"@2`NfY6UX@47n?3D;"
      "cHat='/U/\r\n"
      "@q9._B4u!oF*)PJGBeCZK7nr5LPUeEP*;,qQC!u,R\\HRQV5C/"
      "hWN*81['d?O\\@K2f_o0O6a2lBF\r\n"
      "daQ^rf%8R-g>V&OjQ5OekiqC&o(2MHp@n@XqZ\"J6*ru?D!<E3%!<E3%!<<*\"!!!!\"!"
      "WrQ/\"pYD?\r\n"
      "$4HmP!4<C=!W`?*\"9Sc3\"U\"r.!<RHF!<N?8\"9fr'\"qj4!#@VTc+u4]T'LIqUZ,$_"
      "k1K*]W@WKj'\r\n"
      "(*k`q-1Mcg)&ahL-n-W'2E*TU3^Z;(7Rp!@8lJ\\h<``C+>%;)SAnPdkC3+K>G'A1VH@gd&"
      "KnbA=\r\n"
      "M2II[Pa.Q$R$jD;USO``Vl6SpZEppG[^WcW]#)A'`Q#s>ai`&\\eCE.%f\\,!<j5f="
      "akNM0qo(2MH\r\n"
      "p@n@XqZ#7L$j-M1!YGMH!'^JZre`+s!fAD!!fAD!!fAD!!fAD!!fAD!!fAD!!fAD!!fAD!!"
      "fAD!\r\n"
      "!fAD!!fAD!!fAD!!fAD!!fAD!!fAD!&-(;~>\n"
      "Q\n"
      "Q\n"
      "q\n"
      "q\n"
      "Q\n"
      "Q\n"
      "Q\n"
      "Q\n"
      "\n"
      "restore\n";

  ASSERT_TRUE(OpenDocument("tagged_alt_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf = RenderPageWithFlagsToEmf(page, 0);
  std::string ps_data = GetPostScriptFromEmf(emf);
  EXPECT_STREQ(kExpected, ps_data.c_str());

  UnloadPage(page);
}

TEST_F(PostScriptLevel3EmbedderTest, Image) {
  const char kExpected[] = R"(
save
/im/initmatrix load def
/n/newpath load def/m/moveto load def/l/lineto load def/c/curveto load def/h/closepath load def
/f/fill load def/F/eofill load def/s/stroke load def/W/clip load def/W*/eoclip load def
/rg/setrgbcolor load def/k/setcmykcolor load def
/J/setlinecap load def/j/setlinejoin load def/w/setlinewidth load def/M/setmiterlimit load def/d/setdash load def
/q/gsave load def/Q/grestore load def/iM/imagemask load def
/Tj/show load def/Ff/findfont load def/Fs/scalefont load def/Sf/setfont load def
/cm/concat load def/Cm/currentmatrix load def/mx/matrix load def/sm/setmatrix load def
0 792 m 0 0 l 612 0 l 612 792 l 0 792 l h W n
q
0 792 m 0 0 l 612 0 l 612 792 l 0 792 l h W n
q
Q
q
281 106.7 m 331 106.7 l 331 56.7 l 281 56.7 l 281 106.7 l h W* n
q
[49.9 0 0 -50 281.1 106.6]cm 50 50 8[50 0 0 -50 0 50]currentfile/ASCII85Decode filter /FlateDecode filter false 3 colorimage
Gb"0;0`_7S!5bE%:[N')TE"rlzGQSs[!!*~>
Q
Q
q
q
Q
Q
Q
Q

restore
)";

  ASSERT_TRUE(OpenDocument("tagged_alt_text.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf = RenderPageWithFlagsToEmf(page, 0);
  std::string ps_data = GetPostScriptFromEmf(emf);
  EXPECT_STREQ(kExpected, ps_data.c_str());

  UnloadPage(page);
}
#endif  // defined(OS_WIN)
