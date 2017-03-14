// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_PARA_H_
#define XFA_FXFA_PARSER_CXFA_PARA_H_

#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_data.h"

class CXFA_Node;

class CXFA_Para : public CXFA_Data {
 public:
  explicit CXFA_Para(CXFA_Node* pNode);

  int32_t GetHorizontalAlign();
  int32_t GetVerticalAlign();
  float GetLineHeight();
  float GetMarginLeft();
  float GetMarginRight();
  float GetSpaceAbove();
  float GetSpaceBelow();
  float GetTextIndent();
};

#endif  // XFA_FXFA_PARSER_CXFA_PARA_H_
