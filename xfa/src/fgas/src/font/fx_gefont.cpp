// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../fgas_base.h"
#include "fx_gefont.h"
#include "fx_fontutils.h"
#ifndef _FXPLUS
IFX_Font* IFX_Font::LoadFont(FX_LPCWSTR pszFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage, IFX_FontMgr *pFontMgr)
{
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    if (NULL != pFontMgr) {
        return pFontMgr->GetFontByCodePage(wCodePage, dwFontStyles, pszFontFamily);
    }
    return NULL;
#else
    CFX_GEFont *pFont = FX_NEW CFX_GEFont(pFontMgr);
    if (!pFont->LoadFont(pszFontFamily, dwFontStyles, wCodePage)) {
        pFont->Release();
        return NULL;
    }
    return pFont;
#endif
}
IFX_Font* IFX_Font::LoadFont(FX_LPCBYTE pBuffer, FX_INT32 iLength, IFX_FontMgr *pFontMgr)
{
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    if (NULL != pFontMgr) {
        return pFontMgr->LoadFont(pBuffer, iLength, 0, NULL);
    }
    return NULL;
#else
    CFX_GEFont *pFont = FX_NEW CFX_GEFont(pFontMgr);
    if (!pFont->LoadFont(pBuffer, iLength)) {
        pFont->Release();
        return NULL;
    }
    return pFont;
#endif
}
IFX_Font* IFX_Font::LoadFont(FX_LPCWSTR pszFileName, IFX_FontMgr *pFontMgr)
{
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    if (NULL != pFontMgr) {
        return pFontMgr->LoadFont(pszFileName, 0, NULL);
    }
    return NULL;
#else
    CFX_GEFont *pFont = FX_NEW CFX_GEFont(pFontMgr);
    if (!pFont->LoadFont(pszFileName)) {
        pFont->Release();
        return NULL;
    }
    return pFont;
#endif
}
IFX_Font* IFX_Font::LoadFont(IFX_Stream *pFontStream, IFX_FontMgr *pFontMgr, FX_BOOL bSaveStream)
{
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    if (NULL != pFontMgr) {
        return pFontMgr->LoadFont(pFontStream, 0, NULL);
    }
    return NULL;
#else
    CFX_GEFont *pFont = FX_NEW CFX_GEFont(pFontMgr);
    if (!pFont->LoadFont(pFontStream, bSaveStream)) {
        pFont->Release();
        return NULL;
    }
    return pFont;
#endif
}
IFX_Font* IFX_Font::LoadFont(CFX_Font *pExtFont, IFX_FontMgr *pFontMgr, FX_BOOL bTakeOver)
{
    CFX_GEFont *pFont = FX_NEW CFX_GEFont(pFontMgr);
    if (!pFont->LoadFont(pExtFont, bTakeOver)) {
        pFont->Release();
        return NULL;
    }
    return pFont;
}
CFX_GEFont::CFX_GEFont(IFX_FontMgr *pFontMgr)
    : CFX_ThreadLock()
    , m_pFont(NULL)
    , m_pFontMgr(pFontMgr)
    , m_iRefCount(1)
    , m_bExtFont(FALSE)
    , m_pStream(NULL)
    , m_pFileRead(NULL)
    , m_pFontEncoding(NULL)
    , m_pCharWidthMap(NULL)
    , m_pRectArray(NULL)
    , m_pBBoxMap(NULL)
    , m_wCharSet(0xFFFF)
    , m_pProvider(NULL)
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    , m_bUseLogFontStyle(FALSE)
    , m_dwLogFontStyle(0)
