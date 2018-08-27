// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_EVENT_H_
#define FXJS_XFA_CJX_EVENT_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Event;

class CJX_Event final : public CJX_Node {
 public:
  explicit CJX_Event(CXFA_Event* node);
  ~CJX_Event() override;

  JSE_PROP(activity);
  JSE_PROP(ref);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_EVENT_H_
