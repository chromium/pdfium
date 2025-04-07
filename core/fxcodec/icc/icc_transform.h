// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_ICC_ICC_TRANSFORM_H_
#define CORE_FXCODEC_ICC_ICC_TRANSFORM_H_

#include <stdint.h>

#include <memory>

#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcrt/span.h"

#if defined(USE_SYSTEM_LCMS2)
#include <lcms2.h>
#else
#include "third_party/lcms/include/lcms2.h"
#endif

namespace fxcodec {

class IccTransform {
 public:
  static std::unique_ptr<IccTransform> CreateTransformSRGB(
      pdfium::span<const uint8_t> span);

  ~IccTransform();

  void Translate(pdfium::span<const float> pSrcValues,
                 pdfium::span<float> pDestValues);
  void TranslateScanline(pdfium::span<uint8_t> pDest,
                         pdfium::span<const uint8_t> pSrc,
                         int pixels);

  int components() const { return src_components_; }
  bool IsNormal() const { return normal_; }

  static bool IsValidIccComponents(int components);

 private:
  IccTransform(cmsHTRANSFORM transform,
               int srcComponents,
               bool bIsLab,
               bool bNormal);

  const cmsHTRANSFORM transform_;
  const int src_components_;
  const bool lab_;
  const bool normal_;
};

}  // namespace fxcodec

#endif  // CORE_FXCODEC_ICC_ICC_TRANSFORM_H_
