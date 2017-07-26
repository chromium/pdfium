// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_MEASUREMENT_H_
#define XFA_FXFA_PARSER_CXFA_MEASUREMENT_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/fxfa_basic.h"

class CXFA_Measurement {
 public:
  explicit CXFA_Measurement(const CFX_WideStringC& wsMeasure);
  CXFA_Measurement();
  CXFA_Measurement(float fValue, XFA_UNIT eUnit);

  static XFA_UNIT GetUnitFromString(const CFX_WideStringC& wsUnit);

  void Set(float fValue, XFA_UNIT eUnit) {
    m_fValue = fValue;
    m_eUnit = eUnit;
  }

  XFA_UNIT GetUnit() const { return m_eUnit; }
  float GetValue() const { return m_fValue; }

  bool ToString(CFX_WideString* wsMeasure) const;
  float ToUnit(XFA_UNIT eUnit) const;

 private:
  void SetString(const CFX_WideStringC& wsMeasure);
  bool ToUnitInternal(XFA_UNIT eUnit, float* fValue) const;

  float m_fValue;
  XFA_UNIT m_eUnit;
};

#endif  // XFA_FXFA_PARSER_CXFA_MEASUREMENT_H_
