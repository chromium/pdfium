// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#include "font_int.h"
#include "../fpdf_page/pageint.h"
#include "../../../include/fxge/fx_freetype.h"
FX_BOOL FT_UseTTCharmap(FXFT_Face face, int platform_id, int encoding_id)
{
    for (int i = 0; i < FXFT_Get_Face_CharmapCount(face); i ++) {
        if (FXFT_Get_Charmap_PlatformID(FXFT_Get_Face_Charmaps(face)[i]) == platform_id &&
                FXFT_Get_Charmap_EncodingID(FXFT_Get_Face_Charmaps(face)[i]) == encoding_id) {
            FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[i]);
            return TRUE;
        }
    }
    return FALSE;
}
extern const FX_WORD* PDF_UnicodesForPredefinedCharSet(int);
CPDF_FontGlobals::CPDF_FontGlobals()
{
    FXSYS_memset32(m_EmbeddedCharsets, 0, sizeof m_EmbeddedCharsets);
    FXSYS_memset32(m_EmbeddedToUnicodes, 0, sizeof m_EmbeddedToUnicodes);
    m_pContrastRamps = NULL;
}
CPDF_FontGlobals::~CPDF_FontGlobals()
{
    ClearAll();
    if (m_pContrastRamps) {
        FX_Free(m_pContrastRamps);
    }
}
class CFX_StockFontArray : public CFX_Object
{
public:
    CFX_StockFontArray()
    {
        FXSYS_memset32(m_pStockFonts, 0, sizeof(CPDF_Font*) * 14);
    }
    CPDF_Font*			m_pStockFonts[14];
};
CPDF_Font* CPDF_FontGlobals::Find(void* key, int index)
{
    void* value = NULL;
    if (!m_pStockMap.Lookup(key, value)) {
        return NULL;
    }
    if (!value) {
        return NULL;
    }
    return ((CFX_StockFontArray*)value)->m_pStockFonts[index];
}
void CPDF_FontGlobals::Set(void* key, int index, CPDF_Font* pFont)
{
    void* value = NULL;
    if (m_pStockMap.Lookup(key, value)) {
        ((CFX_StockFontArray*)value)->m_pStockFonts[index] = pFont;
        return;
    }
    CFX_StockFontArray* pFonts = FX_NEW CFX_StockFontArray();
    if (pFonts) {
        pFonts->m_pStockFonts[index] = pFont;
    }
    m_pStockMap.SetAt(key, pFonts);
}
void CPDF_FontGlobals::Clear(void* key)
{
    void* value = NULL;
    if (!m_pStockMap.Lookup(key, value)) {
        return;
    }
    if (value) {
        CFX_StockFontArray* pStockFonts = (CFX_StockFontArray*)value;
        for (int i = 0; i < 14; i ++) {
            if (pStockFonts->m_pStockFonts[i]) {
                CPDF_Dictionary* pFontDict = pStockFonts->m_pStockFonts[i]->GetFontDict();
                if (pFontDict)
                    pFontDict->Release();
                delete pStockFonts->m_pStockFonts[i];
            }
        }
        delete pStockFonts;
    }
    m_pStockMap.RemoveKey(key);
}
void CPDF_FontGlobals::ClearAll()
{
    FX_POSITION pos = m_pStockMap.GetStartPosition();
    while (pos) {
        void *key = NULL;
        void* value = NULL;
        m_pStockMap.GetNextAssoc(pos, key, value);
        if (value) {
            CFX_StockFontArray* pStockFonts = (CFX_StockFontArray*)value;
            for (int i = 0; i < 14; i ++) {
                if (pStockFonts->m_pStockFonts[i]) {
                    CPDF_Dictionary* pFontDict = pStockFonts->m_pStockFonts[i]->GetFontDict();
                    if (pFontDict)
                        pFontDict->Release();
                    delete pStockFonts->m_pStockFonts[i];
                }
            }
            delete pStockFonts;
        }
        m_pStockMap.RemoveKey(key);
    }
}
CPDF_Font::CPDF_Font()
{
    m_FontType = 0;
    m_FontBBox.left = m_FontBBox.right = m_FontBBox.top = m_FontBBox.bottom = 0;
    m_StemV = m_Ascent = m_Descent = m_ItalicAngle = 0;
    m_pFontFile = NULL;
    m_Flags = 0;
    m_pToUnicodeMap = NULL;
    m_bToUnicodeLoaded = FALSE;
    m_pCharMap = NULL;
}
FX_BOOL CPDF_Font::Initialize()
{
    m_pCharMap = FX_NEW CPDF_FontCharMap(this);
    return TRUE;
}
CPDF_Font::~CPDF_Font()
{
    if (m_pCharMap) {
        FX_Free(m_pCharMap);
        m_pCharMap = NULL;
    }
    if (m_pToUnicodeMap) {
        delete m_pToUnicodeMap;
        m_pToUnicodeMap = NULL;
    }
    if (m_pFontFile) {
        m_pDocument->GetPageData()->ReleaseFontFileStreamAcc((CPDF_Stream*)m_pFontFile->GetStream());
    }
}
FX_BOOL CPDF_Font::IsVertWriting() const
{
    FX_BOOL bVertWriting = FALSE;
    CPDF_CIDFont* pCIDFont = GetCIDFont();
    if (pCIDFont) {
        bVertWriting = pCIDFont->IsVertWriting();
    } else {
        bVertWriting = m_Font.IsVertical();
    }
    return bVertWriting;
}
CFX_ByteString CPDF_Font::GetFontTypeName() const
{
    switch (m_FontType) {
        case PDFFONT_TYPE1:
            return FX_BSTRC("Type1");
        case PDFFONT_TRUETYPE:
            return FX_BSTRC("TrueType");
        case PDFFONT_TYPE3:
            return FX_BSTRC("Type3");
        case PDFFONT_CIDFONT:
            return FX_BSTRC("Type0");
    }
    return CFX_ByteString();
}
void CPDF_Font::AppendChar(CFX_ByteString& str, FX_DWORD charcode) const
{
    char buf[4];
    int len = AppendChar(buf, charcode);
    if (len == 1) {
        str += buf[0];
    } else {
        str += CFX_ByteString(buf, len);
    }
}
CFX_WideString CPDF_Font::UnicodeFromCharCode(FX_DWORD charcode) const
{
    if (!m_bToUnicodeLoaded) {
        ((CPDF_Font*)this)->LoadUnicodeMap();
    }
    if (m_pToUnicodeMap) {
        CFX_WideString wsRet = m_pToUnicodeMap->Lookup(charcode);
        if (!wsRet.IsEmpty()) {
            return wsRet;
        }
    }
    FX_WCHAR unicode = _UnicodeFromCharCode(charcode);
    if (unicode == 0) {
        return CFX_WideString();
    }
    return unicode;
}
FX_DWORD CPDF_Font::CharCodeFromUnicode(FX_WCHAR unicode) const
{
    if (!m_bToUnicodeLoaded) {
        ((CPDF_Font*)this)->LoadUnicodeMap();
    }
    if (m_pToUnicodeMap) {
        FX_DWORD charcode = m_pToUnicodeMap->ReverseLookup(unicode);
        if (charcode) {
            return charcode;
        }
    }
    return _CharCodeFromUnicode(unicode);
}
CFX_WideString CPDF_Font::DecodeString(const CFX_ByteString& str) const
{
    CFX_WideString result;
    int src_len = str.GetLength();
    result.Reserve(src_len);
    FX_LPCSTR src_buf = str;
    int src_pos = 0;
    while (src_pos < src_len) {
        FX_DWORD charcode = GetNextChar(src_buf, src_pos);
        CFX_WideString unicode = UnicodeFromCharCode(charcode);
        if (!unicode.IsEmpty()) {
            result += unicode;
        } else {
            result += (FX_WCHAR)charcode;
        }
    }
    return result;
}
CFX_ByteString CPDF_Font::EncodeString(const CFX_WideString& str) const
{
    CFX_ByteString result;
    int src_len = str.GetLength();
    FX_LPSTR dest_buf = result.GetBuffer(src_len * 2);
    FX_LPCWSTR src_buf = str.c_str();
    int dest_pos = 0;
    for (int src_pos = 0; src_pos < src_len; src_pos ++) {
        FX_DWORD charcode = CharCodeFromUnicode(src_buf[src_pos]);
        dest_pos += AppendChar(dest_buf + dest_pos, charcode);
    }
    result.ReleaseBuffer(dest_pos);
    return result;
}
void CPDF_Font::LoadFontDescriptor(CPDF_Dictionary* pFontDesc)
{
    m_Flags = pFontDesc->GetInteger(FX_BSTRC("Flags"), PDFFONT_NONSYMBOLIC);
    int ItalicAngle = 0;
    FX_BOOL bExistItalicAngle = FALSE;
    if (pFontDesc->KeyExist(FX_BSTRC("ItalicAngle"))) {
        ItalicAngle = pFontDesc->GetInteger(FX_BSTRC("ItalicAngle"));
        bExistItalicAngle = TRUE;
    }
    if (ItalicAngle < 0) {
        m_Flags |= PDFFONT_ITALIC;
        m_ItalicAngle = ItalicAngle;
    }
    FX_BOOL bExistStemV = FALSE;
    if (pFontDesc->KeyExist(FX_BSTRC("StemV"))) {
        m_StemV = pFontDesc->GetInteger(FX_BSTRC("StemV"));
        bExistStemV = TRUE;
    }
    FX_BOOL bExistAscent = FALSE;
    if (pFontDesc->KeyExist(FX_BSTRC("Ascent"))) {
        m_Ascent = pFontDesc->GetInteger(FX_BSTRC("Ascent"));
        bExistAscent = TRUE;
    }
    FX_BOOL bExistDescent = FALSE;
    if (pFontDesc->KeyExist(FX_BSTRC("Descent"))) {
        m_Descent = pFontDesc->GetInteger(FX_BSTRC("Descent"));
        bExistDescent = TRUE;
    }
    FX_BOOL bExistCapHeight = FALSE;
    if (pFontDesc->KeyExist(FX_BSTRC("CapHeight"))) {
        bExistCapHeight = TRUE;
    }
    if (bExistItalicAngle && bExistAscent && bExistCapHeight && bExistDescent && bExistStemV) {
        m_Flags |= PDFFONT_USEEXTERNATTR;
    }
    if (m_Descent > 10) {
        m_Descent = -m_Descent;
    }
    CPDF_Array* pBBox = pFontDesc->GetArray(FX_BSTRC("FontBBox"));
    if (pBBox) {
        m_FontBBox.left = pBBox->GetInteger(0);
        m_FontBBox.bottom = pBBox->GetInteger(1);
        m_FontBBox.right = pBBox->GetInteger(2);
        m_FontBBox.top = pBBox->GetInteger(3);
    }
    CPDF_Stream* pFontFile = pFontDesc->GetStream(FX_BSTRC("FontFile"));
    if (pFontFile == NULL) {
        pFontFile = pFontDesc->GetStream(FX_BSTRC("FontFile2"));
    }
    if (pFontFile == NULL) {
        pFontFile = pFontDesc->GetStream(FX_BSTRC("FontFile3"));
    }
    if (pFontFile) {
        m_pFontFile = m_pDocument->LoadFontFile(pFontFile);
        if (m_pFontFile == NULL) {
            return;
        }
        FX_LPCBYTE pFontData = m_pFontFile->GetData();
        FX_DWORD dwFontSize = m_pFontFile->GetSize();
        m_Font.LoadEmbedded(pFontData, dwFontSize);
        if (m_Font.m_Face == NULL) {
            m_pFontFile = NULL;
        }
    }
}
short TT2PDF(int m, FXFT_Face face)
{
    int upm = FXFT_Get_Face_UnitsPerEM(face);
    if (upm == 0) {
        return (short)m;
    }
    return (m * 1000 + upm / 2) / upm;
}
void CPDF_Font::CheckFontMetrics()
{
    if (m_FontBBox.top == 0 && m_FontBBox.bottom == 0 && m_FontBBox.left == 0 && m_FontBBox.right == 0) {
        if (m_Font.m_Face) {
            m_FontBBox.left = TT2PDF(FXFT_Get_Face_xMin(m_Font.m_Face), m_Font.m_Face);
            m_FontBBox.bottom = TT2PDF(FXFT_Get_Face_yMin(m_Font.m_Face), m_Font.m_Face);
            m_FontBBox.right = TT2PDF(FXFT_Get_Face_xMax(m_Font.m_Face), m_Font.m_Face);
            m_FontBBox.top = TT2PDF(FXFT_Get_Face_yMax(m_Font.m_Face), m_Font.m_Face);
            m_Ascent = TT2PDF(FXFT_Get_Face_Ascender(m_Font.m_Face), m_Font.m_Face);
            m_Descent = TT2PDF(FXFT_Get_Face_Descender(m_Font.m_Face), m_Font.m_Face);
        } else {
            FX_BOOL bFirst = TRUE;
            for (int i = 0; i < 256; i ++) {
                FX_RECT rect;
                GetCharBBox(i, rect);
                if (rect.left == rect.right) {
                    continue;
                }
                if (bFirst) {
                    m_FontBBox = rect;
                    bFirst = FALSE;
                } else {
                    if (m_FontBBox.top < rect.top) {
                        m_FontBBox.top = rect.top;
                    }
                    if (m_FontBBox.right < rect.right) {
                        m_FontBBox.right = rect.right;
                    }
                    if (m_FontBBox.left > rect.left) {
                        m_FontBBox.left = rect.left;
                    }
                    if (m_FontBBox.bottom > rect.bottom) {
                        m_FontBBox.bottom = rect.bottom;
                    }
                }
            }
        }
    }
    if (m_Ascent == 0 && m_Descent == 0) {
        FX_RECT rect;
        GetCharBBox('A', rect);
        if (rect.bottom == rect.top) {
            m_Ascent = m_FontBBox.top;
        } else {
            m_Ascent = rect.top;
        }
        GetCharBBox('g', rect);
        if (rect.bottom == rect.top) {
            m_Descent = m_FontBBox.bottom;
        } else {
            m_Descent = rect.bottom;
        }
    }
}
void CPDF_Font::LoadUnicodeMap()
{
    m_bToUnicodeLoaded = TRUE;
    CPDF_Stream* pStream = m_pFontDict->GetStream(FX_BSTRC("ToUnicode"));
    if (pStream == NULL) {
        return;
    }
    m_pToUnicodeMap = FX_NEW CPDF_ToUnicodeMap;
    m_pToUnicodeMap->Load(pStream);
}
int CPDF_Font::GetStringWidth(FX_LPCSTR pString, int size)
{
    int offset = 0;
    int width = 0;
    while (offset < size) {
        FX_DWORD charcode = GetNextChar(pString, offset);
        width += GetCharWidthF(charcode);
    }
    return width;
}
int CPDF_Font::GetCharTypeWidth(FX_DWORD charcode)
{
    if (m_Font.m_Face == NULL) {
        return 0;
    }
    int glyph_index = GlyphFromCharCode(charcode);
    if (glyph_index == 0xffff) {
        return 0;
    }
    return m_Font.GetGlyphWidth(glyph_index);
}
int _PDF_GetStandardFontName(CFX_ByteString& name);
CPDF_Font* CPDF_Font::GetStockFont(CPDF_Document* pDoc, FX_BSTR name)
{
    CFX_ByteString fontname(name);
    int font_id = _PDF_GetStandardFontName(fontname);
    if (font_id < 0) {
        return NULL;
    }
    CPDF_FontGlobals* pFontGlobals = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
    CPDF_Font* pFont = pFontGlobals->Find(pDoc, font_id);
    if (pFont) {
        return pFont;
    }
    CPDF_Dictionary* pDict = CPDF_Dictionary::Create();
    pDict->SetAtName(FX_BSTRC("Type"), FX_BSTRC("Font"));
    pDict->SetAtName(FX_BSTRC("Subtype"), FX_BSTRC("Type1"));
    pDict->SetAtName(FX_BSTRC("BaseFont"), fontname);
    pDict->SetAtName(FX_BSTRC("Encoding"), FX_BSTRC("WinAnsiEncoding"));
    pFont = CPDF_Font::CreateFontF(NULL, pDict);
    pFontGlobals->Set(pDoc, font_id, pFont);
    return pFont;
}
const FX_BYTE ChineseFontNames[][5] = {
    {0xCB, 0xCE, 0xCC, 0xE5, 0x00},
    {0xBF, 0xAC, 0xCC, 0xE5, 0x00},
    {0xBA, 0xDA, 0xCC, 0xE5, 0x00},
    {0xB7, 0xC2, 0xCB, 0xCE, 0x00},
    {0xD0, 0xC2, 0xCB, 0xCE, 0x00}
};
CPDF_Font* CPDF_Font::CreateFontF(CPDF_Document* pDoc, CPDF_Dictionary* pFontDict)
{
    CFX_ByteString type = pFontDict->GetString(FX_BSTRC("Subtype"));
    CPDF_Font* pFont;
    if (type == FX_BSTRC("TrueType")) {
        {
#if _FXM_PLATFORM_  == _FXM_PLATFORM_WINDOWS_ || _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_ || _FXM_PLATFORM_ == _FXM_PLATFORM_ANDROID_ || _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
            CFX_ByteString basefont = pFontDict->GetString(FX_BSTRC("BaseFont"));
            CFX_ByteString tag = basefont.Left(4);
            int i;
            int count = sizeof(ChineseFontNames) / sizeof(ChineseFontNames[0]);
            for (i = 0; i < count; ++i) {
                if (tag == CFX_ByteString((FX_LPCSTR)ChineseFontNames[i])) {
                    break;
                }
            }
            if (i < count) {
                CPDF_Dictionary* pFontDesc = pFontDict->GetDict(FX_BSTRC("FontDescriptor"));
                if (pFontDesc == NULL || !pFontDesc->KeyExist(FX_BSTRC("FontFile2"))) {
                    pFont = FX_NEW CPDF_CIDFont;
                    pFont->Initialize();
                    pFont->m_FontType = PDFFONT_CIDFONT;
                    pFont->m_pFontDict = pFontDict;
                    pFont->m_pDocument = pDoc;
                    if (!pFont->Load()) {
                        delete pFont;
                        return NULL;
                    }
                    return pFont;
                }
            }
#endif
        }
        pFont = FX_NEW CPDF_TrueTypeFont;
        pFont->Initialize();
        pFont->m_FontType = PDFFONT_TRUETYPE;
    } else if (type == FX_BSTRC("Type3")) {
        pFont = FX_NEW CPDF_Type3Font;
        pFont->Initialize();
        pFont->m_FontType = PDFFONT_TYPE3;
    } else if (type == FX_BSTRC("Type0")) {
        pFont = FX_NEW CPDF_CIDFont;
        pFont->Initialize();
        pFont->m_FontType = PDFFONT_CIDFONT;
    } else {
        pFont = FX_NEW CPDF_Type1Font;
        pFont->Initialize();
        pFont->m_FontType = PDFFONT_TYPE1;
    }
    pFont->m_pFontDict = pFontDict;
    pFont->m_pDocument = pDoc;
    if (!pFont->Load()) {
        delete pFont;
        return NULL;
    }
    return pFont;
}
FX_BOOL CPDF_Font::Load()
{
    if (m_pFontDict == NULL) {
        return FALSE;
    }
    CFX_ByteString type = m_pFontDict->GetString(FX_BSTRC("Subtype"));
    m_BaseFont = m_pFontDict->GetString(FX_BSTRC("BaseFont"));
    if (type == FX_BSTRC("MMType1")) {
        type = FX_BSTRC("Type1");
    }
    return _Load();
}
static CFX_WideString _FontMap_GetWideString(CFX_CharMap* pMap, const CFX_ByteString& bytestr)
{
    return ((CPDF_FontCharMap*)pMap)->m_pFont->DecodeString(bytestr);
}
static CFX_ByteString _FontMap_GetByteString(CFX_CharMap* pMap, const CFX_WideString& widestr)
{
    return ((CPDF_FontCharMap*)pMap)->m_pFont->EncodeString(widestr);
}
CPDF_FontCharMap::CPDF_FontCharMap(CPDF_Font* pFont)
{
    m_GetByteString = _FontMap_GetByteString;
    m_GetWideString = _FontMap_GetWideString;
    m_pFont = pFont;
}
CFX_WideString CPDF_ToUnicodeMap::Lookup(FX_DWORD charcode)
{
    FX_DWORD value;
    if (m_Map.Lookup(charcode, value)) {
        FX_WCHAR unicode = (FX_WCHAR)(value & 0xffff);
        if (unicode != 0xffff) {
            return unicode;
        }
        FX_LPCWSTR buf = m_MultiCharBuf.GetBuffer();
        FX_DWORD buf_len = m_MultiCharBuf.GetLength();
        if (buf == NULL || buf_len == 0) {
            return CFX_WideString();
        }
        FX_DWORD index = value >> 16;
        if (index >= buf_len) {
            return CFX_WideString();
        }
        FX_DWORD len = buf[index];
        if (index + len < index || index + len >= buf_len) {
            return CFX_WideString();
        }
        return CFX_WideString(buf + index + 1, len);
    }
    if (m_pBaseMap) {
        return m_pBaseMap->UnicodeFromCID((FX_WORD)charcode);
    }
    return CFX_WideString();
}
FX_DWORD CPDF_ToUnicodeMap::ReverseLookup(FX_WCHAR unicode)
{
    FX_POSITION pos = m_Map.GetStartPosition();
    while (pos) {
        FX_DWORD key, value;
        m_Map.GetNextAssoc(pos, key, value);
        if ((FX_WCHAR)value == unicode) {
            return key;
        }
    }
    return 0;
}
static FX_DWORD _StringToCode(FX_BSTR str)
{
    FX_LPCSTR buf = str.GetCStr();
    int len = str.GetLength();
    if (len == 0) {
        return 0;
    }
    int result = 0;
    if (buf[0] == '<') {
        for (int i = 1; i < len; i ++) {
            int digit;
            if (buf[i] >= '0' && buf[i] <= '9') {
                digit = buf[i] - '0';
            } else if (buf[i] >= 'a' && buf[i] <= 'f') {
                digit = buf[i] - 'a' + 10;
            } else if (buf[i] >= 'A' && buf[i] <= 'F') {
                digit = buf[i] - 'A' + 10;
            } else {
                break;
            }
            result = result * 16 + digit;
        }
        return result;
    } else {
        for (int i = 0; i < len; i ++) {
            if (buf[i] < '0' || buf[i] > '9') {
                break;
            }
            result = result * 10 + buf[i] - '0';
        }
    }
    return result;
}
static CFX_WideString _StringDataAdd(CFX_WideString str)
{
    CFX_WideString ret;
    int len = str.GetLength();
    FX_WCHAR value = 1;
    for (int i = len - 1; i >= 0 ; --i) {
        FX_WCHAR ch = str[i] + value;
        if (ch < str[i]) {
            ret.Insert(0, 0);
        } else {
            ret.Insert(0, ch);
            value = 0;
        }
    }
    if (value) {
        ret.Insert(0, value);
    }
    return ret;
}
static CFX_WideString _StringToWideString(FX_BSTR str)
{
    FX_LPCSTR buf = str.GetCStr();
    int len = str.GetLength();
    if (len == 0) {
        return CFX_WideString();
    }
    CFX_WideString result;
    if (buf[0] == '<') {
        int byte_pos = 0;
        FX_WCHAR ch = 0;
        for (int i = 1; i < len; i ++) {
            int digit;
            if (buf[i] >= '0' && buf[i] <= '9') {
                digit = buf[i] - '0';
            } else if (buf[i] >= 'a' && buf[i] <= 'f') {
                digit = buf[i] - 'a' + 10;
            } else if (buf[i] >= 'A' && buf[i] <= 'F') {
                digit = buf[i] - 'A' + 10;
            } else {
                break;
            }
            ch = ch * 16 + digit;
            byte_pos ++;
            if (byte_pos == 4) {
                result += ch;
                byte_pos = 0;
                ch = 0;
            }
        }
        return result;
    }
    if (buf[0] == '(') {
    }
    return result;
}
void CPDF_ToUnicodeMap::Load(CPDF_Stream* pStream)
{
    int CIDSet = 0;
    CPDF_StreamAcc stream;
    stream.LoadAllData(pStream, FALSE);
    CPDF_SimpleParser parser(stream.GetData(), stream.GetSize());
    m_Map.EstimateSize(stream.GetSize() / 8, 1024);
    while (1) {
        CFX_ByteStringC word = parser.GetWord();
        if (word.IsEmpty()) {
            break;
        }
        if (word == FX_BSTRC("beginbfchar")) {
            while (1) {
                word = parser.GetWord();
                if (word.IsEmpty() || word == FX_BSTRC("endbfchar")) {
                    break;
                }
                FX_DWORD srccode = _StringToCode(word);
                word = parser.GetWord();
                CFX_WideString destcode = _StringToWideString(word);
                int len = destcode.GetLength();
                if (len == 0) {
                    continue;
                }
                if (len == 1) {
                    m_Map.SetAt(srccode, destcode.GetAt(0));
                } else {
                    m_Map.SetAt(srccode, m_MultiCharBuf.GetLength() * 0x10000 + 0xffff);
                    m_MultiCharBuf.AppendChar(destcode.GetLength());
                    m_MultiCharBuf << destcode;
                }
            }
        } else if (word == FX_BSTRC("beginbfrange")) {
            while (1) {
                CFX_ByteString low, high;
                low = parser.GetWord();
                if (low.IsEmpty() || low == FX_BSTRC("endbfrange")) {
                    break;
                }
                high = parser.GetWord();
                FX_DWORD lowcode = _StringToCode(low);
                FX_DWORD highcode = (lowcode & 0xffffff00) | (_StringToCode(high) & 0xff);
                if (highcode == (FX_DWORD) - 1) {
                    break;
                }
                CFX_ByteString start = parser.GetWord();
                if (start == FX_BSTRC("[")) {
                    for (FX_DWORD code = lowcode; code <= highcode; code ++) {
                        CFX_ByteString dest = parser.GetWord();
                        CFX_WideString destcode = _StringToWideString(dest);
                        int len = destcode.GetLength();
                        if (len == 0) {
                            continue;
                        }
                        if (len == 1) {
                            m_Map.SetAt(code, destcode.GetAt(0));
                        } else {
                            m_Map.SetAt(code, m_MultiCharBuf.GetLength() * 0x10000 + 0xffff);
                            m_MultiCharBuf.AppendChar(destcode.GetLength());
                            m_MultiCharBuf << destcode;
                        }
                    }
                    parser.GetWord();
                } else {
                    CFX_WideString destcode = _StringToWideString(start);
                    int len = destcode.GetLength();
                    FX_DWORD value = 0;
                    if (len == 1) {
                        value = _StringToCode(start);
                        for (FX_DWORD code = lowcode; code <= highcode; code ++) {
                            m_Map.SetAt(code, value++);
                        }
                    } else {
                        for (FX_DWORD code = lowcode; code <= highcode; code ++) {
                            CFX_WideString retcode;
                            if (code == lowcode) {
                                retcode = destcode;
                            } else {
                                retcode = _StringDataAdd(destcode);
                            }
                            m_Map.SetAt(code, m_MultiCharBuf.GetLength() * 0x10000 + 0xffff);
                            m_MultiCharBuf.AppendChar(retcode.GetLength());
                            m_MultiCharBuf << retcode;
                            destcode = retcode;
                        }
                    }
                }
            }
        } else if (word == FX_BSTRC("/Adobe-Korea1-UCS2")) {
            CIDSet = CIDSET_KOREA1;
        } else if (word == FX_BSTRC("/Adobe-Japan1-UCS2")) {
            CIDSet = CIDSET_JAPAN1;
        } else if (word == FX_BSTRC("/Adobe-CNS1-UCS2")) {
            CIDSet = CIDSET_CNS1;
        } else if (word == FX_BSTRC("/Adobe-GB1-UCS2")) {
            CIDSet = CIDSET_GB1;
        }
    }
    if (CIDSet) {
        m_pBaseMap = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals()->m_CMapManager.GetCID2UnicodeMap(CIDSet, FALSE);
    } else {
        m_pBaseMap = NULL;
    }
}
static FX_BOOL GetPredefinedEncoding(int& basemap, const CFX_ByteString& value)
{
    if (value == FX_BSTRC("WinAnsiEncoding")) {
        basemap = PDFFONT_ENCODING_WINANSI;
    } else if (value == FX_BSTRC("MacRomanEncoding")) {
        basemap = PDFFONT_ENCODING_MACROMAN;
    } else if (value == FX_BSTRC("MacExpertEncoding")) {
        basemap = PDFFONT_ENCODING_MACEXPERT;
    } else if (value == FX_BSTRC("PDFDocEncoding")) {
        basemap = PDFFONT_ENCODING_PDFDOC;
    } else {
        return FALSE;
    }
    return TRUE;
}
void CPDF_Font::LoadPDFEncoding(CPDF_Object* pEncoding, int& iBaseEncoding, CFX_ByteString*& pCharNames,
                                FX_BOOL bEmbedded, FX_BOOL bTrueType)
{
    if (pEncoding == NULL) {
        if (m_BaseFont == FX_BSTRC("Symbol")) {
            iBaseEncoding = bTrueType ? PDFFONT_ENCODING_MS_SYMBOL : PDFFONT_ENCODING_ADOBE_SYMBOL;
        } else if (!bEmbedded && iBaseEncoding == PDFFONT_ENCODING_BUILTIN) {
            iBaseEncoding = PDFFONT_ENCODING_WINANSI;
        }
        return;
    }
    if (pEncoding->GetType() == PDFOBJ_NAME) {
        if (iBaseEncoding == PDFFONT_ENCODING_ADOBE_SYMBOL || iBaseEncoding == PDFFONT_ENCODING_ZAPFDINGBATS) {
            return;
        }
        if ((m_Flags & PDFFONT_SYMBOLIC) && m_BaseFont == FX_BSTRC("Symbol")) {
            if (!bTrueType) {
                iBaseEncoding = PDFFONT_ENCODING_ADOBE_SYMBOL;
            }
            return;
        }
        CFX_ByteString bsEncoding = pEncoding->GetString();
        if (bsEncoding.Compare(FX_BSTRC("MacExpertEncoding")) == 0) {
            bsEncoding = FX_BSTRC("WinAnsiEncoding");
        }
        GetPredefinedEncoding(iBaseEncoding, bsEncoding);
        return;
    }
    if (pEncoding->GetType() != PDFOBJ_DICTIONARY) {
        return;
    }
    CPDF_Dictionary* pDict = (CPDF_Dictionary*)pEncoding;
    if (iBaseEncoding != PDFFONT_ENCODING_ADOBE_SYMBOL && iBaseEncoding != PDFFONT_ENCODING_ZAPFDINGBATS) {
        CFX_ByteString bsEncoding = pDict->GetString(FX_BSTRC("BaseEncoding"));
        if (bsEncoding.Compare(FX_BSTRC("MacExpertEncoding")) == 0 && bTrueType) {
            bsEncoding = FX_BSTRC("WinAnsiEncoding");
        }
        GetPredefinedEncoding(iBaseEncoding, bsEncoding);
    }
    if ((!bEmbedded || bTrueType) && iBaseEncoding == PDFFONT_ENCODING_BUILTIN) {
        iBaseEncoding = PDFFONT_ENCODING_STANDARD;
    }
    CPDF_Array* pDiffs = pDict->GetArray(FX_BSTRC("Differences"));
    if (pDiffs == NULL) {
        return;
    }
    FX_NEW_VECTOR(pCharNames, CFX_ByteString, 256);
    FX_DWORD cur_code = 0;
    for (FX_DWORD i = 0; i < pDiffs->GetCount(); i ++) {
        CPDF_Object* pElement = pDiffs->GetElementValue(i);
        if (pElement == NULL) {
            continue;
        }
        if (pElement->GetType() == PDFOBJ_NAME) {
            if (cur_code < 256) {
                pCharNames[cur_code] = ((CPDF_Name*)pElement)->GetString();
            }
            cur_code ++;
        } else {
            cur_code = pElement->GetInteger();
        }
    }
}
FX_BOOL CPDF_Font::IsStandardFont() const
{
    if (m_FontType != PDFFONT_TYPE1) {
        return FALSE;
    }
    if (m_pFontFile != NULL) {
        return FALSE;
    }
    if (((CPDF_Type1Font*)this)->GetBase14Font() < 0) {
        return FALSE;
    }
    return TRUE;
}
extern FX_LPCSTR PDF_CharNameFromPredefinedCharSet(int encoding, FX_BYTE charcode);
CPDF_SimpleFont::CPDF_SimpleFont()
{
    FXSYS_memset8(m_CharBBox, 0xff, sizeof m_CharBBox);
    FXSYS_memset8(m_CharWidth, 0xff, sizeof m_CharWidth);
    FXSYS_memset8(m_GlyphIndex, 0xff, sizeof m_GlyphIndex);
    FXSYS_memset8(m_ExtGID, 0xff, sizeof m_ExtGID);
    m_pCharNames = NULL;
    m_BaseEncoding = PDFFONT_ENCODING_BUILTIN;
}
CPDF_SimpleFont::~CPDF_SimpleFont()
{
    if (m_pCharNames) {
        FX_DELETE_VECTOR(m_pCharNames, CFX_ByteString, 256);
    }
}
int CPDF_SimpleFont::GlyphFromCharCode(FX_DWORD charcode, FX_BOOL *pVertGlyph)
{
    if (pVertGlyph) {
        *pVertGlyph = FALSE;
    }
    if (charcode > 0xff) {
        return -1;
    }
    int index = m_GlyphIndex[(FX_BYTE)charcode];
    if (index == 0xffff) {
        return -1;
    }
    return index;
}
void CPDF_SimpleFont::LoadCharMetrics(int charcode)
{
    if (m_Font.m_Face == NULL) {
        return;
    }
    if (charcode < 0 || charcode > 0xff) {
        return;
    }
    int glyph_index = m_GlyphIndex[charcode];
    if (glyph_index == 0xffff) {
        if (m_pFontFile == NULL && charcode != 32) {
            LoadCharMetrics(32);
            m_CharBBox[charcode] = m_CharBBox[32];
            if (m_bUseFontWidth) {
                m_CharWidth[charcode] = m_CharWidth[32];
            }
        }
        return;
    }
    int err = FXFT_Load_Glyph(m_Font.m_Face, glyph_index, FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
    if (err) {
        return;
    }
    m_CharBBox[charcode].Left = TT2PDF(FXFT_Get_Glyph_HoriBearingX(m_Font.m_Face), m_Font.m_Face);
    m_CharBBox[charcode].Right = TT2PDF(FXFT_Get_Glyph_HoriBearingX(m_Font.m_Face) + FXFT_Get_Glyph_Width(m_Font.m_Face), m_Font.m_Face);
    m_CharBBox[charcode].Top = TT2PDF(FXFT_Get_Glyph_HoriBearingY(m_Font.m_Face), m_Font.m_Face);
    m_CharBBox[charcode].Bottom = TT2PDF(FXFT_Get_Glyph_HoriBearingY(m_Font.m_Face) - FXFT_Get_Glyph_Height(m_Font.m_Face), m_Font.m_Face);
    if (m_bUseFontWidth) {
        int TT_Width = TT2PDF(FXFT_Get_Glyph_HoriAdvance(m_Font.m_Face), m_Font.m_Face);
        if (m_CharWidth[charcode] == 0xffff) {
            m_CharWidth[charcode] = TT_Width;
        } else if (TT_Width && !IsEmbedded()) {
            m_CharBBox[charcode].Right = m_CharBBox[charcode].Right * m_CharWidth[charcode] / TT_Width;
            m_CharBBox[charcode].Left = m_CharBBox[charcode].Left * m_CharWidth[charcode] / TT_Width;
        }
    }
}
int CPDF_SimpleFont::GetCharWidthF(FX_DWORD charcode, int level)
{
    if (charcode > 0xff) {
        charcode = 0;
    }
    if (m_CharWidth[charcode] == 0xffff) {
        LoadCharMetrics(charcode);
        if (m_CharWidth[charcode] == 0xffff) {
            m_CharWidth[charcode] = 0;
        }
    }
    return (FX_INT16)m_CharWidth[charcode];
}
void CPDF_SimpleFont::GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level)
{
    if (charcode > 0xff) {
        charcode = 0;
    }
    if (m_CharBBox[charcode].Left == (FX_SHORT)0xffff) {
        LoadCharMetrics(charcode);
    }
    rect.left = m_CharBBox[charcode].Left;
    rect.right = m_CharBBox[charcode].Right;
    rect.bottom = m_CharBBox[charcode].Bottom;
    rect.top = m_CharBBox[charcode].Top;
}
FX_LPCSTR GetAdobeCharName(int iBaseEncoding, const CFX_ByteString* pCharNames, int charcode)
{
    ASSERT(charcode >= 0 && charcode < 256);
    if (charcode < 0 || charcode >= 256) {
        return NULL;
    }
    FX_LPCSTR name = NULL;
    if (pCharNames) {
        name = pCharNames[charcode];
    }
    if ((name == NULL || name[0] == 0) && iBaseEncoding) {
        name = PDF_CharNameFromPredefinedCharSet(iBaseEncoding, charcode);
    }
    if (name == NULL || name[0] == 0) {
        return NULL;
    }
    return name;
}
FX_BOOL CPDF_SimpleFont::LoadCommon()
{
    CPDF_Dictionary* pFontDesc = m_pFontDict->GetDict(FX_BSTRC("FontDescriptor"));
    if (pFontDesc) {
        LoadFontDescriptor(pFontDesc);
    }
    CPDF_Array* pWidthArray = m_pFontDict->GetArray(FX_BSTRC("Widths"));
    int width_start = 0, width_end = -1;
    m_bUseFontWidth = TRUE;
    if (pWidthArray) {
        m_bUseFontWidth = FALSE;
        if (pFontDesc && pFontDesc->KeyExist(FX_BSTRC("MissingWidth"))) {
            int MissingWidth = pFontDesc->GetInteger(FX_BSTRC("MissingWidth"));
            for (int i = 0; i < 256; i ++) {
                m_CharWidth[i] = MissingWidth;
            }
        }
        width_start = m_pFontDict->GetInteger(FX_BSTRC("FirstChar"), 0);
        width_end = m_pFontDict->GetInteger(FX_BSTRC("LastChar"), 0);
        if (width_start >= 0 && width_start <= 255) {
            if (width_end <= 0 || width_end >= width_start + (int)pWidthArray->GetCount()) {
                width_end = width_start + pWidthArray->GetCount() - 1;
            }
            if (width_end > 255) {
                width_end = 255;
            }
            for (int i = width_start; i <= width_end; i ++) {
                m_CharWidth[i] = pWidthArray->GetInteger(i - width_start);
            }
        }
    }
    if (m_pFontFile == NULL) {
        LoadSubstFont();
    } else {
        if (m_BaseFont.GetLength() > 8 && m_BaseFont[7] == '+') {
            m_BaseFont = m_BaseFont.Mid(8);
        }
    }
    if (!(m_Flags & PDFFONT_SYMBOLIC)) {
        m_BaseEncoding = PDFFONT_ENCODING_STANDARD;
    }
    CPDF_Object* pEncoding = m_pFontDict->GetElementValue(FX_BSTRC("Encoding"));
    LoadPDFEncoding(pEncoding, m_BaseEncoding, m_pCharNames, m_pFontFile != NULL, m_Font.IsTTFont());
    LoadGlyphMap();
    if (m_pCharNames) {
        FX_DELETE_VECTOR(m_pCharNames, CFX_ByteString, 256);
        m_pCharNames = NULL;
    }
    if (m_Font.m_Face == NULL) {
        return TRUE;
    }
    if (m_Flags & PDFFONT_ALLCAP) {
        unsigned char lowercases[] = {'a', 'z', 0xe0, 0xf6, 0xf8, 0xfd};
        for (size_t range = 0; range < sizeof lowercases / 2; range ++) {
            for (int i = lowercases[range * 2]; i <= lowercases[range * 2 + 1]; i ++) {
                if (m_GlyphIndex[i] != 0xffff && m_pFontFile != NULL) {
                    continue;
                }
                m_GlyphIndex[i] = m_GlyphIndex[i - 32];
                if (m_CharWidth[i - 32]) {
                    m_CharWidth[i] = m_CharWidth[i - 32];
                    m_CharBBox[i] = m_CharBBox[i - 32];
                }
            }
        }
    }
    CheckFontMetrics();
    return TRUE;
}
void CPDF_SimpleFont::LoadSubstFont()
{
    if (!m_bUseFontWidth && !(m_Flags & PDFFONT_FIXEDPITCH)) {
        int width = 0, i;
        for (i = 0; i < 256; i ++) {
            if (m_CharWidth[i] == 0 || m_CharWidth[i] == 0xffff) {
                continue;
            }
            if (width == 0) {
                width = m_CharWidth[i];
            } else if (width != m_CharWidth[i]) {
                break;
            }
        }
        if (i == 256 && width) {
            m_Flags |= PDFFONT_FIXEDPITCH;
        }
    }
    int weight = m_StemV < 140 ? m_StemV * 5 : (m_StemV * 4 + 140);
    m_Font.LoadSubst(m_BaseFont, m_FontType == PDFFONT_TRUETYPE, m_Flags, weight, m_ItalicAngle, 0);
    if (m_Font.m_pSubstFont->m_SubstFlags & FXFONT_SUBST_NONSYMBOL) {
    }
}
FX_BOOL CPDF_SimpleFont::IsUnicodeCompatible() const
{
    return m_BaseEncoding != PDFFONT_ENCODING_BUILTIN && m_BaseEncoding != PDFFONT_ENCODING_ADOBE_SYMBOL &&
           m_BaseEncoding != PDFFONT_ENCODING_ZAPFDINGBATS;
}
CPDF_Type1Font::CPDF_Type1Font()
{
    m_Base14Font = -1;
}
FX_BOOL CPDF_Type1Font::_Load()
{
    m_Base14Font = _PDF_GetStandardFontName(m_BaseFont);
    if (m_Base14Font >= 0) {
        CPDF_Dictionary* pFontDesc = m_pFontDict->GetDict(FX_BSTRC("FontDescriptor"));
        if (pFontDesc && pFontDesc->KeyExist(FX_BSTRC("Flags"))) {
            m_Flags = pFontDesc->GetInteger(FX_BSTRC("Flags"));
        } else {
            m_Flags = m_Base14Font >= 12 ? PDFFONT_SYMBOLIC : PDFFONT_NONSYMBOLIC;
        }
        if (m_Base14Font < 4)
            for (int i = 0; i < 256; i ++) {
                m_CharWidth[i] = 600;
            }
        if (m_Base14Font == 12) {
            m_BaseEncoding = PDFFONT_ENCODING_ADOBE_SYMBOL;
        } else if (m_Base14Font == 13) {
            m_BaseEncoding = PDFFONT_ENCODING_ZAPFDINGBATS;
        } else if (m_Flags & PDFFONT_NONSYMBOLIC) {
            m_BaseEncoding = PDFFONT_ENCODING_STANDARD;
        }
    }
    return LoadCommon();
}
static FX_BOOL FT_UseType1Charmap(FXFT_Face face)
{
    if (FXFT_Get_Face_CharmapCount(face) == 0) {
        return FALSE;
    }
    if (FXFT_Get_Face_CharmapCount(face) == 1 &&
            FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[0]) == FXFT_ENCODING_UNICODE) {
        return FALSE;
    }
    if (FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[0]) == FXFT_ENCODING_UNICODE) {
        FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[1]);
    } else {
        FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[0]);
    }
    return TRUE;
}
extern FX_WCHAR FT_UnicodeFromCharCode(int encoding, FX_DWORD charcode);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
#include "../../fxge/apple/apple_int.h"
#endif
int CPDF_Type1Font::GlyphFromCharCodeExt(FX_DWORD charcode)
{
    if (charcode > 0xff) {
        return -1;
    }
    int index = m_ExtGID[(FX_BYTE)charcode];
    if (index == 0xffff) {
        return -1;
    }
    return index;
}
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
struct _GlyphNameMap {
    FX_LPCSTR m_pStrAdobe;
    FX_LPCSTR m_pStrUnicode;
};
static const _GlyphNameMap g_GlyphNameSubsts[] = {
    {"ff", "uniFB00"},
    {"fi", "uniFB01"},
    {"fl", "uniFB02"},
    {"ffi", "uniFB03"},
    {"ffl", "uniFB04"}
};
extern "C" {
    static int compareString(const void* key, const void* element)
    {
        return FXSYS_stricmp((FX_LPCSTR)key, ((_GlyphNameMap*)element)->m_pStrAdobe);
    }
}
static FX_LPCSTR _GlyphNameRemap(FX_LPCSTR pStrAdobe)
{
    _GlyphNameMap* found = (_GlyphNameMap*)FXSYS_bsearch(pStrAdobe, g_GlyphNameSubsts,
                           sizeof g_GlyphNameSubsts / sizeof(_GlyphNameMap), sizeof(_GlyphNameMap),
                           compareString);
    if (found) {
        return found->m_pStrUnicode;
    }
    return NULL;
}
#endif
void CPDF_Type1Font::LoadGlyphMap()
{
    if (m_Font.m_Face == NULL) {
        return;
    }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    FX_BOOL bCoreText = TRUE;
    CQuartz2D & quartz2d = ((CApplePlatform *) CFX_GEModule::Get()->GetPlatformData())->_quartz2d;
    if (!m_Font.m_pPlatformFont) {
        if (m_Font.GetPsName() == CFX_WideString::FromLocal("DFHeiStd-W5")) {
            bCoreText = FALSE;
        }
        m_Font.m_pPlatformFont = quartz2d.CreateFont(m_Font.m_pFontData, m_Font.m_dwSize);
        if (NULL == m_Font.m_pPlatformFont) {
            bCoreText = FALSE;
        }
    }
#endif
    if (!IsEmbedded() && (m_Base14Font < 12) && m_Font.IsTTFont()) {
        if (FT_UseTTCharmap(m_Font.m_Face, 3, 0)) {
            FX_BOOL bGotOne = FALSE;
            for (int charcode = 0; charcode < 256; charcode ++) {
                const FX_BYTE prefix[4] = {0x00, 0xf0, 0xf1, 0xf2};
                for (int j = 0; j < 4; j ++) {
                    FX_WORD unicode = prefix[j] * 256 + charcode;
                    m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, unicode);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                    FX_CHAR name_glyph[256];
                    FXFT_Get_Glyph_Name(m_Font.m_Face, m_GlyphIndex[charcode], name_glyph, 256);
                    name_glyph[255] = 0;
                    CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII, kCFAllocatorNull);
                    m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
                    if (name_ct) {
                        CFRelease(name_ct);
                    }
#endif
                    if (m_GlyphIndex[charcode]) {
                        bGotOne = TRUE;
                        break;
                    }
                }
            }
            if (bGotOne) {
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                if (!bCoreText) {
                    FXSYS_memcpy32(m_ExtGID, m_GlyphIndex, 256);
                }
#endif
                return;
            }
        }
        FXFT_Select_Charmap(m_Font.m_Face, FXFT_ENCODING_UNICODE);
        if (m_BaseEncoding == 0) {
            m_BaseEncoding = PDFFONT_ENCODING_STANDARD;
        }
        for (int charcode = 0; charcode < 256; charcode ++) {
            FX_LPCSTR name = GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
            if (name == NULL) {
                continue;
            }
            m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
            m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, m_Encoding.m_Unicodes[charcode]);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
            FX_CHAR name_glyph[256];
            FXFT_Get_Glyph_Name(m_Font.m_Face, m_GlyphIndex[charcode], name_glyph, 256);
            name_glyph[255] = 0;
            CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII, kCFAllocatorNull);
            m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
            if (name_ct) {
                CFRelease(name_ct);
            }
