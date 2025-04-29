// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_sysfontinfo.h"

#include <stddef.h>

#include <memory>
#include <utility>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/systemfontinfo_iface.h"

#ifdef PDF_ENABLE_XFA
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gemodule.h"
#endif

static_assert(FXFONT_ANSI_CHARSET == static_cast<int>(FX_Charset::kANSI));
static_assert(FXFONT_DEFAULT_CHARSET == static_cast<int>(FX_Charset::kDefault));
static_assert(FXFONT_SYMBOL_CHARSET == static_cast<int>(FX_Charset::kSymbol));
static_assert(FXFONT_SHIFTJIS_CHARSET ==
              static_cast<int>(FX_Charset::kShiftJIS));
static_assert(FXFONT_HANGEUL_CHARSET == static_cast<int>(FX_Charset::kHangul));
static_assert(FXFONT_GB2312_CHARSET ==
              static_cast<int>(FX_Charset::kChineseSimplified));
static_assert(FXFONT_CHINESEBIG5_CHARSET ==
              static_cast<int>(FX_Charset::kChineseTraditional));
static_assert(FXFONT_GREEK_CHARSET ==
              static_cast<int>(FX_Charset::kMSWin_Greek));
static_assert(FXFONT_VIETNAMESE_CHARSET ==
              static_cast<int>(FX_Charset::kMSWin_Vietnamese));
static_assert(FXFONT_HEBREW_CHARSET ==
              static_cast<int>(FX_Charset::kMSWin_Hebrew));
static_assert(FXFONT_ARABIC_CHARSET ==
              static_cast<int>(FX_Charset::kMSWin_Arabic));
static_assert(FXFONT_CYRILLIC_CHARSET ==
              static_cast<int>(FX_Charset::kMSWin_Cyrillic));
static_assert(FXFONT_THAI_CHARSET == static_cast<int>(FX_Charset::kThai));
static_assert(FXFONT_EASTERNEUROPEAN_CHARSET ==
              static_cast<int>(FX_Charset::kMSWin_EasternEuropean));
static_assert(pdfium::kFontPitchFamilyFixed == FXFONT_FF_FIXEDPITCH);
static_assert(pdfium::kFontPitchFamilyRoman == FXFONT_FF_ROMAN);
static_assert(pdfium::kFontPitchFamilyScript == FXFONT_FF_SCRIPT);
static_assert(offsetof(CFX_Font::CharsetFontMap, charset) ==
              offsetof(FPDF_CharsetFontMap, charset));
static_assert(offsetof(CFX_Font::CharsetFontMap, fontname) ==
              offsetof(FPDF_CharsetFontMap, fontname));
static_assert(sizeof(CFX_Font::CharsetFontMap) == sizeof(FPDF_CharsetFontMap));

static_assert(FXFONT_FW_NORMAL == pdfium::kFontWeightNormal);
static_assert(FXFONT_FW_BOLD == pdfium::kFontWeightBold);

class CFX_ExternalFontInfo final : public SystemFontInfoIface {
 public:
  explicit CFX_ExternalFontInfo(FPDF_SYSFONTINFO* pInfo) : info_(pInfo) {}
  ~CFX_ExternalFontInfo() override {
    if (info_->Release) {
      info_->Release(info_);
    }
  }

  void EnumFontList(CFX_FontMapper* pMapper) override {
    if (info_->EnumFonts) {
      info_->EnumFonts(info_, pMapper);
    }
  }

  void* MapFont(int weight,
                bool bItalic,
                FX_Charset charset,
                int pitch_family,
                const ByteString& face) override {
    if (!info_->MapFont) {
      return nullptr;
    }

    int iExact;
    return info_->MapFont(info_, weight, bItalic, static_cast<int>(charset),
                          pitch_family, face.c_str(), &iExact);
  }

  void* GetFont(const ByteString& family) override {
    if (!info_->GetFont) {
      return nullptr;
    }
    return info_->GetFont(info_, family.c_str());
  }

  size_t GetFontData(void* hFont,
                     uint32_t table,
                     pdfium::span<uint8_t> buffer) override {
    if (!info_->GetFontData) {
      return 0;
    }
    return info_->GetFontData(info_, hFont, table, buffer.data(),
                              fxcrt::CollectionSize<unsigned long>(buffer));
  }

  bool GetFaceName(void* hFont, ByteString* name) override {
    if (!info_->GetFaceName) {
      return false;
    }
    unsigned long size = info_->GetFaceName(info_, hFont, nullptr, 0);
    if (size == 0) {
      return false;
    }
    ByteString result;
    auto result_span = result.GetBuffer(size);
    size = info_->GetFaceName(info_, hFont, result_span.data(), size);
    result.ReleaseBuffer(size);
    *name = std::move(result);
    return true;
  }

  bool GetFontCharset(void* hFont, FX_Charset* charset) override {
    if (!info_->GetFontCharset) {
      return false;
    }

    *charset = FX_GetCharsetFromInt(info_->GetFontCharset(info_, hFont));
    return true;
  }

  void DeleteFont(void* hFont) override {
    if (info_->DeleteFont) {
      info_->DeleteFont(info_, hFont);
    }
  }

 private:
  UnownedPtr<FPDF_SYSFONTINFO> const info_;
};

