// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_SCRIPT_IMP_H
#define _FXFA_SCRIPT_IMP_H
#define XFA_RESOLVENODE_TagName				0x0002
#define XFA_JSBUILTIN_Initialized			0x0001
#define XFA_JSBUILTIN_HasCount				0x0002
class CXFA_ResolveProcessor;
class CXFA_ScriptContext : public IXFA_ScriptContext, public CFX_Object
{
public:
    CXFA_ScriptContext(CXFA_Document* pDocument);
    ~CXFA_ScriptContext();
    virtual void			Release();
    virtual void			Initialize(FXJSE_HRUNTIME hRuntime);
    virtual void				SetEventParam(CXFA_EventParam* pEventParam)
    {
        m_pEventParam = pEventParam;
    }
    virtual CXFA_EventParam*	GetEventParam()
    {
        return m_pEventParam;
    }
    virtual FX_BOOL			RunScript(XFA_SCRIPTLANGTYPE eScriptType, FX_WSTR wsScript, FXJSE_HVALUE hRetValue, CXFA_Object* pThisObject = NULL);

    virtual FX_INT32		ResolveObjects(CXFA_Object* refNode, FX_WSTR wsExpression, XFA_RESOLVENODE_RS& resolveNodeRS,  FX_DWORD dwStyles = XFA_RESOLVENODE_Children, CXFA_Node* bindNode = NULL);
    virtual FXJSE_HVALUE	GetJSValueFromMap(CXFA_Object* pObject);
    virtual void			CacheList(CXFA_NodeList* pList)
    {
        m_CacheListArray.Add(pList);
    }
    virtual CXFA_Object*	GetThisObject() const
    {
        return m_pThisObject;
    }
    virtual FXJSE_HRUNTIME	GetRuntime() const
    {
        return m_hJsRuntime;
    }

    virtual FX_INT32		GetIndexByName(CXFA_Node* refNode);
    virtual FX_INT32		GetIndexByClassName(CXFA_Node* refNode);
    virtual void			GetSomExpression(CXFA_Node* refNode, CFX_WideString &wsExpression);

    virtual void			SetNodesOfRunScript(CXFA_NodeArray *pArray);
    virtual void			AddNodesOfRunScript(const CXFA_NodeArray& nodes);
    virtual void			AddNodesOfRunScript(CXFA_Node* pNode);
    virtual FXJSE_HCLASS	GetJseNormalClass();

    virtual	void			AddJSBuiltinObject(XFA_LPCJSBUILTININFO pBuitinObject);
    virtual	void			SetRunAtType(XFA_ATTRIBUTEENUM eRunAt)
    {
        m_eRunAtType = eRunAt;
    }
    virtual FX_BOOL			IsRunAtClient()
    {
        return m_eRunAtType != XFA_ATTRIBUTEENUM_Server;
    }
    FX_BOOL					QueryNodeByFlag(CXFA_Node* refNode, FX_WSTR propname, FXJSE_HVALUE hValue, FX_DWORD dwFlag, FX_BOOL bSetting);
    FX_BOOL					QueryVariableHValue(CXFA_Node* pScriptNode, FX_BSTR szPropName, FXJSE_HVALUE hValue, FX_BOOL bGetter);
    FX_BOOL					QueryBuiltinHValue(FX_BSTR szPropName, FXJSE_HVALUE hValue);
    static void				GlobalPropertyGetter(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FXJSE_HVALUE hValue);
    static void				GlobalPropertySetter(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FXJSE_HVALUE hValue);
    static void				NormalPropertyGetter(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FXJSE_HVALUE hValue);
    static void				NormalPropertySetter(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FXJSE_HVALUE hValue);
    static void				NormalMethodCall(FXJSE_HOBJECT hThis, FX_BSTR szFuncName, CFXJSE_Arguments &args);
    static FX_INT32			NormalPropTypeGetter(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FX_BOOL bQueryIn);
    static FX_INT32			GlobalPropTypeGetter(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FX_BOOL bQueryIn);
    FX_BOOL					RunVariablesScript(CXFA_Node* pScriptNode);
    CXFA_Object*			GetVariablesThis(CXFA_Object* pObject, FX_BOOL bScriptNode = FALSE);
    void					ReleaseVariablesMap();
    FX_BOOL					IsStrictScopeInJavaScript();
    XFA_SCRIPTLANGTYPE		GetType();
    CXFA_NodeArray&			GetUpObjectArray()
    {
        return m_upObjectArray;
    }
    CXFA_Document*			GetDocument() const
    {
        return m_pDocument;
    }
protected:
    void					DefineJsContext();
    FXJSE_HCONTEXT			CreateVariablesContext(CXFA_Node* pScriptNode, CXFA_Node* pSubform);

    void					DefineJsClass();
    CXFA_Document*									m_pDocument;
    FXJSE_HCONTEXT									m_hJsContext;
    FXJSE_HRUNTIME									m_hJsRuntime;
    FXJSE_HCLASS									m_hJsClass;
    XFA_SCRIPTLANGTYPE								m_eScriptType;
    FXJSE_CLASS										m_JsGlobalClass;
    FXJSE_CLASS										m_JsNormalClass;
    CFX_MapPtrTemplate<CXFA_Object*, FXJSE_HVALUE>  m_mapXFAToHValue;

    FXJSE_CLASS										m_JsGlobalVariablesClass;
    CFX_MapPtrTemplate<CXFA_Object*, FXJSE_HCONTEXT> m_mapVariableToHValue;
    CXFA_EventParam*								m_pEventParam;
    CXFA_NodeArray									m_upObjectArray;
    CFX_PtrArray									m_CacheListArray;
    CXFA_NodeArray*									m_pScriptNodeArray;
    CXFA_ResolveProcessor*							m_pResolveProcessor;
    XFA_HFM2JSCONTEXT								m_hFM2JSContext;
    CXFA_Object*									m_pThisObject;
    CFX_CMapByteStringToPtr							m_JSBuiltInObjects;
    FX_DWORD										m_dwBuiltInInFlags;
    XFA_ATTRIBUTEENUM								m_eRunAtType;
};
#endif
