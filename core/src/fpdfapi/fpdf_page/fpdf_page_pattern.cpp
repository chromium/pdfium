// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "pageint.h"

CPDF_Pattern::CPDF_Pattern(const CFX_AffineMatrix* pParentMatrix) :
    m_pPatternObj(NULL), m_PatternType(PATTERN_TILING), m_pDocument(NULL), m_bForceClear(FALSE)
{
    if (pParentMatrix) {
        m_ParentMatrix = *pParentMatrix;
    }
}
CPDF_Pattern::~CPDF_Pattern()
{
}
CPDF_TilingPattern::CPDF_TilingPattern(CPDF_Document* pDoc, CPDF_Object* pPatternObj, const CFX_AffineMatrix* parentMatrix) :
    CPDF_Pattern(parentMatrix)
{
    m_PatternType = PATTERN_TILING;
    m_pPatternObj = pPatternObj;
    m_pDocument = pDoc;
    CPDF_Dictionary* pDict = m_pPatternObj->GetDict();
    ASSERT(pDict != NULL);
    m_Pattern2Form = pDict->GetMatrix(FX_BSTRC("Matrix"));
    m_bColored = pDict->GetInteger(FX_BSTRC("PaintType")) == 1;
    if (parentMatrix) {
        m_Pattern2Form.Concat(*parentMatrix);
    }
    m_pForm = NULL;
}
CPDF_TilingPattern::~CPDF_TilingPattern()
{
    if (m_pForm) {
        delete m_pForm;
        m_pForm = NULL;
    }
}
FX_BOOL CPDF_TilingPattern::Load()
{
    if (m_pForm != NULL) {
        return TRUE;
    }
    CPDF_Dictionary* pDict = m_pPatternObj->GetDict();
    if (pDict == NULL) {
        return FALSE;
    }
    m_bColored = pDict->GetInteger(FX_BSTRC("PaintType")) == 1;
    m_XStep = (FX_FLOAT)FXSYS_fabs(pDict->GetNumber(FX_BSTRC("XStep")));
    m_YStep = (FX_FLOAT)FXSYS_fabs(pDict->GetNumber(FX_BSTRC("YStep")));
    if (m_pPatternObj->GetType() != PDFOBJ_STREAM) {
        return FALSE;
    }
    CPDF_Stream* pStream = (CPDF_Stream*)m_pPatternObj;
    m_pForm = new CPDF_Form(m_pDocument, NULL, pStream);
    m_pForm->ParseContent(NULL, &m_ParentMatrix, NULL, NULL);
    m_BBox = pDict->GetRect(FX_BSTRC("BBox"));
    return TRUE;
}
CPDF_ShadingPattern::CPDF_ShadingPattern(CPDF_Document* pDoc, CPDF_Object* pPatternObj, FX_BOOL bShading, const CFX_AffineMatrix* parentMatrix) : CPDF_Pattern(parentMatrix)
{
    m_PatternType = PATTERN_SHADING;
    m_pPatternObj = bShading ? NULL : pPatternObj;
    m_pDocument = pDoc;
    m_bShadingObj = bShading;
    if (!bShading) {
        CPDF_Dictionary* pDict = m_pPatternObj->GetDict();
        ASSERT(pDict != NULL);
        m_Pattern2Form = pDict->GetMatrix(FX_BSTRC("Matrix"));
        m_pShadingObj = pDict->GetElementValue(FX_BSTRC("Shading"));
        if (parentMatrix) {
            m_Pattern2Form.Concat(*parentMatrix);
        }
    } else {
        m_pShadingObj = pPatternObj;
    }
    m_ShadingType = 0;
    m_pCS = NULL;
    m_nFuncs = 0;
    for (int i = 0; i < 4; i ++) {
        m_pFunctions[i] = NULL;
    }
    m_pCountedCS = NULL;
}
CPDF_ShadingPattern::~CPDF_ShadingPattern()
{
    Clear();
}
void CPDF_ShadingPattern::Clear()
{
    for (int i = 0; i < m_nFuncs; i ++) {
        if (m_pFunctions[i]) {
            delete m_pFunctions[i];
        }
        m_pFunctions[i] = NULL;
    }
    CPDF_ColorSpace* pCS = m_pCountedCS ? m_pCountedCS->m_Obj : NULL;
    if (pCS && m_pDocument) {
        m_pDocument->GetPageData()->ReleaseColorSpace(pCS->GetArray());
    }
    m_ShadingType = 0;
    m_pCS = NULL;
    m_pCountedCS = NULL;
    m_nFuncs = 0;
}
FX_BOOL CPDF_ShadingPattern::Load()
{
    if (m_ShadingType != 0) {
        return TRUE;
    }
    CPDF_Dictionary* pShadingDict = m_pShadingObj ? m_pShadingObj->GetDict() : NULL;
    if (pShadingDict == NULL) {
        return FALSE;
    }
    if (m_nFuncs) {
        for (int i = 0; i < m_nFuncs; i ++)
            if (m_pFunctions[i]) {
                delete m_pFunctions[i];
            }
        m_nFuncs = 0;
    }
    CPDF_Object* pFunc = pShadingDict->GetElementValue(FX_BSTRC("Function"));
    if (pFunc) {
        if (pFunc->GetType() == PDFOBJ_ARRAY) {
            m_nFuncs = ((CPDF_Array*)pFunc)->GetCount();
            if (m_nFuncs > 4) {
                m_nFuncs = 4;
            }
            for (int i = 0; i < m_nFuncs; i ++) {
                m_pFunctions[i] = CPDF_Function::Load(((CPDF_Array*)pFunc)->GetElementValue(i));
            }
        } else {
            m_pFunctions[0] = CPDF_Function::Load(pFunc);
            m_nFuncs = 1;
        }
    }
    CPDF_Object* pCSObj = pShadingDict->GetElementValue(FX_BSTRC("ColorSpace"));
    if (pCSObj == NULL) {
        return FALSE;
    }
    CPDF_DocPageData* pDocPageData = m_pDocument->GetPageData();
    m_pCS = pDocPageData->GetColorSpace(pCSObj, NULL);
    if (m_pCS) {
        m_pCountedCS = pDocPageData->FindColorSpacePtr(m_pCS->GetArray());
    }
    m_ShadingType = pShadingDict->GetInteger(FX_BSTRC("ShadingType"));
    return TRUE;
}
FX_BOOL CPDF_ShadingPattern::Reload()
{
    Clear();
    return Load();
}
FX_BOOL CPDF_MeshStream::Load(CPDF_Stream* pShadingStream, CPDF_Function** pFuncs, int nFuncs, CPDF_ColorSpace* pCS)
{
    m_Stream.LoadAllData(pShadingStream);
    m_BitStream.Init(m_Stream.GetData(), m_Stream.GetSize());
    m_pFuncs = pFuncs;
    m_nFuncs = nFuncs;
    m_pCS = pCS;
    CPDF_Dictionary* pDict = pShadingStream->GetDict();
    m_nCoordBits = pDict->GetInteger(FX_BSTRC("BitsPerCoordinate"));
    m_nCompBits = pDict->GetInteger(FX_BSTRC("BitsPerComponent"));
    m_nFlagBits = pDict->GetInteger(FX_BSTRC("BitsPerFlag"));
    if (!m_nCoordBits || !m_nCompBits) {
        return FALSE;
    }
    int nComps = pCS->CountComponents();
    if (nComps > 8) {
        return FALSE;
    }
    m_nComps = nFuncs ? 1 : nComps;
    if (((int)m_nComps < 0) || m_nComps > 8) {
        return FALSE;
    }
    m_CoordMax = m_nCoordBits == 32 ? -1 : (1 << m_nCoordBits) - 1;
    m_CompMax = (1 << m_nCompBits) - 1;
    CPDF_Array* pDecode = pDict->GetArray(FX_BSTRC("Decode"));
    if (pDecode == NULL || pDecode->GetCount() != 4 + m_nComps * 2) {
        return FALSE;
    }
    m_xmin = pDecode->GetNumber(0);
    m_xmax = pDecode->GetNumber(1);
    m_ymin = pDecode->GetNumber(2);
    m_ymax = pDecode->GetNumber(3);
    for (FX_DWORD i = 0; i < m_nComps; i ++) {
        m_ColorMin[i] = pDecode->GetNumber(i * 2 + 4);
        m_ColorMax[i] = pDecode->GetNumber(i * 2 + 5);
    }
    return TRUE;
}
FX_DWORD CPDF_MeshStream::GetFlag()
{
    return m_BitStream.GetBits(m_nFlagBits) & 0x03;
}
void CPDF_MeshStream::GetCoords(FX_FLOAT& x, FX_FLOAT& y)
{
    if (m_nCoordBits == 32) {
        x = m_xmin + (FX_FLOAT)(m_BitStream.GetBits(m_nCoordBits) * (m_xmax - m_xmin) / (double)m_CoordMax);
        y = m_ymin + (FX_FLOAT)(m_BitStream.GetBits(m_nCoordBits) * (m_ymax - m_ymin) / (double)m_CoordMax);
    } else {
        x = m_xmin + m_BitStream.GetBits(m_nCoordBits) * (m_xmax - m_xmin) / m_CoordMax;
        y = m_ymin + m_BitStream.GetBits(m_nCoordBits) * (m_ymax - m_ymin) / m_CoordMax;
    }
}
void CPDF_MeshStream::GetColor(FX_FLOAT& r, FX_FLOAT& g, FX_FLOAT& b)
{
    FX_DWORD i;
    FX_FLOAT color_value[8];
    for (i = 0; i < m_nComps; i ++) {
        color_value[i] = m_ColorMin[i] + m_BitStream.GetBits(m_nCompBits) * (m_ColorMax[i] - m_ColorMin[i]) / m_CompMax;
    }
    if (m_nFuncs) {
        static const int kMaxResults = 8;
        FX_FLOAT result[kMaxResults];
        int nResults;
        FXSYS_memset32(result, 0, sizeof(result));
        for (FX_DWORD i = 0; i < m_nFuncs; i ++) {
            if (m_pFuncs[i] && m_pFuncs[i]->CountOutputs() <= kMaxResults) {
                m_pFuncs[i]->Call(color_value, 1, result, nResults);
            }
        }
        m_pCS->GetRGB(result, r, g, b);
    } else {
        m_pCS->GetRGB(color_value, r, g, b);
    }
}
FX_DWORD CPDF_MeshStream::GetVertex(CPDF_MeshVertex& vertex, CFX_AffineMatrix* pObject2Bitmap)
{
    FX_DWORD flag = GetFlag();
    GetCoords(vertex.x, vertex.y);
    pObject2Bitmap->Transform(vertex.x, vertex.y);
    GetColor(vertex.r, vertex.g, vertex.b);
    m_BitStream.ByteAlign();
    return flag;
}
FX_BOOL CPDF_MeshStream::GetVertexRow(CPDF_MeshVertex* vertex, int count, CFX_AffineMatrix* pObject2Bitmap)
{
    for (int i = 0; i < count; i ++) {
        if (m_BitStream.IsEOF()) {
            return FALSE;
        }
        GetCoords(vertex[i].x, vertex[i].y);
        pObject2Bitmap->Transform(vertex[i].x, vertex[i].y);
        GetColor(vertex[i].r, vertex[i].g, vertex[i].b);
        m_BitStream.ByteAlign();
    }
    return TRUE;
}
CFX_FloatRect _GetShadingBBox(CPDF_Stream* pStream, int type, const CFX_AffineMatrix* pMatrix,
                              CPDF_Function** pFuncs, int nFuncs, CPDF_ColorSpace* pCS)
{
    if (pStream == NULL || pStream->GetType() != PDFOBJ_STREAM || pFuncs == NULL || pCS == NULL) {
        return CFX_FloatRect(0, 0, 0, 0);
    }
    CPDF_MeshStream stream;
    if (!stream.Load(pStream, pFuncs, nFuncs, pCS)) {
        return CFX_FloatRect(0, 0, 0, 0);
    }
    CFX_FloatRect rect;
    FX_BOOL bStarted = FALSE;
    FX_BOOL bGouraud = type == 4 || type == 5;
    int full_point_count = type == 7 ? 16 : (type == 6 ? 12 : 1);
    int full_color_count = (type == 6 || type == 7) ? 4 : 1;
    while (!stream.m_BitStream.IsEOF()) {
        FX_DWORD flag = 0;
        if (type != 5) {
            flag = stream.GetFlag();
        }
        int point_count = full_point_count, color_count = full_color_count;
        if (!bGouraud && flag) {
            point_count -= 4;
            color_count -= 2;
        }
        for (int i = 0; i < point_count; i ++) {
            FX_FLOAT x, y;
            stream.GetCoords(x, y);
            if (bStarted) {
                rect.UpdateRect(x, y);
            } else {
                rect.InitRect(x, y);
                bStarted = TRUE;
            }
        }
        stream.m_BitStream.SkipBits(stream.m_nComps * stream.m_nCompBits * color_count);
        if (bGouraud) {
            stream.m_BitStream.ByteAlign();
        }
    }
    rect.Transform(pMatrix);
    return rect;
}
