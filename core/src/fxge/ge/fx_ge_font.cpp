// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxge/fx_freetype.h"
#include "text_int.h"
#define EM_ADJUST(em, a) (em == 0?(a): (a)*1000/em)
extern void _FPDFAPI_GetInternalFontData(int id1, FX_LPCBYTE& data, FX_DWORD& size);
CFX_Font::CFX_Font()
{
    m_pSubstFont = NULL;
    m_Face = NULL;
    m_bEmbedded = FALSE;
    m_bVertical = FALSE;
    m_pFontData = NULL;
    m_pFontDataAllocation = NULL;
    m_dwSize = 0;
    m_pOwnedStream = NULL;
    m_pGsubData = NULL;
    m_pPlatformFont = NULL;
    m_pPlatformFontCollection = NULL;
    m_pDwFont = NULL;
    m_hHandle = NULL;
    m_bDwLoaded = FALSE;
}
CFX_Font::~CFX_Font()
{
    if (m_pSubstFont) {
        delete m_pSubstFont;
        m_pSubstFont = NULL;
    }
    if (m_pFontDataAllocation) {
        FX_Free(m_pFontDataAllocation);
        m_pFontDataAllocation = NULL;
    }
    if (m_Face) {
        if (FXFT_Get_Face_External_Stream(m_Face)) {
            FXFT_Clear_Face_External_Stream(m_Face);
        }
        if(m_bEmbedded) {
            DeleteFace();
        } else {
            CFX_GEModule::Get()->GetFontMgr()->ReleaseFace(m_Face);
        }
    }
    if (m_pOwnedStream) {
        FX_Free(m_pOwnedStream);
        m_pOwnedStream = NULL;
    }
    if (m_pGsubData) {
        FX_Free(m_pGsubData);
        m_pGsubData = NULL;
    }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    ReleasePlatformResource();
#endif
}
void CFX_Font::DeleteFace()
{
    FXFT_Done_Face(m_Face);
    m_Face = NULL;
}
FX_BOOL CFX_Font::LoadSubst(const CFX_ByteString& face_name, FX_BOOL bTrueType, FX_DWORD flags,
                            int weight, int italic_angle, int CharsetCP, FX_BOOL bVertical)
{
    m_bEmbedded = FALSE;
    m_bVertical = bVertical;
    m_pSubstFont = new CFX_SubstFont;
    m_Face = CFX_GEModule::Get()->GetFontMgr()->FindSubstFont(face_name, bTrueType, flags, weight, italic_angle,
             CharsetCP, m_pSubstFont);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    if(m_pSubstFont->m_ExtHandle) {
        m_pPlatformFont = m_pSubstFont->m_ExtHandle;
        m_pSubstFont->m_ExtHandle = NULL;
    }
#endif
    if (m_Face) {
        m_pFontData = FXFT_Get_Face_Stream_Base(m_Face);
        m_dwSize = FXFT_Get_Face_Stream_Size(m_Face);
    }
    return TRUE;
}
extern "C" {
    unsigned long _FTStreamRead(FXFT_Stream stream, unsigned long offset,
                                unsigned char* buffer, unsigned long count)
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
    void _FTStreamClose(FXFT_Stream stream)
    {
    }
};
FX_BOOL _LoadFile(FXFT_Library library, FXFT_Face* Face, IFX_FileRead* pFile, FXFT_Stream* stream)
{
    FXFT_Stream stream1 = (FXFT_Stream)FX_Alloc(FX_BYTE, sizeof (FXFT_StreamRec));
    stream1->base = NULL;
    stream1->size = (unsigned long)pFile->GetSize();
    stream1->pos = 0;
    stream1->descriptor.pointer = pFile;
    stream1->close = _FTStreamClose;
    stream1->read = _FTStreamRead;
    FXFT_Open_Args args;
    args.flags = FT_OPEN_STREAM;
    args.stream = stream1;
    if (FXFT_Open_Face(library, &args, 0, Face)) {
        FX_Free(stream1);
        return FALSE;
    }
    if (stream) {
        *stream = stream1;
    }
    return TRUE;
}
FX_BOOL CFX_Font::LoadFile(IFX_FileRead* pFile)
{
    m_bEmbedded = FALSE;
    FXFT_Library library;
    if (CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary == NULL) {
        FXFT_Init_FreeType(&CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary);
    }
    library = CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary;
    FXFT_Stream stream = NULL;
    if (!_LoadFile(library, &m_Face, pFile, &stream)) {
        return FALSE;
    }
    m_pOwnedStream = stream;
    FXFT_Set_Pixel_Sizes(m_Face, 0, 64);
    return TRUE;
}
int CFX_Font::GetGlyphWidth(FX_DWORD glyph_index)
{
    if (!m_Face) {
        return 0;
    }
    if (m_pSubstFont && (m_pSubstFont->m_SubstFlags & FXFONT_SUBST_MM)) {
        AdjustMMParams(glyph_index, 0, 0);
    }
    int err = FXFT_Load_Glyph(m_Face, glyph_index, FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    if (err) {
        return 0;
    }
    int width = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face), FXFT_Get_Glyph_HoriAdvance(m_Face));
    return width;
}
static FXFT_Face FT_LoadFont(FX_LPBYTE pData, int size)
{
    FXFT_Library library;
    if (CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary == NULL) {
        FXFT_Init_FreeType(&CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary);
    }
    library = CFX_GEModule::Get()->GetFontMgr()->m_FTLibrary;
    FXFT_Face face = NULL;
    int error = FXFT_New_Memory_Face(library, pData, size, 0, &face);
    if (error) {
        return NULL;
    }
    error = FXFT_Set_Pixel_Sizes(face, 64, 64);
    if (error) {
        return NULL;
    }
    return face;
}
FX_BOOL CFX_Font::LoadEmbedded(FX_LPCBYTE data, FX_DWORD size)
{
    m_pFontDataAllocation = FX_Alloc(FX_BYTE, size);
    FXSYS_memcpy32(m_pFontDataAllocation, data, size);
    m_Face = FT_LoadFont((FX_LPBYTE)m_pFontDataAllocation, size);
    m_pFontData = (FX_LPBYTE)m_pFontDataAllocation;
    m_bEmbedded = TRUE;
    m_dwSize = size;
    return m_Face != NULL;
}
FX_BOOL CFX_Font::IsTTFont()
{
    if (m_Face == NULL) {
        return FALSE;
    }
    return FXFT_Is_Face_TT_OT(m_Face) == FXFT_FACE_FLAG_SFNT;
}
int CFX_Font::GetAscent() const
{
    if (m_Face == NULL) {
        return 0;
    }
    int ascent = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face), FXFT_Get_Face_Ascender(m_Face));
    return ascent;
}
int CFX_Font::GetDescent() const
{
    if (m_Face == NULL) {
        return 0;
    }
    int descent = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face), FXFT_Get_Face_Descender(m_Face));
    return descent;
}
FX_BOOL CFX_Font::GetGlyphBBox(FX_DWORD glyph_index, FX_RECT &bbox)
{
    if (m_Face == NULL) {
        return FALSE;
    }
    if (FXFT_Is_Face_Tricky(m_Face)) {
        int error = FXFT_Set_Char_Size(m_Face, 0, 1000 * 64, 72, 72);
        if (error) {
            return FALSE;
        }
        error = FXFT_Load_Glyph(m_Face, glyph_index, FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
        if (error) {
            return FALSE;
        }
        FXFT_BBox cbox;
        FT_Glyph glyph;
        error = FXFT_Get_Glyph(((FXFT_Face)m_Face)->glyph, &glyph);
        if (error) {
            return FALSE;
        }
        FXFT_Glyph_Get_CBox(glyph, FXFT_GLYPH_BBOX_PIXELS, &cbox);
        int pixel_size_x = ((FXFT_Face)m_Face)->size->metrics.x_ppem,
            pixel_size_y = ((FXFT_Face)m_Face)->size->metrics.y_ppem;
        if (pixel_size_x == 0 || pixel_size_y == 0) {
            bbox.left = cbox.xMin;
            bbox.right = cbox.xMax;
            bbox.top = cbox.yMax;
            bbox.bottom = cbox.yMin;
        } else {
            bbox.left = cbox.xMin * 1000 / pixel_size_x;
            bbox.right = cbox.xMax * 1000 / pixel_size_x;
            bbox.top = cbox.yMax * 1000 / pixel_size_y;
            bbox.bottom = cbox.yMin * 1000 / pixel_size_y;
        }
        if (bbox.top > FXFT_Get_Face_Ascender(m_Face)) {
            bbox.top = FXFT_Get_Face_Ascender(m_Face);
        }
        if (bbox.bottom < FXFT_Get_Face_Descender(m_Face)) {
            bbox.bottom = FXFT_Get_Face_Descender(m_Face);
        }
        FT_Done_Glyph(glyph);
        return FXFT_Set_Pixel_Sizes(m_Face, 0, 64) == 0;
    }
    if (FXFT_Load_Glyph(m_Face, glyph_index, FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH)) {
        return FALSE;
    }
    int em = FXFT_Get_Face_UnitsPerEM(m_Face);
    if (em == 0) {
        bbox.left = FXFT_Get_Glyph_HoriBearingX(m_Face);
        bbox.bottom = FXFT_Get_Glyph_HoriBearingY(m_Face);
        bbox.top = bbox.bottom - FXFT_Get_Glyph_Height(m_Face);
        bbox.right = bbox.left + FXFT_Get_Glyph_Width(m_Face);
    } else {
        bbox.left = FXFT_Get_Glyph_HoriBearingX(m_Face) * 1000 / em;
        bbox.top = (FXFT_Get_Glyph_HoriBearingY(m_Face) - FXFT_Get_Glyph_Height(m_Face)) * 1000 / em;
        bbox.right = (FXFT_Get_Glyph_HoriBearingX(m_Face) + FXFT_Get_Glyph_Width(m_Face)) * 1000 / em;
        bbox.bottom = (FXFT_Get_Glyph_HoriBearingY(m_Face)) * 1000 / em;
    }
    return TRUE;
}
FX_BOOL CFX_Font::IsItalic()
{
    if (m_Face == NULL) {
        return FALSE;
    }
    FX_BOOL ret = FXFT_Is_Face_Italic(m_Face) == FXFT_STYLE_FLAG_ITALIC;
    if (!ret) {
        CFX_ByteString str(FXFT_Get_Face_Style_Name(m_Face));
        str.MakeLower();
        if (str.Find("italic") != -1) {
            ret = TRUE;
        }
    }
    return ret;
}
FX_BOOL CFX_Font::IsBold()
{
    if (m_Face == NULL) {
        return FALSE;
    }
    return FXFT_Is_Face_Bold(m_Face) == FXFT_STYLE_FLAG_BOLD;
}
FX_BOOL CFX_Font::IsFixedWidth()
{
    if (m_Face == NULL) {
        return FALSE;
    }
    return FXFT_Is_Face_fixedwidth(m_Face);
}
CFX_WideString CFX_Font::GetPsName() const
{
    if (m_Face == NULL) {
        return CFX_WideString();
    }
    CFX_WideString psName = CFX_WideString::FromLocal(FXFT_Get_Postscript_Name(m_Face));
    if (psName.IsEmpty()) {
        psName =  CFX_WideString::FromLocal("Untitled");
    }
    return psName;
}
CFX_ByteString CFX_Font::GetFamilyName() const
{
    if (m_Face == NULL && m_pSubstFont == NULL) {
        return CFX_ByteString();
    }
    if (m_Face) {
        return CFX_ByteString(FXFT_Get_Face_Family_Name(m_Face));
    } else {
        return m_pSubstFont->m_Family;
    }
}
CFX_ByteString CFX_Font::GetFaceName() const
{
    if (m_Face == NULL && m_pSubstFont == NULL) {
        return CFX_ByteString();
    }
    if (m_Face) {
        CFX_ByteString facename;
        CFX_ByteString style = CFX_ByteString(FXFT_Get_Face_Style_Name(m_Face));
        facename = GetFamilyName();
        if (facename.IsEmpty()) {
            facename = "Untitled";
        }
        if (!style.IsEmpty() && style != "Regular") {
            facename += " " + style;
        }
        return facename;
    } else {
        return m_pSubstFont->m_Family;
    }
}
FX_BOOL CFX_Font::GetBBox(FX_RECT &bbox)
{
    if (m_Face == NULL) {
        return FALSE;
    }
    int em = FXFT_Get_Face_UnitsPerEM(m_Face);
    if (em == 0) {
        bbox.left = FXFT_Get_Face_xMin(m_Face);
        bbox.bottom = FXFT_Get_Face_yMax(m_Face);
        bbox.top = FXFT_Get_Face_yMin(m_Face);
        bbox.right = FXFT_Get_Face_xMax(m_Face);
    } else {
        bbox.left = FXFT_Get_Face_xMin(m_Face) * 1000 / em;
        bbox.top = FXFT_Get_Face_yMin(m_Face) * 1000 / em;
        bbox.right = FXFT_Get_Face_xMax(m_Face) * 1000 / em;
        bbox.bottom = FXFT_Get_Face_yMax(m_Face) * 1000 / em;
    }
    return TRUE;
}
int CFX_Font::GetHeight()
{
    if (m_Face == NULL) {
        return 0;
    }
    int height = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face), FXFT_Get_Face_Height(m_Face));
    return height;
}
int CFX_Font::GetMaxAdvanceWidth()
{
    if (m_Face == NULL) {
        return 0;
    }
    int width = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face), FXFT_Get_Face_MaxAdvanceWidth(m_Face));
    return width;
}
int CFX_Font::GetULPos()
{
    if (m_Face == NULL) {
        return 0;
    }
    int pos = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face), FXFT_Get_Face_UnderLinePosition(m_Face));
    return pos;
}
int CFX_Font::GetULthickness()
{
    if (m_Face == NULL) {
        return 0;
    }
    int thickness = EM_ADJUST(FXFT_Get_Face_UnitsPerEM(m_Face), FXFT_Get_Face_UnderLineThickness(m_Face));
    return thickness;
}
CFX_UnicodeEncoding::CFX_UnicodeEncoding(CFX_Font* pFont)
{
    m_pFont = pFont;
}
FX_DWORD CFX_UnicodeEncoding::GlyphFromCharCode(FX_DWORD charcode)
{
    FXFT_Face face =  m_pFont->GetFace();
    if (!face) {
        return charcode;
    }
    if (FXFT_Select_Charmap(face, FXFT_ENCODING_UNICODE) == 0) {
        return FXFT_Get_Char_Index(face, charcode);
    }
    if (m_pFont->m_pSubstFont && m_pFont->m_pSubstFont->m_Charset == 2) {
        FX_DWORD index = 0;
        if (FXFT_Select_Charmap(face, FXFT_ENCODING_MS_SYMBOL) == 0) {
            index = FXFT_Get_Char_Index(face, charcode);
        }
        if (!index && !FXFT_Select_Charmap(face, FXFT_ENCODING_APPLE_ROMAN)) {
            return FXFT_Get_Char_Index(face, charcode);
        }
    }
    return charcode;
}
FX_DWORD CFX_UnicodeEncoding::GlyphFromCharCodeEx(FX_DWORD charcode, int encoding)
{
    FXFT_Face face =  m_pFont->GetFace();
    if (!face) {
        return charcode;
    }
    if (encoding == ENCODING_UNICODE) {
        return	GlyphFromCharCode(charcode);
    } else {
        int nmaps = FXFT_Get_Face_CharmapCount(m_pFont->m_Face);
        int i = 0;
        while (i < nmaps) {
            int encoding = FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[i++]);
            if (encoding != FXFT_ENCODING_UNICODE) {
                FXFT_Select_Charmap(face, encoding);
                break;
            }
        }
    }
    return FXFT_Get_Char_Index(face, charcode);
}
IFX_FontEncoding* FXGE_CreateUnicodeEncoding(CFX_Font* pFont)
{
    return new CFX_UnicodeEncoding(pFont);
}
