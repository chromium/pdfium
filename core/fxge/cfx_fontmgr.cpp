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
#include "third_party/base/ptr_util.h"

namespace {

struct BuiltinFont {
  const uint8_t* m_pFontData;  // Raw, POD struct.
  uint32_t m_dwSize;
};

const BuiltinFont g_FoxitFonts[14] = {
    {g_FoxitFixedFontData, 17597},
    {g_FoxitFixedBoldFontData, 18055},
    {g_FoxitFixedBoldItalicFontData, 19151},
    {g_FoxitFixedItalicFontData, 18746},
    {g_FoxitSansFontData, 15025},
    {g_FoxitSansBoldFontData, 16344},
    {g_FoxitSansBoldItalicFontData, 16418},
    {g_FoxitSansItalicFontData, 16339},
    {g_FoxitSerifFontData, 19469},
    {g_FoxitSerifBoldFontData, 19395},
    {g_FoxitSerifBoldItalicFontData, 20733},
    {g_FoxitSerifItalicFontData, 21227},
    {g_FoxitSymbolFontData, 16729},
    {g_FoxitDingbatsFontData, 29513},
};

const BuiltinFont g_MMFonts[2] = {
    {g_FoxitSerifMMFontData, 113417},
    {g_FoxitSansMMFontData, 66919},
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
  ASSERT(index < FX_ArraySize(m_TTCFaces));
  m_TTCFaces[index].Reset(face);
}

CFX_Face* CFX_FontMgr::FontDesc::GetFace(size_t index) const {
  ASSERT(index < FX_ArraySize(m_TTCFaces));
  return m_TTCFaces[index].Get();
}

CFX_FontMgr::CFX_FontMgr()
    : m_FTLibrary(FTLibraryInitHelper()),
      m_pBuiltinMapper(pdfium::MakeUnique<CFX_FontMapper>(this)),
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
                                               int CharsetCP,
                                               CFX_SubstFont* pSubstFont) {
  return m_pBuiltinMapper->FindSubstFont(face_name, bTrueType, flags, weight,
                                         italic_angle, CharsetCP, pSubstFont);
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
  if (index < FX_ArraySize(g_FoxitFonts)) {
    return pdfium::make_span(g_FoxitFonts[index].m_pFontData,
                             g_FoxitFonts[index].m_dwSize);
  }
  size_t mm_index = index - FX_ArraySize(g_FoxitFonts);
  if (mm_index < FX_ArraySize(g_MMFonts)) {
    return pdfium::make_span(g_MMFonts[mm_index].m_pFontData,
                             g_MMFonts[mm_index].m_dwSize);
  }
  return {};
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
