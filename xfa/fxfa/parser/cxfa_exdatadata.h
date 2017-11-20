// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_EXDATADATA_H_
#define XFA_FXFA_PARSER_CXFA_EXDATADATA_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_ExDataData : public CXFA_DataData {
 public:
  explicit CXFA_ExDataData(CXFA_Node* pNode);

  void SetContentType(const WideString& wsContentType);
};

#endif  // XFA_FXFA_PARSER_CXFA_EXDATADATA_H_
