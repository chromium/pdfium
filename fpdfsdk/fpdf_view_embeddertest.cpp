// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/fpdf_view_c_api_test.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_constants.h"
#include "testing/embedder_test_environment.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/file_util.h"
#include "testing/utils/hash.h"
#include "testing/utils/path_service.h"

#if defined(PDF_USE_SKIA)
#include "third_party/skia/include/core/SkCanvas.h"           // nogncheck
#include "third_party/skia/include/core/SkColor.h"            // nogncheck
#include "third_party/skia/include/core/SkColorType.h"        // nogncheck
#include "third_party/skia/include/core/SkImage.h"            // nogncheck
#include "third_party/skia/include/core/SkImageInfo.h"        // nogncheck
#include "third_party/skia/include/core/SkPicture.h"          // nogncheck
#include "third_party/skia/include/core/SkPictureRecorder.h"  // nogncheck
#include "third_party/skia/include/core/SkRefCnt.h"           // nogncheck
#include "third_party/skia/include/core/SkSize.h"             // nogncheck
#include "third_party/skia/include/core/SkSurface.h"          // nogncheck
#endif  // defined(PDF_USE_SKIA)

using pdfium::ManyRectanglesChecksum;

namespace {

constexpr char kFirstAlternate[] = "FirstAlternate";
constexpr char kLastAlternate[] = "LastAlternate";

#if BUILDFLAG(IS_WIN)
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
#endif  // BUILDFLAG(IS_WIN)

class MockDownloadHints final : public FX_DOWNLOADHINTS {
 public:
  static void SAddSegment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
  }

  MockDownloadHints() {
    FX_DOWNLOADHINTS::version = 1;
    FX_DOWNLOADHINTS::AddSegment = SAddSegment;
  }

  ~MockDownloadHints() = default;
};

#if defined(PDF_USE_SKIA)
ScopedFPDFBitmap SkImageToPdfiumBitmap(const SkImage& image) {
  ScopedFPDFBitmap bitmap(
      FPDFBitmap_Create(image.width(), image.height(), /*alpha=*/1));
  if (!bitmap) {
    ADD_FAILURE() << "Could not create FPDF_BITMAP";
    return nullptr;
  }

  if (!image.readPixels(/*context=*/nullptr,
                        image.imageInfo().makeColorType(kBGRA_8888_SkColorType),
                        FPDFBitmap_GetBuffer(bitmap.get()),
                        FPDFBitmap_GetStride(bitmap.get()),
                        /*srcX=*/0, /*srcY=*/0)) {
    ADD_FAILURE() << "Could not read pixels from SkImage";
    return nullptr;
  }

  return bitmap;
}

ScopedFPDFBitmap SkPictureToPdfiumBitmap(sk_sp<SkPicture> picture,
                                         const SkISize& size) {
  sk_sp<SkSurface> surface =
      SkSurfaces::Raster(SkImageInfo::MakeN32Premul(size));
  if (!surface) {
    ADD_FAILURE() << "Could not create SkSurface";
    return nullptr;
  }

  surface->getCanvas()->clear(SK_ColorWHITE);
  surface->getCanvas()->drawPicture(picture);
  sk_sp<SkImage> image = surface->makeImageSnapshot();
  if (!image) {
    ADD_FAILURE() << "Could not snapshot SkSurface";
    return nullptr;
  }

  return SkImageToPdfiumBitmap(*image);
}
#endif  // defined(PDF_USE_SKIA)

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
                                      const char* expected_checksum) {
    ScopedFPDFBitmap bitmap(FPDFBitmap_Create(bitmap_width, bitmap_height, 0));
    ASSERT_TRUE(FPDFBitmap_FillRect(bitmap.get(), 0, 0, bitmap_width,
                                    bitmap_height, 0xFFFFFFFF));
    FPDF_RenderPageBitmapWithMatrix(bitmap.get(), page, &matrix, &rect, 0);
    CompareBitmap(bitmap.get(), bitmap_width, bitmap_height, expected_checksum);
  }

  void TestRenderPageBitmapWithFlags(FPDF_PAGE page,
                                     int flags,
                                     const char* expected_checksum) {
    int bitmap_width = static_cast<int>(FPDF_GetPageWidth(page));
    int bitmap_height = static_cast<int>(FPDF_GetPageHeight(page));
    ScopedFPDFBitmap bitmap(FPDFBitmap_Create(bitmap_width, bitmap_height, 0));
    ASSERT_TRUE(FPDFBitmap_FillRect(bitmap.get(), 0, 0, bitmap_width,
                                    bitmap_height, 0xFFFFFFFF));
    FPDF_RenderPageBitmap(bitmap.get(), page, 0, 0, bitmap_width, bitmap_height,
                          0, flags);
    CompareBitmap(bitmap.get(), bitmap_width, bitmap_height, expected_checksum);
  }

  void TestRenderPageBitmapWithInternalMemory(FPDF_PAGE page,
                                              int format,
                                              const char* expected_checksum) {
    TestRenderPageBitmapWithInternalMemoryAndStride(
        page, format, /*bitmap_stride=*/0, expected_checksum);
  }

  void TestRenderPageBitmapWithInternalMemoryAndStride(
      FPDF_PAGE page,
      int format,
      int bitmap_stride,
      const char* expected_checksum) {
    int bitmap_width = static_cast<int>(FPDF_GetPageWidth(page));
    int bitmap_height = static_cast<int>(FPDF_GetPageHeight(page));
    int bytes_per_pixel = BytesPerPixelForFormat(format);
    ASSERT_NE(0, bytes_per_pixel);

    ScopedFPDFBitmap bitmap(FPDFBitmap_CreateEx(
        bitmap_width, bitmap_height, format, nullptr, bitmap_stride));
    RenderPageToBitmapAndCheck(page, bitmap.get(), expected_checksum);
  }

  void TestRenderPageBitmapWithExternalMemory(FPDF_PAGE page,
                                              int format,
                                              const char* expected_checksum) {
    int bitmap_width = static_cast<int>(FPDF_GetPageWidth(page));
    int bytes_per_pixel = BytesPerPixelForFormat(format);
    ASSERT_NE(0, bytes_per_pixel);

    int bitmap_stride = bytes_per_pixel * bitmap_width;
    return TestRenderPageBitmapWithExternalMemoryImpl(
        page, format, bitmap_stride, expected_checksum);
  }

  void TestRenderPageBitmapWithExternalMemoryAndNoStride(
      FPDF_PAGE page,
      int format,
      const char* expected_checksum) {
    return TestRenderPageBitmapWithExternalMemoryImpl(
        page, format, /*bitmap_stride=*/0, expected_checksum);
  }

#if defined(PDF_USE_SKIA)
  void TestRenderPageSkp(FPDF_PAGE page, const char* expected_checksum) {
    int width = static_cast<int>(FPDF_GetPageWidth(page));
    int height = static_cast<int>(FPDF_GetPageHeight(page));

    sk_sp<SkPicture> picture;
    {
      auto recorder = std::make_unique<SkPictureRecorder>();
      recorder->beginRecording(width, height);

      FPDF_RenderPageSkia(
          FPDFSkiaCanvasFromSkCanvas(recorder->getRecordingCanvas()), page,
          width, height);
      picture = recorder->finishRecordingAsPicture();
      ASSERT_TRUE(picture);
    }

    ScopedFPDFBitmap bitmap = SkPictureToPdfiumBitmap(
        std::move(picture), SkISize::Make(width, height));
    CompareBitmap(bitmap.get(), width, height, expected_checksum);
  }
#endif  // defined(PDF_USE_SKIA)

 private:
  void TestRenderPageBitmapWithExternalMemoryImpl(
      FPDF_PAGE page,
      int format,
      int bitmap_stride,
      const char* expected_checksum) {
    int bitmap_width = static_cast<int>(FPDF_GetPageWidth(page));
    int bitmap_height = static_cast<int>(FPDF_GetPageHeight(page));

    std::vector<uint8_t> external_memory(bitmap_stride * bitmap_height);
    ScopedFPDFBitmap bitmap(FPDFBitmap_CreateEx(bitmap_width, bitmap_height,
                                                format, external_memory.data(),
                                                bitmap_stride));
    RenderPageToBitmapAndCheck(page, bitmap.get(), expected_checksum);
  }

  void RenderPageToBitmapAndCheck(FPDF_PAGE page,
                                  FPDF_BITMAP bitmap,
                                  const char* expected_checksum) {
    int bitmap_width = FPDFBitmap_GetWidth(bitmap);
    int bitmap_height = FPDFBitmap_GetHeight(bitmap);
    EXPECT_EQ(bitmap_width, static_cast<int>(FPDF_GetPageWidth(page)));
    EXPECT_EQ(bitmap_height, static_cast<int>(FPDF_GetPageHeight(page)));
    ASSERT_TRUE(FPDFBitmap_FillRect(bitmap, 0, 0, bitmap_width, bitmap_height,
                                    0xFFFFFFFF));
    FPDF_RenderPageBitmap(bitmap, page, 0, 0, bitmap_width, bitmap_height, 0,
                          FPDF_ANNOT);
    CompareBitmap(bitmap, bitmap_width, bitmap_height, expected_checksum);
  }
};

