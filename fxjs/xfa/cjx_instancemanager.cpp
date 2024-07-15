// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_instancemanager.h"

#include <algorithm>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/span.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_instancemanager.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_subform.h"

const CJX_MethodSpec CJX_InstanceManager::MethodSpecs[] = {
    {"addInstance", addInstance_static},
    {"insertInstance", insertInstance_static},
    {"moveInstance", moveInstance_static},
    {"removeInstance", removeInstance_static},
    {"setInstances", setInstances_static}};

CJX_InstanceManager::CJX_InstanceManager(CXFA_InstanceManager* mgr)
    : CJX_Node(mgr) {
  DefineMethods(MethodSpecs);
}

CJX_InstanceManager::~CJX_InstanceManager() = default;

bool CJX_InstanceManager::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

int32_t CJX_InstanceManager::SetInstances(v8::Isolate* pIsolate,
                                          int32_t iDesired) {
  CXFA_Occur* occur = GetXFANode()->GetOccurIfExists();
  int32_t iMin = occur ? occur->GetMin() : CXFA_Occur::kDefaultMin;
  if (iDesired < iMin) {
    ThrowTooManyOccurrencesException(pIsolate, L"min");
    return 1;
  }

  int32_t iMax = occur ? occur->GetMax() : CXFA_Occur::kDefaultMax;
  if (iMax >= 0 && iDesired > iMax) {
    ThrowTooManyOccurrencesException(pIsolate, L"max");
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
            : wsInstManagerName.Last(wsInstManagerName.GetLength() - 1));
    uint32_t dInstanceNameHash =
        FX_HashCode_GetW(wsInstanceName.AsStringView());
    CXFA_Node* pPrevSibling = iDesired == 0
                                  ? GetXFANode()
                                  : GetXFANode()->GetItemIfExists(iDesired - 1);
    if (!pPrevSibling) {
      // TODO(dsinclair): Better error?
      ThrowIndexOutOfBoundsException(pIsolate);
      return 0;
    }

    while (iCount > iDesired) {
      CXFA_Node* pRemoveInstance = pPrevSibling->GetNextSibling();
      const XFA_Element type = pRemoveInstance->GetElementType();
      if (type != XFA_Element::Subform && type != XFA_Element::SubformSet) {
        continue;
      }
      CHECK_NE(type, XFA_Element::InstanceManager);
      if (pRemoveInstance->GetNameHash() == dInstanceNameHash) {
        GetXFANode()->RemoveItem(pRemoveInstance, true);
        iCount--;
      }
    }
  } else {
    while (iCount < iDesired) {
      CXFA_Node* pNewInstance = GetXFANode()->CreateInstanceIfPossible(true);
      if (!pNewInstance)
        return 0;

      GetXFANode()->InsertItem(pNewInstance, iCount, iCount, false);
      ++iCount;

      CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
      if (!pNotify)
        return 0;

      pNotify->RunNodeInitialize(pNewInstance);
    }
  }
  GetDocument()->GetLayoutProcessor()->SetHasChangedContainer();
  return 0;
}

int32_t CJX_InstanceManager::MoveInstance(v8::Isolate* pIsolate,
                                          int32_t iTo,
                                          int32_t iFrom) {
  int32_t iCount = GetXFANode()->GetCount();
  if (iFrom > iCount || iTo > iCount - 1) {
    ThrowIndexOutOfBoundsException(pIsolate);
    return 1;
  }
  if (iFrom < 0 || iTo < 0 || iFrom == iTo)
    return 0;

  CXFA_Node* pMoveInstance = GetXFANode()->GetItemIfExists(iFrom);
  if (!pMoveInstance) {
    ThrowIndexOutOfBoundsException(pIsolate);
    return 1;
  }

  GetXFANode()->RemoveItem(pMoveInstance, false);
  GetXFANode()->InsertItem(pMoveInstance, iTo, iCount - 1, true);
  GetDocument()->GetLayoutProcessor()->SetHasChangedContainer();
  return 0;
}

CJS_Result CJX_InstanceManager::moveInstance(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  CXFA_Document* doc = runtime->GetDocument();
  if (doc->GetFormType() != FormType::kXFAFull)
    return CJS_Result::Failure(JSMessage::kNotSupportedError);

  if (params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  int32_t iFrom = runtime->ToInt32(params[0]);
  int32_t iTo = runtime->ToInt32(params[1]);
  MoveInstance(runtime->GetIsolate(), iTo, iFrom);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  CXFA_Node* pXFA = GetXFANode();
  auto* pToInstance = CXFA_Subform::FromNode(pXFA->GetItemIfExists(iTo));
  if (pToInstance)
    pNotify->RunSubformIndexChange(pToInstance);

  auto* pFromInstance = CXFA_Subform::FromNode(pXFA->GetItemIfExists(iFrom));
  if (pFromInstance)
    pNotify->RunSubformIndexChange(pFromInstance);

  return CJS_Result::Success();
}

CJS_Result CJX_InstanceManager::removeInstance(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  CXFA_Document* doc = runtime->GetDocument();
  if (doc->GetFormType() != FormType::kXFAFull)
    return CJS_Result::Failure(JSMessage::kNotSupportedError);

  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  int32_t iIndex = runtime->ToInt32(params[0]);
  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex >= iCount)
    return CJS_Result::Failure(JSMessage::kInvalidInputError);

  CXFA_Occur* occur = GetXFANode()->GetOccurIfExists();
  int32_t iMin = occur ? occur->GetMin() : CXFA_Occur::kDefaultMin;
  if (iCount - 1 < iMin)
    return CJS_Result::Failure(JSMessage::kTooManyOccurrences);

  CXFA_Node* pRemoveInstance = GetXFANode()->GetItemIfExists(iIndex);
  if (!pRemoveInstance)
    return CJS_Result::Failure(JSMessage::kParamError);

  GetXFANode()->RemoveItem(pRemoveInstance, true);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    CXFA_Node* pXFA = GetXFANode();
    for (int32_t i = iIndex; i < iCount - 1; i++) {
      auto* pSubformInstance = CXFA_Subform::FromNode(pXFA->GetItemIfExists(i));
      if (pSubformInstance)
        pNotify->RunSubformIndexChange(pSubformInstance);
    }
  }
  GetDocument()->GetLayoutProcessor()->SetHasChangedContainer();
  return CJS_Result::Success();
}

