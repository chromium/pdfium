// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_OCCURDATA_H_
#define XFA_FXFA_PARSER_CXFA_OCCURDATA_H_

#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_OccurData : public CXFA_DataData {
 public:
  explicit CXFA_OccurData(CXFA_Node* pNode);

  int32_t GetMax();
  int32_t GetMin();
  bool GetOccurInfo(int32_t& iMin, int32_t& iMax, int32_t& iInit);
  void SetMax(int32_t iMax);
  void SetMin(int32_t iMin);
};

#endif  // XFA_FXFA_PARSER_CXFA_OCCURDATA_H_
