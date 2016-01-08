// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/src/fpdfapi/fpdf_page/pageint.h"

#include <limits.h>

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fxcodec/fx_codec.h"
#include "core/include/fxcrt/fx_ext.h"
#include "core/include/fxcrt/fx_safe_types.h"

namespace {

const char kPathOperatorSubpath = 'm';
const char kPathOperatorLine = 'l';
const char kPathOperatorCubicBezier1 = 'c';
const char kPathOperatorCubicBezier2 = 'v';
const char kPathOperatorCubicBezier3 = 'y';
const char kPathOperatorClosePath = 'h';
const char kPathOperatorRectangle[] = "re";

}  // namespace

class CPDF_StreamParserAutoClearer {
 public:
  CPDF_StreamParserAutoClearer(CPDF_StreamParser** scoped_variable,
                               CPDF_StreamParser* new_parser)
      : scoped_variable_(scoped_variable) {
    *scoped_variable_ = new_parser;
  }
  ~CPDF_StreamParserAutoClearer() { *scoped_variable_ = NULL; }

 private:
  CPDF_StreamParser** scoped_variable_;
};
FX_DWORD CPDF_StreamContentParser::Parse(const uint8_t* pData,
                                         FX_DWORD dwSize,
                                         FX_DWORD max_cost) {
  if (m_Level > _FPDF_MAX_FORM_LEVEL_) {
    return dwSize;
  }
  FX_DWORD InitObjCount = m_pObjectList->CountObjects();
  CPDF_StreamParser syntax(pData, dwSize);
  CPDF_StreamParserAutoClearer auto_clearer(&m_pSyntax, &syntax);
  m_CompatCount = 0;
  while (1) {
    FX_DWORD cost = m_pObjectList->CountObjects() - InitObjCount;
    if (max_cost && cost >= max_cost) {
      break;
    }
    switch (syntax.ParseNextElement()) {
      case CPDF_StreamParser::EndOfData:
        return m_pSyntax->GetPos();
      case CPDF_StreamParser::Keyword:
        OnOperator((char*)syntax.GetWordBuf());
        ClearAllParams();
        break;
      case CPDF_StreamParser::Number:
        AddNumberParam((char*)syntax.GetWordBuf(), syntax.GetWordSize());
        break;
      case CPDF_StreamParser::Name:
        AddNameParam((const FX_CHAR*)syntax.GetWordBuf() + 1,
                     syntax.GetWordSize() - 1);
        break;
      default:
        AddObjectParam(syntax.GetObject());
    }
  }
  return m_pSyntax->GetPos();
}

