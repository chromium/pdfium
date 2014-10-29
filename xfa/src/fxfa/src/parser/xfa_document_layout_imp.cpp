// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_utils.h"
#include "../common/xfa_object.h"
#include "../common/xfa_document.h"
#include "../common/xfa_parser.h"
#include "../common/xfa_script.h"
#include "../common/xfa_docdata.h"
#include "../common/xfa_doclayout.h"
#include "../common/xfa_debug.h"
#include "../common/xfa_localemgr.h"
#include "../common/xfa_fm2jsapi.h"
#include "xfa_debug_parser.h"
#include "xfa_basic_imp.h"
#include "xfa_document_imp.h"
#include "xfa_document_layout_imp.h"
#include "xfa_document_datamerger_imp.h"
#include "xfa_layout_itemlayout.h"
#include "xfa_layout_pagemgr_new.h"
#include "xfa_layout_appadapter.h"
CXFA_LayoutProcessor* CXFA_Document::GetLayoutProcessor()
{
    if(!m_pLayoutProcessor) {
        m_pLayoutProcessor = FX_NEW CXFA_LayoutProcessor(this);
        ASSERT(m_pLayoutProcessor);
    }
    return m_pLayoutProcessor;
}
IXFA_DocLayout* CXFA_Document::GetDocLayout()
{
    return GetLayoutProcessor();
}
CXFA_LayoutProcessor::CXFA_LayoutProcessor(CXFA_Document* pDocument)
    : m_pDocument(pDocument)
    , m_pRootItemLayoutProcessor(NULL)
    , m_pLayoutPageMgr(NULL)
    , m_nProgressCounter(0)
    , m_bNeeLayout(TRUE)
{
}
CXFA_LayoutProcessor::~CXFA_LayoutProcessor()
{
    ClearLayoutData();
}
CXFA_Document* CXFA_LayoutProcessor::GetDocument() const
{
    return m_pDocument;
}
FX_INT32 CXFA_LayoutProcessor::StartLayout(FX_BOOL bForceRestart)
{
    if (!bForceRestart && !IsNeedLayout()) {
        return 100;
    }
    if(m_pRootItemLayoutProcessor) {
        delete m_pRootItemLayoutProcessor;
        m_pRootItemLayoutProcessor = NULL;
    }
    m_nProgressCounter = 0;
    CXFA_Node* pFormPacketNode = (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form);
    if (!pFormPacketNode) {
        return -1;
    }
    CXFA_Node* pFormRoot = pFormPacketNode->GetFirstChildByClass(XFA_ELEMENT_Subform);
    if(!pFormRoot) {
        return -1;
    }
    if (!m_pLayoutPageMgr) {
        m_pLayoutPageMgr = FX_NEW CXFA_LayoutPageMgr(this);
    }
    if (!m_pLayoutPageMgr->InitLayoutPage(pFormRoot)) {
        return -1;
    }
    if (!m_pLayoutPageMgr->PrepareFirstPage(pFormRoot)) {
        return -1;
    }
    m_pRootItemLayoutProcessor = FX_NEW CXFA_ItemLayoutProcessor(pFormRoot, m_pLayoutPageMgr);
#ifndef _XFA_LAYOUTITEM_ProcessCACHE_
    m_pRootItemLayoutProcessor->m_pPageMgrCreateItem = m_pLayoutPageMgr;
#endif
    m_nProgressCounter = 1;
    return 0;
}
FX_INT32 CXFA_LayoutProcessor::DoLayout(IFX_Pause* pPause )
{
    if (m_nProgressCounter < 1) {
        return -1;
    }
    XFA_ItemLayoutProcessorResult eStatus;
    CXFA_Node* pFormNode =  m_pRootItemLayoutProcessor->GetFormNode();
    FX_FLOAT fPosX = pFormNode->GetMeasure(XFA_ATTRIBUTE_X).ToUnit(XFA_UNIT_Pt);
    FX_FLOAT fPosY = pFormNode->GetMeasure(XFA_ATTRIBUTE_Y).ToUnit(XFA_UNIT_Pt);
    do {
        FX_FLOAT fAvailHeight = m_pLayoutPageMgr->GetAvailHeight();
        eStatus = m_pRootItemLayoutProcessor->DoLayout(TRUE, fAvailHeight, fAvailHeight);
        if(eStatus != XFA_ItemLayoutProcessorResult_Done) {
            m_nProgressCounter++;
        }
        CXFA_ContentLayoutItemImpl* pLayoutItem = m_pRootItemLayoutProcessor->ExtractLayoutItem();
        if(pLayoutItem) {
            pLayoutItem->m_sPos.Set(fPosX, fPosY);
        }
        m_pLayoutPageMgr->SubmitContentItem(pLayoutItem, eStatus);
    } while (eStatus != XFA_ItemLayoutProcessorResult_Done && (!pPause || !pPause->NeedToPauseNow()));
    if (eStatus == XFA_ItemLayoutProcessorResult_Done) {
        m_pLayoutPageMgr->FinishPaginatedPageSets();
        m_pLayoutPageMgr->SyncLayoutData();
        m_bNeeLayout = FALSE;
        m_rgChangedContainers.RemoveAll();
    }
    return 100 * (eStatus == XFA_ItemLayoutProcessorResult_Done ? m_nProgressCounter : m_nProgressCounter - 1) / m_nProgressCounter;
}
FX_BOOL CXFA_LayoutProcessor::IncrementLayout()
{
    if (m_bNeeLayout) {
        StartLayout(TRUE);
        return DoLayout(NULL) == 100;
    }
    for (FX_INT32 i = 0, c = m_rgChangedContainers.GetSize(); i < c; i++) {
        CXFA_Node *pNode = m_rgChangedContainers[i];
        CXFA_Node *pParentNode = pNode->GetNodeItem(XFA_NODEITEM_Parent, XFA_OBJECTTYPE_ContainerNode);
        if(!pParentNode) {
            return FALSE;
        }
        if(!CXFA_ItemLayoutProcessor::IncrementRelayoutNode(this, pNode, pParentNode)) {
            return FALSE;
        }
    }
    m_rgChangedContainers.RemoveAll();
    return TRUE;
}
FX_INT32 CXFA_LayoutProcessor::CountPages() const
{
    return m_pLayoutPageMgr ? m_pLayoutPageMgr->GetPageCount() : 0;
}
IXFA_LayoutPage* CXFA_LayoutProcessor::GetPage(FX_INT32 index) const
{
    return m_pLayoutPageMgr ? m_pLayoutPageMgr->GetPage(index) : NULL;
}
CXFA_LayoutItem* CXFA_LayoutProcessor::GetLayoutItem(CXFA_Node *pFormItem)
{
    return (CXFA_LayoutItem*)pFormItem->GetUserData(XFA_LAYOUTITEMKEY);
}
void CXFA_LayoutProcessor::AddChangedContainer(CXFA_Node* pContainer)
{
    if (m_rgChangedContainers.Find(pContainer) < 0) {
        m_rgChangedContainers.Add(pContainer);
    }
}
CXFA_ContainerLayoutItemImpl* CXFA_LayoutProcessor::GetRootLayoutItem() const
{
    return m_pLayoutPageMgr ? m_pLayoutPageMgr->GetRootLayoutItem() : NULL;
}
void CXFA_LayoutProcessor::ClearLayoutData()
{
    if(m_pLayoutPageMgr) {
        delete m_pLayoutPageMgr;
        m_pLayoutPageMgr = NULL;
    }
    if(m_pRootItemLayoutProcessor) {
        delete m_pRootItemLayoutProcessor;
        m_pRootItemLayoutProcessor = NULL;
    }
    m_nProgressCounter = 0;
}
FX_BOOL CXFA_LayoutProcessor::IsNeedLayout()
{
    return m_bNeeLayout || m_rgChangedContainers.GetSize() > 0;
}
CXFA_LayoutItemImpl::CXFA_LayoutItemImpl(CXFA_Node *pNode, FX_BOOL bIsContentLayoutItem)
    : m_pFormNode(pNode)
    , m_bIsContentLayoutItem(bIsContentLayoutItem)
    , m_pParent(NULL)
    , m_pNextSibling(NULL)
    , m_pFirstChild(NULL)
{
}
CXFA_LayoutItemImpl::~CXFA_LayoutItemImpl()
{
}
CXFA_ContainerLayoutItemImpl::CXFA_ContainerLayoutItemImpl(CXFA_Node *pNode)
    : CXFA_LayoutItemImpl(pNode, FALSE), m_pOldSubform(NULL)
{
}
CXFA_ContentLayoutItemImpl::CXFA_ContentLayoutItemImpl(CXFA_Node *pNode)
    : CXFA_LayoutItemImpl(pNode, TRUE)
    , m_pPrev(NULL)
    , m_pNext(NULL)
    , m_dwStatus(0)
{
}
CXFA_ContentLayoutItemImpl::~CXFA_ContentLayoutItemImpl()
{
    if (m_pFormNode->GetUserData(XFA_LAYOUTITEMKEY) == this) {
        m_pFormNode->SetUserData(XFA_LAYOUTITEMKEY, NULL);
    }
}
