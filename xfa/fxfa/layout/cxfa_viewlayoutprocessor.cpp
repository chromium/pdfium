// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_viewlayoutprocessor.h"

#include <utility>

#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_traversestrategy_layoutitem.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"
#include "xfa/fxfa/parser/cxfa_contentarea.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_medium.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_pageset.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfanode.h"
#include "xfa/fxfa/parser/xfa_document_datamerger_imp.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

namespace {

class TraverseStrategy_ViewLayoutItem {
 public:
  static CXFA_ViewLayoutItem* GetFirstChild(CXFA_ViewLayoutItem* pLayoutItem) {
    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetFirstChild(); pChildItem;
         pChildItem = pChildItem->GetNextSibling()) {
      if (CXFA_ViewLayoutItem* pContainer = pChildItem->AsViewLayoutItem()) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ViewLayoutItem* GetNextSibling(CXFA_ViewLayoutItem* pLayoutItem) {
    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetNextSibling();
         pChildItem; pChildItem = pChildItem->GetNextSibling()) {
      if (CXFA_ViewLayoutItem* pContainer = pChildItem->AsViewLayoutItem()) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ViewLayoutItem* GetParent(CXFA_ViewLayoutItem* pLayoutItem) {
    return ToViewLayoutItem(pLayoutItem->GetParent());
  }
};

using ViewLayoutItemIterator =
    CXFA_NodeIteratorTemplate<CXFA_ViewLayoutItem,
                              TraverseStrategy_ViewLayoutItem>;

class TraverseStrategy_PageSet {
 public:
  static CXFA_ViewLayoutItem* GetFirstChild(CXFA_ViewLayoutItem* pLayoutItem) {
    if (pLayoutItem->GetFormNode()->GetElementType() != XFA_Element::PageSet)
      return nullptr;

    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetFirstChild(); pChildItem;
         pChildItem = pChildItem->GetNextSibling()) {
      CXFA_ViewLayoutItem* pContainer = pChildItem->AsViewLayoutItem();
      if (pContainer &&
          pContainer->GetFormNode()->GetElementType() == XFA_Element::PageSet) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ViewLayoutItem* GetNextSibling(CXFA_ViewLayoutItem* pLayoutItem) {
    for (CXFA_LayoutItem* pChildItem = pLayoutItem->GetNextSibling();
         pChildItem; pChildItem = pChildItem->GetNextSibling()) {
      CXFA_ViewLayoutItem* pContainer = pChildItem->AsViewLayoutItem();
      if (pContainer &&
          pContainer->GetFormNode()->GetElementType() == XFA_Element::PageSet) {
        return pContainer;
      }
    }
    return nullptr;
  }

  static CXFA_ViewLayoutItem* GetParent(CXFA_ViewLayoutItem* pLayoutItem) {
    return ToViewLayoutItem(pLayoutItem->GetParent());
  }
};

using PageSetIterator =
    CXFA_NodeIteratorTemplate<CXFA_ViewLayoutItem, TraverseStrategy_PageSet>;

uint32_t GetRelevant(CXFA_Node* pFormItem, uint32_t dwParentRelvant) {
  uint32_t dwRelevant = XFA_WidgetStatus_Viewable | XFA_WidgetStatus_Printable;
  WideString wsRelevant =
      pFormItem->JSObject()->GetCData(XFA_Attribute::Relevant);
  if (!wsRelevant.IsEmpty()) {
    if (wsRelevant.EqualsASCII("+print") || wsRelevant.EqualsASCII("print"))
      dwRelevant &= ~XFA_WidgetStatus_Viewable;
    else if (wsRelevant.EqualsASCII("-print"))
      dwRelevant &= ~XFA_WidgetStatus_Printable;
  }
  if (!(dwParentRelvant & XFA_WidgetStatus_Viewable) &&
      (dwRelevant != XFA_WidgetStatus_Viewable)) {
    dwRelevant &= ~XFA_WidgetStatus_Viewable;
  }
  if (!(dwParentRelvant & XFA_WidgetStatus_Printable) &&
      (dwRelevant != XFA_WidgetStatus_Printable)) {
    dwRelevant &= ~XFA_WidgetStatus_Printable;
  }
  return dwRelevant;
}

void SyncContainer(CXFA_FFNotify* pNotify,
                   CXFA_LayoutProcessor* pDocLayout,
                   CXFA_LayoutItem* pViewItem,
                   uint32_t dwRelevant,
                   bool bVisible,
                   int32_t nPageIndex) {
  bool bVisibleItem = false;
  uint32_t dwStatus = 0;
  uint32_t dwRelevantContainer = 0;
  if (bVisible) {
    XFA_AttributeValue eAttributeValue =
        pViewItem->GetFormNode()
            ->JSObject()
            ->TryEnum(XFA_Attribute::Presence, true)
            .value_or(XFA_AttributeValue::Visible);
    if (eAttributeValue == XFA_AttributeValue::Visible)
      bVisibleItem = true;

    dwRelevantContainer = GetRelevant(pViewItem->GetFormNode(), dwRelevant);
    dwStatus =
        (bVisibleItem ? XFA_WidgetStatus_Visible : 0) | dwRelevantContainer;
  }
  pNotify->OnLayoutItemAdded(pDocLayout, pViewItem, nPageIndex, dwStatus);
  for (CXFA_LayoutItem* pChild = pViewItem->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pChild->IsContentLayoutItem()) {
      SyncContainer(pNotify, pDocLayout, pChild, dwRelevantContainer,
                    bVisibleItem, nPageIndex);
    }
  }
}

void ReorderLayoutItemToTail(const RetainPtr<CXFA_LayoutItem>& pLayoutItem) {
  CXFA_LayoutItem* pParentLayoutItem = pLayoutItem->GetParent();
  if (!pParentLayoutItem)
    return;

  pParentLayoutItem->RemoveChild(pLayoutItem);
  pParentLayoutItem->AppendLastChild(pLayoutItem);
}

CXFA_Node* ResolveBreakTarget(CXFA_Node* pPageSetRoot,
                              bool bNewExprStyle,
                              WideString* pTargetAll) {
  if (!pPageSetRoot)
    return nullptr;

  CXFA_Document* pDocument = pPageSetRoot->GetDocument();
  if (pTargetAll->IsEmpty())
    return nullptr;

  pTargetAll->Trim();
  int32_t iSplitIndex = 0;
  bool bTargetAllFind = true;
  while (iSplitIndex != -1) {
    WideString wsExpr;
    Optional<size_t> iSplitNextIndex = 0;
    if (!bTargetAllFind) {
      iSplitNextIndex = pTargetAll->Find(' ', iSplitIndex);
      if (!iSplitNextIndex.has_value())
        return nullptr;
      wsExpr =
          pTargetAll->Mid(iSplitIndex, iSplitNextIndex.value() - iSplitIndex);
    } else {
      wsExpr = *pTargetAll;
    }
    if (wsExpr.IsEmpty())
      return nullptr;

    bTargetAllFind = false;
    if (wsExpr[0] == '#') {
      CXFA_Node* pNode = pDocument->GetNodeByID(
          ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Template)),
          wsExpr.Right(wsExpr.GetLength() - 1).AsStringView());
      if (pNode)
        return pNode;
    } else if (bNewExprStyle) {
      WideString wsProcessedTarget = wsExpr;
      if (wsExpr.Left(4).EqualsASCII("som(") && wsExpr.Last() == L')')
        wsProcessedTarget = wsExpr.Mid(4, wsExpr.GetLength() - 5);

      XFA_RESOLVENODE_RS rs;
      bool bRet = pDocument->GetScriptContext()->ResolveObjects(
          pPageSetRoot, wsProcessedTarget.AsStringView(), &rs,
          XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
              XFA_RESOLVENODE_Attributes | XFA_RESOLVENODE_Siblings |
              XFA_RESOLVENODE_Parent,
          nullptr);
      if (bRet && rs.objects.front()->IsNode())
        return rs.objects.front()->AsNode();
    }
    iSplitIndex = iSplitNextIndex.value();
  }
  return nullptr;
}

void SetLayoutGeneratedNodeFlag(CXFA_Node* pNode) {
  pNode->SetFlag(XFA_NodeFlag_LayoutGeneratedNode);
  pNode->ClearFlag(XFA_NodeFlag_UnusedNode);
}

// Note: Returning nullptr is not the same as returning pdfium::nullopt.
Optional<CXFA_ViewLayoutItem*> CheckContentAreaNotUsed(
    CXFA_ViewLayoutItem* pPageAreaLayoutItem,
    CXFA_Node* pContentArea) {
  for (CXFA_LayoutItem* pChild = pPageAreaLayoutItem->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    CXFA_ViewLayoutItem* pLayoutItem = pChild->AsViewLayoutItem();
    if (pLayoutItem && pLayoutItem->GetFormNode() == pContentArea) {
      if (!pLayoutItem->GetFirstChild())
        return pLayoutItem;
      return pdfium::nullopt;
    }
  }
  return nullptr;
}

void SyncRemoveLayoutItem(CXFA_LayoutItem* pLayoutItem,
                          CXFA_FFNotify* pNotify,
                          CXFA_LayoutProcessor* pDocLayout) {
  RetainPtr<CXFA_LayoutItem> pCurLayoutItem(pLayoutItem->GetFirstChild());
  while (pCurLayoutItem) {
    RetainPtr<CXFA_LayoutItem> pNextLayoutItem(
        pCurLayoutItem->GetNextSibling());
    SyncRemoveLayoutItem(pCurLayoutItem.Get(), pNotify, pDocLayout);
    pCurLayoutItem = std::move(pNextLayoutItem);
  }
  pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem);
  pLayoutItem->RemoveSelfIfParented();
}

bool RunBreakTestScript(CXFA_Script* pTestScript) {
  WideString wsExpression = pTestScript->JSObject()->GetContent(false);
  if (wsExpression.IsEmpty())
    return true;
  return pTestScript->GetDocument()->GetNotify()->RunScript(
      pTestScript, pTestScript->GetContainerParent());
}

float CalculateLayoutItemHeight(const CXFA_LayoutItem* pItem) {
  float fHeight = 0;
  for (const CXFA_LayoutItem* pChild = pItem->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    const CXFA_ContentLayoutItem* pContent = pChild->AsContentLayoutItem();
    if (pContent)
      fHeight += pContent->m_sSize.height;
  }
  return fHeight;
}

std::vector<float> GetHeightsForContentAreas(const CXFA_LayoutItem* pItem) {
  std::vector<float> heights;
  for (const CXFA_LayoutItem* pChild = pItem->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pChild->GetFormNode()->GetElementType() == XFA_Element::ContentArea)
      heights.push_back(CalculateLayoutItemHeight(pChild));
  }
  return heights;
}