// Test for conversion of a point in device coordinates to page coordinates
TEST_F(FPDFViewEmbedderTest, DeviceCoordinatesToPageCoordinates) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

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
  EXPECT_TRUE(FPDF_DeviceToPage(page.get(), start_x, start_y, size_x, size_y,
                                rotate, device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(9.5625, page_x, kTolerance);
  EXPECT_NEAR(775.5, page_y, kTolerance);

  // Rotate 90 degrees clockwise
  rotate = 1;
  EXPECT_TRUE(FPDF_DeviceToPage(page.get(), start_x, start_y, size_x, size_y,
                                rotate, device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(12.75, page_x, kTolerance);
  EXPECT_NEAR(12.375, page_y, kTolerance);

  // Rotate 180 degrees
  rotate = 2;
  EXPECT_TRUE(FPDF_DeviceToPage(page.get(), start_x, start_y, size_x, size_y,
                                rotate, device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(602.4374, page_x, kTolerance);
  EXPECT_NEAR(16.5, page_y, kTolerance);

  // Rotate 90 degrees counter-clockwise
  rotate = 3;
  EXPECT_TRUE(FPDF_DeviceToPage(page.get(), start_x, start_y, size_x, size_y,
                                rotate, device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(599.25, page_x, kTolerance);
  EXPECT_NEAR(779.625, page_y, kTolerance);

  // FPDF_DeviceToPage() converts |rotate| into legal rotation by taking
  // modulo by 4. A value of 4 is expected to be converted into 0 (normal
  // rotation)
  rotate = 4;
  EXPECT_TRUE(FPDF_DeviceToPage(page.get(), start_x, start_y, size_x, size_y,
                                rotate, device_x, device_y, &page_x, &page_y));
  EXPECT_NEAR(9.5625, page_x, kTolerance);
  EXPECT_NEAR(775.5, page_y, kTolerance);

  // FPDF_DeviceToPage returns untransformed coordinates if |rotate| % 4 is
  // negative.
  rotate = -1;
  EXPECT_TRUE(FPDF_DeviceToPage(page.get(), start_x, start_y, size_x, size_y,
                                rotate, device_x, device_y, &page_x, &page_y));
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
  EXPECT_FALSE(FPDF_DeviceToPage(page.get(), start_x, start_y, size_x, size_y,
                                 rotate, device_x, device_y, nullptr, nullptr));
}

// Test for conversion of a point in page coordinates to device coordinates.
TEST_F(FPDFViewEmbedderTest, PageCoordinatesToDeviceCoordinates) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

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
  EXPECT_TRUE(FPDF_PageToDevice(page.get(), start_x, start_y, size_x, size_y,
                                rotate, page_x, page_y, &device_x, &device_y));

  EXPECT_EQ(9, device_x);
  EXPECT_EQ(10, device_y);

  // Rotate 90 degrees clockwise
  rotate = 1;
  EXPECT_TRUE(FPDF_PageToDevice(page.get(), start_x, start_y, size_x, size_y,
                                rotate, page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(626, device_x);
  EXPECT_EQ(7, device_y);

  // Rotate 180 degrees
  rotate = 2;
  EXPECT_TRUE(FPDF_PageToDevice(page.get(), start_x, start_y, size_x, size_y,
                                rotate, page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(631, device_x);
  EXPECT_EQ(470, device_y);

  // Rotate 90 degrees counter-clockwise
  rotate = 3;
  EXPECT_TRUE(FPDF_PageToDevice(page.get(), start_x, start_y, size_x, size_y,
                                rotate, page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(14, device_x);
  EXPECT_EQ(473, device_y);

  // FPDF_PageToDevice() converts |rotate| into legal rotation by taking
  // modulo by 4. A value of 4 is expected to be converted into 0 (normal
  // rotation)
  rotate = 4;
  EXPECT_TRUE(FPDF_PageToDevice(page.get(), start_x, start_y, size_x, size_y,
                                rotate, page_x, page_y, &device_x, &device_y));
  EXPECT_EQ(9, device_x);
  EXPECT_EQ(10, device_y);

  // FPDF_PageToDevice() returns untransformed coordinates if |rotate| % 4 is
  // negative.
  rotate = -1;
  EXPECT_TRUE(FPDF_PageToDevice(page.get(), start_x, start_y, size_x, size_y,
                                rotate, page_x, page_y, &device_x, &device_y));
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
  EXPECT_FALSE(FPDF_PageToDevice(page.get(), start_x, start_y, size_x, size_y,
                                 rotate, page_x, page_y, nullptr, nullptr));
}

TEST_F(FPDFViewEmbedderTest, MultipleInitDestroy) {
  FPDF_InitLibrary();  // Redundant given SetUp() in environment, but safe.
  FPDF_InitLibrary();  // Doubly-redundant even, but safe.

  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  CloseDocument();
  CloseDocument();  // Redundant given above, but safe.
  CloseDocument();  // Doubly-redundant even, but safe.

  FPDF_DestroyLibrary();  // Doubly-Redundant even, but safe.
  FPDF_DestroyLibrary();  // Redundant given call to TearDown(), but safe.

  EmbedderTestEnvironment::GetInstance()->TearDown();
  EmbedderTestEnvironment::GetInstance()->SetUp();
}

TEST_F(FPDFViewEmbedderTest, RepeatedInitDestroy) {
  for (int i = 0; i < 3; ++i) {
    if (!OpenDocument("about_blank.pdf"))
      ADD_FAILURE();
    CloseDocument();

    FPDF_DestroyLibrary();
    FPDF_InitLibrary();
  }

  // Puts the test environment back the way it was.
  EmbedderTestEnvironment::GetInstance()->TearDown();
  EmbedderTestEnvironment::GetInstance()->SetUp();
}

TEST_F(FPDFViewEmbedderTest, Document) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_EQ(1, GetPageCount());
  EXPECT_EQ(0, GetFirstPageNum());

  int version;
  EXPECT_TRUE(FPDF_GetFileVersion(document(), &version));
  EXPECT_EQ(14, version);

  // 0xFFFFFFFF because no security handler present
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocUserPermissions(document()));
  EXPECT_EQ(-1, FPDF_GetSecurityHandlerRevision(document()));
  CloseDocument();

  // Safe to open again and do the same things all over again.
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  EXPECT_EQ(1, GetPageCount());
  EXPECT_EQ(0, GetFirstPageNum());

  version = 42;
  EXPECT_TRUE(FPDF_GetFileVersion(document(), &version));
  EXPECT_EQ(14, version);

  // 0xFFFFFFFF because no security handler present
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocPermissions(document()));
  EXPECT_EQ(0xFFFFFFFF, FPDF_GetDocUserPermissions(document()));
  EXPECT_EQ(-1, FPDF_GetSecurityHandlerRevision(document()));
  // CloseDocument() called by TearDown().
}

TEST_F(FPDFViewEmbedderTest, LoadDocument64) {
  std::string file_path = PathService::GetTestFilePath("about_blank.pdf");
  ASSERT_FALSE(file_path.empty());

  std::vector<uint8_t> file_contents = GetFileContents(file_path.c_str());
  ASSERT_FALSE(file_contents.empty());
  ScopedFPDFDocument doc(FPDF_LoadMemDocument64(file_contents.data(),
                                                file_contents.size(), nullptr));
  ASSERT_TRUE(doc);

  int version;
  EXPECT_TRUE(FPDF_GetFileVersion(doc.get(), &version));
  EXPECT_EQ(14, version);
}

TEST_F(FPDFViewEmbedderTest, LoadNonexistentDocument) {
  FPDF_DOCUMENT doc = FPDF_LoadDocument("nonexistent_document.pdf", "");
  ASSERT_FALSE(doc);
  EXPECT_EQ(static_cast<int>(FPDF_GetLastError()), FPDF_ERR_FILE);
}

TEST_F(FPDFViewEmbedderTest, DocumentWithNoPageCount) {
  ASSERT_TRUE(OpenDocument("no_page_count.pdf"));
  ASSERT_EQ(6, FPDF_GetPageCount(document()));
}

TEST_F(FPDFViewEmbedderTest, DocumentWithEmptyPageTreeNode) {
  ASSERT_TRUE(OpenDocument("page_tree_empty_node.pdf"));
  ASSERT_EQ(2, FPDF_GetPageCount(document()));
}

// See https://crbug.com/pdfium/465
TEST_F(FPDFViewEmbedderTest, EmptyDocument) {
  CreateEmptyDocument();
  {
    int version = 42;
    EXPECT_FALSE(FPDF_GetFileVersion(document(), &version));
    EXPECT_EQ(0, version);
  }
  EXPECT_EQ(0U, FPDF_GetDocPermissions(document()));
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

  CreateEmptyDocument();
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_GT(len, 2u);  // Not just "double NUL" end-of-string indicator.
  EXPECT_NE(0u, buf[0]);
  CloseDocument();

  FPDF_SetSandBoxPolicy(FPDF_POLICY_MACHINETIME_ACCESS, false);
  CreateEmptyDocument();
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_EQ(2u, len);  // Only a "double NUL" end-of-string indicator.
  EXPECT_EQ(0u, buf[0]);
  CloseDocument();

  constexpr unsigned long kNoSuchPolicy = 102;
  FPDF_SetSandBoxPolicy(kNoSuchPolicy, true);
  CreateEmptyDocument();
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_EQ(2u, len);  // Only a "double NUL" end-of-string indicator.
  EXPECT_EQ(0u, buf[0]);
  CloseDocument();

  FPDF_SetSandBoxPolicy(FPDF_POLICY_MACHINETIME_ACCESS, true);
  CreateEmptyDocument();
  len = FPDF_GetMetaText(document(), "CreationDate", buf, sizeof(buf));
  EXPECT_GT(len, 2u);  // Not just "double NUL" end-of-string indicator.
  EXPECT_NE(0u, buf[0]);
  CloseDocument();
}

TEST_F(FPDFViewEmbedderTest, LinearizedDocument) {
  ASSERT_TRUE(OpenDocumentLinearized("feature_linearized_loading.pdf"));
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
    std::string pdf_path = PathService::GetTestFilePath("rectangles.pdf");
    ASSERT_FALSE(pdf_path.empty());
    std::vector<uint8_t> file_contents = GetFileContents(pdf_path.c_str());
    ASSERT_FALSE(file_contents.empty());
    std::copy(file_contents.begin(), file_contents.end(),
              std::back_inserter(file_contents_string));

    // Define a FPDF_FILEACCESS object that will go out of scope, while the
    // loaded document in |doc| remains valid.
    FPDF_FILEACCESS file_access = {};
    file_access.m_FileLen = file_contents_string.size();
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
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);

  EXPECT_FLOAT_EQ(612.0f, FPDF_GetPageWidthF(page.get()));
  EXPECT_FLOAT_EQ(792.0f, FPDF_GetPageHeightF(page.get()));

  FS_RECTF rect;
  EXPECT_TRUE(FPDF_GetPageBoundingBox(page.get(), &rect));
  EXPECT_EQ(0.0, rect.left);
  EXPECT_EQ(0.0, rect.bottom);
  EXPECT_EQ(612.0, rect.right);
  EXPECT_EQ(792.0, rect.top);

  // Null arguments return errors rather than crashing,
  EXPECT_EQ(0.0, FPDF_GetPageWidth(nullptr));
  EXPECT_EQ(0.0, FPDF_GetPageHeight(nullptr));
  EXPECT_FALSE(FPDF_GetPageBoundingBox(nullptr, &rect));
  EXPECT_FALSE(FPDF_GetPageBoundingBox(page.get(), nullptr));

  EXPECT_FALSE(LoadScopedPage(1));
}

TEST_F(FPDFViewEmbedderTest, ViewerRefDummy) {
  ASSERT_TRUE(OpenDocument("about_blank.pdf"));
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
  ASSERT_TRUE(OpenDocument("viewer_ref.pdf"));
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
  EXPECT_EQ(4U, FPDF_VIEWERREF_GetName(document(), "Foo", nullptr, 0));
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
  ASSERT_TRUE(OpenDocument("named_dests.pdf"));
  EXPECT_EQ(6u, FPDF_CountNamedDests(document()));

  long buffer_size;
  char fixed_buffer[512];
  FPDF_DEST dest;

  // Query the size of the first item.
  buffer_size = 2000000;  // Absurdly large, check not used for this case.
  dest = FPDF_GetNamedDest(document(), 0, nullptr, &buffer_size);
  EXPECT_TRUE(dest);
  EXPECT_EQ(12, buffer_size);

  // Try to retrieve the first item with too small a buffer.
  buffer_size = 10;
  dest = FPDF_GetNamedDest(document(), 0, fixed_buffer, &buffer_size);
  EXPECT_TRUE(dest);
  EXPECT_EQ(-1, buffer_size);

  // Try to retrieve the first item with correctly sized buffer. Item is
  // taken from Dests NameTree in named_dests.pdf.
  buffer_size = 12;
  dest = FPDF_GetNamedDest(document(), 0, fixed_buffer, &buffer_size);
  EXPECT_TRUE(dest);
  EXPECT_EQ(12, buffer_size);
  EXPECT_EQ("First",
            GetPlatformString(reinterpret_cast<FPDF_WIDESTRING>(fixed_buffer)));

  // Try to retrieve the second item with ample buffer. Item is taken
  // from Dests NameTree but has a sub-dictionary in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 1, fixed_buffer, &buffer_size);
  EXPECT_TRUE(dest);
  EXPECT_EQ(10, buffer_size);
  EXPECT_EQ("Next",
            GetPlatformString(reinterpret_cast<FPDF_WIDESTRING>(fixed_buffer)));

  // Try to retrieve third item with ample buffer. Item is taken
  // from Dests NameTree but has a bad sub-dictionary in named_dests.pdf.
  // in named_dests.pdf).
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 2, fixed_buffer, &buffer_size);
  EXPECT_FALSE(dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  // Try to retrieve the forth item with ample buffer. Item is taken
  // from Dests NameTree but has a vale of the wrong type in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 3, fixed_buffer, &buffer_size);
  EXPECT_FALSE(dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  // Try to retrieve fifth item with ample buffer. Item taken from the
  // old-style Dests dictionary object in named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 4, fixed_buffer, &buffer_size);
  EXPECT_TRUE(dest);
  EXPECT_EQ(30, buffer_size);
  EXPECT_EQ(kFirstAlternate,
            GetPlatformString(reinterpret_cast<FPDF_WIDESTRING>(fixed_buffer)));

  // Try to retrieve sixth item with ample buffer. Item istaken from the
  // old-style Dests dictionary object but has a sub-dictionary in
  // named_dests.pdf.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 5, fixed_buffer, &buffer_size);
  EXPECT_TRUE(dest);
  EXPECT_EQ(28, buffer_size);
  EXPECT_EQ(kLastAlternate,
            GetPlatformString(reinterpret_cast<FPDF_WIDESTRING>(fixed_buffer)));

  // Try to retrieve non-existent item with ample buffer.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), 6, fixed_buffer, &buffer_size);
  EXPECT_FALSE(dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  // Try to underflow/overflow the integer index.
  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), std::numeric_limits<int>::max(),
                           fixed_buffer, &buffer_size);
  EXPECT_FALSE(dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), std::numeric_limits<int>::min(),
                           fixed_buffer, &buffer_size);
  EXPECT_FALSE(dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.

  buffer_size = sizeof(fixed_buffer);
  dest = FPDF_GetNamedDest(document(), -1, fixed_buffer, &buffer_size);
  EXPECT_FALSE(dest);
  EXPECT_EQ(sizeof(fixed_buffer),
            static_cast<size_t>(buffer_size));  // unmodified.
}

TEST_F(FPDFViewEmbedderTest, NamedDestsByName) {
  ASSERT_TRUE(OpenDocument("named_dests.pdf"));

  // Null pointer returns nullptr.
  FPDF_DEST dest = FPDF_GetNamedDestByName(document(), nullptr);
  EXPECT_FALSE(dest);

  // Empty string returns nullptr.
  dest = FPDF_GetNamedDestByName(document(), "");
  EXPECT_FALSE(dest);

  // Item from Dests NameTree.
  dest = FPDF_GetNamedDestByName(document(), "First");
  EXPECT_TRUE(dest);

  long ignore_len = 0;
  FPDF_DEST dest_by_index =
      FPDF_GetNamedDest(document(), 0, nullptr, &ignore_len);
  EXPECT_EQ(dest_by_index, dest);

  // Item from Dests dictionary.
  dest = FPDF_GetNamedDestByName(document(), kFirstAlternate);
  EXPECT_TRUE(dest);

  ignore_len = 0;
  dest_by_index = FPDF_GetNamedDest(document(), 4, nullptr, &ignore_len);
  EXPECT_EQ(dest_by_index, dest);

  // Bad value type for item from Dests NameTree array.
  dest = FPDF_GetNamedDestByName(document(), "WrongType");
  EXPECT_FALSE(dest);

  // No such destination in either Dest NameTree or dictionary.
  dest = FPDF_GetNamedDestByName(document(), "Bogus");
  EXPECT_FALSE(dest);
}

TEST_F(FPDFViewEmbedderTest, NamedDestsOldStyle) {
  ASSERT_TRUE(OpenDocument("named_dests_old_style.pdf"));
  EXPECT_EQ(2u, FPDF_CountNamedDests(document()));

  // Test bad parameters.
  EXPECT_FALSE(FPDF_GetNamedDestByName(document(), nullptr));
  EXPECT_FALSE(FPDF_GetNamedDestByName(document(), ""));
  EXPECT_FALSE(FPDF_GetNamedDestByName(document(), "NoSuchName"));

  // These should return a valid destination.
  EXPECT_TRUE(FPDF_GetNamedDestByName(document(), kFirstAlternate));
  EXPECT_TRUE(FPDF_GetNamedDestByName(document(), kLastAlternate));

  char buffer[512];
  constexpr long kBufferSize = sizeof(buffer);
  long size = kBufferSize;

  // Test bad indices.
  EXPECT_FALSE(FPDF_GetNamedDest(document(), -1, buffer, &size));
  EXPECT_EQ(kBufferSize, size);
  size = kBufferSize;
  EXPECT_FALSE(FPDF_GetNamedDest(document(), 2, buffer, &size));
  EXPECT_EQ(kBufferSize, size);

  // These should return a valid destination.
  size = kBufferSize;
  ASSERT_TRUE(FPDF_GetNamedDest(document(), 0, buffer, &size));
  ASSERT_EQ(static_cast<int>(sizeof(kFirstAlternate) * 2), size);
  EXPECT_EQ(kFirstAlternate,
            GetPlatformString(reinterpret_cast<FPDF_WIDESTRING>(buffer)));
  size = kBufferSize;
  ASSERT_TRUE(FPDF_GetNamedDest(document(), 1, buffer, &size));
  ASSERT_EQ(static_cast<int>(sizeof(kLastAlternate) * 2), size);
  EXPECT_EQ(kLastAlternate,
            GetPlatformString(reinterpret_cast<FPDF_WIDESTRING>(buffer)));
}

// The following tests pass if the document opens without crashing.
TEST_F(FPDFViewEmbedderTest, Crasher113) {
  ASSERT_TRUE(OpenDocument("bug_113.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher451830) {
  // Document is damaged and can't be opened.
  EXPECT_FALSE(OpenDocument("bug_451830.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher452455) {
  ASSERT_TRUE(OpenDocument("bug_452455.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  EXPECT_TRUE(page);
}

TEST_F(FPDFViewEmbedderTest, Crasher454695) {
  // Document is damaged and can't be opened.
  EXPECT_FALSE(OpenDocument("bug_454695.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher572871) {
  ASSERT_TRUE(OpenDocument("bug_572871.pdf"));
}

// It tests that document can still be loaded even the trailer has no 'Size'
// field if other information is right.
TEST_F(FPDFViewEmbedderTest, Failed213) {
  ASSERT_TRUE(OpenDocument("bug_213.pdf"));
}

// The following tests pass if the document opens without infinite looping.
TEST_F(FPDFViewEmbedderTest, Hang298) {
  EXPECT_FALSE(OpenDocument("bug_298.pdf"));
}

TEST_F(FPDFViewEmbedderTest, Crasher773229) {
  ASSERT_TRUE(OpenDocument("bug_773229.pdf"));
}

// Test if the document opens without infinite looping.
// Previously this test will hang in a loop inside LoadAllCrossRefV4. After
// the fix, LoadAllCrossRefV4 will return false after detecting a cross
// reference loop. Cross references will be rebuilt successfully.
TEST_F(FPDFViewEmbedderTest, CrossRefV4Loop) {
  ASSERT_TRUE(OpenDocument("bug_xrefv4_loop.pdf"));
  MockDownloadHints hints;

  // Make sure calling FPDFAvail_IsDocAvail() on this file does not infinite
  // loop either. See bug 875.
  int ret = PDF_DATA_NOTAVAIL;
  while (ret == PDF_DATA_NOTAVAIL)
    ret = FPDFAvail_IsDocAvail(avail(), &hints);
  EXPECT_EQ(PDF_DATA_AVAIL, ret);
}

// The test should pass when circular references to ParseIndirectObject will not
// cause infinite loop.
TEST_F(FPDFViewEmbedderTest, Hang343) {
  EXPECT_FALSE(OpenDocument("bug_343.pdf"));
}

// The test should pass when the absence of 'Contents' field in a signature
// dictionary will not cause an infinite loop in CPDF_SyntaxParser::GetObject().
TEST_F(FPDFViewEmbedderTest, Hang344) {
  EXPECT_FALSE(OpenDocument("bug_344.pdf"));
}

// The test should pass when there is no infinite recursion in
// CPDF_SyntaxParser::GetString().
TEST_F(FPDFViewEmbedderTest, Hang355) {
  EXPECT_FALSE(OpenDocument("bug_355.pdf"));
}
// The test should pass even when the file has circular references to pages.
TEST_F(FPDFViewEmbedderTest, Hang360) {
  EXPECT_FALSE(OpenDocument("bug_360.pdf"));
}

// Deliberately damaged version of linearized.pdf with bad data in the shared
// object hint table.
TEST_F(FPDFViewEmbedderTest, Hang1055) {
  ASSERT_TRUE(OpenDocumentLinearized("linearized_bug_1055.pdf"));
  int version;
  EXPECT_TRUE(FPDF_GetFileVersion(document(), &version));
  EXPECT_EQ(16, version);
}

TEST_F(FPDFViewEmbedderTest, FPDFRenderPageBitmapWithMatrix) {
  const char* clipped_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "d2929fae285593cd1c1d446750d47d60";
    }
    return "a84cab93c102b9b9290fba3047ba702c";
  }();
  const char* top_left_quarter_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "31d24d8c6a2bac380b2f5c393e77ecc9";
    }
    return "f11a11137c8834389e31cf555a4a6979";
  }();
  const char* rotated_90_clockwise_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "b4baa001d201baed576cd6d5d0d5a160";
    }
    return "d8da2c7bf77521550d0f2752b9cf3482";
  }();
  const char* rotated_180_clockwise_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "51819227d0863222aed366d5d7c5d9c8";
    }
    return "0113386bb0bd45125bacc6dee78bfe78";
  }();
  const char* rotated_270_clockwise_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "f2b046e46c2751cebc777a9725ae2f3e";
    }
    return "a287e0f74ce203699cda89f9cc97a240";
  }();
  const char* mirror_hori_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "c7fbec322b4fc6bcf46ec1eb89661c41";
    }
    return "6e8d7a6fde39d8e720fb9e620102918c";
  }();
  const char* mirror_vert_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "a8b00bc40677a73c15a08b9769d1b576";
    }
    return "8f3a555ef9c0d5031831ae3715273707";
  }();
  const char* larger_top_left_quarter_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "35deb5ed4b73675ce33f68328a33c687";
    }
    return "172a2f4adafbadbe98017b1c025b9e27";
  }();
  const char* larger_rotated_diagonal_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "a7179bc24e329341a1a1f6d6be20a1e9";
    }
    return "3d62417468bdaff0eb14391a0c30a3b1";
  }();
  const char* tile_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "387be3a84774f39aaa955314d2fe7106";
    }
    return "0a190003c97220bf8877684c8d7e89cf";
  }();
  const char kHoriStretchedChecksum[] = "48ef9205941ed19691ccfa00d717187e";
  const char kLargerChecksum[] = "c806145641c3e6fc4e022c7065343749";
  const char kLargerClippedChecksum[] = "091d3b1c7933c8f6945eb2cb41e588e9";
  const char kLargerRotatedChecksum[] = "115f13353ebfc82ddb392d1f0059eb12";
  const char kLargerRotatedLandscapeChecksum[] =
      "c901239d17d84ac84cb6f2124da71b0d";

  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  const float page_width = FPDF_GetPageWidthF(page.get());
  const float page_height = FPDF_GetPageHeightF(page.get());
  EXPECT_FLOAT_EQ(200, page_width);
  EXPECT_FLOAT_EQ(300, page_height);

  using pdfium::RectanglesChecksum;
  ScopedFPDFBitmap bitmap = RenderLoadedPage(page.get());
  CompareBitmap(bitmap.get(), page_width, page_height, RectanglesChecksum());

  FS_RECTF page_rect{0, 0, page_width, page_height};

  // Try rendering with an identity matrix. The output should be the same as
  // the RenderLoadedPage() output.
  FS_MATRIX identity_matrix{1, 0, 0, 1, 0, 0};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 identity_matrix, page_rect,
                                 RectanglesChecksum());

  // Again render with an identity matrix but with a smaller clipping rect.
  FS_RECTF middle_of_page_rect{page_width / 4, page_height / 4,
                               page_width * 3 / 4, page_height * 3 / 4};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 identity_matrix, middle_of_page_rect,
                                 clipped_checksum);

  // Now render again with the image scaled smaller.
  FS_MATRIX half_scale_matrix{0.5, 0, 0, 0.5, 0, 0};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 half_scale_matrix, page_rect,
                                 top_left_quarter_checksum);

  // Now render again with the image scaled larger horizontally (the right half
  // will be clipped).
  FS_MATRIX stretch_x_matrix{2, 0, 0, 1, 0, 0};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 stretch_x_matrix, page_rect,
                                 kHoriStretchedChecksum);

  // Try a 90 degree rotation clockwise but with the same bitmap size, so part
  // will be clipped.
  FS_MATRIX rotate_90_matrix{0, 1, -1, 0, page_width, 0};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 rotate_90_matrix, page_rect,
                                 rotated_90_clockwise_checksum);

  // 180 degree rotation clockwise.
  FS_MATRIX rotate_180_matrix{-1, 0, 0, -1, page_width, page_height};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 rotate_180_matrix, page_rect,
                                 rotated_180_clockwise_checksum);

  // 270 degree rotation clockwise.
  FS_MATRIX rotate_270_matrix{0, -1, 1, 0, 0, page_width};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 rotate_270_matrix, page_rect,
                                 rotated_270_clockwise_checksum);

  // Mirror horizontally.
  FS_MATRIX mirror_hori_matrix{-1, 0, 0, 1, page_width, 0};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 mirror_hori_matrix, page_rect,
                                 mirror_hori_checksum);

  // Mirror vertically.
  FS_MATRIX mirror_vert_matrix{1, 0, 0, -1, 0, page_height};
  TestRenderPageBitmapWithMatrix(page.get(), page_width, page_height,
                                 mirror_vert_matrix, page_rect,
                                 mirror_vert_checksum);

  // Tests rendering to a larger bitmap
  const float bitmap_width = page_width * 2;
  const float bitmap_height = page_height * 2;

  // Render using an identity matrix and the whole bitmap area as clipping rect.
  FS_RECTF bitmap_rect{0, 0, bitmap_width, bitmap_height};
  TestRenderPageBitmapWithMatrix(page.get(), bitmap_width, bitmap_height,
                                 identity_matrix, bitmap_rect,
                                 larger_top_left_quarter_checksum);

  // Render using a scaling matrix to fill the larger bitmap.
  FS_MATRIX double_scale_matrix{2, 0, 0, 2, 0, 0};
  TestRenderPageBitmapWithMatrix(page.get(), bitmap_width, bitmap_height,
                                 double_scale_matrix, bitmap_rect,
                                 kLargerChecksum);

  // Render the larger image again but with clipping.
  FS_RECTF middle_of_bitmap_rect{bitmap_width / 4, bitmap_height / 4,
                                 bitmap_width * 3 / 4, bitmap_height * 3 / 4};
  TestRenderPageBitmapWithMatrix(page.get(), bitmap_width, bitmap_height,
                                 double_scale_matrix, middle_of_bitmap_rect,
                                 kLargerClippedChecksum);

  // On the larger bitmap, try a 90 degree rotation but with the same bitmap
  // size, so part will be clipped.
  FS_MATRIX rotate_90_scale_2_matrix{0, 2, -2, 0, bitmap_width, 0};
  TestRenderPageBitmapWithMatrix(page.get(), bitmap_width, bitmap_height,
                                 rotate_90_scale_2_matrix, bitmap_rect,
                                 kLargerRotatedChecksum);

  // On the larger bitmap, apply 90 degree rotation to a bitmap with the
  // appropriate dimensions.
  const float landscape_bitmap_width = bitmap_height;
  const float landscape_bitmap_height = bitmap_width;
  FS_RECTF landscape_bitmap_rect{0, 0, landscape_bitmap_width,
                                 landscape_bitmap_height};
  FS_MATRIX landscape_rotate_90_scale_2_matrix{
      0, 2, -2, 0, landscape_bitmap_width, 0};
  TestRenderPageBitmapWithMatrix(
      page.get(), landscape_bitmap_width, landscape_bitmap_height,
      landscape_rotate_90_scale_2_matrix, landscape_bitmap_rect,
      kLargerRotatedLandscapeChecksum);

  // On the larger bitmap, apply 45 degree rotation to a bitmap with the
  // appropriate dimensions.
  const float sqrt2 = 1.41421356f;
  const float diagonal_bitmap_size =
      ceil((bitmap_width + bitmap_height) / sqrt2);
  FS_RECTF diagonal_bitmap_rect{0, 0, diagonal_bitmap_size,
                                diagonal_bitmap_size};
  FS_MATRIX rotate_45_scale_2_matrix{
      sqrt2, sqrt2, -sqrt2, sqrt2, bitmap_height / sqrt2, 0};
  TestRenderPageBitmapWithMatrix(page.get(), diagonal_bitmap_size,
                                 diagonal_bitmap_size, rotate_45_scale_2_matrix,
                                 diagonal_bitmap_rect,
                                 larger_rotated_diagonal_checksum);

  // Render the (2, 1) tile of the page (third column, second row) when the page
  // is divided in 50x50 pixel tiles. The tile is scaled by a factor of 7.
  const float scale = 7.0;
  const int tile_size = 50;
  const int tile_x = 2;
  const int tile_y = 1;
  float tile_bitmap_size = scale * tile_size;
  FS_RECTF tile_bitmap_rect{0, 0, tile_bitmap_size, tile_bitmap_size};
  FS_MATRIX tile_2_1_matrix{scale,
                            0,
                            0,
                            scale,
                            -tile_x * tile_bitmap_size,
                            -tile_y * tile_bitmap_size};
  TestRenderPageBitmapWithMatrix(page.get(), tile_bitmap_size, tile_bitmap_size,
                                 tile_2_1_matrix, tile_bitmap_rect,
                                 tile_checksum);
}

