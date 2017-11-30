// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_BOXDATA_H_
#define XFA_FXFA_PARSER_CXFA_BOXDATA_H_

#include <tuple>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_edgedata.h"
#include "xfa/fxfa/parser/cxfa_filldata.h"
#include "xfa/fxfa/parser/cxfa_margindata.h"

class CXFA_Node;

class CXFA_BoxData : public CXFA_DataData {
 public:
  explicit CXFA_BoxData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

  bool IsArc() const { return GetElementType() == XFA_Element::Arc; }
  bool IsCircular() const;

  XFA_AttributeEnum GetHand() const;
  XFA_AttributeEnum GetPresence() const;
  std::tuple<XFA_AttributeEnum, bool, float> Get3DStyle() const;

  int32_t CountEdges() const;
  CXFA_EdgeData GetEdgeData(int32_t nIndex) const;
  CXFA_FillData GetFillData(bool bModified) const;
  CXFA_MarginData GetMarginData() const;

  std::vector<CXFA_StrokeData> GetStrokes() const;

  pdfium::Optional<int32_t> GetStartAngle() const;
  pdfium::Optional<int32_t> GetSweepAngle() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_BOXDATA_H_
