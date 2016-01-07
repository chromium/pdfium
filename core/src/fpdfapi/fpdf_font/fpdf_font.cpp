// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "font_int.h"

#include "core/src/fpdfapi/fpdf_page/pageint.h"
#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fpdfapi/fpdf_pageobj.h"
#include "core/include/fpdfapi/fpdf_resource.h"
#include "core/include/fxcrt/fx_ext.h"
#include "core/include/fxge/fx_freetype.h"
#include "third_party/base/stl_util.h"

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
#include "core/src/fxge/apple/apple_int.h"
#endif

FX_BOOL FT_UseTTCharmap(FXFT_Face face, int platform_id, int encoding_id) {
  for (int i = 0; i < FXFT_Get_Face_CharmapCount(face); i++) {
    if (FXFT_Get_Charmap_PlatformID(FXFT_Get_Face_Charmaps(face)[i]) ==
            platform_id &&
        FXFT_Get_Charmap_EncodingID(FXFT_Get_Face_Charmaps(face)[i]) ==
            encoding_id) {
      FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[i]);
      return TRUE;
    }
  }
  return FALSE;
}

CFX_StockFontArray::CFX_StockFontArray() {}

CFX_StockFontArray::~CFX_StockFontArray() {
  for (size_t i = 0; i < FX_ArraySize(m_StockFonts); ++i) {
    if (!m_StockFonts[i])
      continue;
    CPDF_Dictionary* pFontDict = m_StockFonts[i]->GetFontDict();
    if (pFontDict)
      pFontDict->Release();
  }
}

CPDF_Font* CFX_StockFontArray::GetFont(int index) const {
  if (index < 0 || index >= FX_ArraySize(m_StockFonts))
    return nullptr;
  return m_StockFonts[index].get();
}

void CFX_StockFontArray::SetFont(int index, CPDF_Font* font) {
  if (index < 0 || index >= FX_ArraySize(m_StockFonts))
    return;
  m_StockFonts[index].reset(font);
}

CPDF_FontGlobals::CPDF_FontGlobals() {
  FXSYS_memset(m_EmbeddedCharsets, 0, sizeof(m_EmbeddedCharsets));
  FXSYS_memset(m_EmbeddedToUnicodes, 0, sizeof(m_EmbeddedToUnicodes));
}

CPDF_FontGlobals::~CPDF_FontGlobals() {
}

CPDF_Font* CPDF_FontGlobals::Find(CPDF_Document* pDoc, int index) {
  auto it = m_StockMap.find(pDoc);
  if (it == m_StockMap.end())
    return nullptr;
  return it->second ? it->second->GetFont(index) : nullptr;
}

void CPDF_FontGlobals::Set(CPDF_Document* pDoc, int index, CPDF_Font* pFont) {
  if (!pdfium::ContainsKey(m_StockMap, pDoc))
    m_StockMap[pDoc].reset(new CFX_StockFontArray);
  m_StockMap[pDoc]->SetFont(index, pFont);
}

void CPDF_FontGlobals::Clear(CPDF_Document* pDoc) {
  m_StockMap.erase(pDoc);
}