#endif
    , m_SubstFonts()
    , m_FontMapper(16)
{
}
CFX_GEFont::CFX_GEFont(const CFX_GEFont &src, FX_DWORD dwFontStyles)
    : CFX_ThreadLock()
    , m_pFont(NULL)
    , m_pFontMgr(src.m_pFontMgr)
    , m_iRefCount(1)
    , m_bExtFont(FALSE)
    , m_pStream(NULL)
    , m_pFileRead(NULL)
    , m_pFontEncoding(NULL)
    , m_pCharWidthMap(NULL)
    , m_pRectArray(NULL)
    , m_pBBoxMap(NULL)
    , m_wCharSet(0xFFFF)
    , m_pProvider(NULL)
    , m_SubstFonts()
    , m_FontMapper(16)
{
    m_pFont = FX_NEW CFX_Font;
    FXSYS_assert(m_pFont != NULL);
    FXSYS_assert(src.m_pFont != NULL);
    m_pFont->LoadClone(src.m_pFont);
    CFX_SubstFont *&pSubst = m_pFont->m_pSubstFont;
    if (pSubst == NULL) {
        pSubst = FX_NEW CFX_SubstFont;
    }
    if (pSubst) {
        pSubst->m_Weight = (dwFontStyles & FX_FONTSTYLE_Bold) ? FXFONT_FW_BOLD : FXFONT_FW_NORMAL;
        if (dwFontStyles & FX_FONTSTYLE_Italic) {
            pSubst->m_SubstFlags |= FXFONT_SUBST_ITALIC;
        }
    }
    InitFont();
}
CFX_GEFont::~CFX_GEFont()
{
    FX_INT32 iCount = m_SubstFonts.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        IFX_Font *pFont = (IFX_Font*)m_SubstFonts[i];
        pFont->Release();
    }
    m_SubstFonts.RemoveAll();
    m_FontMapper.RemoveAll();
    if (m_pFileRead != NULL) {
        m_pFileRead->Release();
    }
    if (m_pStream != NULL) {
        m_pStream->Release();
    }
    if (m_pFontEncoding != NULL) {
        delete m_pFontEncoding;
    }
    if (m_pCharWidthMap != NULL) {
        FXTARGET_Delete m_pCharWidthMap;
    }
    if (m_pRectArray != NULL) {
        FXTARGET_Delete m_pRectArray;
    }
    if (m_pBBoxMap != NULL) {
        delete m_pBBoxMap;
    }
    if (m_pFont != NULL && !m_bExtFont) {
        delete m_pFont;
    }
}
void CFX_GEFont::Release()
{
    if (-- m_iRefCount < 1) {
        if (m_pFontMgr != NULL) {
            m_pFontMgr->RemoveFont(this);
        }
        delete this;
    }
}
IFX_Font* CFX_GEFont::Retain()
{
    ++ m_iRefCount;
    return this;
}
FX_BOOL CFX_GEFont::LoadFont(FX_LPCWSTR pszFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage)
{
    if (m_pFont != NULL) {
        return FALSE;
    }
    Lock();
    CFX_ByteString csFontFamily;
    if (pszFontFamily != NULL) {
        csFontFamily = CFX_ByteString::FromUnicode(pszFontFamily);
    }
    FX_DWORD dwFlags = 0;
    if (dwFontStyles & FX_FONTSTYLE_FixedPitch) {
        dwFlags |= FXFONT_FIXED_PITCH;
    }
    if (dwFontStyles & FX_FONTSTYLE_Serif) {
        dwFlags |= FXFONT_SERIF;
    }
    if (dwFontStyles & FX_FONTSTYLE_Symbolic) {
        dwFlags |= FXFONT_SYMBOLIC;
    }
    if (dwFontStyles & FX_FONTSTYLE_Script) {
        dwFlags |= FXFONT_SCRIPT;
    }
    if (dwFontStyles & FX_FONTSTYLE_Italic) {
        dwFlags |= FXFONT_ITALIC;
    }
    if (dwFontStyles & FX_FONTSTYLE_Bold) {
        dwFlags |= FXFONT_BOLD;
    }
    if (dwFontStyles & FX_FONTSTYLE_ExactMatch) {
        dwFlags |= FXFONT_EXACTMATCH;
    }
    FX_INT32 iWeight = (dwFontStyles & FX_FONTSTYLE_Bold) ? FXFONT_FW_BOLD : FXFONT_FW_NORMAL;
    FX_WORD wCharSet = FX_GetCharsetFromCodePage(wCodePage);
    if (wCharSet == 0xFFFF) {
        wCharSet = FXSYS_GetACP();
    }
    m_wCharSet = wCharSet;
    m_pFont = FX_NEW CFX_Font;
    if ((dwFlags & FXFONT_ITALIC) && (dwFlags & FXFONT_BOLD)) {
        csFontFamily += ",BoldItalic";
    } else if (dwFlags & FXFONT_BOLD) {
        csFontFamily += ",Bold";
    } else if (dwFlags & FXFONT_ITALIC) {
        csFontFamily += ",Italic";
    }
    FX_BOOL bRet = m_pFont->LoadSubst(csFontFamily, TRUE, dwFlags, iWeight, 0, wCodePage);
    bRet = m_pFont->m_Face != NULL;
    if (bRet) {
        InitFont();
    }
    Unlock();
    return bRet;
}
FX_BOOL CFX_GEFont::LoadFont(FX_LPCBYTE pBuffer, FX_INT32 length)
{
    if (m_pFont != NULL) {
        return FALSE;
    }
    Lock();
    m_pFont = FX_NEW CFX_Font;
    FX_BOOL bRet = m_pFont->LoadEmbedded(pBuffer, length);
    if (bRet) {
        InitFont();
    }
    m_wCharSet = 0xFFFF;
    Unlock();
    return bRet;
}
FX_BOOL CFX_GEFont::LoadFont(FX_LPCWSTR pszFileName)
{
    if (m_pFont != NULL) {
        return FALSE;
    }
    if (m_pStream != NULL) {
        return FALSE;
    }
    if (m_pFileRead != NULL) {
        return FALSE;
    }
    Lock();
    m_pStream = IFX_Stream::CreateStream(pszFileName, FX_STREAMACCESS_Binary | FX_STREAMACCESS_Read);
    m_pFileRead = FX_CreateFileRead(m_pStream);
    FX_BOOL bRet = FALSE;
    if (m_pStream != NULL && m_pFileRead != NULL) {
        m_pFont = FX_NEW CFX_Font;
        bRet = m_pFont->LoadFile(m_pFileRead);
        if (!bRet) {
            m_pFileRead->Release();
            m_pFileRead = NULL;
        } else {
            InitFont();
        }
    }
    m_wCharSet = 0xFFFF;
    Unlock();
    return bRet;
}
FX_BOOL CFX_GEFont::LoadFont(IFX_Stream *pFontStream, FX_BOOL bSaveStream)
{
    if (m_pFont != NULL) {
        return FALSE;
    }
    if (pFontStream == NULL || pFontStream->GetLength() < 1) {
        return FALSE;
    }
    if (m_pFileRead != NULL) {
        return FALSE;
    }
    Lock();
    if (bSaveStream) {
        m_pStream = pFontStream;
    }
    m_pFileRead = FX_CreateFileRead(pFontStream);
    m_pFont = FX_NEW CFX_Font;
    FX_BOOL bRet = m_pFont->LoadFile(m_pFileRead);
    if (!bRet) {
        m_pFileRead->Release();
        m_pFileRead = NULL;
    } else {
        InitFont();
    }
    m_wCharSet = 0xFFFF;
    Unlock();
    return bRet;
}
FX_BOOL CFX_GEFont::LoadFont(CFX_Font *pExtFont, FX_BOOL bTakeOver)
{
    if (m_pFont != NULL) {
        return FALSE;
    }
    if (pExtFont == NULL) {
        return FALSE;
    }
    Lock();
    if ((m_pFont = pExtFont) != NULL) {
        m_bExtFont = !bTakeOver;
        InitFont();
    } else {
        m_bExtFont = TRUE;
    }
    m_wCharSet = 0xFFFF;
    Unlock();
    return m_pFont != NULL;
}
void CFX_GEFont::InitFont()
{
    if (m_pFont == NULL) {
        return;
    }
    if (m_pFontEncoding == NULL) {
#ifdef FXFM_ENCODING_NONE
        m_pFontEncoding = FX_CreateFontEncodingEx(m_pFont);
#else
        m_pFontEncoding = FXGE_CreateUnicodeEncoding(m_pFont);
#endif
    }
    if (m_pCharWidthMap == NULL) {
        m_pCharWidthMap = FXTARGET_New CFX_WordDiscreteArray(1024);
    }
    if (m_pRectArray == NULL) {
        m_pRectArray = FXTARGET_New CFX_RectMassArray(16);
    }
    if (m_pBBoxMap == NULL) {
        m_pBBoxMap = FX_NEW CFX_MapPtrToPtr(16);
    }
}
IFX_Font* CFX_GEFont::Derive(FX_DWORD dwFontStyles, FX_WORD wCodePage )
{
    if (GetFontStyles() == dwFontStyles) {
        return Retain();
    }
    return FX_NEW CFX_GEFont(*this, dwFontStyles);
}
FX_BYTE CFX_GEFont::GetCharSet() const
{
    FXSYS_assert(m_pFont != NULL);
    if (m_wCharSet != 0xFFFF) {
        return (FX_BYTE)m_wCharSet;
    }
    if (m_pFont->m_pSubstFont == NULL) {
        return FX_CHARSET_Default;
    }
    return m_pFont->m_pSubstFont->m_Charset;
}
void CFX_GEFont::GetFamilyName(CFX_WideString &wsFamily) const
{
    FXSYS_assert(m_pFont != NULL);
    if (m_pFont->m_pSubstFont == NULL || m_pFont->m_pSubstFont->m_Family.GetLength() == 0) {
        wsFamily = CFX_WideString::FromLocal(m_pFont->GetFamilyName());
    } else {
        wsFamily = CFX_WideString::FromLocal(m_pFont->m_pSubstFont->m_Family);
    }
}
void CFX_GEFont::GetPsName(CFX_WideString &wsName) const
{
    wsName = m_pFont->GetPsName();
}
FX_DWORD CFX_GEFont::GetFontStyles() const
{
    FXSYS_assert(m_pFont != NULL);
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    if (m_bUseLogFontStyle) {
        return m_dwLogFontStyle;
    }
#endif
    FX_DWORD dwStyles = 0;
    if (m_pFont->m_pSubstFont == NULL) {
        if (m_pFont->IsBold()) {
            dwStyles |= FX_FONTSTYLE_Bold;
        }
        if (m_pFont->IsItalic()) {
            dwStyles |= FX_FONTSTYLE_Italic;
        }
    } else {
        if (m_pFont->m_pSubstFont->m_Weight == FXFONT_FW_BOLD) {
            dwStyles |= FX_FONTSTYLE_Bold;
        }
        if (m_pFont->m_pSubstFont->m_SubstFlags & FXFONT_SUBST_ITALIC) {
            dwStyles |= FX_FONTSTYLE_Italic;
        }
    }
    return dwStyles;
}
FX_BOOL CFX_GEFont::GetCharWidth(FX_WCHAR wUnicode, FX_INT32 &iWidth, FX_BOOL bCharCode)
{
    return GetCharWidth(wUnicode, iWidth, TRUE, bCharCode);
}
FX_BOOL CFX_GEFont::GetCharWidth(FX_WCHAR wUnicode, FX_INT32 &iWidth, FX_BOOL bRecursive, FX_BOOL bCharCode)
{
    FXSYS_assert (m_pCharWidthMap != NULL);
    iWidth = m_pCharWidthMap->GetAt(wUnicode, 0);
    if (iWidth < 1) {
        if (!m_pProvider || !m_pProvider->GetCharWidth(this, wUnicode, iWidth, bCharCode)) {
            IFX_Font *pFont = NULL;
            FX_INT32 iGlyph = GetGlyphIndex(wUnicode, TRUE, &pFont, bCharCode);
            if (iGlyph != 0xFFFF && pFont != NULL) {
                if (pFont == (IFX_Font*)this) {
                    iWidth = m_pFont->GetGlyphWidth(iGlyph);
                    if (iWidth < 0) {
                        iWidth = -1;
                    }
                } else if (((CFX_GEFont*)pFont)->GetCharWidth(wUnicode, iWidth, FALSE, bCharCode)) {
                    return TRUE;
                }
            } else {
                iWidth = -1;
            }
        }
        Lock();
        m_pCharWidthMap->SetAtGrow(wUnicode, (FX_SHORT)iWidth);
        Unlock();
    } else if (iWidth == 65535) {
        iWidth = -1;
    }
    return iWidth > 0;
}
FX_BOOL CFX_GEFont::GetCharBBox(FX_WCHAR wUnicode, CFX_Rect &bbox, FX_BOOL bCharCode)
{
    return GetCharBBox(wUnicode, bbox, TRUE, bCharCode);
}
FX_BOOL CFX_GEFont::GetCharBBox(FX_WCHAR wUnicode, CFX_Rect &bbox, FX_BOOL bRecursive, FX_BOOL bCharCode)
{
    FXSYS_assert(m_pRectArray != NULL);
    FXSYS_assert(m_pBBoxMap != NULL);
    FX_LPVOID pRect = NULL;
    if (!m_pBBoxMap->Lookup((void*)(FX_UINTPTR)wUnicode, pRect)) {
        IFX_Font *pFont = NULL;
        FX_INT32 iGlyph = GetGlyphIndex(wUnicode, TRUE, &pFont, bCharCode);
        if (iGlyph != 0xFFFF && pFont != NULL) {
            if (pFont == (IFX_Font*)this) {
                FX_RECT rtBBox;
                if (m_pFont->GetGlyphBBox(iGlyph, rtBBox)) {
                    Lock();
                    CFX_Rect rt;
                    rt.Set(rtBBox.left, rtBBox.top, rtBBox.Width(), rtBBox.Height());
                    FX_INT32 index = m_pRectArray->Add(rt);
                    pRect = m_pRectArray->GetPtrAt(index);
                    m_pBBoxMap->SetAt((void*)(FX_UINTPTR)wUnicode, pRect);
                    Unlock();
                }
            } else if (((CFX_GEFont*)pFont)->GetCharBBox(wUnicode, bbox, FALSE, bCharCode)) {
                return TRUE;
            }
        }
    }
    if (pRect == NULL) {
        return FALSE;
    }
    bbox = *(FX_LPCRECT)pRect;
    return TRUE;
}
FX_BOOL CFX_GEFont::GetBBox(CFX_Rect &bbox)
{
    FX_RECT rt(0, 0, 0, 0);
    FX_BOOL bRet = m_pFont->GetBBox(rt);
    if (bRet) {
        bbox.left = rt.left;
        bbox.width = rt.Width();
        bbox.top = rt.bottom;
        bbox.height = -rt.Height();
    }
    return bRet;
}
FX_INT32 CFX_GEFont::GetItalicAngle() const
{
    if (m_pFont->m_pSubstFont == NULL) {
        return 0;
    }
    return m_pFont->m_pSubstFont->m_ItalicAngle;
}
FX_INT32 CFX_GEFont::GetGlyphIndex(FX_WCHAR wUnicode, FX_BOOL bCharCode)
{
    return GetGlyphIndex(wUnicode, TRUE, NULL, bCharCode);
}
FX_INT32 CFX_GEFont::GetGlyphIndex(FX_WCHAR wUnicode, FX_BOOL bRecursive, IFX_Font **ppFont, FX_BOOL bCharCode)
{
    FXSYS_assert (m_pFontEncoding != NULL);
    FX_INT32 iGlyphIndex = m_pFontEncoding->GlyphFromCharCode(wUnicode);
    if (iGlyphIndex > 0) {
        if (ppFont != NULL) {
            *ppFont = (IFX_Font*)this;
        }
        return iGlyphIndex;
    }
    FGAS_LPCFONTUSB pFontUSB = FGAS_GetUnicodeBitField(wUnicode);
    if (pFontUSB == NULL) {
        return 0xFFFF;
    }
    FX_WORD wBitField = pFontUSB->wBitField;
    if (wBitField >= 128) {
        return 0xFFFF;
    }
    IFX_Font *pFont = NULL;
    m_FontMapper.Lookup((void*)(FX_UINTPTR)wUnicode, (void*&)pFont);
    if (pFont != NULL && pFont != (IFX_Font*)this) {
        iGlyphIndex = ((CFX_GEFont*)pFont)->GetGlyphIndex(wUnicode, FALSE, NULL, bCharCode);
        if (iGlyphIndex != 0xFFFF) {
            FX_INT32 i = m_SubstFonts.Find(pFont);
            if (i > -1) {
                iGlyphIndex |= ((i + 1) << 24);
                if (ppFont != NULL) {
                    *ppFont = pFont;
                }
                return iGlyphIndex;
            }
        }
    }
    if (m_pFontMgr != NULL && bRecursive) {
        CFX_WideString wsFamily;
        GetFamilyName(wsFamily);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
        IFX_Font *pFont = m_pFontMgr->GetDefFontByUnicode(wUnicode, GetFontStyles(), (FX_LPCWSTR)wsFamily);
#else
        IFX_Font *pFont = m_pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), (FX_LPCWSTR)wsFamily);
        if (NULL == pFont) {
            pFont = m_pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), NULL);
        }
