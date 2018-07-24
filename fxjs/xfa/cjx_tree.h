// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_TREE_H_
#define FXJS_XFA_CJX_TREE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_object.h"

class CXFA_Object;
class CXFA_Node;

class CJX_Tree : public CJX_Object {
 public:
  explicit CJX_Tree(CXFA_Object* obj);
  ~CJX_Tree() override;

  JSE_METHOD(resolveNode, CJX_Tree);
  JSE_METHOD(resolveNodes, CJX_Tree);

  JSE_PROP(all);
  JSE_PROP(classAll);
  JSE_PROP(classIndex);
  JSE_PROP(index);
  JSE_PROP(name);
  JSE_PROP(nodes);
  JSE_PROP(parent);
  JSE_PROP(somExpression);

 private:
  void ResolveNodeList(CFXJSE_Value* pValue,
                       WideString wsExpression,
                       uint32_t dwFlag,
                       CXFA_Node* refNode);

  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_TREE_H_