std::pair<size_t, CXFA_LayoutItem*> GetPageAreaCountAndLastPageAreaFromPageSet(
    CXFA_ViewLayoutItem* pPageSetLayoutItem) {
  size_t nCount = 0;
  CXFA_LayoutItem* pLast = nullptr;
  for (CXFA_LayoutItem* pPageAreaLayoutItem =
           pPageSetLayoutItem->GetFirstChild();
       pPageAreaLayoutItem;
       pPageAreaLayoutItem = pPageAreaLayoutItem->GetNextSibling()) {
    XFA_Element type = pPageAreaLayoutItem->GetFormNode()->GetElementType();
    if (type != XFA_Element::PageArea)
      continue;

    ++nCount;
    pLast = pPageAreaLayoutItem;
  }
  return {nCount, pLast};
}

bool ContentAreasFitInPageAreas(const CXFA_Node* pNode,
                                const std::vector<float>& rgUsedHeights) {
  size_t iCurContentAreaIndex = 0;
  for (const CXFA_Node* pContentAreaNode = pNode->GetFirstChild();
       pContentAreaNode;
       pContentAreaNode = pContentAreaNode->GetNextSibling()) {
    if (pContentAreaNode->GetElementType() != XFA_Element::ContentArea)
      continue;

    if (iCurContentAreaIndex >= rgUsedHeights.size())
      return false;

    const float fHeight = pContentAreaNode->JSObject()->GetMeasureInUnit(
                              XFA_Attribute::H, XFA_Unit::Pt) +
                          kXFALayoutPrecision;
    if (rgUsedHeights[iCurContentAreaIndex] > fHeight)
      return false;

    ++iCurContentAreaIndex;
  }
  return true;
}

}  // namespace

class CXFA_ViewRecord {
 public:
  CXFA_ViewRecord(CXFA_ViewLayoutItem* pPageSet = nullptr,
                  CXFA_ViewLayoutItem* pPageArea = nullptr,
                  CXFA_ViewLayoutItem* pContentArea = nullptr)
      : pCurPageSet(pPageSet),
        pCurPageArea(pPageArea),
        pCurContentArea(pContentArea) {}

  RetainPtr<CXFA_ViewLayoutItem> pCurPageSet;
  RetainPtr<CXFA_ViewLayoutItem> pCurPageArea;
  RetainPtr<CXFA_ViewLayoutItem> pCurContentArea;
};

CXFA_ViewLayoutProcessor::CXFA_ViewLayoutProcessor(
    CXFA_LayoutProcessor* pLayoutProcessor)
    : m_pLayoutProcessor(pLayoutProcessor),
      m_CurrentViewRecordIter(m_ProposedViewRecords.end()) {}

CXFA_ViewLayoutProcessor::~CXFA_ViewLayoutProcessor() {
  ClearData();
  RetainPtr<CXFA_LayoutItem> pLayoutItem(GetRootLayoutItem());
  while (pLayoutItem) {
    CXFA_LayoutItem* pNextLayout = pLayoutItem->GetNextSibling();
    XFA_ReleaseLayoutItem(pLayoutItem);
    pLayoutItem.Reset(pNextLayout);
  }
}

bool CXFA_ViewLayoutProcessor::InitLayoutPage(CXFA_Node* pFormNode) {
  PrepareLayout();
  CXFA_Node* pTemplateNode = pFormNode->GetTemplateNodeIfExists();
  if (!pTemplateNode)
    return false;

  m_pTemplatePageSetRoot =
      pTemplateNode->JSObject()->GetOrCreateProperty<CXFA_PageSet>(
          0, XFA_Element::PageSet);
  ASSERT(m_pTemplatePageSetRoot);

  if (m_pPageSetLayoutItemRoot) {
    m_pPageSetLayoutItemRoot->RemoveSelfIfParented();
  } else {
    m_pPageSetLayoutItemRoot = pdfium::MakeRetain<CXFA_ViewLayoutItem>(
        m_pTemplatePageSetRoot, nullptr);
  }
  m_pPageSetCurRoot = m_pPageSetLayoutItemRoot;
  m_pTemplatePageSetRoot->JSObject()->SetLayoutItem(
      m_pPageSetLayoutItemRoot.Get());

  XFA_AttributeValue eRelation =
      m_pTemplatePageSetRoot->JSObject()->GetEnum(XFA_Attribute::Relation);
  if (eRelation != XFA_AttributeValue::Unknown)
    m_ePageSetMode = eRelation;

  InitPageSetMap();
  CXFA_Node* pPageArea = nullptr;
  int32_t iCount = 0;
  for (pPageArea = m_pTemplatePageSetRoot->GetFirstChild(); pPageArea;
       pPageArea = pPageArea->GetNextSibling()) {
    if (pPageArea->GetElementType() != XFA_Element::PageArea)
      continue;

    iCount++;
    if (pPageArea->GetFirstChildByClass<CXFA_ContentArea>(
            XFA_Element::ContentArea)) {
      return true;
    }
  }
  if (iCount > 0)
    return false;

  CXFA_Document* pDocument = pTemplateNode->GetDocument();
  pPageArea = m_pTemplatePageSetRoot->GetChild<CXFA_Node>(
      0, XFA_Element::PageArea, false);
  if (!pPageArea) {
    pPageArea = pDocument->CreateNode(m_pTemplatePageSetRoot->GetPacketType(),
                                      XFA_Element::PageArea);
    if (!pPageArea)
      return false;

    m_pTemplatePageSetRoot->InsertChildAndNotify(pPageArea, nullptr);
    pPageArea->SetFlagAndNotify(XFA_NodeFlag_Initialized);
  }
  CXFA_ContentArea* pContentArea =
      pPageArea->GetChild<CXFA_ContentArea>(0, XFA_Element::ContentArea, false);
  if (!pContentArea) {
    pContentArea = static_cast<CXFA_ContentArea*>(pDocument->CreateNode(
        pPageArea->GetPacketType(), XFA_Element::ContentArea));
    if (!pContentArea)
      return false;

    pPageArea->InsertChildAndNotify(pContentArea, nullptr);
    pContentArea->SetFlagAndNotify(XFA_NodeFlag_Initialized);
    pContentArea->JSObject()->SetMeasure(
        XFA_Attribute::X, CXFA_Measurement(0.25f, XFA_Unit::In), false);
    pContentArea->JSObject()->SetMeasure(
        XFA_Attribute::Y, CXFA_Measurement(0.25f, XFA_Unit::In), false);
    pContentArea->JSObject()->SetMeasure(
        XFA_Attribute::W, CXFA_Measurement(8.0f, XFA_Unit::In), false);
    pContentArea->JSObject()->SetMeasure(
        XFA_Attribute::H, CXFA_Measurement(10.5f, XFA_Unit::In), false);
  }
  CXFA_Medium* pMedium =
      pPageArea->GetChild<CXFA_Medium>(0, XFA_Element::Medium, false);
  if (!pMedium) {
    pMedium = static_cast<CXFA_Medium*>(
        pDocument->CreateNode(pPageArea->GetPacketType(), XFA_Element::Medium));
    if (!pContentArea)
      return false;

    pPageArea->InsertChildAndNotify(pMedium, nullptr);
    pMedium->SetFlagAndNotify(XFA_NodeFlag_Initialized);
    pMedium->JSObject()->SetMeasure(
        XFA_Attribute::Short, CXFA_Measurement(8.5f, XFA_Unit::In), false);
    pMedium->JSObject()->SetMeasure(
        XFA_Attribute::Long, CXFA_Measurement(11.0f, XFA_Unit::In), false);
  }
  return true;
}

bool CXFA_ViewLayoutProcessor::PrepareFirstPage(CXFA_Node* pRootSubform) {
  bool bProBreakBefore = false;
  const CXFA_Node* pBreakBeforeNode = nullptr;
  while (pRootSubform) {
    for (const CXFA_Node* pBreakNode = pRootSubform->GetFirstChild();
         pBreakNode; pBreakNode = pBreakNode->GetNextSibling()) {
      XFA_Element eType = pBreakNode->GetElementType();
      if (eType == XFA_Element::BreakBefore ||
          (eType == XFA_Element::Break &&
           pBreakNode->JSObject()->GetEnum(XFA_Attribute::Before) !=
               XFA_AttributeValue::Auto)) {
        bProBreakBefore = true;
        pBreakBeforeNode = pBreakNode;
        break;
      }
    }
    if (bProBreakBefore)
      break;

    bProBreakBefore = true;
    pRootSubform =
        pRootSubform->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
    while (pRootSubform && !pRootSubform->PresenceRequiresSpace()) {
      pRootSubform = pRootSubform->GetNextSameClassSibling<CXFA_Subform>(
          XFA_Element::Subform);
    }
  }
  if (pBreakBeforeNode) {
    BreakData ret = ExecuteBreakBeforeOrAfter(pBreakBeforeNode, true);
    if (ret.bCreatePage) {
      ResetToFirstViewRecord();
      return true;
    }
  }
  return AppendNewPage(true);
}

bool CXFA_ViewLayoutProcessor::AppendNewPage(bool bFirstTemPage) {
  if (m_CurrentViewRecordIter != GetTailPosition())
    return true;

  CXFA_Node* pPageNode = GetNextAvailPageArea(nullptr, nullptr, false, false);
  if (!pPageNode)
    return false;

  if (bFirstTemPage && !HasCurrentViewRecord())
    ResetToFirstViewRecord();
  return !bFirstTemPage || HasCurrentViewRecord();
}

void CXFA_ViewLayoutProcessor::RemoveLayoutRecord(
    CXFA_ViewRecord* pNewRecord,
    CXFA_ViewRecord* pPrevRecord) {
  if (!pNewRecord || !pPrevRecord)
    return;
  if (pNewRecord->pCurPageSet != pPrevRecord->pCurPageSet) {
    pNewRecord->pCurPageSet->RemoveSelfIfParented();
    return;
  }
  if (pNewRecord->pCurPageArea != pPrevRecord->pCurPageArea) {
    pNewRecord->pCurPageArea->RemoveSelfIfParented();
    return;
  }
  if (pNewRecord->pCurContentArea != pPrevRecord->pCurContentArea) {
    pNewRecord->pCurContentArea->RemoveSelfIfParented();
    return;
  }
}

void CXFA_ViewLayoutProcessor::ReorderPendingLayoutRecordToTail(
    CXFA_ViewRecord* pNewRecord,
    CXFA_ViewRecord* pPrevRecord) {
  if (!pNewRecord || !pPrevRecord)
    return;
  if (pNewRecord->pCurPageSet != pPrevRecord->pCurPageSet) {
    ReorderLayoutItemToTail(pNewRecord->pCurPageSet);
    return;
  }
  if (pNewRecord->pCurPageArea != pPrevRecord->pCurPageArea) {
    ReorderLayoutItemToTail(pNewRecord->pCurPageArea);
    return;
  }
  if (pNewRecord->pCurContentArea != pPrevRecord->pCurContentArea) {
    ReorderLayoutItemToTail(pNewRecord->pCurContentArea);
    return;
  }
}

