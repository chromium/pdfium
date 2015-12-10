// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_GE_TEXT_INT_H_
#define CORE_SRC_FXGE_GE_TEXT_INT_H_

#include <map>

#include "core/include/fxge/fx_font.h"
#include "core/include/fxge/fx_freetype.h"

struct _CFX_UniqueKeyGen {
  void Generate(int count, ...);
  FX_CHAR m_Key[128];
  int m_KeyLen;
};
class CFX_SizeGlyphCache {
 public:
  CFX_SizeGlyphCache() {}
  ~CFX_SizeGlyphCache();
  std::map<FX_DWORD, CFX_GlyphBitmap*> m_GlyphMap;
};
class CTTFontDesc {
 public:
  CTTFontDesc() {
    m_Type = 0;
    m_pFontData = NULL;
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

#define CHARSET_FLAG_ANSI 1
#define CHARSET_FLAG_SYMBOL 2
#define CHARSET_FLAG_SHIFTJIS 4
#define CHARSET_FLAG_BIG5 8
#define CHARSET_FLAG_GB 16
#define CHARSET_FLAG_KOREAN 32

class CFX_FontFaceInfo {
 public:
  CFX_FontFaceInfo(CFX_ByteString filePath,
                   CFX_ByteString faceName,
                   CFX_ByteString fontTables,
                   FX_DWORD fontOffset,
                   FX_DWORD fileSize)
      : m_FilePath(filePath),
        m_FaceName(faceName),
        m_FontTables(fontTables),
        m_FontOffset(fontOffset),
        m_FileSize(fileSize),
        m_Styles(0),
        m_Charsets(0) {}

  const CFX_ByteString m_FilePath;
  const CFX_ByteString m_FaceName;
  const CFX_ByteString m_FontTables;
  const FX_DWORD m_FontOffset;
  const FX_DWORD m_FileSize;
  FX_DWORD m_Styles;
  FX_DWORD m_Charsets;
};

#endif  // CORE_SRC_FXGE_GE_TEXT_INT_H_
