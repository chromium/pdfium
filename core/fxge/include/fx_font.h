// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_INCLUDE_FX_FONT_H_
#define CORE_FXGE_INCLUDE_FX_FONT_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/include/fx_system.h"
#include "core/fxge/include/fx_dib.h"
#include "core/fxge/include/fx_freetype.h"

typedef struct FT_FaceRec_* FXFT_Face;
typedef void* FXFT_Library;

class CFX_FaceCache;
class CFX_FontCache;
class CFX_PathData;
class CFX_SizeGlyphCache;
class CFX_SubstFont;
class CTTFontDesc;

#ifdef _SKIA_SUPPORT_
class SkTypeface;

using CFX_TypeFace = SkTypeface;
#endif

#define FXFONT_FIXED_PITCH 0x01
#define FXFONT_SERIF 0x02
#define FXFONT_SYMBOLIC 0x04
#define FXFONT_SCRIPT 0x08
#define FXFONT_ITALIC 0x40
#define FXFONT_BOLD 0x40000
#define FXFONT_USEEXTERNATTR 0x80000
#define FXFONT_CIDFONT 0x100000
#ifdef PDF_ENABLE_XFA
#define FXFONT_EXACTMATCH 0x80000000
#endif  // PDF_ENABLE_XFA
#define FXFONT_ANSI_CHARSET 0
#define FXFONT_DEFAULT_CHARSET 1
#define FXFONT_SYMBOL_CHARSET 2
#define FXFONT_SHIFTJIS_CHARSET 128
#define FXFONT_HANGEUL_CHARSET 129
#define FXFONT_GB2312_CHARSET 134
#define FXFONT_CHINESEBIG5_CHARSET 136
#define FXFONT_THAI_CHARSET 222
#define FXFONT_EASTEUROPE_CHARSET 238
#define FXFONT_RUSSIAN_CHARSET 204
#define FXFONT_GREEK_CHARSET 161
#define FXFONT_TURKISH_CHARSET 162
#define FXFONT_HEBREW_CHARSET 177
#define FXFONT_ARABIC_CHARSET 178
#define FXFONT_BALTIC_CHARSET 186
#define FXFONT_FF_FIXEDPITCH 1
#define FXFONT_FF_ROMAN (1 << 4)
#define FXFONT_FF_SCRIPT (4 << 4)
#define FXFONT_FW_NORMAL 400
#define FXFONT_FW_BOLD 700

#define CHARSET_FLAG_ANSI 1
#define CHARSET_FLAG_SYMBOL 2
#define CHARSET_FLAG_SHIFTJIS 4
#define CHARSET_FLAG_BIG5 8
#define CHARSET_FLAG_GB 16
#define CHARSET_FLAG_KOREAN 32

#define GET_TT_SHORT(w) (uint16_t)(((w)[0] << 8) | (w)[1])
#define GET_TT_LONG(w) \
  (uint32_t)(((w)[0] << 24) | ((w)[1] << 16) | ((w)[2] << 8) | (w)[3])

// Sets the given transform on the font, and resets it to the identity when it
// goes out of scope.
class ScopedFontTransform {
 public:
  ScopedFontTransform(FT_Face face, FXFT_Matrix* matrix);
  ~ScopedFontTransform();

 private:
  FT_Face m_Face;
};

class CFX_Font {
 public:
  CFX_Font();
  ~CFX_Font();

  void LoadSubst(const CFX_ByteString& face_name,
                 FX_BOOL bTrueType,
                 uint32_t flags,
                 int weight,
                 int italic_angle,
                 int CharsetCP,
                 FX_BOOL bVertical = FALSE);

  FX_BOOL LoadEmbedded(const uint8_t* data, uint32_t size);
  FXFT_Face GetFace() const { return m_Face; }
  CFX_SubstFont* GetSubstFont() const { return m_pSubstFont.get(); }

#ifdef PDF_ENABLE_XFA
  FX_BOOL LoadFile(IFX_FileRead* pFile,
                   int nFaceIndex = 0,
                   int* pFaceCount = nullptr);

  FX_BOOL LoadClone(const CFX_Font* pFont);
  void SetFace(FXFT_Face face) { m_Face = face; }
  void SetSubstFont(std::unique_ptr<CFX_SubstFont> subst) {
    m_pSubstFont = std::move(subst);
  }
#endif  // PDF_ENABLE_XFA

  CFX_PathData* LoadGlyphPath(uint32_t glyph_index, int dest_width = 0);
  int GetGlyphWidth(uint32_t glyph_index);
  int GetAscent() const;
  int GetDescent() const;
  FX_BOOL GetGlyphBBox(uint32_t glyph_index, FX_RECT& bbox);
  FX_BOOL IsItalic() const;
  FX_BOOL IsBold() const;
  FX_BOOL IsFixedWidth() const;
  FX_BOOL IsVertical() const { return m_bVertical; }
  CFX_ByteString GetPsName() const;
  CFX_ByteString GetFamilyName() const;
  CFX_ByteString GetFaceName() const;
  FX_BOOL IsTTFont() const;
  FX_BOOL GetBBox(FX_RECT& bbox);
  int GetHeight() const;
  int GetULPos() const;
  int GetULthickness() const;
  int GetMaxAdvanceWidth() const;
  FX_BOOL IsEmbedded() const { return m_bEmbedded; }
  uint8_t* GetSubData() const { return m_pGsubData; }
  void SetSubData(uint8_t* data) { m_pGsubData = data; }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  void* GetPlatformFont() const { return m_pPlatformFont; }
  void SetPlatformFont(void* font) { m_pPlatformFont = font; }
#endif
  uint8_t* GetFontData() const { return m_pFontData; }
  uint32_t GetSize() const { return m_dwSize; }
  void AdjustMMParams(int glyph_index, int width, int weight);

