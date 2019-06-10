// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_unicodeencoding.h"

#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/fx_freetype.h"

CFX_UnicodeEncoding::CFX_UnicodeEncoding(CFX_Font* pFont) : m_pFont(pFont) {}

CFX_UnicodeEncoding::~CFX_UnicodeEncoding() {}

uint32_t CFX_UnicodeEncoding::GlyphFromCharCode(uint32_t charcode) {
  FXFT_FaceRec* face = m_pFont->GetFaceRec();
  if (!face)
    return charcode;

  if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) == 0)
    return FT_Get_Char_Index(face, charcode);

  if (m_pFont->GetSubstFont() &&
      m_pFont->GetSubstFont()->m_Charset == FX_CHARSET_Symbol) {
    uint32_t index = 0;
    if (FT_Select_Charmap(face, FT_ENCODING_MS_SYMBOL) == 0)
      index = FT_Get_Char_Index(face, charcode);
    if (!index && !FT_Select_Charmap(face, FT_ENCODING_APPLE_ROMAN))
      return FT_Get_Char_Index(face, charcode);
  }
  return charcode;
}
