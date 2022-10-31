// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_CHECKBUTTON_H_
#define XFA_FXFA_PARSER_CXFA_CHECKBUTTON_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_CheckButton final : public CXFA_Node {
 public:
  static CXFA_CheckButton* FromNode(CXFA_Node* pNode);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_CheckButton() override;

  XFA_FFWidgetType GetDefaultFFWidgetType() const override;

  bool IsRound();
  bool IsAllowNeutral();
  XFA_AttributeValue GetMark();

 private:
  CXFA_CheckButton(CXFA_Document* doc, XFA_PacketType packet);
};

#endif  // XFA_FXFA_PARSER_CXFA_CHECKBUTTON_H_
