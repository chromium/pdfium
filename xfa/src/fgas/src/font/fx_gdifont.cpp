// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/fgas/src/fgas_base.h"
#include "fx_gdifont.h"
#include "fx_stdfontmgr.h"
#ifdef _FXPLUS
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
CFX_GdiFontCache::CFX_GdiFontCache() : m_GlyphMap(128) {}
CFX_GdiFontCache::~CFX_GdiFontCache() {
  FX_POSITION pos = m_GlyphMap.GetStartPosition();
  int32_t iGlyph;
  FX_LPGDIGOCACHE pGlyph;
  while (pos != NULL) {
    pGlyph = NULL;
    m_GlyphMap.GetNextAssoc(pos, (void*&)iGlyph, (void*&)pGlyph);
    if (pGlyph != NULL) {
      FX_Free(pGlyph->pOutline);
      FX_Free(pGlyph);
    }
  }
  m_GlyphMap.RemoveAll();
}
void CFX_GdiFontCache::SetCachedGlyphOutline(FX_DWORD dwGlyph,
                                             const GLYPHMETRICS& gm,
                                             uint8_t* pOutline) {
  FXSYS_assert(pOutline != NULL);
  FX_LPGDIGOCACHE pGlyph = FX_Alloc(FX_GDIGOCACHE, 1);
  pGlyph->gm = gm;
  pGlyph->pOutline = pOutline;
  m_GlyphMap.SetAt((void*)dwGlyph, (void*)pGlyph);
}
FX_LPCGDIGOCACHE CFX_GdiFontCache::GetCachedGlyphOutline(
    FX_DWORD dwGlyph) const {
  FX_LPCGDIGOCACHE pGlyph = NULL;
  if (!m_GlyphMap.Lookup((void*)dwGlyph, (void*&)pGlyph)) {
    return FALSE;
  }
  return pGlyph;
}
IFX_Font* IFX_Font::LoadFont(const FX_WCHAR* pszFontFamily,
                             FX_DWORD dwFontStyles,
                             FX_WORD wCodePage,
                             IFX_FontMgr* pFontMgr) {
  CFX_GdiFont* pFont = new CFX_GdiFont(pFontMgr);
  if (!pFont->LoadFont(pszFontFamily, dwFontStyles, wCodePage)) {
    pFont->Release();
    return NULL;
  }
  return pFont;
}
IFX_Font* IFX_Font::LoadFont(const uint8_t* pBuffer,
                             int32_t iLength,
                             IFX_FontMgr* pFontMgr) {
  CFX_GdiFont* pFont = new CFX_GdiFont(pFontMgr);
  if (!pFont->LoadFont(pBuffer, iLength)) {
    pFont->Release();
    return NULL;
  }
  return pFont;
}
IFX_Font* IFX_Font::LoadFont(const FX_WCHAR* pszFileName,
                             IFX_FontMgr* pFontMgr) {
  CFX_GdiFont* pFont = new CFX_GdiFont(pFontMgr);
  if (!pFont->LoadFont(pszFileName)) {
    pFont->Release();
    return NULL;
  }
  return pFont;
}
IFX_Font* IFX_Font::LoadFont(IFX_Stream* pFontStream,
                             IFX_FontMgr* pFontMgr,
                             FX_BOOL bSaveStream) {
  CFX_GdiFont* pFont = new CFX_GdiFont(pFontMgr);
  if (!pFont->LoadFont(pFontStream)) {
    pFont->Release();
    return NULL;
  }
  return pFont;
}
IFX_Font* IFX_Font::LoadFont(CFX_Font* pExtFont, IFX_FontMgr* pFontMgr) {
  FXSYS_assert(FALSE);
  return NULL;
}
#define FX_GDIFONT_FONTCACHESIZE 8
CFX_GdiFont::CFX_GdiFont(IFX_FontMgr* pFontMgr)
    : m_pFontMgr(pFontMgr),
      m_iRefCount(1),
      m_WidthCache(1024),
      m_hOldFont(NULL),
      m_hFont(NULL),
      m_hDC(NULL),
      m_wsFontFileName(),
      m_FontFamilies(),
      m_hRes(NULL),
      m_dwStyles(0),
      m_SubstFonts(),
      m_FontMapper(16),
      m_FontCache(FX_GDIFONT_FONTCACHESIZE) {
  m_hDC = ::CreateCompatibleDC(NULL);
  FX_memset(&m_LogFont, 0, sizeof(m_LogFont));
  FXSYS_assert(m_hDC != NULL);
}
CFX_GdiFont::~CFX_GdiFont() {
  int32_t iCount = m_SubstFonts.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    IFX_Font* pFont = (IFX_Font*)m_SubstFonts[i];
    pFont->Release();
  }
  m_SubstFonts.RemoveAll();
  m_FontMapper.RemoveAll();
  if (m_hFont != NULL) {
    ::SelectObject(m_hDC, m_hOldFont);
    ::DeleteObject(m_hFont);
  }
  ::DeleteDC(m_hDC);
  if (m_hRes != NULL) {
    if (m_wsFontFileName.GetLength() > 0) {
      ::RemoveFontResourceW((const FX_WCHAR*)m_wsFontFileName);
    } else {
      ::RemoveFontMemResourceEx(m_hRes);
    }
  }
  m_WidthCache.RemoveAll();
  ClearCache();
}
void CFX_GdiFont::ClearCache() {
  int32_t iCount = m_SubstFonts.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    IFX_Font* pFont = (IFX_Font*)m_SubstFonts[i];
    ((CFX_GdiFont*)pFont)->ClearCache();
  }
  FX_POSITION pos = m_FontCache.GetStartPosition();
  FX_DWORD dwMAT2;
  CFX_GdiFontCache* pCache;
  while (pos != NULL) {
    pCache = NULL;
    m_FontCache.GetNextAssoc(pos, (void*&)dwMAT2, (void*&)pCache);
    if (pCache != NULL) {
      delete pCache;
    }
  }
  m_FontCache.RemoveAll();
}
void CFX_GdiFont::Release() {
  if (--m_iRefCount < 1) {
    if (m_pFontMgr != NULL) {
      m_pFontMgr->RemoveFont(this);
    }
    delete this;
  }
}
IFX_Font* CFX_GdiFont::Retain() {
  ++m_iRefCount;
  return this;
}
FX_BOOL CFX_GdiFont::LoadFont(const FX_WCHAR* pszFontFamily,
                              FX_DWORD dwFontStyles,
                              FX_WORD wCodePage) {
  FXSYS_assert(m_hFont == NULL);
  LOGFONTW lf;
  FX_memset(&lf, 0, sizeof(lf));
  lf.lfHeight = -1000;
  lf.lfWeight = (dwFontStyles & FX_FONTSTYLE_Bold) ? FW_BOLD : FW_NORMAL;
  lf.lfItalic = (dwFontStyles & FX_FONTSTYLE_Italic) != 0;
  lf.lfPitchAndFamily =
      (dwFontStyles & FX_FONTSTYLE_FixedPitch) ? FIXED_PITCH : VARIABLE_PITCH;
  if (dwFontStyles & FX_FONTSTYLE_Serif) {
    lf.lfPitchAndFamily |= FF_ROMAN;
  }
  if (dwFontStyles & FX_FONTSTYLE_Script) {
    lf.lfPitchAndFamily |= FF_SCRIPT;
  }
  if (dwFontStyles & FX_FONTSTYLE_Symbolic) {
    lf.lfCharSet = SYMBOL_CHARSET;
  } else {
    FX_WORD wCharSet = FX_GetCharsetFromCodePage(wCodePage);
    lf.lfCharSet = wCharSet != 0xFFFF ? (uint8_t)wCharSet : DEFAULT_CHARSET;
  }
  if (pszFontFamily == NULL) {
    lf.lfFaceName[0] = L'\0';
  } else {
    FXSYS_wcsncpy(lf.lfFaceName, pszFontFamily, 31);
  }
  return LoadFont(lf);
}
FX_BOOL CFX_GdiFont::LoadFont(const uint8_t* pBuffer, int32_t iLength) {
  FXSYS_assert(m_hFont == NULL && pBuffer != NULL && iLength > 0);
  Gdiplus::PrivateFontCollection pfc;
  if (pfc.AddMemoryFont(pBuffer, iLength) != Gdiplus::Ok) {
    return FALSE;
  }
  if (GetFontFamilies(pfc) < 1) {
    return FALSE;
  }
  FX_DWORD dwCount = 0;
  m_hRes = ::AddFontMemResourceEx((void*)pBuffer, iLength, 0, &dwCount);
  if (m_hRes == NULL) {
    return FALSE;
  }
  CFX_WideString wsFamily = m_FontFamilies[0];
  m_hFont =
      ::CreateFontW(-1000, 0, 0, 0, FW_NORMAL, FALSE, 0, 0, DEFAULT_CHARSET, 0,
                    0, 0, 0, (const FX_WCHAR*)wsFamily);
  if (m_hFont == NULL) {
    ::RemoveFontMemResourceEx(m_hRes);
    m_hRes = NULL;
    return FALSE;
  }
  RetrieveFontStyles();
  m_hOldFont = ::SelectObject(m_hDC, m_hFont);
  ::GetOutlineTextMetricsW(m_hDC, sizeof(m_OutlineTM), &m_OutlineTM);
  return TRUE;
}
FX_BOOL CFX_GdiFont::LoadFont(const FX_WCHAR* pszFileName) {
  FXSYS_assert(m_hFont == NULL && pszFileName != NULL);
  Gdiplus::PrivateFontCollection pfc;
  if (pfc.AddFontFile(pszFileName) != Gdiplus::Ok) {
    return FALSE;
  }
  if (GetFontFamilies(pfc) < 1) {
    return FALSE;
  }
  m_wsFontFileName = pszFileName;
  m_hRes = (HANDLE)::AddFontResourceW(pszFileName);
  if (m_hRes == NULL) {
    return FALSE;
  }
  CFX_WideString wsFamily = m_FontFamilies[0];
  m_hFont =
      ::CreateFontW(-1000, 0, 0, 0, FW_NORMAL, FALSE, 0, 0, DEFAULT_CHARSET, 0,
                    0, 0, 0, (const FX_WCHAR*)wsFamily);
  if (m_hFont == NULL) {
    ::RemoveFontResourceW(pszFileName);
    m_hRes = NULL;
    return FALSE;
  }
  RetrieveFontStyles();
  ::SelectObject(m_hDC, m_hFont);
  ::GetOutlineTextMetricsW(m_hDC, sizeof(m_OutlineTM), &m_OutlineTM);
  return TRUE;
}
FX_BOOL CFX_GdiFont::LoadFont(IFX_Stream* pFontStream) {
  FXSYS_assert(m_hFont == NULL && pFontStream != NULL);
  int32_t iLength = pFontStream->GetLength();
  if (iLength < 1) {
    return FALSE;
  }
  uint8_t* pBuf = FX_Alloc(uint8_t, iLength);
  iLength = pFontStream->ReadData(pBuf, iLength);
  FX_BOOL bRet = LoadFont(pBuf, iLength);
  FX_Free(pBuf);
  return bRet;
}
FX_BOOL CFX_GdiFont::LoadFont(const LOGFONTW& lf) {
  FXSYS_assert(m_hFont == NULL);
  m_hFont = ::CreateFontIndirectW((LPLOGFONTW)&lf);
  if (m_hFont == NULL) {
    return FALSE;
  }
  RetrieveFontStyles();
  ::SelectObject(m_hDC, m_hFont);
  ::GetOutlineTextMetricsW(m_hDC, sizeof(m_OutlineTM), &m_OutlineTM);
  return TRUE;
}
int32_t CFX_GdiFont::GetFontFamilies(Gdiplus::FontCollection& fc) {
  int32_t iCount = fc.GetFamilyCount();
  if (iCount < 1) {
    return iCount;
  }
  Gdiplus::FontFamily* pFontFamilies = FX_Alloc(Gdiplus::FontFamily, iCount);
  int32_t iFind = 0;
  fc.GetFamilies(iCount, pFontFamilies, &iFind);
  for (int32_t i = 0; i < iCount; i++) {
    CFX_WideString wsFamilyName;
    FX_WCHAR* pName = wsFamilyName.GetBuffer(LF_FACESIZE);
    pFontFamilies[i].GetFamilyName(pName);
    wsFamilyName.ReleaseBuffer();
    m_FontFamilies.Add(wsFamilyName);
  }
  FX_Free(pFontFamilies);
  return iCount;
}
void CFX_GdiFont::RetrieveFontStyles() {
  FXSYS_assert(m_hFont != NULL);
  FX_memset(&m_LogFont, 0, sizeof(m_LogFont));
  ::GetObjectW(m_hFont, sizeof(m_LogFont), &m_LogFont);
  m_dwStyles = FX_GetGdiFontStyles(m_LogFont);
}
void CFX_GdiFont::GetFamilyName(CFX_WideString& wsFamily) const {
  FXSYS_assert(m_hFont != NULL);
  wsFamily = m_LogFont.lfFaceName;
}
FX_BOOL CFX_GdiFont::GetCharWidth(FX_WCHAR wUnicode,
                                  int32_t& iWidth,
                                  FX_BOOL bRecursive,
                                  FX_BOOL bCharCode) {
  iWidth = (int32_t)(int16_t)m_WidthCache.GetAt(wUnicode, 0);
  if (iWidth == 0 || iWidth == -1) {
    IFX_Font* pFont = NULL;
    int32_t iGlyph = GetGlyphIndex(wUnicode, TRUE, &pFont, bCharCode);
    if (iGlyph != 0xFFFF && pFont != NULL) {
      if (pFont == (IFX_Font*)this) {
        if (!::GetCharWidthI(m_hDC, iGlyph, 1, NULL, &iWidth)) {
          iWidth = -1;
        }
      } else if (((CFX_GdiFont*)pFont)
                     ->GetCharWidth(wUnicode, iWidth, FALSE, bCharCode)) {
        return TRUE;
      }
    } else {
      iWidth = -1;
    }
    Lock();
    m_WidthCache.SetAtGrow(wUnicode, (int16_t)iWidth);
    Unlock();
  }
  return iWidth > 0;
}
FX_BOOL CFX_GdiFont::GetCharWidth(FX_WCHAR wUnicode,
                                  int32_t& iWidth,
                                  FX_BOOL bCharCode) {
  return GetCharWidth(wUnicode, iWidth, TRUE, bCharCode);
}
int32_t CFX_GdiFont::GetGlyphIndex(FX_WCHAR wUnicode,
                                   FX_BOOL bRecursive,
                                   IFX_Font** ppFont,
                                   FX_BOOL bCharCode) {
  int32_t iGlyph = 0XFFFF;
  if (::GetGlyphIndicesW(m_hDC, &wUnicode, 1, (LPWORD)&iGlyph,
                         GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR &&
      iGlyph != 0xFFFF) {
    if (ppFont != NULL) {
      *ppFont = (IFX_Font*)this;
    }
    return iGlyph;
  }
  FX_LPCFONTUSB pFontUSB = FX_GetUnicodeBitField(wUnicode);
  if (pFontUSB == NULL) {
    return 0xFFFF;
  }
  FX_WORD wBitField = pFontUSB->wBitField;
  if (wBitField >= 128) {
    return 0xFFFF;
  }
  IFX_Font* pFont = NULL;
  m_FontMapper.Lookup((void*)wBitField, (void*&)pFont);
  if (pFont != NULL && pFont != (IFX_Font*)this) {
    iGlyph =
        ((CFX_GdiFont*)pFont)->GetGlyphIndex(wUnicode, FALSE, NULL, bCharCode);
    if (iGlyph != 0xFFFF) {
      int32_t i = m_SubstFonts.Find(pFont);
      if (i > -1) {
        iGlyph |= ((i + 1) << 24);
        if (ppFont != NULL) {
          *ppFont = pFont;
        }
        return iGlyph;
      }
    }
  }
  if (m_pFontMgr != NULL && bRecursive) {
    IFX_Font* pFont = m_pFontMgr->GetDefFontByUnicode(wUnicode, m_dwStyles,
                                                      m_LogFont.lfFaceName);
    if (pFont != NULL) {
      if (pFont == (IFX_Font*)this) {
        pFont->Release();
        return 0xFFFF;
      }
      m_FontMapper.SetAt((void*)wBitField, (void*)pFont);
      int32_t i = m_SubstFonts.GetSize();
      m_SubstFonts.Add(pFont);
      iGlyph = ((CFX_GdiFont*)pFont)
                   ->GetGlyphIndex(wUnicode, FALSE, NULL, bCharCode);
      if (iGlyph != 0xFFFF) {
        iGlyph |= ((i + 1) << 24);
        if (ppFont != NULL) {
          *ppFont = pFont;
        }
        return iGlyph;
      }
    }
  }
  return 0xFFFF;
}
int32_t CFX_GdiFont::GetGlyphIndex(FX_WCHAR wUnicode, FX_BOOL bCharCode) {
  return GetGlyphIndex(wUnicode, TRUE, NULL, bCharCode);
}
int32_t CFX_GdiFont::GetAscent() const {
  return m_OutlineTM.otmAscent;
}
int32_t CFX_GdiFont::GetDescent() const {
  return m_OutlineTM.otmDescent;
}
FX_BOOL CFX_GdiFont::GetCharBBox(FX_WCHAR wUnicode,
                                 CFX_Rect& bbox,
                                 FX_BOOL bCharCode) {
  int32_t iGlyphIndex = GetGlyphIndex(wUnicode, bCharCode);
  if (iGlyphIndex == 0xFFFF) {
    return FALSE;
  }
  IFX_Font* pFont = GetSubstFont(iGlyphIndex);
  if (pFont == NULL) {
    return FALSE;
  }
  GLYPHMETRICS gm;
  iGlyphIndex &= 0x00FFFFFF;
  static const MAT2 mat2 = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
  if (::GetGlyphOutlineW(((CFX_GdiFont*)pFont)->m_hDC, iGlyphIndex,
                         GGO_GLYPH_INDEX | GGO_METRICS, &gm, 0, NULL,
                         &mat2) != GDI_ERROR) {
    bbox.left = gm.gmptGlyphOrigin.x;
    bbox.top = gm.gmptGlyphOrigin.y;
    bbox.width = gm.gmBlackBoxX;
    bbox.height = gm.gmBlackBoxY;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CFX_GdiFont::GetBBox(CFX_Rect& bbox) {
  bbox.left = m_OutlineTM.otmrcFontBox.left;
  bbox.top = m_OutlineTM.otmrcFontBox.top;
  bbox.width = m_OutlineTM.otmrcFontBox.right - m_OutlineTM.otmrcFontBox.left;
  bbox.height = m_OutlineTM.otmrcFontBox.bottom - m_OutlineTM.otmrcFontBox.top;
  return TRUE;
}
int32_t CFX_GdiFont::GetItalicAngle() const {
  return m_OutlineTM.otmItalicAngle / 10;
}
void CFX_GdiFont::Reset() {
  Lock();
  m_WidthCache.RemoveAll();
  ClearCache();
  Unlock();
}
IFX_Font* CFX_GdiFont::GetSubstFont(int32_t iGlyphIndex) const {
  int32_t iHigher = (iGlyphIndex & 0x7F000000) >> 24;
  if (iHigher == 0) {
    return (IFX_Font*)this;
  }
  if (iHigher > m_SubstFonts.GetSize()) {
    return (IFX_Font*)this;
  }
  return (IFX_Font*)m_SubstFonts[iHigher - 1];
}
FX_DWORD CFX_GdiFont::GetGlyphDIBits(int32_t iGlyphIndex,
                                     FX_ARGB argb,
                                     const MAT2* pMatrix,
                                     GLYPHMETRICS& gm,
                                     void* pBuffer,
                                     FX_DWORD bufSize) {
  static const UINT uFormat = GGO_GLYPH_INDEX | GGO_GRAY8_BITMAP;
  IFX_Font* pFont = GetSubstFont(iGlyphIndex);
  if (pFont == NULL) {
    return 0;
  }
  if (pFont != (IFX_Font*)this) {
    return ((CFX_GdiFont*)pFont)
        ->GetGlyphDIBits(iGlyphIndex & 0x00FFFFFF, argb, pMatrix, gm, pBuffer,
                         bufSize);
  }
  uint8_t* pGlyphOutline = NULL;
  FXSYS_assert(pMatrix != NULL);
  FX_DWORD dwMAT2 = GetMAT2HashCode((const FIXED*)pMatrix);
  CFX_GdiFontCache* pCache = NULL;
  if (m_FontCache.Lookup((void*)dwMAT2, (void*&)pCache) && pCache != NULL) {
    FX_LPCGDIGOCACHE pGO = pCache->GetCachedGlyphOutline(iGlyphIndex);
    if (pGO != NULL) {
      gm = pGO->gm;
      pGlyphOutline = pGO->pOutline;
    }
  }
  if (pGlyphOutline == NULL) {
    FX_DWORD dwGlyphSize =
        ::GetGlyphOutlineW(m_hDC, iGlyphIndex, uFormat, &gm, 0, NULL, pMatrix);
    if (dwGlyphSize == 0 || dwGlyphSize == GDI_ERROR) {
      return 0;
    }
    pGlyphOutline = FX_Alloc(uint8_t, dwGlyphSize);
    ::GetGlyphOutlineW(m_hDC, iGlyphIndex, uFormat, &gm, dwGlyphSize,
                       pGlyphOutline, pMatrix);
    if (pCache == NULL) {
      pCache = new CFX_GdiFontCache;
      if (m_FontCache.GetCount() >= FX_GDIFONT_FONTCACHESIZE) {
        ClearCache();
      }
      m_FontCache.SetAt((void*)dwMAT2, (void*)pCache);
    }
    pCache->SetCachedGlyphOutline(iGlyphIndex, gm, pGlyphOutline);
  }
  FX_DWORD dwDibSize = gm.gmBlackBoxX * 4 * gm.gmBlackBoxY;
  if (pBuffer == NULL || bufSize < dwDibSize) {
    return dwDibSize;
  }
  CreateGlyphBitmap(gm.gmBlackBoxX, gm.gmBlackBoxY, pGlyphOutline,
                    (FX_DWORD*)pBuffer, argb);
  return dwDibSize;
}
FX_DWORD CFX_GdiFont::GetHashCode() const {
  return FX_GetFontHashCode(FX_GetCodePageFromCharset(m_LogFont.lfCharSet),
                            FX_GetGdiFontStyles(m_LogFont));
}
FX_DWORD CFX_GdiFont::GetMAT2HashCode(const FIXED* pFixed) {
  FXSYS_assert(pFixed != NULL);
  FX_DWORD dwHash1 = 0, dwHash2 = 5381, dwRet;
  for (int i = 0; i < 4; i++) {
    dwRet = *((const FX_DWORD*)pFixed);
    dwHash1 = 1313 * dwHash1 + dwRet;
    dwHash2 += (dwHash2 << 5) + dwRet;
    pFixed++;
  }
  return ((dwHash1 & 0x0000FFFF) << 16) | (dwHash2 & 0x0000FFFF);
}
void CFX_GdiFont::CreateGlyphBitmap(int32_t iWidth,
                                    int32_t iHeight,
                                    uint8_t* pOutline,
                                    FX_DWORD* pDIB,
                                    FX_ARGB argb) {
  int32_t padding = ((iWidth + 3) / 4) * 4 - iWidth;
  FX_DWORD alpha;
  int32_t i, j;
  for (j = iHeight - 1; j >= 0; --j) {
    for (i = iWidth - 1; i >= 0; --i) {
      if ((alpha = *pOutline++) == 0) {
        *pDIB++ = 0;
      } else {
        alpha = (argb >> 24) * (alpha * 4 - 1) / 256;
        *pDIB++ = (alpha << 24) | (argb & 0x00FFFFFF);
      }
    }
    pOutline += padding;
  }
}
#endif
#endif
