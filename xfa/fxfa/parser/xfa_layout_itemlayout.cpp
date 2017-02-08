// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_layout_itemlayout.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/app/xfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_containerlayoutitem.h"
#include "xfa/fxfa/parser/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_layoutpagemgr.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/xfa_localemgr.h"
#include "xfa/fxfa/parser/xfa_object.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

std::vector<CFX_WideString> SeparateStringW(const FX_WCHAR* pStr,
                                            int32_t iStrLen,
                                            FX_WCHAR delimiter) {
  std::vector<CFX_WideString> ret;
  if (!pStr)
    return ret;
  if (iStrLen < 0)
    iStrLen = FXSYS_wcslen(pStr);

  const FX_WCHAR* pToken = pStr;
  const FX_WCHAR* pEnd = pStr + iStrLen;
  while (true) {
    if (pStr >= pEnd || delimiter == *pStr) {
      ret.push_back(CFX_WideString(pToken, pStr - pToken));
      pToken = pStr + 1;
      if (pStr >= pEnd)
        break;
    }
    pStr++;
  }
  return ret;
}

void UpdateWidgetSize(CXFA_ContentLayoutItem* pLayoutItem,
                      FX_FLOAT* fWidth,
                      FX_FLOAT* fHeight) {
  CXFA_Node* pNode = pLayoutItem->m_pFormNode;
  switch (pNode->GetElementType()) {
    case XFA_Element::Subform:
    case XFA_Element::Area:
    case XFA_Element::ExclGroup:
    case XFA_Element::SubformSet: {
      if (*fWidth < -XFA_LAYOUT_FLOAT_PERCISION)
        *fWidth = pLayoutItem->m_sSize.width;
      if (*fHeight < -XFA_LAYOUT_FLOAT_PERCISION)
        *fHeight = pLayoutItem->m_sSize.height;
      break;
    }
    case XFA_Element::Draw:
    case XFA_Element::Field: {
      pNode->GetDocument()->GetNotify()->StartFieldDrawLayout(pNode, *fWidth,
                                                              *fHeight);
      break;
    }
    default:
      ASSERT(false);
  }
}

CFX_SizeF CalculateContainerSpecifiedSize(CXFA_Node* pFormNode,
                                          bool* bContainerWidthAutoSize,
                                          bool* bContainerHeightAutoSize) {
  *bContainerWidthAutoSize = true;
  *bContainerHeightAutoSize = true;

  XFA_Element eType = pFormNode->GetElementType();
  CXFA_Measurement mTmpValue;
  CFX_SizeF containerSize;
  if ((eType == XFA_Element::Subform || eType == XFA_Element::ExclGroup) &&
      pFormNode->TryMeasure(XFA_ATTRIBUTE_W, mTmpValue, false) &&
      mTmpValue.GetValue() > XFA_LAYOUT_FLOAT_PERCISION) {
    containerSize.width = mTmpValue.ToUnit(XFA_UNIT_Pt);
    *bContainerWidthAutoSize = false;
  }
  if ((eType == XFA_Element::Subform || eType == XFA_Element::ExclGroup) &&
      pFormNode->TryMeasure(XFA_ATTRIBUTE_H, mTmpValue, false) &&
      mTmpValue.GetValue() > XFA_LAYOUT_FLOAT_PERCISION) {
    containerSize.height = mTmpValue.ToUnit(XFA_UNIT_Pt);
    *bContainerHeightAutoSize = false;
  }
  if (*bContainerWidthAutoSize && eType == XFA_Element::Subform &&
      pFormNode->TryMeasure(XFA_ATTRIBUTE_MaxW, mTmpValue, false) &&
      mTmpValue.GetValue() > XFA_LAYOUT_FLOAT_PERCISION) {
    containerSize.width = mTmpValue.ToUnit(XFA_UNIT_Pt);
    *bContainerWidthAutoSize = false;
  }
  if (*bContainerHeightAutoSize && eType == XFA_Element::Subform &&
      pFormNode->TryMeasure(XFA_ATTRIBUTE_MaxH, mTmpValue, false) &&
      mTmpValue.GetValue() > XFA_LAYOUT_FLOAT_PERCISION) {
    containerSize.height = mTmpValue.ToUnit(XFA_UNIT_Pt);
    *bContainerHeightAutoSize = false;
  }
  return containerSize;
}

CFX_SizeF CalculateContainerComponentSizeFromContentSize(
    CXFA_Node* pFormNode,
    bool bContainerWidthAutoSize,
    FX_FLOAT fContentCalculatedWidth,
    bool bContainerHeightAutoSize,
    FX_FLOAT fContentCalculatedHeight,
    const CFX_SizeF& currentContainerSize) {
  CFX_SizeF componentSize = currentContainerSize;
  CXFA_Node* pMarginNode = pFormNode->GetFirstChildByClass(XFA_Element::Margin);
  CXFA_Measurement mTmpValue;
  if (bContainerWidthAutoSize) {
    componentSize.width = fContentCalculatedWidth;
    if (pMarginNode) {
      if (pMarginNode->TryMeasure(XFA_ATTRIBUTE_LeftInset, mTmpValue, false))
        componentSize.width += mTmpValue.ToUnit(XFA_UNIT_Pt);
      if (pMarginNode->TryMeasure(XFA_ATTRIBUTE_RightInset, mTmpValue, false))
        componentSize.width += mTmpValue.ToUnit(XFA_UNIT_Pt);
    }
  }

  if (bContainerHeightAutoSize) {
    componentSize.height = fContentCalculatedHeight;
    if (pMarginNode) {
      if (pMarginNode->TryMeasure(XFA_ATTRIBUTE_TopInset, mTmpValue, false))
        componentSize.height += mTmpValue.ToUnit(XFA_UNIT_Pt);
      if (pMarginNode->TryMeasure(XFA_ATTRIBUTE_BottomInset, mTmpValue,
                                  false)) {
        componentSize.height += mTmpValue.ToUnit(XFA_UNIT_Pt);
      }
    }
  }
  return componentSize;
}

void RelocateTableRowCells(
    CXFA_ContentLayoutItem* pLayoutRow,
    const CFX_ArrayTemplate<FX_FLOAT>& rgSpecifiedColumnWidths,
    XFA_ATTRIBUTEENUM eLayout) {
  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      pLayoutRow->m_pFormNode, &bContainerWidthAutoSize,
      &bContainerHeightAutoSize);
  CXFA_Node* pMarginNode =
      pLayoutRow->m_pFormNode->GetFirstChildByClass(XFA_Element::Margin);
  FX_FLOAT fLeftInset = 0;
  FX_FLOAT fTopInset = 0;
  FX_FLOAT fRightInset = 0;
  FX_FLOAT fBottomInset = 0;
  if (pMarginNode) {
    fLeftInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_LeftInset).ToUnit(XFA_UNIT_Pt);
    fTopInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_TopInset).ToUnit(XFA_UNIT_Pt);
    fRightInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_RightInset).ToUnit(XFA_UNIT_Pt);
    fBottomInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_BottomInset).ToUnit(XFA_UNIT_Pt);
  }

  FX_FLOAT fContentWidthLimit =
      bContainerWidthAutoSize ? FLT_MAX
                              : containerSize.width - fLeftInset - fRightInset;
  FX_FLOAT fContentCurrentHeight =
      pLayoutRow->m_sSize.height - fTopInset - fBottomInset;
  FX_FLOAT fContentCalculatedWidth = 0;
  FX_FLOAT fContentCalculatedHeight = 0;
  FX_FLOAT fCurrentColX = 0;
  int32_t nCurrentColIdx = 0;
  bool bMetWholeRowCell = false;

  for (auto pLayoutChild =
           static_cast<CXFA_ContentLayoutItem*>(pLayoutRow->m_pFirstChild);
       pLayoutChild; pLayoutChild = static_cast<CXFA_ContentLayoutItem*>(
                         pLayoutChild->m_pNextSibling)) {
    int32_t nOriginalColSpan =
        pLayoutChild->m_pFormNode->GetInteger(XFA_ATTRIBUTE_ColSpan);
    int32_t nColSpan = nOriginalColSpan;
    FX_FLOAT fColSpanWidth = 0;
    if (nColSpan == -1 ||
        nCurrentColIdx + nColSpan > rgSpecifiedColumnWidths.GetSize()) {
      nColSpan = rgSpecifiedColumnWidths.GetSize() - nCurrentColIdx;
    }
    for (int32_t i = 0; i < nColSpan; i++)
      fColSpanWidth += rgSpecifiedColumnWidths[nCurrentColIdx + i];

    if (nColSpan != nOriginalColSpan) {
      fColSpanWidth =
          bMetWholeRowCell ? 0 : std::max(fColSpanWidth,
                                          pLayoutChild->m_sSize.height);
    }
    if (nOriginalColSpan == -1)
      bMetWholeRowCell = true;

    pLayoutChild->m_sPos = CFX_PointF(fCurrentColX, 0);
    pLayoutChild->m_sSize.width = fColSpanWidth;
    if (!XFA_ItemLayoutProcessor_IsTakingSpace(pLayoutChild->m_pFormNode))
      continue;

    fCurrentColX += fColSpanWidth;
    nCurrentColIdx += nColSpan;
    FX_FLOAT fNewHeight = bContainerHeightAutoSize ? -1 : fContentCurrentHeight;
    UpdateWidgetSize(pLayoutChild, &fColSpanWidth, &fNewHeight);
    pLayoutChild->m_sSize.height = fNewHeight;
    if (bContainerHeightAutoSize) {
      fContentCalculatedHeight =
          std::max(fContentCalculatedHeight, pLayoutChild->m_sSize.height);
    }
  }

  if (bContainerHeightAutoSize) {
    for (CXFA_ContentLayoutItem* pLayoutChild =
             (CXFA_ContentLayoutItem*)pLayoutRow->m_pFirstChild;
         pLayoutChild;
         pLayoutChild = (CXFA_ContentLayoutItem*)pLayoutChild->m_pNextSibling) {
      UpdateWidgetSize(pLayoutChild, &pLayoutChild->m_sSize.width,
                       &fContentCalculatedHeight);
      FX_FLOAT fOldChildHeight = pLayoutChild->m_sSize.height;
      pLayoutChild->m_sSize.height = fContentCalculatedHeight;
      CXFA_Node* pParaNode =
          pLayoutChild->m_pFormNode->GetFirstChildByClass(XFA_Element::Para);
      if (pParaNode && pLayoutChild->m_pFirstChild) {
        FX_FLOAT fOffHeight = fContentCalculatedHeight - fOldChildHeight;
        XFA_ATTRIBUTEENUM eVType = pParaNode->GetEnum(XFA_ATTRIBUTE_VAlign);
        switch (eVType) {
          case XFA_ATTRIBUTEENUM_Middle:
            fOffHeight = fOffHeight / 2;
            break;
          case XFA_ATTRIBUTEENUM_Bottom:
            break;
          case XFA_ATTRIBUTEENUM_Top:
          default:
            fOffHeight = 0;
            break;
        }
        if (fOffHeight > 0) {
          for (CXFA_ContentLayoutItem* pInnerLayoutChild =
                   (CXFA_ContentLayoutItem*)pLayoutChild->m_pFirstChild;
               pInnerLayoutChild;
               pInnerLayoutChild =
                   (CXFA_ContentLayoutItem*)pInnerLayoutChild->m_pNextSibling) {
            pInnerLayoutChild->m_sPos.y += fOffHeight;
          }
        }
      }
    }
  }

  if (bContainerWidthAutoSize) {
    FX_FLOAT fChildSuppliedWidth = fCurrentColX;
    if (fContentWidthLimit < FLT_MAX &&
        fContentWidthLimit > fChildSuppliedWidth) {
      fChildSuppliedWidth = fContentWidthLimit;
    }
    fContentCalculatedWidth =
        std::max(fContentCalculatedWidth, fChildSuppliedWidth);
  } else {
    fContentCalculatedWidth = containerSize.width - fLeftInset - fRightInset;
  }

  if (pLayoutRow->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout) ==
      XFA_ATTRIBUTEENUM_Rl_row) {
    for (CXFA_ContentLayoutItem* pLayoutChild =
             (CXFA_ContentLayoutItem*)pLayoutRow->m_pFirstChild;
         pLayoutChild;
         pLayoutChild = (CXFA_ContentLayoutItem*)pLayoutChild->m_pNextSibling) {
      pLayoutChild->m_sPos.x = fContentCalculatedWidth -
                               pLayoutChild->m_sPos.x -
                               pLayoutChild->m_sSize.width;
    }
  }
  pLayoutRow->m_sSize = CalculateContainerComponentSizeFromContentSize(
      pLayoutRow->m_pFormNode, bContainerWidthAutoSize, fContentCalculatedWidth,
      bContainerHeightAutoSize, fContentCalculatedHeight, containerSize);
}

void UpdatePendingItemLayout(CXFA_ItemLayoutProcessor* pProcessor,
                             CXFA_ContentLayoutItem* pLayoutItem) {
  XFA_ATTRIBUTEENUM eLayout =
      pLayoutItem->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
  switch (eLayout) {
    case XFA_ATTRIBUTEENUM_Row:
    case XFA_ATTRIBUTEENUM_Rl_row:
      RelocateTableRowCells(pLayoutItem, pProcessor->m_rgSpecifiedColumnWidths,
                            eLayout);
      break;
    default:
      break;
  }
}

void AddTrailerBeforeSplit(CXFA_ItemLayoutProcessor* pProcessor,
                           FX_FLOAT fSplitPos,
                           CXFA_ContentLayoutItem* pTrailerLayoutItem,
                           bool bUseInherited) {
  if (!pTrailerLayoutItem)
    return;

  FX_FLOAT fHeight = pTrailerLayoutItem->m_sSize.height;
  if (bUseInherited) {
    FX_FLOAT fNewSplitPos = 0;
    if (fSplitPos - fHeight > XFA_LAYOUT_FLOAT_PERCISION)
      fNewSplitPos = pProcessor->FindSplitPos(fSplitPos - fHeight);
    if (fNewSplitPos > XFA_LAYOUT_FLOAT_PERCISION)
      pProcessor->SplitLayoutItem(fNewSplitPos);
    return;
  }

  UpdatePendingItemLayout(pProcessor, pTrailerLayoutItem);
  CXFA_Node* pMarginNode =
      pProcessor->m_pFormNode->GetFirstChildByClass(XFA_Element::Margin);
  FX_FLOAT fLeftInset = 0;
  FX_FLOAT fTopInset = 0;
  FX_FLOAT fRightInset = 0;
  FX_FLOAT fBottomInset = 0;
  if (pMarginNode) {
    fLeftInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_LeftInset).ToUnit(XFA_UNIT_Pt);
    fTopInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_TopInset).ToUnit(XFA_UNIT_Pt);
    fRightInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_RightInset).ToUnit(XFA_UNIT_Pt);
    fBottomInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_BottomInset).ToUnit(XFA_UNIT_Pt);
  }

  if (!pProcessor->IsAddNewRowForTrailer(pTrailerLayoutItem)) {
    pTrailerLayoutItem->m_sPos.y = pProcessor->m_fLastRowY;
    pTrailerLayoutItem->m_sPos.x = pProcessor->m_fLastRowWidth;
    pProcessor->m_pLayoutItem->m_sSize.width +=
        pTrailerLayoutItem->m_sSize.width;
    pProcessor->m_pLayoutItem->AddChild(pTrailerLayoutItem);
    return;
  }

  FX_FLOAT fNewSplitPos = 0;
  if (fSplitPos - fHeight > XFA_LAYOUT_FLOAT_PERCISION)
    fNewSplitPos = pProcessor->FindSplitPos(fSplitPos - fHeight);

  if (fNewSplitPos > XFA_LAYOUT_FLOAT_PERCISION) {
    pProcessor->SplitLayoutItem(fNewSplitPos);
    pTrailerLayoutItem->m_sPos.y = fNewSplitPos - fTopInset - fBottomInset;
  } else {
    pTrailerLayoutItem->m_sPos.y = fSplitPos - fTopInset - fBottomInset;
  }

  switch (pTrailerLayoutItem->m_pFormNode->GetEnum(XFA_ATTRIBUTE_HAlign)) {
    case XFA_ATTRIBUTEENUM_Right:
      pTrailerLayoutItem->m_sPos.x = pProcessor->m_pLayoutItem->m_sSize.width -
                                     fRightInset -
                                     pTrailerLayoutItem->m_sSize.width;
      break;
    case XFA_ATTRIBUTEENUM_Center:
      pTrailerLayoutItem->m_sPos.x =
          (pProcessor->m_pLayoutItem->m_sSize.width - fLeftInset - fRightInset -
           pTrailerLayoutItem->m_sSize.width) /
          2;
      break;
    case XFA_ATTRIBUTEENUM_Left:
    default:
      pTrailerLayoutItem->m_sPos.x = fLeftInset;
      break;
  }
  pProcessor->m_pLayoutItem->m_sSize.height += fHeight;
  pProcessor->m_pLayoutItem->AddChild(pTrailerLayoutItem);
}

