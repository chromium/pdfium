// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_CAPTION_H_
#define XFA_FXFA_PARSER_CXFA_CAPTION_H_

#include "xfa/fxfa/parser/cxfa_fontdata.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Margin;
class CXFA_Value;

class CXFA_Caption : public CXFA_Node {
 public:
  CXFA_Caption(CXFA_Document* doc, XFA_PacketType packet);
  ~CXFA_Caption() override;

  bool IsVisible();
  bool IsHidden();
  XFA_AttributeEnum GetPlacementType();
  float GetReserve() const;
  CXFA_Margin* GetMargin();
  CXFA_FontData GetFontData();
  CXFA_Value* GetValue();
};

#endif  // XFA_FXFA_PARSER_CXFA_CAPTION_H_
