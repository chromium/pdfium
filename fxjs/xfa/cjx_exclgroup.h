// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_EXCLGROUP_H_
#define FXJS_XFA_CJX_EXCLGROUP_H_

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_ExclGroup;

class CJX_ExclGroup final : public CJX_Node {
 public:
  explicit CJX_ExclGroup(CXFA_ExclGroup* group);
  ~CJX_ExclGroup() override;

  JSE_METHOD(execCalculate);
  JSE_METHOD(execEvent);
  JSE_METHOD(execInitialize);
  JSE_METHOD(execValidate);
  JSE_METHOD(selectedMember);

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(borderColor);
  JSE_PROP(borderWidth);
  JSE_PROP(fillColor);
  JSE_PROP(mandatory);
  JSE_PROP(mandatoryMessage);
  JSE_PROP(rawValue);
  JSE_PROP(transient);
  JSE_PROP(validationMessage);

 private:
  using Type__ = CJX_ExclGroup;

  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_EXCLGROUP_H_