void AddLeaderAfterSplit(CXFA_ItemLayoutProcessor* pProcessor,
                         CXFA_ContentLayoutItem* pLeaderLayoutItem) {
  UpdatePendingItemLayout(pProcessor, pLeaderLayoutItem);

  CXFA_Node* pMarginNode =
      pProcessor->m_pFormNode->GetFirstChildByClass(XFA_Element::Margin);
  FX_FLOAT fLeftInset = 0;
  FX_FLOAT fRightInset = 0;
  if (pMarginNode) {
    fLeftInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_LeftInset).ToUnit(XFA_UNIT_Pt);
    fRightInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_RightInset).ToUnit(XFA_UNIT_Pt);
  }

  FX_FLOAT fHeight = pLeaderLayoutItem->m_sSize.height;
  for (CXFA_ContentLayoutItem* pChildItem =
           (CXFA_ContentLayoutItem*)pProcessor->m_pLayoutItem->m_pFirstChild;
       pChildItem;
       pChildItem = (CXFA_ContentLayoutItem*)pChildItem->m_pNextSibling) {
    pChildItem->m_sPos.y += fHeight;
  }
  pLeaderLayoutItem->m_sPos.y = 0;

  switch (pLeaderLayoutItem->m_pFormNode->GetEnum(XFA_ATTRIBUTE_HAlign)) {
    case XFA_ATTRIBUTEENUM_Right:
      pLeaderLayoutItem->m_sPos.x = pProcessor->m_pLayoutItem->m_sSize.width -
                                    fRightInset -
                                    pLeaderLayoutItem->m_sSize.width;
      break;
    case XFA_ATTRIBUTEENUM_Center:
      pLeaderLayoutItem->m_sPos.x =
          (pProcessor->m_pLayoutItem->m_sSize.width - fLeftInset - fRightInset -
           pLeaderLayoutItem->m_sSize.width) /
          2;
      break;
    case XFA_ATTRIBUTEENUM_Left:
    default:
      pLeaderLayoutItem->m_sPos.x = fLeftInset;
      break;
  }
  pProcessor->m_pLayoutItem->m_sSize.height += fHeight;
  pProcessor->m_pLayoutItem->AddChild(pLeaderLayoutItem);
}

void AddPendingNode(CXFA_ItemLayoutProcessor* pProcessor,
                    CXFA_Node* pPendingNode,
                    bool bBreakPending) {
  pProcessor->m_PendingNodes.push_back(pPendingNode);
  pProcessor->m_bBreakPending = bBreakPending;
}

FX_FLOAT InsertPendingItems(CXFA_ItemLayoutProcessor* pProcessor,
                            CXFA_Node* pCurChildNode) {
  FX_FLOAT fTotalHeight = 0;
  if (pProcessor->m_PendingNodes.empty())
    return fTotalHeight;

  if (!pProcessor->m_pLayoutItem) {
    pProcessor->m_pLayoutItem =
        pProcessor->CreateContentLayoutItem(pCurChildNode);
    pProcessor->m_pLayoutItem->m_sSize.clear();
  }

  while (!pProcessor->m_PendingNodes.empty()) {
    auto pPendingProcessor = pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(
        pProcessor->m_PendingNodes.front(), nullptr);
    pProcessor->m_PendingNodes.pop_front();
    pPendingProcessor->DoLayout(false, FLT_MAX, FLT_MAX, nullptr);
    CXFA_ContentLayoutItem* pPendingLayoutItem =
        pPendingProcessor->HasLayoutItem()
            ? pPendingProcessor->ExtractLayoutItem()
            : nullptr;
    if (pPendingLayoutItem) {
      AddLeaderAfterSplit(pProcessor, pPendingLayoutItem);
      if (pProcessor->m_bBreakPending)
        fTotalHeight += pPendingLayoutItem->m_sSize.height;
    }
  }
  return fTotalHeight;
}

XFA_ATTRIBUTEENUM GetLayout(CXFA_Node* pFormNode, bool* bRootForceTb) {
  *bRootForceTb = false;
  XFA_ATTRIBUTEENUM eLayoutMode;
  if (pFormNode->TryEnum(XFA_ATTRIBUTE_Layout, eLayoutMode, false))
    return eLayoutMode;

  CXFA_Node* pParentNode = pFormNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (pParentNode && pParentNode->GetElementType() == XFA_Element::Form) {
    *bRootForceTb = true;
    return XFA_ATTRIBUTEENUM_Tb;
  }
  return XFA_ATTRIBUTEENUM_Position;
}

bool ExistContainerKeep(CXFA_Node* pCurNode, bool bPreFind) {
  if (!pCurNode || !XFA_ItemLayoutProcessor_IsTakingSpace(pCurNode))
    return false;

  XFA_NODEITEM eItemType = XFA_NODEITEM_PrevSibling;
  if (!bPreFind)
    eItemType = XFA_NODEITEM_NextSibling;

  CXFA_Node* pPreContainer =
      pCurNode->GetNodeItem(eItemType, XFA_ObjectType::ContainerNode);
  if (!pPreContainer)
    return false;

  CXFA_Node* pKeep = pCurNode->GetFirstChildByClass(XFA_Element::Keep);
  if (pKeep) {
    XFA_ATTRIBUTEENUM ePrevious;
    XFA_ATTRIBUTE eKeepType = XFA_ATTRIBUTE_Previous;
    if (!bPreFind)
      eKeepType = XFA_ATTRIBUTE_Next;

    if (pKeep->TryEnum(eKeepType, ePrevious, false)) {
      if (ePrevious == XFA_ATTRIBUTEENUM_ContentArea ||
          ePrevious == XFA_ATTRIBUTEENUM_PageArea) {
        return true;
      }
    }
  }

  pKeep = pPreContainer->GetFirstChildByClass(XFA_Element::Keep);
  if (!pKeep)
    return false;

  XFA_ATTRIBUTE eKeepType = XFA_ATTRIBUTE_Next;
  if (!bPreFind)
    eKeepType = XFA_ATTRIBUTE_Previous;

  XFA_ATTRIBUTEENUM eNext;
  if (!pKeep->TryEnum(eKeepType, eNext, false))
    return false;
  if (eNext == XFA_ATTRIBUTEENUM_ContentArea ||
      eNext == XFA_ATTRIBUTEENUM_PageArea) {
    return true;
  }
  return false;
}

bool FindBreakNode(CXFA_Node* pContainerNode,
                   CXFA_Node*& pCurActionNode,
                   XFA_ItemLayoutProcessorStages* nCurStage,
                   bool bBreakBefore) {
  bool bFindRs = false;
  for (CXFA_Node* pBreakNode = pContainerNode; pBreakNode;
       pBreakNode = pBreakNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_ATTRIBUTE eAttributeType = XFA_ATTRIBUTE_Before;
    if (!bBreakBefore)
      eAttributeType = XFA_ATTRIBUTE_After;

    switch (pBreakNode->GetElementType()) {
      case XFA_Element::BreakBefore: {
        if (bBreakBefore) {
          pCurActionNode = pBreakNode;
          *nCurStage = XFA_ItemLayoutProcessorStages::BreakBefore;
          bFindRs = true;
        }
        break;
      }
      case XFA_Element::BreakAfter: {
        if (!bBreakBefore) {
          pCurActionNode = pBreakNode;
          *nCurStage = XFA_ItemLayoutProcessorStages::BreakAfter;
          bFindRs = true;
        }
        break;
      }
      case XFA_Element::Break:
        if (pBreakNode->GetEnum(eAttributeType) != XFA_ATTRIBUTEENUM_Auto) {
          pCurActionNode = pBreakNode;
          *nCurStage = XFA_ItemLayoutProcessorStages::BreakBefore;
          if (!bBreakBefore)
            *nCurStage = XFA_ItemLayoutProcessorStages::BreakAfter;

          bFindRs = true;
        }
        break;
      default:
        break;
    }
    if (bFindRs)
      break;
  }
  return bFindRs;
}

void DeleteLayoutGeneratedNode(CXFA_Node* pGenerateNode) {
  CXFA_FFNotify* pNotify = pGenerateNode->GetDocument()->GetNotify();
  CXFA_LayoutProcessor* pDocLayout =
      pGenerateNode->GetDocument()->GetDocLayout();
  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode> sIterator(
      pGenerateNode);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    CXFA_ContentLayoutItem* pCurLayoutItem =
        (CXFA_ContentLayoutItem*)pNode->GetUserData(XFA_LAYOUTITEMKEY);
    CXFA_ContentLayoutItem* pNextLayoutItem = nullptr;
    while (pCurLayoutItem) {
      pNextLayoutItem = pCurLayoutItem->m_pNext;
      pNotify->OnLayoutItemRemoving(pDocLayout, pCurLayoutItem);
      delete pCurLayoutItem;
      pCurLayoutItem = pNextLayoutItem;
    }
  }
  pGenerateNode->GetNodeItem(XFA_NODEITEM_Parent)->RemoveChild(pGenerateNode);
}

uint8_t HAlignEnumToInt(XFA_ATTRIBUTEENUM eHAlign) {
  switch (eHAlign) {
    case XFA_ATTRIBUTEENUM_Center:
      return 1;
    case XFA_ATTRIBUTEENUM_Right:
      return 2;
    case XFA_ATTRIBUTEENUM_Left:
    default:
      return 0;
  }
}

