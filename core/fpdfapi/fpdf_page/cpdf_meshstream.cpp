// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_meshstream.h"

#include "core/fpdfapi/fpdf_page/include/cpdf_colorspace.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"

namespace {

// See PDF Reference 1.7, page 315, table 4.32. (Also table 4.33 and 4.34)
bool IsValidBitsPerComponent(uint32_t x) {
  switch (x) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 12:
    case 16:
      return true;
    default:
      return false;
  }
}

// Same references as IsValidBitsPerComponent() above.
bool IsValidBitsPerCoordinate(uint32_t x) {
  switch (x) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 12:
    case 16:
    case 24:
    case 32:
      return true;
    default:
      return false;
  }
}

}  // namespace

CPDF_MeshStream::CPDF_MeshStream(
    const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
    CPDF_ColorSpace* pCS)
    : m_funcs(funcs), m_pCS(pCS) {}

bool CPDF_MeshStream::Load(CPDF_Stream* pShadingStream) {
  m_Stream.LoadAllData(pShadingStream);
  m_BitStream.Init(m_Stream.GetData(), m_Stream.GetSize());
  CPDF_Dictionary* pDict = pShadingStream->GetDict();
  m_nCoordBits = pDict->GetIntegerBy("BitsPerCoordinate");
  if (!IsValidBitsPerCoordinate(m_nCoordBits))
    return false;

  m_nCompBits = pDict->GetIntegerBy("BitsPerComponent");
  if (!IsValidBitsPerComponent(m_nCompBits))
    return false;

  m_nFlagBits = pDict->GetIntegerBy("BitsPerFlag");
  uint32_t nComps = m_pCS->CountComponents();
  if (nComps > 8)
    return false;

  m_nComps = m_funcs.empty() ? nComps : 1;
  m_CoordMax = m_nCoordBits == 32 ? -1 : (1 << m_nCoordBits) - 1;
  m_CompMax = (1 << m_nCompBits) - 1;
  CPDF_Array* pDecode = pDict->GetArrayBy("Decode");
  if (!pDecode || pDecode->GetCount() != 4 + m_nComps * 2)
    return false;

  m_xmin = pDecode->GetNumberAt(0);
  m_xmax = pDecode->GetNumberAt(1);
  m_ymin = pDecode->GetNumberAt(2);
  m_ymax = pDecode->GetNumberAt(3);
  for (uint32_t i = 0; i < m_nComps; ++i) {
    m_ColorMin[i] = pDecode->GetNumberAt(i * 2 + 4);
    m_ColorMax[i] = pDecode->GetNumberAt(i * 2 + 5);
  }
  return true;
}

uint32_t CPDF_MeshStream::GetFlag() {
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
  static const int kMaxResults = 8;
  FX_FLOAT color_value[kMaxResults];
  for (uint32_t i = 0; i < m_nComps; ++i) {
    color_value[i] = m_ColorMin[i] +
                     m_BitStream.GetBits(m_nCompBits) *
                         (m_ColorMax[i] - m_ColorMin[i]) / m_CompMax;
  }
  if (m_funcs.empty()) {
    m_pCS->GetRGB(color_value, r, g, b);
    return;
  }

  FX_FLOAT result[kMaxResults];
  FXSYS_memset(result, 0, sizeof(result));
  int nResults;
  for (const auto& func : m_funcs) {
    if (func && func->CountOutputs() <= kMaxResults)
      func->Call(color_value, 1, result, nResults);
  }
  m_pCS->GetRGB(result, r, g, b);
}

uint32_t CPDF_MeshStream::GetVertex(CPDF_MeshVertex& vertex,
                                    CFX_Matrix* pObject2Bitmap) {
  uint32_t flag = GetFlag();
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
    if (m_BitStream.IsEOF())
      return FALSE;

    GetCoords(vertex[i].x, vertex[i].y);
    pObject2Bitmap->Transform(vertex[i].x, vertex[i].y);
    GetColor(vertex[i].r, vertex[i].g, vertex[i].b);
    m_BitStream.ByteAlign();
  }
  return TRUE;
}