void CXFA_ViewLayoutProcessor::SubmitContentItem(
    const RetainPtr<CXFA_ContentLayoutItem>& pContentLayoutItem,
    CXFA_ContentLayoutProcessor::Result eStatus) {
  if (pContentLayoutItem) {
    GetCurrentViewRecord()->pCurContentArea->AppendLastChild(
        pContentLayoutItem);
    m_bCreateOverFlowPage = false;
  }

  if (eStatus != CXFA_ContentLayoutProcessor::Result::kDone) {
    if (eStatus == CXFA_ContentLayoutProcessor::Result::kPageFullBreak &&
        m_CurrentViewRecordIter == GetTailPosition()) {
      AppendNewPage(false);
    }
    m_CurrentViewRecordIter = GetTailPosition();
    m_pCurPageArea = GetCurrentViewRecord()->pCurPageArea->GetFormNode();
  }
}

float CXFA_ViewLayoutProcessor::GetAvailHeight() {
  RetainPtr<CXFA_ViewLayoutItem> pLayoutItem =
      GetCurrentViewRecord()->pCurContentArea;
  if (!pLayoutItem || !pLayoutItem->GetFormNode())
    return 0.0f;

  float fAvailHeight = pLayoutItem->GetFormNode()->JSObject()->GetMeasureInUnit(
      XFA_Attribute::H, XFA_Unit::Pt);
  if (fAvailHeight >= kXFALayoutPrecision)
    return fAvailHeight;
  if (m_CurrentViewRecordIter == m_ProposedViewRecords.begin())
    return 0.0f;
  return FLT_MAX;
}

CXFA_ViewRecord* CXFA_ViewLayoutProcessor::AppendNewRecord(
    std::unique_ptr<CXFA_ViewRecord> pNewRecord) {
  m_ProposedViewRecords.push_back(std::move(pNewRecord));
  return m_ProposedViewRecords.back().get();
}

CXFA_ViewRecord* CXFA_ViewLayoutProcessor::CreateViewRecord(
    CXFA_Node* pPageNode,
    bool bCreateNew) {
  ASSERT(pPageNode);
  auto pNewRecord = pdfium::MakeUnique<CXFA_ViewRecord>();
  if (!HasCurrentViewRecord()) {
    CXFA_Node* pPageSet = pPageNode->GetParent();
    if (pPageSet == m_pTemplatePageSetRoot) {
      pNewRecord->pCurPageSet = m_pPageSetLayoutItemRoot;
    } else {
      auto pPageSetLayoutItem =
          pdfium::MakeRetain<CXFA_ViewLayoutItem>(pPageSet, nullptr);
      pPageSet->JSObject()->SetLayoutItem(pPageSetLayoutItem.Get());
      m_pPageSetLayoutItemRoot->AppendLastChild(pPageSetLayoutItem);
      pNewRecord->pCurPageSet = std::move(pPageSetLayoutItem);
    }
    return AppendNewRecord(std::move(pNewRecord));
  }

  if (!IsPageSetRootOrderedOccurrence()) {
    *pNewRecord = *GetCurrentViewRecord();
    return AppendNewRecord(std::move(pNewRecord));
  }

  CXFA_Node* pPageSet = pPageNode->GetParent();
  if (!bCreateNew) {
    if (pPageSet == m_pTemplatePageSetRoot) {
      pNewRecord->pCurPageSet = m_pPageSetCurRoot;
    } else {
      RetainPtr<CXFA_ViewLayoutItem> pParentLayoutItem(
          ToViewLayoutItem(pPageSet->JSObject()->GetLayoutItem()));
      if (!pParentLayoutItem)
        pParentLayoutItem = m_pPageSetCurRoot;

      pNewRecord->pCurPageSet = pParentLayoutItem;
    }
    return AppendNewRecord(std::move(pNewRecord));
  }

  CXFA_ViewLayoutItem* pParentPageSetLayout = nullptr;
  if (pPageSet == GetCurrentViewRecord()->pCurPageSet->GetFormNode()) {
    pParentPageSetLayout =
        ToViewLayoutItem(GetCurrentViewRecord()->pCurPageSet->GetParent());
  } else {
    pParentPageSetLayout =
        ToViewLayoutItem(pPageSet->GetParent()->JSObject()->GetLayoutItem());
  }
  auto pPageSetLayoutItem =
      pdfium::MakeRetain<CXFA_ViewLayoutItem>(pPageSet, nullptr);
  pPageSet->JSObject()->SetLayoutItem(pPageSetLayoutItem.Get());
  if (!pParentPageSetLayout) {
    RetainPtr<CXFA_ViewLayoutItem> pPrePageSet(m_pPageSetLayoutItemRoot);
    while (pPrePageSet->GetNextSibling()) {
      pPrePageSet.Reset(pPrePageSet->GetNextSibling()->AsViewLayoutItem());
    }
    if (pPrePageSet->GetParent()) {
      pPrePageSet->GetParent()->InsertAfter(pPageSetLayoutItem,
                                            pPrePageSet.Get());
    }
    m_pPageSetCurRoot = pPageSetLayoutItem;
  } else {
    pParentPageSetLayout->AppendLastChild(pPageSetLayoutItem);
  }
  pNewRecord->pCurPageSet = pPageSetLayoutItem;
  return AppendNewRecord(std::move(pNewRecord));
}

CXFA_ViewRecord* CXFA_ViewLayoutProcessor::CreateViewRecordSimple() {
  auto pNewRecord = pdfium::MakeUnique<CXFA_ViewRecord>();
  if (HasCurrentViewRecord())
    *pNewRecord = *GetCurrentViewRecord();
  else
    pNewRecord->pCurPageSet = m_pPageSetLayoutItemRoot;
  return AppendNewRecord(std::move(pNewRecord));
}

void CXFA_ViewLayoutProcessor::AddPageAreaLayoutItem(
    CXFA_ViewRecord* pNewRecord,
    CXFA_Node* pNewPageArea) {
  RetainPtr<CXFA_ViewLayoutItem> pNewPageAreaLayoutItem;
  if (pdfium::IndexInBounds(m_PageArray, m_nAvailPages)) {
    RetainPtr<CXFA_ViewLayoutItem> pViewItem = m_PageArray[m_nAvailPages];
    pViewItem->SetFormNode(pNewPageArea);
    m_nAvailPages++;
    pNewPageAreaLayoutItem = std::move(pViewItem);
  } else {
    CXFA_FFNotify* pNotify = pNewPageArea->GetDocument()->GetNotify();
    auto pViewItem = pdfium::MakeRetain<CXFA_ViewLayoutItem>(
        pNewPageArea, pNotify->OnCreateViewLayoutItem(pNewPageArea));
    m_PageArray.push_back(pViewItem);
    m_nAvailPages++;
    pNotify->OnPageEvent(pViewItem.Get(), XFA_PAGEVIEWEVENT_PostRemoved);
    pNewPageAreaLayoutItem = pViewItem;
  }
  pNewRecord->pCurPageSet->AppendLastChild(pNewPageAreaLayoutItem);
  pNewRecord->pCurPageArea = pNewPageAreaLayoutItem;
  pNewRecord->pCurContentArea = nullptr;
}

void CXFA_ViewLayoutProcessor::AddContentAreaLayoutItem(
    CXFA_ViewRecord* pNewRecord,
    CXFA_Node* pContentArea) {
  if (!pContentArea) {
    pNewRecord->pCurContentArea = nullptr;
    return;
  }
  auto pNewViewLayoutItem =
      pdfium::MakeRetain<CXFA_ViewLayoutItem>(pContentArea, nullptr);
  pNewRecord->pCurPageArea->AppendLastChild(pNewViewLayoutItem);
  pNewRecord->pCurContentArea = std::move(pNewViewLayoutItem);
}

void CXFA_ViewLayoutProcessor::FinishPaginatedPageSets() {
  for (CXFA_ViewLayoutItem* pRootPageSetLayoutItem =
           m_pPageSetLayoutItemRoot.Get();
       pRootPageSetLayoutItem; pRootPageSetLayoutItem = ToViewLayoutItem(
                                   pRootPageSetLayoutItem->GetNextSibling())) {
    PageSetIterator sIterator(pRootPageSetLayoutItem);
    for (CXFA_ViewLayoutItem* pPageSetLayoutItem = sIterator.GetCurrent();
         pPageSetLayoutItem; pPageSetLayoutItem = sIterator.MoveToNext()) {
      XFA_AttributeValue ePageRelation =
          pPageSetLayoutItem->GetFormNode()->JSObject()->GetEnum(
              XFA_Attribute::Relation);
      switch (ePageRelation) {
        case XFA_AttributeValue::SimplexPaginated:
        case XFA_AttributeValue::DuplexPaginated:
          ProcessSimplexOrDuplexPageSets(
              pPageSetLayoutItem,
              ePageRelation == XFA_AttributeValue::SimplexPaginated);
          break;
        default:
          ProcessLastPageSet();
          break;
      }
    }
  }
}

int32_t CXFA_ViewLayoutProcessor::GetPageCount() const {
  return pdfium::CollectionSize<int32_t>(m_PageArray);
}

CXFA_ViewLayoutItem* CXFA_ViewLayoutProcessor::GetPage(int32_t index) const {
  if (!pdfium::IndexInBounds(m_PageArray, index))
    return nullptr;
  return m_PageArray[index].Get();
}

int32_t CXFA_ViewLayoutProcessor::GetPageIndex(
    const CXFA_ViewLayoutItem* pPage) const {
  auto it = std::find(m_PageArray.begin(), m_PageArray.end(), pPage);
  return it != m_PageArray.end() ? it - m_PageArray.begin() : -1;
}

