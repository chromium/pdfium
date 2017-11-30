// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_CALCULATEDATA_H_
#define XFA_FXFA_PARSER_CXFA_CALCULATEDATA_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_scriptdata.h"

class CXFA_Node;

class CXFA_CalculateData : public CXFA_DataData {
 public:
  explicit CXFA_CalculateData(CXFA_Node* pNode);

  XFA_AttributeEnum GetOverride() const;
  CXFA_ScriptData GetScriptData() const;
  WideString GetMessageText() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_CALCULATEDATA_H_
