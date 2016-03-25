// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_MESHSTREAM_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_MESHSTREAM_H_

#include "core/fpdfapi/fpdf_parser/include/cpdf_stream_acc.h"
#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_system.h"

struct CPDF_MeshVertex {
  FX_FLOAT x;
  FX_FLOAT y;
  FX_FLOAT r;
  FX_FLOAT g;
  FX_FLOAT b;
};

class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_Function;
class CPDF_Stream;

class CPDF_MeshStream {
 public:
  FX_BOOL Load(CPDF_Stream* pShadingStream,
               CPDF_Function** pFuncs,
               int nFuncs,
               CPDF_ColorSpace* pCS);

  uint32_t GetFlag();

  void GetCoords(FX_FLOAT& x, FX_FLOAT& y);
  void GetColor(FX_FLOAT& r, FX_FLOAT& g, FX_FLOAT& b);

  uint32_t GetVertex(CPDF_MeshVertex& vertex, CFX_Matrix* pObject2Bitmap);
  FX_BOOL GetVertexRow(CPDF_MeshVertex* vertex,
                       int count,
                       CFX_Matrix* pObject2Bitmap);

  CPDF_Function** m_pFuncs;
  CPDF_ColorSpace* m_pCS;
  uint32_t m_nFuncs;
  uint32_t m_nCoordBits;
  uint32_t m_nCompBits;
  uint32_t m_nFlagBits;
  uint32_t m_nComps;
  uint32_t m_CoordMax;
  uint32_t m_CompMax;
  FX_FLOAT m_xmin;
  FX_FLOAT m_xmax;
  FX_FLOAT m_ymin;
  FX_FLOAT m_ymax;
  FX_FLOAT m_ColorMin[8];
  FX_FLOAT m_ColorMax[8];
  CPDF_StreamAcc m_Stream;
  CFX_BitStream m_BitStream;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_MESHSTREAM_H_
