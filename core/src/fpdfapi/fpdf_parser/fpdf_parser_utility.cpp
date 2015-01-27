// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_parser.h"
const char PDF_CharType[256] = {
  //NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL  BS   HT   LF   VT   FF   CR   SO   SI
    'W', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'W', 'W', 'R', 'W', 'W', 'R', 'R',

  //DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB  CAN  EM   SUB  ESC  FS   GS   RS   US
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',

  //SP    !    "    #    $    %    &    ´    (    )    *    +    ,    -    .    /
    'W', 'R', 'R', 'R', 'R', 'D', 'R', 'R', 'D', 'D', 'R', 'N', 'R', 'N', 'N', 'D',

  // 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ?
    'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'N', 'R', 'R', 'D', 'R', 'D', 'R',

  // @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',

  // P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'D', 'R', 'D', 'R', 'R',

  // `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',

  // p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~   DEL
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'D', 'R', 'D', 'R', 'R',

    'W', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R',
    'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'R', 'W'
};

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif
CPDF_SimpleParser::CPDF_SimpleParser(FX_LPCBYTE pData, FX_DWORD dwSize)
{
    m_pData = pData;
    m_dwSize = dwSize;
    m_dwCurPos = 0;
}
CPDF_SimpleParser::CPDF_SimpleParser(FX_BSTR str)
{
    m_pData = str;
    m_dwSize = str.GetLength();
    m_dwCurPos = 0;
}
void CPDF_SimpleParser::ParseWord(FX_LPCBYTE& pStart, FX_DWORD& dwSize, int& type)
{
    pStart = NULL;
    dwSize = 0;
    type = PDFWORD_EOF;
    FX_BYTE ch;
    char chartype;
    while (1) {
        if (m_dwSize <= m_dwCurPos) {
            return;
        }
        ch = m_pData[m_dwCurPos++];
        chartype = PDF_CharType[ch];
        while (chartype == 'W') {
            if (m_dwSize <= m_dwCurPos) {
                return;
            }
            ch = m_pData[m_dwCurPos++];
            chartype = PDF_CharType[ch];
        }
        if (ch != '%') {
            break;
        }
        while (1) {
            if (m_dwSize <= m_dwCurPos) {
                return;
            }
            ch = m_pData[m_dwCurPos++];
            if (ch == '\r' || ch == '\n') {
                break;
            }
        }
        chartype = PDF_CharType[ch];
    }
    FX_DWORD start_pos = m_dwCurPos - 1;
    pStart = m_pData + start_pos;
    if (chartype == 'D') {
        if (ch == '/') {
            while (1) {
                if (m_dwSize <= m_dwCurPos) {
                    return;
                }
                ch = m_pData[m_dwCurPos++];
                chartype = PDF_CharType[ch];
                if (chartype != 'R' && chartype != 'N') {
                    m_dwCurPos --;
                    dwSize = m_dwCurPos - start_pos;
                    type = PDFWORD_NAME;
                    return;
                }
            }
        } else {
            type = PDFWORD_DELIMITER;
            dwSize = 1;
            if (ch == '<') {
                if (m_dwSize <= m_dwCurPos) {
                    return;
                }
                ch = m_pData[m_dwCurPos++];
                if (ch == '<') {
                    dwSize = 2;
                } else {
                    m_dwCurPos --;
                }
            } else if (ch == '>') {
                if (m_dwSize <= m_dwCurPos) {
                    return;
                }
                ch = m_pData[m_dwCurPos++];
                if (ch == '>') {
                    dwSize = 2;
                } else {
                    m_dwCurPos --;
                }
            }
        }
        return;
    }
    type = PDFWORD_NUMBER;
    dwSize = 1;
    while (1) {
        if (chartype != 'N') {
            type = PDFWORD_TEXT;
        }
        if (m_dwSize <= m_dwCurPos) {
            return;
        }
        ch = m_pData[m_dwCurPos++];
        chartype = PDF_CharType[ch];
        if (chartype == 'D' || chartype == 'W') {
            m_dwCurPos --;
            break;
        }
        dwSize ++;
    }
}
CFX_ByteStringC CPDF_SimpleParser::GetWord()
{
    FX_LPCBYTE pStart;
    FX_DWORD dwSize;
    int type;
    ParseWord(pStart, dwSize, type);
    if (dwSize == 1 && pStart[0] == '<') {
        while (m_dwCurPos < m_dwSize && m_pData[m_dwCurPos] != '>') {
            m_dwCurPos ++;
        }
        if (m_dwCurPos < m_dwSize) {
            m_dwCurPos ++;
        }
        return CFX_ByteStringC(pStart, (FX_STRSIZE)(m_dwCurPos - (pStart - m_pData)));
    } else if (dwSize == 1 && pStart[0] == '(') {
        int level = 1;
        while (m_dwCurPos < m_dwSize) {
            if (m_pData[m_dwCurPos] == ')') {
                level --;
                if (level == 0) {
                    break;
                }
            }
            if (m_pData[m_dwCurPos] == '\\') {
                if (m_dwSize <= m_dwCurPos) {
                    break;
                }
                m_dwCurPos ++;
            } else if (m_pData[m_dwCurPos] == '(') {
                level ++;
            }
            if (m_dwSize <= m_dwCurPos) {
                break;
            }
            m_dwCurPos ++;
        }
        if (m_dwCurPos < m_dwSize) {
            m_dwCurPos ++;
        }
        return CFX_ByteStringC(pStart, (FX_STRSIZE)(m_dwCurPos - (pStart - m_pData)));
    }
    return CFX_ByteStringC(pStart, dwSize);
}
FX_BOOL CPDF_SimpleParser::SearchToken(FX_BSTR token)
{
    int token_len = token.GetLength();
    while (m_dwCurPos < m_dwSize - token_len) {
        if (FXSYS_memcmp32(m_pData + m_dwCurPos, token, token_len) == 0) {
            break;
        }
        m_dwCurPos ++;
    }
    if (m_dwCurPos == m_dwSize - token_len) {
        return FALSE;
    }
    m_dwCurPos += token_len;
    return TRUE;
}
FX_BOOL CPDF_SimpleParser::SkipWord(FX_BSTR token)
{
    while (1) {
        CFX_ByteStringC word = GetWord();
        if (word.IsEmpty()) {
            return FALSE;
        }
        if (word == token) {
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_SimpleParser::FindTagPair(FX_BSTR start_token, FX_BSTR end_token,
                                       FX_DWORD& start_pos, FX_DWORD& end_pos)
{
    if (!start_token.IsEmpty()) {
        if (!SkipWord(start_token)) {
            return FALSE;
        }
        start_pos = m_dwCurPos;
    }
    while (1) {
        end_pos = m_dwCurPos;
        CFX_ByteStringC word = GetWord();
        if (word.IsEmpty()) {
            return FALSE;
        }
        if (word == end_token) {
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_SimpleParser::FindTagParam(FX_BSTR token, int nParams)
{
    nParams ++;
    FX_DWORD* pBuf = FX_Alloc(FX_DWORD, nParams);
    int buf_index = 0;
    int buf_count = 0;
    while (1) {
        pBuf[buf_index++] = m_dwCurPos;
        if (buf_index == nParams) {
            buf_index = 0;
        }
        buf_count ++;
        if (buf_count > nParams) {
            buf_count = nParams;
        }
        CFX_ByteStringC word = GetWord();
        if (word.IsEmpty()) {
            FX_Free(pBuf);
            return FALSE;
        }
        if (word == token) {
            if (buf_count < nParams) {
                continue;
            }
            m_dwCurPos = pBuf[buf_index];
            FX_Free(pBuf);
            return TRUE;
        }
    }
    return FALSE;
}
static int _hex2dec(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return 0;
}
CFX_ByteString PDF_NameDecode(FX_BSTR bstr)
{
    int size = bstr.GetLength();
    FX_LPCSTR pSrc = bstr.GetCStr();
    if (FXSYS_memchr(pSrc, '#', size) == NULL) {
        return bstr;
    }
    CFX_ByteString result;
    FX_LPSTR pDestStart = result.GetBuffer(size);
    FX_LPSTR pDest = pDestStart;
    for (int i = 0; i < size; i ++) {
        if (pSrc[i] == '#' && i < size - 2) {
            *pDest ++ = _hex2dec(pSrc[i + 1]) * 16 + _hex2dec(pSrc[i + 2]);
            i += 2;
        } else {
            *pDest ++ = pSrc[i];
        }
    }
    result.ReleaseBuffer((FX_STRSIZE)(pDest - pDestStart));
    return result;
}
CFX_ByteString PDF_NameDecode(const CFX_ByteString& orig)
{
    if (FXSYS_memchr(orig.c_str(), '#', orig.GetLength()) == NULL) {
        return orig;
    }
    return PDF_NameDecode(CFX_ByteStringC(orig));
}
CFX_ByteString PDF_NameEncode(const CFX_ByteString& orig)
{
    FX_LPBYTE src_buf = (FX_LPBYTE)orig.c_str();
    int src_len = orig.GetLength();
    int dest_len = 0;
    int i;
    for (i = 0; i < src_len; i ++) {
        FX_BYTE ch = src_buf[i];
        if (ch >= 0x80 || PDF_CharType[ch] == 'W' || ch == '#' ||
                PDF_CharType[ch] == 'D') {
            dest_len += 3;
        } else {
            dest_len ++;
        }
    }
    if (dest_len == src_len) {
        return orig;
    }
    CFX_ByteString res;
    FX_LPSTR dest_buf = res.GetBuffer(dest_len);
    dest_len = 0;
    for (i = 0; i < src_len; i ++) {
        FX_BYTE ch = src_buf[i];
        if (ch >= 0x80 || PDF_CharType[ch] == 'W' || ch == '#' ||
                PDF_CharType[ch] == 'D') {
            dest_buf[dest_len++] = '#';
            dest_buf[dest_len++] = "0123456789ABCDEF"[ch / 16];
            dest_buf[dest_len++] = "0123456789ABCDEF"[ch % 16];
        } else {
            dest_buf[dest_len++] = ch;
        }
    }
    dest_buf[dest_len] = 0;
    res.ReleaseBuffer();
    return res;
}
CFX_ByteTextBuf& operator << (CFX_ByteTextBuf& buf, const CPDF_Object* pObj)
{
    if (pObj == NULL) {
        buf << FX_BSTRC(" null");
        return buf;
    }
    switch (pObj->GetType()) {
        case PDFOBJ_NULL:
            buf << FX_BSTRC(" null");
            break;
        case PDFOBJ_BOOLEAN:
        case PDFOBJ_NUMBER:
            buf << " " << pObj->GetString();
            break;
        case PDFOBJ_STRING: {
                CFX_ByteString str = pObj->GetString();
                FX_BOOL bHex = ((CPDF_String*)pObj)->IsHex();
                buf << PDF_EncodeString(str, bHex);
                break;
            }
        case PDFOBJ_NAME: {
                CFX_ByteString str = pObj->GetString();
                buf << FX_BSTRC("/") << PDF_NameEncode(str);
                break;
            }
        case PDFOBJ_REFERENCE: {
                CPDF_Reference* p = (CPDF_Reference*)pObj;
                buf << " " << p->GetRefObjNum() << FX_BSTRC(" 0 R ");
                break;
            }
        case PDFOBJ_ARRAY: {
                CPDF_Array* p = (CPDF_Array*)pObj;
                buf << FX_BSTRC("[");
                for (FX_DWORD i = 0; i < p->GetCount(); i ++) {
                    CPDF_Object* pElement = p->GetElement(i);
                    if (pElement->GetObjNum()) {
                        buf << " " << pElement->GetObjNum() << FX_BSTRC(" 0 R");
                    } else {
                        buf << pElement;
                    }
                }
                buf << FX_BSTRC("]");
                break;
            }
        case PDFOBJ_DICTIONARY: {
                CPDF_Dictionary* p = (CPDF_Dictionary*)pObj;
                buf << FX_BSTRC("<<");
                FX_POSITION pos = p->GetStartPos();
                while (pos) {
                    CFX_ByteString key;
                    CPDF_Object* pValue = p->GetNextElement(pos, key);
                    buf << FX_BSTRC("/") << PDF_NameEncode(key);
                    if (pValue->GetObjNum()) {
                        buf << " " << pValue->GetObjNum() << FX_BSTRC(" 0 R ");
                    } else {
                        buf << pValue;
                    }
                }
                buf << FX_BSTRC(">>");
                break;
            }
        case PDFOBJ_STREAM: {
                CPDF_Stream* p = (CPDF_Stream*)pObj;
                buf << p->GetDict() << FX_BSTRC("stream\r\n");
                CPDF_StreamAcc acc;
                acc.LoadAllData(p, TRUE);
                buf.AppendBlock(acc.GetData(), acc.GetSize());
                buf << FX_BSTRC("\r\nendstream");
                break;
            }
        default:
            ASSERT(FALSE);
            break;
    }
    return buf;
}
FX_FLOAT PDF_ClipFloat(FX_FLOAT f)
{
    if (f < 0) {
        return 0;
    }
    if (f > 1.0f) {
        return 1.0f;
    }
    return f;
}
static CPDF_Object* SearchNumberNode(CPDF_Dictionary* pNode, int num)
{
    CPDF_Array* pLimits = pNode->GetArray("Limits");
    if (pLimits && (num < pLimits->GetInteger(0) || num > pLimits->GetInteger(1))) {
        return NULL;
    }
    CPDF_Array* pNumbers = pNode->GetArray("Nums");
    if (pNumbers) {
        FX_DWORD dwCount = pNumbers->GetCount() / 2;
        for (FX_DWORD i = 0; i < dwCount; i ++) {
            int index = pNumbers->GetInteger(i * 2);
            if (num == index) {
                return pNumbers->GetElementValue(i * 2 + 1);
            }
            if (index > num) {
                break;
            }
        }
        return NULL;
    }
    CPDF_Array* pKids = pNode->GetArray("Kids");
    if (pKids == NULL) {
        return NULL;
    }
    for (FX_DWORD i = 0; i < pKids->GetCount(); i ++) {
        CPDF_Dictionary* pKid = pKids->GetDict(i);
        if (pKid == NULL) {
            continue;
        }
        CPDF_Object* pFound = SearchNumberNode(pKid, num);
        if (pFound) {
            return pFound;
        }
    }
    return NULL;
}
CPDF_Object* CPDF_NumberTree::LookupValue(int num)
{
    return SearchNumberNode(m_pRoot, num);
}
