// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_scriptcontext.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"
#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_class.h"
#include "fxjs/cfxjse_value.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/app/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodehelper.h"
#include "xfa/fxfa/parser/cxfa_nodelist.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#include "xfa/fxfa/parser/cxfa_resolveprocessor.h"
#include "xfa/fxfa/parser/cxfa_thisproxy.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

const FXJSE_CLASS_DESCRIPTOR GlobalClassDescriptor = {
    "Root",   // name
    nullptr,  // constructor
    nullptr,  // properties
    nullptr,  // methods
    0,        // property count
    0,        // method count
    CXFA_ScriptContext::GlobalPropTypeGetter,
    CXFA_ScriptContext::GlobalPropertyGetter,
    CXFA_ScriptContext::GlobalPropertySetter,
    nullptr,  // property deleter
    CXFA_ScriptContext::NormalMethodCall,
};

const FXJSE_CLASS_DESCRIPTOR NormalClassDescriptor = {
    "XFAObject",  // name
    nullptr,      // constructor
    nullptr,      // properties
    nullptr,      // methods
    0,            // property count
    0,            // method count
    CXFA_ScriptContext::NormalPropTypeGetter,
    CXFA_ScriptContext::NormalPropertyGetter,
    CXFA_ScriptContext::NormalPropertySetter,
    nullptr,  // property deleter
    CXFA_ScriptContext::NormalMethodCall,
};

const FXJSE_CLASS_DESCRIPTOR VariablesClassDescriptor = {
    "XFAScriptObject",  // name
    nullptr,            // constructor
    nullptr,            // properties
    nullptr,            // methods
    0,                  // property count
    0,                  // method count
    CXFA_ScriptContext::NormalPropTypeGetter,
    CXFA_ScriptContext::GlobalPropertyGetter,
    CXFA_ScriptContext::GlobalPropertySetter,
    nullptr,  // property deleter
    CXFA_ScriptContext::NormalMethodCall,
};

const char kFormCalcRuntime[] = "pfm_rt";

CXFA_ThisProxy* ToThisProxy(CFXJSE_Value* pValue, CFXJSE_Class* pClass) {
  return static_cast<CXFA_ThisProxy*>(pValue->ToHostObject(pClass));
}

const XFA_METHODINFO* GetMethodByName(XFA_Element eElement,
                                      const CFX_WideStringC& wsMethodName) {
  if (wsMethodName.IsEmpty())
    return nullptr;

  int32_t iElementIndex = static_cast<int32_t>(eElement);
  while (iElementIndex >= 0 && iElementIndex < g_iScriptIndexCount) {
    const XFA_SCRIPTHIERARCHY* scriptIndex = g_XFAScriptIndex + iElementIndex;
    int32_t icount = scriptIndex->wMethodCount;
    if (icount == 0) {
      iElementIndex = scriptIndex->wParentIndex;
      continue;
    }
    uint32_t uHash = FX_HashCode_GetW(wsMethodName, false);
    int32_t iStart = scriptIndex->wMethodStart;
    // TODO(dsinclair): Switch to std::lower_bound.
    int32_t iEnd = iStart + icount - 1;
    do {
      int32_t iMid = (iStart + iEnd) / 2;
      const XFA_METHODINFO* pInfo = g_SomMethodData + iMid;
      if (uHash == pInfo->uHash)
        return pInfo;
      if (uHash < pInfo->uHash)
        iEnd = iMid - 1;
      else
        iStart = iMid + 1;
    } while (iStart <= iEnd);
    iElementIndex = scriptIndex->wParentIndex;
  }
  return nullptr;
}

}  // namespace

// static.
CXFA_Object* CXFA_ScriptContext::ToObject(CFXJSE_Value* pValue,
                                          CFXJSE_Class* pClass) {
  CFXJSE_HostObject* pHostObj = pValue->ToHostObject(pClass);
  if (!pHostObj || pHostObj->type() != CFXJSE_HostObject::kXFA)
    return nullptr;
  return static_cast<CXFA_Object*>(pHostObj);
}