#endif
        if (pFont != NULL) {
            if (pFont == (IFX_Font*)this) {
                pFont->Release();
                return 0xFFFF;
            }
            m_FontMapper.SetAt((void*)(FX_UINTPTR)wUnicode, (void*)pFont);
            FX_INT32 i = m_SubstFonts.GetSize();
            m_SubstFonts.Add(pFont);
            iGlyphIndex = ((CFX_GEFont*)pFont)->GetGlyphIndex(wUnicode, FALSE, NULL, bCharCode);
            if (iGlyphIndex != 0xFFFF) {
                iGlyphIndex |= ((i + 1) << 24);
                if (ppFont != NULL) {
                    *ppFont = pFont;
                }
                return iGlyphIndex;
            }
        }
    }
    return 0xFFFF;
}
FX_INT32 CFX_GEFont::GetAscent() const
{
    return m_pFont->GetAscent();
}
FX_INT32 CFX_GEFont::GetDescent() const
{
    return m_pFont->GetDescent();
}
void CFX_GEFont::Reset()
{
    Lock();
    FX_INT32 iCount = m_SubstFonts.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        IFX_Font *pFont = (IFX_Font*)m_SubstFonts[i];
        ((CFX_GEFont*)pFont)->Reset();
    }
    if (m_pCharWidthMap != NULL) {
        m_pCharWidthMap->RemoveAll();
    }
    if (m_pBBoxMap != NULL) {
        m_pBBoxMap->RemoveAll();
    }
    if (m_pRectArray != NULL) {
        m_pRectArray->RemoveAll();
    }
    Unlock();
}
IFX_Font* CFX_GEFont::GetSubstFont(FX_INT32 iGlyphIndex) const
{
    iGlyphIndex = ((FX_DWORD)iGlyphIndex) >> 24;
    return iGlyphIndex == 0 ? (IFX_Font*)this : (IFX_Font*)m_SubstFonts[iGlyphIndex - 1];
}
#endif
