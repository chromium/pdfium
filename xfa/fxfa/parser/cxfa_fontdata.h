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

  float GetBaselineShift() const;
  float GetHorizontalScale() const;
  float GetVerticalScale() const;
  float GetLetterSpacing() const;
  int32_t GetLineThrough() const;
  int32_t GetUnderline() const;
  XFA_AttributeEnum GetUnderlinePeriod() const;
  float GetFontSize() const;
  WideString GetTypeface() const;

  bool IsBold() const;
  bool IsItalic() const;

  FX_ARGB GetColor() const;
  void SetColor(FX_ARGB color);
};

#endif  // XFA_FXFA_PARSER_CXFA_FONTDATA_H_