bool CXFA_ViewLayoutProcessor::RunBreak(XFA_Element eBreakType,
                                        XFA_AttributeValue eTargetType,
                                        CXFA_Node* pTarget,
                                        bool bStartNew) {
  bool bRet = false;
  switch (eTargetType) {
    case XFA_AttributeValue::ContentArea:
      if (pTarget && pTarget->GetElementType() != XFA_Element::ContentArea)
        pTarget = nullptr;
      if (ShouldGetNextPageArea(pTarget, bStartNew)) {
        CXFA_Node* pPageArea = nullptr;
        if (pTarget)
          pPageArea = pTarget->GetParent();

        pPageArea = GetNextAvailPageArea(pPageArea, pTarget, false, false);
        bRet = !!pPageArea;
      }
      break;
    case XFA_AttributeValue::PageArea:
      if (pTarget && pTarget->GetElementType() != XFA_Element::PageArea)
        pTarget = nullptr;
      if (ShouldGetNextPageArea(pTarget, bStartNew)) {
        CXFA_Node* pPageArea =
            GetNextAvailPageArea(pTarget, nullptr, true, false);
        bRet = !!pPageArea;
      }
      break;
    case XFA_AttributeValue::PageOdd:
      if (pTarget && pTarget->GetElementType() != XFA_Element::PageArea)
        pTarget = nullptr;
      break;
    case XFA_AttributeValue::PageEven:
      if (pTarget && pTarget->GetElementType() != XFA_Element::PageArea)
        pTarget = nullptr;
      break;
    case XFA_AttributeValue::Auto:
    default:
      break;
  }
  return bRet;
}

bool CXFA_ViewLayoutProcessor::ShouldGetNextPageArea(CXFA_Node* pTarget,
                                                     bool bStartNew) const {
  return bStartNew || !pTarget || !HasCurrentViewRecord() ||
         pTarget != GetCurrentViewRecord()->pCurPageArea->GetFormNode();
}

CXFA_ViewLayoutProcessor::BreakData
CXFA_ViewLayoutProcessor::ExecuteBreakBeforeOrAfter(const CXFA_Node* pCurNode,
                                                    bool bBefore) {
  BreakData ret = {nullptr, nullptr, false};
  XFA_Element eType = pCurNode->GetElementType();
  switch (eType) {
    case XFA_Element::BreakBefore:
    case XFA_Element::BreakAfter: {
      WideString wsBreakLeader;
      WideString wsBreakTrailer;
      CXFA_Node* pFormNode = pCurNode->GetContainerParent();
      CXFA_Node* pContainer = pFormNode->GetTemplateNodeIfExists();
      bool bStartNew =
          pCurNode->JSObject()->GetInteger(XFA_Attribute::StartNew) != 0;
      CXFA_Script* pScript =
          pCurNode->GetFirstChildByClass<CXFA_Script>(XFA_Element::Script);
      if (pScript && !RunBreakTestScript(pScript))
        break;

      WideString wsTarget =
          pCurNode->JSObject()->GetCData(XFA_Attribute::Target);
      CXFA_Node* pTarget =
          ResolveBreakTarget(m_pTemplatePageSetRoot, true, &wsTarget);
      wsBreakTrailer = pCurNode->JSObject()->GetCData(XFA_Attribute::Trailer);
      wsBreakLeader = pCurNode->JSObject()->GetCData(XFA_Attribute::Leader);
      ret.pLeader = ResolveBreakTarget(pContainer, true, &wsBreakLeader);
      ret.pTrailer = ResolveBreakTarget(pContainer, true, &wsBreakTrailer);
      if (RunBreak(eType,
                   pCurNode->JSObject()->GetEnum(XFA_Attribute::TargetType),
                   pTarget, bStartNew)) {
        ret.bCreatePage = true;
        break;
      }
      if (!m_ProposedViewRecords.empty() &&
          m_CurrentViewRecordIter == m_ProposedViewRecords.begin() &&
          eType == XFA_Element::BreakBefore) {
        CXFA_Node* pParentNode = pFormNode->GetContainerParent();
        if (!pParentNode ||
            pFormNode != pParentNode->GetFirstContainerChild()) {
          break;
        }
        pParentNode = pParentNode->GetParent();
        if (!pParentNode ||
            pParentNode->GetElementType() != XFA_Element::Form) {
          break;
        }
        ret.bCreatePage = true;
      }
      break;
    }
    case XFA_Element::Break: {
      bool bStartNew =
          pCurNode->JSObject()->GetInteger(XFA_Attribute::StartNew) != 0;
      WideString wsTarget = pCurNode->JSObject()->GetCData(
          bBefore ? XFA_Attribute::BeforeTarget : XFA_Attribute::AfterTarget);
      CXFA_Node* pTarget =
          ResolveBreakTarget(m_pTemplatePageSetRoot, true, &wsTarget);
      if (RunBreak(bBefore ? XFA_Element::BreakBefore : XFA_Element::BreakAfter,
                   pCurNode->JSObject()->GetEnum(
                       bBefore ? XFA_Attribute::Before : XFA_Attribute::After),
                   pTarget, bStartNew)) {
        ret.bCreatePage = true;
      }
      break;
    }
    default:
      break;
  }
  return ret;
}

Optional<CXFA_ViewLayoutProcessor::BreakData>
CXFA_ViewLayoutProcessor::ProcessBreakBefore(const CXFA_Node* pBreakNode) {
  return ProcessBreakBeforeOrAfter(pBreakNode, /*before=*/true);
}

Optional<CXFA_ViewLayoutProcessor::BreakData>
CXFA_ViewLayoutProcessor::ProcessBreakAfter(const CXFA_Node* pBreakNode) {
  return ProcessBreakBeforeOrAfter(pBreakNode, /*before=*/false);
}

Optional<CXFA_ViewLayoutProcessor::BreakData>
CXFA_ViewLayoutProcessor::ProcessBreakBeforeOrAfter(const CXFA_Node* pBreakNode,
                                                    bool bBefore) {
  CXFA_Node* pFormNode = pBreakNode->GetContainerParent();
  if (!pFormNode->PresenceRequiresSpace())
    return pdfium::nullopt;

  BreakData break_data = ExecuteBreakBeforeOrAfter(pBreakNode, bBefore);
  CXFA_Document* pDocument = pBreakNode->GetDocument();
  CXFA_Node* pDataScope = nullptr;
  pFormNode = pFormNode->GetContainerParent();
  if (break_data.pLeader) {
    if (!break_data.pLeader->IsContainerNode())
      return pdfium::nullopt;

    pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
    break_data.pLeader = pDocument->DataMerge_CopyContainer(
        break_data.pLeader, pFormNode, pDataScope, true, true, true);
    if (!break_data.pLeader)
      return pdfium::nullopt;

    pDocument->DataMerge_UpdateBindingRelations(break_data.pLeader);
    SetLayoutGeneratedNodeFlag(break_data.pLeader);
  }
  if (break_data.pTrailer) {
    if (!break_data.pTrailer->IsContainerNode())
      return pdfium::nullopt;

    if (!pDataScope)
      pDataScope = XFA_DataMerge_FindDataScope(pFormNode);

    break_data.pTrailer = pDocument->DataMerge_CopyContainer(
        break_data.pTrailer, pFormNode, pDataScope, true, true, true);
    if (!break_data.pTrailer)
      return pdfium::nullopt;

    pDocument->DataMerge_UpdateBindingRelations(break_data.pTrailer);
    SetLayoutGeneratedNodeFlag(break_data.pTrailer);
  }
  return break_data;
}

CXFA_Node* CXFA_ViewLayoutProcessor::ProcessBookendLeader(
    const CXFA_Node* pBookendNode) {
  return ProcessBookendLeaderOrTrailer(pBookendNode, /*leader=*/true);
}

CXFA_Node* CXFA_ViewLayoutProcessor::ProcessBookendTrailer(
    const CXFA_Node* pBookendNode) {
  return ProcessBookendLeaderOrTrailer(pBookendNode, /*leader=*/false);
}

CXFA_Node* CXFA_ViewLayoutProcessor::ProcessBookendLeaderOrTrailer(
    const CXFA_Node* pBookendNode,
    bool bLeader) {
  CXFA_Node* pFormNode = pBookendNode->GetContainerParent();
  CXFA_Node* pLeaderTemplate =
      ResolveBookendLeaderOrTrailer(pBookendNode, bLeader);
  if (!pLeaderTemplate)
    return nullptr;

  CXFA_Document* pDocument = pBookendNode->GetDocument();
  CXFA_Node* pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
  CXFA_Node* pBookendAppendNode = pDocument->DataMerge_CopyContainer(
      pLeaderTemplate, pFormNode, pDataScope, true, true, true);
  if (!pBookendAppendNode)
    return nullptr;

  pDocument->DataMerge_UpdateBindingRelations(pBookendAppendNode);
  SetLayoutGeneratedNodeFlag(pBookendAppendNode);
  return pBookendAppendNode;
}

bool CXFA_ViewLayoutProcessor::BreakOverflow(const CXFA_Node* pOverflowNode,
                                             bool bCreatePage,
                                             CXFA_Node** pLeaderTemplate,
                                             CXFA_Node** pTrailerTemplate) {
  CXFA_Node* pContainer =
      pOverflowNode->GetContainerParent()->GetTemplateNodeIfExists();
  if (pOverflowNode->GetElementType() == XFA_Element::Break) {
    WideString wsOverflowLeader =
        pOverflowNode->JSObject()->GetCData(XFA_Attribute::OverflowLeader);
    WideString wsOverflowTarget =
        pOverflowNode->JSObject()->GetCData(XFA_Attribute::OverflowTarget);
    WideString wsOverflowTrailer =
        pOverflowNode->JSObject()->GetCData(XFA_Attribute::OverflowTrailer);
    if (wsOverflowTarget.IsEmpty() && wsOverflowLeader.IsEmpty() &&
        wsOverflowTrailer.IsEmpty()) {
      return false;
    }

    if (!wsOverflowTarget.IsEmpty() && bCreatePage && !m_bCreateOverFlowPage) {
      CXFA_Node* pTarget =
          ResolveBreakTarget(m_pTemplatePageSetRoot, true, &wsOverflowTarget);
      if (pTarget) {
        m_bCreateOverFlowPage = true;
        switch (pTarget->GetElementType()) {
          case XFA_Element::PageArea:
            RunBreak(XFA_Element::Overflow, XFA_AttributeValue::PageArea,
                     pTarget, true);
            break;
          case XFA_Element::ContentArea:
            RunBreak(XFA_Element::Overflow, XFA_AttributeValue::ContentArea,
                     pTarget, true);
            break;
          default:
            break;
        }
      }
    }
    if (!bCreatePage) {
      *pLeaderTemplate =
          ResolveBreakTarget(pContainer, true, &wsOverflowLeader);
      *pTrailerTemplate =
          ResolveBreakTarget(pContainer, true, &wsOverflowTrailer);
    }
    return true;
  }

  if (pOverflowNode->GetElementType() != XFA_Element::Overflow)
    return false;

  WideString wsOverflowTarget =
      pOverflowNode->JSObject()->GetCData(XFA_Attribute::Target);
  if (!wsOverflowTarget.IsEmpty() && bCreatePage && !m_bCreateOverFlowPage) {
    CXFA_Node* pTarget =
        ResolveBreakTarget(m_pTemplatePageSetRoot, true, &wsOverflowTarget);
    if (pTarget) {
      m_bCreateOverFlowPage = true;
      switch (pTarget->GetElementType()) {
        case XFA_Element::PageArea:
          RunBreak(XFA_Element::Overflow, XFA_AttributeValue::PageArea, pTarget,
                   true);
          break;
        case XFA_Element::ContentArea:
          RunBreak(XFA_Element::Overflow, XFA_AttributeValue::ContentArea,
                   pTarget, true);
          break;
        default:
          break;
      }
    }
  }
  if (!bCreatePage) {
    WideString wsLeader =
        pOverflowNode->JSObject()->GetCData(XFA_Attribute::Leader);
    WideString wsTrailer =
        pOverflowNode->JSObject()->GetCData(XFA_Attribute::Trailer);
    *pLeaderTemplate = ResolveBreakTarget(pContainer, true, &wsLeader);
    *pTrailerTemplate = ResolveBreakTarget(pContainer, true, &wsTrailer);
  }
  return true;
}

