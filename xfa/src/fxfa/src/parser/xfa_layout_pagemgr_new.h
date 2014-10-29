// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_LAYOUT_PAGEMGR_H_
#define _XFA_LAYOUT_PAGEMGR_H_
class CXFA_ContainerRecord : public CFX_Object
{
public:
    CXFA_ContainerRecord(CXFA_ContainerLayoutItemImpl* pPageSet = NULL, CXFA_ContainerLayoutItemImpl* pPageArea = NULL, CXFA_ContainerLayoutItemImpl* pContentArea = NULL)
        : pCurPageSet(pPageSet), pCurPageArea(pPageArea), pCurContentArea(pContentArea)
    {
    }
    CXFA_ContainerLayoutItemImpl* pCurPageSet;
    CXFA_ContainerLayoutItemImpl* pCurPageArea;
    CXFA_ContainerLayoutItemImpl* pCurContentArea;
};
class CXFA_LayoutPageMgr : public CFX_Object
{
public:
    CXFA_LayoutPageMgr(CXFA_LayoutProcessor* pLayoutProcessor);
    ~CXFA_LayoutPageMgr();
    FX_BOOL		InitLayoutPage(CXFA_Node* pFormNode);
    FX_BOOL		PrepareFirstPage(CXFA_Node* pRootSubform);
    FX_FLOAT	GetAvailHeight();
    FX_BOOL		GetNextAvailContentHeight(FX_FLOAT fChildHeight);
    void		SubmitContentItem(CXFA_ContentLayoutItemImpl* pContentLayoutItem, XFA_ItemLayoutProcessorResult eStatus);
    void		FinishPaginatedPageSets();
    void		SyncLayoutData();
    FX_INT32			GetPageCount() const;
    IXFA_LayoutPage*	GetPage(FX_INT32 index) const;
    FX_INT32			GetPageIndex(IXFA_LayoutPage* pPage) const;
    inline CXFA_ContainerLayoutItemImpl*	GetRootLayoutItem() const
    {
        return m_pPageSetLayoutItemRoot;
    }
    FX_BOOL		ProcessBreakBeforeOrAfter(CXFA_Node* pBreakNode, FX_BOOL bBefore, CXFA_Node* &pBreakLeaderNode, CXFA_Node* &pBreakTrailerNode, FX_BOOL& bCreatePage);
    FX_BOOL		ProcessOverflow(CXFA_Node* pFormNode, CXFA_Node* &pLeaderNode, CXFA_Node* &pTrailerNode, FX_BOOL bDataMerge = FALSE, FX_BOOL bCreatePage = TRUE);
    CXFA_Node*	QueryOverflow(CXFA_Node* pFormNode, CXFA_LayoutContext* pLayoutContext = NULL);
    FX_BOOL		ProcessBookendLeaderOrTrailer(CXFA_Node* pBookendNode, FX_BOOL bLeader, CXFA_Node* &pBookendAppendNode);
    CXFA_LayoutItemImpl* FindOrCreateLayoutItem(CXFA_Node* pFormNode);
protected:
    FX_BOOL 	AppendNewPage(FX_BOOL bFirstTemPage = FALSE);
    void		ReorderPendingLayoutRecordToTail(CXFA_ContainerRecord* pNewRecord, CXFA_ContainerRecord* pPrevRecord);
    void		RemoveLayoutRecord(CXFA_ContainerRecord* pNewRecord, CXFA_ContainerRecord* pPrevRecord);
    inline CXFA_ContainerRecord* GetCurrentContainerRecord()
    {
        CXFA_ContainerRecord* result = ((CXFA_ContainerRecord*)m_rgProposedContainerRecord.GetAt(m_pCurrentContainerRecord));
        ASSERT(result);
        return result;
    }
    CXFA_ContainerRecord* CreateContainerRecord(CXFA_Node* pPageNode = NULL, FX_BOOL bCreateNew = FALSE);
    void		AddPageAreaLayoutItem(CXFA_ContainerRecord* pNewRecord, CXFA_Node* pNewPageArea);
    void		AddContentAreaLayoutItem(CXFA_ContainerRecord* pNewRecord, CXFA_Node* pContentArea);
    FX_BOOL		RunBreak(XFA_ELEMENT eBreakType, XFA_ATTRIBUTEENUM eTargetType, CXFA_Node* pTarget, FX_BOOL bStartNew);
    CXFA_Node*	BreakOverflow(CXFA_Node* pOverflowNode, CXFA_Node*& pLeaderTemplate, CXFA_Node*& pTrailerTemplate, FX_BOOL bCreatePage = TRUE);
    FX_BOOL		ResolveBookendLeaderOrTrailer(CXFA_Node* pBookendNode, FX_BOOL bLeader, CXFA_Node* &pBookendAppendTemplate);
    FX_BOOL		ExecuteBreakBeforeOrAfter(CXFA_Node* pCurNode, FX_BOOL bBefore, CXFA_Node* &pBreakLeaderTemplate, CXFA_Node* &pBreakTrailerTemplate);

