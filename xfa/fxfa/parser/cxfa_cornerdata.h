// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_CORNERDATA_H_
#define XFA_FXFA_PARSER_CXFA_CORNERDATA_H_

#include "xfa/fxfa/parser/cxfa_strokedata.h"

class CXFA_Node;

class CXFA_CornerData : public CXFA_StrokeData {
 public:
  explicit CXFA_CornerData(CXFA_Node* pNode) : CXFA_StrokeData(pNode) {}
};

#endif  // XFA_FXFA_PARSER_CXFA_CORNERDATA_H_
