// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/cfde_textout.h"

#include <memory>

#include "core/fdrm/fx_crypt.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/hash.h"
#include "xfa/fgas/font/cfgas_gefont.h"

namespace {

const char kEmptyImageChecksum[] = "a042237c5493fdb9656b94a83608d11a";

}  // namespace

class CFDETextOutTest : public testing::Test {
 public:
  void SetUp() override {
    bitmap_ = pdfium::MakeRetain<CFX_DIBitmap>();
    ASSERT_TRUE(bitmap_->Create(200, 100, FXDIB_Format::kArgb));

    device_ = std::make_unique<CFX_DefaultRenderDevice>();
    device_->Attach(bitmap_);

    const wchar_t kFontFamily[] = L"Arimo Bold";
    font_ = CFGAS_GEFont::LoadFont(kFontFamily, 0, FX_CodePage::kDefANSI);
    ASSERT_TRUE(font_);

    text_out_ = std::make_unique<CFDE_TextOut>();
    text_out_->SetFont(font_);
    text_out_->SetFontSize(12.0f);

    EXPECT_STREQ(kEmptyImageChecksum, GetBitmapChecksum().c_str());
  }

  void TearDown() override {
    text_out_.reset();
    font_.Reset();
    device_.reset();
    bitmap_.Reset();
  }

  CFX_DefaultRenderDevice* device() { return device_.get(); }
  CFDE_TextOut& text_out() { return *text_out_; }

  ByteString GetBitmapChecksum() {
    CRYPT_md5_context context = CRYPT_MD5Start();
    for (int i = 0; i < bitmap_->GetHeight(); ++i)
      CRYPT_MD5Update(&context, bitmap_->GetScanline(i));
    uint8_t digest[16];
    CRYPT_MD5Finish(&context, digest);
    return ByteString(CryptToBase16(digest).c_str());
  }

 private:
  RetainPtr<CFX_DIBitmap> bitmap_;
  std::unique_ptr<CFX_DefaultRenderDevice> device_;
  RetainPtr<CFGAS_GEFont> font_;
  std::unique_ptr<CFDE_TextOut> text_out_;
};

TEST_F(CFDETextOutTest, DrawLogicTextBasic) {
  text_out().DrawLogicText(device(), L"foo", CFX_RectF(0, 0, 200, 100));
  EXPECT_STREQ("b26f1c171fcdbf185823364185adacf0", GetBitmapChecksum().c_str());
}

TEST_F(CFDETextOutTest, DrawLogicTextEmptyRect) {
  text_out().DrawLogicText(device(), L"foo", CFX_RectF());
  EXPECT_STREQ(kEmptyImageChecksum, GetBitmapChecksum().c_str());
}
