// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_CHECKBUTTON_H_
#define FXJS_XFA_CJX_CHECKBUTTON_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_CheckButton;

class CJX_CheckButton final : public CJX_Node {
 public:
  explicit CJX_CheckButton(CXFA_CheckButton* node);
  ~CJX_CheckButton() override;

  JSE_PROP(allowNeutral);
  JSE_PROP(mark);
  JSE_PROP(shape);
  JSE_PROP(size);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_CHECKBUTTON_H_
