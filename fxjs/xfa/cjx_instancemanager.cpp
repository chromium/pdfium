// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_instancemanager.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
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

void CJX_InstanceManager::moveInstance(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 2) {
    pArguments->GetReturnValue()->SetUndefined();
    return;
  }

  int32_t iFrom = pArguments->GetInt32(0);
  int32_t iTo = pArguments->GetInt32(1);
  InstanceManager_MoveInstance(iTo, iFrom);
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_Node* pToInstance = GetXFANode()->GetItem(iTo);
  if (pToInstance && pToInstance->GetElementType() == XFA_Element::Subform)
    pNotify->RunSubformIndexChange(pToInstance);

  CXFA_Node* pFromInstance = GetXFANode()->GetItem(iFrom);
  if (pFromInstance &&
      pFromInstance->GetElementType() == XFA_Element::Subform) {
    pNotify->RunSubformIndexChange(pFromInstance);
  }
}

void CJX_InstanceManager::removeInstance(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    pArguments->GetReturnValue()->SetUndefined();
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex >= iCount) {
    ThrowIndexOutOfBoundsException();
    return;
  }

  int32_t iMin = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMin();
  if (iCount - 1 < iMin) {
    ThrowTooManyOccurancesException(L"min");
    return;
  }

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
  if (!pLayoutPro)
    return;

  pLayoutPro->AddChangedContainer(
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
}

void CJX_InstanceManager::setInstances(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    pArguments->GetReturnValue()->SetUndefined();
    return;
  }

  int32_t iDesired = pArguments->GetInt32(0);
  InstanceManager_SetInstances(iDesired);
}

void CJX_InstanceManager::addInstance(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 0 && argc != 1) {
    ThrowParamCountMismatchException(L"addInstance");
    return;
  }

  bool fFlags = true;
  if (argc == 1)
    fFlags = pArguments->GetInt32(0) == 0 ? false : true;

  int32_t iCount = GetXFANode()->GetCount();
  int32_t iMax = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax();
  if (iMax >= 0 && iCount >= iMax) {
    ThrowTooManyOccurancesException(L"max");
    return;
  }

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(fFlags);
  GetXFANode()->InsertItem(pNewInstance, iCount, iCount, false);
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewInstance));
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->RunNodeInitialize(pNewInstance);
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (!pLayoutPro)
    return;

  pLayoutPro->AddChangedContainer(
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
}

void CJX_InstanceManager::insertInstance(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1 && argc != 2) {
    ThrowParamCountMismatchException(L"insertInstance");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  bool bBind = false;
  if (argc == 2)
    bBind = pArguments->GetInt32(1) == 0 ? false : true;

  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex > iCount) {
    ThrowIndexOutOfBoundsException();
    return;
  }

  int32_t iMax = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax();
  if (iMax >= 0 && iCount >= iMax) {
    ThrowTooManyOccurancesException(L"max");
    return;
  }

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(bBind);
  GetXFANode()->InsertItem(pNewInstance, iIndex, iCount, true);
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewInstance));
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->RunNodeInitialize(pNewInstance);
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (!pLayoutPro)
    return;

  pLayoutPro->AddChangedContainer(
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
}
