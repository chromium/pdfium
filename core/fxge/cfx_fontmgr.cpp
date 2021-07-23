// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_fontmgr.h"

#include <memory>
#include <utility>

#include "core/fxge/cfx_face.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/fontdata/chromefontdata/chromefontdata.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/systemfontinfo_iface.h"
#include "third_party/base/check.h"
#include "third_party/base/cxx17_backports.h"

namespace {

struct BuiltinFont {
  const uint8_t* m_pFontData;  // Raw, POD struct.
  uint32_t m_dwSize;
};

constexpr BuiltinFont kFoxitFonts[14] = {
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

const BuiltinFont kMMFonts[2] = {
    {kFoxitSerifMMFontData, 113417},
    {kFoxitSansMMFontData, 66919},
};

ByteString KeyNameFromFace(const ByteString& face_name,
                           int weight,
                           bool bItalic) {
  ByteString key(face_name);
  key += ',';
  key += ByteString::FormatInteger(weight);
  key += bItalic ? 'I' : 'N';
  return key;
}

ByteString KeyNameFromSize(int ttc_size, uint32_t checksum) {
  return ByteString::Format("%d:%d", ttc_size, checksum);
}

FXFT_LibraryRec* FTLibraryInitHelper() {
  FXFT_LibraryRec* pLibrary = nullptr;
  FT_Init_FreeType(&pLibrary);
  return pLibrary;
}

}  // namespace

CFX_FontMgr::FontDesc::FontDesc(std::unique_ptr<uint8_t, FxFreeDeleter> pData,
                                size_t size)
    : m_Size(size), m_pFontData(std::move(pData)) {}

CFX_FontMgr::FontDesc::~FontDesc() = default;

void CFX_FontMgr::FontDesc::SetFace(size_t index, CFX_Face* face) {
  DCHECK(index < pdfium::size(m_TTCFaces));
  m_TTCFaces[index].Reset(face);
}

CFX_Face* CFX_FontMgr::FontDesc::GetFace(size_t index) const {
  DCHECK(index < pdfium::size(m_TTCFaces));
  return m_TTCFaces[index].Get();
}

CFX_FontMgr::CFX_FontMgr()
    : m_FTLibrary(FTLibraryInitHelper()),
      m_pBuiltinMapper(std::make_unique<CFX_FontMapper>(this)),
      m_FTLibrarySupportsHinting(SetLcdFilterMode() ||
                                 FreeTypeVersionSupportsHinting()) {}

CFX_FontMgr::~CFX_FontMgr() = default;

void CFX_FontMgr::SetSystemFontInfo(
    std::unique_ptr<SystemFontInfoIface> pFontInfo) {
  m_pBuiltinMapper->SetSystemFontInfo(std::move(pFontInfo));
}

RetainPtr<CFX_Face> CFX_FontMgr::FindSubstFont(const ByteString& face_name,
                                               bool bTrueType,
                                               uint32_t flags,
                                               int weight,
                                               int italic_angle,
                                               FX_CodePage code_page,
                                               CFX_SubstFont* pSubstFont) {
  return m_pBuiltinMapper->FindSubstFont(face_name, bTrueType, flags, weight,
                                         italic_angle, code_page, pSubstFont);
}

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::GetCachedFontDesc(
    const ByteString& face_name,
    int weight,
    bool bItalic) {
  auto it = m_FaceMap.find(KeyNameFromFace(face_name, weight, bItalic));
  return it != m_FaceMap.end() ? pdfium::WrapRetain(it->second.Get()) : nullptr;
}

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::AddCachedFontDesc(
    const ByteString& face_name,
    int weight,
    bool bItalic,
    std::unique_ptr<uint8_t, FxFreeDeleter> pData,
    uint32_t size) {
  auto pFontDesc = pdfium::MakeRetain<FontDesc>(std::move(pData), size);
  m_FaceMap[KeyNameFromFace(face_name, weight, bItalic)].Reset(pFontDesc.Get());
  return pFontDesc;
}

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::GetCachedTTCFontDesc(
    int ttc_size,
    uint32_t checksum) {
  auto it = m_FaceMap.find(KeyNameFromSize(ttc_size, checksum));
  return it != m_FaceMap.end() ? pdfium::WrapRetain(it->second.Get()) : nullptr;
}

RetainPtr<CFX_FontMgr::FontDesc> CFX_FontMgr::AddCachedTTCFontDesc(
    int ttc_size,
    uint32_t checksum,
    std::unique_ptr<uint8_t, FxFreeDeleter> pData,
    uint32_t size) {
  auto pNewDesc = pdfium::MakeRetain<FontDesc>(std::move(pData), size);
  m_FaceMap[KeyNameFromSize(ttc_size, checksum)].Reset(pNewDesc.Get());
  return pNewDesc;
}

RetainPtr<CFX_Face> CFX_FontMgr::NewFixedFace(const RetainPtr<FontDesc>& pDesc,
                                              pdfium::span<const uint8_t> span,
                                              int face_index) {
  RetainPtr<CFX_Face> face =
      CFX_Face::New(m_FTLibrary.get(), pDesc, span, face_index);
  if (!face)
    return nullptr;

  if (FT_Set_Pixel_Sizes(face->GetRec(), 64, 64) != 0)
    return nullptr;

  return face;
}

// static
Optional<pdfium::span<const uint8_t>> CFX_FontMgr::GetBuiltinFont(
    size_t index) {
  if (index < pdfium::size(kFoxitFonts)) {
    return pdfium::make_span(kFoxitFonts[index].m_pFontData,
                             kFoxitFonts[index].m_dwSize);
  }
  size_t mm_index = index - pdfium::size(kFoxitFonts);
  if (mm_index < pdfium::size(kMMFonts)) {
    return pdfium::make_span(kMMFonts[mm_index].m_pFontData,
                             kMMFonts[mm_index].m_dwSize);
  }
  return pdfium::nullopt;
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
