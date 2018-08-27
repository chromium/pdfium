// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_ARC_H_
#define FXJS_XFA_CJX_ARC_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Arc;

class CJX_Arc final : public CJX_Node {
 public:
  explicit CJX_Arc(CXFA_Arc* node);
  ~CJX_Arc() override;

  JSE_PROP(circular);
  JSE_PROP(hand);
  JSE_PROP(startAngle);
  JSE_PROP(sweepAngle);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_ARC_H_