void CPDF_StreamContentParser::Handle_BeginImage() {
  FX_FILESIZE savePos = m_pSyntax->GetPos();
  CPDF_Dictionary* pDict = new CPDF_Dictionary;
  while (1) {
    CPDF_StreamParser::SyntaxType type = m_pSyntax->ParseNextElement();
    if (type == CPDF_StreamParser::Keyword) {
      CFX_ByteString bsKeyword(m_pSyntax->GetWordBuf(),
                               m_pSyntax->GetWordSize());
      if (bsKeyword != "ID") {
        m_pSyntax->SetPos(savePos);
        pDict->Release();
        return;
      }
    }
    if (type != CPDF_StreamParser::Name) {
      break;
    }
    CFX_ByteString key((const FX_CHAR*)m_pSyntax->GetWordBuf() + 1,
                       m_pSyntax->GetWordSize() - 1);
    std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>> pObj(
        m_pSyntax->ReadNextObject());
    if (!key.IsEmpty()) {
      FX_DWORD dwObjNum = pObj ? pObj->GetObjNum() : 0;
      if (dwObjNum)
        pDict->SetAtReference(key, m_pDocument, dwObjNum);
      else
        pDict->SetAt(key, pObj.release());
    }
  }
  PDF_ReplaceAbbr(pDict);
  CPDF_Object* pCSObj = NULL;
  if (pDict->KeyExist("ColorSpace")) {
    pCSObj = pDict->GetElementValue("ColorSpace");
    if (pCSObj->IsName()) {
      CFX_ByteString name = pCSObj->GetString();
      if (name != "DeviceRGB" && name != "DeviceGray" && name != "DeviceCMYK") {
        pCSObj = FindResourceObj("ColorSpace", name);
        if (pCSObj && !pCSObj->GetObjNum()) {
          pCSObj = pCSObj->Clone();
          pDict->SetAt("ColorSpace", pCSObj);
        }
      }
    }
  }
  CPDF_Stream* pStream = m_pSyntax->ReadInlineStream(
      m_pDocument, pDict, pCSObj, m_Options.m_bDecodeInlineImage);
  while (1) {
    CPDF_StreamParser::SyntaxType type = m_pSyntax->ParseNextElement();
    if (type == CPDF_StreamParser::EndOfData) {
      break;
    }
    if (type != CPDF_StreamParser::Keyword) {
      continue;
    }
    if (m_pSyntax->GetWordSize() == 2 && m_pSyntax->GetWordBuf()[0] == 'E' &&
        m_pSyntax->GetWordBuf()[1] == 'I') {
      break;
    }
  }
  if (m_Options.m_bTextOnly) {
    if (pStream) {
      pStream->Release();
    } else {
      pDict->Release();
    }
    return;
  }
  pDict->SetAtName("Subtype", "Image");
  CPDF_ImageObject* pImgObj = AddImage(pStream, NULL, TRUE);
  if (!pImgObj) {
    if (pStream) {
      pStream->Release();
    } else {
      pDict->Release();
    }
  }
}
void CPDF_StreamContentParser::ParsePathObject() {
  FX_FLOAT params[6] = {};
  int nParams = 0;
  int last_pos = m_pSyntax->GetPos();
  while (1) {
    CPDF_StreamParser::SyntaxType type = m_pSyntax->ParseNextElement();
    FX_BOOL bProcessed = TRUE;
    switch (type) {
      case CPDF_StreamParser::EndOfData:
        return;
      case CPDF_StreamParser::Keyword: {
        int len = m_pSyntax->GetWordSize();
        if (len == 1) {
          switch (m_pSyntax->GetWordBuf()[0]) {
            case kPathOperatorSubpath:
              AddPathPoint(params[0], params[1], FXPT_MOVETO);
              nParams = 0;
              break;
            case kPathOperatorLine:
              AddPathPoint(params[0], params[1], FXPT_LINETO);
              nParams = 0;
              break;
            case kPathOperatorCubicBezier1:
              AddPathPoint(params[0], params[1], FXPT_BEZIERTO);
              AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
              AddPathPoint(params[4], params[5], FXPT_BEZIERTO);
              nParams = 0;
              break;
            case kPathOperatorCubicBezier2:
              AddPathPoint(m_PathCurrentX, m_PathCurrentY, FXPT_BEZIERTO);
              AddPathPoint(params[0], params[1], FXPT_BEZIERTO);
              AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
              nParams = 0;
              break;
            case kPathOperatorCubicBezier3:
              AddPathPoint(params[0], params[1], FXPT_BEZIERTO);
              AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
              AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
              nParams = 0;
              break;
            case kPathOperatorClosePath:
              Handle_ClosePath();
              nParams = 0;
              break;
            default:
              bProcessed = FALSE;
              break;
          }
        } else if (len == 2) {
          if (m_pSyntax->GetWordBuf()[0] == kPathOperatorRectangle[0] &&
              m_pSyntax->GetWordBuf()[1] == kPathOperatorRectangle[1]) {
            AddPathRect(params[0], params[1], params[2], params[3]);
            nParams = 0;
          } else {
            bProcessed = FALSE;
          }
        } else {
          bProcessed = FALSE;
        }
        if (bProcessed) {
          last_pos = m_pSyntax->GetPos();
        }
        break;
      }
      case CPDF_StreamParser::Number: {
        if (nParams == 6) {
          break;
        }
        FX_BOOL bInteger;
        int value;
        FX_atonum(
            CFX_ByteStringC(m_pSyntax->GetWordBuf(), m_pSyntax->GetWordSize()),
            bInteger, &value);
        params[nParams++] = bInteger ? (FX_FLOAT)value : *(FX_FLOAT*)&value;
        break;
      }
      default:
        bProcessed = FALSE;
    }
    if (!bProcessed) {
      m_pSyntax->SetPos(last_pos);
      return;
    }
  }
}
CPDF_StreamParser::CPDF_StreamParser(const uint8_t* pData, FX_DWORD dwSize) {
  m_pBuf = pData;
  m_Size = dwSize;
  m_Pos = 0;
  m_pLastObj = NULL;
}
CPDF_StreamParser::~CPDF_StreamParser() {
  if (m_pLastObj) {
    m_pLastObj->Release();
  }
}
FX_DWORD _DecodeAllScanlines(ICodec_ScanlineDecoder* pDecoder,
                             uint8_t*& dest_buf,
                             FX_DWORD& dest_size) {
  if (!pDecoder) {
    return (FX_DWORD)-1;
  }
  int ncomps = pDecoder->CountComps();
  int bpc = pDecoder->GetBPC();
  int width = pDecoder->GetWidth();
  int height = pDecoder->GetHeight();
  int pitch = (width * ncomps * bpc + 7) / 8;
  if (height == 0 || pitch > (1 << 30) / height) {
    delete pDecoder;
    return -1;
  }
  dest_buf = FX_Alloc2D(uint8_t, pitch, height);
  dest_size = pitch * height;  // Safe since checked alloc returned.
  for (int row = 0; row < height; row++) {
    const uint8_t* pLine = pDecoder->GetScanline(row);
    if (!pLine)
      break;

    FXSYS_memcpy(dest_buf + row * pitch, pLine, pitch);
  }
  FX_DWORD srcoff = pDecoder->GetSrcOffset();
  delete pDecoder;
  return srcoff;
}
ICodec_ScanlineDecoder* FPDFAPI_CreateFaxDecoder(
    const uint8_t* src_buf,
    FX_DWORD src_size,
    int width,
    int height,
    const CPDF_Dictionary* pParams);

