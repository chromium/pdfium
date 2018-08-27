// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_UPDATE_H_
#define FXJS_XFA_CJX_UPDATE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_textnode.h"

class CXFA_Update;

class CJX_Update final : public CJX_TextNode {
 public:
  explicit CJX_Update(CXFA_Update* node);
  ~CJX_Update() override;

  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_UPDATE_H_