CXFA_ScriptContext::CXFA_ScriptContext(CXFA_Document* pDocument)
    : m_pDocument(pDocument),
      m_pIsolate(nullptr),
      m_pJsClass(nullptr),
      m_eScriptType(XFA_SCRIPTLANGTYPE_Unkown),
      m_pScriptNodeArray(nullptr),
      m_pThisObject(nullptr),
      m_dwBuiltInInFlags(0),
      m_eRunAtType(XFA_ATTRIBUTEENUM_Client) {}

CXFA_ScriptContext::~CXFA_ScriptContext() {
  for (const auto& pair : m_mapVariableToContext)
    delete ToThisProxy(pair.second->GetGlobalObject().get(), nullptr);
}

void CXFA_ScriptContext::Initialize(v8::Isolate* pIsolate) {
  m_pIsolate = pIsolate;
  DefineJsContext();
  DefineJsClass();
  m_ResolveProcessor = pdfium::MakeUnique<CXFA_ResolveProcessor>();
}

bool CXFA_ScriptContext::RunScript(XFA_SCRIPTLANGTYPE eScriptType,
                                   const CFX_WideStringC& wsScript,
                                   CFXJSE_Value* hRetValue,
                                   CXFA_Object* pThisObject) {
  CFX_ByteString btScript;
  CFX_AutoRestorer<XFA_SCRIPTLANGTYPE> typeRestorer(&m_eScriptType);
  m_eScriptType = eScriptType;
  if (eScriptType == XFA_SCRIPTLANGTYPE_Formcalc) {
    if (!m_FM2JSContext) {
      m_FM2JSContext = pdfium::MakeUnique<CXFA_FM2JSContext>(
          m_pIsolate, m_JsContext.get(), m_pDocument.Get());
    }
    CFX_WideTextBuf wsJavaScript;
    if (!CXFA_FM2JSContext::Translate(wsScript, &wsJavaScript)) {
      hRetValue->SetUndefined();
      return false;
    }
    btScript = FX_UTF8Encode(wsJavaScript.AsStringC());
  } else {
    btScript = FX_UTF8Encode(wsScript);
  }
  CFX_AutoRestorer<CXFA_Object*> nodeRestorer(&m_pThisObject);
  m_pThisObject = pThisObject;
  CFXJSE_Value* pValue = pThisObject ? GetJSValueFromMap(pThisObject) : nullptr;
  return m_JsContext->ExecuteScript(btScript.c_str(), hRetValue, pValue);
}

