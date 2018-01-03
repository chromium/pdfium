// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_CAPTIONDATA_H_
#define XFA_FXFA_PARSER_CXFA_CAPTIONDATA_H_

#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_fontdata.h"

class CXFA_Margin;
class CXFA_Node;
class CXFA_Value;

class CXFA_CaptionData : public CXFA_DataData {
 public:
  explicit CXFA_CaptionData(CXFA_Node* pNode);

  bool IsVisible() const;
  bool IsHidden() const;
  XFA_AttributeEnum GetPlacementType() const;
  float GetReserve() const;
  CXFA_Margin* GetMargin() const;
  CXFA_FontData GetFontData() const;
  CXFA_Value* GetValue() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_CAPTIONDATA_H_
