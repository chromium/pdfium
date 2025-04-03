// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_MEASUREMENT_H_
#define XFA_FXFA_PARSER_CXFA_MEASUREMENT_H_

#include "core/fxcrt/widestring.h"
#include "xfa/fxfa/fxfa_basic.h"

class CXFA_Measurement {
 public:
  explicit CXFA_Measurement(WideStringView wsMeasure);
  CXFA_Measurement();
  CXFA_Measurement(float fValue, XFA_Unit eUnit);

  static XFA_Unit GetUnitFromString(WideStringView wsUnit);

  void Set(float fValue, XFA_Unit eUnit) {
    value_ = fValue;
    unit_ = eUnit;
  }

  XFA_Unit GetUnit() const { return unit_; }
  float GetValue() const { return value_; }

  WideString ToString() const;
  float ToUnit(XFA_Unit eUnit) const;

 private:
  void SetString(WideStringView wsMeasure);
  bool ToUnitInternal(XFA_Unit eUnit, float* fValue) const;

  float value_ = 0.0f;
  XFA_Unit unit_ = XFA_Unit::Percent;
};

#endif  // XFA_FXFA_PARSER_CXFA_MEASUREMENT_H_