void CXFA_ScriptContext::GlobalPropertySetter(CFXJSE_Value* pObject,
                                              const CFX_ByteStringC& szPropName,
                                              CFXJSE_Value* pValue) {
  CXFA_Object* lpOrginalNode = ToObject(pObject, nullptr);
  CXFA_Document* pDoc = lpOrginalNode->GetDocument();
  CXFA_ScriptContext* lpScriptContext = pDoc->GetScriptContext();
  CXFA_Object* lpCurNode = lpScriptContext->GetVariablesThis(lpOrginalNode);
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  uint32_t dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings |
                    XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                    XFA_RESOLVENODE_Attributes;
  CXFA_Node* pRefNode = ToNode(lpScriptContext->GetThisObject());
  if (lpOrginalNode->IsVariablesThis())
    pRefNode = ToNode(lpCurNode);
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), pValue,
                                       dwFlag, true)) {
    return;
  }
  if (lpOrginalNode->IsVariablesThis()) {
    if (pValue && pValue->IsUndefined()) {
      pObject->SetObjectOwnProperty(szPropName, pValue);
      return;
    }
  }
  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->GetDocEnvironment()->SetGlobalProperty(pNotify->GetHDOC(),
                                                  szPropName, pValue);
}
bool CXFA_ScriptContext::QueryNodeByFlag(CXFA_Node* refNode,
                                         const CFX_WideStringC& propname,
                                         CFXJSE_Value* pValue,
                                         uint32_t dwFlag,
                                         bool bSetting) {
  if (!refNode)
    return false;
  XFA_RESOLVENODE_RS resolveRs;
  if (ResolveObjects(refNode, propname, resolveRs, dwFlag) <= 0)
    return false;
  if (resolveRs.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    pValue->Assign(GetJSValueFromMap(resolveRs.objects.front()));
    return true;
  }
  if (resolveRs.dwFlags == XFA_RESOVENODE_RSTYPE_Attribute) {
    const XFA_SCRIPTATTRIBUTEINFO* lpAttributeInfo = resolveRs.pScriptAttribute;
    if (lpAttributeInfo) {
      (resolveRs.objects.front()->*(lpAttributeInfo->lpfnCallback))(
          pValue, bSetting, (XFA_ATTRIBUTE)lpAttributeInfo->eAttribute);
    }
  }
  return true;
}
void CXFA_ScriptContext::GlobalPropertyGetter(CFXJSE_Value* pObject,
                                              const CFX_ByteStringC& szPropName,
                                              CFXJSE_Value* pValue) {
  CXFA_Object* pOriginalObject = ToObject(pObject, nullptr);
  CXFA_Document* pDoc = pOriginalObject->GetDocument();
  CXFA_ScriptContext* lpScriptContext = pDoc->GetScriptContext();
  CXFA_Object* lpCurNode = lpScriptContext->GetVariablesThis(pOriginalObject);
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  if (lpScriptContext->GetType() == XFA_SCRIPTLANGTYPE_Formcalc) {
    if (szPropName == kFormCalcRuntime) {
      lpScriptContext->m_FM2JSContext->GlobalPropertyGetter(pValue);
      return;
    }
    XFA_HashCode uHashCode = static_cast<XFA_HashCode>(
        FX_HashCode_GetW(wsPropName.AsStringC(), false));
    if (uHashCode != XFA_HASHCODE_Layout) {
      CXFA_Object* pObj =
          lpScriptContext->GetDocument()->GetXFAObject(uHashCode);
      if (pObj) {
        pValue->Assign(lpScriptContext->GetJSValueFromMap(pObj));
        return;
      }
    }
  }
  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                    XFA_RESOLVENODE_Attributes;
  CXFA_Node* pRefNode = ToNode(lpScriptContext->GetThisObject());
  if (pOriginalObject->IsVariablesThis()) {
    pRefNode = ToNode(lpCurNode);
  }
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), pValue,
                                       dwFlag, false)) {
    return;
  }
  dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings;
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), pValue,
                                       dwFlag, false)) {
    return;
  }
  CXFA_Object* pScriptObject =
      lpScriptContext->GetVariablesThis(pOriginalObject, true);
  if (pScriptObject &&
      lpScriptContext->QueryVariableValue(pScriptObject->AsNode(), szPropName,
                                          pValue, true)) {
    return;
  }
  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->GetDocEnvironment()->GetGlobalProperty(pNotify->GetHDOC(),
                                                  szPropName, pValue);
}
void CXFA_ScriptContext::NormalPropertyGetter(CFXJSE_Value* pOriginalValue,
                                              const CFX_ByteStringC& szPropName,
                                              CFXJSE_Value* pReturnValue) {
  CXFA_Object* pOriginalObject = ToObject(pOriginalValue, nullptr);
  if (!pOriginalObject) {
    pReturnValue->SetUndefined();
    return;
  }
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  CXFA_ScriptContext* lpScriptContext =
      pOriginalObject->GetDocument()->GetScriptContext();
  CXFA_Object* pObject = lpScriptContext->GetVariablesThis(pOriginalObject);
  if (wsPropName == L"xfa") {
    CFXJSE_Value* pValue = lpScriptContext->GetJSValueFromMap(
        lpScriptContext->GetDocument()->GetRoot());
    pReturnValue->Assign(pValue);
    return;
  }
  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                    XFA_RESOLVENODE_Attributes;
  bool bRet = lpScriptContext->QueryNodeByFlag(
      ToNode(pObject), wsPropName.AsStringC(), pReturnValue, dwFlag, false);
  if (bRet) {
    return;
  }
  if (pObject == lpScriptContext->GetThisObject() ||
      (lpScriptContext->GetType() == XFA_SCRIPTLANGTYPE_Javascript &&
       !lpScriptContext->IsStrictScopeInJavaScript())) {
    dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings;
    bRet = lpScriptContext->QueryNodeByFlag(
        ToNode(pObject), wsPropName.AsStringC(), pReturnValue, dwFlag, false);
  }
  if (bRet) {
    return;
  }
  CXFA_Object* pScriptObject =
      lpScriptContext->GetVariablesThis(pOriginalObject, true);
  if (pScriptObject) {
    bRet = lpScriptContext->QueryVariableValue(ToNode(pScriptObject),
                                               szPropName, pReturnValue, true);
  }
  if (!bRet) {
    pReturnValue->SetUndefined();
  }
}
void CXFA_ScriptContext::NormalPropertySetter(CFXJSE_Value* pOriginalValue,
                                              const CFX_ByteStringC& szPropName,
                                              CFXJSE_Value* pReturnValue) {
  CXFA_Object* pOriginalObject = ToObject(pOriginalValue, nullptr);
  if (!pOriginalObject)
    return;

  CXFA_ScriptContext* lpScriptContext =
      pOriginalObject->GetDocument()->GetScriptContext();
  CXFA_Object* pObject = lpScriptContext->GetVariablesThis(pOriginalObject);
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  const XFA_SCRIPTATTRIBUTEINFO* lpAttributeInfo = XFA_GetScriptAttributeByName(
      pObject->GetElementType(), wsPropName.AsStringC());
  if (lpAttributeInfo) {
    (pObject->*(lpAttributeInfo->lpfnCallback))(
        pReturnValue, true, (XFA_ATTRIBUTE)lpAttributeInfo->eAttribute);
  } else {
    if (pObject->IsNode()) {
      if (wsPropName.GetAt(0) == '#') {
        wsPropName = wsPropName.Right(wsPropName.GetLength() - 1);
      }
      CXFA_Node* pNode = ToNode(pObject);
      CXFA_Node* pPropOrChild = nullptr;
      XFA_Element eType = XFA_GetElementTypeForName(wsPropName.AsStringC());
      if (eType != XFA_Element::Unknown)
        pPropOrChild = pNode->GetProperty(0, eType);
      else
        pPropOrChild = pNode->GetFirstChildByName(wsPropName.AsStringC());

      if (pPropOrChild) {
        CFX_WideString wsDefaultName(L"{default}");
        const XFA_SCRIPTATTRIBUTEINFO* lpAttrInfo =
            XFA_GetScriptAttributeByName(pPropOrChild->GetElementType(),
                                         wsDefaultName.AsStringC());
        if (lpAttrInfo) {
          (pPropOrChild->*(lpAttrInfo->lpfnCallback))(
              pReturnValue, true, (XFA_ATTRIBUTE)lpAttrInfo->eAttribute);
          return;
        }
      }
    }
    CXFA_Object* pScriptObject =
        lpScriptContext->GetVariablesThis(pOriginalObject, true);
    if (pScriptObject) {
      lpScriptContext->QueryVariableValue(ToNode(pScriptObject), szPropName,
                                          pReturnValue, false);
    }
  }
}
int32_t CXFA_ScriptContext::NormalPropTypeGetter(
    CFXJSE_Value* pOriginalValue,
    const CFX_ByteStringC& szPropName,
    bool bQueryIn) {
  CXFA_Object* pObject = ToObject(pOriginalValue, nullptr);
  if (!pObject)
    return FXJSE_ClassPropType_None;

  CXFA_ScriptContext* lpScriptContext =
      pObject->GetDocument()->GetScriptContext();
  pObject = lpScriptContext->GetVariablesThis(pObject);
  XFA_Element eType = pObject->GetElementType();
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  if (GetMethodByName(eType, wsPropName.AsStringC())) {
    return FXJSE_ClassPropType_Method;
  }
  if (bQueryIn &&
      !XFA_GetScriptAttributeByName(eType, wsPropName.AsStringC())) {
    return FXJSE_ClassPropType_None;
  }
  return FXJSE_ClassPropType_Property;
}
int32_t CXFA_ScriptContext::GlobalPropTypeGetter(
    CFXJSE_Value* pOriginalValue,
    const CFX_ByteStringC& szPropName,
    bool bQueryIn) {
  CXFA_Object* pObject = ToObject(pOriginalValue, nullptr);
  if (!pObject)
    return FXJSE_ClassPropType_None;

  CXFA_ScriptContext* lpScriptContext =
      pObject->GetDocument()->GetScriptContext();
  pObject = lpScriptContext->GetVariablesThis(pObject);
  XFA_Element eType = pObject->GetElementType();
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  if (GetMethodByName(eType, wsPropName.AsStringC())) {
    return FXJSE_ClassPropType_Method;
  }
  return FXJSE_ClassPropType_Property;
}
void CXFA_ScriptContext::NormalMethodCall(CFXJSE_Value* pThis,
                                          const CFX_ByteStringC& szFuncName,
                                          CFXJSE_Arguments& args) {
  CXFA_Object* pObject = ToObject(pThis, nullptr);
  if (!pObject)
    return;

  CXFA_ScriptContext* lpScriptContext =
      pObject->GetDocument()->GetScriptContext();
  pObject = lpScriptContext->GetVariablesThis(pObject);
  CFX_WideString wsFunName = CFX_WideString::FromUTF8(szFuncName);
  const XFA_METHODINFO* lpMethodInfo =
      GetMethodByName(pObject->GetElementType(), wsFunName.AsStringC());
  if (!lpMethodInfo)
    return;

  (pObject->*(lpMethodInfo->lpfnCallback))(&args);
}
bool CXFA_ScriptContext::IsStrictScopeInJavaScript() {
  return m_pDocument->HasFlag(XFA_DOCFLAG_StrictScoping);
}
XFA_SCRIPTLANGTYPE CXFA_ScriptContext::GetType() {
  return m_eScriptType;
}
void CXFA_ScriptContext::DefineJsContext() {
  m_JsContext = CFXJSE_Context::Create(m_pIsolate, &GlobalClassDescriptor,
                                       m_pDocument->GetRoot());
  RemoveBuiltInObjs(m_JsContext.get());
  m_JsContext->EnableCompatibleMode();
}