CJS_Result CJX_InstanceManager::setInstances(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  CXFA_Document* doc = runtime->GetDocument();
  if (doc->GetFormType() != FormType::kXFAFull)
    return CJS_Result::Failure(JSMessage::kNotSupportedError);

  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  SetInstances(runtime->GetIsolate(), runtime->ToInt32(params[0]));
  return CJS_Result::Success();
}

CJS_Result CJX_InstanceManager::addInstance(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  CXFA_Document* doc = runtime->GetDocument();
  if (doc->GetFormType() != FormType::kXFAFull)
    return CJS_Result::Failure(JSMessage::kNotSupportedError);

  if (!params.empty() && params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  bool fFlags = true;
  if (params.size() == 1)
    fFlags = runtime->ToBoolean(params[0]);

  int32_t iCount = GetXFANode()->GetCount();
  CXFA_Occur* occur = GetXFANode()->GetOccurIfExists();
  int32_t iMax = occur ? occur->GetMax() : CXFA_Occur::kDefaultMax;
  if (iMax >= 0 && iCount >= iMax)
    return CJS_Result::Failure(JSMessage::kTooManyOccurrences);

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstanceIfPossible(fFlags);
  if (!pNewInstance)
    return CJS_Result::Success(runtime->NewNull());

  GetXFANode()->InsertItem(pNewInstance, iCount, iCount, false);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->RunNodeInitialize(pNewInstance);
    GetDocument()->GetLayoutProcessor()->SetHasChangedContainer();
  }

  return CJS_Result::Success(
      runtime->GetOrCreateJSBindingFromMap(pNewInstance));
}

CJS_Result CJX_InstanceManager::insertInstance(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  CXFA_Document* doc = runtime->GetDocument();
  if (doc->GetFormType() != FormType::kXFAFull)
    return CJS_Result::Failure(JSMessage::kNotSupportedError);

  if (params.size() != 1 && params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  int32_t iIndex = runtime->ToInt32(params[0]);
  bool bBind = false;
  if (params.size() == 2)
    bBind = runtime->ToBoolean(params[1]);

  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex > iCount)
    return CJS_Result::Failure(JSMessage::kInvalidInputError);

  CXFA_Occur* occur = GetXFANode()->GetOccurIfExists();
  int32_t iMax = occur ? occur->GetMax() : CXFA_Occur::kDefaultMax;
  if (iMax >= 0 && iCount >= iMax)
    return CJS_Result::Failure(JSMessage::kInvalidInputError);

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstanceIfPossible(bBind);
  if (!pNewInstance)
    return CJS_Result::Success(runtime->NewNull());

  GetXFANode()->InsertItem(pNewInstance, iIndex, iCount, true);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->RunNodeInitialize(pNewInstance);
    GetDocument()->GetLayoutProcessor()->SetHasChangedContainer();
  }

  return CJS_Result::Success(
      runtime->GetOrCreateJSBindingFromMap(pNewInstance));
}

void CJX_InstanceManager::max(v8::Isolate* pIsolate,
                              v8::Local<v8::Value>* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  CXFA_Occur* occur = GetXFANode()->GetOccurIfExists();
  *pValue = fxv8::NewNumberHelper(
      pIsolate, occur ? occur->GetMax() : CXFA_Occur::kDefaultMax);
}

void CJX_InstanceManager::min(v8::Isolate* pIsolate,
                              v8::Local<v8::Value>* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  CXFA_Occur* occur = GetXFANode()->GetOccurIfExists();
  *pValue = fxv8::NewNumberHelper(
      pIsolate, occur ? occur->GetMin() : CXFA_Occur::kDefaultMin);
}

void CJX_InstanceManager::count(v8::Isolate* pIsolate,
                                v8::Local<v8::Value>* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute) {
  if (bSetting) {
    SetInstances(pIsolate, fxv8::ReentrantToInt32Helper(pIsolate, *pValue));
    return;
  }
  *pValue = fxv8::NewNumberHelper(pIsolate, GetXFANode()->GetCount());
}
