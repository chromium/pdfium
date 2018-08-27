// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_INSTANCEMANAGER_H_
#define FXJS_XFA_CJX_INSTANCEMANAGER_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_InstanceManager;

class CJX_InstanceManager final : public CJX_Node {
 public:
  explicit CJX_InstanceManager(CXFA_InstanceManager* mgr);
  ~CJX_InstanceManager() override;

  JSE_METHOD(addInstance, CJX_InstanceManager);
  JSE_METHOD(insertInstance, CJX_InstanceManager);
  JSE_METHOD(moveInstance, CJX_InstanceManager);
  JSE_METHOD(removeInstance, CJX_InstanceManager);
  JSE_METHOD(setInstances, CJX_InstanceManager);

  JSE_PROP(count);
  JSE_PROP(max);
  JSE_PROP(min);

  int32_t MoveInstance(int32_t iTo, int32_t iFrom);

 private:
  int32_t SetInstances(int32_t iDesired);

  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_INSTANCEMANAGER_H_
