// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_ENCRYPT_H_
#define FXJS_XFA_CJX_ENCRYPT_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Encrypt;

class CJX_Encrypt final : public CJX_Node {
 public:
  explicit CJX_Encrypt(CXFA_Encrypt* node);
  ~CJX_Encrypt() override;

  JSE_PROP(format);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_ENCRYPT_H_
