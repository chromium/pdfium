// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_FONTDATA_H_
#define XFA_FXFA_PARSER_CXFA_FONTDATA_H_

#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_FontData : public CXFA_DataData {
 public:
  explicit CXFA_FontData(CXFA_Node* pNode);

  float GetBaselineShift();
  float GetHorizontalScale();
  float GetVerticalScale();
  float GetLetterSpacing();
  int32_t GetLineThrough();
  int32_t GetUnderline();
  int32_t GetUnderlinePeriod();
  float GetFontSize();
  void GetTypeface(WideStringView& wsTypeFace);

  bool IsBold();
  bool IsItalic();

  FX_ARGB GetColor();
  void SetColor(FX_ARGB color);
};

#endif  // XFA_FXFA_PARSER_CXFA_FONTDATA_H_
