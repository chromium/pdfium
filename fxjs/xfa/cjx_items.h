// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_ITEMS_H_
#define FXJS_XFA_CJX_ITEMS_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Items;

class CJX_Items final : public CJX_Node {
 public:
  explicit CJX_Items(CXFA_Items* node);
  ~CJX_Items() override;

  JSE_PROP(presence);
  JSE_PROP(ref);
  JSE_PROP(save);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_ITEMS_H_
