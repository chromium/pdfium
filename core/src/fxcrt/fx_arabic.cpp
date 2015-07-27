// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_ucd.h"
#include "fx_arabic.h"

extern const FX_DWORD gs_FX_TextLayout_CodeProperties[65536];
IFX_BidiChar* IFX_BidiChar::Create()
{
    return new CFX_BidiChar;
}
CFX_BidiChar::CFX_BidiChar()
    : m_bSeparateNeutral(TRUE)
    , m_iCurStart(0)
    , m_iCurCount(0)
    , m_iCurBidi(0)
    , m_iLastBidi(0)
    , m_iLastStart(0)
    , m_iLastCount(0)
{
}
void CFX_BidiChar::SetPolicy(FX_BOOL bSeparateNeutral)
{
    m_bSeparateNeutral = bSeparateNeutral;
}

FX_BOOL CFX_BidiChar::AppendChar(FX_WCHAR wch)
{
    FX_DWORD dwProps = gs_FX_TextLayout_CodeProperties[(FX_WORD)wch];
    int32_t iBidiCls = (dwProps & FX_BIDICLASSBITSMASK) >> FX_BIDICLASSBITS;
    int32_t iContext = 0;
    switch (iBidiCls) {
        case FX_BIDICLASS_L:
        case FX_BIDICLASS_AN:
        case FX_BIDICLASS_EN:
            iContext = 1;
            break;
        case FX_BIDICLASS_R:
        case FX_BIDICLASS_AL:
            iContext = 2;
            break;
    }
    FX_BOOL bRet = FALSE;
    if (iContext != m_iCurBidi) {
        if (m_bSeparateNeutral) {
            bRet = TRUE;
        } else {
            if (m_iCurBidi == 0) {
                bRet = (m_iCurCount > 0);
            } else {
                bRet = (iContext != 0);
            }
        }
        if (bRet) {
            m_iLastBidi = m_iCurBidi;
            m_iLastStart = m_iCurStart;
            m_iCurStart = m_iCurCount;
            m_iLastCount = m_iCurCount - m_iLastStart;
        }
        if (m_bSeparateNeutral || iContext != 0) {
            m_iCurBidi = iContext;
        }
    }
    m_iCurCount ++;
    return bRet;
}
FX_BOOL CFX_BidiChar::EndChar()
{
    m_iLastBidi = m_iCurBidi;
    m_iLastStart = m_iCurStart;
    m_iCurStart = m_iCurCount;
    m_iLastCount = m_iCurCount - m_iLastStart;
    return m_iLastCount > 0;
}
int32_t CFX_BidiChar::GetBidiInfo(int32_t &iStart, int32_t &iCount)
{
    iStart = m_iLastStart;
    iCount = m_iLastCount;
    return m_iLastBidi;
}
void CFX_BidiChar::Reset()
{
    m_iCurStart = 0;
    m_iCurCount = 0;
    m_iCurBidi = 0;
    m_iLastBidi = 0;
    m_iLastStart = 0;
    m_iLastCount = 0;
}
