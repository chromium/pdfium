// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_TREE_H_
#define FXJS_XFA_CJX_TREE_H_

#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cjx_object.h"
#include "fxjs/xfa/jse_define.h"

class CXFA_Object;
class CXFA_Node;

class CJX_Tree : public CJX_Object {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Tree() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(resolveNode);
  JSE_METHOD(resolveNodes);

  JSE_PROP(all);
  JSE_PROP(classAll);
  JSE_PROP(classIndex);
  JSE_PROP(index);
  JSE_PROP(nodes);
  JSE_PROP(parent);
  JSE_PROP(somExpression);

 protected:
  explicit CJX_Tree(CXFA_Object* obj);

 private:
  using Type__ = CJX_Tree;
  using ParentType__ = CJX_Object;

  static const TypeTag static_type__ = TypeTag::Tree;
  static const CJX_MethodSpec MethodSpecs[];

  v8::Local<v8::Value> ResolveNodeList(v8::Isolate* pIsolate,
                                       WideString wsExpression,
                                       Mask<XFA_ResolveFlag> dwFlag,
                                       CXFA_Node* refNode);
};

#endif  // FXJS_XFA_CJX_TREE_H_
