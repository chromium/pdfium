// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fwl/cfx_barcode.h"

#include <memory>
#include <string>
#include <utility>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_renderdevice.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

class BarcodeTest : public testing::Test {
 public:
  void SetUp() override {
    BC_Library_Init();
    barcode_ = pdfium::MakeUnique<CFX_Barcode>();

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

  bool Create(BC_TYPE type) {
    if (!barcode_->Create(type))
      return false;

    barcode_->SetModuleHeight(300);
    barcode_->SetModuleWidth(420);
    barcode_->SetHeight(298);
    barcode_->SetWidth(418);
    return true;
  }

  bool RenderDevice() {
    return barcode_->RenderDevice(device_.get(), &matrix_);
  }

  std::string BitmapChecksum() {
    return GenerateMD5Base16(bitmap_->GetBuffer(),
                             bitmap_->GetPitch() * bitmap_->GetHeight());
  }

 protected:
  CFX_Matrix matrix_;
  std::unique_ptr<CFX_Barcode> barcode_;
  std::unique_ptr<CFX_RenderDevice> device_;
  CFX_RetainPtr<CFX_DIBitmap> bitmap_;
};

TEST_F(BarcodeTest, Code39) {
  EXPECT_TRUE(Create(BC_CODE39));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("e0d784db2d4fb5dab7836722a1ad7001", BitmapChecksum());
}

TEST_F(BarcodeTest, CodaBar) {
  EXPECT_TRUE(Create(BC_CODABAR));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("26b50593e698a0a9714fd2a60131ef70", BitmapChecksum());
}

TEST_F(BarcodeTest, Code128) {
  EXPECT_TRUE(Create(BC_CODE128));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("0beb98f447c632e3085a6b1eae49379f", BitmapChecksum());
}

TEST_F(BarcodeTest, Code128_B) {
  EXPECT_TRUE(Create(BC_CODE128_B));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("0beb98f447c632e3085a6b1eae49379f", BitmapChecksum());
}

TEST_F(BarcodeTest, Code128_C) {
  EXPECT_TRUE(Create(BC_CODE128_C));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("5d75bfdd601494fc4f6451cce7452922", BitmapChecksum());
}

TEST_F(BarcodeTest, Ean8) {
  EXPECT_TRUE(Create(BC_EAN8));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("0a9a7bc34e6d0c82c2950fa592a0039e", BitmapChecksum());
}

TEST_F(BarcodeTest, UPCA) {
  EXPECT_TRUE(Create(BC_UPCA));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("d3f993c0fc0131ce11b863a73e27f8e6", BitmapChecksum());
}

TEST_F(BarcodeTest, Ean13) {
  EXPECT_TRUE(Create(BC_EAN13));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("461acb9e4f1a284a8e699f9f121a09d3", BitmapChecksum());
}

TEST_F(BarcodeTest, Pdf417) {
  EXPECT_TRUE(Create(BC_PDF417));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("2bdb9b39f20c5763da6a0d7c7b1f6933", BitmapChecksum());
}

TEST_F(BarcodeTest, DataMatrix) {
  EXPECT_TRUE(Create(BC_DATAMATRIX));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("5e5cd9a680b86fcd4ffd53ed36e3c980", BitmapChecksum());
}

TEST_F(BarcodeTest, QrCode) {
  EXPECT_TRUE(Create(BC_QR_CODE));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
  RenderDevice();
  EXPECT_EQ("4751c6e0f67749fabe24f787128decee", BitmapChecksum());
}
