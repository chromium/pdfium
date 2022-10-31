// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_MODEL_H_
#define FXJS_XFA_CJX_MODEL_H_

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_Node;

class CJX_Model : public CJX_Node {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Model() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(clearErrorList);
  JSE_METHOD(createNode);
  JSE_METHOD(isCompatibleNS);

  JSE_PROP(aliasNode);
  JSE_PROP(context);

 protected:
  explicit CJX_Model(CXFA_Node* obj);

 private:
  using Type__ = CJX_Model;
  using ParentType__ = CJX_Node;

  static const TypeTag static_type__ = TypeTag::Model;
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_MODEL_H_