CFXJSE_Context* CXFA_ScriptContext::CreateVariablesContext(
    CXFA_Node* pScriptNode,
    CXFA_Node* pSubform) {
  if (!pScriptNode || !pSubform)
    return nullptr;

  auto pNewContext =
      CFXJSE_Context::Create(m_pIsolate, &VariablesClassDescriptor,
                             new CXFA_ThisProxy(pSubform, pScriptNode));
  RemoveBuiltInObjs(pNewContext.get());
  pNewContext->EnableCompatibleMode();
  CFXJSE_Context* pResult = pNewContext.get();
  m_mapVariableToContext[pScriptNode] = std::move(pNewContext);
  return pResult;
}

CXFA_Object* CXFA_ScriptContext::GetVariablesThis(CXFA_Object* pObject,
                                                  bool bScriptNode) {
  if (!pObject->IsVariablesThis())
    return pObject;

  CXFA_ThisProxy* pProxy = static_cast<CXFA_ThisProxy*>(pObject);
  return bScriptNode ? pProxy->GetScriptNode() : pProxy->GetThisNode();
}

bool CXFA_ScriptContext::RunVariablesScript(CXFA_Node* pScriptNode) {
  if (!pScriptNode)
    return false;

  if (pScriptNode->GetElementType() != XFA_Element::Script)
    return true;

  CXFA_Node* pParent = pScriptNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pParent || pParent->GetElementType() != XFA_Element::Variables)
    return false;

  auto it = m_mapVariableToContext.find(pScriptNode);
  if (it != m_mapVariableToContext.end() && it->second)
    return true;

  CXFA_Node* pTextNode = pScriptNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pTextNode)
    return false;

  CFX_WideStringC wsScript;
  if (!pTextNode->TryCData(XFA_ATTRIBUTE_Value, wsScript))
    return false;

  CFX_ByteString btScript = FX_UTF8Encode(wsScript);
  auto hRetValue = pdfium::MakeUnique<CFXJSE_Value>(m_pIsolate);
  CXFA_Node* pThisObject = pParent->GetNodeItem(XFA_NODEITEM_Parent);
  CFXJSE_Context* pVariablesContext =
      CreateVariablesContext(pScriptNode, pThisObject);
  CFX_AutoRestorer<CXFA_Object*> nodeRestorer(&m_pThisObject);
  m_pThisObject = pThisObject;
  return pVariablesContext->ExecuteScript(btScript.c_str(), hRetValue.get());
}

