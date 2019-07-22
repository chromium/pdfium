// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_contentlayoutprocessor.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_keep.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfanode.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

std::vector<WideString> SeparateStringOnSpace(
    pdfium::span<const wchar_t> spStr) {
  std::vector<WideString> ret;
  if (spStr.empty())
    return ret;

  size_t nPos = 0;
  size_t nToken = 0;
  while (nPos < spStr.size()) {
    if (spStr[nPos] == L' ') {
      ret.emplace_back(WideStringView(spStr.subspan(nToken, nPos - nToken)));
      nToken = nPos + 1;
    }
    nPos++;
  }
  ret.emplace_back(WideStringView(spStr.subspan(nToken, nPos - nToken)));
  return ret;
}

void UpdateWidgetSize(CXFA_ContentLayoutItem* pLayoutItem,
                      float* pWidth,
                      float* pHeight) {
  CXFA_Node* pNode = pLayoutItem->GetFormNode();
  switch (pNode->GetElementType()) {
    case XFA_Element::Subform:
    case XFA_Element::Area:
    case XFA_Element::ExclGroup:
    case XFA_Element::SubformSet: {
      if (*pWidth < -kXFALayoutPrecision)
        *pWidth = pLayoutItem->m_sSize.width;
      if (*pHeight < -kXFALayoutPrecision)
        *pHeight = pLayoutItem->m_sSize.height;
      break;
    }
    case XFA_Element::Draw:
    case XFA_Element::Field: {
      pNode->GetDocument()->GetNotify()->StartFieldDrawLayout(pNode, pWidth,
                                                              pHeight);
      break;
    }
    default:
      NOTREACHED();
  }
}

CFX_SizeF CalculateContainerSpecifiedSize(CXFA_Node* pFormNode,
                                          bool* bContainerWidthAutoSize,
                                          bool* bContainerHeightAutoSize) {
  *bContainerWidthAutoSize = true;
  *bContainerHeightAutoSize = true;

  XFA_Element eType = pFormNode->GetElementType();

  CFX_SizeF containerSize;
  if (eType == XFA_Element::Subform || eType == XFA_Element::ExclGroup) {
    Optional<CXFA_Measurement> wValue =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::W, false);
    if (wValue && wValue->GetValue() > kXFALayoutPrecision) {
      containerSize.width = wValue->ToUnit(XFA_Unit::Pt);
      *bContainerWidthAutoSize = false;
    }

    Optional<CXFA_Measurement> hValue =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::H, false);
    if (hValue && hValue->GetValue() > kXFALayoutPrecision) {
      containerSize.height = hValue->ToUnit(XFA_Unit::Pt);
      *bContainerHeightAutoSize = false;
    }
  }

  if (*bContainerWidthAutoSize && eType == XFA_Element::Subform) {
    Optional<CXFA_Measurement> maxW =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::MaxW, false);
    if (maxW && maxW->GetValue() > kXFALayoutPrecision) {
      containerSize.width = maxW->ToUnit(XFA_Unit::Pt);
      *bContainerWidthAutoSize = false;
    }

    Optional<CXFA_Measurement> maxH =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::MaxH, false);
    if (maxH && maxH->GetValue() > kXFALayoutPrecision) {
      containerSize.height = maxH->ToUnit(XFA_Unit::Pt);
      *bContainerHeightAutoSize = false;
    }
  }
  return containerSize;
}

CFX_SizeF CalculateContainerComponentSizeFromContentSize(
    CXFA_Node* pFormNode,
    bool bContainerWidthAutoSize,
    float fContentCalculatedWidth,
    bool bContainerHeightAutoSize,
    float fContentCalculatedHeight,
    const CFX_SizeF& currentContainerSize) {
  CFX_SizeF componentSize = currentContainerSize;
  CXFA_Margin* pMarginNode =
      pFormNode->GetFirstChildByClass<CXFA_Margin>(XFA_Element::Margin);
  if (bContainerWidthAutoSize) {
    componentSize.width = fContentCalculatedWidth;
    if (pMarginNode) {
      Optional<CXFA_Measurement> leftInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::LeftInset, false);
      if (leftInset)
        componentSize.width += leftInset->ToUnit(XFA_Unit::Pt);

      Optional<CXFA_Measurement> rightInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::RightInset, false);
      if (rightInset)
        componentSize.width += rightInset->ToUnit(XFA_Unit::Pt);
    }
  }

  if (bContainerHeightAutoSize) {
    componentSize.height = fContentCalculatedHeight;
    if (pMarginNode) {
      Optional<CXFA_Measurement> topInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::TopInset, false);
      if (topInset)
        componentSize.height += topInset->ToUnit(XFA_Unit::Pt);

      Optional<CXFA_Measurement> bottomInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::BottomInset,
                                              false);
      if (bottomInset)
        componentSize.height += bottomInset->ToUnit(XFA_Unit::Pt);
    }
  }
  return componentSize;
}

CFX_FloatRect GetMarginInset(const CXFA_Margin* pMargin) {
  CFX_FloatRect inset;
  if (!pMargin)
    return inset;

  inset.left = pMargin->JSObject()->GetMeasureInUnit(XFA_Attribute::LeftInset,
                                                     XFA_Unit::Pt);
  inset.top = pMargin->JSObject()->GetMeasureInUnit(XFA_Attribute::TopInset,
                                                    XFA_Unit::Pt);
  inset.right = pMargin->JSObject()->GetMeasureInUnit(XFA_Attribute::RightInset,
                                                      XFA_Unit::Pt);
  inset.bottom = pMargin->JSObject()->GetMeasureInUnit(
      XFA_Attribute::BottomInset, XFA_Unit::Pt);
  return inset;
}

void RelocateTableRowCells(CXFA_ContentLayoutItem* pLayoutRow,
                           const std::vector<float>& rgSpecifiedColumnWidths,
                           XFA_AttributeValue eLayout) {
  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      pLayoutRow->GetFormNode(), &bContainerWidthAutoSize,
      &bContainerHeightAutoSize);
  CXFA_Margin* pMargin =
      pLayoutRow->GetFormNode()->GetFirstChildByClass<CXFA_Margin>(
          XFA_Element::Margin);
  CFX_FloatRect inset = GetMarginInset(pMargin);
  float fContentWidthLimit =
      bContainerWidthAutoSize ? FLT_MAX
                              : containerSize.width - inset.left - inset.right;
  float fContentCurrentHeight =
      pLayoutRow->m_sSize.height - inset.top - inset.bottom;
  float fContentCalculatedWidth = 0;
  float fContentCalculatedHeight = 0;
  float fCurrentColX = 0;
  int32_t nCurrentColIdx = 0;
  bool bMetWholeRowCell = false;

  for (CXFA_LayoutItem* pIter = pLayoutRow->GetFirstChild(); pIter;
       pIter = pIter->GetNextSibling()) {
    CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
    if (!pLayoutChild)
      continue;

    int32_t nOriginalColSpan =
        pLayoutChild->GetFormNode()->JSObject()->GetInteger(
            XFA_Attribute::ColSpan);
    if (nOriginalColSpan <= 0 && nOriginalColSpan != -1)
      continue;

    int32_t nColSpan = nOriginalColSpan;
    float fColSpanWidth = 0;
    if (nColSpan == -1 ||
        nCurrentColIdx + nColSpan >
            pdfium::CollectionSize<int32_t>(rgSpecifiedColumnWidths)) {
      nColSpan = pdfium::CollectionSize<int32_t>(rgSpecifiedColumnWidths) -
                 nCurrentColIdx;
    }
    for (int32_t i = 0; i < nColSpan; i++)
      fColSpanWidth += rgSpecifiedColumnWidths[nCurrentColIdx + i];

    if (nColSpan != nOriginalColSpan) {
      fColSpanWidth = bMetWholeRowCell ? 0
                                       : std::max(fColSpanWidth,
                                                  pLayoutChild->m_sSize.height);
    }
    if (nOriginalColSpan == -1)
      bMetWholeRowCell = true;

    pLayoutChild->m_sPos = CFX_PointF(fCurrentColX, 0);
    pLayoutChild->m_sSize.width = fColSpanWidth;
    if (!pLayoutChild->GetFormNode()->PresenceRequiresSpace())
      continue;

    fCurrentColX += fColSpanWidth;
    nCurrentColIdx += nColSpan;
    float fNewHeight = bContainerHeightAutoSize ? -1 : fContentCurrentHeight;
    UpdateWidgetSize(pLayoutChild, &fColSpanWidth, &fNewHeight);
    pLayoutChild->m_sSize.height = fNewHeight;
    if (bContainerHeightAutoSize) {
      fContentCalculatedHeight =
          std::max(fContentCalculatedHeight, pLayoutChild->m_sSize.height);
    }
  }

  if (bContainerHeightAutoSize) {
    for (CXFA_LayoutItem* pIter = pLayoutRow->GetFirstChild(); pIter;
         pIter = pIter->GetNextSibling()) {
      CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
      if (!pLayoutChild)
        continue;

      UpdateWidgetSize(pLayoutChild, &pLayoutChild->m_sSize.width,
                       &fContentCalculatedHeight);
      float fOldChildHeight = pLayoutChild->m_sSize.height;
      pLayoutChild->m_sSize.height = fContentCalculatedHeight;
      CXFA_Para* pParaNode =
          pLayoutChild->GetFormNode()->GetFirstChildByClass<CXFA_Para>(
              XFA_Element::Para);
      if (pParaNode && pLayoutChild->GetFirstChild()) {
        float fOffHeight = fContentCalculatedHeight - fOldChildHeight;
        XFA_AttributeValue eVType =
            pParaNode->JSObject()->GetEnum(XFA_Attribute::VAlign);
        switch (eVType) {
          case XFA_AttributeValue::Middle:
            fOffHeight = fOffHeight / 2;
            break;
          case XFA_AttributeValue::Bottom:
            break;
          case XFA_AttributeValue::Top:
          default:
            fOffHeight = 0;
            break;
        }
        if (fOffHeight > 0) {
          for (CXFA_LayoutItem* pInnerIter = pLayoutChild->GetFirstChild();
               pInnerIter; pInnerIter = pInnerIter->GetNextSibling()) {
            CXFA_ContentLayoutItem* pInnerChild =
                pInnerIter->AsContentLayoutItem();
            if (!pInnerChild)
              continue;

            pInnerChild->m_sPos.y += fOffHeight;
          }
        }
      }
    }
  }

  if (bContainerWidthAutoSize) {
    float fChildSuppliedWidth = fCurrentColX;
    if (fContentWidthLimit < FLT_MAX &&
        fContentWidthLimit > fChildSuppliedWidth) {
      fChildSuppliedWidth = fContentWidthLimit;
    }
    fContentCalculatedWidth =
        std::max(fContentCalculatedWidth, fChildSuppliedWidth);
  } else {
    fContentCalculatedWidth = containerSize.width - inset.left - inset.right;
  }

  if (pLayoutRow->GetFormNode()->JSObject()->GetEnum(XFA_Attribute::Layout) ==
      XFA_AttributeValue::Rl_row) {
    for (CXFA_LayoutItem* pIter = pLayoutRow->GetFirstChild(); pIter;
         pIter = pIter->GetNextSibling()) {
      CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
      if (!pLayoutChild)
        continue;

      pLayoutChild->m_sPos.x = fContentCalculatedWidth -
                               pLayoutChild->m_sPos.x -
                               pLayoutChild->m_sSize.width;
    }
  }
  pLayoutRow->m_sSize = CalculateContainerComponentSizeFromContentSize(
      pLayoutRow->GetFormNode(), bContainerWidthAutoSize,
      fContentCalculatedWidth, bContainerHeightAutoSize,
      fContentCalculatedHeight, containerSize);
}

XFA_AttributeValue GetLayout(CXFA_Node* pFormNode, bool* bRootForceTb) {
  *bRootForceTb = false;
  Optional<XFA_AttributeValue> layoutMode =
      pFormNode->JSObject()->TryEnum(XFA_Attribute::Layout, false);
  if (layoutMode)
    return *layoutMode;

  CXFA_Node* pParentNode = pFormNode->GetParent();
  if (pParentNode && pParentNode->GetElementType() == XFA_Element::Form) {
    *bRootForceTb = true;
    return XFA_AttributeValue::Tb;
  }
  return XFA_AttributeValue::Position;
}

bool ExistContainerKeep(CXFA_Node* pCurNode, bool bPreFind) {
  if (!pCurNode || !pCurNode->PresenceRequiresSpace())
    return false;

  CXFA_Node* pPreContainer = bPreFind ? pCurNode->GetPrevContainerSibling()
                                      : pCurNode->GetNextContainerSibling();
  if (!pPreContainer)
    return false;

  CXFA_Keep* pKeep =
      pCurNode->GetFirstChildByClass<CXFA_Keep>(XFA_Element::Keep);
  if (pKeep) {
    XFA_Attribute eKeepType = XFA_Attribute::Previous;
    if (!bPreFind)
      eKeepType = XFA_Attribute::Next;

    Optional<XFA_AttributeValue> previous =
        pKeep->JSObject()->TryEnum(eKeepType, false);
    if (previous) {
      if (*previous == XFA_AttributeValue::ContentArea ||
          *previous == XFA_AttributeValue::PageArea) {
        return true;
      }
    }
  }

  pKeep = pPreContainer->GetFirstChildByClass<CXFA_Keep>(XFA_Element::Keep);
  if (!pKeep)
    return false;

  XFA_Attribute eKeepType = XFA_Attribute::Next;
  if (!bPreFind)
    eKeepType = XFA_Attribute::Previous;

  Optional<XFA_AttributeValue> next =
      pKeep->JSObject()->TryEnum(eKeepType, false);
  if (!next)
    return false;
  if (*next == XFA_AttributeValue::ContentArea ||
      *next == XFA_AttributeValue::PageArea) {
    return true;
  }
  return false;
}

