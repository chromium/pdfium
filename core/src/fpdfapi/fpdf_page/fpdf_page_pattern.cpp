// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "pageint.h"

#include "core/include/fpdfapi/fpdf_page.h"

namespace {

const int kSingleCoordinatePair = 1;
const int kTensorCoordinatePairs = 16;
const int kCoonsCoordinatePairs = 12;

const int kSingleColorPerPatch = 1;
const int kQuadColorsPerPatch = 4;

ShadingType ToShadingType(int type) {
  return (type > static_cast<int>(kInvalidShading) &&
          type < static_cast<int>(kMaxShading))
             ? static_cast<ShadingType>(type)
             : kInvalidShading;
}

}  // namespace

CPDF_Pattern::CPDF_Pattern(PatternType type,
                           CPDF_Document* pDoc,
                           CPDF_Object* pObj,
                           const CFX_Matrix* pParentMatrix)
    : m_PatternType(type),
      m_pDocument(pDoc),
      m_pPatternObj(pObj),
      m_bForceClear(FALSE) {
  if (pParentMatrix)
    m_ParentMatrix = *pParentMatrix;
}
CPDF_Pattern::~CPDF_Pattern() {}
CPDF_TilingPattern::CPDF_TilingPattern(CPDF_Document* pDoc,
                                       CPDF_Object* pPatternObj,
                                       const CFX_Matrix* parentMatrix)
    : CPDF_Pattern(TILING, pDoc, pPatternObj, parentMatrix) {
  CPDF_Dictionary* pDict = m_pPatternObj->GetDict();
  m_Pattern2Form = pDict->GetMatrix("Matrix");
  m_bColored = pDict->GetInteger("PaintType") == 1;
  if (parentMatrix) {
    m_Pattern2Form.Concat(*parentMatrix);
  }
  m_pForm = NULL;
}
CPDF_TilingPattern::~CPDF_TilingPattern() {
  delete m_pForm;
  m_pForm = NULL;
}
FX_BOOL CPDF_TilingPattern::Load() {
  if (m_pForm)
    return TRUE;

  CPDF_Dictionary* pDict = m_pPatternObj->GetDict();
  if (!pDict)
    return FALSE;

  m_bColored = pDict->GetInteger("PaintType") == 1;
  m_XStep = (FX_FLOAT)FXSYS_fabs(pDict->GetNumber("XStep"));
  m_YStep = (FX_FLOAT)FXSYS_fabs(pDict->GetNumber("YStep"));

  CPDF_Stream* pStream = m_pPatternObj->AsStream();
  if (!pStream)
    return FALSE;

  m_pForm = new CPDF_Form(m_pDocument, NULL, pStream);
  m_pForm->ParseContent(NULL, &m_ParentMatrix, NULL, NULL);
  m_BBox = pDict->GetRect("BBox");
  return TRUE;
}
CPDF_ShadingPattern::CPDF_ShadingPattern(CPDF_Document* pDoc,
                                         CPDF_Object* pPatternObj,
                                         FX_BOOL bShading,
                                         const CFX_Matrix* parentMatrix)
    : CPDF_Pattern(SHADING,
                   pDoc,
                   bShading ? nullptr : pPatternObj,
                   parentMatrix),
      m_ShadingType(kInvalidShading),
      m_bShadingObj(bShading),
      m_pShadingObj(pPatternObj),
      m_pCS(nullptr),
      m_pCountedCS(nullptr),
      m_nFuncs(0) {
  if (!bShading) {
    CPDF_Dictionary* pDict = m_pPatternObj->GetDict();
    m_Pattern2Form = pDict->GetMatrix("Matrix");
    m_pShadingObj = pDict->GetElementValue("Shading");
    if (parentMatrix)
      m_Pattern2Form.Concat(*parentMatrix);
  }
  for (int i = 0; i < FX_ArraySize(m_pFunctions); ++i)
    m_pFunctions[i] = nullptr;
}

