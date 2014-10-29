// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_RESOLVEPROCESSOR_H_
#define _XFA_RESOLVEPROCESSOR_H_
class CXFA_NodeHelper;
class CXFA_ScriptContext;
class CXFA_ResolveNodesData : public CFX_Object
{
public:
    CXFA_ResolveNodesData(CXFA_ScriptContext *pSC = NULL)
        : m_pSC(pSC)
        , m_CurNode(NULL)
        , m_wsName()
        , m_uHashName(0)
        , m_wsCondition()
        , m_nLevel(0)
        , m_Nodes()
        , m_dwStyles(XFA_RESOLVENODE_Children)
        , m_pScriptAttribute(NULL)
        , m_dwFlag(XFA_RESOVENODE_RSTYPE_Nodes)
    {
    }
    ~CXFA_ResolveNodesData()
    {
        m_Nodes.RemoveAll();
    }
    CXFA_ScriptContext*			m_pSC;
    CXFA_Object*				m_CurNode;
    CFX_WideString				m_wsName;
    FX_UINT32					m_uHashName;
    CFX_WideString				m_wsCondition;
    FX_INT32					m_nLevel;
    CXFA_ObjArray				m_Nodes;
    FX_DWORD					m_dwStyles;
    XFA_LPCSCRIPTATTRIBUTEINFO	m_pScriptAttribute;
    XFA_RESOVENODE_RSTYPE		m_dwFlag;
};
class CXFA_ResolveProcessor : public CFX_Object
{
public:
    CXFA_ResolveProcessor(void);
    ~CXFA_ResolveProcessor(void);
    FX_INT32	XFA_ResolveNodes(CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_AnyChild(CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_Dollar(CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_Excalmatory(CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_NumberSign(CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_Asterisk(CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_Normal(CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_ForAttributeRs(CXFA_Object* curNode, CXFA_ResolveNodesData &rnd, FX_WSTR strAttr);
    void		XFA_ResolveNode_ConditionArray(FX_INT32 iCurIndex, CFX_WideString wsCondition, FX_INT32 iFoundCount, CXFA_ResolveNodesData &rnd);
    void		XFA_ResolveNode_DoPredicateFilter(FX_INT32 iCurIndex, CFX_WideString wsCondition, FX_INT32 iFoundCount, CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNodes_GetFilter(FX_WSTR wsExpression, FX_INT32 nStart, CXFA_ResolveNodesData &rnd);
    void		XFA_ResolveNode_FilterCondition(CXFA_ResolveNodesData &rnd, CFX_WideString wsCondition);
    FX_INT32	XFA_ResolveNodes_PopStack(CFX_Int32Array &stack);
    void		XFA_ResolveNodes_SetStylesForChild(FX_DWORD dwParentStyles, CXFA_ResolveNodesData &rnd);
    FX_INT32	XFA_ResolveNode_SetResultCreateNode(XFA_RESOLVENODE_RS& resolveNodeRS, CFX_WideString &wsLastCondition);
    void	    XFA_ResolveNode_SetIndexDataBind(CFX_WideString &wsNextCondition, FX_INT32 &iIndex, FX_INT32 iCount);
    CXFA_NodeHelper* GetNodeHelper()
    {
        return m_pNodeHelper;
    }
private:
    CXFA_NodeHelper* m_pNodeHelper;
public:
    FX_INT32		 m_iCurStart;
};
#endif