bool CXFA_ScriptContext::QueryVariableValue(CXFA_Node* pScriptNode,
                                            const CFX_ByteStringC& szPropName,
                                            CFXJSE_Value* pValue,
                                            bool bGetter) {
  if (!pScriptNode || pScriptNode->GetElementType() != XFA_Element::Script)
    return false;

  CXFA_Node* variablesNode = pScriptNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!variablesNode ||
      variablesNode->GetElementType() != XFA_Element::Variables)
    return false;

  auto it = m_mapVariableToContext.find(pScriptNode);
  if (it == m_mapVariableToContext.end() || !it->second)
    return false;

  CFXJSE_Context* pVariableContext = it->second.get();
  std::unique_ptr<CFXJSE_Value> pObject = pVariableContext->GetGlobalObject();
  auto hVariableValue = pdfium::MakeUnique<CFXJSE_Value>(m_pIsolate);
  if (!bGetter) {
    pObject->SetObjectOwnProperty(szPropName, pValue);
    return true;
  }
  if (pObject->HasObjectOwnProperty(szPropName, false)) {
    pObject->GetObjectProperty(szPropName, hVariableValue.get());
    if (hVariableValue->IsFunction())
      pValue->SetFunctionBind(hVariableValue.get(), pObject.get());
    else if (bGetter)
      pValue->Assign(hVariableValue.get());
    else
      hVariableValue.get()->Assign(pValue);
    return true;
  }
  return false;
}