FX_DWORD PDF_DecodeInlineStream(const uint8_t* src_buf,
                                FX_DWORD limit,
                                int width,
                                int height,
                                CFX_ByteString& decoder,
                                CPDF_Dictionary* pParam,
                                uint8_t*& dest_buf,
                                FX_DWORD& dest_size) {
  if (decoder == "CCITTFaxDecode" || decoder == "CCF") {
    ICodec_ScanlineDecoder* pDecoder =
        FPDFAPI_CreateFaxDecoder(src_buf, limit, width, height, pParam);
    return _DecodeAllScanlines(pDecoder, dest_buf, dest_size);
  }
  if (decoder == "ASCII85Decode" || decoder == "A85") {
    return A85Decode(src_buf, limit, dest_buf, dest_size);
  }
  if (decoder == "ASCIIHexDecode" || decoder == "AHx") {
    return HexDecode(src_buf, limit, dest_buf, dest_size);
  }
  if (decoder == "FlateDecode" || decoder == "Fl") {
    return FPDFAPI_FlateOrLZWDecode(FALSE, src_buf, limit, pParam, dest_size,
                                    dest_buf, dest_size);
  }
  if (decoder == "LZWDecode" || decoder == "LZW") {
    return FPDFAPI_FlateOrLZWDecode(TRUE, src_buf, limit, pParam, 0, dest_buf,
                                    dest_size);
  }
  if (decoder == "DCTDecode" || decoder == "DCT") {
    ICodec_ScanlineDecoder* pDecoder =
        CPDF_ModuleMgr::Get()->GetJpegModule()->CreateDecoder(
            src_buf, limit, width, height, 0,
            pParam ? pParam->GetInteger("ColorTransform", 1) : 1);
    return _DecodeAllScanlines(pDecoder, dest_buf, dest_size);
  }
  if (decoder == "RunLengthDecode" || decoder == "RL") {
    return RunLengthDecode(src_buf, limit, dest_buf, dest_size);
  }
  dest_size = 0;
  dest_buf = 0;
  return (FX_DWORD)-1;
}
CPDF_Stream* CPDF_StreamParser::ReadInlineStream(CPDF_Document* pDoc,
                                                 CPDF_Dictionary* pDict,
                                                 CPDF_Object* pCSObj,
                                                 FX_BOOL bDecode) {
  if (m_Pos == m_Size)
    return nullptr;

  if (PDFCharIsWhitespace(m_pBuf[m_Pos]))
    m_Pos++;

  CFX_ByteString Decoder;
  CPDF_Dictionary* pParam = nullptr;
  CPDF_Object* pFilter = pDict->GetElementValue("Filter");
  if (pFilter) {
    if (CPDF_Array* pArray = pFilter->AsArray()) {
      Decoder = pArray->GetString(0);
      CPDF_Array* pParams = pDict->GetArray("DecodeParms");
      if (pParams)
        pParam = pParams->GetDict(0);
    } else {
      Decoder = pFilter->GetString();
      pParam = pDict->GetDict("DecodeParms");
    }
  }
  FX_DWORD width = pDict->GetInteger("Width");
  FX_DWORD height = pDict->GetInteger("Height");
  FX_DWORD OrigSize = 0;
  if (pCSObj) {
    FX_DWORD bpc = pDict->GetInteger("BitsPerComponent");
    FX_DWORD nComponents = 1;
    CPDF_ColorSpace* pCS = pDoc->LoadColorSpace(pCSObj);
    if (!pCS) {
      nComponents = 3;
    } else {
      nComponents = pCS->CountComponents();
      pDoc->GetPageData()->ReleaseColorSpace(pCSObj);
    }
    FX_DWORD pitch = width;
    if (bpc && pitch > INT_MAX / bpc) {
      return NULL;
    }
    pitch *= bpc;
    if (nComponents && pitch > INT_MAX / nComponents) {
      return NULL;
    }
    pitch *= nComponents;
    if (pitch > INT_MAX - 7) {
      return NULL;
    }
    pitch += 7;
    pitch /= 8;
    OrigSize = pitch;
  } else {
    if (width > INT_MAX - 7) {
      return NULL;
    }
    OrigSize = ((width + 7) / 8);
  }
  if (height && OrigSize > INT_MAX / height) {
    return NULL;
  }
  OrigSize *= height;
  uint8_t* pData = NULL;
  FX_DWORD dwStreamSize;
  if (Decoder.IsEmpty()) {
    if (OrigSize > m_Size - m_Pos) {
      OrigSize = m_Size - m_Pos;
    }
    pData = FX_Alloc(uint8_t, OrigSize);
    FXSYS_memcpy(pData, m_pBuf + m_Pos, OrigSize);
    dwStreamSize = OrigSize;
    m_Pos += OrigSize;
  } else {
    FX_DWORD dwDestSize = OrigSize;
    dwStreamSize =
        PDF_DecodeInlineStream(m_pBuf + m_Pos, m_Size - m_Pos, width, height,
                               Decoder, pParam, pData, dwDestSize);
    if ((int)dwStreamSize < 0) {
      FX_Free(pData);
      return NULL;
    }
    if (bDecode) {
      m_Pos += dwStreamSize;
      dwStreamSize = dwDestSize;
      if (CPDF_Array* pArray = pFilter->AsArray()) {
        pArray->RemoveAt(0);
        CPDF_Array* pParams = pDict->GetArray("DecodeParms");
        if (pParams)
          pParams->RemoveAt(0);
      } else {
        pDict->RemoveAt("Filter");
        pDict->RemoveAt("DecodeParms");
      }
    } else {
      FX_Free(pData);
      FX_DWORD dwSavePos = m_Pos;
      m_Pos += dwStreamSize;
      while (1) {
        FX_DWORD dwPrevPos = m_Pos;
        CPDF_StreamParser::SyntaxType type = ParseNextElement();
        if (type == CPDF_StreamParser::EndOfData) {
          break;
        }
        if (type != CPDF_StreamParser::Keyword) {
          dwStreamSize += m_Pos - dwPrevPos;
          continue;
        }
        if (GetWordSize() == 2 && GetWordBuf()[0] == 'E' &&
            GetWordBuf()[1] == 'I') {
          m_Pos = dwPrevPos;
          break;
        }
        dwStreamSize += m_Pos - dwPrevPos;
      }
      m_Pos = dwSavePos;
      pData = FX_Alloc(uint8_t, dwStreamSize);
      FXSYS_memcpy(pData, m_pBuf + m_Pos, dwStreamSize);
      m_Pos += dwStreamSize;
    }
  }
  pDict->SetAtInteger("Length", (int)dwStreamSize);
  return new CPDF_Stream(pData, dwStreamSize, pDict);
}

