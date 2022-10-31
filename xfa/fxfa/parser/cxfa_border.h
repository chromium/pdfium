// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_BORDER_H_
#define XFA_FXFA_PARSER_CXFA_BORDER_H_

#include "xfa/fxfa/parser/cxfa_rectangle.h"

class CXFA_Border final : public CXFA_Rectangle {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Border() override;

 private:
  CXFA_Border(CXFA_Document* doc, XFA_PacketType packet);
};

#endif  // XFA_FXFA_PARSER_CXFA_BORDER_H_
