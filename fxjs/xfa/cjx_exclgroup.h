// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_EXCLGROUP_H_
#define FXJS_XFA_CJX_EXCLGROUP_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_ExclGroup;

class CJX_ExclGroup final : public CJX_Node {
 public:
  explicit CJX_ExclGroup(CXFA_ExclGroup* group);
  ~CJX_ExclGroup() override;

  JSE_METHOD(execCalculate, CJX_ExclGroup);
  JSE_METHOD(execEvent, CJX_ExclGroup);
  JSE_METHOD(execInitialize, CJX_ExclGroup);
  JSE_METHOD(execValidate, CJX_ExclGroup);
  JSE_METHOD(selectedMember, CJX_ExclGroup);

  JSE_PROP(defaultValue); /* {default} */
  JSE_PROP(access);
  JSE_PROP(accessKey);
  JSE_PROP(anchorType);
  JSE_PROP(borderColor);
  JSE_PROP(borderWidth);
  JSE_PROP(colSpan);
  JSE_PROP(fillColor);
  JSE_PROP(h);
  JSE_PROP(hAlign);
  JSE_PROP(layout);
  JSE_PROP(mandatory);
  JSE_PROP(mandatoryMessage);
  JSE_PROP(maxH);
  JSE_PROP(maxW);
  JSE_PROP(minH);
  JSE_PROP(minW);
  JSE_PROP(presence);
  JSE_PROP(rawValue);
  JSE_PROP(relevant);
  JSE_PROP(transient);
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

#endif  // FXJS_XFA_CJX_EXCLGROUP_H_
