// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_INSTANCEMANAGER_H_
#define FXJS_XFA_CJX_INSTANCEMANAGER_H_

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/jse_define.h"
#include "v8/include/v8-forward.h"

class CXFA_InstanceManager;

class CJX_InstanceManager final : public CJX_Node {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_InstanceManager() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(addInstance);
  JSE_METHOD(insertInstance);
  JSE_METHOD(moveInstance);
  JSE_METHOD(removeInstance);
  JSE_METHOD(setInstances);

  JSE_PROP(count);
  JSE_PROP(max);
  JSE_PROP(min);

  int32_t MoveInstance(v8::Isolate* pIsolate, int32_t iTo, int32_t iFrom);

 private:
  explicit CJX_InstanceManager(CXFA_InstanceManager* mgr);

  using Type__ = CJX_InstanceManager;
  using ParentType__ = CJX_Node;

  static const TypeTag static_type__ = TypeTag::InstanceManager;
  static const CJX_MethodSpec MethodSpecs[];

  int32_t SetInstances(v8::Isolate* pIsolate, int32_t iDesired);
};

#endif  // FXJS_XFA_CJX_INSTANCEMANAGER_H_