Optional<CXFA_ViewLayoutProcessor::OverflowData>
CXFA_ViewLayoutProcessor::ProcessOverflow(CXFA_Node* pFormNode,
                                          bool bCreatePage) {
  if (!pFormNode)
    return pdfium::nullopt;

  CXFA_Node* pLeaderTemplate = nullptr;
  CXFA_Node* pTrailerTemplate = nullptr;
  bool bIsOverflowNode = pFormNode->GetElementType() == XFA_Element::Overflow ||
                         pFormNode->GetElementType() == XFA_Element::Break;
  OverflowData overflow_data{nullptr, nullptr};
  for (CXFA_Node* pCurNode = bIsOverflowNode ? pFormNode
                                             : pFormNode->GetFirstChild();
       pCurNode; pCurNode = pCurNode->GetNextSibling()) {
    if (BreakOverflow(pCurNode, bCreatePage, &pLeaderTemplate,
                      &pTrailerTemplate)) {
      if (bIsOverflowNode)
        pFormNode = pCurNode->GetParent();

      CXFA_Document* pDocument = pCurNode->GetDocument();
      CXFA_Node* pDataScope = nullptr;
      if (pLeaderTemplate) {
        pDataScope = XFA_DataMerge_FindDataScope(pFormNode);

        overflow_data.pLeader = pDocument->DataMerge_CopyContainer(
            pLeaderTemplate, pFormNode, pDataScope, true, true, true);
        if (!overflow_data.pLeader)
          return pdfium::nullopt;

        pDocument->DataMerge_UpdateBindingRelations(overflow_data.pLeader);
        SetLayoutGeneratedNodeFlag(overflow_data.pLeader);
      }
      if (pTrailerTemplate) {
        if (!pDataScope)
          pDataScope = XFA_DataMerge_FindDataScope(pFormNode);

        overflow_data.pTrailer = pDocument->DataMerge_CopyContainer(
            pTrailerTemplate, pFormNode, pDataScope, true, true, true);
        if (!overflow_data.pTrailer)
          return pdfium::nullopt;

        pDocument->DataMerge_UpdateBindingRelations(overflow_data.pTrailer);
        SetLayoutGeneratedNodeFlag(overflow_data.pTrailer);
      }
      return overflow_data;
    }
    if (bIsOverflowNode)
      break;
  }
  return pdfium::nullopt;
}

CXFA_Node* CXFA_ViewLayoutProcessor::ResolveBookendLeaderOrTrailer(
    const CXFA_Node* pBookendNode,
    bool bLeader) {
  CXFA_Node* pContainer =
      pBookendNode->GetContainerParent()->GetTemplateNodeIfExists();
  if (pBookendNode->GetElementType() == XFA_Element::Break) {
    WideString leader = pBookendNode->JSObject()->GetCData(
        bLeader ? XFA_Attribute::BookendLeader : XFA_Attribute::BookendTrailer);
    if (leader.IsEmpty())
      return nullptr;
    return ResolveBreakTarget(pContainer, false, &leader);
  }

  if (pBookendNode->GetElementType() != XFA_Element::Bookend)
    return nullptr;

  WideString leader = pBookendNode->JSObject()->GetCData(
      bLeader ? XFA_Attribute::Leader : XFA_Attribute::Trailer);
  return ResolveBreakTarget(pContainer, true, &leader);
}

bool CXFA_ViewLayoutProcessor::FindPageAreaFromPageSet(
    CXFA_Node* pPageSet,
    CXFA_Node* pStartChild,
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    bool bNewPage,
    bool bQuery) {
  if (!pPageSet && !pStartChild)
    return false;

  if (IsPageSetRootOrderedOccurrence()) {
    return FindPageAreaFromPageSet_Ordered(pPageSet, pStartChild,
                                           pTargetPageArea, pTargetContentArea,
                                           bNewPage, bQuery);
  }
  XFA_AttributeValue ePreferredPosition = HasCurrentViewRecord()
                                              ? XFA_AttributeValue::Rest
                                              : XFA_AttributeValue::First;
  return FindPageAreaFromPageSet_SimplexDuplex(
      pPageSet, pStartChild, pTargetPageArea, pTargetContentArea, bNewPage,
      bQuery, ePreferredPosition);
}

bool CXFA_ViewLayoutProcessor::FindPageAreaFromPageSet_Ordered(
    CXFA_Node* pPageSet,
    CXFA_Node* pStartChild,
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    bool bNewPage,
    bool bQuery) {
  int32_t iPageSetCount = 0;
  if (!pStartChild && !bQuery) {
    auto it = m_pPageSetMap.find(pPageSet);
    if (it != m_pPageSetMap.end())
      iPageSetCount = it->second;
    int32_t iMax = -1;
    CXFA_Node* pOccurNode =
        pPageSet->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
    if (pOccurNode) {
      Optional<int32_t> ret =
          pOccurNode->JSObject()->TryInteger(XFA_Attribute::Max, false);
      if (ret)
        iMax = *ret;
    }
    if (iMax >= 0 && iMax <= iPageSetCount)
      return false;
  }

  bool bRes = false;
  CXFA_Node* pCurrentNode =
      pStartChild ? pStartChild->GetNextSibling() : pPageSet->GetFirstChild();
  for (; pCurrentNode; pCurrentNode = pCurrentNode->GetNextSibling()) {
    if (pCurrentNode->GetElementType() == XFA_Element::PageArea) {
      if ((pTargetPageArea == pCurrentNode || !pTargetPageArea)) {
        if (!pCurrentNode->GetFirstChildByClass<CXFA_ContentArea>(
                XFA_Element::ContentArea)) {
          if (pTargetPageArea == pCurrentNode) {
            CreateMinPageRecord(pCurrentNode, true, false);
            pTargetPageArea = nullptr;
          }
          continue;
        }
        if (!bQuery) {
          CXFA_ViewRecord* pNewRecord =
              CreateViewRecord(pCurrentNode, !pStartChild);
          AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
          if (!pTargetContentArea) {
            pTargetContentArea =
                pCurrentNode->GetFirstChildByClass<CXFA_ContentArea>(
                    XFA_Element::ContentArea);
          }
          AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
        }
        m_pCurPageArea = pCurrentNode;
        m_nCurPageCount = 1;
        bRes = true;
        break;
      }
      if (!bQuery)
        CreateMinPageRecord(pCurrentNode, false, false);
    } else if (pCurrentNode->GetElementType() == XFA_Element::PageSet) {
      if (FindPageAreaFromPageSet_Ordered(pCurrentNode, nullptr,
                                          pTargetPageArea, pTargetContentArea,
                                          bNewPage, bQuery)) {
        bRes = true;
        break;
      }
      if (!bQuery)
        CreateMinPageSetRecord(pCurrentNode, true);
    }
  }
  if (!pStartChild && bRes && !bQuery)
    m_pPageSetMap[pPageSet] = ++iPageSetCount;
  return bRes;
}

bool CXFA_ViewLayoutProcessor::FindPageAreaFromPageSet_SimplexDuplex(
    CXFA_Node* pPageSet,
    CXFA_Node* pStartChild,
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    bool bNewPage,
    bool bQuery,
    XFA_AttributeValue ePreferredPosition) {
  const XFA_AttributeValue eFallbackPosition = XFA_AttributeValue::Any;
  CXFA_Node* pPreferredPageArea = nullptr;
  CXFA_Node* pFallbackPageArea = nullptr;
  CXFA_Node* pCurrentNode = nullptr;
  if (!pStartChild || pStartChild->GetElementType() == XFA_Element::PageArea)
    pCurrentNode = pPageSet->GetFirstChild();
  else
    pCurrentNode = pStartChild->GetNextSibling();

  for (; pCurrentNode; pCurrentNode = pCurrentNode->GetNextSibling()) {
    if (pCurrentNode->GetElementType() == XFA_Element::PageArea) {
      if (!MatchPageAreaOddOrEven(pCurrentNode))
        continue;

      XFA_AttributeValue eCurPagePosition =
          pCurrentNode->JSObject()->GetEnum(XFA_Attribute::PagePosition);
      if (ePreferredPosition == XFA_AttributeValue::Last) {
        if (eCurPagePosition != ePreferredPosition)
          continue;
        if (m_ePageSetMode == XFA_AttributeValue::SimplexPaginated ||
            pCurrentNode->JSObject()->GetEnum(XFA_Attribute::OddOrEven) ==
                XFA_AttributeValue::Any) {
          pPreferredPageArea = pCurrentNode;
          break;
        }
        CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
        AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
        AddContentAreaLayoutItem(
            pNewRecord, pCurrentNode->GetFirstChildByClass<CXFA_ContentArea>(
                            XFA_Element::ContentArea));
        return false;
      }
      if (ePreferredPosition == XFA_AttributeValue::Only) {
        if (eCurPagePosition != ePreferredPosition)
          continue;
        if (m_ePageSetMode != XFA_AttributeValue::DuplexPaginated ||
            pCurrentNode->JSObject()->GetEnum(XFA_Attribute::OddOrEven) ==
                XFA_AttributeValue::Any) {
          pPreferredPageArea = pCurrentNode;
          break;
        }
        return false;
      }
      if ((pTargetPageArea == pCurrentNode || !pTargetPageArea)) {
        if (!pCurrentNode->GetFirstChildByClass<CXFA_ContentArea>(
                XFA_Element::ContentArea)) {
          if (pTargetPageArea == pCurrentNode) {
            CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
            AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
            pTargetPageArea = nullptr;
          }
          continue;
        }
        if ((ePreferredPosition == XFA_AttributeValue::Rest &&
             eCurPagePosition == XFA_AttributeValue::Any) ||
            eCurPagePosition == ePreferredPosition) {
          pPreferredPageArea = pCurrentNode;
          break;
        }
        if (eCurPagePosition == eFallbackPosition && !pFallbackPageArea) {
          pFallbackPageArea = pCurrentNode;
        }
      } else if (pTargetPageArea && !MatchPageAreaOddOrEven(pTargetPageArea)) {
        CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
        AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
        AddContentAreaLayoutItem(
            pNewRecord, pCurrentNode->GetFirstChildByClass<CXFA_ContentArea>(
                            XFA_Element::ContentArea));
      }
    } else if (pCurrentNode->GetElementType() == XFA_Element::PageSet) {
      if (FindPageAreaFromPageSet_SimplexDuplex(
              pCurrentNode, nullptr, pTargetPageArea, pTargetContentArea,
              bNewPage, bQuery, ePreferredPosition)) {
        break;
      }
    }
  }

  CXFA_Node* pCurPageArea = nullptr;
  if (pPreferredPageArea)
    pCurPageArea = pPreferredPageArea;
  else if (pFallbackPageArea)
    pCurPageArea = pFallbackPageArea;

  if (!pCurPageArea)
    return false;

  if (!bQuery) {
    CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
    AddPageAreaLayoutItem(pNewRecord, pCurPageArea);
    if (!pTargetContentArea) {
      pTargetContentArea = pCurPageArea->GetFirstChildByClass<CXFA_ContentArea>(
          XFA_Element::ContentArea);
    }
    AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
  }
  m_pCurPageArea = pCurPageArea;
  return true;
}

