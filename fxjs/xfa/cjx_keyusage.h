// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_KEYUSAGE_H_
#define FXJS_XFA_CJX_KEYUSAGE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_KeyUsage;

class CJX_KeyUsage final : public CJX_Node {
 public:
  explicit CJX_KeyUsage(CXFA_KeyUsage* node);
  ~CJX_KeyUsage() override;

  JSE_PROP(crlSign);
  JSE_PROP(dataEncipherment);
  JSE_PROP(decipherOnly);
  JSE_PROP(digitalSignature);
  JSE_PROP(encipherOnly);
  JSE_PROP(keyAgreement);
  JSE_PROP(keyCertSign);
  JSE_PROP(keyEncipherment);
  JSE_PROP(nonRepudiation);
  JSE_PROP(type);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_KEYUSAGE_H_
