// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fx_fpf.h"
#if _FX_OS_ == _FX_ANDROID_
#define FPF_SKIAMATCHWEIGHT_NAME1	62
#define FPF_SKIAMATCHWEIGHT_NAME2	60
#define FPF_SKIAMATCHWEIGHT_1		16
#define FPF_SKIAMATCHWEIGHT_2		8
#include "fpf_skiafontmgr.h"
#include "fpf_skiafont.h"
#ifdef __cplusplus
extern "C" {
#endif
static unsigned long FPF_SkiaStream_Read(FXFT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    IFX_FileRead *pFileRead = (IFX_FileRead*)stream->descriptor.pointer;
    if (!pFileRead) {
        return 0;
    }
    if (count > 0) {
        if (pFileRead->ReadBlock(buffer, (FX_FILESIZE)offset, (size_t)count) != count) {
            return 0;
        }
    }
    return count;
}
static void FPF_SkiaStream_Close(FXFT_Stream stream)
{
}
#ifdef __cplusplus
};
#endif
typedef struct _FPF_SKIAFONTMAP {
    FX_DWORD	dwFamily;
    FX_DWORD	dwSubSt;
} FPF_SKIAFONTMAP, *FPF_LPSKIAFONTMAP;
typedef FPF_SKIAFONTMAP const * FPF_LPCSKIAFONTMAP;
static const FPF_SKIAFONTMAP g_SkiaFontmap[] = {
    {0x58c5083,		0xc8d2e345},
    {0x5dfade2,		0xe1633081},
    {0x684317d,		0xe1633081},
    {0x14ee2d13,	0xc8d2e345},
    {0x3918fe2d,	0xbbeeec72},
    {0x3b98b31c,	0xe1633081},
    {0x3d49f40e,	0xe1633081},
    {0x432c41c5,	0xe1633081},
    {0x491b6ad0,	0xe1633081},
    {0x5612cab1,	0x59b9f8f1},
    {0x779ce19d,	0xc8d2e345},
    {0x7cc9510b,	0x59b9f8f1},
    {0x83746053,	0xbbeeec72},
    {0xaaa60c03,	0xbbeeec72},
    {0xbf85ff26,	0xe1633081},
    {0xc04fe601,	0xbbeeec72},
    {0xca3812d5,	0x59b9f8f1},
    {0xca383e15,	0x59b9f8f1},
    {0xcad5eaf6,	0x59b9f8f1},
    {0xcb7a04c8,	0xc8d2e345},
    {0xfb4ce0de,	0xe1633081},
};
FX_DWORD FPF_SkiaGetSubstFont(FX_DWORD dwHash)
{
    FX_INT32 iStart = 0;
    FX_INT32 iEnd = sizeof(g_SkiaFontmap) / sizeof(FPF_SKIAFONTMAP);
    while (iStart <= iEnd) {
        FX_INT32 iMid = (iStart + iEnd) / 2;
        FPF_LPCSKIAFONTMAP pItem = &g_SkiaFontmap[iMid];
        if (dwHash < pItem->dwFamily) {
            iEnd = iMid - 1;
        } else if (dwHash > pItem->dwFamily) {
            iStart = iMid + 1;
        } else {
            return pItem->dwSubSt;
        }
    }
    return 0;
}
static const FPF_SKIAFONTMAP g_SkiaSansFontMap[] = {
    {0x58c5083,		0xd5b8d10f},
    {0x14ee2d13,	0xd5b8d10f},
    {0x779ce19d,	0xd5b8d10f},
    {0xcb7a04c8,	0xd5b8d10f},
    {0xfb4ce0de,	0xd5b8d10f},
};
FX_DWORD FPF_SkiaGetSansFont(FX_DWORD dwHash)
{
    FX_INT32 iStart = 0;
    FX_INT32 iEnd = sizeof(g_SkiaSansFontMap) / sizeof(FPF_SKIAFONTMAP);
    while (iStart <= iEnd) {
        FX_INT32 iMid = (iStart + iEnd) / 2;
        FPF_LPCSKIAFONTMAP pItem = &g_SkiaSansFontMap[iMid];
        if (dwHash < pItem->dwFamily) {
            iEnd = iMid - 1;
        } else if (dwHash > pItem->dwFamily) {
            iStart = iMid + 1;
        } else {
            return pItem->dwSubSt;
        }
    }
    return 0;
}
static FX_UINT32 FPF_GetHashCode_StringA(FX_LPCSTR pStr, FX_INT32 iLength, FX_BOOL bIgnoreCase = FALSE)
{
    if (!pStr) {
        return 0;
    }
    if (iLength < 0) {
        iLength = FXSYS_strlen(pStr);
    }
    FX_LPCSTR pStrEnd = pStr + iLength;
    FX_UINT32 uHashCode = 0;
    if (bIgnoreCase) {
        while (pStr < pStrEnd) {
            uHashCode = 31 * uHashCode + FXSYS_tolower(*pStr++);
        }
    } else {
        while (pStr < pStrEnd) {
            uHashCode = 31 * uHashCode + *pStr ++;
        }
    }
    return uHashCode;
}
enum FPF_SKIACHARSET {
    FPF_SKIACHARSET_Ansi			= 1 << 0,
    FPF_SKIACHARSET_Default			= 1 << 1,
    FPF_SKIACHARSET_Symbol			= 1 << 2,
    FPF_SKIACHARSET_ShiftJIS		= 1 << 3,
    FPF_SKIACHARSET_Korean			= 1 << 4,
    FPF_SKIACHARSET_Johab			= 1 << 5,
    FPF_SKIACHARSET_GB2312			= 1 << 6,
    FPF_SKIACHARSET_BIG5			= 1 << 7,
    FPF_SKIACHARSET_Greek			= 1 << 8,
    FPF_SKIACHARSET_Turkish			= 1 << 9,
    FPF_SKIACHARSET_Vietnamese		= 1 << 10,
    FPF_SKIACHARSET_Hebrew			= 1 << 11,
    FPF_SKIACHARSET_Arabic			= 1 << 12,
    FPF_SKIACHARSET_Baltic			= 1 << 13,
    FPF_SKIACHARSET_Cyrillic		= 1 << 14,
    FPF_SKIACHARSET_Thai			= 1 << 15,
    FPF_SKIACHARSET_EeasternEuropean = 1 << 16,
    FPF_SKIACHARSET_PC				= 1 << 17,
    FPF_SKIACHARSET_OEM				= 1 << 18,
};
static FX_DWORD FPF_SkiaGetCharset(FX_BYTE uCharset)
{
    switch (uCharset) {
        case FXFONT_ANSI_CHARSET:
            return FPF_SKIACHARSET_Ansi;
        case FXFONT_DEFAULT_CHARSET:
            return FPF_SKIACHARSET_Default;
        case FXFONT_SYMBOL_CHARSET:
            return FPF_SKIACHARSET_Symbol;
        case FXFONT_SHIFTJIS_CHARSET:
            return FPF_SKIACHARSET_ShiftJIS;
        case FXFONT_HANGEUL_CHARSET:
            return FPF_SKIACHARSET_Korean;
        case FXFONT_GB2312_CHARSET:
            return FPF_SKIACHARSET_GB2312;
        case FXFONT_CHINESEBIG5_CHARSET:
            return FPF_SKIACHARSET_BIG5;
        case FXFONT_GREEK_CHARSET:
            return FPF_SKIACHARSET_Greek;
        case FXFONT_TURKISH_CHARSET:
            return FPF_SKIACHARSET_Turkish;
        case FXFONT_HEBREW_CHARSET:
            return FPF_SKIACHARSET_Hebrew;
        case FXFONT_ARABIC_CHARSET:
            return FPF_SKIACHARSET_Arabic;
        case FXFONT_BALTIC_CHARSET:
            return FPF_SKIACHARSET_Baltic;
        case FXFONT_RUSSIAN_CHARSET:
            return FPF_SKIACHARSET_Cyrillic;
        case FXFONT_THAI_CHARSET:
            return FPF_SKIACHARSET_Thai;
        case FXFONT_EASTEUROPE_CHARSET:
            return FPF_SKIACHARSET_EeasternEuropean;
    }
    return FPF_SKIACHARSET_Default;
}
static FX_DWORD FPF_SKIANormalizeFontName(FX_BSTR bsfamily)
{
    FX_DWORD dwHash = 0;
    FX_INT32 iLength = bsfamily.GetLength();
    FX_LPCSTR pBuffer = bsfamily.GetCStr();
    for (FX_INT32 i = 0; i < iLength; i++) {
        FX_CHAR ch = pBuffer[i];
        if (ch == ' ' || ch == '-' || ch == ',') {
            continue;
        }
        dwHash = 31 * dwHash + FXSYS_tolower(ch);
    }
    return dwHash;
}
static FX_DWORD	FPF_SKIAGetFamilyHash(FX_BSTR bsFamily, FX_DWORD dwStyle, FX_BYTE uCharset)
{
    CFX_ByteString bsFont(bsFamily);
    if (dwStyle & FXFONT_BOLD) {
        bsFont += "Bold";
    }
    if (dwStyle & FXFONT_ITALIC) {
        bsFont += "Italic";
    }
    if (dwStyle & FXFONT_SERIF)	{
        bsFont += "Serif";
    }
    bsFont += uCharset;
    return FPF_GetHashCode_StringA(bsFont.c_str(), bsFont.GetLength(), TRUE);
}
static FX_BOOL FPF_SkiaIsCJK(FX_BYTE uCharset)
{
    return (uCharset == FXFONT_GB2312_CHARSET) || (uCharset == FXFONT_CHINESEBIG5_CHARSET)
           || (uCharset == FXFONT_HANGEUL_CHARSET) || (uCharset == FXFONT_SHIFTJIS_CHARSET);
}
static FX_BOOL FPF_SkiaMaybeSymbol(FX_BSTR bsFacename)
{
    CFX_ByteString bsName = bsFacename;
    bsName.MakeLower();
    return bsName.Find("symbol") > -1;
}
static FX_BOOL FPF_SkiaMaybeArabic(FX_BSTR bsFacename)
{
    CFX_ByteString bsName = bsFacename;
    bsName.MakeLower();
    return bsName.Find("arabic") > -1;
}
CFPF_SkiaFontMgr::CFPF_SkiaFontMgr()
    : m_bLoaded(FALSE), m_FTLibrary(NULL)
{
}
CFPF_SkiaFontMgr::~CFPF_SkiaFontMgr()
{
    void *pkey = NULL;
    CFPF_SkiaFont *pValue = NULL;
    FX_POSITION pos = m_FamilyFonts.GetStartPosition();
    while (pos) {
        m_FamilyFonts.GetNextAssoc(pos, pkey, (void*&)pValue);
        if (pValue) {
            pValue->Release();
        }
    }
    m_FamilyFonts.RemoveAll();
    for (FX_INT32 i = m_FontFaces.GetUpperBound(); i >= 0; i--) {
        CFPF_SkiaFontDescriptor *pFont = (CFPF_SkiaFontDescriptor*)m_FontFaces.ElementAt(i);
        if (pFont) {
            delete pFont;
        }
    }
    m_FontFaces.RemoveAll();
    if (m_FTLibrary) {
        FXFT_Done_FreeType(m_FTLibrary);
    }
}
FX_BOOL CFPF_SkiaFontMgr::InitFTLibrary()
{
    if (m_FTLibrary == NULL) {
        FXFT_Init_FreeType(&m_FTLibrary);
    }
    return m_FTLibrary != NULL;
}
void CFPF_SkiaFontMgr::LoadSystemFonts()
{
    if (m_bLoaded) {
        return;
    }
    ScanPath(FX_BSTRC("/system/fonts"));
    OutputSystemFonts();
    m_bLoaded = TRUE;
}
void CFPF_SkiaFontMgr::LoadPrivateFont(IFX_FileRead* pFontFile)
{
}
void CFPF_SkiaFontMgr::LoadPrivateFont(FX_BSTR bsFileName)
{
}
void CFPF_SkiaFontMgr::LoadPrivateFont(FX_LPVOID pBuffer, size_t szBuffer)
{
}
IFPF_Font* CFPF_SkiaFontMgr::CreateFont(FX_BSTR bsFamilyname, FX_BYTE uCharset, FX_DWORD dwStyle, FX_DWORD dwMatch)
{
    FX_DWORD dwHash = FPF_SKIAGetFamilyHash(bsFamilyname, dwStyle, uCharset);
    IFPF_Font *pFont = NULL;
    if (m_FamilyFonts.Lookup((void*)(FX_UINTPTR)dwHash, (void*&)pFont)) {
        if (pFont) {
            return pFont->Retain();
        }
    }
    FX_DWORD dwFaceName = FPF_SKIANormalizeFontName(bsFamilyname);
    FX_DWORD dwSubst = FPF_SkiaGetSubstFont(dwFaceName);
    FX_DWORD dwSubstSans = FPF_SkiaGetSansFont(dwFaceName);
    FX_BOOL bMaybeSymbol = FPF_SkiaMaybeSymbol(bsFamilyname);
    if (uCharset != FXFONT_ARABIC_CHARSET && FPF_SkiaMaybeArabic(bsFamilyname)) {
        uCharset = FXFONT_ARABIC_CHARSET;
    } else if (uCharset == FXFONT_ANSI_CHARSET && (dwMatch & FPF_MATCHFONT_REPLACEANSI)) {
        uCharset = FXFONT_DEFAULT_CHARSET;
    }
    FX_INT32 nExpectVal = FPF_SKIAMATCHWEIGHT_NAME1 + FPF_SKIAMATCHWEIGHT_1 * 3 + FPF_SKIAMATCHWEIGHT_2 * 2;
    FX_INT32 nItem = -1;
    FX_INT32 nMax = -1;
    FX_INT32 nGlyphNum = 0;
    for (FX_INT32 i = m_FontFaces.GetUpperBound(); i >= 0; i--) {
        CFPF_SkiaPathFont *pFontDes = (CFPF_SkiaPathFont*)m_FontFaces.ElementAt(i);
        if(!(pFontDes->m_dwCharsets & FPF_SkiaGetCharset(uCharset))) {
            continue;
        }
        FX_INT32 nFind = 0;
        FX_DWORD dwSysFontName = FPF_SKIANormalizeFontName(pFontDes->m_pFamily);
        if (dwFaceName == dwSysFontName) {
            nFind += FPF_SKIAMATCHWEIGHT_NAME1;
        }
        FX_BOOL bMatchedName = (nFind == FPF_SKIAMATCHWEIGHT_NAME1);
        if ((dwStyle & FXFONT_BOLD) == (pFontDes->m_dwStyle & FXFONT_BOLD)) {
            nFind += FPF_SKIAMATCHWEIGHT_1;
        }
        if ((dwStyle & FXFONT_ITALIC) == (pFontDes->m_dwStyle & FXFONT_ITALIC)) {
            nFind += FPF_SKIAMATCHWEIGHT_1;
        }
        if ((dwStyle & FXFONT_FIXED_PITCH) == (pFontDes->m_dwStyle & FXFONT_FIXED_PITCH)) {
            nFind += FPF_SKIAMATCHWEIGHT_2;
        }
        if ((dwStyle & FXFONT_SERIF) == (pFontDes->m_dwStyle & FXFONT_SERIF)) {
            nFind += FPF_SKIAMATCHWEIGHT_1;
        }
        if ((dwStyle & FXFONT_SCRIPT) == (pFontDes->m_dwStyle & FXFONT_SCRIPT)) {
            nFind += FPF_SKIAMATCHWEIGHT_2;
        }
        if (dwSubst == dwSysFontName || dwSubstSans == dwSysFontName) {
            nFind += FPF_SKIAMATCHWEIGHT_NAME2;
            bMatchedName = TRUE;
        }
        if (uCharset == FXFONT_DEFAULT_CHARSET || bMaybeSymbol) {
            if (nFind > nMax && bMatchedName) {
                nMax = nFind;
                nItem = i;
            }
        } else if (FPF_SkiaIsCJK(uCharset)) {
            if (bMatchedName || pFontDes->m_iGlyphNum > nGlyphNum) {
                nItem = i;
                nGlyphNum = pFontDes->m_iGlyphNum;
            }
        } else if (nFind > nMax) {
            nMax = nFind;
            nItem = i;
        }
        if (nExpectVal <= nFind) {
            nItem = i;
            break;
        }
    }
    if (nItem > -1) {
        CFPF_SkiaFontDescriptor *pFontDes = (CFPF_SkiaFontDescriptor*)m_FontFaces.ElementAt(nItem);
        CFPF_SkiaFont *pFont = new CFPF_SkiaFont;
        if (pFont->InitFont(this, pFontDes, bsFamilyname, dwStyle, uCharset)) {
            m_FamilyFonts.SetAt((void*)(FX_UINTPTR)dwHash, (void*)pFont);
            return pFont->Retain();
        }
        pFont->Release();
    }
    return NULL;
}
FXFT_Face CFPF_SkiaFontMgr::GetFontFace(IFX_FileRead *pFileRead, FX_INT32 iFaceIndex)
{
    if (!pFileRead) {
        return NULL;
    }
    if (pFileRead->GetSize() == 0) {
        return NULL;
    }
    if (iFaceIndex < 0) {
        return NULL;
    }
    FXFT_StreamRec streamRec;
    FXSYS_memset32(&streamRec, 0, sizeof(FXFT_StreamRec));
    streamRec.size = pFileRead->GetSize();
    streamRec.descriptor.pointer = pFileRead;
    streamRec.read = FPF_SkiaStream_Read;
    streamRec.close = FPF_SkiaStream_Close;
    FXFT_Open_Args args;
    args.flags = FT_OPEN_STREAM;
    args.stream = &streamRec;
    FXFT_Face face;
    if (FXFT_Open_Face(m_FTLibrary, &args, iFaceIndex, &face)) {
        return NULL;
    }
    FXFT_Set_Pixel_Sizes(face, 0, 64);
    return face;
}
FXFT_Face CFPF_SkiaFontMgr::GetFontFace(FX_BSTR bsFile, FX_INT32 iFaceIndex )
{
    if (bsFile.IsEmpty()) {
        return NULL;
    }
    if (iFaceIndex < 0) {
        return NULL;
    }
    FXFT_Open_Args args;
    args.flags = FT_OPEN_PATHNAME;
    args.pathname = (FT_String*)bsFile.GetCStr();
    FXFT_Face face;
    if (FXFT_Open_Face(m_FTLibrary, &args, iFaceIndex, &face)) {
        return FALSE;
    }
    FXFT_Set_Pixel_Sizes(face, 0, 64);
    return face;
}
FXFT_Face CFPF_SkiaFontMgr::GetFontFace(FX_LPCBYTE pBuffer, size_t szBuffer, FX_INT32 iFaceIndex )
{
    if (!pBuffer || szBuffer < 1) {
        return NULL;
    }
    if (iFaceIndex < 0) {
        return NULL;
    }
    FXFT_Open_Args args;
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = pBuffer;
    args.memory_size = szBuffer;
    FXFT_Face face;
    if (FXFT_Open_Face(m_FTLibrary, &args, iFaceIndex, &face)) {
        return FALSE;
    }
    FXFT_Set_Pixel_Sizes(face, 0, 64);
    return face;
}
void CFPF_SkiaFontMgr::ScanPath(FX_BSTR path)
{
    void *handle = FX_OpenFolder(path.GetCStr());
    if (!handle) {
        return;
    }
    CFX_ByteString filename;
    FX_BOOL	bFolder = FALSE;
    while (FX_GetNextFile(handle, filename, bFolder)) {
        if (bFolder) {
            if (filename == FX_BSTRC(".") || filename == FX_BSTRC("..")) {
                continue;
            }
        } else {
            CFX_ByteString ext = filename.Right(4);
            ext.MakeLower();
            if (ext != FX_BSTRC(".ttf") && ext != FX_BSTRC(".ttc")) {
                continue;
            }
        }
        CFX_ByteString fullpath = path;
        fullpath += "/";
        fullpath += filename;
        if (bFolder) {
            ScanPath(fullpath);
        } else {
            ScanFile(fullpath);
        }
    }
    FX_CloseFolder(handle);
}
void CFPF_SkiaFontMgr::ScanFile(FX_BSTR file)
{
    FXFT_Face face = GetFontFace(file);
    if (face) {
        CFPF_SkiaPathFont *pFontDesc = new CFPF_SkiaPathFont;
        pFontDesc->SetPath(file.GetCStr());
        ReportFace(face, pFontDesc);
        m_FontFaces.Add(pFontDesc);
        FXFT_Done_Face(face);
    }
}
static const FX_DWORD g_FPFSkiaFontCharsets [] = {
    FPF_SKIACHARSET_Ansi,
    FPF_SKIACHARSET_EeasternEuropean,
    FPF_SKIACHARSET_Cyrillic,
    FPF_SKIACHARSET_Greek,
    FPF_SKIACHARSET_Turkish,
    FPF_SKIACHARSET_Hebrew,
    FPF_SKIACHARSET_Arabic,
    FPF_SKIACHARSET_Baltic,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    FPF_SKIACHARSET_Thai,
    FPF_SKIACHARSET_ShiftJIS,
    FPF_SKIACHARSET_GB2312,
    FPF_SKIACHARSET_Korean,
    FPF_SKIACHARSET_BIG5,
    FPF_SKIACHARSET_Johab,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    FPF_SKIACHARSET_OEM,
    FPF_SKIACHARSET_Symbol,
};
static FX_DWORD FPF_SkiaGetFaceCharset(TT_OS2 *pOS2)
{
    FX_DWORD dwCharset = 0;
    if (pOS2) {
        for (FX_INT32 i = 0; i < 32; i++) {
            if (pOS2->ulCodePageRange1 & (1 << i)) {
                dwCharset |= g_FPFSkiaFontCharsets[i];
            }
        }
    }
    dwCharset |= FPF_SKIACHARSET_Default;
    return dwCharset;
}
void CFPF_SkiaFontMgr::ReportFace(FXFT_Face face, CFPF_SkiaFontDescriptor *pFontDesc)
{
    if (!face || !pFontDesc) {
        return;
    }
    pFontDesc->SetFamily(FXFT_Get_Face_Family_Name(face));
    if (FXFT_Is_Face_Bold(face))	{
        pFontDesc->m_dwStyle |= FXFONT_BOLD;
    }
    if (FXFT_Is_Face_Italic(face))	{
        pFontDesc->m_dwStyle |= FXFONT_ITALIC;
    }
    if (FT_IS_FIXED_WIDTH(face))	{
        pFontDesc->m_dwStyle |= FXFONT_FIXED_PITCH;
    }
    TT_OS2 *pOS2 = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (pOS2) {
        if (pOS2->ulCodePageRange1 & (1 << 31)) {
            pFontDesc->m_dwStyle |= FXFONT_SYMBOLIC;
        }
        if (pOS2->panose[0] == 2) {
            FX_BYTE uSerif = pOS2->panose[1];
            if ((uSerif > 1 && uSerif < 10) || uSerif > 13) {
                pFontDesc->m_dwStyle |= FXFONT_SERIF;
            }
        }
    }
    if (pOS2 && (pOS2->ulCodePageRange1 & (1 << 31))) {
        pFontDesc->m_dwStyle |= FXFONT_SYMBOLIC;
    }
    pFontDesc->m_dwCharsets = FPF_SkiaGetFaceCharset(pOS2);
    pFontDesc->m_iFaceIndex = face->face_index;
    pFontDesc->m_iGlyphNum = face->num_glyphs;
}
void CFPF_SkiaFontMgr::OutputSystemFonts()
{
}
#endif