CPDF_ShadingPattern::~CPDF_ShadingPattern() {
  for (int i = 0; i < m_nFuncs; ++i)
    delete m_pFunctions[i];

  CPDF_ColorSpace* pCS = m_pCountedCS ? m_pCountedCS->get() : NULL;
  if (pCS && m_pDocument)
    m_pDocument->GetPageData()->ReleaseColorSpace(pCS->GetArray());
}

FX_BOOL CPDF_ShadingPattern::Load() {
  if (m_ShadingType != kInvalidShading)
    return TRUE;

  CPDF_Dictionary* pShadingDict =
      m_pShadingObj ? m_pShadingObj->GetDict() : NULL;
  if (!pShadingDict) {
    return FALSE;
  }
  if (m_nFuncs) {
    for (int i = 0; i < m_nFuncs; i++)
      delete m_pFunctions[i];
    m_nFuncs = 0;
  }
  CPDF_Object* pFunc = pShadingDict->GetElementValue("Function");
  if (pFunc) {
    if (CPDF_Array* pArray = pFunc->AsArray()) {
      m_nFuncs = std::min<int>(pArray->GetCount(), 4);

      for (int i = 0; i < m_nFuncs; i++) {
        m_pFunctions[i] = CPDF_Function::Load(pArray->GetElementValue(i));
      }
    } else {
      m_pFunctions[0] = CPDF_Function::Load(pFunc);
      m_nFuncs = 1;
    }
  }
  CPDF_Object* pCSObj = pShadingDict->GetElementValue("ColorSpace");
  if (!pCSObj) {
    return FALSE;
  }
  CPDF_DocPageData* pDocPageData = m_pDocument->GetPageData();
  m_pCS = pDocPageData->GetColorSpace(pCSObj, NULL);
  if (m_pCS) {
    m_pCountedCS = pDocPageData->FindColorSpacePtr(m_pCS->GetArray());
  }

  m_ShadingType = ToShadingType(pShadingDict->GetInteger("ShadingType"));

  // We expect to have a stream if our shading type is a mesh.
  if (IsMeshShading() && !ToStream(m_pShadingObj))
    return FALSE;

  return TRUE;
}
FX_BOOL CPDF_MeshStream::Load(CPDF_Stream* pShadingStream,
                              CPDF_Function** pFuncs,
                              int nFuncs,
                              CPDF_ColorSpace* pCS) {
  m_Stream.LoadAllData(pShadingStream);
  m_BitStream.Init(m_Stream.GetData(), m_Stream.GetSize());
  m_pFuncs = pFuncs;
  m_nFuncs = nFuncs;
  m_pCS = pCS;
  CPDF_Dictionary* pDict = pShadingStream->GetDict();
  m_nCoordBits = pDict->GetInteger("BitsPerCoordinate");
  m_nCompBits = pDict->GetInteger("BitsPerComponent");
  m_nFlagBits = pDict->GetInteger("BitsPerFlag");
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
  CPDF_Array* pDecode = pDict->GetArray("Decode");
  if (!pDecode || pDecode->GetCount() != 4 + m_nComps * 2) {
    return FALSE;
  }
  m_xmin = pDecode->GetNumber(0);
  m_xmax = pDecode->GetNumber(1);
  m_ymin = pDecode->GetNumber(2);
  m_ymax = pDecode->GetNumber(3);
  for (FX_DWORD i = 0; i < m_nComps; i++) {
    m_ColorMin[i] = pDecode->GetNumber(i * 2 + 4);
    m_ColorMax[i] = pDecode->GetNumber(i * 2 + 5);
  }
  return TRUE;
}
FX_DWORD CPDF_MeshStream::GetFlag() {
  return m_BitStream.GetBits(m_nFlagBits) & 0x03;
}
void CPDF_MeshStream::GetCoords(FX_FLOAT& x, FX_FLOAT& y) {
  if (m_nCoordBits == 32) {
    x = m_xmin + (FX_FLOAT)(m_BitStream.GetBits(m_nCoordBits) *
                            (m_xmax - m_xmin) / (double)m_CoordMax);
    y = m_ymin + (FX_FLOAT)(m_BitStream.GetBits(m_nCoordBits) *
                            (m_ymax - m_ymin) / (double)m_CoordMax);
  } else {
    x = m_xmin +
        m_BitStream.GetBits(m_nCoordBits) * (m_xmax - m_xmin) / m_CoordMax;
    y = m_ymin +
        m_BitStream.GetBits(m_nCoordBits) * (m_ymax - m_ymin) / m_CoordMax;
  }
}
void CPDF_MeshStream::GetColor(FX_FLOAT& r, FX_FLOAT& g, FX_FLOAT& b) {
  FX_DWORD i;
  FX_FLOAT color_value[8];
  for (i = 0; i < m_nComps; i++) {
    color_value[i] = m_ColorMin[i] +
                     m_BitStream.GetBits(m_nCompBits) *
                         (m_ColorMax[i] - m_ColorMin[i]) / m_CompMax;
  }
  if (m_nFuncs) {
    static const int kMaxResults = 8;
    FX_FLOAT result[kMaxResults];
    int nResults;
    FXSYS_memset(result, 0, sizeof(result));
    for (FX_DWORD i = 0; i < m_nFuncs; i++) {
      if (m_pFuncs[i] && m_pFuncs[i]->CountOutputs() <= kMaxResults) {
        m_pFuncs[i]->Call(color_value, 1, result, nResults);
      }
    }
    m_pCS->GetRGB(result, r, g, b);
  } else {
    m_pCS->GetRGB(color_value, r, g, b);
  }
}
FX_DWORD CPDF_MeshStream::GetVertex(CPDF_MeshVertex& vertex,
                                    CFX_Matrix* pObject2Bitmap) {
  FX_DWORD flag = GetFlag();
  GetCoords(vertex.x, vertex.y);
  pObject2Bitmap->Transform(vertex.x, vertex.y);
  GetColor(vertex.r, vertex.g, vertex.b);
  m_BitStream.ByteAlign();
  return flag;
}
FX_BOOL CPDF_MeshStream::GetVertexRow(CPDF_MeshVertex* vertex,
                                      int count,
                                      CFX_Matrix* pObject2Bitmap) {
  for (int i = 0; i < count; i++) {
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

CFX_FloatRect GetShadingBBox(CPDF_Stream* pStream,
                             ShadingType type,
                             const CFX_Matrix* pMatrix,
                             CPDF_Function** pFuncs,
                             int nFuncs,
                             CPDF_ColorSpace* pCS) {
  if (!pStream || !pStream->IsStream() || !pFuncs || !pCS)
    return CFX_FloatRect(0, 0, 0, 0);

  CPDF_MeshStream stream;
  if (!stream.Load(pStream, pFuncs, nFuncs, pCS))
    return CFX_FloatRect(0, 0, 0, 0);

  CFX_FloatRect rect;
  FX_BOOL bStarted = FALSE;
  FX_BOOL bGouraud = type == kFreeFormGouraudTriangleMeshShading ||
                     type == kLatticeFormGouraudTriangleMeshShading;

  int point_count = kSingleCoordinatePair;
  if (type == kTensorProductPatchMeshShading)
    point_count = kTensorCoordinatePairs;
  else if (type == kCoonsPatchMeshShading)
    point_count = kCoonsCoordinatePairs;

  int color_count = kSingleColorPerPatch;
  if (type == kCoonsPatchMeshShading || type == kTensorProductPatchMeshShading)
    color_count = kQuadColorsPerPatch;

  while (!stream.m_BitStream.IsEOF()) {
    FX_DWORD flag = 0;
    if (type != kLatticeFormGouraudTriangleMeshShading)
      flag = stream.GetFlag();

    if (!bGouraud && flag) {
      point_count -= 4;
      color_count -= 2;
    }

    for (int i = 0; i < point_count; i++) {
      FX_FLOAT x, y;
      stream.GetCoords(x, y);
      if (bStarted) {
        rect.UpdateRect(x, y);
      } else {
        rect.InitRect(x, y);
        bStarted = TRUE;
      }
    }
    stream.m_BitStream.SkipBits(stream.m_nComps * stream.m_nCompBits *
                                color_count);
    if (bGouraud)
      stream.m_BitStream.ByteAlign();
  }
  rect.Transform(pMatrix);
  return rect;
}
