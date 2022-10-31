// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_fontmgr.h"

#include <iterator>
#include <memory>
#include <utility>

#include "core/fxcrt/fixed_uninit_data_vector.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/fontdata/chromefontdata/chromefontdata.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "third_party/base/check_op.h"

namespace {

struct BuiltinFont {
  const uint8_t* m_pFontData;  // Raw, POD struct.
  uint32_t m_dwSize;
};

constexpr BuiltinFont kFoxitFonts[] = {
    {kFoxitFixedFontData, 17597},
    {kFoxitFixedBoldFontData, 18055},
    {kFoxitFixedBoldItalicFontData, 19151},
    {kFoxitFixedItalicFontData, 18746},
    {kFoxitSansFontData, 15025},
    {kFoxitSansBoldFontData, 16344},
    {kFoxitSansBoldItalicFontData, 16418},
    {kFoxitSansItalicFontData, 16339},
    {kFoxitSerifFontData, 19469},
    {kFoxitSerifBoldFontData, 19395},
    {kFoxitSerifBoldItalicFontData, 20733},
    {kFoxitSerifItalicFontData, 21227},
    {kFoxitSymbolFontData, 16729},
    {kFoxitDingbatsFontData, 29513},
};
static_assert(std::size(kFoxitFonts) == CFX_FontMapper::kNumStandardFonts,
              "Wrong font count");

constexpr BuiltinFont kGenericSansFont = {kFoxitSansMMFontData, 66919};
constexpr BuiltinFont kGenericSerifFont = {kFoxitSerifMMFontData, 113417};

FXFT_LibraryRec* FTLibraryInitHelper() {
  FXFT_LibraryRec* pLibrary = nullptr;
  FT_Init_FreeType(&pLibrary);
  return pLibrary;
}

}  // namespace

CFX_FontMgr::FontDesc::FontDesc(FixedUninitDataVector<uint8_t> data)
    : m_pFontData(std::move(data)) {}

CFX_FontMgr::FontDesc::~FontDesc() = default;

void CFX_FontMgr::FontDesc::SetFace(size_t index, CFX_Face* face) {
  CHECK_LT(index, std::size(m_TTCFaces));
  m_TTCFaces[index].Reset(face);
}

CFX_Face* CFX_FontMgr::FontDesc::GetFace(size_t index) const {
  CHECK_LT(index, std::size(m_TTCFaces));
  return m_TTCFaces[index].Get();
}

CFX_FontMgr::CFX_FontMgr()
    : m_FTLibrary(FTLibraryInitHelper()),
      m_pBuiltinMapper(std::make_unique<CFX_FontMapper>(this)),
      m_FTLibrarySupportsHinting(SetLcdFilterMode() ||
                                 FreeTypeVersionSupportsHinting()) {}

CFX_FontMgr::~CFX_FontMgr() = default;

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::GetCachedFontDesc(
    const ByteString& face_name,
    int weight,
    bool bItalic) {
  auto it = m_FaceMap.find({face_name, weight, bItalic});
  return it != m_FaceMap.end() ? pdfium::WrapRetain(it->second.Get()) : nullptr;
}

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::AddCachedFontDesc(
    const ByteString& face_name,
    int weight,
    bool bItalic,
    FixedUninitDataVector<uint8_t> data) {
  auto pFontDesc = pdfium::MakeRetain<FontDesc>(std::move(data));
  m_FaceMap[{face_name, weight, bItalic}].Reset(pFontDesc.Get());
  return pFontDesc;
}

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::GetCachedTTCFontDesc(
    size_t ttc_size,
    uint32_t checksum) {
  auto it = m_TTCFaceMap.find({ttc_size, checksum});
  return it != m_TTCFaceMap.end() ? pdfium::WrapRetain(it->second.Get())
                                  : nullptr;
}

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::AddCachedTTCFontDesc(
    size_t ttc_size,
    uint32_t checksum,
    FixedUninitDataVector<uint8_t> data) {
  auto pNewDesc = pdfium::MakeRetain<FontDesc>(std::move(data));
  m_TTCFaceMap[{ttc_size, checksum}].Reset(pNewDesc.Get());
  return pNewDesc;
}

RetainPtr<CFX_Face> CFX_FontMgr::NewFixedFace(RetainPtr<FontDesc> pDesc,
                                              pdfium::span<const uint8_t> span,
                                              size_t face_index) {
  RetainPtr<CFX_Face> face =
      CFX_Face::New(m_FTLibrary.get(), std::move(pDesc), span,
                    static_cast<FT_Long>(face_index));
  if (!face)
    return nullptr;

  if (FT_Set_Pixel_Sizes(face->GetRec(), 64, 64) != 0)
    return nullptr;

  return face;
}

// static
pdfium::span<const uint8_t> CFX_FontMgr::GetStandardFont(size_t index) {
  CHECK_LT(index, std::size(kFoxitFonts));
  return pdfium::make_span(kFoxitFonts[index].m_pFontData,
                           kFoxitFonts[index].m_dwSize);
}

// static
pdfium::span<const uint8_t> CFX_FontMgr::GetGenericSansFont() {
  return pdfium::make_span(kGenericSansFont.m_pFontData,
                           kGenericSansFont.m_dwSize);
}

// static
pdfium::span<const uint8_t> CFX_FontMgr::GetGenericSerifFont() {
  return pdfium::make_span(kGenericSerifFont.m_pFontData,
                           kGenericSerifFont.m_dwSize);
}

bool CFX_FontMgr::FreeTypeVersionSupportsHinting() const {
  FT_Int major;
  FT_Int minor;
  FT_Int patch;
  FT_Library_Version(m_FTLibrary.get(), &major, &minor, &patch);
  // Freetype versions >= 2.8.1 support hinting even if subpixel rendering is
  // disabled. https://sourceforge.net/projects/freetype/files/freetype2/2.8.1/
  return major > 2 || (major == 2 && minor > 8) ||
         (major == 2 && minor == 8 && patch >= 1);
}

bool CFX_FontMgr::SetLcdFilterMode() const {
  return FT_Library_SetLcdFilter(m_FTLibrary.get(), FT_LCD_FILTER_DEFAULT) !=
         FT_Err_Unimplemented_Feature;
}
