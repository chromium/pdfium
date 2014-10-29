// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "fde_txtedtblock.h"
#ifdef FDE_USEFORMATBLOCK
#define FDE_TXTEDT_FORMATBLOCK_BGN		0xFFF9
#define FDE_TXTEDT_FORMATBLOCK_END		0xFFFB
#define FDE_TXTEDT_ZEROWIDTHSPACE		0x200B
#define FDE_TXTEDT_ISINTEGER(a)	((a) >= L'0' && (a) <= L'9')
#define FDE_TXTEDT_ISSIGN(a)	(((a) == L'-') || ((a) == L'+'))
CFDE_TxtEdtBlock::CFDE_TxtEdtBlock(CFDE_TxtEdtEngine * pEngine, const CFX_WideString &wsBlock, FX_INT32 nPosition)
    : m_pEngine(pEngine)
    , m_nDisplayLength(0)
    , m_nIndex(0)
    , m_nPosition(nPosition)
{
    FX_LPCWSTR lpBuf = FX_LPCWSTR(wsBlock);
    FX_INT32 nCount	= wsBlock.GetLength();
    FX_INT32 i		= 0;
    CFX_WideString wsFix;
    FX_INT32 j = 0;
    while (i < nCount) {
        if (lpBuf[i] != L'%') {
            wsFix += lpBuf[i];
        } else {
            i ++;
            if (i < nCount) {
                if (lpBuf[i] == L'%') {
                    wsFix += lpBuf[i];
                } else {
                    if (!wsFix.IsEmpty()) {
                        CFDE_TxtEdtField * pField = CFDE_TxtEdtField::Create(wsFix, j, this);
                        j ++;
                        FXSYS_assert(pField);
                        m_FieldArr.Add(pField);
                        m_nDisplayLength += pField->GetDisplayLength();
                        wsFix.Empty();
                    }
                    FX_INT32 nPos = i - 1;
                    while (lpBuf[i ++] != L')')
                        ;
                    i ++;
                    CFX_WideStringC wsField(lpBuf + nPos, i - nPos);
                    CFDE_TxtEdtField * pField = CFDE_TxtEdtField::Create(wsField, j, this);
                    j ++;
                    FXSYS_assert(pField);
                    m_FieldArr.Add(pField);
                    m_EditFieldArr.Add(pField);
                    m_nDisplayLength += pField->GetDisplayLength();
                    i --;
                }
            }
        }
        i ++;
    }
    if (!wsFix.IsEmpty()) {
        CFDE_TxtEdtField * pField = CFDE_TxtEdtField::Create(wsFix, j, this);
        FXSYS_assert(pField);
        m_FieldArr.Add(pField);
        m_nDisplayLength += pField->GetDisplayLength();
    }
}
CFDE_TxtEdtBlock::~CFDE_TxtEdtBlock()
{
    FX_INT32 nCount = m_FieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFDE_TxtEdtField * pField = m_FieldArr[i];
        pField->Release();
    }
    m_FieldArr.RemoveAll();
}
void CFDE_TxtEdtBlock::GetDisplayText(CFX_WideString &wsDisplay)
{
    FX_INT32 nCount = m_FieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFDE_TxtEdtField * pField = m_FieldArr[i];
        CFX_WideString wsTemp;
        pField->GetDisplayText(wsTemp);
        wsDisplay += wsTemp;
    }
}
FX_INT32 CFDE_TxtEdtBlock::GetLength() const
{
    FX_INT32 nDisplayLength = 0;
    FX_INT32 nCount = m_FieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFDE_TxtEdtField * pField = m_FieldArr[i];
        nDisplayLength += pField->GetDisplayLength();
    }
    return nDisplayLength;
}
void CFDE_TxtEdtBlock::GetBlockText(CFX_WideString &wsBlock)
{
    FX_INT32 nCount = m_FieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFDE_TxtEdtField * pField = m_FieldArr[i];
        CFX_WideString wsTemp;
        pField->GetFieldText(wsTemp);
        wsBlock += wsTemp;
    }
}
FX_INT32 CFDE_TxtEdtBlock::CountField() const
{
    return m_EditFieldArr.GetSize();
}
void CFDE_TxtEdtBlock::GetFieldText(FX_INT32 nIndex, CFX_WideString &wsField)
{
    CFDE_TxtEdtField * pField = m_EditFieldArr[nIndex];
    pField->GetFieldText(wsField);
}
FX_INT32 CFDE_TxtEdtBlock::GetFieldTextLength() const
{
    FX_INT32 nTotalLength = 0;
    FX_INT32 nCount = m_EditFieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFDE_TxtEdtField * pField = m_EditFieldArr[i];
        nTotalLength = pField->GetFieldTextLength();
    }
    return nTotalLength;
}
FX_INT32 CFDE_TxtEdtBlock::GetPos() const
{
    return m_nPosition;
}
void CFDE_TxtEdtBlock::GetRealText(CFX_WideString &wsText) const
{
    FX_INT32 nCount = m_FieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        CFDE_TxtEdtField * pField = m_FieldArr[i];
        CFX_WideString wsTemp;
        pField->GetFieldText(wsTemp);
        wsText += wsTemp;
    }
}
void CFDE_TxtEdtBlock::Backup()
{
    FX_INT32 nCount = m_EditFieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        m_EditFieldArr[i]->Backup();
    }
}
void CFDE_TxtEdtBlock::Restore()
{
    FX_INT32 nCount = m_EditFieldArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        m_EditFieldArr[i]->Restore();
    }
}
CFDE_TxtEdtFieldFormatParser::CFDE_TxtEdtFieldFormatParser()
{
}
CFDE_TxtEdtFieldFormatParser::~CFDE_TxtEdtFieldFormatParser()
{
    FDE_LPTXTEDTFORMATITEM lpItem = NULL;
    FX_INT32 nCount = m_ItemArr.GetSize();
    for (FX_INT32 i = 0; i < nCount; i ++) {
        lpItem = m_ItemArr[i];
        delete lpItem;
    }
    m_ItemArr.RemoveAll();
}
FX_BOOL CFDE_TxtEdtFieldFormatParser::Parse(const CFX_WideString &wsFormat)
{
    m_wsFormat = wsFormat;
    FX_LPCWSTR	pBuf	= FX_LPCWSTR(m_wsFormat);
    FX_INT32	nCount	= m_wsFormat.GetLength();
    nCount -= 2;
    FX_INT32 i = 0;
    for (; i < nCount; i ++) {
        FX_WCHAR wChar = pBuf[i];
        if (wChar == L'(') {
            break;
        }
    }
    i ++;
    FDE_TXTEDTFORMATITEM FormatItem;
    for (; i < nCount; i ++) {
        while (pBuf[i] == L' ') {
            i ++;
        }
        FormatItem.nKeyStart = i;
        while (pBuf[i] != L':') {
            i ++;
        }
        FormatItem.nKeyCount = i - FormatItem.nKeyStart;
        i ++;
        FormatItem.nValStart = i;
        while (pBuf[i] != L';' && i < nCount) {
            i ++;
        }
        FormatItem.nValCount = i - FormatItem.nValStart;
        FDE_LPTXTEDTFORMATITEM pFormatItem = FX_NEW FDE_TXTEDTFORMATITEM;
        FXSYS_memcpy(pFormatItem, &FormatItem, sizeof(FDE_TXTEDTFORMATITEM));
        m_ItemArr.Add(pFormatItem);
    }
    return TRUE;
}
FX_INT32 CFDE_TxtEdtFieldFormatParser::CountItems() const
{
    return m_ItemArr.GetSize();
}
void CFDE_TxtEdtFieldFormatParser::GetItem(FX_INT32 nIndex, CFX_WideString &wsKey, CFX_WideString &wsValue) const
{
    FDE_LPTXTEDTFORMATITEM lpItem = m_ItemArr[nIndex];
    FX_LPCWSTR lpSrcBuf = FX_LPCWSTR(m_wsFormat);
    FX_LPWSTR lpDstBuf = wsKey.GetBuffer(lpItem->nKeyCount);
    FXSYS_memcpy(lpDstBuf, lpSrcBuf + lpItem->nKeyStart, lpItem->nKeyCount * sizeof(FX_WCHAR));
    wsKey.ReleaseBuffer(lpItem->nKeyCount);
    lpDstBuf = wsValue.GetBuffer(lpItem->nValCount);
    FXSYS_memcpy(lpDstBuf, lpSrcBuf + lpItem->nValStart, lpItem->nValCount * sizeof(FX_WCHAR));
    wsValue.ReleaseBuffer(lpItem->nValCount);
}
CFDE_TxtEdtField * CFDE_TxtEdtField::Create(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock)
{
    if (wsField[0] != L'%' || (wsField[0] == L'%' && wsField[1] == L'%')) {
        return FX_NEW CFDE_TxtEdtField_Fixed(wsField, nIndex, pBlock);
    }
    FX_WCHAR wcType = wsField[wsField.GetLength() - 1];
    switch(wcType) {
        case L'd':
            return FX_NEW CFDE_TxtEdtField_Integer(wsField, nIndex, pBlock);
        case L'f':
            return FX_NEW CFDE_TxtEdtField_Float(wsField, nIndex, pBlock);
        case L's':
            return FX_NEW CFDE_TxtEdtField_String(wsField, nIndex, pBlock);
        case L'p':
            return FX_NEW CFDE_TxtEdtField_Password(wsField, nIndex, pBlock);
        default:
            break;
    }
    return NULL;
}
void CFDE_TxtEdtField::Release()
{
    delete this;
}
CFDE_TxtEdtField::CFDE_TxtEdtField(FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock)
    : m_nLength(-1)
    , m_wcFill(L' ')
    , m_bReserveSpace(FALSE)
    , m_bLeftAlignment(TRUE)
    , m_nIndex(nIndex)
    , m_pBlock(pBlock)
{
    FXSYS_assert(pBlock);
}
FX_INT32 CFDE_TxtEdtField::Insert(FX_INT32 nIndex, const CFX_WideString &wsIns,
                                  FX_INT32 &nCaret, FX_BOOL &bBefore)
{
    FX_INT32 nFieldLength	= m_wsField.GetLength();
    FX_INT32 nInnerIndex	= nIndex - FDE_FORMAT_EDIT_FIELD_HADERSIZE;
    if (m_bReserveSpace && !m_bLeftAlignment) {
        nInnerIndex -= (m_nLength - nFieldLength);
    }
    FXSYS_assert(nInnerIndex >= 0 && nInnerIndex <= nFieldLength);
    CFX_WideString wsTemp = m_wsField;
    FX_INT32 nInsLength = wsIns.GetLength();
    for (FX_INT32 i = 0; i < nInsLength; i ++, nInnerIndex ++) {
        wsTemp.Insert(nInnerIndex, wsIns[i]);
    }
    FX_INT32 nRet = Validate(wsTemp);
    switch (nRet) {
        case FDE_FORMAT_FIELD_VALIDATE_F_FULL:
            return FDE_FORMAT_FIELD_INSERT_RET_F_FULL;
        case FDE_FORMAT_FIELD_VALIDATE_F_INVALIDATE:
            return FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE;
        case FDE_FORMAT_FIELD_VALIDATE_S:
        default:
            break;
    }
    m_wsField	= wsTemp;
    nCaret		= nIndex + ((m_bReserveSpace && !m_bLeftAlignment) ? -nInsLength : nInsLength);
    bBefore		= TRUE;
    return (nFieldLength + nInsLength < m_nLength) ? FDE_FORMAT_FIELD_INSERT_RET_S_NORMAL : FDE_FORMAT_FIELD_INSERT_RET_S_FULL;
}
FX_INT32 CFDE_TxtEdtField::Delete(FX_INT32 nIndex, FX_INT32 nCount,
                                  CFX_WideString &wsDel, FX_INT32 &nCaret, FX_BOOL &bBefore)
{
    FX_INT32 nFieldLength	= m_wsField.GetLength();
    FX_INT32 nInnerIndex	= nIndex - FDE_FORMAT_EDIT_FIELD_HADERSIZE;
    if (m_bReserveSpace && !m_bLeftAlignment) {
        nInnerIndex -= (m_nLength - nFieldLength);
    }
    if (nInnerIndex < 0 || (nInnerIndex + nCount) > nFieldLength) {
        return FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY;
    }
    CFX_WideString wsTemp = m_wsField;
    wsTemp.Delete(nInnerIndex, nCount);
    FX_INT32 nRet = Validate(wsTemp);
    switch (nRet) {
        case FDE_FORMAT_FIELD_VALIDATE_F_FULL:
            return FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY;
        case FDE_FORMAT_FIELD_VALIDATE_F_INVALIDATE:
            return FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE;
        case FDE_FORMAT_FIELD_VALIDATE_S:
        default:
            break;
    }
    FX_LPWSTR lpBuf = wsDel.GetBuffer(nCount);
    FXSYS_memcpy(lpBuf, FX_LPCWSTR(m_wsField) + nInnerIndex, nCount * sizeof(FX_WCHAR));
    wsDel.ReleaseBuffer(nCount);
    m_wsField	= wsTemp;
    nCaret		= nIndex + (m_bReserveSpace && !m_bLeftAlignment) ? nCount : 0;
    bBefore		= TRUE;
    return FDE_FORMAT_FIELD_DELETE_RET_S;
}
FX_INT32 CFDE_TxtEdtField::Replace(FX_INT32 nIndex, FX_INT32 nCount, const CFX_WideString &wsIns,
                                   CFX_WideString &wsDel, FX_INT32 &nCaret, FX_BOOL &bBefore)
{
    FX_INT32		nInnerIndex		= nIndex - FDE_FORMAT_EDIT_FIELD_HADERSIZE;
    FX_INT32		nInsLength		= wsIns.GetLength();
    FX_INT32		nFieldLength	= m_wsField.GetLength();
    CFX_WideString	wsTemp			= m_wsField;
    if (m_bReserveSpace && !m_bLeftAlignment) {
        nInnerIndex -= (m_nLength - nFieldLength);
    }
    FXSYS_assert(nInnerIndex >= 0 && nInnerIndex <= nFieldLength);
    if (nInnerIndex + nCount > nFieldLength) {
        return FALSE;
    }
    wsTemp.Delete(nInnerIndex, nCount);
    FX_INT32 nInnerIndexBK = nInnerIndex;
    for (FX_INT32 i = 0; i < nInsLength; i ++, nInnerIndex ++) {
        wsTemp.Insert(nInnerIndex, wsIns[i]);
    }
    FX_INT32 nRet = Validate(wsTemp);
    switch(nRet) {
        case FDE_FORMAT_FIELD_VALIDATE_F_FULL:
            return FDE_FORMAT_FIELD_INSERT_RET_F_FULL;
        case FDE_FORMAT_FIELD_VALIDATE_F_INVALIDATE:
            return FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE;
        default:
            break;
    }
    FX_LPWSTR	lpBuffer = wsDel.GetBuffer(nCount);
    FX_LPCWSTR	lpSrcBuf = FX_LPCWSTR(m_wsField);
    FXSYS_memcpy(lpBuffer, lpSrcBuf + nInnerIndexBK, nCount * sizeof(FX_WCHAR));
    wsDel.ReleaseBuffer(nCount);
    m_wsField	= wsTemp;
    nCaret		= nIndex + ((m_bReserveSpace && !m_bLeftAlignment) ? (nCount - nInsLength) : (nInsLength));
    return FDE_FORMAT_FIELD_INSERT_RET_S_NORMAL;
}
void CFDE_TxtEdtField::GetDisplayText(CFX_WideString &wsDisplay)
{
    CFX_WideString wsField;
    GetNormalizedFieldText(wsField);
    FX_INT32 nLength	= wsField.GetLength() + FDE_FORMAT_EDIT_FIELD_HADERSIZE + FDE_FORMAT_EDIT_FIELD_TAILSIZE;
    FX_LPWSTR lpBuffer	= wsDisplay.GetBuffer(nLength);
    lpBuffer[0]				= FDE_TXTEDT_FORMATBLOCK_BGN;
    lpBuffer[nLength - 1]	= FDE_TXTEDT_FORMATBLOCK_END;
    FX_DWORD nAddress = (FX_DWORD)this;
    FXSYS_memcpy(lpBuffer + 1, &nAddress, sizeof(FX_DWORD));
    FXSYS_memcpy(lpBuffer + 3, FX_LPCWSTR(wsField), (nLength - 4) * sizeof(FX_WCHAR));
    wsDisplay.ReleaseBuffer(nLength);
}
FX_INT32 CFDE_TxtEdtField::GetDisplayLength()
{
    return (m_bReserveSpace ? m_nLength : m_wsField.GetLength()) + FDE_FORMAT_EDIT_FIELD_HADERSIZE + FDE_FORMAT_EDIT_FIELD_TAILSIZE;
}
void CFDE_TxtEdtField::GetFieldText(CFX_WideString &wsField)
{
    wsField = m_wsField;
}
FX_INT32 CFDE_TxtEdtField::GetFieldTextLength() const
{
    return m_wsField.GetLength();
}
FX_INT32 CFDE_TxtEdtField::GetRealIndex(FX_INT32 nIndex) const
{
    FX_INT32 nInnerIndex	= nIndex - FDE_FORMAT_EDIT_FIELD_HADERSIZE;
    if (nInnerIndex < 0) {
        return 0;
    }
    FX_INT32 nFieldLength	= m_wsField.GetLength();
    if (m_bReserveSpace && !m_bLeftAlignment) {
        nInnerIndex -= (m_nLength - nFieldLength);
    }
    if (nInnerIndex < 0) {
        return 0;
    }
    if (nInnerIndex >= nFieldLength) {
        return nFieldLength;
    }
    return nInnerIndex + 1;
}
FX_INT32 CFDE_TxtEdtField::NormalizeCaretPos(FX_INT32 nIndex, FDE_FORMAT_CARET_DIRECTION eDirection ) const
{
    nIndex -= FDE_FORMAT_EDIT_FIELD_HADERSIZE;
    FX_INT32 nLength = m_wsField.GetLength();
    if (m_bReserveSpace) {
        FX_INT32 nFieldLength = m_wsField.GetLength();
        if (m_bLeftAlignment) {
            if (nIndex > nFieldLength) {
                if (eDirection == FDE_FORMAT_CARET_FORWARD) {
                    return -1;
                }
                nIndex = nFieldLength;
            }
        } else {
            FX_INT32 nReserveLength = m_nLength - nFieldLength;
            if (nIndex < nReserveLength) {
                if (eDirection == FDE_FORMAT_CARET_BACKWARD) {
                    return -2;
                }
                nIndex = nReserveLength;
            }
        }
    }
    return nIndex + FDE_FORMAT_EDIT_FIELD_HADERSIZE;
}
FX_BOOL CFDE_TxtEdtField::GetEditableRange(FX_INT32 &nBgn, FX_INT32 &nEnd) const
{
    if (m_bReserveSpace && !m_bLeftAlignment) {
        nEnd = FDE_FORMAT_EDIT_FIELD_HADERSIZE + m_nLength;
        nBgn = nEnd - m_wsField.GetLength();
    } else {
        nBgn = FDE_FORMAT_EDIT_FIELD_HADERSIZE;
        nEnd = nBgn + m_wsField.GetLength();
    }
    return TRUE;
}
void CFDE_TxtEdtField::Backup()
{
    m_wsBackup = m_wsField;
}
void CFDE_TxtEdtField::Restore()
{
    m_wsField = m_wsBackup;
}
FX_INT32 CFDE_TxtEdtField::Validate(const CFX_WideString & wsText) const
{
    if (m_nLength < 0) {
        return FDE_FORMAT_FIELD_DELETE_RET_S;
    }
    return wsText.GetLength() <= m_nLength ? FDE_FORMAT_FIELD_VALIDATE_S : FDE_FORMAT_FIELD_VALIDATE_F_FULL;
}
void CFDE_TxtEdtField::GetNormalizedFieldText(CFX_WideString &wsField) const
{
    wsField = m_wsField;
    if (m_nLength == -1) {
        return;
    }
    if (m_bReserveSpace) {
        FX_INT32 nField	= wsField.GetLength();
        FX_INT32 nFill	= m_nLength - nField;
        if (m_bLeftAlignment) {
            while (nFill --) {
                wsField.Insert(nField++, m_wcFill);
            }
        } else {
            while (nFill --) {
                wsField.Insert(0, m_wcFill);
            }
        }
    }
}
CFDE_TxtEdtField_Integer::CFDE_TxtEdtField_Integer(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock)
    : m_bSign(FALSE)
    , CFDE_TxtEdtField(nIndex, pBlock)
{
    CFDE_TxtEdtFieldFormatParser FormatParser;
    FormatParser.Parse(wsField);
    FX_INT32 nCount = FormatParser.CountItems();
    CFX_WideString wskey;
    CFX_WideString wsVal;
    for (FX_INT32 i = 0; i < nCount; i ++) {
        FormatParser.GetItem(i, wskey, wsVal);
        if (wskey.Equal(L"Length")) {
            m_nLength = wsVal.GetInteger();
        } else if (wskey.Equal(L"Sign")) {
            m_bSign = wsVal.GetInteger() != 0;
        } else if (wskey.Equal(L"FillChar")) {
            m_wcFill = wsVal[0];
        } else {
            FXSYS_assert(0);
        }
        wskey.Empty();
        wsVal.Empty();
    }
    if (m_nLength == -1) {
        m_bReserveSpace = FALSE;
    }
}
FX_INT32 CFDE_TxtEdtField_Integer::Validate(const CFX_WideString &wsText) const
{
    FX_INT32 i	= 0;
    if (m_bSign) {
        FX_WCHAR wcTemp = wsText[0];
        if (FDE_TXTEDT_ISSIGN(wcTemp)) {
            i ++;
        }
    }
    FX_INT32 nLength = wsText.GetLength();
    if (m_nLength > 0) {
        if (nLength - i > (m_nLength - (m_bSign ? 1 : 0))) {
            return FDE_FORMAT_FIELD_VALIDATE_F_FULL;
        }
    }
    for (; i < nLength; i ++) {
        FX_WCHAR wcTemp = wsText[i];
        if (!FDE_TXTEDT_ISINTEGER(wcTemp)) {
            return FDE_FORMAT_FIELD_VALIDATE_F_INVALIDATE;
        }
    }
    return FDE_FORMAT_FIELD_VALIDATE_S;
}
CFDE_TxtEdtField_Float::CFDE_TxtEdtField_Float(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock)
    : CFDE_TxtEdtField(nIndex, pBlock)
    , m_bSigned(FALSE)
    , m_nIntPartlength(-1)
    , m_nDecPartLength(-1)
{
    CFDE_TxtEdtFieldFormatParser FormatParser;
    FormatParser.Parse(wsField);
    FX_INT32 nCount = FormatParser.CountItems();
    CFX_WideString wskey;
    CFX_WideString wsVal;
    for (FX_INT32 i = 0; i < nCount; i ++) {
        FormatParser.GetItem(i, wskey, wsVal);
        if (wskey.Equal(L"DecLength")) {
            m_nDecPartLength = wsVal.GetInteger();
        } else if (wskey.Equal(L"IntLength")) {
            m_nIntPartlength = wsVal.GetInteger();
        } else if (wskey.Equal(L"Sign")) {
            m_bSigned = wsVal.GetInteger() != 0;
        } else if (wskey.Equal(L"FillChar")) {
            m_wcFill = wsVal[0];
        } else {
            FXSYS_assert(0);
        }
        if (m_nIntPartlength == -1 || m_nDecPartLength == -1) {
            m_nLength = -1;
        } else {
            m_nLength = m_nIntPartlength + m_nDecPartLength + 1 + (m_bSigned ? 1 : 0);
        }
        m_bReserveSpace = TRUE;
        wskey.Empty();
        wsVal.Empty();
    }
}
FX_INT32 CFDE_TxtEdtField_Float::Validate(const CFX_WideString & wsText) const
{
    FX_INT32 nLength = wsText.GetLength();
    if (m_nLength != -1 && (nLength > m_nLength)) {
        return FDE_FORMAT_FIELD_VALIDATE_F_FULL;
    }
    FX_LPCWSTR lpBuf = FX_LPCWSTR(wsText);
    FX_INT32 i = 0;
    if (m_bSigned) {
        FX_WCHAR wcTemp = lpBuf[0];
        if (FDE_TXTEDT_ISSIGN(wcTemp)) {
            i ++;
        }
    }
    FX_INT32 nIntPart	= 0;
    FX_INT32 nPoint		= 0;
    for (; i < nLength; i ++) {
        FX_WCHAR wcTemp = lpBuf[i];
        if (!FDE_TXTEDT_ISINTEGER(wcTemp)) {
            if (wcTemp != L'.') {
                return FDE_FORMAT_FIELD_VALIDATE_F_INVALIDATE;
            }
            nPoint = 1;
            break;
        }
        nIntPart ++;
    }
    if (m_nIntPartlength != -1 && (nIntPart > m_nIntPartlength)) {
        return FDE_FORMAT_FIELD_VALIDATE_F_FULL;
    }
    if (m_nDecPartLength != -1 && (nLength - nIntPart - nPoint > m_nDecPartLength)) {
        return FDE_FORMAT_FIELD_VALIDATE_F_FULL;
    }
    i ++;
    for (; i < nLength; i ++) {
        FX_WCHAR wcTemp = lpBuf[i];
        if (!FDE_TXTEDT_ISINTEGER(wcTemp)) {
            return FDE_FORMAT_FIELD_VALIDATE_F_FULL;
        }
    }
    return FDE_FORMAT_FIELD_VALIDATE_S;
}
CFDE_TxtEdtField_Password::CFDE_TxtEdtField_Password(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock)
    : CFDE_TxtEdtField(nIndex, pBlock)
    , m_wcAlias(L'*')
{
    CFDE_TxtEdtFieldFormatParser FormatParser;
    FormatParser.Parse(wsField);
    FX_INT32 nCount = FormatParser.CountItems();
    CFX_WideString wskey;
    CFX_WideString wsVal;
    for (FX_INT32 i = 0; i < nCount; i ++) {
        FormatParser.GetItem(i, wskey, wsVal);
        if (wskey.Equal(L"Length")) {
            m_nLength = wsVal.GetInteger();
        } else if (wskey.Equal(L"AliasChar")) {
            m_wcAlias = wsVal[0];
        } else {
            FXSYS_assert(0);
        }
        wskey.Empty();
        wsVal.Empty();
    }
    if (m_nLength == -1) {
        m_bReserveSpace = FALSE;
    }
}
void CFDE_TxtEdtField_Password::GetNormalizedFieldText(CFX_WideString &wsField) const
{
    FX_INT32 nFiledLength	= m_wsField.GetLength();
    FX_INT32 nLength		= m_bReserveSpace ? m_nLength : nFiledLength;
    FX_LPWSTR lpBuf			= wsField.GetBuffer(nLength);
    FX_INT32 nSpaceLength	= nLength - nFiledLength;
    FX_INT32 nFirstPart		= m_bLeftAlignment ? nFiledLength : nSpaceLength;
    FX_WCHAR wFirstChar		= m_bLeftAlignment ? m_wcAlias : L' ';
    FX_WCHAR wSecondChar	= m_bLeftAlignment ? L' ' : m_wcAlias;
    FX_INT32 i = 0;
    for (; i < nFirstPart; i ++) {
        lpBuf[i] = wFirstChar;
    }
    for (; i < nLength; i ++) {
        lpBuf[i] = wSecondChar;
    }
    wsField.ReleaseBuffer(nLength);
}
CFDE_TxtEdtField_String::CFDE_TxtEdtField_String(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock)
    : CFDE_TxtEdtField(nIndex, pBlock)
{
    CFDE_TxtEdtFieldFormatParser FormatParser;
    FormatParser.Parse(wsField);
    FX_INT32 nCount = FormatParser.CountItems();
    CFX_WideString wskey;
    CFX_WideString wsVal;
    for (FX_INT32 i = 0; i < nCount; i ++) {
        FormatParser.GetItem(i, wskey, wsVal);
        if (wskey.Equal(L"Length")) {
            m_nLength = wsVal.GetInteger();
        } else {
            FXSYS_assert(0);
        }
        wskey.Empty();
        wsVal.Empty();
    }
}
CFDE_TxtEdtField_Fixed::CFDE_TxtEdtField_Fixed(const CFX_WideString &wsField, FX_INT32 nIndex, CFDE_TxtEdtBlock * pBlock)
    : CFDE_TxtEdtField(nIndex, pBlock)
{
    m_wsField = wsField;
    m_nLength = wsField.GetLength();
}
void CFDE_TxtEdtField_Fixed::GetDisplayText(CFX_WideString &wsDisplay)
{
    FX_INT32 nLength	= m_wsField.GetLength() + FDE_FORMAT_EDIT_FIELD_HADERSIZE + FDE_FORMAT_EDIT_FIELD_TAILSIZE;
    FX_LPWSTR lpBuffer	= wsDisplay.GetBuffer(nLength);
    lpBuffer[0]				= FDE_TXTEDT_FORMATBLOCK_BGN;
    lpBuffer[nLength - 1]	= FDE_TXTEDT_FORMATBLOCK_END;
    FX_DWORD nAddress = (FX_DWORD)this;
    FXSYS_memcpy(lpBuffer + 1, &nAddress, sizeof(FX_DWORD));
    FXSYS_memcpy(lpBuffer + 3, FX_LPCWSTR(m_wsField), (nLength - 4) * sizeof(FX_WCHAR));
    wsDisplay.ReleaseBuffer(nLength);
}
FX_INT32 CFDE_TxtEdtField_Fixed::NormalizeCaretPos(FX_INT32 nIndex, FDE_FORMAT_CARET_DIRECTION eDirection ) const
{
    FXSYS_assert(nIndex >= 0 && nIndex <= m_nLength);
    if (eDirection == FDE_FORMAT_CARET_MIDDLE) {
        return (nIndex > m_wsField.GetLength() / 2) ? -1 : -2;
    }
    return eDirection == FDE_FORMAT_CARET_BACKWARD ? -2 : -1;
}
#endif
