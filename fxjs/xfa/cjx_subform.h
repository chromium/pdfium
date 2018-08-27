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

  JSE_PROP(allowMacro);
  JSE_PROP(anchorType);
  JSE_PROP(colSpan);
  JSE_PROP(columnWidths);
  JSE_PROP(h);
  JSE_PROP(hAlign);
  JSE_PROP(instanceIndex);
  JSE_PROP(layout);
  JSE_PROP(locale);
  JSE_PROP(maxH);
  JSE_PROP(maxW);
  JSE_PROP(minH);
  JSE_PROP(minW);
  JSE_PROP(presence);
  JSE_PROP(relevant);
  JSE_PROP(restoreState);
  JSE_PROP(scope);
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(validationMessage);
  JSE_PROP(vAlign);
  JSE_PROP(w);
  JSE_PROP(x);
  JSE_PROP(y);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_SUBFORM_H_
