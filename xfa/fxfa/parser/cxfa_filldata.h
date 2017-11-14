// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_FILLDATA_H_
#define XFA_FXFA_PARSER_CXFA_FILLDATA_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_FillData : public CXFA_DataData {
 public:
  explicit CXFA_FillData(CXFA_Node* pNode);
  ~CXFA_FillData();

  int32_t GetPresence();
  FX_ARGB GetColor(bool bText = false);
  XFA_Element GetFillType();
  int32_t GetPattern(FX_ARGB& foreColor);
  int32_t GetStipple(FX_ARGB& stippleColor);
  int32_t GetLinear(FX_ARGB& endColor);
  int32_t GetRadial(FX_ARGB& endColor);
  void SetColor(FX_ARGB color);
};

#endif  // XFA_FXFA_PARSER_CXFA_FILLDATA_H_