XFA_ItemLayoutProcessorResult InsertFlowedItem(
    CXFA_ItemLayoutProcessor* pThis,
    CXFA_ItemLayoutProcessor* pProcessor,
    bool bContainerWidthAutoSize,
    bool bContainerHeightAutoSize,
    FX_FLOAT fContainerHeight,
    XFA_ATTRIBUTEENUM eFlowStrategy,
    uint8_t* uCurHAlignState,
    CFX_ArrayTemplate<CXFA_ContentLayoutItem*> (&rgCurLineLayoutItems)[3],
    bool bUseBreakControl,
    FX_FLOAT fAvailHeight,
    FX_FLOAT fRealHeight,
    FX_FLOAT fContentWidthLimit,
    FX_FLOAT* fContentCurRowY,
    FX_FLOAT* fContentCurRowAvailWidth,
    FX_FLOAT* fContentCurRowHeight,
    bool* bAddedItemInRow,
    bool* bForceEndPage,
    CXFA_LayoutContext* pLayoutContext,
    bool bNewRow) {
  bool bTakeSpace =
      XFA_ItemLayoutProcessor_IsTakingSpace(pProcessor->m_pFormNode);
  uint8_t uHAlign =
      HAlignEnumToInt(pThis->m_pCurChildNode->GetEnum(XFA_ATTRIBUTE_HAlign));
  if (bContainerWidthAutoSize)
    uHAlign = 0;

  if ((eFlowStrategy != XFA_ATTRIBUTEENUM_Rl_tb &&
       uHAlign < *uCurHAlignState) ||
      (eFlowStrategy == XFA_ATTRIBUTEENUM_Rl_tb &&
       uHAlign > *uCurHAlignState)) {
    return XFA_ItemLayoutProcessorResult::RowFullBreak;
  }

  *uCurHAlignState = uHAlign;
  bool bIsOwnSplit =
      pProcessor->m_pFormNode->GetIntact() == XFA_ATTRIBUTEENUM_None;
  bool bUseRealHeight =
      bTakeSpace && bContainerHeightAutoSize && bIsOwnSplit &&
      pProcessor->m_pFormNode->GetNodeItem(XFA_NODEITEM_Parent)->GetIntact() ==
          XFA_ATTRIBUTEENUM_None;
  bool bIsTransHeight = bTakeSpace;
  if (bIsTransHeight && !bIsOwnSplit) {
    bool bRootForceTb = false;
    XFA_ATTRIBUTEENUM eLayoutStrategy =
        GetLayout(pProcessor->m_pFormNode, &bRootForceTb);
    if (eLayoutStrategy == XFA_ATTRIBUTEENUM_Lr_tb ||
        eLayoutStrategy == XFA_ATTRIBUTEENUM_Rl_tb) {
      bIsTransHeight = false;
    }
  }

  bool bUseInherited = false;
  CXFA_LayoutContext layoutContext;
  if (pThis->m_pPageMgr) {
    CXFA_Node* pOverflowNode =
        pThis->m_pPageMgr->QueryOverflow(pThis->m_pFormNode);
    if (pOverflowNode) {
      layoutContext.m_pOverflowNode = pOverflowNode;
      layoutContext.m_pOverflowProcessor = pThis;
      pLayoutContext = &layoutContext;
    }
  }

  XFA_ItemLayoutProcessorResult eRetValue = XFA_ItemLayoutProcessorResult::Done;
  if (!bNewRow ||
      pProcessor->m_ePreProcessRs == XFA_ItemLayoutProcessorResult::Done) {
    eRetValue = pProcessor->DoLayout(
        bTakeSpace ? bUseBreakControl : false,
        bUseRealHeight ? fRealHeight - *fContentCurRowY : FLT_MAX,
        bIsTransHeight ? fRealHeight - *fContentCurRowY : FLT_MAX,
        pLayoutContext);
    pProcessor->m_ePreProcessRs = eRetValue;
  } else {
    eRetValue = pProcessor->m_ePreProcessRs;
    pProcessor->m_ePreProcessRs = XFA_ItemLayoutProcessorResult::Done;
  }
  if (pProcessor->HasLayoutItem() == false)
    return eRetValue;

  CFX_SizeF childSize = pProcessor->GetCurrentComponentSize();
  if (bUseRealHeight && fRealHeight < XFA_LAYOUT_FLOAT_PERCISION) {
    fRealHeight = FLT_MAX;
    fAvailHeight = FLT_MAX;
  }
  if (bTakeSpace && (childSize.width >
                     *fContentCurRowAvailWidth + XFA_LAYOUT_FLOAT_PERCISION) &&
      (fContentWidthLimit - *fContentCurRowAvailWidth >
       XFA_LAYOUT_FLOAT_PERCISION)) {
    return XFA_ItemLayoutProcessorResult::RowFullBreak;
  }

  CXFA_Node* pOverflowLeaderNode = nullptr;
  CXFA_Node* pOverflowTrailerNode = nullptr;
  CXFA_Node* pFormNode = nullptr;
  CXFA_ContentLayoutItem* pTrailerLayoutItem = nullptr;
  bool bIsAddTrailerHeight = false;
  if (pThis->m_pPageMgr &&
      pProcessor->m_pFormNode->GetIntact() == XFA_ATTRIBUTEENUM_None) {
    pFormNode = pThis->m_pPageMgr->QueryOverflow(pProcessor->m_pFormNode);
    if (!pFormNode && pLayoutContext && pLayoutContext->m_pOverflowProcessor) {
      pFormNode = pLayoutContext->m_pOverflowNode;
      bUseInherited = true;
    }
    if (pThis->m_pPageMgr->ProcessOverflow(pFormNode, pOverflowLeaderNode,
                                           pOverflowTrailerNode, false,
                                           false)) {
      if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowTrailerNode)) {
        if (pOverflowTrailerNode) {
          auto pOverflowLeaderProcessor =
              pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(pOverflowTrailerNode,
                                                           nullptr);
          pOverflowLeaderProcessor->DoLayout(false, FLT_MAX, FLT_MAX, nullptr);
          pTrailerLayoutItem =
              pOverflowLeaderProcessor->HasLayoutItem()
                  ? pOverflowLeaderProcessor->ExtractLayoutItem()
                  : nullptr;
        }

        bIsAddTrailerHeight =
            bUseInherited
                ? pThis->IsAddNewRowForTrailer(pTrailerLayoutItem)
                : pProcessor->IsAddNewRowForTrailer(pTrailerLayoutItem);
        if (bIsAddTrailerHeight) {
          childSize.height += pTrailerLayoutItem->m_sSize.height;
          bIsAddTrailerHeight = true;
        }
      }
    }
  }

  if (!bTakeSpace ||
      *fContentCurRowY + childSize.height <=
          fAvailHeight + XFA_LAYOUT_FLOAT_PERCISION ||
      (!bContainerHeightAutoSize &&
       pThis->m_fUsedSize + fAvailHeight + XFA_LAYOUT_FLOAT_PERCISION >=
           fContainerHeight)) {
    if (!bTakeSpace || eRetValue == XFA_ItemLayoutProcessorResult::Done) {
      if (pProcessor->m_bUseInheriated) {
        if (pTrailerLayoutItem)
          AddTrailerBeforeSplit(pProcessor, childSize.height,
                                pTrailerLayoutItem, false);
        if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
          AddPendingNode(pProcessor, pOverflowLeaderNode, false);

        pProcessor->m_bUseInheriated = false;
      } else {
        if (bIsAddTrailerHeight)
          childSize.height -= pTrailerLayoutItem->m_sSize.height;

        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
      }

      CXFA_ContentLayoutItem* pChildLayoutItem =
          pProcessor->ExtractLayoutItem();
      if (ExistContainerKeep(pProcessor->m_pFormNode, false) &&
          pProcessor->m_pFormNode->GetIntact() == XFA_ATTRIBUTEENUM_None) {
        pThis->m_arrayKeepItems.push_back(pChildLayoutItem);
      } else {
        pThis->m_arrayKeepItems.clear();
      }
      rgCurLineLayoutItems[uHAlign].Add(pChildLayoutItem);
      *bAddedItemInRow = true;
      if (bTakeSpace) {
        *fContentCurRowAvailWidth -= childSize.width;
        *fContentCurRowHeight =
            std::max(*fContentCurRowHeight, childSize.height);
      }
      return XFA_ItemLayoutProcessorResult::Done;
    }

    if (eRetValue == XFA_ItemLayoutProcessorResult::PageFullBreak) {
      if (pProcessor->m_bUseInheriated) {
        if (pTrailerLayoutItem) {
          AddTrailerBeforeSplit(pProcessor, childSize.height,
                                pTrailerLayoutItem, false);
        }
        if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
          AddPendingNode(pProcessor, pOverflowLeaderNode, false);

        pProcessor->m_bUseInheriated = false;
      } else {
        if (bIsAddTrailerHeight)
          childSize.height -= pTrailerLayoutItem->m_sSize.height;

        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
      }
    }
    rgCurLineLayoutItems[uHAlign].Add(pProcessor->ExtractLayoutItem());
    *bAddedItemInRow = true;
    *fContentCurRowAvailWidth -= childSize.width;
    *fContentCurRowHeight = std::max(*fContentCurRowHeight, childSize.height);
    return eRetValue;
  }

  XFA_ItemLayoutProcessorResult eResult;
  if (pThis->ProcessKeepForSplit(
          pThis, pProcessor, eRetValue, &rgCurLineLayoutItems[uHAlign],
          fContentCurRowAvailWidth, fContentCurRowHeight, fContentCurRowY,
          bAddedItemInRow, bForceEndPage, &eResult)) {
    return eResult;
  }

  *bForceEndPage = true;
  FX_FLOAT fSplitPos =
      pProcessor->FindSplitPos(fAvailHeight - *fContentCurRowY);
  if (fSplitPos > XFA_LAYOUT_FLOAT_PERCISION) {
    XFA_ATTRIBUTEENUM eLayout =
        pProcessor->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
    if (eLayout == XFA_ATTRIBUTEENUM_Tb &&
        eRetValue == XFA_ItemLayoutProcessorResult::Done) {
      pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                       pOverflowTrailerNode, pTrailerLayoutItem,
                                       pFormNode);
      rgCurLineLayoutItems[uHAlign].Add(pProcessor->ExtractLayoutItem());
      *bAddedItemInRow = true;
      if (bTakeSpace) {
        *fContentCurRowAvailWidth -= childSize.width;
        *fContentCurRowHeight =
            std::max(*fContentCurRowHeight, childSize.height);
      }
      return XFA_ItemLayoutProcessorResult::PageFullBreak;
    }

    CXFA_Node* pTempLeaderNode = nullptr;
    CXFA_Node* pTempTrailerNode = nullptr;
    if (pThis->m_pPageMgr && !pProcessor->m_bUseInheriated &&
        eRetValue != XFA_ItemLayoutProcessorResult::PageFullBreak) {
      pThis->m_pPageMgr->ProcessOverflow(pFormNode, pTempLeaderNode,
                                         pTempTrailerNode, false, true);
    }
    if (pTrailerLayoutItem && bIsAddTrailerHeight) {
      AddTrailerBeforeSplit(pProcessor, fSplitPos, pTrailerLayoutItem,
                            bUseInherited);
    } else {
      pProcessor->SplitLayoutItem(fSplitPos);
    }

    if (bUseInherited) {
      pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                       pOverflowTrailerNode, pTrailerLayoutItem,
                                       pFormNode);
      pThis->m_bUseInheriated = true;
    } else {
      CXFA_LayoutItem* firstChild = pProcessor->m_pLayoutItem->m_pFirstChild;
      if (firstChild && !firstChild->m_pNextSibling &&
          firstChild->m_pFormNode->IsLayoutGeneratedNode()) {
        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
      } else if (pProcessor->JudgeLeaderOrTrailerForOccur(
                     pOverflowLeaderNode)) {
        AddPendingNode(pProcessor, pOverflowLeaderNode, false);
      }
    }

    if (pProcessor->m_pLayoutItem->m_pNextSibling) {
      childSize = pProcessor->GetCurrentComponentSize();
      rgCurLineLayoutItems[uHAlign].Add(pProcessor->ExtractLayoutItem());
      *bAddedItemInRow = true;
      if (bTakeSpace) {
        *fContentCurRowAvailWidth -= childSize.width;
        *fContentCurRowHeight =
            std::max(*fContentCurRowHeight, childSize.height);
      }
    }
    return XFA_ItemLayoutProcessorResult::PageFullBreak;
  }

  if (*fContentCurRowY <= XFA_LAYOUT_FLOAT_PERCISION) {
    childSize = pProcessor->GetCurrentComponentSize();
    if (pProcessor->m_pPageMgr->GetNextAvailContentHeight(childSize.height)) {
      CXFA_Node* pTempLeaderNode = nullptr;
      CXFA_Node* pTempTrailerNode = nullptr;
      if (pThis->m_pPageMgr) {
        if (!pFormNode && pLayoutContext)
          pFormNode = pLayoutContext->m_pOverflowProcessor->m_pFormNode;

        pThis->m_pPageMgr->ProcessOverflow(pFormNode, pTempLeaderNode,
                                           pTempTrailerNode, false, true);
      }
      if (bUseInherited) {
        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
        pThis->m_bUseInheriated = true;
      }
      return XFA_ItemLayoutProcessorResult::PageFullBreak;
    }

    rgCurLineLayoutItems[uHAlign].Add(pProcessor->ExtractLayoutItem());
    *bAddedItemInRow = true;
    if (bTakeSpace) {
      *fContentCurRowAvailWidth -= childSize.width;
      *fContentCurRowHeight = std::max(*fContentCurRowHeight, childSize.height);
    }
    if (eRetValue == XFA_ItemLayoutProcessorResult::Done)
      *bForceEndPage = false;

    return eRetValue;
  }

  XFA_ATTRIBUTEENUM eLayout =
      pProcessor->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
  if (pProcessor->m_pFormNode->GetIntact() == XFA_ATTRIBUTEENUM_None &&
      eLayout == XFA_ATTRIBUTEENUM_Tb) {
    if (pThis->m_pPageMgr) {
      pThis->m_pPageMgr->ProcessOverflow(pFormNode, pOverflowLeaderNode,
                                         pOverflowTrailerNode, false, true);
    }
    if (pTrailerLayoutItem)
      AddTrailerBeforeSplit(pProcessor, fSplitPos, pTrailerLayoutItem, false);
    if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
      AddPendingNode(pProcessor, pOverflowLeaderNode, false);

    return XFA_ItemLayoutProcessorResult::PageFullBreak;
  }

  if (eRetValue != XFA_ItemLayoutProcessorResult::Done)
    return XFA_ItemLayoutProcessorResult::PageFullBreak;

  if (!pFormNode && pLayoutContext)
    pFormNode = pLayoutContext->m_pOverflowProcessor->m_pFormNode;
  if (pThis->m_pPageMgr) {
    pThis->m_pPageMgr->ProcessOverflow(pFormNode, pOverflowLeaderNode,
                                       pOverflowTrailerNode, false, true);
  }
  if (bUseInherited) {
    pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode, pOverflowTrailerNode,
                                     pTrailerLayoutItem, pFormNode);
    pThis->m_bUseInheriated = true;
  }
  return XFA_ItemLayoutProcessorResult::PageFullBreak;
}

bool FindLayoutItemSplitPos(CXFA_ContentLayoutItem* pLayoutItem,
                            FX_FLOAT fCurVerticalOffset,
                            FX_FLOAT* fProposedSplitPos,
                            bool* bAppChange,
                            bool bCalculateMargin) {
  CXFA_Node* pFormNode = pLayoutItem->m_pFormNode;
  if (*fProposedSplitPos <= fCurVerticalOffset + XFA_LAYOUT_FLOAT_PERCISION ||
      *fProposedSplitPos > fCurVerticalOffset + pLayoutItem->m_sSize.height -
                               XFA_LAYOUT_FLOAT_PERCISION) {
    return false;
  }

  switch (pFormNode->GetIntact()) {
    case XFA_ATTRIBUTEENUM_None: {
      bool bAnyChanged = false;
      CXFA_Document* pDocument = pFormNode->GetDocument();
      CXFA_FFNotify* pNotify = pDocument->GetNotify();
      FX_FLOAT fCurTopMargin = 0, fCurBottomMargin = 0;
      CXFA_Node* pMarginNode =
          pFormNode->GetFirstChildByClass(XFA_Element::Margin);
      if (pMarginNode && bCalculateMargin) {
        fCurTopMargin =
            pMarginNode->GetMeasure(XFA_ATTRIBUTE_TopInset).ToUnit(XFA_UNIT_Pt);
        fCurBottomMargin = pMarginNode->GetMeasure(XFA_ATTRIBUTE_BottomInset)
                               .ToUnit(XFA_UNIT_Pt);
      }
      bool bChanged = true;
      while (bChanged) {
        bChanged = false;
        {
          FX_FLOAT fRelSplitPos = *fProposedSplitPos - fCurVerticalOffset;
          if (pNotify->FindSplitPos(pFormNode, pLayoutItem->GetIndex(),
                                    fRelSplitPos)) {
            bAnyChanged = true;
            bChanged = true;
            *fProposedSplitPos = fCurVerticalOffset + fRelSplitPos;
            *bAppChange = true;
            if (*fProposedSplitPos <=
                fCurVerticalOffset + XFA_LAYOUT_FLOAT_PERCISION) {
              return true;
            }
          }
        }
        FX_FLOAT fRelSplitPos = *fProposedSplitPos - fCurBottomMargin;
        for (CXFA_ContentLayoutItem* pChildItem =
                 (CXFA_ContentLayoutItem*)pLayoutItem->m_pFirstChild;
             pChildItem;
             pChildItem = (CXFA_ContentLayoutItem*)pChildItem->m_pNextSibling) {
          FX_FLOAT fChildOffset =
              fCurVerticalOffset + fCurTopMargin + pChildItem->m_sPos.y;
          bool bChange = false;
          if (FindLayoutItemSplitPos(pChildItem, fChildOffset, &fRelSplitPos,
                                     &bChange, bCalculateMargin)) {
            if (fRelSplitPos - fChildOffset < XFA_LAYOUT_FLOAT_PERCISION &&
                bChange) {
              *fProposedSplitPos = fRelSplitPos - fCurTopMargin;
            } else {
              *fProposedSplitPos = fRelSplitPos + fCurBottomMargin;
            }
            bAnyChanged = true;
            bChanged = true;
            if (*fProposedSplitPos <=
                fCurVerticalOffset + XFA_LAYOUT_FLOAT_PERCISION) {
              return true;
            }
            if (bAnyChanged)
              break;
          }
        }
      }
      return bAnyChanged;
    }
    case XFA_ATTRIBUTEENUM_ContentArea:
    case XFA_ATTRIBUTEENUM_PageArea: {
      *fProposedSplitPos = fCurVerticalOffset;
      return true;
    }
    default:
      return false;
  }
}

CFX_PointF CalculatePositionedContainerPos(CXFA_Node* pNode,
                                           const CFX_SizeF& size) {
  XFA_ATTRIBUTEENUM eAnchorType = pNode->GetEnum(XFA_ATTRIBUTE_AnchorType);
  int32_t nAnchorType = 0;
  switch (eAnchorType) {
    case XFA_ATTRIBUTEENUM_TopLeft:
      nAnchorType = 0;
      break;
    case XFA_ATTRIBUTEENUM_TopCenter:
      nAnchorType = 1;
      break;
    case XFA_ATTRIBUTEENUM_TopRight:
      nAnchorType = 2;
      break;
    case XFA_ATTRIBUTEENUM_MiddleLeft:
      nAnchorType = 3;
      break;
    case XFA_ATTRIBUTEENUM_MiddleCenter:
      nAnchorType = 4;
      break;
    case XFA_ATTRIBUTEENUM_MiddleRight:
      nAnchorType = 5;
      break;
    case XFA_ATTRIBUTEENUM_BottomLeft:
      nAnchorType = 6;
      break;
    case XFA_ATTRIBUTEENUM_BottomCenter:
      nAnchorType = 7;
      break;
    case XFA_ATTRIBUTEENUM_BottomRight:
      nAnchorType = 8;
      break;
    default:
      break;
  }
  static const uint8_t nNextPos[4][9] = {{0, 1, 2, 3, 4, 5, 6, 7, 8},
                                         {6, 3, 0, 7, 4, 1, 8, 5, 2},
                                         {8, 7, 6, 5, 4, 3, 2, 1, 0},
                                         {2, 5, 8, 1, 4, 7, 0, 3, 6}};

  CFX_PointF pos(pNode->GetMeasure(XFA_ATTRIBUTE_X).ToUnit(XFA_UNIT_Pt),
                 pNode->GetMeasure(XFA_ATTRIBUTE_Y).ToUnit(XFA_UNIT_Pt));
  int32_t nRotate =
      FXSYS_round(pNode->GetMeasure(XFA_ATTRIBUTE_Rotate).GetValue());
  nRotate = XFA_MapRotation(nRotate) / 90;
  int32_t nAbsoluteAnchorType = nNextPos[nRotate][nAnchorType];
  switch (nAbsoluteAnchorType / 3) {
    case 1:
      pos.y -= size.height / 2;
      break;
    case 2:
      pos.y -= size.height;
      break;
    default:
      break;
  }
  switch (nAbsoluteAnchorType % 3) {
    case 1:
      pos.x -= size.width / 2;
      break;
    case 2:
      pos.x -= size.width;
      break;
    default:
      break;
  }
  return pos;
}

}  // namespace

