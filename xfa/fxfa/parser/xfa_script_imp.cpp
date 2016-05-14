// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_script_imp.h"

#include "core/fxcrt/include/fx_ext.h"
#include "xfa/fxfa/app/xfa_ffnotify.h"
#include "xfa/fxfa/fm2js/xfa_fm2jsapi.h"
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

CXFA_ScriptContext::CXFA_ScriptContext(CXFA_Document* pDocument)
    : m_pDocument(pDocument),
      m_hJsContext(nullptr),
      m_hJsRuntime(nullptr),
      m_hJsClass(nullptr),
      m_eScriptType(XFA_SCRIPTLANGTYPE_Unkown),
      m_pScriptNodeArray(nullptr),
      m_pResolveProcessor(nullptr),
      m_hFM2JSContext(nullptr),
      m_pThisObject(nullptr),
      m_dwBuiltInInFlags(0),
      m_eRunAtType(XFA_ATTRIBUTEENUM_Client) {
  FXSYS_memset(&m_JsGlobalClass, 0, sizeof(FXJSE_CLASS));
  FXSYS_memset(&m_JsNormalClass, 0, sizeof(FXJSE_CLASS));
}
CXFA_ScriptContext::~CXFA_ScriptContext() {
  FX_POSITION ps = m_mapXFAToHValue.GetStartPosition();
  while (ps) {
    CXFA_Object* pXFAObj;
    FXJSE_HVALUE pValue;
    m_mapXFAToHValue.GetNextAssoc(ps, pXFAObj, pValue);
    FXJSE_Value_Release(pValue);
  }
  m_mapXFAToHValue.RemoveAll();
  ReleaseVariablesMap();
  if (m_hFM2JSContext) {
    XFA_FM2JS_ContextRelease(m_hFM2JSContext);
    m_hFM2JSContext = NULL;
  }
  if (m_hJsContext) {
    FXJSE_Context_Release(m_hJsContext);
    m_hJsContext = NULL;
  }
  delete m_pResolveProcessor;
  m_upObjectArray.RemoveAll();
  for (int32_t i = 0; i < m_CacheListArray.GetSize(); i++)
    delete m_CacheListArray[i];
}
void CXFA_ScriptContext::Initialize(FXJSE_HRUNTIME hRuntime) {
  m_hJsRuntime = hRuntime;
  DefineJsContext();
  DefineJsClass();
  m_pResolveProcessor = new CXFA_ResolveProcessor;
}
void CXFA_ScriptContext::Release() {
  delete this;
}
FX_BOOL CXFA_ScriptContext::RunScript(XFA_SCRIPTLANGTYPE eScriptType,
                                      const CFX_WideStringC& wsScript,
                                      FXJSE_HVALUE hRetValue,
                                      CXFA_Object* pThisObject) {
  CFX_ByteString btScript;
  XFA_SCRIPTLANGTYPE eSaveType = m_eScriptType;
  m_eScriptType = eScriptType;
  if (eScriptType == XFA_SCRIPTLANGTYPE_Formcalc) {
    if (!m_hFM2JSContext) {
      m_hFM2JSContext = XFA_FM2JS_ContextCreate();
      XFA_FM2JS_ContextInitialize(m_hFM2JSContext, m_hJsRuntime, m_hJsContext,
                                  m_pDocument);
    }
    CFX_WideTextBuf wsJavaScript;
    CFX_WideString wsErrorInfo;
    int32_t iFlags = XFA_FM2JS_Translate(wsScript, wsJavaScript, wsErrorInfo);
    if (iFlags) {
      FXJSE_Value_SetUndefined(hRetValue);
      return FALSE;
    }
    btScript =
        FX_UTF8Encode(wsJavaScript.GetBuffer(), wsJavaScript.GetLength());
  } else {
    btScript = FX_UTF8Encode(wsScript.c_str(), wsScript.GetLength());
  }
  CXFA_Object* pOriginalObject = m_pThisObject;
  m_pThisObject = pThisObject;
  FXJSE_HVALUE pValue = pThisObject ? GetJSValueFromMap(pThisObject) : NULL;
  FX_BOOL bRet =
      FXJSE_ExecuteScript(m_hJsContext, btScript.c_str(), hRetValue, pValue);
  m_pThisObject = pOriginalObject;
  m_eScriptType = eSaveType;
  return bRet;
}
void CXFA_ScriptContext::GlobalPropertySetter(FXJSE_HOBJECT hObject,
                                              const CFX_ByteStringC& szPropName,
                                              FXJSE_HVALUE hValue) {
  CXFA_Object* lpOrginalNode =
      (CXFA_Object*)FXJSE_Value_ToObject(hObject, NULL);
  CXFA_Document* pDoc = lpOrginalNode->GetDocument();
  CXFA_ScriptContext* lpScriptContext =
      (CXFA_ScriptContext*)pDoc->GetScriptContext();
  CXFA_Object* lpCurNode = lpScriptContext->GetVariablesThis(lpOrginalNode);
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  uint32_t dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings |
                    XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                    XFA_RESOLVENODE_Attributes;
  CXFA_Node* pRefNode = ToNode(lpScriptContext->GetThisObject());
  if (lpOrginalNode->GetObjectType() == XFA_OBJECTTYPE_VariablesThis) {
    pRefNode = ToNode(lpCurNode);
  }
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), hValue,
                                       dwFlag, TRUE)) {
    return;
  }
  if (lpOrginalNode->GetObjectType() == XFA_OBJECTTYPE_VariablesThis) {
    if (FXJSE_Value_IsUndefined(hValue)) {
      FXJSE_Value_SetObjectOwnProp(hObject, szPropName, hValue);
      return;
    }
  }
  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->GetDocProvider()->SetGlobalProperty(pNotify->GetHDOC(), szPropName,
                                               hValue);
}
FX_BOOL CXFA_ScriptContext::QueryNodeByFlag(CXFA_Node* refNode,
                                            const CFX_WideStringC& propname,
                                            FXJSE_HVALUE hValue,
                                            uint32_t dwFlag,
                                            FX_BOOL bSetting) {
  if (!refNode)
    return false;
  XFA_RESOLVENODE_RS resolveRs;
  if (ResolveObjects(refNode, propname, resolveRs, dwFlag) <= 0)
    return false;
  if (resolveRs.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    FXJSE_HVALUE pValue = GetJSValueFromMap(resolveRs.nodes[0]);
    FXJSE_Value_Set(hValue, pValue);
    return true;
  }
  if (resolveRs.dwFlags == XFA_RESOVENODE_RSTYPE_Attribute) {
    const XFA_SCRIPTATTRIBUTEINFO* lpAttributeInfo = resolveRs.pScriptAttribute;
    if (lpAttributeInfo) {
      (resolveRs.nodes[0]->*(lpAttributeInfo->lpfnCallback))(
          hValue, bSetting, (XFA_ATTRIBUTE)lpAttributeInfo->eAttribute);
    }
  }
  return true;
}
void CXFA_ScriptContext::GlobalPropertyGetter(FXJSE_HOBJECT hObject,
                                              const CFX_ByteStringC& szPropName,
                                              FXJSE_HVALUE hValue) {
  CXFA_Object* pOrginalObject =
      (CXFA_Object*)FXJSE_Value_ToObject(hObject, NULL);
  CXFA_Document* pDoc = pOrginalObject->GetDocument();
  CXFA_ScriptContext* lpScriptContext =
      (CXFA_ScriptContext*)pDoc->GetScriptContext();
  CXFA_Object* lpCurNode = lpScriptContext->GetVariablesThis(pOrginalObject);
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  if (lpScriptContext->GetType() == XFA_SCRIPTLANGTYPE_Formcalc) {
    if (szPropName == FOXIT_XFA_FM2JS_FORMCALC_RUNTIME) {
      XFA_FM2JS_GlobalPropertyGetter(lpScriptContext->m_hFM2JSContext, hValue);
      return;
    }
    uint32_t uHashCode = FX_HashCode_GetW(wsPropName.AsStringC(), false);
    if (uHashCode != XFA_HASHCODE_Layout) {
      CXFA_Object* pObject =
          lpScriptContext->GetDocument()->GetXFAObject(uHashCode);
      if (pObject) {
        FXJSE_Value_Set(hValue, lpScriptContext->GetJSValueFromMap(pObject));
        return;
      }
    }
  }
  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                    XFA_RESOLVENODE_Attributes;
  CXFA_Node* pRefNode = ToNode(lpScriptContext->GetThisObject());
  if (pOrginalObject->GetObjectType() == XFA_OBJECTTYPE_VariablesThis) {
    pRefNode = ToNode(lpCurNode);
  }
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), hValue,
                                       dwFlag, FALSE)) {
    return;
  }
  dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings;
  if (lpScriptContext->QueryNodeByFlag(pRefNode, wsPropName.AsStringC(), hValue,
                                       dwFlag, FALSE)) {
    return;
  }
  CXFA_Object* pScriptObject =
      lpScriptContext->GetVariablesThis(pOrginalObject, TRUE);
  if (pScriptObject &&
      lpScriptContext->QueryVariableHValue(pScriptObject->AsNode(), szPropName,
                                           hValue, TRUE)) {
    return;
  }
  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return;
  }
  pNotify->GetDocProvider()->GetGlobalProperty(pNotify->GetHDOC(), szPropName,
                                               hValue);
}
void CXFA_ScriptContext::NormalPropertyGetter(FXJSE_HOBJECT hObject,
                                              const CFX_ByteStringC& szPropName,
                                              FXJSE_HVALUE hValue) {
  CXFA_Object* pOrginalObject =
      (CXFA_Object*)FXJSE_Value_ToObject(hObject, NULL);
  if (pOrginalObject == NULL) {
    FXJSE_Value_SetUndefined(hValue);
    return;
  }
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  CXFA_ScriptContext* lpScriptContext =
      (CXFA_ScriptContext*)pOrginalObject->GetDocument()->GetScriptContext();
  CXFA_Object* pObject = lpScriptContext->GetVariablesThis(pOrginalObject);
  if (wsPropName == FX_WSTRC(L"xfa")) {
    FXJSE_HVALUE pValue = lpScriptContext->GetJSValueFromMap(
        lpScriptContext->GetDocument()->GetRoot());
    FXJSE_Value_Set(hValue, pValue);
    return;
  }
  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                    XFA_RESOLVENODE_Attributes;
  FX_BOOL bRet = lpScriptContext->QueryNodeByFlag(
      ToNode(pObject), wsPropName.AsStringC(), hValue, dwFlag, FALSE);
  if (bRet) {
    return;
  }
  if (pObject == lpScriptContext->GetThisObject() ||
      (lpScriptContext->GetType() == XFA_SCRIPTLANGTYPE_Javascript &&
       !lpScriptContext->IsStrictScopeInJavaScript())) {
    dwFlag = XFA_RESOLVENODE_Parent | XFA_RESOLVENODE_Siblings;
    bRet = lpScriptContext->QueryNodeByFlag(
        ToNode(pObject), wsPropName.AsStringC(), hValue, dwFlag, FALSE);
  }
  if (bRet) {
    return;
  }
  CXFA_Object* pScriptObject =
      lpScriptContext->GetVariablesThis(pOrginalObject, TRUE);
  if (pScriptObject) {
    bRet = lpScriptContext->QueryVariableHValue(ToNode(pScriptObject),
                                                szPropName, hValue, TRUE);
  }
  if (!bRet) {
    FXJSE_Value_SetUndefined(hValue);
  }
}
void CXFA_ScriptContext::NormalPropertySetter(FXJSE_HOBJECT hObject,
                                              const CFX_ByteStringC& szPropName,
                                              FXJSE_HVALUE hValue) {
  CXFA_Object* pOrginalObject =
      (CXFA_Object*)FXJSE_Value_ToObject(hObject, NULL);
  if (pOrginalObject == NULL) {
    return;
  }
  CXFA_ScriptContext* lpScriptContext =
      (CXFA_ScriptContext*)pOrginalObject->GetDocument()->GetScriptContext();
  CXFA_Object* pObject = lpScriptContext->GetVariablesThis(pOrginalObject);
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  const XFA_SCRIPTATTRIBUTEINFO* lpAttributeInfo = XFA_GetScriptAttributeByName(
      pObject->GetClassID(), wsPropName.AsStringC());
  if (lpAttributeInfo) {
    (pObject->*(lpAttributeInfo->lpfnCallback))(
        hValue, TRUE, (XFA_ATTRIBUTE)lpAttributeInfo->eAttribute);
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
        const XFA_SCRIPTATTRIBUTEINFO* lpAttributeInfo =
            XFA_GetScriptAttributeByName(pPropOrChild->GetClassID(),
                                         wsDefaultName.AsStringC());
        if (lpAttributeInfo) {
          (pPropOrChild->*(lpAttributeInfo->lpfnCallback))(
              hValue, TRUE, (XFA_ATTRIBUTE)lpAttributeInfo->eAttribute);
          return;
        }
      }
    }
    CXFA_Object* pScriptObject =
        lpScriptContext->GetVariablesThis(pOrginalObject, TRUE);
    if (pScriptObject) {
      lpScriptContext->QueryVariableHValue(ToNode(pScriptObject), szPropName,
                                           hValue, FALSE);
    }
  }
}
int32_t CXFA_ScriptContext::NormalPropTypeGetter(
    FXJSE_HOBJECT hObject,
    const CFX_ByteStringC& szPropName,
    FX_BOOL bQueryIn) {
  CXFA_Object* pObject = (CXFA_Object*)FXJSE_Value_ToObject(hObject, NULL);
  if (pObject == NULL) {
    return FXJSE_ClassPropType_None;
  }
  CXFA_ScriptContext* lpScriptContext =
      (CXFA_ScriptContext*)pObject->GetDocument()->GetScriptContext();
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
    FXJSE_HOBJECT hObject,
    const CFX_ByteStringC& szPropName,
    FX_BOOL bQueryIn) {
  CXFA_Object* pObject = (CXFA_Object*)FXJSE_Value_ToObject(hObject, NULL);
  if (pObject == NULL) {
    return FXJSE_ClassPropType_None;
  }
  CXFA_ScriptContext* lpScriptContext =
      (CXFA_ScriptContext*)pObject->GetDocument()->GetScriptContext();
  pObject = lpScriptContext->GetVariablesThis(pObject);
  XFA_ELEMENT objElement = pObject->GetClassID();
  CFX_WideString wsPropName = CFX_WideString::FromUTF8(szPropName);
  if (XFA_GetMethodByName(objElement, wsPropName.AsStringC())) {
    return FXJSE_ClassPropType_Method;
  }
  return FXJSE_ClassPropType_Property;
}
void CXFA_ScriptContext::NormalMethodCall(FXJSE_HOBJECT hThis,
                                          const CFX_ByteStringC& szFuncName,
                                          CFXJSE_Arguments& args) {
  CXFA_Object* pObject = (CXFA_Object*)FXJSE_Value_ToObject(hThis, NULL);
  if (pObject == NULL) {
    return;
  }
  CXFA_ScriptContext* lpScriptContext =
      (CXFA_ScriptContext*)pObject->GetDocument()->GetScriptContext();
  pObject = lpScriptContext->GetVariablesThis(pObject);
  CFX_WideString wsFunName = CFX_WideString::FromUTF8(szFuncName);
  const XFA_METHODINFO* lpMethodInfo =
      XFA_GetMethodByName(pObject->GetClassID(), wsFunName.AsStringC());
  if (NULL == lpMethodInfo) {
    return;
  }
  (pObject->*(lpMethodInfo->lpfnCallback))(&args);
}
FX_BOOL CXFA_ScriptContext::IsStrictScopeInJavaScript() {
  return m_pDocument->HasFlag(XFA_DOCFLAG_StrictScoping);
}
XFA_SCRIPTLANGTYPE CXFA_ScriptContext::GetType() {
  return m_eScriptType;
}
void CXFA_ScriptContext::DefineJsContext() {
  m_JsGlobalClass.constructor = NULL;
  m_JsGlobalClass.name = "Root";
  m_JsGlobalClass.propNum = 0;
  m_JsGlobalClass.properties = NULL;
  m_JsGlobalClass.methNum = 0;
  m_JsGlobalClass.methods = NULL;
  m_JsGlobalClass.dynPropGetter = CXFA_ScriptContext::GlobalPropertyGetter;
  m_JsGlobalClass.dynPropSetter = CXFA_ScriptContext::GlobalPropertySetter;
  m_JsGlobalClass.dynPropTypeGetter = CXFA_ScriptContext::GlobalPropTypeGetter;
  m_JsGlobalClass.dynPropDeleter = NULL;
  m_JsGlobalClass.dynMethodCall = CXFA_ScriptContext::NormalMethodCall;
  m_hJsContext = FXJSE_Context_Create(m_hJsRuntime, &m_JsGlobalClass,
                                      m_pDocument->GetRoot());
  RemoveBuiltInObjs(m_hJsContext);
  FXJSE_Context_EnableCompatibleMode(
      m_hJsContext, FXJSE_COMPATIBLEMODEFLAG_CONSTRUCTOREXTRAMETHODS);
}
FXJSE_HCONTEXT CXFA_ScriptContext::CreateVariablesContext(
    CXFA_Node* pScriptNode,
    CXFA_Node* pSubform) {
  if (!pScriptNode || !pSubform)
    return nullptr;

  if (m_mapVariableToHValue.GetCount() == 0) {
    m_JsGlobalVariablesClass.constructor = nullptr;
    m_JsGlobalVariablesClass.name = "XFAScriptObject";
    m_JsGlobalVariablesClass.propNum = 0;
    m_JsGlobalVariablesClass.properties = nullptr;
    m_JsGlobalVariablesClass.methNum = 0;
    m_JsGlobalVariablesClass.methods = nullptr;
    m_JsGlobalVariablesClass.dynPropGetter =
        CXFA_ScriptContext::GlobalPropertyGetter;
    m_JsGlobalVariablesClass.dynPropSetter =
        CXFA_ScriptContext::GlobalPropertySetter;
    m_JsGlobalVariablesClass.dynPropTypeGetter =
        CXFA_ScriptContext::NormalPropTypeGetter;
    m_JsGlobalVariablesClass.dynPropDeleter = nullptr;
    m_JsGlobalVariablesClass.dynMethodCall =
        CXFA_ScriptContext::NormalMethodCall;
  }
  FXJSE_HCONTEXT hVariablesContext =
      FXJSE_Context_Create(m_hJsRuntime, &m_JsGlobalVariablesClass,
                           new CXFA_ThisProxy(pSubform, pScriptNode));
  RemoveBuiltInObjs(hVariablesContext);
  FXJSE_Context_EnableCompatibleMode(
      hVariablesContext, FXJSE_COMPATIBLEMODEFLAG_CONSTRUCTOREXTRAMETHODS);
  m_mapVariableToHValue.SetAt(pScriptNode, hVariablesContext);
  return hVariablesContext;
}
CXFA_Object* CXFA_ScriptContext::GetVariablesThis(CXFA_Object* pObject,
                                                  FX_BOOL bScriptNode) {
  if (pObject->GetObjectType() == XFA_OBJECTTYPE_VariablesThis) {
    return bScriptNode ? ((CXFA_ThisProxy*)pObject)->GetScriptNode()
                       : ((CXFA_ThisProxy*)pObject)->GetThisNode();
  }
  return pObject;
}

