// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_PARA_H_
#define FXJS_XFA_CJX_PARA_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Para;

class CJX_Para final : public CJX_Node {
 public:
  explicit CJX_Para(CXFA_Para* node);
  ~CJX_Para() override;

  JSE_PROP(hAlign);
  JSE_PROP(lineHeight);
  JSE_PROP(marginLeft);
  JSE_PROP(marginRight);
  JSE_PROP(preserve);
  JSE_PROP(radixOffset);
  JSE_PROP(spaceAbove);
  JSE_PROP(spaceBelow);
  JSE_PROP(tabDefault);
  JSE_PROP(tabStops);
  JSE_PROP(textIndent);
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(vAlign);
};

#endif  // FXJS_XFA_CJX_PARA_H_