TEST_F(FPDFViewEmbedderTest, FPDFGetPageSizeByIndexF) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));

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
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_FLOAT_EQ(size.width, FPDF_GetPageWidthF(page.get()));
  EXPECT_FLOAT_EQ(size.height, FPDF_GetPageHeightF(page.get()));
  EXPECT_EQ(1u, pDoc->GetParsedPageCountForTesting());
}

TEST_F(FPDFViewEmbedderTest, FPDFGetPageSizeByIndex) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));

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
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  EXPECT_EQ(width, FPDF_GetPageWidth(page.get()));
  EXPECT_EQ(height, FPDF_GetPageHeight(page.get()));
  EXPECT_EQ(1u, pDoc->GetParsedPageCountForTesting());
}

TEST_F(FPDFViewEmbedderTest, GetXFAArrayData) {
  static constexpr struct {
    int index;
    const char* name;
    size_t content_length;
    const char* content_checksum;
  } kTestCases[]{
      {0, "preamble", 124u, "71be364e53292596412242bfcdb46eab"},
      {1, "config", 642u, "bcd1ca1d420ee31a561273a54a06435f"},
      {2, "template", 541u, "0f48cb2fa1bb9cbf9eee802d66e81bf4"},
      {3, "localeSet", 3455u, "bb1f253d3e5c719ac0da87d055bc164e"},
      {4, "postamble", 11u, "6b79e25da35d86634ea27c38f64cf243"},
  };

  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
  ASSERT_EQ(static_cast<int>(std::size(kTestCases)),
            FPDF_GetXFAPacketCount(document()));

  for (const auto& testcase : kTestCases) {
    char name_buffer[20] = {};
    ASSERT_EQ(strlen(testcase.name) + 1,
              FPDF_GetXFAPacketName(document(), testcase.index, nullptr, 0));
    EXPECT_EQ(strlen(testcase.name) + 1,
              FPDF_GetXFAPacketName(document(), testcase.index, name_buffer,
                                    sizeof(name_buffer)));
    EXPECT_STREQ(testcase.name, name_buffer);

    unsigned long buflen;
    ASSERT_TRUE(FPDF_GetXFAPacketContent(document(), testcase.index, nullptr, 0,
                                         &buflen));
    ASSERT_EQ(testcase.content_length, buflen);
    std::vector<uint8_t> data_buffer(buflen);
    EXPECT_TRUE(FPDF_GetXFAPacketContent(document(), testcase.index,
                                         data_buffer.data(), data_buffer.size(),
                                         &buflen));
    EXPECT_EQ(testcase.content_length, buflen);
    EXPECT_EQ(testcase.content_checksum, GenerateMD5Base16(data_buffer));
  }

  // Test bad parameters.
  EXPECT_EQ(-1, FPDF_GetXFAPacketCount(nullptr));

  EXPECT_EQ(0u, FPDF_GetXFAPacketName(nullptr, 0, nullptr, 0));
  EXPECT_EQ(0u, FPDF_GetXFAPacketName(document(), -1, nullptr, 0));
  EXPECT_EQ(
      0u, FPDF_GetXFAPacketName(document(), std::size(kTestCases), nullptr, 0));

  unsigned long buflen = 123;
  EXPECT_FALSE(FPDF_GetXFAPacketContent(nullptr, 0, nullptr, 0, &buflen));
  EXPECT_EQ(123u, buflen);
  EXPECT_FALSE(FPDF_GetXFAPacketContent(document(), -1, nullptr, 0, &buflen));
  EXPECT_EQ(123u, buflen);
  EXPECT_FALSE(FPDF_GetXFAPacketContent(document(), std::size(kTestCases),
                                        nullptr, 0, &buflen));
  EXPECT_EQ(123u, buflen);
  EXPECT_FALSE(FPDF_GetXFAPacketContent(document(), 0, nullptr, 0, nullptr));
}

