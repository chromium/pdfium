// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/cfx_barcode.h"

#include <memory>
#include <string>
#include <utility>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/bitmap_saver.h"
#include "testing/utils/hash.h"
#include "third_party/base/ptr_util.h"

class BarcodeTest : public testing::Test {
 public:
  void SetUp() override {
    BC_Library_Init();

    auto device = pdfium::MakeUnique<CFX_DefaultRenderDevice>();
    auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    if (bitmap->Create(640, 480, FXDIB_Rgb32))
      bitmap_ = bitmap;
    ASSERT_TRUE(bitmap_);
    ASSERT_TRUE(device->Attach(bitmap_, false, nullptr, false));
    device_ = std::move(device);
  }

  void TearDown() override {
    bitmap_.Reset();
    device_.reset();
    barcode_.reset();
    BC_Library_Destroy();
  }

  CFX_Barcode* barcode() const { return barcode_.get(); }

  void Create(BC_TYPE type) {
    barcode_ = CFX_Barcode::Create(type);
    barcode_->SetHeight(298);
    barcode_->SetWidth(418);
  }

  bool RenderDevice() {
    return barcode_->RenderDevice(device_.get(), &matrix_);
  }

  std::string BitmapChecksum() {
    return GenerateMD5Base16(bitmap_->GetBuffer(),
                             bitmap_->GetPitch() * bitmap_->GetHeight());
  }

  // Manually insert calls to this as needed for debugging.
  void SaveBitmap(const std::string& filename) {
    BitmapSaver::WriteBitmapToPng(bitmap_.Get(), filename);
  }

 protected:
  CFX_Matrix matrix_;
  std::unique_ptr<CFX_Barcode> barcode_;
  std::unique_ptr<CFX_RenderDevice> device_;
  RetainPtr<CFX_DIBitmap> bitmap_;
};

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Code39 DISABLED_Code39
#else
#define MAYBE_Code39 Code39
#endif
TEST_F(BarcodeTest, MAYBE_Code39) {
  Create(BC_CODE39);
  EXPECT_TRUE(barcode()->Encode(L"CLAMS"));
  RenderDevice();
  EXPECT_EQ("cd4cd3f36da38ff58d9f621827018903", BitmapChecksum());
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_CodaBar DISABLED_CodaBar
#else
#define MAYBE_CodaBar CodaBar
#endif
TEST_F(BarcodeTest, MAYBE_CodaBar) {
  Create(BC_CODABAR);
  EXPECT_TRUE(barcode()->Encode(L"$123-456"));
  RenderDevice();
  EXPECT_EQ("5fad4fc19f099001a0fe83c89430c977", BitmapChecksum());
}

TEST_F(BarcodeTest, DISABLED_CodaBarLetters) {
  Create(BC_CODABAR);
  EXPECT_FALSE(barcode()->Encode(L"clams"));
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Code128 DISABLED_Code128
#else
#define MAYBE_Code128 Code128
#endif
TEST_F(BarcodeTest, MAYBE_Code128) {
  Create(BC_CODE128);
  EXPECT_TRUE(barcode()->Encode(L"Clams"));
  RenderDevice();
  EXPECT_EQ("6351f0f6e997050e4658bbb4777aef74", BitmapChecksum());
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Code128B DISABLED_Code128B
#else
#define MAYBE_Code128B Code128B
#endif
TEST_F(BarcodeTest, MAYBE_Code128B) {
  Create(BC_CODE128_B);
  EXPECT_TRUE(barcode()->Encode(L"Clams"));
  RenderDevice();
  EXPECT_EQ("6351f0f6e997050e4658bbb4777aef74", BitmapChecksum());
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Code128C DISABLED_Code128C
#else
#define MAYBE_Code128C Code128C
#endif
TEST_F(BarcodeTest, MAYBE_Code128C) {
  Create(BC_CODE128_C);
  EXPECT_TRUE(barcode()->Encode(L"123456"));
  RenderDevice();
  EXPECT_EQ("fba730a807ba6363f9bd2bc7f8c56d1f", BitmapChecksum());
}

TEST_F(BarcodeTest, DISABLED_Code128CLetters) {
  Create(BC_CODE128_C);
  EXPECT_FALSE(barcode()->Encode(L"clams"));
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Ean8 DISABLED_Ean8
#else
#define MAYBE_Ean8 Ean8
#endif
TEST_F(BarcodeTest, MAYBE_Ean8) {
  Create(BC_EAN8);
  EXPECT_TRUE(barcode()->Encode(L"123456"));
  RenderDevice();
  EXPECT_EQ("aff88491ac46ca6217d780d185300cde", BitmapChecksum());
}

TEST_F(BarcodeTest, DISABLED_Ean8Letters) {
  Create(BC_EAN8);
  EXPECT_FALSE(barcode()->Encode(L"clams"));
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_UPCA DISABLED_UPCA
#else
#define MAYBE_UPCA UPCA
#endif
TEST_F(BarcodeTest, MAYBE_UPCA) {
  Create(BC_UPCA);
  EXPECT_TRUE(barcode()->Encode(L"123456"));
  RenderDevice();
  EXPECT_EQ("fe26a5714cff7ffe3f9b02183efc435b", BitmapChecksum());
}

TEST_F(BarcodeTest, DISABLED_UPCALetters) {
  Create(BC_UPCA);
  EXPECT_FALSE(barcode()->Encode(L"clams"));
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Ean13 DISABLED_Ean13
#else
#define MAYBE_Ean13 Ean13
#endif
TEST_F(BarcodeTest, MAYBE_Ean13) {
  Create(BC_EAN13);
  EXPECT_TRUE(barcode()->Encode(L"123456"));
  RenderDevice();
  EXPECT_EQ("72d2190b98d635c32834bf67552e561e", BitmapChecksum());
}

TEST_F(BarcodeTest, DISABLED_Ean13Letters) {
  Create(BC_EAN13);
  EXPECT_FALSE(barcode()->Encode(L"clams"));
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_Pdf417 DISABLED_Pdf417
#else
#define MAYBE_Pdf417 Pdf417
#endif
TEST_F(BarcodeTest, MAYBE_Pdf417) {
  Create(BC_PDF417);
  EXPECT_TRUE(barcode()->Encode(L"clams"));
  RenderDevice();
  EXPECT_EQ("191e35d11613901b7d5d51033689aa89", BitmapChecksum());
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_DataMatrix DISABLED_DataMatrix
#else
#define MAYBE_DataMatrix DataMatrix
#endif
TEST_F(BarcodeTest, MAYBE_DataMatrix) {
  Create(BC_DATAMATRIX);
  EXPECT_TRUE(barcode()->Encode(L"clams"));
  RenderDevice();
  EXPECT_EQ("5e5cd9a680b86fcd4ffd53ed36e3c980", BitmapChecksum());
}

// https://crbug.com/pdfium/738
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_QrCode DISABLED_QrCode
#else
#define MAYBE_QrCode QrCode
#endif
TEST_F(BarcodeTest, MAYBE_QrCode) {
  Create(BC_QR_CODE);
  EXPECT_TRUE(barcode()->Encode(L"clams"));
  RenderDevice();
  EXPECT_EQ("4751c6e0f67749fabe24f787128decee", BitmapChecksum());
}
