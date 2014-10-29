// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_FONT_H_
#define _FX_FONT_H_
#ifndef _FXCRT_EXTENSION_
#include "../../include/fxcrt/fx_ext.h"
#endif
#ifndef _FX_DIB_H_
#include "fx_dib.h"
#endif
typedef struct FT_FaceRec_* FXFT_Face;
typedef void* FXFT_Library;
class IFX_FontEncoding;
class CFX_PathData;
class CFX_SubstFont;
class CFX_FaceCache;
class IFX_FontMapper;
class CFX_FontMapper;
class IFX_SystemFontInfo;
class CFontFileFaceInfo;
#define FXFONT_FIXED_PITCH		0x01
#define FXFONT_SERIF			0x02
#define FXFONT_SYMBOLIC			0x04
#define FXFONT_SCRIPT			0x08
#define FXFONT_ITALIC			0x40
#define FXFONT_BOLD				0x40000
#define FXFONT_USEEXTERNATTR	0x80000
#define FXFONT_EXACTMATCH		0x80000000
#define FXFONT_CIDFONT			0x100000
#define FXFONT_ANSI_CHARSET		0
#define FXFONT_DEFAULT_CHARSET	1
#define FXFONT_SYMBOL_CHARSET	2
#define FXFONT_SHIFTJIS_CHARSET	128
#define FXFONT_HANGEUL_CHARSET	129
#define FXFONT_GB2312_CHARSET	134
#define FXFONT_CHINESEBIG5_CHARSET	136
#define FXFONT_THAI_CHARSET		222
#define FXFONT_EASTEUROPE_CHARSET	238
#define FXFONT_RUSSIAN_CHARSET	204
#define FXFONT_GREEK_CHARSET	161
#define FXFONT_TURKISH_CHARSET	162
#define FXFONT_HEBREW_CHARSET	177
#define FXFONT_ARABIC_CHARSET	178
#define FXFONT_BALTIC_CHARSET	186
#define FXFONT_FF_FIXEDPITCH	1
#define FXFONT_FF_ROMAN			(1<<4)
#define FXFONT_FF_SCRIPT		(4<<4)
#define FXFONT_FW_NORMAL		400
#define FXFONT_FW_BOLD			700
class CFX_Font : public CFX_Object
{
public:
    CFX_Font();
    ~CFX_Font();

    FX_BOOL					LoadSubst(const CFX_ByteString& face_name, FX_BOOL bTrueType, FX_DWORD flags,
                                      int weight, int italic_angle, int CharsetCP, FX_BOOL bVertical = FALSE);

    FX_BOOL					LoadEmbedded(FX_LPCBYTE data, FX_DWORD size);

	FX_BOOL					LoadFile(IFX_FileRead* pFile, int nFaceIndex = 0, int* pFaceCount = NULL);

    FX_BOOL                 LoadClone(const CFX_Font* pFont);


    FXFT_Face				GetFace() const
    {
        return m_Face;
    }


    const CFX_SubstFont*	GetSubstFont() const
    {
        return m_pSubstFont;
    }

    CFX_PathData*			LoadGlyphPath(FX_DWORD glyph_index, int dest_width = 0);

    int						GetGlyphWidth(FX_DWORD glyph_index);

    int						GetAscent() const;

    int						GetDescent() const;

    FX_BOOL                 GetGlyphBBox(FX_DWORD glyph_index, FX_RECT &bbox);

    FX_BOOL                 IsItalic();

    FX_BOOL                 IsBold();

    FX_BOOL                 IsFixedWidth();

    FX_BOOL					IsVertical() const
    {
        return m_bVertical;
    }

    CFX_WideString          GetPsName() const;


    CFX_ByteString          GetFamilyName() const;

    CFX_ByteString          GetFaceName() const;


    FX_BOOL                 IsTTFont();

    FX_BOOL                 GetBBox(FX_RECT &bbox);

    int                     GetHeight();

    int                     GetULPos();

    int                     GetULthickness();

    int                     GetMaxAdvanceWidth();

    FXFT_Face				m_Face;

    CFX_SubstFont*			m_pSubstFont;
    FX_BOOL					IsEmbedded()
    {
        return m_bEmbedded;
    }

