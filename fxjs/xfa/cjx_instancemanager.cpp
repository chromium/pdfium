// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_instancemanager.h"

#include <vector>

#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "fxjs/js_resources.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_instancemanager.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_occurdata.h"

const CJX_MethodSpec CJX_InstanceManager::MethodSpecs[] = {
    {"addInstance", addInstance_static},
    {"insertInstance", insertInstance_static},
    {"moveInstance", moveInstance_static},
    {"removeInstance", removeInstance_static},
    {"setInstances", setInstances_static},
    {"", nullptr}};

CJX_InstanceManager::CJX_InstanceManager(CXFA_InstanceManager* mgr)
    : CJX_Node(mgr) {
  DefineMethods(MethodSpecs);
}

CJX_InstanceManager::~CJX_InstanceManager() {}

CJS_Return CJX_InstanceManager::moveInstance(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  int32_t iFrom = runtime->ToInt32(params[0]);
  int32_t iTo = runtime->ToInt32(params[1]);
  InstanceManager_MoveInstance(iTo, iFrom);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Return(true);

  CXFA_Node* pToInstance = GetXFANode()->GetItem(iTo);
  if (pToInstance && pToInstance->GetElementType() == XFA_Element::Subform)
    pNotify->RunSubformIndexChange(pToInstance);

  CXFA_Node* pFromInstance = GetXFANode()->GetItem(iFrom);
  if (pFromInstance &&
      pFromInstance->GetElementType() == XFA_Element::Subform) {
    pNotify->RunSubformIndexChange(pFromInstance);
  }

  return CJS_Return(true);
}

CJS_Return CJX_InstanceManager::removeInstance(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  int32_t iIndex = runtime->ToInt32(params[0]);
  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex >= iCount)
    return CJS_Return(JSGetStringFromID(JSMessage::kInvalidInputError));

  int32_t iMin = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMin();
  if (iCount - 1 < iMin)
    return CJS_Return(JSGetStringFromID(JSMessage::kTooManyOccurances));

  CXFA_Node* pRemoveInstance = GetXFANode()->GetItem(iIndex);
  GetXFANode()->RemoveItem(pRemoveInstance, true);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    for (int32_t i = iIndex; i < iCount - 1; i++) {
      CXFA_Node* pSubformInstance = GetXFANode()->GetItem(i);
      if (pSubformInstance &&
          pSubformInstance->GetElementType() == XFA_Element::Subform) {
        pNotify->RunSubformIndexChange(pSubformInstance);
      }
    }
  }
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (pLayoutPro) {
    pLayoutPro->AddChangedContainer(
        ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
  }
  return CJS_Return(true);
}

CJS_Return CJX_InstanceManager::setInstances(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  InstanceManager_SetInstances(runtime->ToInt32(params[0]));
  return CJS_Return(true);
}

CJS_Return CJX_InstanceManager::addInstance(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty() && params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  bool fFlags = true;
  if (params.size() == 1)
    fFlags = runtime->ToBoolean(params[0]);

  int32_t iCount = GetXFANode()->GetCount();
  int32_t iMax = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax();
  if (iMax >= 0 && iCount >= iMax)
    return CJS_Return(JSGetStringFromID(JSMessage::kTooManyOccurances));

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(fFlags);
  GetXFANode()->InsertItem(pNewInstance, iCount, iCount, false);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->RunNodeInitialize(pNewInstance);

    CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
    if (pLayoutPro) {
      pLayoutPro->AddChangedContainer(
          ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
    }
  }

  CFXJSE_Value* value =
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewInstance);
  if (!value)
    return CJS_Return(runtime->NewNull());

  return CJS_Return(value->DirectGetValue().Get(runtime->GetIsolate()));
}

CJS_Return CJX_InstanceManager::insertInstance(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1 && params.size() != 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  int32_t iIndex = runtime->ToInt32(params[0]);
  bool bBind = false;
  if (params.size() == 2)
    bBind = runtime->ToBoolean(params[1]);

  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex > iCount)
    return CJS_Return(JSGetStringFromID(JSMessage::kInvalidInputError));

  int32_t iMax = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax();
  if (iMax >= 0 && iCount >= iMax)
    return CJS_Return(JSGetStringFromID(JSMessage::kInvalidInputError));

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(bBind);
  GetXFANode()->InsertItem(pNewInstance, iIndex, iCount, true);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->RunNodeInitialize(pNewInstance);
    CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
    if (pLayoutPro) {
      pLayoutPro->AddChangedContainer(
          ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
    }
  }

  CFXJSE_Value* value =
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewInstance);
  if (!value)
    return CJS_Return(runtime->NewNull());

  return CJS_Return(value->DirectGetValue().Get(runtime->GetIsolate()));
}