CXFA_ItemLayoutProcessor::CXFA_ItemLayoutProcessor(CXFA_Node* pNode,
                                                   CXFA_LayoutPageMgr* pPageMgr)
    : m_pFormNode(pNode),
      m_pLayoutItem(nullptr),
      m_pCurChildNode(XFA_LAYOUT_INVALIDNODE),
      m_fUsedSize(0),
      m_pPageMgr(pPageMgr),
      m_bBreakPending(true),
      m_fLastRowWidth(0),
      m_fLastRowY(0),
      m_bUseInheriated(false),
      m_ePreProcessRs(XFA_ItemLayoutProcessorResult::Done),
      m_bKeepBreakFinish(false),
      m_bIsProcessKeep(false),
      m_pKeepHeadNode(nullptr),
      m_pKeepTailNode(nullptr),
      m_pOldLayoutItem(nullptr),
      m_pCurChildPreprocessor(nullptr),
      m_nCurChildNodeStage(XFA_ItemLayoutProcessorStages::None),
      m_fWidthLimite(0),
      m_bHasAvailHeight(true) {
  ASSERT(m_pFormNode && (m_pFormNode->IsContainerNode() ||
                         m_pFormNode->GetElementType() == XFA_Element::Form));
  m_pOldLayoutItem =
      (CXFA_ContentLayoutItem*)m_pFormNode->GetUserData(XFA_LAYOUTITEMKEY);
}

CXFA_ItemLayoutProcessor::~CXFA_ItemLayoutProcessor() {}

CXFA_ContentLayoutItem* CXFA_ItemLayoutProcessor::CreateContentLayoutItem(
    CXFA_Node* pFormNode) {
  if (!pFormNode)
    return nullptr;

  CXFA_ContentLayoutItem* pLayoutItem = nullptr;
  if (m_pOldLayoutItem) {
    pLayoutItem = m_pOldLayoutItem;
    m_pOldLayoutItem = m_pOldLayoutItem->m_pNext;
    return pLayoutItem;
  }
  pLayoutItem = (CXFA_ContentLayoutItem*)pFormNode->GetDocument()
                    ->GetNotify()
                    ->OnCreateLayoutItem(pFormNode);
  CXFA_ContentLayoutItem* pPrevLayoutItem =
      (CXFA_ContentLayoutItem*)pFormNode->GetUserData(XFA_LAYOUTITEMKEY);
  if (pPrevLayoutItem) {
    while (pPrevLayoutItem->m_pNext)
      pPrevLayoutItem = pPrevLayoutItem->m_pNext;

    pPrevLayoutItem->m_pNext = pLayoutItem;
    pLayoutItem->m_pPrev = pPrevLayoutItem;
  } else {
    pFormNode->SetUserData(XFA_LAYOUTITEMKEY, pLayoutItem);
  }
  return pLayoutItem;
}

FX_FLOAT CXFA_ItemLayoutProcessor::FindSplitPos(FX_FLOAT fProposedSplitPos) {
  ASSERT(m_pLayoutItem);
  XFA_ATTRIBUTEENUM eLayout = m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
  bool bCalculateMargin = eLayout != XFA_ATTRIBUTEENUM_Position;
  while (fProposedSplitPos > XFA_LAYOUT_FLOAT_PERCISION) {
    bool bAppChange = false;
    if (!FindLayoutItemSplitPos(m_pLayoutItem, 0, &fProposedSplitPos,
                                &bAppChange, bCalculateMargin)) {
      break;
    }
  }
  return fProposedSplitPos;
}

void CXFA_ItemLayoutProcessor::SplitLayoutItem(
    CXFA_ContentLayoutItem* pLayoutItem,
    CXFA_ContentLayoutItem* pSecondParent,
    FX_FLOAT fSplitPos) {
  FX_FLOAT fCurTopMargin = 0, fCurBottomMargin = 0;
  XFA_ATTRIBUTEENUM eLayout = m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
  bool bCalculateMargin = true;
  if (eLayout == XFA_ATTRIBUTEENUM_Position)
    bCalculateMargin = false;

  CXFA_Node* pMarginNode =
      pLayoutItem->m_pFormNode->GetFirstChildByClass(XFA_Element::Margin);
  if (pMarginNode && bCalculateMargin) {
    fCurTopMargin =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_TopInset).ToUnit(XFA_UNIT_Pt);
    fCurBottomMargin =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_BottomInset).ToUnit(XFA_UNIT_Pt);
  }

  CXFA_ContentLayoutItem* pSecondLayoutItem = nullptr;
  if (m_pCurChildPreprocessor &&
      m_pCurChildPreprocessor->m_pFormNode == pLayoutItem->m_pFormNode) {
    pSecondLayoutItem = m_pCurChildPreprocessor->CreateContentLayoutItem(
        pLayoutItem->m_pFormNode);
  } else {
    pSecondLayoutItem = CreateContentLayoutItem(pLayoutItem->m_pFormNode);
  }
  pSecondLayoutItem->m_sPos.x = pLayoutItem->m_sPos.x;
  pSecondLayoutItem->m_sSize.width = pLayoutItem->m_sSize.width;
  pSecondLayoutItem->m_sPos.y = 0;
  pSecondLayoutItem->m_sSize.height = pLayoutItem->m_sSize.height - fSplitPos;
  pLayoutItem->m_sSize.height -= pSecondLayoutItem->m_sSize.height;
  if (pLayoutItem->m_pFirstChild)
    pSecondLayoutItem->m_sSize.height += fCurTopMargin;

  if (pSecondParent) {
    pSecondParent->AddChild(pSecondLayoutItem);
    if (fCurTopMargin > 0 && pLayoutItem->m_pFirstChild) {
      pSecondParent->m_sSize.height += fCurTopMargin;
      CXFA_ContentLayoutItem* pParentItem =
          (CXFA_ContentLayoutItem*)pSecondParent->m_pParent;
      while (pParentItem) {
        pParentItem->m_sSize.height += fCurTopMargin;
        pParentItem = (CXFA_ContentLayoutItem*)pParentItem->m_pParent;
      }
    }
  } else {
    pSecondLayoutItem->m_pParent = pLayoutItem->m_pParent;
    pSecondLayoutItem->m_pNextSibling = pLayoutItem->m_pNextSibling;
    pLayoutItem->m_pNextSibling = pSecondLayoutItem;
  }

  CXFA_ContentLayoutItem* pChildren =
      (CXFA_ContentLayoutItem*)pLayoutItem->m_pFirstChild;
  pLayoutItem->m_pFirstChild = nullptr;
  FX_FLOAT lHeightForKeep = 0;
  CFX_ArrayTemplate<CXFA_ContentLayoutItem*> keepLayoutItems;
  FX_FLOAT fAddMarginHeight = 0;
  for (CXFA_ContentLayoutItem *pChildItem = pChildren, *pChildNext = nullptr;
       pChildItem; pChildItem = pChildNext) {
    pChildNext = (CXFA_ContentLayoutItem*)pChildItem->m_pNextSibling;
    pChildItem->m_pNextSibling = nullptr;
    if (fSplitPos <= fCurTopMargin + pChildItem->m_sPos.y + fCurBottomMargin +
                         XFA_LAYOUT_FLOAT_PERCISION) {
      if (!ExistContainerKeep(pChildItem->m_pFormNode, true)) {
        pChildItem->m_sPos.y -= fSplitPos - fCurBottomMargin;
        pChildItem->m_sPos.y += lHeightForKeep;
        pChildItem->m_sPos.y += fAddMarginHeight;
        pSecondLayoutItem->AddChild(pChildItem);
        continue;
      }

      if (lHeightForKeep < XFA_LAYOUT_FLOAT_PERCISION) {
        for (int32_t iIndex = 0; iIndex < keepLayoutItems.GetSize(); iIndex++) {
          CXFA_ContentLayoutItem* pPreItem = keepLayoutItems[iIndex];
          pLayoutItem->RemoveChild(pPreItem);
          pPreItem->m_sPos.y -= fSplitPos;
          if (pPreItem->m_sPos.y < 0)
            pPreItem->m_sPos.y = 0;

          if (pPreItem->m_sPos.y + pPreItem->m_sSize.height > lHeightForKeep) {
            pPreItem->m_sPos.y = lHeightForKeep;
            lHeightForKeep += pPreItem->m_sSize.height;
            pSecondLayoutItem->m_sSize.height += pPreItem->m_sSize.height;
            if (pSecondParent)
              pSecondParent->m_sSize.height += pPreItem->m_sSize.height;
          }
          pSecondLayoutItem->AddChild(pPreItem);
        }
      }

      pChildItem->m_sPos.y -= fSplitPos;
      pChildItem->m_sPos.y += lHeightForKeep;
      pChildItem->m_sPos.y += fAddMarginHeight;
      pSecondLayoutItem->AddChild(pChildItem);
      continue;
    }

    if (fSplitPos + XFA_LAYOUT_FLOAT_PERCISION >=
        fCurTopMargin + fCurBottomMargin + pChildItem->m_sPos.y +
            pChildItem->m_sSize.height) {
      pLayoutItem->AddChild(pChildItem);
      if (ExistContainerKeep(pChildItem->m_pFormNode, false))
        keepLayoutItems.Add(pChildItem);
      else
        keepLayoutItems.RemoveAll();

      continue;
    }

    FX_FLOAT fOldHeight = pSecondLayoutItem->m_sSize.height;
    SplitLayoutItem(
        pChildItem, pSecondLayoutItem,
        fSplitPos - fCurTopMargin - fCurBottomMargin - pChildItem->m_sPos.y);
    fAddMarginHeight = pSecondLayoutItem->m_sSize.height - fOldHeight;
    pLayoutItem->AddChild(pChildItem);
  }
}

void CXFA_ItemLayoutProcessor::SplitLayoutItem(FX_FLOAT fSplitPos) {
  ASSERT(m_pLayoutItem);
  SplitLayoutItem(m_pLayoutItem, nullptr, fSplitPos);
}

CXFA_ContentLayoutItem* CXFA_ItemLayoutProcessor::ExtractLayoutItem() {
  CXFA_ContentLayoutItem* pLayoutItem = m_pLayoutItem;
  if (pLayoutItem) {
    m_pLayoutItem =
        static_cast<CXFA_ContentLayoutItem*>(pLayoutItem->m_pNextSibling);
    pLayoutItem->m_pNextSibling = nullptr;
  }

  if (m_nCurChildNodeStage != XFA_ItemLayoutProcessorStages::Done ||
      !ToContentLayoutItem(m_pOldLayoutItem)) {
    return pLayoutItem;
  }

  if (m_pOldLayoutItem->m_pPrev)
    m_pOldLayoutItem->m_pPrev->m_pNext = nullptr;

  CXFA_FFNotify* pNotify =
      m_pOldLayoutItem->m_pFormNode->GetDocument()->GetNotify();
  CXFA_LayoutProcessor* pDocLayout =
      m_pOldLayoutItem->m_pFormNode->GetDocument()->GetDocLayout();
  CXFA_ContentLayoutItem* pOldLayoutItem = m_pOldLayoutItem;
  while (pOldLayoutItem) {
    CXFA_ContentLayoutItem* pNextOldLayoutItem = pOldLayoutItem->m_pNext;
    pNotify->OnLayoutItemRemoving(pDocLayout, pOldLayoutItem);
    if (pOldLayoutItem->m_pParent)
      pOldLayoutItem->m_pParent->RemoveChild(pOldLayoutItem);

    delete pOldLayoutItem;
    pOldLayoutItem = pNextOldLayoutItem;
  }
  m_pOldLayoutItem = nullptr;
  return pLayoutItem;
}

