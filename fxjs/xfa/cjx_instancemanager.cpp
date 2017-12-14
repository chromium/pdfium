// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_instancemanager.h"

#include <algorithm>
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
    {"setInstances", setInstances_static}};

CJX_InstanceManager::CJX_InstanceManager(CXFA_InstanceManager* mgr)
    : CJX_Node(mgr) {
  DefineMethods(MethodSpecs, FX_ArraySize(MethodSpecs));
}

CJX_InstanceManager::~CJX_InstanceManager() {}

int32_t CJX_InstanceManager::SetInstances(int32_t iDesired) {
  CXFA_OccurData occurData(GetXFANode()->GetOccurNode());
  if (iDesired < occurData.GetMin()) {
    ThrowTooManyOccurancesException(L"min");
    return 1;
  }

  int32_t iMax = occurData.GetMax();
  if (iMax >= 0 && iDesired > iMax) {
    ThrowTooManyOccurancesException(L"max");
    return 2;
  }

  int32_t iCount = GetXFANode()->GetCount();
  if (iDesired == iCount)
    return 0;

  if (iDesired < iCount) {
    WideString wsInstManagerName = GetCData(XFA_Attribute::Name);
    WideString wsInstanceName = WideString(
        wsInstManagerName.IsEmpty()
            ? wsInstManagerName
            : wsInstManagerName.Right(wsInstManagerName.GetLength() - 1));
    uint32_t dInstanceNameHash =
        FX_HashCode_GetW(wsInstanceName.AsStringView(), false);
    CXFA_Node* pPrevSibling =
        iDesired == 0 ? GetXFANode() : GetXFANode()->GetItem(iDesired - 1);
    while (iCount > iDesired) {
      CXFA_Node* pRemoveInstance =
          pPrevSibling->GetNodeItem(XFA_NODEITEM_NextSibling);
      if (pRemoveInstance->GetElementType() != XFA_Element::Subform &&
          pRemoveInstance->GetElementType() != XFA_Element::SubformSet) {
        continue;
      }
      if (pRemoveInstance->GetElementType() == XFA_Element::InstanceManager) {
        NOTREACHED();
        break;
      }
      if (pRemoveInstance->GetNameHash() == dInstanceNameHash) {
        GetXFANode()->RemoveItem(pRemoveInstance, true);
        iCount--;
      }
    }
  } else {
    while (iCount < iDesired) {
      CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(true);
      GetXFANode()->InsertItem(pNewInstance, iCount, iCount, false);
      iCount++;
      CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
      if (!pNotify)
        return 0;

      pNotify->RunNodeInitialize(pNewInstance);
    }
  }

  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (pLayoutPro) {
    pLayoutPro->AddChangedContainer(
        ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
  }
  return 0;
}

int32_t CJX_InstanceManager::MoveInstance(int32_t iTo, int32_t iFrom) {
  int32_t iCount = GetXFANode()->GetCount();
  if (iFrom > iCount || iTo > iCount - 1) {
    ThrowIndexOutOfBoundsException();
    return 1;
  }
  if (iFrom < 0 || iTo < 0 || iFrom == iTo)
    return 0;

  CXFA_Node* pMoveInstance = GetXFANode()->GetItem(iFrom);
  GetXFANode()->RemoveItem(pMoveInstance, false);
  GetXFANode()->InsertItem(pMoveInstance, iTo, iCount - 1, true);
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (pLayoutPro) {
    pLayoutPro->AddChangedContainer(
        ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
  }
  return 0;
}

CJS_Return CJX_InstanceManager::moveInstance(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  int32_t iFrom = runtime->ToInt32(params[0]);
  int32_t iTo = runtime->ToInt32(params[1]);
  MoveInstance(iTo, iFrom);

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

  SetInstances(runtime->ToInt32(params[0]));
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

void CJX_InstanceManager::max(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax());
}

void CJX_InstanceManager::min(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMin());
}

void CJX_InstanceManager::count(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute) {
  if (bSetting) {
    pValue->SetInteger(GetXFANode()->GetCount());
    return;
  }
  SetInstances(pValue->ToInteger());
}
