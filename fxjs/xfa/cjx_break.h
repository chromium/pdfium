// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_BREAK_H_
#define FXJS_XFA_CJX_BREAK_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Break;

class CJX_Break final : public CJX_Node {
 public:
  explicit CJX_Break(CXFA_Break* node);
  ~CJX_Break() override;

  JSE_PROP(after);
  JSE_PROP(afterTarget);
  JSE_PROP(before);
  JSE_PROP(beforeTarget);
  JSE_PROP(bookendLeader);
  JSE_PROP(bookendTrailer);
  JSE_PROP(overflowLeader);
  JSE_PROP(overflowTarget);
  JSE_PROP(overflowTrailer);
  JSE_PROP(startNew);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_BREAK_H_
