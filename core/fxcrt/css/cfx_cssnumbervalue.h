// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSNUMBERVALUE_H_
#define CORE_FXCRT_CSS_CFX_CSSNUMBERVALUE_H_

#include "core/fxcrt/css/cfx_cssvalue.h"

class CFX_CSSNumberValue final : public CFX_CSSValue {
 public:
  enum class Unit {
    kNumber,
    kPercent,
    kEMS,
    kEXS,
    kPixels,
    kCentiMeters,
    kMilliMeters,
    kInches,
    kPoints,
    kPicas,
  };

  CFX_CSSNumberValue(Unit unit, float value);
  ~CFX_CSSNumberValue() override;

  Unit unit() const { return unit_; }
  float value() const { return value_; }
  float Apply(float percentBase) const;

 private:
  Unit unit_;
  float value_;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSNUMBERVALUE_H_
