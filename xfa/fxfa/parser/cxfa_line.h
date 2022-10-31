// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_LINE_H_
#define XFA_FXFA_PARSER_CXFA_LINE_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Edge;

class CXFA_Line final : public CXFA_Node {
 public:
  static CXFA_Line* FromNode(CXFA_Node* pNode);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Line() override;

  XFA_AttributeValue GetHand();
  bool GetSlope();
  CXFA_Edge* GetEdgeIfExists();

 private:
  CXFA_Line(CXFA_Document* doc, XFA_PacketType packet);
};

#endif  // XFA_FXFA_PARSER_CXFA_LINE_H_