Optional<CXFA_ContentLayoutProcessor::Stage> FindBreakNode(
    CXFA_Node* pContainerNode,
    bool bBreakBefore,
    CXFA_Node** pCurActionNode) {
  for (CXFA_Node* pBreakNode = pContainerNode; pBreakNode;
       pBreakNode = pBreakNode->GetNextSibling()) {
    XFA_Attribute eAttributeType =
        bBreakBefore ? XFA_Attribute::Before : XFA_Attribute::After;

    switch (pBreakNode->GetElementType()) {
      case XFA_Element::BreakBefore: {
        if (!bBreakBefore)
          break;

        *pCurActionNode = pBreakNode;
        return CXFA_ContentLayoutProcessor::Stage::kBreakBefore;
      }
      case XFA_Element::BreakAfter: {
        if (bBreakBefore)
          break;

        *pCurActionNode = pBreakNode;
        return CXFA_ContentLayoutProcessor::Stage::kBreakAfter;
      }
      case XFA_Element::Break:
        if (pBreakNode->JSObject()->GetEnum(eAttributeType) ==
            XFA_AttributeValue::Auto) {
          break;
        }

        *pCurActionNode = pBreakNode;
        return bBreakBefore ? CXFA_ContentLayoutProcessor::Stage::kBreakBefore
                            : CXFA_ContentLayoutProcessor::Stage::kBreakAfter;
      default:
        break;
    }
  }
  return {};
}

void DeleteLayoutGeneratedNode(CXFA_Node* pGenerateNode) {
  CXFA_FFNotify* pNotify = pGenerateNode->GetDocument()->GetNotify();
  auto* pDocLayout =
      CXFA_LayoutProcessor::FromDocument(pGenerateNode->GetDocument());
  CXFA_NodeIterator sIterator(pGenerateNode);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    CXFA_ContentLayoutItem* pCurLayoutItem =
        ToContentLayoutItem(pNode->JSObject()->GetLayoutItem());
    while (pCurLayoutItem) {
      CXFA_ContentLayoutItem* pNextLayoutItem = pCurLayoutItem->GetNext();
      pNotify->OnLayoutItemRemoving(pDocLayout, pCurLayoutItem);
      delete pCurLayoutItem;
      pCurLayoutItem = pNextLayoutItem;
    }
  }
  pGenerateNode->GetParent()->RemoveChildAndNotify(pGenerateNode, true);
}

uint8_t HAlignEnumToInt(XFA_AttributeValue eHAlign) {
  switch (eHAlign) {
    case XFA_AttributeValue::Center:
      return 1;
    case XFA_AttributeValue::Right:
      return 2;
    case XFA_AttributeValue::Left:
    default:
      return 0;
  }
}

bool FindLayoutItemSplitPos(CXFA_ContentLayoutItem* pLayoutItem,
                            float fCurVerticalOffset,
                            float* fProposedSplitPos,
                            bool* bAppChange,
                            bool bCalculateMargin) {
  CXFA_Node* pFormNode = pLayoutItem->GetFormNode();
  if (*fProposedSplitPos <= fCurVerticalOffset + kXFALayoutPrecision ||
      *fProposedSplitPos > fCurVerticalOffset + pLayoutItem->m_sSize.height -
                               kXFALayoutPrecision) {
    return false;
  }

  switch (pFormNode->GetIntact()) {
    case XFA_AttributeValue::None: {
      bool bAnyChanged = false;
      CXFA_Document* pDocument = pFormNode->GetDocument();
      CXFA_FFNotify* pNotify = pDocument->GetNotify();
      float fCurTopMargin = 0, fCurBottomMargin = 0;
      CXFA_Margin* pMarginNode =
          pFormNode->GetFirstChildByClass<CXFA_Margin>(XFA_Element::Margin);
      if (pMarginNode && bCalculateMargin) {
        fCurTopMargin = pMarginNode->JSObject()->GetMeasureInUnit(
            XFA_Attribute::TopInset, XFA_Unit::Pt);
        fCurBottomMargin = pMarginNode->JSObject()->GetMeasureInUnit(
            XFA_Attribute::BottomInset, XFA_Unit::Pt);
      }
      bool bChanged = true;
      while (bChanged) {
        bChanged = false;
        {
          Optional<float> fRelSplitPos = pFormNode->FindSplitPos(
              pNotify->GetHDOC()->GetDocView(), pLayoutItem->GetIndex(),
              *fProposedSplitPos - fCurVerticalOffset);
          if (fRelSplitPos.has_value()) {
            bAnyChanged = true;
            bChanged = true;
            *fProposedSplitPos = fCurVerticalOffset + fRelSplitPos.value();
            *bAppChange = true;
            if (*fProposedSplitPos <=
                fCurVerticalOffset + kXFALayoutPrecision) {
              return true;
            }
          }
        }
        float fRelSplitPos = *fProposedSplitPos - fCurBottomMargin;
        for (CXFA_LayoutItem* pIter = pLayoutItem->GetFirstChild(); pIter;
             pIter = pIter->GetNextSibling()) {
          CXFA_ContentLayoutItem* pChildItem = pIter->AsContentLayoutItem();
          if (!pChildItem)
            continue;

          float fChildOffset =
              fCurVerticalOffset + fCurTopMargin + pChildItem->m_sPos.y;
          bool bChange = false;
          if (FindLayoutItemSplitPos(pChildItem, fChildOffset, &fRelSplitPos,
                                     &bChange, bCalculateMargin)) {
            if (fRelSplitPos - fChildOffset < kXFALayoutPrecision && bChange) {
              *fProposedSplitPos = fRelSplitPos - fCurTopMargin;
            } else {
              *fProposedSplitPos = fRelSplitPos + fCurBottomMargin;
            }
            bAnyChanged = true;
            bChanged = true;
            if (*fProposedSplitPos <=
                fCurVerticalOffset + kXFALayoutPrecision) {
              return true;
            }
            if (bAnyChanged)
              break;
          }
        }
      }
      return bAnyChanged;
    }
    case XFA_AttributeValue::ContentArea:
    case XFA_AttributeValue::PageArea: {
      *fProposedSplitPos = fCurVerticalOffset;
      return true;
    }
    default:
      return false;
  }
}