TEST_F(FPDFViewEmbedderTest, GetXFAStreamData) {
  ASSERT_TRUE(OpenDocument("bug_1265.pdf"));

  ASSERT_EQ(1, FPDF_GetXFAPacketCount(document()));

  char name_buffer[20] = {};
  ASSERT_EQ(1u, FPDF_GetXFAPacketName(document(), 0, nullptr, 0));
  EXPECT_EQ(1u, FPDF_GetXFAPacketName(document(), 0, name_buffer,
                                      sizeof(name_buffer)));
  EXPECT_STREQ("", name_buffer);

  unsigned long buflen;
  ASSERT_TRUE(FPDF_GetXFAPacketContent(document(), 0, nullptr, 0, &buflen));
  ASSERT_EQ(121u, buflen);
  std::vector<uint8_t> data_buffer(buflen);
  EXPECT_TRUE(FPDF_GetXFAPacketContent(document(), 0, data_buffer.data(),
                                       data_buffer.size(), &buflen));
  EXPECT_EQ(121u, buflen);
  EXPECT_EQ("8f912eaa1e66c9341cb3032ede71e147", GenerateMD5Base16(data_buffer));
}

TEST_F(FPDFViewEmbedderTest, GetXFADataForNoForm) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  EXPECT_EQ(0, FPDF_GetXFAPacketCount(document()));
}