void CXFA_ItemLayoutProcessor::GotoNextContainerNode(
    CXFA_Node*& pCurActionNode,
    XFA_ItemLayoutProcessorStages& nCurStage,
    CXFA_Node* pParentContainer,
    bool bUsePageBreak) {
  CXFA_Node* pEntireContainer = pParentContainer;
  CXFA_Node* pChildContainer = XFA_LAYOUT_INVALIDNODE;
  switch (nCurStage) {
    case XFA_ItemLayoutProcessorStages::BreakBefore:
    case XFA_ItemLayoutProcessorStages::BreakAfter: {
      pChildContainer = pCurActionNode->GetNodeItem(XFA_NODEITEM_Parent);
      break;
    }
    case XFA_ItemLayoutProcessorStages::Keep:
    case XFA_ItemLayoutProcessorStages::Container:
      pChildContainer = pCurActionNode;
      break;
    default:
      pChildContainer = XFA_LAYOUT_INVALIDNODE;
      break;
  }

  switch (nCurStage) {
    case XFA_ItemLayoutProcessorStages::Keep: {
      CXFA_Node* pBreakAfterNode =
          pChildContainer->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!m_bKeepBreakFinish &&
          FindBreakNode(pBreakAfterNode, pCurActionNode, &nCurStage, false)) {
        return;
      }
      goto CheckNextChildContainer;
    }
    case XFA_ItemLayoutProcessorStages::None: {
      pCurActionNode = XFA_LAYOUT_INVALIDNODE;
      case XFA_ItemLayoutProcessorStages::BookendLeader:
        for (CXFA_Node* pBookendNode =
                 pCurActionNode == XFA_LAYOUT_INVALIDNODE
                     ? pEntireContainer->GetNodeItem(XFA_NODEITEM_FirstChild)
                     : pCurActionNode->GetNodeItem(XFA_NODEITEM_NextSibling);
             pBookendNode; pBookendNode = pBookendNode->GetNodeItem(
                               XFA_NODEITEM_NextSibling)) {
          switch (pBookendNode->GetElementType()) {
            case XFA_Element::Bookend:
            case XFA_Element::Break:
              pCurActionNode = pBookendNode;
              nCurStage = XFA_ItemLayoutProcessorStages::BookendLeader;
              return;
            default:
              break;
          }
        }
    }
      {
        pCurActionNode = XFA_LAYOUT_INVALIDNODE;
        case XFA_ItemLayoutProcessorStages::BreakBefore:
          if (pCurActionNode != XFA_LAYOUT_INVALIDNODE) {
            CXFA_Node* pBreakBeforeNode =
                pCurActionNode->GetNodeItem(XFA_NODEITEM_NextSibling);
            if (!m_bKeepBreakFinish &&
                FindBreakNode(pBreakBeforeNode, pCurActionNode, &nCurStage,
                              true)) {
              return;
            }
            if (m_bIsProcessKeep) {
              if (ProcessKeepNodesForBreakBefore(pCurActionNode, nCurStage,
                                                 pChildContainer)) {
                return;
              }
              goto CheckNextChildContainer;
            }
            pCurActionNode = pChildContainer;
            nCurStage = XFA_ItemLayoutProcessorStages::Container;
            return;
          }
          goto CheckNextChildContainer;
      }
    case XFA_ItemLayoutProcessorStages::Container: {
      pCurActionNode = XFA_LAYOUT_INVALIDNODE;
      case XFA_ItemLayoutProcessorStages::BreakAfter: {
        if (pCurActionNode == XFA_LAYOUT_INVALIDNODE) {
          CXFA_Node* pBreakAfterNode =
              pChildContainer->GetNodeItem(XFA_NODEITEM_FirstChild);
          if (!m_bKeepBreakFinish &&
              FindBreakNode(pBreakAfterNode, pCurActionNode, &nCurStage,
                            false)) {
            return;
          }
        } else {
          CXFA_Node* pBreakAfterNode =
              pCurActionNode->GetNodeItem(XFA_NODEITEM_NextSibling);
          if (FindBreakNode(pBreakAfterNode, pCurActionNode, &nCurStage,
                            false)) {
            return;
          }
        }
        goto CheckNextChildContainer;
      }
    }

    CheckNextChildContainer : {
      CXFA_Node* pNextChildContainer =
          pChildContainer == XFA_LAYOUT_INVALIDNODE
              ? pEntireContainer->GetNodeItem(XFA_NODEITEM_FirstChild,
                                              XFA_ObjectType::ContainerNode)
              : pChildContainer->GetNodeItem(XFA_NODEITEM_NextSibling,
                                             XFA_ObjectType::ContainerNode);
      while (pNextChildContainer &&
             pNextChildContainer->IsLayoutGeneratedNode()) {
        CXFA_Node* pSaveNode = pNextChildContainer;
        pNextChildContainer = pNextChildContainer->GetNodeItem(
            XFA_NODEITEM_NextSibling, XFA_ObjectType::ContainerNode);
        if (pSaveNode->IsUnusedNode())
          DeleteLayoutGeneratedNode(pSaveNode);
      }
      if (!pNextChildContainer)
        goto NoMoreChildContainer;

      bool bLastKeep = false;
      if (ProcessKeepNodesForCheckNext(pCurActionNode, nCurStage,
                                       pNextChildContainer, bLastKeep)) {
        return;
      }
      if (!m_bKeepBreakFinish && !bLastKeep &&
          FindBreakNode(
              pNextChildContainer->GetNodeItem(XFA_NODEITEM_FirstChild),
              pCurActionNode, &nCurStage, true)) {
        return;
      }
      pCurActionNode = pNextChildContainer;
      if (m_bIsProcessKeep)
        nCurStage = XFA_ItemLayoutProcessorStages::Keep;
      else
        nCurStage = XFA_ItemLayoutProcessorStages::Container;
      return;
    }

    NoMoreChildContainer : {
      pCurActionNode = XFA_LAYOUT_INVALIDNODE;
      case XFA_ItemLayoutProcessorStages::BookendTrailer:
        for (CXFA_Node* pBookendNode =
                 pCurActionNode == XFA_LAYOUT_INVALIDNODE
                     ? pEntireContainer->GetNodeItem(XFA_NODEITEM_FirstChild)
                     : pCurActionNode->GetNodeItem(XFA_NODEITEM_NextSibling);
             pBookendNode; pBookendNode = pBookendNode->GetNodeItem(
                               XFA_NODEITEM_NextSibling)) {
          switch (pBookendNode->GetElementType()) {
            case XFA_Element::Bookend:
            case XFA_Element::Break:
              pCurActionNode = pBookendNode;
              nCurStage = XFA_ItemLayoutProcessorStages::BookendTrailer;
              return;
            default:
              break;
          }
        }
    }
    default:
      pCurActionNode = nullptr;
      nCurStage = XFA_ItemLayoutProcessorStages::Done;
  }
}

bool CXFA_ItemLayoutProcessor::ProcessKeepNodesForCheckNext(
    CXFA_Node*& pCurActionNode,
    XFA_ItemLayoutProcessorStages& nCurStage,
    CXFA_Node*& pNextContainer,
    bool& bLastKeepNode) {
  const bool bCanSplit = pNextContainer->GetIntact() == XFA_ATTRIBUTEENUM_None;
  bool bNextKeep = false;
  if (ExistContainerKeep(pNextContainer, false))
    bNextKeep = true;

  if (bNextKeep && !bCanSplit) {
    if (!m_bIsProcessKeep && !m_bKeepBreakFinish) {
      m_pKeepHeadNode = pNextContainer;
      m_bIsProcessKeep = true;
    }
    return false;
  }

  if (m_bIsProcessKeep && m_pKeepHeadNode) {
    m_pKeepTailNode = pNextContainer;
    if (!m_bKeepBreakFinish &&
        FindBreakNode(pNextContainer->GetNodeItem(XFA_NODEITEM_FirstChild),
                      pCurActionNode, &nCurStage, true)) {
      return true;
    }

    pNextContainer = m_pKeepHeadNode;
    m_bKeepBreakFinish = true;
    m_pKeepHeadNode = nullptr;
    m_pKeepTailNode = nullptr;
    m_bIsProcessKeep = false;
  } else {
    if (m_bKeepBreakFinish)
      bLastKeepNode = true;
    m_bKeepBreakFinish = false;
  }

  return false;
}

bool CXFA_ItemLayoutProcessor::ProcessKeepNodesForBreakBefore(
    CXFA_Node*& pCurActionNode,
    XFA_ItemLayoutProcessorStages& nCurStage,
    CXFA_Node* pContainerNode) {
  if (m_pKeepTailNode == pContainerNode) {
    pCurActionNode = m_pKeepHeadNode;
    m_bKeepBreakFinish = true;
    m_pKeepHeadNode = nullptr;
    m_pKeepTailNode = nullptr;
    m_bIsProcessKeep = false;
    nCurStage = XFA_ItemLayoutProcessorStages::Container;
    return true;
  }

  CXFA_Node* pBreakAfterNode =
      pContainerNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  return FindBreakNode(pBreakAfterNode, pCurActionNode, &nCurStage, false);
}

bool XFA_ItemLayoutProcessor_IsTakingSpace(CXFA_Node* pNode) {
  XFA_ATTRIBUTEENUM ePresence = pNode->GetEnum(XFA_ATTRIBUTE_Presence);
  return ePresence == XFA_ATTRIBUTEENUM_Visible ||
         ePresence == XFA_ATTRIBUTEENUM_Invisible;
}

bool CXFA_ItemLayoutProcessor::IncrementRelayoutNode(
    CXFA_LayoutProcessor* pLayoutProcessor,
    CXFA_Node* pNode,
    CXFA_Node* pParentNode) {
  return false;
}

void CXFA_ItemLayoutProcessor::DoLayoutPageArea(
    CXFA_ContainerLayoutItem* pPageAreaLayoutItem) {
  CXFA_Node* pFormNode = pPageAreaLayoutItem->m_pFormNode;
  CXFA_Node* pCurChildNode = XFA_LAYOUT_INVALIDNODE;
  XFA_ItemLayoutProcessorStages nCurChildNodeStage =
      XFA_ItemLayoutProcessorStages::None;
  CXFA_LayoutItem* pBeforeItem = nullptr;
  for (GotoNextContainerNode(pCurChildNode, nCurChildNodeStage, pFormNode,
                             false);
       pCurChildNode; GotoNextContainerNode(pCurChildNode, nCurChildNodeStage,
                                            pFormNode, false)) {
    if (nCurChildNodeStage != XFA_ItemLayoutProcessorStages::Container)
      continue;
    if (pCurChildNode->GetElementType() == XFA_Element::Variables)
      continue;

    auto pProcessor =
        pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(pCurChildNode, nullptr);
    pProcessor->DoLayout(false, FLT_MAX, FLT_MAX, nullptr);
    if (!pProcessor->HasLayoutItem())
      continue;

    pProcessor->SetCurrentComponentPos(CalculatePositionedContainerPos(
        pCurChildNode, pProcessor->GetCurrentComponentSize()));
    CXFA_LayoutItem* pProcessItem = pProcessor->ExtractLayoutItem();
    if (!pBeforeItem)
      pPageAreaLayoutItem->AddHeadChild(pProcessItem);
    else
      pPageAreaLayoutItem->InsertChild(pBeforeItem, pProcessItem);

    pBeforeItem = pProcessItem;
  }

  pBeforeItem = nullptr;
  CXFA_LayoutItem* pLayoutItem = pPageAreaLayoutItem->m_pFirstChild;
  while (pLayoutItem) {
    if (!pLayoutItem->IsContentLayoutItem() ||
        pLayoutItem->m_pFormNode->GetElementType() != XFA_Element::Draw) {
      pLayoutItem = pLayoutItem->m_pNextSibling;
      continue;
    }
    if (pLayoutItem->m_pFormNode->GetElementType() != XFA_Element::Draw)
      continue;

    CXFA_LayoutItem* pNextLayoutItem = pLayoutItem->m_pNextSibling;
    pPageAreaLayoutItem->RemoveChild(pLayoutItem);
    if (!pBeforeItem)
      pPageAreaLayoutItem->AddHeadChild(pLayoutItem);
    else
      pPageAreaLayoutItem->InsertChild(pBeforeItem, pLayoutItem);

    pBeforeItem = pLayoutItem;
    pLayoutItem = pNextLayoutItem;
  }
}

void CXFA_ItemLayoutProcessor::DoLayoutPositionedContainer(
    CXFA_LayoutContext* pContext) {
  if (m_pLayoutItem)
    return;

  m_pLayoutItem = CreateContentLayoutItem(m_pFormNode);
  bool bIgnoreXY = (m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout) !=
                    XFA_ATTRIBUTEENUM_Position);
  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      m_pFormNode, &bContainerWidthAutoSize, &bContainerHeightAutoSize);

  FX_FLOAT fContentCalculatedWidth = 0;
  FX_FLOAT fContentCalculatedHeight = 0;
  FX_FLOAT fHiddenContentCalculatedWidth = 0;
  FX_FLOAT fHiddenContentCalculatedHeight = 0;
  if (m_pCurChildNode == XFA_LAYOUT_INVALIDNODE) {
    GotoNextContainerNode(m_pCurChildNode, m_nCurChildNodeStage, m_pFormNode,
                          false);
  }

  int32_t iColIndex = 0;
  for (; m_pCurChildNode; GotoNextContainerNode(
           m_pCurChildNode, m_nCurChildNodeStage, m_pFormNode, false)) {
    if (m_nCurChildNodeStage != XFA_ItemLayoutProcessorStages::Container)
      continue;
    if (m_pCurChildNode->GetElementType() == XFA_Element::Variables)
      continue;

    auto pProcessor = pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(
        m_pCurChildNode, m_pPageMgr);
    if (pContext && pContext->m_prgSpecifiedColumnWidths) {
      int32_t iColSpan = m_pCurChildNode->GetInteger(XFA_ATTRIBUTE_ColSpan);
      if (iColSpan <=
          pContext->m_prgSpecifiedColumnWidths->GetSize() - iColIndex) {
        pContext->m_fCurColumnWidth = 0;
        pContext->m_bCurColumnWidthAvaiable = true;
        if (iColSpan == -1)
          iColSpan = pContext->m_prgSpecifiedColumnWidths->GetSize();

        for (int32_t i = 0; iColIndex + i < iColSpan; ++i) {
          pContext->m_fCurColumnWidth +=
              pContext->m_prgSpecifiedColumnWidths->GetAt(iColIndex + i);
        }
        if (pContext->m_fCurColumnWidth == 0)
          pContext->m_bCurColumnWidthAvaiable = false;

        iColIndex += iColSpan >= 0 ? iColSpan : 0;
      }
    }

    pProcessor->DoLayout(false, FLT_MAX, FLT_MAX, pContext);
    if (!pProcessor->HasLayoutItem())
      continue;

    CFX_SizeF size = pProcessor->GetCurrentComponentSize();
    bool bChangeParentSize = false;
    if (XFA_ItemLayoutProcessor_IsTakingSpace(m_pCurChildNode))
      bChangeParentSize = true;

    CFX_PointF absolutePos;
    if (!bIgnoreXY)
      absolutePos = CalculatePositionedContainerPos(m_pCurChildNode, size);

    pProcessor->SetCurrentComponentPos(absolutePos);
    if (bContainerWidthAutoSize) {
      FX_FLOAT fChildSuppliedWidth = absolutePos.x + size.width;
      if (bChangeParentSize) {
        fContentCalculatedWidth =
            std::max(fContentCalculatedWidth, fChildSuppliedWidth);
      } else {
        if (fHiddenContentCalculatedWidth < fChildSuppliedWidth &&
            m_pCurChildNode->GetElementType() != XFA_Element::Subform) {
          fHiddenContentCalculatedWidth = fChildSuppliedWidth;
        }
      }
    }

    if (bContainerHeightAutoSize) {
      FX_FLOAT fChildSuppliedHeight = absolutePos.y + size.height;
      if (bChangeParentSize) {
        fContentCalculatedHeight =
            std::max(fContentCalculatedHeight, fChildSuppliedHeight);
      } else {
        if (fHiddenContentCalculatedHeight < fChildSuppliedHeight &&
            m_pCurChildNode->GetElementType() != XFA_Element::Subform) {
          fHiddenContentCalculatedHeight = fChildSuppliedHeight;
        }
      }
    }
    m_pLayoutItem->AddChild(pProcessor->ExtractLayoutItem());
  }

  XFA_VERSION eVersion = m_pFormNode->GetDocument()->GetCurVersionMode();
  if (fContentCalculatedWidth == 0 && eVersion < XFA_VERSION_207)
    fContentCalculatedWidth = fHiddenContentCalculatedWidth;
  if (fContentCalculatedHeight == 0 && eVersion < XFA_VERSION_207)
    fContentCalculatedHeight = fHiddenContentCalculatedHeight;

  containerSize = CalculateContainerComponentSizeFromContentSize(
      m_pFormNode, bContainerWidthAutoSize, fContentCalculatedWidth,
      bContainerHeightAutoSize, fContentCalculatedHeight, containerSize);
  SetCurrentComponentSize(containerSize);
}

