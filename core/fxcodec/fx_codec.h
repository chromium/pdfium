// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_FX_CODEC_H_
#define CORE_FXCODEC_FX_CODEC_H_

#include <stdint.h>

#include "third_party/abseil-cpp/absl/types/optional.h"

namespace fxcodec {

#ifdef PDF_ENABLE_XFA
class CFX_DIBAttribute {
 public:
  // Not an enum class yet because we still blindly cast integer results
  // from third-party libraries to this type.
  enum ResUnit : uint16_t {
    kResUnitNone = 0,
    kResUnitInch,
    kResUnitCentimeter,
    kResUnitMeter
  };

  CFX_DIBAttribute();
  ~CFX_DIBAttribute();

  int32_t m_nXDPI = -1;
  int32_t m_nYDPI = -1;
  ResUnit m_wDPIUnit = kResUnitNone;
};
#endif  // PDF_ENABLE_XFA

void ReverseRGB(uint8_t* pDestBuf, const uint8_t* pSrcBuf, int pixels);

uint32_t CalculatePitch8OrDie(uint32_t bpc, uint32_t components, int width);
uint32_t CalculatePitch32OrDie(int bpp, int width);
absl::optional<uint32_t> CalculatePitch8(uint32_t bpc,
                                         uint32_t components,
                                         int width);
absl::optional<uint32_t> CalculatePitch32(int bpp, int width);

}  // namespace fxcodec

#ifdef PDF_ENABLE_XFA
using CFX_DIBAttribute = fxcodec::CFX_DIBAttribute;
#endif

#endif  // CORE_FXCODEC_FX_CODEC_H_
