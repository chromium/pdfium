// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_CORNER_H_
#define FXJS_XFA_CJX_CORNER_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Corner;

class CJX_Corner final : public CJX_Node {
 public:
  explicit CJX_Corner(CXFA_Corner* node);
  ~CJX_Corner() override;

  JSE_PROP(inverted);
  JSE_PROP(join);
  JSE_PROP(presence);
  JSE_PROP(radius);
  JSE_PROP(stroke);
  JSE_PROP(thickness);
  JSE_PROP(usehref);
  JSE_PROP(use);
};

#endif  // FXJS_XFA_CJX_CORNER_H_