TEST_F(FPDFViewEmbedderTest, GetXFADataForAcroForm) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));

  EXPECT_EQ(0, FPDF_GetXFAPacketCount(document()));
}

class RecordUnsupportedErrorDelegate final : public EmbedderTest::Delegate {
 public:
  RecordUnsupportedErrorDelegate() = default;
  ~RecordUnsupportedErrorDelegate() override = default;

  void UnsupportedHandler(int type) override { type_ = type; }

  int type_ = -1;
};

TEST_F(FPDFViewEmbedderTest, UnSupportedOperationsNotFound) {
  RecordUnsupportedErrorDelegate delegate;
  SetDelegate(&delegate);
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(delegate.type_, -1);
  SetDelegate(nullptr);
}

TEST_F(FPDFViewEmbedderTest, UnSupportedOperationsLoadCustomDocument) {
  RecordUnsupportedErrorDelegate delegate;
  SetDelegate(&delegate);
  ASSERT_TRUE(OpenDocument("unsupported_feature.pdf"));
  EXPECT_EQ(FPDF_UNSP_DOC_PORTABLECOLLECTION, delegate.type_);
  SetDelegate(nullptr);
}

TEST_F(FPDFViewEmbedderTest, UnSupportedOperationsLoadDocument) {
  std::string file_path =
      PathService::GetTestFilePath("unsupported_feature.pdf");
  ASSERT_FALSE(file_path.empty());

  RecordUnsupportedErrorDelegate delegate;
  SetDelegate(&delegate);
  {
    ScopedFPDFDocument doc(FPDF_LoadDocument(file_path.c_str(), ""));
    EXPECT_TRUE(doc);
    EXPECT_EQ(FPDF_UNSP_DOC_PORTABLECOLLECTION, delegate.type_);
  }
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

  std::string file_path = PathService::GetTestFilePath("empty_xref.pdf");
  ASSERT_FALSE(file_path.empty());
  {
    ScopedFPDFDocument doc(FPDF_LoadDocument(file_path.c_str(), ""));
    ASSERT_TRUE(doc);
    EXPECT_TRUE(FPDF_DocumentHasValidCrossReferenceTable(doc.get()));
  }
  {
    std::vector<uint8_t> file_contents = GetFileContents(file_path.c_str());
    ASSERT_FALSE(file_contents.empty());
    ScopedFPDFDocument doc(
        FPDF_LoadMemDocument(file_contents.data(), file_contents.size(), ""));
    ASSERT_TRUE(doc);
    EXPECT_TRUE(FPDF_DocumentHasValidCrossReferenceTable(doc.get()));
  }
}