void CXFA_ScriptContext::DefineJsClass() {
  m_pJsClass =
      CFXJSE_Class::Create(m_JsContext.get(), &NormalClassDescriptor, false);
}

void CXFA_ScriptContext::RemoveBuiltInObjs(CFXJSE_Context* pContext) const {
  static const CFX_ByteStringC OBJ_NAME[2] = {"Number", "Date"};
  std::unique_ptr<CFXJSE_Value> pObject = pContext->GetGlobalObject();
  auto hProp = pdfium::MakeUnique<CFXJSE_Value>(m_pIsolate);
  for (int i = 0; i < 2; ++i) {
    if (pObject->GetObjectProperty(OBJ_NAME[i], hProp.get()))
      pObject->DeleteObjectProperty(OBJ_NAME[i]);
  }
}
CFXJSE_Class* CXFA_ScriptContext::GetJseNormalClass() {
  return m_pJsClass;
}

int32_t CXFA_ScriptContext::ResolveObjects(CXFA_Object* refObject,
                                           const CFX_WideStringC& wsExpression,
                                           XFA_RESOLVENODE_RS& resolveNodeRS,
                                           uint32_t dwStyles,
                                           CXFA_Node* bindNode) {
  if (wsExpression.IsEmpty())
    return 0;

  if (m_eScriptType != XFA_SCRIPTLANGTYPE_Formcalc ||
      (dwStyles & (XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings))) {
    m_upObjectArray.clear();
  }
  if (refObject && refObject->IsNode() &&
      (dwStyles & (XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings))) {
    m_upObjectArray.push_back(refObject->AsNode());
  }

  bool bNextCreate = false;
  if (dwStyles & XFA_RESOLVENODE_CreateNode) {
    m_ResolveProcessor->GetNodeHelper()->SetCreateNodeType(bindNode);
  }
  m_ResolveProcessor->GetNodeHelper()->m_pCreateParent = nullptr;
  m_ResolveProcessor->GetNodeHelper()->m_iCurAllStart = -1;

  CXFA_ResolveNodesData rndFind;
  int32_t nStart = 0;
  int32_t nLevel = 0;
  int32_t nRet = -1;
  rndFind.m_pSC = this;
  std::vector<CXFA_Object*> findObjects;
  findObjects.push_back(refObject ? refObject : m_pDocument->GetRoot());
  int32_t nNodes = 0;
  while (true) {
    nNodes = pdfium::CollectionSize<int32_t>(findObjects);
    int32_t i = 0;
    rndFind.m_dwStyles = dwStyles;
    m_ResolveProcessor->SetCurStart(nStart);
    nStart = m_ResolveProcessor->GetFilter(wsExpression, nStart, rndFind);
    if (nStart < 1) {
      if ((dwStyles & XFA_RESOLVENODE_CreateNode) && !bNextCreate) {
        CXFA_Node* pDataNode = nullptr;
        nStart = m_ResolveProcessor->GetNodeHelper()->m_iCurAllStart;
        if (nStart != -1) {
          pDataNode = m_pDocument->GetNotBindNode(findObjects);
          if (pDataNode) {
            findObjects.clear();
            findObjects.push_back(pDataNode);
            break;
          }
        } else {
          pDataNode = findObjects.front()->AsNode();
          findObjects.clear();
          findObjects.push_back(pDataNode);
          break;
        }
        dwStyles |= XFA_RESOLVENODE_Bind;
        findObjects.clear();
        findObjects.push_back(
            m_ResolveProcessor->GetNodeHelper()->m_pAllStartParent);
        continue;
      } else {
        break;
      }
    }
    if (bNextCreate) {
      bool bCreate =
          m_ResolveProcessor->GetNodeHelper()->ResolveNodes_CreateNode(
              rndFind.m_wsName, rndFind.m_wsCondition,
              nStart == wsExpression.GetLength(), this);
      if (bCreate) {
        continue;
      } else {
        break;
      }
    }
    std::vector<CXFA_Object*> retObjects;
    while (i < nNodes) {
      bool bDataBind = false;
      if (((dwStyles & XFA_RESOLVENODE_Bind) ||
           (dwStyles & XFA_RESOLVENODE_CreateNode)) &&
          nNodes > 1) {
        CXFA_ResolveNodesData rndBind;
        m_ResolveProcessor->GetFilter(wsExpression, nStart, rndBind);
        m_ResolveProcessor->SetIndexDataBind(rndBind.m_wsCondition, i, nNodes);
        bDataBind = true;
      }
      rndFind.m_CurObject = findObjects[i++];
      rndFind.m_nLevel = nLevel;
      rndFind.m_dwFlag = XFA_RESOVENODE_RSTYPE_Nodes;
      nRet = m_ResolveProcessor->Resolve(rndFind);
      if (nRet < 1) {
        continue;
      }
      if (rndFind.m_dwFlag == XFA_RESOVENODE_RSTYPE_Attribute &&
          rndFind.m_pScriptAttribute && nStart < wsExpression.GetLength()) {
        auto pValue = pdfium::MakeUnique<CFXJSE_Value>(m_pIsolate);
        (rndFind.m_Objects.front()
             ->*(rndFind.m_pScriptAttribute->lpfnCallback))(
            pValue.get(), false,
            (XFA_ATTRIBUTE)rndFind.m_pScriptAttribute->eAttribute);
        rndFind.m_Objects.front() = ToObject(pValue.get(), nullptr);
      }
      if (!m_upObjectArray.empty())
        m_upObjectArray.pop_back();
      retObjects.insert(retObjects.end(), rndFind.m_Objects.begin(),
                        rndFind.m_Objects.end());
      rndFind.m_Objects.clear();
      if (bDataBind)
        break;
    }
    findObjects.clear();
    nNodes = pdfium::CollectionSize<int32_t>(retObjects);
    if (nNodes < 1) {
      if (dwStyles & XFA_RESOLVENODE_CreateNode) {
        bNextCreate = true;
        if (!m_ResolveProcessor->GetNodeHelper()->m_pCreateParent) {
          m_ResolveProcessor->GetNodeHelper()->m_pCreateParent =
              ToNode(rndFind.m_CurObject);
          m_ResolveProcessor->GetNodeHelper()->m_iCreateCount = 1;
        }
        bool bCreate =
            m_ResolveProcessor->GetNodeHelper()->ResolveNodes_CreateNode(
                rndFind.m_wsName, rndFind.m_wsCondition,
                nStart == wsExpression.GetLength(), this);
        if (bCreate) {
          continue;
        } else {
          break;
        }
      } else {
        break;
      }
    }
    findObjects =
        std::vector<CXFA_Object*>(retObjects.begin(), retObjects.end());
    rndFind.m_Objects.clear();
    if (nLevel == 0) {
      dwStyles &= ~(XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings);
    }
    nLevel++;
  }
  if (!bNextCreate) {
    resolveNodeRS.dwFlags = rndFind.m_dwFlag;
    if (nNodes > 0) {
      resolveNodeRS.objects.insert(resolveNodeRS.objects.end(),
                                   findObjects.begin(), findObjects.end());
    }
    if (rndFind.m_dwFlag == XFA_RESOVENODE_RSTYPE_Attribute) {
      resolveNodeRS.pScriptAttribute = rndFind.m_pScriptAttribute;
      return 1;
    }
  }
  if (dwStyles & (XFA_RESOLVENODE_CreateNode | XFA_RESOLVENODE_Bind |
                  XFA_RESOLVENODE_BindNew)) {
    m_ResolveProcessor->SetResultCreateNode(resolveNodeRS,
                                            rndFind.m_wsCondition);
    if (!bNextCreate && (dwStyles & XFA_RESOLVENODE_CreateNode)) {
      resolveNodeRS.dwFlags = XFA_RESOVENODE_RSTYPE_ExistNodes;
    }
    return pdfium::CollectionSize<int32_t>(resolveNodeRS.objects);
  }
  return nNodes;
}

