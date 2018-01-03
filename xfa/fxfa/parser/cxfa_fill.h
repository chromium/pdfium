// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_FILL_H_
#define XFA_FXFA_PARSER_CXFA_FILL_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Linear;
class CXFA_Pattern;
class CXFA_Radial;
class CXFA_Stipple;

class CXFA_Fill : public CXFA_Node {
 public:
  CXFA_Fill(CXFA_Document* doc, XFA_PacketType packet);
  ~CXFA_Fill() override;

  bool IsVisible();

  FX_ARGB GetColor(bool bText);
  void SetColor(FX_ARGB color);

  XFA_Element GetFillType() const;

  XFA_AttributeEnum GetPatternType();
  FX_ARGB GetPatternColor();

  XFA_AttributeEnum GetLinearType();
  FX_ARGB GetLinearColor();

  int32_t GetStippleRate();
  FX_ARGB GetStippleColor();

  bool IsRadialToEdge();
  FX_ARGB GetRadialColor();

 private:
  CXFA_Stipple* GetStipple();
  CXFA_Radial* GetRadial();
  CXFA_Linear* GetLinear();
  CXFA_Pattern* GetPattern();
};

#endif  // XFA_FXFA_PARSER_CXFA_FILL_H_
