// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../fgas_base.h"
#include "fx_stdfontmgr.h"
#include "fx_fontutils.h"
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
IFX_FontMgr* IFX_FontMgr::Create(FX_LPEnumAllFonts pEnumerator, FX_LPMatchFont pMatcher , FX_LPVOID pUserData )
{
    return FX_NEW CFX_StdFontMgrImp(pEnumerator, pMatcher, pUserData);
}
CFX_StdFontMgrImp::CFX_StdFontMgrImp(FX_LPEnumAllFonts pEnumerator, FX_LPMatchFont pMatcher, FX_LPVOID pUserData)
    : m_pMatcher(pMatcher)
    , m_pEnumerator(pEnumerator)
    , m_FontFaces()
    , m_Fonts()
    , m_CPFonts(8)
    , m_FamilyFonts(16)
    , m_UnicodeFonts(16)
    , m_BufferFonts(4)
    , m_FileFonts(4)
    , m_StreamFonts(4)
    , m_DeriveFonts(4)
    , m_pUserData(pUserData)
{
    if (m_pEnumerator != NULL) {
        m_pEnumerator(m_FontFaces, m_pUserData, NULL, 0xFEFF);
    }
    if (m_pMatcher == NULL) {
        m_pMatcher = FX_DefFontMatcher;
    }
    FXSYS_assert(m_pMatcher != NULL);
}
CFX_StdFontMgrImp::~CFX_StdFontMgrImp()
{
    m_FontFaces.RemoveAll();
    m_CPFonts.RemoveAll();
    m_FamilyFonts.RemoveAll();
    m_UnicodeFonts.RemoveAll();
    m_BufferFonts.RemoveAll();
    m_FileFonts.RemoveAll();
    m_StreamFonts.RemoveAll();
    m_DeriveFonts.RemoveAll();
    for (FX_INT32 i = m_Fonts.GetUpperBound(); i >= 0; i--) {
        IFX_Font *pFont = (IFX_Font*)m_Fonts[i];
        if (pFont != NULL) {
            pFont->Release();
        }
    }
    m_Fonts.RemoveAll();
}
IFX_Font* CFX_StdFontMgrImp::GetDefFontByCodePage(FX_WORD wCodePage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily)
{
    FX_DWORD dwHash = FGAS_GetFontHashCode(wCodePage, dwFontStyles);
    IFX_Font *pFont = NULL;
    if (m_CPFonts.Lookup((void*)(FX_UINTPTR)dwHash, (void*&)pFont)) {
        return pFont ? LoadFont(pFont, dwFontStyles, wCodePage) : NULL;
    }
    FX_LPCFONTDESCRIPTOR pFD;
    if ((pFD = FindFont(pszFontFamily, dwFontStyles, TRUE, wCodePage)) == NULL)
        if ((pFD = FindFont(NULL, dwFontStyles, TRUE, wCodePage)) == NULL)
            if ((pFD = FindFont(NULL, dwFontStyles, FALSE, wCodePage)) == NULL) {
                return NULL;
            }
    FXSYS_assert(pFD != NULL);
    pFont = IFX_Font::LoadFont(pFD->wsFontFace, dwFontStyles, wCodePage, this);
    if (pFont != NULL) {
        m_Fonts.Add(pFont);
        m_CPFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        dwHash = FGAS_GetFontFamilyHash(pFD->wsFontFace, dwFontStyles, wCodePage);
        m_FamilyFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        return LoadFont(pFont, dwFontStyles, wCodePage);
    }
    return NULL;
}
IFX_Font* CFX_StdFontMgrImp::GetDefFontByCharset(FX_BYTE nCharset, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily)
{
    return GetDefFontByCodePage(FX_GetCodePageFromCharset(nCharset), dwFontStyles, pszFontFamily);
}
#define _FX_USEGASFONTMGR_
IFX_Font* CFX_StdFontMgrImp::GetDefFontByUnicode(FX_WCHAR wUnicode, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily)
{
    FGAS_LPCFONTUSB pRet = FGAS_GetUnicodeBitField(wUnicode);
    if (pRet->wBitField == 999) {
        return NULL;
    }
    FX_DWORD dwHash = FGAS_GetFontFamilyHash(pszFontFamily, dwFontStyles, pRet->wBitField);
    IFX_Font *pFont = NULL;
    if (m_UnicodeFonts.Lookup((void*)(FX_UINTPTR)dwHash, (void*&)pFont)) {
        return pFont ? LoadFont(pFont, dwFontStyles, pRet->wCodePage) : NULL;
    }
#ifdef _FX_USEGASFONTMGR_
    FX_LPCFONTDESCRIPTOR pFD = FindFont(pszFontFamily, dwFontStyles, FALSE, pRet->wCodePage, pRet->wBitField, wUnicode);
    if (pFD == NULL && pszFontFamily) {
        pFD = FindFont(NULL, dwFontStyles, FALSE, pRet->wCodePage, pRet->wBitField, wUnicode);
    }
    if (pFD == NULL) {
        return NULL;
    }
    FXSYS_assert(pFD);
    FX_WORD wCodePage = FX_GetCodePageFromCharset(pFD->uCharSet);
    FX_LPCWSTR pFontFace = pFD->wsFontFace;
    pFont = IFX_Font::LoadFont(pFontFace, dwFontStyles, wCodePage, this);
#else
    CFX_FontMapper* pBuiltinMapper = CFX_GEModule::Get()->GetFontMgr()->m_pBuiltinMapper;
    if (pBuiltinMapper == NULL) {
        return NULL;
    }
    FX_INT32 iWeight = (dwFontStyles & FX_FONTSTYLE_Bold) ? FXFONT_FW_BOLD : FXFONT_FW_NORMAL;
    int italic_angle = 0;
    FXFT_Face ftFace = pBuiltinMapper->FindSubstFontByUnicode(wUnicode, dwFontStyles, iWeight, italic_angle);
    if (ftFace == NULL) {
        return NULL;
    }
    CFX_Font* pFXFont = FX_NEW CFX_Font;
    if (pFXFont == NULL) {
        return NULL;
    }
    pFXFont->m_Face = ftFace;
    pFXFont->m_pFontData = FXFT_Get_Face_Stream_Base(ftFace);
    pFXFont->m_dwSize = FXFT_Get_Face_Stream_Size(ftFace);
    pFont = IFX_Font::LoadFont(pFXFont, this);
    FX_WORD wCodePage = pRet->wCodePage;
    CFX_WideString wsPsName = pFXFont->GetPsName();
    FX_LPCWSTR pFontFace = wsPsName;
#endif
    if (pFont != NULL) {
        m_Fonts.Add(pFont);
        m_UnicodeFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        dwHash = FGAS_GetFontHashCode(wCodePage, dwFontStyles);
        m_CPFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        dwHash = FGAS_GetFontFamilyHash(pFontFace, dwFontStyles, wCodePage);
        m_FamilyFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        return LoadFont(pFont, dwFontStyles, wCodePage);
    }
    return NULL;
}
IFX_Font* CFX_StdFontMgrImp::GetDefFontByLanguage(FX_WORD wLanguage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily)
{
    return GetDefFontByCodePage(FX_GetDefCodePageByLanguage(wLanguage), dwFontStyles, pszFontFamily);
}
IFX_Font* CFX_StdFontMgrImp::LoadFont(FX_LPCWSTR pszFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage)
{
    FX_DWORD dwHash = FGAS_GetFontFamilyHash(pszFontFamily, dwFontStyles, wCodePage);
    IFX_Font *pFont = NULL;
    if (m_FamilyFonts.Lookup((void*)(FX_UINTPTR)dwHash, (void*&)pFont)) {
        return pFont ? LoadFont(pFont, dwFontStyles, wCodePage) : NULL;
    }
    FX_LPCFONTDESCRIPTOR pFD = NULL;
    if ((pFD = FindFont(pszFontFamily, dwFontStyles, TRUE, wCodePage)) == NULL)
        if ((pFD = FindFont(pszFontFamily, dwFontStyles, FALSE, wCodePage)) == NULL) {
            return NULL;
        }
    FXSYS_assert(pFD != NULL);
    if (wCodePage == 0xFFFF) {
        wCodePage = FX_GetCodePageFromCharset(pFD->uCharSet);
    }
    pFont = IFX_Font::LoadFont(pFD->wsFontFace, dwFontStyles, wCodePage, this);
    if (pFont != NULL) {
        m_Fonts.Add(pFont);
        m_FamilyFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        dwHash = FGAS_GetFontHashCode(wCodePage, dwFontStyles);
        m_CPFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        return LoadFont(pFont, dwFontStyles, wCodePage);
    }
    return NULL;
}
IFX_Font* CFX_StdFontMgrImp::LoadFont(FX_LPCBYTE pBuffer, FX_INT32 iLength)
{
    FXSYS_assert(pBuffer != NULL && iLength > 0);
    IFX_Font *pFont = NULL;
    if (m_BufferFonts.Lookup((void*)pBuffer, (void*&)pFont)) {
        if (pFont != NULL) {
            return pFont->Retain();
        }
    }
    pFont = IFX_Font::LoadFont(pBuffer, iLength, this);
    if (pFont != NULL) {
        m_Fonts.Add(pFont);
        m_BufferFonts.SetAt((void*)pBuffer, pFont);
        return pFont->Retain();
    }
    return NULL;
}
IFX_Font* CFX_StdFontMgrImp::LoadFont(FX_LPCWSTR pszFileName)
{
    FXSYS_assert(pszFileName != NULL);
    FX_DWORD dwHash = FX_HashCode_String_GetW(pszFileName, -1);
    IFX_Font *pFont = NULL;
    if (m_FileFonts.Lookup((void*)(FX_UINTPTR)dwHash, (void*&)pFont)) {
        if (pFont != NULL) {
            return pFont->Retain();
        }
    }
    pFont = IFX_Font::LoadFont(pszFileName, NULL);
    if (pFont != NULL) {
        m_Fonts.Add(pFont);
        m_FileFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        return pFont->Retain();
    }
    return NULL;
}
IFX_Font* CFX_StdFontMgrImp::LoadFont(IFX_Stream *pFontStream, FX_LPCWSTR pszFontAlias , FX_DWORD dwFontStyles , FX_WORD wCodePage , FX_BOOL bSaveStream )
{
    FXSYS_assert(pFontStream != NULL && pFontStream->GetLength() > 0);
    IFX_Font *pFont = NULL;
    if (m_StreamFonts.Lookup((void*)pFontStream, (void*&)pFont)) {
        if (pFont != NULL) {
            if (pszFontAlias != NULL) {
                FX_DWORD dwHash = FGAS_GetFontFamilyHash(pszFontAlias, dwFontStyles, wCodePage);
                m_FamilyFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
            }
            return LoadFont(pFont, dwFontStyles, wCodePage);
        }
    }
    pFont = IFX_Font::LoadFont(pFontStream, this, bSaveStream);
    if (pFont != NULL) {
        m_Fonts.Add(pFont);
        m_StreamFonts.SetAt((void*)pFontStream, (void*)pFont);
        if (pszFontAlias != NULL) {
            FX_DWORD dwHash = FGAS_GetFontFamilyHash(pszFontAlias, dwFontStyles, wCodePage);
            m_FamilyFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        }
        return LoadFont(pFont, dwFontStyles, wCodePage);
    }
    return NULL;
}
IFX_Font* CFX_StdFontMgrImp::LoadFont(IFX_Font *pSrcFont, FX_DWORD dwFontStyles, FX_WORD wCodePage)
{
    FXSYS_assert(pSrcFont != NULL);
    if (pSrcFont->GetFontStyles() == dwFontStyles) {
        return pSrcFont->Retain();
    }
    void* buffer[3] = {pSrcFont, (void*)(FX_UINTPTR)dwFontStyles, (void*)(FX_UINTPTR)wCodePage};
    FX_DWORD dwHash = FX_HashCode_String_GetA((FX_LPCSTR)buffer, 3 * sizeof(void*));
    IFX_Font *pFont = NULL;
    if (m_DeriveFonts.GetCount() > 0) {
        m_DeriveFonts.Lookup((void*)(FX_UINTPTR)dwHash, (void*&)pFont);
        if (pFont != NULL) {
            return pFont->Retain();
        }
    }
    pFont = pSrcFont->Derive(dwFontStyles, wCodePage);
    if (pFont != NULL) {
        m_DeriveFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
        FX_INT32 index = m_Fonts.Find(pFont);
        if (index < 0) {
            m_Fonts.Add(pFont);
            pFont->Retain();
        }
        return pFont;
    }
    return NULL;
}
void CFX_StdFontMgrImp::ClearFontCache()
{
    FX_INT32 iCount = m_Fonts.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        IFX_Font *pFont = (IFX_Font*)m_Fonts[i];
        if (pFont != NULL) {
            pFont->Reset();
        }
    }
}
void CFX_StdFontMgrImp::RemoveFont(CFX_MapPtrToPtr &fontMap, IFX_Font *pFont)
{
    FX_POSITION pos = fontMap.GetStartPosition();
    FX_LPVOID pKey;
    FX_LPVOID pFind;
    while (pos != NULL) {
        pFind = NULL;
        fontMap.GetNextAssoc(pos, pKey, pFind);
        if (pFind != (void*)pFont) {
            continue;
        }
        fontMap.RemoveKey(pKey);
        break;
    }
}
void CFX_StdFontMgrImp::RemoveFont(IFX_Font *pFont)
{
    RemoveFont(m_CPFonts, pFont);
    RemoveFont(m_FamilyFonts, pFont);
    RemoveFont(m_UnicodeFonts, pFont);
    RemoveFont(m_BufferFonts, pFont);
    RemoveFont(m_FileFonts, pFont);
    RemoveFont(m_StreamFonts, pFont);
    RemoveFont(m_DeriveFonts, pFont);
    FX_INT32 iFind = m_Fonts.Find(pFont);
    if (iFind > -1) {
        m_Fonts.RemoveAt(iFind, 1);
    }
}
FX_LPCFONTDESCRIPTOR CFX_StdFontMgrImp::FindFont(FX_LPCWSTR pszFontFamily, FX_DWORD dwFontStyles, FX_DWORD dwMatchFlags, FX_WORD wCodePage, FX_DWORD dwUSB , FX_WCHAR wUnicode )
{
    if (m_pMatcher == NULL) {
        return NULL;
    }
    FX_FONTMATCHPARAMS params;
    FX_memset(&params, 0, sizeof(params));
    params.dwUSB = dwUSB;
    params.wUnicode = wUnicode;
    params.wCodePage = wCodePage;
    params.pwsFamily = pszFontFamily;
    params.dwFontStyles = dwFontStyles;
    params.dwMatchFlags = dwMatchFlags;
    FX_LPCFONTDESCRIPTOR pDesc = m_pMatcher(&params, m_FontFaces, m_pUserData);
    if (pDesc) {
        return pDesc;
    }
    if (pszFontFamily && m_pEnumerator) {
        CFX_FontDescriptors namedFonts;
        m_pEnumerator(namedFonts, m_pUserData, pszFontFamily, wUnicode);
        params.pwsFamily = NULL;
        pDesc = m_pMatcher(&params, namedFonts, m_pUserData);
        if (pDesc == NULL) {
            return NULL;
        }
        for (FX_INT32 i = m_FontFaces.GetSize() - 1; i >= 0; i--) {
            FX_LPCFONTDESCRIPTOR pMatch = m_FontFaces.GetPtrAt(i);
            if (*pMatch == *pDesc) {
                return pMatch;
            }
        }
        int index = m_FontFaces.Add(*pDesc);
        return m_FontFaces.GetPtrAt(index);
    }
    return NULL;
}
FX_LPCFONTDESCRIPTOR FX_DefFontMatcher(FX_LPFONTMATCHPARAMS pParams, const CFX_FontDescriptors &fonts, FX_LPVOID pUserData)
{
    FX_LPCFONTDESCRIPTOR pBestFont = NULL;
    FX_INT32 iBestSimilar = 0;
    FX_BOOL bMatchStyle = (pParams->dwMatchFlags & FX_FONTMATCHPARA_MacthStyle) > 0;
    FX_INT32 iCount = fonts.GetSize();
    for (FX_INT32 i = 0; i < iCount; ++i) {
        FX_LPCFONTDESCRIPTOR pFont = fonts.GetPtrAt(i);
        if ((pFont->dwFontStyles & FX_FONTSTYLE_BoldItalic) == FX_FONTSTYLE_BoldItalic) {
            continue;
        }
        if (pParams->pwsFamily) {
            if (FXSYS_wcsicmp(pParams->pwsFamily, pFont->wsFontFace)) {
                continue;
            }
            if (pFont->uCharSet == FX_CHARSET_Symbol) {
                return pFont;
            }
        }
        if (pFont->uCharSet == FX_CHARSET_Symbol) {
            continue;
        }
        if (pParams->wCodePage != 0xFFFF) {
            if (FX_GetCodePageFromCharset(pFont->uCharSet) != pParams->wCodePage) {
                continue;
            }
        } else {
            if (pParams->dwUSB < 128) {
                FX_DWORD dwByte = pParams->dwUSB / 32;
                FX_DWORD dwUSB = 1 << (pParams->dwUSB % 32);
                if ((pFont->FontSignature.fsUsb[dwByte] & dwUSB) == 0) {
                    continue;
                }
            }
        }
        if (bMatchStyle) {
            if ((pFont->dwFontStyles & 0x0F) == (pParams->dwFontStyles & 0x0F)) {
                return pFont;
            } else {
                continue;
            }
        }
        if (pParams->pwsFamily != NULL) {
            if (FXSYS_wcsicmp(pParams->pwsFamily, pFont->wsFontFace) == 0) {
                return pFont;
            }
        }
        FX_INT32 iSimilarValue = FX_GetSimilarValue(pFont, pParams->dwFontStyles);
        if (iBestSimilar < iSimilarValue) {
            iBestSimilar = iSimilarValue;
            pBestFont = pFont;
        }
    }
    return iBestSimilar < 1 ? NULL : pBestFont;
}
FX_INT32 FX_GetSimilarValue(FX_LPCFONTDESCRIPTOR pFont, FX_DWORD dwFontStyles)
{
    FX_INT32 iValue = 0;
    if ((dwFontStyles & FX_FONTSTYLE_Symbolic) == (pFont->dwFontStyles & FX_FONTSTYLE_Symbolic)) {
        iValue += 64;
    }
    if ((dwFontStyles & FX_FONTSTYLE_FixedPitch) == (pFont->dwFontStyles & FX_FONTSTYLE_FixedPitch)) {
        iValue += 32;
    }
    if ((dwFontStyles & FX_FONTSTYLE_Serif) == (pFont->dwFontStyles & FX_FONTSTYLE_Serif)) {
        iValue += 16;
    }
    if ((dwFontStyles & FX_FONTSTYLE_Script) == (pFont->dwFontStyles & FX_FONTSTYLE_Script)) {
        iValue += 8;
    }
    return iValue;
}
FX_LPMatchFont FX_GetDefFontMatchor()
{
    return FX_DefFontMatcher;
}
FX_DWORD FX_GetGdiFontStyles(const LOGFONTW &lf)
{
    FX_DWORD dwStyles = 0;
    if ((lf.lfPitchAndFamily & 0x03) == FIXED_PITCH) {
        dwStyles |= FX_FONTSTYLE_FixedPitch;
    }
    FX_BYTE nFamilies = lf.lfPitchAndFamily & 0xF0;
    if (nFamilies == FF_ROMAN) {
        dwStyles |= FX_FONTSTYLE_Serif;
    }
    if (nFamilies == FF_SCRIPT) {
        dwStyles |= FX_FONTSTYLE_Script;
    }
    if (lf.lfCharSet == SYMBOL_CHARSET) {
        dwStyles |= FX_FONTSTYLE_Symbolic;
    }
    return dwStyles;
}
static FX_INT32 CALLBACK FX_GdiFontEnumProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD dwFontType, LPARAM lParam)
{
    if (dwFontType != TRUETYPE_FONTTYPE) {
        return 1;
    }
    const LOGFONTW &lf = ((LPENUMLOGFONTEXW)lpelfe)->elfLogFont;
    if (lf.lfFaceName[0] == L'@') {
        return 1;
    }
    FX_LPFONTDESCRIPTOR pFont = FX_Alloc(FX_FONTDESCRIPTOR, 1);
    if (NULL == pFont) {
        return 0;
    }
    FXSYS_memset(pFont, 0, sizeof(FX_FONTDESCRIPTOR));
    pFont->uCharSet = lf.lfCharSet;
    pFont->dwFontStyles = FX_GetGdiFontStyles(lf);
    FXSYS_wcsncpy(pFont->wsFontFace, (FX_LPCWSTR)lf.lfFaceName, 31);
    pFont->wsFontFace[31] = 0;
    FX_memcpy(&pFont->FontSignature, &lpntme->ntmFontSig, sizeof(lpntme->ntmFontSig));
    ((CFX_FontDescriptors*)lParam)->Add(*pFont);
    FX_Free(pFont);
    return 1;
}
static void FX_EnumGdiFonts(CFX_FontDescriptors &fonts, FX_LPVOID pUserData, FX_LPCWSTR pwsFaceName, FX_WCHAR wUnicode)
{
    HDC hDC = ::GetDC(NULL);
    LOGFONTW lfFind;
    FX_memset(&lfFind, 0, sizeof(lfFind));
    lfFind.lfCharSet = DEFAULT_CHARSET;
    if (pwsFaceName) {
        FXSYS_wcsncpy((FX_LPWSTR)lfFind.lfFaceName, pwsFaceName, 31);
        lfFind.lfFaceName[31] = 0;
    }
    EnumFontFamiliesExW(hDC, (LPLOGFONTW)&lfFind, (FONTENUMPROCW)FX_GdiFontEnumProc, (LPARAM)&fonts, 0);
    ::ReleaseDC(NULL, hDC);
}
FX_LPEnumAllFonts FX_GetDefFontEnumerator()
{
    return FX_EnumGdiFonts;
}
#else
FX_LPCSTR g_FontFolders[] = {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_
    (FX_LPCSTR)"/usr/share/fonts",
    (FX_LPCSTR)"/usr/share/X11/fonts/Type1",
    (FX_LPCSTR)"/usr/share/X11/fonts/TTF",
    (FX_LPCSTR)"/usr/local/share/fonts",
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    (FX_LPCSTR)"~/Library/Fonts",
    (FX_LPCSTR)"/Library/Fonts",
    (FX_LPCSTR)"/System/Library/Fonts",
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_ANDROID_
    (FX_LPCSTR)"/system/fonts",
#endif
};
CFX_FontSourceEnum_File::CFX_FontSourceEnum_File()
{
    for (FX_INT32 i = 0; i < sizeof(g_FontFolders) / sizeof(FX_LPCSTR); i++) {
        m_FolderPaths.Add(g_FontFolders[i]);
    }
}
CFX_ByteString CFX_FontSourceEnum_File::GetNextFile()
{
Restart:
    FX_LPVOID pCurHandle = m_FolderQueue.GetSize() == 0 ? NULL : m_FolderQueue.GetDataPtr(m_FolderQueue.GetSize() - 1)->pFileHandle;
    if (NULL == pCurHandle) {
        if (m_FolderPaths.GetSize() < 1) {
            return "";
        }
        pCurHandle = FX_OpenFolder(m_FolderPaths[m_FolderPaths.GetSize() - 1]);
        FX_HandleParentPath hpp;
        hpp.pFileHandle = pCurHandle;
        hpp.bsParentPath = m_FolderPaths[m_FolderPaths.GetSize() - 1];
        m_FolderQueue.Add(hpp);
    }
    CFX_ByteString bsName;
    FX_BOOL	bFolder;
    CFX_ByteString bsFolderSpearator = CFX_ByteString::FromUnicode(CFX_WideString(FX_GetFolderSeparator()));
    while (TRUE) {
        if (!FX_GetNextFile(pCurHandle, bsName, bFolder)) {
            FX_CloseFolder(pCurHandle);
            m_FolderQueue.RemoveAt(m_FolderQueue.GetSize() - 1);
            if (m_FolderQueue.GetSize() == 0) {
                m_FolderPaths.RemoveAt(m_FolderPaths.GetSize() - 1);
                if (m_FolderPaths.GetSize() == 0) {
                    return "";
                } else {
                    goto Restart;
                }
            }
            pCurHandle = m_FolderQueue.GetDataPtr(m_FolderQueue.GetSize() - 1)->pFileHandle;
            continue;
        }
        if (bsName == "." || bsName == "..") {
            continue;
        }
        if (bFolder) {
            FX_HandleParentPath hpp;
            hpp.bsParentPath = m_FolderQueue.GetDataPtr(m_FolderQueue.GetSize() - 1)->bsParentPath + bsFolderSpearator + bsName;
            hpp.pFileHandle = FX_OpenFolder(hpp.bsParentPath);
            if (hpp.pFileHandle == NULL) {
                continue;
            }
            m_FolderQueue.Add(hpp);
            pCurHandle = hpp.pFileHandle;
            continue;
        }
        bsName = m_FolderQueue.GetDataPtr(m_FolderQueue.GetSize() - 1)->bsParentPath + bsFolderSpearator + bsName;
        break;
    }
    return bsName;
}
FX_POSITION CFX_FontSourceEnum_File::GetStartPosition( FX_LPVOID pUserData  )
{
    m_wsNext = GetNextFile().UTF8Decode();
    if (0 == m_wsNext.GetLength()) {
        return (FX_POSITION)0;
    }
    return (FX_POSITION) - 1;
}
IFX_FileAccess* CFX_FontSourceEnum_File::GetNext( FX_POSITION& pos, FX_LPVOID pUserData  )
{
    IFX_FileAccess* pAccess = FX_CreateDefaultFileAccess(m_wsNext);
    m_wsNext = GetNextFile().UTF8Decode();
    pos = 0 != m_wsNext.GetLength() ? pAccess : NULL;
    return (IFX_FileAccess*)pAccess;
}
IFX_FontSourceEnum*	FX_CreateDefaultFontSourceEnum()
{
    return (IFX_FontSourceEnum*)FX_NEW CFX_FontSourceEnum_File;
}
IFX_FontMgr* IFX_FontMgr::Create(IFX_FontSourceEnum* pFontEnum, IFX_FontMgrDelegate* pDelegate, FX_LPVOID pUserData)
{
    if (NULL == pFontEnum) {
        return NULL;
    }
    CFX_FontMgrImp* pFontMgr = FX_NEW CFX_FontMgrImp(pFontEnum, pDelegate, pUserData);
    if (NULL == pFontMgr) {
        return NULL;
    }
    if (pFontMgr->EnumFonts()) {
        return pFontMgr;
    }
    delete pFontMgr;
    return NULL;
}
CFX_FontMgrImp::CFX_FontMgrImp( IFX_FontSourceEnum* pFontEnum, IFX_FontMgrDelegate* pDelegate , FX_LPVOID pUserData  )
    : m_pFontSource(pFontEnum)
    , m_pDelegate(pDelegate)
    , m_pUserData(pUserData)
{
}
FX_BOOL CFX_FontMgrImp::EnumFonts()
{
    FXFT_Library& library = CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary;
    if (library == NULL) {
        FXFT_Init_FreeType(&library);
    }
    if (library == NULL) {
        return FALSE;
    }
    FXFT_Face pFace = NULL;
    FX_POSITION pos = m_pFontSource->GetStartPosition();
    IFX_FileAccess* pFontSource = NULL;
    IFX_FileRead* pFontStream = NULL;
    while (pos) {
        pFontSource = m_pFontSource->GetNext(pos);
        pFontStream = pFontSource->CreateFileStream(FX_FILEMODE_ReadOnly);
        if (NULL == pFontStream) {
            pFontSource->Release();
            continue;
        }
        if (NULL == (pFace = LoadFace(pFontStream, 0))) {
            pFontStream->Release();
            pFontSource->Release();
            continue;
        }
        FX_INT32 nFaceCount = pFace->num_faces;
        ReportFace(pFace, m_InstalledFonts, pFontSource);
        if (FXFT_Get_Face_External_Stream(pFace)) {
            FXFT_Clear_Face_External_Stream(pFace);
        }
        FXFT_Done_Face(pFace);
        for (FX_INT32 i = 1; i < nFaceCount; i++) {
            if (NULL == (pFace = LoadFace(pFontStream, i))) {
                continue;
            }
            ReportFace(pFace, m_InstalledFonts, pFontSource);
            if (FXFT_Get_Face_External_Stream(pFace)) {
                FXFT_Clear_Face_External_Stream(pFace);
            }
            FXFT_Done_Face(pFace);
        }
        pFontStream->Release();
        pFontSource->Release();
    }
    return TRUE;
}
void CFX_FontMgrImp::Release()
{
    for (FX_INT32 i = 0; i < m_InstalledFonts.GetSize(); i++) {
        delete m_InstalledFonts[i];
    }
    FX_POSITION pos = m_Hash2CandidateList.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        CFX_FontDescriptorInfos* pDescs;
        m_Hash2CandidateList.GetNextAssoc(pos, dwHash, pDescs);
        if (NULL != pDescs) {
            delete pDescs;
        }
    }
    pos = m_Hash2Fonts.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        CFX_ArrayTemplate<IFX_Font*>* pFonts;
        m_Hash2Fonts.GetNextAssoc(pos, dwHash, pFonts);
        if (NULL != pFonts) {
            delete pFonts;
        }
    }
    m_Hash2Fonts.RemoveAll();
    pos = m_Hash2FileAccess.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        IFX_FileAccess* pFileAccess;
        m_Hash2FileAccess.GetNextAssoc(pos, dwHash, pFileAccess);
        if (NULL != pFileAccess) {
            pFileAccess->Release();
        }
    }
    pos = m_FileAccess2IFXFont.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        IFX_Font* pFont;
        m_FileAccess2IFXFont.GetNextAssoc(pos, dwHash, pFont);
        if (NULL != pFont) {
            pFont->Release();
        }
    }
    pos = m_IFXFont2FileRead.GetStartPosition();
    while (pos) {
        IFX_Font* pFont;
        IFX_FileRead* pFileRead;
        m_IFXFont2FileRead.GetNextAssoc(pos, pFont, pFileRead);
        pFileRead->Release();
    }
    delete this;
}
IFX_Font* CFX_FontMgrImp::GetDefFontByCodePage( FX_WORD wCodePage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    return NULL == m_pDelegate ? NULL : m_pDelegate->GetDefFontByCodePage(this, wCodePage, dwFontStyles, pszFontFamily);
}
IFX_Font* CFX_FontMgrImp::GetDefFontByCharset( FX_BYTE nCharset, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    return NULL == m_pDelegate ? NULL : m_pDelegate->GetDefFontByCharset(this, nCharset, dwFontStyles, pszFontFamily);
}
IFX_Font* CFX_FontMgrImp::GetDefFontByUnicode( FX_WCHAR wUnicode, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    return NULL == m_pDelegate ? NULL : m_pDelegate->GetDefFontByUnicode(this, wUnicode, dwFontStyles, pszFontFamily);
}
IFX_Font* CFX_FontMgrImp::GetDefFontByLanguage( FX_WORD wLanguage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    return NULL == m_pDelegate ? NULL : m_pDelegate->GetDefFontByLanguage(this, wLanguage, dwFontStyles, pszFontFamily);
}
IFX_Font* CFX_FontMgrImp::GetFontByCodePage( FX_WORD wCodePage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    CFX_ByteString bsHash;
    bsHash.Format("%d, %d", wCodePage, dwFontStyles);
    bsHash += CFX_WideString(pszFontFamily).UTF8Encode();
    FX_DWORD dwHash = FX_HashCode_String_GetA(bsHash, bsHash.GetLength());
    CFX_ArrayTemplate<IFX_Font*>* pFonts = NULL;
    IFX_Font* pFont = NULL;
    if (m_Hash2Fonts.Lookup(dwHash, pFonts)) {
        if (NULL == pFonts) {
            return NULL;
        }
        if (0 != pFonts->GetSize()) {
            return pFonts->GetAt(0)->Retain();
        }
    }
    if (NULL == pFonts) {
        pFonts = FX_NEW CFX_ArrayTemplate<IFX_Font*>;
    }
    if (NULL == pFonts) {
        return NULL;
    }
    m_Hash2Fonts.SetAt(dwHash, pFonts);
    CFX_FontDescriptorInfos* sortedFonts = NULL;
    if (!m_Hash2CandidateList.Lookup(dwHash, sortedFonts)) {
        sortedFonts = FX_NEW CFX_FontDescriptorInfos;
        MatchFonts(*sortedFonts, wCodePage, dwFontStyles, CFX_WideString(pszFontFamily), 0);
        m_Hash2CandidateList.SetAt(dwHash, sortedFonts);
    }
    if (sortedFonts->GetSize() == 0) {
        return NULL;
    }
    CFX_FontDescriptor* pDesc = sortedFonts->GetAt(0).pFont;
    pFont = LoadFont(pDesc->m_pFileAccess, pDesc->m_nFaceIndex, NULL);
    if (NULL != pFont) {
        pFont->SetLogicalFontStyle(dwFontStyles);
    }
    pFonts->Add(pFont);
    return pFont;
}
IFX_Font* CFX_FontMgrImp::GetFontByCharset( FX_BYTE nCharset, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    return GetFontByCodePage(FX_GetCodePageFromCharset(nCharset), dwFontStyles, pszFontFamily);
}
IFX_Font* CFX_FontMgrImp::GetFontByUnicode( FX_WCHAR wUnicode, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    IFX_Font* pFont = NULL;
    if (m_FailedUnicodes2NULL.Lookup(wUnicode, pFont)) {
        return NULL;
    }
    FGAS_LPCFONTUSB x = FGAS_GetUnicodeBitField(wUnicode);
    FX_WORD wCodePage = NULL == x ? 0xFFFF : x->wCodePage;
    FX_WORD wBitField = NULL == x ? 999 : x->wBitField;
    CFX_ByteString bsHash;
    if (wCodePage == 0xFFFF) {
        bsHash.Format("%d, %d, %d", wCodePage, wBitField, dwFontStyles);
    } else {
        bsHash.Format("%d, %d", wCodePage, dwFontStyles);
    }
    bsHash += CFX_WideString(pszFontFamily).UTF8Encode();
    FX_DWORD dwHash = FX_HashCode_String_GetA(bsHash, bsHash.GetLength());
    CFX_ArrayTemplate<IFX_Font*>* pFonts = NULL;
    if (m_Hash2Fonts.Lookup(dwHash, pFonts)) {
        if (NULL == pFonts) {
            return NULL;
        }
        if (0 != pFonts->GetSize()) {
            for (FX_INT32 i = 0; i < pFonts->GetSize(); i++) {
                if (VerifyUnicode(pFonts->GetAt(i), wUnicode)) {
                    return pFonts->GetAt(i)->Retain();
                }
            }
        }
    }
    if (NULL == pFonts) {
        pFonts = FX_NEW CFX_ArrayTemplate<IFX_Font*>;
    }
    if (NULL == pFonts) {
        return NULL;
    }
    m_Hash2Fonts.SetAt(dwHash, pFonts);
    CFX_FontDescriptorInfos* sortedFonts = NULL;
    if (!m_Hash2CandidateList.Lookup(dwHash, sortedFonts)) {
        sortedFonts = FX_NEW CFX_FontDescriptorInfos;
        MatchFonts(*sortedFonts, wCodePage, dwFontStyles, CFX_WideString(pszFontFamily), wUnicode);
        m_Hash2CandidateList.SetAt(dwHash, sortedFonts);
    }
    for (FX_INT32 i = 0; i < sortedFonts->GetSize(); i++) {
        CFX_FontDescriptor* pDesc = sortedFonts->GetAt(i).pFont;
        if (VerifyUnicode(pDesc, wUnicode)) {
            pFont = LoadFont(pDesc->m_pFileAccess, pDesc->m_nFaceIndex, NULL);
            if (NULL != pFont) {
                pFont->SetLogicalFontStyle(dwFontStyles);
            }
            pFonts->Add(pFont);
            return pFont;
        }
    }
    if (NULL == pszFontFamily) {
        m_FailedUnicodes2NULL.SetAt(wUnicode, NULL);
    }
    return NULL;
}
FX_BOOL CFX_FontMgrImp::VerifyUnicode( CFX_FontDescriptor* pDesc, FX_WCHAR wcUnicode )
{
    IFX_FileRead* pFileRead = pDesc->m_pFileAccess->CreateFileStream(FX_FILEMODE_ReadOnly);
    if (NULL == pFileRead) {
        return FALSE;
    }
    FXFT_Face pFace = LoadFace(pFileRead, pDesc->m_nFaceIndex);
    if (NULL == pFace) {
        goto BadRet;
    }
    if (0 != FXFT_Select_Charmap(pFace, FXFT_ENCODING_UNICODE)) {
        goto BadRet;
    }
    if (0 == FXFT_Get_Char_Index(pFace, wcUnicode)) {
        goto BadRet;
    }
    pFileRead->Release();
    if (FXFT_Get_Face_External_Stream(pFace)) {
        FXFT_Clear_Face_External_Stream(pFace);
    }
    FXFT_Done_Face(pFace);
    return TRUE;
BadRet:
    if (NULL != pFileRead) {
        pFileRead->Release();
    }
    if (NULL != pFace) {
        if (FXFT_Get_Face_External_Stream(pFace)) {
            FXFT_Clear_Face_External_Stream(pFace);
        }
        FXFT_Done_Face(pFace);
    }
    return FALSE;
}
FX_BOOL CFX_FontMgrImp::VerifyUnicode( IFX_Font* pFont, FX_WCHAR wcUnicode )
{
    if (NULL == pFont) {
        return FALSE;
    }
    FXFT_Face pFace = ((CFX_Font*)pFont->GetDevFont())->m_Face;
    FXFT_CharMap charmap = FXFT_Get_Face_Charmap(pFace);
    if (0 != FXFT_Select_Charmap(pFace, FXFT_ENCODING_UNICODE)) {
        return FALSE;
    }
    if (0 == FXFT_Get_Char_Index(pFace, wcUnicode)) {
        FXFT_Set_Charmap(pFace, charmap);
        return FALSE;
    }
    return TRUE;
}
IFX_Font* CFX_FontMgrImp::GetFontByLanguage( FX_WORD wLanguage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily  )
{
    return GetFontByCodePage(FX_GetDefCodePageByLanguage(wLanguage), dwFontStyles, pszFontFamily);
}
IFX_Font* CFX_FontMgrImp::LoadFont( FX_LPCBYTE pBuffer, FX_INT32 iLength, FX_INT32 iFaceIndex, FX_INT32* pFaceCount )
{
    FX_LPVOID Hash[2] = {(FX_LPVOID)(FX_UINTPTR)pBuffer, (FX_LPVOID)(FX_UINTPTR)iLength};
    FX_DWORD dwHash = FX_HashCode_String_GetA((FX_LPCSTR)Hash, 2 * sizeof (FX_LPVOID));
    IFX_FileAccess* pFontAccess = NULL;
    if (!m_Hash2FileAccess.Lookup(dwHash, pFontAccess)) {
    }
    if (NULL != pFontAccess) {
        return LoadFont(pFontAccess, iFaceIndex, pFaceCount, TRUE);
    } else {
        return NULL;
    }
}
IFX_Font* CFX_FontMgrImp::LoadFont( FX_LPCWSTR pszFileName, FX_INT32 iFaceIndex, FX_INT32* pFaceCount )
{
    CFX_ByteString bsHash;
    bsHash += CFX_WideString(pszFileName).UTF8Encode();
    FX_DWORD dwHash = FX_HashCode_String_GetA((FX_LPCSTR)bsHash, bsHash.GetLength());
    IFX_FileAccess* pFontAccess = NULL;
    if (!m_Hash2FileAccess.Lookup(dwHash, pFontAccess)) {
        pFontAccess = FX_CreateDefaultFileAccess(pszFileName);
        m_Hash2FileAccess.SetAt(dwHash, pFontAccess);
    }
    if (NULL != pFontAccess) {
        return LoadFont(pFontAccess, iFaceIndex, pFaceCount, TRUE);
    } else {
        return NULL;
    }
}
IFX_Font* CFX_FontMgrImp::LoadFont( IFX_Stream* pFontStream, FX_INT32 iFaceIndex, FX_INT32* pFaceCount, FX_BOOL bSaveStream  )
{
    FX_LPVOID Hash[1] = {(FX_LPVOID)(FX_UINTPTR)pFontStream};
    FX_DWORD dwHash = FX_HashCode_String_GetA((FX_LPCSTR)Hash, 1 * sizeof (FX_LPVOID));
    IFX_FileAccess* pFontAccess = NULL;
    if (!m_Hash2FileAccess.Lookup(dwHash, pFontAccess)) {
    }
    if (NULL != pFontAccess) {
        return LoadFont(pFontAccess, iFaceIndex, pFaceCount, TRUE);
    } else {
        return NULL;
    }
}
IFX_Font* CFX_FontMgrImp::LoadFont( IFX_FileAccess* pFontAccess, FX_INT32 iFaceIndex, FX_INT32* pFaceCount, FX_BOOL bWantCache )
{
    FX_DWORD dwHash = 0;
    IFX_Font* pFont = NULL;
    if (bWantCache) {
        CFX_ByteString bsHash;
        bsHash.Format("%d, %d", (FX_UINTPTR)pFontAccess, iFaceIndex);
        dwHash = FX_HashCode_String_GetA(bsHash, bsHash.GetLength());
        if (m_FileAccess2IFXFont.Lookup(dwHash, pFont)) {
            if (NULL != pFont) {
                if (NULL != pFaceCount) {
                    *pFaceCount = ((CFX_Font*)pFont->GetDevFont())->m_Face->num_faces;
                }
                return pFont->Retain();
            }
        }
    }
    CFX_Font* pInternalFont = FX_NEW CFX_Font;
    if (NULL == pInternalFont) {
        return NULL;
    }
    IFX_FileRead* pFontStream = pFontAccess->CreateFileStream(FX_FILEMODE_ReadOnly);
    if (NULL == pFontStream) {
        delete pInternalFont;
        return NULL;
    }
    if (!pInternalFont->LoadFile(pFontStream, iFaceIndex)) {
        delete pInternalFont;
        pFontStream->Release();
        return NULL;
    }
    pFont = IFX_Font::LoadFont(pInternalFont, this, TRUE);
    if (NULL == pFont) {
        delete pInternalFont;
        pFontStream->Release();
        return NULL;
    }
    if (bWantCache) {
        m_FileAccess2IFXFont.SetAt(dwHash, pFont);
    }
    m_IFXFont2FileRead.SetAt(pFont, pFontStream);
    if (NULL != pFaceCount) {
        *pFaceCount = ((CFX_Font*)pFont->GetDevFont())->m_Face->num_faces;
    }
    return pFont;
}
extern "C"
{
    unsigned long _ftStreamRead(FXFT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
    {
        if (count == 0) {
            return 0;
        }
        IFX_FileRead* pFile = (IFX_FileRead*)stream->descriptor.pointer;
        int res = pFile->ReadBlock(buffer, offset, count);
        if (res) {
            return count;
        }
        return 0;
    }
    void _ftStreamClose(FXFT_Stream stream)
    {
    }
};
FXFT_Face CFX_FontMgrImp::LoadFace( IFX_FileRead* pFontStream, FX_INT32 iFaceIndex )
{
    FXFT_Library& library = CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary;
    FXFT_Open_Args ftArgs;
    FXFT_Stream ftStream = FX_Alloc(FXFT_StreamRec, 1);
    FXFT_Face pFace = NULL;
    if (library == NULL) {
        FXFT_Init_FreeType(&library);
    }
    if (library == NULL) {
        goto BadRet;
    }
    FXSYS_memset(&ftArgs, 0, sizeof(FXFT_Open_Args));
    if (NULL == ftStream) {
        goto BadRet;
    }
    FXSYS_memset(ftStream, 0, sizeof(FXFT_StreamRec));
    if (NULL == pFontStream) {
        goto BadRet;
    }
    ftStream->base = NULL;
    ftStream->descriptor.pointer = pFontStream;
    ftStream->pos = 0;
    ftStream->size = (unsigned long)pFontStream->GetSize();
    ftStream->read = _ftStreamRead;
    ftStream->close = _ftStreamClose;
    ftArgs.flags |= FT_OPEN_STREAM;
    ftArgs.stream = ftStream;
    if (0 != FXFT_Open_Face(library, &ftArgs, iFaceIndex, &pFace)) {
        goto BadRet;
    }
    FXFT_Set_Pixel_Sizes(pFace, 0, 64);
    return pFace;
BadRet:
    if (NULL != ftStream) {
        FX_Free(ftStream);
    }
    return NULL;
}
FX_INT32 CFX_FontMgrImp::MatchFonts( CFX_FontDescriptorInfos& MatchedFonts, FX_WORD wCodePage, FX_DWORD dwFontStyles, const CFX_WideString& FontName, FX_WCHAR wcUnicode )
{
    MatchedFonts.RemoveAll();
    CFX_WideString wsNormalizedFontName = FontName;
    NormalizeFontName(wsNormalizedFontName);
    static const FX_INT32 nMax = 0xffff;
    CFX_FontDescriptor* pFont = NULL;
    FX_INT32 nCount = m_InstalledFonts.GetSize();
    for (FX_INT32 i = 0; i < nCount; i++) {
        pFont = m_InstalledFonts[i];
        FX_INT32 nPenalty = CalcPenalty(pFont, wCodePage, dwFontStyles, wsNormalizedFontName, wcUnicode);
        if (nPenalty >= 0xFFFF) {
            continue;
        }
        FX_FontDescriptorInfo FontInfo;
        FontInfo.pFont = pFont;
        FontInfo.nPenalty = nPenalty;
        MatchedFonts.Add(FontInfo);
        if (MatchedFonts.GetSize() == nMax) {
            break;
        }
    }
    if (MatchedFonts.GetSize() == 0) {
        return 0;
    }
    CFX_SSortTemplate<FX_FontDescriptorInfo> ssort;
    ssort.ShellSort(MatchedFonts.GetData(), MatchedFonts.GetSize());
    return MatchedFonts.GetSize();
}
struct FX_BitCodePage {
    FX_WORD wBit;
    FX_WORD wCodePage;
};
static const FX_BitCodePage g_Bit2CodePage[] = {
    {0, 1252},
    {1, 1250},
    {2, 1251},
    {3, 1253},
    {4, 1254},
    {5, 1255},
    {6, 1256},
    {7, 1257},
    {8, 1258},
    {9, 0},
    {10, 0},
    {11, 0},
    {12, 0},
    {13, 0},
    {14, 0},
    {15, 0},
    {16, 874},
    {17, 932},
    {18, 936},
    {19, 949},
    {20, 950},
    {21, 1361},
    {22, 0},
    {23, 0},
    {24, 0},
    {25, 0},
    {26, 0},
    {27, 0},
    {28, 0},
    {29, 0},
    {30, 0},
    {31, 0},
    {32, 0},
    {33, 0},
    {34, 0},
    {35, 0},
    {36, 0},
    {37, 0},
    {38, 0},
    {39, 0},
    {40, 0},
    {41, 0},
    {42, 0},
    {43, 0},
    {44, 0},
    {45, 0},
    {46, 0},
    {47, 0},
    {48, 869},
    {49, 866},
    {50, 865},
    {51, 864},
    {52, 863},
    {53, 862},
    {54, 861},
    {55, 860},
    {56, 857},
    {57, 855},
    {58, 852},
    {59, 775},
    {60, 737},
    {61, 708},
    {62, 850},
    {63, 437},
};
FX_WORD FX_GetCodePageBit(FX_WORD wCodePage)
{
    for (FX_INT32 i = 0; i < sizeof(g_Bit2CodePage) / sizeof (FX_BitCodePage); i++) {
        if (g_Bit2CodePage[i].wCodePage == wCodePage) {
            return g_Bit2CodePage[i].wBit;
        }
    }
    return (FX_WORD) - 1;
}
FX_WORD FX_GetUnicodeBit(FX_WCHAR wcUnicode)
{
    FGAS_LPCFONTUSB x = FGAS_GetUnicodeBitField(wcUnicode);
    if (NULL == x) {
        return 999;
    }
    return x->wBitField;
}
FX_INT32 CFX_FontMgrImp::CalcPenalty( CFX_FontDescriptor* pInstalled, FX_WORD wCodePage, FX_DWORD dwFontStyles, const CFX_WideString& FontName, FX_WCHAR wcUnicode )
{
    FX_INT32 nPenalty = 30000;
    if (0 != FontName.GetLength()) {
        if (FontName != pInstalled->m_wsFaceName) {
            FX_INT32 i;
            for (i = 0; i < pInstalled->m_wsFamilyNames.GetSize(); i++) {
                if (pInstalled->m_wsFamilyNames[i] == FontName) {
                    break;
                }
            }
            if (i == pInstalled->m_wsFamilyNames.GetSize()) {
                nPenalty += 0xFFFF;
            } else {
                nPenalty -= 28000;
            }
        } else {
            nPenalty -= 30000;
        }
        if (30000 == nPenalty && 0 == IsPartName(pInstalled->m_wsFaceName, FontName)) {
            FX_INT32 i;
            for (i = 0; i < pInstalled->m_wsFamilyNames.GetSize(); i++) {
                if (0 != IsPartName(pInstalled->m_wsFamilyNames[i], FontName)) {
                    break;
                }
            }
            if (i == pInstalled->m_wsFamilyNames.GetSize()) {
                nPenalty += 0xFFFF;
            } else {
                nPenalty -= 26000;
            }
        } else {
            nPenalty -= 27000;
        }
    }
    FX_DWORD dwStyleMask = pInstalled->m_dwFontStyles ^ dwFontStyles;
    if (dwStyleMask & FX_FONTSTYLE_Bold) {
        nPenalty += 4500;
    }
    if (dwStyleMask & FX_FONTSTYLE_FixedPitch) {
        nPenalty += 10000;
    }
    if (dwStyleMask & FX_FONTSTYLE_Italic) {
        nPenalty += 10000;
    }
    if (dwStyleMask & FX_FONTSTYLE_Serif) {
        nPenalty += 500;
    }
    if (dwStyleMask & FX_FONTSTYLE_Symbolic) {
        nPenalty += 0xFFFF;
    }
    if (nPenalty >= 0xFFFF) {
        return 0xFFFF;
    }
    FX_WORD wBit = ((0 == wCodePage || 0xFFFF == wCodePage) ? (FX_WORD) - 1 : FX_GetCodePageBit(wCodePage));
    if (wBit != (FX_WORD) - 1) {
        FXSYS_assert(wBit < 64);
        if (0 == (pInstalled->m_dwCsb[wBit / 32] & (1 << (wBit % 32)))) {
            nPenalty += 0xFFFF;
        } else {
            nPenalty -= 60000;
        }
    }
    wBit = ((0 == wcUnicode || 0xFFFE == wcUnicode) ? (FX_WORD)999 : FX_GetUnicodeBit(wcUnicode));
    if (wBit != (FX_WORD)999) {
        FXSYS_assert(wBit < 128);
        if (0 == (pInstalled->m_dwUsb[wBit / 32] & (1 << (wBit % 32)))) {
            nPenalty += 0xFFFF;
        } else {
            nPenalty -= 60000;
        }
    }
    return nPenalty;
}
void CFX_FontMgrImp::ClearFontCache()
{
    FX_POSITION pos = m_Hash2CandidateList.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        CFX_FontDescriptorInfos* pDescs;
        m_Hash2CandidateList.GetNextAssoc(pos, dwHash, pDescs);
        if (NULL != pDescs) {
            delete pDescs;
        }
    }
    pos = m_FileAccess2IFXFont.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        IFX_Font* pFont;
        m_FileAccess2IFXFont.GetNextAssoc(pos, dwHash, pFont);
        if (NULL != pFont) {
            pFont->Release();
        }
    }
    pos = m_IFXFont2FileRead.GetStartPosition();
    while (pos) {
        IFX_Font* pFont;
        IFX_FileRead* pFileRead;
        m_IFXFont2FileRead.GetNextAssoc(pos, pFont, pFileRead);
        pFileRead->Release();
    }
}
void CFX_FontMgrImp::RemoveFont( IFX_Font* pEFont )
{
    if (NULL == pEFont) {
        return ;
    }
    IFX_FileRead* pFileRead;
    if (m_IFXFont2FileRead.Lookup(pEFont, pFileRead)) {
        pFileRead->Release();
        m_IFXFont2FileRead.RemoveKey(pEFont);
    }
    FX_POSITION pos;
    pos = m_FileAccess2IFXFont.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        IFX_Font* pCFont;
        m_FileAccess2IFXFont.GetNextAssoc(pos, dwHash, pCFont);
        if (pCFont == pEFont) {
            m_FileAccess2IFXFont.RemoveKey(dwHash);
            break;
        }
    }
    pos = m_Hash2Fonts.GetStartPosition();
    while (pos) {
        FX_DWORD dwHash;
        CFX_ArrayTemplate<IFX_Font*>* pFonts;
        m_Hash2Fonts.GetNextAssoc(pos, dwHash, pFonts);
        if (NULL != pFonts) {
            for (FX_INT32 i = 0; i < pFonts->GetSize(); i++) {
                if (pFonts->GetAt(i) == pEFont) {
                    pFonts->SetAt(i, NULL);
                }
            }
        } else {
            m_Hash2Fonts.RemoveKey(dwHash);
        }
    }
}
void CFX_FontMgrImp::ReportFace( FXFT_Face pFace, CFX_FontDescriptors& Fonts, IFX_FileAccess* pFontAccess)
{
    if (0 == (pFace->face_flags & FT_FACE_FLAG_SCALABLE)) {
        return ;
    }
    CFX_FontDescriptor* pFont = FX_NEW CFX_FontDescriptor;
    if (NULL == pFont) {
        return;
    }
    pFont->m_dwFontStyles |= FXFT_Is_Face_Bold(pFace) ? FX_FONTSTYLE_Bold : 0;
    pFont->m_dwFontStyles |= FXFT_Is_Face_Italic(pFace) ? FX_FONTSTYLE_Italic : 0;
    pFont->m_dwFontStyles |= GetFlags(pFace);
    CFX_WordArray Charsets;
    GetCharsets(pFace, Charsets);
    GetUSBCSB(pFace, pFont->m_dwUsb, pFont->m_dwCsb);
    unsigned long nLength = 0;
    FT_ULong dwTag;
    FX_LPBYTE pTable = NULL;
    FT_ENC_TAG(dwTag, 'n', 'a', 'm', 'e');
    unsigned int error = FXFT_Load_Sfnt_Table(pFace, dwTag, 0, NULL, &nLength);
    if (0 == error && 0 != nLength) {
        pTable = FX_Alloc(FX_BYTE, nLength);
        if (NULL != pTable) {
            error = FXFT_Load_Sfnt_Table(pFace, dwTag, 0, pTable, NULL);
            if (0 != error) {
                FX_Free(pTable);
                pTable = NULL;
            }
        }
    }
    GetNames(pTable, pFont->m_wsFamilyNames);
    if (NULL != pTable) {
        FX_Free(pTable);
    }
    pFont->m_wsFamilyNames.Add(CFX_ByteString(pFace->family_name).UTF8Decode());
    pFont->m_wsFaceName = CFX_WideString::FromLocal(FXFT_Get_Postscript_Name(pFace));
    pFont->m_nFaceIndex = pFace->face_index;
    pFont->m_pFileAccess = pFontAccess->Retain();
    NormalizeFontName(pFont->m_wsFaceName);
    for (FX_INT32 i = 0; i < pFont->m_wsFamilyNames.GetSize(); i++) {
        NormalizeFontName(pFont->m_wsFamilyNames[i]);
    }
    Fonts.Add(pFont);
}
FX_DWORD CFX_FontMgrImp::GetFlags(FXFT_Face pFace)
{
    FX_DWORD flag = 0;
    if (FT_IS_FIXED_WIDTH(pFace)) {
        flag |= FX_FONTSTYLE_FixedPitch;
    }
    TT_OS2* pOS2 = (TT_OS2*)FT_Get_Sfnt_Table(pFace, ft_sfnt_os2);
    if (!pOS2) {
        return flag;
    }
    if (pOS2->ulCodePageRange1 & (1 << 31)) {
        flag |= FX_FONTSTYLE_Symbolic;
    }
    if (pOS2->panose[0] == 2) {
        FX_BYTE uSerif = pOS2->panose[1];
        if ((uSerif > 1 && uSerif < 10) || uSerif > 13) {
            flag |= FX_FONTSTYLE_Serif;
        }
    }
    return flag;
}
#define GetUInt8(p) ((FX_UINT8)((p)[0]))
#define GetUInt16(p) ((FX_UINT16)((p)[0] << 8 | (p)[1]))
#define GetUInt32(p) ((FX_UINT32)((p)[0] << 24 | (p)[1] << 16 | (p)[2] << 8 | (p)[3]))
void CFX_FontMgrImp::GetNames(FX_LPCBYTE name_table, CFX_WideStringArray& Names)
{
    if (NULL == name_table) {
        return;
    }
    FX_LPBYTE lpTable = (FX_LPBYTE)name_table;
    CFX_WideString wsFamily;
    FX_LPBYTE sp = lpTable + 2;
    FX_LPBYTE lpNameRecord = lpTable + 6;
    FX_UINT16 nNameCount = GetUInt16(sp);
    FX_LPBYTE lpStr = lpTable + GetUInt16(sp + 2);
    for (FX_UINT16 j = 0; j < nNameCount; j++) {
        FX_UINT16 nNameID = GetUInt16(lpNameRecord + j * 12 + 6);
        if (nNameID != 1) {
            continue;
        }
        FX_UINT16 nPlatformID = GetUInt16(lpNameRecord + j * 12 + 0);
        FX_UINT16 nNameLength = GetUInt16(lpNameRecord + j * 12 + 8);
        FX_UINT16 nNameOffset = GetUInt16(lpNameRecord + j * 12 + 10);
        wsFamily.Empty();
        if (nPlatformID != 1) {
            for (FX_UINT16 k = 0; k < nNameLength / 2; k++) {
                FX_WCHAR wcTemp = GetUInt16(lpStr + nNameOffset + k * 2);
                wsFamily += wcTemp;
            }
            Names.Add(wsFamily);
        } else {
            for (FX_UINT16 k = 0; k < nNameLength; k++) {
                FX_WCHAR wcTemp = GetUInt8(lpStr + nNameOffset + k);
                wsFamily += wcTemp;
            }
            Names.Add(wsFamily);
        }
    }
}
#undef GetUInt8
#undef GetUInt16
#undef GetUInt32
struct FX_BIT2CHARSET {
    FX_WORD wBit;
    FX_WORD wCharset;
};
FX_BIT2CHARSET g_FX_Bit2Charset1[16] = {
    {1 << 0 , FX_CHARSET_ANSI},
    {1 << 1 , FX_CHARSET_MSWin_EasterEuropean},
    {1 << 2 , FX_CHARSET_MSWin_Cyrillic},
    {1 << 3 , FX_CHARSET_MSWin_Greek},
    {1 << 4 , FX_CHARSET_MSWin_Turkish},
    {1 << 5 , FX_CHARSET_MSWin_Hebrew},
    {1 << 6 , FX_CHARSET_MSWin_Arabic},
    {1 << 7 , FX_CHARSET_MSWin_Baltic},
    {1 << 8 , FX_CHARSET_MSWin_Vietnamese},
    {1 << 9 , FX_CHARSET_Default},
    {1 << 10 , FX_CHARSET_Default},
    {1 << 11 , FX_CHARSET_Default},
    {1 << 12 , FX_CHARSET_Default},
    {1 << 13 , FX_CHARSET_Default},
    {1 << 14 , FX_CHARSET_Default},
    {1 << 15 , FX_CHARSET_Default},
};
FX_BIT2CHARSET g_FX_Bit2Charset2[16] = {
    {1 << 0 , FX_CHARSET_Thai},
    {1 << 1 , FX_CHARSET_ShiftJIS},
    {1 << 2 , FX_CHARSET_ChineseSimplified},
    {1 << 3 , FX_CHARSET_Korean},
    {1 << 4 , FX_CHARSET_ChineseTriditional},
    {1 << 5 , FX_CHARSET_Johab},
    {1 << 6 , FX_CHARSET_Default},
    {1 << 7 , FX_CHARSET_Default},
    {1 << 8 , FX_CHARSET_Default},
    {1 << 9 , FX_CHARSET_Default},
    {1 << 10 , FX_CHARSET_Default},
    {1 << 11 , FX_CHARSET_Default},
    {1 << 12 , FX_CHARSET_Default},
    {1 << 13 , FX_CHARSET_Default},
    {1 << 14 , FX_CHARSET_OEM},
    {1 << 15 , FX_CHARSET_Symbol},
};
FX_BIT2CHARSET g_FX_Bit2Charset3[16] = {
    {1 << 0 , FX_CHARSET_Default},
    {1 << 1 , FX_CHARSET_Default},
    {1 << 2 , FX_CHARSET_Default},
    {1 << 3 , FX_CHARSET_Default},
    {1 << 4 , FX_CHARSET_Default},
    {1 << 5 , FX_CHARSET_Default},
    {1 << 6 , FX_CHARSET_Default},
    {1 << 7 , FX_CHARSET_Default},
    {1 << 8 , FX_CHARSET_Default},
    {1 << 9 , FX_CHARSET_Default},
    {1 << 10 , FX_CHARSET_Default},
    {1 << 11 , FX_CHARSET_Default},
    {1 << 12 , FX_CHARSET_Default},
    {1 << 13 , FX_CHARSET_Default},
    {1 << 14 , FX_CHARSET_Default},
    {1 << 15 , FX_CHARSET_Default},
};
FX_BIT2CHARSET g_FX_Bit2Charset4[16] = {
    {1 << 0 , FX_CHARSET_Default },
    {1 << 1 , FX_CHARSET_Default },
    {1 << 2 , FX_CHARSET_Default },
    {1 << 3 , FX_CHARSET_Default },
    {1 << 4 , FX_CHARSET_Default },
    {1 << 5 , FX_CHARSET_Default },
    {1 << 6 , FX_CHARSET_Default },
    {1 << 7 , FX_CHARSET_Default },
    {1 << 8 , FX_CHARSET_Default },
    {1 << 9 , FX_CHARSET_Default },
    {1 << 10 , FX_CHARSET_Default },
    {1 << 11 , FX_CHARSET_Default },
    {1 << 12 , FX_CHARSET_Default },
    {1 << 13 , FX_CHARSET_Default },
    {1 << 14 , FX_CHARSET_Default },
    {1 << 15 , FX_CHARSET_US },
};
#define CODEPAGERANGE_IMPLEMENT(n) \
    for (FX_INT32 i = 0; i < 16; i++)\
    {\
        if ((a##n & g_FX_Bit2Charset##n[i].wBit) != 0)\
        {\
            Charsets.Add(g_FX_Bit2Charset##n[i].wCharset);\
        }\
    }
void CFX_FontMgrImp::GetCharsets( FXFT_Face pFace, CFX_WordArray& Charsets )
{
    Charsets.RemoveAll();
    TT_OS2* pOS2 = (TT_OS2*)FT_Get_Sfnt_Table(pFace, ft_sfnt_os2);
    if (NULL != pOS2) {
        FX_WORD a1, a2, a3, a4;
        a1 = pOS2->ulCodePageRange1 & 0x0000ffff;
        CODEPAGERANGE_IMPLEMENT(1);
        a2 = (pOS2->ulCodePageRange1 >> 16) & 0x0000ffff;
        CODEPAGERANGE_IMPLEMENT(2);
        a3 = pOS2->ulCodePageRange2 & 0x0000ffff;
        CODEPAGERANGE_IMPLEMENT(3);
        a4 = (pOS2->ulCodePageRange2 >> 16) & 0x0000ffff;
        CODEPAGERANGE_IMPLEMENT(4);
    } else {
        Charsets.Add(FX_CHARSET_Default);
    }
}
#undef CODEPAGERANGE_IMPLEMENT
void CFX_FontMgrImp::GetUSBCSB( FXFT_Face pFace, FX_DWORD* USB, FX_DWORD* CSB )
{
    TT_OS2* pOS2 = (TT_OS2*)FT_Get_Sfnt_Table(pFace, ft_sfnt_os2);
    if (NULL != pOS2) {
        USB[0] = pOS2->ulUnicodeRange1;
        USB[1] = pOS2->ulUnicodeRange2;
        USB[2] = pOS2->ulUnicodeRange3;
        USB[3] = pOS2->ulUnicodeRange4;
        CSB[0] = pOS2->ulCodePageRange1;
        CSB[1] = pOS2->ulCodePageRange2;
    } else {
        USB[0] = 0;
        USB[1] = 0;
        USB[2] = 0;
        USB[3] = 0;
        CSB[0] = 0;
        CSB[1] = 0;
    }
}
void CFX_FontMgrImp::NormalizeFontName( CFX_WideString& FontName )
{
    FontName.MakeLower();
    FontName.Remove(' ');
    FontName.Remove('-');
}
FX_INT32 CFX_FontMgrImp::IsPartName( const CFX_WideString& Name1, const CFX_WideString& Name2 )
{
    if (Name1.Find((FX_LPCWSTR)Name2) != -1) {
        return 1;
    }
    return 0;
}
#endif
