// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_MDP_H_
#define FXJS_XFA_CJX_MDP_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Mdp;

class CJX_Mdp final : public CJX_Node {
 public:
  explicit CJX_Mdp(CXFA_Mdp* node);
  ~CJX_Mdp() override;

  JSE_PROP(permissions);
  JSE_PROP(signatureType);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_MDP_H_
