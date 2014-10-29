// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_DOCUMENT_LAYOUT_IMP_H_
#define _XFA_DOCUMENT_LAYOUT_IMP_H_
class CXFA_ItemLayoutProcessor;
class CXFA_LayoutPageMgr;
class CXFA_LayoutAppAdapter;
class CXFA_ContainerLayoutItemImpl;
class CXFA_LayoutProcessor : public IXFA_DocLayout, public CFX_Object
{
public:
    CXFA_LayoutProcessor(CXFA_Document* pDocument);
    ~CXFA_LayoutProcessor();
    virtual CXFA_Document*		GetDocument() const;
    virtual	FX_INT32			StartLayout(FX_BOOL bForceRestart = FALSE);
    virtual FX_INT32			DoLayout(IFX_Pause *pPause = NULL);
    virtual FX_BOOL				IncrementLayout();
    virtual FX_INT32			CountPages() const;
    virtual IXFA_LayoutPage*	GetPage(FX_INT32 index) const;
    virtual CXFA_LayoutItem*	GetLayoutItem(CXFA_Node *pFormItem);

    void				AddChangedContainer(CXFA_Node* pContainer);
    void				SetForceReLayout(FX_BOOL bForceRestart)
    {
        m_bNeeLayout = bForceRestart;
    }
    CXFA_ContainerLayoutItemImpl*	GetRootLayoutItem() const;
    CXFA_ItemLayoutProcessor*	GetRootRootItemLayoutProcessor()
    {
        return m_pRootItemLayoutProcessor;
    }
    CXFA_LayoutPageMgr*			GetLayoutPageMgr()
    {
        return m_pLayoutPageMgr;
    }
protected:
    void		ClearLayoutData();

    FX_BOOL		IsNeedLayout();

    CXFA_Document*				m_pDocument;
    CXFA_ItemLayoutProcessor*   m_pRootItemLayoutProcessor;
    CXFA_LayoutPageMgr*			m_pLayoutPageMgr;
    CXFA_NodeArray				m_rgChangedContainers;
    FX_UINT32					m_nProgressCounter;
    FX_BOOL						m_bNeeLayout;
};
#endif
