// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_basic.h"
#include "../../../third_party/base/numerics/safe_math.h"

static int _Buffer_itoa(char* buf, int i, FX_DWORD flags)
{
    if (i == 0) {
        buf[0] = '0';
        return 1;
    }
    char buf1[32];
    int buf_pos = 31;
    FX_DWORD u = i;
    if ((flags & FXFORMAT_SIGNED) && i < 0) {
        u = -i;
    }
    int base = 10;
    FX_LPCSTR string = "0123456789abcdef";
    if (flags & FXFORMAT_HEX) {
        base = 16;
        if (flags & FXFORMAT_CAPITAL) {
            string = "0123456789ABCDEF";
        }
    }
    while (u != 0) {
        buf1[buf_pos--] = string[u % base];
        u = u / base;
    }
    if ((flags & FXFORMAT_SIGNED) && i < 0) {
        buf1[buf_pos--] = '-';
    }
    int len = 31 - buf_pos;
    for (int ii = 0; ii < len; ii ++) {
        buf[ii] = buf1[ii + buf_pos + 1];
    }
    return len;
}
CFX_ByteString CFX_ByteString::FormatInteger(int i, FX_DWORD flags)
{
    char buf[32];
    return CFX_ByteStringC(buf, _Buffer_itoa(buf, i, flags));
}
static CFX_StringData* FX_AllocString(int nLen)
{
    // |nLen| is currently declared as in |int|. TODO(palmer): It should be
    // a |size_t|, or at least unsigned.
    if (nLen == 0 || nLen < 0) {
        return NULL;
    }
    pdfium::base::CheckedNumeric<int> nSize = nLen;
    nSize += sizeof(long) * 3 + 1;
    CFX_StringData* pData = (CFX_StringData*)FX_Alloc(FX_BYTE, nSize.ValueOrDie());
    if (!pData) {
        return NULL;
    }
    pData->m_nAllocLength = nLen;
    pData->m_nDataLength = nLen;
    pData->m_nRefs = 1;
    pData->m_String[nLen] = 0;
    return pData;
}
static void FX_ReleaseString(CFX_StringData* pData)
{
    if (pData == NULL) {
        return;
    }
    pData->m_nRefs --;
    if (pData->m_nRefs <= 0) {
        FX_Free(pData);
    }
}
CFX_ByteString::~CFX_ByteString()
{
    if (m_pData == NULL) {
        return;
    }
    m_pData->m_nRefs --;
    if (m_pData->m_nRefs < 1) {
        FX_Free(m_pData);
    }
}
CFX_ByteString::CFX_ByteString(FX_LPCSTR lpsz, FX_STRSIZE nLen)
{
    if (nLen < 0) {
        nLen = lpsz ? (FX_STRSIZE)FXSYS_strlen(lpsz) : 0;
    }
    if (nLen) {
        m_pData = FX_AllocString(nLen);
        if (m_pData) {
            FXSYS_memcpy32(m_pData->m_String, lpsz, nLen);
        }
    } else {
        m_pData = NULL;
    }
}
CFX_ByteString::CFX_ByteString(FX_LPCBYTE lpsz, FX_STRSIZE nLen)
{
    if (nLen > 0) {
        m_pData = FX_AllocString(nLen);
        if (m_pData) {
            FXSYS_memcpy32(m_pData->m_String, lpsz, nLen);
        }
    } else {
        m_pData = NULL;
    }
}
CFX_ByteString::CFX_ByteString(char ch)
{
    m_pData = FX_AllocString(1);
    if (m_pData) {
        m_pData->m_String[0] = ch;
    }
}
CFX_ByteString::CFX_ByteString(const CFX_ByteString& stringSrc)
{
    if (stringSrc.m_pData == NULL) {
        m_pData = NULL;
        return;
    }
    if (stringSrc.m_pData->m_nRefs >= 0) {
        m_pData = stringSrc.m_pData;
        m_pData->m_nRefs ++;
    } else {
        m_pData = NULL;
        *this = stringSrc;
    }
}
CFX_ByteString::CFX_ByteString(FX_BSTR stringSrc)
{
    if (stringSrc.IsEmpty()) {
        m_pData = NULL;
        return;
    } else {
        m_pData = NULL;
        *this = stringSrc;
    }
}
CFX_ByteString::CFX_ByteString(FX_BSTR str1, FX_BSTR str2)
{
    m_pData = NULL;
    int nNewLen = str1.GetLength() + str2.GetLength();
    if (nNewLen == 0) {
        return;
    }
    m_pData = FX_AllocString(nNewLen);
    if (m_pData) {
        FXSYS_memcpy32(m_pData->m_String, str1.GetCStr(), str1.GetLength());
        FXSYS_memcpy32(m_pData->m_String + str1.GetLength(), str2.GetCStr(), str2.GetLength());
    }
}
const CFX_ByteString& CFX_ByteString::operator=(FX_LPCSTR lpsz)
{
    if (lpsz == NULL || lpsz[0] == 0) {
        Empty();
    } else {
        AssignCopy((FX_STRSIZE)FXSYS_strlen(lpsz), lpsz);
    }
    return *this;
}
const CFX_ByteString& CFX_ByteString::operator=(FX_BSTR str)
{
    if (str.IsEmpty()) {
        Empty();
    } else {
        AssignCopy(str.GetLength(), str.GetCStr());
    }
    return *this;
}
const CFX_ByteString& CFX_ByteString::operator=(const CFX_ByteString& stringSrc)
{
    if (m_pData == stringSrc.m_pData) {
        return *this;
    }
    if (stringSrc.IsEmpty()) {
        Empty();
    } else if ((m_pData && m_pData->m_nRefs < 0) ||
               (stringSrc.m_pData && stringSrc.m_pData->m_nRefs < 0)) {
        AssignCopy(stringSrc.m_pData->m_nDataLength, stringSrc.m_pData->m_String);
    } else {
        Empty();
        m_pData = stringSrc.m_pData;
        if (m_pData) {
            m_pData->m_nRefs ++;
        }
    }
    return *this;
}
const CFX_ByteString& CFX_ByteString::operator=(const CFX_BinaryBuf& buf)
{
    Load(buf.GetBuffer(), buf.GetSize());
    return *this;
}
void CFX_ByteString::Load(FX_LPCBYTE buf, FX_STRSIZE len)
{
    Empty();
    if (len) {
        m_pData = FX_AllocString(len);
        if (m_pData) {
            FXSYS_memcpy32(m_pData->m_String, buf, len);
        }
    } else {
        m_pData = NULL;
    }
}
const CFX_ByteString& CFX_ByteString::operator+=(FX_LPCSTR lpsz)
{
    if (lpsz) {
        ConcatInPlace((FX_STRSIZE)FXSYS_strlen(lpsz), lpsz);
    }
    return *this;
}
const CFX_ByteString& CFX_ByteString::operator+=(char ch)
{
    ConcatInPlace(1, &ch);
    return *this;
}
const CFX_ByteString& CFX_ByteString::operator+=(const CFX_ByteString& string)
{
    if (string.m_pData == NULL) {
        return *this;
    }
    ConcatInPlace(string.m_pData->m_nDataLength, string.m_pData->m_String);
    return *this;
}
const CFX_ByteString& CFX_ByteString::operator+=(FX_BSTR string)
{
    if (string.IsEmpty()) {
        return *this;
    }
    ConcatInPlace(string.GetLength(), string.GetCStr());
    return *this;
}
bool CFX_ByteString::Equal(FX_BSTR str) const
{
    if (m_pData == NULL) {
        return str.IsEmpty();
    }
    return m_pData->m_nDataLength == str.GetLength() &&
           FXSYS_memcmp32(m_pData->m_String, str.GetCStr(), str.GetLength()) == 0;
}
bool CFX_ByteString::operator ==(const CFX_ByteString& s2) const
{
    if (m_pData == NULL) {
        return s2.IsEmpty();
    }
    if (s2.m_pData == NULL) {
        return false;
    }
    return m_pData->m_nDataLength == s2.m_pData->m_nDataLength &&
           FXSYS_memcmp32(m_pData->m_String, s2.m_pData->m_String, m_pData->m_nDataLength) == 0;
}
void CFX_ByteString::Empty()
{
    if (m_pData == NULL) {
        return;
    }
    if (m_pData->m_nRefs > 1) {
        m_pData->m_nRefs --;
    } else {
        FX_Free(m_pData);
    }
    m_pData = NULL;
}
bool CFX_ByteString::EqualNoCase(FX_BSTR str) const
{
    if (m_pData == NULL) {
        return str.IsEmpty();
    }
    FX_STRSIZE len = str.GetLength();
    if (m_pData->m_nDataLength != len) {
        return false;
    }
    FX_LPCBYTE pThis = (FX_LPCBYTE)m_pData->m_String;
    FX_LPCBYTE pThat = (FX_LPCBYTE)str;
    for (FX_STRSIZE i = 0; i < len; i ++) {
        if ((*pThis) != (*pThat)) {
            FX_BYTE bThis = *pThis;
            if (bThis >= 'A' && bThis <= 'Z') {
                bThis += 'a' - 'A';
            }
            FX_BYTE bThat = *pThat;
            if (bThat >= 'A' && bThat <= 'Z') {
                bThat += 'a' - 'A';
            }
            if (bThis != bThat) {
                return false;
            }
        }
        pThis ++;
        pThat ++;
    }
    return true;
}
void CFX_ByteString::AssignCopy(FX_STRSIZE nSrcLen, FX_LPCSTR lpszSrcData)
{
    AllocBeforeWrite(nSrcLen);
    FXSYS_memcpy32(m_pData->m_String, lpszSrcData, nSrcLen);
    m_pData->m_nDataLength = nSrcLen;
    m_pData->m_String[nSrcLen] = 0;
}
void CFX_ByteString::CopyBeforeWrite()
{
    if (m_pData == NULL || m_pData->m_nRefs <= 1) {
        return;
    }
    CFX_StringData* pData = m_pData;
    m_pData->m_nRefs --;
    FX_STRSIZE nDataLength = pData->m_nDataLength;
    m_pData = FX_AllocString(nDataLength);
    if (m_pData != NULL) {
        FXSYS_memcpy32(m_pData->m_String, pData->m_String, nDataLength + 1);
    }
}
void CFX_ByteString::AllocBeforeWrite(FX_STRSIZE nLen)
{
    if (m_pData && m_pData->m_nRefs <= 1 && m_pData->m_nAllocLength >= nLen) {
        return;
    }
    Empty();
    m_pData = FX_AllocString(nLen);
}
void CFX_ByteString::ReleaseBuffer(FX_STRSIZE nNewLength)
{
    if (m_pData == NULL) {
        return;
    }
    CopyBeforeWrite();
    if (nNewLength == -1) {
        nNewLength = (FX_STRSIZE)FXSYS_strlen((FX_LPCSTR)m_pData->m_String);
    }
    if (nNewLength == 0) {
        Empty();
        return;
    }
    FXSYS_assert(nNewLength <= m_pData->m_nAllocLength);
    m_pData->m_nDataLength = nNewLength;
    m_pData->m_String[nNewLength] = 0;
}
FX_LPSTR CFX_ByteString::LockBuffer()
{
    if (m_pData == NULL) {
        return NULL;
    }
    FX_LPSTR lpsz = GetBuffer(0);
    m_pData->m_nRefs = -1;
    return lpsz;
}
void CFX_ByteString::Reserve(FX_STRSIZE len)
{
    GetBuffer(len);
    ReleaseBuffer(GetLength());
}
FX_LPSTR CFX_ByteString::GetBuffer(FX_STRSIZE nMinBufLength)
{
    if (m_pData == NULL && nMinBufLength == 0) {
        return NULL;
    }
    if (m_pData && m_pData->m_nRefs <= 1 && m_pData->m_nAllocLength >= nMinBufLength) {
        return m_pData->m_String;
    }
    if (m_pData == NULL) {
        m_pData = FX_AllocString(nMinBufLength);
        if (!m_pData) {
            return NULL;
        }
        m_pData->m_nDataLength = 0;
        m_pData->m_String[0] = 0;
        return m_pData->m_String;
    }
    CFX_StringData* pOldData = m_pData;
    FX_STRSIZE nOldLen = pOldData->m_nDataLength;
    if (nMinBufLength < nOldLen) {
        nMinBufLength = nOldLen;
    }
    m_pData = FX_AllocString(nMinBufLength);
    if (!m_pData) {
        return NULL;
    }
    FXSYS_memcpy32(m_pData->m_String, pOldData->m_String, (nOldLen + 1));
    m_pData->m_nDataLength = nOldLen;
    pOldData->m_nRefs --;
    if (pOldData->m_nRefs <= 0) {
        FX_Free(pOldData);
    }
    return m_pData->m_String;
}
FX_STRSIZE CFX_ByteString::Delete(FX_STRSIZE nIndex, FX_STRSIZE nCount)
{
    if (m_pData == NULL) {
        return 0;
    }
    if (nIndex < 0) {
        nIndex = 0;
    }
    FX_STRSIZE nOldLength = m_pData->m_nDataLength;
    if (nCount > 0 && nIndex < nOldLength) {
        FX_STRSIZE mLength = nIndex + nCount;
        if (mLength >= nOldLength) {
            m_pData->m_nDataLength = nIndex;
            return m_pData->m_nDataLength;
        }
        CopyBeforeWrite();
        int nBytesToCopy = nOldLength - mLength + 1;
        FXSYS_memmove32(m_pData->m_String + nIndex,
                        m_pData->m_String + mLength, nBytesToCopy);
        m_pData->m_nDataLength = nOldLength - nCount;
    }
    return m_pData->m_nDataLength;
}
void CFX_ByteString::ConcatInPlace(FX_STRSIZE nSrcLen, FX_LPCSTR lpszSrcData)
{
    if (nSrcLen == 0 || lpszSrcData == NULL) {
        return;
    }
    if (m_pData == NULL) {
        m_pData = FX_AllocString(nSrcLen);
        if (!m_pData) {
            return;
        }
        FXSYS_memcpy32(m_pData->m_String, lpszSrcData, nSrcLen);
        return;
    }
    if (m_pData->m_nRefs > 1 || m_pData->m_nDataLength + nSrcLen > m_pData->m_nAllocLength) {
        CFX_StringData* pOldData = m_pData;
        ConcatCopy(m_pData->m_nDataLength, m_pData->m_String, nSrcLen, lpszSrcData);
        FX_ReleaseString(pOldData);
    } else {
        FXSYS_memcpy32(m_pData->m_String + m_pData->m_nDataLength, lpszSrcData, nSrcLen);
        m_pData->m_nDataLength += nSrcLen;
        m_pData->m_String[m_pData->m_nDataLength] = 0;
    }
}
void CFX_ByteString::ConcatCopy(FX_STRSIZE nSrc1Len, FX_LPCSTR lpszSrc1Data,
                                FX_STRSIZE nSrc2Len, FX_LPCSTR lpszSrc2Data)
{
    int nNewLen = nSrc1Len + nSrc2Len;
    if (nNewLen == 0) {
        return;
    }
    m_pData = FX_AllocString(nNewLen);
    if (m_pData) {
        FXSYS_memcpy32(m_pData->m_String, lpszSrc1Data, nSrc1Len);
        FXSYS_memcpy32(m_pData->m_String + nSrc1Len, lpszSrc2Data, nSrc2Len);
    }
}
CFX_ByteString CFX_ByteString::Mid(FX_STRSIZE nFirst) const
{
    if (m_pData == NULL) {
        return CFX_ByteString();
    }
    return Mid(nFirst, m_pData->m_nDataLength - nFirst);
}
CFX_ByteString CFX_ByteString::Mid(FX_STRSIZE nFirst, FX_STRSIZE nCount) const
{
    if (nFirst < 0) {
        nFirst = 0;
    }
    if (nCount < 0) {
        nCount = 0;
    }
    if (nFirst + nCount > m_pData->m_nDataLength) {
        nCount = m_pData->m_nDataLength - nFirst;
    }
    if (nFirst > m_pData->m_nDataLength) {
        nCount = 0;
    }
    if (nFirst == 0 && nFirst + nCount == m_pData->m_nDataLength) {
        return *this;
    }
    CFX_ByteString dest;
    AllocCopy(dest, nCount, nFirst);
    return dest;
}
void CFX_ByteString::AllocCopy(CFX_ByteString& dest, FX_STRSIZE nCopyLen, FX_STRSIZE nCopyIndex) const
{
    // |FX_STRSIZE| is currently typedef'd as in |int|. TODO(palmer): It
    // should be a |size_t|, or at least unsigned.
    if (nCopyLen == 0 || nCopyLen < 0) {
        return;
    }
    ASSERT(dest.m_pData == NULL);
    dest.m_pData = FX_AllocString(nCopyLen);
    if (dest.m_pData) {
        FXSYS_memcpy32(dest.m_pData->m_String, m_pData->m_String + nCopyIndex, nCopyLen);
    }
}
#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000
#define FORCE_INT64     0x40000
void CFX_ByteString::FormatV(FX_LPCSTR lpszFormat, va_list argList)
{
    va_list argListSave;
#if defined(__ARMCC_VERSION) || (!defined(_MSC_VER) && (_FX_CPU_ == _FX_X64_ || _FX_CPU_ == _FX_IA64_ || _FX_CPU_ == _FX_ARM64_)) || defined(__native_client__)
    va_copy(argListSave, argList);
#else
    argListSave = argList;
#endif
    int nMaxLen = 0;
    for (FX_LPCSTR lpsz = lpszFormat; *lpsz != 0; lpsz ++) {
        if (*lpsz != '%' || *(lpsz = lpsz + 1) == '%') {
            nMaxLen += (FX_STRSIZE)FXSYS_strlen(lpsz);
            continue;
        }
        int nItemLen = 0;
        int nWidth = 0;
        for (; *lpsz != 0; lpsz ++) {
            if (*lpsz == '#') {
                nMaxLen += 2;
            } else if (*lpsz == '*') {
                nWidth = va_arg(argList, int);
            } else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
                       *lpsz == ' ')
                ;
            else {
                break;
            }
        }
        if (nWidth == 0) {
            nWidth = FXSYS_atoi(lpsz);
            for (; (*lpsz) >= '0' && (*lpsz) <= '9'; lpsz ++)
                ;
        }
        if (nWidth < 0 || nWidth > 128 * 1024) {
            lpszFormat = "Bad width";
            nMaxLen = 10;
            break;
        }
        int nPrecision = 0;
        if (*lpsz == '.') {
            lpsz ++;
            if (*lpsz == '*') {
                nPrecision = va_arg(argList, int);
                lpsz ++;
            } else {
                nPrecision = FXSYS_atoi(lpsz);
                for (; (*lpsz) >= '0' && (*lpsz) <= '9'; lpsz ++)
                    ;
            }
        }
        if (nPrecision < 0 || nPrecision > 128 * 1024) {
            lpszFormat = "Bad precision";
            nMaxLen = 14;
            break;
        }
        int nModifier = 0;
        if (FXSYS_strncmp(lpsz, "I64", 3) == 0) {
            lpsz += 3;
            nModifier = FORCE_INT64;
        } else {
            switch (*lpsz) {
                case 'h':
                    nModifier = FORCE_ANSI;
                    lpsz ++;
                    break;
                case 'l':
                    nModifier = FORCE_UNICODE;
                    lpsz ++;
                    break;
                case 'F':
                case 'N':
                case 'L':
                    lpsz ++;
                    break;
            }
        }
        switch (*lpsz | nModifier) {
            case 'c':
            case 'C':
                nItemLen = 2;
                va_arg(argList, int);
                break;
            case 'c'|FORCE_ANSI:
            case 'C'|FORCE_ANSI:
                nItemLen = 2;
                va_arg(argList, int);
                break;
            case 'c'|FORCE_UNICODE:
            case 'C'|FORCE_UNICODE:
                nItemLen = 2;
                va_arg(argList, int);
                break;
            case 's': {
                    FX_LPCSTR pstrNextArg = va_arg(argList, FX_LPCSTR);
                    if (pstrNextArg == NULL) {
                        nItemLen = 6;
                    } else {
                        nItemLen = (FX_STRSIZE)FXSYS_strlen(pstrNextArg);
                        if (nItemLen < 1) {
                            nItemLen = 1;
                        }
                    }
                }
                break;
            case 'S': {
                    FX_LPWSTR pstrNextArg = va_arg(argList, FX_LPWSTR);
                    if (pstrNextArg == NULL) {
                        nItemLen = 6;
                    } else {
                        nItemLen = (FX_STRSIZE)FXSYS_wcslen(pstrNextArg);
                        if (nItemLen < 1) {
                            nItemLen = 1;
                        }
                    }
                }
                break;
            case 's'|FORCE_ANSI:
            case 'S'|FORCE_ANSI: {
                    FX_LPCSTR pstrNextArg = va_arg(argList, FX_LPCSTR);
                    if (pstrNextArg == NULL) {
                        nItemLen = 6;
                    } else {
                        nItemLen = (FX_STRSIZE)FXSYS_strlen(pstrNextArg);
                        if (nItemLen < 1) {
                            nItemLen = 1;
                        }
                    }
                }
                break;
            case 's'|FORCE_UNICODE:
            case 'S'|FORCE_UNICODE: {
                    FX_LPWSTR pstrNextArg = va_arg(argList, FX_LPWSTR);
                    if (pstrNextArg == NULL) {
                        nItemLen = 6;
                    } else {
                        nItemLen = (FX_STRSIZE)FXSYS_wcslen(pstrNextArg);
                        if (nItemLen < 1) {
                            nItemLen = 1;
                        }
                    }
                }
                break;
        }
        if (nItemLen != 0) {
            if (nPrecision != 0 && nItemLen > nPrecision) {
                nItemLen = nPrecision;
            }
            if (nItemLen < nWidth) {
                nItemLen = nWidth;
            }
        } else {
            switch (*lpsz) {
                case 'd':
                case 'i':
                case 'u':
                case 'x':
                case 'X':
                case 'o':
                    if (nModifier & FORCE_INT64) {
                        va_arg(argList, FX_INT64);
                    } else {
                        va_arg(argList, int);
                    }
                    nItemLen = 32;
                    if (nItemLen < nWidth + nPrecision) {
                        nItemLen = nWidth + nPrecision;
                    }
                    break;
                case 'a':
                case 'A':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                    va_arg(argList, double);
                    nItemLen = 128;
                    if (nItemLen < nWidth + nPrecision) {
                        nItemLen = nWidth + nPrecision;
                    }
                    break;
                case 'f':
                    if (nWidth + nPrecision > 100) {
                        nItemLen = nPrecision + nWidth + 128;
                    } else {
                        double f;
                        char pszTemp[256];
                        f = va_arg(argList, double);
                        FXSYS_sprintf(pszTemp, "%*.*f", nWidth, nPrecision + 6, f );
                        nItemLen = (FX_STRSIZE)FXSYS_strlen(pszTemp);
                    }
                    break;
                case 'p':
                    va_arg(argList, void*);
                    nItemLen = 32;
                    if (nItemLen < nWidth + nPrecision) {
                        nItemLen = nWidth + nPrecision;
                    }
                    break;
                case 'n':
                    va_arg(argList, int*);
                    break;
            }
        }
        nMaxLen += nItemLen;
    }
    GetBuffer(nMaxLen);
    if (m_pData) {
        FXSYS_vsprintf(m_pData->m_String, lpszFormat, argListSave);
        ReleaseBuffer();
    }
    va_end(argListSave);
}
void CFX_ByteString::Format(FX_LPCSTR lpszFormat, ...)
{
    va_list argList;
    va_start(argList, lpszFormat);
    FormatV(lpszFormat, argList);
    va_end(argList);
}
FX_STRSIZE CFX_ByteString::Insert(FX_STRSIZE nIndex, FX_CHAR ch)
{
    CopyBeforeWrite();
    if (nIndex < 0) {
        nIndex = 0;
    }
    FX_STRSIZE nNewLength = m_pData ? m_pData->m_nDataLength : 0;
    if (nIndex > nNewLength) {
        nIndex = nNewLength;
    }
    nNewLength++;
    if (m_pData == NULL || m_pData->m_nAllocLength < nNewLength) {
        CFX_StringData* pOldData = m_pData;
        FX_LPCSTR pstr = m_pData->m_String;
        m_pData = FX_AllocString(nNewLength);
        if (!m_pData) {
            return 0;
        }
        if(pOldData != NULL) {
            FXSYS_memmove32(m_pData->m_String, pstr, (pOldData->m_nDataLength + 1));
            FX_ReleaseString(pOldData);
        } else {
            m_pData->m_String[0] = 0;
        }
    }
    FXSYS_memmove32(m_pData->m_String + nIndex + 1,
                    m_pData->m_String + nIndex, (nNewLength - nIndex));
    m_pData->m_String[nIndex] = ch;
    m_pData->m_nDataLength = nNewLength;
    return nNewLength;
}
CFX_ByteString CFX_ByteString::Right(FX_STRSIZE nCount) const
{
    if (m_pData == NULL) {
        return CFX_ByteString();
    }
    if (nCount < 0) {
        nCount = 0;
    }
    if (nCount >= m_pData->m_nDataLength) {
        return *this;
    }
    CFX_ByteString dest;
    AllocCopy(dest, nCount, m_pData->m_nDataLength - nCount);
    return dest;
}
CFX_ByteString CFX_ByteString::Left(FX_STRSIZE nCount) const
{
    if (m_pData == NULL) {
        return CFX_ByteString();
    }
    if (nCount < 0) {
        nCount = 0;
    }
    if (nCount >= m_pData->m_nDataLength) {
        return *this;
    }
    CFX_ByteString dest;
    AllocCopy(dest, nCount, 0);
    return dest;
}
FX_STRSIZE CFX_ByteString::Find(FX_CHAR ch, FX_STRSIZE nStart) const
{
    if (m_pData == NULL) {
        return -1;
    }
    FX_STRSIZE nLength = m_pData->m_nDataLength;
    if (nStart >= nLength) {
        return -1;
    }
    FX_LPCSTR lpsz = FXSYS_strchr(m_pData->m_String + nStart, ch);
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pData->m_String);
}
FX_STRSIZE CFX_ByteString::ReverseFind(FX_CHAR ch) const
{
    if (m_pData == NULL) {
        return -1;
    }
    FX_STRSIZE nLength = m_pData->m_nDataLength;
    while (nLength) {
        if (m_pData->m_String[nLength - 1] == ch) {
            return nLength - 1;
        }
        nLength --;
    }
    return -1;
}
FX_LPCSTR FX_strstr(FX_LPCSTR str1, int len1, FX_LPCSTR str2, int len2)
{
    if (len2 > len1 || len2 == 0) {
        return NULL;
    }
    FX_LPCSTR end_ptr = str1 + len1 - len2;
    while (str1 <= end_ptr) {
        int i = 0;
        while (1) {
            if (str1[i] != str2[i]) {
                break;
            }
            i ++;
            if (i == len2) {
                return str1;
            }
        }
        str1 ++;
    }
    return NULL;
}
FX_STRSIZE CFX_ByteString::Find(FX_BSTR lpszSub, FX_STRSIZE nStart) const
{
    if (m_pData == NULL) {
        return -1;
    }
    FX_STRSIZE nLength = m_pData->m_nDataLength;
    if (nStart > nLength) {
        return -1;
    }
    FX_LPCSTR lpsz = FX_strstr(m_pData->m_String + nStart, m_pData->m_nDataLength - nStart,
                               lpszSub.GetCStr(), lpszSub.GetLength());
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pData->m_String);
}
void CFX_ByteString::MakeLower()
{
    if (m_pData == NULL) {
        return;
    }
    CopyBeforeWrite();
    if (GetLength() < 1) {
        return;
    }
    FXSYS_strlwr(m_pData->m_String);
}
void CFX_ByteString::MakeUpper()
{
    if (m_pData == NULL) {
        return;
    }
    CopyBeforeWrite();
    if (GetLength() < 1) {
        return;
    }
    FXSYS_strupr(m_pData->m_String);
}
FX_STRSIZE CFX_ByteString::Remove(FX_CHAR chRemove)
{
    if (m_pData == NULL) {
        return 0;
    }
    CopyBeforeWrite();
    if (GetLength() < 1) {
        return 0;
    }
    FX_LPSTR pstrSource = m_pData->m_String;
    FX_LPSTR pstrDest = m_pData->m_String;
    FX_LPSTR pstrEnd = m_pData->m_String + m_pData->m_nDataLength;
    while (pstrSource < pstrEnd) {
        if (*pstrSource != chRemove) {
            *pstrDest = *pstrSource;
            pstrDest ++;
        }
        pstrSource ++;
    }
    *pstrDest = 0;
    FX_STRSIZE nCount = (FX_STRSIZE)(pstrSource - pstrDest);
    m_pData->m_nDataLength -= nCount;
    return nCount;
}
FX_STRSIZE CFX_ByteString::Replace(FX_BSTR lpszOld, FX_BSTR lpszNew)
{
    if (m_pData == NULL) {
        return 0;
    }
    if (lpszOld.IsEmpty()) {
        return 0;
    }
    FX_STRSIZE nSourceLen = lpszOld.GetLength();
    FX_STRSIZE nReplacementLen = lpszNew.GetLength();
    FX_STRSIZE nCount = 0;
    FX_LPCSTR pStart = m_pData->m_String;
    FX_LPSTR pEnd = m_pData->m_String + m_pData->m_nDataLength;
    while (1) {
        FX_LPCSTR pTarget = FX_strstr(pStart, (FX_STRSIZE)(pEnd - pStart), lpszOld.GetCStr(), nSourceLen);
        if (pTarget == NULL) {
            break;
        }
        nCount++;
        pStart = pTarget + nSourceLen;
    }
    if (nCount == 0) {
        return 0;
    }
    FX_STRSIZE nNewLength =  m_pData->m_nDataLength + (nReplacementLen - nSourceLen) * nCount;
    if (nNewLength == 0) {
        Empty();
        return nCount;
    }
    CFX_StringData* pNewData = FX_AllocString(nNewLength);
    if (!pNewData) {
        return 0;
    }
    pStart = m_pData->m_String;
    FX_LPSTR pDest = pNewData->m_String;
    for (FX_STRSIZE i = 0; i < nCount; i ++) {
        FX_LPCSTR pTarget = FX_strstr(pStart, (FX_STRSIZE)(pEnd - pStart), lpszOld.GetCStr(), nSourceLen);
        FXSYS_memcpy32(pDest, pStart, pTarget - pStart);
        pDest += pTarget - pStart;
        FXSYS_memcpy32(pDest, lpszNew.GetCStr(), lpszNew.GetLength());
        pDest += lpszNew.GetLength();
        pStart = pTarget + nSourceLen;
    }
    FXSYS_memcpy32(pDest, pStart, pEnd - pStart);
    FX_ReleaseString(m_pData);
    m_pData = pNewData;
    return nCount;
}
void CFX_ByteString::SetAt(FX_STRSIZE nIndex, FX_CHAR ch)
{
    if (m_pData == NULL) {
        return;
    }
    FXSYS_assert(nIndex >= 0);
    FXSYS_assert(nIndex < m_pData->m_nDataLength);
    CopyBeforeWrite();
    m_pData->m_String[nIndex] = ch;
}
CFX_WideString CFX_ByteString::UTF8Decode() const
{
    CFX_UTF8Decoder decoder;
    for (FX_STRSIZE i = 0; i < GetLength(); i ++) {
        decoder.Input((FX_BYTE)m_pData->m_String[i]);
    }
    return decoder.GetResult();
}
CFX_ByteString CFX_ByteString::FromUnicode(FX_LPCWSTR str, FX_STRSIZE len)
{
    if (len < 0) {
        len = (FX_STRSIZE)FXSYS_wcslen(str);
    }
    CFX_ByteString bstr;
    bstr.ConvertFrom(CFX_WideString(str, len));
    return bstr;
}
CFX_ByteString CFX_ByteString::FromUnicode(const CFX_WideString& str)
{
    return FromUnicode(str.c_str(), str.GetLength());
}
void CFX_ByteString::ConvertFrom(const CFX_WideString& str, CFX_CharMap* pCharMap)
{
    if (pCharMap == NULL) {
        pCharMap = CFX_CharMap::GetDefaultMapper();
    }
    *this = (*pCharMap->m_GetByteString)(pCharMap, str);
}
int CFX_ByteString::Compare(FX_BSTR str) const
{
    if (m_pData == NULL) {
        return str.IsEmpty() ? 0 : -1;
    }
    int this_len = m_pData->m_nDataLength;
    int that_len = str.GetLength();
    int min_len = this_len < that_len ? this_len : that_len;
    for (int i = 0; i < min_len; i ++) {
        if ((FX_BYTE)m_pData->m_String[i] < str.GetAt(i)) {
            return -1;
        } else if ((FX_BYTE)m_pData->m_String[i] > str.GetAt(i)) {
            return 1;
        }
    }
    if (this_len < that_len) {
        return -1;
    } else if (this_len > that_len) {
        return 1;
    }
    return 0;
}
void CFX_ByteString::TrimRight(FX_BSTR lpszTargets)
{
    if (m_pData == NULL || lpszTargets.IsEmpty()) {
        return;
    }
    CopyBeforeWrite();
    FX_STRSIZE pos = GetLength();
    if (pos < 1) {
        return;
    }
    while (pos) {
        FX_STRSIZE i = 0;
        while (i < lpszTargets.GetLength() && lpszTargets[i] != m_pData->m_String[pos - 1]) {
            i ++;
        }
        if (i == lpszTargets.GetLength()) {
            break;
        }
        pos --;
    }
    if (pos < m_pData->m_nDataLength) {
        m_pData->m_String[pos] = 0;
        m_pData->m_nDataLength = pos;
    }
}
void CFX_ByteString::TrimRight(FX_CHAR chTarget)
{
    TrimRight(CFX_ByteStringC(chTarget));
}
void CFX_ByteString::TrimRight()
{
    TrimRight(FX_BSTRC("\x09\x0a\x0b\x0c\x0d\x20"));
}
void CFX_ByteString::TrimLeft(FX_BSTR lpszTargets)
{
    if (m_pData == NULL) {
        return;
    }
    if (lpszTargets.IsEmpty()) {
        return;
    }
    CopyBeforeWrite();
    FX_STRSIZE len = GetLength();
    if (len < 1) {
        return;
    }
    FX_STRSIZE pos = 0;
    while (pos < len) {
        FX_STRSIZE i = 0;
        while (i < lpszTargets.GetLength() && lpszTargets[i] != m_pData->m_String[pos]) {
            i ++;
        }
        if (i == lpszTargets.GetLength()) {
            break;
        }
        pos ++;
    }
    if (pos) {
        FX_STRSIZE nDataLength = len - pos;
        FXSYS_memmove32(m_pData->m_String, m_pData->m_String + pos, (nDataLength + 1)*sizeof(FX_CHAR));
        m_pData->m_nDataLength = nDataLength;
    }
}
void CFX_ByteString::TrimLeft(FX_CHAR chTarget)
{
    TrimLeft(CFX_ByteStringC(chTarget));
}
void CFX_ByteString::TrimLeft()
{
    TrimLeft(FX_BSTRC("\x09\x0a\x0b\x0c\x0d\x20"));
}
FX_DWORD CFX_ByteString::GetID(FX_STRSIZE start_pos) const
{
    return CFX_ByteStringC(*this).GetID(start_pos);
}
FX_DWORD CFX_ByteStringC::GetID(FX_STRSIZE start_pos) const
{
    if (m_Length == 0) {
        return 0;
    }
    if (start_pos < 0 || start_pos >= m_Length) {
        return 0;
    }
    FX_DWORD strid = 0;
    if (start_pos + 4 > m_Length) {
        for (FX_STRSIZE i = 0; i < m_Length - start_pos; i ++) {
            strid = strid * 256 + m_Ptr[start_pos + i];
        }
        strid = strid << ((4 - m_Length + start_pos) * 8);
    } else {
        for (int i = 0; i < 4; i ++) {
            strid = strid * 256 + m_Ptr[start_pos + i];
        }
    }
    return strid;
}
FX_STRSIZE FX_ftoa(FX_FLOAT d, FX_LPSTR buf)
{
    buf[0] = '0';
    buf[1] = '\0';
    if (d == 0.0f) {
        return 1;
    }
    FX_BOOL bNegative = FALSE;
    if (d < 0) {
        bNegative = TRUE;
        d = -d;
    }
    int scale = 1;
    int scaled = FXSYS_round(d);
    while (scaled < 100000) {
        if (scale == 1000000) {
            break;
        }
        scale *= 10;
        scaled = FXSYS_round(d * scale);
    }
    if (scaled == 0) {
        return 1;
    }
    char buf2[32];
    int buf_size = 0;
    if (bNegative) {
        buf[buf_size++] = '-';
    }
    int i = scaled / scale;
    FXSYS_itoa(i, buf2, 10);
    FX_STRSIZE len = (FX_STRSIZE)FXSYS_strlen(buf2);
    FXSYS_memcpy32(buf + buf_size, buf2, len);
    buf_size += len;
    int fraction = scaled % scale;
    if (fraction == 0) {
        return buf_size;
    }
    buf[buf_size++] = '.';
    scale /= 10;
    while (fraction) {
        buf[buf_size++] = '0' + fraction / scale;
        fraction %= scale;
        scale /= 10;
    }
    return buf_size;
}
CFX_ByteString CFX_ByteString::FormatFloat(FX_FLOAT d, int precision)
{
    FX_CHAR buf[32];
    FX_STRSIZE len = FX_ftoa(d, buf);
    return CFX_ByteString(buf, len);
}
void CFX_StringBufBase::Copy(FX_BSTR str)
{
    m_Size = str.GetLength();
    if (m_Size > m_Limit) {
        m_Size = m_Limit;
    }
    FX_CHAR* pBuffer = (FX_CHAR*)(this + 1);
    FXSYS_memcpy32(pBuffer, str.GetPtr(), m_Size);
}
void CFX_StringBufBase::Append(FX_BSTR str)
{
    int len = str.GetLength();
    if (len > m_Limit - m_Size) {
        len = m_Limit - m_Size;
    }
    FX_CHAR* pBuffer = (FX_CHAR*)(this + 1);
    FXSYS_memcpy32(pBuffer + m_Size, str.GetPtr(), len);
    m_Size += len;
}
void CFX_StringBufBase::Append(int i, FX_DWORD flags)
{
    char buf[32];
    int len = _Buffer_itoa(buf, i, flags);
    Append(CFX_ByteStringC(buf, len));
}