bool CXFA_ViewLayoutProcessor::MatchPageAreaOddOrEven(CXFA_Node* pPageArea) {
  if (m_ePageSetMode != XFA_AttributeValue::DuplexPaginated)
    return true;

  Optional<XFA_AttributeValue> ret =
      pPageArea->JSObject()->TryEnum(XFA_Attribute::OddOrEven, true);
  if (!ret || *ret == XFA_AttributeValue::Any)
    return true;

  int32_t iPageLast = GetPageCount() % 2;
  return *ret == XFA_AttributeValue::Odd ? iPageLast == 0 : iPageLast == 1;
}

CXFA_Node* CXFA_ViewLayoutProcessor::GetNextAvailPageArea(
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    bool bNewPage,
    bool bQuery) {
  if (!m_pCurPageArea) {
    FindPageAreaFromPageSet(m_pTemplatePageSetRoot, nullptr, pTargetPageArea,
                            pTargetContentArea, bNewPage, bQuery);
    ASSERT(m_pCurPageArea);
    return m_pCurPageArea;
  }

  if (!pTargetPageArea || pTargetPageArea == m_pCurPageArea) {
    if (!bNewPage && GetNextContentArea(pTargetContentArea))
      return m_pCurPageArea;

    if (IsPageSetRootOrderedOccurrence()) {
      int32_t iMax = -1;
      CXFA_Node* pOccurNode =
          m_pCurPageArea->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
      if (pOccurNode) {
        Optional<int32_t> ret =
            pOccurNode->JSObject()->TryInteger(XFA_Attribute::Max, false);
        if (ret)
          iMax = *ret;
      }
      if ((iMax < 0 || m_nCurPageCount < iMax)) {
        if (!bQuery) {
          CXFA_ViewRecord* pNewRecord = CreateViewRecord(m_pCurPageArea, false);
          AddPageAreaLayoutItem(pNewRecord, m_pCurPageArea);
          if (!pTargetContentArea) {
            pTargetContentArea =
                m_pCurPageArea->GetFirstChildByClass<CXFA_ContentArea>(
                    XFA_Element::ContentArea);
          }
          AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
        }
        m_nCurPageCount++;
        return m_pCurPageArea;
      }
    }
  }

  if (!bQuery && IsPageSetRootOrderedOccurrence())
    CreateMinPageRecord(m_pCurPageArea, false, true);
  if (FindPageAreaFromPageSet(m_pCurPageArea->GetParent(), m_pCurPageArea,
                              pTargetPageArea, pTargetContentArea, bNewPage,
                              bQuery)) {
    return m_pCurPageArea;
  }

  CXFA_Node* pPageSet = m_pCurPageArea->GetParent();
  while (true) {
    if (FindPageAreaFromPageSet(pPageSet, nullptr, pTargetPageArea,
                                pTargetContentArea, bNewPage, bQuery)) {
      return m_pCurPageArea;
    }
    if (!bQuery && IsPageSetRootOrderedOccurrence())
      CreateMinPageSetRecord(pPageSet, false);
    if (FindPageAreaFromPageSet(nullptr, pPageSet, pTargetPageArea,
                                pTargetContentArea, bNewPage, bQuery)) {
      return m_pCurPageArea;
    }
    if (pPageSet == m_pTemplatePageSetRoot)
      break;

    pPageSet = pPageSet->GetParent();
  }
  return nullptr;
}

bool CXFA_ViewLayoutProcessor::GetNextContentArea(CXFA_Node* pContentArea) {
  CXFA_Node* pCurContentNode =
      GetCurrentViewRecord()->pCurContentArea->GetFormNode();
  if (!pContentArea) {
    pContentArea = pCurContentNode->GetNextSameClassSibling<CXFA_ContentArea>(
        XFA_Element::ContentArea);
    if (!pContentArea)
      return false;
  } else {
    if (pContentArea->GetParent() != m_pCurPageArea)
      return false;

    Optional<CXFA_ViewLayoutItem*> pContentAreaLayout = CheckContentAreaNotUsed(
        GetCurrentViewRecord()->pCurPageArea.Get(), pContentArea);
    if (!pContentAreaLayout.has_value())
      return false;
    if (pContentAreaLayout.value()) {
      if (pContentAreaLayout.value()->GetFormNode() == pCurContentNode)
        return false;

      CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
      pNewRecord->pCurContentArea.Reset(pContentAreaLayout.value());
      return true;
    }
  }

  CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
  AddContentAreaLayoutItem(pNewRecord, pContentArea);
  return true;
}

void CXFA_ViewLayoutProcessor::InitPageSetMap() {
  if (!IsPageSetRootOrderedOccurrence())
    return;

  CXFA_NodeIterator sIterator(m_pTemplatePageSetRoot);
  for (CXFA_Node* pPageSetNode = sIterator.GetCurrent(); pPageSetNode;
       pPageSetNode = sIterator.MoveToNext()) {
    if (pPageSetNode->GetElementType() == XFA_Element::PageSet) {
      XFA_AttributeValue eRelation =
          pPageSetNode->JSObject()->GetEnum(XFA_Attribute::Relation);
      if (eRelation == XFA_AttributeValue::OrderedOccurrence)
        m_pPageSetMap[pPageSetNode] = 0;
    }
  }
}

int32_t CXFA_ViewLayoutProcessor::CreateMinPageRecord(CXFA_Node* pPageArea,
                                                      bool bTargetPageArea,
                                                      bool bCreateLast) {
  if (!pPageArea)
    return 0;

  int32_t iMin = 0;
  Optional<int32_t> ret;
  CXFA_Node* pOccurNode =
      pPageArea->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
  if (pOccurNode) {
    ret = pOccurNode->JSObject()->TryInteger(XFA_Attribute::Min, false);
    if (ret)
      iMin = *ret;
  }

  if (!ret && !bTargetPageArea)
    return iMin;

  CXFA_Node* pContentArea = pPageArea->GetFirstChildByClass<CXFA_ContentArea>(
      XFA_Element::ContentArea);
  if (iMin < 1 && bTargetPageArea && !pContentArea)
    iMin = 1;

  int32_t i = 0;
  if (bCreateLast)
    i = m_nCurPageCount;

  for (; i < iMin; i++) {
    CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
    AddPageAreaLayoutItem(pNewRecord, pPageArea);
    AddContentAreaLayoutItem(pNewRecord, pContentArea);
  }
  return iMin;
}

void CXFA_ViewLayoutProcessor::CreateMinPageSetRecord(CXFA_Node* pPageSet,
                                                      bool bCreateAll) {
  if (!pPageSet)
    return;

  auto it = m_pPageSetMap.find(pPageSet);
  if (it == m_pPageSetMap.end())
    return;

  int32_t iCurSetCount = it->second;
  if (bCreateAll)
    iCurSetCount = 0;

  CXFA_Node* pOccurNode =
      pPageSet->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
  if (!pOccurNode)
    return;

  Optional<int32_t> iMin =
      pOccurNode->JSObject()->TryInteger(XFA_Attribute::Min, false);
  if (!iMin || iCurSetCount >= *iMin)
    return;

  for (int32_t i = 0; i < *iMin - iCurSetCount; i++) {
    for (CXFA_Node* node = pPageSet->GetFirstChild(); node;
         node = node->GetNextSibling()) {
      if (node->GetElementType() == XFA_Element::PageArea)
        CreateMinPageRecord(node, false, false);
      else if (node->GetElementType() == XFA_Element::PageSet)
        CreateMinPageSetRecord(node, true);
    }
  }
  m_pPageSetMap[pPageSet] = *iMin;
}

void CXFA_ViewLayoutProcessor::CreateNextMinRecord(CXFA_Node* pRecordNode) {
  if (!pRecordNode)
    return;

  for (CXFA_Node* pCurrentNode = pRecordNode->GetNextSibling(); pCurrentNode;
       pCurrentNode = pCurrentNode->GetNextSibling()) {
    if (pCurrentNode->GetElementType() == XFA_Element::PageArea)
      CreateMinPageRecord(pCurrentNode, false, false);
    else if (pCurrentNode->GetElementType() == XFA_Element::PageSet)
      CreateMinPageSetRecord(pCurrentNode, true);
  }
}

void CXFA_ViewLayoutProcessor::ProcessLastPageSet() {
  CreateMinPageRecord(m_pCurPageArea, false, true);
  CreateNextMinRecord(m_pCurPageArea);
  CXFA_Node* pPageSet = m_pCurPageArea->GetParent();
  while (true) {
    CreateMinPageSetRecord(pPageSet, false);
    if (pPageSet == m_pTemplatePageSetRoot)
      break;

    CreateNextMinRecord(pPageSet);
    pPageSet = pPageSet->GetParent();
  }
}

