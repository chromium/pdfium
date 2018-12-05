// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_SUBFORM_H_
#define FXJS_XFA_CJX_SUBFORM_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_container.h"

class CXFA_Delta;

class CJX_Subform final : public CJX_Container {
 public:
  explicit CJX_Subform(CXFA_Node* container);
  ~CJX_Subform() override;

  JSE_METHOD(execCalculate, CJX_Subform);
  JSE_METHOD(execEvent, CJX_Subform);
  JSE_METHOD(execInitialize, CJX_Subform);
  JSE_METHOD(execValidate, CJX_Subform);

  JSE_PROP(instanceIndex);
  JSE_PROP(layout);
  JSE_PROP(locale);
  JSE_PROP(validationMessage);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_SUBFORM_H_
