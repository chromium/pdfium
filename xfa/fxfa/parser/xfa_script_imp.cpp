// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_script_imp.h"

#include "core/fxcrt/include/fx_ext.h"
#include "xfa/fxfa/app/xfa_ffnotify.h"
#include "xfa/fxfa/parser/xfa_doclayout.h"
#include "xfa/fxfa/parser/xfa_document.h"
#include "xfa/fxfa/parser/xfa_localemgr.h"
#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxfa/parser/xfa_parser.h"
#include "xfa/fxfa/parser/xfa_script.h"
#include "xfa/fxfa/parser/xfa_script_nodehelper.h"
#include "xfa/fxfa/parser/xfa_script_resolveprocessor.h"
#include "xfa/fxfa/parser/xfa_utils.h"
#include "xfa/fxjse/cfxjse_arguments.h"
#include "xfa/fxjse/value.h"

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

const char kFormCalcRuntime[] = "foxit_xfa_formcalc_runtime";

CXFA_ThisProxy* ToThisProxy(CFXJSE_Value* pValue, CFXJSE_Class* pClass) {
  return static_cast<CXFA_ThisProxy*>(pValue->ToHostObject(pClass));
}

}  // namespace

// static.
CXFA_Object* CXFA_ScriptContext::ToObject(CFXJSE_Value* pValue,
                                          CFXJSE_Class* pClass) {
  return static_cast<CXFA_Object*>(pValue->ToHostObject(pClass));
}

CXFA_ScriptContext::CXFA_ScriptContext(CXFA_Document* pDocument)
    : m_pDocument(pDocument),
      m_pJsContext(nullptr),
      m_pIsolate(nullptr),
      m_pJsClass(nullptr),
      m_eScriptType(XFA_SCRIPTLANGTYPE_Unkown),
      m_pScriptNodeArray(nullptr),
      m_pResolveProcessor(nullptr),
      m_hFM2JSContext(nullptr),
      m_pThisObject(nullptr),
      m_dwBuiltInInFlags(0),
      m_eRunAtType(XFA_ATTRIBUTEENUM_Client) {
}

