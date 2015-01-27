// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "pageint.h"
#include <limits.h>
extern const FX_LPCSTR _PDF_OpCharType =
    "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"
    "IIVIIIIVIIVIIIIIVVIIIIIIIIIIIIII"
    "IIVVVVVVIVVVVVVIVVVVVIIVVIIIIIII"
    "IIVVVVVVVVVVVVVVIVVVIIVVIVVIIIII"
    "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"
    "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"
    "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"
    "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII";
FX_BOOL _PDF_HasInvalidOpChar(FX_LPCSTR op)
{
    if(!op) {
        return FALSE;
    }
    FX_BYTE ch;
    while((ch = *op++)) {
        if(_PDF_OpCharType[ch] == 'I') {
            return TRUE;
        }
    }
    return FALSE;
}
class CPDF_StreamParserAutoClearer {
  public:
    CPDF_StreamParserAutoClearer(CPDF_StreamParser** scoped_variable, CPDF_StreamParser* new_parser)
            : scoped_variable_(scoped_variable) {
        *scoped_variable_ = new_parser;
    }
    ~CPDF_StreamParserAutoClearer() { *scoped_variable_ = NULL; }
  private:
    CPDF_StreamParser** scoped_variable_;
};
FX_DWORD CPDF_StreamContentParser::Parse(FX_LPCBYTE pData, FX_DWORD dwSize, FX_DWORD max_cost)
{
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
                if(!OnOperator((char*)syntax.GetWordBuf()) && _PDF_HasInvalidOpChar((char*)syntax.GetWordBuf())) {
                    m_bAbort = TRUE;
                }
                if (m_bAbort) {
                    return m_pSyntax->GetPos();
                }
                ClearAllParams();
                break;
            case CPDF_StreamParser::Number:
                AddNumberParam((char*)syntax.GetWordBuf(), syntax.GetWordSize());
                break;
            case CPDF_StreamParser::Name:
                AddNameParam((FX_LPCSTR)syntax.GetWordBuf() + 1, syntax.GetWordSize() - 1);
                break;
            default:
                AddObjectParam(syntax.GetObject());
        }
    }
    return m_pSyntax->GetPos();
}
void _PDF_ReplaceAbbr(CPDF_Object* pObj);
void CPDF_StreamContentParser::Handle_BeginImage()
{
    FX_FILESIZE savePos = m_pSyntax->GetPos();
    CPDF_Dictionary* pDict = CPDF_Dictionary::Create();
    while (1) {
        CPDF_StreamParser::SyntaxType type = m_pSyntax->ParseNextElement();
        if (type == CPDF_StreamParser::Keyword) {
            CFX_ByteString bsKeyword(m_pSyntax->GetWordBuf(), m_pSyntax->GetWordSize());
            if (bsKeyword != FX_BSTRC("ID")) {
                m_pSyntax->SetPos(savePos);
                pDict->Release();
                return;
            }
        }
        if (type != CPDF_StreamParser::Name) {
            break;
        }
        CFX_ByteString key((FX_LPCSTR)m_pSyntax->GetWordBuf() + 1, m_pSyntax->GetWordSize() - 1);
        CPDF_Object* pObj = m_pSyntax->ReadNextObject();
        if (!key.IsEmpty()) {
            pDict->SetAt(key, pObj, m_pDocument);
        } else if (pObj) {
            pObj->Release();
        }
    }
    _PDF_ReplaceAbbr(pDict);
    CPDF_Object* pCSObj = NULL;
    if (pDict->KeyExist(FX_BSTRC("ColorSpace"))) {
        pCSObj = pDict->GetElementValue(FX_BSTRC("ColorSpace"));
        if (pCSObj->GetType() == PDFOBJ_NAME) {
            CFX_ByteString name = pCSObj->GetString();
            if (name != FX_BSTRC("DeviceRGB") && name != FX_BSTRC("DeviceGray") && name != FX_BSTRC("DeviceCMYK")) {
                pCSObj = FindResourceObj(FX_BSTRC("ColorSpace"), name);
                if (pCSObj && !pCSObj->GetObjNum()) {
                    pCSObj = pCSObj->Clone();
                    pDict->SetAt(FX_BSTRC("ColorSpace"), pCSObj, m_pDocument);
                }
            }
        }
    }
    CPDF_Stream* pStream = m_pSyntax->ReadInlineStream(m_pDocument, pDict, pCSObj, m_Options.m_bDecodeInlineImage);
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
    pDict->SetAtName(FX_BSTRC("Subtype"), FX_BSTRC("Image"));
    CPDF_ImageObject *pImgObj = AddImage(pStream, NULL, TRUE);
    if (!pImgObj) {
        if (pStream) {
            pStream->Release();
        } else {
            pDict->Release();
        }
    }
}
void CPDF_StreamContentParser::ParsePathObject()
{
    FX_FLOAT params[6] = {0};
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
                            case 'm':
                                AddPathPoint(params[0], params[1], FXPT_MOVETO);
                                nParams = 0;
                                break;
                            case 'l':
                                AddPathPoint(params[0], params[1], FXPT_LINETO);
                                nParams = 0;
                                break;
                            case 'c':
                                AddPathPoint(params[0], params[1], FXPT_BEZIERTO);
                                AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
                                AddPathPoint(params[4], params[5], FXPT_BEZIERTO);
                                nParams = 0;
                                break;
                            case 'v':
                                AddPathPoint(m_PathCurrentX, m_PathCurrentY, FXPT_BEZIERTO);
                                AddPathPoint(params[0], params[1], FXPT_BEZIERTO);
                                AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
                                nParams = 0;
                                break;
                            case 'y':
                                AddPathPoint(params[0], params[1], FXPT_BEZIERTO);
                                AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
                                AddPathPoint(params[2], params[3], FXPT_BEZIERTO);
                                nParams = 0;
                                break;
                            case 'h':
                                Handle_ClosePath();
                                nParams = 0;
                                break;
                            default:
                                bProcessed = FALSE;
                                break;
                        }
                    } else if (len == 2) {
                        if (m_pSyntax->GetWordBuf()[0] == 'r' && m_pSyntax->GetWordBuf()[1] == 'e') {
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
                    FX_atonum(CFX_ByteStringC(m_pSyntax->GetWordBuf(), m_pSyntax->GetWordSize()), bInteger, &value);
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
CPDF_StreamParser::CPDF_StreamParser(const FX_BYTE* pData, FX_DWORD dwSize)
{
    m_pBuf = pData;
    m_Size = dwSize;
    m_Pos = 0;
    m_pLastObj = NULL;
}
CPDF_StreamParser::~CPDF_StreamParser()
{
    if (m_pLastObj) {
        m_pLastObj->Release();
    }
}
FX_DWORD _DecodeAllScanlines(ICodec_ScanlineDecoder* pDecoder, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    if (pDecoder == NULL) {
        return (FX_DWORD) - 1;
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
    dest_size = pitch * height;
    dest_buf = FX_Alloc( FX_BYTE, dest_size);
    for (int row = 0; row < height; row ++) {
        FX_LPBYTE pLine = pDecoder->GetScanline(row);
        if (pLine == NULL) {
            break;
        }
        FXSYS_memcpy32(dest_buf + row * pitch, pLine, pitch);
    }
    FX_DWORD srcoff = pDecoder->GetSrcOffset();
    delete pDecoder;
    return srcoff;
}
ICodec_ScanlineDecoder* FPDFAPI_CreateFaxDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        const CPDF_Dictionary* pParams);
FX_DWORD _A85Decode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
FX_DWORD _HexDecode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
FX_DWORD FPDFAPI_FlateOrLZWDecode(FX_BOOL bLZW, const FX_BYTE* src_buf, FX_DWORD src_size, CPDF_Dictionary* pParams,
                                  FX_DWORD estimated_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
FX_DWORD PDF_DecodeInlineStream(const FX_BYTE* src_buf, FX_DWORD limit,
                                int width, int height, CFX_ByteString& decoder,
                                CPDF_Dictionary* pParam, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    if (decoder == FX_BSTRC("CCITTFaxDecode") || decoder == FX_BSTRC("CCF")) {
        ICodec_ScanlineDecoder* pDecoder = FPDFAPI_CreateFaxDecoder(src_buf, limit, width, height, pParam);
        return _DecodeAllScanlines(pDecoder, dest_buf, dest_size);
    } else if (decoder == FX_BSTRC("ASCII85Decode") || decoder == FX_BSTRC("A85")) {
        return _A85Decode(src_buf, limit, dest_buf, dest_size);
    } else if (decoder == FX_BSTRC("ASCIIHexDecode") || decoder == FX_BSTRC("AHx")) {
        return _HexDecode(src_buf, limit, dest_buf, dest_size);
    } else if (decoder == FX_BSTRC("FlateDecode") || decoder == FX_BSTRC("Fl")) {
        return FPDFAPI_FlateOrLZWDecode(FALSE, src_buf, limit, pParam, dest_size, dest_buf, dest_size);
    } else if (decoder == FX_BSTRC("LZWDecode") || decoder == FX_BSTRC("LZW")) {
        return FPDFAPI_FlateOrLZWDecode(TRUE, src_buf, limit, pParam, 0, dest_buf, dest_size);
    } else if (decoder == FX_BSTRC("DCTDecode") || decoder == FX_BSTRC("DCT")) {
        ICodec_ScanlineDecoder* pDecoder = CPDF_ModuleMgr::Get()->GetJpegModule()->CreateDecoder(
                                               src_buf, limit, width, height, 0, pParam ? pParam->GetInteger(FX_BSTRC("ColorTransform"), 1) : 1);
        return _DecodeAllScanlines(pDecoder, dest_buf, dest_size);
    } else if (decoder == FX_BSTRC("RunLengthDecode") || decoder == FX_BSTRC("RL")) {
        return RunLengthDecode(src_buf, limit, dest_buf, dest_size);
    }
    dest_size = 0;
    dest_buf = 0;
    return (FX_DWORD) - 1;
}
CPDF_Stream* CPDF_StreamParser::ReadInlineStream(CPDF_Document* pDoc, CPDF_Dictionary* pDict, CPDF_Object* pCSObj, FX_BOOL bDecode)
{
    if (m_Pos == m_Size) {
        return NULL;
    }
    if (PDF_CharType[m_pBuf[m_Pos]] == 'W') {
        m_Pos ++;
    }
    CFX_ByteString Decoder;
    CPDF_Dictionary* pParam = NULL;
    CPDF_Object* pFilter = pDict->GetElementValue(FX_BSTRC("Filter"));
    if (pFilter == NULL) {
    } else if (pFilter->GetType() == PDFOBJ_ARRAY) {
        Decoder = ((CPDF_Array*)pFilter)->GetString(0);
        CPDF_Array* pParams = pDict->GetArray(FX_BSTRC("DecodeParms"));
        if (pParams) {
            pParam = pParams->GetDict(0);
        }
    } else {
        Decoder = pFilter->GetString();
        pParam = pDict->GetDict(FX_BSTRC("DecodeParms"));
    }
    FX_DWORD width = pDict->GetInteger(FX_BSTRC("Width"));
    FX_DWORD height = pDict->GetInteger(FX_BSTRC("Height"));
    FX_DWORD OrigSize = 0;
    if (pCSObj != NULL) {
        FX_DWORD bpc = pDict->GetInteger(FX_BSTRC("BitsPerComponent"));
        FX_DWORD nComponents = 1;
        CPDF_ColorSpace* pCS = pDoc->LoadColorSpace(pCSObj);
        if (pCS == NULL) {
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
    FX_LPBYTE pData = NULL;
    FX_DWORD dwStreamSize;
    if (Decoder.IsEmpty()) {
        if (OrigSize > m_Size - m_Pos) {
            OrigSize = m_Size - m_Pos;
        }
        pData = FX_Alloc(FX_BYTE, OrigSize);
        FXSYS_memcpy32(pData, m_pBuf + m_Pos, OrigSize);
        dwStreamSize = OrigSize;
        m_Pos += OrigSize;
    } else {
        FX_DWORD dwDestSize = OrigSize;
        dwStreamSize = PDF_DecodeInlineStream(m_pBuf + m_Pos, m_Size - m_Pos, width, height, Decoder, pParam,
                                              pData, dwDestSize);
        if ((int)dwStreamSize < 0) {
            return NULL;
        }
        if (bDecode) {
            m_Pos += dwStreamSize;
            dwStreamSize = dwDestSize;
            if (pFilter->GetType() == PDFOBJ_ARRAY) {
                ((CPDF_Array*)pFilter)->RemoveAt(0);
                CPDF_Array* pParams = pDict->GetArray(FX_BSTRC("DecodeParms"));
                if (pParams) {
                    pParams->RemoveAt(0);
                }
            } else {
                pDict->RemoveAt(FX_BSTRC("Filter"));
                pDict->RemoveAt(FX_BSTRC("DecodeParms"));
            }
        } else {
            if (pData) {
                FX_Free(pData);
            }
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
            pData = FX_Alloc(FX_BYTE, dwStreamSize);
            FXSYS_memcpy32(pData, m_pBuf + m_Pos, dwStreamSize);
            m_Pos += dwStreamSize;
        }
    }
    pDict->SetAtInteger(FX_BSTRC("Length"), (int)dwStreamSize);
    return CPDF_Stream::Create(pData, dwStreamSize, pDict);
}
#define MAX_WORD_BUFFER 256
#define MAX_STRING_LENGTH	32767
#define FXDWORD_TRUE FXDWORD_FROM_LSBFIRST(0x65757274)
#define FXDWORD_NULL FXDWORD_FROM_LSBFIRST(0x6c6c756e)
#define FXDWORD_FALS FXDWORD_FROM_LSBFIRST(0x736c6166)
CPDF_StreamParser::SyntaxType CPDF_StreamParser::ParseNextElement()
{
    if (m_pLastObj) {
        m_pLastObj->Release();
        m_pLastObj = NULL;
    }
    m_WordSize = 0;
    FX_BOOL bIsNumber = TRUE;
    if (m_Pos >= m_Size) {
        return EndOfData;
    }
    int ch = m_pBuf[m_Pos++];
    int type = PDF_CharType[ch];
    while (1) {
        while (type == 'W') {
            if (m_Size <= m_Pos) {
                return EndOfData;
            }
            ch = m_pBuf[m_Pos++];
            type = PDF_CharType[ch];
        }
        if (ch != '%') {
            break;
        }
        while (1) {
            if (m_Size <= m_Pos) {
                return EndOfData;
            }
            ch = m_pBuf[m_Pos++];
            if (ch == '\r' || ch == '\n') {
                break;
            }
        }
        type = PDF_CharType[ch];
    }
    if (type == 'D' && ch != '/') {
        m_Pos --;
        m_pLastObj = ReadNextObject();
        return Others;
    }
    while (1) {
        if (m_WordSize < MAX_WORD_BUFFER) {
            m_WordBuffer[m_WordSize++] = ch;
        }
        if (type != 'N') {
            bIsNumber = FALSE;
        }
        if (m_Size <= m_Pos) {
            break;
        }
        ch = m_pBuf[m_Pos++];
        type = PDF_CharType[ch];
        if (type == 'D' || type == 'W') {
            m_Pos --;
            break;
        }
    }
    m_WordBuffer[m_WordSize] = 0;
    if (bIsNumber) {
        return Number;
    }
    if (m_WordBuffer[0] == '/') {
        return Name;
    }
    if (m_WordSize == 4) {
        if (*(FX_DWORD*)m_WordBuffer == FXDWORD_TRUE) {
            m_pLastObj = CPDF_Boolean::Create(TRUE);
            return Others;
        }
        if (*(FX_DWORD*)m_WordBuffer == FXDWORD_NULL) {
            m_pLastObj = CPDF_Null::Create();
            return Others;
        }
    } else if (m_WordSize == 5) {
        if (*(FX_DWORD*)m_WordBuffer == FXDWORD_FALS && m_WordBuffer[4] == 'e') {
            m_pLastObj = CPDF_Boolean::Create(FALSE);
            return Others;
        }
    }
    return Keyword;
}
void CPDF_StreamParser::SkipPathObject()
{
    FX_DWORD command_startpos = m_Pos;
    if (m_Pos >= m_Size) {
        return;
    }
    int ch = m_pBuf[m_Pos++];
    int type = PDF_CharType[ch];
    while (1) {
        while (type == 'W') {
            if (m_Pos >= m_Size) {
                return;
            }
            ch = m_pBuf[m_Pos++];
            type = PDF_CharType[ch];
        }
        if (type != 'N') {
            m_Pos = command_startpos;
            return;
        }
        while (1) {
            while (type != 'W') {
                if (m_Pos >= m_Size) {
                    return;
                }
                ch = m_pBuf[m_Pos++];
                type = PDF_CharType[ch];
            }
            while (type == 'W') {
                if (m_Pos >= m_Size) {
                    return;
                }
                ch = m_pBuf[m_Pos++];
                type = PDF_CharType[ch];
            }
            if (type == 'N') {
                continue;
            }
            FX_DWORD op_startpos = m_Pos - 1;
            while (type != 'W' && type != 'D') {
                if (m_Pos >= m_Size) {
                    return;
                }
                ch = m_pBuf[m_Pos++];
                type = PDF_CharType[ch];
            }
            if (m_Pos - op_startpos == 2) {
                int op = m_pBuf[op_startpos];
                if (op == 'm' || op == 'l' || op == 'c' || op == 'v' || op == 'y') {
                    command_startpos = m_Pos;
                    break;
                }
            } else if (m_Pos - op_startpos == 3) {
                if (m_pBuf[op_startpos] == 'r' && m_pBuf[op_startpos + 1] == 'e') {
                    command_startpos = m_Pos;
                    break;
                }
            }
            m_Pos = command_startpos;
            return;
        }
    }
}
CPDF_Object* CPDF_StreamParser::ReadNextObject(FX_BOOL bAllowNestedArray, FX_BOOL bInArray)
{
    FX_BOOL bIsNumber;
    GetNextWord(bIsNumber);
    if (m_WordSize == 0) {
        return NULL;
    }
    if (bIsNumber) {
        m_WordBuffer[m_WordSize] = 0;
        return CPDF_Number::Create(CFX_ByteStringC(m_WordBuffer, m_WordSize));
    }
    int first_char = m_WordBuffer[0];
    if (first_char == '/') {
        return CPDF_Name::Create(PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1)));
    }
    if (first_char == '(') {
        return CPDF_String::Create(ReadString());
    }
    if (first_char == '<') {
        if (m_WordSize == 1) {
            return CPDF_String::Create(ReadHexString(), TRUE);
        }
        CPDF_Dictionary* pDict = CPDF_Dictionary::Create();
        while (1) {
            GetNextWord(bIsNumber);
            if (m_WordSize == 0) {
                pDict->Release();
                return NULL;
            }
            if (m_WordSize == 2 && m_WordBuffer[0] == '>') {
                break;
            }
            if (m_WordBuffer[0] != '/') {
                pDict->Release();
                return NULL;
            }
            CFX_ByteString key = PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1));
            CPDF_Object* pObj = ReadNextObject(TRUE);
            if (pObj == NULL) {
                if (pDict) {
                    pDict->Release();
                }
                return NULL;
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
        CPDF_Array* pArray = CPDF_Array::Create();
        while (1) {
            CPDF_Object* pObj = ReadNextObject(bAllowNestedArray, TRUE);
            if (pObj == NULL) {
                if (m_WordSize == 0 || m_WordBuffer[0] == ']') {
                    return pArray;
                }
                if (m_WordBuffer[0] == '[') {
                    continue;
                }
            } else {
                pArray->Add(pObj);
            }
        }
    }
    if (m_WordSize == 4) {
        if (*(FX_DWORD*)m_WordBuffer == FXDWORD_TRUE) {
            return CPDF_Boolean::Create(TRUE);
        }
        if (*(FX_DWORD*)m_WordBuffer == FXDWORD_NULL) {
            return CPDF_Null::Create();
        }
    } else if (m_WordSize == 5) {
        if (*(FX_DWORD*)m_WordBuffer == FXDWORD_FALS && m_WordBuffer[4] == 'e') {
            return CPDF_Boolean::Create(FALSE);
        }
    }
    return NULL;
}
void CPDF_StreamParser::GetNextWord(FX_BOOL& bIsNumber)
{
    m_WordSize = 0;
    bIsNumber = TRUE;
    if (m_Size <= m_Pos) {
        return;
    }
    int ch = m_pBuf[m_Pos++];
    int type = PDF_CharType[ch];
    while (1) {
        while (type == 'W') {
            if (m_Size <= m_Pos) {
                return;
            }
            ch = m_pBuf[m_Pos++];
            type = PDF_CharType[ch];
        }
        if (ch != '%') {
            break;
        }
        while (1) {
            if (m_Size <= m_Pos) {
                return;
            }
            ch = m_pBuf[m_Pos++];
            if (ch == '\r' || ch == '\n') {
                break;
            }
        }
        type = PDF_CharType[ch];
    }
    if (type == 'D') {
        bIsNumber = FALSE;
        m_WordBuffer[m_WordSize++] = ch;
        if (ch == '/') {
            while (1) {
                if (m_Size <= m_Pos) {
                    return;
                }
                ch = m_pBuf[m_Pos++];
                type = PDF_CharType[ch];
                if (type != 'R' && type != 'N') {
                    m_Pos --;
                    return;
                }
                if (m_WordSize < MAX_WORD_BUFFER) {
                    m_WordBuffer[m_WordSize++] = ch;
                }
            }
        } else if (ch == '<') {
            if (m_Size <= m_Pos) {
                return;
            }
            ch = m_pBuf[m_Pos++];
            if (ch == '<') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        } else if (ch == '>') {
            if (m_Size <= m_Pos) {
                return;
            }
            ch = m_pBuf[m_Pos++];
            if (ch == '>') {
                m_WordBuffer[m_WordSize++] = ch;
            } else {
                m_Pos --;
            }
        }
        return;
    }
    while (1) {
        if (m_WordSize < MAX_WORD_BUFFER) {
            m_WordBuffer[m_WordSize++] = ch;
        }
        if (type != 'N') {
            bIsNumber = FALSE;
        }
        if (m_Size <= m_Pos) {
            return;
        }
        ch = m_pBuf[m_Pos++];
        type = PDF_CharType[ch];
        if (type == 'D' || type == 'W') {
            m_Pos --;
            break;
        }
    }
}
CFX_ByteString CPDF_StreamParser::ReadString()
{
    if (m_Size <= m_Pos) {
        return CFX_ByteString();
    }
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
                    parlevel --;
                    buf.AppendChar(')');
                } else if (ch == '(') {
                    parlevel ++;
                    buf.AppendChar('(');
                } else if (ch == '\\') {
                    status = 1;
                } else {
                    buf.AppendChar((char)ch);
                }
                break;
            case 1:
                if (ch >= '0' && ch <= '7') {
                    iEscCode = ch - '0';
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
                    iEscCode = iEscCode * 8 + ch - '0';
                    status = 3;
                } else {
                    buf.AppendChar(iEscCode);
                    status = 0;
                    continue;
                }
                break;
            case 3:
                if (ch >= '0' && ch <= '7') {
                    iEscCode = iEscCode * 8 + ch - '0';
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
        if (m_Size <= m_Pos) {
            break;
        }
        ch = m_pBuf[m_Pos++];
    }
    if (m_Size > m_Pos) {
        ch = m_pBuf[m_Pos++];
    }
    if (buf.GetLength() > MAX_STRING_LENGTH) {
        return CFX_ByteString(buf.GetBuffer(), MAX_STRING_LENGTH);
    }
    return buf.GetByteString();
}
CFX_ByteString CPDF_StreamParser::ReadHexString()
{
    if (m_Size <= m_Pos) {
        return CFX_ByteString();
    }
    int ch = m_pBuf[m_Pos++];
    CFX_ByteTextBuf buf;
    FX_BOOL bFirst = TRUE;
    int code = 0;
    while (1) {
        if (ch == '>') {
            break;
        }
        if (ch >= '0' && ch <= '9') {
            if (bFirst) {
                code = (ch - '0') * 16;
            } else {
                code += ch - '0';
                buf.AppendChar((char)code);
            }
            bFirst = !bFirst;
        } else if (ch >= 'A' && ch <= 'F') {
            if (bFirst) {
                code = (ch - 'A' + 10) * 16;
            } else {
                code += ch - 'A' + 10;
                buf.AppendChar((char)code);
            }
            bFirst = !bFirst;
        } else if (ch >= 'a' && ch <= 'f') {
            if (bFirst) {
                code = (ch - 'a' + 10) * 16;
            } else {
                code += ch - 'a' + 10;
                buf.AppendChar((char)code);
            }
            bFirst = !bFirst;
        }
        if (m_Size <= m_Pos) {
            break;
        }
        ch = m_pBuf[m_Pos++];
    }
    if (!bFirst) {
        buf.AppendChar((char)code);
    }
    if (buf.GetLength() > MAX_STRING_LENGTH) {
        return CFX_ByteString(buf.GetBuffer(), MAX_STRING_LENGTH);
    }
    return buf.GetByteString();
}
#define PAGEPARSE_STAGE_GETCONTENT		1
#define PAGEPARSE_STAGE_PARSE			2
#define PAGEPARSE_STAGE_CHECKCLIP		3
CPDF_ContentParser::CPDF_ContentParser()
{
    m_pParser = NULL;
    m_pStreamArray = NULL;
    m_pSingleStream = NULL;
    m_pData = NULL;
    m_Status = Ready;
    m_pType3Char = NULL;
}
CPDF_ContentParser::~CPDF_ContentParser()
{
    Clear();
}
void CPDF_ContentParser::Clear()
{
    if (m_pParser) {
        delete m_pParser;
    }
    if (m_pSingleStream) {
        delete m_pSingleStream;
    }
    if (m_pStreamArray) {
        for (FX_DWORD i = 0; i < m_nStreams; i ++)
            if (m_pStreamArray[i]) {
                delete m_pStreamArray[i];
            }
        FX_Free(m_pStreamArray);
    }
    if (m_pData && m_pSingleStream == NULL) {
        FX_Free((void*)m_pData);
    }
    m_pParser = NULL;
    m_pStreamArray = NULL;
    m_pSingleStream = NULL;
    m_pData = NULL;
    m_Status = Ready;
}
void CPDF_ContentParser::Start(CPDF_Page* pPage, CPDF_ParseOptions* pOptions)
{
    if (m_Status != Ready || pPage == NULL || pPage->m_pDocument == NULL || pPage->m_pFormDict == NULL) {
        m_Status = Done;
        return;
    }
    m_pObjects = pPage;
    m_bForm = FALSE;
    if (pOptions) {
        m_Options = *pOptions;
    }
    m_Status = ToBeContinued;
    m_InternalStage = PAGEPARSE_STAGE_GETCONTENT;
    m_CurrentOffset = 0;
    CPDF_Object* pContent = pPage->m_pFormDict->GetElementValue(FX_BSTRC("Contents"));
    if (pContent == NULL) {
        m_Status = Done;
        return;
    }
    if (pContent->GetType() == PDFOBJ_STREAM) {
        m_nStreams = 0;
        m_pSingleStream = FX_NEW CPDF_StreamAcc;
        m_pSingleStream->LoadAllData((CPDF_Stream*)pContent, FALSE);
    } else if (pContent->GetType() == PDFOBJ_ARRAY) {
        CPDF_Array* pArray = (CPDF_Array*)pContent;
        m_nStreams = pArray->GetCount();
        if (m_nStreams == 0) {
            m_Status = Done;
            return;
        }
        m_pStreamArray = FX_Alloc(CPDF_StreamAcc*, m_nStreams);
    } else {
        m_Status = Done;
        return;
    }
}
void CPDF_ContentParser::Start(CPDF_Form* pForm, CPDF_AllStates* pGraphicStates,
                               CFX_AffineMatrix* pParentMatrix, CPDF_Type3Char* pType3Char, CPDF_ParseOptions* pOptions, int level)
{
    m_pType3Char = pType3Char;
    m_pObjects = pForm;
    m_bForm = TRUE;
    CFX_AffineMatrix form_matrix = pForm->m_pFormDict->GetMatrix(FX_BSTRC("Matrix"));
    if (pGraphicStates) {
        form_matrix.Concat(pGraphicStates->m_CTM);
    }
    CPDF_Array* pBBox = pForm->m_pFormDict->GetArray(FX_BSTRC("BBox"));
    CFX_FloatRect form_bbox;
    CPDF_Path ClipPath;
    if (pBBox) {
        form_bbox = pBBox->GetRect();
        ClipPath.New();
        ClipPath.AppendRect(form_bbox.left, form_bbox.bottom, form_bbox.right, form_bbox.top);
        ClipPath.Transform(&form_matrix);
        if (pParentMatrix) {
            ClipPath.Transform(pParentMatrix);
        }
        form_bbox.Transform(&form_matrix);
        if (pParentMatrix) {
            form_bbox.Transform(pParentMatrix);
        }
    }
    CPDF_Dictionary* pResources = pForm->m_pFormDict->GetDict(FX_BSTRC("Resources"));
    m_pParser = FX_NEW CPDF_StreamContentParser;
    m_pParser->Initialize();
    m_pParser->PrepareParse(pForm->m_pDocument, pForm->m_pPageResources, pForm->m_pResources, pParentMatrix, pForm,
                            pResources, &form_bbox, pOptions, pGraphicStates, level);
    m_pParser->m_pCurStates->m_CTM = form_matrix;
    m_pParser->m_pCurStates->m_ParentMatrix = form_matrix;
    if (ClipPath.NotNull()) {
        m_pParser->m_pCurStates->m_ClipPath.AppendPath(ClipPath, FXFILL_WINDING, TRUE);
    }
    if (pForm->m_Transparency & PDFTRANS_GROUP) {
        CPDF_GeneralStateData* pData = m_pParser->m_pCurStates->m_GeneralState.GetModify();
        pData->m_BlendType = FXDIB_BLEND_NORMAL;
        pData->m_StrokeAlpha = 1.0f;
        pData->m_FillAlpha = 1.0f;
        pData->m_pSoftMask = NULL;
    }
    m_nStreams = 0;
    m_pSingleStream = FX_NEW CPDF_StreamAcc;
    if (pForm->m_pDocument) {
        m_pSingleStream->LoadAllData(pForm->m_pFormStream, FALSE);
    } else {
        m_pSingleStream->LoadAllData(pForm->m_pFormStream, FALSE);
    }
    m_pData = (FX_LPBYTE)m_pSingleStream->GetData();
    m_Size = m_pSingleStream->GetSize();
    m_Status = ToBeContinued;
    m_InternalStage = PAGEPARSE_STAGE_PARSE;
    m_CurrentOffset = 0;
}
void CPDF_ContentParser::Continue(IFX_Pause* pPause)
{
    int steps = 0;
    while (m_Status == ToBeContinued) {
        if (m_InternalStage == PAGEPARSE_STAGE_GETCONTENT) {
            if (m_CurrentOffset == m_nStreams) {
                if (m_pStreamArray) {
                    m_Size = 0;
                    FX_DWORD i;
                    for (i = 0; i < m_nStreams; i ++) {
                        FX_DWORD size = m_pStreamArray[i]->GetSize();
                        if (m_Size + size + 1 <= m_Size) {
                            m_Status = Done;
                            return;
                        }
                        m_Size += size + 1;
                    }
                    m_pData = FX_Alloc(FX_BYTE, m_Size);
                    if (!m_pData) {
                        m_Status = Done;
                        return;
                    }
                    FX_DWORD pos = 0;
                    for (i = 0; i < m_nStreams; i ++) {
                        FXSYS_memcpy32(m_pData + pos, m_pStreamArray[i]->GetData(), m_pStreamArray[i]->GetSize());
                        pos += m_pStreamArray[i]->GetSize() + 1;
                        m_pData[pos - 1] = ' ';
                        delete m_pStreamArray[i];
                    }
                    FX_Free(m_pStreamArray);
                    m_pStreamArray = NULL;
                } else {
                    m_pData = (FX_LPBYTE)m_pSingleStream->GetData();
                    m_Size = m_pSingleStream->GetSize();
                }
                m_InternalStage = PAGEPARSE_STAGE_PARSE;
                m_CurrentOffset = 0;
            } else {
                CPDF_Array* pContent = m_pObjects->m_pFormDict->GetArray(FX_BSTRC("Contents"));
                m_pStreamArray[m_CurrentOffset] = FX_NEW CPDF_StreamAcc;
                CPDF_Stream* pStreamObj = (CPDF_Stream*)(pContent ? pContent->GetElementValue(m_CurrentOffset) : NULL);
                m_pStreamArray[m_CurrentOffset]->LoadAllData(pStreamObj, FALSE);
                m_CurrentOffset ++;
            }
        }
        if (m_InternalStage == PAGEPARSE_STAGE_PARSE) {
            if (m_pParser == NULL) {
                m_pParser = FX_NEW CPDF_StreamContentParser;
                m_pParser->Initialize();
                m_pParser->PrepareParse(m_pObjects->m_pDocument, m_pObjects->m_pPageResources, NULL, NULL, m_pObjects,
                                        m_pObjects->m_pResources, &m_pObjects->m_BBox, &m_Options, NULL, 0);
                m_pParser->m_pCurStates->m_ColorState.GetModify()->Default();
            }
            if (m_CurrentOffset >= m_Size) {
                m_InternalStage = PAGEPARSE_STAGE_CHECKCLIP;
            } else {
                m_CurrentOffset += m_pParser->Parse(m_pData + m_CurrentOffset, m_Size - m_CurrentOffset, PARSE_STEP_LIMIT);
                if (m_pParser->m_bAbort) {
                    m_InternalStage = PAGEPARSE_STAGE_CHECKCLIP;
                    continue;
                }
            }
        }
        if (m_InternalStage == PAGEPARSE_STAGE_CHECKCLIP) {
            if (m_pType3Char) {
                m_pType3Char->m_bColored = m_pParser->m_bColored;
                m_pType3Char->m_Width = FXSYS_round(m_pParser->m_Type3Data[0] * 1000);
                m_pType3Char->m_BBox.left = FXSYS_round(m_pParser->m_Type3Data[2] * 1000);
                m_pType3Char->m_BBox.bottom = FXSYS_round(m_pParser->m_Type3Data[3] * 1000);
                m_pType3Char->m_BBox.right = FXSYS_round(m_pParser->m_Type3Data[4] * 1000);
                m_pType3Char->m_BBox.top = FXSYS_round(m_pParser->m_Type3Data[5] * 1000);
            }
            FX_POSITION pos = m_pObjects->m_ObjectList.GetHeadPosition();
            while (pos) {
                CPDF_PageObject* pObj = (CPDF_PageObject*)m_pObjects->m_ObjectList.GetNext(pos);
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
                CFX_FloatRect obj_rect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right, pObj->m_Top);
                if (old_rect.Contains(obj_rect)) {
                    pObj->m_ClipPath.SetNull();
                }
            }
            m_Status = Done;
            return;
        }
        steps ++;
        if (pPause && pPause->NeedToPauseNow()) {
            break;
        }
    }
}
int CPDF_ContentParser::EstimateProgress()
{
    if (m_Status == Ready) {
        return 0;
    }
    if (m_Status == Done) {
        return 100;
    }
    if (m_InternalStage == PAGEPARSE_STAGE_GETCONTENT) {
        return 10;
    }
    if (m_InternalStage == PAGEPARSE_STAGE_CHECKCLIP) {
        return 90;
    }
    return 10 + 80 * m_CurrentOffset / m_Size;
}