#endif
            if (m_GlyphIndex[charcode] == 0 && FXSYS_strcmp(name, ".notdef") == 0) {
                m_Encoding.m_Unicodes[charcode] = 0x20;
                m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, 0x20);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                FX_CHAR name_glyph[256];
                FXFT_Get_Glyph_Name(m_Font.m_Face, m_GlyphIndex[charcode], name_glyph, 256);
                name_glyph[255] = 0;
                CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII, kCFAllocatorNull);
                m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
                if (name_ct) {
                    CFRelease(name_ct);
                }
#endif
            }
        }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
        if (!bCoreText) {
            FXSYS_memcpy32(m_ExtGID, m_GlyphIndex, 256);
        }
#endif
        return;
    }
    FT_UseType1Charmap(m_Font.m_Face);
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    if (bCoreText) {
        if (m_Flags & PDFFONT_SYMBOLIC) {
            for (int charcode = 0; charcode < 256; charcode ++) {
                FX_LPCSTR name = GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
                if (name) {
                    m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
                    m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.m_Face, (char*)name);
                    CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name, kCFStringEncodingASCII, kCFAllocatorNull);
                    m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
                    if (name_ct) {
                        CFRelease(name_ct);
                    }
                } else {
                    m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, charcode);
                    FX_WCHAR unicode = 0;
                    if (m_GlyphIndex[charcode]) {
                        unicode = FT_UnicodeFromCharCode(PDFFONT_ENCODING_STANDARD, charcode);
                    }
                    FX_CHAR name_glyph[256];
                    FXSYS_memset32(name_glyph, 0, sizeof(name_glyph));
                    FXFT_Get_Glyph_Name(m_Font.m_Face, m_GlyphIndex[charcode], name_glyph, 256);
                    name_glyph[255] = 0;
                    if (unicode == 0 && name_glyph[0] != 0) {
                        unicode = PDF_UnicodeFromAdobeName(name_glyph);
                    }
                    m_Encoding.m_Unicodes[charcode] = unicode;
                    CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII, kCFAllocatorNull);
                    m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
                    if (name_ct) {
                        CFRelease(name_ct);
                    }
                }
            }
            return;
        }
        FX_BOOL bUnicode = FALSE;
        if (0 == FXFT_Select_Charmap(m_Font.m_Face, FXFT_ENCODING_UNICODE)) {
            bUnicode = TRUE;
        }
        for (int charcode = 0; charcode < 256; charcode ++) {
            FX_LPCSTR name = GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
            if (name == NULL) {
                continue;
            }
            m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
            FX_LPCSTR pStrUnicode = _GlyphNameRemap(name);
            if (pStrUnicode && 0 == FXFT_Get_Name_Index(m_Font.m_Face, (char*)name)) {
                name = pStrUnicode;
            }
            m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.m_Face, (char*)name);
            CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name, kCFStringEncodingASCII, kCFAllocatorNull);
            m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
            if (name_ct) {
                CFRelease(name_ct);
            }
            if (m_GlyphIndex[charcode] == 0) {
                if (FXSYS_strcmp(name, ".notdef") != 0 && FXSYS_strcmp(name, "space") != 0) {
                    m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, bUnicode ? m_Encoding.m_Unicodes[charcode] : charcode);
                    FX_CHAR name_glyph[256];
                    FXFT_Get_Glyph_Name(m_Font.m_Face, m_GlyphIndex[charcode], name_glyph, 256);
                    name_glyph[255] = 0;
                    CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII, kCFAllocatorNull);
                    m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
                    if (name_ct) {
                        CFRelease(name_ct);
                    }
                } else {
                    m_Encoding.m_Unicodes[charcode] = 0x20;
                    m_GlyphIndex[charcode] = bUnicode ? FXFT_Get_Char_Index(m_Font.m_Face, 0x20) : 0xffff;
                    FX_CHAR name_glyph[256];
                    FXFT_Get_Glyph_Name(m_Font.m_Face, m_GlyphIndex[charcode], name_glyph, 256);
                    name_glyph[255] = 0;
                    CFStringRef name_ct = CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII, kCFAllocatorNull);
                    m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName((CGFontRef)m_Font.m_pPlatformFont, name_ct);
                    if (name_ct) {
                        CFRelease(name_ct);
                    }
                }
            }
        }
        return;
    }