#define MAX_WORD_BUFFER 256
#define MAX_STRING_LENGTH 32767
#define FXDWORD_TRUE FXDWORD_FROM_LSBFIRST(0x65757274)
#define FXDWORD_NULL FXDWORD_FROM_LSBFIRST(0x6c6c756e)
#define FXDWORD_FALS FXDWORD_FROM_LSBFIRST(0x736c6166)
CPDF_StreamParser::SyntaxType CPDF_StreamParser::ParseNextElement() {
  if (m_pLastObj) {
    m_pLastObj->Release();
    m_pLastObj = nullptr;
  }

  m_WordSize = 0;
  FX_BOOL bIsNumber = TRUE;
  if (!PositionIsInBounds())
    return EndOfData;

  int ch = m_pBuf[m_Pos++];
  while (1) {
    while (PDFCharIsWhitespace(ch)) {
      if (!PositionIsInBounds())
        return EndOfData;

      ch = m_pBuf[m_Pos++];
    }

    if (ch != '%')
      break;

    while (1) {
      if (!PositionIsInBounds())
        return EndOfData;

      ch = m_pBuf[m_Pos++];
      if (PDFCharIsLineEnding(ch))
        break;
    }
  }

  if (PDFCharIsDelimiter(ch) && ch != '/') {
    m_Pos--;
    m_pLastObj = ReadNextObject();
    return Others;
  }

  while (1) {
    if (m_WordSize < MAX_WORD_BUFFER)
      m_WordBuffer[m_WordSize++] = ch;

    if (!PDFCharIsNumeric(ch))
      bIsNumber = FALSE;

    if (!PositionIsInBounds())
      break;

    ch = m_pBuf[m_Pos++];

    if (PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch)) {
      m_Pos--;
      break;
    }
  }

  m_WordBuffer[m_WordSize] = 0;
  if (bIsNumber)
    return Number;
  if (m_WordBuffer[0] == '/')
    return Name;

  if (m_WordSize == 4) {
    if (*(FX_DWORD*)m_WordBuffer == FXDWORD_TRUE) {
      m_pLastObj = new CPDF_Boolean(TRUE);
      return Others;
    }
    if (*(FX_DWORD*)m_WordBuffer == FXDWORD_NULL) {
      m_pLastObj = new CPDF_Null;
      return Others;
    }
  } else if (m_WordSize == 5) {
    if (*(FX_DWORD*)m_WordBuffer == FXDWORD_FALS && m_WordBuffer[4] == 'e') {
      m_pLastObj = new CPDF_Boolean(FALSE);
      return Others;
    }
  }
  return Keyword;
}

