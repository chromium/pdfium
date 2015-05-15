// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_render.h"
#include "pageint.h"
#include "../fpdf_render/render_int.h"
void CPDF_GraphicStates::DefaultStates()
{
    m_ColorState.New()->Default();
}
void CPDF_GraphicStates::CopyStates(const CPDF_GraphicStates& src)
{
    m_ClipPath = src.m_ClipPath;
    m_GraphState = src.m_GraphState;
    m_ColorState = src.m_ColorState;
    m_TextState = src.m_TextState;
    m_GeneralState = src.m_GeneralState;
}
CPDF_ClipPathData::CPDF_ClipPathData()
{
    m_PathCount = 0;
    m_pPathList = NULL;
    m_pTypeList = NULL;
    m_TextCount = 0;
    m_pTextList = NULL;
}
CPDF_ClipPathData::~CPDF_ClipPathData()
{
    int i;
    delete[] m_pPathList;
    if (m_pTypeList) {
        FX_Free(m_pTypeList);
    }
    for (i = m_TextCount - 1; i > -1; i --)
        if (m_pTextList[i]) {
            delete m_pTextList[i];
        }
    if (m_pTextList) {
        FX_Free(m_pTextList);
    }
}
CPDF_ClipPathData::CPDF_ClipPathData(const CPDF_ClipPathData& src)
{
    m_pPathList = NULL;
    m_pPathList = NULL;
    m_pTextList = NULL;
    m_PathCount = src.m_PathCount;
    if (m_PathCount) {
        int alloc_size = m_PathCount;
        if (alloc_size % 8) {
            alloc_size += 8 - (alloc_size % 8);
        }
        m_pPathList = new CPDF_Path[alloc_size];
        for (int i = 0; i < m_PathCount; i ++) {
            m_pPathList[i] = src.m_pPathList[i];
        }
        m_pTypeList = FX_Alloc(FX_BYTE, alloc_size);
        FXSYS_memcpy32(m_pTypeList, src.m_pTypeList, m_PathCount);
    } else {
        m_pPathList = NULL;
        m_pTypeList = NULL;
    }
    m_TextCount = src.m_TextCount;
    if (m_TextCount) {
        m_pTextList = FX_Alloc(CPDF_TextObject*, m_TextCount);
        for (int i = 0; i < m_TextCount; i ++) {
            if (src.m_pTextList[i]) {
                m_pTextList[i] = new CPDF_TextObject;
                m_pTextList[i]->Copy(src.m_pTextList[i]);
            } else {
                m_pTextList[i] = NULL;
            }
        }
    } else {
        m_pTextList = NULL;
    }
}
void CPDF_ClipPathData::SetCount(int path_count, int text_count)
{
    ASSERT(m_TextCount == 0 && m_PathCount == 0);
    if (path_count) {
        m_PathCount = path_count;
        int alloc_size = (path_count + 7) / 8 * 8;
        m_pPathList = new CPDF_Path[alloc_size];
        m_pTypeList = FX_Alloc(FX_BYTE, alloc_size);
    }
    if (text_count) {
        m_TextCount = text_count;
        m_pTextList = FX_Alloc(CPDF_TextObject*, text_count);
    }
}
CPDF_Rect CPDF_ClipPath::GetClipBox() const
{
    CPDF_Rect rect;
    FX_BOOL bStarted = FALSE;
    int count = GetPathCount();
    if (count) {
        rect = GetPath(0).GetBoundingBox();
        for (int i = 1; i < count; i ++) {
            CPDF_Rect path_rect = GetPath(i).GetBoundingBox();
            rect.Intersect(path_rect);
        }
        bStarted = TRUE;
    }
    count = GetTextCount();
    if (count) {
        CPDF_Rect layer_rect;
        FX_BOOL bLayerStarted = FALSE;
        for (int i = 0; i < count; i ++) {
            CPDF_TextObject* pTextObj = GetText(i);
            if (pTextObj == NULL) {
                if (!bStarted) {
                    rect = layer_rect;
                    bStarted = TRUE;
                } else {
                    rect.Intersect(layer_rect);
                }
                bLayerStarted = FALSE;
            } else {
                if (!bLayerStarted) {
                    layer_rect = pTextObj->GetBBox(NULL);
                    bLayerStarted = TRUE;
                } else {
                    layer_rect.Union(pTextObj->GetBBox(NULL));
                }
            }
        }
    }
    return rect;
}
void CPDF_ClipPath::AppendPath(CPDF_Path path, int type, FX_BOOL bAutoMerge)
{
    CPDF_ClipPathData* pData = GetModify();
    if (pData->m_PathCount && bAutoMerge) {
        CPDF_Path old_path = pData->m_pPathList[pData->m_PathCount - 1];
        if (old_path.IsRect()) {
            CPDF_Rect old_rect(old_path.GetPointX(0), old_path.GetPointY(0),
                               old_path.GetPointX(2), old_path.GetPointY(2));
            CPDF_Rect new_rect = path.GetBoundingBox();
            if (old_rect.Contains(new_rect)) {
                pData->m_PathCount --;
                pData->m_pPathList[pData->m_PathCount].SetNull();
            }
        }
    }
    if (pData->m_PathCount % 8 == 0) {
        CPDF_Path* pNewPath = new CPDF_Path[pData->m_PathCount + 8];
        for (int i = 0; i < pData->m_PathCount; i ++) {
            pNewPath[i] = pData->m_pPathList[i];
        }
        delete[] pData->m_pPathList;
        FX_BYTE* pNewType = FX_Alloc(FX_BYTE, pData->m_PathCount + 8);
        FXSYS_memcpy32(pNewType, pData->m_pTypeList, pData->m_PathCount);
        if (pData->m_pTypeList) {
            FX_Free(pData->m_pTypeList);
        }
        pData->m_pPathList = pNewPath;
        pData->m_pTypeList = pNewType;
    }
    pData->m_pPathList[pData->m_PathCount] = path;
    pData->m_pTypeList[pData->m_PathCount] = (FX_BYTE)type;
    pData->m_PathCount ++;
}
void CPDF_ClipPath::DeletePath(int index)
{
    CPDF_ClipPathData* pData = GetModify();
    if (index >= pData->m_PathCount) {
        return;
    }
    pData->m_pPathList[index].SetNull();
    for (int i = index; i < pData->m_PathCount - 1; i ++) {
        pData->m_pPathList[i] = pData->m_pPathList[i + 1];
    }
    pData->m_pPathList[pData->m_PathCount - 1].SetNull();
    FXSYS_memmove32(pData->m_pTypeList + index, pData->m_pTypeList + index + 1, pData->m_PathCount - index - 1);
    pData->m_PathCount --;
}
#define FPDF_CLIPPATH_MAX_TEXTS 1024
void CPDF_ClipPath::AppendTexts(CPDF_TextObject** pTexts, int count)
{
    CPDF_ClipPathData* pData = GetModify();
    if (pData->m_TextCount + count > FPDF_CLIPPATH_MAX_TEXTS) {
        for (int i = 0; i < count; i ++) {
            delete pTexts[i];
        }
        return;
    }
    CPDF_TextObject** pNewList = FX_Alloc(CPDF_TextObject*, pData->m_TextCount + count + 1);
    if (pData->m_pTextList) {
        FXSYS_memcpy32(pNewList, pData->m_pTextList, pData->m_TextCount * sizeof(CPDF_TextObject*));
        FX_Free(pData->m_pTextList);
    }
    pData->m_pTextList = pNewList;
    for (int i = 0; i < count; i ++) {
        pData->m_pTextList[pData->m_TextCount + i] = pTexts[i];
    }
    pData->m_pTextList[pData->m_TextCount + count] = NULL;
    pData->m_TextCount += count + 1;
}
void CPDF_ClipPath::Transform(const CPDF_Matrix& matrix)
{
    CPDF_ClipPathData* pData = GetModify();
    int i;
    for (i = 0; i < pData->m_PathCount; i ++) {
        pData->m_pPathList[i].Transform(&matrix);
    }
    for (i = 0; i < pData->m_TextCount; i ++)
        if (pData->m_pTextList[i]) {
            pData->m_pTextList[i]->Transform(matrix);
        }
}
CPDF_ColorStateData::CPDF_ColorStateData(const CPDF_ColorStateData& src)
{
    m_FillColor.Copy(&src.m_FillColor);
    m_FillRGB = src.m_FillRGB;
    m_StrokeColor.Copy(&src.m_StrokeColor);
    m_StrokeRGB = src.m_StrokeRGB;
}
void CPDF_ColorStateData::Default()
{
    m_FillRGB = m_StrokeRGB = 0;
    m_FillColor.SetColorSpace(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY));
    m_StrokeColor.SetColorSpace(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY));
}
void CPDF_ColorState::SetFillColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues)
{
    CPDF_ColorStateData* pData = GetModify();
    SetColor(pData->m_FillColor, pData->m_FillRGB, pCS, pValue, nValues);
}
void CPDF_ColorState::SetStrokeColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues)
{
    CPDF_ColorStateData* pData = GetModify();
    SetColor(pData->m_StrokeColor, pData->m_StrokeRGB, pCS, pValue, nValues);
}
void CPDF_ColorState::SetColor(CPDF_Color& color, FX_DWORD& rgb, CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues)
{
    if (pCS) {
        color.SetColorSpace(pCS);
    } else if (color.IsNull()) {
        color.SetColorSpace(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY));
    }
    if (color.m_pCS->CountComponents() > nValues) {
        return;
    }
    color.SetValue(pValue);
    int R, G, B;
    rgb = color.GetRGB(R, G, B) ? FXSYS_RGB(R, G, B) : (FX_DWORD) - 1;
}
void CPDF_ColorState::SetFillPattern(CPDF_Pattern* pPattern, FX_FLOAT* pValue, int nValues)
{
    CPDF_ColorStateData* pData = GetModify();
    pData->m_FillColor.SetValue(pPattern, pValue, nValues);
    int R, G, B;
    FX_BOOL ret = pData->m_FillColor.GetRGB(R, G, B);
    if (pPattern->m_PatternType == 1 && ((CPDF_TilingPattern*)pPattern)->m_bColored && !ret) {
        pData->m_FillRGB = 0x00BFBFBF;
        return;
    }
    pData->m_FillRGB = ret ? FXSYS_RGB(R, G, B) : (FX_DWORD) - 1;
}
void CPDF_ColorState::SetStrokePattern(CPDF_Pattern* pPattern, FX_FLOAT* pValue, int nValues)
{
    CPDF_ColorStateData* pData = GetModify();
    pData->m_StrokeColor.SetValue(pPattern, pValue, nValues);
    int R, G, B;
    FX_BOOL ret = pData->m_StrokeColor.GetRGB(R, G, B);
    if (pPattern->m_PatternType == 1 && ((CPDF_TilingPattern*)pPattern)->m_bColored && !ret) {
        pData->m_StrokeRGB = 0x00BFBFBF;
        return;
    }
    pData->m_StrokeRGB = pData->m_StrokeColor.GetRGB(R, G, B) ? FXSYS_RGB(R, G, B) : (FX_DWORD) - 1;
}
CPDF_TextStateData::CPDF_TextStateData()
{
    m_pFont = NULL;
    m_pDocument = NULL;
    m_FontSize = 1.0f;
    m_WordSpace = 0;
    m_CharSpace = 0;
    m_TextMode = 0;
    m_Matrix[0] = m_Matrix[3] = 1.0f;
    m_Matrix[1] = m_Matrix[2] = 0;
    m_CTM[0] = m_CTM[3] = 1.0f;
    m_CTM[1] = m_CTM[2] = 0;
}
CPDF_TextStateData::CPDF_TextStateData(const CPDF_TextStateData& src)
{
    if (this == &src) {
        return;
    }
    FXSYS_memcpy32(this, &src, sizeof(CPDF_TextStateData));
    if (m_pDocument && m_pFont) {
        m_pFont = m_pDocument->GetPageData()->GetFont(m_pFont->GetFontDict(), FALSE);
    }
}
CPDF_TextStateData::~CPDF_TextStateData()
{
    if (m_pDocument && m_pFont) {
        CPDF_DocPageData *pPageData = m_pDocument->GetPageData();
        if (pPageData && !pPageData->IsForceClear()) {
            pPageData->ReleaseFont(m_pFont->GetFontDict());
        }
    }
}
void CPDF_TextState::SetFont(CPDF_Font* pFont)
{
    CPDF_TextStateData* pStateData = GetModify();
    if (pStateData) {
        CPDF_Document* pDoc = pStateData->m_pDocument;
        CPDF_DocPageData *pPageData = pDoc ? pDoc->GetPageData() : NULL;
        if (pPageData && pStateData->m_pFont && !pPageData->IsForceClear()) {
            pPageData->ReleaseFont(pStateData->m_pFont->GetFontDict());
        }
        pStateData->m_pDocument = pFont ? pFont->m_pDocument : NULL;
        pStateData->m_pFont = pFont;
    }
}
FX_FLOAT CPDF_TextState::GetFontSizeV() const
{
    FX_FLOAT* pMatrix = GetMatrix();
    FX_FLOAT unit = FXSYS_sqrt2(pMatrix[1], pMatrix[3]);
    FX_FLOAT size = FXSYS_Mul(unit, GetFontSize());
    return (FX_FLOAT)FXSYS_fabs(size);
}
FX_FLOAT CPDF_TextState::GetFontSizeH() const
{
    FX_FLOAT* pMatrix = GetMatrix();
    FX_FLOAT unit = FXSYS_sqrt2(pMatrix[0], pMatrix[2]);
    FX_FLOAT size = FXSYS_Mul(unit, GetFontSize());
    return (FX_FLOAT)FXSYS_fabs(size);
}
FX_FLOAT CPDF_TextState::GetBaselineAngle() const
{
    FX_FLOAT* m_Matrix = GetMatrix();
    return FXSYS_atan2(m_Matrix[2], m_Matrix[0]);
}
FX_FLOAT CPDF_TextState::GetShearAngle() const
{
    FX_FLOAT* m_Matrix = GetMatrix();
    FX_FLOAT shear_angle = FXSYS_atan2(m_Matrix[1], m_Matrix[3]);
    return GetBaselineAngle() + shear_angle;
}
CPDF_GeneralStateData::CPDF_GeneralStateData()
{
    FXSYS_memset32(this, 0, sizeof(CPDF_GeneralStateData));
    FXSYS_strcpy((FX_LPSTR)m_BlendMode, "Normal");
    m_StrokeAlpha = 1.0f;
    m_FillAlpha = 1.0f;
    m_Flatness = 1.0f;
    m_Matrix.SetIdentity();
}
CPDF_GeneralStateData::CPDF_GeneralStateData(const CPDF_GeneralStateData& src)
{
    FXSYS_memcpy32(this, &src, sizeof(CPDF_GeneralStateData));
    if (src.m_pTransferFunc && src.m_pTransferFunc->m_pPDFDoc) {
        CPDF_DocRenderData* pDocCache = src.m_pTransferFunc->m_pPDFDoc->GetRenderData();
        if (!pDocCache) {
            return;
        }
        m_pTransferFunc = pDocCache->GetTransferFunc(m_pTR);
    }
}
CPDF_GeneralStateData::~CPDF_GeneralStateData()
{
    if (m_pTransferFunc && m_pTransferFunc->m_pPDFDoc) {
        CPDF_DocRenderData* pDocCache = m_pTransferFunc->m_pPDFDoc->GetRenderData();
        if (!pDocCache) {
            return;
        }
        pDocCache->ReleaseTransferFunc(m_pTR);
    }
}
static int GetBlendType(FX_BSTR mode)
{
    switch (mode.GetID()) {
        case FXBSTR_ID('N', 'o', 'r', 'm'):
        case FXBSTR_ID('C', 'o', 'm', 'p'):
            return FXDIB_BLEND_NORMAL;
        case FXBSTR_ID('M', 'u', 'l', 't'):
            return FXDIB_BLEND_MULTIPLY;
        case FXBSTR_ID('S', 'c', 'r', 'e'):
            return FXDIB_BLEND_SCREEN;
        case FXBSTR_ID('O', 'v', 'e', 'r'):
            return FXDIB_BLEND_OVERLAY;
        case FXBSTR_ID('D', 'a', 'r', 'k'):
            return FXDIB_BLEND_DARKEN;
        case FXBSTR_ID('L', 'i', 'g', 'h'):
            return FXDIB_BLEND_LIGHTEN;
        case FXBSTR_ID('C', 'o', 'l', 'o'):
            if (mode.GetLength() == 10) {
                return FXDIB_BLEND_COLORDODGE;
            }
            if (mode.GetLength() == 9) {
                return FXDIB_BLEND_COLORBURN;
            }
            return FXDIB_BLEND_COLOR;
        case FXBSTR_ID('H', 'a', 'r', 'd'):
            return FXDIB_BLEND_HARDLIGHT;
        case FXBSTR_ID('S', 'o', 'f', 't'):
            return FXDIB_BLEND_SOFTLIGHT;
        case FXBSTR_ID('D', 'i', 'f', 'f'):
            return FXDIB_BLEND_DIFFERENCE;
        case FXBSTR_ID('E', 'x', 'c', 'l'):
            return FXDIB_BLEND_EXCLUSION;
        case FXBSTR_ID('H', 'u', 'e', 0):
            return FXDIB_BLEND_HUE;
        case FXBSTR_ID('S', 'a', 't', 'u'):
            return FXDIB_BLEND_SATURATION;
        case FXBSTR_ID('L', 'u', 'm', 'i'):
            return FXDIB_BLEND_LUMINOSITY;
    }
    return FXDIB_BLEND_NORMAL;
}
void CPDF_GeneralStateData::SetBlendMode(FX_BSTR blend_mode)
{
    if (blend_mode.GetLength() > 15) {
        return;
    }
    FXSYS_memcpy32(m_BlendMode, blend_mode.GetPtr(), blend_mode.GetLength());
    m_BlendMode[blend_mode.GetLength()] = 0;
    m_BlendType = ::GetBlendType(blend_mode);
}
int RI_StringToId(const CFX_ByteString& ri)
{
    FX_DWORD id = ri.GetID();
    if (id == FXBSTR_ID('A', 'b', 's', 'o')) {
        return 1;
    }
    if (id == FXBSTR_ID('S', 'a', 't', 'u')) {
        return 2;
    }
    if (id == FXBSTR_ID('P', 'e', 'r', 'c')) {
        return 3;
    }
    return 0;
}
void CPDF_GeneralState::SetRenderIntent(const CFX_ByteString& ri)
{
    GetModify()->m_RenderIntent = RI_StringToId(ri);
}
CPDF_AllStates::CPDF_AllStates()
{
    m_TextX = m_TextY = m_TextLineX = m_TextLineY = 0;
    m_TextLeading = 0;
    m_TextRise = 0;
    m_TextHorzScale = 1.0f;
}
CPDF_AllStates::~CPDF_AllStates()
{
}
void CPDF_AllStates::Copy(const CPDF_AllStates& src)
{
    CopyStates(src);
    m_TextMatrix.Copy(src.m_TextMatrix);
    m_ParentMatrix.Copy(src.m_ParentMatrix);
    m_CTM.Copy(src.m_CTM);
    m_TextX = src.m_TextX;
    m_TextY = src.m_TextY;
    m_TextLineX = src.m_TextLineX;
    m_TextLineY = src.m_TextLineY;
    m_TextLeading = src.m_TextLeading;
    m_TextRise = src.m_TextRise;
    m_TextHorzScale = src.m_TextHorzScale;
}
void CPDF_AllStates::SetLineDash(CPDF_Array* pArray, FX_FLOAT phase, FX_FLOAT scale)
{
    CFX_GraphStateData* pData = m_GraphState.GetModify();
    pData->m_DashPhase = FXSYS_Mul(phase, scale);
    pData->SetDashCount(pArray->GetCount());
    for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
        pData->m_DashArray[i] = FXSYS_Mul(pArray->GetNumber(i), scale);
    }
}
void CPDF_AllStates::ProcessExtGS(CPDF_Dictionary* pGS, CPDF_StreamContentParser* pParser)
{
    CPDF_GeneralStateData* pGeneralState = m_GeneralState.GetModify();
    FX_POSITION pos = pGS->GetStartPos();
    while (pos) {
        CFX_ByteString key_str;
        CPDF_Object* pElement = pGS->GetNextElement(pos, key_str);
        CPDF_Object* pObject = pElement ? pElement->GetDirect() : NULL;
        if (pObject == NULL) {
            continue;
        }
        FX_DWORD key = key_str.GetID();
        switch (key) {
            case FXBSTR_ID('L', 'W', 0, 0):
                m_GraphState.GetModify()->m_LineWidth = pObject->GetNumber();
                break;
            case FXBSTR_ID('L', 'C', 0, 0):
                m_GraphState.GetModify()->m_LineCap = (CFX_GraphStateData::LineCap)pObject->GetInteger();
                break;
            case FXBSTR_ID('L', 'J', 0, 0):
                m_GraphState.GetModify()->m_LineJoin = (CFX_GraphStateData::LineJoin)pObject->GetInteger();
                break;
            case FXBSTR_ID('M', 'L', 0, 0):
                m_GraphState.GetModify()->m_MiterLimit = pObject->GetNumber();
                break;
            case FXBSTR_ID('D', 0, 0, 0):	{
                    if (pObject->GetType() != PDFOBJ_ARRAY) {
                        break;
                    }
                    CPDF_Array* pDash = (CPDF_Array*)pObject;
                    CPDF_Array* pArray = pDash->GetArray(0);
                    if (pArray == NULL) {
                        break;
                    }
                    SetLineDash(pArray, pDash->GetNumber(1), 1.0f);
                    break;
                }
            case FXBSTR_ID('R', 'I', 0, 0):
                m_GeneralState.SetRenderIntent(pObject->GetString());
                break;
            case FXBSTR_ID('F', 'o', 'n', 't'):	{
                    if (pObject->GetType() != PDFOBJ_ARRAY) {
                        break;
                    }
                    CPDF_Array* pFont = (CPDF_Array*)pObject;
                    m_TextState.GetModify()->m_FontSize = pFont->GetNumber(1);
                    m_TextState.SetFont(pParser->FindFont(pFont->GetString(0)));
                    break;
                }
            case FXBSTR_ID('T', 'R', 0, 0):
                if (pGS->KeyExist(FX_BSTRC("TR2"))) {
                    continue;
                }
            case FXBSTR_ID('T', 'R', '2', 0):
                if (pObject && pObject->GetType() != PDFOBJ_NAME) {
                    pGeneralState->m_pTR = pObject;
                } else {
                    pGeneralState->m_pTR = NULL;
                }
                break;
            case FXBSTR_ID('B', 'M', 0, 0):	{
                    CFX_ByteString mode;
                    if (pObject->GetType() == PDFOBJ_ARRAY) {
                        mode = ((CPDF_Array*)pObject)->GetString(0);
                    } else {
                        mode = pObject->GetString();
                    }
                    pGeneralState->SetBlendMode(mode);
                    if (pGeneralState->m_BlendType > FXDIB_BLEND_MULTIPLY) {
                        pParser->m_pObjectList->m_bBackgroundAlphaNeeded = TRUE;
                    }
                    break;
                }
            case FXBSTR_ID('S', 'M', 'a', 's'):
                if (pObject && pObject->GetType() == PDFOBJ_DICTIONARY) {
                    pGeneralState->m_pSoftMask = pObject;
                    FXSYS_memcpy32(pGeneralState->m_SMaskMatrix, &pParser->m_pCurStates->m_CTM, sizeof(CPDF_Matrix));
                } else {
                    pGeneralState->m_pSoftMask = NULL;
                }
                break;
            case FXBSTR_ID('C', 'A', 0, 0):
                pGeneralState->m_StrokeAlpha = PDF_ClipFloat(pObject->GetNumber());
                break;
            case FXBSTR_ID('c', 'a', 0, 0):
                pGeneralState->m_FillAlpha = PDF_ClipFloat(pObject->GetNumber());
                break;
            case FXBSTR_ID('O', 'P', 0, 0):
                pGeneralState->m_StrokeOP = pObject->GetInteger();
                if (!pGS->KeyExist(FX_BSTRC("op"))) {
                    pGeneralState->m_FillOP = pObject->GetInteger();
                }
                break;
            case FXBSTR_ID('o', 'p', 0, 0):
                pGeneralState->m_FillOP = pObject->GetInteger();
                break;
            case FXBSTR_ID('O', 'P', 'M', 0):
                pGeneralState->m_OPMode = pObject->GetInteger();
                break;
            case FXBSTR_ID('B', 'G', 0, 0):
                if (pGS->KeyExist(FX_BSTRC("BG2"))) {
                    continue;
                }
            case FXBSTR_ID('B', 'G', '2', 0):
                pGeneralState->m_pBG = pObject;
                break;
            case FXBSTR_ID('U', 'C', 'R', 0):
                if (pGS->KeyExist(FX_BSTRC("UCR2"))) {
                    continue;
                }
            case FXBSTR_ID('U', 'C', 'R', '2'):
                pGeneralState->m_pUCR = pObject;
                break;
            case FXBSTR_ID('H', 'T', 0, 0):
                pGeneralState->m_pHT = pObject;
                break;
            case FXBSTR_ID('F', 'L', 0, 0):
                pGeneralState->m_Flatness = pObject->GetNumber();
                break;
            case FXBSTR_ID('S', 'M', 0, 0):
                pGeneralState->m_Smoothness = pObject->GetNumber();
                break;
            case FXBSTR_ID('S', 'A', 0, 0):
                pGeneralState->m_StrokeAdjust = pObject->GetInteger();
                break;
            case FXBSTR_ID('A', 'I', 'S', 0):
                pGeneralState->m_AlphaSource = pObject->GetInteger();
                break;
            case FXBSTR_ID('T', 'K', 0, 0):
                pGeneralState->m_TextKnockout = pObject->GetInteger();
                break;
        }
    }
    pGeneralState->m_Matrix = m_CTM;
}
CPDF_ContentMarkItem::CPDF_ContentMarkItem()
{
    m_ParamType = None;
}
CPDF_ContentMarkItem::CPDF_ContentMarkItem(const CPDF_ContentMarkItem& src)
{
    m_MarkName = src.m_MarkName;
    m_ParamType = src.m_ParamType;
    if (m_ParamType == DirectDict) {
        m_pParam = ((CPDF_Dictionary*)src.m_pParam)->Clone();
    } else {
        m_pParam = src.m_pParam;
    }
}
CPDF_ContentMarkItem::~CPDF_ContentMarkItem()
{
    if (m_ParamType == DirectDict && m_pParam) {
        ((CPDF_Dictionary*)m_pParam)->Release();
    }
}
FX_BOOL	CPDF_ContentMarkItem::HasMCID() const
{
    if (m_pParam && (m_ParamType == DirectDict || m_ParamType == PropertiesDict)) {
        return ((CPDF_Dictionary *)m_pParam)->KeyExist(FX_BSTRC("MCID"));
    }
    return FALSE;
}
CPDF_ContentMarkData::CPDF_ContentMarkData(const CPDF_ContentMarkData& src)
{
    for (int i = 0; i < src.m_Marks.GetSize(); i ++) {
        m_Marks.Add(src.m_Marks[i]);
    }
}
int CPDF_ContentMarkData::GetMCID() const
{
    CPDF_ContentMarkItem::ParamType type = CPDF_ContentMarkItem::None;
    for (int i = 0; i < m_Marks.GetSize(); i ++) {
        type = m_Marks[i].GetParamType();
        if (type == CPDF_ContentMarkItem::PropertiesDict || type == CPDF_ContentMarkItem::DirectDict) {
            CPDF_Dictionary *pDict = (CPDF_Dictionary *)m_Marks[i].GetParam();
            if (pDict->KeyExist(FX_BSTRC("MCID"))) {
                return pDict->GetInteger(FX_BSTRC("MCID"));
            }
        }
    }
    return -1;
}
void CPDF_ContentMarkData::AddMark(const CFX_ByteString& name, CPDF_Dictionary* pDict, FX_BOOL bDirect)
{
    CPDF_ContentMarkItem& item = m_Marks.Add();
    item.SetName(name);
    if (pDict == NULL) {
        return;
    }
    item.SetParam(bDirect ? CPDF_ContentMarkItem::DirectDict : CPDF_ContentMarkItem::PropertiesDict,
                  bDirect ? pDict->Clone() : pDict);
}
void CPDF_ContentMarkData::DeleteLastMark()
{
    int size = m_Marks.GetSize();
    if (size == 0) {
        return;
    }
    m_Marks.RemoveAt(size - 1);
}
FX_BOOL CPDF_ContentMark::HasMark(FX_BSTR mark) const
{
    if (m_pObject == NULL) {
        return FALSE;
    }
    for (int i = 0; i < m_pObject->CountItems(); i ++) {
        CPDF_ContentMarkItem& item = m_pObject->GetItem(i);
        if (item.GetName() == mark) {
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_ContentMark::LookupMark(FX_BSTR mark, CPDF_Dictionary*& pDict) const
{
    if (m_pObject == NULL) {
        return FALSE;
    }
    for (int i = 0; i < m_pObject->CountItems(); i ++) {
        CPDF_ContentMarkItem& item = m_pObject->GetItem(i);
        if (item.GetName() == mark) {
            pDict = NULL;
            if (item.GetParamType() == CPDF_ContentMarkItem::PropertiesDict ||
                    item.GetParamType() == CPDF_ContentMarkItem::DirectDict) {
                pDict = (CPDF_Dictionary*)item.GetParam();
            }
            return TRUE;
        }
    }
    return FALSE;
}