void CXFA_ScriptContext::AddToCacheList(std::unique_ptr<CXFA_NodeList> pList) {
  m_CacheList.push_back(std::move(pList));
}

CFXJSE_Value* CXFA_ScriptContext::GetJSValueFromMap(CXFA_Object* pObject) {
  if (!pObject)
    return nullptr;
  if (pObject->IsNode())
    RunVariablesScript(pObject->AsNode());

  auto iter = m_mapObjectToValue.find(pObject);
  if (iter != m_mapObjectToValue.end())
    return iter->second.get();

  auto jsValue = pdfium::MakeUnique<CFXJSE_Value>(m_pIsolate);
  jsValue->SetObject(pObject, m_pJsClass);
  CFXJSE_Value* pValue = jsValue.get();
  m_mapObjectToValue.insert(std::make_pair(pObject, std::move(jsValue)));
  return pValue;
}
int32_t CXFA_ScriptContext::GetIndexByName(CXFA_Node* refNode) {
  CXFA_NodeHelper* lpNodeHelper = m_ResolveProcessor->GetNodeHelper();
  return lpNodeHelper->GetIndex(refNode, XFA_LOGIC_Transparent,
                                lpNodeHelper->NodeIsProperty(refNode), false);
}
int32_t CXFA_ScriptContext::GetIndexByClassName(CXFA_Node* refNode) {
  CXFA_NodeHelper* lpNodeHelper = m_ResolveProcessor->GetNodeHelper();
  return lpNodeHelper->GetIndex(refNode, XFA_LOGIC_Transparent,
                                lpNodeHelper->NodeIsProperty(refNode), true);
}
void CXFA_ScriptContext::GetSomExpression(CXFA_Node* refNode,
                                          CFX_WideString& wsExpression) {
  CXFA_NodeHelper* lpNodeHelper = m_ResolveProcessor->GetNodeHelper();
  lpNodeHelper->GetNameExpression(refNode, wsExpression, true,
                                  XFA_LOGIC_Transparent);
}
void CXFA_ScriptContext::SetNodesOfRunScript(std::vector<CXFA_Node*>* pArray) {
  m_pScriptNodeArray = pArray;
}

void CXFA_ScriptContext::AddNodesOfRunScript(
    const std::vector<CXFA_Node*>& nodes) {
  if (m_pScriptNodeArray && !nodes.empty())
    *m_pScriptNodeArray = nodes;
}

void CXFA_ScriptContext::AddNodesOfRunScript(CXFA_Node* pNode) {
  if (m_pScriptNodeArray && !pdfium::ContainsValue(*m_pScriptNodeArray, pNode))
    m_pScriptNodeArray->push_back(pNode);
}
