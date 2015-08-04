// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_GDIOBJECT_IMP
#define _FX_GDIOBJECT_IMP
#ifdef _FXPLUS
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
typedef struct _FX_GDIGOCACHE {
  GLYPHMETRICS gm;
  uint8_t* pOutline;
} FX_GDIGOCACHE, *FX_LPGDIGOCACHE;
typedef FX_GDIGOCACHE const* FX_LPCGDIGOCACHE;
class CFX_GdiFontCache {
 public:
  CFX_GdiFontCache();
  ~CFX_GdiFontCache();
  void SetCachedGlyphOutline(FX_DWORD dwGlyph,
                             const GLYPHMETRICS& gm,
                             uint8_t* pOutline);
  FX_LPCGDIGOCACHE GetCachedGlyphOutline(FX_DWORD dwGlyph) const;

 protected:
  CFX_MapPtrToPtr m_GlyphMap;
};
class CFX_GdiFont : public IFX_Font, public CFX_ThreadLock {
 public:
  CFX_GdiFont(IFX_FontMgr* pFontMgr);
  ~CFX_GdiFont();
  virtual void Release();
  virtual IFX_Font* Retain();
  FX_BOOL LoadFont(const FX_WCHAR* pszFontFamily,
                   FX_DWORD dwFontStyles,
                   FX_WORD wCodePage);
  FX_BOOL LoadFont(const uint8_t* pBuffer, int32_t iLength);
  FX_BOOL LoadFont(const FX_WCHAR* pszFileName);
  FX_BOOL LoadFont(IFX_Stream* pFontStream);
  FX_BOOL LoadFont(const LOGFONTW& lf);
  virtual IFX_Font* Derive(FX_DWORD dwFontStyles, FX_WORD wCodePage = 0) {
    return NULL;
  }
  virtual void GetFamilyName(CFX_WideString& wsFamily) const;
  virtual FX_DWORD GetFontStyles() const { return m_dwStyles; }
  virtual uint8_t GetCharSet() const { return m_LogFont.lfCharSet; }
  virtual FX_BOOL GetCharWidth(FX_WCHAR wUnicode,
                               int32_t& iWidth,
                               FX_BOOL bCharCode = FALSE);
  virtual int32_t GetGlyphIndex(FX_WCHAR wUnicode, FX_BOOL bCharCode = FALSE);
  virtual int32_t GetAscent() const;
  virtual int32_t GetDescent() const;
  virtual FX_BOOL GetCharBBox(FX_WCHAR wUnicode,
                              CFX_Rect& bbox,
                              FX_BOOL bCharCode = FALSE);
  virtual FX_BOOL GetBBox(CFX_Rect& bbox);
  virtual int32_t GetItalicAngle() const;
  virtual void Reset();
  FX_DWORD GetGlyphDIBits(int32_t iGlyphIndex,
                          FX_ARGB argb,
                          const MAT2* pMatrix,
                          GLYPHMETRICS& gm,
                          void* pBuffer,
                          FX_DWORD bufSize);
  FX_DWORD GetHashCode() const;

 protected:
  IFX_FontMgr* m_pFontMgr;
  int32_t m_iRefCount;
  CFX_WordDiscreteArray m_WidthCache;
  OUTLINETEXTMETRICW m_OutlineTM;
  HGDIOBJ m_hOldFont;
  HFONT m_hFont;
  HDC m_hDC;
  LOGFONTW m_LogFont;
  CFX_WideString m_wsFontFileName;
  CFX_WideStringArray m_FontFamilies;
  HANDLE m_hRes;
  FX_DWORD m_dwStyles;
  CFX_PtrArray m_SubstFonts;
  CFX_MapPtrToPtr m_FontMapper;
  CFX_MapPtrToPtr m_FontCache;
  void ClearCache();
  int32_t GetFontFamilies(Gdiplus::FontCollection& fc);
  void RetrieveFontStyles();
  IFX_Font* GetSubstFont(int32_t iGlyphIndex) const;
  FX_BOOL GetCharWidth(FX_WCHAR wUnicode,
                       int32_t& iWidth,
                       FX_BOOL bRecursive,
                       FX_BOOL bCharCode = FALSE);
  int32_t GetGlyphIndex(FX_WCHAR wUnicode,
                        FX_BOOL bRecursive,
                        IFX_Font** ppFont,
                        FX_BOOL bCharCode = FALSE);
  FX_DWORD GetMAT2HashCode(const FIXED* pFixed);
  void CreateGlyphBitmap(int32_t iWidth,
                         int32_t iHeight,
                         uint8_t* pOutline,
                         FX_DWORD* pDIB,
                         FX_ARGB argb);
  friend class CFX_GdiFontMgr;
};
#endif
#endif
#endif