void CPDF_StreamParser::SkipPathObject() {
  FX_DWORD command_startpos = m_Pos;
  if (!PositionIsInBounds())
    return;

  int ch = m_pBuf[m_Pos++];
  while (1) {
    while (PDFCharIsWhitespace(ch)) {
      if (!PositionIsInBounds())
        return;
      ch = m_pBuf[m_Pos++];
    }

    if (!PDFCharIsNumeric(ch)) {
      m_Pos = command_startpos;
      return;
    }

    while (1) {
      while (!PDFCharIsWhitespace(ch)) {
        if (!PositionIsInBounds())
          return;
        ch = m_pBuf[m_Pos++];
      }

      while (PDFCharIsWhitespace(ch)) {
        if (!PositionIsInBounds())
          return;
        ch = m_pBuf[m_Pos++];
      }

      if (PDFCharIsNumeric(ch))
        continue;

      FX_DWORD op_startpos = m_Pos - 1;
      while (!PDFCharIsWhitespace(ch) && !PDFCharIsDelimiter(ch)) {
        if (!PositionIsInBounds())
          return;
        ch = m_pBuf[m_Pos++];
      }

      if (m_Pos - op_startpos == 2) {
        int op = m_pBuf[op_startpos];
        if (op == kPathOperatorSubpath || op == kPathOperatorLine ||
            op == kPathOperatorCubicBezier1 ||
            op == kPathOperatorCubicBezier2 ||
            op == kPathOperatorCubicBezier3) {
          command_startpos = m_Pos;
          break;
        }
      } else if (m_Pos - op_startpos == 3) {
        if (m_pBuf[op_startpos] == kPathOperatorRectangle[0] &&
            m_pBuf[op_startpos + 1] == kPathOperatorRectangle[1]) {
          command_startpos = m_Pos;
          break;
        }
      }
      m_Pos = command_startpos;
      return;
    }
  }
}
CPDF_Object* CPDF_StreamParser::ReadNextObject(FX_BOOL bAllowNestedArray,
                                               FX_BOOL bInArray) {
  FX_BOOL bIsNumber;
  GetNextWord(bIsNumber);
  if (m_WordSize == 0) {
    return NULL;
  }
  if (bIsNumber) {
    m_WordBuffer[m_WordSize] = 0;
    return new CPDF_Number(CFX_ByteStringC(m_WordBuffer, m_WordSize));
  }
  int first_char = m_WordBuffer[0];
  if (first_char == '/') {
    return new CPDF_Name(
        PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1)));
  }
  if (first_char == '(') {
    return new CPDF_String(ReadString(), FALSE);
  }
  if (first_char == '<') {
    if (m_WordSize == 1) {
      return new CPDF_String(ReadHexString(), TRUE);
    }
    CPDF_Dictionary* pDict = new CPDF_Dictionary;
    while (1) {
      GetNextWord(bIsNumber);
      if (m_WordSize == 0) {
        pDict->Release();
        return nullptr;
      }
      if (m_WordSize == 2 && m_WordBuffer[0] == '>') {
        break;
      }
      if (m_WordBuffer[0] != '/') {
        pDict->Release();
        return nullptr;
      }
      CFX_ByteString key =
          PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1));
      CPDF_Object* pObj = ReadNextObject(TRUE);
      if (!pObj) {
        pDict->Release();
        return nullptr;
      }
      if (!key.IsEmpty()) {
        pDict->SetAt(key, pObj);
      } else {
        pObj->Release();
      }
    }
    return pDict;
  }
  if (first_char == '[') {
    if (!bAllowNestedArray && bInArray) {
      return NULL;
    }
    CPDF_Array* pArray = new CPDF_Array;
    while (1) {
      CPDF_Object* pObj = ReadNextObject(bAllowNestedArray, TRUE);
      if (pObj) {
        pArray->Add(pObj);
        continue;
      }

      if (m_WordSize == 0 || m_WordBuffer[0] == ']')
        break;
    }
    return pArray;
  }
  if (m_WordSize == 4) {
    if (*(FX_DWORD*)m_WordBuffer == FXDWORD_TRUE) {
      return new CPDF_Boolean(TRUE);
    }
    if (*(FX_DWORD*)m_WordBuffer == FXDWORD_NULL) {
      return new CPDF_Null;
    }
  } else if (m_WordSize == 5) {
    if (*(FX_DWORD*)m_WordBuffer == FXDWORD_FALS && m_WordBuffer[4] == 'e') {
      return new CPDF_Boolean(FALSE);
    }
  }
  return NULL;
}
void CPDF_StreamParser::GetNextWord(FX_BOOL& bIsNumber) {
  m_WordSize = 0;
  bIsNumber = TRUE;
  if (!PositionIsInBounds())
    return;

  int ch = m_pBuf[m_Pos++];
  while (1) {
    while (PDFCharIsWhitespace(ch)) {
      if (!PositionIsInBounds()) {
        return;
      }
      ch = m_pBuf[m_Pos++];
    }

    if (ch != '%')
      break;

    while (1) {
      if (!PositionIsInBounds())
        return;
      ch = m_pBuf[m_Pos++];
      if (PDFCharIsLineEnding(ch))
        break;
    }
  }

  if (PDFCharIsDelimiter(ch)) {
    bIsNumber = FALSE;
    m_WordBuffer[m_WordSize++] = ch;
    if (ch == '/') {
      while (1) {
        if (!PositionIsInBounds())
          return;
        ch = m_pBuf[m_Pos++];
        if (!PDFCharIsOther(ch) && !PDFCharIsNumeric(ch)) {
          m_Pos--;
          return;
        }

        if (m_WordSize < MAX_WORD_BUFFER)
          m_WordBuffer[m_WordSize++] = ch;
      }
    } else if (ch == '<') {
      if (!PositionIsInBounds())
        return;
      ch = m_pBuf[m_Pos++];
      if (ch == '<')
        m_WordBuffer[m_WordSize++] = ch;
      else
        m_Pos--;
    } else if (ch == '>') {
      if (!PositionIsInBounds())
        return;
      ch = m_pBuf[m_Pos++];
      if (ch == '>')
        m_WordBuffer[m_WordSize++] = ch;
      else
        m_Pos--;
    }
    return;
  }

  while (1) {
    if (m_WordSize < MAX_WORD_BUFFER)
      m_WordBuffer[m_WordSize++] = ch;
    if (!PDFCharIsNumeric(ch))
      bIsNumber = FALSE;

    if (!PositionIsInBounds())
      return;
    ch = m_pBuf[m_Pos++];
    if (PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch)) {
      m_Pos--;
      break;
    }
  }
}

