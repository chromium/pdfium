// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_SCRIPT_IMP_H_
#define XFA_FXFA_PARSER_XFA_SCRIPT_IMP_H_

#include <map>

#include "xfa/fxfa/fm2js/xfa_fm2jsapi.h"
#include "xfa/fxfa/parser/xfa_document.h"
#include "xfa/fxfa/parser/xfa_script.h"
#include "xfa/fxjse/cfxjse_arguments.h"

#define XFA_RESOLVENODE_TagName 0x0002

class CXFA_ResolveProcessor;

class CXFA_ScriptContext {
 public:
  explicit CXFA_ScriptContext(CXFA_Document* pDocument);
  ~CXFA_ScriptContext();

  void Release();
  void Initialize(FXJSE_HRUNTIME hRuntime);
  void SetEventParam(CXFA_EventParam param) { m_eventParam = param; }
  CXFA_EventParam* GetEventParam() { return &m_eventParam; }
  FX_BOOL RunScript(XFA_SCRIPTLANGTYPE eScriptType,
                    const CFX_WideStringC& wsScript,
                    FXJSE_HVALUE hRetValue,
                    CXFA_Object* pThisObject = NULL);

  int32_t ResolveObjects(CXFA_Object* refNode,
                         const CFX_WideStringC& wsExpression,
                         XFA_RESOLVENODE_RS& resolveNodeRS,
                         uint32_t dwStyles = XFA_RESOLVENODE_Children,
                         CXFA_Node* bindNode = NULL);
  FXJSE_HVALUE GetJSValueFromMap(CXFA_Object* pObject);
  void CacheList(CXFA_NodeList* pList) { m_CacheListArray.Add(pList); }
  CXFA_Object* GetThisObject() const { return m_pThisObject; }
  FXJSE_HRUNTIME GetRuntime() const { return m_hJsRuntime; }

  int32_t GetIndexByName(CXFA_Node* refNode);
  int32_t GetIndexByClassName(CXFA_Node* refNode);
  void GetSomExpression(CXFA_Node* refNode, CFX_WideString& wsExpression);

  void SetNodesOfRunScript(CXFA_NodeArray* pArray);
  void AddNodesOfRunScript(const CXFA_NodeArray& nodes);
  void AddNodesOfRunScript(CXFA_Node* pNode);
  FXJSE_HCLASS GetJseNormalClass();

  void SetRunAtType(XFA_ATTRIBUTEENUM eRunAt) { m_eRunAtType = eRunAt; }
  FX_BOOL IsRunAtClient() { return m_eRunAtType != XFA_ATTRIBUTEENUM_Server; }
  FX_BOOL QueryNodeByFlag(CXFA_Node* refNode,
                          const CFX_WideStringC& propname,
                          FXJSE_HVALUE hValue,
                          uint32_t dwFlag,
                          FX_BOOL bSetting);
  FX_BOOL QueryVariableHValue(CXFA_Node* pScriptNode,
                              const CFX_ByteStringC& szPropName,
                              FXJSE_HVALUE hValue,
                              FX_BOOL bGetter);
  FX_BOOL QueryBuiltinHValue(const CFX_ByteStringC& szPropName,
                             FXJSE_HVALUE hValue);
  static void GlobalPropertyGetter(FXJSE_HOBJECT hObject,
                                   const CFX_ByteStringC& szPropName,
                                   FXJSE_HVALUE hValue);
  static void GlobalPropertySetter(FXJSE_HOBJECT hObject,
                                   const CFX_ByteStringC& szPropName,
                                   FXJSE_HVALUE hValue);
  static void NormalPropertyGetter(FXJSE_HOBJECT hObject,
                                   const CFX_ByteStringC& szPropName,
                                   FXJSE_HVALUE hValue);
  static void NormalPropertySetter(FXJSE_HOBJECT hObject,
                                   const CFX_ByteStringC& szPropName,
                                   FXJSE_HVALUE hValue);
  static void NormalMethodCall(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args);
  static int32_t NormalPropTypeGetter(FXJSE_HOBJECT hObject,
                                      const CFX_ByteStringC& szPropName,
                                      FX_BOOL bQueryIn);
  static int32_t GlobalPropTypeGetter(FXJSE_HOBJECT hObject,
                                      const CFX_ByteStringC& szPropName,
                                      FX_BOOL bQueryIn);
  FX_BOOL RunVariablesScript(CXFA_Node* pScriptNode);
  CXFA_Object* GetVariablesThis(CXFA_Object* pObject,
                                FX_BOOL bScriptNode = FALSE);
  void ReleaseVariablesMap();
  FX_BOOL IsStrictScopeInJavaScript();
  XFA_SCRIPTLANGTYPE GetType();
  CXFA_NodeArray& GetUpObjectArray() { return m_upObjectArray; }
  CXFA_Document* GetDocument() const { return m_pDocument; }

 protected:
  void DefineJsContext();
  FXJSE_HCONTEXT CreateVariablesContext(CXFA_Node* pScriptNode,
                                        CXFA_Node* pSubform);
  void DefineJsClass();
  void RemoveBuiltInObjs(FXJSE_HCONTEXT jsContext) const;

  CXFA_Document* m_pDocument;
  FXJSE_HCONTEXT m_hJsContext;
  FXJSE_HRUNTIME m_hJsRuntime;
  FXJSE_HCLASS m_hJsClass;
  XFA_SCRIPTLANGTYPE m_eScriptType;
  FXJSE_CLASS m_JsGlobalClass;
  FXJSE_CLASS m_JsNormalClass;
  CFX_MapPtrTemplate<CXFA_Object*, FXJSE_HVALUE> m_mapXFAToHValue;
  FXJSE_CLASS m_JsGlobalVariablesClass;
  CFX_MapPtrTemplate<CXFA_Object*, FXJSE_HCONTEXT> m_mapVariableToHValue;
  CXFA_EventParam m_eventParam;
  CXFA_NodeArray m_upObjectArray;
  CFX_ArrayTemplate<CXFA_NodeList*> m_CacheListArray;
  CXFA_NodeArray* m_pScriptNodeArray;
  CXFA_ResolveProcessor* m_pResolveProcessor;
  XFA_HFM2JSCONTEXT m_hFM2JSContext;
  CXFA_Object* m_pThisObject;
  uint32_t m_dwBuiltInInFlags;
  XFA_ATTRIBUTEENUM m_eRunAtType;
};

#endif  //  XFA_FXFA_PARSER_XFA_SCRIPT_IMP_H_
