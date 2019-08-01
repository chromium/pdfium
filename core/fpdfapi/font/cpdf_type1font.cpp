// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_type1font.h"

#include <algorithm>

#include "build/build_config.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/fx_freetype.h"

#if defined(OS_MACOSX)
#include "core/fxge/apple/apple_int.h"
#endif

namespace {

#if defined(OS_MACOSX)
struct GlyphNameMap {
  const char* m_pStrAdobe;    // Raw, POD struct.
  const char* m_pStrUnicode;  // Raw, POD struct.
};

const GlyphNameMap g_GlyphNameSubsts[] = {{"ff", "uniFB00"},
                                          {"ffi", "uniFB03"},
                                          {"ffl", "uniFB04"},
                                          {"fi", "uniFB01"},
                                          {"fl", "uniFB02"}};

const char* GlyphNameRemap(const char* pStrAdobe) {
  for (const auto& element : g_GlyphNameSubsts) {
    if (!FXSYS_stricmp(element.m_pStrAdobe, pStrAdobe))
      return element.m_pStrUnicode;
  }
  return nullptr;
}

#endif  // defined(OS_MACOSX)

bool FT_UseType1Charmap(FXFT_FaceRec* face) {
  if (FXFT_Get_Face_CharmapCount(face) == 0) {
    return false;
  }
  if (FXFT_Get_Face_CharmapCount(face) == 1 &&
      FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[0]) ==
          FT_ENCODING_UNICODE) {
    return false;
  }
  if (FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[0]) ==
      FT_ENCODING_UNICODE) {
    FT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[1]);
  } else {
    FT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[0]);
  }
  return true;
}

}  // namespace

CPDF_Type1Font::CPDF_Type1Font(CPDF_Document* pDocument,
                               CPDF_Dictionary* pFontDict)
    : CPDF_SimpleFont(pDocument, pFontDict) {
#if defined(OS_MACOSX)
  memset(m_ExtGID, 0xff, sizeof(m_ExtGID));
#endif
}

CPDF_Type1Font::~CPDF_Type1Font() = default;

bool CPDF_Type1Font::IsType1Font() const {
  return true;
}

const CPDF_Type1Font* CPDF_Type1Font::AsType1Font() const {
  return this;
}

CPDF_Type1Font* CPDF_Type1Font::AsType1Font() {
  return this;
}

bool CPDF_Type1Font::Load() {
  m_Base14Font = CFX_FontMapper::GetStandardFontName(&m_BaseFontName);
  if (!IsBase14Font())
    return LoadCommon();

  const CPDF_Dictionary* pFontDesc = m_pFontDict->GetDictFor("FontDescriptor");
  if (pFontDesc && pFontDesc->KeyExist("Flags")) {
    m_Flags = pFontDesc->GetIntegerFor("Flags");
  } else if (IsSymbolicFont()) {
    m_Flags = FXFONT_SYMBOLIC;
  } else {
    m_Flags = FXFONT_NONSYMBOLIC;
  }
  if (IsFixedFont()) {
    for (int i = 0; i < 256; i++)
      m_CharWidth[i] = 600;
  }
  if (m_Base14Font == CFX_FontMapper::kSymbol)
    m_BaseEncoding = PDFFONT_ENCODING_ADOBE_SYMBOL;
  else if (m_Base14Font == CFX_FontMapper::kDingbats)
    m_BaseEncoding = PDFFONT_ENCODING_ZAPFDINGBATS;
  else if (FontStyleIsNonSymbolic(m_Flags))
    m_BaseEncoding = PDFFONT_ENCODING_STANDARD;
  return LoadCommon();
}

#if defined(OS_MACOSX)
int CPDF_Type1Font::GlyphFromCharCodeExt(uint32_t charcode) {
  if (charcode > 0xff)
    return -1;

  int index = m_ExtGID[static_cast<uint8_t>(charcode)];
  return index != 0xffff ? index : -1;
}
#endif

