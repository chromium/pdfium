// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../../include/ifde_txtedtbuf.h"
#include "../../include/ifde_txtedtengine.h"
#include "../../include/fx_wordbreak.h"
#include "fde_txtedtparag.h"
#include "fde_txtedtengine.h"
#include "fde_txtedtbuf.h"
CFDE_TxtEdtParag::CFDE_TxtEdtParag(CFDE_TxtEdtEngine * pEngine)
    : m_nLineCount(0)
    , m_nCharStart(0)
    , m_nCharCount(0)
    , m_lpData(NULL)
    , m_pEngine(pEngine)
{
    FXSYS_assert(m_pEngine);
}
CFDE_TxtEdtParag::~CFDE_TxtEdtParag()
{
    if (m_lpData != NULL) {
        FX_Free(m_lpData);
    }
}
void CFDE_TxtEdtParag::LoadParag()
{
    if (m_lpData != NULL) {
        ((FX_INT32*)m_lpData)[0]++;
        return;
    }
    IFX_TxtBreak * pTxtBreak	= m_pEngine->GetTextBreak();
    IFDE_TxtEdtBuf * pTxtBuf	= m_pEngine->GetTextBuf();
    const FDE_TXTEDTPARAMS *pParam	= m_pEngine->GetEditParams();
    FX_WCHAR wcAlias = 0;
    if (pParam->dwMode & FDE_TEXTEDITMODE_Password) {
        wcAlias = m_pEngine->GetAliasChar();
    }
    IFX_CharIter * pIter	= FX_NEW CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)pTxtBuf, wcAlias);
    pIter->SetAt(m_nCharStart);
    FX_INT32 nEndIndex = m_nCharStart + m_nCharCount;
    CFX_ArrayTemplate<FX_INT32> LineBaseArr;
    FX_BOOL		bReload			= FALSE;
    FX_DWORD	dwBreakStatus	= FX_TXTBREAK_None;
    FX_INT32 nTextEnd = m_pEngine->GetTextBufLength();
    do	{
        if (bReload) {
            dwBreakStatus = pTxtBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
        } else {
            FX_WCHAR wAppend = pIter->GetChar();
            dwBreakStatus = pTxtBreak->AppendChar(wAppend);
        }
        if (pIter->GetAt() + 1 == nEndIndex && dwBreakStatus < FX_TXTBREAK_LineBreak) {
            dwBreakStatus = pTxtBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
        }
        if (dwBreakStatus > FX_TXTBREAK_PieceBreak) {
            FX_INT32 nCount = pTxtBreak->CountBreakPieces();
            FX_INT32 nTotal = 0;
            for (FX_INT32 j = 0; j < nCount; j ++) {
                const CFX_TxtPiece * Piece = pTxtBreak->GetBreakPiece(j);
                nTotal += Piece->GetLength();
            }
            LineBaseArr.Add(nTotal);
            pTxtBreak->ClearBreakPieces();
        }
        if ((pIter->GetAt() + 1 == nEndIndex) && (dwBreakStatus == FX_TXTBREAK_LineBreak)) {
            bReload = TRUE;
            pIter->Next(TRUE);
        }
    } while (pIter->Next(FALSE) && (pIter->GetAt() < nEndIndex));
    pIter->Release();
    pTxtBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
    pTxtBreak->ClearBreakPieces();
    FX_INT32 nLineCount = LineBaseArr.GetSize();
    m_nLineCount = nLineCount;
    if (m_lpData == NULL) {
        m_lpData = FX_Alloc(FX_INT32, nLineCount + 1);
    } else {
        m_lpData = FX_Realloc(FX_INT32, m_lpData, (nLineCount + 1));
    }
    FX_INT32 * pIntArr = (FX_INT32*)m_lpData;
    pIntArr[0] = 1;
    m_nLineCount = nLineCount;
    pIntArr ++;
    for (FX_INT32 j = 0; j < nLineCount; j ++, pIntArr ++) {
        *pIntArr = LineBaseArr[j];
    }
    LineBaseArr.RemoveAll();
}
void CFDE_TxtEdtParag::UnloadParag()
{
    FXSYS_assert(m_lpData != NULL);
    ((FX_INT32*)m_lpData)[0]--;
    FXSYS_assert(((FX_INT32*)m_lpData)[0] >= 0);
    if (((FX_INT32*)m_lpData)[0] == 0) {
        FX_Free(m_lpData);
        m_lpData = NULL;
    }
}
void CFDE_TxtEdtParag::CalcLines()
{
    IFX_TxtBreak * pTxtBreak	= m_pEngine->GetTextBreak();
    IFDE_TxtEdtBuf * pTxtBuf	= m_pEngine->GetTextBuf();
    IFX_CharIter * pIter	= FX_NEW CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)pTxtBuf);
    FX_INT32 nCount = 0;
    FX_DWORD dwBreakStatus = FX_TXTBREAK_None;
    FX_INT32 nEndIndex = m_nCharStart + m_nCharCount;
    pIter->SetAt(m_nCharStart);
    FX_BOOL bReload = FALSE;
    FX_INT32 nTextEnd = m_pEngine->GetTextBufLength();
    do {
        if (bReload) {
            dwBreakStatus = pTxtBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
        } else {
            FX_WCHAR wAppend = pIter->GetChar();
            dwBreakStatus = pTxtBreak->AppendChar(wAppend);
        }
        if (pIter->GetAt() + 1 == nEndIndex && dwBreakStatus < FX_TXTBREAK_LineBreak) {
            dwBreakStatus = pTxtBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
        }
        if (dwBreakStatus > FX_TXTBREAK_PieceBreak) {
            nCount ++;
            pTxtBreak->ClearBreakPieces();
        }
        if ((pIter->GetAt() + 1 == nEndIndex) && (dwBreakStatus == FX_TXTBREAK_LineBreak)) {
            bReload = TRUE;
            pIter->Next(TRUE);
        }
    } while (pIter->Next(FALSE) && (pIter->GetAt() < nEndIndex));
    pIter->Release();
    pTxtBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
    pTxtBreak->ClearBreakPieces();
    m_nLineCount = nCount;
}
void CFDE_TxtEdtParag::GetLineRange(FX_INT32 nLineIndex, FX_INT32& nStart, FX_INT32& nCount) const
{
    FX_INT32 * pLineBaseArr = (FX_INT32*)m_lpData;
    FXSYS_assert(nLineIndex < m_nLineCount);
    nStart = m_nCharStart;
    pLineBaseArr ++;
    for (FX_INT32 i = 0; i < nLineIndex; i ++) {
        nStart += *pLineBaseArr;
        pLineBaseArr ++;
    }
    nCount = *pLineBaseArr;
}
