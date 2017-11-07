// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_EVENTDATA_H_
#define XFA_FXFA_PARSER_CXFA_EVENTDATA_H_

#include <stdint.h>

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_data.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/cxfa_submit.h"

class CXFA_Node;

class CXFA_EventData : public CXFA_Data {
 public:
  explicit CXFA_EventData(CXFA_Node* pNode);

  int32_t GetActivity();
  XFA_Element GetEventType() const;
  CXFA_Script GetScript() const;
  CXFA_Submit GetSubmit() const;
  void GetRef(WideStringView& wsRef);
  void GetSignDataTarget(WideString& wsTarget);
};

#endif  // XFA_FXFA_PARSER_CXFA_EVENTDATA_H_
