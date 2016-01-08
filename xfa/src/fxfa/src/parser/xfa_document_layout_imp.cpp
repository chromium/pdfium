// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_basic_imp.h"
#include "xfa_document_layout_imp.h"
#include "xfa_document_datamerger_imp.h"
#include "xfa_layout_itemlayout.h"
#include "xfa_layout_pagemgr_new.h"
#include "xfa_layout_appadapter.h"
CXFA_LayoutProcessor* CXFA_Document::GetLayoutProcessor() {
  if (!m_pLayoutProcessor) {
    m_pLayoutProcessor = new CXFA_LayoutProcessor(this);
    ASSERT(m_pLayoutProcessor);
  }
  return m_pLayoutProcessor;
}
IXFA_DocLayout* CXFA_Document::GetDocLayout() {
  return GetLayoutProcessor();
}
CXFA_LayoutProcessor::CXFA_LayoutProcessor(CXFA_Document* pDocument)
    : m_pDocument(pDocument),
      m_pRootItemLayoutProcessor(NULL),
      m_pLayoutPageMgr(NULL),
      m_nProgressCounter(0),
      m_bNeeLayout(TRUE) {}
CXFA_LayoutProcessor::~CXFA_LayoutProcessor() {
  ClearLayoutData();
}
CXFA_Document* CXFA_LayoutProcessor::GetDocument() const {
  return m_pDocument;
}
int32_t CXFA_LayoutProcessor::StartLayout(FX_BOOL bForceRestart) {
  if (!bForceRestart && !IsNeedLayout()) {
    return 100;
  }
  if (m_pRootItemLayoutProcessor) {
    delete m_pRootItemLayoutProcessor;
    m_pRootItemLayoutProcessor = NULL;
  }
  m_nProgressCounter = 0;
  CXFA_Node* pFormPacketNode =
      (CXFA_Node*)m_pDocument->GetXFANode(XFA_HASHCODE_Form);
  if (!pFormPacketNode) {
    return -1;
  }
  CXFA_Node* pFormRoot =
      pFormPacketNode->GetFirstChildByClass(XFA_ELEMENT_Subform);
  if (!pFormRoot) {
    return -1;
  }
  if (!m_pLayoutPageMgr) {
    m_pLayoutPageMgr = new CXFA_LayoutPageMgr(this);
  }
  if (!m_pLayoutPageMgr->InitLayoutPage(pFormRoot)) {
    return -1;
  }
  if (!m_pLayoutPageMgr->PrepareFirstPage(pFormRoot)) {
    return -1;
  }
  m_pRootItemLayoutProcessor =
      new CXFA_ItemLayoutProcessor(pFormRoot, m_pLayoutPageMgr);
#ifndef _XFA_LAYOUTITEM_ProcessCACHE_
  m_pRootItemLayoutProcessor->m_pPageMgrCreateItem = m_pLayoutPageMgr;
#endif
  m_nProgressCounter = 1;
  return 0;
}
int32_t CXFA_LayoutProcessor::DoLayout(IFX_Pause* pPause) {
  if (m_nProgressCounter < 1) {
    return -1;
  }
  XFA_ItemLayoutProcessorResult eStatus;
  CXFA_Node* pFormNode = m_pRootItemLayoutProcessor->GetFormNode();
  FX_FLOAT fPosX = pFormNode->GetMeasure(XFA_ATTRIBUTE_X).ToUnit(XFA_UNIT_Pt);
  FX_FLOAT fPosY = pFormNode->GetMeasure(XFA_ATTRIBUTE_Y).ToUnit(XFA_UNIT_Pt);
  do {
    FX_FLOAT fAvailHeight = m_pLayoutPageMgr->GetAvailHeight();
    eStatus =
        m_pRootItemLayoutProcessor->DoLayout(TRUE, fAvailHeight, fAvailHeight);
    if (eStatus != XFA_ItemLayoutProcessorResult_Done) {
      m_nProgressCounter++;
    }
    CXFA_ContentLayoutItem* pLayoutItem =
        m_pRootItemLayoutProcessor->ExtractLayoutItem();
    if (pLayoutItem) {
      pLayoutItem->m_sPos.Set(fPosX, fPosY);
    }
    m_pLayoutPageMgr->SubmitContentItem(pLayoutItem, eStatus);
  } while (eStatus != XFA_ItemLayoutProcessorResult_Done &&
           (!pPause || !pPause->NeedToPauseNow()));
  if (eStatus == XFA_ItemLayoutProcessorResult_Done) {
    m_pLayoutPageMgr->FinishPaginatedPageSets();
    m_pLayoutPageMgr->SyncLayoutData();
    m_bNeeLayout = FALSE;
    m_rgChangedContainers.RemoveAll();
  }
  return 100 * (eStatus == XFA_ItemLayoutProcessorResult_Done
                    ? m_nProgressCounter
                    : m_nProgressCounter - 1) /
         m_nProgressCounter;
}
FX_BOOL CXFA_LayoutProcessor::IncrementLayout() {
  if (m_bNeeLayout) {
    StartLayout(TRUE);
    return DoLayout(NULL) == 100;
  }
  for (int32_t i = 0, c = m_rgChangedContainers.GetSize(); i < c; i++) {
    CXFA_Node* pNode = m_rgChangedContainers[i];
    CXFA_Node* pParentNode =
        pNode->GetNodeItem(XFA_NODEITEM_Parent, XFA_OBJECTTYPE_ContainerNode);
    if (!pParentNode) {
      return FALSE;
    }
    if (!CXFA_ItemLayoutProcessor::IncrementRelayoutNode(this, pNode,
                                                         pParentNode)) {
      return FALSE;
    }
  }
  m_rgChangedContainers.RemoveAll();
  return TRUE;
}
int32_t CXFA_LayoutProcessor::CountPages() const {
  return m_pLayoutPageMgr ? m_pLayoutPageMgr->GetPageCount() : 0;
}
IXFA_LayoutPage* CXFA_LayoutProcessor::GetPage(int32_t index) const {
  return m_pLayoutPageMgr ? m_pLayoutPageMgr->GetPage(index) : NULL;
}
CXFA_LayoutItem* CXFA_LayoutProcessor::GetLayoutItem(CXFA_Node* pFormItem) {
  return static_cast<CXFA_LayoutItem*>(
      pFormItem->GetUserData(XFA_LAYOUTITEMKEY));
}
void CXFA_LayoutProcessor::AddChangedContainer(CXFA_Node* pContainer) {
  if (m_rgChangedContainers.Find(pContainer) < 0) {
    m_rgChangedContainers.Add(pContainer);
  }
}
CXFA_ContainerLayoutItem* CXFA_LayoutProcessor::GetRootLayoutItem() const {
  return m_pLayoutPageMgr ? m_pLayoutPageMgr->GetRootLayoutItem() : NULL;
}
void CXFA_LayoutProcessor::ClearLayoutData() {
  if (m_pLayoutPageMgr) {
    delete m_pLayoutPageMgr;
    m_pLayoutPageMgr = NULL;
  }
  if (m_pRootItemLayoutProcessor) {
    delete m_pRootItemLayoutProcessor;
    m_pRootItemLayoutProcessor = NULL;
  }
  m_nProgressCounter = 0;
}
FX_BOOL CXFA_LayoutProcessor::IsNeedLayout() {
  return m_bNeeLayout || m_rgChangedContainers.GetSize() > 0;
}
CXFA_LayoutItem::CXFA_LayoutItem(CXFA_Node* pNode, FX_BOOL bIsContentLayoutItem)
    : m_pFormNode(pNode),
      m_pParent(NULL),
      m_pNextSibling(NULL),
      m_pFirstChild(NULL),
      m_bIsContentLayoutItem(bIsContentLayoutItem) {
}
CXFA_LayoutItem::~CXFA_LayoutItem() {
}
CXFA_ContainerLayoutItem::CXFA_ContainerLayoutItem(CXFA_Node* pNode)
    : CXFA_LayoutItem(pNode, FALSE), m_pOldSubform(NULL) {
}
IXFA_DocLayout* CXFA_ContainerLayoutItem::GetLayout() const {
  return m_pFormNode->GetDocument()->GetLayoutProcessor();
}
int32_t CXFA_ContainerLayoutItem::GetPageIndex() const {
  return m_pFormNode->GetDocument()
      ->GetLayoutProcessor()
      ->GetLayoutPageMgr()
      ->GetPageIndex(this);
}
void CXFA_ContainerLayoutItem::GetPageSize(CFX_SizeF& size) {
  size.Set(0, 0);
  CXFA_Node* pMedium = m_pFormNode->GetFirstChildByClass(XFA_ELEMENT_Medium);
  if (pMedium) {
    size.x = pMedium->GetMeasure(XFA_ATTRIBUTE_Short).ToUnit(XFA_UNIT_Pt);
    size.y = pMedium->GetMeasure(XFA_ATTRIBUTE_Long).ToUnit(XFA_UNIT_Pt);
    if (pMedium->GetEnum(XFA_ATTRIBUTE_Orientation) ==
        XFA_ATTRIBUTEENUM_Landscape) {
      size.Set(size.y, size.x);
    }
  }
}
CXFA_Node* CXFA_ContainerLayoutItem::GetMasterPage() const {
  return m_pFormNode;
}
CXFA_ContentLayoutItem::CXFA_ContentLayoutItem(CXFA_Node* pNode)
    : CXFA_LayoutItem(pNode, TRUE),
      m_pPrev(NULL),
      m_pNext(NULL),
      m_dwStatus(0) {
}
CXFA_ContentLayoutItem::~CXFA_ContentLayoutItem() {
  if (m_pFormNode->GetUserData(XFA_LAYOUTITEMKEY) == this) {
    m_pFormNode->SetUserData(XFA_LAYOUTITEMKEY, NULL);
  }
}
