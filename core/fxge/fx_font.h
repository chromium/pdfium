// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_FX_FONT_H_
#define CORE_FXGE_FX_FONT_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_freetype.h"
#include "third_party/base/span.h"

/* Font pitch and family flags */
#define FXFONT_FF_FIXEDPITCH (1 << 0)
#define FXFONT_FF_ROMAN (1 << 4)
#define FXFONT_FF_SCRIPT (4 << 4)

/* Typical weight values */
#define FXFONT_FW_NORMAL 400
#define FXFONT_FW_BOLD 700
#define FXFONT_FW_BOLD_BOLD 900

/* Font styles as defined in PDF 1.7 Table 5.20 */
#define FXFONT_NORMAL (0)
#define FXFONT_FIXED_PITCH (1 << 0)
#define FXFONT_SERIF (1 << 1)
#define FXFONT_SYMBOLIC (1 << 2)
#define FXFONT_SCRIPT (1 << 3)
#define FXFONT_NONSYMBOLIC (1 << 5)
#define FXFONT_ITALIC (1 << 6)
#define FXFONT_ALLCAP (1 << 16)
#define FXFONT_SMALLCAP (1 << 17)
#define FXFONT_FORCE_BOLD (1 << 18)

/* Other font flags */
#define FXFONT_USEEXTERNATTR 0x80000
#define FXFONT_CIDFONT 0x100000

#define GET_TT_SHORT(w) (uint16_t)(((w)[0] << 8) | (w)[1])
#define GET_TT_LONG(w) \
  (uint32_t)(((w)[0] << 24) | ((w)[1] << 16) | ((w)[2] << 8) | (w)[3])

#if defined _SKIA_SUPPORT_ || defined _SKIA_SUPPORT_PATHS_
class SkTypeface;

using CFX_TypeFace = SkTypeface;
#endif

class TextGlyphPos;

FX_RECT GetGlyphsBBox(const std::vector<TextGlyphPos>& glyphs, int anti_alias);

ByteString GetNameFromTT(pdfium::span<const uint8_t> name_table, uint32_t name);
int GetTTCIndex(pdfium::span<const uint8_t> pFontData, uint32_t font_offset);

inline bool FontStyleIsForceBold(uint32_t style) {
  return !!(style & FXFONT_FORCE_BOLD);
}
inline bool FontStyleIsItalic(uint32_t style) {
  return !!(style & FXFONT_ITALIC);
}
inline bool FontStyleIsFixedPitch(uint32_t style) {
  return !!(style & FXFONT_FIXED_PITCH);
}
inline bool FontStyleIsSymbolic(uint32_t style) {
  return !!(style & FXFONT_SYMBOLIC);
}
inline bool FontStyleIsNonSymbolic(uint32_t style) {
  return !!(style & FXFONT_NONSYMBOLIC);
}
inline bool FontStyleIsAllCaps(uint32_t style) {
  return !!(style & FXFONT_ALLCAP);
}
inline bool FontStyleIsSerif(uint32_t style) {
  return !!(style & FXFONT_SERIF);
}
inline bool FontStyleIsScript(uint32_t style) {
  return !!(style & FXFONT_SCRIPT);
}

inline bool FontFamilyIsFixedPitch(uint32_t family) {
  return !!(family & FXFONT_FF_FIXEDPITCH);
}
inline bool FontFamilyIsRoman(uint32_t family) {
  return !!(family & FXFONT_FF_ROMAN);
}
inline bool FontFamilyIsScript(int32_t family) {
  return !!(family & FXFONT_FF_SCRIPT);
}

wchar_t PDF_UnicodeFromAdobeName(const char* name);
ByteString PDF_AdobeNameFromUnicode(wchar_t unicode);

#endif  // CORE_FXGE_FX_FONT_H_
