// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_MANIFEST_H_
#define FXJS_XFA_CJX_MANIFEST_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Manifest;

class CJX_Manifest final : public CJX_Node {
 public:
  explicit CJX_Manifest(CXFA_Manifest* manifest);
  ~CJX_Manifest() override;

  JSE_METHOD(evaluate, CJX_Manifest);

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(action);
  JSE_PROP(use);
  JSE_PROP(usehref);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_MANIFEST_H_