TEST_F(FPDFViewEmbedderTest, RenderBug664284WithNoNativeText) {
  // For Skia, since the font used in bug_664284.pdf is not a CID font,
  // ShouldDrawDeviceText() will always return true. Therefore
  // FPDF_NO_NATIVETEXT and the font widths defined in the PDF determines
  // whether to go through the rendering path in
  // CFX_SkiaDeviceDriver::DrawDeviceText(). In this case, it returns false and
  // affects the rendering results across all platforms.

  // For AGG, since CFX_AggDeviceDriver::DrawDeviceText() always returns false,
  // FPDF_NO_NATIVETEXT won't affect device-specific rendering path and it will
  // only disable native text support on macOS. Therefore Windows and Linux
  // rendering results remain the same as rendering with no flags, while the
  // macOS rendering result doesn't.

  const char* original_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "1c5d8217aca4f6fa86a8ed192f34b210";
#elif BUILDFLAG(IS_APPLE)
      return "b7ac2ca2b934f4e213ab4ba36c5f8ffd";
#else
      return "29cb8045c21cfa2c920fdf43de70efd8";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "0e339d606aafb63077f49e238dc27cb0";
#else
    return "288502887ffc63291f35a0573b944375";
#endif
  }();
  static const char kNoNativeTextChecksum[] =
      "288502887ffc63291f35a0573b944375";
  ASSERT_TRUE(OpenDocument("bug_664284.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  TestRenderPageBitmapWithFlags(page.get(), 0, original_checksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_NO_NATIVETEXT,
                                kNoNativeTextChecksum);
}

TEST_F(FPDFViewEmbedderTest, RenderAnnotationWithPrintingFlag) {
  const char* annotation_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "eaece6b8041c0cb9b33398e5b6d5ddda";
    }
    return "c108ba6e0a9743652f12e4bc223f9b32";
  }();
  static const char kPrintingChecksum[] = "3e235b9f88f652f2b97b1fc393924849";
  ASSERT_TRUE(OpenDocument("bug_1658.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // A yellow highlight is rendered with `FPDF_ANNOT` flag.
  TestRenderPageBitmapWithFlags(page.get(), FPDF_ANNOT, annotation_checksum);

  // After adding `FPDF_PRINTING` flag, the yellow highlight is not rendered.
  TestRenderPageBitmapWithFlags(page.get(), FPDF_PRINTING | FPDF_ANNOT,
                                kPrintingChecksum);
}

// TODO(crbug.com/pdfium/1955): Remove this test once pixel tests can pass with
// `reverse-byte-order` option.
TEST_F(FPDFViewEmbedderTest, RenderBlueAndRedImagesWithReverByteOrderFlag) {
  // When rendering with `FPDF_REVERSE_BYTE_ORDER` flag, the blue and red
  // channels should be reversed.
  ASSERT_TRUE(OpenDocument("bug_1396264.pdf"));
  ScopedFPDFPage page(FPDF_LoadPage(document(), 0));
  ASSERT_TRUE(page);

  TestRenderPageBitmapWithFlags(page.get(), 0,
                                "81e7f4498090977c848a21b5c6510d3a");
  TestRenderPageBitmapWithFlags(page.get(), FPDF_REVERSE_BYTE_ORDER,
                                "505ba6d1c7f4044c11c91873452a8bde");
}

TEST_F(FPDFViewEmbedderTest, RenderJpxLzwImageWithFlags) {
  static const char kNormalChecksum[] = "4bcd56cae1ca2622403e8af07242e71a";
  static const char kGrayscaleChecksum[] = "fe45ad56efe868ba82285fa5ffedc0cb";

  ASSERT_TRUE(OpenDocument("jpx_lzw.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  TestRenderPageBitmapWithFlags(page.get(), 0, kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_ANNOT, kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_LCD_TEXT, kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_NO_NATIVETEXT,
                                kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_GRAYSCALE, kGrayscaleChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_LIMITEDIMAGECACHE,
                                kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_FORCEHALFTONE,
                                kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_PRINTING, kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHTEXT,
                                kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHIMAGE,
                                kNormalChecksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHPATH,
                                kNormalChecksum);
}

TEST_F(FPDFViewEmbedderTest, RenderManyRectanglesWithFlags) {
  const char* grayscale_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "b596ac8bbe64e7bff31888ab05e4dcf4";
    }
    return "7b553f1052069a9c61237a05db0955d6";
  }();
  const char* no_smoothpath_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "4d71ed53d9f6e6a761876ebb4ff23e19";
    }
    return "ff6e5c509d1f6984bcdfd18b26a4203a";
  }();

  ASSERT_TRUE(OpenDocument("many_rectangles.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  TestRenderPageBitmapWithFlags(page.get(), 0, ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_ANNOT,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_LCD_TEXT,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_NO_NATIVETEXT,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_GRAYSCALE, grayscale_checksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_LIMITEDIMAGECACHE,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_FORCEHALFTONE,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_PRINTING,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHTEXT,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHIMAGE,
                                ManyRectanglesChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHPATH,
                                no_smoothpath_checksum);
}

TEST_F(FPDFViewEmbedderTest, RenderManyRectanglesWithAndWithoutExternalMemory) {
  ASSERT_TRUE(OpenDocument("many_rectangles.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  const char* bgr_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "4d52e5cc1d4a8067bf918b85b232fff0";
    }
    return "ab6312e04c0d3f4e46fb302a45173d05";
  }();
  static constexpr int kBgrStride = 600;  // Width of 200 * 24 bits per pixel.
  TestRenderPageBitmapWithInternalMemory(page.get(), FPDFBitmap_BGR,
                                         bgr_checksum);
  TestRenderPageBitmapWithInternalMemoryAndStride(page.get(), FPDFBitmap_BGR,
                                                  kBgrStride, bgr_checksum);
  TestRenderPageBitmapWithExternalMemory(page.get(), FPDFBitmap_BGR,
                                         bgr_checksum);
  TestRenderPageBitmapWithExternalMemoryAndNoStride(page.get(), FPDFBitmap_BGR,
                                                    bgr_checksum);

  const char* gray_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "3dfe1fc3889123d68e1748fefac65e72";
    }
    return "b561c11edc44dc3972125a9b8744fa2f";
  }();

  TestRenderPageBitmapWithInternalMemory(page.get(), FPDFBitmap_Gray,
                                         gray_checksum);
  static constexpr int kGrayStride = 200;  // Width of 200 * 8 bits per pixel.
  TestRenderPageBitmapWithInternalMemoryAndStride(page.get(), FPDFBitmap_Gray,
                                                  kGrayStride, gray_checksum);
  TestRenderPageBitmapWithExternalMemory(page.get(), FPDFBitmap_Gray,
                                         gray_checksum);
  TestRenderPageBitmapWithExternalMemoryAndNoStride(page.get(), FPDFBitmap_Gray,
                                                    gray_checksum);

  static constexpr int kBgrxStride = 800;  // Width of 200 * 32 bits per pixel.
  TestRenderPageBitmapWithInternalMemory(page.get(), FPDFBitmap_BGRx,
                                         ManyRectanglesChecksum());
  TestRenderPageBitmapWithInternalMemoryAndStride(
      page.get(), FPDFBitmap_BGRx, kBgrxStride, ManyRectanglesChecksum());
  TestRenderPageBitmapWithExternalMemory(page.get(), FPDFBitmap_BGRx,
                                         ManyRectanglesChecksum());
  TestRenderPageBitmapWithExternalMemoryAndNoStride(page.get(), FPDFBitmap_BGRx,
                                                    ManyRectanglesChecksum());

  TestRenderPageBitmapWithInternalMemory(page.get(), FPDFBitmap_BGRA,
                                         ManyRectanglesChecksum());
  TestRenderPageBitmapWithInternalMemoryAndStride(
      page.get(), FPDFBitmap_BGRA, kBgrxStride, ManyRectanglesChecksum());
  TestRenderPageBitmapWithExternalMemory(page.get(), FPDFBitmap_BGRA,
                                         ManyRectanglesChecksum());
  TestRenderPageBitmapWithExternalMemoryAndNoStride(page.get(), FPDFBitmap_BGRA,
                                                    ManyRectanglesChecksum());
}

