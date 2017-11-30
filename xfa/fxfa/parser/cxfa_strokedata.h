// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_STROKEDATA_H_
#define XFA_FXFA_PARSER_CXFA_STROKEDATA_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

enum StrokeSameStyle {
  XFA_STROKE_SAMESTYLE_NoPresence = 1,
  XFA_STROKE_SAMESTYLE_Corner = 2
};

class CXFA_Node;

class CXFA_StrokeData : public CXFA_DataData {
 public:
  CXFA_StrokeData() : CXFA_StrokeData(nullptr) {}
  explicit CXFA_StrokeData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

  bool IsCorner() const { return GetElementType() == XFA_Element::Corner; }
  bool IsVisible() const;
  bool IsInverted() const;

  XFA_AttributeEnum GetCapType() const;
  XFA_AttributeEnum GetStrokeType() const;
  XFA_AttributeEnum GetJoinType() const;
  float GetRadius() const;
  float GetThickness() const;

  CXFA_Measurement GetMSThickness() const;
  void SetMSThickness(CXFA_Measurement msThinkness);

  FX_ARGB GetColor() const;
  void SetColor(FX_ARGB argb);

  bool SameStyles(CXFA_StrokeData stroke, uint32_t dwFlags) const;
};

#endif  // XFA_FXFA_PARSER_CXFA_STROKEDATA_H_
