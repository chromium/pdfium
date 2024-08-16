// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/cfde_textout.h"

#include <memory>

#include "build/build_config.h"
#include "core/fdrm/fx_crypt.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_glyphcache.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/utils/hash.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/font/cfgas_gemodule.h"

namespace pdfium {

class CFDETextOutTest : public testing::Test {
 public:
  CFDETextOutTest() = default;
  ~CFDETextOutTest() override = default;

  void SetUp() override {
#if defined(PDF_USE_SKIA)
    CFX_GlyphCache::InitializeGlobals();
#endif
    CFX_Size bitmap_size = GetBitmapSize();
    bitmap_ = MakeRetain<CFX_DIBitmap>();
    ASSERT_TRUE(bitmap_->Create(bitmap_size.width, bitmap_size.height,
                                FXDIB_Format::kBgra));

    device_ = std::make_unique<CFX_DefaultRenderDevice>();
    device_->Attach(bitmap_);

    font_ = LoadFont();
    ASSERT_TRUE(font_);

    text_out_ = std::make_unique<CFDE_TextOut>();
    text_out_->SetFont(font_);
    text_out_->SetFontSize(12.0f);

    EXPECT_EQ(GetEmptyBitmapChecksum(), GetBitmapChecksum());
  }

  void TearDown() override {
    // reverse order form SetUp()
    text_out_.reset();
    font_.Reset();
    device_.reset();
    bitmap_.Reset();
#if defined(PDF_USE_SKIA)
    CFX_GlyphCache::DestroyGlobals();
#endif
  }

  virtual RetainPtr<CFGAS_GEFont> LoadFont() {
    const wchar_t kFontFamily[] = L"Arimo Bold";
    return CFGAS_GEFont::LoadFont(kFontFamily, /*dwFontStyles=*/0,
                                  FX_CodePage::kDefANSI);
  }

  virtual CFX_Size GetBitmapSize() { return CFX_Size(200, 100); }

  virtual const char* GetEmptyBitmapChecksum() {
    static const char kEmptyBitmapChecksum[] =
        "a042237c5493fdb9656b94a83608d11a";
    return kEmptyBitmapChecksum;
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
  text_out().DrawLogicText(device(), L"foo", CFX_RectF(0, 0, 2100, 100));
  const char* checksum = []() {
#if BUILDFLAG(IS_WIN)
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "bc1f736237b08d13db06c09f6becc9f7";
    }
#endif
    return "b26f1c171fcdbf185823364185adacf0";
  }();
  EXPECT_EQ(checksum, GetBitmapChecksum());
}

TEST_F(CFDETextOutTest, DrawLogicTextEmptyRect) {
  text_out().DrawLogicText(device(), L"foo", CFX_RectF());
  EXPECT_EQ(GetEmptyBitmapChecksum(), GetBitmapChecksum());
}

#if !BUILDFLAG(IS_WIN)
// This test depends on a particular font being present.
class CFDETextOutLargeBitmapTest : public CFDETextOutTest {
 public:
  CFDETextOutLargeBitmapTest() = default;
  ~CFDETextOutLargeBitmapTest() override = default;

  RetainPtr<CFGAS_GEFont> LoadFont() override {
    const wchar_t kFontFamily[] = L"DejaVu Sans";
    auto* font_manager = CFGAS_GEModule::Get()->GetFontMgr();
    return font_manager->LoadFont(kFontFamily, /*dwFontStyles=*/0,
                                  FX_CodePage::kFailure);
  }

  CFX_Size GetBitmapSize() override { return CFX_Size(2100, 20); }

  const char* GetEmptyBitmapChecksum() override {
    static const char kEmptyLargeBitmapChecksum[] =
        "101745f76351fd5d916bf3817b71563c";
    return kEmptyLargeBitmapChecksum;
  }

  const char* GetLargeTextBlobChecksum() {
    if (CFX_DefaultRenderDevice::UseSkiaRenderer()) {
      return "6181929583fd7651169306852397806f";
    }
    return "268b71a8660b51e31c6bf30fc7ff1e08";
  }
};

TEST_F(CFDETextOutLargeBitmapTest, DrawLogicTextBug953881) {
  FDE_TextStyle styles;
  styles.single_line_ = true;
  text_out().SetStyles(styles);
  text_out().SetAlignment(FDE_TextAlignment::kCenterLeft);
  text_out().SetFontSize(10.0f);

  static const wchar_t kText[] =
      L"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS"
      L"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSssssssssss"
      L"sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      L"sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      L"sssssssssssssssssssssssssssssssssssssssssssssssssnnnnnnnnnnn"
      "\xfeba"
      L"Sssssssssssssssssss"
      "\xfeba"
      L"iiiiisssss";
  text_out().DrawLogicText(device(), WideString(kText),
                           CFX_RectF(3, 3, 2048, 10));
  EXPECT_EQ(GetLargeTextBlobChecksum(), GetBitmapChecksum());
}

TEST_F(CFDETextOutLargeBitmapTest, DrawLogicTextBug1342078) {
  FDE_TextStyle styles;
  styles.single_line_ = true;
  text_out().SetStyles(styles);
  text_out().SetAlignment(FDE_TextAlignment::kCenterLeft);
  text_out().SetFontSize(10.0f);

  static const wchar_t kText[] =
      L"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS"
      L"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSssssssssss"
      L"sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      L"sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
      L"sssssssssssssssssssssssssssssssssssssssssssssssssnnnnnnnnnnn"
      "\xfeba"
      L"Sssssssssssssssssss"
      "\xfeba"
      L"iiiiiiiiiisssss";
  text_out().DrawLogicText(device(), WideString(kText),
                           CFX_RectF(3, 3, 2048, 10));
  EXPECT_EQ(GetLargeTextBlobChecksum(), GetBitmapChecksum());
}
#endif  // !BUILDFLAG(IS_WIN)

}  // namespace pdfium