FX_BOOL CXFA_ScriptContext::RunVariablesScript(CXFA_Node* pScriptNode) {
  if (!pScriptNode)
    return FALSE;

  if (pScriptNode->GetClassID() != XFA_ELEMENT_Script)
    return TRUE;

  CXFA_Node* pParent = pScriptNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pParent || pParent->GetClassID() != XFA_ELEMENT_Variables)
    return FALSE;

  if (m_mapVariableToHValue.GetValueAt(pScriptNode))
    return TRUE;

  CXFA_Node* pTextNode = pScriptNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pTextNode)
    return FALSE;

  CFX_WideStringC wsScript;
  if (!pTextNode->TryCData(XFA_ATTRIBUTE_Value, wsScript))
    return FALSE;

  CFX_ByteString btScript =
      FX_UTF8Encode(wsScript.c_str(), wsScript.GetLength());
  FXJSE_HVALUE hRetValue = FXJSE_Value_Create(m_hJsRuntime);
  CXFA_Node* pThisObject = pParent->GetNodeItem(XFA_NODEITEM_Parent);
  FXJSE_HCONTEXT hVariablesContext =
      CreateVariablesContext(pScriptNode, pThisObject);
  CXFA_Object* pOriginalObject = m_pThisObject;
  m_pThisObject = pThisObject;
  FX_BOOL bRet =
      FXJSE_ExecuteScript(hVariablesContext, btScript.c_str(), hRetValue);
  m_pThisObject = pOriginalObject;
  FXJSE_Value_Release(hRetValue);
  return bRet;
}

