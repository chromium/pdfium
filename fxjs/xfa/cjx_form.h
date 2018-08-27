// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_FORM_H_
#define FXJS_XFA_CJX_FORM_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_model.h"

class CXFA_Form;

class CJX_Form final : public CJX_Model {
 public:
  explicit CJX_Form(CXFA_Form* form);
  ~CJX_Form() override;

  JSE_METHOD(execCalculate, CJX_Form);
  JSE_METHOD(execInitialize, CJX_Form);
  JSE_METHOD(execValidate, CJX_Form);
  JSE_METHOD(formNodes, CJX_Form);
  JSE_METHOD(recalculate, CJX_Form);
  JSE_METHOD(remerge, CJX_Form);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_FORM_H_