CXFA_ScriptContext::~CXFA_ScriptContext() {
  FX_POSITION ps = m_mapXFAToValue.GetStartPosition();
  while (ps) {
    CXFA_Object* pXFAObj;
    CFXJSE_Value* pValue;
    m_mapXFAToValue.GetNextAssoc(ps, pXFAObj, pValue);
    delete pValue;
  }
  m_mapXFAToValue.RemoveAll();
  ReleaseVariablesMap();

  delete m_hFM2JSContext;

  if (m_pJsContext) {
    FXJSE_Context_Release(m_pJsContext);
    m_pJsContext = NULL;
  }
  delete m_pResolveProcessor;
  m_upObjectArray.RemoveAll();
  for (int32_t i = 0; i < m_CacheListArray.GetSize(); i++)
    delete m_CacheListArray[i];
}
void CXFA_ScriptContext::Initialize(v8::Isolate* pIsolate) {
  m_pIsolate = pIsolate;
  DefineJsContext();
  DefineJsClass();
  m_pResolveProcessor = new CXFA_ResolveProcessor;
}
FX_BOOL CXFA_ScriptContext::RunScript(XFA_SCRIPTLANGTYPE eScriptType,
                                      const CFX_WideStringC& wsScript,
                                      CFXJSE_Value* hRetValue,
                                      CXFA_Object* pThisObject) {
  CFX_ByteString btScript;
  XFA_SCRIPTLANGTYPE eSaveType = m_eScriptType;
  m_eScriptType = eScriptType;
  if (eScriptType == XFA_SCRIPTLANGTYPE_Formcalc) {
    if (!m_hFM2JSContext) {
      m_hFM2JSContext =
          new CXFA_FM2JSContext(m_pIsolate, m_pJsContext, m_pDocument);
    }
    CFX_WideTextBuf wsJavaScript;
    CFX_WideString wsErrorInfo;
    int32_t iFlags =
        CXFA_FM2JSContext::Translate(wsScript, wsJavaScript, wsErrorInfo);
    if (iFlags) {
      hRetValue->SetUndefined();
      return FALSE;
    }
    btScript =
        FX_UTF8Encode(wsJavaScript.GetBuffer(), wsJavaScript.GetLength());
  } else {
    btScript = FX_UTF8Encode(wsScript.c_str(), wsScript.GetLength());
  }
  CXFA_Object* pOriginalObject = m_pThisObject;
  m_pThisObject = pThisObject;
  CFXJSE_Value* pValue = pThisObject ? GetJSValueFromMap(pThisObject) : NULL;
  FX_BOOL bRet =
      FXJSE_ExecuteScript(m_pJsContext, btScript.c_str(), hRetValue, pValue);
  m_pThisObject = pOriginalObject;
  m_eScriptType = eSaveType;
  return bRet;
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
  if (lpOrginalNode->GetObjectType() == XFA_OBJECTTYPE_VariablesThis) {
    pRefNode = ToNode(lpCurNode);
  }
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), pValue,
                                       dwFlag, TRUE)) {
    return;
  }
  if (lpOrginalNode->GetObjectType() == XFA_OBJECTTYPE_VariablesThis) {
    if (FXJSE_Value_IsUndefined(pValue)) {
      pObject->SetObjectOwnProperty(szPropName, pValue);
      return;
    }
  }
  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->GetDocProvider()->SetGlobalProperty(pNotify->GetHDOC(), szPropName,
                                               pValue);
}
FX_BOOL CXFA_ScriptContext::QueryNodeByFlag(CXFA_Node* refNode,
                                            const CFX_WideStringC& propname,
                                            CFXJSE_Value* pValue,
                                            uint32_t dwFlag,
                                            FX_BOOL bSetting) {
  if (!refNode)
    return false;
  XFA_RESOLVENODE_RS resolveRs;
  if (ResolveObjects(refNode, propname, resolveRs, dwFlag) <= 0)
    return false;
  if (resolveRs.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    pValue->Assign(GetJSValueFromMap(resolveRs.nodes[0]));
    return true;
  }
  if (resolveRs.dwFlags == XFA_RESOVENODE_RSTYPE_Attribute) {
    const XFA_SCRIPTATTRIBUTEINFO* lpAttributeInfo = resolveRs.pScriptAttribute;
    if (lpAttributeInfo) {
      (resolveRs.nodes[0]->*(lpAttributeInfo->lpfnCallback))(
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
      lpScriptContext->m_hFM2JSContext->GlobalPropertyGetter(pValue);
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
  if (pOriginalObject->GetObjectType() == XFA_OBJECTTYPE_VariablesThis) {
    pRefNode = ToNode(lpCurNode);
  }
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), pValue,
                                       dwFlag, FALSE)) {
    return;
  }
  dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings;
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), pValue,
                                       dwFlag, FALSE)) {
    return;
  }
  CXFA_Object* pScriptObject =
      lpScriptContext->GetVariablesThis(pOriginalObject, TRUE);
  if (pScriptObject &&
      lpScriptContext->QueryVariableValue(pScriptObject->AsNode(), szPropName,
                                          pValue, TRUE)) {
    return;
  }
  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->GetDocProvider()->GetGlobalProperty(pNotify->GetHDOC(), szPropName,
                                               pValue);
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
  if (wsPropName == FX_WSTRC(L"xfa")) {
    CFXJSE_Value* pValue = lpScriptContext->GetJSValueFromMap(
        lpScriptContext->GetDocument()->GetRoot());
    pReturnValue->Assign(pValue);
    return;
  }
  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                    XFA_RESOLVENODE_Attributes;
  FX_BOOL bRet = lpScriptContext->QueryNodeByFlag(
      ToNode(pObject), wsPropName.AsStringC(), pReturnValue, dwFlag, FALSE);
  if (bRet) {
    return;
  }
  if (pObject == lpScriptContext->GetThisObject() ||
      (lpScriptContext->GetType() == XFA_SCRIPTLANGTYPE_Javascript &&
       !lpScriptContext->IsStrictScopeInJavaScript())) {
    dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings;
    bRet = lpScriptContext->QueryNodeByFlag(
        ToNode(pObject), wsPropName.AsStringC(), pReturnValue, dwFlag, FALSE);
  }
  if (bRet) {
    return;
  }
  CXFA_Object* pScriptObject =
      lpScriptContext->GetVariablesThis(pOriginalObject, TRUE);
  if (pScriptObject) {
    bRet = lpScriptContext->QueryVariableValue(ToNode(pScriptObject),
                                               szPropName, pReturnValue, TRUE);
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
      pObject->GetClassID(), wsPropName.AsStringC());
  if (lpAttributeInfo) {
    (pObject->*(lpAttributeInfo->lpfnCallback))(
        pReturnValue, TRUE, (XFA_ATTRIBUTE)lpAttributeInfo->eAttribute);
  } else {
    if (pObject->IsNode()) {
      if (wsPropName.GetAt(0) == '#') {
        wsPropName = wsPropName.Right(wsPropName.GetLength() - 1);
      }
      CXFA_Node* pNode = ToNode(pObject);
      CXFA_Node* pPropOrChild = NULL;
      const XFA_ELEMENTINFO* lpElementInfo =
          XFA_GetElementByName(wsPropName.AsStringC());
      if (lpElementInfo) {
        pPropOrChild = pNode->GetProperty(0, lpElementInfo->eName);
      } else {
        pPropOrChild = pNode->GetFirstChildByName(wsPropName.AsStringC());
      }
      if (pPropOrChild) {
        CFX_WideString wsDefaultName(L"{default}");
        const XFA_SCRIPTATTRIBUTEINFO* lpAttrInfo =
            XFA_GetScriptAttributeByName(pPropOrChild->GetClassID(),
                                         wsDefaultName.AsStringC());
        if (lpAttrInfo) {
          (pPropOrChild->*(lpAttrInfo->lpfnCallback))(
              pReturnValue, TRUE, (XFA_ATTRIBUTE)lpAttrInfo->eAttribute);
          return;
        }
      }
    }
    CXFA_Object* pScriptObject =
        lpScriptContext->GetVariablesThis(pOriginalObject, TRUE);
    if (pScriptObject) {
      lpScriptContext->QueryVariableValue(ToNode(pScriptObject), szPropName,
                                          pReturnValue, FALSE);
    }
  }
}
int32_t CXFA_ScriptContext::NormalPropTypeGetter(
    CFXJSE_Value* pOriginalValue,
    const CFX_ByteStringC& szPropName,
    FX_BOOL bQueryIn) {
  CXFA_Object* pObject = ToObject(pOriginalValue, nullptr);
  if (!pObject)
    return FXJSE_ClassPropType_None;

  CXFA_ScriptContext* lpScriptContext =
      pObject->GetDocument()->GetScriptContext();
  pObject = lpScriptContext->GetVariablesThis(pObject);
  XFA_ELEMENT objElement = pObject->GetClassID();
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  if (XFA_GetMethodByName(objElement, wsPropName.AsStringC())) {
    return FXJSE_ClassPropType_Method;
  }
  if (bQueryIn &&
      !XFA_GetScriptAttributeByName(objElement, wsPropName.AsStringC())) {
    return FXJSE_ClassPropType_None;
  }
  return FXJSE_ClassPropType_Property;
}
int32_t CXFA_ScriptContext::GlobalPropTypeGetter(
    CFXJSE_Value* pOriginalValue,
    const CFX_ByteStringC& szPropName,
    FX_BOOL bQueryIn) {
  CXFA_Object* pObject = ToObject(pOriginalValue, nullptr);
  if (!pObject)
    return FXJSE_ClassPropType_None;

  CXFA_ScriptContext* lpScriptContext =
      pObject->GetDocument()->GetScriptContext();
  pObject = lpScriptContext->GetVariablesThis(pObject);
  XFA_ELEMENT objElement = pObject->GetClassID();
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  if (XFA_GetMethodByName(objElement, wsPropName.AsStringC())) {
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
      XFA_GetMethodByName(pObject->GetClassID(), wsFunName.AsStringC());
  if (!lpMethodInfo)
    return;

  (pObject->*(lpMethodInfo->lpfnCallback))(&args);
}
FX_BOOL CXFA_ScriptContext::IsStrictScopeInJavaScript() {
  return m_pDocument->HasFlag(XFA_DOCFLAG_StrictScoping);
}
XFA_SCRIPTLANGTYPE CXFA_ScriptContext::GetType() {
  return m_eScriptType;
}
void CXFA_ScriptContext::DefineJsContext() {
  m_pJsContext = FXJSE_Context_Create(m_pIsolate, &GlobalClassDescriptor,
                                      m_pDocument->GetRoot());
  RemoveBuiltInObjs(m_pJsContext);
  FXJSE_Context_EnableCompatibleMode(m_pJsContext);
}
CFXJSE_Context* CXFA_ScriptContext::CreateVariablesContext(
    CXFA_Node* pScriptNode,
    CXFA_Node* pSubform) {
  if (!pScriptNode || !pSubform)
    return nullptr;

  CFXJSE_Context* pVariablesContext =
      FXJSE_Context_Create(m_pIsolate, &VariablesClassDescriptor,
                           new CXFA_ThisProxy(pSubform, pScriptNode));
  RemoveBuiltInObjs(pVariablesContext);
  FXJSE_Context_EnableCompatibleMode(pVariablesContext);
  m_mapVariableToContext.SetAt(pScriptNode, pVariablesContext);
  return pVariablesContext;
}
CXFA_Object* CXFA_ScriptContext::GetVariablesThis(CXFA_Object* pObject,
                                                  FX_BOOL bScriptNode) {
  if (pObject->GetObjectType() != XFA_OBJECTTYPE_VariablesThis)
    return pObject;

  CXFA_ThisProxy* pProxy = static_cast<CXFA_ThisProxy*>(pObject);
  return bScriptNode ? pProxy->GetScriptNode() : pProxy->GetThisNode();
}

FX_BOOL CXFA_ScriptContext::RunVariablesScript(CXFA_Node* pScriptNode) {
  if (!pScriptNode)
    return FALSE;

  if (pScriptNode->GetClassID() != XFA_ELEMENT_Script)
    return TRUE;

  CXFA_Node* pParent = pScriptNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pParent || pParent->GetClassID() != XFA_ELEMENT_Variables)
    return FALSE;

  if (m_mapVariableToContext.GetValueAt(pScriptNode))
    return TRUE;

  CXFA_Node* pTextNode = pScriptNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pTextNode)
    return FALSE;

  CFX_WideStringC wsScript;
  if (!pTextNode->TryCData(XFA_ATTRIBUTE_Value, wsScript))
    return FALSE;

  CFX_ByteString btScript =
      FX_UTF8Encode(wsScript.c_str(), wsScript.GetLength());
  std::unique_ptr<CFXJSE_Value> hRetValue(new CFXJSE_Value(m_pIsolate));
  CXFA_Node* pThisObject = pParent->GetNodeItem(XFA_NODEITEM_Parent);
  CFXJSE_Context* pVariablesContext =
      CreateVariablesContext(pScriptNode, pThisObject);
  CXFA_Object* pOriginalObject = m_pThisObject;
  m_pThisObject = pThisObject;
  FX_BOOL bRet =
      FXJSE_ExecuteScript(pVariablesContext, btScript.c_str(), hRetValue.get());
  m_pThisObject = pOriginalObject;
  return bRet;
}

