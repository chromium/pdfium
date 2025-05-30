// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_MESHSTREAM_H_
#define CORE_FPDFAPI_PAGE_CPDF_MESHSTREAM_H_

#include <stdint.h>

#include <array>
#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CPDF_StreamAcc;

class CPDF_MeshVertex {
 public:
  CPDF_MeshVertex();
  CPDF_MeshVertex(const CPDF_MeshVertex&);
  ~CPDF_MeshVertex();

  CFX_PointF position;
  FX_RGB_STRUCT<float> rgb = {};
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

  uint32_t ReadFlag() const;
  CFX_PointF ReadCoords() const;
  FX_RGB_STRUCT<float> ReadColor() const;

  bool ReadVertex(const CFX_Matrix& pObject2Bitmap,
                  CPDF_MeshVertex* vertex,
                  uint32_t* flag);
  std::vector<CPDF_MeshVertex> ReadVertexRow(const CFX_Matrix& pObject2Bitmap,
                                             int count);

  uint32_t ComponentBits() const { return component_bits_; }
  uint32_t Components() const { return components_; }

  float component_min(size_t component_index) const {
    return color_min_[component_index];
  }
  float component_max(size_t component_index) const {
    return color_max_[component_index];
  }

 private:
  static constexpr uint32_t kMaxComponents = 8;

  const ShadingType type_;
  const std::vector<std::unique_ptr<CPDF_Function>>& funcs_;
  RetainPtr<const CPDF_Stream> const shading_stream_;
  RetainPtr<CPDF_ColorSpace> const cs_;
  uint32_t coord_bits_ = 0;
  uint32_t component_bits_ = 0;
  uint32_t flag_bits_ = 0;
  uint32_t components_ = 0;
  uint32_t coord_max_ = 0;
  uint32_t component_max_ = 0;
  float xmin_ = 0.0f;
  float xmax_ = 0.0f;
  float ymin_ = 0.0f;
  float ymax_ = 0.0f;
  RetainPtr<CPDF_StreamAcc> stream_;
  std::unique_ptr<CFX_BitStream> bit_stream_;
  std::array<float, kMaxComponents> color_min_ = {};
  std::array<float, kMaxComponents> color_max_ = {};
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_MESHSTREAM_H_
