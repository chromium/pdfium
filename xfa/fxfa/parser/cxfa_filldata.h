// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_FILLDATA_H_
#define XFA_FXFA_PARSER_CXFA_FILLDATA_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Linear;
class CXFA_Node;
class CXFA_Pattern;
class CXFA_Radial;
class CXFA_Stipple;

class CXFA_FillData : public CXFA_DataData {
 public:
  explicit CXFA_FillData(CXFA_Node* pNode);
  ~CXFA_FillData() override;

  bool IsVisible() const;

  FX_ARGB GetColor(bool bText) const;
  void SetColor(FX_ARGB color);

  XFA_Element GetFillType() const;

  XFA_AttributeEnum GetPatternType() const;
  FX_ARGB GetPatternColor() const;

  XFA_AttributeEnum GetLinearType() const;
  FX_ARGB GetLinearColor() const;

  int32_t GetStippleRate() const;
  FX_ARGB GetStippleColor() const;

  bool IsRadialToEdge() const;
  FX_ARGB GetRadialColor() const;

 private:
  CXFA_Stipple* GetStipple() const;
  CXFA_Radial* GetRadial() const;
  CXFA_Linear* GetLinear() const;
  CXFA_Pattern* GetPattern() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_FILLDATA_H_