  static const size_t kAngleSkewArraySize = 30;
  static const char s_AngleSkew[kAngleSkewArraySize];
  static const size_t kWeightPowArraySize = 100;
  static const uint8_t s_WeightPow[kWeightPowArraySize];
  static const uint8_t s_WeightPow_11[kWeightPowArraySize];
  static const uint8_t s_WeightPow_SHIFTJIS[kWeightPowArraySize];

#ifdef PDF_ENABLE_XFA
 protected:
  CFX_BinaryBuf m_OtfFontData;
  FX_BOOL m_bLogic;
  void* m_pOwnedStream;
#endif  // PDF_ENABLE_XFA

 private:
  void ReleasePlatformResource();
  void DeleteFace();

  FXFT_Face m_Face;
  std::unique_ptr<CFX_SubstFont> m_pSubstFont;
  std::vector<uint8_t> m_pFontDataAllocation;
  uint8_t* m_pFontData;
  uint8_t* m_pGsubData;
  uint32_t m_dwSize;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  void* m_pPlatformFont;
#endif
  FX_BOOL m_bEmbedded;
  FX_BOOL m_bVertical;
};

#define FXFONT_SUBST_MM 0x01
#define FXFONT_SUBST_GLYPHPATH 0x04
#define FXFONT_SUBST_CLEARTYPE 0x08
#define FXFONT_SUBST_TRANSFORM 0x10
#define FXFONT_SUBST_NONSYMBOL 0x20
#define FXFONT_SUBST_EXACT 0x40
#define FXFONT_SUBST_STANDARD 0x80

class CFX_SubstFont {
 public:
  CFX_SubstFont();

  CFX_ByteString m_Family;
  int m_Charset;
  uint32_t m_SubstFlags;
  int m_Weight;
  int m_ItalicAngle;
  bool m_bSubstCJK;
  int m_WeightCJK;
  bool m_bItalicCJK;
};

#define FX_FONT_FLAG_SERIF 0x01
#define FX_FONT_FLAG_FIXEDPITCH 0x02
#define FX_FONT_FLAG_ITALIC 0x04
#define FX_FONT_FLAG_BOLD 0x08
#define FX_FONT_FLAG_SYMBOLIC_SYMBOL 0x10
#define FX_FONT_FLAG_SYMBOLIC_DINGBATS 0x20
#define FX_FONT_FLAG_MULTIPLEMASTER 0x40

class CTTFontDesc {
 public:
  CTTFontDesc() {
    m_Type = 0;
    m_pFontData = nullptr;
    m_RefCount = 0;
  }
  ~CTTFontDesc();
  // ret < 0, releaseface not appropriate for this object.
  // ret == 0, object released
  // ret > 0, object still alive, other referrers.
  int ReleaseFace(FXFT_Face face);
  int m_Type;
  union {
    struct {
      FX_BOOL m_bItalic;
      FX_BOOL m_bBold;
      FXFT_Face m_pFace;
    } m_SingleFace;
    struct {
      FXFT_Face m_pFaces[16];
    } m_TTCFace;
  };
  uint8_t* m_pFontData;
  int m_RefCount;
};

class CFX_FontFaceInfo {
 public:
  CFX_FontFaceInfo(CFX_ByteString filePath,
                   CFX_ByteString faceName,
                   CFX_ByteString fontTables,
                   uint32_t fontOffset,
                   uint32_t fileSize);

  const CFX_ByteString m_FilePath;
  const CFX_ByteString m_FaceName;
  const CFX_ByteString m_FontTables;
  const uint32_t m_FontOffset;
  const uint32_t m_FileSize;
  uint32_t m_Styles;
  uint32_t m_Charsets;
};

class CFX_CountedFaceCache {
 public:
  CFX_FaceCache* m_Obj;
  uint32_t m_nCount;
};

class CFX_GlyphBitmap {
 public:
  int m_Top;
  int m_Left;
  CFX_DIBitmap m_Bitmap;
};

struct FXTEXT_GLYPHPOS {
  const CFX_GlyphBitmap* m_pGlyph;
  int m_OriginX;
  int m_OriginY;
  FX_FLOAT m_fOriginX;
  FX_FLOAT m_fOriginY;
};

FX_RECT FXGE_GetGlyphsBBox(const std::vector<FXTEXT_GLYPHPOS>& glyphs,
                           int anti_alias,
                           FX_FLOAT retinaScaleX = 1.0f,
                           FX_FLOAT retinaScaleY = 1.0f);

CFX_ByteString GetNameFromTT(const uint8_t* name_table,
                             uint32_t name_table_size,
                             uint32_t name);

int PDF_GetStandardFontName(CFX_ByteString* name);

#endif  // CORE_FXGE_INCLUDE_FX_FONT_H_
