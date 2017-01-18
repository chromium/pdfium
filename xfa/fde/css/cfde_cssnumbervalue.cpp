// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssnumbervalue.h"

CFDE_CSSNumberValue::CFDE_CSSNumberValue(FDE_CSSNumberType type, FX_FLOAT value)
    : CFDE_CSSValue(FDE_CSSPrimitiveType::Number), type_(type), value_(value) {
  if (type_ == FDE_CSSNumberType::Number && FXSYS_fabs(value_) < 0.001f)
    value_ = 0.0f;
}

CFDE_CSSNumberValue::~CFDE_CSSNumberValue() {}

FX_FLOAT CFDE_CSSNumberValue::Apply(FX_FLOAT percentBase) const {
  switch (type_) {
    case FDE_CSSNumberType::Pixels:
    case FDE_CSSNumberType::Number:
      return value_ * 72 / 96;
    case FDE_CSSNumberType::EMS:
    case FDE_CSSNumberType::EXS:
      return value_ * percentBase;
    case FDE_CSSNumberType::Percent:
      return value_ * percentBase / 100.0f;
    case FDE_CSSNumberType::CentiMeters:
      return value_ * 28.3464f;
    case FDE_CSSNumberType::MilliMeters:
      return value_ * 2.8346f;
    case FDE_CSSNumberType::Inches:
      return value_ * 72.0f;
    case FDE_CSSNumberType::Picas:
      return value_ / 12.0f;
    case FDE_CSSNumberType::Points:
      return value_;
  }
  return value_;
}