bool CXFA_ViewLayoutProcessor::GetNextAvailContentHeight(float fChildHeight) {
  CXFA_Node* pCurContentNode =
      GetCurrentViewRecord()->pCurContentArea->GetFormNode();
  if (!pCurContentNode)
    return false;

  pCurContentNode = pCurContentNode->GetNextSameClassSibling<CXFA_ContentArea>(
      XFA_Element::ContentArea);
  if (pCurContentNode) {
    float fNextContentHeight = pCurContentNode->JSObject()->GetMeasureInUnit(
        XFA_Attribute::H, XFA_Unit::Pt);
    return fNextContentHeight > fChildHeight;
  }

  CXFA_Node* pPageNode = GetCurrentViewRecord()->pCurPageArea->GetFormNode();
  CXFA_Node* pOccurNode =
      pPageNode->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
  int32_t iMax = 0;
  Optional<int32_t> ret;
  if (pOccurNode) {
    ret = pOccurNode->JSObject()->TryInteger(XFA_Attribute::Max, false);
    if (ret)
      iMax = *ret;
  }
  if (ret) {
    if (m_nCurPageCount == iMax) {
      CXFA_Node* pSrcPage = m_pCurPageArea;
      int32_t nSrcPageCount = m_nCurPageCount;
      auto psSrcIter = GetTailPosition();
      CXFA_Node* pNextPage =
          GetNextAvailPageArea(nullptr, nullptr, false, true);
      m_pCurPageArea = pSrcPage;
      m_nCurPageCount = nSrcPageCount;
      CXFA_ViewRecord* pPrevRecord = psSrcIter->get();
      ++psSrcIter;
      while (psSrcIter != m_ProposedViewRecords.end()) {
        auto psSaveIter = psSrcIter++;
        RemoveLayoutRecord(psSaveIter->get(), pPrevRecord);
        m_ProposedViewRecords.erase(psSaveIter);
      }
      if (pNextPage) {
        CXFA_Node* pContentArea =
            pNextPage->GetFirstChildByClass<CXFA_ContentArea>(
                XFA_Element::ContentArea);
        if (pContentArea) {
          float fNextContentHeight = pContentArea->JSObject()->GetMeasureInUnit(
              XFA_Attribute::H, XFA_Unit::Pt);
          if (fNextContentHeight > fChildHeight)
            return true;
        }
      }
      return false;
    }
  }

  CXFA_Node* pContentArea = pPageNode->GetFirstChildByClass<CXFA_ContentArea>(
      XFA_Element::ContentArea);
  if (!pContentArea)
    return false;

  float fNextContentHeight = pContentArea->JSObject()->GetMeasureInUnit(
      XFA_Attribute::H, XFA_Unit::Pt);
  return fNextContentHeight < kXFALayoutPrecision ||
         fNextContentHeight > fChildHeight;
}

void CXFA_ViewLayoutProcessor::ClearData() {
  if (!m_pTemplatePageSetRoot)
    return;

  m_ProposedViewRecords.clear();
  m_CurrentViewRecordIter = m_ProposedViewRecords.end();
  m_pCurPageArea = nullptr;
  m_nCurPageCount = 0;
  m_bCreateOverFlowPage = false;
  m_pPageSetMap.clear();
}

void CXFA_ViewLayoutProcessor::SaveLayoutItemChildren(
    CXFA_LayoutItem* pParentLayoutItem) {
  CXFA_Document* pDocument = m_pTemplatePageSetRoot->GetDocument();
  CXFA_FFNotify* pNotify = pDocument->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(pDocument);
  RetainPtr<CXFA_LayoutItem> pCurLayoutItem(pParentLayoutItem->GetFirstChild());
  while (pCurLayoutItem) {
    RetainPtr<CXFA_LayoutItem> pNextLayoutItem(
        pCurLayoutItem->GetNextSibling());
    if (pCurLayoutItem->IsContentLayoutItem()) {
      if (pCurLayoutItem->GetFormNode()->HasRemovedChildren()) {
        SyncRemoveLayoutItem(pCurLayoutItem.Get(), pNotify, pDocLayout);
        pCurLayoutItem = std::move(pNextLayoutItem);
        continue;
      }
      if (pCurLayoutItem->GetFormNode()->IsLayoutGeneratedNode())
        pCurLayoutItem->GetFormNode()->SetNodeAndDescendantsUnused();
    }
    SaveLayoutItemChildren(pCurLayoutItem.Get());
    pCurLayoutItem = std::move(pNextLayoutItem);
  }
}

CXFA_Node* CXFA_ViewLayoutProcessor::QueryOverflow(CXFA_Node* pFormNode) {
  for (CXFA_Node* pCurNode = pFormNode->GetFirstChild(); pCurNode;
       pCurNode = pCurNode->GetNextSibling()) {
    if (pCurNode->GetElementType() == XFA_Element::Break) {
      WideString wsOverflowLeader =
          pCurNode->JSObject()->GetCData(XFA_Attribute::OverflowLeader);
      WideString wsOverflowTarget =
          pCurNode->JSObject()->GetCData(XFA_Attribute::OverflowTarget);
      WideString wsOverflowTrailer =
          pCurNode->JSObject()->GetCData(XFA_Attribute::OverflowTrailer);

      if (!wsOverflowLeader.IsEmpty() || !wsOverflowTrailer.IsEmpty() ||
          !wsOverflowTarget.IsEmpty()) {
        return pCurNode;
      }
      return nullptr;
    }
    if (pCurNode->GetElementType() == XFA_Element::Overflow)
      return pCurNode;
  }
  return nullptr;
}

void CXFA_ViewLayoutProcessor::MergePageSetContents() {
  CXFA_Document* pDocument = m_pTemplatePageSetRoot->GetDocument();
  pDocument->SetPendingNodesUnusedAndUnbound();

  CXFA_FFNotify* pNotify = pDocument->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(pDocument);
  CXFA_ViewLayoutItem* pRootLayout = GetRootLayoutItem();

  int32_t iIndex = 0;
  for (; pRootLayout;
       pRootLayout = ToViewLayoutItem(pRootLayout->GetNextSibling())) {
    CXFA_Node* pPendingPageSet = nullptr;
    ViewLayoutItemIterator iterator(pRootLayout);
    CXFA_ViewLayoutItem* pRootPageSetViewItem = iterator.GetCurrent();
    ASSERT(pRootPageSetViewItem->GetFormNode()->GetElementType() ==
           XFA_Element::PageSet);
    if (iIndex <
        pdfium::CollectionSize<int32_t>(pDocument->m_pPendingPageSet)) {
      pPendingPageSet = pDocument->m_pPendingPageSet[iIndex];
      iIndex++;
    }
    if (!pPendingPageSet) {
      if (pRootPageSetViewItem->GetFormNode()->GetPacketType() ==
          XFA_PacketType::Template) {
        pPendingPageSet =
            pRootPageSetViewItem->GetFormNode()->CloneTemplateToForm(false);
      } else {
        pPendingPageSet = pRootPageSetViewItem->GetFormNode();
      }
    }
    if (pRootPageSetViewItem->GetFormNode()->JSObject()->GetLayoutItem() ==
        pRootPageSetViewItem) {
      pRootPageSetViewItem->GetFormNode()->JSObject()->SetLayoutItem(nullptr);
    }
    pRootPageSetViewItem->SetFormNode(pPendingPageSet);
    pPendingPageSet->ClearFlag(XFA_NodeFlag_UnusedNode);
    for (CXFA_ViewLayoutItem* pViewItem = iterator.MoveToNext(); pViewItem;
         pViewItem = iterator.MoveToNext()) {
      CXFA_Node* pNode = pViewItem->GetFormNode();
      if (pNode->GetPacketType() != XFA_PacketType::Template)
        continue;

      switch (pNode->GetElementType()) {
        case XFA_Element::PageSet: {
          CXFA_Node* pParentNode = pViewItem->GetParent()->GetFormNode();
          pViewItem->SetFormNode(XFA_NodeMerge_CloneOrMergeContainer(
              pDocument, pParentNode, pViewItem->GetFormNode(), true, nullptr));
          break;
        }
        case XFA_Element::PageArea: {
          CXFA_LayoutItem* pFormLayout = pViewItem;
          CXFA_Node* pParentNode = pViewItem->GetParent()->GetFormNode();
          bool bIsExistForm = true;
          for (int32_t iLevel = 0; iLevel < 3; iLevel++) {
            pFormLayout = pFormLayout->GetFirstChild();
            if (iLevel == 2) {
              while (pFormLayout &&
                     !pFormLayout->GetFormNode()->PresenceRequiresSpace()) {
                pFormLayout = pFormLayout->GetNextSibling();
              }
            }
            if (!pFormLayout) {
              bIsExistForm = false;
              break;
            }
          }
          if (bIsExistForm) {
            CXFA_Node* pNewSubform = pFormLayout->GetFormNode();
            if (pViewItem->m_pOldSubform &&
                pViewItem->m_pOldSubform != pNewSubform) {
              CXFA_Node* pExistingNode = XFA_DataMerge_FindFormDOMInstance(
                  pDocument, pViewItem->GetFormNode()->GetElementType(),
                  pViewItem->GetFormNode()->GetNameHash(), pParentNode);
              CXFA_ContainerIterator sIterator(pExistingNode);
              for (CXFA_Node* pIter = sIterator.GetCurrent(); pIter;
                   pIter = sIterator.MoveToNext()) {
                if (pIter->GetElementType() != XFA_Element::ContentArea) {
                  RetainPtr<CXFA_LayoutItem> pLayoutItem(
                      pIter->JSObject()->GetLayoutItem());
                  if (pLayoutItem) {
                    pNotify->OnLayoutItemRemoving(pDocLayout,
                                                  pLayoutItem.Get());
                    pLayoutItem->RemoveSelfIfParented();
                  }
                }
              }
              if (pExistingNode) {
                pParentNode->RemoveChildAndNotify(pExistingNode, true);
              }
            }
            pViewItem->m_pOldSubform = pNewSubform;
          }
          pViewItem->SetFormNode(pDocument->DataMerge_CopyContainer(
              pViewItem->GetFormNode(), pParentNode,
              ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Record)), true, true,
              true));
          break;
        }
        case XFA_Element::ContentArea: {
          CXFA_Node* pParentNode = pViewItem->GetParent()->GetFormNode();
          for (CXFA_Node* pChildNode = pParentNode->GetFirstChild(); pChildNode;
               pChildNode = pChildNode->GetNextSibling()) {
            if (pChildNode->GetTemplateNodeIfExists() !=
                pViewItem->GetFormNode()) {
              continue;
            }
            pViewItem->SetFormNode(pChildNode);
            break;
          }
          break;
        }
        default:
          break;
      }
    }
    if (!pPendingPageSet->GetParent()) {
      CXFA_Node* pNode = ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Form));
      if (pNode) {
        CXFA_Node* pFormToplevelSubform =
            pNode->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
        if (pFormToplevelSubform)
          pFormToplevelSubform->InsertChildAndNotify(pPendingPageSet, nullptr);
      }
    }
    pDocument->DataMerge_UpdateBindingRelations(pPendingPageSet);
    pPendingPageSet->SetFlagAndNotify(XFA_NodeFlag_Initialized);
  }

  CXFA_Node* pPageSet = GetRootLayoutItem()->GetFormNode();
  while (pPageSet) {
    CXFA_Node* pNextPageSet =
        pPageSet->GetNextSameClassSibling<CXFA_PageSet>(XFA_Element::PageSet);
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
        sIterator(pPageSet);
    CXFA_Node* pNode = sIterator.GetCurrent();
    while (pNode) {
      if (pNode->IsUnusedNode()) {
        if (pNode->IsContainerNode()) {
          XFA_Element eType = pNode->GetElementType();
          if (eType == XFA_Element::PageArea || eType == XFA_Element::PageSet) {
            CXFA_ContainerIterator iteChild(pNode);
            CXFA_Node* pChildNode = iteChild.MoveToNext();
            for (; pChildNode; pChildNode = iteChild.MoveToNext()) {
              RetainPtr<CXFA_LayoutItem> pLayoutItem(
                  pChildNode->JSObject()->GetLayoutItem());
              if (pLayoutItem) {
                pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem.Get());
                pLayoutItem->RemoveSelfIfParented();
              }
            }
          } else if (eType != XFA_Element::ContentArea) {
            RetainPtr<CXFA_LayoutItem> pLayoutItem(
                pNode->JSObject()->GetLayoutItem());
            if (pLayoutItem) {
              pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem.Get());
              pLayoutItem->RemoveSelfIfParented();
            }
          }
          CXFA_Node* pNext = sIterator.SkipChildrenAndMoveToNext();
          pNode->GetParent()->RemoveChildAndNotify(pNode, true);
          pNode = pNext;
        } else {
          pNode->ClearFlag(XFA_NodeFlag_UnusedNode);
          pNode->SetFlagAndNotify(XFA_NodeFlag_Initialized);
          pNode = sIterator.MoveToNext();
        }
      } else {
        pNode->SetFlagAndNotify(XFA_NodeFlag_Initialized);
        pNode = sIterator.MoveToNext();
      }
    }
    pPageSet = pNextPageSet;
  }
}

