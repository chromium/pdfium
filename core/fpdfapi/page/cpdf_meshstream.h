// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_MESHSTREAM_H_
#define CORE_FPDFAPI_PAGE_CPDF_MESHSTREAM_H_

#include <stdint.h>

#include <memory>
#include <tuple>
#include <vector>

#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_StreamAcc;

class CPDF_MeshVertex {
 public:
  CPDF_MeshVertex();
  CPDF_MeshVertex(const CPDF_MeshVertex&);
  ~CPDF_MeshVertex();

  CFX_PointF position;
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
};

class CFX_BitStream;
class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_Function;
class CPDF_Stream;

class CPDF_MeshStream {
 public:
  CPDF_MeshStream(ShadingType type,
                  const std::vector<std::unique_ptr<CPDF_Function>>& funcs,
                  RetainPtr<const CPDF_Stream> pShadingStream,
                  RetainPtr<CPDF_ColorSpace> pCS);
  ~CPDF_MeshStream();

  bool Load();
  void SkipBits(uint32_t nbits);
  void ByteAlign();

  bool IsEOF() const;
  bool CanReadFlag() const;
  bool CanReadCoords() const;
  bool CanReadColor() const;

  uint32_t ReadFlag();
  CFX_PointF ReadCoords();
  std::tuple<float, float, float> ReadColor();

  bool ReadVertex(const CFX_Matrix& pObject2Bitmap,
                  CPDF_MeshVertex* vertex,
                  uint32_t* flag);
  std::vector<CPDF_MeshVertex> ReadVertexRow(const CFX_Matrix& pObject2Bitmap,
                                             int count);

  uint32_t ComponentBits() const { return m_nComponentBits; }
  uint32_t Components() const { return m_nComponents; }

 private:
  static constexpr uint32_t kMaxComponents = 8;

  const ShadingType m_type;
  const std::vector<std::unique_ptr<CPDF_Function>>& m_funcs;
  RetainPtr<const CPDF_Stream> const m_pShadingStream;
  RetainPtr<CPDF_ColorSpace> const m_pCS;
  uint32_t m_nCoordBits = 0;
  uint32_t m_nComponentBits = 0;
  uint32_t m_nFlagBits = 0;
  uint32_t m_nComponents = 0;
  uint32_t m_CoordMax = 0;
  uint32_t m_ComponentMax = 0;
  float m_xmin = 0.0f;
  float m_xmax = 0.0f;
  float m_ymin = 0.0f;
  float m_ymax = 0.0f;
  RetainPtr<CPDF_StreamAcc> m_pStream;
  std::unique_ptr<CFX_BitStream> m_BitStream;
  float m_ColorMin[kMaxComponents] = {};
  float m_ColorMax[kMaxComponents] = {};
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_MESHSTREAM_H_
