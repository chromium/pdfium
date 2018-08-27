// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_VALUE_H_
#define FXJS_XFA_CJX_VALUE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Value;

class CJX_Value final : public CJX_Node {
 public:
  explicit CJX_Value(CXFA_Value* node);
  ~CJX_Value() override;

  JSE_PROP(override);
  JSE_PROP(relevant);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_VALUE_H_