void CXFA_ItemLayoutProcessor::DoLayoutTableContainer(CXFA_Node* pLayoutNode) {
  if (m_pLayoutItem)
    return;
  if (!pLayoutNode)
    pLayoutNode = m_pFormNode;

  ASSERT(m_pCurChildNode == XFA_LAYOUT_INVALIDNODE);

  m_pLayoutItem = CreateContentLayoutItem(m_pFormNode);
  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      m_pFormNode, &bContainerWidthAutoSize, &bContainerHeightAutoSize);
  FX_FLOAT fContentCalculatedWidth = 0;
  FX_FLOAT fContentCalculatedHeight = 0;
  CXFA_Node* pMarginNode =
      m_pFormNode->GetFirstChildByClass(XFA_Element::Margin);
  FX_FLOAT fLeftInset = 0;
  FX_FLOAT fRightInset = 0;
  if (pMarginNode) {
    fLeftInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_LeftInset).ToUnit(XFA_UNIT_Pt);
    fRightInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_RightInset).ToUnit(XFA_UNIT_Pt);
  }

  FX_FLOAT fContentWidthLimit =
      bContainerWidthAutoSize ? FLT_MAX
                              : containerSize.width - fLeftInset - fRightInset;
  CFX_WideStringC wsColumnWidths;
  if (pLayoutNode->TryCData(XFA_ATTRIBUTE_ColumnWidths, wsColumnWidths)) {
    auto widths = SeparateStringW(wsColumnWidths.c_str(),
                                  wsColumnWidths.GetLength(), L' ');
    for (auto& width : widths) {
      width.TrimLeft(L' ');
      if (width.IsEmpty())
        continue;

      CXFA_Measurement measure(width.AsStringC());
      m_rgSpecifiedColumnWidths.Add(measure.ToUnit(XFA_UNIT_Pt));
    }
  }

  int32_t iSpecifiedColumnCount = m_rgSpecifiedColumnWidths.GetSize();
  CXFA_LayoutContext layoutContext;
  layoutContext.m_prgSpecifiedColumnWidths = &m_rgSpecifiedColumnWidths;
  CXFA_LayoutContext* pLayoutContext =
      iSpecifiedColumnCount > 0 ? &layoutContext : nullptr;
  if (m_pCurChildNode == XFA_LAYOUT_INVALIDNODE) {
    GotoNextContainerNode(m_pCurChildNode, m_nCurChildNodeStage, m_pFormNode,
                          false);
  }

  for (; m_pCurChildNode; GotoNextContainerNode(
           m_pCurChildNode, m_nCurChildNodeStage, m_pFormNode, false)) {
    layoutContext.m_bCurColumnWidthAvaiable = false;
    layoutContext.m_fCurColumnWidth = 0;
    if (m_nCurChildNodeStage != XFA_ItemLayoutProcessorStages::Container)
      continue;

    auto pProcessor = pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(
        m_pCurChildNode, m_pPageMgr);
    pProcessor->DoLayout(false, FLT_MAX, FLT_MAX, pLayoutContext);
    if (!pProcessor->HasLayoutItem())
      continue;

    m_pLayoutItem->AddChild(pProcessor->ExtractLayoutItem());
  }

  int32_t iRowCount = 0;
  int32_t iColCount = 0;
  {
    CFX_ArrayTemplate<CXFA_ContentLayoutItem*> rgRowItems;
    CFX_ArrayTemplate<int32_t> rgRowItemsSpan;
    CFX_ArrayTemplate<FX_FLOAT> rgRowItemsWidth;
    for (CXFA_ContentLayoutItem* pLayoutChild =
             (CXFA_ContentLayoutItem*)m_pLayoutItem->m_pFirstChild;
         pLayoutChild;
         pLayoutChild = (CXFA_ContentLayoutItem*)pLayoutChild->m_pNextSibling) {
      if (pLayoutChild->m_pFormNode->GetElementType() != XFA_Element::Subform)
        continue;
      if (!XFA_ItemLayoutProcessor_IsTakingSpace(pLayoutChild->m_pFormNode))
        continue;

      XFA_ATTRIBUTEENUM eLayout =
          pLayoutChild->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
      if (eLayout != XFA_ATTRIBUTEENUM_Row &&
          eLayout != XFA_ATTRIBUTEENUM_Rl_row) {
        continue;
      }
      if (CXFA_ContentLayoutItem* pRowLayoutCell =
              (CXFA_ContentLayoutItem*)pLayoutChild->m_pFirstChild) {
        rgRowItems.Add(pRowLayoutCell);
        int32_t iColSpan =
            pRowLayoutCell->m_pFormNode->GetInteger(XFA_ATTRIBUTE_ColSpan);
        rgRowItemsSpan.Add(iColSpan);
        rgRowItemsWidth.Add(pRowLayoutCell->m_sSize.width);
      }
    }

    iRowCount = rgRowItems.GetSize();
    iColCount = 0;
    bool bMoreColumns = true;
    while (bMoreColumns) {
      bMoreColumns = false;
      bool bAutoCol = false;
      for (int32_t i = 0; i < iRowCount; i++) {
        while (rgRowItems[i] && (rgRowItemsSpan[i] <= 0 ||
                                 !XFA_ItemLayoutProcessor_IsTakingSpace(
                                     rgRowItems[i]->m_pFormNode))) {
          CXFA_ContentLayoutItem* pNewCell =
              (CXFA_ContentLayoutItem*)rgRowItems[i]->m_pNextSibling;
          if (rgRowItemsSpan[i] < 0 && XFA_ItemLayoutProcessor_IsTakingSpace(
                                           rgRowItems[i]->m_pFormNode)) {
            pNewCell = nullptr;
          }
          rgRowItems[i] = pNewCell;
          rgRowItemsSpan[i] =
              pNewCell
                  ? pNewCell->m_pFormNode->GetInteger(XFA_ATTRIBUTE_ColSpan)
                  : 0;
          rgRowItemsWidth[i] = pNewCell ? pNewCell->m_sSize.width : 0;
        }
        CXFA_ContentLayoutItem* pCell = rgRowItems[i];
        if (!pCell)
          continue;

        bMoreColumns = true;
        if (rgRowItemsSpan[i] != 1)
          continue;

        if (iColCount >= iSpecifiedColumnCount) {
          int32_t c = iColCount + 1 - m_rgSpecifiedColumnWidths.GetSize();
          for (int32_t j = 0; j < c; j++)
            m_rgSpecifiedColumnWidths.Add(0);
        }
        if (m_rgSpecifiedColumnWidths[iColCount] < XFA_LAYOUT_FLOAT_PERCISION)
          bAutoCol = true;
        if (bAutoCol &&
            m_rgSpecifiedColumnWidths[iColCount] < rgRowItemsWidth[i]) {
          m_rgSpecifiedColumnWidths[iColCount] = rgRowItemsWidth[i];
        }
      }

      if (!bMoreColumns)
        continue;

      FX_FLOAT fFinalColumnWidth = 0.0f;
      if (iColCount < m_rgSpecifiedColumnWidths.GetSize())
        fFinalColumnWidth = m_rgSpecifiedColumnWidths[iColCount];
      for (int32_t i = 0; i < iRowCount; ++i) {
        if (!rgRowItems[i])
          continue;
        --rgRowItemsSpan[i];
        rgRowItemsWidth[i] -= fFinalColumnWidth;
      }
      ++iColCount;
    }
  }

  FX_FLOAT fCurrentRowY = 0;
  for (CXFA_ContentLayoutItem* pLayoutChild =
           (CXFA_ContentLayoutItem*)m_pLayoutItem->m_pFirstChild;
       pLayoutChild;
       pLayoutChild = (CXFA_ContentLayoutItem*)pLayoutChild->m_pNextSibling) {
    if (!XFA_ItemLayoutProcessor_IsTakingSpace(pLayoutChild->m_pFormNode))
      continue;

    if (pLayoutChild->m_pFormNode->GetElementType() == XFA_Element::Subform) {
      XFA_ATTRIBUTEENUM eSubformLayout =
          pLayoutChild->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
      if (eSubformLayout == XFA_ATTRIBUTEENUM_Row ||
          eSubformLayout == XFA_ATTRIBUTEENUM_Rl_row) {
        RelocateTableRowCells(pLayoutChild, m_rgSpecifiedColumnWidths,
                              eSubformLayout);
      }
    }

    pLayoutChild->m_sPos.y = fCurrentRowY;
    if (bContainerWidthAutoSize) {
      pLayoutChild->m_sPos.x = 0;
    } else {
      switch (pLayoutChild->m_pFormNode->GetEnum(XFA_ATTRIBUTE_HAlign)) {
        case XFA_ATTRIBUTEENUM_Center:
          pLayoutChild->m_sPos.x =
              (fContentWidthLimit - pLayoutChild->m_sSize.width) / 2;
          break;
        case XFA_ATTRIBUTEENUM_Right:
          pLayoutChild->m_sPos.x =
              fContentWidthLimit - pLayoutChild->m_sSize.width;
          break;
        case XFA_ATTRIBUTEENUM_Left:
        default:
          pLayoutChild->m_sPos.x = 0;
          break;
      }
    }

    if (bContainerWidthAutoSize) {
      FX_FLOAT fChildSuppliedWidth =
          pLayoutChild->m_sPos.x + pLayoutChild->m_sSize.width;
      if (fContentWidthLimit < FLT_MAX &&
          fContentWidthLimit > fChildSuppliedWidth) {
        fChildSuppliedWidth = fContentWidthLimit;
      }
      fContentCalculatedWidth =
          std::max(fContentCalculatedWidth, fChildSuppliedWidth);
    }
    fCurrentRowY += pLayoutChild->m_sSize.height;
  }

  if (bContainerHeightAutoSize)
    fContentCalculatedHeight = std::max(fContentCalculatedHeight, fCurrentRowY);

  containerSize = CalculateContainerComponentSizeFromContentSize(
      m_pFormNode, bContainerWidthAutoSize, fContentCalculatedWidth,
      bContainerHeightAutoSize, fContentCalculatedHeight, containerSize);
  SetCurrentComponentSize(containerSize);
}

bool CXFA_ItemLayoutProcessor::IsAddNewRowForTrailer(
    CXFA_ContentLayoutItem* pTrailerItem) {
  if (!pTrailerItem)
    return false;

  FX_FLOAT fWidth = pTrailerItem->m_sSize.width;
  XFA_ATTRIBUTEENUM eLayout = m_pFormNode->GetEnum(XFA_ATTRIBUTE_Layout);
  return eLayout == XFA_ATTRIBUTEENUM_Tb || m_fWidthLimite <= fWidth;
}

FX_FLOAT CXFA_ItemLayoutProcessor::InsertKeepLayoutItems() {
  if (m_arrayKeepItems.empty())
    return 0;

  if (!m_pLayoutItem) {
    m_pLayoutItem = CreateContentLayoutItem(m_pFormNode);
    m_pLayoutItem->m_sSize.clear();
  }

  FX_FLOAT fTotalHeight = 0;
  for (auto iter = m_arrayKeepItems.rbegin(); iter != m_arrayKeepItems.rend();
       iter++) {
    AddLeaderAfterSplit(this, *iter);
    fTotalHeight += (*iter)->m_sSize.height;
  }
  m_arrayKeepItems.clear();

  return fTotalHeight;
}

bool CXFA_ItemLayoutProcessor::ProcessKeepForSplit(
    CXFA_ItemLayoutProcessor* pParentProcessor,
    CXFA_ItemLayoutProcessor* pChildProcessor,
    XFA_ItemLayoutProcessorResult eRetValue,
    CFX_ArrayTemplate<CXFA_ContentLayoutItem*>* rgCurLineLayoutItem,
    FX_FLOAT* fContentCurRowAvailWidth,
    FX_FLOAT* fContentCurRowHeight,
    FX_FLOAT* fContentCurRowY,
    bool* bAddedItemInRow,
    bool* bForceEndPage,
    XFA_ItemLayoutProcessorResult* result) {
  if (!pParentProcessor || !pChildProcessor)
    return false;

  if (pParentProcessor->m_pCurChildNode->GetIntact() ==
          XFA_ATTRIBUTEENUM_None &&
      pChildProcessor->m_bHasAvailHeight)
    return false;

  if (!ExistContainerKeep(pParentProcessor->m_pCurChildNode, true))
    return false;

  CFX_SizeF childSize = pChildProcessor->GetCurrentComponentSize();
  std::vector<CXFA_ContentLayoutItem*> keepLayoutItems;
  if (pParentProcessor->JudgePutNextPage(pParentProcessor->m_pLayoutItem,
                                         childSize.height, &keepLayoutItems)) {
    m_arrayKeepItems.clear();

    for (auto item : keepLayoutItems) {
      pParentProcessor->m_pLayoutItem->RemoveChild(item);
      *fContentCurRowY -= item->m_sSize.height;
      m_arrayKeepItems.push_back(item);
    }
    *bAddedItemInRow = true;
    *bForceEndPage = true;
    *result = XFA_ItemLayoutProcessorResult::PageFullBreak;
    return true;
  }

  rgCurLineLayoutItem->Add(pChildProcessor->ExtractLayoutItem());
  *bAddedItemInRow = true;
  *fContentCurRowAvailWidth -= childSize.width;
  *fContentCurRowHeight = std::max(*fContentCurRowHeight, childSize.height);
  *result = eRetValue;

  return true;
}

bool CXFA_ItemLayoutProcessor::JudgePutNextPage(
    CXFA_ContentLayoutItem* pParentLayoutItem,
    FX_FLOAT fChildHeight,
    std::vector<CXFA_ContentLayoutItem*>* pKeepItems) {
  if (!pParentLayoutItem)
    return false;

  FX_FLOAT fItemsHeight = 0;
  for (CXFA_ContentLayoutItem* pChildLayoutItem =
           (CXFA_ContentLayoutItem*)pParentLayoutItem->m_pFirstChild;
       pChildLayoutItem;
       pChildLayoutItem =
           (CXFA_ContentLayoutItem*)pChildLayoutItem->m_pNextSibling) {
    if (ExistContainerKeep(pChildLayoutItem->m_pFormNode, false)) {
      pKeepItems->push_back(pChildLayoutItem);
      fItemsHeight += pChildLayoutItem->m_sSize.height;
    } else {
      pKeepItems->clear();
      fItemsHeight = 0;
    }
  }
  fItemsHeight += fChildHeight;
  return m_pPageMgr->GetNextAvailContentHeight(fItemsHeight);
}

void CXFA_ItemLayoutProcessor::ProcessUnUseBinds(CXFA_Node* pFormNode) {
  if (!pFormNode)
    return;

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode> sIterator(
      pFormNode);
  for (CXFA_Node* pNode = sIterator.MoveToNext(); pNode;
       pNode = sIterator.MoveToNext()) {
    if (pNode->IsContainerNode()) {
      CXFA_Node* pBindNode = pNode->GetBindData();
      if (pBindNode) {
        pBindNode->RemoveBindItem(pNode);
        pNode->SetObject(XFA_ATTRIBUTE_BindingNode, nullptr);
      }
    }
    pNode->SetFlag(XFA_NodeFlag_UnusedNode, true);
  }
}

void CXFA_ItemLayoutProcessor::ProcessUnUseOverFlow(
    CXFA_Node* pLeaderNode,
    CXFA_Node* pTrailerNode,
    CXFA_ContentLayoutItem* pTrailerItem,
    CXFA_Node* pFormNode) {
  ProcessUnUseBinds(pLeaderNode);
  ProcessUnUseBinds(pTrailerNode);
  if (!pFormNode)
    return;

  if (pFormNode->GetElementType() == XFA_Element::Overflow ||
      pFormNode->GetElementType() == XFA_Element::Break) {
    pFormNode = pFormNode->GetNodeItem(XFA_NODEITEM_Parent);
  }
  if (pLeaderNode && pFormNode)
    pFormNode->RemoveChild(pLeaderNode);
  if (pTrailerNode && pFormNode)
    pFormNode->RemoveChild(pTrailerNode);
  if (pTrailerItem)
    XFA_ReleaseLayoutItem(pTrailerItem);
}

