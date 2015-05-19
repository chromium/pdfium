// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "pageint.h"
CPDF_PageObject* CPDF_PageObject::Create(int type)
{
    switch (type) {
        case PDFPAGE_TEXT:
            return new CPDF_TextObject;
        case PDFPAGE_IMAGE:
            return new CPDF_ImageObject;
        case PDFPAGE_PATH:
            return new CPDF_PathObject;
        case PDFPAGE_SHADING:
            return new CPDF_ShadingObject;
        case PDFPAGE_FORM:
            return new CPDF_FormObject;
    }
    return NULL;
}
CPDF_PageObject::~CPDF_PageObject()
{
}
CPDF_PageObject* CPDF_PageObject::Clone() const
{
    CPDF_PageObject* pObj = Create(m_Type);
    pObj->Copy(this);
    return pObj;
}
void CPDF_PageObject::Copy(const CPDF_PageObject* pSrc)
{
    if (m_Type != pSrc->m_Type) {
        return;
    }
    CopyData(pSrc);
    CopyStates(*pSrc);
    m_Left = pSrc->m_Left;
    m_Right = pSrc->m_Right;
    m_Top = pSrc->m_Top;
    m_Bottom = pSrc->m_Bottom;
}
void CPDF_PageObject::AppendClipPath(CPDF_Path path, int type, FX_BOOL bAutoMerge)
{
    m_ClipPath.AppendPath(path, type, bAutoMerge);
}
void CPDF_PageObject::CopyClipPath(CPDF_PageObject* pObj)
{
    m_ClipPath = pObj->m_ClipPath;
}
void CPDF_PageObject::RemoveClipPath()
{
    m_ClipPath.SetNull();
}
void CPDF_PageObject::RecalcBBox()
{
    switch (m_Type) {
        case PDFPAGE_TEXT:
            ((CPDF_TextObject*)this)->RecalcPositionData();
            break;
        case PDFPAGE_PATH:
            ((CPDF_PathObject*)this)->CalcBoundingBox();
            break;
        case PDFPAGE_SHADING:
            ((CPDF_ShadingObject*)this)->CalcBoundingBox();
            break;
    }
}
void CPDF_PageObject::TransformClipPath(CFX_AffineMatrix& matrix)
{
    if (m_ClipPath.IsNull()) {
        return;
    }
    m_ClipPath.GetModify();
    m_ClipPath.Transform(matrix);
}
void CPDF_PageObject::TransformGeneralState(CFX_AffineMatrix& matrix)
{
    if(m_GeneralState.IsNull()) {
        return;
    }
    CPDF_GeneralStateData* pGS = m_GeneralState.GetModify();
    pGS->m_Matrix.Concat(matrix);
}
FX_RECT CPDF_PageObject::GetBBox(const CFX_AffineMatrix* pMatrix) const
{
    CFX_FloatRect rect(m_Left, m_Bottom, m_Right, m_Top);
    if (pMatrix) {
        pMatrix->TransformRect(rect);
    }
    return rect.GetOutterRect();
}
CPDF_TextObject::CPDF_TextObject()
{
    m_Type = PDFPAGE_TEXT;
    m_pCharCodes = NULL;
    m_pCharPos = NULL;
    m_nChars = 0;
    m_PosX = m_PosY = 0;
}
CPDF_TextObject::~CPDF_TextObject()
{
    if (m_nChars > 1 && m_pCharCodes) {
        FX_Free(m_pCharCodes);
    }
    if (m_pCharPos) {
        FX_Free(m_pCharPos);
    }
}
void CPDF_TextObject::GetItemInfo(int index, CPDF_TextObjectItem* pInfo) const
{
    pInfo->m_CharCode = m_nChars == 1 ? (FX_DWORD)(FX_UINTPTR)m_pCharCodes : m_pCharCodes[index];
    pInfo->m_OriginX = index ? m_pCharPos[index - 1] : 0;
    pInfo->m_OriginY = 0;
    if (pInfo->m_CharCode == -1) {
        return;
    }
    CPDF_Font* pFont = m_TextState.GetFont();
    if (pFont->GetFontType() != PDFFONT_CIDFONT) {
        return;
    }
    if (!((CPDF_CIDFont*)pFont)->IsVertWriting()) {
        return;
    }
    FX_WORD CID = ((CPDF_CIDFont*)pFont)->CIDFromCharCode(pInfo->m_CharCode);
    pInfo->m_OriginY = pInfo->m_OriginX;
    pInfo->m_OriginX = 0;
    short vx, vy;
    ((CPDF_CIDFont*)pFont)->GetVertOrigin(CID, vx, vy);
    FX_FLOAT fontsize = m_TextState.GetFontSize();
    pInfo->m_OriginX -= fontsize * vx / 1000;
    pInfo->m_OriginY -= fontsize * vy / 1000;
}
int CPDF_TextObject::CountChars() const
{
    if (m_nChars == 1) {
        return 1;
    }
    int count = 0;
    for (int i = 0; i < m_nChars; i ++)
        if (m_pCharCodes[i] != (FX_DWORD) - 1) {
            count ++;
        }
    return count;
}
void CPDF_TextObject::GetCharInfo(int index, FX_DWORD& charcode, FX_FLOAT& kerning) const
{
    if (m_nChars == 1) {
        charcode = (FX_DWORD)(FX_UINTPTR)m_pCharCodes;
        kerning = 0;
        return;
    }
    int count = 0;
    for (int i = 0; i < m_nChars; i ++) {
        if (m_pCharCodes[i] != (FX_DWORD) - 1) {
            if (count == index) {
                charcode = m_pCharCodes[i];
                if (i == m_nChars - 1 || m_pCharCodes[i + 1] != (FX_DWORD) - 1) {
                    kerning = 0;
                } else {
                    kerning = m_pCharPos[i];
                }
                return;
            }
            count ++;
        }
    }
}
void CPDF_TextObject::GetCharInfo(int index, CPDF_TextObjectItem* pInfo) const
{
    if (m_nChars == 1) {
        GetItemInfo(0, pInfo);
        return;
    }
    int count = 0;
    for (int i = 0; i < m_nChars; i ++) {
        FX_DWORD charcode = m_pCharCodes[i];
        if (charcode == (FX_DWORD) - 1) {
            continue;
        }
        if (count == index) {
            GetItemInfo(i, pInfo);
            break;
        }
        count ++;
    }
}
void CPDF_TextObject::CopyData(const CPDF_PageObject* pSrc)
{
    const CPDF_TextObject* pSrcObj = (const CPDF_TextObject*)pSrc;
    if (m_nChars > 1 && m_pCharCodes) {
        FX_Free(m_pCharCodes);
        m_pCharCodes = NULL;
    }
    if (m_pCharPos) {
        FX_Free(m_pCharPos);
        m_pCharPos = NULL;
    }
    m_nChars = pSrcObj->m_nChars;
    if (m_nChars > 1) {
        m_pCharCodes = FX_Alloc(FX_DWORD, m_nChars);
        m_pCharPos = FX_Alloc(FX_FLOAT, m_nChars - 1);
        for (int i = 0; i < m_nChars; i ++) {
            m_pCharCodes[i] = pSrcObj->m_pCharCodes[i];
        }
        for (int i = 0; i < m_nChars - 1; i ++) {
            m_pCharPos[i] = pSrcObj->m_pCharPos[i];
        }
    } else {
        m_pCharCodes = pSrcObj->m_pCharCodes;
    }
    m_PosX = pSrcObj->m_PosX;
    m_PosY = pSrcObj->m_PosY;
}
void CPDF_TextObject::GetTextMatrix(CFX_AffineMatrix* pMatrix) const
{
    FX_FLOAT* pTextMatrix = m_TextState.GetMatrix();
    pMatrix->Set(pTextMatrix[0], pTextMatrix[2], pTextMatrix[1], pTextMatrix[3], m_PosX, m_PosY);
}
void CPDF_TextObject::SetSegments(const CFX_ByteString* pStrs, FX_FLOAT* pKerning, int nsegs)
{
    if (m_nChars > 1 && m_pCharCodes) {
        FX_Free(m_pCharCodes);
        m_pCharCodes = NULL;
    }
    if (m_pCharPos) {
        FX_Free(m_pCharPos);
        m_pCharPos = NULL;
    }
    CPDF_Font* pFont = m_TextState.GetFont();
    m_nChars = 0;
    for (int i = 0; i < nsegs; i ++) {
        m_nChars += pFont->CountChar(pStrs[i], pStrs[i].GetLength());
    }
    m_nChars += nsegs - 1;
    if (m_nChars > 1) {
        m_pCharCodes = FX_Alloc(FX_DWORD, m_nChars);
        m_pCharPos = FX_Alloc(FX_FLOAT, m_nChars - 1);
        int index = 0;
        for (int i = 0; i < nsegs; i ++) {
            FX_LPCSTR segment = pStrs[i];
            int offset = 0, len = pStrs[i].GetLength();
            while (offset < len) {
                m_pCharCodes[index++] = pFont->GetNextChar(segment, len, offset);
            }
            if (i != nsegs - 1) {
                m_pCharPos[index - 1] = pKerning[i];
                m_pCharCodes[index ++] = (FX_DWORD) - 1;
            }
        }
    } else {
        int offset = 0;
        m_pCharCodes = (FX_DWORD*)(FX_UINTPTR)pFont->GetNextChar(pStrs[0], pStrs[0].GetLength(), offset);
    }
}
void CPDF_TextObject::SetText(const CFX_ByteString& str)
{
    SetSegments(&str, NULL, 1);
    RecalcPositionData();
}
void CPDF_TextObject::SetEmpty()
{
    if (m_nChars > 1 && m_pCharCodes) {
        FX_Free(m_pCharCodes);
    }
    if (m_nChars > 1 && m_pCharPos) {
        FX_Free(m_pCharPos);
    }
    m_nChars = 0;
    m_pCharCodes = NULL;
    m_pCharPos = NULL;
    m_Left = m_Right = m_PosX;
    m_Top = m_Bottom = m_PosY;
}
void CPDF_TextObject::SetText(CFX_ByteString* pStrs, FX_FLOAT* pKerning, int nSegs)
{
    SetSegments(pStrs, pKerning, nSegs);
    RecalcPositionData();
}
void CPDF_TextObject::SetText(int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pKernings)
{
    if (m_nChars > 1 && m_pCharCodes) {
        FX_Free(m_pCharCodes);
        m_pCharCodes = NULL;
    }
    if (m_pCharPos) {
        FX_Free(m_pCharPos);
        m_pCharPos = NULL;
    }
    int nKernings = 0;
    int i;
    for (i = 0; i < nChars - 1; i ++)
        if (pKernings[i] != 0) {
            nKernings ++;
        }
    m_nChars = nChars + nKernings;
    if (m_nChars > 1) {
        m_pCharCodes = FX_Alloc(FX_DWORD, m_nChars);
        m_pCharPos = FX_Alloc(FX_FLOAT, m_nChars - 1);
        int index = 0;
        for (int i = 0; i < nChars; i ++) {
            m_pCharCodes[index++] = pCharCodes[i];
            if (pKernings[i] != 0 && i != nChars - 1) {
                m_pCharCodes[index] = (FX_DWORD) - 1;
                m_pCharPos[index - 1] = pKernings[i];
                index ++;
            }
        }
    } else {
        m_pCharCodes = (FX_DWORD*)(FX_UINTPTR)pCharCodes[0];
    }
    RecalcPositionData();
}
FX_FLOAT CPDF_TextObject::GetCharWidth(FX_DWORD charcode) const
{
    FX_FLOAT fontsize = m_TextState.GetFontSize() / 1000;
    CPDF_Font* pFont = m_TextState.GetFont();
    FX_BOOL bVertWriting = FALSE;
    CPDF_CIDFont* pCIDFont = pFont->GetCIDFont();
    if (pCIDFont) {
        bVertWriting = pCIDFont->IsVertWriting();
    }
    if (!bVertWriting) {
        return pFont->GetCharWidthF(charcode, 0) * fontsize;
    } else {
        FX_WORD CID = pCIDFont->CIDFromCharCode(charcode);
        return pCIDFont->GetVertWidth(CID) * fontsize;
    }
}
FX_FLOAT CPDF_TextObject::GetSpaceCharWidth() const
{
    CPDF_Font* pFont = m_TextState.GetFont();
    FX_DWORD charCode = m_TextState.GetFont()->CharCodeFromUnicode(32);
    if (charCode != (FX_DWORD) - 1) {
        return GetCharWidth(charCode);
    }
    FX_FLOAT fontSize = m_TextState.GetFontSize() / 4000.0f;
    FX_BOOL bVertWriting = FALSE;
    CPDF_CIDFont* pCIDFont = pFont->GetCIDFont();
    if (pCIDFont) {
        bVertWriting = pCIDFont->IsVertWriting();
    }
    FX_RECT fontRect;
    pFont->GetFontBBox(fontRect);
    fontSize *= bVertWriting ? (FX_FLOAT)fontRect.Height() : (FX_FLOAT)fontRect.Width();
    return fontSize;
}
void CPDF_TextObject::GetCharRect(int index, CFX_FloatRect& rect) const
{
    CPDF_Font* pFont = m_TextState.GetFont();
    FX_BOOL bVertWriting = FALSE;
    CPDF_CIDFont* pCIDFont = pFont->GetCIDFont();
    if (pCIDFont) {
        bVertWriting = pCIDFont->IsVertWriting();
    }
    FX_FLOAT fontsize = m_TextState.GetFontSize() / 1000;
    int count = 0;
    for (int i = 0; i < m_nChars; i ++) {
        FX_DWORD charcode = m_nChars == 1 ? (FX_DWORD)(FX_UINTPTR)m_pCharCodes : m_pCharCodes[i];
        if (charcode == (FX_DWORD) - 1) {
            continue;
        }
        if( count != index) {
            count++;
            continue;
        }
        FX_FLOAT curpos = i > 0 ? m_pCharPos[i - 1] : 0;
        FX_RECT char_rect;
        pFont->GetCharBBox(charcode, char_rect, 0);
        if (!bVertWriting) {
            rect.left = curpos + char_rect.left * fontsize;
            rect.right = curpos + char_rect.right * fontsize;
            rect.top = char_rect.top * fontsize;
            rect.bottom = char_rect.bottom * fontsize;
        } else {
            FX_WORD CID = pCIDFont->CIDFromCharCode(charcode);
            short vx, vy;
            pCIDFont->GetVertOrigin(CID, vx, vy);
            char_rect.left -= vx;
            char_rect.right -= vx;
            char_rect.top -= vy;
            char_rect.bottom -= vy;
            rect.left = char_rect.left * fontsize;
            rect.right = char_rect.right * fontsize;
            rect.top = curpos + char_rect.top * fontsize;
            rect.bottom = curpos + char_rect.bottom * fontsize;
        }
        return;
    }
}
void CPDF_TextObject::CalcPositionData(FX_FLOAT* pTextAdvanceX, FX_FLOAT* pTextAdvanceY, FX_FLOAT horz_scale, int level)
{
    FX_FLOAT curpos = 0;
    FX_FLOAT min_x = 10000 * 1.0f, max_x = -10000 * 1.0f, min_y = 10000 * 1.0f, max_y = -10000 * 1.0f;
    CPDF_Font* pFont = m_TextState.GetFont();
    FX_BOOL bVertWriting = FALSE;
    CPDF_CIDFont* pCIDFont = pFont->GetCIDFont();
    if (pCIDFont) {
        bVertWriting = pCIDFont->IsVertWriting();
    }
    FX_FLOAT fontsize = m_TextState.GetFontSize();
    for (int i = 0; i < m_nChars; i ++) {
        FX_DWORD charcode = m_nChars == 1 ? (FX_DWORD)(FX_UINTPTR)m_pCharCodes : m_pCharCodes[i];
        if (charcode == (FX_DWORD) - 1) {
            curpos -= FXSYS_Mul(m_pCharPos[i - 1], fontsize) / 1000;
            continue;
        }
        if (i) {
            m_pCharPos[i - 1] = curpos;
        }
        FX_RECT char_rect;
        pFont->GetCharBBox(charcode, char_rect, level);
        FX_FLOAT charwidth;
        if (!bVertWriting) {
            if (min_y > char_rect.top) {
                min_y = (FX_FLOAT)char_rect.top;
            }
            if (max_y < char_rect.top) {
                max_y = (FX_FLOAT)char_rect.top;
            }
            if (min_y > char_rect.bottom) {
                min_y = (FX_FLOAT)char_rect.bottom;
            }
            if (max_y < char_rect.bottom) {
                max_y = (FX_FLOAT)char_rect.bottom;
            }
            FX_FLOAT char_left = curpos + char_rect.left * fontsize / 1000;
            FX_FLOAT char_right = curpos + char_rect.right * fontsize / 1000;
            if (min_x > char_left) {
                min_x = char_left;
            }
            if (max_x < char_left) {
                max_x = char_left;
            }
            if (min_x > char_right) {
                min_x = char_right;
            }
            if (max_x < char_right) {
                max_x = char_right;
            }
            charwidth = pFont->GetCharWidthF(charcode, level) * fontsize / 1000;
        } else {
            FX_WORD CID = pCIDFont->CIDFromCharCode(charcode);
            short vx, vy;
            pCIDFont->GetVertOrigin(CID, vx, vy);
            char_rect.left -= vx;
            char_rect.right -= vx;
            char_rect.top -= vy;
            char_rect.bottom -= vy;
            if (min_x > char_rect.left) {
                min_x = (FX_FLOAT)char_rect.left;
            }
            if (max_x < char_rect.left) {
                max_x = (FX_FLOAT)char_rect.left;
            }
            if (min_x > char_rect.right) {
                min_x = (FX_FLOAT)char_rect.right;
            }
            if (max_x < char_rect.right) {
                max_x = (FX_FLOAT)char_rect.right;
            }
            FX_FLOAT char_top = curpos + char_rect.top * fontsize / 1000;
            FX_FLOAT char_bottom = curpos + char_rect.bottom * fontsize / 1000;
            if (min_y > char_top) {
                min_y = char_top;
            }
            if (max_y < char_top) {
                max_y = char_top;
            }
            if (min_y > char_bottom) {
                min_y = char_bottom;
            }
            if (max_y < char_bottom) {
                max_y = char_bottom;
            }
            charwidth = pCIDFont->GetVertWidth(CID) * fontsize / 1000;
        }
        curpos += charwidth;
        if (charcode == ' ' && (pCIDFont == NULL || pCIDFont->GetCharSize(32) == 1)) {
            curpos += m_TextState.GetObject()->m_WordSpace;
        }
        curpos += m_TextState.GetObject()->m_CharSpace;
    }
    if (bVertWriting) {
        if (pTextAdvanceX) {
            *pTextAdvanceX = 0;
        }
        if (pTextAdvanceY) {
            *pTextAdvanceY = curpos;
        }
        min_x = min_x * fontsize / 1000;
        max_x = max_x * fontsize / 1000;
    } else {
        if (pTextAdvanceX) {
            *pTextAdvanceX = FXSYS_Mul(curpos, horz_scale);
        }
        if (pTextAdvanceY) {
            *pTextAdvanceY = 0;
        }
        min_y = min_y * fontsize / 1000;
        max_y = max_y * fontsize / 1000;
    }
    CFX_AffineMatrix matrix;
    GetTextMatrix(&matrix);
    m_Left = min_x;
    m_Right = max_x;
    m_Bottom = min_y;
    m_Top = max_y;
    matrix.TransformRect(m_Left, m_Right, m_Top, m_Bottom);
    int textmode = m_TextState.GetObject()->m_TextMode;
    if (textmode == 1 || textmode == 2 || textmode == 5 || textmode == 6) {
        FX_FLOAT half_width = m_GraphState.GetObject()->m_LineWidth / 2;
        m_Left -= half_width;
        m_Right += half_width;
        m_Top += half_width;
        m_Bottom -= half_width;
    }
}
void CPDF_TextObject::CalcCharPos(FX_FLOAT* pPosArray) const
{
    CPDF_Font* pFont = m_TextState.GetFont();
    FX_BOOL bVertWriting = FALSE;
    CPDF_CIDFont* pCIDFont = pFont->GetCIDFont();
    if (pCIDFont) {
        bVertWriting = pCIDFont->IsVertWriting();
    }
    FX_FLOAT fontsize = m_TextState.GetFontSize();
    int index = 0;
    for (int i = 0; i < m_nChars; i ++) {
        FX_DWORD charcode = m_nChars == 1 ? (FX_DWORD)(FX_UINTPTR)m_pCharCodes : m_pCharCodes[i];
        if (charcode == (FX_DWORD) - 1) {
            continue;
        }
        pPosArray[index++] = i ? m_pCharPos[i - 1] : 0;
        FX_FLOAT charwidth;
        if (bVertWriting) {
            FX_WORD CID = pCIDFont->CIDFromCharCode(charcode);
            charwidth = pCIDFont->GetVertWidth(CID) * fontsize / 1000;
        } else {
            charwidth = pFont->GetCharWidthF(charcode) * fontsize / 1000;
        }
        pPosArray[index] = pPosArray[index - 1] + charwidth;
        index++;
    }
}
void CPDF_TextObject::Transform(const CFX_AffineMatrix& matrix)
{
    m_TextState.GetModify();
    CFX_AffineMatrix text_matrix;
    GetTextMatrix(&text_matrix);
    text_matrix.Concat(matrix);
    FX_FLOAT* pTextMatrix = m_TextState.GetMatrix();
    pTextMatrix[0] = text_matrix.GetA();
    pTextMatrix[1] = text_matrix.GetC();
    pTextMatrix[2] = text_matrix.GetB();
    pTextMatrix[3] = text_matrix.GetD();
    m_PosX = text_matrix.GetE();
    m_PosY = text_matrix.GetF();
    CalcPositionData(NULL, NULL, 0);
}
void CPDF_TextObject::SetPosition(FX_FLOAT x, FX_FLOAT y)
{
    FX_FLOAT dx = x - m_PosX;
    FX_FLOAT dy = y - m_PosY;
    m_PosX = x;
    m_PosY = y;
    m_Left += dx;
    m_Right += dx;
    m_Top += dy;
    m_Bottom += dy;
}
void CPDF_TextObject::SetData(int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos, FX_FLOAT x, FX_FLOAT y)
{
    ASSERT(m_nChars == 0);
    m_nChars = nChars;
    m_PosX = x;
    m_PosY = y;
    if (nChars == 0) {
        return;
    }
    if (nChars == 1) {
        m_pCharCodes = (FX_DWORD*)(FX_UINTPTR) * pCharCodes;
    } else {
        m_pCharCodes = FX_Alloc(FX_DWORD, nChars);
        FXSYS_memcpy32(m_pCharCodes, pCharCodes, sizeof(FX_DWORD)*nChars);
        m_pCharPos = FX_Alloc(FX_FLOAT, nChars - 1);
        FXSYS_memcpy32(m_pCharPos, pCharPos, sizeof(FX_FLOAT) * (nChars - 1));
    }
    RecalcPositionData();
}
void CPDF_TextObject::SetTextState(CPDF_TextState TextState)
{
    m_TextState = TextState;
    CalcPositionData(NULL, NULL, 0);
}
CPDF_ShadingObject::CPDF_ShadingObject()
{
    m_pShading = NULL;
    m_Type = PDFPAGE_SHADING;
}
CPDF_ShadingObject::~CPDF_ShadingObject()
{
}
void CPDF_ShadingObject::CopyData(const CPDF_PageObject* pSrc)
{
    CPDF_ShadingObject* pSrcObj = (CPDF_ShadingObject*)pSrc;
    m_pShading = pSrcObj->m_pShading;
    if (m_pShading && m_pShading->m_pDocument) {
        CPDF_DocPageData* pDocPageData = m_pShading->m_pDocument->GetPageData();
        m_pShading = (CPDF_ShadingPattern*)pDocPageData->GetPattern(m_pShading->m_pShadingObj, m_pShading->m_bShadingObj, &m_pShading->m_ParentMatrix);
    }
    m_Matrix = pSrcObj->m_Matrix;
}
void CPDF_ShadingObject::Transform(const CFX_AffineMatrix& matrix)
{
    if (!m_ClipPath.IsNull()) {
        m_ClipPath.GetModify();
        m_ClipPath.Transform(matrix);
    }
    m_Matrix.Concat(matrix);
    if (!m_ClipPath.IsNull()) {
        CalcBoundingBox();
    } else {
        matrix.TransformRect(m_Left, m_Right, m_Top, m_Bottom);
    }
}
void CPDF_ShadingObject::CalcBoundingBox()
{
    if (m_ClipPath.IsNull()) {
        return;
    }
    CFX_FloatRect rect = m_ClipPath.GetClipBox();
    m_Left = rect.left;
    m_Bottom = rect.bottom;
    m_Right = rect.right;
    m_Top = rect.top;
}
CPDF_FormObject::~CPDF_FormObject()
{
    if (m_pForm) {
        delete m_pForm;
    }
}
void CPDF_FormObject::Transform(const CFX_AffineMatrix& matrix)
{
    m_FormMatrix.Concat(matrix);
    CalcBoundingBox();
}
void CPDF_FormObject::CopyData(const CPDF_PageObject* pSrc)
{
    const CPDF_FormObject* pSrcObj = (const CPDF_FormObject*)pSrc;
    if (m_pForm) {
        delete m_pForm;
    }
    m_pForm = pSrcObj->m_pForm->Clone();
    m_FormMatrix = pSrcObj->m_FormMatrix;
}
void CPDF_FormObject::CalcBoundingBox()
{
    CFX_FloatRect form_rect = m_pForm->CalcBoundingBox();
    form_rect.Transform(&m_FormMatrix);
    m_Left = form_rect.left;
    m_Bottom = form_rect.bottom;
    m_Right = form_rect.right;
    m_Top = form_rect.top;
}
CPDF_PageObjects::CPDF_PageObjects(FX_BOOL bReleaseMembers) : m_ObjectList(128)
{
    m_bBackgroundAlphaNeeded = FALSE;
    m_bReleaseMembers = bReleaseMembers;
    m_ParseState = PDF_CONTENT_NOT_PARSED;
    m_pParser = NULL;
    m_pFormStream = NULL;
    m_pResources = NULL;
}
CPDF_PageObjects::~CPDF_PageObjects()
{
    if (m_pParser) {
        delete m_pParser;
    }
    if (!m_bReleaseMembers) {
        return;
    }
    FX_POSITION pos = m_ObjectList.GetHeadPosition();
    while (pos) {
        delete (CPDF_PageObject*)m_ObjectList.GetNext(pos);
    }
}
void CPDF_PageObjects::ContinueParse(IFX_Pause* pPause)
{
    if (m_pParser == NULL) {
        return;
    }
    m_pParser->Continue(pPause);
    if (m_pParser->GetStatus() == CPDF_ContentParser::Done) {
        m_ParseState = PDF_CONTENT_PARSED;
        delete m_pParser;
        m_pParser = NULL;
    }
}
int CPDF_PageObjects::EstimateParseProgress() const
{
    if (m_pParser == NULL) {
        return m_ParseState == PDF_CONTENT_PARSED ? 100 : 0;
    }
    return m_pParser->EstimateProgress();
}
FX_POSITION CPDF_PageObjects::InsertObject(FX_POSITION posInsertAfter, CPDF_PageObject* pNewObject)
{
    if (posInsertAfter == NULL) {
        return m_ObjectList.AddHead(pNewObject);
    } else {
        return m_ObjectList.InsertAfter(posInsertAfter, pNewObject);
    }
}
int CPDF_PageObjects::GetObjectIndex(CPDF_PageObject* pObj) const
{
    int index = 0;
    FX_POSITION pos = m_ObjectList.GetHeadPosition();
    while (pos) {
        CPDF_PageObject* pThisObj = (CPDF_PageObject*)m_ObjectList.GetNext(pos);
        if (pThisObj == pObj) {
            return index;
        }
        index ++;
    }
    return -1;
}
CPDF_PageObject* CPDF_PageObjects::GetObjectByIndex(int index) const
{
    FX_POSITION pos = m_ObjectList.FindIndex(index);
    if (pos == NULL) {
        return NULL;
    }
    return (CPDF_PageObject*)m_ObjectList.GetAt(pos);
}
void CPDF_PageObjects::Transform(const CFX_AffineMatrix& matrix)
{
    FX_POSITION pos = m_ObjectList.GetHeadPosition();
    while (pos) {
        CPDF_PageObject* pObj = (CPDF_PageObject*)m_ObjectList.GetNext(pos);
        pObj->Transform(matrix);
    }
}
CFX_FloatRect CPDF_PageObjects::CalcBoundingBox() const
{
    if (m_ObjectList.GetCount() == 0) {
        return CFX_FloatRect(0, 0, 0, 0);
    }
    FX_FLOAT left, right, top, bottom;
    left = bottom = 1000000 * 1.0f;
    right = top = -1000000 * 1.0f;
    FX_POSITION pos = m_ObjectList.GetHeadPosition();
    while (pos) {
        CPDF_PageObject* pObj = (CPDF_PageObject*)m_ObjectList.GetNext(pos);
        if (left > pObj->m_Left) {
            left = pObj->m_Left;
        }
        if (right < pObj->m_Right) {
            right = pObj->m_Right;
        }
        if (top < pObj->m_Top) {
            top = pObj->m_Top;
        }
        if (bottom > pObj->m_Bottom) {
            bottom = pObj->m_Bottom;
        }
    }
    return CFX_FloatRect(left, bottom, right, top);
}
void CPDF_PageObjects::LoadTransInfo()
{
    if (m_pFormDict == NULL) {
        return;
    }
    CPDF_Dictionary* pGroup = m_pFormDict->GetDict(FX_BSTRC("Group"));
    if (pGroup == NULL) {
        return;
    }
    if (pGroup->GetString(FX_BSTRC("S")) != FX_BSTRC("Transparency")) {
        return;
    }
    m_Transparency |= PDFTRANS_GROUP;
    if (pGroup->GetInteger(FX_BSTRC("I"))) {
        m_Transparency |= PDFTRANS_ISOLATED;
    }
    if (pGroup->GetInteger(FX_BSTRC("K"))) {
        m_Transparency |= PDFTRANS_KNOCKOUT;
    }
}
void CPDF_PageObjects::ClearCacheObjects()
{
    m_ParseState = PDF_CONTENT_NOT_PARSED;
    if (m_pParser) {
        delete m_pParser;
    }
    m_pParser = NULL;
    if (m_bReleaseMembers) {
        FX_POSITION pos = m_ObjectList.GetHeadPosition();
        while (pos) {
            delete (CPDF_PageObject*)m_ObjectList.GetNext(pos);
        }
    }
    m_ObjectList.RemoveAll();
}
CPDF_Page::CPDF_Page()
{
    m_pPageRender = NULL;
}
void CPDF_Page::Load(CPDF_Document* pDocument, CPDF_Dictionary* pPageDict, FX_BOOL bPageCache)
{
    m_pDocument = (CPDF_Document*)pDocument;
    m_pFormDict = pPageDict;
    if (bPageCache) {
        m_pPageRender = CPDF_ModuleMgr::Get()->GetRenderModule()->CreatePageCache(this);
    }
    if (pPageDict == NULL) {
        m_PageWidth = m_PageHeight = 100 * 1.0f;
        m_pPageResources = m_pResources = NULL;
        return;
    }
    CPDF_Object* pageAttr = GetPageAttr(FX_BSTRC("Resources"));
    m_pResources = pageAttr ? pageAttr->GetDict() : NULL;
    m_pPageResources = m_pResources;
    CPDF_Object* pRotate = GetPageAttr(FX_BSTRC("Rotate"));
    int rotate = 0;
    if (pRotate) {
        rotate = pRotate->GetInteger() / 90 % 4;
    }
    if (rotate < 0) {
        rotate += 4;
    }
    CPDF_Array* pMediaBox, *pCropBox;
    pMediaBox = (CPDF_Array*)GetPageAttr(FX_BSTRC("MediaBox"));
    CFX_FloatRect mediabox;
    if (pMediaBox) {
        mediabox = pMediaBox->GetRect();
        mediabox.Normalize();
    }
    if (mediabox.IsEmpty()) {
        mediabox = CFX_FloatRect(0, 0, 612, 792);
    }
    pCropBox = (CPDF_Array*)GetPageAttr(FX_BSTRC("CropBox"));
    if (pCropBox) {
        m_BBox = pCropBox->GetRect();
        m_BBox.Normalize();
    }
    if (m_BBox.IsEmpty()) {
        m_BBox = mediabox;
    } else {
        m_BBox.Intersect(mediabox);
    }
    if (rotate % 2) {
        m_PageHeight = m_BBox.right - m_BBox.left;
        m_PageWidth = m_BBox.top - m_BBox.bottom;
    } else {
        m_PageWidth = m_BBox.right - m_BBox.left;
        m_PageHeight = m_BBox.top - m_BBox.bottom;
    }
    switch (rotate) {
        case 0:
            m_PageMatrix.Set(1.0f, 0, 0, 1.0f, -m_BBox.left, -m_BBox.bottom);
            break;
        case 1:
            m_PageMatrix.Set(0, -1.0f, 1.0f, 0, -m_BBox.bottom, m_BBox.right);
            break;
        case 2:
            m_PageMatrix.Set(-1.0f, 0, 0, -1.0f, m_BBox.right, m_BBox.top);
            break;
        case 3:
            m_PageMatrix.Set(0, 1.0f, -1.0f, 0, m_BBox.top, -m_BBox.left);
            break;
    }
    m_Transparency = PDFTRANS_ISOLATED;
    LoadTransInfo();
}
void CPDF_Page::StartParse(CPDF_ParseOptions* pOptions, FX_BOOL bReParse)
{
    if (bReParse) {
        ClearCacheObjects();
    }
    if (m_ParseState == PDF_CONTENT_PARSED || m_ParseState == PDF_CONTENT_PARSING) {
        return;
    }
    m_pParser = new CPDF_ContentParser;
    m_pParser->Start(this, pOptions);
    m_ParseState = PDF_CONTENT_PARSING;
}
void CPDF_Page::ParseContent(CPDF_ParseOptions* pOptions, FX_BOOL bReParse)
{
    StartParse(pOptions, bReParse);
    ContinueParse(NULL);
}
CPDF_Page::~CPDF_Page()
{
    if (m_pPageRender) {
        CPDF_RenderModuleDef* pModule = CPDF_ModuleMgr::Get()->GetRenderModule();
        pModule->DestroyPageCache(m_pPageRender);
    }
}
CPDF_Object* FPDFAPI_GetPageAttr(CPDF_Dictionary* pPageDict, FX_BSTR name)
{
    int level = 0;
    while (1) {
        CPDF_Object* pObj = pPageDict->GetElementValue(name);
        if (pObj) {
            return pObj;
        }
        CPDF_Dictionary* pParent = pPageDict->GetDict(FX_BSTRC("Parent"));
        if (!pParent || pParent == pPageDict) {
            return NULL;
        }
        pPageDict = pParent;
        level ++;
        if (level == 1000) {
            return NULL;
        }
    }
}
CPDF_Object* CPDF_Page::GetPageAttr(FX_BSTR name) const
{
    return FPDFAPI_GetPageAttr(m_pFormDict, name);
}
CPDF_Form::CPDF_Form(CPDF_Document* pDoc, CPDF_Dictionary* pPageResources, CPDF_Stream* pFormStream, CPDF_Dictionary* pParentResources)
{
    m_pDocument = pDoc;
    m_pFormStream = pFormStream;
    m_pFormDict = pFormStream ? pFormStream->GetDict() : NULL;
    m_pResources = m_pFormDict->GetDict(FX_BSTRC("Resources"));
    m_pPageResources = pPageResources;
    if (m_pResources == NULL) {
        m_pResources = pParentResources;
    }
    if (m_pResources == NULL) {
        m_pResources = pPageResources;
    }
    m_Transparency = 0;
    LoadTransInfo();
}
CPDF_Form::~CPDF_Form()
{
}
void CPDF_Form::StartParse(CPDF_AllStates* pGraphicStates, CFX_AffineMatrix* pParentMatrix,
                           CPDF_Type3Char* pType3Char, CPDF_ParseOptions* pOptions, int level)
{
    if (m_ParseState == PDF_CONTENT_PARSED || m_ParseState == PDF_CONTENT_PARSING) {
        return;
    }
    m_pParser = new CPDF_ContentParser;
    m_pParser->Start(this, pGraphicStates, pParentMatrix, pType3Char, pOptions, level);
    m_ParseState = PDF_CONTENT_PARSING;
}
void CPDF_Form::ParseContent(CPDF_AllStates* pGraphicStates, CFX_AffineMatrix* pParentMatrix,
                             CPDF_Type3Char* pType3Char, CPDF_ParseOptions* pOptions, int level)
{
    StartParse(pGraphicStates, pParentMatrix, pType3Char, pOptions, level);
    ContinueParse(NULL);
}
CPDF_Form* CPDF_Form::Clone() const
{
    CPDF_Form* pClone = new CPDF_Form(m_pDocument, m_pPageResources, m_pFormStream, m_pResources);
    FX_POSITION pos = m_ObjectList.GetHeadPosition();
    while (pos) {
        CPDF_PageObject* pObj = (CPDF_PageObject*)m_ObjectList.GetNext(pos);
        pClone->m_ObjectList.AddTail(pObj->Clone());
    }
    return pClone;
}
void CPDF_Page::GetDisplayMatrix(CFX_AffineMatrix& matrix, int xPos, int yPos,
                                 int xSize, int ySize, int iRotate) const
{
    if (m_PageWidth == 0 || m_PageHeight == 0) {
        return;
    }
    CFX_AffineMatrix display_matrix;
    int x0, y0, x1, y1, x2, y2;
    iRotate %= 4;
    switch (iRotate) {
        case 0:
            x0 = xPos;
            y0 = yPos + ySize;
            x1 = xPos;
            y1 = yPos;
            x2 = xPos + xSize;
            y2 = yPos + ySize;
            break;
        case 1:
            x0 = xPos;
            y0 = yPos;
            x1 = xPos + xSize;
            y1 = yPos;
            x2 = xPos;
            y2 = yPos + ySize;
            break;
        case 2:
            x0 = xPos + xSize;
            y0 = yPos;
            x1 = xPos + xSize;
            y1 = yPos + ySize;
            x2 = xPos;
            y2 = yPos;
            break;
        case 3:
            x0 = xPos + xSize;
            y0 = yPos + ySize;
            x1 = xPos;
            y1 = yPos + ySize;
            x2 = xPos + xSize;
            y2 = yPos;
            break;
    }
    display_matrix.Set(FXSYS_Div((FX_FLOAT)(x2 - x0), m_PageWidth),
                       FXSYS_Div((FX_FLOAT)(y2 - y0), m_PageWidth),
                       FXSYS_Div((FX_FLOAT)(x1 - x0), m_PageHeight),
                       FXSYS_Div((FX_FLOAT)(y1 - y0), m_PageHeight),
                       (FX_FLOAT)x0, (FX_FLOAT)y0);
    matrix = m_PageMatrix;
    matrix.Concat(display_matrix);
}
CPDF_ParseOptions::CPDF_ParseOptions()
{
    m_bTextOnly = FALSE;
    m_bMarkedContent = TRUE;
    m_bSeparateForm = TRUE;
    m_bDecodeInlineImage = FALSE;
}