TEST_F(FPDFViewEmbedderTest, RenderHelloWorldWithFlags) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  using pdfium::HelloWorldChecksum;
  TestRenderPageBitmapWithFlags(page.get(), 0, HelloWorldChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_ANNOT, HelloWorldChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_GRAYSCALE,
                                HelloWorldChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_LIMITEDIMAGECACHE,
                                HelloWorldChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_FORCEHALFTONE,
                                HelloWorldChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_PRINTING,
                                HelloWorldChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHIMAGE,
                                HelloWorldChecksum());
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHPATH,
                                HelloWorldChecksum());

  const char* lcd_text_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "496d1f907349b153c5ecdc87c8073c7b";
#elif BUILDFLAG(IS_APPLE)
      return "b110924c4af6e87232249ea2a564f0e4";
#else
      return "d1decde2de1c07b5274cc8cb44f92427";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "6eef7237f7591f07616e238422086737";
#else
    return "09152e25e51fa8ca31fc28d0937bf477";
#endif  // BUILDFLAG(IS_APPLE)
  }();
  const char* no_smoothtext_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "04dcf7d221437081034ca1152c717a8a";
#elif BUILDFLAG(IS_APPLE)
      return "8c99ca392ecff724da0d04b17453a45a";
#else
      return "cd5bbe9407c3fcc85d365172a9a55abd";
#endif
    }
#if BUILDFLAG(IS_APPLE)
    return "6eef7237f7591f07616e238422086737";
#else
    return "6dec98c848028fa4be3ad38d6782e304";
#endif
  }();

  TestRenderPageBitmapWithFlags(page.get(), FPDF_LCD_TEXT, lcd_text_checksum);
  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHTEXT,
                                no_smoothtext_checksum);

  // For text rendering, When anti-aliasing is disabled, LCD Optimization flag
  // will be ignored.
  TestRenderPageBitmapWithFlags(page.get(),
                                FPDF_LCD_TEXT | FPDF_RENDER_NO_SMOOTHTEXT,
                                no_smoothtext_checksum);
}