FX_BOOL CXFA_ScriptContext::QueryVariableValue(
    CXFA_Node* pScriptNode,
    const CFX_ByteStringC& szPropName,
    CFXJSE_Value* pValue,
    FX_BOOL bGetter) {
  if (!pScriptNode || pScriptNode->GetClassID() != XFA_ELEMENT_Script)
    return FALSE;

  CXFA_Node* variablesNode = pScriptNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!variablesNode || variablesNode->GetClassID() != XFA_ELEMENT_Variables)
    return FALSE;

  void* lpVariables = m_mapVariableToContext.GetValueAt(pScriptNode);
  if (!lpVariables)
    return FALSE;

  FX_BOOL bRes = FALSE;
  CFXJSE_Context* pVariableContext = static_cast<CFXJSE_Context*>(lpVariables);
  std::unique_ptr<CFXJSE_Value> pObject(
      FXJSE_Context_GetGlobalObject(pVariableContext));
  std::unique_ptr<CFXJSE_Value> hVariableValue(new CFXJSE_Value(m_pIsolate));
  if (!bGetter) {
    pObject->SetObjectOwnProperty(szPropName, pValue);
    bRes = TRUE;
  } else if (pObject->HasObjectOwnProperty(szPropName, FALSE)) {
    pObject->GetObjectProperty(szPropName, hVariableValue.get());
    if (FXJSE_Value_IsFunction(hVariableValue.get()))
      pValue->SetFunctionBind(hVariableValue.get(), pObject.get());
    else if (bGetter)
      pValue->Assign(hVariableValue.get());
    else
      hVariableValue.get()->Assign(pValue);
    bRes = TRUE;
  }
  return bRes;
}

