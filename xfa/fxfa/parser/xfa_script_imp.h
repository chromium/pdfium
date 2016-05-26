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

  void Initialize(v8::Isolate* pIsolate);
  void SetEventParam(CXFA_EventParam param) { m_eventParam = param; }
  CXFA_EventParam* GetEventParam() { return &m_eventParam; }
  FX_BOOL RunScript(XFA_SCRIPTLANGTYPE eScriptType,
                    const CFX_WideStringC& wsScript,
                    CFXJSE_Value* pRetValue,
                    CXFA_Object* pThisObject = NULL);

  int32_t ResolveObjects(CXFA_Object* refNode,
                         const CFX_WideStringC& wsExpression,
                         XFA_RESOLVENODE_RS& resolveNodeRS,
                         uint32_t dwStyles = XFA_RESOLVENODE_Children,
                         CXFA_Node* bindNode = NULL);
  CFXJSE_Value* GetJSValueFromMap(CXFA_Object* pObject);
  void CacheList(CXFA_NodeList* pList) { m_CacheListArray.Add(pList); }
  CXFA_Object* GetThisObject() const { return m_pThisObject; }
  v8::Isolate* GetRuntime() const { return m_pIsolate; }

  int32_t GetIndexByName(CXFA_Node* refNode);
  int32_t GetIndexByClassName(CXFA_Node* refNode);
  void GetSomExpression(CXFA_Node* refNode, CFX_WideString& wsExpression);

  void SetNodesOfRunScript(CXFA_NodeArray* pArray);
  void AddNodesOfRunScript(const CXFA_NodeArray& nodes);
  void AddNodesOfRunScript(CXFA_Node* pNode);
  CFXJSE_Class* GetJseNormalClass();

  void SetRunAtType(XFA_ATTRIBUTEENUM eRunAt) { m_eRunAtType = eRunAt; }
  FX_BOOL IsRunAtClient() { return m_eRunAtType != XFA_ATTRIBUTEENUM_Server; }
  FX_BOOL QueryNodeByFlag(CXFA_Node* refNode,
                          const CFX_WideStringC& propname,
                          CFXJSE_Value* pValue,
                          uint32_t dwFlag,
                          FX_BOOL bSetting);
  FX_BOOL QueryVariableValue(CXFA_Node* pScriptNode,
                             const CFX_ByteStringC& szPropName,
                             CFXJSE_Value* pValue,
                             FX_BOOL bGetter);
  FX_BOOL QueryBuiltinValue(const CFX_ByteStringC& szPropName,
                            CFXJSE_Value* pValue);
  static void GlobalPropertyGetter(CFXJSE_Value* pObject,
                                   const CFX_ByteStringC& szPropName,
                                   CFXJSE_Value* pValue);
  static void GlobalPropertySetter(CFXJSE_Value* pObject,
                                   const CFX_ByteStringC& szPropName,
                                   CFXJSE_Value* pValue);
  static void NormalPropertyGetter(CFXJSE_Value* pObject,
                                   const CFX_ByteStringC& szPropName,
                                   CFXJSE_Value* pValue);
  static void NormalPropertySetter(CFXJSE_Value* pObject,
                                   const CFX_ByteStringC& szPropName,
                                   CFXJSE_Value* pValue);
  static void NormalMethodCall(CFXJSE_Value* hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args);
  static int32_t NormalPropTypeGetter(CFXJSE_Value* pObject,
                                      const CFX_ByteStringC& szPropName,
                                      FX_BOOL bQueryIn);
  static int32_t GlobalPropTypeGetter(CFXJSE_Value* pObject,
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

 private:
  void DefineJsContext();
  CFXJSE_Context* CreateVariablesContext(CXFA_Node* pScriptNode,
                                         CXFA_Node* pSubform);
  void DefineJsClass();
  void RemoveBuiltInObjs(CFXJSE_Context* pContext) const;

  CXFA_Document* m_pDocument;
  CFXJSE_Context* m_pJsContext;
  v8::Isolate* m_pIsolate;
  CFXJSE_Class* m_pJsClass;
  XFA_SCRIPTLANGTYPE m_eScriptType;
  FXJSE_CLASS_DESCRIPTOR m_JsGlobalClass;
  FXJSE_CLASS_DESCRIPTOR m_JsNormalClass;
  CFX_MapPtrTemplate<CXFA_Object*, CFXJSE_Value*> m_mapXFAToValue;
  FXJSE_CLASS_DESCRIPTOR m_JsGlobalVariablesClass;
  CFX_MapPtrTemplate<CXFA_Object*, CFXJSE_Context*> m_mapVariableToContext;
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
