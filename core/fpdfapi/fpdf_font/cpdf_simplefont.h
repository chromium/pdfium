// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_FONT_CPDF_SIMPLEFONT_H_
#define CORE_FPDFAPI_FPDF_FONT_CPDF_SIMPLEFONT_H_

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fpdfapi/fpdf_font/include/cpdf_fontencoding.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_SimpleFont : public CPDF_Font {
 public:
  CPDF_SimpleFont();
  virtual ~CPDF_SimpleFont();

  // CPDF_Font:
  int GetCharWidthF(FX_DWORD charcode, int level = 0) override;
  FX_RECT GetCharBBox(FX_DWORD charcode, int level = 0) override;
  int GlyphFromCharCode(FX_DWORD charcode, FX_BOOL* pVertGlyph = NULL) override;
  FX_BOOL IsUnicodeCompatible() const override;
  CFX_WideString UnicodeFromCharCode(FX_DWORD charcode) const override;
  FX_DWORD CharCodeFromUnicode(FX_WCHAR Unicode) const override;

  CPDF_FontEncoding* GetEncoding() { return &m_Encoding; }

 protected:
  virtual void LoadGlyphMap() = 0;

  FX_BOOL LoadCommon();
  void LoadSubstFont();
  void LoadFaceMetrics();
  void LoadCharMetrics(int charcode);

  CPDF_FontEncoding m_Encoding;
  uint16_t m_GlyphIndex[256];
  uint16_t m_ExtGID[256];
  CFX_ByteString* m_pCharNames;
  int m_BaseEncoding;
  uint16_t m_CharWidth[256];
  FX_SMALL_RECT m_CharBBox[256];
  FX_BOOL m_bUseFontWidth;
};

#endif  // CORE_FPDFAPI_FPDF_FONT_CPDF_SIMPLEFONT_H_
