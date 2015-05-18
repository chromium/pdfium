// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../fx_zlib.h"
#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "filters_int.h"
CFX_DataFilter::CFX_DataFilter()
{
    m_bEOF = FALSE;
    m_pDestFilter = NULL;
    m_SrcPos = 0;
}
CFX_DataFilter::~CFX_DataFilter()
{
    if (m_pDestFilter) {
        delete m_pDestFilter;
    }
}
void CFX_DataFilter::SetDestFilter(CFX_DataFilter* pFilter)
{
    if (m_pDestFilter) {
        m_pDestFilter->SetDestFilter(pFilter);
    } else {
        m_pDestFilter = pFilter;
    }
}
void CFX_DataFilter::FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    if (m_bEOF) {
        return;
    }
    m_SrcPos += src_size;
    if (m_pDestFilter) {
        CFX_BinaryBuf temp_buf;
        temp_buf.EstimateSize(FPDF_FILTER_BUFFER_SIZE, FPDF_FILTER_BUFFER_SIZE);
        v_FilterIn(src_buf, src_size, temp_buf);
        m_pDestFilter->FilterIn(temp_buf.GetBuffer(), temp_buf.GetSize(), dest_buf);
    } else {
        v_FilterIn(src_buf, src_size, dest_buf);
    }
}
void CFX_DataFilter::FilterFinish(CFX_BinaryBuf& dest_buf)
{
    if (m_pDestFilter) {
        CFX_BinaryBuf temp_buf;
        v_FilterFinish(temp_buf);
        if (temp_buf.GetSize()) {
            m_pDestFilter->FilterIn(temp_buf.GetBuffer(), temp_buf.GetSize(), dest_buf);
        }
        m_pDestFilter->FilterFinish(dest_buf);
    } else {
        v_FilterFinish(dest_buf);
    }
    m_bEOF = TRUE;
}
void CFX_DataFilter::ReportEOF(FX_DWORD left_input)
{
    if (m_bEOF) {
        return;
    }
    m_bEOF = TRUE;
    m_SrcPos -= left_input;
}
CFX_DataFilter* FPDF_CreateFilter(FX_BSTR name, const CPDF_Dictionary* pParam, int width, int height)
{
    FX_DWORD id = name.GetID();
    switch (id) {
        case FXBSTR_ID('F', 'l', 'a', 't'):
        case FXBSTR_ID('F', 'l', 0, 0):
        case FXBSTR_ID('L', 'Z', 'W', 'D'):
        case FXBSTR_ID('L', 'Z', 'W', 0): {
                CFX_DataFilter* pFilter;
                if (id == FXBSTR_ID('L', 'Z', 'W', 'D') || id == FXBSTR_ID('L', 'Z', 'W', 0)) {
                    pFilter = new CPDF_LzwFilter(pParam ? pParam->GetInteger("EarlyChange", 1) : 1);
                } else {
                    pFilter = new CPDF_FlateFilter;
                }
                if ((pParam ? pParam->GetInteger("Predictor", 1) : 1) > 1) {
                    CFX_DataFilter* pPredictor = new CPDF_PredictorFilter(pParam->GetInteger(FX_BSTRC("Predictor"), 1),
                                                 pParam->GetInteger(FX_BSTRC("Colors"), 1), pParam->GetInteger(FX_BSTRC("BitsPerComponent"), 8),
                                                 pParam->GetInteger(FX_BSTRC("Columns"), 1));
                    pFilter->SetDestFilter(pPredictor);
                }
                return pFilter;
            }
        case FXBSTR_ID('A', 'S', 'C', 'I'):
            if (name == "ASCIIHexDecode") {
                return new CPDF_AsciiHexFilter;
            }
            return new CPDF_Ascii85Filter;
        case FXBSTR_ID('A', 'H', 'x', 0):
            return new CPDF_AsciiHexFilter;
        case FXBSTR_ID('A', '8', '5', 0):
            return new CPDF_Ascii85Filter;
        case FXBSTR_ID('R', 'u', 'n', 'L'):
            return new CPDF_RunLenFilter;
        case FXBSTR_ID('C', 'C', 'I', 'T'): {
                int Encoding = 0;
                int bEndOfLine = FALSE;
                int bByteAlign = FALSE;
                int bBlack = FALSE;
                int nRows = 0;
                int nColumns = 1728;
                if (pParam) {
                    Encoding = pParam->GetInteger(FX_BSTRC("K"));
                    bEndOfLine = pParam->GetInteger(FX_BSTRC("EndOfLine"));
                    bByteAlign = pParam->GetInteger(FX_BSTRC("EncodedByteAlign"));
                    bBlack = pParam->GetInteger(FX_BSTRC("BlackIs1"));
                    nColumns = pParam->GetInteger(FX_BSTRC("Columns"), 1728);
                    nRows = pParam->GetInteger(FX_BSTRC("Rows"));
                }
                if (nColumns == 0) {
                    nColumns = width;
                }
                if (nRows == 0) {
                    nRows = height;
                }
                CPDF_FaxFilter* pFilter = new CPDF_FaxFilter();
                pFilter->Initialize(Encoding, bEndOfLine, bByteAlign, bBlack, nRows, nColumns);
                return pFilter;
            }
        case FXBSTR_ID('D', 'C', 'T', 'D'):
            return new CPDF_JpegFilter;
        default:
            return NULL;
    }
}
CFX_DataFilter* _FPDF_CreateFilterFromDict(CPDF_Dictionary* pDict)
{
    CPDF_Object* pDecoder = pDict->GetElementValue("Filter");
    if (pDecoder == NULL) {
        return NULL;
    }
    CFX_DataFilter* pFirstFilter = NULL;
    int width = pDict->GetInteger(FX_BSTRC("Width")), height = pDict->GetInteger(FX_BSTRC("Height"));
    CPDF_Object* pParams = pDict->GetElementValue("DecodeParms");
    if (pDecoder->GetType() == PDFOBJ_ARRAY) {
        if (pParams && pParams->GetType() != PDFOBJ_ARRAY) {
            pParams = NULL;
        }
        for (FX_DWORD i = 0; i < ((CPDF_Array*)pDecoder)->GetCount(); i ++) {
            CFX_ByteString name = ((CPDF_Array*)pDecoder)->GetString(i);
            CPDF_Dictionary* pParam = NULL;
            if (pParams) {
                pParam = ((CPDF_Array*)pParams)->GetDict(i);
            }
            CFX_DataFilter* pDestFilter = FPDF_CreateFilter(name, pParam, width, height);
            if (pDestFilter) {
                if (pFirstFilter == NULL) {
                    pFirstFilter = pDestFilter;
                } else {
                    pFirstFilter->SetDestFilter(pDestFilter);
                }
            }
        }
    } else {
        if (pParams && pParams->GetType() != PDFOBJ_DICTIONARY) {
            pParams = NULL;
        }
        pFirstFilter = FPDF_CreateFilter(pDecoder->GetString(), (CPDF_Dictionary*)pParams, width, height);
    }
    return pFirstFilter;
}
CPDF_StreamFilter* CPDF_Stream::GetStreamFilter(FX_BOOL bRaw) const
{
    CFX_DataFilter* pFirstFilter = NULL;
    if (m_pCryptoHandler) {
        pFirstFilter = new CPDF_DecryptFilter(m_pCryptoHandler, m_ObjNum, m_GenNum);
    }
    if (!bRaw) {
        CFX_DataFilter* pFilter = _FPDF_CreateFilterFromDict(m_pDict);
        if (pFilter) {
            if (pFirstFilter == NULL) {
                pFirstFilter = pFilter;
            } else {
                pFirstFilter->SetDestFilter(pFilter);
            }
        }
    }
    CPDF_StreamFilter* pStreamFilter = new CPDF_StreamFilter;
    pStreamFilter->m_pStream = this;
    pStreamFilter->m_pFilter = pFirstFilter;
    pStreamFilter->m_pBuffer = NULL;
    pStreamFilter->m_SrcOffset = 0;
    return pStreamFilter;
}
CPDF_StreamFilter::~CPDF_StreamFilter()
{
    if (m_pFilter) {
        delete m_pFilter;
    }
    if (m_pBuffer) {
        delete m_pBuffer;
    }
}
#define FPDF_FILTER_BUFFER_IN_SIZE	FPDF_FILTER_BUFFER_SIZE
FX_DWORD CPDF_StreamFilter::ReadBlock(FX_LPBYTE buffer, FX_DWORD buf_size)
{
    if (m_pFilter == NULL) {
        FX_DWORD read_size = m_pStream->GetRawSize() - m_SrcOffset;
        if (read_size == 0) {
            return 0;
        }
        if (read_size > buf_size) {
            read_size = buf_size;
        }
        m_pStream->ReadRawData(m_SrcOffset, buffer, read_size);
        m_SrcOffset += read_size;
        return read_size;
    }
    FX_DWORD read_size = 0;
    if (m_pBuffer) {
        read_size = ReadLeftOver(buffer, buf_size);
        if (read_size == buf_size) {
            return read_size;
        }
        buffer += read_size;
        buf_size -= read_size;
    }
    ASSERT(m_pBuffer == NULL);
    if (m_pFilter->IsEOF()) {
        return read_size;
    }
    m_pBuffer = new CFX_BinaryBuf;
    m_pBuffer->EstimateSize(FPDF_FILTER_BUFFER_SIZE, FPDF_FILTER_BUFFER_SIZE);
    m_BufOffset = 0;
    while (1) {
        int src_size = m_pStream->GetRawSize() - m_SrcOffset;
        if (src_size == 0) {
            m_pFilter->FilterFinish(*m_pBuffer);
            break;
        }
        if (src_size > FPDF_FILTER_BUFFER_IN_SIZE) {
            src_size = FPDF_FILTER_BUFFER_IN_SIZE;
        }
        if (!m_pStream->ReadRawData(m_SrcOffset, m_SrcBuffer, src_size)) {
            return 0;
        }
        m_SrcOffset += src_size;
        m_pFilter->FilterIn(m_SrcBuffer, src_size, *m_pBuffer);
        if (m_pBuffer->GetSize() >= (int)buf_size) {
            break;
        }
    }
    return read_size + ReadLeftOver(buffer, buf_size);
}
FX_DWORD CPDF_StreamFilter::ReadLeftOver(FX_LPBYTE buffer, FX_DWORD buf_size)
{
    FX_DWORD read_size = m_pBuffer->GetSize() - m_BufOffset;
    if (read_size > buf_size) {
        read_size = buf_size;
    }
    FXSYS_memcpy32(buffer, m_pBuffer->GetBuffer() + m_BufOffset, read_size);
    m_BufOffset += read_size;
    if (m_BufOffset == (FX_DWORD)m_pBuffer->GetSize()) {
        delete m_pBuffer;
        m_pBuffer = NULL;
    }
    return read_size;
}
CPDF_DecryptFilter::CPDF_DecryptFilter(CPDF_CryptoHandler* pCryptoHandler, FX_DWORD objnum, FX_DWORD gennum)
{
    m_pCryptoHandler = pCryptoHandler;
    m_pContext = NULL;
    m_ObjNum = objnum;
    m_GenNum = gennum;
}
CPDF_DecryptFilter::~CPDF_DecryptFilter()
{
    CFX_BinaryBuf buf;
    if (m_pContext) {
        m_pCryptoHandler->DecryptFinish(m_pContext, buf);
    }
}
void CPDF_DecryptFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    if (m_pContext == NULL) {
        m_pContext = m_pCryptoHandler->DecryptStart(m_ObjNum, m_GenNum);
    }
    m_pCryptoHandler->DecryptStream(m_pContext, src_buf, src_size, dest_buf);
}
void CPDF_DecryptFilter::v_FilterFinish(CFX_BinaryBuf& dest_buf)
{
    m_bEOF = TRUE;
    if (m_pContext == NULL) {
        return;
    }
    m_pCryptoHandler->DecryptFinish(m_pContext, dest_buf);
    m_pContext = NULL;
}
extern "C" {
    static void* my_alloc_func (void* opaque, unsigned int items, unsigned int size)
    {
        return FX_Alloc2D(FX_BYTE, items, size);
    }
    static void   my_free_func  (void* opaque, void* address)
    {
        FX_Free(address);
    }
    void* FPDFAPI_FlateInit(void* (*alloc_func)(void*, unsigned int, unsigned int),
                            void (*free_func)(void*, void*));
    void FPDFAPI_FlateInput(void* context, const unsigned char* src_buf, unsigned int src_size);
    int FPDFAPI_FlateOutput(void* context, unsigned char* dest_buf, unsigned int dest_size);
    int FPDFAPI_FlateGetAvailIn(void* context);
    int FPDFAPI_FlateGetAvailOut(void* context);
    void FPDFAPI_FlateEnd(void* context);
}
CPDF_FlateFilter::CPDF_FlateFilter()
{
    m_pContext = NULL;
}
CPDF_FlateFilter::~CPDF_FlateFilter()
{
    if (m_pContext) {
        FPDFAPI_FlateEnd(m_pContext);
    }
}
void CPDF_FlateFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    if (m_pContext == NULL) {
        m_pContext = FPDFAPI_FlateInit(my_alloc_func, my_free_func);
    }
    FPDFAPI_FlateInput(m_pContext, src_buf, src_size);
    while (1) {
        int ret = FPDFAPI_FlateOutput(m_pContext, m_DestBuffer, FPDF_FILTER_BUFFER_SIZE);
        int out_size = FPDF_FILTER_BUFFER_SIZE - FPDFAPI_FlateGetAvailOut(m_pContext);
        dest_buf.AppendBlock(m_DestBuffer, out_size);
        if (ret == Z_BUF_ERROR) {
            break;
        }
        if (ret != Z_OK) {
            ReportEOF(FPDFAPI_FlateGetAvailIn(m_pContext));
            break;
        }
    }
}
CPDF_LzwFilter::CPDF_LzwFilter(FX_BOOL bEarlyChange)
{
    m_bEarlyChange = bEarlyChange ? 1 : 0;
    m_CodeLen = 9;
    m_nCodes = 0;
    m_nLeftBits = 0;
    m_LeftBits = 0;
    m_OldCode = (FX_DWORD) - 1;
}
void CPDF_LzwFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    for (FX_DWORD i = 0; i < src_size; i ++) {
        if (m_nLeftBits + 8 < m_CodeLen) {
            m_nLeftBits += 8;
            m_LeftBits = (m_LeftBits << 8) | src_buf[i];
            continue;
        }
        FX_DWORD new_bits = m_CodeLen - m_nLeftBits;
        FX_DWORD code = (m_LeftBits << new_bits) | (src_buf[i] >> (8 - new_bits));
        m_nLeftBits = 8 - new_bits;
        m_LeftBits = src_buf[i] % (1 << m_nLeftBits);
        if (code < 256) {
            dest_buf.AppendByte((FX_BYTE)code);
            m_LastChar = (FX_BYTE)code;
            if (m_OldCode != -1) {
                AddCode(m_OldCode, m_LastChar);
            }
            m_OldCode = code;
        } else if (code == 256) {
            m_CodeLen = 9;
            m_nCodes = 0;
            m_OldCode = (FX_DWORD) - 1;
        } else if (code == 257) {
            ReportEOF(src_size - i - 1);
            return;
        } else {
            if (m_OldCode == -1) {
                ReportEOF(src_size - i - 1);
                return;
            }
            m_StackLen = 0;
            if (code >= m_nCodes + 258) {
                if (m_StackLen < sizeof(m_DecodeStack)) {
                    m_DecodeStack[m_StackLen++] = m_LastChar;
                }
                DecodeString(m_OldCode);
            } else {
                DecodeString(code);
            }
            dest_buf.AppendBlock(NULL, m_StackLen);
            FX_LPBYTE pOutput = dest_buf.GetBuffer() + dest_buf.GetSize() - m_StackLen;
            for (FX_DWORD cc = 0; cc < m_StackLen; cc ++) {
                pOutput[cc] = m_DecodeStack[m_StackLen - cc - 1];
            }
            m_LastChar = m_DecodeStack[m_StackLen - 1];
            if (m_OldCode < 256) {
                AddCode(m_OldCode, m_LastChar);
            } else if (m_OldCode - 258 >= m_nCodes) {
                ReportEOF(src_size - i - 1);
                return;
            } else {
                AddCode(m_OldCode, m_LastChar);
            }
            m_OldCode = code;
        }
    }
}
void CPDF_LzwFilter::AddCode(FX_DWORD prefix_code, FX_BYTE append_char)
{
    if (m_nCodes + m_bEarlyChange == 4094) {
        return;
    }
    m_CodeArray[m_nCodes ++] = (prefix_code << 16) | append_char;
    if (m_nCodes + m_bEarlyChange == 512 - 258) {
        m_CodeLen = 10;
    } else if (m_nCodes + m_bEarlyChange == 1024 - 258) {
        m_CodeLen = 11;
    } else if (m_nCodes + m_bEarlyChange == 2048 - 258) {
        m_CodeLen = 12;
    }
}
void CPDF_LzwFilter::DecodeString(FX_DWORD code)
{
    while (1) {
        int index = code - 258;
        if (index < 0 || index >= (int)m_nCodes) {
            break;
        }
        FX_DWORD data = m_CodeArray[index];
        if (m_StackLen >= sizeof(m_DecodeStack)) {
            return;
        }
        m_DecodeStack[m_StackLen++] = (FX_BYTE)data;
        code = data >> 16;
    }
    if (m_StackLen >= sizeof(m_DecodeStack)) {
        return;
    }
    m_DecodeStack[m_StackLen++] = (FX_BYTE)code;
}
CPDF_PredictorFilter::CPDF_PredictorFilter(int predictor, int colors, int bpc, int cols)
{
    m_bTiff = predictor < 10;
    m_pRefLine = NULL;
    m_pCurLine = NULL;
    m_iLine = 0;
    m_LineInSize = 0;
    m_Bpp = (colors * bpc + 7) / 8;
    m_Pitch = (colors * bpc * cols + 7) / 8;
    if (!m_bTiff) {
        m_Pitch ++;
    }
}
CPDF_PredictorFilter::~CPDF_PredictorFilter()
{
    if (m_pCurLine) {
        FX_Free(m_pCurLine);
    }
    if (m_pRefLine) {
        FX_Free(m_pRefLine);
    }
}
static FX_BYTE PaethPredictor(int a, int b, int c)
{
    int p = a + b - c;
    int pa = FXSYS_abs(p - a);
    int pb = FXSYS_abs(p - b);
    int pc = FXSYS_abs(p - c);
    if (pa <= pb && pa <= pc) {
        return (FX_BYTE)a;
    }
    if (pb <= pc) {
        return (FX_BYTE)b;
    }
    return (FX_BYTE)c;
}
static void PNG_PredictorLine(FX_LPBYTE cur_buf, FX_LPBYTE ref_buf, int pitch, int Bpp)
{
    FX_BYTE tag = cur_buf[0];
    if (tag == 0) {
        return;
    }
    cur_buf ++;
    if (ref_buf) {
        ref_buf ++;
    }
    for (int byte = 0; byte < pitch; byte ++) {
        FX_BYTE raw_byte = cur_buf[byte];
        switch (tag) {
            case 1:	{
                    FX_BYTE left = 0;
                    if (byte >= Bpp) {
                        left = cur_buf[byte - Bpp];
                    }
                    cur_buf[byte] = raw_byte + left;
                    break;
                }
            case 2: {
                    FX_BYTE up = 0;
                    if (ref_buf) {
                        up = ref_buf[byte];
                    }
                    cur_buf[byte] = raw_byte + up;
                    break;
                }
            case 3: {
                    FX_BYTE left = 0;
                    if (byte >= Bpp) {
                        left = cur_buf[byte - Bpp];
                    }
                    FX_BYTE up = 0;
                    if (ref_buf) {
                        up = ref_buf[byte];
                    }
                    cur_buf[byte] = raw_byte + (up + left) / 2;
                    break;
                }
            case 4: {
                    FX_BYTE left = 0;
                    if (byte >= Bpp) {
                        left = cur_buf[byte - Bpp];
                    }
                    FX_BYTE up = 0;
                    if (ref_buf) {
                        up = ref_buf[byte];
                    }
                    FX_BYTE upper_left = 0;
                    if (byte >= Bpp && ref_buf) {
                        upper_left = ref_buf[byte - Bpp];
                    }
                    cur_buf[byte] = raw_byte + PaethPredictor(left, up, upper_left);
                    break;
                }
        }
    }
}
void CPDF_PredictorFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    if (m_pCurLine == NULL) {
        m_pCurLine = FX_Alloc(FX_BYTE, m_Pitch);
        if (!m_bTiff) {
            m_pRefLine = FX_Alloc(FX_BYTE, m_Pitch);
        }
    }
    while (1) {
        FX_DWORD read_size = m_Pitch - m_LineInSize;
        if (read_size > src_size) {
            read_size = src_size;
        }
        FXSYS_memcpy32(m_pCurLine + m_LineInSize, src_buf, read_size);
        m_LineInSize += read_size;
        if (m_LineInSize < m_Pitch) {
            break;
        }
        src_buf += read_size;
        src_size -= read_size;
        if (m_bTiff) {
            for (FX_DWORD byte = m_Bpp; byte < m_Pitch; byte ++) {
                m_pCurLine[byte] += m_pCurLine[byte - m_Bpp];
            }
            dest_buf.AppendBlock(m_pCurLine, m_Pitch);
        } else {
            PNG_PredictorLine(m_pCurLine, m_iLine ? m_pRefLine : NULL, m_Pitch - 1, m_Bpp);
            dest_buf.AppendBlock(m_pCurLine + 1, m_Pitch - 1);
            m_iLine ++;
            FX_LPBYTE temp = m_pCurLine;
            m_pCurLine = m_pRefLine;
            m_pRefLine = temp;
        }
        m_LineInSize = 0;
    }
}
CPDF_Ascii85Filter::CPDF_Ascii85Filter()
{
    m_State = 0;
    m_CharCount = 0;
}
void CPDF_Ascii85Filter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    for (FX_DWORD i = 0; i < src_size; i ++) {
        FX_BYTE byte = src_buf[i];
        if (PDF_CharType[byte] == 'W') {
            continue;
        }
        switch (m_State) {
            case 0:
                if (byte >= '!' && byte <= 'u') {
                    int digit = byte - '!';
                    m_CurDWord = digit;
                    m_CharCount = 1;
                    m_State = 1;
                } else if (byte == 'z') {
                    int zero = 0;
                    dest_buf.AppendBlock(&zero, 4);
                } else if (byte == '~') {
                    m_State = 2;
                }
                break;
            case 1: {
                    if (byte >= '!' && byte <= 'u') {
                        int digit = byte - '!';
                        m_CurDWord = m_CurDWord * 85 + digit;
                        m_CharCount ++;
                        if (m_CharCount == 5) {
                            for (int i = 0; i < 4; i ++) {
                                dest_buf.AppendByte((FX_BYTE)(m_CurDWord >> (3 - i) * 8));
                            }
                            m_State = 0;
                        }
                    } else if (byte == '~') {
                        if (m_CharCount > 1) {
                            int i;
                            for (i = m_CharCount; i < 5; i ++) {
                                m_CurDWord = m_CurDWord * 85 + 84;
                            }
                            for (i = 0; i < m_CharCount - 1; i ++) {
                                dest_buf.AppendByte((FX_BYTE)(m_CurDWord >> (3 - i) * 8));
                            }
                        }
                        m_State = 2;
                    }
                    break;
                }
            case 2:
                if (byte == '>') {
                    ReportEOF(src_size - i - 1);
                    return;
                }
                break;
        }
    }
}
CPDF_AsciiHexFilter::CPDF_AsciiHexFilter()
{
    m_State = 0;
}
void CPDF_AsciiHexFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    for (FX_DWORD i = 0; i < src_size; i ++) {
        FX_BYTE byte = src_buf[i];
        if (PDF_CharType[byte] == 'W') {
            continue;
        }
        int digit;
        if (byte >= '0' && byte <= '9') {
            digit = byte - '0';
        } else if (byte >= 'a' && byte <= 'f') {
            digit = byte - 'a' + 10;
        } else if (byte >= 'A' && byte <= 'F') {
            digit = byte - 'A' + 10;
        } else {
            if (m_State) {
                dest_buf.AppendByte(m_FirstDigit * 16);
            }
            ReportEOF(src_size - i - 1);
            return;
        }
        if (m_State == 0) {
            m_FirstDigit = digit;
            m_State ++;
        } else {
            dest_buf.AppendByte(m_FirstDigit * 16 + digit);
            m_State --;
        }
    }
}
CPDF_RunLenFilter::CPDF_RunLenFilter()
{
    m_State = 0;
    m_Count = 0;
}
void CPDF_RunLenFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    for (FX_DWORD i = 0; i < src_size; i ++) {
        FX_BYTE byte = src_buf[i];
        switch (m_State) {
            case 0:
                if (byte < 128) {
                    m_State = 1;
                    m_Count = byte + 1;
                } else if (byte == 128) {
                    ReportEOF(src_size - i - 1);
                    return;
                } else {
                    m_State = 2;
                    m_Count = 257 - byte;
                }
                break;
            case 1:
                dest_buf.AppendByte(byte);
                m_Count --;
                if (m_Count == 0) {
                    m_State = 0;
                }
                break;
            case 2:	{
                    dest_buf.AppendBlock(NULL, m_Count);
                    FXSYS_memset8(dest_buf.GetBuffer() + dest_buf.GetSize() - m_Count, byte, m_Count);
                    m_State = 0;
                    break;
                }
        }
    }
}
CPDF_JpegFilter::CPDF_JpegFilter()
{
    m_pContext = NULL;
    m_bGotHeader = FALSE;
    m_pScanline = NULL;
    m_iLine = 0;
}
CPDF_JpegFilter::~CPDF_JpegFilter()
{
    if (m_pScanline) {
        FX_Free(m_pScanline);
    }
    if (m_pContext) {
        CPDF_ModuleMgr::Get()->GetJpegModule()->Finish(m_pContext);
    }
}
void CPDF_JpegFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    if (m_pContext == NULL) {
        m_pContext = CPDF_ModuleMgr::Get()->GetJpegModule()->Start();
    }
    FX_LPCBYTE jpeg_src_buf;
    FX_DWORD jpeg_src_size;
    CFX_BinaryBuf temp_buf;
    if (m_InputBuf.GetSize()) {
        temp_buf.EstimateSize(m_InputBuf.GetSize() + src_size);
        temp_buf.AppendBlock(m_InputBuf.GetBuffer(), m_InputBuf.GetSize());
        m_InputBuf.Clear();
        temp_buf.AppendBlock(src_buf, src_size);
        jpeg_src_buf = temp_buf.GetBuffer();
        jpeg_src_size = temp_buf.GetSize();
    } else {
        jpeg_src_buf = src_buf;
        jpeg_src_size = src_size;
    }
    CPDF_ModuleMgr::Get()->GetJpegModule()->Input(m_pContext, jpeg_src_buf, jpeg_src_size);
    if (!m_bGotHeader) {
        int ret = CPDF_ModuleMgr::Get()->GetJpegModule()->ReadHeader(m_pContext, &m_Width, &m_Height, &m_nComps);
        int left_size = CPDF_ModuleMgr::Get()->GetJpegModule()->GetAvailInput(m_pContext);
        if (ret == 1) {
            ReportEOF(left_size);
            return;
        }
        if (ret == 2) {
            m_InputBuf.AppendBlock(jpeg_src_buf + jpeg_src_size - left_size, left_size);
            return;
        }
        CPDF_ModuleMgr::Get()->GetJpegModule()->StartScanline(m_pContext, 1);
        m_bGotHeader = TRUE;
        m_Pitch = m_Width * m_nComps;
    }
    if (m_pScanline == NULL) {
        m_pScanline = FX_Alloc(FX_BYTE, m_Pitch + 4);
    }
    while (1) {
        if (!CPDF_ModuleMgr::Get()->GetJpegModule()->ReadScanline(m_pContext, m_pScanline)) {
            int left_size = CPDF_ModuleMgr::Get()->GetJpegModule()->GetAvailInput(m_pContext);
            m_InputBuf.AppendBlock(jpeg_src_buf + jpeg_src_size - left_size, left_size);
            break;
        }
        dest_buf.AppendBlock(m_pScanline, m_Pitch);
        m_iLine ++;
        if (m_iLine == m_Height) {
            ReportEOF(CPDF_ModuleMgr::Get()->GetJpegModule()->GetAvailInput(m_pContext));
            return;
        }
    }
}
CPDF_FaxFilter::CPDF_FaxFilter()
{
    m_Encoding = 0;
    m_bEndOfLine = FALSE;
    m_bByteAlign = FALSE;
    m_bBlack = FALSE;
    m_nRows = 0;
    m_nColumns = 0;
    m_Pitch = 0;
    m_pScanlineBuf = NULL;
    m_pRefBuf = NULL;
    m_iRow = 0;
    m_InputBitPos = 0;
}
CPDF_FaxFilter::~CPDF_FaxFilter()
{
    if (m_pScanlineBuf) {
        FX_Free(m_pScanlineBuf);
    }
    if (m_pRefBuf) {
        FX_Free(m_pRefBuf);
    }
}
FX_BOOL CPDF_FaxFilter::Initialize(int Encoding, int bEndOfLine, int bByteAlign, int bBlack, int nRows, int nColumns)
{
    m_Encoding = Encoding;
    m_bEndOfLine = bEndOfLine;
    m_bByteAlign = bByteAlign;
    m_bBlack = bBlack;
    m_nRows = nRows;
    m_nColumns = nColumns;
    m_Pitch = (m_nColumns + 7) / 8;
    m_pScanlineBuf = FX_Alloc(FX_BYTE, m_Pitch);
    m_pRefBuf = FX_Alloc(FX_BYTE, m_Pitch);
    FXSYS_memset8(m_pScanlineBuf, 0xff, m_Pitch);
    FXSYS_memset8(m_pRefBuf, 0xff, m_Pitch);
    m_iRow = 0;
    m_InputBitPos = 0;
    return TRUE;
}
void CPDF_FaxFilter::v_FilterIn(FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf)
{
    FX_LPCBYTE fax_src_buf;
    FX_DWORD fax_src_size;
    CFX_BinaryBuf temp_buf;
    int bitpos;
    if (m_InputBuf.GetSize()) {
        temp_buf.EstimateSize(m_InputBuf.GetSize() + src_size);
        temp_buf.AppendBlock(m_InputBuf.GetBuffer(), m_InputBuf.GetSize());
        m_InputBuf.Clear();
        temp_buf.AppendBlock(src_buf, src_size);
        fax_src_buf = temp_buf.GetBuffer();
        fax_src_size = temp_buf.GetSize();
        bitpos = m_InputBitPos;
    } else {
        fax_src_buf = src_buf;
        fax_src_size = src_size;
        bitpos = 0;
    }
    ProcessData(fax_src_buf, fax_src_size, bitpos, FALSE, dest_buf);
    int left_bits = fax_src_size * 8 - bitpos;
    m_InputBuf.AppendBlock(fax_src_buf + bitpos / 8, (left_bits + 7) / 8);
    m_InputBitPos = bitpos % 8;
}
void CPDF_FaxFilter::v_FilterFinish(CFX_BinaryBuf& dest_buf)
{
    ProcessData(m_InputBuf.GetBuffer(), m_InputBuf.GetSize(), m_InputBitPos, TRUE, dest_buf);
}
FX_BOOL _FaxSkipEOL(const FX_BYTE* src_buf, int bitsize, int& bitpos);
FX_BOOL _FaxG4GetRow(const FX_BYTE* src_buf, int bitsize, int& bitpos, FX_LPBYTE dest_buf, const FX_BYTE* ref_buf, int columns);
FX_BOOL _FaxGet1DLine(const FX_BYTE* src_buf, int bitsize, int& bitpos, FX_LPBYTE dest_buf, int columns);
void CPDF_FaxFilter::ProcessData(FX_LPCBYTE src_buf, FX_DWORD src_size, int& bitpos, FX_BOOL bFinish,
                                 CFX_BinaryBuf& dest_buf)
{
    int bitsize = src_size * 8;
    while (1) {
        if ((bitsize < bitpos + 256) && !bFinish) {
            return;
        }
        int start_bitpos = bitpos;
        FXSYS_memset8(m_pScanlineBuf, 0xff, m_Pitch);
        if (!ReadLine(src_buf, bitsize, bitpos)) {
            bitpos = start_bitpos;
            return;
        }
        if (m_Encoding) {
            FXSYS_memcpy32(m_pRefBuf, m_pScanlineBuf, m_Pitch);
        }
        if (m_bBlack) {
            for (int i = 0; i < m_Pitch; i ++) {
                m_pScanlineBuf[i] = ~m_pScanlineBuf[i];
            }
        }
        dest_buf.AppendBlock(m_pScanlineBuf, m_Pitch);
        m_iRow ++;
        if (m_iRow == m_nRows) {
            ReportEOF(src_size - (bitpos + 7) / 8);
            return;
        }
    }
}
FX_BOOL CPDF_FaxFilter::ReadLine(FX_LPCBYTE src_buf, int bitsize, int& bitpos)
{
    if (!_FaxSkipEOL(src_buf, bitsize, bitpos)) {
        return FALSE;
    }
    FX_BOOL ret;
    if (m_Encoding < 0) {
        ret = _FaxG4GetRow(src_buf, bitsize, bitpos, m_pScanlineBuf, m_pRefBuf, m_nColumns);
    } else if (m_Encoding == 0) {
        ret = _FaxGet1DLine(src_buf, bitsize, bitpos, m_pScanlineBuf, m_nColumns);
    } else {
        if (bitpos == bitsize) {
            return FALSE;
        }
        FX_BOOL bNext1D = src_buf[bitpos / 8] & (1 << (7 - bitpos % 8));
        bitpos ++;
        if (bNext1D) {
            ret = _FaxGet1DLine(src_buf, bitsize, bitpos, m_pScanlineBuf, m_nColumns);
        } else {
            ret = _FaxG4GetRow(src_buf, bitsize, bitpos, m_pScanlineBuf, m_pRefBuf, m_nColumns);
        }
    }
    if (!ret) {
        return FALSE;
    }
    if (m_bEndOfLine)
        if (!_FaxSkipEOL(src_buf, bitsize, bitpos)) {
            return FALSE;
        }
    if (m_bByteAlign) {
        bitpos = (bitpos + 7) / 8 * 8;
    }
    return TRUE;
}