FX_BOOL CXFA_ScriptContext::QueryVariableHValue(
    CXFA_Node* pScriptNode,
    const CFX_ByteStringC& szPropName,
    FXJSE_HVALUE hValue,
    FX_BOOL bGetter) {
  if (!pScriptNode || pScriptNode->GetClassID() != XFA_ELEMENT_Script)
    return FALSE;

  CXFA_Node* variablesNode = pScriptNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!variablesNode || variablesNode->GetClassID() != XFA_ELEMENT_Variables)
    return FALSE;

  void* lpVariables = m_mapVariableToHValue.GetValueAt(pScriptNode);
  if (!lpVariables)
    return FALSE;

  FX_BOOL bRes = FALSE;
  FXJSE_HCONTEXT hVariableContext = (FXJSE_HCONTEXT)lpVariables;
  FXJSE_HVALUE hObject = FXJSE_Context_GetGlobalObject(hVariableContext);
  FXJSE_HVALUE hVariableValue = FXJSE_Value_Create(m_hJsRuntime);
  if (!bGetter) {
    FXJSE_Value_SetObjectOwnProp(hObject, szPropName, hValue);
    bRes = TRUE;
  } else if (FXJSE_Value_ObjectHasOwnProp(hObject, szPropName, FALSE)) {
    FXJSE_Value_GetObjectProp(hObject, szPropName, hVariableValue);
    if (FXJSE_Value_IsFunction(hVariableValue))
      FXJSE_Value_SetFunctionBind(hValue, hVariableValue, hObject);
    else if (bGetter)
      FXJSE_Value_Set(hValue, hVariableValue);
    else
      FXJSE_Value_Set(hVariableValue, hValue);
    bRes = TRUE;
  }
  FXJSE_Value_Release(hVariableValue);
  FXJSE_Value_Release(hObject);
  return bRes;
}