void CXFA_ScriptContext::ReleaseVariablesMap() {
  FX_POSITION ps = m_mapVariableToContext.GetStartPosition();
  while (ps) {
    CXFA_Object* pScriptNode;
    CFXJSE_Context* pVariableContext = nullptr;
    m_mapVariableToContext.GetNextAssoc(ps, pScriptNode, pVariableContext);
    std::unique_ptr<CFXJSE_Value> pObject(
        FXJSE_Context_GetGlobalObject(pVariableContext));
    delete ToThisProxy(pObject.get(), nullptr);
    FXJSE_Context_Release(pVariableContext);
  }
  m_mapVariableToContext.RemoveAll();
}

void CXFA_ScriptContext::DefineJsClass() {
  m_pJsClass = FXJSE_DefineClass(m_pJsContext, &NormalClassDescriptor);
}

void CXFA_ScriptContext::RemoveBuiltInObjs(CFXJSE_Context* pContext) const {
  static const CFX_ByteStringC OBJ_NAME[2] = {"Number", "Date"};
  std::unique_ptr<CFXJSE_Value> pObject(
      FXJSE_Context_GetGlobalObject(pContext));
  std::unique_ptr<CFXJSE_Value> hProp(new CFXJSE_Value(m_pIsolate));
  for (int i = 0; i < 2; ++i) {
    if (pObject->GetObjectProperty(OBJ_NAME[i], hProp.get()))
      pObject.get()->DeleteObjectProperty(OBJ_NAME[i]);
  }
}
CFXJSE_Class* CXFA_ScriptContext::GetJseNormalClass() {
  return m_pJsClass;
}
int32_t CXFA_ScriptContext::ResolveObjects(CXFA_Object* refNode,
                                           const CFX_WideStringC& wsExpression,
                                           XFA_RESOLVENODE_RS& resolveNodeRS,
                                           uint32_t dwStyles,
                                           CXFA_Node* bindNode) {
  if (wsExpression.IsEmpty()) {
    return 0;
  }
  if (m_eScriptType != XFA_SCRIPTLANGTYPE_Formcalc ||
      (dwStyles & (XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings))) {
    m_upObjectArray.RemoveAll();
  }
  if (refNode && refNode->IsNode() &&
      (dwStyles & (XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings))) {
    m_upObjectArray.Add(refNode->AsNode());
  }
  FX_BOOL bNextCreate = FALSE;
  if (dwStyles & XFA_RESOLVENODE_CreateNode) {
    m_pResolveProcessor->GetNodeHelper()->XFA_SetCreateNodeType(bindNode);
  }
  m_pResolveProcessor->GetNodeHelper()->m_pCreateParent = NULL;
  m_pResolveProcessor->GetNodeHelper()->m_iCurAllStart = -1;
  CXFA_ResolveNodesData rndFind;
  int32_t nStart = 0;
  int32_t nLevel = 0;
  int32_t nRet = -1;
  rndFind.m_pSC = this;
  CXFA_ObjArray findNodes;
  findNodes.Add(refNode ? refNode : m_pDocument->GetRoot());
  int32_t nNodes = 0;
  while (TRUE) {
    nNodes = findNodes.GetSize();
    int32_t i = 0;
    rndFind.m_dwStyles = dwStyles;
    m_pResolveProcessor->m_iCurStart = nStart;
    nStart = m_pResolveProcessor->XFA_ResolveNodes_GetFilter(wsExpression,
                                                             nStart, rndFind);
    if (nStart < 1) {
      if ((dwStyles & XFA_RESOLVENODE_CreateNode) && !bNextCreate) {
        CXFA_Node* pDataNode = NULL;
        nStart = m_pResolveProcessor->GetNodeHelper()->m_iCurAllStart;
        if (nStart != -1) {
          pDataNode = m_pDocument->GetNotBindNode(findNodes);
          if (pDataNode) {
            findNodes.RemoveAll();
            findNodes.Add(pDataNode);
            break;
          }
        } else {
          pDataNode = findNodes[0]->AsNode();
          findNodes.RemoveAll();
          findNodes.Add(pDataNode);
          break;
        }
        dwStyles |= XFA_RESOLVENODE_Bind;
        findNodes.RemoveAll();
        findNodes.Add(m_pResolveProcessor->GetNodeHelper()->m_pAllStartParent);
        continue;
      } else {
        break;
      }
    }
    if (bNextCreate) {
      FX_BOOL bCreate =
          m_pResolveProcessor->GetNodeHelper()->XFA_ResolveNodes_CreateNode(
              rndFind.m_wsName, rndFind.m_wsCondition,
              nStart == wsExpression.GetLength(), this);
      if (bCreate) {
        continue;
      } else {
        break;
      }
    }
    CXFA_ObjArray retNodes;
    while (i < nNodes) {
      FX_BOOL bDataBind = FALSE;
      if (((dwStyles & XFA_RESOLVENODE_Bind) ||
           (dwStyles & XFA_RESOLVENODE_CreateNode)) &&
          nNodes > 1) {
        CXFA_ResolveNodesData rndBind;
        m_pResolveProcessor->XFA_ResolveNodes_GetFilter(wsExpression, nStart,
                                                        rndBind);
        m_pResolveProcessor->XFA_ResolveNode_SetIndexDataBind(
            rndBind.m_wsCondition, i, nNodes);
        bDataBind = TRUE;
      }
      rndFind.m_CurNode = findNodes[i++];
      rndFind.m_nLevel = nLevel;
      rndFind.m_dwFlag = XFA_RESOVENODE_RSTYPE_Nodes;
      nRet = m_pResolveProcessor->XFA_ResolveNodes(rndFind);
      if (nRet < 1) {
        continue;
      }
      if (rndFind.m_dwFlag == XFA_RESOVENODE_RSTYPE_Attribute &&
          rndFind.m_pScriptAttribute && nStart < wsExpression.GetLength()) {
        std::unique_ptr<CFXJSE_Value> pValue(new CFXJSE_Value(m_pIsolate));
        (rndFind.m_Nodes[0]->*(rndFind.m_pScriptAttribute->lpfnCallback))(
            pValue.get(), FALSE,
            (XFA_ATTRIBUTE)rndFind.m_pScriptAttribute->eAttribute);
        rndFind.m_Nodes.SetAt(0, ToObject(pValue.get(), nullptr));
      }
      int32_t iSize = m_upObjectArray.GetSize();
      if (iSize) {
        m_upObjectArray.RemoveAt(iSize - 1);
      }
      retNodes.Append(rndFind.m_Nodes);
      rndFind.m_Nodes.RemoveAll();
      if (bDataBind) {
        break;
      }
    }
    findNodes.RemoveAll();
    nNodes = retNodes.GetSize();
    if (nNodes < 1) {
      if (dwStyles & XFA_RESOLVENODE_CreateNode) {
        bNextCreate = TRUE;
        if (m_pResolveProcessor->GetNodeHelper()->m_pCreateParent == NULL) {
          m_pResolveProcessor->GetNodeHelper()->m_pCreateParent =
              ToNode(rndFind.m_CurNode);
          m_pResolveProcessor->GetNodeHelper()->m_iCreateCount = 1;
        }
        FX_BOOL bCreate =
            m_pResolveProcessor->GetNodeHelper()->XFA_ResolveNodes_CreateNode(
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
    findNodes.Copy(retNodes);
    rndFind.m_Nodes.RemoveAll();
    if (nLevel == 0) {
      dwStyles &= ~(XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings);
    }
    nLevel++;
  }
  if (!bNextCreate) {
    resolveNodeRS.dwFlags = rndFind.m_dwFlag;
    if (nNodes > 0) {
      resolveNodeRS.nodes.Append(findNodes);
    }
    if (rndFind.m_dwFlag == XFA_RESOVENODE_RSTYPE_Attribute) {
      resolveNodeRS.pScriptAttribute = rndFind.m_pScriptAttribute;
      return 1;
    }
  }
  if (dwStyles & (XFA_RESOLVENODE_CreateNode | XFA_RESOLVENODE_Bind |
                  XFA_RESOLVENODE_BindNew)) {
    m_pResolveProcessor->XFA_ResolveNode_SetResultCreateNode(
        resolveNodeRS, rndFind.m_wsCondition);
    if (!bNextCreate && (dwStyles & XFA_RESOLVENODE_CreateNode)) {
      resolveNodeRS.dwFlags = XFA_RESOVENODE_RSTYPE_ExistNodes;
    }
    return resolveNodeRS.nodes.GetSize();
  }
  return nNodes;
}
CFXJSE_Value* CXFA_ScriptContext::GetJSValueFromMap(CXFA_Object* pObject) {
  if (!pObject)
    return nullptr;
  if (pObject->IsNode())
    RunVariablesScript(pObject->AsNode());

  void* pValue = m_mapXFAToValue.GetValueAt(pObject);
  if (!pValue) {
    CFXJSE_Value* jsValue = new CFXJSE_Value(m_pIsolate);
    jsValue->SetObject(pObject, m_pJsClass);
    m_mapXFAToValue.SetAt(pObject, jsValue);
    pValue = jsValue;
  }
  return static_cast<CFXJSE_Value*>(pValue);
}
int32_t CXFA_ScriptContext::GetIndexByName(CXFA_Node* refNode) {
  CXFA_NodeHelper* lpNodeHelper = m_pResolveProcessor->GetNodeHelper();
  return lpNodeHelper->XFA_GetIndex(refNode, XFA_LOGIC_Transparent,
                                    lpNodeHelper->XFA_NodeIsProperty(refNode),
                                    FALSE);
}
int32_t CXFA_ScriptContext::GetIndexByClassName(CXFA_Node* refNode) {
  CXFA_NodeHelper* lpNodeHelper = m_pResolveProcessor->GetNodeHelper();
  return lpNodeHelper->XFA_GetIndex(refNode, XFA_LOGIC_Transparent,
                                    lpNodeHelper->XFA_NodeIsProperty(refNode),
                                    TRUE);
}
void CXFA_ScriptContext::GetSomExpression(CXFA_Node* refNode,
                                          CFX_WideString& wsExpression) {
  CXFA_NodeHelper* lpNodeHelper = m_pResolveProcessor->GetNodeHelper();
  lpNodeHelper->XFA_GetNameExpression(refNode, wsExpression, TRUE,
                                      XFA_LOGIC_Transparent);
}
void CXFA_ScriptContext::SetNodesOfRunScript(CXFA_NodeArray* pArray) {
  m_pScriptNodeArray = pArray;
}
void CXFA_ScriptContext::AddNodesOfRunScript(const CXFA_NodeArray& nodes) {
  if (!m_pScriptNodeArray) {
    return;
  }
  if (nodes.GetSize() > 0) {
    m_pScriptNodeArray->Copy(nodes);
  }
}
void CXFA_ScriptContext::AddNodesOfRunScript(CXFA_Node* pNode) {
  if (!m_pScriptNodeArray) {
    return;
  }
  if (m_pScriptNodeArray->Find(pNode) == -1) {
    m_pScriptNodeArray->Add(pNode);
  }
}
