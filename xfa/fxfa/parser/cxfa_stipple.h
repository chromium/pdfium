// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_STIPPLE_H_
#define XFA_FXFA_PARSER_CXFA_STIPPLE_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Color;

class CXFA_Stipple : public CXFA_Node {
 public:
  static int32_t GetDefaultRate() { return 50; }

  CXFA_Stipple(CXFA_Document* doc, XFA_PacketType packet);
  ~CXFA_Stipple() override;

  CXFA_Color* GetColorIfExists();
  int32_t GetRate();
};

#endif  // XFA_FXFA_PARSER_CXFA_STIPPLE_H_