XFA_ItemLayoutProcessorResult CXFA_ItemLayoutProcessor::DoLayoutFlowedContainer(
    bool bUseBreakControl,
    XFA_ATTRIBUTEENUM eFlowStrategy,
    FX_FLOAT fHeightLimit,
    FX_FLOAT fRealHeight,
    CXFA_LayoutContext* pContext,
    bool bRootForceTb) {
  m_bHasAvailHeight = true;
  bool bBreakDone = false;
  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  bool bForceEndPage = false;
  bool bIsManualBreak = false;
  if (m_pCurChildPreprocessor) {
    m_pCurChildPreprocessor->m_ePreProcessRs =
        XFA_ItemLayoutProcessorResult::Done;
  }

  CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      m_pFormNode, &bContainerWidthAutoSize, &bContainerHeightAutoSize);
  if (pContext && pContext->m_bCurColumnWidthAvaiable) {
    bContainerWidthAutoSize = false;
    containerSize.width = pContext->m_fCurColumnWidth;
  }
  if (!bContainerHeightAutoSize)
    containerSize.height -= m_fUsedSize;

  if (!bContainerHeightAutoSize) {
    CXFA_Node* pParentNode = m_pFormNode->GetNodeItem(XFA_NODEITEM_Parent);
    bool bFocrTb = false;
    if (pParentNode &&
        GetLayout(pParentNode, &bFocrTb) == XFA_ATTRIBUTEENUM_Row) {
      CXFA_Node* pChildContainer = m_pFormNode->GetNodeItem(
          XFA_NODEITEM_FirstChild, XFA_ObjectType::ContainerNode);
      if (pChildContainer &&
          pChildContainer->GetNodeItem(XFA_NODEITEM_NextSibling,
                                       XFA_ObjectType::ContainerNode)) {
        containerSize.height = 0;
        bContainerHeightAutoSize = true;
      }
    }
  }

  CXFA_Node* pMarginNode =
      m_pFormNode->GetFirstChildByClass(XFA_Element::Margin);
  FX_FLOAT fLeftInset = 0;
  FX_FLOAT fTopInset = 0;
  FX_FLOAT fRightInset = 0;
  FX_FLOAT fBottomInset = 0;
  if (pMarginNode) {
    fLeftInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_LeftInset).ToUnit(XFA_UNIT_Pt);
    fTopInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_TopInset).ToUnit(XFA_UNIT_Pt);
    fRightInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_RightInset).ToUnit(XFA_UNIT_Pt);
    fBottomInset =
        pMarginNode->GetMeasure(XFA_ATTRIBUTE_BottomInset).ToUnit(XFA_UNIT_Pt);
  }
  FX_FLOAT fContentWidthLimit =
      bContainerWidthAutoSize ? FLT_MAX
                              : containerSize.width - fLeftInset - fRightInset;
  FX_FLOAT fContentCalculatedWidth = 0;
  FX_FLOAT fContentCalculatedHeight = 0;
  FX_FLOAT fAvailHeight = fHeightLimit - fTopInset - fBottomInset;
  if (fAvailHeight < 0)
    m_bHasAvailHeight = false;

  fRealHeight = fRealHeight - fTopInset - fBottomInset;
  FX_FLOAT fContentCurRowY = 0;
  CXFA_ContentLayoutItem* pLayoutChild = nullptr;
  if (m_pLayoutItem) {
    if (m_nCurChildNodeStage != XFA_ItemLayoutProcessorStages::Done &&
        eFlowStrategy != XFA_ATTRIBUTEENUM_Tb) {
      pLayoutChild = (CXFA_ContentLayoutItem*)m_pLayoutItem->m_pFirstChild;
      for (CXFA_ContentLayoutItem* pLayoutNext = pLayoutChild; pLayoutNext;
           pLayoutNext = (CXFA_ContentLayoutItem*)pLayoutNext->m_pNextSibling) {
        if (pLayoutNext->m_sPos.y != pLayoutChild->m_sPos.y)
          pLayoutChild = pLayoutNext;
      }
    }

    for (CXFA_ContentLayoutItem* pLayoutTempChild =
             (CXFA_ContentLayoutItem*)m_pLayoutItem->m_pFirstChild;
         pLayoutTempChild != pLayoutChild;
         pLayoutTempChild =
             (CXFA_ContentLayoutItem*)pLayoutTempChild->m_pNextSibling) {
      if (!XFA_ItemLayoutProcessor_IsTakingSpace(pLayoutTempChild->m_pFormNode))
        continue;

      fContentCalculatedWidth = std::max(
          fContentCalculatedWidth,
          pLayoutTempChild->m_sPos.x + pLayoutTempChild->m_sSize.width);
      fContentCalculatedHeight = std::max(
          fContentCalculatedHeight,
          pLayoutTempChild->m_sPos.y + pLayoutTempChild->m_sSize.height);
    }

    if (pLayoutChild)
      fContentCurRowY = pLayoutChild->m_sPos.y;
    else
      fContentCurRowY = fContentCalculatedHeight;
  }

  fContentCurRowY += InsertKeepLayoutItems();
  if (m_nCurChildNodeStage == XFA_ItemLayoutProcessorStages::None) {
    GotoNextContainerNode(m_pCurChildNode, m_nCurChildNodeStage, m_pFormNode,
                          true);
  }

  fContentCurRowY += InsertPendingItems(this, m_pFormNode);
  if (m_pCurChildPreprocessor &&
      m_nCurChildNodeStage == XFA_ItemLayoutProcessorStages::Container) {
    if (ExistContainerKeep(m_pCurChildPreprocessor->GetFormNode(), false)) {
      m_pKeepHeadNode = m_pCurChildNode;
      m_bIsProcessKeep = true;
      m_nCurChildNodeStage = XFA_ItemLayoutProcessorStages::Keep;
    }
  }

  while (m_nCurChildNodeStage != XFA_ItemLayoutProcessorStages::Done) {
    FX_FLOAT fContentCurRowHeight = 0;
    FX_FLOAT fContentCurRowAvailWidth = fContentWidthLimit;
    m_fWidthLimite = fContentCurRowAvailWidth;
    CFX_ArrayTemplate<CXFA_ContentLayoutItem*> rgCurLineLayoutItems[3];
    uint8_t uCurHAlignState =
        (eFlowStrategy != XFA_ATTRIBUTEENUM_Rl_tb ? 0 : 2);
    if (pLayoutChild) {
      for (CXFA_ContentLayoutItem* pLayoutNext = pLayoutChild; pLayoutNext;
           pLayoutNext = (CXFA_ContentLayoutItem*)pLayoutNext->m_pNextSibling) {
        if (!pLayoutNext->m_pNextSibling && m_pCurChildPreprocessor &&
            m_pCurChildPreprocessor->m_pFormNode == pLayoutNext->m_pFormNode) {
          pLayoutNext->m_pNext = m_pCurChildPreprocessor->m_pLayoutItem;
          m_pCurChildPreprocessor->m_pLayoutItem = pLayoutNext;
          break;
        }
        uint8_t uHAlign = HAlignEnumToInt(
            pLayoutNext->m_pFormNode->GetEnum(XFA_ATTRIBUTE_HAlign));
        rgCurLineLayoutItems[uHAlign].Add(pLayoutNext);
        if (eFlowStrategy == XFA_ATTRIBUTEENUM_Lr_tb) {
          if (uHAlign > uCurHAlignState)
            uCurHAlignState = uHAlign;
        } else if (uHAlign < uCurHAlignState) {
          uCurHAlignState = uHAlign;
        }
        if (XFA_ItemLayoutProcessor_IsTakingSpace(pLayoutNext->m_pFormNode)) {
          if (pLayoutNext->m_sSize.height > fContentCurRowHeight)
            fContentCurRowHeight = pLayoutNext->m_sSize.height;
          fContentCurRowAvailWidth -= pLayoutNext->m_sSize.width;
        }
      }

      if ((CXFA_ContentLayoutItem*)m_pLayoutItem->m_pFirstChild ==
          pLayoutChild) {
        m_pLayoutItem->m_pFirstChild = nullptr;
      } else {
        CXFA_ContentLayoutItem* pLayoutNext =
            (CXFA_ContentLayoutItem*)m_pLayoutItem->m_pFirstChild;
        for (; pLayoutNext;
             pLayoutNext =
                 (CXFA_ContentLayoutItem*)pLayoutNext->m_pNextSibling) {
          if ((CXFA_ContentLayoutItem*)pLayoutNext->m_pNextSibling ==
              pLayoutChild) {
            pLayoutNext->m_pNextSibling = nullptr;
            break;
          }
        }
      }

      CXFA_ContentLayoutItem* pLayoutNextTemp =
          (CXFA_ContentLayoutItem*)pLayoutChild;
      while (pLayoutNextTemp) {
        pLayoutNextTemp->m_pParent = nullptr;
        CXFA_ContentLayoutItem* pSaveLayoutNext =
            (CXFA_ContentLayoutItem*)pLayoutNextTemp->m_pNextSibling;
        pLayoutNextTemp->m_pNextSibling = nullptr;
        pLayoutNextTemp = pSaveLayoutNext;
      }
      pLayoutChild = nullptr;
    }

    while (m_pCurChildNode) {
      std::unique_ptr<CXFA_ItemLayoutProcessor> pProcessor;
      bool bAddedItemInRow = false;
      fContentCurRowY += InsertPendingItems(this, m_pFormNode);
      switch (m_nCurChildNodeStage) {
        case XFA_ItemLayoutProcessorStages::Keep:
        case XFA_ItemLayoutProcessorStages::None:
          break;
        case XFA_ItemLayoutProcessorStages::BreakBefore: {
          for (auto item : m_arrayKeepItems) {
            m_pLayoutItem->RemoveChild(item);
            fContentCalculatedHeight -= item->m_sSize.height;
          }

          CXFA_Node* pLeaderNode = nullptr;
          CXFA_Node* pTrailerNode = nullptr;
          bool bCreatePage = false;
          if (!bUseBreakControl || !m_pPageMgr ||
              !m_pPageMgr->ProcessBreakBeforeOrAfter(m_pCurChildNode, true,
                                                     pLeaderNode, pTrailerNode,
                                                     bCreatePage) ||
              m_pFormNode->GetElementType() == XFA_Element::Form ||
              !bCreatePage) {
            break;
          }

          if (JudgeLeaderOrTrailerForOccur(pLeaderNode))
            AddPendingNode(this, pLeaderNode, true);

          if (JudgeLeaderOrTrailerForOccur(pTrailerNode)) {
            if (m_pFormNode->GetNodeItem(XFA_NODEITEM_Parent)
                        ->GetElementType() == XFA_Element::Form &&
                !m_pLayoutItem) {
              AddPendingNode(this, pTrailerNode, true);
            } else {
              auto pTempProcessor =
                  pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(pTrailerNode,
                                                               nullptr);
              InsertFlowedItem(
                  this, pTempProcessor.get(), bContainerWidthAutoSize,
                  bContainerHeightAutoSize, containerSize.height, eFlowStrategy,
                  &uCurHAlignState, rgCurLineLayoutItems, false, FLT_MAX,
                  FLT_MAX, fContentWidthLimit, &fContentCurRowY,
                  &fContentCurRowAvailWidth, &fContentCurRowHeight,
                  &bAddedItemInRow, &bForceEndPage, pContext, false);
            }
          }
          GotoNextContainerNode(m_pCurChildNode, m_nCurChildNodeStage,
                                m_pFormNode, true);
          bForceEndPage = true;
          bIsManualBreak = true;
          goto SuspendAndCreateNewRow;
        }
        case XFA_ItemLayoutProcessorStages::BreakAfter: {
          CXFA_Node* pLeaderNode = nullptr;
          CXFA_Node* pTrailerNode = nullptr;
          bool bCreatePage = false;
          if (!bUseBreakControl || !m_pPageMgr ||
              !m_pPageMgr->ProcessBreakBeforeOrAfter(m_pCurChildNode, false,
                                                     pLeaderNode, pTrailerNode,
                                                     bCreatePage) ||
              m_pFormNode->GetElementType() == XFA_Element::Form) {
            break;
          }

          if (JudgeLeaderOrTrailerForOccur(pTrailerNode)) {
            auto pTempProcessor = pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(
                pTrailerNode, nullptr);
            InsertFlowedItem(
                this, pTempProcessor.get(), bContainerWidthAutoSize,
                bContainerHeightAutoSize, containerSize.height, eFlowStrategy,
                &uCurHAlignState, rgCurLineLayoutItems, false, FLT_MAX, FLT_MAX,
                fContentWidthLimit, &fContentCurRowY, &fContentCurRowAvailWidth,
                &fContentCurRowHeight, &bAddedItemInRow, &bForceEndPage,
                pContext, false);
          }
          if (!bCreatePage) {
            if (JudgeLeaderOrTrailerForOccur(pLeaderNode)) {
              CalculateRowChildPosition(
                  rgCurLineLayoutItems, eFlowStrategy, bContainerHeightAutoSize,
                  bContainerWidthAutoSize, &fContentCalculatedWidth,
                  &fContentCalculatedHeight, &fContentCurRowY,
                  fContentCurRowHeight, fContentWidthLimit, false);
              rgCurLineLayoutItems->RemoveAll();
              auto pTempProcessor =
                  pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(pLeaderNode,
                                                               nullptr);
              InsertFlowedItem(
                  this, pTempProcessor.get(), bContainerWidthAutoSize,
                  bContainerHeightAutoSize, containerSize.height, eFlowStrategy,
                  &uCurHAlignState, rgCurLineLayoutItems, false, FLT_MAX,
                  FLT_MAX, fContentWidthLimit, &fContentCurRowY,
                  &fContentCurRowAvailWidth, &fContentCurRowHeight,
                  &bAddedItemInRow, &bForceEndPage, pContext, false);
            }
          } else {
            if (JudgeLeaderOrTrailerForOccur(pLeaderNode))
              AddPendingNode(this, pLeaderNode, true);
          }

          GotoNextContainerNode(m_pCurChildNode, m_nCurChildNodeStage,
                                m_pFormNode, true);
          if (bCreatePage) {
            bForceEndPage = true;
            bIsManualBreak = true;
            if (m_nCurChildNodeStage == XFA_ItemLayoutProcessorStages::Done)
              bBreakDone = true;
          }
          goto SuspendAndCreateNewRow;
        }
        case XFA_ItemLayoutProcessorStages::BookendLeader: {
          CXFA_Node* pLeaderNode = nullptr;
          if (m_pCurChildPreprocessor) {
            pProcessor.reset(m_pCurChildPreprocessor);
            m_pCurChildPreprocessor = nullptr;
          } else if (m_pPageMgr &&
                     m_pPageMgr->ProcessBookendLeaderOrTrailer(
                         m_pCurChildNode, true, pLeaderNode)) {
            pProcessor = pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(
                pLeaderNode, m_pPageMgr);
          }

          if (pProcessor) {
            if (InsertFlowedItem(
                    this, pProcessor.get(), bContainerWidthAutoSize,
                    bContainerHeightAutoSize, containerSize.height,
                    eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems,
                    bUseBreakControl, fAvailHeight, fRealHeight,
                    fContentWidthLimit, &fContentCurRowY,
                    &fContentCurRowAvailWidth, &fContentCurRowHeight,
                    &bAddedItemInRow, &bForceEndPage, pContext,
                    false) != XFA_ItemLayoutProcessorResult::Done) {
              goto SuspendAndCreateNewRow;
            } else {
              pProcessor.reset();
            }
          }
          break;
        }
        case XFA_ItemLayoutProcessorStages::BookendTrailer: {
          CXFA_Node* pTrailerNode = nullptr;
          if (m_pCurChildPreprocessor) {
            pProcessor.reset(m_pCurChildPreprocessor);
            m_pCurChildPreprocessor = nullptr;
          } else if (m_pPageMgr &&
                     m_pPageMgr->ProcessBookendLeaderOrTrailer(
                         m_pCurChildNode, false, pTrailerNode)) {
            pProcessor = pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(
                pTrailerNode, m_pPageMgr);
          }
          if (pProcessor) {
            if (InsertFlowedItem(
                    this, pProcessor.get(), bContainerWidthAutoSize,
                    bContainerHeightAutoSize, containerSize.height,
                    eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems,
                    bUseBreakControl, fAvailHeight, fRealHeight,
                    fContentWidthLimit, &fContentCurRowY,
                    &fContentCurRowAvailWidth, &fContentCurRowHeight,
                    &bAddedItemInRow, &bForceEndPage, pContext,
                    false) != XFA_ItemLayoutProcessorResult::Done) {
              goto SuspendAndCreateNewRow;
            } else {
              pProcessor.reset();
            }
          }
          break;
        }
        case XFA_ItemLayoutProcessorStages::Container: {
          ASSERT(m_pCurChildNode->IsContainerNode());
          if (m_pCurChildNode->GetElementType() == XFA_Element::Variables)
            break;
          if (fContentCurRowY >= fHeightLimit + XFA_LAYOUT_FLOAT_PERCISION &&
              XFA_ItemLayoutProcessor_IsTakingSpace(m_pCurChildNode)) {
            bForceEndPage = true;
            goto SuspendAndCreateNewRow;
          }
          if (!m_pCurChildNode->IsContainerNode())
            break;

          bool bNewRow = false;
          if (m_pCurChildPreprocessor) {
            pProcessor.reset(m_pCurChildPreprocessor);
            m_pCurChildPreprocessor = nullptr;
            bNewRow = true;
          } else {
            pProcessor = pdfium::MakeUnique<CXFA_ItemLayoutProcessor>(
                m_pCurChildNode, m_pPageMgr);
          }

          InsertPendingItems(pProcessor.get(), m_pCurChildNode);
          XFA_ItemLayoutProcessorResult rs = InsertFlowedItem(
              this, pProcessor.get(), bContainerWidthAutoSize,
              bContainerHeightAutoSize, containerSize.height, eFlowStrategy,
              &uCurHAlignState, rgCurLineLayoutItems, bUseBreakControl,
              fAvailHeight, fRealHeight, fContentWidthLimit, &fContentCurRowY,
              &fContentCurRowAvailWidth, &fContentCurRowHeight,
              &bAddedItemInRow, &bForceEndPage, pContext, bNewRow);
          switch (rs) {
            case XFA_ItemLayoutProcessorResult::ManualBreak:
              bIsManualBreak = true;
            case XFA_ItemLayoutProcessorResult::PageFullBreak:
              bForceEndPage = true;
            case XFA_ItemLayoutProcessorResult::RowFullBreak:
              goto SuspendAndCreateNewRow;
            case XFA_ItemLayoutProcessorResult::Done:
            default:
              fContentCurRowY +=
                  InsertPendingItems(pProcessor.get(), m_pCurChildNode);
              pProcessor.reset();
          }
          break;
        }
        case XFA_ItemLayoutProcessorStages::Done:
          break;
        default:
          break;
      }
      GotoNextContainerNode(m_pCurChildNode, m_nCurChildNodeStage, m_pFormNode,
                            true);
      if (bAddedItemInRow && eFlowStrategy == XFA_ATTRIBUTEENUM_Tb)
        break;
      else
        continue;
    SuspendAndCreateNewRow:
      if (pProcessor)
        m_pCurChildPreprocessor = pProcessor.release();
      break;
    }

    CalculateRowChildPosition(
        rgCurLineLayoutItems, eFlowStrategy, bContainerHeightAutoSize,
        bContainerWidthAutoSize, &fContentCalculatedWidth,
        &fContentCalculatedHeight, &fContentCurRowY, fContentCurRowHeight,
        fContentWidthLimit, bRootForceTb);
    m_fWidthLimite = fContentCurRowAvailWidth;
    if (bForceEndPage)
      break;
  }

  bool bRetValue =
      m_nCurChildNodeStage == XFA_ItemLayoutProcessorStages::Done &&
      m_PendingNodes.empty();
  if (bBreakDone)
    bRetValue = false;

  containerSize = CalculateContainerComponentSizeFromContentSize(
      m_pFormNode, bContainerWidthAutoSize, fContentCalculatedWidth,
      bContainerHeightAutoSize, fContentCalculatedHeight, containerSize);

  if (containerSize.height >= XFA_LAYOUT_FLOAT_PERCISION || m_pLayoutItem ||
      bRetValue) {
    if (!m_pLayoutItem)
      m_pLayoutItem = CreateContentLayoutItem(m_pFormNode);
    containerSize.height = std::max(containerSize.height, 0.f);

    SetCurrentComponentSize(containerSize);
    if (bForceEndPage)
      m_fUsedSize = 0;
    else
      m_fUsedSize += m_pLayoutItem->m_sSize.height;
  }

  return bRetValue
             ? XFA_ItemLayoutProcessorResult::Done
             : (bIsManualBreak ? XFA_ItemLayoutProcessorResult::ManualBreak
                               : XFA_ItemLayoutProcessorResult::PageFullBreak);
}