void CXFA_ViewLayoutProcessor::LayoutPageSetContents() {
  for (CXFA_ViewLayoutItem* pRootLayoutItem = GetRootLayoutItem();
       pRootLayoutItem;
       pRootLayoutItem = ToViewLayoutItem(pRootLayoutItem->GetNextSibling())) {
    ViewLayoutItemIterator iterator(pRootLayoutItem);
    for (CXFA_ViewLayoutItem* pViewItem = iterator.GetCurrent(); pViewItem;
         pViewItem = iterator.MoveToNext()) {
      XFA_Element type = pViewItem->GetFormNode()->GetElementType();
      if (type != XFA_Element::PageArea)
        continue;

      m_pLayoutProcessor->GetRootContentLayoutProcessor()->DoLayoutPageArea(
          pViewItem);
    }
  }
}

void CXFA_ViewLayoutProcessor::SyncLayoutData() {
  MergePageSetContents();
  LayoutPageSetContents();
  CXFA_FFNotify* pNotify = m_pTemplatePageSetRoot->GetDocument()->GetNotify();
  int32_t nPageIdx = -1;
  for (CXFA_ViewLayoutItem* pRootLayoutItem = GetRootLayoutItem();
       pRootLayoutItem;
       pRootLayoutItem = ToViewLayoutItem(pRootLayoutItem->GetNextSibling())) {
    ViewLayoutItemIterator iteratorParent(pRootLayoutItem);
    for (CXFA_ViewLayoutItem* pViewItem = iteratorParent.GetCurrent();
         pViewItem; pViewItem = iteratorParent.MoveToNext()) {
      XFA_Element type = pViewItem->GetFormNode()->GetElementType();
      if (type != XFA_Element::PageArea)
        continue;

      nPageIdx++;
      uint32_t dwRelevant =
          XFA_WidgetStatus_Viewable | XFA_WidgetStatus_Printable;
      CXFA_LayoutItemIterator iterator(pViewItem);
      CXFA_LayoutItem* pChildLayoutItem = iterator.GetCurrent();
      while (pChildLayoutItem) {
        CXFA_ContentLayoutItem* pContentItem =
            pChildLayoutItem->AsContentLayoutItem();
        if (!pContentItem) {
          pChildLayoutItem = iterator.MoveToNext();
          continue;
        }

        XFA_AttributeValue presence =
            pContentItem->GetFormNode()
                ->JSObject()
                ->TryEnum(XFA_Attribute::Presence, true)
                .value_or(XFA_AttributeValue::Visible);
        bool bVisible = presence == XFA_AttributeValue::Visible;
        uint32_t dwRelevantChild =
            GetRelevant(pContentItem->GetFormNode(), dwRelevant);
        SyncContainer(pNotify, m_pLayoutProcessor, pContentItem,
                      dwRelevantChild, bVisible, nPageIdx);
        pChildLayoutItem = iterator.SkipChildrenAndMoveToNext();
      }
    }
  }

  int32_t nPage = pdfium::CollectionSize<int32_t>(m_PageArray);
  for (int32_t i = nPage - 1; i >= m_nAvailPages; i--) {
    RetainPtr<CXFA_ViewLayoutItem> pPage = m_PageArray[i];
    m_PageArray.erase(m_PageArray.begin() + i);
    pNotify->OnPageEvent(pPage.Get(), XFA_PAGEVIEWEVENT_PostRemoved);
  }
  ClearData();
}

void CXFA_ViewLayoutProcessor::PrepareLayout() {
  m_pPageSetCurRoot = nullptr;
  m_ePageSetMode = XFA_AttributeValue::OrderedOccurrence;
  m_nAvailPages = 0;
  ClearData();
  if (!m_pPageSetLayoutItemRoot)
    return;

  RetainPtr<CXFA_ViewLayoutItem> pRootLayoutItem = m_pPageSetLayoutItemRoot;
  if (pRootLayoutItem &&
      pRootLayoutItem->GetFormNode()->GetPacketType() == XFA_PacketType::Form) {
    CXFA_Node* pPageSetFormNode = pRootLayoutItem->GetFormNode();
    pRootLayoutItem->GetFormNode()->GetDocument()->m_pPendingPageSet.clear();
    if (pPageSetFormNode->HasRemovedChildren()) {
      XFA_ReleaseLayoutItem(pRootLayoutItem);
      m_pPageSetLayoutItemRoot = nullptr;
      pRootLayoutItem = nullptr;
      pPageSetFormNode = nullptr;
      m_PageArray.clear();
    }
    while (pPageSetFormNode) {
      CXFA_Node* pNextPageSet =
          pPageSetFormNode->GetNextSameClassSibling<CXFA_PageSet>(
              XFA_Element::PageSet);
      pPageSetFormNode->GetParent()->RemoveChildAndNotify(pPageSetFormNode,
                                                          false);
      pRootLayoutItem->GetFormNode()
          ->GetDocument()
          ->m_pPendingPageSet.push_back(pPageSetFormNode);
      pPageSetFormNode = pNextPageSet;
    }
  }
  pRootLayoutItem = m_pPageSetLayoutItemRoot;
  CXFA_ViewLayoutItem* pNextLayout = nullptr;
  for (; pRootLayoutItem; pRootLayoutItem.Reset(pNextLayout)) {
    pNextLayout = ToViewLayoutItem(pRootLayoutItem->GetNextSibling());
    SaveLayoutItemChildren(pRootLayoutItem.Get());
    pRootLayoutItem->RemoveSelfIfParented();
  }
  m_pPageSetLayoutItemRoot = nullptr;
}

void CXFA_ViewLayoutProcessor::ProcessSimplexOrDuplexPageSets(
    CXFA_ViewLayoutItem* pPageSetLayoutItem,
    bool bIsSimplex) {
  size_t nPageAreaCount;
  CXFA_LayoutItem* pLastPageAreaLayoutItem;
  std::tie(nPageAreaCount, pLastPageAreaLayoutItem) =
      GetPageAreaCountAndLastPageAreaFromPageSet(pPageSetLayoutItem);
  if (!pLastPageAreaLayoutItem)
    return;

  if (!FindPageAreaFromPageSet_SimplexDuplex(
          pPageSetLayoutItem->GetFormNode(), nullptr, nullptr, nullptr, true,
          true,
          nPageAreaCount == 1 ? XFA_AttributeValue::Only
                              : XFA_AttributeValue::Last) &&
      (nPageAreaCount == 1 &&
       !FindPageAreaFromPageSet_SimplexDuplex(
           pPageSetLayoutItem->GetFormNode(), nullptr, nullptr, nullptr, true,
           true, XFA_AttributeValue::Last))) {
    return;
  }

  CXFA_Node* pNode = m_pCurPageArea;
  XFA_AttributeValue eCurChoice =
      pNode->JSObject()->GetEnum(XFA_Attribute::PagePosition);
  if (eCurChoice == XFA_AttributeValue::Last) {
    XFA_AttributeValue eOddOrEven =
        pNode->JSObject()->GetEnum(XFA_Attribute::OddOrEven);
    XFA_AttributeValue eLastChoice =
        pLastPageAreaLayoutItem->GetFormNode()->JSObject()->GetEnum(
            XFA_Attribute::PagePosition);
    if (eLastChoice == XFA_AttributeValue::First &&
        (bIsSimplex || eOddOrEven != XFA_AttributeValue::Odd)) {
      CXFA_ViewRecord* pRecord = CreateViewRecordSimple();
      AddPageAreaLayoutItem(pRecord, pNode);
      return;
    }
  }

  std::vector<float> rgUsedHeights =
      GetHeightsForContentAreas(pLastPageAreaLayoutItem);
  if (ContentAreasFitInPageAreas(pNode, rgUsedHeights)) {
    CXFA_LayoutItem* pChildLayoutItem =
        pLastPageAreaLayoutItem->GetFirstChild();
    CXFA_Node* pContentAreaNode = pNode->GetFirstChild();
    pLastPageAreaLayoutItem->SetFormNode(pNode);
    while (pChildLayoutItem && pContentAreaNode) {
      if (pChildLayoutItem->GetFormNode()->GetElementType() !=
          XFA_Element::ContentArea) {
        pChildLayoutItem = pChildLayoutItem->GetNextSibling();
        continue;
      }
      if (pContentAreaNode->GetElementType() != XFA_Element::ContentArea) {
        pContentAreaNode = pContentAreaNode->GetNextSibling();
        continue;
      }
      pChildLayoutItem->SetFormNode(pContentAreaNode);
      pChildLayoutItem = pChildLayoutItem->GetNextSibling();
      pContentAreaNode = pContentAreaNode->GetNextSibling();
    }
    return;
  }

  if (pNode->JSObject()->GetEnum(XFA_Attribute::PagePosition) ==
      XFA_AttributeValue::Last) {
    CXFA_ViewRecord* pRecord = CreateViewRecordSimple();
    AddPageAreaLayoutItem(pRecord, pNode);
  }
}
