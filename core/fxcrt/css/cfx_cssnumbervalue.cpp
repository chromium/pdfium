// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssnumbervalue.h"

#include <math.h>

CFX_CSSNumberValue::CFX_CSSNumberValue(Unit unit, float value)
    : CFX_CSSValue(PrimitiveType::kNumber), unit_(unit), value_(value) {
  if (unit_ == Unit::kNumber && fabs(value_) < 0.001f)
    value_ = 0.0f;
}

CFX_CSSNumberValue::~CFX_CSSNumberValue() = default;

float CFX_CSSNumberValue::Apply(float percentBase) const {
  switch (unit_) {
    case Unit::kPixels:
    case Unit::kNumber:
      return value_ * 72 / 96;
    case Unit::kEMS:
    case Unit::kEXS:
      return value_ * percentBase;
    case Unit::kPercent:
      return value_ * percentBase / 100.0f;
    case Unit::kCentiMeters:
      return value_ * 28.3464f;
    case Unit::kMilliMeters:
      return value_ * 2.8346f;
    case Unit::kInches:
      return value_ * 72.0f;
    case Unit::kPicas:
      return value_ / 12.0f;
    case Unit::kPoints:
      return value_;
  }
  return value_;
}
