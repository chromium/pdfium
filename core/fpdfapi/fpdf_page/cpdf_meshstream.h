// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_MESHSTREAM_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_MESHSTREAM_H_

#include <memory>
#include <vector>

#include "core/fpdfapi/fpdf_page/cpdf_shadingpattern.h"
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
  CPDF_MeshStream(ShadingType type,
                  const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                  CPDF_Stream* pShadingStream,
                  CPDF_ColorSpace* pCS);

  bool Load();

  uint32_t GetFlag();
  void GetCoords(FX_FLOAT& x, FX_FLOAT& y);
  void GetColor(FX_FLOAT& r, FX_FLOAT& g, FX_FLOAT& b);

  uint32_t GetVertex(CPDF_MeshVertex& vertex, CFX_Matrix* pObject2Bitmap);
  FX_BOOL GetVertexRow(CPDF_MeshVertex* vertex,
                       int count,
                       CFX_Matrix* pObject2Bitmap);

  CFX_BitStream* BitStream() { return &m_BitStream; }
  uint32_t CompBits() const { return m_nCompBits; }
  uint32_t comps() const { return m_nComps; }

 private:
  static const uint32_t kMaxResults = 8;

  const ShadingType m_type;
  const std::vector<std::unique_ptr<CPDF_Function>>& m_funcs;
  CPDF_Stream* const m_pShadingStream;
  CPDF_ColorSpace* const m_pCS;
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
  FX_FLOAT m_ColorMin[kMaxResults];
  FX_FLOAT m_ColorMax[kMaxResults];
  CPDF_StreamAcc m_Stream;
  CFX_BitStream m_BitStream;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_MESHSTREAM_H_