    void					AdjustMMParams(int glyph_index, int width, int weight);
    FX_LPBYTE				m_pFontDataAllocation;
    FX_LPBYTE               m_pFontData;
    FX_LPBYTE				m_pGsubData;
    FX_DWORD                m_dwSize;
    CFX_BinaryBuf           m_OtfFontData;
    void*                   m_hHandle;
    void*                   m_pPlatformFont;
    void*                   m_pPlatformFontCollection;
    void*                   m_pDwFont;
    FX_BOOL                 m_bDwLoaded;
    void                    ReleasePlatformResource();

    void					DeleteFace();
protected:

    FX_BOOL					m_bEmbedded;
    FX_BOOL					m_bVertical;
    FX_BOOL                 m_bLogic;
    void*					m_pOwnedStream;
};
#define ENCODING_INTERNAL		0
#define ENCODING_UNICODE		1
class IFX_FontEncoding : public CFX_Object
{
public:
    virtual ~IFX_FontEncoding() {}

    virtual FX_DWORD		GlyphFromCharCode(FX_DWORD charcode) = 0;

    virtual CFX_WideString	UnicodeFromCharCode(FX_DWORD charcode) const = 0;

    virtual FX_DWORD		CharCodeFromUnicode(FX_WCHAR Unicode) const = 0;
};
IFX_FontEncoding* FXGE_CreateUnicodeEncoding(CFX_Font* pFont);
#define FXFM_ENC_TAG(a, b, c, d) (((FX_DWORD)(a) << 24) | ((FX_DWORD)(b) << 16) | ((FX_DWORD)(c) << 8) | (FX_DWORD)(d))
#define FXFM_ENCODING_NONE				FXFM_ENC_TAG(0, 0, 0, 0)
#define FXFM_ENCODING_MS_SYMBOL			FXFM_ENC_TAG('s', 'y', 'm', 'b')
#define FXFM_ENCODING_UNICODE			FXFM_ENC_TAG('u', 'n', 'i', 'c')
#define FXFM_ENCODING_MS_SJIS			FXFM_ENC_TAG('s', 'j', 'i', 's')
#define FXFM_ENCODING_MS_GB2312			FXFM_ENC_TAG('g', 'b', ' ', ' ')
#define FXFM_ENCODING_MS_BIG5			FXFM_ENC_TAG('b', 'i', 'g', '5')
#define FXFM_ENCODING_MS_WANSUNG		FXFM_ENC_TAG('w', 'a', 'n', 's')
#define FXFM_ENCODING_MS_JOHAB			FXFM_ENC_TAG('j', 'o', 'h', 'a')
#define FXFM_ENCODING_ADOBE_STANDARD	FXFM_ENC_TAG('A', 'D', 'O', 'B')
#define FXFM_ENCODING_ADOBE_EXPERT		FXFM_ENC_TAG('A', 'D', 'B', 'E')
#define FXFM_ENCODING_ADOBE_CUSTOM		FXFM_ENC_TAG('A', 'D', 'B', 'C')
#define FXFM_ENCODING_ADOBE_LATIN_1		FXFM_ENC_TAG('l', 'a', 't', '1')
#define FXFM_ENCODING_OLD_LATIN_2		FXFM_ENC_TAG('l', 'a', 't', '2')
#define FXFM_ENCODING_APPLE_ROMAN		FXFM_ENC_TAG('a', 'r', 'm', 'n')
class IFX_FontEncodingEx : public IFX_FontEncoding
{
public:

    virtual FX_DWORD		GlyphIndexFromName(FX_LPCSTR pStrName) = 0;

    virtual CFX_ByteString	NameFromGlyphIndex(FX_DWORD dwGlyphIndex) = 0;

    virtual FX_DWORD		CharCodeFromGlyphIndex(FX_DWORD dwGlyphIndex) = 0;
};
IFX_FontEncodingEx*	FX_CreateFontEncodingEx(CFX_Font* pFont, FX_DWORD nEncodingID = FXFM_ENCODING_NONE);
#define FXFONT_SUBST_MM				0x01
#define FXFONT_SUBST_GLYPHPATH		0x04
#define FXFONT_SUBST_CLEARTYPE		0x08
#define FXFONT_SUBST_TRANSFORM		0x10
#define FXFONT_SUBST_NONSYMBOL		0x20
#define FXFONT_SUBST_EXACT			0x40
#define FXFONT_SUBST_STANDARD		0x80
class CFX_SubstFont : public CFX_Object
{
public:

