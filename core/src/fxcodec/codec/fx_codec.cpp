// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxcodec/fx_codec.h"
#include "codec_int.h"
CCodec_ModuleMgr::CCodec_ModuleMgr()
{
    m_pBasicModule = FX_NEW CCodec_BasicModule;
    m_pFaxModule = FX_NEW CCodec_FaxModule;
    m_pJpegModule = FX_NEW CCodec_JpegModule;
    m_pJpxModule = FX_NEW CCodec_JpxModule;
    m_pJbig2Module = FX_NEW CCodec_Jbig2Module;
    m_pIccModule = FX_NEW CCodec_IccModule;
    m_pPngModule = FX_NEW CCodec_PngModule;
    m_pGifModule = FX_NEW CCodec_GifModule;
    m_pBmpModule = FX_NEW CCodec_BmpModule;
    m_pTiffModule = FX_NEW CCodec_TiffModule;
    m_pFlateModule = FX_NEW CCodec_FlateModule;
}
CCodec_ModuleMgr::~CCodec_ModuleMgr()
{
    delete m_pBasicModule;
    delete m_pFaxModule;
    delete m_pJpegModule;
    delete m_pFlateModule;
    delete m_pJpxModule;
    delete m_pJbig2Module;
    delete m_pIccModule;
}
void CCodec_ModuleMgr::InitJbig2Decoder()
{
}
void CCodec_ModuleMgr::InitJpxDecoder()
{
}
void CCodec_ModuleMgr::InitIccDecoder()
{
}
CCodec_ScanlineDecoder::CCodec_ScanlineDecoder()
{
    m_NextLine = -1;
    m_pDataCache = NULL;
    m_pLastScanline = NULL;
}
CCodec_ScanlineDecoder::~CCodec_ScanlineDecoder()
{
    if (m_pDataCache) {
        FX_Free(m_pDataCache);
    }
}
FX_LPBYTE CCodec_ScanlineDecoder::GetScanline(int line)
{
    if (m_pDataCache && line < m_pDataCache->m_nCachedLines) {
        return &m_pDataCache->m_Data + line * m_Pitch;
    }
    if (m_NextLine == line + 1) {
        return m_pLastScanline;
    }
    if (m_NextLine < 0 || m_NextLine > line) {
        if (!v_Rewind()) {
            return NULL;
        }
        m_NextLine = 0;
    }
    while (m_NextLine < line) {
        ReadNextLine();
        m_NextLine ++;
    }
    m_pLastScanline = ReadNextLine();
    m_NextLine ++;
    return m_pLastScanline;
}
FX_BOOL CCodec_ScanlineDecoder::SkipToScanline(int line, IFX_Pause* pPause)
{
    if (m_pDataCache && line < m_pDataCache->m_nCachedLines) {
        return FALSE;
    }
    if (m_NextLine == line || m_NextLine == line + 1) {
        return FALSE;
    }
    if (m_NextLine < 0 || m_NextLine > line) {
        v_Rewind();
        m_NextLine = 0;
    }
    m_pLastScanline = NULL;
    while (m_NextLine < line) {
        m_pLastScanline = ReadNextLine();
        m_NextLine ++;
        if (pPause && pPause->NeedToPauseNow()) {
            return TRUE;
        }
    }
    return FALSE;
}
FX_LPBYTE CCodec_ScanlineDecoder::ReadNextLine()
{
    FX_LPBYTE pLine = v_GetNextLine();
    if (pLine == NULL) {
        return NULL;
    }
    if (m_pDataCache && m_NextLine == m_pDataCache->m_nCachedLines) {
        FXSYS_memcpy32(&m_pDataCache->m_Data + m_NextLine * m_Pitch, pLine, m_Pitch);
        m_pDataCache->m_nCachedLines ++;
    }
    return pLine;
}
void CCodec_ScanlineDecoder::DownScale(int dest_width, int dest_height)
{
    if (dest_width < 0) {
        dest_width = -dest_width;
    }
    if (dest_height < 0) {
        dest_height = -dest_height;
    }
    v_DownScale(dest_width, dest_height);
    if (m_pDataCache) {
        if (m_pDataCache->m_Height == m_OutputHeight && m_pDataCache->m_Width == m_OutputWidth) {
            return;
        }
        FX_Free(m_pDataCache);
        m_pDataCache = NULL;
    }
    m_pDataCache = (CCodec_ImageDataCache*)FX_AllocNL(FX_BYTE, sizeof(CCodec_ImageDataCache) + m_Pitch * m_OutputHeight);
    if (m_pDataCache == NULL) {
        return;
    }
    m_pDataCache->m_Height = m_OutputHeight;
    m_pDataCache->m_Width = m_OutputWidth;
    m_pDataCache->m_nCachedLines = 0;
}
FX_BOOL CCodec_BasicModule::RunLengthEncode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf,
        FX_DWORD& dest_size)
{
    return FALSE;
}
extern "C" double FXstrtod(const char* nptr, char** endptr)
{
    double ret = 0.0;
    const char* ptr = nptr;
    const char* exp_ptr = NULL;
    int	e_number = 0,
        e_signal = 0,
        e_point = 0,
        is_negative = 0;
    int exp_ret = 0, exp_sig = 1,
        fra_ret = 0, fra_count = 0, fra_base = 1;
    if(nptr == NULL) {
        return 0.0;
    }
    for (;; ptr++) {
        if(!e_number && !e_point && (*ptr == '\t' || *ptr == ' ')) {
            continue;
        }
        if(*ptr >= '0' && *ptr <= '9') {
            if(!e_number) {
                e_number = 1;
            }
            if(!e_point) {
                ret *= 10;
                ret += (*ptr - '0');
            } else {
                fra_count++;
                fra_ret *= 10;
                fra_ret += (*ptr - '0');
            }
            continue;
        }
        if(!e_point && *ptr == '.') {
            e_point = 1;
            continue;
        }
        if(!e_number && !e_point && !e_signal) {
            switch(*ptr) {
                case '-':
                    is_negative = 1;
                case '+':
                    e_signal = 1;
                    continue;
            }
        }
        if(e_number && (*ptr == 'e' || *ptr == 'E')) {
#define EXPONENT_DETECT(ptr)	\
    for(;;ptr++){		\
        if(*ptr < '0' || *ptr > '9'){	\
            if(endptr)	*endptr = (char*)ptr;	\
            break;	\
        }else{		\
            exp_ret *= 10;	\
            exp_ret += (*ptr - '0');	\
            continue;		\
        }	\
    }
            exp_ptr = ptr++;
            if(*ptr == '+' || *ptr == '-') {
                exp_sig = (*ptr++ == '+') ? 1 : -1;
                if(*ptr < '0' || *ptr > '9') {
                    if(endptr)	{
                        *endptr = (char*)exp_ptr;
                    }
                    break;
                }
                EXPONENT_DETECT(ptr);
            } else if(*ptr >= '0' && *ptr <= '9') {
                EXPONENT_DETECT(ptr);
            } else {
                if(endptr)	{
                    *endptr = (char*)exp_ptr;
                }
                break;
            }
#undef EXPONENT_DETECT
            break;
        }
        if(ptr != nptr && !e_number) {
            if(endptr)	{
                *endptr = (char*)nptr;
            }
            break;
        }
        if(endptr)	{
            *endptr = (char*)ptr;
        }
        break;
    }
    while(fra_count--) {
        fra_base *= 10;
    }
    ret += (double)fra_ret / (double)fra_base;
    if(exp_sig == 1) {
        while(exp_ret--) {
            ret *= 10.0;
        }
    } else {
        while(exp_ret--) {
            ret /= 10.0;
        }
    }
    return is_negative ? -ret : ret;
}
FX_BOOL CCodec_BasicModule::A85Encode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf,
                                      FX_DWORD& dest_size)
{
    return FALSE;
}
CCodec_ModuleMgr* CCodec_ModuleMgr::Create()
{
    return FX_NEW CCodec_ModuleMgr;
}
void CCodec_ModuleMgr::Destroy()
{
    delete this;
}
CFX_DIBAttribute::CFX_DIBAttribute()
{
    FXSYS_memset32(this, 0, sizeof(CFX_DIBAttribute));
    m_nXDPI = -1;
    m_nYDPI = -1;
    m_fAspectRatio = -1.0f;
    m_pExif = FX_NEW CFX_DIBAttributeExif;
}
CFX_DIBAttribute::~CFX_DIBAttribute()
{
    if (m_pExif) {
        delete m_pExif;
    }
}
CFX_DIBAttributeExif::CFX_DIBAttributeExif()
{
    m_pExifData = NULL;
    m_dwExifDataLen = 0;
}
CFX_DIBAttributeExif::~CFX_DIBAttributeExif()
{
    clear();
}
void CFX_DIBAttributeExif::clear()
{
    if (m_pExifData) {
        FX_Free(m_pExifData);
    }
    m_pExifData = NULL;
    FX_DWORD key = 0;
    FX_LPBYTE buf = NULL;
    FX_POSITION pos = NULL;
    pos = m_TagHead.GetStartPosition();
    while (pos) {
        m_TagHead.GetNextAssoc(pos, key, buf);
        if (buf) {
            FX_Free(buf);
        }
    }
    m_TagHead.RemoveAll();
    pos = m_TagVal.GetStartPosition();
    while (pos) {
        m_TagVal.GetNextAssoc(pos, key, buf);
        if (buf) {
            FX_Free(buf);
        }
    }
    m_TagVal.RemoveAll();
}
static FX_WORD _Read2BytesL(FX_LPBYTE data)
{
    ASSERT(data);
    return data[0] | (data[1] << 8);
}
static FX_WORD _Read2BytesB(FX_LPBYTE data)
{
    ASSERT(data);
    return data[1] | (data[0] << 8);
}
static FX_DWORD _Read4BytesL(FX_LPBYTE data)
{
    return _Read2BytesL(data) | (_Read2BytesL(data + 2) << 16);
}
static FX_DWORD _Read4BytesB(FX_LPBYTE data)
{
    return _Read2BytesB(data + 2) | (_Read2BytesB(data) << 16);
}
typedef FX_WORD (*_Read2Bytes) (FX_LPBYTE data);
typedef FX_DWORD (*_Read4Bytes) (FX_LPBYTE data);
typedef void (*_Write2Bytes) (FX_LPBYTE data, FX_WORD val);
typedef void (*_Write4Bytes) (FX_LPBYTE data, FX_DWORD val);
FX_LPBYTE CFX_DIBAttributeExif::ParseExifIFH(FX_LPBYTE data, FX_DWORD len, _Read2Bytes* pReadWord, _Read4Bytes* pReadDword)
{
    if (len > 8) {
        FX_BOOL tag = FALSE;
        if (FXSYS_memcmp32(data, "\x49\x49\x2a\x00", 4) == 0) {
            if (pReadWord) {
                *pReadWord = _Read2BytesL;
            }
            if (pReadDword) {
                *pReadDword = _Read4BytesL;
            }
            tag = TRUE;
        } else if (FXSYS_memcmp32(data, "\x4d\x4d\x00\x2a", 4) == 0) {
            if (pReadWord) {
                *pReadWord = _Read2BytesB;
            }
            if (pReadDword) {
                *pReadDword = _Read4BytesB;
            }
            tag = TRUE;
        }
        if (tag) {
            data += 4;
            if (pReadDword) {
                data += (*pReadDword)(data) - 4;
            } else {
                data += 4;
            }
        }
    }
    return data;
}
FX_BOOL CFX_DIBAttributeExif::ParseExifIFD(CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE>* pMap, FX_LPBYTE data, FX_DWORD len)
{
    if (pMap && data) {
        if (len > 8) {
            FX_WORD wTagNum = m_readWord(data);
            data += 2;
            FX_DWORD wTag;
            FX_LPBYTE buf;
            while (wTagNum--) {
                wTag = m_readWord(data);
                data += 2;
                if (!pMap->Lookup(wTag, buf)) {
                    buf = FX_Alloc(FX_BYTE, 10);
                    if (buf == NULL) {
                        return FALSE;
                    }
                    FXSYS_memcpy32(buf, data, 10);
                    pMap->SetAt(wTag, buf);
                }
                data += 10;
            }
            FX_DWORD dwIFDOffset;
            dwIFDOffset = m_readDword(data);
            while (dwIFDOffset && dwIFDOffset < len) {
                data = m_pExifData + dwIFDOffset;
                wTagNum = m_readWord(data);
                data += 2;
                while (wTagNum--) {
                    wTag = m_readWord(data);
                    data += 2;
                    if (!pMap->Lookup(wTag, buf)) {
                        buf = FX_Alloc(FX_BYTE, 10);
                        if (buf == NULL) {
                            return FALSE;
                        }
                        FXSYS_memcpy32(buf, data, 10);
                        pMap->SetAt(wTag, buf);
                    }
                    data += 10;
                }
                dwIFDOffset = m_readDword(data);
            }
            return TRUE;
        }
    }
    return FALSE;
}
enum FX_ExifDataType {
    FX_UnsignedByte = 1,
    FX_AscString,
    FX_UnsignedShort,
    FX_UnsignedLong,
    FX_UnsignedRation,
    FX_SignedByte,
    FX_Undefined,
    FX_SignedShort,
    FX_SignedLong,
    FX_SignedRation,
    FX_SignedFloat,
    FX_DoubleFloat
};
FX_BOOL CFX_DIBAttributeExif::ParseExif(CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE>* pHead, FX_LPBYTE data, FX_DWORD len, CFX_MapPtrTemplate<FX_DWORD, FX_LPBYTE>* pVal)
{
    if (pHead && data && pVal) {
        if (len > 8) {
            FX_LPBYTE old_data = data;
            data = ParseExifIFH(data, len, &m_readWord, &m_readDword);
            if (data == old_data) {
                return FALSE;
            }
            if (pHead->GetCount() == 0) {
                if (!ParseExifIFD(pHead, data, len)) {
                    return FALSE;
                }
            }
            FX_DWORD dwModuleNum;
            FX_WORD type;
            FX_DWORD dwSize;
            FX_DWORD tag;
            FX_LPBYTE head;
            FX_POSITION pos = pHead->GetStartPosition();
            while (pos) {
                pHead->GetNextAssoc(pos, tag, head);
                FX_LPBYTE val = NULL, buf = NULL, temp = NULL;
                int i;
                if (head) {
                    type = m_readWord(head);
                    head += 2;
                    dwModuleNum = m_readDword(head);
                    head += 4;
                    switch (type) {
                        case FX_UnsignedByte:
                        case FX_AscString:
                        case FX_SignedByte:
                        case FX_Undefined:
                            dwSize = dwModuleNum;
                            val = FX_Alloc(FX_BYTE, dwSize);
                            if (val == NULL) {
                                return FALSE;
                            }
                            if (dwSize > 4) {
                                FXSYS_memcpy32(val, old_data + m_readDword(head), dwSize);
                            } else {
                                FXSYS_memcpy32(val, head, dwSize);
                            }
                            break;
                        case FX_UnsignedShort:
                        case FX_SignedShort:
                            dwSize = dwModuleNum << 1;
                            val = FX_Alloc(FX_BYTE, dwSize);
                            if (val == NULL) {
                                return FALSE;
                            }
                            if (dwSize > 4) {
                                FXSYS_memcpy32(val, old_data + m_readDword(head), dwSize);
                            } else {
                                FXSYS_memcpy32(val, head, dwSize);
                            }
                            buf = val;
                            for(i = 0; i < (int)dwModuleNum; i ++) {
                                *(FX_WORD*)buf = m_readWord(buf);
                                buf += 2;
                            }
                            break;
                        case FX_UnsignedLong:
                        case FX_SignedLong:
                        case FX_SignedFloat:
                            dwSize = dwModuleNum << 2;
                            val = FX_Alloc(FX_BYTE, dwSize);
                            if (val == NULL) {
                                return FALSE;
                            }
                            if (dwSize > 4) {
                                FXSYS_memcpy32(val, old_data + m_readDword(head), dwSize);
                            } else {
                                FXSYS_memcpy32(val, head, dwSize);
                            }
                            buf = val;
                            for(i = 0; i < (int)dwModuleNum; i ++) {
                                *(FX_DWORD*)buf = m_readDword(buf);
                                buf += 4;
                            }
                            break;
                        case FX_UnsignedRation:
                        case FX_SignedRation: {
                                dwSize = dwModuleNum << 3;
                                buf = FX_Alloc(FX_BYTE, dwSize);
                                if (buf == NULL) {
                                    return FALSE;
                                }
                                if (dwSize > 4) {
                                    FXSYS_memcpy32(buf, old_data + m_readDword(head), dwSize);
                                } else {
                                    FXSYS_memcpy32(buf, head, dwSize);
                                }
                                temp = buf;
                                val = FX_Alloc(FX_BYTE, dwSize / 2);
                                if (val == NULL) {
                                    FX_Free(buf);
                                    return FALSE;
                                }
                                for(i = 0; i < (int)dwModuleNum; i ++) {
                                    *(FX_DWORD*)temp = m_readDword(temp);
                                    *(FX_DWORD*)(temp + 4) = m_readDword(temp + 4);
                                    FX_DWORD* lNumerator = (FX_DWORD*)temp;
                                    FX_DWORD* lNenominator = (FX_DWORD*)(temp + 4);
                                    *(FX_FLOAT*)(val + i * 4) = (FX_FLOAT)(*lNumerator) / (FX_FLOAT)(*lNenominator);
                                    temp += 8;
                                }
                                FX_Free(buf);
                            }
                            break;
                        case FX_DoubleFloat:
                            dwSize = dwModuleNum << 3;
                            val = FX_Alloc(FX_BYTE, dwSize);
                            if (val == NULL) {
                                return FALSE;
                            }
                            if (dwSize > 4) {
                                FXSYS_memcpy32(val, old_data + m_readDword(head), dwSize);
                            } else {
                                FXSYS_memcpy32(val, head, dwSize);
                            }
                            buf = val;
                            for(i = 0; i < (int)dwModuleNum; i ++) {
                                *(FX_DWORD*)buf = m_readDword(buf);
                                *(FX_DWORD*)(buf + 4) = m_readDword(buf + 4);
                                buf += 8;
                            }
                            break;
                        default:
                            return FALSE;
                    }
                }
                pVal->SetAt(tag, val);
            }
            return TRUE;
        }
    }
    return FALSE;
}
#define FXEXIF_INFOCONVERT(T) {T* src = (T*)ptr;	T* dst = (T*)val;	*dst = *src;}
FX_BOOL CFX_DIBAttributeExif::GetInfo( FX_WORD tag, FX_LPVOID val )
{
    if (m_TagVal.GetCount() == 0) {
        if (!ParseExif(&m_TagHead, m_pExifData, m_dwExifDataLen, &m_TagVal)) {
            return FALSE;
        }
    }
    FX_LPBYTE ptr = NULL;
    if (m_TagVal.Lookup(tag, ptr)) {
        switch (tag) {
            case EXIFTAG_USHORT_RESUNIT:
                FXEXIF_INFOCONVERT(FX_WORD);
                {
                    FX_WORD* ptr = (FX_WORD*)val;
                    *ptr -= 1;
                }
                break;
            case EXIFTAG_FLOAT_DPIX:
            case EXIFTAG_FLOAT_DPIY:
                FXEXIF_INFOCONVERT(FX_FLOAT);
                break;
            case EXIFTAG_USHORT_ORIENTATION:
                FXEXIF_INFOCONVERT(FX_WORD);
                break;
            default: {
                    FX_LPBYTE* dst = (FX_LPBYTE*)val;
                    *dst = ptr;
                }
        }
    }
    return TRUE;
}
class CCodec_RLScanlineDecoder : public CCodec_ScanlineDecoder
{
public:
    CCodec_RLScanlineDecoder();
    virtual ~CCodec_RLScanlineDecoder();
    FX_BOOL				Create(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height, int nComps, int bpc);
    virtual void		v_DownScale(int dest_width, int dest_height) {}
    virtual FX_BOOL		v_Rewind();
    virtual FX_LPBYTE	v_GetNextLine();
    virtual FX_DWORD	GetSrcOffset()
    {
        return m_SrcOffset;
    }
protected:
    FX_BOOL				CheckDestSize();
    void				GetNextOperator();
    void				UpdateOperator(FX_BYTE used_bytes);

