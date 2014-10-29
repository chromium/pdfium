// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa_fm2js.h"
struct XFA_FMDChar {
    static FX_LPCWSTR inc(FX_LPCWSTR &p)
    {
        ++p;
        return p;
    }
    static FX_LPCWSTR dec(FX_LPCWSTR &p)
    {
        --p;
        return p;
    }
    static FX_UINT16 get(FX_LPCWSTR p)
    {
        return *p;
    }
    static FX_BOOL isWhiteSpace(FX_LPCWSTR p)
    {
        return (*p) == 0x09 || (*p) == 0x0b || (*p) == 0x0c || (*p) == 0x20;
    }
    static FX_BOOL isLineTerminator(FX_LPCWSTR p)
    {
        return *p == 0x0A || *p == 0x0D;
    }
    static FX_BOOL isBinary(FX_LPCWSTR p)
    {
        return (*p) >= '0' && (*p) <= '1';
    }
    static FX_BOOL isOctal(FX_LPCWSTR p)
    {
        return (*p) >= '0' && (*p) <= '7';
    }
    static FX_BOOL isDigital(FX_LPCWSTR p)
    {
        return (*p) >= '0' && (*p) <= '9';
    }
    static FX_BOOL isHex(FX_LPCWSTR p)
    {
        return isDigital(p) || ((*p) >= 'a' && (*p) <= 'f') || ((*p) >= 'A' && (*p) <= 'F');
    }
    static FX_BOOL isAlpha(FX_LPCWSTR p)
    {
        return ((*p) <= 'z' && (*p) >= 'a') || ((*p) <= 'Z' && (*p) >= 'A');
    }
    static FX_BOOL isAvalid(FX_LPCWSTR p, FX_BOOL flag = 0);
    static FX_BOOL string2number(FX_LPCWSTR s, FX_DOUBLE *pValue, FX_LPCWSTR &pEnd);
    static FX_BOOL isUnicodeAlpha(FX_UINT16 ch);
};
inline FX_BOOL XFA_FMDChar::isAvalid(FX_LPCWSTR p, FX_BOOL flag)
{
    if (*p == 0) {
        return 1;
    }
    if ((*p <= 0x0A && *p >= 0x09) || *p == 0x0D || (*p <= 0xd7ff && *p >= 0x20) || (*p <= 0xfffd && *p >= 0xe000)) {
        return 1;
    }
    if (!flag) {
        if (*p == 0x0B || *p == 0x0C) {
            return 1;
        }
    }
    return 0;
}
inline FX_BOOL XFA_FMDChar::string2number(FX_LPCWSTR s, FX_DOUBLE *pValue, FX_LPCWSTR &pEnd)
{
    if (s) {
        *pValue = wcstod((wchar_t *)s, (wchar_t **)&pEnd);
    }
    return 0;
}
inline FX_BOOL XFA_FMDChar::isUnicodeAlpha(FX_UINT16 ch)
{
    if (ch == 0 || ch == 0x0A || ch == 0x0D || ch == 0x09 || ch == 0x0B || ch == 0x0C || ch == 0x20 || ch == '.'
            || ch == ';' || ch == '"' || ch == '=' || ch == '<' || ch == '>' || ch == ',' || ch == '(' || ch == ')'
            || ch == ']' || ch == '[' || ch == '&' || ch == '|' || ch == '+' || ch == '-' || ch == '*' || ch == '/') {
        return FALSE;
    } else {
        return TRUE;
    }
}
static XFA_FMKeyword keyWords[] = {
    {TOKand,		0x00000026,		(FX_LPCWSTR)(L"&")},
    {TOKlparen,		0x00000028,		(FX_LPCWSTR)(L"(")},
    {TOKrparen,		0x00000029,		(FX_LPCWSTR)(L")")},
    {TOKmul,		0x0000002a,		(FX_LPCWSTR)(L"*")},
    {TOKplus,		0x0000002b,		(FX_LPCWSTR)(L"+")},
    {TOKcomma,		0x0000002c,		(FX_LPCWSTR)(L",")},
    {TOKminus,		0x0000002d,		(FX_LPCWSTR)(L"-")},
    {TOKdot,		0x0000002e,		(FX_LPCWSTR)(L".")},
    {TOKdiv,		0x0000002f,		(FX_LPCWSTR)(L"/")},
    {TOKlt,			0x0000003c,		(FX_LPCWSTR)(L"<")},
    {TOKassign,		0x0000003d,		(FX_LPCWSTR)(L"=")},
    {TOKgt,			0x0000003e,		(FX_LPCWSTR)(L">")},
    {TOKlbracket,	0x0000005b,		(FX_LPCWSTR)(L"[")},
    {TOKrbracket,	0x0000005d,		(FX_LPCWSTR)(L"]")},
    {TOKor,			0x0000007c,		(FX_LPCWSTR)(L"|")},
    {TOKdotscream,	0x0000ec11,		(FX_LPCWSTR)(L".#")},
    {TOKdotstar,	0x0000ec18,		(FX_LPCWSTR)(L".*")},
    {TOKdotdot,		0x0000ec1c,		(FX_LPCWSTR)(L"..")},
    {TOKle,			0x000133f9,		(FX_LPCWSTR)(L"<=")},
    {TOKne,			0x000133fa,		(FX_LPCWSTR)(L"<>")},
    {TOKeq,			0x0001391a,		(FX_LPCWSTR)(L"==")},
    {TOKge,			0x00013e3b,		(FX_LPCWSTR)(L">=")},
    {TOKdo,			0x00020153,		(FX_LPCWSTR)(L"do")},
    {TOKkseq,		0x00020676,		(FX_LPCWSTR)(L"eq")},
    {TOKksge,		0x000210ac,		(FX_LPCWSTR)(L"ge")},
    {TOKksgt,		0x000210bb,		(FX_LPCWSTR)(L"gt")},
    {TOKif,			0x00021aef,		(FX_LPCWSTR)(L"if")},
    {TOKin,			0x00021af7,		(FX_LPCWSTR)(L"in")},
    {TOKksle,		0x00022a51,		(FX_LPCWSTR)(L"le")},
    {TOKkslt,		0x00022a60,		(FX_LPCWSTR)(L"lt")},
    {TOKksne,		0x00023493,		(FX_LPCWSTR)(L"ne")},
    {TOKksor,		0x000239c1,		(FX_LPCWSTR)(L"or")},
    {TOKnull,		0x052931bb,		(FX_LPCWSTR)(L"null")},
    {TOKbreak,		0x05518c25,		(FX_LPCWSTR)(L"break")},
    {TOKksand,		0x09f9db33,		(FX_LPCWSTR)(L"and")},
    {TOKend,		0x0a631437,		(FX_LPCWSTR)(L"end")},
    {TOKeof,		0x0a63195a,		(FX_LPCWSTR)(L"eof")},
    {TOKfor,		0x0a7d67a7,		(FX_LPCWSTR)(L"for")},
    {TOKnan,		0x0b4f91dd,		(FX_LPCWSTR)(L"nan")},
    {TOKksnot,		0x0b4fd9b1,		(FX_LPCWSTR)(L"not")},
    {TOKvar,		0x0c2203e9,		(FX_LPCWSTR)(L"var")},
    {TOKthen,		0x2d5738cf,		(FX_LPCWSTR)(L"then")},
    {TOKelse,		0x45f65ee9,		(FX_LPCWSTR)(L"else")},
    {TOKexit,		0x4731d6ba,		(FX_LPCWSTR)(L"exit")},
    {TOKdownto,		0x4caadc3b,		(FX_LPCWSTR)(L"downto")},
    {TOKreturn,		0x4db8bd60,		(FX_LPCWSTR)(L"return")},
    {TOKinfinity,	0x5c0a010a,		(FX_LPCWSTR)(L"infinity")},
    {TOKendwhile,	0x5c64bff0,		(FX_LPCWSTR)(L"endwhile")},
    {TOKforeach,	0x67e31f38,		(FX_LPCWSTR)(L"foreach")},
    {TOKendfunc,	0x68f984a3,		(FX_LPCWSTR)(L"endfunc")},
    {TOKelseif,		0x78253218,		(FX_LPCWSTR)(L"elseif")},
    {TOKwhile,		0x84229259,		(FX_LPCWSTR)(L"while")},
    {TOKendfor,		0x8ab49d7e,		(FX_LPCWSTR)(L"endfor")},
    {TOKthrow,		0x8db05c94,		(FX_LPCWSTR)(L"throw")},
    {TOKstep,		0xa7a7887c,		(FX_LPCWSTR)(L"step")},
    {TOKupto,		0xb5155328,		(FX_LPCWSTR)(L"upto")},
    {TOKcontinue,	0xc0340685,		(FX_LPCWSTR)(L"continue")},
    {TOKfunc,		0xcdce60ec,		(FX_LPCWSTR)(L"func")},
    {TOKendif,		0xe0e8fee6,		(FX_LPCWSTR)(L"endif")},
};
static const FX_WORD KEYWORD_START = TOKdo;
static const FX_WORD KEYWORD_END = TOKendif;
FX_LPCWSTR XFA_FM_KeywordToString(XFA_FM_TOKEN op)
{
    return keyWords[op].m_keword;
}
CXFA_FMToken::CXFA_FMToken()
{
    m_type = TOKreserver;
    m_uLinenum = 1;
    m_pNext = 0;
}
CXFA_FMToken::CXFA_FMToken( FX_DWORD uLineNum )
{
    m_type = TOKreserver;
    m_uLinenum = uLineNum;
    m_pNext = 0;
}
CXFA_FMToken::~CXFA_FMToken()
{
}
CXFA_FMLexer::CXFA_FMLexer(FX_WSTR wsFormCalc, CXFA_FMErrorInfo *pErrorInfo)
{
    m_pScript = wsFormCalc.GetPtr();
    m_uLength = wsFormCalc.GetLength();
    m_uCurrentLine = 1;
    m_ptr = m_pScript;
    m_pToken = 0;
    m_pErrorInfo = pErrorInfo;
}
CXFA_FMToken *CXFA_FMLexer::NextToken()
{
    CXFA_FMToken *t = 0;
    if (!m_pToken) {
        m_pToken = Scan();
    } else {
        if (m_pToken->m_pNext) {
            t = m_pToken->m_pNext;
            delete m_pToken;
            m_pToken = t;
        } else {
            t = m_pToken;
            m_pToken = Scan();
            delete t;
        }
    }
    return m_pToken;
}
CXFA_FMToken * CXFA_FMLexer::Scan()
{
    FX_UINT16 ch = 0;
    CXFA_FMToken * p = FX_NEW CXFA_FMToken(m_uCurrentLine);
    if (!XFA_FMDChar::isAvalid(m_ptr)) {
        ch = XFA_FMDChar::get(m_ptr);
        Error(FMERR_UNSUPPORTED_CHAR, ch);
        return p;
    }
    int iRet = 0;
    while (1) {
        if (!XFA_FMDChar::isAvalid(m_ptr)) {
            ch = XFA_FMDChar::get(m_ptr);
            Error(FMERR_UNSUPPORTED_CHAR, ch);
            return p;
        }
        ch = XFA_FMDChar::get(m_ptr);
        switch (ch) {
            case 0:
                p->m_type = TOKeof;
                return p;
            case 0x0A:
                ++m_uCurrentLine;
                p->m_uLinenum = m_uCurrentLine;
                XFA_FMDChar::inc(m_ptr);
                break;
            case 0x0D:
                XFA_FMDChar::inc(m_ptr);
                break;
            case ';': {
                    FX_LPCWSTR pTemp = 0;
                    Comment(m_ptr, pTemp);
                    m_ptr = pTemp;
                }
                break;
            case '"': {
                    FX_LPCWSTR pTemp = 0;
                    p->m_type = TOKstring;
                    iRet = String(p, m_ptr, pTemp);
                    if (iRet) {
                        return p;
                    }
                    m_ptr = pTemp;
                }
                return p;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                    p->m_type = TOKnumber;
                    FX_LPCWSTR pTemp = 0;
                    iRet = Number(p, m_ptr, pTemp);
                    m_ptr = pTemp;
                    if (iRet) {
                        Error(FMERR_BAD_SUFFIX_NUMBER);
                        return p;
                    }
                }
                return p;
            case '=':
                XFA_FMDChar::inc(m_ptr);
                if (XFA_FMDChar::isAvalid(m_ptr)) {
                    ch = XFA_FMDChar::get(m_ptr);
                    if (ch == '=') {
                        p->m_type = TOKeq;
                        XFA_FMDChar::inc(m_ptr);
                        return p;
                    } else {
                        p->m_type = TOKassign;
                        return p;
                    }
                } else {
                    ch = XFA_FMDChar::get(m_ptr);
                    Error(FMERR_UNSUPPORTED_CHAR, ch);
                    return p;
                }
                break;
            case '<':
                XFA_FMDChar::inc(m_ptr);
                if (XFA_FMDChar::isAvalid(m_ptr)) {
                    ch = XFA_FMDChar::get(m_ptr);
                    if (ch == '=') {
                        p->m_type = TOKle;
                        XFA_FMDChar::inc(m_ptr);
                        return p;
                    } else if (ch == '>') {
                        p->m_type = TOKne;
                        XFA_FMDChar::inc(m_ptr);
                        return p;
                    } else {
                        p->m_type = TOKlt;
                        return p;
                    }
                } else {
                    ch = XFA_FMDChar::get(m_ptr);
                    Error(FMERR_UNSUPPORTED_CHAR, ch);
                    return p;
                }
                break;
            case '>':
                XFA_FMDChar::inc(m_ptr);
                if (XFA_FMDChar::isAvalid(m_ptr)) {
                    ch = XFA_FMDChar::get(m_ptr);
                    if (ch == '=') {
                        p->m_type = TOKge;
                        XFA_FMDChar::inc(m_ptr);
                        return p;
                    } else {
                        p->m_type = TOKgt;
                        return p;
                    }
                } else {
                    ch = XFA_FMDChar::get(m_ptr);
                    Error(FMERR_UNSUPPORTED_CHAR, ch);
                    return p;
                }
                break;
            case ',':
                p->m_type = TOKcomma;
                XFA_FMDChar::inc(m_ptr);
                return p;
            case '(':
                p->m_type = TOKlparen;
                XFA_FMDChar::inc(m_ptr);
                return p;
            case ')':
                p->m_type = TOKrparen;
                XFA_FMDChar::inc(m_ptr);
                return p;
            case '[':
                p->m_type = TOKlbracket;
                XFA_FMDChar::inc(m_ptr);
                return p;
            case ']':
                p->m_type = TOKrbracket;
                XFA_FMDChar::inc(m_ptr);
                return p;
            case '&':
                XFA_FMDChar::inc(m_ptr);
                p->m_type = TOKand;
                return p;
            case '|':
                XFA_FMDChar::inc(m_ptr);
                p->m_type = TOKor;
                return p;
            case '+':
                XFA_FMDChar::inc(m_ptr);
                p->m_type = TOKplus;
                return p;
            case '-':
                XFA_FMDChar::inc(m_ptr);
                p->m_type = TOKminus;
                return p;
            case '*':
                XFA_FMDChar::inc(m_ptr);
                p->m_type = TOKmul;
                return p;
            case '/':
                XFA_FMDChar::inc(m_ptr);
                if (XFA_FMDChar::isAvalid(m_ptr)) {
                    ch = XFA_FMDChar::get(m_ptr);
                    if (ch == '/') {
                        FX_LPCWSTR pTemp = 0;
                        Comment(m_ptr, pTemp);
                        m_ptr = pTemp;
                        break;
                    } else {
                        p->m_type = TOKdiv;
                        return p;
                    }
                } else {
                    ch = XFA_FMDChar::get(m_ptr);
                    Error(FMERR_UNSUPPORTED_CHAR, ch);
                    return p;
                }
                break;
            case '.':
                XFA_FMDChar::inc(m_ptr);
                if (XFA_FMDChar::isAvalid(m_ptr)) {
                    ch = XFA_FMDChar::get(m_ptr);
                    if (ch == '.') {
                        p->m_type = TOKdotdot;
                        XFA_FMDChar::inc(m_ptr);
                        return p;
                    } else if (ch == '*') {
                        p->m_type = TOKdotstar;
                        XFA_FMDChar::inc(m_ptr);
                        return p;
                    } else if (ch == '#') {
                        p->m_type = TOKdotscream;
                        XFA_FMDChar::inc(m_ptr);
                        return p;
                    } else if (ch <= '9' && ch >= '0') {
                        p->m_type = TOKnumber;
                        FX_LPCWSTR pTemp = 0;
                        XFA_FMDChar::dec(m_ptr);
                        iRet = Number(p, m_ptr, pTemp);
                        m_ptr = pTemp;
                        if (iRet) {
                            Error(FMERR_BAD_SUFFIX_NUMBER);
                        }
                        return p;
                    } else {
                        p->m_type = TOKdot;
                        return p;
                    }
                } else {
                    ch = XFA_FMDChar::get(m_ptr);
                    Error(FMERR_UNSUPPORTED_CHAR, ch);
                    return p;
                }
            case 0x09:
            case 0x0B:
            case 0x0C:
            case 0x20:
                XFA_FMDChar::inc(m_ptr);
                break;
            default: {
                    FX_LPCWSTR pTemp = 0;
                    iRet = Identifiers(p, m_ptr, pTemp);
                    m_ptr = pTemp;
                    if (iRet) {
                        return p;
                    }
                    p->m_type = IsKeyword(p->m_wstring);
                }
                return p;
        }
    }
}
FX_DWORD CXFA_FMLexer::Number(CXFA_FMToken *t, FX_LPCWSTR p, FX_LPCWSTR &pEnd)
{
    FX_DOUBLE number = 0;
    if (XFA_FMDChar::string2number(p, &number, pEnd)) {
        return 1;
    }
    if (pEnd && XFA_FMDChar::isAlpha(pEnd)) {
        return 1;
    }
    t->m_wstring = CFX_WideStringC(p, (pEnd - p));
    return 0;
}
FX_DWORD CXFA_FMLexer::String(CXFA_FMToken *t, FX_LPCWSTR p, FX_LPCWSTR &pEnd)
{
    FX_LPCWSTR pStart = p;
    FX_UINT16 ch = 0;
    XFA_FMDChar::inc(p);
    ch = XFA_FMDChar::get(p);
    while (ch) {
        if (!XFA_FMDChar::isAvalid(p)) {
            ch = XFA_FMDChar::get(p);
            pEnd = p;
            t->m_wstring = CFX_WideStringC(pStart, (pEnd - pStart));
            Error(FMERR_UNSUPPORTED_CHAR, ch);
            return 1;
        }
        if (ch == '"') {
            XFA_FMDChar::inc(p);
            if (!XFA_FMDChar::isAvalid(p)) {
                ch = XFA_FMDChar::get(p);
                pEnd = p;
                t->m_wstring = CFX_WideStringC(pStart, (pEnd - pStart));
                Error(FMERR_UNSUPPORTED_CHAR, ch);
                return 1;
            }
            ch = XFA_FMDChar::get(p);
            if (ch == '"') {
                goto NEXT;
            } else {
                break;
            }
        }
NEXT:
        XFA_FMDChar::inc(p);
        ch = XFA_FMDChar::get(p);
    }
    pEnd = p;
    t->m_wstring = CFX_WideStringC(pStart, (pEnd - pStart));
    return 0;
}
FX_DWORD CXFA_FMLexer::Identifiers(CXFA_FMToken *t, FX_LPCWSTR p, FX_LPCWSTR &pEnd)
{
    FX_LPCWSTR pStart = p;
    FX_UINT16 ch = 0;
    ch = XFA_FMDChar::get(p);
    XFA_FMDChar::inc(p);
    if (!XFA_FMDChar::isAvalid(p)) {
        pEnd = p;
        t->m_wstring = CFX_WideStringC(pStart, (pEnd - pStart));
        Error(FMERR_UNSUPPORTED_CHAR, ch);
        return 1;
    }
    ch = XFA_FMDChar::get(p);
    while (ch) {
        if (!XFA_FMDChar::isAvalid(p)) {
            pEnd = p;
            t->m_wstring = CFX_WideStringC(pStart, (pEnd - pStart));
            Error(FMERR_UNSUPPORTED_CHAR, ch);
            return 1;
        }
        ch = XFA_FMDChar::get(p);
        if (XFA_FMDChar::isUnicodeAlpha(ch)) {
            XFA_FMDChar::inc(p);
        } else {
            pEnd = p;
            t->m_wstring = CFX_WideStringC(pStart, (pEnd - pStart));
            return 0;
        }
    }
    pEnd = p;
    t->m_wstring = CFX_WideStringC(pStart, (pEnd - pStart));
    return 0;
}
void CXFA_FMLexer::Comment( FX_LPCWSTR p, FX_LPCWSTR &pEnd )
{
    unsigned ch = 0;
    XFA_FMDChar::inc(p);
    ch = XFA_FMDChar::get(p);
    while (ch) {
        if (ch == 0x0D) {
            XFA_FMDChar::inc(p);
            pEnd = p;
            return;
        }
        if (ch == 0x0A) {
            ++m_uCurrentLine;
            XFA_FMDChar::inc(p);
            pEnd = p;
            return;
        }
        XFA_FMDChar::inc(p);
        ch = XFA_FMDChar::get(p);
    }
    pEnd = p;
}
XFA_FM_TOKEN CXFA_FMLexer::IsKeyword(FX_WSTR str)
{
    FX_INT32 iLength = str.GetLength();
    FX_UINT32 uHash = FX_HashCode_String_GetW(str.GetPtr(), iLength, TRUE);
    FX_INT32 iStart = KEYWORD_START, iEnd = KEYWORD_END;
    FX_INT32 iMid = (iStart + iEnd) / 2;
    XFA_FMKeyword keyword;
    do {
        iMid = (iStart + iEnd) / 2;
        keyword = keyWords[iMid];
        if (uHash == keyword.m_uHash) {
            return keyword.m_type;
        } else if (uHash < keyword.m_uHash) {
            iEnd = iMid - 1;
        } else {
            iStart = iMid + 1;
        }
    } while (iStart <= iEnd);
    return TOKidentifier;
}
CXFA_FMLexer::~CXFA_FMLexer()
{
    m_pScript = 0;
    m_ptr = m_pScript;
    if (m_pToken) {
        CXFA_FMToken *t1 = m_pToken;
        CXFA_FMToken *t2 = t1->m_pNext;
        while (t2) {
            delete t1;
            t1 = t2;
            t2 = t2->m_pNext;
        }
        delete m_pToken;
        m_pToken = 0;
    }
    m_pErrorInfo = 0;
}
void CXFA_FMLexer::Error(XFA_FM_ERRMSG msg, ...)
{
    m_pErrorInfo->linenum = m_uCurrentLine;
    FX_LPCWSTR lpMessageInfo = XFA_FM_ErrorMsg(msg);
    va_list ap;
    va_start(ap, msg);
    m_pErrorInfo->message.FormatV(lpMessageInfo, ap);
    va_end(ap);
}
FX_BOOL CXFA_FMLexer::HasError() const
{
    if (m_pErrorInfo->message.IsEmpty()) {
        return FALSE;
    }
    return TRUE;
}