#endif
    if (m_Flags & PDFFONT_SYMBOLIC) {
        for (int charcode = 0; charcode < 256; charcode ++) {
            FX_LPCSTR name = GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
            if (name) {
                m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
                m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.m_Face, (char*)name);
            } else {
                m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, charcode);
                if (m_GlyphIndex[charcode]) {
                    FX_WCHAR unicode = FT_UnicodeFromCharCode(PDFFONT_ENCODING_STANDARD, charcode);
                    if (unicode == 0) {
                        FX_CHAR name_glyph[256];
                        FXSYS_memset32(name_glyph, 0, sizeof(name_glyph));
                        FXFT_Get_Glyph_Name(m_Font.m_Face, m_GlyphIndex[charcode], name_glyph, 256);
                        name_glyph[255] = 0;
                        if (name_glyph[0] != 0) {
                            unicode = PDF_UnicodeFromAdobeName(name_glyph);
                        }
                    }
                    m_Encoding.m_Unicodes[charcode] = unicode;
                }
            }
        }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
        if (!bCoreText) {
            FXSYS_memcpy32(m_ExtGID, m_GlyphIndex, 256);
        }
#endif
        return;
    }
    FX_BOOL bUnicode = FALSE;
    if (0 == FXFT_Select_Charmap(m_Font.m_Face, FXFT_ENCODING_UNICODE)) {
        bUnicode = TRUE;
    }
    for (int charcode = 0; charcode < 256; charcode ++) {
        FX_LPCSTR name = GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
        if (name == NULL) {
            continue;
        }
        m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
        m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.m_Face, (char*)name);
        if (m_GlyphIndex[charcode] == 0) {
            if (FXSYS_strcmp(name, ".notdef") != 0 && FXSYS_strcmp(name, "space") != 0) {
                m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, bUnicode ? m_Encoding.m_Unicodes[charcode] : charcode);
            } else {
                m_Encoding.m_Unicodes[charcode] = 0x20;
                m_GlyphIndex[charcode] = 0xffff;
            }
        }
    }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    if (!bCoreText) {
        FXSYS_memcpy32(m_ExtGID, m_GlyphIndex, 256);
    }