CFX_PointF CalculatePositionedContainerPos(CXFA_Node* pNode,
                                           const CFX_SizeF& size) {
  XFA_AttributeValue eAnchorType =
      pNode->JSObject()->GetEnum(XFA_Attribute::AnchorType);
  int32_t nAnchorType = 0;
  switch (eAnchorType) {
    case XFA_AttributeValue::TopLeft:
      nAnchorType = 0;
      break;
    case XFA_AttributeValue::TopCenter:
      nAnchorType = 1;
      break;
    case XFA_AttributeValue::TopRight:
      nAnchorType = 2;
      break;
    case XFA_AttributeValue::MiddleLeft:
      nAnchorType = 3;
      break;
    case XFA_AttributeValue::MiddleCenter:
      nAnchorType = 4;
      break;
    case XFA_AttributeValue::MiddleRight:
      nAnchorType = 5;
      break;
    case XFA_AttributeValue::BottomLeft:
      nAnchorType = 6;
      break;
    case XFA_AttributeValue::BottomCenter:
      nAnchorType = 7;
      break;
    case XFA_AttributeValue::BottomRight:
      nAnchorType = 8;
      break;
    default:
      break;
  }
  static const uint8_t nNextPos[4][9] = {{0, 1, 2, 3, 4, 5, 6, 7, 8},
                                         {6, 3, 0, 7, 4, 1, 8, 5, 2},
                                         {8, 7, 6, 5, 4, 3, 2, 1, 0},
                                         {2, 5, 8, 1, 4, 7, 0, 3, 6}};

  CFX_PointF pos(
      pNode->JSObject()->GetMeasureInUnit(XFA_Attribute::X, XFA_Unit::Pt),
      pNode->JSObject()->GetMeasureInUnit(XFA_Attribute::Y, XFA_Unit::Pt));
  int32_t nRotate =
      XFA_MapRotation(pNode->JSObject()->GetInteger(XFA_Attribute::Rotate)) /
      90;
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

CXFA_ContentLayoutProcessor::CXFA_ContentLayoutProcessor(
    CXFA_Node* pNode,
    CXFA_ViewLayoutProcessor* pViewLayoutProcessor)
    : m_pFormNode(pNode), m_pViewLayoutProcessor(pViewLayoutProcessor) {
  ASSERT(GetFormNode());
  ASSERT(GetFormNode()->IsContainerNode() ||
         GetFormNode()->GetElementType() == XFA_Element::Form);
  m_pOldLayoutItem =
      ToContentLayoutItem(GetFormNode()->JSObject()->GetLayoutItem());
}

CXFA_ContentLayoutProcessor::~CXFA_ContentLayoutProcessor() {}

CXFA_ContentLayoutItem* CXFA_ContentLayoutProcessor::CreateContentLayoutItem(
    CXFA_Node* pFormNode) {
  if (!pFormNode)
    return nullptr;

  CXFA_ContentLayoutItem* pLayoutItem = nullptr;
  if (m_pOldLayoutItem) {
    pLayoutItem = m_pOldLayoutItem;
    m_pOldLayoutItem = m_pOldLayoutItem->GetNext();
    return pLayoutItem;
  }
  CXFA_FFNotify* pNotify = pFormNode->GetDocument()->GetNotify();
  pLayoutItem = pdfium::MakeUnique<CXFA_ContentLayoutItem>(
                    pFormNode, pNotify->OnCreateContentLayoutItem(pFormNode))
                    .release();
  CXFA_ContentLayoutItem* pPrevLayoutItem =
      ToContentLayoutItem(pFormNode->JSObject()->GetLayoutItem());
  if (pPrevLayoutItem) {
    pPrevLayoutItem->GetLast()->InsertAfter(pLayoutItem);
  } else {
    pFormNode->JSObject()->SetLayoutItem(pLayoutItem);
  }
  return pLayoutItem;
}

float CXFA_ContentLayoutProcessor::FindSplitPos(float fProposedSplitPos) {
  ASSERT(m_pLayoutItem);
  auto value = GetFormNode()->JSObject()->TryEnum(XFA_Attribute::Layout, true);
  XFA_AttributeValue eLayout = value.value_or(XFA_AttributeValue::Position);
  bool bCalculateMargin = eLayout != XFA_AttributeValue::Position;
  while (fProposedSplitPos > kXFALayoutPrecision) {
    bool bAppChange = false;
    if (!FindLayoutItemSplitPos(m_pLayoutItem, 0, &fProposedSplitPos,
                                &bAppChange, bCalculateMargin)) {
      break;
    }
  }
  return fProposedSplitPos;
}

void CXFA_ContentLayoutProcessor::SplitLayoutItem(
    CXFA_ContentLayoutItem* pLayoutItem,
    CXFA_ContentLayoutItem* pSecondParent,
    float fSplitPos) {
  float fCurTopMargin = 0;
  float fCurBottomMargin = 0;
  auto value = GetFormNode()->JSObject()->TryEnum(XFA_Attribute::Layout, true);
  XFA_AttributeValue eLayout = value.value_or(XFA_AttributeValue::Position);
  bool bCalculateMargin = true;
  if (eLayout == XFA_AttributeValue::Position)
    bCalculateMargin = false;

  CXFA_Margin* pMarginNode =
      pLayoutItem->GetFormNode()->GetFirstChildByClass<CXFA_Margin>(
          XFA_Element::Margin);
  if (pMarginNode && bCalculateMargin) {
    fCurTopMargin = pMarginNode->JSObject()->GetMeasureInUnit(
        XFA_Attribute::TopInset, XFA_Unit::Pt);
    fCurBottomMargin = pMarginNode->JSObject()->GetMeasureInUnit(
        XFA_Attribute::BottomInset, XFA_Unit::Pt);
  }

  CXFA_ContentLayoutItem* pSecondLayoutItem = nullptr;
  if (m_pCurChildPreprocessor &&
      m_pCurChildPreprocessor->GetFormNode() == pLayoutItem->GetFormNode()) {
    pSecondLayoutItem = m_pCurChildPreprocessor->CreateContentLayoutItem(
        pLayoutItem->GetFormNode());
  } else {
    pSecondLayoutItem = CreateContentLayoutItem(pLayoutItem->GetFormNode());
  }
  pSecondLayoutItem->m_sPos.x = pLayoutItem->m_sPos.x;
  pSecondLayoutItem->m_sSize.width = pLayoutItem->m_sSize.width;
  pSecondLayoutItem->m_sPos.y = 0;
  pSecondLayoutItem->m_sSize.height = pLayoutItem->m_sSize.height - fSplitPos;
  pLayoutItem->m_sSize.height -= pSecondLayoutItem->m_sSize.height;
  if (pLayoutItem->GetFirstChild())
    pSecondLayoutItem->m_sSize.height += fCurTopMargin;

  bool bOrphanedItem = false;
  if (pSecondParent) {
    pSecondParent->AppendLastChild(pSecondLayoutItem);
    if (fCurTopMargin > 0 && pLayoutItem->GetFirstChild()) {
      pSecondParent->m_sSize.height += fCurTopMargin;
      for (CXFA_LayoutItem* pParentIter = pSecondParent->GetParent();
           pParentIter; pParentIter = pParentIter->GetParent()) {
        CXFA_ContentLayoutItem* pContentItem =
            pParentIter->AsContentLayoutItem();
        if (!pContentItem)
          continue;

        pContentItem->m_sSize.height += fCurTopMargin;
      }
    }
  } else if (pLayoutItem->GetParent()) {
    pLayoutItem->GetParent()->InsertAfter(pSecondLayoutItem, pLayoutItem);
  } else {
    // Parentless |pLayoutitem| would like to have |pSecondLayoutItem| as a
    // sibling, but that would violate the tree invariant. Instead, keep
    // it an orphan and add it as a child of |pLayoutItem| after performing
    // the split.
    bOrphanedItem = true;
  }

  std::vector<CXFA_ContentLayoutItem*> children;
  while (auto* pFirst = ToContentLayoutItem(pLayoutItem->GetFirstChild())) {
    children.push_back(pFirst);
    pLayoutItem->RemoveChild(pFirst);
  }

  float lHeightForKeep = 0;
  float fAddMarginHeight = 0;
  std::vector<CXFA_ContentLayoutItem*> keepLayoutItems;
  for (auto* pChildItem : children) {
    if (fSplitPos <= fCurTopMargin + pChildItem->m_sPos.y + fCurBottomMargin +
                         kXFALayoutPrecision) {
      if (!ExistContainerKeep(pChildItem->GetFormNode(), true)) {
        pChildItem->m_sPos.y -= fSplitPos - fCurBottomMargin;
        pChildItem->m_sPos.y += lHeightForKeep;
        pChildItem->m_sPos.y += fAddMarginHeight;
        pSecondLayoutItem->AppendLastChild(pChildItem);
        continue;
      }
      if (lHeightForKeep < kXFALayoutPrecision) {
        for (auto* pPreItem : keepLayoutItems) {
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
          pSecondLayoutItem->AppendLastChild(pPreItem);
        }
      }
      pChildItem->m_sPos.y -= fSplitPos;
      pChildItem->m_sPos.y += lHeightForKeep;
      pChildItem->m_sPos.y += fAddMarginHeight;
      pSecondLayoutItem->AppendLastChild(pChildItem);
      continue;
    }
    if (fSplitPos + kXFALayoutPrecision >= fCurTopMargin + fCurBottomMargin +
                                               pChildItem->m_sPos.y +
                                               pChildItem->m_sSize.height) {
      pLayoutItem->AppendLastChild(pChildItem);
      if (ExistContainerKeep(pChildItem->GetFormNode(), false))
        keepLayoutItems.push_back(pChildItem);
      else
        keepLayoutItems.clear();
      continue;
    }

    float fOldHeight = pSecondLayoutItem->m_sSize.height;
    SplitLayoutItem(
        pChildItem, pSecondLayoutItem,
        fSplitPos - fCurTopMargin - fCurBottomMargin - pChildItem->m_sPos.y);
    fAddMarginHeight = pSecondLayoutItem->m_sSize.height - fOldHeight;
    pLayoutItem->AppendLastChild(pChildItem);
  }
  if (bOrphanedItem)
    pLayoutItem->AppendLastChild(pSecondLayoutItem);
}

void CXFA_ContentLayoutProcessor::SplitLayoutItem(float fSplitPos) {
  ASSERT(m_pLayoutItem);
  SplitLayoutItem(m_pLayoutItem, nullptr, fSplitPos);
}

CXFA_ContentLayoutItem* CXFA_ContentLayoutProcessor::ExtractLayoutItem() {
  CXFA_ContentLayoutItem* pLayoutItem = m_pLayoutItem;
  if (pLayoutItem) {
    m_pLayoutItem = ToContentLayoutItem(pLayoutItem->GetNextSibling());
    pLayoutItem->RemoveSelfIfParented();
  }
  if (m_nCurChildNodeStage != Stage::kDone || !m_pOldLayoutItem)
    return pLayoutItem;

  CXFA_FFNotify* pNotify =
      m_pOldLayoutItem->GetFormNode()->GetDocument()->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(
      m_pOldLayoutItem->GetFormNode()->GetDocument());

  while (m_pOldLayoutItem) {
    CXFA_ContentLayoutItem* pToDeleteItem = m_pOldLayoutItem;
    m_pOldLayoutItem = pToDeleteItem->GetNext();
    if (pToDeleteItem == pLayoutItem)
      break;
    pNotify->OnLayoutItemRemoving(pDocLayout, pToDeleteItem);
    pToDeleteItem->RemoveSelfIfParented();
    delete pToDeleteItem;
  }
  return pLayoutItem;
}

void CXFA_ContentLayoutProcessor::GotoNextContainerNodeSimple(
    bool bUsePageBreak) {
  m_nCurChildNodeStage = GotoNextContainerNode(
      m_nCurChildNodeStage, bUsePageBreak, GetFormNode(), &m_pCurChildNode);
}

CXFA_ContentLayoutProcessor::Stage
CXFA_ContentLayoutProcessor::GotoNextContainerNode(Stage nCurStage,
                                                   bool bUsePageBreak,
                                                   CXFA_Node* pParentContainer,
                                                   CXFA_Node** pCurActionNode) {
  CXFA_Node* pChildContainer = nullptr;
  switch (nCurStage) {
    case Stage::kBreakBefore:
    case Stage::kBreakAfter: {
      pChildContainer = (*pCurActionNode)->GetParent();
      break;
    }
    case Stage::kKeep:
    case Stage::kContainer:
      pChildContainer = *pCurActionNode;
      break;
    default:
      pChildContainer = nullptr;
      break;
  }

  Optional<Stage> ret;
  switch (nCurStage) {
    case Stage::kKeep:
      ret = HandleKeep(pChildContainer->GetFirstChild(), pCurActionNode);
      if (ret.has_value())
        return ret.value();
      break;

    case Stage::kNone:
      *pCurActionNode = nullptr;
      FALLTHROUGH;

    case Stage::kBookendLeader:
      ret = HandleBookendLeader(pParentContainer, pCurActionNode);
      if (ret.has_value())
        return ret.value();

      *pCurActionNode = nullptr;
      FALLTHROUGH;

    case Stage::kBreakBefore:
      ret = HandleBreakBefore(pChildContainer, pCurActionNode);
      if (ret.has_value())
        return ret.value();
      break;

    case Stage::kContainer:
      *pCurActionNode = nullptr;
      FALLTHROUGH;

    case Stage::kBreakAfter:
      ret = HandleBreakAfter(pChildContainer, pCurActionNode);
      if (ret.has_value())
        return ret.value();
      break;

    case Stage::kBookendTrailer:
      ret = HandleBookendTrailer(pParentContainer, pCurActionNode);
      if (ret.has_value())
        return ret.value();
      FALLTHROUGH;

    default:
      *pCurActionNode = nullptr;
      return Stage::kDone;
  }

  ret = HandleCheckNextChildContainer(pParentContainer, pChildContainer,
                                      pCurActionNode);
  if (ret.has_value())
    return ret.value();

  *pCurActionNode = nullptr;
  ret = HandleBookendTrailer(pParentContainer, pCurActionNode);
  if (ret.has_value())
    return ret.value();

  *pCurActionNode = nullptr;
  return Stage::kDone;
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::ProcessKeepNodesForCheckNext(
    CXFA_Node** pCurActionNode,
    CXFA_Node** pNextContainer,
    bool* pLastKeepNode) {
  const bool bCanSplit =
      (*pNextContainer)->GetIntact() == XFA_AttributeValue::None;
  const bool bNextKeep = ExistContainerKeep(*pNextContainer, false);

  if (bNextKeep && !bCanSplit) {
    if (!m_bIsProcessKeep && !m_bKeepBreakFinish) {
      m_pKeepHeadNode = *pNextContainer;
      m_bIsProcessKeep = true;
    }
    return {};
  }

  if (!m_bIsProcessKeep || !m_pKeepHeadNode) {
    if (m_bKeepBreakFinish)
      *pLastKeepNode = true;
    m_bKeepBreakFinish = false;
    return {};
  }

  m_pKeepTailNode = *pNextContainer;
  if (m_bKeepBreakFinish) {
    *pNextContainer = m_pKeepHeadNode;
    ProcessKeepNodesEnd();
    return {};
  }

  Optional<Stage> ret =
      FindBreakNode((*pNextContainer)->GetFirstChild(), true, pCurActionNode);
  if (!ret.has_value()) {
    *pNextContainer = m_pKeepHeadNode;
    ProcessKeepNodesEnd();
    return {};
  }

  return ret;
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::ProcessKeepNodesForBreakBefore(
    CXFA_Node** pCurActionNode,
    CXFA_Node* pContainerNode) {
  if (m_pKeepTailNode == pContainerNode) {
    *pCurActionNode = m_pKeepHeadNode;
    ProcessKeepNodesEnd();
    return Stage::kContainer;
  }

  CXFA_Node* pBreakAfterNode = pContainerNode->GetFirstChild();
  return FindBreakNode(pBreakAfterNode, false, pCurActionNode);
}

void CXFA_ContentLayoutProcessor::DoLayoutPageArea(
    CXFA_ViewLayoutItem* pPageAreaLayoutItem) {
  CXFA_Node* pFormNode = pPageAreaLayoutItem->GetFormNode();
  CXFA_Node* pCurChildNode = nullptr;
  CXFA_LayoutItem* pBeforeItem = nullptr;
  for (Stage nCurChildNodeStage = GotoNextContainerNode(
           Stage::kNone, false, pFormNode, &pCurChildNode);
       pCurChildNode;
       nCurChildNodeStage = GotoNextContainerNode(nCurChildNodeStage, false,
                                                  pFormNode, &pCurChildNode)) {
    if (nCurChildNodeStage != Stage::kContainer)
      continue;
    if (pCurChildNode->GetElementType() == XFA_Element::Variables)
      continue;

    auto pProcessor =
        pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(pCurChildNode, nullptr);
    pProcessor->DoLayout(false, FLT_MAX, FLT_MAX);
    if (!pProcessor->HasLayoutItem())
      continue;

    pProcessor->SetCurrentComponentPos(CalculatePositionedContainerPos(
        pCurChildNode, pProcessor->GetCurrentComponentSize()));
    CXFA_LayoutItem* pProcessItem = pProcessor->ExtractLayoutItem();
    if (!pBeforeItem)
      pPageAreaLayoutItem->AppendFirstChild(pProcessItem);
    else
      pPageAreaLayoutItem->InsertAfter(pProcessItem, pBeforeItem);

    pBeforeItem = pProcessItem;
  }

  pBeforeItem = nullptr;
  CXFA_LayoutItem* pLayoutItem = pPageAreaLayoutItem->GetFirstChild();
  while (pLayoutItem) {
    if (!pLayoutItem->IsContentLayoutItem() ||
        pLayoutItem->GetFormNode()->GetElementType() != XFA_Element::Draw) {
      pLayoutItem = pLayoutItem->GetNextSibling();
      continue;
    }
    if (pLayoutItem->GetFormNode()->GetElementType() != XFA_Element::Draw)
      continue;

    CXFA_LayoutItem* pNextLayoutItem = pLayoutItem->GetNextSibling();
    pPageAreaLayoutItem->RemoveChild(pLayoutItem);
    if (!pBeforeItem)
      pPageAreaLayoutItem->AppendFirstChild(pLayoutItem);
    else
      pPageAreaLayoutItem->InsertAfter(pLayoutItem, pBeforeItem);

    pBeforeItem = pLayoutItem;
    pLayoutItem = pNextLayoutItem;
  }
}

void CXFA_ContentLayoutProcessor::DoLayoutPositionedContainer(
    Context* pContext) {
  if (m_pLayoutItem)
    return;

  m_pLayoutItem = CreateContentLayoutItem(GetFormNode());
  auto value = GetFormNode()->JSObject()->TryEnum(XFA_Attribute::Layout, true);
  bool bIgnoreXY = value.value_or(XFA_AttributeValue::Position) !=
                   XFA_AttributeValue::Position;
  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      GetFormNode(), &bContainerWidthAutoSize, &bContainerHeightAutoSize);

  float fContentCalculatedWidth = 0;
  float fContentCalculatedHeight = 0;
  float fHiddenContentCalculatedWidth = 0;
  float fHiddenContentCalculatedHeight = 0;
  if (!m_pCurChildNode)
    GotoNextContainerNodeSimple(false);

  int32_t iColIndex = 0;
  for (; m_pCurChildNode; GotoNextContainerNodeSimple(false)) {
    if (m_nCurChildNodeStage != Stage::kContainer)
      continue;
    if (m_pCurChildNode->GetElementType() == XFA_Element::Variables)
      continue;

    auto pProcessor = pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
        m_pCurChildNode, m_pViewLayoutProcessor.Get());
    if (pContext && pContext->m_prgSpecifiedColumnWidths) {
      int32_t iColSpan =
          m_pCurChildNode->JSObject()->GetInteger(XFA_Attribute::ColSpan);
      if (iColSpan <= pdfium::CollectionSize<int32_t>(
                          *pContext->m_prgSpecifiedColumnWidths) -
                          iColIndex) {
        pContext->m_fCurColumnWidth = 0.0f;
        if (iColSpan == -1) {
          iColSpan = pdfium::CollectionSize<int32_t>(
              *pContext->m_prgSpecifiedColumnWidths);
        }
        for (int32_t i = 0; iColIndex + i < iColSpan; ++i) {
          pContext->m_fCurColumnWidth.value() +=
              (*pContext->m_prgSpecifiedColumnWidths)[iColIndex + i];
        }
        if (pContext->m_fCurColumnWidth.value() == 0)
          pContext->m_fCurColumnWidth.reset();

        iColIndex += iColSpan >= 0 ? iColSpan : 0;
      }
    }

    pProcessor->DoLayoutInternal(false, FLT_MAX, FLT_MAX, pContext);
    if (!pProcessor->HasLayoutItem())
      continue;

    CFX_SizeF size = pProcessor->GetCurrentComponentSize();
    bool bChangeParentSize = false;
    if (m_pCurChildNode->PresenceRequiresSpace())
      bChangeParentSize = true;

    CFX_PointF absolutePos;
    if (!bIgnoreXY)
      absolutePos = CalculatePositionedContainerPos(m_pCurChildNode, size);

    pProcessor->SetCurrentComponentPos(absolutePos);
    if (bContainerWidthAutoSize) {
      float fChildSuppliedWidth = absolutePos.x + size.width;
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
      float fChildSuppliedHeight = absolutePos.y + size.height;
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
    m_pLayoutItem->AppendLastChild(pProcessor->ExtractLayoutItem());
  }

  XFA_VERSION eVersion = GetFormNode()->GetDocument()->GetCurVersionMode();
  if (fContentCalculatedWidth == 0 && eVersion < XFA_VERSION_207)
    fContentCalculatedWidth = fHiddenContentCalculatedWidth;
  if (fContentCalculatedHeight == 0 && eVersion < XFA_VERSION_207)
    fContentCalculatedHeight = fHiddenContentCalculatedHeight;

  containerSize = CalculateContainerComponentSizeFromContentSize(
      GetFormNode(), bContainerWidthAutoSize, fContentCalculatedWidth,
      bContainerHeightAutoSize, fContentCalculatedHeight, containerSize);
  SetCurrentComponentSize(containerSize);
}

void CXFA_ContentLayoutProcessor::DoLayoutTableContainer(
    CXFA_Node* pLayoutNode) {
  if (m_pLayoutItem)
    return;
  if (!pLayoutNode)
    pLayoutNode = GetFormNode();

  ASSERT(!m_pCurChildNode);

  m_pLayoutItem = CreateContentLayoutItem(GetFormNode());
  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      GetFormNode(), &bContainerWidthAutoSize, &bContainerHeightAutoSize);
  float fContentCalculatedWidth = 0;
  float fContentCalculatedHeight = 0;
  CXFA_Margin* pMarginNode =
      GetFormNode()->GetFirstChildByClass<CXFA_Margin>(XFA_Element::Margin);
  float fLeftInset = 0;
  float fRightInset = 0;
  if (pMarginNode) {
    fLeftInset = pMarginNode->JSObject()->GetMeasureInUnit(
        XFA_Attribute::LeftInset, XFA_Unit::Pt);
    fRightInset = pMarginNode->JSObject()->GetMeasureInUnit(
        XFA_Attribute::RightInset, XFA_Unit::Pt);
  }

  float fContentWidthLimit =
      bContainerWidthAutoSize ? FLT_MAX
                              : containerSize.width - fLeftInset - fRightInset;
  WideString wsColumnWidths =
      pLayoutNode->JSObject()->GetCData(XFA_Attribute::ColumnWidths);
  if (!wsColumnWidths.IsEmpty()) {
    for (auto& width : SeparateStringOnSpace(wsColumnWidths.span())) {
      width.TrimLeft(L' ');
      if (width.IsEmpty())
        continue;

      m_rgSpecifiedColumnWidths.push_back(
          CXFA_Measurement(width.AsStringView()).ToUnit(XFA_Unit::Pt));
    }
  }

  int32_t iSpecifiedColumnCount =
      pdfium::CollectionSize<int32_t>(m_rgSpecifiedColumnWidths);
  Context layoutContext;
  layoutContext.m_prgSpecifiedColumnWidths = &m_rgSpecifiedColumnWidths;
  Context* pLayoutContext =
      iSpecifiedColumnCount > 0 ? &layoutContext : nullptr;
  if (!m_pCurChildNode)
    GotoNextContainerNodeSimple(false);

  for (; m_pCurChildNode; GotoNextContainerNodeSimple(false)) {
    layoutContext.m_fCurColumnWidth.reset();
    if (m_nCurChildNodeStage != Stage::kContainer)
      continue;

    auto pProcessor = pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
        m_pCurChildNode, m_pViewLayoutProcessor.Get());
    pProcessor->DoLayoutInternal(false, FLT_MAX, FLT_MAX, pLayoutContext);
    if (!pProcessor->HasLayoutItem())
      continue;

    m_pLayoutItem->AppendLastChild(pProcessor->ExtractLayoutItem());
  }

  int32_t iRowCount = 0;
  int32_t iColCount = 0;
  {
    std::vector<CXFA_ContentLayoutItem*> rgRowItems;
    std::vector<int32_t> rgRowItemsSpan;
    std::vector<float> rgRowItemsWidth;
    for (CXFA_LayoutItem* pIter = m_pLayoutItem->GetFirstChild(); pIter;
         pIter = pIter->GetNextSibling()) {
      CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
      if (!pLayoutChild)
        continue;
      if (pLayoutChild->GetFormNode()->GetElementType() != XFA_Element::Subform)
        continue;
      if (!pLayoutChild->GetFormNode()->PresenceRequiresSpace())
        continue;

      XFA_AttributeValue eLayout =
          pLayoutChild->GetFormNode()->JSObject()->GetEnum(
              XFA_Attribute::Layout);
      if (eLayout != XFA_AttributeValue::Row &&
          eLayout != XFA_AttributeValue::Rl_row) {
        continue;
      }
      CXFA_ContentLayoutItem* pRowLayoutCell =
          ToContentLayoutItem(pLayoutChild->GetFirstChild());
      if (pRowLayoutCell) {
        rgRowItems.push_back(pRowLayoutCell);
        int32_t iColSpan =
            pRowLayoutCell->GetFormNode()->JSObject()->GetInteger(
                XFA_Attribute::ColSpan);
        rgRowItemsSpan.push_back(iColSpan);
        rgRowItemsWidth.push_back(pRowLayoutCell->m_sSize.width);
      }
    }

    iRowCount = pdfium::CollectionSize<int32_t>(rgRowItems);
    iColCount = 0;
    bool bMoreColumns = true;
    while (bMoreColumns) {
      bMoreColumns = false;
      bool bAutoCol = false;
      for (int32_t i = 0; i < iRowCount; i++) {
        while (rgRowItems[i] &&
               (rgRowItemsSpan[i] <= 0 ||
                !rgRowItems[i]->GetFormNode()->PresenceRequiresSpace())) {
          CXFA_ContentLayoutItem* pNewCell =
              ToContentLayoutItem(rgRowItems[i]->GetNextSibling());
          if (rgRowItemsSpan[i] < 0 &&
              rgRowItems[i]->GetFormNode()->PresenceRequiresSpace()) {
            pNewCell = nullptr;
          }
          rgRowItems[i] = pNewCell;
          rgRowItemsSpan[i] =
              pNewCell ? pNewCell->GetFormNode()->JSObject()->GetInteger(
                             XFA_Attribute::ColSpan)
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
          int32_t c =
              iColCount + 1 -
              pdfium::CollectionSize<int32_t>(m_rgSpecifiedColumnWidths);
          for (int32_t j = 0; j < c; j++)
            m_rgSpecifiedColumnWidths.push_back(0);
        }
        if (m_rgSpecifiedColumnWidths[iColCount] < kXFALayoutPrecision)
          bAutoCol = true;
        if (bAutoCol &&
            m_rgSpecifiedColumnWidths[iColCount] < rgRowItemsWidth[i]) {
          m_rgSpecifiedColumnWidths[iColCount] = rgRowItemsWidth[i];
        }
      }

      if (!bMoreColumns)
        continue;

      float fFinalColumnWidth = 0.0f;
      if (pdfium::IndexInBounds(m_rgSpecifiedColumnWidths, iColCount))
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

  float fCurrentRowY = 0;
  for (CXFA_LayoutItem* pIter = m_pLayoutItem->GetFirstChild(); pIter;
       pIter = pIter->GetNextSibling()) {
    CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
    if (!pLayoutChild)
      continue;
    if (!pLayoutChild->GetFormNode()->PresenceRequiresSpace())
      continue;

    if (pLayoutChild->GetFormNode()->GetElementType() == XFA_Element::Subform) {
      XFA_AttributeValue eSubformLayout =
          pLayoutChild->GetFormNode()->JSObject()->GetEnum(
              XFA_Attribute::Layout);
      if (eSubformLayout == XFA_AttributeValue::Row ||
          eSubformLayout == XFA_AttributeValue::Rl_row) {
        RelocateTableRowCells(pLayoutChild, m_rgSpecifiedColumnWidths,
                              eSubformLayout);
      }
    }

    pLayoutChild->m_sPos.y = fCurrentRowY;
    if (bContainerWidthAutoSize) {
      pLayoutChild->m_sPos.x = 0;
    } else {
      switch (pLayoutChild->GetFormNode()->JSObject()->GetEnum(
          XFA_Attribute::HAlign)) {
        case XFA_AttributeValue::Center:
          pLayoutChild->m_sPos.x =
              (fContentWidthLimit - pLayoutChild->m_sSize.width) / 2;
          break;
        case XFA_AttributeValue::Right:
          pLayoutChild->m_sPos.x =
              fContentWidthLimit - pLayoutChild->m_sSize.width;
          break;
        case XFA_AttributeValue::Left:
        default:
          pLayoutChild->m_sPos.x = 0;
          break;
      }
    }

    if (bContainerWidthAutoSize) {
      float fChildSuppliedWidth =
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
      GetFormNode(), bContainerWidthAutoSize, fContentCalculatedWidth,
      bContainerHeightAutoSize, fContentCalculatedHeight, containerSize);
  SetCurrentComponentSize(containerSize);
}

bool CXFA_ContentLayoutProcessor::IsAddNewRowForTrailer(
    CXFA_ContentLayoutItem* pTrailerItem) {
  if (!pTrailerItem)
    return false;

  float fWidth = pTrailerItem->m_sSize.width;
  XFA_AttributeValue eLayout =
      GetFormNode()->JSObject()->GetEnum(XFA_Attribute::Layout);
  return eLayout == XFA_AttributeValue::Tb || m_fWidthLimit <= fWidth;
}

float CXFA_ContentLayoutProcessor::InsertKeepLayoutItems() {
  if (m_arrayKeepItems.empty())
    return 0;

  if (!m_pLayoutItem) {
    m_pLayoutItem = CreateContentLayoutItem(GetFormNode());
    m_pLayoutItem->m_sSize.clear();
  }

  float fTotalHeight = 0;
  for (auto iter = m_arrayKeepItems.rbegin(); iter != m_arrayKeepItems.rend();
       iter++) {
    AddLeaderAfterSplit(*iter);
    fTotalHeight += (*iter)->m_sSize.height;
  }
  m_arrayKeepItems.clear();

  return fTotalHeight;
}

bool CXFA_ContentLayoutProcessor::ProcessKeepForSplit(
    CXFA_ContentLayoutProcessor* pChildProcessor,
    Result eRetValue,
    std::vector<CXFA_ContentLayoutItem*>* rgCurLineLayoutItem,
    float* fContentCurRowAvailWidth,
    float* fContentCurRowHeight,
    float* fContentCurRowY,
    bool* bAddedItemInRow,
    bool* bForceEndPage,
    Result* result) {
  if (!pChildProcessor)
    return false;

  if (m_pCurChildNode->GetIntact() == XFA_AttributeValue::None &&
      pChildProcessor->m_bHasAvailHeight)
    return false;

  if (!ExistContainerKeep(m_pCurChildNode, true))
    return false;

  CFX_SizeF childSize = pChildProcessor->GetCurrentComponentSize();
  std::vector<CXFA_ContentLayoutItem*> keepLayoutItems;
  if (JudgePutNextPage(m_pLayoutItem, childSize.height, &keepLayoutItems)) {
    m_arrayKeepItems.clear();

    for (auto* item : keepLayoutItems) {
      m_pLayoutItem->RemoveChild(item);
      *fContentCurRowY -= item->m_sSize.height;
      m_arrayKeepItems.push_back(item);
    }
    *bAddedItemInRow = true;
    *bForceEndPage = true;
    *result = Result::kPageFullBreak;
    return true;
  }

  rgCurLineLayoutItem->push_back(pChildProcessor->ExtractLayoutItem());
  *bAddedItemInRow = true;
  *fContentCurRowAvailWidth -= childSize.width;
  *fContentCurRowHeight = std::max(*fContentCurRowHeight, childSize.height);
  *result = eRetValue;
  return true;
}

bool CXFA_ContentLayoutProcessor::JudgePutNextPage(
    CXFA_ContentLayoutItem* pParentLayoutItem,
    float fChildHeight,
    std::vector<CXFA_ContentLayoutItem*>* pKeepItems) {
  if (!pParentLayoutItem)
    return false;

  float fItemsHeight = 0;
  for (CXFA_LayoutItem* pIter = pParentLayoutItem->GetFirstChild(); pIter;
       pIter = pIter->GetNextSibling()) {
    CXFA_ContentLayoutItem* pChildLayoutItem = pIter->AsContentLayoutItem();
    if (!pChildLayoutItem)
      continue;

    if (ExistContainerKeep(pChildLayoutItem->GetFormNode(), false)) {
      pKeepItems->push_back(pChildLayoutItem);
      fItemsHeight += pChildLayoutItem->m_sSize.height;
    } else {
      pKeepItems->clear();
      fItemsHeight = 0;
    }
  }
  fItemsHeight += fChildHeight;
  return m_pViewLayoutProcessor->GetNextAvailContentHeight(fItemsHeight);
}

void CXFA_ContentLayoutProcessor::ProcessUnUseBinds(CXFA_Node* pFormNode) {
  if (!pFormNode)
    return;

  CXFA_NodeIterator sIterator(pFormNode);
  for (CXFA_Node* pNode = sIterator.MoveToNext(); pNode;
       pNode = sIterator.MoveToNext()) {
    if (pNode->IsContainerNode()) {
      CXFA_Node* pBindNode = pNode->GetBindData();
      if (pBindNode) {
        pBindNode->RemoveBindItem(pNode);
        pNode->SetBindingNode(nullptr);
      }
    }
    pNode->SetFlag(XFA_NodeFlag_UnusedNode);
  }
}

void CXFA_ContentLayoutProcessor::ProcessUnUseOverFlow(
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
    pFormNode = pFormNode->GetParent();
  }
  if (pLeaderNode && pFormNode)
    pFormNode->RemoveChildAndNotify(pLeaderNode, true);
  if (pTrailerNode && pFormNode)
    pFormNode->RemoveChildAndNotify(pTrailerNode, true);
  if (pTrailerItem)
    XFA_ReleaseLayoutItem(pTrailerItem);
}

CXFA_ContentLayoutProcessor::Result
CXFA_ContentLayoutProcessor::DoLayoutFlowedContainer(
    bool bUseBreakControl,
    XFA_AttributeValue eFlowStrategy,
    float fHeightLimit,
    float fRealHeight,
    Context* pContext,
    bool bRootForceTb) {
  m_bHasAvailHeight = true;
  if (m_pCurChildPreprocessor)
    m_pCurChildPreprocessor->m_ePreProcessRs = Result::kDone;

  bool bContainerWidthAutoSize = true;
  bool bContainerHeightAutoSize = true;
  CFX_SizeF container_size = CalculateContainerSpecifiedSize(
      GetFormNode(), &bContainerWidthAutoSize, &bContainerHeightAutoSize);
  AdjustContainerSpecifiedSize(pContext, &container_size,
                               &bContainerWidthAutoSize,
                               &bContainerHeightAutoSize);

  CXFA_Margin* pMargin =
      GetFormNode()->GetFirstChildByClass<CXFA_Margin>(XFA_Element::Margin);
  CFX_FloatRect inset = GetMarginInset(pMargin);
  float fContentWidthLimit =
      bContainerWidthAutoSize ? FLT_MAX
                              : container_size.width - inset.left - inset.right;
  float fAvailHeight = fHeightLimit - inset.top - inset.bottom;
  if (fAvailHeight < 0)
    m_bHasAvailHeight = false;

  fRealHeight = fRealHeight - inset.top - inset.bottom;
  CFX_SizeF calculated_size;
  float fContentCurRowY = 0;
  CXFA_ContentLayoutItem* pLastChild = nullptr;
  if (m_pLayoutItem) {
    pLastChild = FindLastContentLayoutItem(eFlowStrategy);
    calculated_size = CalculateLayoutItemSize(pLastChild);
    fContentCurRowY =
        pLastChild ? pLastChild->m_sPos.y : calculated_size.height;
  }

  fContentCurRowY += InsertKeepLayoutItems();
  if (m_nCurChildNodeStage == Stage::kNone)
    GotoNextContainerNodeSimple(true);

  fContentCurRowY += InsertPendingItems(GetFormNode());
  if (m_pCurChildPreprocessor && m_nCurChildNodeStage == Stage::kContainer) {
    if (ExistContainerKeep(m_pCurChildPreprocessor->GetFormNode(), false)) {
      m_pKeepHeadNode = m_pCurChildNode;
      m_bIsProcessKeep = true;
      m_nCurChildNodeStage = Stage::kKeep;
    }
  }

  bool bForceEndPage = false;
  bool bBreakDone = false;
  bool bIsManualBreak = false;
  while (m_nCurChildNodeStage != Stage::kDone) {
    float fContentCurRowHeight = 0;
    float fContentCurRowAvailWidth = fContentWidthLimit;
    m_fWidthLimit = fContentCurRowAvailWidth;
    std::vector<CXFA_ContentLayoutItem*> rgCurLineLayoutItems[3];
    uint8_t uCurHAlignState =
        (eFlowStrategy != XFA_AttributeValue::Rl_tb ? 0 : 2);
    if (pLastChild) {
      for (CXFA_LayoutItem* pNext = pLastChild; pNext;
           pNext = pNext->GetNextSibling()) {
        CXFA_ContentLayoutItem* pLayoutNext = pNext->AsContentLayoutItem();
        if (!pLayoutNext)
          continue;
        if (!pLayoutNext->GetNextSibling() && m_pCurChildPreprocessor &&
            m_pCurChildPreprocessor->GetFormNode() ==
                pLayoutNext->GetFormNode()) {
          if (m_pCurChildPreprocessor->m_pLayoutItem)
            pLayoutNext->InsertAfter(m_pCurChildPreprocessor->m_pLayoutItem);
          m_pCurChildPreprocessor->m_pLayoutItem = pLayoutNext;
          break;
        }
        uint8_t uHAlign =
            HAlignEnumToInt(pLayoutNext->GetFormNode()->JSObject()->GetEnum(
                XFA_Attribute::HAlign));
        rgCurLineLayoutItems[uHAlign].push_back(pLayoutNext);
        if (eFlowStrategy == XFA_AttributeValue::Lr_tb) {
          if (uHAlign > uCurHAlignState)
            uCurHAlignState = uHAlign;
        } else if (uHAlign < uCurHAlignState) {
          uCurHAlignState = uHAlign;
        }
        if (pLayoutNext->GetFormNode()->PresenceRequiresSpace()) {
          if (pLayoutNext->m_sSize.height > fContentCurRowHeight)
            fContentCurRowHeight = pLayoutNext->m_sSize.height;
          fContentCurRowAvailWidth -= pLayoutNext->m_sSize.width;
        }
      }

      CXFA_ContentLayoutItem* pLayoutNextTemp = pLastChild;
      while (pLayoutNextTemp) {
        CXFA_ContentLayoutItem* pSaveLayoutNext =
            ToContentLayoutItem(pLayoutNextTemp->GetNextSibling());
        pLayoutNextTemp->RemoveSelfIfParented();
        pLayoutNextTemp = pSaveLayoutNext;
      }
      pLastChild = nullptr;
    }

    while (m_pCurChildNode) {
      std::unique_ptr<CXFA_ContentLayoutProcessor> pProcessor;
      bool bAddedItemInRow = false;
      fContentCurRowY += InsertPendingItems(GetFormNode());
      switch (m_nCurChildNodeStage) {
        case Stage::kKeep:
        case Stage::kNone:
          break;
        case Stage::kBreakBefore: {
          for (auto* item : m_arrayKeepItems) {
            m_pLayoutItem->RemoveChild(item);
            calculated_size.height -= item->m_sSize.height;
          }

          if (!bUseBreakControl || !m_pViewLayoutProcessor)
            break;

          Optional<CXFA_ViewLayoutProcessor::BreakData> break_data =
              m_pViewLayoutProcessor->ProcessBreakBefore(m_pCurChildNode);
          if (!break_data.has_value() || !break_data.value().bCreatePage ||
              GetFormNode()->GetElementType() == XFA_Element::Form) {
            break;
          }

          CXFA_Node* pLeaderNode = break_data.value().pLeader;
          CXFA_Node* pTrailerNode = break_data.value().pTrailer;
          if (JudgeLeaderOrTrailerForOccur(pLeaderNode))
            AddPendingNode(pLeaderNode, true);

          if (JudgeLeaderOrTrailerForOccur(pTrailerNode)) {
            if (GetFormNode()->GetParent()->GetElementType() ==
                    XFA_Element::Form &&
                !m_pLayoutItem) {
              AddPendingNode(pTrailerNode, true);
            } else {
              auto pTempProcessor =
                  pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(pTrailerNode,
                                                                  nullptr);
              InsertFlowedItem(
                  pTempProcessor.get(), bContainerWidthAutoSize,
                  bContainerHeightAutoSize, container_size.height,
                  eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems, false,
                  FLT_MAX, FLT_MAX, fContentWidthLimit, &fContentCurRowY,
                  &fContentCurRowAvailWidth, &fContentCurRowHeight,
                  &bAddedItemInRow, &bForceEndPage, pContext, false);
            }
          }
          GotoNextContainerNodeSimple(true);
          bForceEndPage = true;
          bIsManualBreak = true;
          goto SuspendAndCreateNewRow;
        }
        case Stage::kBreakAfter: {
          if (!bUseBreakControl || !m_pViewLayoutProcessor)
            break;

          Optional<CXFA_ViewLayoutProcessor::BreakData> break_data =
              m_pViewLayoutProcessor->ProcessBreakAfter(m_pCurChildNode);
          if (!break_data.has_value() ||
              GetFormNode()->GetElementType() == XFA_Element::Form) {
            break;
          }

          CXFA_Node* pLeaderNode = break_data.value().pLeader;
          CXFA_Node* pTrailerNode = break_data.value().pTrailer;
          bool bCreatePage = break_data.value().bCreatePage;
          if (JudgeLeaderOrTrailerForOccur(pTrailerNode)) {
            auto pTempProcessor =
                pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(pTrailerNode,
                                                                nullptr);
            InsertFlowedItem(pTempProcessor.get(), bContainerWidthAutoSize,
                             bContainerHeightAutoSize, container_size.height,
                             eFlowStrategy, &uCurHAlignState,
                             rgCurLineLayoutItems, false, FLT_MAX, FLT_MAX,
                             fContentWidthLimit, &fContentCurRowY,
                             &fContentCurRowAvailWidth, &fContentCurRowHeight,
                             &bAddedItemInRow, &bForceEndPage, pContext, false);
          }
          if (!bCreatePage) {
            if (JudgeLeaderOrTrailerForOccur(pLeaderNode)) {
              CalculateRowChildPosition(
                  rgCurLineLayoutItems, eFlowStrategy, bContainerHeightAutoSize,
                  bContainerWidthAutoSize, &calculated_size.width,
                  &calculated_size.height, &fContentCurRowY,
                  fContentCurRowHeight, fContentWidthLimit, false);
              rgCurLineLayoutItems->clear();
              auto pTempProcessor =
                  pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(pLeaderNode,
                                                                  nullptr);
              InsertFlowedItem(
                  pTempProcessor.get(), bContainerWidthAutoSize,
                  bContainerHeightAutoSize, container_size.height,
                  eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems, false,
                  FLT_MAX, FLT_MAX, fContentWidthLimit, &fContentCurRowY,
                  &fContentCurRowAvailWidth, &fContentCurRowHeight,
                  &bAddedItemInRow, &bForceEndPage, pContext, false);
            }
          } else {
            if (JudgeLeaderOrTrailerForOccur(pLeaderNode))
              AddPendingNode(pLeaderNode, true);
          }

          GotoNextContainerNodeSimple(true);
          if (bCreatePage) {
            bForceEndPage = true;
            bIsManualBreak = true;
            if (m_nCurChildNodeStage == Stage::kDone)
              bBreakDone = true;
          }
          goto SuspendAndCreateNewRow;
        }
        case Stage::kBookendLeader: {
          if (m_pCurChildPreprocessor) {
            pProcessor = std::move(m_pCurChildPreprocessor);
          } else if (m_pViewLayoutProcessor) {
            CXFA_Node* pLeaderNode =
                m_pViewLayoutProcessor->ProcessBookendLeader(m_pCurChildNode);
            if (pLeaderNode) {
              pProcessor = pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
                  pLeaderNode, m_pViewLayoutProcessor.Get());
            }
          }

          if (pProcessor) {
            if (InsertFlowedItem(
                    pProcessor.get(), bContainerWidthAutoSize,
                    bContainerHeightAutoSize, container_size.height,
                    eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems,
                    bUseBreakControl, fAvailHeight, fRealHeight,
                    fContentWidthLimit, &fContentCurRowY,
                    &fContentCurRowAvailWidth, &fContentCurRowHeight,
                    &bAddedItemInRow, &bForceEndPage, pContext,
                    false) != Result::kDone) {
              goto SuspendAndCreateNewRow;
            }
            pProcessor.reset();
          }
          break;
        }
        case Stage::kBookendTrailer: {
          if (m_pCurChildPreprocessor) {
            pProcessor = std::move(m_pCurChildPreprocessor);
          } else if (m_pViewLayoutProcessor) {
            CXFA_Node* pTrailerNode =
                m_pViewLayoutProcessor->ProcessBookendTrailer(m_pCurChildNode);
            if (pTrailerNode) {
              pProcessor = pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
                  pTrailerNode, m_pViewLayoutProcessor.Get());
            }
          }
          if (pProcessor) {
            if (InsertFlowedItem(
                    pProcessor.get(), bContainerWidthAutoSize,
                    bContainerHeightAutoSize, container_size.height,
                    eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems,
                    bUseBreakControl, fAvailHeight, fRealHeight,
                    fContentWidthLimit, &fContentCurRowY,
                    &fContentCurRowAvailWidth, &fContentCurRowHeight,
                    &bAddedItemInRow, &bForceEndPage, pContext,
                    false) != Result::kDone) {
              goto SuspendAndCreateNewRow;
            }
            pProcessor.reset();
          }
          break;
        }
        case Stage::kContainer: {
          ASSERT(m_pCurChildNode->IsContainerNode());
          if (m_pCurChildNode->GetElementType() == XFA_Element::Variables)
            break;
          if (fContentCurRowY >= fHeightLimit + kXFALayoutPrecision &&
              m_pCurChildNode->PresenceRequiresSpace()) {
            bForceEndPage = true;
            goto SuspendAndCreateNewRow;
          }
          if (!m_pCurChildNode->IsContainerNode())
            break;

          bool bNewRow = false;
          if (m_pCurChildPreprocessor) {
            pProcessor = std::move(m_pCurChildPreprocessor);
            bNewRow = true;
          } else {
            pProcessor = pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
                m_pCurChildNode, m_pViewLayoutProcessor.Get());
          }

          pProcessor->InsertPendingItems(m_pCurChildNode);
          Result rs = InsertFlowedItem(
              pProcessor.get(), bContainerWidthAutoSize,
              bContainerHeightAutoSize, container_size.height, eFlowStrategy,
              &uCurHAlignState, rgCurLineLayoutItems, bUseBreakControl,
              fAvailHeight, fRealHeight, fContentWidthLimit, &fContentCurRowY,
              &fContentCurRowAvailWidth, &fContentCurRowHeight,
              &bAddedItemInRow, &bForceEndPage, pContext, bNewRow);
          switch (rs) {
            case Result::kManualBreak:
              bIsManualBreak = true;
              FALLTHROUGH;
            case Result::kPageFullBreak:
              bForceEndPage = true;
              FALLTHROUGH;
            case Result::kRowFullBreak:
              goto SuspendAndCreateNewRow;
            case Result::kDone:
            default:
              fContentCurRowY +=
                  pProcessor->InsertPendingItems(m_pCurChildNode);
              pProcessor.reset();
              break;
          }
          break;
        }
        case Stage::kDone:
          break;
        default:
          break;
      }
      GotoNextContainerNodeSimple(true);
      if (bAddedItemInRow && eFlowStrategy == XFA_AttributeValue::Tb)
        break;
      continue;
    SuspendAndCreateNewRow:
      if (pProcessor)
        m_pCurChildPreprocessor = std::move(pProcessor);
      break;
    }

    CalculateRowChildPosition(rgCurLineLayoutItems, eFlowStrategy,
                              bContainerHeightAutoSize, bContainerWidthAutoSize,
                              &calculated_size.width, &calculated_size.height,
                              &fContentCurRowY, fContentCurRowHeight,
                              fContentWidthLimit, bRootForceTb);
    m_fWidthLimit = fContentCurRowAvailWidth;
    if (bForceEndPage)
      break;
  }

  bool bRetValue =
      m_nCurChildNodeStage == Stage::kDone && m_PendingNodes.empty();
  if (bBreakDone)
    bRetValue = false;

  container_size = CalculateContainerComponentSizeFromContentSize(
      GetFormNode(), bContainerWidthAutoSize, calculated_size.width,
      bContainerHeightAutoSize, calculated_size.height, container_size);

  if (container_size.height >= kXFALayoutPrecision || m_pLayoutItem ||
      bRetValue) {
    if (!m_pLayoutItem)
      m_pLayoutItem = CreateContentLayoutItem(GetFormNode());
    container_size.height = std::max(container_size.height, 0.f);

    SetCurrentComponentSize(container_size);
    if (bForceEndPage)
      m_fUsedSize = 0;
    else
      m_fUsedSize += m_pLayoutItem->m_sSize.height;
  }

  if (bRetValue)
    return Result::kDone;
  return bIsManualBreak ? Result::kManualBreak : Result::kPageFullBreak;
}

