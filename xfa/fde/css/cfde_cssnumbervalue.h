// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSNUMBERVALUE_H_
#define XFA_FDE_CSS_CFDE_CSSNUMBERVALUE_H_

#include "core/fxcrt/fx_system.h"
#include "xfa/fde/css/cfde_cssvalue.h"

enum class FDE_CSSNumberType {
  Number,
  Percent,
  EMS,
  EXS,
  Pixels,
  CentiMeters,
  MilliMeters,
  Inches,
  Points,
  Picas,
};

class CFDE_CSSNumberValue : public CFDE_CSSValue {
 public:
  CFDE_CSSNumberValue(FDE_CSSNumberType type, FX_FLOAT value);
  ~CFDE_CSSNumberValue() override;

  FX_FLOAT Value() const { return value_; }
  FDE_CSSNumberType Kind() const { return type_; }

  FX_FLOAT Apply(FX_FLOAT percentBase) const;

 private:
  FDE_CSSNumberType type_;
  FX_FLOAT value_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSNUMBERVALUE_H_