void CXFA_ScriptContext::ReleaseVariablesMap() {
  FX_POSITION ps = m_mapVariableToHValue.GetStartPosition();
  while (ps) {
    CXFA_Object* pScriptNode;
    FXJSE_HCONTEXT hVariableContext = nullptr;
    m_mapVariableToHValue.GetNextAssoc(ps, pScriptNode, hVariableContext);
    FXJSE_HVALUE hObject = FXJSE_Context_GetGlobalObject(hVariableContext);
    delete static_cast<CXFA_ThisProxy*>(FXJSE_Value_ToObject(hObject, nullptr));
    FXJSE_Value_Release(hObject);
    FXJSE_Context_Release(hVariableContext);
  }
  m_mapVariableToHValue.RemoveAll();
}

void CXFA_ScriptContext::DefineJsClass() {
  m_JsNormalClass.constructor = NULL;
  m_JsNormalClass.name = "XFAObject";
  m_JsNormalClass.propNum = 0;
  m_JsNormalClass.properties = NULL;
  m_JsNormalClass.methNum = 0;
  m_JsNormalClass.methods = NULL;
  m_JsNormalClass.dynPropGetter = CXFA_ScriptContext::NormalPropertyGetter;
  m_JsNormalClass.dynPropSetter = CXFA_ScriptContext::NormalPropertySetter;
  m_JsNormalClass.dynPropTypeGetter = CXFA_ScriptContext::NormalPropTypeGetter;
  m_JsNormalClass.dynPropDeleter = NULL;
  m_JsNormalClass.dynMethodCall = CXFA_ScriptContext::NormalMethodCall;
  m_hJsClass = FXJSE_DefineClass(m_hJsContext, &m_JsNormalClass);
}
void CXFA_ScriptContext::RemoveBuiltInObjs(FXJSE_HCONTEXT jsContext) const {
  static const CFX_ByteStringC OBJ_NAME[2] = {"Number", "Date"};
  FXJSE_HVALUE hObject = FXJSE_Context_GetGlobalObject(jsContext);
  FXJSE_HVALUE hProp = FXJSE_Value_Create(m_hJsRuntime);
  for (int i = 0; i < 2; ++i) {
    if (FXJSE_Value_GetObjectProp(hObject, OBJ_NAME[i], hProp))
      FXJSE_Value_DeleteObjectProp(hObject, OBJ_NAME[i]);
  }
  FXJSE_Value_Release(hProp);
  FXJSE_Value_Release(hObject);
}
FXJSE_HCLASS CXFA_ScriptContext::GetJseNormalClass() {
  return m_hJsClass;
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
        FXJSE_HVALUE hValue = FXJSE_Value_Create(m_hJsRuntime);
        (rndFind.m_Nodes[0]->*(rndFind.m_pScriptAttribute->lpfnCallback))(
            hValue, FALSE,
            (XFA_ATTRIBUTE)rndFind.m_pScriptAttribute->eAttribute);
        rndFind.m_Nodes.SetAt(0,
                              (CXFA_Object*)FXJSE_Value_ToObject(hValue, NULL));
        FXJSE_Value_Release(hValue);
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
FXJSE_HVALUE CXFA_ScriptContext::GetJSValueFromMap(CXFA_Object* pObject) {
  if (!pObject) {
    return NULL;
  }
  if (pObject->IsNode()) {
    RunVariablesScript(pObject->AsNode());
  }
  void* pValue = m_mapXFAToHValue.GetValueAt(pObject);
  if (pValue == NULL) {
    FXJSE_HVALUE jsHvalue = FXJSE_Value_Create(m_hJsRuntime);
    FXJSE_Value_SetObject(jsHvalue, pObject, m_hJsClass);
    m_mapXFAToHValue.SetAt(pObject, jsHvalue);
    pValue = jsHvalue;
  }
  return (FXJSE_HVALUE)pValue;
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