FPDF_EXPORT void FPDF_CALLCONV FPDF_AddInstalledFont(void* mapper,
                                                     const char* face,
                                                     int charset) {
  CFX_FontMapper* pMapper = static_cast<CFX_FontMapper*>(mapper);
  pMapper->AddInstalledFont(face, FX_GetCharsetFromInt(charset));
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_SetSystemFontInfo(FPDF_SYSFONTINFO* pFontInfoExt) {
  auto* mapper = CFX_GEModule::Get()->GetFontMgr()->GetBuiltinMapper();
  if (!pFontInfoExt) {
    std::unique_ptr<SystemFontInfoIface> info = mapper->TakeSystemFontInfo();
    // Delete `info` when it goes out of scope here.
    return;
  }

  if (pFontInfoExt->version != 1) {
    return;
  }

  mapper->SetSystemFontInfo(
      std::make_unique<CFX_ExternalFontInfo>(pFontInfoExt));

#ifdef PDF_ENABLE_XFA
  CFGAS_GEModule::Get()->GetFontMgr()->EnumFonts();
#endif
}

FPDF_EXPORT const FPDF_CharsetFontMap* FPDF_CALLCONV FPDF_GetDefaultTTFMap() {
  return reinterpret_cast<const FPDF_CharsetFontMap*>(
      CFX_Font::GetDefaultTTFMapSpan().data());
}

FPDF_EXPORT size_t FPDF_CALLCONV FPDF_GetDefaultTTFMapCount() {
  return CFX_Font::GetDefaultTTFMapSpan().size();
}

FPDF_EXPORT const FPDF_CharsetFontMap* FPDF_CALLCONV
FPDF_GetDefaultTTFMapEntry(size_t index) {
  pdfium::span<const CFX_Font::CharsetFontMap> entries =
      CFX_Font::GetDefaultTTFMapSpan();
  return index < entries.size()
             ? reinterpret_cast<const FPDF_CharsetFontMap*>(&entries[index])
             : nullptr;
}

struct FPDF_SYSFONTINFO_DEFAULT final : public FPDF_SYSFONTINFO {
  UnownedPtr<SystemFontInfoIface> font_info_;
};

static void DefaultRelease(struct _FPDF_SYSFONTINFO* pThis) {
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  pDefault->font_info_.ClearAndDelete();
}

static void DefaultEnumFonts(struct _FPDF_SYSFONTINFO* pThis, void* pMapper) {
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  pDefault->font_info_->EnumFontList(static_cast<CFX_FontMapper*>(pMapper));
}

static void* DefaultMapFont(struct _FPDF_SYSFONTINFO* pThis,
                            int weight,
                            FPDF_BOOL use_italic,
                            int charset,
                            int pitch_family,
                            const char* family,
                            FPDF_BOOL* /*exact*/) {
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  return pDefault->font_info_->MapFont(weight, !!use_italic,
                                       FX_GetCharsetFromInt(charset),
                                       pitch_family, family);
}

void* DefaultGetFont(struct _FPDF_SYSFONTINFO* pThis, const char* family) {
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  return pDefault->font_info_->GetFont(family);
}

// TODO(tsepez): should be UNSAFE_BUFFER_USAGE.
static unsigned long DefaultGetFontData(struct _FPDF_SYSFONTINFO* pThis,
                                        void* hFont,
                                        unsigned int table,
                                        unsigned char* buffer,
                                        unsigned long buf_size) {
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  // SAFETY: required from caller.
  return pdfium::checked_cast<unsigned long>(pDefault->font_info_->GetFontData(
      hFont, table, UNSAFE_BUFFERS(pdfium::span(buffer, buf_size))));
}

// TODO(tsepez): should be UNSAFE_BUFFER_USAGE.
static unsigned long DefaultGetFaceName(struct _FPDF_SYSFONTINFO* pThis,
                                        void* hFont,
                                        char* buffer,
                                        unsigned long buf_size) {
  ByteString name;
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  if (!pDefault->font_info_->GetFaceName(hFont, &name)) {
    return 0;
  }

  const unsigned long copy_length =
      pdfium::checked_cast<unsigned long>(name.GetLength() + 1);
  if (copy_length <= buf_size) {
    UNSAFE_TODO(strncpy(buffer, name.c_str(),
                        copy_length * sizeof(ByteString::CharType)));
  }
  return copy_length;
}

static int DefaultGetFontCharset(struct _FPDF_SYSFONTINFO* pThis, void* hFont) {
  FX_Charset charset;
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  if (!pDefault->font_info_->GetFontCharset(hFont, &charset)) {
    return 0;
  }
  return static_cast<int>(charset);
}

static void DefaultDeleteFont(struct _FPDF_SYSFONTINFO* pThis, void* hFont) {
  auto* pDefault = static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pThis);
  pDefault->font_info_->DeleteFont(hFont);
}

FPDF_EXPORT FPDF_SYSFONTINFO* FPDF_CALLCONV FPDF_GetDefaultSystemFontInfo() {
  std::unique_ptr<SystemFontInfoIface> pFontInfo =
      CFX_GEModule::Get()->GetPlatform()->CreateDefaultSystemFontInfo();
  if (!pFontInfo) {
    return nullptr;
  }

  FPDF_SYSFONTINFO_DEFAULT* pFontInfoExt =
      FX_Alloc(FPDF_SYSFONTINFO_DEFAULT, 1);
  pFontInfoExt->DeleteFont = DefaultDeleteFont;
  pFontInfoExt->EnumFonts = DefaultEnumFonts;
  pFontInfoExt->GetFaceName = DefaultGetFaceName;
  pFontInfoExt->GetFont = DefaultGetFont;
  pFontInfoExt->GetFontCharset = DefaultGetFontCharset;
  pFontInfoExt->GetFontData = DefaultGetFontData;
  pFontInfoExt->MapFont = DefaultMapFont;
  pFontInfoExt->Release = DefaultRelease;
  pFontInfoExt->version = 1;
  pFontInfoExt->font_info_ = pFontInfo.release();
  return pFontInfoExt;
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_FreeDefaultSystemFontInfo(FPDF_SYSFONTINFO* pFontInfo) {
  FX_Free(static_cast<FPDF_SYSFONTINFO_DEFAULT*>(pFontInfo));
}
