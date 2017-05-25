// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_FX_FONT_H_
#define CORE_FXGE_FX_FONT_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "core/fxge/fx_freetype.h"

typedef struct FT_FaceRec_* FXFT_Face;
typedef void* FXFT_Library;

class CFX_FaceCache;
class CFX_GlyphBitmap;
class CFX_PathData;
class CFX_SizeGlyphCache;

#if defined _SKIA_SUPPORT_ || defined _SKIA_SUPPORT_PATHS_
class SkTypeface;

using CFX_TypeFace = SkTypeface;
#endif

/* Font pitch and family flags */
#define FXFONT_FF_FIXEDPITCH 1
#define FXFONT_FF_ROMAN (1 << 4)
#define FXFONT_FF_SCRIPT (4 << 4)

/* Typical weight values */
#define FXFONT_FW_NORMAL 400
#define FXFONT_FW_BOLD 700

/* Font styles as defined in PDF 1.7 Table 5.20 */
#define FXFONT_FIXED_PITCH (1 << 0)
#define FXFONT_SERIF (1 << 1)
#define FXFONT_SYMBOLIC (1 << 2)
#define FXFONT_SCRIPT (1 << 3)
#define FXFONT_NONSYMBOLIC (1 << 5)
#define FXFONT_ITALIC (1 << 6)
#define FXFONT_ALLCAP (1 << 16)
#define FXFONT_SMALLCAP (1 << 17)
#define FXFONT_BOLD (1 << 18)

/* Other font flags */
#define FXFONT_USEEXTERNATTR 0x80000
#define FXFONT_CIDFONT 0x100000
#ifdef PDF_ENABLE_XFA
#define FXFONT_EXACTMATCH 0x80000000
#endif  // PDF_ENABLE_XFA

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
                 bool bTrueType,
                 uint32_t flags,
                 int weight,
                 int italic_angle,
                 int CharsetCP,
                 bool bVertical);

  bool LoadEmbedded(const uint8_t* data, uint32_t size);
  FXFT_Face GetFace() const { return m_Face; }
  CFX_SubstFont* GetSubstFont() const { return m_pSubstFont.get(); }

#ifdef PDF_ENABLE_XFA
  bool LoadFile(const CFX_RetainPtr<IFX_SeekableReadStream>& pFile,
                int nFaceIndex);

  bool LoadClone(const CFX_Font* pFont);
  void SetFace(FXFT_Face face);
  void SetSubstFont(std::unique_ptr<CFX_SubstFont> subst) {
    m_pSubstFont = std::move(subst);
  }
#endif  // PDF_ENABLE_XFA

  const CFX_GlyphBitmap* LoadGlyphBitmap(uint32_t glyph_index,
                                         bool bFontStyle,
                                         const CFX_Matrix* pMatrix,
                                         int dest_width,
                                         int anti_alias,
                                         int& text_flags) const;
  const CFX_PathData* LoadGlyphPath(uint32_t glyph_index, int dest_width) const;

#if defined _SKIA_SUPPORT_ || defined _SKIA_SUPPORT_PATHS_
  CFX_TypeFace* GetDeviceCache() const;
#endif

  int GetGlyphWidth(uint32_t glyph_index);
  int GetAscent() const;
  int GetDescent() const;
  bool GetGlyphBBox(uint32_t glyph_index, FX_RECT& bbox);
  bool IsItalic() const;
  bool IsBold() const;
  bool IsFixedWidth() const;
  bool IsVertical() const { return m_bVertical; }
  CFX_ByteString GetPsName() const;
  CFX_ByteString GetFamilyName() const;
  CFX_ByteString GetFaceName() const;
  bool IsTTFont() const;
  bool GetBBox(FX_RECT& bbox);
  bool IsEmbedded() const { return m_bEmbedded; }
  uint8_t* GetSubData() const { return m_pGsubData; }
  void SetSubData(uint8_t* data) { m_pGsubData = data; }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  void* GetPlatformFont() const { return m_pPlatformFont; }
  void SetPlatformFont(void* font) { m_pPlatformFont = font; }
#endif
  uint8_t* GetFontData() const { return m_pFontData; }
  uint32_t GetSize() const { return m_dwSize; }
  void AdjustMMParams(int glyph_index, int width, int weight) const;

  static const size_t kAngleSkewArraySize = 30;
  static const char s_AngleSkew[kAngleSkewArraySize];
  static const size_t kWeightPowArraySize = 100;
  static const uint8_t s_WeightPow[kWeightPowArraySize];
  static const uint8_t s_WeightPow_11[kWeightPowArraySize];
  static const uint8_t s_WeightPow_SHIFTJIS[kWeightPowArraySize];

#ifdef PDF_ENABLE_XFA
 protected:
  bool m_bShallowCopy;
  FXFT_StreamRec* m_pOwnedStream;
#endif  // PDF_ENABLE_XFA

 private:
  friend class CFX_FaceCache;
  CFX_PathData* LoadGlyphPathImpl(uint32_t glyph_index, int dest_width) const;
  CFX_FaceCache* GetFaceCache() const;
  void ReleasePlatformResource();
  void DeleteFace();
  void ClearFaceCache();

  FXFT_Face m_Face;
  mutable CFX_UnownedPtr<CFX_FaceCache> m_FaceCache;
  std::unique_ptr<CFX_SubstFont> m_pSubstFont;
  std::vector<uint8_t> m_pFontDataAllocation;
  uint8_t* m_pFontData;
  uint8_t* m_pGsubData;
  uint32_t m_dwSize;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  void* m_pPlatformFont;
#endif
  bool m_bEmbedded;
  bool m_bVertical;
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

class CFX_GlyphBitmap {
 public:
  CFX_GlyphBitmap();
  ~CFX_GlyphBitmap();

  int m_Top;
  int m_Left;
  CFX_RetainPtr<CFX_DIBitmap> m_pBitmap;
};

inline CFX_GlyphBitmap::CFX_GlyphBitmap()
    : m_pBitmap(pdfium::MakeRetain<CFX_DIBitmap>()) {}

inline CFX_GlyphBitmap::~CFX_GlyphBitmap() {}

class FXTEXT_GLYPHPOS {
 public:
  FXTEXT_GLYPHPOS();
  FXTEXT_GLYPHPOS(const FXTEXT_GLYPHPOS&);
  ~FXTEXT_GLYPHPOS();

  const CFX_GlyphBitmap* m_pGlyph;
  CFX_Point m_Origin;
  CFX_PointF m_fOrigin;
};

FX_RECT FXGE_GetGlyphsBBox(const std::vector<FXTEXT_GLYPHPOS>& glyphs,
                           int anti_alias,
                           float retinaScaleX,
                           float retinaScaleY);

CFX_ByteString GetNameFromTT(const uint8_t* name_table,
                             uint32_t name_table_size,
                             uint32_t name);

int PDF_GetStandardFontName(CFX_ByteString* name);

#endif  // CORE_FXGE_FX_FONT_H_