CFX_ByteString CPDF_StreamParser::ReadString() {
  if (!PositionIsInBounds())
    return CFX_ByteString();

  int ch = m_pBuf[m_Pos++];
  CFX_ByteTextBuf buf;
  int parlevel = 0;
  int status = 0, iEscCode = 0;
  while (1) {
    switch (status) {
      case 0:
        if (ch == ')') {
          if (parlevel == 0) {
            if (buf.GetLength() > MAX_STRING_LENGTH) {
              return CFX_ByteString(buf.GetBuffer(), MAX_STRING_LENGTH);
            }
            return buf.GetByteString();
          }
          parlevel--;
          buf.AppendChar(')');
        } else if (ch == '(') {
          parlevel++;
          buf.AppendChar('(');
        } else if (ch == '\\') {
          status = 1;
        } else {
          buf.AppendChar((char)ch);
        }
        break;
      case 1:
        if (ch >= '0' && ch <= '7') {
          iEscCode = FXSYS_toDecimalDigit(ch);
          status = 2;
          break;
        }
        if (ch == 'n') {
          buf.AppendChar('\n');
        } else if (ch == 'r') {
          buf.AppendChar('\r');
        } else if (ch == 't') {
          buf.AppendChar('\t');
        } else if (ch == 'b') {
          buf.AppendChar('\b');
        } else if (ch == 'f') {
          buf.AppendChar('\f');
        } else if (ch == '\r') {
          status = 4;
          break;
        } else if (ch == '\n') {
        } else {
          buf.AppendChar(ch);
        }
        status = 0;
        break;
      case 2:
        if (ch >= '0' && ch <= '7') {
          iEscCode = iEscCode * 8 + FXSYS_toDecimalDigit(ch);
          status = 3;
        } else {
          buf.AppendChar(iEscCode);
          status = 0;
          continue;
        }
        break;
      case 3:
        if (ch >= '0' && ch <= '7') {
          iEscCode = iEscCode * 8 + FXSYS_toDecimalDigit(ch);
          buf.AppendChar(iEscCode);
          status = 0;
        } else {
          buf.AppendChar(iEscCode);
          status = 0;
          continue;
        }
        break;
      case 4:
        status = 0;
        if (ch != '\n') {
          continue;
        }
        break;
    }
    if (!PositionIsInBounds())
      break;

    ch = m_pBuf[m_Pos++];
  }
  if (PositionIsInBounds())
    ch = m_pBuf[m_Pos++];

  if (buf.GetLength() > MAX_STRING_LENGTH) {
    return CFX_ByteString(buf.GetBuffer(), MAX_STRING_LENGTH);
  }
  return buf.GetByteString();
}
CFX_ByteString CPDF_StreamParser::ReadHexString() {
  if (!PositionIsInBounds())
    return CFX_ByteString();

  CFX_ByteTextBuf buf;
  bool bFirst = true;
  int code = 0;
  while (PositionIsInBounds()) {
    int ch = m_pBuf[m_Pos++];

    if (ch == '>')
      break;

    if (!std::isxdigit(ch))
      continue;

    int val = FXSYS_toHexDigit(ch);
    if (bFirst) {
      code = val * 16;
    } else {
      code += val;
      buf.AppendByte((uint8_t)code);
    }
    bFirst = !bFirst;
  }
  if (!bFirst)
    buf.AppendChar((char)code);

  if (buf.GetLength() > MAX_STRING_LENGTH)
    return CFX_ByteString(buf.GetBuffer(), MAX_STRING_LENGTH);

  return buf.GetByteString();
}