    FX_INT32	CreateMinPageRecord(CXFA_Node* pPageArea, FX_BOOL bTargetPageArea, FX_BOOL bCreateLast = FALSE);
    void		CreateMinPageSetRecord(CXFA_Node* pPageSet, FX_BOOL bCreateAll = FALSE);
    void		CreateNextMinRecord(CXFA_Node* pRecordNode);
    FX_BOOL		FindPageAreaFromPageSet(CXFA_Node* pPageSet, CXFA_Node* pStartChild, CXFA_Node* pTargetPageArea = NULL, CXFA_Node* pTargetContentArea = NULL, FX_BOOL bNewPage = FALSE, FX_BOOL bQuery = FALSE);
    FX_BOOL		FindPageAreaFromPageSet_Ordered(CXFA_Node* pPageSet, CXFA_Node* pStartChild, CXFA_Node* pTargetPageArea = NULL, CXFA_Node* pTargetContentArea = NULL, FX_BOOL bNewPage = FALSE, FX_BOOL bQuery = FALSE);
    FX_BOOL		FindPageAreaFromPageSet_SimplexDuplex(CXFA_Node* pPageSet, CXFA_Node* pStartChild, CXFA_Node* pTargetPageArea = NULL, CXFA_Node* pTargetContentArea = NULL, FX_BOOL bNewPage = FALSE, FX_BOOL bQuery = FALSE, XFA_ATTRIBUTEENUM ePreferredPosition = XFA_ATTRIBUTEENUM_First);
    FX_BOOL		MatchPageAreaOddOrEven(CXFA_Node* pPageArea, FX_BOOL bLastMatch);
    CXFA_Node*	GetNextAvailPageArea(CXFA_Node* pTargetPageArea, CXFA_Node* pTargetContentArea = NULL, FX_BOOL bNewPage = FALSE, FX_BOOL bQuery = FALSE);
    FX_BOOL		GetNextContentArea(CXFA_Node* pTargetContentArea);
    void		InitPageSetMap();
    void		ProcessLastPageSet();
    inline	FX_BOOL	IsPageSetRootOrderedOccurrence()
    {
        return m_ePageSetMode == XFA_ATTRIBUTEENUM_OrderedOccurrence;
    }
    void		ClearData();
    void		ClearRecordList();
    void		MergePageSetContents();
    void		LayoutPageSetContents();
    void		PrepareLayout();
    CXFA_LayoutProcessor*				m_pLayoutProcessor;
    CXFA_Node*							m_pTemplatePageSetRoot;
    CXFA_ContainerLayoutItemImpl*		m_pPageSetLayoutItemRoot;
    CXFA_ContainerLayoutItemImpl*		m_pPageSetCurRoot;
    FX_POSITION							m_pCurrentContainerRecord;
    CFX_PtrList							m_rgProposedContainerRecord;
    CXFA_Node*							m_pCurPageArea;
    FX_INT32							m_nAvailPages;
    FX_INT32							m_nCurPageCount;
    XFA_ATTRIBUTEENUM					m_ePageSetMode;
    FX_BOOL								m_bCreateOverFlowPage;
    CFX_MapPtrTemplate<CXFA_Node*, FX_INT32> m_pPageSetMap;
#ifdef _XFA_LAYOUTITEM_MAPCACHE_
    void						SaveLayoutItem(CXFA_LayoutItemImpl* pParentLayoutItem);
    CFX_MapPtrToPtr				m_NodeToContent;
#elif defined(_XFA_LAYOUTITEM_ProcessCACHE_)
    void						SaveLayoutItem(CXFA_LayoutItemImpl* pParentLayoutItem);
#endif
    CFX_ArrayTemplate<CXFA_ContainerLayoutItemImpl*>				m_PageArray;
};
#endif