// Deliberately disabled because this test case renders a large bitmap, which is
// very slow for debug builds.
#if defined(NDEBUG)
#define MAYBE_LargeImageDoesNotRenderBlank LargeImageDoesNotRenderBlank
#else
#define MAYBE_LargeImageDoesNotRenderBlank DISABLED_LargeImageDoesNotRenderBlank
#endif
TEST_F(FPDFViewEmbedderTest, MAYBE_LargeImageDoesNotRenderBlank) {
  static const char kChecksum[] = "a6056db6961f4e65c42ab2e246171fe1";

  ASSERT_TRUE(OpenDocument("bug_1646.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  constexpr int kWidth = 40000;
  constexpr int kHeight = 100;
  TestRenderPageBitmapWithMatrix(page.get(), kWidth, kHeight,
                                 {1000, 0, 0, 1, 0, 0}, {0, 0, kWidth, kHeight},
                                 kChecksum);
}

#if BUILDFLAG(IS_WIN)
TEST_F(FPDFViewEmbedderTest, FPDFRenderPageEmf) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf_normal = RenderPageWithFlagsToEmf(page.get(), 0);
  EXPECT_EQ(3772u, emf_normal.size());

  // FPDF_REVERSE_BYTE_ORDER is ignored since EMFs are always BGR.
  std::vector<uint8_t> emf_reverse_byte_order =
      RenderPageWithFlagsToEmf(page.get(), FPDF_REVERSE_BYTE_ORDER);
  EXPECT_EQ(emf_normal, emf_reverse_byte_order);
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
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf_normal = RenderPageWithFlagsToEmf(page.get(), 0);
  std::string ps_data = GetPostScriptFromEmf(emf_normal);
  EXPECT_EQ(kExpectedRectanglePostScript, ps_data);

  // FPDF_REVERSE_BYTE_ORDER is ignored since PostScript is not bitmap-based.
  std::vector<uint8_t> emf_reverse_byte_order =
      RenderPageWithFlagsToEmf(page.get(), FPDF_REVERSE_BYTE_ORDER);
  EXPECT_EQ(emf_normal, emf_reverse_byte_order);
}

TEST_F(PostScriptLevel3EmbedderTest, Rectangles) {
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf_normal = RenderPageWithFlagsToEmf(page.get(), 0);
  std::string ps_data = GetPostScriptFromEmf(emf_normal);
  EXPECT_EQ(kExpectedRectanglePostScript, ps_data);

  // FPDF_REVERSE_BYTE_ORDER is ignored since PostScript is not bitmap-based.
  std::vector<uint8_t> emf_reverse_byte_order =
      RenderPageWithFlagsToEmf(page.get(), FPDF_REVERSE_BYTE_ORDER);
  EXPECT_EQ(emf_normal, emf_reverse_byte_order);
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
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf = RenderPageWithFlagsToEmf(page.get(), 0);
  std::string ps_data = GetPostScriptFromEmf(emf);
  EXPECT_EQ(kExpected, ps_data);
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
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  std::vector<uint8_t> emf = RenderPageWithFlagsToEmf(page.get(), 0);
  std::string ps_data = GetPostScriptFromEmf(emf);
  EXPECT_EQ(kExpected, ps_data);
}

TEST_F(FPDFViewEmbedderTest, ImageMask) {
  ASSERT_TRUE(OpenDocument("bug_674771.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Render the page with more efficient processing of image masks.
  FPDF_SetPrintMode(FPDF_PRINTMODE_EMF_IMAGE_MASKS);
  std::vector<uint8_t> emf_image_masks =
      RenderPageWithFlagsToEmf(page.get(), 0);

  // Render the page normally.
  FPDF_SetPrintMode(FPDF_PRINTMODE_EMF);
  std::vector<uint8_t> emf_normal = RenderPageWithFlagsToEmf(page.get(), 0);

  EXPECT_LT(emf_image_masks.size(), emf_normal.size());
}
#endif  // BUILDFLAG(IS_WIN)

TEST_F(FPDFViewEmbedderTest, GetTrailerEnds) {
  ASSERT_TRUE(OpenDocument("two_signatures.pdf"));

  // FPDF_GetTrailerEnds() positive testing.
  unsigned long size = FPDF_GetTrailerEnds(document(), nullptr, 0);
  const std::vector<unsigned int> kExpectedEnds{633, 1703, 2781};
  ASSERT_EQ(kExpectedEnds.size(), size);
  std::vector<unsigned int> ends(size);
  ASSERT_EQ(size, FPDF_GetTrailerEnds(document(), ends.data(), size));
  ASSERT_EQ(kExpectedEnds, ends);

  // FPDF_GetTrailerEnds() negative testing.
  ASSERT_EQ(0U, FPDF_GetTrailerEnds(nullptr, nullptr, 0));

  ends.resize(2);
  ends[0] = 0;
  ends[1] = 1;
  size = FPDF_GetTrailerEnds(document(), ends.data(), ends.size());
  ASSERT_EQ(kExpectedEnds.size(), size);
  EXPECT_EQ(0U, ends[0]);
  EXPECT_EQ(1U, ends[1]);
}

TEST_F(FPDFViewEmbedderTest, GetTrailerEndsHelloWorld) {
  // Single trailer, \n line ending at the trailer end.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  // FPDF_GetTrailerEnds() positive testing.
  unsigned long size = FPDF_GetTrailerEnds(document(), nullptr, 0);
  const std::vector<unsigned int> kExpectedEnds{840};
  ASSERT_EQ(kExpectedEnds.size(), size);
  std::vector<unsigned int> ends(size);
  ASSERT_EQ(size, FPDF_GetTrailerEnds(document(), ends.data(), size));
  ASSERT_EQ(kExpectedEnds, ends);
}

TEST_F(FPDFViewEmbedderTest, GetTrailerEndsAnnotationStamp) {
  // Multiple trailers, \r\n line ending at the trailer ends.
  ASSERT_TRUE(OpenDocument("annotation_stamp_with_ap.pdf"));

  // FPDF_GetTrailerEnds() positive testing.
  unsigned long size = FPDF_GetTrailerEnds(document(), nullptr, 0);
  const std::vector<unsigned int> kExpectedEnds{441, 7945, 101719};
  ASSERT_EQ(kExpectedEnds.size(), size);
  std::vector<unsigned int> ends(size);
  ASSERT_EQ(size, FPDF_GetTrailerEnds(document(), ends.data(), size));
  ASSERT_EQ(kExpectedEnds, ends);
}

TEST_F(FPDFViewEmbedderTest, GetTrailerEndsLinearized) {
  // Set up linearized PDF.
  FileAccessForTesting file_acc("linearized.pdf");
  FakeFileAccess fake_acc(&file_acc);
  CreateAvail(fake_acc.GetFileAvail(), fake_acc.GetFileAccess());
  fake_acc.SetWholeFileAvailable();

  // Multiple trailers, \r line ending at the trailer ends (no \n).
  SetDocumentFromAvail();
  ASSERT_TRUE(document());

  // FPDF_GetTrailerEnds() positive testing.
  unsigned long size = FPDF_GetTrailerEnds(document(), nullptr, 0);
  const std::vector<unsigned int> kExpectedEnds{474, 11384};
  ASSERT_EQ(kExpectedEnds.size(), size);
  std::vector<unsigned int> ends(size);
  ASSERT_EQ(size, FPDF_GetTrailerEnds(document(), ends.data(), size));
  ASSERT_EQ(kExpectedEnds, ends);
}

TEST_F(FPDFViewEmbedderTest, GetTrailerEndsWhitespace) {
  // Whitespace between 'endstream'/'endobj' and the newline.
  ASSERT_TRUE(OpenDocument("trailer_end_trailing_space.pdf"));

  unsigned long size = FPDF_GetTrailerEnds(document(), nullptr, 0);
  const std::vector<unsigned int> kExpectedEnds{1193};
  // Without the accompanying fix in place, this test would have failed, as the
  // size was 0, not 1, i.e. no trailer ends were found.
  ASSERT_EQ(kExpectedEnds.size(), size);
  std::vector<unsigned int> ends(size);
  ASSERT_EQ(size, FPDF_GetTrailerEnds(document(), ends.data(), size));
  EXPECT_EQ(kExpectedEnds, ends);
}

TEST_F(FPDFViewEmbedderTest, RenderXfaPage) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Should always be blank, as we're not testing `FPDF_FFLDraw()` here.
  TestRenderPageBitmapWithFlags(page.get(), 0,
                                pdfium::kBlankPage612By792Checksum);
}

#if defined(PDF_USE_SKIA)
TEST_F(FPDFViewEmbedderTest, RenderPageToSkp) {
  if (!CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    GTEST_SKIP() << "FPDF_RenderPageSkp() only makes sense with Skia";
  }

  ASSERT_TRUE(OpenDocument("rectangles.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  TestRenderPageSkp(page.get(), pdfium::RectanglesChecksum());
}

TEST_F(FPDFViewEmbedderTest, RenderXfaPageToSkp) {
  if (!CFX_DefaultRenderDevice::UseSkiaRenderer()) {
    GTEST_SKIP() << "FPDF_RenderPageSkp() only makes sense with Skia";
  }

  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  // Should always be blank, as we're not testing `FPDF_FFLRecord()` here.
  TestRenderPageSkp(page.get(), pdfium::kBlankPage612By792Checksum);
}

TEST_F(FPDFViewEmbedderTest, Bug2087) {
  FPDF_DestroyLibrary();

  std::string agg_checksum;
  const FPDF_LIBRARY_CONFIG kAggConfig = {
      .version = 4,
      .m_pUserFontPaths = nullptr,
      .m_pIsolate = nullptr,
      .m_v8EmbedderSlot = 0,
      .m_pPlatform = nullptr,
      .m_RendererType = FPDF_RENDERERTYPE_AGG,
  };
  FPDF_InitLibraryWithConfig(&kAggConfig);
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  {
    ScopedEmbedderTestPage page = LoadScopedPage(0);
    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    agg_checksum = HashBitmap(bitmap.get());
  }
  CloseDocument();
  FPDF_DestroyLibrary();

  std::string skia_checksum;
  const FPDF_LIBRARY_CONFIG kSkiaConfig = {
      .version = 2,
      .m_pUserFontPaths = nullptr,
      .m_pIsolate = nullptr,
      .m_v8EmbedderSlot = 0,
  };
  FPDF_InitLibraryWithConfig(&kSkiaConfig);
  ASSERT_TRUE(OpenDocument("rectangles.pdf"));
  {
    ScopedEmbedderTestPage page = LoadScopedPage(0);
    ScopedFPDFBitmap bitmap = RenderPage(page.get());
    skia_checksum = HashBitmap(bitmap.get());
  }
  CloseDocument();

  EXPECT_NE(agg_checksum, skia_checksum);

  EmbedderTestEnvironment::GetInstance()->TearDown();
  EmbedderTestEnvironment::GetInstance()->SetUp();
}
#endif  // defined(PDF_USE_SKIA)

TEST_F(FPDFViewEmbedderTest, NoSmoothTextItalicOverlappingGlyphs) {
  ASSERT_TRUE(OpenDocument("bug_1919.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  const char* checksum = []() {
#if BUILDFLAG(IS_WIN)
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "d97d0a9da6a5955f68a58a3f25466bd7";
    }
#elif !BUILDFLAG(IS_APPLE)
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "ceeb93d2bcdb586d62c95b33cadcd873";
    }
#endif
    return "5f99e2fa2bad09393d6428e105a83c96";
  }();

  TestRenderPageBitmapWithFlags(page.get(), FPDF_RENDER_NO_SMOOTHTEXT,
                                checksum);
}

TEST_F(FPDFViewEmbedderTest, RenderTransparencyOnWhiteBackground) {
  ASSERT_TRUE(OpenDocument("bug_1302355.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  constexpr int kWidth = 200;
  constexpr int kHeight = 200;
  EXPECT_EQ(kWidth, static_cast<int>(FPDF_GetPageWidthF(page.get())));
  EXPECT_EQ(kHeight, static_cast<int>(FPDF_GetPageHeightF(page.get())));
  EXPECT_TRUE(FPDFPage_HasTransparency(page.get()));
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(kWidth, kHeight, /*alpha=*/true));
  ASSERT_TRUE(
      FPDFBitmap_FillRect(bitmap.get(), 0, 0, kWidth, kHeight, 0xFFFFFFFF));
  FPDF_RenderPageBitmap(bitmap.get(), page.get(), /*start_x=*/0,
                        /*start_y=*/0, kWidth, kHeight, /*rotate=*/0,
                        /*flags=*/0);
  // TODO(crbug.com/1302355): This page should not render blank.
  EXPECT_EQ("eee4600ac08b458ac7ac2320e225674c", HashBitmap(bitmap.get()));
}

TEST_F(FPDFViewEmbedderTest, Bug2112) {
  constexpr int kWidth = 595;
  constexpr int kHeight = 842;
  constexpr int kStride = kWidth * 3;
  std::vector<uint8_t> vec(kStride * kHeight);
  ScopedFPDFBitmap bitmap(FPDFBitmap_CreateEx(kWidth, kHeight, FPDFBitmap_BGR,
                                              vec.data(), kStride));
  EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(bitmap.get()));
}

TEST_F(FPDFViewEmbedderTest, RenderAnnotsGrayScale) {
  ASSERT_TRUE(OpenDocument("annotation_highlight_square_with_ap.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  const char* const gray_checksum = []() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
#if BUILDFLAG(IS_WIN)
      return "c18c1b7ee995f16dfb18e6da73a3c2d3";
#elif BUILDFLAG(IS_APPLE)
      return "92e96cad5e6b93fee3e2017ea27e2497";
#else
      return "b73df08d5252615ad6ed2fe7d6c73883";
#endif
    }
    return "c02f449666bf2633d06b909c76bc1c1d";
  }();

  TestRenderPageBitmapWithInternalMemory(page.get(), FPDFBitmap_Gray,
                                         gray_checksum);
}

TEST_F(FPDFViewEmbedderTest, BadFillRectInput) {
  constexpr int kWidth = 200;
  constexpr int kHeight = 200;
  constexpr char kExpectedChecksum[] = "acc736435c9f84aa82941ba561bc5dbc";
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(200, 200, /*alpha=*/true));
  ASSERT_TRUE(FPDFBitmap_FillRect(bitmap.get(), /*left=*/0, /*top=*/0,
                                  /*width=*/kWidth,
                                  /*height=*/kHeight, 0xFFFF0000));
  EXPECT_EQ(kExpectedChecksum, HashBitmap(bitmap.get()));

  // Empty rect dimensions is a no-op.
  ASSERT_TRUE(FPDFBitmap_FillRect(bitmap.get(), /*left=*/0, /*top=*/0,
                                  /*width=*/0,
                                  /*height=*/0, 0xFF0000FF));
  EXPECT_EQ(kExpectedChecksum, HashBitmap(bitmap.get()));

  // Rect dimension overflows are also no-ops.
  ASSERT_FALSE(FPDFBitmap_FillRect(
      bitmap.get(), /*left=*/std::numeric_limits<int>::max(),
      /*top=*/0, /*width=*/std::numeric_limits<int>::max(),
      /*height=*/kHeight, 0xFF0000FF));
  EXPECT_EQ(kExpectedChecksum, HashBitmap(bitmap.get()));

  ASSERT_FALSE(FPDFBitmap_FillRect(
      bitmap.get(), /*left=*/0,
      /*top=*/std::numeric_limits<int>::max(), /*width=*/kWidth,
      /*height=*/std::numeric_limits<int>::max(), 0xFF0000FF));
  EXPECT_EQ(kExpectedChecksum, HashBitmap(bitmap.get()));

  // Make sure null bitmap handle does not trigger a crash.
  ASSERT_FALSE(FPDFBitmap_FillRect(nullptr, 0, 0, kWidth, kHeight, 0xFF0000FF));
}
