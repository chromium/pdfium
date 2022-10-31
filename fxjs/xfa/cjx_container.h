// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_CONTAINER_H_
#define FXJS_XFA_CJX_CONTAINER_H_

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_Node;

class CJX_Container : public CJX_Node {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Container() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(getDelta);
  JSE_METHOD(getDeltas);

 protected:
  explicit CJX_Container(CXFA_Node* node);

 private:
  using Type__ = CJX_Container;
  using ParentType__ = CJX_Node;

  static const TypeTag static_type__ = TypeTag::Container;
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_CONTAINER_H_
