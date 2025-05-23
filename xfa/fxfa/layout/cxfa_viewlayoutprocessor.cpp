// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_viewlayoutprocessor.h"

#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/stl_util.h"
#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cjx_object.h"
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
    if (pLayoutItem->GetFormNode()->GetElementType() != XFA_Element::PageSet) {
      return nullptr;
    }

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

Mask<XFA_WidgetStatus> GetRelevant(CXFA_Node* pFormItem,
                                   Mask<XFA_WidgetStatus> dwParentRelvant) {
  Mask<XFA_WidgetStatus> dwRelevant = {XFA_WidgetStatus::kViewable,
                                       XFA_WidgetStatus::kPrintable};
  WideString wsRelevant =
      pFormItem->JSObject()->GetCData(XFA_Attribute::Relevant);
  if (!wsRelevant.IsEmpty()) {
    if (wsRelevant.EqualsASCII("+print") || wsRelevant.EqualsASCII("print")) {
      dwRelevant.Clear(XFA_WidgetStatus::kViewable);
    } else if (wsRelevant.EqualsASCII("-print")) {
      dwRelevant.Clear(XFA_WidgetStatus::kPrintable);
    }
  }
  if (!(dwParentRelvant & XFA_WidgetStatus::kViewable) &&
      (dwRelevant != XFA_WidgetStatus::kViewable)) {
    dwRelevant.Clear(XFA_WidgetStatus::kViewable);
  }
  if (!(dwParentRelvant & XFA_WidgetStatus::kPrintable) &&
      (dwRelevant != XFA_WidgetStatus::kPrintable)) {
    dwRelevant.Clear(XFA_WidgetStatus::kPrintable);
  }
  return dwRelevant;
}

void SyncContainer(CXFA_FFNotify* pNotify,
                   CXFA_LayoutProcessor* pDocLayout,
                   CXFA_LayoutItem* pViewItem,
                   Mask<XFA_WidgetStatus> dwRelevant,
                   bool bVisible,
                   int32_t nPageIndex) {
  bool bVisibleItem = false;
  Mask<XFA_WidgetStatus> dwStatus;
  Mask<XFA_WidgetStatus> dwRelevantContainer;
  if (bVisible) {
    XFA_AttributeValue eAttributeValue =
        pViewItem->GetFormNode()
            ->JSObject()
            ->TryEnum(XFA_Attribute::Presence, true)
            .value_or(XFA_AttributeValue::Visible);
    if (eAttributeValue == XFA_AttributeValue::Visible) {
      bVisibleItem = true;
    }

    dwRelevantContainer = GetRelevant(pViewItem->GetFormNode(), dwRelevant);
    dwStatus = dwRelevantContainer;
    if (bVisibleItem) {
      dwStatus |= XFA_WidgetStatus::kVisible;
    }
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

CXFA_Node* ResolveBreakTarget(CXFA_Node* pPageSetRoot,
                              bool bNewExprStyle,
                              WideString* pTargetAll) {
  if (!pPageSetRoot) {
    return nullptr;
  }

  CXFA_Document* document = pPageSetRoot->GetDocument();
  if (pTargetAll->IsEmpty()) {
    return nullptr;
  }

  pTargetAll->TrimWhitespace();
  size_t iSplitIndex = 0;
  bool bTargetAllFind = true;
  while (true) {
    WideString wsExpr;
    std::optional<size_t> iSplitNextIndex = 0;
    if (!bTargetAllFind) {
      iSplitNextIndex = pTargetAll->Find(' ', iSplitIndex);
      if (!iSplitNextIndex.has_value()) {
        return nullptr;
      }
      wsExpr = pTargetAll->Substr(iSplitIndex,
                                  iSplitNextIndex.value() - iSplitIndex);
    } else {
      wsExpr = *pTargetAll;
    }
    if (wsExpr.IsEmpty()) {
      return nullptr;
    }

    bTargetAllFind = false;
    if (wsExpr[0] == '#') {
      CXFA_Node* pNode = document->GetNodeByID(
          ToNode(document->GetXFAObject(XFA_HASHCODE_Template)),
          wsExpr.Last(wsExpr.GetLength() - 1).AsStringView());
      if (pNode) {
        return pNode;
      }
    } else if (bNewExprStyle) {
      WideString wsProcessedTarget = wsExpr;
      if (wsExpr.First(4).EqualsASCII("som(") && wsExpr.Back() == L')') {
        wsProcessedTarget = wsExpr.Substr(4, wsExpr.GetLength() - 5);
      }

      std::optional<CFXJSE_Engine::ResolveResult> maybeResult =
          document->GetScriptContext()->ResolveObjects(
              pPageSetRoot, wsProcessedTarget.AsStringView(),
              Mask<XFA_ResolveFlag>{
                  XFA_ResolveFlag::kChildren, XFA_ResolveFlag::kProperties,
                  XFA_ResolveFlag::kAttributes, XFA_ResolveFlag::kSiblings,
                  XFA_ResolveFlag::kParent});
      if (maybeResult.has_value() &&
          maybeResult.value().objects.front()->IsNode()) {
        return maybeResult.value().objects.front()->AsNode();
      }
    }
    iSplitIndex = iSplitNextIndex.value();
  }
}

void SetLayoutGeneratedNodeFlag(CXFA_Node* pNode) {
  pNode->SetFlag(XFA_NodeFlag::kLayoutGeneratedNode);
  pNode->ClearFlag(XFA_NodeFlag::kUnusedNode);
}

// Note: Returning nullptr is not the same as returning std::nullopt.
std::optional<CXFA_ViewLayoutItem*> CheckContentAreaNotUsed(
    CXFA_ViewLayoutItem* pPageAreaLayoutItem,
    CXFA_Node* pContentArea) {
  for (CXFA_LayoutItem* pChild = pPageAreaLayoutItem->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    CXFA_ViewLayoutItem* pLayoutItem = pChild->AsViewLayoutItem();
    if (pLayoutItem && pLayoutItem->GetFormNode() == pContentArea) {
      if (!pLayoutItem->GetFirstChild()) {
        return pLayoutItem;
      }
      return std::nullopt;
    }
  }
  return nullptr;
}

void SyncRemoveLayoutItem(CXFA_LayoutItem* pLayoutItem,
                          CXFA_FFNotify* pNotify,
                          CXFA_LayoutProcessor* pDocLayout) {
  CXFA_LayoutItem* pCurLayoutItem = pLayoutItem->GetFirstChild();
  while (pCurLayoutItem) {
    CXFA_LayoutItem* pNextLayoutItem = pCurLayoutItem->GetNextSibling();
    SyncRemoveLayoutItem(pCurLayoutItem, pNotify, pDocLayout);
    pCurLayoutItem = pNextLayoutItem;
  }
  pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem);
  pLayoutItem->RemoveSelfIfParented();
}

bool RunBreakTestScript(CXFA_Script* pTestScript) {
  WideString wsExpression = pTestScript->JSObject()->GetContent(false);
  if (wsExpression.IsEmpty()) {
    return true;
  }
  return pTestScript->GetDocument()->GetNotify()->RunScript(
      pTestScript, pTestScript->GetContainerParent());
}

float CalculateLayoutItemHeight(const CXFA_LayoutItem* pItem) {
  float fHeight = 0;
  for (const CXFA_LayoutItem* pChild = pItem->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    const CXFA_ContentLayoutItem* pContent = pChild->AsContentLayoutItem();
    if (pContent) {
      fHeight += pContent->s_size_.height;
    }
  }
  return fHeight;
}

std::vector<float> GetHeightsForContentAreas(const CXFA_LayoutItem* pItem) {
  std::vector<float> heights;
  for (const CXFA_LayoutItem* pChild = pItem->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pChild->GetFormNode()->GetElementType() == XFA_Element::ContentArea) {
      heights.push_back(CalculateLayoutItemHeight(pChild));
    }
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
    if (type != XFA_Element::PageArea) {
      continue;
    }

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
    if (pContentAreaNode->GetElementType() != XFA_Element::ContentArea) {
      continue;
    }

    if (iCurContentAreaIndex >= rgUsedHeights.size()) {
      return false;
    }

    const float fHeight = pContentAreaNode->JSObject()->GetMeasureInUnit(
                              XFA_Attribute::H, XFA_Unit::Pt) +
                          kXFALayoutPrecision;
    if (rgUsedHeights[iCurContentAreaIndex] > fHeight) {
      return false;
    }

    ++iCurContentAreaIndex;
  }
  return true;
}

}  // namespace

