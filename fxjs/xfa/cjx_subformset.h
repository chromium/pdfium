// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_SUBFORMSET_H_
#define FXJS_XFA_CJX_SUBFORMSET_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_container.h"

class CXFA_SubformSet;

class CJX_SubformSet final : public CJX_Container {
 public:
  explicit CJX_SubformSet(CXFA_SubformSet* node);
  ~CJX_SubformSet() override;

  JSE_PROP(instanceIndex);
  JSE_PROP(relation);
  JSE_PROP(relevant);
  JSE_PROP(use);
  JSE_PROP(usehref);
};

#endif  // FXJS_XFA_CJX_SUBFORMSET_H_