bool CXFA_ItemLayoutProcessor::CalculateRowChildPosition(
    CFX_ArrayTemplate<CXFA_ContentLayoutItem*> (&rgCurLineLayoutItems)[3],
    XFA_ATTRIBUTEENUM eFlowStrategy,
    bool bContainerHeightAutoSize,
    bool bContainerWidthAutoSize,
    FX_FLOAT* fContentCalculatedWidth,
    FX_FLOAT* fContentCalculatedHeight,
    FX_FLOAT* fContentCurRowY,
    FX_FLOAT fContentCurRowHeight,
    FX_FLOAT fContentWidthLimit,
    bool bRootForceTb) {
  int32_t nGroupLengths[3] = {0, 0, 0};
  FX_FLOAT fGroupWidths[3] = {0, 0, 0};
  int32_t nTotalLength = 0;
  for (int32_t i = 0; i < 3; i++) {
    nGroupLengths[i] = rgCurLineLayoutItems[i].GetSize();
    for (int32_t c = nGroupLengths[i], j = 0; j < c; j++) {
      nTotalLength++;
      if (XFA_ItemLayoutProcessor_IsTakingSpace(
              rgCurLineLayoutItems[i][j]->m_pFormNode)) {
        fGroupWidths[i] += rgCurLineLayoutItems[i][j]->m_sSize.width;
      }
    }
  }

  if (!nTotalLength) {
    if (bContainerHeightAutoSize) {
      *fContentCalculatedHeight =
          std::min(*fContentCalculatedHeight, *fContentCurRowY);
    }
    return false;
  }

  if (!m_pLayoutItem)
    m_pLayoutItem = CreateContentLayoutItem(m_pFormNode);

  if (eFlowStrategy != XFA_ATTRIBUTEENUM_Rl_tb) {
    FX_FLOAT fCurPos;
    fCurPos = 0;
    for (int32_t c = nGroupLengths[0], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[0][j]->m_sPos = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[0][j]->m_pFormNode,
            rgCurLineLayoutItems[0][j]->m_sSize);
      } else {
        rgCurLineLayoutItems[0][j]->m_sPos =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (XFA_ItemLayoutProcessor_IsTakingSpace(
                rgCurLineLayoutItems[0][j]->m_pFormNode)) {
          fCurPos += rgCurLineLayoutItems[0][j]->m_sSize.width;
        }
      }
      m_pLayoutItem->AddChild(rgCurLineLayoutItems[0][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = (fContentWidthLimit + fGroupWidths[0] - fGroupWidths[1] -
               fGroupWidths[2]) /
              2;
    for (int32_t c = nGroupLengths[1], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[1][j]->m_sPos = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[1][j]->m_pFormNode,
            rgCurLineLayoutItems[1][j]->m_sSize);
      } else {
        rgCurLineLayoutItems[1][j]->m_sPos =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (XFA_ItemLayoutProcessor_IsTakingSpace(
                rgCurLineLayoutItems[1][j]->m_pFormNode)) {
          fCurPos += rgCurLineLayoutItems[1][j]->m_sSize.width;
        }
      }
      m_pLayoutItem->AddChild(rgCurLineLayoutItems[1][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = fContentWidthLimit - fGroupWidths[2];
    for (int32_t c = nGroupLengths[2], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[2][j]->m_sPos = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[2][j]->m_pFormNode,
            rgCurLineLayoutItems[2][j]->m_sSize);
      } else {
        rgCurLineLayoutItems[2][j]->m_sPos =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (XFA_ItemLayoutProcessor_IsTakingSpace(
                rgCurLineLayoutItems[2][j]->m_pFormNode)) {
          fCurPos += rgCurLineLayoutItems[2][j]->m_sSize.width;
        }
      }
      m_pLayoutItem->AddChild(rgCurLineLayoutItems[2][j]);
      m_fLastRowWidth = fCurPos;
    }
  } else {
    FX_FLOAT fCurPos;
    fCurPos = fGroupWidths[0];
    for (int32_t c = nGroupLengths[0], j = 0; j < c; j++) {
      if (XFA_ItemLayoutProcessor_IsTakingSpace(
              rgCurLineLayoutItems[0][j]->m_pFormNode)) {
        fCurPos -= rgCurLineLayoutItems[0][j]->m_sSize.width;
      }
      rgCurLineLayoutItems[0][j]->m_sPos =
          CFX_PointF(fCurPos, *fContentCurRowY);
      m_pLayoutItem->AddChild(rgCurLineLayoutItems[0][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = (fContentWidthLimit + fGroupWidths[0] + fGroupWidths[1] -
               fGroupWidths[2]) /
              2;
    for (int32_t c = nGroupLengths[1], j = 0; j < c; j++) {
      if (XFA_ItemLayoutProcessor_IsTakingSpace(
              rgCurLineLayoutItems[1][j]->m_pFormNode)) {
        fCurPos -= rgCurLineLayoutItems[1][j]->m_sSize.width;
      }
      rgCurLineLayoutItems[1][j]->m_sPos =
          CFX_PointF(fCurPos, *fContentCurRowY);
      m_pLayoutItem->AddChild(rgCurLineLayoutItems[1][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = fContentWidthLimit;
    for (int32_t c = nGroupLengths[2], j = 0; j < c; j++) {
      if (XFA_ItemLayoutProcessor_IsTakingSpace(
              rgCurLineLayoutItems[2][j]->m_pFormNode)) {
        fCurPos -= rgCurLineLayoutItems[2][j]->m_sSize.width;
      }
      rgCurLineLayoutItems[2][j]->m_sPos =
          CFX_PointF(fCurPos, *fContentCurRowY);
      m_pLayoutItem->AddChild(rgCurLineLayoutItems[2][j]);
      m_fLastRowWidth = fCurPos;
    }
  }
  m_fLastRowY = *fContentCurRowY;
  *fContentCurRowY += fContentCurRowHeight;
  if (bContainerWidthAutoSize) {
    FX_FLOAT fChildSuppliedWidth = fGroupWidths[0];
    if (fContentWidthLimit < FLT_MAX &&
        fContentWidthLimit > fChildSuppliedWidth) {
      fChildSuppliedWidth = fContentWidthLimit;
    }
    *fContentCalculatedWidth =
        std::max(*fContentCalculatedWidth, fChildSuppliedWidth);
  }
  if (bContainerHeightAutoSize) {
    *fContentCalculatedHeight =
        std::max(*fContentCalculatedHeight, *fContentCurRowY);
  }
  return true;
}

CXFA_Node* CXFA_ItemLayoutProcessor::GetSubformSetParent(
    CXFA_Node* pSubformSet) {
  if (pSubformSet && pSubformSet->GetElementType() == XFA_Element::SubformSet) {
    CXFA_Node* pParent = pSubformSet->GetNodeItem(XFA_NODEITEM_Parent);
    while (pParent) {
      if (pParent->GetElementType() != XFA_Element::SubformSet)
        return pParent;
      pParent = pParent->GetNodeItem(XFA_NODEITEM_Parent);
    }
  }
  return pSubformSet;
}

void CXFA_ItemLayoutProcessor::DoLayoutField() {
  if (m_pLayoutItem)
    return;

  ASSERT(m_pCurChildNode == XFA_LAYOUT_INVALIDNODE);
  m_pLayoutItem = CreateContentLayoutItem(m_pFormNode);
  if (!m_pLayoutItem)
    return;

  CXFA_Document* pDocument = m_pFormNode->GetDocument();
  CXFA_FFNotify* pNotify = pDocument->GetNotify();
  CFX_SizeF size(-1, -1);
  pNotify->StartFieldDrawLayout(m_pFormNode, size.width, size.height);

  int32_t nRotate =
      FXSYS_round(m_pFormNode->GetMeasure(XFA_ATTRIBUTE_Rotate).GetValue());
  nRotate = XFA_MapRotation(nRotate);
  if (nRotate == 90 || nRotate == 270)
    std::swap(size.width, size.height);

  SetCurrentComponentSize(size);
}

XFA_ItemLayoutProcessorResult CXFA_ItemLayoutProcessor::DoLayout(
    bool bUseBreakControl,
    FX_FLOAT fHeightLimit,
    FX_FLOAT fRealHeight,
    CXFA_LayoutContext* pContext) {
  switch (m_pFormNode->GetElementType()) {
    case XFA_Element::Subform:
    case XFA_Element::Area:
    case XFA_Element::ExclGroup:
    case XFA_Element::SubformSet: {
      bool bRootForceTb = false;
      CXFA_Node* pLayoutNode = GetSubformSetParent(m_pFormNode);
      XFA_ATTRIBUTEENUM eLayoutStrategy = GetLayout(pLayoutNode, &bRootForceTb);
      switch (eLayoutStrategy) {
        case XFA_ATTRIBUTEENUM_Tb:
        case XFA_ATTRIBUTEENUM_Lr_tb:
        case XFA_ATTRIBUTEENUM_Rl_tb:
          return DoLayoutFlowedContainer(bUseBreakControl, eLayoutStrategy,
                                         fHeightLimit, fRealHeight, pContext,
                                         bRootForceTb);
        case XFA_ATTRIBUTEENUM_Position:
        case XFA_ATTRIBUTEENUM_Row:
        case XFA_ATTRIBUTEENUM_Rl_row:
        default:
          DoLayoutPositionedContainer(pContext);
          m_nCurChildNodeStage = XFA_ItemLayoutProcessorStages::Done;
          return XFA_ItemLayoutProcessorResult::Done;
        case XFA_ATTRIBUTEENUM_Table:
          DoLayoutTableContainer(pLayoutNode);
          m_nCurChildNodeStage = XFA_ItemLayoutProcessorStages::Done;
          return XFA_ItemLayoutProcessorResult::Done;
      }
    }
    case XFA_Element::Draw:
    case XFA_Element::Field:
      DoLayoutField();
      m_nCurChildNodeStage = XFA_ItemLayoutProcessorStages::Done;
      return XFA_ItemLayoutProcessorResult::Done;
    case XFA_Element::ContentArea:
      return XFA_ItemLayoutProcessorResult::Done;
    default:
      return XFA_ItemLayoutProcessorResult::Done;
  }
}

CFX_SizeF CXFA_ItemLayoutProcessor::GetCurrentComponentSize() {
  return CFX_SizeF(m_pLayoutItem->m_sSize.width, m_pLayoutItem->m_sSize.height);
}

void CXFA_ItemLayoutProcessor::SetCurrentComponentPos(const CFX_PointF& pos) {
  m_pLayoutItem->m_sPos = pos;
}

void CXFA_ItemLayoutProcessor::SetCurrentComponentSize(const CFX_SizeF& size) {
  m_pLayoutItem->m_sSize = size;
}

bool CXFA_ItemLayoutProcessor::JudgeLeaderOrTrailerForOccur(
    CXFA_Node* pFormNode) {
  if (!pFormNode)
    return false;

  CXFA_Node* pTemplate = pFormNode->GetTemplateNode();
  if (!pTemplate)
    pTemplate = pFormNode;

  CXFA_Occur NodeOccur(pTemplate->GetFirstChildByClass(XFA_Element::Occur));
  int32_t iMax = NodeOccur.GetMax();
  if (iMax < 0)
    return true;

  int32_t iCount = m_PendingNodesCount[pTemplate];
  if (iCount >= iMax)
    return false;

  m_PendingNodesCount[pTemplate] = iCount + 1;
  return true;
}