bool CPDF_StreamParser::PositionIsInBounds() const {
  return m_Pos < m_Size;
}

CPDF_ContentParser::CPDF_ContentParser()
    : m_Status(Ready),
      m_InternalStage(STAGE_GETCONTENT),
      m_pObjects(nullptr),
      m_bForm(false),
      m_pType3Char(nullptr),
      m_pData(nullptr),
      m_Size(0),
      m_CurrentOffset(0) {}
CPDF_ContentParser::~CPDF_ContentParser() {
  if (!m_pSingleStream)
    FX_Free(m_pData);
}
void CPDF_ContentParser::Start(CPDF_Page* pPage, CPDF_ParseOptions* pOptions) {
  if (m_Status != Ready || !pPage || !pPage->m_pDocument ||
      !pPage->m_pFormDict) {
    m_Status = Done;
    return;
  }
  m_pObjects = pPage;
  m_bForm = FALSE;
  if (pOptions) {
    m_Options = *pOptions;
  }
  m_Status = ToBeContinued;
  m_InternalStage = STAGE_GETCONTENT;
  m_CurrentOffset = 0;

  CPDF_Object* pContent = pPage->m_pFormDict->GetElementValue("Contents");
  if (!pContent) {
    m_Status = Done;
    return;
  }
  if (CPDF_Stream* pStream = pContent->AsStream()) {
    m_nStreams = 0;
    m_pSingleStream.reset(new CPDF_StreamAcc);
    m_pSingleStream->LoadAllData(pStream, FALSE);
  } else if (CPDF_Array* pArray = pContent->AsArray()) {
    m_nStreams = pArray->GetCount();
    if (m_nStreams)
      m_StreamArray.resize(m_nStreams);
    else
      m_Status = Done;
  } else {
    m_Status = Done;
  }
}
void CPDF_ContentParser::Start(CPDF_Form* pForm,
                               CPDF_AllStates* pGraphicStates,
                               CFX_Matrix* pParentMatrix,
                               CPDF_Type3Char* pType3Char,
                               CPDF_ParseOptions* pOptions,
                               int level) {
  m_pType3Char = pType3Char;
  m_pObjects = pForm;
  m_bForm = TRUE;
  CFX_Matrix form_matrix = pForm->m_pFormDict->GetMatrix("Matrix");
  if (pGraphicStates) {
    form_matrix.Concat(pGraphicStates->m_CTM);
  }
  CPDF_Array* pBBox = pForm->m_pFormDict->GetArray("BBox");
  CFX_FloatRect form_bbox;
  CPDF_Path ClipPath;
  if (pBBox) {
    form_bbox = pBBox->GetRect();
    ClipPath.New();
    ClipPath.AppendRect(form_bbox.left, form_bbox.bottom, form_bbox.right,
                        form_bbox.top);
    ClipPath.Transform(&form_matrix);
    if (pParentMatrix) {
      ClipPath.Transform(pParentMatrix);
    }
    form_bbox.Transform(&form_matrix);
    if (pParentMatrix) {
      form_bbox.Transform(pParentMatrix);
    }
  }
  CPDF_Dictionary* pResources = pForm->m_pFormDict->GetDict("Resources");
  m_pParser.reset(new CPDF_StreamContentParser(
      pForm->m_pDocument, pForm->m_pPageResources, pForm->m_pResources,
      pParentMatrix, pForm, pResources, &form_bbox, pOptions, pGraphicStates,
      level));
  m_pParser->GetCurStates()->m_CTM = form_matrix;
  m_pParser->GetCurStates()->m_ParentMatrix = form_matrix;
  if (ClipPath.NotNull()) {
    m_pParser->GetCurStates()->m_ClipPath.AppendPath(ClipPath, FXFILL_WINDING,
                                                     TRUE);
  }
  if (pForm->m_Transparency & PDFTRANS_GROUP) {
    CPDF_GeneralStateData* pData =
        m_pParser->GetCurStates()->m_GeneralState.GetModify();
    pData->m_BlendType = FXDIB_BLEND_NORMAL;
    pData->m_StrokeAlpha = 1.0f;
    pData->m_FillAlpha = 1.0f;
    pData->m_pSoftMask = NULL;
  }
  m_nStreams = 0;
  m_pSingleStream.reset(new CPDF_StreamAcc);
  m_pSingleStream->LoadAllData(pForm->m_pFormStream, FALSE);
  m_pData = (uint8_t*)m_pSingleStream->GetData();
  m_Size = m_pSingleStream->GetSize();
  m_Status = ToBeContinued;
  m_InternalStage = STAGE_PARSE;
  m_CurrentOffset = 0;
}
void CPDF_ContentParser::Continue(IFX_Pause* pPause) {
  int steps = 0;
  while (m_Status == ToBeContinued) {
    if (m_InternalStage == STAGE_GETCONTENT) {
      if (m_CurrentOffset == m_nStreams) {
        if (!m_StreamArray.empty()) {
          FX_SAFE_DWORD safeSize = 0;
          for (const auto& stream : m_StreamArray) {
            safeSize += stream->GetSize();
            safeSize += 1;
          }
          if (!safeSize.IsValid()) {
            m_Status = Done;
            return;
          }
          m_Size = safeSize.ValueOrDie();
          m_pData = FX_Alloc(uint8_t, m_Size);
          FX_DWORD pos = 0;
          for (const auto& stream : m_StreamArray) {
            FXSYS_memcpy(m_pData + pos, stream->GetData(), stream->GetSize());
            pos += stream->GetSize();
            m_pData[pos++] = ' ';
          }
          m_StreamArray.clear();
        } else {
          m_pData = (uint8_t*)m_pSingleStream->GetData();
          m_Size = m_pSingleStream->GetSize();
        }
        m_InternalStage = STAGE_PARSE;
        m_CurrentOffset = 0;
      } else {
        CPDF_Array* pContent = m_pObjects->m_pFormDict->GetArray("Contents");
        m_StreamArray[m_CurrentOffset].reset(new CPDF_StreamAcc);
        CPDF_Stream* pStreamObj = ToStream(
            pContent ? pContent->GetElementValue(m_CurrentOffset) : nullptr);
        m_StreamArray[m_CurrentOffset]->LoadAllData(pStreamObj, FALSE);
        m_CurrentOffset++;
      }
    }
    if (m_InternalStage == STAGE_PARSE) {
      if (!m_pParser) {
        m_pParser.reset(new CPDF_StreamContentParser(
            m_pObjects->m_pDocument, m_pObjects->m_pPageResources, nullptr,
            nullptr, m_pObjects, m_pObjects->m_pResources, &m_pObjects->m_BBox,
            &m_Options, nullptr, 0));
        m_pParser->GetCurStates()->m_ColorState.GetModify()->Default();
      }
      if (m_CurrentOffset >= m_Size) {
        m_InternalStage = STAGE_CHECKCLIP;
      } else {
        m_CurrentOffset +=
            m_pParser->Parse(m_pData + m_CurrentOffset,
                             m_Size - m_CurrentOffset, PARSE_STEP_LIMIT);
      }
    }
    if (m_InternalStage == STAGE_CHECKCLIP) {
      if (m_pType3Char) {
        m_pType3Char->m_bColored = m_pParser->IsColored();
        m_pType3Char->m_Width =
            FXSYS_round(m_pParser->GetType3Data()[0] * 1000);
        m_pType3Char->m_BBox.left =
            FXSYS_round(m_pParser->GetType3Data()[2] * 1000);
        m_pType3Char->m_BBox.bottom =
            FXSYS_round(m_pParser->GetType3Data()[3] * 1000);
        m_pType3Char->m_BBox.right =
            FXSYS_round(m_pParser->GetType3Data()[4] * 1000);
        m_pType3Char->m_BBox.top =
            FXSYS_round(m_pParser->GetType3Data()[5] * 1000);
      }
      FX_POSITION pos = m_pObjects->m_ObjectList.GetHeadPosition();
      while (pos) {
        CPDF_PageObject* pObj =
            (CPDF_PageObject*)m_pObjects->m_ObjectList.GetNext(pos);
        if (pObj->m_ClipPath.IsNull()) {
          continue;
        }
        if (pObj->m_ClipPath.GetPathCount() != 1) {
          continue;
        }
        if (pObj->m_ClipPath.GetTextCount()) {
          continue;
        }
        CPDF_Path ClipPath = pObj->m_ClipPath.GetPath(0);
        if (!ClipPath.IsRect() || pObj->m_Type == PDFPAGE_SHADING) {
          continue;
        }
        CFX_FloatRect old_rect(ClipPath.GetPointX(0), ClipPath.GetPointY(0),
                               ClipPath.GetPointX(2), ClipPath.GetPointY(2));
        CFX_FloatRect obj_rect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right,
                               pObj->m_Top);
        if (old_rect.Contains(obj_rect)) {
          pObj->m_ClipPath.SetNull();
        }
      }
      m_Status = Done;
      return;
    }
    steps++;
    if (pPause && pPause->NeedToPauseNow()) {
      break;
    }
  }
}
