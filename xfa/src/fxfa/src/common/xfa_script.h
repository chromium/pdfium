// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_SCRIPT_H
#define _FXFA_SCRIPT_H
#define XFA_RESOLVENODE_Children 0x0001
#define XFA_RESOLVENODE_Attributes 0x0004
#define XFA_RESOLVENODE_Properties 0x0008
#define XFA_RESOLVENODE_Siblings 0x0020
#define XFA_RESOLVENODE_Parent 0x0040
#define XFA_RESOLVENODE_AnyChild 0x0080
#define XFA_RESOLVENODE_ALL 0x0100
#define XFA_RESOLVENODE_CreateNode 0x0400
#define XFA_RESOLVENODE_Bind 0x0800
#define XFA_RESOLVENODE_BindNew 0x1000
enum XFA_SCRIPTLANGTYPE {
  XFA_SCRIPTLANGTYPE_Formcalc = XFA_SCRIPTTYPE_Formcalc,
  XFA_SCRIPTLANGTYPE_Javascript = XFA_SCRIPTTYPE_Javascript,
  XFA_SCRIPTLANGTYPE_Unkown = XFA_SCRIPTTYPE_Unkown,
};
enum XFA_RESOVENODE_RSTYPE {
  XFA_RESOVENODE_RSTYPE_Nodes,
  XFA_RESOVENODE_RSTYPE_Attribute,
  XFA_RESOLVENODE_RSTYPE_CreateNodeOne,
  XFA_RESOLVENODE_RSTYPE_CreateNodeAll,
  XFA_RESOLVENODE_RSTYPE_CreateNodeMidAll,
  XFA_RESOVENODE_RSTYPE_ExistNodes,
};
class CXFA_HVALUEArray : public CFX_ArrayTemplate<FXJSE_HVALUE> {
 public:
  CXFA_HVALUEArray(FXJSE_HRUNTIME hRunTime) : m_hRunTime(hRunTime){};
  ~CXFA_HVALUEArray() {
    for (int32_t i = 0; i < GetSize(); i++) {
      FXJSE_Value_Release(GetAt(i));
    }
  }
  void GetAttributeObject(CXFA_ObjArray& objArray) {
    for (int32_t i = 0; i < GetSize(); i++) {
      CXFA_Object* pObject = (CXFA_Object*)FXJSE_Value_ToObject(GetAt(i), NULL);
      objArray.Add(pObject);
    }
  }
  FXJSE_HRUNTIME m_hRunTime;
};
typedef struct _XFA_RESOLVENODE_RS {
  _XFA_RESOLVENODE_RS()
      : dwFlags(XFA_RESOVENODE_RSTYPE_Nodes), pScriptAttribute(NULL) {}
  ~_XFA_RESOLVENODE_RS() { nodes.RemoveAll(); }
  int32_t GetAttributeResult(CXFA_HVALUEArray& hValueArray) const {
    if (pScriptAttribute && pScriptAttribute->eValueType == XFA_SCRIPT_Object) {
      FXJSE_HRUNTIME hRunTime = hValueArray.m_hRunTime;
      for (int32_t i = 0; i < nodes.GetSize(); i++) {
        FXJSE_HVALUE hValue = FXJSE_Value_Create(hRunTime);
        (nodes[i]->*(pScriptAttribute->lpfnCallback))(
            hValue, FALSE, (XFA_ATTRIBUTE)pScriptAttribute->eAttribute);
        hValueArray.Add(hValue);
      }
    }
    return hValueArray.GetSize();
  }
  CXFA_ObjArray nodes;
  XFA_RESOVENODE_RSTYPE dwFlags;
  XFA_LPCSCRIPTATTRIBUTEINFO pScriptAttribute;
} XFA_RESOLVENODE_RS, *XFA_LPRESOLVENODE_RS;
typedef struct _XFA_JSBUILTININFO {
  uint32_t uUnicodeHash;
  const FX_CHAR* pName;
} XFA_JSBUILTININFO, *XFA_LPJSBUILTININFO;
typedef XFA_JSBUILTININFO const* XFA_LPCJSBUILTININFO;
XFA_LPCJSBUILTININFO XFA_GetJSBuiltinByHash(uint32_t uHashCode);
class IXFA_ScriptContext {
 public:
  virtual ~IXFA_ScriptContext() {}
  virtual void Release() = 0;
  virtual void Initialize(FXJSE_HRUNTIME hRuntime) = 0;

  virtual void SetEventParam(CXFA_EventParam* pEventParam) = 0;
  virtual CXFA_EventParam* GetEventParam() = 0;
  virtual FX_BOOL RunScript(XFA_SCRIPTLANGTYPE eScriptType,
                            const CFX_WideStringC& wsScript,
                            FXJSE_HVALUE hRetValue,
                            CXFA_Object* pThisObject = NULL) = 0;
  virtual int32_t ResolveObjects(CXFA_Object* refNode,
                                 const CFX_WideStringC& wsExpression,
                                 XFA_RESOLVENODE_RS& resolveNodeRS,
                                 FX_DWORD dwStyles = XFA_RESOLVENODE_Children,
                                 CXFA_Node* bindNode = NULL) = 0;
  virtual FXJSE_HVALUE GetJSValueFromMap(CXFA_Object* pObject) = 0;
  virtual void CacheList(CXFA_NodeList* pList) = 0;
  virtual CXFA_Object* GetThisObject() const = 0;
  virtual FXJSE_HRUNTIME GetRuntime() const = 0;
  virtual int32_t GetIndexByName(CXFA_Node* refNode) = 0;
  virtual int32_t GetIndexByClassName(CXFA_Node* refNode) = 0;
  virtual void GetSomExpression(CXFA_Node* refNode,
                                CFX_WideString& wsExpression) = 0;

  virtual void SetNodesOfRunScript(CXFA_NodeArray* pArray) = 0;
  virtual void AddNodesOfRunScript(const CXFA_NodeArray& nodes) = 0;
  virtual void AddNodesOfRunScript(CXFA_Node* pNode) = 0;
  virtual FXJSE_HCLASS GetJseNormalClass() = 0;
  virtual XFA_SCRIPTLANGTYPE GetType() = 0;
  virtual void AddJSBuiltinObject(XFA_LPCJSBUILTININFO pBuitinObject) = 0;
  virtual void SetRunAtType(XFA_ATTRIBUTEENUM eRunAt) = 0;
  virtual FX_BOOL IsRunAtClient() = 0;
};
IXFA_ScriptContext* XFA_ScriptContext_Create(CXFA_Document* pDocument);
#endif