void CPDF_Type1Font::LoadGlyphMap() {
  if (!m_Font.GetFaceRec())
    return;

#if defined(OS_MACOSX)
  bool bCoreText = true;
  CQuartz2D& quartz2d =
      static_cast<CApplePlatform*>(CFX_GEModule::Get()->GetPlatform())
          ->m_quartz2d;
  if (!m_Font.GetPlatformFont()) {
    if (m_Font.GetPsName() == "DFHeiStd-W5")
      bCoreText = false;

    pdfium::span<const uint8_t> span = m_Font.GetFontSpan();
    m_Font.SetPlatformFont(quartz2d.CreateFont(span.data(), span.size()));
    if (!m_Font.GetPlatformFont())
      bCoreText = false;
  }
#endif
  if (!IsEmbedded() && !IsSymbolicFont() && m_Font.IsTTFont()) {
    if (FT_UseTTCharmap(m_Font.GetFaceRec(), 3, 0)) {
      bool bGotOne = false;
      for (uint32_t charcode = 0; charcode < 256; charcode++) {
        const uint8_t prefix[4] = {0x00, 0xf0, 0xf1, 0xf2};
        for (int j = 0; j < 4; j++) {
          uint16_t unicode = prefix[j] * 256 + charcode;
          m_GlyphIndex[charcode] =
              FT_Get_Char_Index(m_Font.GetFaceRec(), unicode);
#if defined(OS_MACOSX)
          CalcExtGID(charcode);
#endif
          if (m_GlyphIndex[charcode]) {
            bGotOne = true;
            break;
          }
        }
      }
      if (bGotOne) {
#if defined(OS_MACOSX)
        if (!bCoreText)
          memcpy(m_ExtGID, m_GlyphIndex, 256);
#endif
        return;
      }
    }
    FXFT_Select_Charmap(m_Font.GetFaceRec(), FT_ENCODING_UNICODE);
    if (m_BaseEncoding == 0)
      m_BaseEncoding = PDFFONT_ENCODING_STANDARD;

    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      const char* name =
          GetAdobeCharName(m_BaseEncoding, m_CharNames, charcode);
      if (!name)
        continue;

      m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
      m_GlyphIndex[charcode] = FT_Get_Char_Index(
          m_Font.GetFaceRec(), m_Encoding.UnicodeFromCharCode(charcode));
#if defined(OS_MACOSX)
      CalcExtGID(charcode);
#endif
      if (m_GlyphIndex[charcode] == 0 && strcmp(name, ".notdef") == 0) {
        m_Encoding.SetUnicode(charcode, 0x20);
        m_GlyphIndex[charcode] = FT_Get_Char_Index(m_Font.GetFaceRec(), 0x20);
#if defined(OS_MACOSX)
        CalcExtGID(charcode);
#endif
      }
    }
#if defined(OS_MACOSX)
    if (!bCoreText)
      memcpy(m_ExtGID, m_GlyphIndex, 256);
#endif
    return;
  }
  FT_UseType1Charmap(m_Font.GetFaceRec());
#if defined(OS_MACOSX)
  if (bCoreText) {
    if (FontStyleIsSymbolic(m_Flags)) {
      for (uint32_t charcode = 0; charcode < 256; charcode++) {
        const char* name =
            GetAdobeCharName(m_BaseEncoding, m_CharNames, charcode);
        if (name) {
          m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
          m_GlyphIndex[charcode] =
              FXFT_Get_Name_Index(m_Font.GetFaceRec(), name);
          SetExtGID(name, charcode);
        } else {
          m_GlyphIndex[charcode] =
              FT_Get_Char_Index(m_Font.GetFaceRec(), charcode);
          wchar_t unicode = 0;
          if (m_GlyphIndex[charcode]) {
            unicode =
                FT_UnicodeFromCharCode(PDFFONT_ENCODING_STANDARD, charcode);
          }
          char name_glyph[256];
          memset(name_glyph, 0, sizeof(name_glyph));
          FT_Get_Glyph_Name(m_Font.GetFaceRec(), m_GlyphIndex[charcode],
                            name_glyph, 256);
          name_glyph[255] = 0;
          if (unicode == 0 && name_glyph[0] != 0)
            unicode = PDF_UnicodeFromAdobeName(name_glyph);

          m_Encoding.SetUnicode(charcode, unicode);
          SetExtGID(name_glyph, charcode);
        }
      }
      return;
    }

    bool bUnicode =
        FXFT_Select_Charmap(m_Font.GetFaceRec(), FT_ENCODING_UNICODE) == 0;
    for (uint32_t charcode = 0; charcode < 256; charcode++) {
      const char* name =
          GetAdobeCharName(m_BaseEncoding, m_CharNames, charcode);
      if (!name)
        continue;

      m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
      const char* pStrUnicode = GlyphNameRemap(name);
      if (pStrUnicode && FXFT_Get_Name_Index(m_Font.GetFaceRec(), name) == 0) {
        name = pStrUnicode;
      }
      m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.GetFaceRec(), name);
      SetExtGID(name, charcode);
      if (m_GlyphIndex[charcode] != 0)
        continue;

      if (strcmp(name, ".notdef") != 0 && strcmp(name, "space") != 0) {
        m_GlyphIndex[charcode] = FT_Get_Char_Index(
            m_Font.GetFaceRec(),
            bUnicode ? m_Encoding.UnicodeFromCharCode(charcode) : charcode);
        CalcExtGID(charcode);
      } else {
        m_Encoding.SetUnicode(charcode, 0x20);
        m_GlyphIndex[charcode] =
            bUnicode ? FT_Get_Char_Index(m_Font.GetFaceRec(), 0x20) : 0xffff;
        CalcExtGID(charcode);
      }
    }
    return;
  }