CXFA_ViewLayoutProcessor::CXFA_ViewRecord::CXFA_ViewRecord() = default;

CXFA_ViewLayoutProcessor::CXFA_ViewRecord::~CXFA_ViewRecord() = default;

void CXFA_ViewLayoutProcessor::CXFA_ViewRecord::Trace(
    cppgc::Visitor* visitor) const {
  visitor->Trace(pCurPageSet);
  visitor->Trace(pCurPageArea);
  visitor->Trace(pCurContentArea);
}

CXFA_ViewLayoutProcessor::CXFA_ViewLayoutProcessor(
    cppgc::Heap* pHeap,
    CXFA_LayoutProcessor* pLayoutProcessor)
    : heap_(pHeap),
      layout_processor_(pLayoutProcessor),
      current_view_record_iter_(proposed_view_records_.end()) {}

CXFA_ViewLayoutProcessor::~CXFA_ViewLayoutProcessor() = default;

void CXFA_ViewLayoutProcessor::PreFinalize() {
  ClearData();
  CXFA_LayoutItem* pLayoutItem = GetRootLayoutItem();
  while (pLayoutItem) {
    CXFA_LayoutItem* pNextLayout = pLayoutItem->GetNextSibling();
    XFA_ReleaseLayoutItem(pLayoutItem);
    pLayoutItem = pNextLayout;
  }
}

void CXFA_ViewLayoutProcessor::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(layout_processor_);
  visitor->Trace(page_set_node_);
  visitor->Trace(cur_page_area_);
  visitor->Trace(page_set_root_layout_item_);
  visitor->Trace(page_set_cur_layout_item_);
  ContainerTrace(visitor, proposed_view_records_);

  if (current_view_record_iter_ != proposed_view_records_.end()) {
    visitor->Trace(*current_view_record_iter_);
  }

  ContainerTrace(visitor, page_array_);
  ContainerTrace(visitor, page_set_map_);
}

bool CXFA_ViewLayoutProcessor::InitLayoutPage(CXFA_Node* pFormNode) {
  PrepareLayout();
  CXFA_Node* pTemplateNode = pFormNode->GetTemplateNodeIfExists();
  if (!pTemplateNode) {
    return false;
  }

  page_set_node_ = pTemplateNode->JSObject()->GetOrCreateProperty<CXFA_PageSet>(
      0, XFA_Element::PageSet);
  DCHECK(page_set_node_);

  if (page_set_root_layout_item_) {
    page_set_root_layout_item_->RemoveSelfIfParented();
  } else {
    page_set_root_layout_item_ =
        cppgc::MakeGarbageCollected<CXFA_ViewLayoutItem>(
            GetHeap()->GetAllocationHandle(), page_set_node_, nullptr);
  }
  page_set_cur_layout_item_ = page_set_root_layout_item_;
  page_set_node_->JSObject()->SetLayoutItem(page_set_root_layout_item_.Get());

  XFA_AttributeValue eRelation =
      page_set_node_->JSObject()->GetEnum(XFA_Attribute::Relation);
  if (eRelation != XFA_AttributeValue::Unknown) {
    page_set_mode_ = eRelation;
  }

  InitPageSetMap();
  CXFA_Node* pPageArea = nullptr;
  int32_t iCount = 0;
  for (pPageArea = page_set_node_->GetFirstChild(); pPageArea;
       pPageArea = pPageArea->GetNextSibling()) {
    if (pPageArea->GetElementType() != XFA_Element::PageArea) {
      continue;
    }

    iCount++;
    if (pPageArea->GetFirstChildByClass<CXFA_ContentArea>(
            XFA_Element::ContentArea)) {
      return true;
    }
  }
  if (iCount > 0) {
    return false;
  }

  CXFA_Document* document = pTemplateNode->GetDocument();
  pPageArea =
      page_set_node_->GetChild<CXFA_Node>(0, XFA_Element::PageArea, false);
  if (!pPageArea) {
    pPageArea = document->CreateNode(page_set_node_->GetPacketType(),
                                     XFA_Element::PageArea);
    if (!pPageArea) {
      return false;
    }

    page_set_node_->InsertChildAndNotify(pPageArea, nullptr);
    pPageArea->SetInitializedFlagAndNotify();
  }
  CXFA_ContentArea* pContentArea =
      pPageArea->GetChild<CXFA_ContentArea>(0, XFA_Element::ContentArea, false);
  if (!pContentArea) {
    pContentArea = static_cast<CXFA_ContentArea*>(document->CreateNode(
        pPageArea->GetPacketType(), XFA_Element::ContentArea));
    if (!pContentArea) {
      return false;
    }

    pPageArea->InsertChildAndNotify(pContentArea, nullptr);
    pContentArea->SetInitializedFlagAndNotify();
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
        document->CreateNode(pPageArea->GetPacketType(), XFA_Element::Medium));
    if (!pContentArea) {
      return false;
    }

    pPageArea->InsertChildAndNotify(pMedium, nullptr);
    pMedium->SetInitializedFlagAndNotify();
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
    if (bProBreakBefore) {
      break;
    }

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
  if (current_view_record_iter_ != GetTailPosition()) {
    return true;
  }

  CXFA_Node* pPageNode = GetNextAvailPageArea(nullptr, nullptr, false, false);
  if (!pPageNode) {
    return false;
  }

  if (bFirstTemPage && !HasCurrentViewRecord()) {
    ResetToFirstViewRecord();
  }
  return !bFirstTemPage || HasCurrentViewRecord();
}