CPDF_Font::CPDF_Font(int fonttype) : m_FontType(fonttype) {
  m_FontBBox.left = m_FontBBox.right = m_FontBBox.top = m_FontBBox.bottom = 0;
  m_StemV = m_Ascent = m_Descent = m_ItalicAngle = 0;
  m_pFontFile = NULL;
  m_Flags = 0;
  m_pToUnicodeMap = NULL;
  m_bToUnicodeLoaded = FALSE;
  m_pCharMap = new CPDF_FontCharMap(this);
}
CPDF_Font::~CPDF_Font() {
  delete m_pCharMap;
  m_pCharMap = NULL;

  delete m_pToUnicodeMap;
  m_pToUnicodeMap = NULL;

  if (m_pFontFile) {
    m_pDocument->GetPageData()->ReleaseFontFileStreamAcc(
        const_cast<CPDF_Stream*>(m_pFontFile->GetStream()->AsStream()));
  }
}
FX_BOOL CPDF_Font::IsVertWriting() const {
  FX_BOOL bVertWriting = FALSE;
  CPDF_CIDFont* pCIDFont = GetCIDFont();
  if (pCIDFont) {
    bVertWriting = pCIDFont->IsVertWriting();
  } else {
    bVertWriting = m_Font.IsVertical();
  }
  return bVertWriting;
}
CFX_ByteString CPDF_Font::GetFontTypeName() const {
  switch (m_FontType) {
    case PDFFONT_TYPE1:
      return "Type1";
    case PDFFONT_TRUETYPE:
      return "TrueType";
    case PDFFONT_TYPE3:
      return "Type3";
    case PDFFONT_CIDFONT:
      return "Type0";
  }
  return CFX_ByteString();
}
void CPDF_Font::AppendChar(CFX_ByteString& str, FX_DWORD charcode) const {
  char buf[4];
  int len = AppendChar(buf, charcode);
  if (len == 1) {
    str += buf[0];
  } else {
    str += CFX_ByteString(buf, len);
  }
}
CFX_WideString CPDF_Font::UnicodeFromCharCode(FX_DWORD charcode) const {
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
FX_DWORD CPDF_Font::CharCodeFromUnicode(FX_WCHAR unicode) const {
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
CFX_WideString CPDF_Font::DecodeString(const CFX_ByteString& str) const {
  CFX_WideString result;
  int src_len = str.GetLength();
  result.Reserve(src_len);
  const FX_CHAR* src_buf = str;
  int src_pos = 0;
  while (src_pos < src_len) {
    FX_DWORD charcode = GetNextChar(src_buf, src_len, src_pos);
    CFX_WideString unicode = UnicodeFromCharCode(charcode);
    if (!unicode.IsEmpty()) {
      result += unicode;
    } else {
      result += (FX_WCHAR)charcode;
    }
  }
  return result;
}
CFX_ByteString CPDF_Font::EncodeString(const CFX_WideString& str) const {
  CFX_ByteString result;
  int src_len = str.GetLength();
  FX_CHAR* dest_buf = result.GetBuffer(src_len * 2);
  const FX_WCHAR* src_buf = str.c_str();
  int dest_pos = 0;
  for (int src_pos = 0; src_pos < src_len; src_pos++) {
    FX_DWORD charcode = CharCodeFromUnicode(src_buf[src_pos]);
    dest_pos += AppendChar(dest_buf + dest_pos, charcode);
  }
  result.ReleaseBuffer(dest_pos);
  return result;
}

void CPDF_Font::LoadFontDescriptor(CPDF_Dictionary* pFontDesc) {
  m_Flags = pFontDesc->GetInteger("Flags", PDFFONT_NONSYMBOLIC);
  int ItalicAngle = 0;
  FX_BOOL bExistItalicAngle = FALSE;
  if (pFontDesc->KeyExist("ItalicAngle")) {
    ItalicAngle = pFontDesc->GetInteger("ItalicAngle");
    bExistItalicAngle = TRUE;
  }
  if (ItalicAngle < 0) {
    m_Flags |= PDFFONT_ITALIC;
    m_ItalicAngle = ItalicAngle;
  }
  FX_BOOL bExistStemV = FALSE;
  if (pFontDesc->KeyExist("StemV")) {
    m_StemV = pFontDesc->GetInteger("StemV");
    bExistStemV = TRUE;
  }
  FX_BOOL bExistAscent = FALSE;
  if (pFontDesc->KeyExist("Ascent")) {
    m_Ascent = pFontDesc->GetInteger("Ascent");
    bExistAscent = TRUE;
  }
  FX_BOOL bExistDescent = FALSE;
  if (pFontDesc->KeyExist("Descent")) {
    m_Descent = pFontDesc->GetInteger("Descent");
    bExistDescent = TRUE;
  }
  FX_BOOL bExistCapHeight = FALSE;
  if (pFontDesc->KeyExist("CapHeight")) {
    bExistCapHeight = TRUE;
  }
  if (bExistItalicAngle && bExistAscent && bExistCapHeight && bExistDescent &&
      bExistStemV) {
    m_Flags |= PDFFONT_USEEXTERNATTR;
  }
  if (m_Descent > 10) {
    m_Descent = -m_Descent;
  }
  CPDF_Array* pBBox = pFontDesc->GetArray("FontBBox");
  if (pBBox) {
    m_FontBBox.left = pBBox->GetInteger(0);
    m_FontBBox.bottom = pBBox->GetInteger(1);
    m_FontBBox.right = pBBox->GetInteger(2);
    m_FontBBox.top = pBBox->GetInteger(3);
  }

  CPDF_Stream* pFontFile = pFontDesc->GetStream("FontFile");
  if (!pFontFile)
    pFontFile = pFontDesc->GetStream("FontFile2");
  if (!pFontFile)
    pFontFile = pFontDesc->GetStream("FontFile3");
  if (!pFontFile)
    return;

  m_pFontFile = m_pDocument->LoadFontFile(pFontFile);
  if (!m_pFontFile)
    return;

  const uint8_t* pFontData = m_pFontFile->GetData();
  FX_DWORD dwFontSize = m_pFontFile->GetSize();
  if (!m_Font.LoadEmbedded(pFontData, dwFontSize)) {
    m_pDocument->GetPageData()->ReleaseFontFileStreamAcc(
        const_cast<CPDF_Stream*>(m_pFontFile->GetStream()->AsStream()));
    m_pFontFile = nullptr;
  }
}

short TT2PDF(int m, FXFT_Face face) {
  int upm = FXFT_Get_Face_UnitsPerEM(face);
  if (upm == 0) {
    return (short)m;
  }
  return (m * 1000 + upm / 2) / upm;
}
void CPDF_Font::CheckFontMetrics() {
  if (m_FontBBox.top == 0 && m_FontBBox.bottom == 0 && m_FontBBox.left == 0 &&
      m_FontBBox.right == 0) {
    FXFT_Face face = m_Font.GetFace();
    if (face) {
      m_FontBBox.left = TT2PDF(FXFT_Get_Face_xMin(face), face);
      m_FontBBox.bottom = TT2PDF(FXFT_Get_Face_yMin(face), face);
      m_FontBBox.right = TT2PDF(FXFT_Get_Face_xMax(face), face);
      m_FontBBox.top = TT2PDF(FXFT_Get_Face_yMax(face), face);
      m_Ascent = TT2PDF(FXFT_Get_Face_Ascender(face), face);
      m_Descent = TT2PDF(FXFT_Get_Face_Descender(face), face);
    } else {
      FX_BOOL bFirst = TRUE;
      for (int i = 0; i < 256; i++) {
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
void CPDF_Font::LoadUnicodeMap() {
  m_bToUnicodeLoaded = TRUE;
  CPDF_Stream* pStream = m_pFontDict->GetStream("ToUnicode");
  if (!pStream) {
    return;
  }
  m_pToUnicodeMap = new CPDF_ToUnicodeMap;
  m_pToUnicodeMap->Load(pStream);
}
int CPDF_Font::GetStringWidth(const FX_CHAR* pString, int size) {
  int offset = 0;
  int width = 0;
  while (offset < size) {
    FX_DWORD charcode = GetNextChar(pString, size, offset);
    width += GetCharWidthF(charcode);
  }
  return width;
}
int CPDF_Font::GetCharTypeWidth(FX_DWORD charcode) {
  if (!m_Font.GetFace())
    return 0;

  int glyph_index = GlyphFromCharCode(charcode);
  if (glyph_index == 0xffff) {
    return 0;
  }
  return m_Font.GetGlyphWidth(glyph_index);
}

CPDF_Font* CPDF_Font::GetStockFont(CPDF_Document* pDoc,
                                   const CFX_ByteStringC& name) {
  CFX_ByteString fontname(name);
  int font_id = PDF_GetStandardFontName(&fontname);
  if (font_id < 0) {
    return nullptr;
  }
  CPDF_FontGlobals* pFontGlobals =
      CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
  CPDF_Font* pFont = pFontGlobals->Find(pDoc, font_id);
  if (pFont) {
    return pFont;
  }
  CPDF_Dictionary* pDict = new CPDF_Dictionary;
  pDict->SetAtName("Type", "Font");
  pDict->SetAtName("Subtype", "Type1");
  pDict->SetAtName("BaseFont", fontname);
  pDict->SetAtName("Encoding", "WinAnsiEncoding");
  pFont = CPDF_Font::CreateFontF(NULL, pDict);
  pFontGlobals->Set(pDoc, font_id, pFont);
  return pFont;
}
const uint8_t ChineseFontNames[][5] = {{0xCB, 0xCE, 0xCC, 0xE5, 0x00},
                                       {0xBF, 0xAC, 0xCC, 0xE5, 0x00},
                                       {0xBA, 0xDA, 0xCC, 0xE5, 0x00},
                                       {0xB7, 0xC2, 0xCB, 0xCE, 0x00},
                                       {0xD0, 0xC2, 0xCB, 0xCE, 0x00}};
CPDF_Font* CPDF_Font::CreateFontF(CPDF_Document* pDoc,
                                  CPDF_Dictionary* pFontDict) {
  CFX_ByteString type = pFontDict->GetString("Subtype");
  CPDF_Font* pFont;
  if (type == "TrueType") {
    {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ || \
    _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_ ||   \
    _FXM_PLATFORM_ == _FXM_PLATFORM_ANDROID_ || \
    _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      CFX_ByteString basefont = pFontDict->GetString("BaseFont");
      CFX_ByteString tag = basefont.Left(4);
      int i;
      int count = sizeof(ChineseFontNames) / sizeof(ChineseFontNames[0]);
      for (i = 0; i < count; ++i) {
        if (tag == CFX_ByteString((const FX_CHAR*)ChineseFontNames[i])) {
          break;
        }
      }
      if (i < count) {
        CPDF_Dictionary* pFontDesc = pFontDict->GetDict("FontDescriptor");
        if (!pFontDesc || !pFontDesc->KeyExist("FontFile2")) {
          pFont = new CPDF_CIDFont;
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
    pFont = new CPDF_TrueTypeFont;
  } else if (type == "Type3") {
    pFont = new CPDF_Type3Font;
  } else if (type == "Type0") {
    pFont = new CPDF_CIDFont;
  } else {
    pFont = new CPDF_Type1Font;
  }
  pFont->m_pFontDict = pFontDict;
  pFont->m_pDocument = pDoc;
  if (!pFont->Load()) {
    delete pFont;
    return NULL;
  }
  return pFont;
}
FX_BOOL CPDF_Font::Load() {
  if (!m_pFontDict) {
    return FALSE;
  }
  CFX_ByteString type = m_pFontDict->GetString("Subtype");
  m_BaseFont = m_pFontDict->GetString("BaseFont");
  if (type == "MMType1") {
    type = "Type1";
  }
  return _Load();
}
static CFX_WideString _FontMap_GetWideString(CFX_CharMap* pMap,
                                             const CFX_ByteString& bytestr) {
  return ((CPDF_FontCharMap*)pMap)->m_pFont->DecodeString(bytestr);
}
static CFX_ByteString _FontMap_GetByteString(CFX_CharMap* pMap,
                                             const CFX_WideString& widestr) {
  return ((CPDF_FontCharMap*)pMap)->m_pFont->EncodeString(widestr);
}
CPDF_FontCharMap::CPDF_FontCharMap(CPDF_Font* pFont) {
  m_GetByteString = _FontMap_GetByteString;
  m_GetWideString = _FontMap_GetWideString;
  m_pFont = pFont;
}
CFX_WideString CPDF_ToUnicodeMap::Lookup(FX_DWORD charcode) {
  auto it = m_Map.find(charcode);
  if (it != m_Map.end()) {
    FX_DWORD value = it->second;
    FX_WCHAR unicode = (FX_WCHAR)(value & 0xffff);
    if (unicode != 0xffff) {
      return unicode;
    }
    const FX_WCHAR* buf = m_MultiCharBuf.GetBuffer();
    FX_DWORD buf_len = m_MultiCharBuf.GetLength();
    if (!buf || buf_len == 0) {
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
FX_DWORD CPDF_ToUnicodeMap::ReverseLookup(FX_WCHAR unicode) {
  for (const auto& pair : m_Map) {
    if (pair.second == unicode)
      return pair.first;
  }
  return 0;
}

// Static.
FX_DWORD CPDF_ToUnicodeMap::StringToCode(const CFX_ByteStringC& str) {
  const FX_CHAR* buf = str.GetCStr();
  int len = str.GetLength();
  if (len == 0)
    return 0;

  int result = 0;
  if (buf[0] == '<') {
    for (int i = 1; i < len && std::isxdigit(buf[i]); ++i)
      result = result * 16 + FXSYS_toHexDigit(buf[i]);
    return result;
  }

  for (int i = 0; i < len && std::isdigit(buf[i]); ++i)
    result = result * 10 + FXSYS_toDecimalDigit(buf[i]);

  return result;
}
static CFX_WideString StringDataAdd(CFX_WideString str) {
  CFX_WideString ret;
  int len = str.GetLength();
  FX_WCHAR value = 1;
  for (int i = len - 1; i >= 0; --i) {
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

// Static.
CFX_WideString CPDF_ToUnicodeMap::StringToWideString(
    const CFX_ByteStringC& str) {
  const FX_CHAR* buf = str.GetCStr();
  int len = str.GetLength();
  if (len == 0)
    return CFX_WideString();

  CFX_WideString result;
  if (buf[0] == '<') {
    int byte_pos = 0;
    FX_WCHAR ch = 0;
    for (int i = 1; i < len && std::isxdigit(buf[i]); ++i) {
      ch = ch * 16 + FXSYS_toHexDigit(buf[i]);
      byte_pos++;
      if (byte_pos == 4) {
        result += ch;
        byte_pos = 0;
        ch = 0;
      }
    }
    return result;
  }
  return result;
}
void CPDF_ToUnicodeMap::Load(CPDF_Stream* pStream) {
  CIDSet cid_set = CIDSET_UNKNOWN;
  CPDF_StreamAcc stream;
  stream.LoadAllData(pStream, FALSE);
  CPDF_SimpleParser parser(stream.GetData(), stream.GetSize());
  while (1) {
    CFX_ByteStringC word = parser.GetWord();
    if (word.IsEmpty()) {
      break;
    }
    if (word == "beginbfchar") {
      while (1) {
        word = parser.GetWord();
        if (word.IsEmpty() || word == "endbfchar") {
          break;
        }
        FX_DWORD srccode = StringToCode(word);
        word = parser.GetWord();
        CFX_WideString destcode = StringToWideString(word);
        int len = destcode.GetLength();
        if (len == 0) {
          continue;
        }
        if (len == 1) {
          m_Map[srccode] = destcode.GetAt(0);
        } else {
          m_Map[srccode] = m_MultiCharBuf.GetLength() * 0x10000 + 0xffff;
          m_MultiCharBuf.AppendChar(destcode.GetLength());
          m_MultiCharBuf << destcode;
        }
      }
    } else if (word == "beginbfrange") {
      while (1) {
        CFX_ByteString low, high;
        low = parser.GetWord();
        if (low.IsEmpty() || low == "endbfrange") {
          break;
        }
        high = parser.GetWord();
        FX_DWORD lowcode = StringToCode(low);
        FX_DWORD highcode =
            (lowcode & 0xffffff00) | (StringToCode(high) & 0xff);
        if (highcode == (FX_DWORD)-1) {
          break;
        }
        CFX_ByteString start = parser.GetWord();
        if (start == "[") {
          for (FX_DWORD code = lowcode; code <= highcode; code++) {
            CFX_ByteString dest = parser.GetWord();
            CFX_WideString destcode = StringToWideString(dest);
            int len = destcode.GetLength();
            if (len == 0) {
              continue;
            }
            if (len == 1) {
              m_Map[code] = destcode.GetAt(0);
            } else {
              m_Map[code] = m_MultiCharBuf.GetLength() * 0x10000 + 0xffff;
              m_MultiCharBuf.AppendChar(destcode.GetLength());
              m_MultiCharBuf << destcode;
            }
          }
          parser.GetWord();
        } else {
          CFX_WideString destcode = StringToWideString(start);
          int len = destcode.GetLength();
          FX_DWORD value = 0;
          if (len == 1) {
            value = StringToCode(start);
            for (FX_DWORD code = lowcode; code <= highcode; code++) {
              m_Map[code] = value++;
            }
          } else {
            for (FX_DWORD code = lowcode; code <= highcode; code++) {
              CFX_WideString retcode;
              if (code == lowcode) {
                retcode = destcode;
              } else {
                retcode = StringDataAdd(destcode);
              }
              m_Map[code] = m_MultiCharBuf.GetLength() * 0x10000 + 0xffff;
              m_MultiCharBuf.AppendChar(retcode.GetLength());
              m_MultiCharBuf << retcode;
              destcode = retcode;
            }
          }
        }
      }
    } else if (word == "/Adobe-Korea1-UCS2") {
      cid_set = CIDSET_KOREA1;
    } else if (word == "/Adobe-Japan1-UCS2") {
      cid_set = CIDSET_JAPAN1;
    } else if (word == "/Adobe-CNS1-UCS2") {
      cid_set = CIDSET_CNS1;
    } else if (word == "/Adobe-GB1-UCS2") {
      cid_set = CIDSET_GB1;
    }
  }
  if (cid_set) {
    m_pBaseMap = CPDF_ModuleMgr::Get()
                     ->GetPageModule()
                     ->GetFontGlobals()
                     ->m_CMapManager.GetCID2UnicodeMap(cid_set, FALSE);
  } else {
    m_pBaseMap = NULL;
  }
}
static FX_BOOL GetPredefinedEncoding(int& basemap,
                                     const CFX_ByteString& value) {
  if (value == "WinAnsiEncoding") {
    basemap = PDFFONT_ENCODING_WINANSI;
  } else if (value == "MacRomanEncoding") {
    basemap = PDFFONT_ENCODING_MACROMAN;
  } else if (value == "MacExpertEncoding") {
    basemap = PDFFONT_ENCODING_MACEXPERT;
  } else if (value == "PDFDocEncoding") {
    basemap = PDFFONT_ENCODING_PDFDOC;
  } else {
    return FALSE;
  }
  return TRUE;
}
void CPDF_Font::LoadPDFEncoding(CPDF_Object* pEncoding,
                                int& iBaseEncoding,
                                CFX_ByteString*& pCharNames,
                                FX_BOOL bEmbedded,
                                FX_BOOL bTrueType) {
  if (!pEncoding) {
    if (m_BaseFont == "Symbol") {
      iBaseEncoding = bTrueType ? PDFFONT_ENCODING_MS_SYMBOL
                                : PDFFONT_ENCODING_ADOBE_SYMBOL;
    } else if (!bEmbedded && iBaseEncoding == PDFFONT_ENCODING_BUILTIN) {
      iBaseEncoding = PDFFONT_ENCODING_WINANSI;
    }
    return;
  }
  if (pEncoding->IsName()) {
    if (iBaseEncoding == PDFFONT_ENCODING_ADOBE_SYMBOL ||
        iBaseEncoding == PDFFONT_ENCODING_ZAPFDINGBATS) {
      return;
    }
    if ((m_Flags & PDFFONT_SYMBOLIC) && m_BaseFont == "Symbol") {
      if (!bTrueType) {
        iBaseEncoding = PDFFONT_ENCODING_ADOBE_SYMBOL;
      }
      return;
    }
    CFX_ByteString bsEncoding = pEncoding->GetString();
    if (bsEncoding.Compare("MacExpertEncoding") == 0) {
      bsEncoding = "WinAnsiEncoding";
    }
    GetPredefinedEncoding(iBaseEncoding, bsEncoding);
    return;
  }

  CPDF_Dictionary* pDict = pEncoding->AsDictionary();
  if (!pDict)
    return;

  if (iBaseEncoding != PDFFONT_ENCODING_ADOBE_SYMBOL &&
      iBaseEncoding != PDFFONT_ENCODING_ZAPFDINGBATS) {
    CFX_ByteString bsEncoding = pDict->GetString("BaseEncoding");
    if (bsEncoding.Compare("MacExpertEncoding") == 0 && bTrueType) {
      bsEncoding = "WinAnsiEncoding";
    }
    GetPredefinedEncoding(iBaseEncoding, bsEncoding);
  }
  if ((!bEmbedded || bTrueType) && iBaseEncoding == PDFFONT_ENCODING_BUILTIN) {
    iBaseEncoding = PDFFONT_ENCODING_STANDARD;
  }
  CPDF_Array* pDiffs = pDict->GetArray("Differences");
  if (!pDiffs) {
    return;
  }
  pCharNames = new CFX_ByteString[256];
  FX_DWORD cur_code = 0;
  for (FX_DWORD i = 0; i < pDiffs->GetCount(); i++) {
    CPDF_Object* pElement = pDiffs->GetElementValue(i);
    if (!pElement)
      continue;

    if (CPDF_Name* pName = pElement->AsName()) {
      if (cur_code < 256)
        pCharNames[cur_code] = pName->GetString();
      cur_code++;
    } else {
      cur_code = pElement->GetInteger();
    }
  }
}

FX_BOOL CPDF_Font::IsStandardFont() const {
  if (m_FontType != PDFFONT_TYPE1)
    return FALSE;
  if (m_pFontFile)
    return FALSE;
  if (((CPDF_Type1Font*)this)->GetBase14Font() < 0)
    return FALSE;
  return TRUE;
}
CPDF_SimpleFont::CPDF_SimpleFont(int fonttype) : CPDF_Font(fonttype) {
  FXSYS_memset(m_CharBBox, 0xff, sizeof m_CharBBox);
  FXSYS_memset(m_CharWidth, 0xff, sizeof m_CharWidth);
  FXSYS_memset(m_GlyphIndex, 0xff, sizeof m_GlyphIndex);
  FXSYS_memset(m_ExtGID, 0xff, sizeof m_ExtGID);
  m_pCharNames = NULL;
  m_BaseEncoding = PDFFONT_ENCODING_BUILTIN;
}
CPDF_SimpleFont::~CPDF_SimpleFont() {
  delete[] m_pCharNames;
}
int CPDF_SimpleFont::GlyphFromCharCode(FX_DWORD charcode, FX_BOOL* pVertGlyph) {
  if (pVertGlyph) {
    *pVertGlyph = FALSE;
  }
  if (charcode > 0xff) {
    return -1;
  }
  int index = m_GlyphIndex[(uint8_t)charcode];
  if (index == 0xffff) {
    return -1;
  }
  return index;
}
void CPDF_SimpleFont::LoadCharMetrics(int charcode) {
  if (!m_Font.GetFace())
    return;

  if (charcode < 0 || charcode > 0xff) {
    return;
  }
  int glyph_index = m_GlyphIndex[charcode];
  if (glyph_index == 0xffff) {
    if (!m_pFontFile && charcode != 32) {
      LoadCharMetrics(32);
      m_CharBBox[charcode] = m_CharBBox[32];
      if (m_bUseFontWidth) {
        m_CharWidth[charcode] = m_CharWidth[32];
      }
    }
    return;
  }
  FXFT_Face face = m_Font.GetFace();
  int err = FXFT_Load_Glyph(
      face, glyph_index,
      FXFT_LOAD_NO_SCALE | FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
  if (err) {
    return;
  }
  m_CharBBox[charcode].Left = TT2PDF(FXFT_Get_Glyph_HoriBearingX(face), face);
  m_CharBBox[charcode].Right = TT2PDF(
      FXFT_Get_Glyph_HoriBearingX(face) + FXFT_Get_Glyph_Width(face), face);
  m_CharBBox[charcode].Top = TT2PDF(FXFT_Get_Glyph_HoriBearingY(face), face);
  m_CharBBox[charcode].Bottom = TT2PDF(
      FXFT_Get_Glyph_HoriBearingY(face) - FXFT_Get_Glyph_Height(face), face);
  if (m_bUseFontWidth) {
    int TT_Width = TT2PDF(FXFT_Get_Glyph_HoriAdvance(face), face);
    if (m_CharWidth[charcode] == 0xffff) {
      m_CharWidth[charcode] = TT_Width;
    } else if (TT_Width && !IsEmbedded()) {
      m_CharBBox[charcode].Right =
          m_CharBBox[charcode].Right * m_CharWidth[charcode] / TT_Width;
      m_CharBBox[charcode].Left =
          m_CharBBox[charcode].Left * m_CharWidth[charcode] / TT_Width;
    }
  }
}
int CPDF_SimpleFont::GetCharWidthF(FX_DWORD charcode, int level) {
  if (charcode > 0xff) {
    charcode = 0;
  }
  if (m_CharWidth[charcode] == 0xffff) {
    LoadCharMetrics(charcode);
    if (m_CharWidth[charcode] == 0xffff) {
      m_CharWidth[charcode] = 0;
    }
  }
  return (int16_t)m_CharWidth[charcode];
}
void CPDF_SimpleFont::GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level) {
  if (charcode > 0xff) {
    charcode = 0;
  }
  if (m_CharBBox[charcode].Left == (int16_t)0xffff) {
    LoadCharMetrics(charcode);
  }
  rect.left = m_CharBBox[charcode].Left;
  rect.right = m_CharBBox[charcode].Right;
  rect.bottom = m_CharBBox[charcode].Bottom;
  rect.top = m_CharBBox[charcode].Top;
}
const FX_CHAR* GetAdobeCharName(int iBaseEncoding,
                                const CFX_ByteString* pCharNames,
                                int charcode) {
  ASSERT(charcode >= 0 && charcode < 256);
  if (charcode < 0 || charcode >= 256) {
    return NULL;
  }
  const FX_CHAR* name = NULL;
  if (pCharNames) {
    name = pCharNames[charcode];
  }
  if ((!name || name[0] == 0) && iBaseEncoding) {
    name = PDF_CharNameFromPredefinedCharSet(iBaseEncoding, charcode);
  }
  return name && name[0] ? name : nullptr;
}
FX_BOOL CPDF_SimpleFont::LoadCommon() {
  CPDF_Dictionary* pFontDesc = m_pFontDict->GetDict("FontDescriptor");
  if (pFontDesc) {
    LoadFontDescriptor(pFontDesc);
  }
  CPDF_Array* pWidthArray = m_pFontDict->GetArray("Widths");
  int width_start = 0, width_end = -1;
  m_bUseFontWidth = TRUE;
  if (pWidthArray) {
    m_bUseFontWidth = FALSE;
    if (pFontDesc && pFontDesc->KeyExist("MissingWidth")) {
      int MissingWidth = pFontDesc->GetInteger("MissingWidth");
      for (int i = 0; i < 256; i++) {
        m_CharWidth[i] = MissingWidth;
      }
    }
    width_start = m_pFontDict->GetInteger("FirstChar", 0);
    width_end = m_pFontDict->GetInteger("LastChar", 0);
    if (width_start >= 0 && width_start <= 255) {
      if (width_end <= 0 ||
          width_end >= width_start + (int)pWidthArray->GetCount()) {
        width_end = width_start + pWidthArray->GetCount() - 1;
      }
      if (width_end > 255) {
        width_end = 255;
      }
      for (int i = width_start; i <= width_end; i++) {
        m_CharWidth[i] = pWidthArray->GetInteger(i - width_start);
      }
    }
  }
  if (m_pFontFile) {
    if (m_BaseFont.GetLength() > 8 && m_BaseFont[7] == '+') {
      m_BaseFont = m_BaseFont.Mid(8);
    }
  } else {
    LoadSubstFont();
  }
  if (!(m_Flags & PDFFONT_SYMBOLIC)) {
    m_BaseEncoding = PDFFONT_ENCODING_STANDARD;
  }
  CPDF_Object* pEncoding = m_pFontDict->GetElementValue("Encoding");
  LoadPDFEncoding(pEncoding, m_BaseEncoding, m_pCharNames, m_pFontFile != NULL,
                  m_Font.IsTTFont());
  LoadGlyphMap();
  delete[] m_pCharNames;
  m_pCharNames = NULL;
  if (!m_Font.GetFace())
    return TRUE;

  if (m_Flags & PDFFONT_ALLCAP) {
    unsigned char lowercases[] = {'a', 'z', 0xe0, 0xf6, 0xf8, 0xfd};
    for (size_t range = 0; range < sizeof lowercases / 2; range++) {
      for (int i = lowercases[range * 2]; i <= lowercases[range * 2 + 1]; i++) {
        if (m_GlyphIndex[i] != 0xffff && m_pFontFile) {
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
void CPDF_SimpleFont::LoadSubstFont() {
  if (!m_bUseFontWidth && !(m_Flags & PDFFONT_FIXEDPITCH)) {
    int width = 0, i;
    for (i = 0; i < 256; i++) {
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
  m_Font.LoadSubst(m_BaseFont, IsFontType(PDFFONT_TRUETYPE), m_Flags, weight,
                   m_ItalicAngle, 0);
  if (m_Font.GetSubstFont()->m_SubstFlags & FXFONT_SUBST_NONSYMBOL) {
  }
}
FX_BOOL CPDF_SimpleFont::IsUnicodeCompatible() const {
  return m_BaseEncoding != PDFFONT_ENCODING_BUILTIN &&
         m_BaseEncoding != PDFFONT_ENCODING_ADOBE_SYMBOL &&
         m_BaseEncoding != PDFFONT_ENCODING_ZAPFDINGBATS;
}
CPDF_Type1Font::CPDF_Type1Font() : CPDF_SimpleFont(PDFFONT_TYPE1) {
  m_Base14Font = -1;
}
FX_BOOL CPDF_Type1Font::_Load() {
  m_Base14Font = PDF_GetStandardFontName(&m_BaseFont);
  if (m_Base14Font >= 0) {
    CPDF_Dictionary* pFontDesc = m_pFontDict->GetDict("FontDescriptor");
    if (pFontDesc && pFontDesc->KeyExist("Flags")) {
      m_Flags = pFontDesc->GetInteger("Flags");
    } else {
      m_Flags = m_Base14Font >= 12 ? PDFFONT_SYMBOLIC : PDFFONT_NONSYMBOLIC;
    }
    if (m_Base14Font < 4)
      for (int i = 0; i < 256; i++) {
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
static FX_BOOL FT_UseType1Charmap(FXFT_Face face) {
  if (FXFT_Get_Face_CharmapCount(face) == 0) {
    return FALSE;
  }
  if (FXFT_Get_Face_CharmapCount(face) == 1 &&
      FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[0]) ==
          FXFT_ENCODING_UNICODE) {
    return FALSE;
  }
  if (FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[0]) ==
      FXFT_ENCODING_UNICODE) {
    FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[1]);
  } else {
    FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[0]);
  }
  return TRUE;
}
int CPDF_Type1Font::GlyphFromCharCodeExt(FX_DWORD charcode) {
  if (charcode > 0xff) {
    return -1;
  }
  int index = m_ExtGID[(uint8_t)charcode];
  if (index == 0xffff) {
    return -1;
  }
  return index;
}
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
struct _GlyphNameMap {
  const FX_CHAR* m_pStrAdobe;
  const FX_CHAR* m_pStrUnicode;
};
static const _GlyphNameMap g_GlyphNameSubsts[] = {{"ff", "uniFB00"},
                                                  {"fi", "uniFB01"},
                                                  {"fl", "uniFB02"},
                                                  {"ffi", "uniFB03"},
                                                  {"ffl", "uniFB04"}};
extern "C" {
static int compareString(const void* key, const void* element) {
  return FXSYS_stricmp((const FX_CHAR*)key,
                       ((_GlyphNameMap*)element)->m_pStrAdobe);
}
}
static const FX_CHAR* _GlyphNameRemap(const FX_CHAR* pStrAdobe) {
  _GlyphNameMap* found = (_GlyphNameMap*)FXSYS_bsearch(
      pStrAdobe, g_GlyphNameSubsts,
      sizeof g_GlyphNameSubsts / sizeof(_GlyphNameMap), sizeof(_GlyphNameMap),
      compareString);
  if (found) {
    return found->m_pStrUnicode;
  }
  return NULL;
}
#endif
void CPDF_Type1Font::LoadGlyphMap() {
  if (!m_Font.GetFace())
    return;

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  FX_BOOL bCoreText = TRUE;
  CQuartz2D& quartz2d =
      ((CApplePlatform*)CFX_GEModule::Get()->GetPlatformData())->_quartz2d;
  if (!m_Font.GetPlatformFont()) {
    if (m_Font.GetPsName() == CFX_WideString::FromLocal("DFHeiStd-W5")) {
      bCoreText = FALSE;
    }
    m_Font.SetPlatformFont(
        quartz2d.CreateFont(m_Font.GetFontData(), m_Font.GetSize()));
    if (!m_Font.GetPlatformFont()) {
      bCoreText = FALSE;
    }
  }
#endif
  if (!IsEmbedded() && (m_Base14Font < 12) && m_Font.IsTTFont()) {
    if (FT_UseTTCharmap(m_Font.GetFace(), 3, 0)) {
      FX_BOOL bGotOne = FALSE;
      for (int charcode = 0; charcode < 256; charcode++) {
        const uint8_t prefix[4] = {0x00, 0xf0, 0xf1, 0xf2};
        for (int j = 0; j < 4; j++) {
          FX_WORD unicode = prefix[j] * 256 + charcode;
          m_GlyphIndex[charcode] =
              FXFT_Get_Char_Index(m_Font.GetFace(), unicode);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
          FX_CHAR name_glyph[256];
          FXFT_Get_Glyph_Name(m_Font.GetFace(), m_GlyphIndex[charcode],
                              name_glyph, 256);
          name_glyph[255] = 0;
          CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
              kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII,
              kCFAllocatorNull);
          m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
              (CGFontRef)m_Font.GetPlatformFont(), name_ct);
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
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
        if (!bCoreText) {
          FXSYS_memcpy(m_ExtGID, m_GlyphIndex, 256);
        }
#endif
        return;
      }
    }
    FXFT_Select_Charmap(m_Font.GetFace(), FXFT_ENCODING_UNICODE);
    if (m_BaseEncoding == 0) {
      m_BaseEncoding = PDFFONT_ENCODING_STANDARD;
    }
    for (int charcode = 0; charcode < 256; charcode++) {
      const FX_CHAR* name =
          GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
      if (!name) {
        continue;
      }
      m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
      m_GlyphIndex[charcode] = FXFT_Get_Char_Index(
          m_Font.GetFace(), m_Encoding.m_Unicodes[charcode]);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      FX_CHAR name_glyph[256];
      FXFT_Get_Glyph_Name(m_Font.GetFace(), m_GlyphIndex[charcode], name_glyph,
                          256);
      name_glyph[255] = 0;
      CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
          kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII,
          kCFAllocatorNull);
      m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
          (CGFontRef)m_Font.GetPlatformFont(), name_ct);
      if (name_ct) {
        CFRelease(name_ct);
      }
#endif
      if (m_GlyphIndex[charcode] == 0 && FXSYS_strcmp(name, ".notdef") == 0) {
        m_Encoding.m_Unicodes[charcode] = 0x20;
        m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.GetFace(), 0x20);
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
        FX_CHAR name_glyph[256];
        FXFT_Get_Glyph_Name(m_Font.GetFace(), m_GlyphIndex[charcode],
                            name_glyph, 256);
        name_glyph[255] = 0;
        CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
            kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII,
            kCFAllocatorNull);
        m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
            (CGFontRef)m_Font.GetPlatformFont(), name_ct);
        if (name_ct) {
          CFRelease(name_ct);
        }
#endif
      }
    }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    if (!bCoreText) {
      FXSYS_memcpy(m_ExtGID, m_GlyphIndex, 256);
    }
#endif
    return;
  }
  FT_UseType1Charmap(m_Font.GetFace());
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  if (bCoreText) {
    if (m_Flags & PDFFONT_SYMBOLIC) {
      for (int charcode = 0; charcode < 256; charcode++) {
        const FX_CHAR* name =
            GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
        if (name) {
          m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
          m_GlyphIndex[charcode] =
              FXFT_Get_Name_Index(m_Font.GetFace(), (char*)name);
          CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
              kCFAllocatorDefault, name, kCFStringEncodingASCII,
              kCFAllocatorNull);
          m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
              (CGFontRef)m_Font.GetPlatformFont(), name_ct);
          if (name_ct) {
            CFRelease(name_ct);
          }
        } else {
          m_GlyphIndex[charcode] =
              FXFT_Get_Char_Index(m_Font.GetFace(), charcode);
          FX_WCHAR unicode = 0;
          if (m_GlyphIndex[charcode]) {
            unicode =
                FT_UnicodeFromCharCode(PDFFONT_ENCODING_STANDARD, charcode);
          }
          FX_CHAR name_glyph[256];
          FXSYS_memset(name_glyph, 0, sizeof(name_glyph));
          FXFT_Get_Glyph_Name(m_Font.GetFace(), m_GlyphIndex[charcode],
                              name_glyph, 256);
          name_glyph[255] = 0;
          if (unicode == 0 && name_glyph[0] != 0) {
            unicode = PDF_UnicodeFromAdobeName(name_glyph);
          }
          m_Encoding.m_Unicodes[charcode] = unicode;
          CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
              kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII,
              kCFAllocatorNull);
          m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
              (CGFontRef)m_Font.GetPlatformFont(), name_ct);
          if (name_ct) {
            CFRelease(name_ct);
          }
        }
      }
      return;
    }
    FX_BOOL bUnicode = FALSE;
    if (0 == FXFT_Select_Charmap(m_Font.GetFace(), FXFT_ENCODING_UNICODE)) {
      bUnicode = TRUE;
    }
    for (int charcode = 0; charcode < 256; charcode++) {
      const FX_CHAR* name =
          GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
      if (!name) {
        continue;
      }
      m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
      const FX_CHAR* pStrUnicode = _GlyphNameRemap(name);
      if (pStrUnicode &&
          0 == FXFT_Get_Name_Index(m_Font.GetFace(), (char*)name)) {
        name = pStrUnicode;
      }
      m_GlyphIndex[charcode] =
          FXFT_Get_Name_Index(m_Font.GetFace(), (char*)name);
      CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
          kCFAllocatorDefault, name, kCFStringEncodingASCII, kCFAllocatorNull);
      m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
          (CGFontRef)m_Font.GetPlatformFont(), name_ct);
      if (name_ct) {
        CFRelease(name_ct);
      }
      if (m_GlyphIndex[charcode] == 0) {
        if (FXSYS_strcmp(name, ".notdef") != 0 &&
            FXSYS_strcmp(name, "space") != 0) {
          m_GlyphIndex[charcode] = FXFT_Get_Char_Index(
              m_Font.GetFace(),
              bUnicode ? m_Encoding.m_Unicodes[charcode] : charcode);
          FX_CHAR name_glyph[256];
          FXFT_Get_Glyph_Name(m_Font.GetFace(), m_GlyphIndex[charcode],
                              name_glyph, 256);
          name_glyph[255] = 0;
          CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
              kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII,
              kCFAllocatorNull);
          m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
              (CGFontRef)m_Font.GetPlatformFont(), name_ct);
          if (name_ct) {
            CFRelease(name_ct);
          }
        } else {
          m_Encoding.m_Unicodes[charcode] = 0x20;
          m_GlyphIndex[charcode] =
              bUnicode ? FXFT_Get_Char_Index(m_Font.GetFace(), 0x20) : 0xffff;
          FX_CHAR name_glyph[256];
          FXFT_Get_Glyph_Name(m_Font.GetFace(), m_GlyphIndex[charcode],
                              name_glyph, 256);
          name_glyph[255] = 0;
          CFStringRef name_ct = CFStringCreateWithCStringNoCopy(
              kCFAllocatorDefault, name_glyph, kCFStringEncodingASCII,
              kCFAllocatorNull);
          m_ExtGID[charcode] = CGFontGetGlyphWithGlyphName(
              (CGFontRef)m_Font.GetPlatformFont(), name_ct);
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
    for (int charcode = 0; charcode < 256; charcode++) {
      const FX_CHAR* name =
          GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
      if (name) {
        m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
        m_GlyphIndex[charcode] =
            FXFT_Get_Name_Index(m_Font.GetFace(), (char*)name);
      } else {
        m_GlyphIndex[charcode] =
            FXFT_Get_Char_Index(m_Font.GetFace(), charcode);
        if (m_GlyphIndex[charcode]) {
          FX_WCHAR unicode =
              FT_UnicodeFromCharCode(PDFFONT_ENCODING_STANDARD, charcode);
          if (unicode == 0) {
            FX_CHAR name_glyph[256];
            FXSYS_memset(name_glyph, 0, sizeof(name_glyph));
            FXFT_Get_Glyph_Name(m_Font.GetFace(), m_GlyphIndex[charcode],
                                name_glyph, 256);
            name_glyph[255] = 0;
            if (name_glyph[0] != 0) {
              unicode = PDF_UnicodeFromAdobeName(name_glyph);
            }
          }
          m_Encoding.m_Unicodes[charcode] = unicode;
        }
      }
    }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    if (!bCoreText) {
      FXSYS_memcpy(m_ExtGID, m_GlyphIndex, 256);
    }
#endif
    return;
  }
  FX_BOOL bUnicode = FALSE;
  if (0 == FXFT_Select_Charmap(m_Font.GetFace(), FXFT_ENCODING_UNICODE)) {
    bUnicode = TRUE;
  }
  for (int charcode = 0; charcode < 256; charcode++) {
    const FX_CHAR* name =
        GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
    if (!name) {
      continue;
    }
    m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
    m_GlyphIndex[charcode] = FXFT_Get_Name_Index(m_Font.GetFace(), (char*)name);
    if (m_GlyphIndex[charcode] == 0) {
      if (FXSYS_strcmp(name, ".notdef") != 0 &&
          FXSYS_strcmp(name, "space") != 0) {
        m_GlyphIndex[charcode] = FXFT_Get_Char_Index(
            m_Font.GetFace(),
            bUnicode ? m_Encoding.m_Unicodes[charcode] : charcode);
      } else {
        m_Encoding.m_Unicodes[charcode] = 0x20;
        m_GlyphIndex[charcode] = 0xffff;
      }
    }
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  if (!bCoreText) {
    FXSYS_memcpy(m_ExtGID, m_GlyphIndex, 256);
  }
#endif
}

CPDF_FontEncoding::CPDF_FontEncoding() {
  FXSYS_memset(m_Unicodes, 0, sizeof(m_Unicodes));
}
int CPDF_FontEncoding::CharCodeFromUnicode(FX_WCHAR unicode) const {
  for (int i = 0; i < 256; i++)
    if (m_Unicodes[i] == unicode) {
      return i;
    }
  return -1;
}
CPDF_FontEncoding::CPDF_FontEncoding(int PredefinedEncoding) {
  const FX_WORD* pSrc = PDF_UnicodesForPredefinedCharSet(PredefinedEncoding);
  if (!pSrc) {
    FXSYS_memset(m_Unicodes, 0, sizeof(m_Unicodes));
  } else
    for (int i = 0; i < 256; i++) {
      m_Unicodes[i] = pSrc[i];
    }
}
FX_BOOL CPDF_FontEncoding::IsIdentical(CPDF_FontEncoding* pAnother) const {
  return FXSYS_memcmp(m_Unicodes, pAnother->m_Unicodes, sizeof(m_Unicodes)) ==
         0;
}
CPDF_Object* CPDF_FontEncoding::Realize() {
  int predefined = 0;
  for (int cs = PDFFONT_ENCODING_WINANSI; cs < PDFFONT_ENCODING_ZAPFDINGBATS;
       cs++) {
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
      return new CPDF_Name("WinAnsiEncoding");
    }
    if (predefined == PDFFONT_ENCODING_MACROMAN) {
      return new CPDF_Name("MacRomanEncoding");
    }
    if (predefined == PDFFONT_ENCODING_MACEXPERT) {
      return new CPDF_Name("MacExpertEncoding");
    }
    return NULL;
  }
  const FX_WORD* pStandard =
      PDF_UnicodesForPredefinedCharSet(PDFFONT_ENCODING_WINANSI);
  CPDF_Array* pDiff = new CPDF_Array;
  for (int i = 0; i < 256; i++) {
    if (pStandard[i] == m_Unicodes[i]) {
      continue;
    }
    pDiff->Add(new CPDF_Number(i));
    pDiff->Add(new CPDF_Name(PDF_AdobeNameFromUnicode(m_Unicodes[i])));
  }

  CPDF_Dictionary* pDict = new CPDF_Dictionary;
  pDict->SetAtName("BaseEncoding", "WinAnsiEncoding");
  pDict->SetAt("Differences", pDiff);
  return pDict;
}
CPDF_TrueTypeFont::CPDF_TrueTypeFont() : CPDF_SimpleFont(PDFFONT_TRUETYPE) {}
FX_BOOL CPDF_TrueTypeFont::_Load() {
  return LoadCommon();
}
void CPDF_TrueTypeFont::LoadGlyphMap() {
  if (!m_Font.GetFace())
    return;

  int baseEncoding = m_BaseEncoding;
  if (m_pFontFile && m_Font.GetFace()->num_charmaps > 0 &&
      (baseEncoding == PDFFONT_ENCODING_MACROMAN ||
       baseEncoding == PDFFONT_ENCODING_WINANSI) &&
      (m_Flags & PDFFONT_SYMBOLIC)) {
    FX_BOOL bSupportWin = FALSE;
    FX_BOOL bSupportMac = FALSE;
    for (int i = 0; i < FXFT_Get_Face_CharmapCount(m_Font.GetFace()); i++) {
      int platform_id = FXFT_Get_Charmap_PlatformID(
          FXFT_Get_Face_Charmaps(m_Font.GetFace())[i]);
      if (platform_id == 0 || platform_id == 3) {
        bSupportWin = TRUE;
      } else if (platform_id == 0 || platform_id == 1) {
        bSupportMac = TRUE;
      }
    }
    if (baseEncoding == PDFFONT_ENCODING_WINANSI && !bSupportWin) {
      baseEncoding =
          bSupportMac ? PDFFONT_ENCODING_MACROMAN : PDFFONT_ENCODING_BUILTIN;
    } else if (baseEncoding == PDFFONT_ENCODING_MACROMAN && !bSupportMac) {
      baseEncoding =
          bSupportWin ? PDFFONT_ENCODING_WINANSI : PDFFONT_ENCODING_BUILTIN;
    }
  }
  if (((baseEncoding == PDFFONT_ENCODING_MACROMAN ||
        baseEncoding == PDFFONT_ENCODING_WINANSI) &&
       !m_pCharNames) ||
      (m_Flags & PDFFONT_NONSYMBOLIC)) {
    if (!FXFT_Has_Glyph_Names(m_Font.GetFace()) &&
        (!m_Font.GetFace()->num_charmaps || !m_Font.GetFace()->charmaps)) {
      int nStartChar = m_pFontDict->GetInteger("FirstChar");
      if (nStartChar < 0 || nStartChar > 255)
        return;

      int charcode = 0;
      for (; charcode < nStartChar; charcode++) {
        m_GlyphIndex[charcode] = 0;
      }
      FX_WORD nGlyph = charcode - nStartChar + 3;
      for (; charcode < 256; charcode++, nGlyph++) {
        m_GlyphIndex[charcode] = nGlyph;
      }
      return;
    }
    FX_BOOL bMSUnicode = FT_UseTTCharmap(m_Font.GetFace(), 3, 1);
    FX_BOOL bMacRoman = FALSE, bMSSymbol = FALSE;
    if (!bMSUnicode) {
      if (m_Flags & PDFFONT_NONSYMBOLIC) {
        bMacRoman = FT_UseTTCharmap(m_Font.GetFace(), 1, 0);
        bMSSymbol = !bMacRoman && FT_UseTTCharmap(m_Font.GetFace(), 3, 0);
      } else {
        bMSSymbol = FT_UseTTCharmap(m_Font.GetFace(), 3, 0);
        bMacRoman = !bMSSymbol && FT_UseTTCharmap(m_Font.GetFace(), 1, 0);
      }
    }
    FX_BOOL bToUnicode = m_pFontDict->KeyExist("ToUnicode");
    for (int charcode = 0; charcode < 256; charcode++) {
      const FX_CHAR* name =
          GetAdobeCharName(baseEncoding, m_pCharNames, charcode);
      if (!name) {
        m_GlyphIndex[charcode] =
            m_pFontFile ? FXFT_Get_Char_Index(m_Font.GetFace(), charcode) : -1;
        continue;
      }
      m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
      if (bMSSymbol) {
        const uint8_t prefix[4] = {0x00, 0xf0, 0xf1, 0xf2};
        for (int j = 0; j < 4; j++) {
          FX_WORD unicode = prefix[j] * 256 + charcode;
          m_GlyphIndex[charcode] =
              FXFT_Get_Char_Index(m_Font.GetFace(), unicode);
          if (m_GlyphIndex[charcode]) {
            break;
          }
        }
      } else if (m_Encoding.m_Unicodes[charcode]) {
        if (bMSUnicode) {
          m_GlyphIndex[charcode] = FXFT_Get_Char_Index(
              m_Font.GetFace(), m_Encoding.m_Unicodes[charcode]);
        } else if (bMacRoman) {
          FX_DWORD maccode = FT_CharCodeFromUnicode(
              FXFT_ENCODING_APPLE_ROMAN, m_Encoding.m_Unicodes[charcode]);
          if (!maccode) {
            m_GlyphIndex[charcode] =
                FXFT_Get_Name_Index(m_Font.GetFace(), (char*)name);
          } else {
            m_GlyphIndex[charcode] =
                FXFT_Get_Char_Index(m_Font.GetFace(), maccode);
          }
        }
      }
      if ((m_GlyphIndex[charcode] == 0 || m_GlyphIndex[charcode] == 0xffff) &&
          name) {
        if (name[0] == '.' && FXSYS_strcmp(name, ".notdef") == 0) {
          m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.GetFace(), 32);
        } else {
          m_GlyphIndex[charcode] =
              FXFT_Get_Name_Index(m_Font.GetFace(), (char*)name);
          if (m_GlyphIndex[charcode] == 0) {
            if (bToUnicode) {
              CFX_WideString wsUnicode = UnicodeFromCharCode(charcode);
              if (!wsUnicode.IsEmpty()) {
                m_GlyphIndex[charcode] =
                    FXFT_Get_Char_Index(m_Font.GetFace(), wsUnicode[0]);
                m_Encoding.m_Unicodes[charcode] = wsUnicode[0];
              }
            }
            if (m_GlyphIndex[charcode] == 0) {
              m_GlyphIndex[charcode] =
                  FXFT_Get_Char_Index(m_Font.GetFace(), charcode);
            }
          }
        }
      }
    }
    return;
  }
  if (FT_UseTTCharmap(m_Font.GetFace(), 3, 0)) {
    const uint8_t prefix[4] = {0x00, 0xf0, 0xf1, 0xf2};
    FX_BOOL bGotOne = FALSE;
    for (int charcode = 0; charcode < 256; charcode++) {
      for (int j = 0; j < 4; j++) {
        FX_WORD unicode = prefix[j] * 256 + charcode;
        m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.GetFace(), unicode);
        if (m_GlyphIndex[charcode]) {
          bGotOne = TRUE;
          break;
        }
      }
    }
    if (bGotOne) {
      if (baseEncoding != PDFFONT_ENCODING_BUILTIN) {
        for (int charcode = 0; charcode < 256; charcode++) {
          const FX_CHAR* name =
              GetAdobeCharName(baseEncoding, m_pCharNames, charcode);
          if (!name) {
            continue;
          }
          m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
        }
      } else if (FT_UseTTCharmap(m_Font.GetFace(), 1, 0)) {
        for (int charcode = 0; charcode < 256; charcode++) {
          m_Encoding.m_Unicodes[charcode] =
              FT_UnicodeFromCharCode(FXFT_ENCODING_APPLE_ROMAN, charcode);
        }
      }
      return;
    }
  }
  if (FT_UseTTCharmap(m_Font.GetFace(), 1, 0)) {
    FX_BOOL bGotOne = FALSE;
    for (int charcode = 0; charcode < 256; charcode++) {
      m_GlyphIndex[charcode] = FXFT_Get_Char_Index(m_Font.GetFace(), charcode);
      m_Encoding.m_Unicodes[charcode] =
          FT_UnicodeFromCharCode(FXFT_ENCODING_APPLE_ROMAN, charcode);
      if (m_GlyphIndex[charcode]) {
        bGotOne = TRUE;
      }
    }
    if (m_pFontFile || bGotOne) {
      return;
    }
  }
  if (FXFT_Select_Charmap(m_Font.GetFace(), FXFT_ENCODING_UNICODE) == 0) {
    FX_BOOL bGotOne = FALSE;
    const FX_WORD* pUnicodes = PDF_UnicodesForPredefinedCharSet(baseEncoding);
    for (int charcode = 0; charcode < 256; charcode++) {
      if (m_pFontFile) {
        m_Encoding.m_Unicodes[charcode] = charcode;
      } else {
        const FX_CHAR* name = GetAdobeCharName(0, m_pCharNames, charcode);
        if (name) {
          m_Encoding.m_Unicodes[charcode] = PDF_UnicodeFromAdobeName(name);
        } else if (pUnicodes) {
          m_Encoding.m_Unicodes[charcode] = pUnicodes[charcode];
        }
      }
      m_GlyphIndex[charcode] = FXFT_Get_Char_Index(
          m_Font.GetFace(), m_Encoding.m_Unicodes[charcode]);
      if (m_GlyphIndex[charcode]) {
        bGotOne = TRUE;
      }
    }
    if (bGotOne) {
      return;
    }
  }
  for (int charcode = 0; charcode < 256; charcode++) {
    m_GlyphIndex[charcode] = charcode;
  }
}

CPDF_Type3Font::CPDF_Type3Font()
    : CPDF_SimpleFont(PDFFONT_TYPE3),
      m_pCharProcs(nullptr),
      m_pPageResources(nullptr),
      m_pFontResources(nullptr) {
  FXSYS_memset(m_CharWidthL, 0, sizeof(m_CharWidthL));
}

CPDF_Type3Font::~CPDF_Type3Font() {
  for (auto it : m_CacheMap)
    delete it.second;
}

FX_BOOL CPDF_Type3Font::_Load() {
  m_pFontResources = m_pFontDict->GetDict("Resources");
  CPDF_Array* pMatrix = m_pFontDict->GetArray("FontMatrix");
  FX_FLOAT xscale = 1.0f, yscale = 1.0f;
  if (pMatrix) {
    m_FontMatrix = pMatrix->GetMatrix();
    xscale = m_FontMatrix.a;
    yscale = m_FontMatrix.d;
  }
  CPDF_Array* pBBox = m_pFontDict->GetArray("FontBBox");
  if (pBBox) {
    m_FontBBox.left = (int32_t)(FXSYS_Mul(pBBox->GetNumber(0), xscale) * 1000);
    m_FontBBox.bottom =
        (int32_t)(FXSYS_Mul(pBBox->GetNumber(1), yscale) * 1000);
    m_FontBBox.right = (int32_t)(FXSYS_Mul(pBBox->GetNumber(2), xscale) * 1000);
    m_FontBBox.top = (int32_t)(FXSYS_Mul(pBBox->GetNumber(3), yscale) * 1000);
  }
  int StartChar = m_pFontDict->GetInteger("FirstChar");
  CPDF_Array* pWidthArray = m_pFontDict->GetArray("Widths");
  if (pWidthArray && (StartChar >= 0 && StartChar < 256)) {
    FX_DWORD count = pWidthArray->GetCount();
    if (count > 256) {
      count = 256;
    }
    if (StartChar + count > 256) {
      count = 256 - StartChar;
    }
    for (FX_DWORD i = 0; i < count; i++) {
      m_CharWidthL[StartChar + i] =
          FXSYS_round(FXSYS_Mul(pWidthArray->GetNumber(i), xscale) * 1000);
    }
  }
  m_pCharProcs = m_pFontDict->GetDict("CharProcs");
  CPDF_Object* pEncoding = m_pFontDict->GetElementValue("Encoding");
  if (pEncoding) {
    LoadPDFEncoding(pEncoding, m_BaseEncoding, m_pCharNames, FALSE, FALSE);
    if (m_pCharNames) {
      for (int i = 0; i < 256; i++) {
        m_Encoding.m_Unicodes[i] = PDF_UnicodeFromAdobeName(m_pCharNames[i]);
        if (m_Encoding.m_Unicodes[i] == 0) {
          m_Encoding.m_Unicodes[i] = i;
        }
      }
    }
  }
  return TRUE;
}
void CPDF_Type3Font::CheckType3FontMetrics() {
  CheckFontMetrics();
}

CPDF_Type3Char* CPDF_Type3Font::LoadChar(FX_DWORD charcode, int level) {
  if (level >= _FPDF_MAX_TYPE3_FORM_LEVEL_)
    return nullptr;

  auto it = m_CacheMap.find(charcode);
  if (it != m_CacheMap.end())
    return it->second;

  const FX_CHAR* name =
      GetAdobeCharName(m_BaseEncoding, m_pCharNames, charcode);
  if (!name)
    return nullptr;

  CPDF_Stream* pStream =
      ToStream(m_pCharProcs ? m_pCharProcs->GetElementValue(name) : nullptr);
  if (!pStream)
    return nullptr;

  std::unique_ptr<CPDF_Type3Char> pNewChar(new CPDF_Type3Char(new CPDF_Form(
      m_pDocument, m_pFontResources ? m_pFontResources : m_pPageResources,
      pStream, nullptr)));

  // This can trigger recursion into this method. The content of |m_CacheMap|
  // can change as a result. Thus after it returns, check the cache again for
  // a cache hit.
  pNewChar->m_pForm->ParseContent(nullptr, nullptr, pNewChar.get(), nullptr,
                                  level + 1);
  it = m_CacheMap.find(charcode);
  if (it != m_CacheMap.end())
    return it->second;

  FX_FLOAT scale = m_FontMatrix.GetXUnit();
  pNewChar->m_Width = (int32_t)(pNewChar->m_Width * scale + 0.5f);
  FX_RECT& rcBBox = pNewChar->m_BBox;
  CFX_FloatRect char_rect(
      (FX_FLOAT)rcBBox.left / 1000.0f, (FX_FLOAT)rcBBox.bottom / 1000.0f,
      (FX_FLOAT)rcBBox.right / 1000.0f, (FX_FLOAT)rcBBox.top / 1000.0f);
  if (rcBBox.right <= rcBBox.left || rcBBox.bottom >= rcBBox.top)
    char_rect = pNewChar->m_pForm->CalcBoundingBox();

  char_rect.Transform(&m_FontMatrix);
  rcBBox.left = FXSYS_round(char_rect.left * 1000);
  rcBBox.right = FXSYS_round(char_rect.right * 1000);
  rcBBox.top = FXSYS_round(char_rect.top * 1000);
  rcBBox.bottom = FXSYS_round(char_rect.bottom * 1000);

  ASSERT(!pdfium::ContainsKey(m_CacheMap, charcode));
  CPDF_Type3Char* pCachedChar = pNewChar.release();
  m_CacheMap[charcode] = pCachedChar;
  if (pCachedChar->m_pForm->CountObjects() == 0) {
    delete pCachedChar->m_pForm;
    pCachedChar->m_pForm = nullptr;
  }
  return pCachedChar;
}

int CPDF_Type3Font::GetCharWidthF(FX_DWORD charcode, int level) {
  if (charcode >= FX_ArraySize(m_CharWidthL))
    charcode = 0;

  if (m_CharWidthL[charcode])
    return m_CharWidthL[charcode];

  const CPDF_Type3Char* pChar = LoadChar(charcode, level);
  return pChar ? pChar->m_Width : 0;
}

void CPDF_Type3Font::GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level) {
  const CPDF_Type3Char* pChar = LoadChar(charcode, level);
  if (!pChar) {
    rect.left = 0;
    rect.right = 0;
    rect.top = 0;
    rect.bottom = 0;
    return;
  }
  rect = pChar->m_BBox;
}

CPDF_Type3Char::CPDF_Type3Char(CPDF_Form* pForm)
    : m_pForm(pForm), m_pBitmap(nullptr), m_bColored(FALSE) {}

CPDF_Type3Char::~CPDF_Type3Char() {
  delete m_pForm;
  delete m_pBitmap;
}