    CFX_SubstFont();

    FX_LPVOID				m_ExtHandle;

    CFX_ByteString			m_Family;

    int						m_Charset;

    FX_DWORD				m_SubstFlags;

    int						m_Weight;

    int						m_ItalicAngle;

    FX_BOOL					m_bSubstOfCJK;

    int						m_WeightCJK;

    FX_BOOL					m_bItlicCJK;
};
#define FX_FONT_FLAG_SERIF              0x01
#define FX_FONT_FLAG_FIXEDPITCH			0x02
#define FX_FONT_FLAG_ITALIC				0x04
#define FX_FONT_FLAG_BOLD				0x08
#define FX_FONT_FLAG_SYMBOLIC_SYMBOL	0x10
#define FX_FONT_FLAG_SYMBOLIC_DINGBATS	0x20
#define FX_FONT_FLAG_MULTIPLEMASTER		0x40
typedef struct {
    FX_LPCBYTE	m_pFontData;
    FX_DWORD	m_dwSize;
} FoxitFonts;
class CFX_FontMgr : public CFX_Object
{
public:
    CFX_FontMgr();
    ~CFX_FontMgr();
    void			InitFTLibrary();
    FXFT_Face		GetCachedFace(const CFX_ByteString& face_name,
                                  int weight, FX_BOOL bItalic, FX_LPBYTE& pFontData);
    FXFT_Face		AddCachedFace(const CFX_ByteString& face_name,
                                  int weight, FX_BOOL bItalic, FX_LPBYTE pData, FX_DWORD size, int face_index);
    FXFT_Face		GetCachedTTCFace(int ttc_size, FX_DWORD checksum,
                                     int font_offset, FX_LPBYTE& pFontData);
    FXFT_Face		AddCachedTTCFace(int ttc_size, FX_DWORD checksum,
                                     FX_LPBYTE pData, FX_DWORD size, int font_offset);
    FXFT_Face		GetFileFace(FX_LPCSTR filename, int face_index);
    FXFT_Face		GetFixedFace(FX_LPCBYTE pData, FX_DWORD size, int face_index);
    void			ReleaseFace(FXFT_Face face);
    void			SetSystemFontInfo(IFX_SystemFontInfo* pFontInfo);
    FXFT_Face		FindSubstFont(const CFX_ByteString& face_name, FX_BOOL bTrueType, FX_DWORD flags,
                                  int weight, int italic_angle, int CharsetCP, CFX_SubstFont* pSubstFont);

    void			FreeCache();

    FX_BOOL			GetStandardFont(FX_LPCBYTE& pFontData, FX_DWORD& size, int index);
    CFX_FontMapper*	m_pBuiltinMapper;
    IFX_FontMapper*	m_pExtMapper;
    CFX_MapByteStringToPtr	m_FaceMap;
    FXFT_Library	m_FTLibrary;
    FoxitFonts m_ExternalFonts[16];
};
class IFX_FontMapper : public CFX_Object
{
public:

    virtual ~IFX_FontMapper() {}

    virtual FXFT_Face	FindSubstFont(const CFX_ByteString& face_name, FX_BOOL bTrueType, FX_DWORD flags,
                                      int weight, int italic_angle, int CharsetCP, CFX_SubstFont* pSubstFont) = 0;

    CFX_FontMgr*		m_pFontMgr;
};
class IFX_FontEnumerator
{
public:

    virtual void		HitFont() = 0;

