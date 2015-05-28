// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_GE_TEXT_INT_H_
#define CORE_SRC_FXGE_GE_TEXT_INT_H_

struct _CFX_UniqueKeyGen {
    void		Generate(int count, ...);
    FX_CHAR		m_Key[128];
    int			m_KeyLen;
};
class CFX_SizeGlyphCache 
{
public:
    CFX_SizeGlyphCache()
    {
        m_GlyphMap.InitHashTable(253);
    }
    ~CFX_SizeGlyphCache();
    CFX_MapPtrToPtr			m_GlyphMap;
};
class CTTFontDesc 
{
public:
    CTTFontDesc()
    {
        m_Type = 0;
        m_pFontData = NULL;
        m_RefCount = 0;
    }
    ~CTTFontDesc();
    FX_BOOL			ReleaseFace(FXFT_Face face);
    int				m_Type;
    union {
        struct {
            FX_BOOL		m_bItalic;
            FX_BOOL		m_bBold;
            FXFT_Face	m_pFace;
        } m_SingleFace;
        struct {
            FXFT_Face	m_pFaces[16];
        } m_TTCFace;
    };
    FX_BYTE*		m_pFontData;
    int				m_RefCount;
};
class CFX_UnicodeEncoding : public IFX_FontEncoding
{
public:
    CFX_UnicodeEncoding(CFX_Font* pFont);
    virtual FX_DWORD		GlyphFromCharCodeEx(FX_DWORD charcode, int encoding = ENCODING_UNICODE);
private:
    CFX_Font*			m_pFont;
    virtual FX_DWORD		GlyphFromCharCode(FX_DWORD charcode);
    virtual CFX_WideString	UnicodeFromCharCode(FX_DWORD charcode) const
    {
        return CFX_WideString((FX_WCHAR)charcode);
    }
    virtual FX_DWORD		CharCodeFromUnicode(FX_WCHAR Unicode) const
    {
        return Unicode;
    }
    virtual FX_BOOL			IsUnicodeCompatible() const
    {
        return TRUE;
    }
};
#define CHARSET_FLAG_ANSI		1
#define CHARSET_FLAG_SYMBOL		2
#define CHARSET_FLAG_SHIFTJIS	4
#define CHARSET_FLAG_BIG5		8
#define CHARSET_FLAG_GB			16
#define CHARSET_FLAG_KOREAN		32
class CFontFaceInfo 
{
public:
    CFX_ByteString		m_FilePath;
    CFX_ByteString		m_FaceName;
    FX_DWORD			m_Styles;
    FX_DWORD			m_Charsets;
    FX_DWORD			m_FontOffset;
    FX_DWORD			m_FileSize;
    CFX_ByteString		m_FontTables;
};
class CFontFileFaceInfo 
{
public:
    CFontFileFaceInfo();
    ~CFontFileFaceInfo();
    IFX_FileStream*		m_pFile;
    FXFT_Face			m_Face;
    CFX_ByteString		m_FaceName;
    FX_DWORD			m_Charsets;
    FX_DWORD			m_FileSize;
    FX_DWORD			m_FontOffset;
    int					m_Weight;
    FX_BOOL				m_bItalic;
    int					m_PitchFamily;
    CFX_ByteString		m_FontTables;
};

#endif  // CORE_SRC_FXGE_GE_TEXT_INT_H_
