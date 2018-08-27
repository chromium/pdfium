// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_VALIDATE_H_
#define FXJS_XFA_CJX_VALIDATE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Validate;

class CJX_Validate final : public CJX_Node {
 public:
  explicit CJX_Validate(CXFA_Validate* node);
  ~CJX_Validate() override;

  JSE_PROP(formatTest);
  JSE_PROP(nullTest);
  JSE_PROP(scriptTest);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_VALIDATE_H_