    virtual void		Finish() = 0;
};
class IFX_AdditionalFontEnum
{
public:
    virtual int  CountFiles() = 0;
    virtual IFX_FileStream* GetFontFile(int index) = 0;
};
class CFX_FontMapper : public IFX_FontMapper
{
public:
    CFX_FontMapper();
    virtual ~CFX_FontMapper();
    void				SetSystemFontInfo(IFX_SystemFontInfo* pFontInfo);
    IFX_SystemFontInfo*	GetSystemFontInfo()
    {
        return m_pFontInfo;
    }
    void				AddInstalledFont(const CFX_ByteString& name, int charset);
    void				LoadInstalledFonts();
    CFX_ByteStringArray	m_InstalledTTFonts;
    void				SetFontEnumerator(IFX_FontEnumerator* pFontEnumerator)
    {
        m_pFontEnumerator = pFontEnumerator;
    }
    IFX_FontEnumerator*	GetFontEnumerator() const
    {
        return m_pFontEnumerator;
    }
    virtual FXFT_Face	FindSubstFont(const CFX_ByteString& face_name, FX_BOOL bTrueType, FX_DWORD flags,
                                      int weight, int italic_angle, int CharsetCP, CFX_SubstFont* pSubstFont);
    FXFT_Face	FindSubstFontByUnicode(FX_DWORD dwUnicode, FX_DWORD flags, int weight, int italic_angle);
private:
    CFX_ByteString		GetPSNameFromTT(void* hFont);
    CFX_ByteString		MatchInstalledFonts(const CFX_ByteString& norm_name);
    FXFT_Face			UseInternalSubst(CFX_SubstFont* pSubstFont, int iBaseFont, int italic_angle, int weight, int picthfamily);

    FX_BOOL				m_bListLoaded;
    FXFT_Face			m_MMFaces[2];
    CFX_ByteString		m_LastFamily;
    CFX_DWordArray		m_CharsetArray;
    CFX_ByteStringArray	m_FaceArray;
    IFX_SystemFontInfo*	m_pFontInfo;
    FXFT_Face			m_FoxitFaces[14];
    IFX_FontEnumerator*		m_pFontEnumerator;
};
class IFX_SystemFontInfo : public CFX_Object
{
public:
    static IFX_SystemFontInfo*	CreateDefault();
    virtual void		Release() = 0;
    virtual	FX_BOOL		EnumFontList(CFX_FontMapper* pMapper) = 0;
    virtual void*		MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR face, FX_BOOL& bExact) = 0;
    virtual void*		MapFontByUnicode(FX_DWORD dwUnicode, int weight, FX_BOOL bItalic, int pitch_family)
    {
        return NULL;
    }
    virtual void*		GetFont(FX_LPCSTR face) = 0;
    virtual FX_DWORD	GetFontData(void* hFont, FX_DWORD table, FX_LPBYTE buffer, FX_DWORD size) = 0;
    virtual FX_BOOL		GetFaceName(void* hFont, CFX_ByteString& name) = 0;
    virtual FX_BOOL		GetFontCharset(void* hFont, int& charset) = 0;
    virtual int			GetFaceIndex(void* hFont)
    {
        return 0;
    }
    virtual void		DeleteFont(void* hFont) = 0;
    virtual void*       RetainFont(void* hFont)
    {
        return NULL;
    }
};
class CFX_FolderFontInfo : public IFX_SystemFontInfo
{
public:
    CFX_FolderFontInfo();
    virtual ~CFX_FolderFontInfo();
    void				AddPath(FX_BSTR path);
    virtual void		Release();
    virtual	FX_BOOL		EnumFontList(CFX_FontMapper* pMapper);
    virtual void*		MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR face, FX_BOOL& bExact);
    virtual void*		MapFontByUnicode(FX_DWORD dwUnicode, int weight, FX_BOOL bItalic, int pitch_family);
    virtual void*		GetFont(FX_LPCSTR face);
    virtual FX_DWORD	GetFontData(void* hFont, FX_DWORD table, FX_LPBYTE buffer, FX_DWORD size);
    virtual void		DeleteFont(void* hFont);
    virtual	FX_BOOL		GetFaceName(void* hFont, CFX_ByteString& name);
    virtual FX_BOOL		GetFontCharset(void* hFont, int& charset);