#endif  // defined(OS_MACOSX)
  if (FontStyleIsSymbolic(m_Flags)) {
    for (int charcode = 0; charcode < 256; charcode++) {
      const char* name =
          GetAdobeCharName(m_BaseEncoding, m_CharNames, charcode);
      if (name) {
        m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
        m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.GetFaceRec(), name);
      } else {
        m_GlyphIndex[charcode] =
            FT_Get_Char_Index(m_Font.GetFaceRec(), charcode);
        if (m_GlyphIndex[charcode]) {
          wchar_t unicode =
              FT_UnicodeFromCharCode(PDFFONT_ENCODING_STANDARD, charcode);
          if (unicode == 0) {
            char name_glyph[256];
            memset(name_glyph, 0, sizeof(name_glyph));
            FT_Get_Glyph_Name(m_Font.GetFaceRec(), m_GlyphIndex[charcode],
                              name_glyph, 256);
            name_glyph[255] = 0;
            if (name_glyph[0] != 0)
              unicode = PDF_UnicodeFromAdobeName(name_glyph);
          }
          m_Encoding.SetUnicode(charcode, unicode);
        }
      }
    }
#if defined(OS_MACOSX)
    if (!bCoreText)
      memcpy(m_ExtGID, m_GlyphIndex, 256);

#endif
    return;
  }

  bool bUnicode =
      FXFT_Select_Charmap(m_Font.GetFaceRec(), FT_ENCODING_UNICODE) == 0;
  for (int charcode = 0; charcode < 256; charcode++) {
    const char* name = GetAdobeCharName(m_BaseEncoding, m_CharNames, charcode);
    if (!name)
      continue;

    m_Encoding.SetUnicode(charcode, PDF_UnicodeFromAdobeName(name));
    m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.GetFaceRec(), name);
    if (m_GlyphIndex[charcode] != 0)
      continue;

    if (strcmp(name, ".notdef") != 0 && strcmp(name, "space") != 0) {
      m_GlyphIndex[charcode] = FT_Get_Char_Index(
          m_Font.GetFaceRec(),
          bUnicode ? m_Encoding.UnicodeFromCharCode(charcode) : charcode);
    } else {
      m_Encoding.SetUnicode(charcode, 0x20);
      m_GlyphIndex[charcode] = 0xffff;
    }
  }
#if defined(OS_MACOSX)
  if (!bCoreText)
    memcpy(m_ExtGID, m_GlyphIndex, 256);
#endif
}

bool CPDF_Type1Font::IsSymbolicFont() const {
  return m_Base14Font.has_value() &&
         CFX_FontMapper::IsSymbolicFont(m_Base14Font.value());
}

bool CPDF_Type1Font::IsFixedFont() const {
  return m_Base14Font.has_value() &&
         CFX_FontMapper::IsFixedFont(m_Base14Font.value());
}

#if defined(OS_MACOSX)
void CPDF_Type1Font::SetExtGID(const char* name, uint32_t charcode) {
  CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
      kCFAllocatorDefault, name, kCFStringEncodingASCII, kCFAllocatorNull);
  m_ExtGID[charcode] =
      CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.GetPlatformFont(), name_ct);
  if (name_ct)
    CFRelease(name_ct);
}

void CPDF_Type1Font::CalcExtGID(uint32_t charcode) {
  char name_glyph[256];
  FT_Get_Glyph_Name(m_Font.GetFaceRec(), m_GlyphIndex[charcode], name_glyph,
                    256);
  name_glyph[255] = 0;
  SetExtGID(name_glyph, charcode);
}
#endif  // defined(OS_MACOSX)
