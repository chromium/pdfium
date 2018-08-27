// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_BIND_H_
#define FXJS_XFA_CJX_BIND_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Bind;

class CJX_Bind final : public CJX_Node {
 public:
  explicit CJX_Bind(CXFA_Bind* node);
  ~CJX_Bind() override;

  JSE_PROP(contentType);
  JSE_PROP(match);
  JSE_PROP(ref);
  JSE_PROP(transferEncoding);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_BIND_H_
