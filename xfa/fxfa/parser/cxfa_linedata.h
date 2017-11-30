// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_LINEDATA_H_
#define XFA_FXFA_PARSER_CXFA_LINEDATA_H_

#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_edgedata.h"

class CXFA_Node;

class CXFA_LineData : public CXFA_DataData {
 public:
  explicit CXFA_LineData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

  XFA_AttributeEnum GetHand() const;
  bool GetSlope() const;
  CXFA_EdgeData GetEdgeData() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_LINEDATA_H_