void CXFA_ViewLayoutProcessor::RemoveLayoutRecord(
    CXFA_ViewRecord* pNewRecord,
    CXFA_ViewRecord* pPrevRecord) {
  if (!pNewRecord || !pPrevRecord) {
    return;
  }
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

void CXFA_ViewLayoutProcessor::SubmitContentItem(
    CXFA_ContentLayoutItem* pContentLayoutItem,
    CXFA_ContentLayoutProcessor::Result eStatus) {
  if (pContentLayoutItem) {
    CXFA_ViewRecord* pViewRecord = GetCurrentViewRecord();
    if (!pViewRecord) {
      return;
    }

    pViewRecord->pCurContentArea->AppendLastChild(pContentLayoutItem);
    create_over_flow_page_ = false;
  }
  if (eStatus != CXFA_ContentLayoutProcessor::Result::kDone) {
    if (eStatus == CXFA_ContentLayoutProcessor::Result::kPageFullBreak &&
        current_view_record_iter_ == GetTailPosition()) {
      AppendNewPage(false);
    }
    current_view_record_iter_ = GetTailPosition();
    cur_page_area_ = GetCurrentViewRecord()->pCurPageArea->GetFormNode();
  }
}

float CXFA_ViewLayoutProcessor::GetAvailHeight() {
  CXFA_ViewRecord* pViewRecord = GetCurrentViewRecord();
  if (!pViewRecord) {
    return 0.0f;
  }

  CXFA_ViewLayoutItem* pLayoutItem = pViewRecord->pCurContentArea;
  if (!pLayoutItem || !pLayoutItem->GetFormNode()) {
    return 0.0f;
  }

  float fAvailHeight = pLayoutItem->GetFormNode()->JSObject()->GetMeasureInUnit(
      XFA_Attribute::H, XFA_Unit::Pt);
  if (fAvailHeight >= kXFALayoutPrecision) {
    return fAvailHeight;
  }
  if (current_view_record_iter_ == proposed_view_records_.begin()) {
    return 0.0f;
  }
  return FLT_MAX;
}

void CXFA_ViewLayoutProcessor::AppendNewRecord(CXFA_ViewRecord* pNewRecord) {
  proposed_view_records_.emplace_back(pNewRecord);
}

CXFA_ViewLayoutProcessor::CXFA_ViewRecord*
CXFA_ViewLayoutProcessor::CreateViewRecord(CXFA_Node* pPageNode,
                                           bool bCreateNew) {
  DCHECK(pPageNode);
  auto* pNewRecord = cppgc::MakeGarbageCollected<CXFA_ViewRecord>(
      GetHeap()->GetAllocationHandle());
  if (!HasCurrentViewRecord()) {
    CXFA_Node* pPageSet = pPageNode->GetParent();
    if (pPageSet == page_set_node_) {
      pNewRecord->pCurPageSet = page_set_root_layout_item_;
    } else {
      auto* pPageSetLayoutItem =
          cppgc::MakeGarbageCollected<CXFA_ViewLayoutItem>(
              GetHeap()->GetAllocationHandle(), pPageSet, nullptr);
      pPageSet->JSObject()->SetLayoutItem(pPageSetLayoutItem);
      page_set_root_layout_item_->AppendLastChild(pPageSetLayoutItem);
      pNewRecord->pCurPageSet = pPageSetLayoutItem;
    }
    AppendNewRecord(pNewRecord);
    return pNewRecord;
  }

  if (!IsPageSetRootOrderedOccurrence()) {
    *pNewRecord = *GetCurrentViewRecord();
    AppendNewRecord(pNewRecord);
    return pNewRecord;
  }

  CXFA_Node* pPageSet = pPageNode->GetParent();
  if (!bCreateNew) {
    if (pPageSet == page_set_node_) {
      pNewRecord->pCurPageSet = page_set_cur_layout_item_;
    } else {
      CXFA_ViewLayoutItem* pParentLayoutItem =
          ToViewLayoutItem(pPageSet->JSObject()->GetLayoutItem());
      if (!pParentLayoutItem) {
        pParentLayoutItem = page_set_cur_layout_item_;
      }
      pNewRecord->pCurPageSet = pParentLayoutItem;
    }
    AppendNewRecord(pNewRecord);
    return pNewRecord;
  }

  CXFA_ViewLayoutItem* pParentPageSetLayout = nullptr;
  if (pPageSet == GetCurrentViewRecord()->pCurPageSet->GetFormNode()) {
    pParentPageSetLayout =
        ToViewLayoutItem(GetCurrentViewRecord()->pCurPageSet->GetParent());
  } else {
    pParentPageSetLayout =
        ToViewLayoutItem(pPageSet->GetParent()->JSObject()->GetLayoutItem());
  }
  auto* pPageSetLayoutItem = cppgc::MakeGarbageCollected<CXFA_ViewLayoutItem>(
      GetHeap()->GetAllocationHandle(), pPageSet, nullptr);
  pPageSet->JSObject()->SetLayoutItem(pPageSetLayoutItem);
  if (!pParentPageSetLayout) {
    CXFA_ViewLayoutItem* pPrePageSet = page_set_root_layout_item_;
    while (pPrePageSet->GetNextSibling()) {
      pPrePageSet = pPrePageSet->GetNextSibling()->AsViewLayoutItem();
    }

    if (pPrePageSet->GetParent()) {
      pPrePageSet->GetParent()->InsertAfter(pPageSetLayoutItem, pPrePageSet);
    }
    page_set_cur_layout_item_ = pPageSetLayoutItem;
  } else {
    pParentPageSetLayout->AppendLastChild(pPageSetLayoutItem);
  }
  pNewRecord->pCurPageSet = pPageSetLayoutItem;
  AppendNewRecord(pNewRecord);
  return pNewRecord;
}

CXFA_ViewLayoutProcessor::CXFA_ViewRecord*
CXFA_ViewLayoutProcessor::CreateViewRecordSimple() {
  auto* pNewRecord = cppgc::MakeGarbageCollected<CXFA_ViewRecord>(
      GetHeap()->GetAllocationHandle());
  CXFA_ViewRecord* pCurrentRecord = GetCurrentViewRecord();
  if (pCurrentRecord) {
    *pNewRecord = *pCurrentRecord;
  } else {
    pNewRecord->pCurPageSet = page_set_root_layout_item_;
  }
  AppendNewRecord(pNewRecord);
  return pNewRecord;
}

void CXFA_ViewLayoutProcessor::AddPageAreaLayoutItem(
    CXFA_ViewRecord* pNewRecord,
    CXFA_Node* pNewPageArea) {
  CXFA_ViewLayoutItem* pNewPageAreaLayoutItem = nullptr;
  if (fxcrt::IndexInBounds(page_array_, avail_pages_)) {
    CXFA_ViewLayoutItem* pViewItem = page_array_[avail_pages_];
    pViewItem->SetFormNode(pNewPageArea);
    avail_pages_++;
    pNewPageAreaLayoutItem = pViewItem;
  } else {
    CXFA_FFNotify* pNotify = pNewPageArea->GetDocument()->GetNotify();
    auto* pViewItem = cppgc::MakeGarbageCollected<CXFA_ViewLayoutItem>(
        GetHeap()->GetAllocationHandle(), pNewPageArea,
        pNotify->OnCreateViewLayoutItem(pNewPageArea));
    page_array_.push_back(pViewItem);
    avail_pages_++;
    pNotify->OnPageViewEvent(pViewItem,
                             CXFA_FFDoc::PageViewEvent::kPostRemoved);
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
  auto* pNewViewLayoutItem = cppgc::MakeGarbageCollected<CXFA_ViewLayoutItem>(
      GetHeap()->GetAllocationHandle(), pContentArea, nullptr);
  pNewRecord->pCurPageArea->AppendLastChild(pNewViewLayoutItem);
  pNewRecord->pCurContentArea = pNewViewLayoutItem;
}

void CXFA_ViewLayoutProcessor::FinishPaginatedPageSets() {
  for (CXFA_ViewLayoutItem* pRootPageSetLayoutItem =
           page_set_root_layout_item_.Get();
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
  return fxcrt::CollectionSize<int32_t>(page_array_);
}

CXFA_ViewLayoutItem* CXFA_ViewLayoutProcessor::GetPage(int32_t index) const {
  if (!fxcrt::IndexInBounds(page_array_, index)) {
    return nullptr;
  }
  return page_array_[index].Get();
}

int32_t CXFA_ViewLayoutProcessor::GetPageIndex(
    const CXFA_ViewLayoutItem* pPage) const {
  auto it = std::ranges::find(page_array_, pPage);
  return it != page_array_.end()
             ? pdfium::checked_cast<int32_t>(it - page_array_.begin())
             : -1;
}

bool CXFA_ViewLayoutProcessor::RunBreak(XFA_Element eBreakType,
                                        XFA_AttributeValue eTargetType,
                                        CXFA_Node* pTarget,
                                        bool bStartNew) {
  bool bRet = false;
  switch (eTargetType) {
    case XFA_AttributeValue::ContentArea:
      if (pTarget && pTarget->GetElementType() != XFA_Element::ContentArea) {
        pTarget = nullptr;
      }
      if (ShouldGetNextPageArea(pTarget, bStartNew)) {
        CXFA_Node* pPageArea = nullptr;
        if (pTarget) {
          pPageArea = pTarget->GetParent();
        }

        pPageArea = GetNextAvailPageArea(pPageArea, pTarget, false, false);
        bRet = !!pPageArea;
      }
      break;
    case XFA_AttributeValue::PageArea:
      if (pTarget && pTarget->GetElementType() != XFA_Element::PageArea) {
        pTarget = nullptr;
      }
      if (ShouldGetNextPageArea(pTarget, bStartNew)) {
        CXFA_Node* pPageArea =
            GetNextAvailPageArea(pTarget, nullptr, true, false);
        bRet = !!pPageArea;
      }
      break;
    case XFA_AttributeValue::PageOdd:
      if (pTarget && pTarget->GetElementType() != XFA_Element::PageArea) {
        pTarget = nullptr;
      }
      break;
    case XFA_AttributeValue::PageEven:
      if (pTarget && pTarget->GetElementType() != XFA_Element::PageArea) {
        pTarget = nullptr;
      }
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
      if (pScript && !RunBreakTestScript(pScript)) {
        break;
      }

      WideString wsTarget =
          pCurNode->JSObject()->GetCData(XFA_Attribute::Target);
      CXFA_Node* pTarget = ResolveBreakTarget(page_set_node_, true, &wsTarget);
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
      if (!proposed_view_records_.empty() &&
          current_view_record_iter_ == proposed_view_records_.begin() &&
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
      CXFA_Node* pTarget = ResolveBreakTarget(page_set_node_, true, &wsTarget);
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

std::optional<CXFA_ViewLayoutProcessor::BreakData>
CXFA_ViewLayoutProcessor::ProcessBreakBefore(const CXFA_Node* pBreakNode) {
  return ProcessBreakBeforeOrAfter(pBreakNode, /*bBefore=*/true);
}

std::optional<CXFA_ViewLayoutProcessor::BreakData>
CXFA_ViewLayoutProcessor::ProcessBreakAfter(const CXFA_Node* pBreakNode) {
  return ProcessBreakBeforeOrAfter(pBreakNode, /*bBefore=*/false);
}

std::optional<CXFA_ViewLayoutProcessor::BreakData>
CXFA_ViewLayoutProcessor::ProcessBreakBeforeOrAfter(const CXFA_Node* pBreakNode,
                                                    bool bBefore) {
  CXFA_Node* pFormNode = pBreakNode->GetContainerParent();
  if (!pFormNode->PresenceRequiresSpace()) {
    return std::nullopt;
  }

  BreakData break_data = ExecuteBreakBeforeOrAfter(pBreakNode, bBefore);
  CXFA_Document* document = pBreakNode->GetDocument();
  CXFA_Node* pDataScope = nullptr;
  pFormNode = pFormNode->GetContainerParent();
  if (break_data.pLeader) {
    if (!break_data.pLeader->IsContainerNode()) {
      return std::nullopt;
    }

    pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
    break_data.pLeader = document->DataMerge_CopyContainer(
        break_data.pLeader, pFormNode, pDataScope, true, true, true);
    if (!break_data.pLeader) {
      return std::nullopt;
    }

    document->DataMerge_UpdateBindingRelations(break_data.pLeader);
    SetLayoutGeneratedNodeFlag(break_data.pLeader);
  }
  if (break_data.pTrailer) {
    if (!break_data.pTrailer->IsContainerNode()) {
      return std::nullopt;
    }

    if (!pDataScope) {
      pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
    }

    break_data.pTrailer = document->DataMerge_CopyContainer(
        break_data.pTrailer, pFormNode, pDataScope, true, true, true);
    if (!break_data.pTrailer) {
      return std::nullopt;
    }

    document->DataMerge_UpdateBindingRelations(break_data.pTrailer);
    SetLayoutGeneratedNodeFlag(break_data.pTrailer);
  }
  return break_data;
}

CXFA_Node* CXFA_ViewLayoutProcessor::ProcessBookendLeader(
    const CXFA_Node* pBookendNode) {
  return ProcessBookendLeaderOrTrailer(pBookendNode, /*bLeader=*/true);
}

CXFA_Node* CXFA_ViewLayoutProcessor::ProcessBookendTrailer(
    const CXFA_Node* pBookendNode) {
  return ProcessBookendLeaderOrTrailer(pBookendNode, /*bLeader=*/false);
}

CXFA_Node* CXFA_ViewLayoutProcessor::ProcessBookendLeaderOrTrailer(
    const CXFA_Node* pBookendNode,
    bool bLeader) {
  CXFA_Node* pFormNode = pBookendNode->GetContainerParent();
  CXFA_Node* pLeaderTemplate =
      ResolveBookendLeaderOrTrailer(pBookendNode, bLeader);
  if (!pLeaderTemplate || !pLeaderTemplate->IsContainerNode()) {
    return nullptr;
  }

  CXFA_Document* document = pBookendNode->GetDocument();
  CXFA_Node* pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
  CXFA_Node* pBookendAppendNode = document->DataMerge_CopyContainer(
      pLeaderTemplate, pFormNode, pDataScope, true, true, true);
  if (!pBookendAppendNode) {
    return nullptr;
  }

  document->DataMerge_UpdateBindingRelations(pBookendAppendNode);
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

    if (!wsOverflowTarget.IsEmpty() && bCreatePage && !create_over_flow_page_) {
      CXFA_Node* pTarget =
          ResolveBreakTarget(page_set_node_, true, &wsOverflowTarget);
      if (pTarget) {
        create_over_flow_page_ = true;
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

  if (pOverflowNode->GetElementType() != XFA_Element::Overflow) {
    return false;
  }

  WideString wsOverflowTarget =
      pOverflowNode->JSObject()->GetCData(XFA_Attribute::Target);
  if (!wsOverflowTarget.IsEmpty() && bCreatePage && !create_over_flow_page_) {
    CXFA_Node* pTarget =
        ResolveBreakTarget(page_set_node_, true, &wsOverflowTarget);
    if (pTarget) {
      create_over_flow_page_ = true;
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

std::optional<CXFA_ViewLayoutProcessor::OverflowData>
CXFA_ViewLayoutProcessor::ProcessOverflow(CXFA_Node* pFormNode,
                                          bool bCreatePage) {
  if (!pFormNode) {
    return std::nullopt;
  }

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
      if (bIsOverflowNode) {
        pFormNode = pCurNode->GetParent();
      }

      CXFA_Document* document = pCurNode->GetDocument();
      CXFA_Node* pDataScope = nullptr;
      if (pLeaderTemplate) {
        pDataScope = XFA_DataMerge_FindDataScope(pFormNode);

        overflow_data.pLeader = document->DataMerge_CopyContainer(
            pLeaderTemplate, pFormNode, pDataScope, true, true, true);
        if (!overflow_data.pLeader) {
          return std::nullopt;
        }

        document->DataMerge_UpdateBindingRelations(overflow_data.pLeader);
        SetLayoutGeneratedNodeFlag(overflow_data.pLeader);
      }
      if (pTrailerTemplate) {
        if (!pDataScope) {
          pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
        }

        overflow_data.pTrailer = document->DataMerge_CopyContainer(
            pTrailerTemplate, pFormNode, pDataScope, true, true, true);
        if (!overflow_data.pTrailer) {
          return std::nullopt;
        }

        document->DataMerge_UpdateBindingRelations(overflow_data.pTrailer);
        SetLayoutGeneratedNodeFlag(overflow_data.pTrailer);
      }
      return overflow_data;
    }
    if (bIsOverflowNode) {
      break;
    }
  }
  return std::nullopt;
}

CXFA_Node* CXFA_ViewLayoutProcessor::ResolveBookendLeaderOrTrailer(
    const CXFA_Node* pBookendNode,
    bool bLeader) {
  CXFA_Node* pContainer =
      pBookendNode->GetContainerParent()->GetTemplateNodeIfExists();
  if (pBookendNode->GetElementType() == XFA_Element::Break) {
    WideString leader = pBookendNode->JSObject()->GetCData(
        bLeader ? XFA_Attribute::BookendLeader : XFA_Attribute::BookendTrailer);
    if (leader.IsEmpty()) {
      return nullptr;
    }
    return ResolveBreakTarget(pContainer, false, &leader);
  }

  if (pBookendNode->GetElementType() != XFA_Element::Bookend) {
    return nullptr;
  }

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
  if (!pPageSet && !pStartChild) {
    return false;
  }

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
    auto it = page_set_map_.find(pPageSet);
    if (it != page_set_map_.end()) {
      iPageSetCount = it->second;
    }
    int32_t iMax = -1;
    CXFA_Node* pOccurNode =
        pPageSet->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
    if (pOccurNode) {
      std::optional<int32_t> ret =
          pOccurNode->JSObject()->TryInteger(XFA_Attribute::Max, false);
      if (ret.has_value()) {
        iMax = ret.value();
      }
    }
    if (iMax >= 0 && iMax <= iPageSetCount) {
      return false;
    }
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
        cur_page_area_ = pCurrentNode;
        cur_page_count_ = 1;
        bRes = true;
        break;
      }
      if (!bQuery) {
        CreateMinPageRecord(pCurrentNode, false, false);
      }
    } else if (pCurrentNode->GetElementType() == XFA_Element::PageSet) {
      if (FindPageAreaFromPageSet_Ordered(pCurrentNode, nullptr,
                                          pTargetPageArea, pTargetContentArea,
                                          bNewPage, bQuery)) {
        bRes = true;
        break;
      }
      if (!bQuery) {
        CreateMinPageSetRecord(pCurrentNode, true);
      }
    }
  }
  if (!pStartChild && bRes && !bQuery) {
    page_set_map_[pPageSet] = ++iPageSetCount;
  }
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
  if (!pStartChild || pStartChild->GetElementType() == XFA_Element::PageArea) {
    pCurrentNode = pPageSet->GetFirstChild();
  } else {
    pCurrentNode = pStartChild->GetNextSibling();
  }

  for (; pCurrentNode; pCurrentNode = pCurrentNode->GetNextSibling()) {
    if (pCurrentNode->GetElementType() == XFA_Element::PageArea) {
      if (!MatchPageAreaOddOrEven(pCurrentNode)) {
        continue;
      }

      XFA_AttributeValue eCurPagePosition =
          pCurrentNode->JSObject()->GetEnum(XFA_Attribute::PagePosition);
      if (ePreferredPosition == XFA_AttributeValue::Last) {
        if (eCurPagePosition != ePreferredPosition) {
          continue;
        }
        if (page_set_mode_ == XFA_AttributeValue::SimplexPaginated ||
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
        if (eCurPagePosition != ePreferredPosition) {
          continue;
        }
        if (page_set_mode_ != XFA_AttributeValue::DuplexPaginated ||
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
  if (pPreferredPageArea) {
    pCurPageArea = pPreferredPageArea;
  } else if (pFallbackPageArea) {
    pCurPageArea = pFallbackPageArea;
  }

  if (!pCurPageArea) {
    return false;
  }

  if (!bQuery) {
    CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
    AddPageAreaLayoutItem(pNewRecord, pCurPageArea);
    if (!pTargetContentArea) {
      pTargetContentArea = pCurPageArea->GetFirstChildByClass<CXFA_ContentArea>(
          XFA_Element::ContentArea);
    }
    AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
  }
  cur_page_area_ = pCurPageArea;
  return true;
}

bool CXFA_ViewLayoutProcessor::MatchPageAreaOddOrEven(CXFA_Node* pPageArea) {
  if (page_set_mode_ != XFA_AttributeValue::DuplexPaginated) {
    return true;
  }

  std::optional<XFA_AttributeValue> ret =
      pPageArea->JSObject()->TryEnum(XFA_Attribute::OddOrEven, true);
  if (!ret.has_value() || ret == XFA_AttributeValue::Any) {
    return true;
  }

  int32_t iPageLast = GetPageCount() % 2;
  return ret == XFA_AttributeValue::Odd ? iPageLast == 0 : iPageLast == 1;
}

CXFA_Node* CXFA_ViewLayoutProcessor::GetNextAvailPageArea(
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    bool bNewPage,
    bool bQuery) {
  if (!cur_page_area_) {
    FindPageAreaFromPageSet(page_set_node_, nullptr, pTargetPageArea,
                            pTargetContentArea, bNewPage, bQuery);
    return cur_page_area_;
  }

  if (!pTargetPageArea || pTargetPageArea == cur_page_area_) {
    if (!bNewPage && GetNextContentArea(pTargetContentArea)) {
      return cur_page_area_;
    }

    if (IsPageSetRootOrderedOccurrence()) {
      int32_t iMax = -1;
      CXFA_Node* pOccurNode =
          cur_page_area_->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
      if (pOccurNode) {
        std::optional<int32_t> ret =
            pOccurNode->JSObject()->TryInteger(XFA_Attribute::Max, false);
        if (ret.has_value()) {
          iMax = ret.value();
        }
      }
      if ((iMax < 0 || cur_page_count_ < iMax)) {
        if (!bQuery) {
          CXFA_ViewRecord* pNewRecord = CreateViewRecord(cur_page_area_, false);
          AddPageAreaLayoutItem(pNewRecord, cur_page_area_);
          if (!pTargetContentArea) {
            pTargetContentArea =
                cur_page_area_->GetFirstChildByClass<CXFA_ContentArea>(
                    XFA_Element::ContentArea);
          }
          AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
        }
        cur_page_count_++;
        return cur_page_area_;
      }
    }
  }

  if (!bQuery && IsPageSetRootOrderedOccurrence()) {
    CreateMinPageRecord(cur_page_area_, false, true);
  }
  if (FindPageAreaFromPageSet(cur_page_area_->GetParent(), cur_page_area_,
                              pTargetPageArea, pTargetContentArea, bNewPage,
                              bQuery)) {
    return cur_page_area_;
  }

  CXFA_Node* pPageSet = cur_page_area_->GetParent();
  while (pPageSet) {
    if (FindPageAreaFromPageSet(pPageSet, nullptr, pTargetPageArea,
                                pTargetContentArea, bNewPage, bQuery)) {
      return cur_page_area_;
    }
    if (!bQuery && IsPageSetRootOrderedOccurrence()) {
      CreateMinPageSetRecord(pPageSet, false);
    }
    if (FindPageAreaFromPageSet(nullptr, pPageSet, pTargetPageArea,
                                pTargetContentArea, bNewPage, bQuery)) {
      return cur_page_area_;
    }
    if (pPageSet == page_set_node_) {
      break;
    }

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
    if (!pContentArea) {
      return false;
    }
  } else {
    if (pContentArea->GetParent() != cur_page_area_) {
      return false;
    }

    std::optional<CXFA_ViewLayoutItem*> pContentAreaLayout =
        CheckContentAreaNotUsed(GetCurrentViewRecord()->pCurPageArea.Get(),
                                pContentArea);
    if (!pContentAreaLayout.has_value()) {
      return false;
    }
    if (pContentAreaLayout.value()) {
      if (pContentAreaLayout.value()->GetFormNode() == pCurContentNode) {
        return false;
      }

      CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
      pNewRecord->pCurContentArea = pContentAreaLayout.value();
      return true;
    }
  }

  CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
  AddContentAreaLayoutItem(pNewRecord, pContentArea);
  return true;
}

void CXFA_ViewLayoutProcessor::InitPageSetMap() {
  if (!IsPageSetRootOrderedOccurrence()) {
    return;
  }

  CXFA_NodeIterator sIterator(page_set_node_);
  for (CXFA_Node* pPageSetNode = sIterator.GetCurrent(); pPageSetNode;
       pPageSetNode = sIterator.MoveToNext()) {
    if (pPageSetNode->GetElementType() == XFA_Element::PageSet) {
      XFA_AttributeValue eRelation =
          pPageSetNode->JSObject()->GetEnum(XFA_Attribute::Relation);
      if (eRelation == XFA_AttributeValue::OrderedOccurrence) {
        page_set_map_[pPageSetNode] = 0;
      }
    }
  }
}

int32_t CXFA_ViewLayoutProcessor::CreateMinPageRecord(CXFA_Node* pPageArea,
                                                      bool bTargetPageArea,
                                                      bool bCreateLast) {
  if (!pPageArea) {
    return 0;
  }

  int32_t iMin = 0;
  std::optional<int32_t> ret;
  CXFA_Node* pOccurNode =
      pPageArea->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
  if (pOccurNode) {
    ret = pOccurNode->JSObject()->TryInteger(XFA_Attribute::Min, false);
    if (ret.has_value()) {
      iMin = ret.value();
    }
  }

  if (!ret.has_value() && !bTargetPageArea) {
    return iMin;
  }

  CXFA_Node* pContentArea = pPageArea->GetFirstChildByClass<CXFA_ContentArea>(
      XFA_Element::ContentArea);
  if (iMin < 1 && bTargetPageArea && !pContentArea) {
    iMin = 1;
  }

  int32_t i = 0;
  if (bCreateLast) {
    i = cur_page_count_;
  }

  for (; i < iMin; i++) {
    CXFA_ViewRecord* pNewRecord = CreateViewRecordSimple();
    AddPageAreaLayoutItem(pNewRecord, pPageArea);
    AddContentAreaLayoutItem(pNewRecord, pContentArea);
  }
  return iMin;
}

void CXFA_ViewLayoutProcessor::CreateMinPageSetRecord(CXFA_Node* pPageSet,
                                                      bool bCreateAll) {
  auto it = page_set_map_.find(pPageSet);
  if (it == page_set_map_.end()) {
    return;
  }

  int32_t iCurSetCount = it->second;
  if (bCreateAll) {
    iCurSetCount = 0;
  }

  CXFA_Node* pOccurNode =
      pPageSet->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
  if (!pOccurNode) {
    return;
  }

  std::optional<int32_t> iMin =
      pOccurNode->JSObject()->TryInteger(XFA_Attribute::Min, false);
  if (!iMin.has_value() || iCurSetCount >= iMin.value()) {
    return;
  }

  for (int32_t i = 0; i < iMin.value() - iCurSetCount; i++) {
    for (CXFA_Node* node = pPageSet->GetFirstChild(); node;
         node = node->GetNextSibling()) {
      if (node->GetElementType() == XFA_Element::PageArea) {
        CreateMinPageRecord(node, false, false);
      } else if (node->GetElementType() == XFA_Element::PageSet) {
        CreateMinPageSetRecord(node, true);
      }
    }
  }
  page_set_map_[pPageSet] = iMin.value();
}

void CXFA_ViewLayoutProcessor::CreateNextMinRecord(CXFA_Node* pRecordNode) {
  if (!pRecordNode) {
    return;
  }

  for (CXFA_Node* pCurrentNode = pRecordNode->GetNextSibling(); pCurrentNode;
       pCurrentNode = pCurrentNode->GetNextSibling()) {
    if (pCurrentNode->GetElementType() == XFA_Element::PageArea) {
      CreateMinPageRecord(pCurrentNode, false, false);
    } else if (pCurrentNode->GetElementType() == XFA_Element::PageSet) {
      CreateMinPageSetRecord(pCurrentNode, true);
    }
  }
}

void CXFA_ViewLayoutProcessor::ProcessLastPageSet() {
  if (!cur_page_area_) {
    return;
  }

  CreateMinPageRecord(cur_page_area_, false, true);
  CreateNextMinRecord(cur_page_area_);
  CXFA_Node* pPageSet = cur_page_area_->GetParent();
  while (pPageSet) {
    CreateMinPageSetRecord(pPageSet, false);
    if (pPageSet == page_set_node_) {
      break;
    }

    CreateNextMinRecord(pPageSet);
    pPageSet = pPageSet->GetParent();
  }
}

bool CXFA_ViewLayoutProcessor::GetNextAvailContentHeight(float fChildHeight) {
  CXFA_ViewRecord* pViewRecord = GetCurrentViewRecord();
  if (!pViewRecord) {
    return false;
  }

  CXFA_Node* pCurContentNode = pViewRecord->pCurContentArea->GetFormNode();
  if (!pCurContentNode) {
    return false;
  }

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
  std::optional<int32_t> ret;
  if (pOccurNode) {
    ret = pOccurNode->JSObject()->TryInteger(XFA_Attribute::Max, false);
    if (ret.has_value()) {
      iMax = ret.value();
    }
  }
  if (ret.has_value()) {
    if (cur_page_count_ == iMax) {
      CXFA_Node* pSrcPage = cur_page_area_;
      int32_t nSrcPageCount = cur_page_count_;
      auto psSrcIter = GetTailPosition();
      CXFA_Node* pNextPage =
          GetNextAvailPageArea(nullptr, nullptr, false, true);
      cur_page_area_ = pSrcPage;
      cur_page_count_ = nSrcPageCount;
      CXFA_ViewRecord* pPrevRecord = psSrcIter->Get();
      ++psSrcIter;
      while (psSrcIter != proposed_view_records_.end()) {
        auto psSaveIter = psSrcIter++;
        RemoveLayoutRecord(psSaveIter->Get(), pPrevRecord);
        proposed_view_records_.erase(psSaveIter);
      }
      if (pNextPage) {
        CXFA_Node* pContentArea =
            pNextPage->GetFirstChildByClass<CXFA_ContentArea>(
                XFA_Element::ContentArea);
        if (pContentArea) {
          float fNextContentHeight = pContentArea->JSObject()->GetMeasureInUnit(
              XFA_Attribute::H, XFA_Unit::Pt);
          if (fNextContentHeight > fChildHeight) {
            return true;
          }
        }
      }
      return false;
    }
  }

  CXFA_Node* pContentArea = pPageNode->GetFirstChildByClass<CXFA_ContentArea>(
      XFA_Element::ContentArea);
  if (!pContentArea) {
    return false;
  }

  float fNextContentHeight = pContentArea->JSObject()->GetMeasureInUnit(
      XFA_Attribute::H, XFA_Unit::Pt);
  return fNextContentHeight < kXFALayoutPrecision ||
         fNextContentHeight > fChildHeight;
}

void CXFA_ViewLayoutProcessor::ClearData() {
  if (!page_set_node_) {
    return;
  }

  proposed_view_records_.clear();
  current_view_record_iter_ = proposed_view_records_.end();
  cur_page_area_ = nullptr;
  cur_page_count_ = 0;
  create_over_flow_page_ = false;
  page_set_map_.clear();
}

void CXFA_ViewLayoutProcessor::SaveLayoutItemChildren(
    CXFA_LayoutItem* pParentLayoutItem) {
  CXFA_Document* document = page_set_node_->GetDocument();
  CXFA_FFNotify* pNotify = document->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(document);
  CXFA_LayoutItem* pCurLayoutItem = pParentLayoutItem->GetFirstChild();
  while (pCurLayoutItem) {
    CXFA_LayoutItem* pNextLayoutItem = pCurLayoutItem->GetNextSibling();
    if (pCurLayoutItem->IsContentLayoutItem()) {
      if (pCurLayoutItem->GetFormNode()->HasRemovedChildren()) {
        SyncRemoveLayoutItem(pCurLayoutItem, pNotify, pDocLayout);
        pCurLayoutItem = pNextLayoutItem;
        continue;
      }
      if (pCurLayoutItem->GetFormNode()->IsLayoutGeneratedNode()) {
        pCurLayoutItem->GetFormNode()->SetNodeAndDescendantsUnused();
      }
    }
    SaveLayoutItemChildren(pCurLayoutItem);
    pCurLayoutItem = pNextLayoutItem;
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
    if (pCurNode->GetElementType() == XFA_Element::Overflow) {
      return pCurNode;
    }
  }
  return nullptr;
}

void CXFA_ViewLayoutProcessor::MergePageSetContents() {
  CXFA_Document* document = page_set_node_->GetDocument();
  document->SetPendingNodesUnusedAndUnbound();

  CXFA_FFNotify* pNotify = document->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(document);
  CXFA_ViewLayoutItem* pRootLayout = GetRootLayoutItem();

  size_t pending_index = 0;
  for (; pRootLayout;
       pRootLayout = ToViewLayoutItem(pRootLayout->GetNextSibling())) {
    CXFA_Node* pPendingPageSet = nullptr;
    ViewLayoutItemIterator iterator(pRootLayout);
    CXFA_ViewLayoutItem* pRootPageSetViewItem = iterator.GetCurrent();
    DCHECK(pRootPageSetViewItem->GetFormNode()->GetElementType() ==
           XFA_Element::PageSet);
    if (pending_index < document->GetPendingNodesCount()) {
      pPendingPageSet = document->GetPendingNodeAtIndex(pending_index);
      ++pending_index;
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
    pPendingPageSet->ClearFlag(XFA_NodeFlag::kUnusedNode);
    for (CXFA_ViewLayoutItem* pViewItem = iterator.MoveToNext(); pViewItem;
         pViewItem = iterator.MoveToNext()) {
      CXFA_Node* pNode = pViewItem->GetFormNode();
      if (pNode->GetPacketType() != XFA_PacketType::Template) {
        continue;
      }

      switch (pNode->GetElementType()) {
        case XFA_Element::PageSet: {
          CXFA_Node* pParentNode = pViewItem->GetParent()->GetFormNode();
          CXFA_Node* pOldNode = pViewItem->GetFormNode();
          CXFA_Node* pNewNode = XFA_NodeMerge_CloneOrMergeContainer(
              document, pParentNode, pOldNode, true, nullptr);
          if (pOldNode != pNewNode) {
            pOldNode->JSObject()->SetLayoutItem(nullptr);
            pViewItem->SetFormNode(pNewNode);
          }
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
            if (pViewItem->GetOldSubform() &&
                pViewItem->GetOldSubform() != pNewSubform) {
              CXFA_Node* pExistingNode = XFA_DataMerge_FindFormDOMInstance(
                  document, pViewItem->GetFormNode()->GetElementType(),
                  pViewItem->GetFormNode()->GetNameHash(), pParentNode);
              CXFA_ContainerIterator sIterator(pExistingNode);
              for (CXFA_Node* pIter = sIterator.GetCurrent(); pIter;
                   pIter = sIterator.MoveToNext()) {
                if (pIter->GetElementType() != XFA_Element::ContentArea) {
                  CXFA_LayoutItem* pLayoutItem =
                      pIter->JSObject()->GetLayoutItem();
                  if (pLayoutItem) {
                    pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem);
                    pLayoutItem->RemoveSelfIfParented();
                  }
                }
              }
              if (pExistingNode) {
                pParentNode->RemoveChildAndNotify(pExistingNode, true);
              }
            }
            pViewItem->SetOldSubform(pNewSubform);
          }
          CXFA_Node* pOldNode = pViewItem->GetFormNode();
          CXFA_Node* pNewNode = document->DataMerge_CopyContainer(
              pOldNode, pParentNode,
              ToNode(document->GetXFAObject(XFA_HASHCODE_Record)), true, true,
              true);
          if (pOldNode != pNewNode) {
            pOldNode->JSObject()->SetLayoutItem(nullptr);
            pViewItem->SetFormNode(pNewNode);
          }
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
      CXFA_Node* pNode = ToNode(document->GetXFAObject(XFA_HASHCODE_Form));
      if (pNode) {
        CXFA_Node* pFormToplevelSubform =
            pNode->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
        if (pFormToplevelSubform) {
          pFormToplevelSubform->InsertChildAndNotify(pPendingPageSet, nullptr);
        }
      }
    }
    document->DataMerge_UpdateBindingRelations(pPendingPageSet);
    pPendingPageSet->SetInitializedFlagAndNotify();
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
              CXFA_LayoutItem* pLayoutItem =
                  pChildNode->JSObject()->GetLayoutItem();
              if (pLayoutItem) {
                pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem);
                pLayoutItem->RemoveSelfIfParented();
              }
            }
          } else if (eType != XFA_Element::ContentArea) {
            CXFA_LayoutItem* pLayoutItem = pNode->JSObject()->GetLayoutItem();
            if (pLayoutItem) {
              pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem);
              pLayoutItem->RemoveSelfIfParented();
            }
          }
          CXFA_Node* pNext = sIterator.SkipChildrenAndMoveToNext();
          pNode->GetParent()->RemoveChildAndNotify(pNode, true);
          pNode = pNext;
        } else {
          pNode->ClearFlag(XFA_NodeFlag::kUnusedNode);
          pNode->SetInitializedFlagAndNotify();
          pNode = sIterator.MoveToNext();
        }
      } else {
        pNode->SetInitializedFlagAndNotify();
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
      if (type != XFA_Element::PageArea) {
        continue;
      }

      layout_processor_->GetRootContentLayoutProcessor()->DoLayoutPageArea(
          pViewItem);
    }
  }
}

void CXFA_ViewLayoutProcessor::SyncLayoutData() {
  MergePageSetContents();
  LayoutPageSetContents();
  CXFA_FFNotify* pNotify = page_set_node_->GetDocument()->GetNotify();
  int32_t nPageIdx = -1;
  for (CXFA_ViewLayoutItem* pRootLayoutItem = GetRootLayoutItem();
       pRootLayoutItem;
       pRootLayoutItem = ToViewLayoutItem(pRootLayoutItem->GetNextSibling())) {
    ViewLayoutItemIterator iteratorParent(pRootLayoutItem);
    for (CXFA_ViewLayoutItem* pViewItem = iteratorParent.GetCurrent();
         pViewItem; pViewItem = iteratorParent.MoveToNext()) {
      XFA_Element type = pViewItem->GetFormNode()->GetElementType();
      if (type != XFA_Element::PageArea) {
        continue;
      }

      nPageIdx++;
      Mask<XFA_WidgetStatus> dwRelevant = {XFA_WidgetStatus::kViewable,
                                           XFA_WidgetStatus::kPrintable};
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
        Mask<XFA_WidgetStatus> dwRelevantChild =
            GetRelevant(pContentItem->GetFormNode(), dwRelevant);
        SyncContainer(pNotify, layout_processor_, pContentItem, dwRelevantChild,
                      bVisible, nPageIdx);
        pChildLayoutItem = iterator.SkipChildrenAndMoveToNext();
      }
    }
  }

  int32_t nPage = fxcrt::CollectionSize<int32_t>(page_array_);
  for (int32_t i = nPage - 1; i >= avail_pages_; i--) {
    CXFA_ViewLayoutItem* pPage = page_array_[i];
    page_array_.erase(page_array_.begin() + i);
    pNotify->OnPageViewEvent(pPage, CXFA_FFDoc::PageViewEvent::kPostRemoved);
  }
  ClearData();
}

void CXFA_ViewLayoutProcessor::PrepareLayout() {
  page_set_cur_layout_item_ = nullptr;
  page_set_mode_ = XFA_AttributeValue::OrderedOccurrence;
  avail_pages_ = 0;
  ClearData();
  if (!page_set_root_layout_item_) {
    return;
  }

  CXFA_ViewLayoutItem* pRootLayoutItem = page_set_root_layout_item_;
  if (pRootLayoutItem &&
      pRootLayoutItem->GetFormNode()->GetPacketType() == XFA_PacketType::Form) {
    CXFA_Document* const pRootDocument =
        pRootLayoutItem->GetFormNode()->GetDocument();
    CXFA_Node* pPageSetFormNode = pRootLayoutItem->GetFormNode();
    pRootDocument->ClearPendingNodes();
    if (pPageSetFormNode->HasRemovedChildren()) {
      XFA_ReleaseLayoutItem(pRootLayoutItem);
      page_set_root_layout_item_ = nullptr;
      pRootLayoutItem = nullptr;
      pPageSetFormNode = nullptr;
      page_array_.clear();
    }
    while (pPageSetFormNode) {
      CXFA_Node* pNextPageSet =
          pPageSetFormNode->GetNextSameClassSibling<CXFA_PageSet>(
              XFA_Element::PageSet);
      pPageSetFormNode->GetParent()->RemoveChildAndNotify(pPageSetFormNode,
                                                          false);
      pRootDocument->AppendPendingNode(pPageSetFormNode);
      pPageSetFormNode = pNextPageSet;
    }
  }
  pRootLayoutItem = page_set_root_layout_item_;
  CXFA_ViewLayoutItem* pNextLayout = nullptr;
  for (; pRootLayoutItem; pRootLayoutItem = pNextLayout) {
    pNextLayout = ToViewLayoutItem(pRootLayoutItem->GetNextSibling());
    SaveLayoutItemChildren(pRootLayoutItem);
    pRootLayoutItem->RemoveSelfIfParented();
  }
  page_set_root_layout_item_ = nullptr;
}

void CXFA_ViewLayoutProcessor::ProcessSimplexOrDuplexPageSets(
    CXFA_ViewLayoutItem* pPageSetLayoutItem,
    bool bIsSimplex) {
  auto [nPageAreaCount, pLastPageAreaLayoutItem] =
      GetPageAreaCountAndLastPageAreaFromPageSet(pPageSetLayoutItem);
  if (!pLastPageAreaLayoutItem) {
    return;
  }

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

  CXFA_Node* pNode = cur_page_area_;
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