#endif
}
CPDF_FontEncoding::CPDF_FontEncoding()
{
    FXSYS_memset32(m_Unicodes, 0, sizeof(m_Unicodes));
}
int CPDF_FontEncoding::CharCodeFromUnicode(FX_WCHAR unicode) const
{
    for (int i = 0; i < 256; i ++)
        if (m_Unicodes[i] == unicode) {
            return i;
        }
    return -1;
}
CPDF_FontEncoding::CPDF_FontEncoding(int PredefinedEncoding)
{
    const FX_WORD* pSrc = PDF_UnicodesForPredefinedCharSet(PredefinedEncoding);
    if (!pSrc) {
        FXSYS_memset32(m_Unicodes, 0, sizeof(m_Unicodes));
    } else
        for (int i = 0; i < 256; i++) {
            m_Unicodes[i] = pSrc[i];
        }
}
FX_BOOL CPDF_FontEncoding::IsIdentical(CPDF_FontEncoding* pAnother) const
{
    return FXSYS_memcmp32(m_Unicodes, pAnother->m_Unicodes, sizeof(m_Unicodes)) == 0;
}
CPDF_Object* CPDF_FontEncoding::Realize()
{
    int predefined = 0;
    for (int cs = PDFFONT_ENCODING_WINANSI; cs < PDFFONT_ENCODING_ZAPFDINGBATS; cs ++) {
        const FX_WORD* pSrc = PDF_UnicodesForPredefinedCharSet(cs);
        FX_BOOL match = TRUE;
        for (int i = 0; i < 256; ++i) {
            if (m_Unicodes[i] != pSrc[i]) {
                match = FALSE;
                break;
            }
        }
        if (match) {
            predefined = cs;
            break;
        }
    }
    if (predefined) {
        if (predefined == PDFFONT_ENCODING_WINANSI) {
            return CPDF_Name::Create("WinAnsiEncoding");
        }
        if (predefined == PDFFONT_ENCODING_MACROMAN) {
            return CPDF_Name::Create("MacRomanEncoding");
        }
        if (predefined == PDFFONT_ENCODING_MACEXPERT) {
            return CPDF_Name::Create("MacExpertEncoding");
        }
        return NULL;
    }
    CPDF_Dictionary* pDict = CPDF_Dictionary::Create();
    pDict->SetAtName(FX_BSTRC("BaseEncoding"), FX_BSTRC("WinAnsiEncoding"));
    const FX_WORD* pStandard = PDF_UnicodesForPredefinedCharSet(PDFFONT_ENCODING_WINANSI);
    CPDF_Array* pDiff = CPDF_Array::Create();
    for (int i = 0; i < 256; i ++) {
        if (pStandard[i] == m_Unicodes[i]) {
            continue;
        }
        pDiff->Add(CPDF_Number::Create(i));
        pDiff->Add(CPDF_Name::Create(PDF_AdobeNameFromUnicode(m_Unicodes[i])));
    }
    pDict->SetAt(FX_BSTRC("Differences"), pDiff);
    return pDict;
}
CPDF_TrueTypeFont::CPDF_TrueTypeFont()
{
}
FX_BOOL CPDF_TrueTypeFont::_Load()
{
    return LoadCommon();
}
extern FX_DWORD FT_CharCodeFromUnicode(int encoding, FX_WCHAR unicode);
void CPDF_TrueTypeFont::LoadGlyphMap()
{
    if (m_Font.m_Face == NULL) {
        return;
    }
    int baseEncoding = m_BaseEncoding;
    if (m_pFontFile && m_Font.m_Face->num_charmaps > 0
            && (baseEncoding == PDFFONT_ENCODING_MACROMAN || baseEncoding == PDFFONT_ENCODING_WINANSI)
            && (m_Flags & PDFFONT_SYMBOLIC)) {
        FX_BOOL bSupportWin = FALSE;
        FX_BOOL bSupportMac = FALSE;
        for (int i = 0; i < FXFT_Get_Face_CharmapCount(m_Font.m_Face); i++) {
            int platform_id = FXFT_Get_Charmap_PlatformID(FXFT_Get_Face_Charmaps(m_Font.m_Face)[i]);
            if (platform_id == 0 || platform_id == 3) {
                bSupportWin = TRUE;
            } else if (platform_id == 0 || platform_id == 1) {
                bSupportMac = TRUE;
            }
        }
        if (baseEncoding == PDFFONT_ENCODING_WINANSI && !bSupportWin) {
            baseEncoding = bSupportMac ? PDFFONT_ENCODING_MACROMAN : PDFFONT_ENCODING_BUILTIN;
        } else if (baseEncoding == PDFFONT_ENCODING_MACROMAN && !bSupportMac) {
            baseEncoding = bSupportWin ? PDFFONT_ENCODING_WINANSI : PDFFONT_ENCODING_BUILTIN;
        }
    }
    if (((baseEncoding == PDFFONT_ENCODING_MACROMAN || baseEncoding == PDFFONT_ENCODING_WINANSI)
            && m_pCharNames == NULL) || (m_Flags & PDFFONT_NONSYMBOLIC)) {
        if (!FXFT_Has_Glyph_Names(m_Font.m_Face) && (!m_Font.m_Face->num_charmaps || !m_Font.m_Face->charmaps)) {
            int nStartChar = m_pFontDict->GetInteger(FX_BSTRC("FirstChar"));
            if(nStartChar < 0 || nStartChar > 255)
                return;

            int charcode = 0;
            for (; charcode < nStartChar; charcode ++) {
                m_GlyphIndex[charcode] = 0;
            }
            FX_WORD nGlyph = charcode - nStartChar + 3;
            for (; charcode < 256; charcode ++, nGlyph ++) {
                m_GlyphIndex[charcode] = nGlyph;
            }
            return;
        }
        FX_BOOL bMSUnicode = FT_UseTTCharmap(m_Font.m_Face, 3, 1);
        FX_BOOL bMacRoman = FALSE, bMSSymbol = FALSE;
        if (!bMSUnicode) {
            if (m_Flags & PDFFONT_NONSYMBOLIC) {
                bMacRoman = FT_UseTTCharmap(m_Font.m_Face, 1, 0);
                bMSSymbol = !bMacRoman && FT_UseTTCharmap(m_Font.m_Face, 3, 0);
            } else {
                bMSSymbol = FT_UseTTCharmap(m_Font.m_Face, 3, 0);
                bMacRoman = !bMSSymbol && FT_UseTTCharmap(m_Font.m_Face, 1, 0);
            }
        }
        FX_BOOL bToUnicode = m_pFontDict->KeyExist(FX_BSTRC("ToUnicode"));
        for (int charcode = 0; charcode < 256; charcode ++) {
            FX_LPCSTR name = GetAdobeCharName(baseEncoding, m_pCharNames, charcode);
            if (name == NULL) {
                m_GlyphIndex[charcode] = m_pFontFile ? FXFT_Get_Char_Index(m_Font.m_Face, charcode) : -1;
                continue;
            }
            m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
            if (bMSSymbol) {
                const FX_BYTE prefix[4] = {0x00, 0xf0, 0xf1, 0xf2};
                for (int j = 0; j < 4; j ++) {
                    FX_WORD unicode = prefix[j] * 256 + charcode;
                    m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, unicode);
                    if (m_GlyphIndex[charcode]) {
                        break;
                    }
                }
            } else if (m_Encoding.m_Unicodes[charcode]) {
                if (bMSUnicode) {
                    m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, m_Encoding.m_Unicodes[charcode]);
                } else if (bMacRoman) {
                    FX_DWORD maccode = FT_CharCodeFromUnicode(FXFT_ENCODING_APPLE_ROMAN, m_Encoding.m_Unicodes[charcode]);
                    if (!maccode) {
                        m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.m_Face, (char *)name);
                    } else {
                        m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, maccode);
                    }
                }
            }
            if ((m_GlyphIndex[charcode] == 0 || m_GlyphIndex[charcode] == 0xffff) && name != NULL) {
                if (name[0] == '.' && FXSYS_strcmp(name, ".notdef") == 0) {
                    m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, 32);
                } else {
                    m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.m_Face, (char*)name);
                    if (m_GlyphIndex[charcode] == 0) {
                        if (bToUnicode) {
                            CFX_WideString wsUnicode = UnicodeFromCharCode(charcode);
                            if (!wsUnicode.IsEmpty()) {
                                m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, wsUnicode[0]);
                                m_Encoding.m_Unicodes[charcode] = wsUnicode[0];
                            }
                        }
                        if (m_GlyphIndex[charcode] == 0) {
                            m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, charcode);
                        }
                    }
                }
            }
        }
        return;
    }
    if (FT_UseTTCharmap(m_Font.m_Face, 3, 0)) {
        const FX_BYTE prefix[4] = {0x00, 0xf0, 0xf1, 0xf2};
        FX_BOOL bGotOne = FALSE;
        for (int charcode = 0; charcode < 256; charcode ++) {
            for (int j = 0; j < 4; j ++) {
                FX_WORD unicode = prefix[j] * 256 + charcode;
                m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, unicode);
                if (m_GlyphIndex[charcode]) {
                    bGotOne = TRUE;
                    break;
                }
            }
        }
        if (bGotOne) {
            if (baseEncoding != PDFFONT_ENCODING_BUILTIN) {
                for (int charcode = 0; charcode < 256; charcode ++) {
                    FX_LPCSTR name = GetAdobeCharName(baseEncoding, m_pCharNames, charcode);
                    if (name == NULL) {
                        continue;
                    }
                    m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
                }
            } else if (FT_UseTTCharmap(m_Font.m_Face, 1, 0)) {
                for (int charcode = 0; charcode < 256; charcode ++) {
                    m_Encoding.m_Unicodes[charcode] = FT_UnicodeFromCharCode(FXFT_ENCODING_APPLE_ROMAN, charcode);
                }
            }
            return;
        }
    }
    if (FT_UseTTCharmap(m_Font.m_Face, 1, 0)) {
        FX_BOOL bGotOne = FALSE;
        for (int charcode = 0; charcode < 256; charcode ++) {
            m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, charcode);
            m_Encoding.m_Unicodes[charcode] = FT_UnicodeFromCharCode(FXFT_ENCODING_APPLE_ROMAN, charcode);
            if (m_GlyphIndex[charcode]) {
                bGotOne = TRUE;
            }
        }
        if (m_pFontFile || bGotOne) {
            return;
        }
    }
    if (FXFT_Select_Charmap(m_Font.m_Face, FXFT_ENCODING_UNICODE) == 0) {
        FX_BOOL bGotOne = FALSE;
        const FX_WORD* pUnicodes = PDF_UnicodesForPredefinedCharSet(baseEncoding);
        for (int charcode = 0; charcode < 256; charcode ++) {
            if (m_pFontFile == NULL) {
                FX_LPCSTR name = GetAdobeCharName(0, m_pCharNames, charcode);
                if (name != NULL) {
                    m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
                } else if (pUnicodes) {
                    m_Encoding.m_Unicodes[charcode] = pUnicodes[charcode];
                }
            } else {
                m_Encoding.m_Unicodes[charcode] = charcode;
            }
            m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.m_Face, m_Encoding.m_Unicodes[charcode]);
            if (m_GlyphIndex[charcode]) {
                bGotOne = TRUE;
            }
        }
        if (bGotOne) {
            return;
        }
    }
    for (int charcode = 0; charcode < 256; charcode ++) {
        m_GlyphIndex[charcode] = charcode;
    }
}
CPDF_Type3Font::CPDF_Type3Font()
{
    m_pPageResources = NULL;
    FXSYS_memset32(m_CharWidthL, 0, sizeof m_CharWidthL);
}
CPDF_Type3Font::~CPDF_Type3Font()
{
    FX_POSITION pos = m_CacheMap.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_CacheMap.GetNextAssoc(pos, key, value);
        delete (CPDF_Type3Char*)value;
    }
    m_CacheMap.RemoveAll();
    pos = m_DeletedMap.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_DeletedMap.GetNextAssoc(pos, key, value);
        delete (CPDF_Type3Char*)key;
    }
}
FX_BOOL CPDF_Type3Font::_Load()
{
    m_pFontResources = m_pFontDict->GetDict(FX_BSTRC("Resources"));
    CPDF_Array* pMatrix = m_pFontDict->GetArray(FX_BSTRC("FontMatrix"));
    FX_FLOAT xscale = 1.0f, yscale = 1.0f;
    if (pMatrix) {
        m_FontMatrix = pMatrix->GetMatrix();
        xscale = m_FontMatrix.a;
        yscale = m_FontMatrix.d;
    }
    CPDF_Array* pBBox = m_pFontDict->GetArray(FX_BSTRC("FontBBox"));
    if (pBBox) {
        m_FontBBox.left = (FX_INT32)(FXSYS_Mul(pBBox->GetNumber(0), xscale) * 1000);
        m_FontBBox.bottom = (FX_INT32)(FXSYS_Mul(pBBox->GetNumber(1), yscale) * 1000);
        m_FontBBox.right = (FX_INT32)(FXSYS_Mul(pBBox->GetNumber(2), xscale) * 1000);
        m_FontBBox.top = (FX_INT32)(FXSYS_Mul(pBBox->GetNumber(3), yscale) * 1000);
    }
    int StartChar = m_pFontDict->GetInteger(FX_BSTRC("FirstChar"));
    CPDF_Array* pWidthArray = m_pFontDict->GetArray(FX_BSTRC("Widths"));
    if (pWidthArray && (StartChar >= 0 && StartChar < 256)) {
        FX_DWORD count = pWidthArray->GetCount();
        if (count > 256) {
            count = 256;
        }
        if (StartChar + count > 256) {
            count = 256 - StartChar;
        }
        for (FX_DWORD i = 0; i < count; i ++) {
            m_CharWidthL[StartChar + i] = FXSYS_round(FXSYS_Mul(pWidthArray->GetNumber(i), xscale) * 1000);
        }
    }
    m_pCharProcs = m_pFontDict->GetDict(FX_BSTRC("CharProcs"));
    CPDF_Object* pEncoding = m_pFontDict->GetElementValue(FX_BSTRC("Encoding"));
    if (pEncoding) {
        LoadPDFEncoding(pEncoding, m_BaseEncoding, m_pCharNames, FALSE, FALSE);
        if (m_pCharNames) {
            for (int i = 0; i < 256; i ++) {
                m_Encoding.m_Unicodes[i] = PDF_UnicodeFromAdobeName(m_pCharNames[i]);
                if (m_Encoding.m_Unicodes[i] == 0) {
                    m_Encoding.m_Unicodes[i] = i;
                }
            }
        }
    }
    return TRUE;
}
void CPDF_Type3Font::CheckType3FontMetrics()
{
    CheckFontMetrics();
}
CPDF_Type3Char* CPDF_Type3Font::LoadChar(FX_DWORD charcode, int level)
{
    if (level >= _FPDF_MAX_TYPE3_FORM_LEVEL_) {
        return NULL;
    }
    CPDF_Type3Char* pChar = NULL;
    if (m_CacheMap.Lookup((FX_LPVOID)(FX_UINTPTR)charcode, (FX_LPVOID&)pChar)) {
        if (pChar->m_bPageRequired && m_pPageResources) {
            delete pChar;
            m_CacheMap.RemoveKey((FX_LPVOID)(FX_UINTPTR)charcode);
            return LoadChar(charcode, level + 1);
        }
        return pChar;
    }
    FX_LPCSTR name = GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
    if (name == NULL) {
        return NULL;
    }
    CPDF_Stream* pStream = (CPDF_Stream*)(m_pCharProcs ? m_pCharProcs->GetElementValue(name) : NULL);
    if (pStream == NULL || pStream->GetType() != PDFOBJ_STREAM) {
        return NULL;
    }
    pChar = FX_NEW CPDF_Type3Char;
    pChar->m_pForm = FX_NEW CPDF_Form(m_pDocument, m_pFontResources ? m_pFontResources : m_pPageResources, pStream, NULL);
    pChar->m_pForm->ParseContent(NULL, NULL, pChar, NULL, level + 1);
    FX_FLOAT scale = m_FontMatrix.GetXUnit();
    pChar->m_Width = (FX_INT32)(pChar->m_Width * scale + 0.5f);
    FX_RECT &rcBBox = pChar->m_BBox;
    CFX_FloatRect char_rect((FX_FLOAT)rcBBox.left / 1000.0f, (FX_FLOAT)rcBBox.bottom / 1000.0f,
                            (FX_FLOAT)rcBBox.right / 1000.0f, (FX_FLOAT)rcBBox.top / 1000.0f);
    if (rcBBox.right <= rcBBox.left || rcBBox.bottom >= rcBBox.top) {
        char_rect = pChar->m_pForm->CalcBoundingBox();
    }
    char_rect.Transform(&m_FontMatrix);
    rcBBox.left = FXSYS_round(char_rect.left * 1000);
    rcBBox.right = FXSYS_round(char_rect.right * 1000);
    rcBBox.top = FXSYS_round(char_rect.top * 1000);
    rcBBox.bottom = FXSYS_round(char_rect.bottom * 1000);
    m_CacheMap.SetAt((FX_LPVOID)(FX_UINTPTR)charcode, pChar);
    if (pChar->m_pForm->CountObjects() == 0) {
        delete pChar->m_pForm;
        pChar->m_pForm = NULL;
    }
    return pChar;
}
int CPDF_Type3Font::GetCharWidthF(FX_DWORD charcode, int level)
{
    if (charcode > 0xff) {
        charcode = 0;
    }
    if (m_CharWidthL[charcode]) {
        return m_CharWidthL[charcode];
    }
    CPDF_Type3Char* pChar = LoadChar(charcode, level);
    if (pChar == NULL) {
        return 0;
    }
    return pChar->m_Width;
}
void CPDF_Type3Font::GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level)
{
    CPDF_Type3Char* pChar = LoadChar(charcode, level);
    if (pChar == NULL) {
        rect.left = rect.right = rect.top = rect.bottom = 0;
        return;
    }
    rect = pChar->m_BBox;
}
CPDF_Type3Char::CPDF_Type3Char()
{
    m_pForm = NULL;
    m_pBitmap = NULL;
    m_bPageRequired = FALSE;
    m_bColored = FALSE;
}
CPDF_Type3Char::~CPDF_Type3Char()
{
    if (m_pForm) {
        delete m_pForm;
    }
    if (m_pBitmap) {
        delete m_pBitmap;
    }
}
