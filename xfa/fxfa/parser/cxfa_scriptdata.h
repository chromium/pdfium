// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_SCRIPTDATA_H_
#define XFA_FXFA_PARSER_CXFA_SCRIPTDATA_H_

#include <stdint.h>

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_ScriptData : public CXFA_DataData {
 public:
  enum class Type {
    Formcalc = 0,
    Javascript,
    Unknown,
  };

  explicit CXFA_ScriptData(CXFA_Node* pNode);

  Type GetContentType() const;
  XFA_AttributeEnum GetRunAt() const;
  WideString GetExpression() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_SCRIPTDATA_H_
