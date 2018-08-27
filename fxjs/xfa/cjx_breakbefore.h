// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_BREAKBEFORE_H_
#define FXJS_XFA_CJX_BREAKBEFORE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_BreakBefore;

class CJX_BreakBefore final : public CJX_Node {
 public:
  explicit CJX_BreakBefore(CXFA_BreakBefore* node);
  ~CJX_BreakBefore() override;

  JSE_PROP(leader);
  JSE_PROP(startNew);
  JSE_PROP(target);
  JSE_PROP(targetType);
  JSE_PROP(trailer);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_BREAKBEFORE_H_
