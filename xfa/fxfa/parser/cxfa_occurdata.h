// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_OCCURDATA_H_
#define XFA_FXFA_PARSER_CXFA_OCCURDATA_H_

#include <tuple>

#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_OccurData : public CXFA_DataData {
 public:
  explicit CXFA_OccurData(CXFA_Node* pNode);

  int32_t GetMax() const;
  void SetMax(int32_t iMax);

  int32_t GetMin() const;
  void SetMin(int32_t iMin);

  std::tuple<int32_t, int32_t, int32_t> GetOccurInfo() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_OCCURDATA_H_
