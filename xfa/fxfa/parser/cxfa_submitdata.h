// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_SUBMITDATA_H_
#define XFA_FXFA_PARSER_CXFA_SUBMITDATA_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_SubmitData : public CXFA_DataData {
 public:
  explicit CXFA_SubmitData(CXFA_Node* pNode);

  bool IsSubmitEmbedPDF() const;
  XFA_AttributeEnum GetSubmitFormat() const;
  WideString GetSubmitTarget() const;
  WideString GetSubmitXDPContent() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_SUBMITDATA_H_
