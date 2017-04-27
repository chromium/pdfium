// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fwl/cfx_barcode.h"

#include <memory>
#include <utility>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxge/cfx_renderdevice.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/mock_ifx_renderdevicedriver.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

using testing::_;
using testing::AtLeast;

class BarcodeTest : public testing::Test {
 public:
  void SetUp() override {
    BC_Library_Init();
    barcode_ = pdfium::MakeUnique<CFX_Barcode>();
    device_ = pdfium::MakeUnique<CFX_RenderDevice>();
    driver_ = pdfium::MakeUnique<MockIFXRenderDeviceDriver>();
  }

  void TearDown() override {
    driver_.reset();
    device_.reset();
    barcode_.reset();
    BC_Library_Destroy();
  }

  CFX_Barcode* barcode() const { return barcode_.get(); }
  CFX_RenderDevice* device() const { return device_.get(); }
  MockIFXRenderDeviceDriver* driver() const { return driver_.get(); }

  bool Create(BC_TYPE type) {
    if (!barcode_->Create(type))
      return false;

    barcode_->SetModuleHeight(300);
    barcode_->SetModuleWidth(420);
    barcode_->SetHeight(298);
    barcode_->SetWidth(418);
    return true;
  }

  void HandoffDriverToDevice() { device_->SetDeviceDriver(std::move(driver_)); }
  bool RenderDevice() {
    return barcode_->RenderDevice(device_.get(), &matrix_);
  }

 protected:
  CFX_Matrix matrix_;
  std::unique_ptr<CFX_Barcode> barcode_;
  std::unique_ptr<CFX_RenderDevice> device_;
  std::unique_ptr<MockIFXRenderDeviceDriver> driver_;
};

TEST_F(BarcodeTest, Code39) {
  EXPECT_TRUE(Create(BC_CODE39));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));

  EXPECT_CALL(*driver(), GetDeviceCaps(_)).Times(AtLeast(1));
  EXPECT_CALL(*driver(), GetClipBox(_)).Times(AtLeast(1));
  EXPECT_CALL(*driver(), DrawPath(_, _, _, _, _, _, _)).Times(AtLeast(1));
  HandoffDriverToDevice();
  RenderDevice();
}

TEST_F(BarcodeTest, CodaBar) {
  EXPECT_TRUE(Create(BC_CODABAR));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, Code128) {
  EXPECT_TRUE(Create(BC_CODE128));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, Code128_B) {
  EXPECT_TRUE(Create(BC_CODE128_B));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, Code128_C) {
  EXPECT_TRUE(Create(BC_CODE128_C));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, Ean8) {
  EXPECT_TRUE(Create(BC_EAN8));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, UPCA) {
  EXPECT_TRUE(Create(BC_UPCA));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, Ean13) {
  EXPECT_TRUE(Create(BC_EAN13));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, Pdf417) {
  EXPECT_TRUE(Create(BC_PDF417));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, DataMatrix) {
  EXPECT_TRUE(Create(BC_DATAMATRIX));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}

TEST_F(BarcodeTest, QrCode) {
  EXPECT_TRUE(Create(BC_QR_CODE));
  EXPECT_TRUE(barcode()->Encode(L"clams", false));
}
