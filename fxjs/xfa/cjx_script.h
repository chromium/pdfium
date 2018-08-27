// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_SCRIPT_H_
#define FXJS_XFA_CJX_SCRIPT_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Script;

class CJX_Script final : public CJX_Node {
 public:
  explicit CJX_Script(CXFA_Script* node);
  ~CJX_Script() override;

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(binding);
  JSE_PROP(contentType);
  JSE_PROP(runAt);
  JSE_PROP(stateless);
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(value);
};

#endif  // FXJS_XFA_CJX_SCRIPT_H_
