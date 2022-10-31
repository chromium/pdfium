// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_unicodeencodingex.h"

#include <memory>

#include "core/fxge/cfx_font.h"
#include "core/fxge/freetype/fx_freetype.h"
#include "core/fxge/fx_font.h"

#define ENC_TAG(a, b, c, d)                                               \
  (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | \
   (uint32_t)(d))

namespace {

constexpr uint32_t kEncodingExSymbol = ENC_TAG('s', 'y', 'm', 'b');
constexpr uint32_t kEncodingExUnicode = ENC_TAG('u', 'n', 'i', 'c');
constexpr uint32_t kEncodingExSjis = ENC_TAG('s', 'j', 'i', 's');
constexpr uint32_t kEncodingExGB2312 = ENC_TAG('g', 'b', ' ', ' ');
constexpr uint32_t kEncodingExBig5 = ENC_TAG('b', 'i', 'g', '5');
constexpr uint32_t kEncodingExWansung = ENC_TAG('w', 'a', 'n', 's');
constexpr uint32_t kEncodingExJohab = ENC_TAG('j', 'o', 'h', 'a');
constexpr uint32_t kEncodingExAdobeStandard = ENC_TAG('A', 'D', 'O', 'B');
constexpr uint32_t kEncodingExAdobeExpert = ENC_TAG('A', 'D', 'B', 'E');
constexpr uint32_t kEncodingExAdobeCustom = ENC_TAG('A', 'D', 'B', 'C');
constexpr uint32_t kEncodingExLatin1 = ENC_TAG('l', 'a', 't', '1');
constexpr uint32_t kEncodingExOldLatin2 = ENC_TAG('l', 'a', 't', '2');
constexpr uint32_t kEncodingExAppleRoman = ENC_TAG('a', 'r', 'm', 'n');

constexpr uint32_t kEncodingID[] = {
    kEncodingExSymbol,      kEncodingExUnicode,       kEncodingExSjis,
    kEncodingExGB2312,      kEncodingExBig5,          kEncodingExWansung,
    kEncodingExJohab,       kEncodingExAdobeStandard, kEncodingExAdobeExpert,
    kEncodingExAdobeCustom, kEncodingExLatin1,        kEncodingExOldLatin2,
    kEncodingExAppleRoman,
};

std::unique_ptr<CFX_UnicodeEncodingEx> FXFM_CreateFontEncoding(
    CFX_Font* pFont,
    uint32_t nEncodingID) {
  if (FXFT_Select_Charmap(pFont->GetFaceRec(), nEncodingID))
    return nullptr;
  return std::make_unique<CFX_UnicodeEncodingEx>(pFont, nEncodingID);
}

}  // namespace

CFX_UnicodeEncodingEx::CFX_UnicodeEncodingEx(CFX_Font* pFont,
                                             uint32_t EncodingID)
    : CFX_UnicodeEncoding(pFont), m_nEncodingID(EncodingID) {}

CFX_UnicodeEncodingEx::~CFX_UnicodeEncodingEx() = default;

uint32_t CFX_UnicodeEncodingEx::GlyphFromCharCode(uint32_t charcode) {
  FXFT_FaceRec* face = m_pFont->GetFaceRec();
  FT_UInt nIndex = FT_Get_Char_Index(face, charcode);
  if (nIndex > 0)
    return nIndex;
  int m = 0;
  while (m < face->num_charmaps) {
    uint32_t nEncodingID = FXFT_Get_Charmap_Encoding(face->charmaps[m++]);
    if (m_nEncodingID == nEncodingID)
      continue;
    int error = FXFT_Select_Charmap(face, nEncodingID);
    if (error)
      continue;
    nIndex = FT_Get_Char_Index(face, charcode);
    if (nIndex > 0) {
      m_nEncodingID = nEncodingID;
      return nIndex;
    }
  }
  FXFT_Select_Charmap(face, m_nEncodingID);
  return 0;
}

uint32_t CFX_UnicodeEncodingEx::CharCodeFromUnicode(wchar_t Unicode) const {
  if (m_nEncodingID == kEncodingExUnicode ||
      m_nEncodingID == kEncodingExSymbol) {
    return Unicode;
  }
  FXFT_FaceRec* face = m_pFont->GetFaceRec();
  for (int i = 0; i < face->num_charmaps; i++) {
    int nEncodingID = FXFT_Get_Charmap_Encoding(face->charmaps[i]);
    if (nEncodingID == kEncodingExUnicode || nEncodingID == kEncodingExSymbol) {
      return Unicode;
    }
  }
  return kInvalidCharCode;
}

std::unique_ptr<CFX_UnicodeEncodingEx> FX_CreateFontEncodingEx(
    CFX_Font* pFont) {
  if (!pFont || !pFont->GetFaceRec())
    return nullptr;

  for (uint32_t id : kEncodingID) {
    auto pFontEncoding = FXFM_CreateFontEncoding(pFont, id);
    if (pFontEncoding)
      return pFontEncoding;
  }
  return nullptr;
}