    FX_LPBYTE			m_pScanline;
    FX_LPCBYTE			m_pSrcBuf;
    FX_DWORD			m_SrcSize;
    FX_DWORD			m_dwLineBytes;
    FX_DWORD			m_SrcOffset;
    FX_BOOL				m_bEOD;
    FX_BYTE				m_Operator;
};
CCodec_RLScanlineDecoder::CCodec_RLScanlineDecoder()
    : m_pScanline(NULL)
    , m_pSrcBuf(NULL)
    , m_SrcSize(0)
    , m_dwLineBytes(0)
    , m_SrcOffset(0)
    , m_bEOD(FALSE)
    , m_Operator(0)
{
}
CCodec_RLScanlineDecoder::~CCodec_RLScanlineDecoder()
{
    if (m_pScanline) {
        FX_Free(m_pScanline);
    }
}
FX_BOOL CCodec_RLScanlineDecoder::CheckDestSize()
{
    FX_DWORD i = 0;
    FX_DWORD old_size = 0;
    FX_DWORD dest_size = 0;
    while (i < m_SrcSize) {
        if (m_pSrcBuf[i] < 128) {
            old_size = dest_size;
            dest_size += m_pSrcBuf[i] + 1;
            if (dest_size < old_size) {
                return FALSE;
            }
            i += m_pSrcBuf[i] + 2;
        } else if (m_pSrcBuf[i] > 128) {
            old_size = dest_size;
            dest_size += 257 - m_pSrcBuf[i];
            if (dest_size < old_size) {
                return FALSE;
            }
            i += 2;
        } else {
            break;
        }
    }
    if (((FX_DWORD)m_OrigWidth * m_nComps * m_bpc * m_OrigHeight + 7) / 8 > dest_size) {
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CCodec_RLScanlineDecoder::Create(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height, int nComps, int bpc)
{
    m_pSrcBuf = src_buf;
    m_SrcSize = src_size;
    m_OutputWidth = m_OrigWidth = width;
    m_OutputHeight = m_OrigHeight = height;
    m_nComps = nComps;
    m_bpc = bpc;
    m_bColorTransformed = FALSE;
    m_DownScale = 1;
    m_Pitch = (width * nComps * bpc + 31) / 32 * 4;
    m_dwLineBytes = (width * nComps * bpc + 7) / 8;
    m_pScanline = FX_Alloc(FX_BYTE, m_Pitch);
    if (m_pScanline == NULL) {
        return FALSE;
    }
    return CheckDestSize();
}
FX_BOOL CCodec_RLScanlineDecoder::v_Rewind()
{
    FXSYS_memset32(m_pScanline, 0, m_Pitch);
    m_SrcOffset = 0;
    m_bEOD = FALSE;
    m_Operator = 0;
    return TRUE;
}
FX_LPBYTE CCodec_RLScanlineDecoder::v_GetNextLine()
{
    if (m_SrcOffset == 0) {
        GetNextOperator();
    } else {
        if (m_bEOD) {
            return NULL;
        }
    }
    FXSYS_memset32(m_pScanline, 0, m_Pitch);
    FX_DWORD col_pos = 0;
    FX_BOOL	eol = FALSE;
    while (m_SrcOffset < m_SrcSize && !eol) {
        if (m_Operator < 128) {
            FX_DWORD copy_len = m_Operator + 1;
            if (col_pos + copy_len >= m_dwLineBytes) {
                copy_len = m_dwLineBytes - col_pos;
                eol = TRUE;
            }
            if (copy_len >= m_SrcSize - m_SrcOffset) {
                copy_len = m_SrcSize - m_SrcOffset;
                m_bEOD = TRUE;
            }
            FXSYS_memcpy32(m_pScanline + col_pos, m_pSrcBuf + m_SrcOffset, copy_len);
            col_pos += copy_len;
            UpdateOperator((FX_BYTE)copy_len);
        } else if (m_Operator > 128) {
            int fill = 0;
            if (m_SrcOffset - 1 < m_SrcSize - 1) {
                fill = m_pSrcBuf[m_SrcOffset];
            }
            FX_DWORD duplicate_len = 257 - m_Operator;
            if (col_pos + duplicate_len >= m_dwLineBytes) {
                duplicate_len = m_dwLineBytes - col_pos;
                eol = TRUE;
            }
            FXSYS_memset8(m_pScanline + col_pos, fill, duplicate_len);
            col_pos += duplicate_len;
            UpdateOperator((FX_BYTE)duplicate_len);
        } else {
            m_bEOD = TRUE;
            break;
        }
    }
    return m_pScanline;
}
void CCodec_RLScanlineDecoder::GetNextOperator()
{
    if (m_SrcOffset >= m_SrcSize) {
        m_Operator = 128;
        return;
    }
    m_Operator = m_pSrcBuf[m_SrcOffset];
    m_SrcOffset ++;
}
void CCodec_RLScanlineDecoder::UpdateOperator(FX_BYTE used_bytes)
{
    if (used_bytes == 0) {
        return;
    }
    if (m_Operator < 128) {
        FXSYS_assert((FX_DWORD)m_Operator + 1 >= used_bytes);
        if (used_bytes == m_Operator + 1) {
            m_SrcOffset += used_bytes;
            GetNextOperator();
            return;
        }
        m_Operator -= used_bytes;
        m_SrcOffset += used_bytes;
        if (m_SrcOffset >= m_SrcSize) {
            m_Operator = 128;
        }
        return;
    }
    FX_BYTE count = 257 - m_Operator;
    FXSYS_assert((FX_DWORD)count >= used_bytes);
    if (used_bytes == count) {
        m_SrcOffset ++;
        GetNextOperator();
        return;
    }
    count -= used_bytes;
    m_Operator = 257 - count;
}
ICodec_ScanlineDecoder* CCodec_BasicModule::CreateRunLengthDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        int nComps, int bpc)
{
    CCodec_RLScanlineDecoder* pRLScanlineDecoder = FX_NEW CCodec_RLScanlineDecoder;
    if (pRLScanlineDecoder == NULL) {
        return NULL;
    }
    if (!pRLScanlineDecoder->Create(src_buf, src_size, width, height, nComps, bpc)) {
        delete pRLScanlineDecoder;
        return NULL;
    }
    return pRLScanlineDecoder;
}