protected:
    CFX_MapByteStringToPtr	m_FontList;
    CFX_ByteStringArray	m_PathList;
    CFX_FontMapper*		m_pMapper;
    void				ScanPath(CFX_ByteString& path);
    void				ScanFile(CFX_ByteString& path);
    void				ReportFace(CFX_ByteString& path, FXSYS_FILE* pFile, FX_DWORD filesize, FX_DWORD offset);
};
class CFX_CountedFaceCache : public CFX_Object
{
public:
    CFX_FaceCache*	m_Obj;
    FX_DWORD		m_nCount;
};
typedef CFX_MapPtrTemplate<FXFT_Face, CFX_CountedFaceCache*> CFX_FTCacheMap;
class CFX_FontCache : public CFX_Object
{
public:
    ~CFX_FontCache();
    CFX_FaceCache*			GetCachedFace(CFX_Font* pFont);
    void					ReleaseCachedFace(CFX_Font* pFont);
    void					FreeCache(FX_BOOL bRelease = FALSE);

private:
    CFX_FTCacheMap			m_FTFaceMap;
    CFX_FTCacheMap			m_ExtFaceMap;
};
class CFX_AutoFontCache
{
public:
    CFX_AutoFontCache(CFX_FontCache* pFontCache, CFX_Font* pFont)
        : m_pFontCache(pFontCache)
        , m_pFont(pFont)
    {
    }
    ~CFX_AutoFontCache()
    {
        m_pFontCache->ReleaseCachedFace(m_pFont);
    }
    CFX_FontCache* m_pFontCache;
    CFX_Font* m_pFont;
};
#define FX_FONTCACHE_DEFINE(pFontCache, pFont) CFX_AutoFontCache autoFontCache((pFontCache), (pFont))
class CFX_GlyphBitmap : public CFX_Object
{
public:
    int						m_Top;
    int						m_Left;
    CFX_DIBitmap			m_Bitmap;
};
class CFX_FaceCache : public CFX_Object
{
public:
    ~CFX_FaceCache();
    const CFX_GlyphBitmap*	LoadGlyphBitmap(CFX_Font* pFont, FX_DWORD glyph_index, FX_BOOL bFontStyle, const CFX_AffineMatrix* pMatrix,
                                            int dest_width, int anti_alias, int& text_flags);
    const CFX_PathData*		LoadGlyphPath(CFX_Font* pFont, FX_DWORD glyph_index, int dest_width);


    CFX_FaceCache(FXFT_Face face);
private:
    FXFT_Face				m_Face;
    CFX_GlyphBitmap*		RenderGlyph(CFX_Font* pFont, FX_DWORD glyph_index, FX_BOOL bFontStyle,
                                        const CFX_AffineMatrix* pMatrix, int dest_width, int anti_alias);
    CFX_GlyphBitmap*		RenderGlyph_Nativetext(CFX_Font* pFont, FX_DWORD glyph_index,
            const CFX_AffineMatrix* pMatrix, int dest_width, int anti_alias);
    CFX_GlyphBitmap*        LookUpGlyphBitmap(CFX_Font* pFont, const CFX_AffineMatrix* pMatrix, CFX_ByteStringC& FaceGlyphsKey,
            FX_DWORD glyph_index, FX_BOOL bFontStyle, int dest_width, int anti_alias);
    CFX_MapByteStringToPtr	m_SizeMap;
    CFX_MapPtrToPtr			m_PathMap;
    CFX_DIBitmap*           m_pBitmap;
    void*                   m_pPlatformGraphics;
    void*                   m_pPlatformBitmap;
    void*                   m_hDC;
    void*                   m_hBitmap;
    void*                   m_hOldBitmap;
    void*                   m_hGdiFont;
    void*                   m_hOldGdiFont;

    void				    InitPlatform();
    void				    DestroyPlatform();
};
typedef struct {
    const CFX_GlyphBitmap*	m_pGlyph;
    int					m_OriginX, m_OriginY;
    FX_FLOAT			m_fOriginX, m_fOriginY;
} FXTEXT_GLYPHPOS;
FX_RECT FXGE_GetGlyphsBBox(FXTEXT_GLYPHPOS* pGlyphAndPos, int nChars, int anti_alias, FX_FLOAT retinaScaleX = 1.0f, FX_FLOAT retinaScaleY = 1.0f);
FX_BOOL	OutputGlyph(void* dib, int x, int y, CFX_Font* pFont, double font_size,
                    CFX_AffineMatrix* pMatrix, unsigned long glyph_index, unsigned long argb);
FX_BOOL	OutputText(void* dib, int x, int y, CFX_Font* pFont, double font_size,
                   CFX_AffineMatrix* pText_matrix, unsigned short const* text, unsigned long argb);
class IFX_GSUBTable
{
public:
    virtual void	Release() = 0;
    virtual FX_BOOL GetVerticalGlyph(FX_DWORD glyphnum, FX_DWORD* vglyphnum) = 0;
};
IFX_GSUBTable* FXGE_CreateGSUBTable(CFX_Font* pFont);
#endif
