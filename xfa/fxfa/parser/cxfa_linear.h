// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_LINEAR_H_
#define XFA_FXFA_PARSER_CXFA_LINEAR_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Color;

class CXFA_Linear : public CXFA_Node {
 public:
  static constexpr XFA_AttributeEnum kDefaultType = XFA_AttributeEnum::ToRight;

  CXFA_Linear(CXFA_Document* doc, XFA_PacketType packet);
  ~CXFA_Linear() override;

  XFA_AttributeEnum GetType();
  CXFA_Color* GetColorIfExists();
};

#endif  // XFA_FXFA_PARSER_CXFA_LINEAR_H_