bool CXFA_ContentLayoutProcessor::CalculateRowChildPosition(
    std::vector<CXFA_ContentLayoutItem*> (&rgCurLineLayoutItems)[3],
    XFA_AttributeValue eFlowStrategy,
    bool bContainerHeightAutoSize,
    bool bContainerWidthAutoSize,
    float* fContentCalculatedWidth,
    float* fContentCalculatedHeight,
    float* fContentCurRowY,
    float fContentCurRowHeight,
    float fContentWidthLimit,
    bool bRootForceTb) {
  int32_t nGroupLengths[3] = {0, 0, 0};
  float fGroupWidths[3] = {0, 0, 0};
  int32_t nTotalLength = 0;
  for (int32_t i = 0; i < 3; i++) {
    nGroupLengths[i] = pdfium::CollectionSize<int32_t>(rgCurLineLayoutItems[i]);
    for (int32_t c = nGroupLengths[i], j = 0; j < c; j++) {
      nTotalLength++;
      if (rgCurLineLayoutItems[i][j]->GetFormNode()->PresenceRequiresSpace())
        fGroupWidths[i] += rgCurLineLayoutItems[i][j]->m_sSize.width;
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
    m_pLayoutItem = CreateContentLayoutItem(GetFormNode());

  if (eFlowStrategy != XFA_AttributeValue::Rl_tb) {
    float fCurPos;
    fCurPos = 0;
    for (int32_t c = nGroupLengths[0], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[0][j]->m_sPos = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[0][j]->GetFormNode(),
            rgCurLineLayoutItems[0][j]->m_sSize);
      } else {
        rgCurLineLayoutItems[0][j]->m_sPos =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (rgCurLineLayoutItems[0][j]->GetFormNode()->PresenceRequiresSpace())
          fCurPos += rgCurLineLayoutItems[0][j]->m_sSize.width;
      }
      m_pLayoutItem->AppendLastChild(rgCurLineLayoutItems[0][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = (fContentWidthLimit + fGroupWidths[0] - fGroupWidths[1] -
               fGroupWidths[2]) /
              2;
    for (int32_t c = nGroupLengths[1], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[1][j]->m_sPos = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[1][j]->GetFormNode(),
            rgCurLineLayoutItems[1][j]->m_sSize);
      } else {
        rgCurLineLayoutItems[1][j]->m_sPos =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (rgCurLineLayoutItems[1][j]->GetFormNode()->PresenceRequiresSpace())
          fCurPos += rgCurLineLayoutItems[1][j]->m_sSize.width;
      }
      m_pLayoutItem->AppendLastChild(rgCurLineLayoutItems[1][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = fContentWidthLimit - fGroupWidths[2];
    for (int32_t c = nGroupLengths[2], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[2][j]->m_sPos = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[2][j]->GetFormNode(),
            rgCurLineLayoutItems[2][j]->m_sSize);
      } else {
        rgCurLineLayoutItems[2][j]->m_sPos =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (rgCurLineLayoutItems[2][j]->GetFormNode()->PresenceRequiresSpace())
          fCurPos += rgCurLineLayoutItems[2][j]->m_sSize.width;
      }
      m_pLayoutItem->AppendLastChild(rgCurLineLayoutItems[2][j]);
      m_fLastRowWidth = fCurPos;
    }
  } else {
    float fCurPos;
    fCurPos = fGroupWidths[0];
    for (int32_t c = nGroupLengths[0], j = 0; j < c; j++) {
      if (rgCurLineLayoutItems[0][j]->GetFormNode()->PresenceRequiresSpace())
        fCurPos -= rgCurLineLayoutItems[0][j]->m_sSize.width;

      rgCurLineLayoutItems[0][j]->m_sPos =
          CFX_PointF(fCurPos, *fContentCurRowY);
      m_pLayoutItem->AppendLastChild(rgCurLineLayoutItems[0][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = (fContentWidthLimit + fGroupWidths[0] + fGroupWidths[1] -
               fGroupWidths[2]) /
              2;
    for (int32_t c = nGroupLengths[1], j = 0; j < c; j++) {
      if (rgCurLineLayoutItems[1][j]->GetFormNode()->PresenceRequiresSpace())
        fCurPos -= rgCurLineLayoutItems[1][j]->m_sSize.width;

      rgCurLineLayoutItems[1][j]->m_sPos =
          CFX_PointF(fCurPos, *fContentCurRowY);
      m_pLayoutItem->AppendLastChild(rgCurLineLayoutItems[1][j]);
      m_fLastRowWidth = fCurPos;
    }
    fCurPos = fContentWidthLimit;
    for (int32_t c = nGroupLengths[2], j = 0; j < c; j++) {
      if (rgCurLineLayoutItems[2][j]->GetFormNode()->PresenceRequiresSpace())
        fCurPos -= rgCurLineLayoutItems[2][j]->m_sSize.width;

      rgCurLineLayoutItems[2][j]->m_sPos =
          CFX_PointF(fCurPos, *fContentCurRowY);
      m_pLayoutItem->AppendLastChild(rgCurLineLayoutItems[2][j]);
      m_fLastRowWidth = fCurPos;
    }
  }
  m_fLastRowY = *fContentCurRowY;
  *fContentCurRowY += fContentCurRowHeight;
  if (bContainerWidthAutoSize) {
    float fChildSuppliedWidth = fGroupWidths[0];
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

CXFA_Node* CXFA_ContentLayoutProcessor::GetSubformSetParent(
    CXFA_Node* pSubformSet) {
  if (pSubformSet && pSubformSet->GetElementType() == XFA_Element::SubformSet) {
    CXFA_Node* pParent = pSubformSet->GetParent();
    while (pParent) {
      if (pParent->GetElementType() != XFA_Element::SubformSet)
        return pParent;
      pParent = pParent->GetParent();
    }
  }
  return pSubformSet;
}

void CXFA_ContentLayoutProcessor::DoLayoutField() {
  if (m_pLayoutItem)
    return;

  ASSERT(!m_pCurChildNode);
  m_pLayoutItem = CreateContentLayoutItem(GetFormNode());
  if (!m_pLayoutItem)
    return;

  CXFA_Document* pDocument = GetFormNode()->GetDocument();
  CXFA_FFNotify* pNotify = pDocument->GetNotify();
  CFX_SizeF size(-1, -1);
  pNotify->StartFieldDrawLayout(GetFormNode(), &size.width, &size.height);

  int32_t nRotate = XFA_MapRotation(
      GetFormNode()->JSObject()->GetInteger(XFA_Attribute::Rotate));
  if (nRotate == 90 || nRotate == 270)
    std::swap(size.width, size.height);

  SetCurrentComponentSize(size);
}

CXFA_ContentLayoutProcessor::Result CXFA_ContentLayoutProcessor::DoLayout(
    bool bUseBreakControl,
    float fHeightLimit,
    float fRealHeight) {
  return DoLayoutInternal(bUseBreakControl, fHeightLimit, fRealHeight, nullptr);
}

CXFA_ContentLayoutProcessor::Result
CXFA_ContentLayoutProcessor::DoLayoutInternal(bool bUseBreakControl,
                                              float fHeightLimit,
                                              float fRealHeight,
                                              Context* pContext) {
  switch (GetFormNode()->GetElementType()) {
    case XFA_Element::Subform:
    case XFA_Element::Area:
    case XFA_Element::ExclGroup:
    case XFA_Element::SubformSet: {
      bool bRootForceTb = false;
      CXFA_Node* pLayoutNode = GetSubformSetParent(GetFormNode());
      XFA_AttributeValue eLayoutStrategy =
          GetLayout(pLayoutNode, &bRootForceTb);
      switch (eLayoutStrategy) {
        case XFA_AttributeValue::Tb:
        case XFA_AttributeValue::Lr_tb:
        case XFA_AttributeValue::Rl_tb:
          return DoLayoutFlowedContainer(bUseBreakControl, eLayoutStrategy,
                                         fHeightLimit, fRealHeight, pContext,
                                         bRootForceTb);
        case XFA_AttributeValue::Position:
        case XFA_AttributeValue::Row:
        case XFA_AttributeValue::Rl_row:
        default:
          DoLayoutPositionedContainer(pContext);
          m_nCurChildNodeStage = Stage::kDone;
          return Result::kDone;
        case XFA_AttributeValue::Table:
          DoLayoutTableContainer(pLayoutNode);
          m_nCurChildNodeStage = Stage::kDone;
          return Result::kDone;
      }
    }
    case XFA_Element::Draw:
    case XFA_Element::Field:
      DoLayoutField();
      m_nCurChildNodeStage = Stage::kDone;
      return Result::kDone;
    case XFA_Element::ContentArea:
    default:
      return Result::kDone;
  }
}

CFX_SizeF CXFA_ContentLayoutProcessor::GetCurrentComponentSize() {
  return CFX_SizeF(m_pLayoutItem->m_sSize.width, m_pLayoutItem->m_sSize.height);
}

void CXFA_ContentLayoutProcessor::SetCurrentComponentPos(
    const CFX_PointF& pos) {
  m_pLayoutItem->m_sPos = pos;
}

void CXFA_ContentLayoutProcessor::SetCurrentComponentSize(
    const CFX_SizeF& size) {
  m_pLayoutItem->m_sSize = size;
}

bool CXFA_ContentLayoutProcessor::JudgeLeaderOrTrailerForOccur(
    CXFA_Node* pFormNode) {
  if (!pFormNode)
    return false;

  CXFA_Node* pTemplate = pFormNode->GetTemplateNodeIfExists();
  if (!pTemplate)
    pTemplate = pFormNode;

  auto* pOccur =
      pTemplate->GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
  if (!pOccur)
    return false;

  int32_t iMax = pOccur->GetMax();
  if (iMax < 0)
    return true;

  int32_t iCount = m_PendingNodesCount[pTemplate];
  if (iCount >= iMax)
    return false;

  m_PendingNodesCount[pTemplate] = iCount + 1;
  return true;
}

void CXFA_ContentLayoutProcessor::UpdatePendingItemLayout(
    CXFA_ContentLayoutItem* pLayoutItem) {
  XFA_AttributeValue eLayout =
      pLayoutItem->GetFormNode()->JSObject()->GetEnum(XFA_Attribute::Layout);
  switch (eLayout) {
    case XFA_AttributeValue::Row:
    case XFA_AttributeValue::Rl_row:
      RelocateTableRowCells(pLayoutItem, m_rgSpecifiedColumnWidths, eLayout);
      break;
    default:
      break;
  }
}

void CXFA_ContentLayoutProcessor::AddTrailerBeforeSplit(
    float fSplitPos,
    CXFA_ContentLayoutItem* pTrailerLayoutItem,
    bool bUseInherited) {
  if (!pTrailerLayoutItem)
    return;

  float fHeight = pTrailerLayoutItem->m_sSize.height;
  if (bUseInherited) {
    float fNewSplitPos = 0;
    if (fSplitPos - fHeight > kXFALayoutPrecision)
      fNewSplitPos = FindSplitPos(fSplitPos - fHeight);
    if (fNewSplitPos > kXFALayoutPrecision)
      SplitLayoutItem(fNewSplitPos);
    return;
  }

  UpdatePendingItemLayout(pTrailerLayoutItem);
  CXFA_Margin* pMargin =
      GetFormNode()->GetFirstChildByClass<CXFA_Margin>(XFA_Element::Margin);
  CFX_FloatRect inset = GetMarginInset(pMargin);
  if (!IsAddNewRowForTrailer(pTrailerLayoutItem)) {
    pTrailerLayoutItem->m_sPos.y = m_fLastRowY;
    pTrailerLayoutItem->m_sPos.x = m_fLastRowWidth;
    m_pLayoutItem->m_sSize.width += pTrailerLayoutItem->m_sSize.width;
    m_pLayoutItem->AppendLastChild(pTrailerLayoutItem);
    return;
  }

  float fNewSplitPos = 0;
  if (fSplitPos - fHeight > kXFALayoutPrecision)
    fNewSplitPos = FindSplitPos(fSplitPos - fHeight);

  if (fNewSplitPos > kXFALayoutPrecision) {
    SplitLayoutItem(fNewSplitPos);
    pTrailerLayoutItem->m_sPos.y = fNewSplitPos - inset.top - inset.bottom;
  } else {
    pTrailerLayoutItem->m_sPos.y = fSplitPos - inset.top - inset.bottom;
  }

  switch (pTrailerLayoutItem->GetFormNode()->JSObject()->GetEnum(
      XFA_Attribute::HAlign)) {
    case XFA_AttributeValue::Right:
      pTrailerLayoutItem->m_sPos.x = m_pLayoutItem->m_sSize.width -
                                     inset.right -
                                     pTrailerLayoutItem->m_sSize.width;
      break;
    case XFA_AttributeValue::Center:
      pTrailerLayoutItem->m_sPos.x =
          (m_pLayoutItem->m_sSize.width - inset.left - inset.right -
           pTrailerLayoutItem->m_sSize.width) /
          2;
      break;
    case XFA_AttributeValue::Left:
    default:
      pTrailerLayoutItem->m_sPos.x = inset.left;
      break;
  }
  m_pLayoutItem->m_sSize.height += fHeight;
  m_pLayoutItem->AppendLastChild(pTrailerLayoutItem);
}

void CXFA_ContentLayoutProcessor::AddLeaderAfterSplit(
    CXFA_ContentLayoutItem* pLeaderLayoutItem) {
  UpdatePendingItemLayout(pLeaderLayoutItem);

  CXFA_Margin* pMarginNode =
      GetFormNode()->GetFirstChildByClass<CXFA_Margin>(XFA_Element::Margin);
  float fLeftInset = 0;
  float fRightInset = 0;
  if (pMarginNode) {
    fLeftInset = pMarginNode->JSObject()->GetMeasureInUnit(
        XFA_Attribute::LeftInset, XFA_Unit::Pt);
    fRightInset = pMarginNode->JSObject()->GetMeasureInUnit(
        XFA_Attribute::RightInset, XFA_Unit::Pt);
  }

  float fHeight = pLeaderLayoutItem->m_sSize.height;
  for (CXFA_LayoutItem* pIter = m_pLayoutItem->GetFirstChild(); pIter;
       pIter = pIter->GetNextSibling()) {
    CXFA_ContentLayoutItem* pContentItem = pIter->AsContentLayoutItem();
    if (!pContentItem)
      continue;

    pContentItem->m_sPos.y += fHeight;
  }
  pLeaderLayoutItem->m_sPos.y = 0;

  switch (pLeaderLayoutItem->GetFormNode()->JSObject()->GetEnum(
      XFA_Attribute::HAlign)) {
    case XFA_AttributeValue::Right:
      pLeaderLayoutItem->m_sPos.x = m_pLayoutItem->m_sSize.width - fRightInset -
                                    pLeaderLayoutItem->m_sSize.width;
      break;
    case XFA_AttributeValue::Center:
      pLeaderLayoutItem->m_sPos.x =
          (m_pLayoutItem->m_sSize.width - fLeftInset - fRightInset -
           pLeaderLayoutItem->m_sSize.width) /
          2;
      break;
    case XFA_AttributeValue::Left:
    default:
      pLeaderLayoutItem->m_sPos.x = fLeftInset;
      break;
  }
  m_pLayoutItem->m_sSize.height += fHeight;
  m_pLayoutItem->AppendLastChild(pLeaderLayoutItem);
}

void CXFA_ContentLayoutProcessor::AddPendingNode(CXFA_Node* pPendingNode,
                                                 bool bBreakPending) {
  m_PendingNodes.push_back(pPendingNode);
  m_bBreakPending = bBreakPending;
}

float CXFA_ContentLayoutProcessor::InsertPendingItems(
    CXFA_Node* pCurChildNode) {
  float fTotalHeight = 0;
  if (m_PendingNodes.empty())
    return fTotalHeight;

  if (!m_pLayoutItem) {
    m_pLayoutItem = CreateContentLayoutItem(pCurChildNode);
    m_pLayoutItem->m_sSize.clear();
  }

  while (!m_PendingNodes.empty()) {
    auto pPendingProcessor = pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
        m_PendingNodes.front(), nullptr);
    m_PendingNodes.pop_front();
    pPendingProcessor->DoLayout(false, FLT_MAX, FLT_MAX);
    CXFA_ContentLayoutItem* pPendingLayoutItem =
        pPendingProcessor->HasLayoutItem()
            ? pPendingProcessor->ExtractLayoutItem()
            : nullptr;
    if (pPendingLayoutItem) {
      AddLeaderAfterSplit(pPendingLayoutItem);
      if (m_bBreakPending)
        fTotalHeight += pPendingLayoutItem->m_sSize.height;
    }
  }
  return fTotalHeight;
}

CXFA_ContentLayoutProcessor::Result
CXFA_ContentLayoutProcessor::InsertFlowedItem(
    CXFA_ContentLayoutProcessor* pProcessor,
    bool bContainerWidthAutoSize,
    bool bContainerHeightAutoSize,
    float fContainerHeight,
    XFA_AttributeValue eFlowStrategy,
    uint8_t* uCurHAlignState,
    std::vector<CXFA_ContentLayoutItem*> (&rgCurLineLayoutItems)[3],
    bool bUseBreakControl,
    float fAvailHeight,
    float fRealHeight,
    float fContentWidthLimit,
    float* fContentCurRowY,
    float* fContentCurRowAvailWidth,
    float* fContentCurRowHeight,
    bool* bAddedItemInRow,
    bool* bForceEndPage,
    Context* pLayoutContext,
    bool bNewRow) {
  bool bTakeSpace = pProcessor->GetFormNode()->PresenceRequiresSpace();
  uint8_t uHAlign = HAlignEnumToInt(
      m_pCurChildNode->JSObject()->GetEnum(XFA_Attribute::HAlign));
  if (bContainerWidthAutoSize)
    uHAlign = 0;

  if ((eFlowStrategy != XFA_AttributeValue::Rl_tb &&
       uHAlign < *uCurHAlignState) ||
      (eFlowStrategy == XFA_AttributeValue::Rl_tb &&
       uHAlign > *uCurHAlignState)) {
    return Result::kRowFullBreak;
  }

  *uCurHAlignState = uHAlign;
  bool bIsOwnSplit =
      pProcessor->GetFormNode()->GetIntact() == XFA_AttributeValue::None;
  bool bUseRealHeight = bTakeSpace && bContainerHeightAutoSize && bIsOwnSplit &&
                        pProcessor->GetFormNode()->GetParent()->GetIntact() ==
                            XFA_AttributeValue::None;
  bool bIsTransHeight = bTakeSpace;
  if (bIsTransHeight && !bIsOwnSplit) {
    bool bRootForceTb = false;
    XFA_AttributeValue eLayoutStrategy =
        GetLayout(pProcessor->GetFormNode(), &bRootForceTb);
    if (eLayoutStrategy == XFA_AttributeValue::Lr_tb ||
        eLayoutStrategy == XFA_AttributeValue::Rl_tb) {
      bIsTransHeight = false;
    }
  }

  bool bUseInherited = false;
  Context layoutContext;
  if (m_pViewLayoutProcessor) {
    CXFA_Node* pOverflowNode =
        m_pViewLayoutProcessor->QueryOverflow(GetFormNode());
    if (pOverflowNode) {
      layoutContext.m_pOverflowNode = pOverflowNode;
      layoutContext.m_pOverflowProcessor = this;
      pLayoutContext = &layoutContext;
    }
  }

  Result eRetValue = Result::kDone;
  if (!bNewRow || pProcessor->m_ePreProcessRs == Result::kDone) {
    eRetValue = pProcessor->DoLayoutInternal(
        bTakeSpace ? bUseBreakControl : false,
        bUseRealHeight ? fRealHeight - *fContentCurRowY : FLT_MAX,
        bIsTransHeight ? fRealHeight - *fContentCurRowY : FLT_MAX,
        pLayoutContext);
    pProcessor->m_ePreProcessRs = eRetValue;
  } else {
    eRetValue = pProcessor->m_ePreProcessRs;
    pProcessor->m_ePreProcessRs = Result::kDone;
  }
  if (pProcessor->HasLayoutItem() == false)
    return eRetValue;

  CFX_SizeF childSize = pProcessor->GetCurrentComponentSize();
  if (bUseRealHeight && fRealHeight < kXFALayoutPrecision) {
    fRealHeight = FLT_MAX;
    fAvailHeight = FLT_MAX;
  }
  if (bTakeSpace &&
      (childSize.width > *fContentCurRowAvailWidth + kXFALayoutPrecision) &&
      (fContentWidthLimit - *fContentCurRowAvailWidth > kXFALayoutPrecision)) {
    return Result::kRowFullBreak;
  }

  CXFA_Node* pOverflowLeaderNode = nullptr;
  CXFA_Node* pOverflowTrailerNode = nullptr;
  CXFA_Node* pFormNode = nullptr;
  CXFA_ContentLayoutItem* pTrailerLayoutItem = nullptr;
  bool bIsAddTrailerHeight = false;
  if (m_pViewLayoutProcessor &&
      pProcessor->GetFormNode()->GetIntact() == XFA_AttributeValue::None) {
    pFormNode =
        m_pViewLayoutProcessor->QueryOverflow(pProcessor->GetFormNode());
    if (!pFormNode && pLayoutContext && pLayoutContext->m_pOverflowProcessor) {
      pFormNode = pLayoutContext->m_pOverflowNode.Get();
      bUseInherited = true;
    }
    Optional<CXFA_ViewLayoutProcessor::OverflowData> overflow_data =
        m_pViewLayoutProcessor->ProcessOverflow(pFormNode, false);
    if (overflow_data.has_value()) {
      pOverflowLeaderNode = overflow_data.value().pLeader;
      pOverflowTrailerNode = overflow_data.value().pTrailer;
      if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowTrailerNode)) {
        if (pOverflowTrailerNode) {
          auto pOverflowLeaderProcessor =
              pdfium::MakeUnique<CXFA_ContentLayoutProcessor>(
                  pOverflowTrailerNode, nullptr);
          pOverflowLeaderProcessor->DoLayout(false, FLT_MAX, FLT_MAX);
          pTrailerLayoutItem =
              pOverflowLeaderProcessor->HasLayoutItem()
                  ? pOverflowLeaderProcessor->ExtractLayoutItem()
                  : nullptr;
        }

        bIsAddTrailerHeight =
            bUseInherited
                ? IsAddNewRowForTrailer(pTrailerLayoutItem)
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
          fAvailHeight + kXFALayoutPrecision ||
      (!bContainerHeightAutoSize &&
       m_fUsedSize + fAvailHeight + kXFALayoutPrecision >= fContainerHeight)) {
    if (!bTakeSpace || eRetValue == Result::kDone) {
      if (pProcessor->m_bUseInherited) {
        if (pTrailerLayoutItem)
          pProcessor->AddTrailerBeforeSplit(childSize.height,
                                            pTrailerLayoutItem, false);
        if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
          pProcessor->AddPendingNode(pOverflowLeaderNode, false);

        pProcessor->m_bUseInherited = false;
      } else {
        if (bIsAddTrailerHeight)
          childSize.height -= pTrailerLayoutItem->m_sSize.height;

        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
      }

      CXFA_ContentLayoutItem* pChildLayoutItem =
          pProcessor->ExtractLayoutItem();
      if (ExistContainerKeep(pProcessor->GetFormNode(), false) &&
          pProcessor->GetFormNode()->GetIntact() == XFA_AttributeValue::None) {
        m_arrayKeepItems.push_back(pChildLayoutItem);
      } else {
        m_arrayKeepItems.clear();
      }
      rgCurLineLayoutItems[uHAlign].push_back(pChildLayoutItem);
      *bAddedItemInRow = true;
      if (bTakeSpace) {
        *fContentCurRowAvailWidth -= childSize.width;
        *fContentCurRowHeight =
            std::max(*fContentCurRowHeight, childSize.height);
      }
      return Result::kDone;
    }

    if (eRetValue == Result::kPageFullBreak) {
      if (pProcessor->m_bUseInherited) {
        if (pTrailerLayoutItem) {
          pProcessor->AddTrailerBeforeSplit(childSize.height,
                                            pTrailerLayoutItem, false);
        }
        if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
          pProcessor->AddPendingNode(pOverflowLeaderNode, false);

        pProcessor->m_bUseInherited = false;
      } else {
        if (bIsAddTrailerHeight)
          childSize.height -= pTrailerLayoutItem->m_sSize.height;

        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
      }
    }
    rgCurLineLayoutItems[uHAlign].push_back(pProcessor->ExtractLayoutItem());
    *bAddedItemInRow = true;
    *fContentCurRowAvailWidth -= childSize.width;
    *fContentCurRowHeight = std::max(*fContentCurRowHeight, childSize.height);
    return eRetValue;
  }

  Result eResult;
  if (ProcessKeepForSplit(pProcessor, eRetValue, &rgCurLineLayoutItems[uHAlign],
                          fContentCurRowAvailWidth, fContentCurRowHeight,
                          fContentCurRowY, bAddedItemInRow, bForceEndPage,
                          &eResult)) {
    return eResult;
  }

  *bForceEndPage = true;
  float fSplitPos = pProcessor->FindSplitPos(fAvailHeight - *fContentCurRowY);
  if (fSplitPos > kXFALayoutPrecision) {
    XFA_AttributeValue eLayout =
        pProcessor->GetFormNode()->JSObject()->GetEnum(XFA_Attribute::Layout);
    if (eLayout == XFA_AttributeValue::Tb && eRetValue == Result::kDone) {
      pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                       pOverflowTrailerNode, pTrailerLayoutItem,
                                       pFormNode);
      rgCurLineLayoutItems[uHAlign].push_back(pProcessor->ExtractLayoutItem());
      *bAddedItemInRow = true;
      if (bTakeSpace) {
        *fContentCurRowAvailWidth -= childSize.width;
        *fContentCurRowHeight =
            std::max(*fContentCurRowHeight, childSize.height);
      }
      return Result::kPageFullBreak;
    }

    if (m_pViewLayoutProcessor && !pProcessor->m_bUseInherited &&
        eRetValue != Result::kPageFullBreak) {
      m_pViewLayoutProcessor->ProcessOverflow(pFormNode, true);
    }
    if (pTrailerLayoutItem && bIsAddTrailerHeight) {
      pProcessor->AddTrailerBeforeSplit(fSplitPos, pTrailerLayoutItem,
                                        bUseInherited);
    } else {
      pProcessor->SplitLayoutItem(fSplitPos);
    }

    if (bUseInherited) {
      pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                       pOverflowTrailerNode, pTrailerLayoutItem,
                                       pFormNode);
      m_bUseInherited = true;
    } else {
      CXFA_LayoutItem* firstChild = pProcessor->m_pLayoutItem->GetFirstChild();
      if (firstChild && !firstChild->GetNextSibling() &&
          firstChild->GetFormNode()->IsLayoutGeneratedNode()) {
        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
      } else if (pProcessor->JudgeLeaderOrTrailerForOccur(
                     pOverflowLeaderNode)) {
        pProcessor->AddPendingNode(pOverflowLeaderNode, false);
      }
    }

    if (pProcessor->m_pLayoutItem->GetNextSibling()) {
      childSize = pProcessor->GetCurrentComponentSize();
      rgCurLineLayoutItems[uHAlign].push_back(pProcessor->ExtractLayoutItem());
      *bAddedItemInRow = true;
      if (bTakeSpace) {
        *fContentCurRowAvailWidth -= childSize.width;
        *fContentCurRowHeight =
            std::max(*fContentCurRowHeight, childSize.height);
      }
    }
    return Result::kPageFullBreak;
  }

  if (*fContentCurRowY <= kXFALayoutPrecision) {
    childSize = pProcessor->GetCurrentComponentSize();
    if (pProcessor->m_pViewLayoutProcessor->GetNextAvailContentHeight(
            childSize.height)) {
      if (m_pViewLayoutProcessor) {
        if (!pFormNode && pLayoutContext)
          pFormNode = pLayoutContext->m_pOverflowProcessor->GetFormNode();

        m_pViewLayoutProcessor->ProcessOverflow(pFormNode, true);
      }
      if (bUseInherited) {
        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
        m_bUseInherited = true;
      }
      return Result::kPageFullBreak;
    }

    rgCurLineLayoutItems[uHAlign].push_back(pProcessor->ExtractLayoutItem());
    *bAddedItemInRow = true;
    if (bTakeSpace) {
      *fContentCurRowAvailWidth -= childSize.width;
      *fContentCurRowHeight = std::max(*fContentCurRowHeight, childSize.height);
    }
    if (eRetValue == Result::kDone)
      *bForceEndPage = false;

    return eRetValue;
  }

  XFA_AttributeValue eLayout =
      pProcessor->GetFormNode()->JSObject()->GetEnum(XFA_Attribute::Layout);
  if (pProcessor->GetFormNode()->GetIntact() == XFA_AttributeValue::None &&
      eLayout == XFA_AttributeValue::Tb) {
    if (m_pViewLayoutProcessor) {
      Optional<CXFA_ViewLayoutProcessor::OverflowData> overflow_data =
          m_pViewLayoutProcessor->ProcessOverflow(pFormNode, true);
      if (overflow_data.has_value()) {
        pOverflowLeaderNode = overflow_data.value().pLeader;
        pOverflowTrailerNode = overflow_data.value().pTrailer;
      }
    }
    if (pTrailerLayoutItem)
      pProcessor->AddTrailerBeforeSplit(fSplitPos, pTrailerLayoutItem, false);
    if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
      pProcessor->AddPendingNode(pOverflowLeaderNode, false);

    return Result::kPageFullBreak;
  }

  if (eRetValue != Result::kDone)
    return Result::kPageFullBreak;

  if (!pFormNode && pLayoutContext)
    pFormNode = pLayoutContext->m_pOverflowProcessor->GetFormNode();
  if (m_pViewLayoutProcessor) {
    Optional<CXFA_ViewLayoutProcessor::OverflowData> overflow_data =
        m_pViewLayoutProcessor->ProcessOverflow(pFormNode, true);
    if (overflow_data.has_value()) {
      pOverflowLeaderNode = overflow_data.value().pLeader;
      pOverflowTrailerNode = overflow_data.value().pTrailer;
    }
  }
  if (bUseInherited) {
    pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode, pOverflowTrailerNode,
                                     pTrailerLayoutItem, pFormNode);
    m_bUseInherited = true;
  }
  return Result::kPageFullBreak;
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleKeep(CXFA_Node* pBreakAfterNode,
                                        CXFA_Node** pCurActionNode) {
  if (m_bKeepBreakFinish)
    return {};
  return FindBreakNode(pBreakAfterNode, false, pCurActionNode);
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleBookendLeader(CXFA_Node* pParentContainer,
                                                 CXFA_Node** pCurActionNode) {
  for (CXFA_Node* pBookendNode = *pCurActionNode
                                     ? (*pCurActionNode)->GetNextSibling()
                                     : pParentContainer->GetFirstChild();
       pBookendNode; pBookendNode = pBookendNode->GetNextSibling()) {
    switch (pBookendNode->GetElementType()) {
      case XFA_Element::Bookend:
      case XFA_Element::Break:
        *pCurActionNode = pBookendNode;
        return Stage::kBookendLeader;
      default:
        break;
    }
  }
  return {};
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleBreakBefore(CXFA_Node* pChildContainer,
                                               CXFA_Node** pCurActionNode) {
  if (!*pCurActionNode)
    return {};

  CXFA_Node* pBreakBeforeNode = (*pCurActionNode)->GetNextSibling();
  if (!m_bKeepBreakFinish) {
    Optional<Stage> ret = FindBreakNode(pBreakBeforeNode, true, pCurActionNode);
    if (ret.has_value())
      return ret.value();
  }
  if (m_bIsProcessKeep)
    return ProcessKeepNodesForBreakBefore(pCurActionNode, pChildContainer);

  *pCurActionNode = pChildContainer;
  return Stage::kContainer;
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleBreakAfter(CXFA_Node* pChildContainer,
                                              CXFA_Node** pCurActionNode) {
  if (*pCurActionNode) {
    CXFA_Node* pBreakAfterNode = (*pCurActionNode)->GetNextSibling();
    return FindBreakNode(pBreakAfterNode, false, pCurActionNode);
  }

  CXFA_Node* pBreakAfterNode = pChildContainer->GetFirstChild();
  return HandleKeep(pBreakAfterNode, pCurActionNode);
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleCheckNextChildContainer(
    CXFA_Node* pParentContainer,
    CXFA_Node* pChildContainer,
    CXFA_Node** pCurActionNode) {
  CXFA_Node* pNextChildContainer =
      pChildContainer ? pChildContainer->GetNextContainerSibling()
                      : pParentContainer->GetFirstContainerChild();
  while (pNextChildContainer && pNextChildContainer->IsLayoutGeneratedNode()) {
    CXFA_Node* pSaveNode = pNextChildContainer;
    pNextChildContainer = pNextChildContainer->GetNextContainerSibling();
    if (pSaveNode->IsUnusedNode())
      DeleteLayoutGeneratedNode(pSaveNode);
  }
  if (!pNextChildContainer)
    return {};

  bool bLastKeep = false;
  Optional<Stage> ret = ProcessKeepNodesForCheckNext(
      pCurActionNode, &pNextChildContainer, &bLastKeep);
  if (ret.has_value())
    return ret.value();

  if (!m_bKeepBreakFinish && !bLastKeep) {
    ret = FindBreakNode(pNextChildContainer->GetFirstChild(), true,
                        pCurActionNode);
    if (ret.has_value())
      return ret.value();
  }
  *pCurActionNode = pNextChildContainer;
  return m_bIsProcessKeep ? Stage::kKeep : Stage::kContainer;
}

Optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleBookendTrailer(CXFA_Node* pParentContainer,
                                                  CXFA_Node** pCurActionNode) {
  for (CXFA_Node* pBookendNode = *pCurActionNode
                                     ? (*pCurActionNode)->GetNextSibling()
                                     : pParentContainer->GetFirstChild();
       pBookendNode; pBookendNode = pBookendNode->GetNextSibling()) {
    switch (pBookendNode->GetElementType()) {
      case XFA_Element::Bookend:
      case XFA_Element::Break:
        *pCurActionNode = pBookendNode;
        return Stage::kBookendTrailer;
      default:
        break;
    }
  }
  return {};
}

void CXFA_ContentLayoutProcessor::ProcessKeepNodesEnd() {
  m_bKeepBreakFinish = true;
  m_pKeepHeadNode = nullptr;
  m_pKeepTailNode = nullptr;
  m_bIsProcessKeep = false;
}

void CXFA_ContentLayoutProcessor::AdjustContainerSpecifiedSize(
    Context* pContext,
    CFX_SizeF* pSize,
    bool* pContainerWidthAutoSize,
    bool* pContainerHeightAutoSize) {
  if (pContext && pContext->m_fCurColumnWidth.has_value()) {
    pSize->width = pContext->m_fCurColumnWidth.value();
    *pContainerWidthAutoSize = false;
  }
  if (*pContainerHeightAutoSize)
    return;

  pSize->height -= m_fUsedSize;
  CXFA_Node* pParentNode = GetFormNode()->GetParent();
  bool bFocrTb = false;
  if (!pParentNode ||
      GetLayout(pParentNode, &bFocrTb) != XFA_AttributeValue::Row) {
    return;
  }

  CXFA_Node* pChildContainer = GetFormNode()->GetFirstContainerChild();
  if (!pChildContainer || !pChildContainer->GetNextContainerSibling())
    return;

  pSize->height = 0;
  *pContainerHeightAutoSize = true;
}

CXFA_ContentLayoutItem* CXFA_ContentLayoutProcessor::FindLastContentLayoutItem(
    XFA_AttributeValue eFlowStrategy) {
  if (m_nCurChildNodeStage == Stage::kDone ||
      eFlowStrategy == XFA_AttributeValue::Tb) {
    return nullptr;
  }

  CXFA_ContentLayoutItem* pLastChild =
      ToContentLayoutItem(m_pLayoutItem->GetFirstChild());
  for (CXFA_LayoutItem* pNext = pLastChild; pNext;
       pNext = pNext->GetNextSibling()) {
    CXFA_ContentLayoutItem* pContentNext = pNext->AsContentLayoutItem();
    if (pContentNext && pContentNext->m_sPos.y != pLastChild->m_sPos.y)
      pLastChild = pContentNext;
  }
  return pLastChild;
}

CFX_SizeF CXFA_ContentLayoutProcessor::CalculateLayoutItemSize(
    const CXFA_ContentLayoutItem* pLastChild) {
  CFX_SizeF size;
  for (CXFA_LayoutItem* pChild = m_pLayoutItem->GetFirstChild();
       pChild != pLastChild; pChild = pChild->GetNextSibling()) {
    CXFA_ContentLayoutItem* pLayout = pChild->AsContentLayoutItem();
    if (!pLayout || !pLayout->GetFormNode()->PresenceRequiresSpace())
      continue;

    float fWidth = pLayout->m_sPos.x + pLayout->m_sSize.width;
    float fHeight = pLayout->m_sPos.y + pLayout->m_sSize.height;
    size.width = std::max(size.width, fWidth);
    size.height = std::max(size.height, fHeight);
  }
  return size;
}

CXFA_ContentLayoutProcessor::Context::Context() = default;

CXFA_ContentLayoutProcessor::Context::~Context() = default;
