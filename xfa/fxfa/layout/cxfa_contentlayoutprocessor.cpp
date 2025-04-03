// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_contentlayoutprocessor.h"

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/adapters.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/stl_util.h"
#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cjx_object.h"
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

using NextPosRow = std::array<uint8_t, 9>;
constexpr std::array<const NextPosRow, 4> kNextPosTable = {{
    {{0, 1, 2, 3, 4, 5, 6, 7, 8}},
    {{6, 3, 0, 7, 4, 1, 8, 5, 2}},
    {{8, 7, 6, 5, 4, 3, 2, 1, 0}},
    {{2, 5, 8, 1, 4, 7, 0, 3, 6}},
}};

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
        *pWidth = pLayoutItem->s_size_.width;
      if (*pHeight < -kXFALayoutPrecision)
        *pHeight = pLayoutItem->s_size_.height;
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
    std::optional<CXFA_Measurement> wValue =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::W, false);
    if (wValue.has_value() && wValue->GetValue() > kXFALayoutPrecision) {
      containerSize.width = wValue->ToUnit(XFA_Unit::Pt);
      *bContainerWidthAutoSize = false;
    }

    std::optional<CXFA_Measurement> hValue =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::H, false);
    if (hValue.has_value() && hValue->GetValue() > kXFALayoutPrecision) {
      containerSize.height = hValue->ToUnit(XFA_Unit::Pt);
      *bContainerHeightAutoSize = false;
    }
  }

  if (*bContainerWidthAutoSize && eType == XFA_Element::Subform) {
    std::optional<CXFA_Measurement> maxW =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::MaxW, false);
    if (maxW.has_value() && maxW->GetValue() > kXFALayoutPrecision) {
      containerSize.width = maxW->ToUnit(XFA_Unit::Pt);
      *bContainerWidthAutoSize = false;
    }

    std::optional<CXFA_Measurement> maxH =
        pFormNode->JSObject()->TryMeasure(XFA_Attribute::MaxH, false);
    if (maxH.has_value() && maxH->GetValue() > kXFALayoutPrecision) {
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
      std::optional<CXFA_Measurement> leftInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::LeftInset, false);
      if (leftInset.has_value())
        componentSize.width += leftInset->ToUnit(XFA_Unit::Pt);

      std::optional<CXFA_Measurement> rightInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::RightInset, false);
      if (rightInset.has_value())
        componentSize.width += rightInset->ToUnit(XFA_Unit::Pt);
    }
  }

  if (bContainerHeightAutoSize) {
    componentSize.height = fContentCalculatedHeight;
    if (pMarginNode) {
      std::optional<CXFA_Measurement> topInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::TopInset, false);
      if (topInset.has_value())
        componentSize.height += topInset->ToUnit(XFA_Unit::Pt);

      std::optional<CXFA_Measurement> bottomInset =
          pMarginNode->JSObject()->TryMeasure(XFA_Attribute::BottomInset,
                                              false);
      if (bottomInset.has_value())
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
  const CFX_SizeF containerSize = CalculateContainerSpecifiedSize(
      pLayoutRow->GetFormNode(), &bContainerWidthAutoSize,
      &bContainerHeightAutoSize);

  CXFA_Margin* pMargin =
      pLayoutRow->GetFormNode()->GetFirstChildByClass<CXFA_Margin>(
          XFA_Element::Margin);
  const CFX_FloatRect inset = GetMarginInset(pMargin);

  const float fContentWidthLimit =
      bContainerWidthAutoSize ? FLT_MAX
                              : containerSize.width - inset.left - inset.right;
  const float fContentCurrentHeight =
      pLayoutRow->s_size_.height - inset.top - inset.bottom;

  float fContentCalculatedWidth = 0;
  float fContentCalculatedHeight = 0;
  float fCurrentColX = 0;
  size_t nCurrentColIdx = 0;
  bool bMetWholeRowCell = false;

  for (CXFA_LayoutItem* pIter = pLayoutRow->GetFirstChild(); pIter;
       pIter = pIter->GetNextSibling()) {
    CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
    if (!pLayoutChild)
      continue;

    const int32_t nOriginalColSpan =
        pLayoutChild->GetFormNode()->JSObject()->GetInteger(
            XFA_Attribute::ColSpan);

    size_t nColSpan;
    if (nOriginalColSpan > 0)
      nColSpan = static_cast<size_t>(nOriginalColSpan);
    else if (nOriginalColSpan == -1)
      nColSpan = rgSpecifiedColumnWidths.size();
    else
      continue;

    CHECK(nCurrentColIdx <= rgSpecifiedColumnWidths.size());
    const size_t remaining = rgSpecifiedColumnWidths.size() - nCurrentColIdx;
    nColSpan = std::min(nColSpan, remaining);

    float fColSpanWidth = 0;
    for (size_t i = 0; i < nColSpan; i++)
      fColSpanWidth += rgSpecifiedColumnWidths[nCurrentColIdx + i];

    if (nOriginalColSpan == -1 ||
        nColSpan != static_cast<size_t>(nOriginalColSpan)) {
      fColSpanWidth = bMetWholeRowCell ? 0
                                       : std::max(fColSpanWidth,
                                                  pLayoutChild->s_size_.height);
    }
    if (nOriginalColSpan == -1)
      bMetWholeRowCell = true;

    pLayoutChild->s_pos_ = CFX_PointF(fCurrentColX, 0);
    pLayoutChild->s_size_.width = fColSpanWidth;
    if (!pLayoutChild->GetFormNode()->PresenceRequiresSpace())
      continue;

    fCurrentColX += fColSpanWidth;
    nCurrentColIdx += nColSpan;
    float fNewHeight = bContainerHeightAutoSize ? -1 : fContentCurrentHeight;
    UpdateWidgetSize(pLayoutChild, &fColSpanWidth, &fNewHeight);
    pLayoutChild->s_size_.height = fNewHeight;
    if (bContainerHeightAutoSize) {
      fContentCalculatedHeight =
          std::max(fContentCalculatedHeight, pLayoutChild->s_size_.height);
    }
  }

  if (bContainerHeightAutoSize) {
    for (CXFA_LayoutItem* pIter = pLayoutRow->GetFirstChild(); pIter;
         pIter = pIter->GetNextSibling()) {
      CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
      if (!pLayoutChild)
        continue;

      UpdateWidgetSize(pLayoutChild, &pLayoutChild->s_size_.width,
                       &fContentCalculatedHeight);
      float fOldChildHeight = pLayoutChild->s_size_.height;
      pLayoutChild->s_size_.height = fContentCalculatedHeight;
      CXFA_Para* pParaNode =
          pLayoutChild->GetFormNode()->GetFirstChildByClass<CXFA_Para>(
              XFA_Element::Para);
      if (!pParaNode || !pLayoutChild->GetFirstChild())
        continue;

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
      if (fOffHeight <= 0)
        continue;

      for (CXFA_LayoutItem* pInnerIter = pLayoutChild->GetFirstChild();
           pInnerIter; pInnerIter = pInnerIter->GetNextSibling()) {
        CXFA_ContentLayoutItem* pInnerChild = pInnerIter->AsContentLayoutItem();
        if (pInnerChild)
          pInnerChild->s_pos_.y += fOffHeight;
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

      pLayoutChild->s_pos_.x = fContentCalculatedWidth -
                               pLayoutChild->s_pos_.x -
                               pLayoutChild->s_size_.width;
    }
  }
  pLayoutRow->s_size_ = CalculateContainerComponentSizeFromContentSize(
      pLayoutRow->GetFormNode(), bContainerWidthAutoSize,
      fContentCalculatedWidth, bContainerHeightAutoSize,
      fContentCalculatedHeight, containerSize);
}

XFA_AttributeValue GetLayout(CXFA_Node* pFormNode, bool* bRootForceTb) {
  *bRootForceTb = false;
  std::optional<XFA_AttributeValue> layoutMode =
      pFormNode->JSObject()->TryEnum(XFA_Attribute::Layout, false);
  if (layoutMode.has_value())
    return layoutMode.value();

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

    std::optional<XFA_AttributeValue> previous =
        pKeep->JSObject()->TryEnum(eKeepType, false);
    if (previous == XFA_AttributeValue::ContentArea ||
        previous == XFA_AttributeValue::PageArea) {
      return true;
    }
  }

  pKeep = pPreContainer->GetFirstChildByClass<CXFA_Keep>(XFA_Element::Keep);
  if (!pKeep)
    return false;

  XFA_Attribute eKeepType = XFA_Attribute::Next;
  if (!bPreFind)
    eKeepType = XFA_Attribute::Previous;

  std::optional<XFA_AttributeValue> next =
      pKeep->JSObject()->TryEnum(eKeepType, false);
  if (next == XFA_AttributeValue::ContentArea ||
      next == XFA_AttributeValue::PageArea) {
    return true;
  }
  return false;
}

std::optional<CXFA_ContentLayoutProcessor::Stage> FindBreakBeforeNode(
    CXFA_Node* pContainerNode,
    CXFA_Node** pCurActionNode) {
  for (CXFA_Node* pBreakNode = pContainerNode; pBreakNode;
       pBreakNode = pBreakNode->GetNextSibling()) {
    switch (pBreakNode->GetElementType()) {
      case XFA_Element::BreakBefore:
        *pCurActionNode = pBreakNode;
        return CXFA_ContentLayoutProcessor::Stage::kBreakBefore;
      case XFA_Element::Break:
        if (pBreakNode->JSObject()->GetEnum(XFA_Attribute::Before) ==
            XFA_AttributeValue::Auto) {
          break;
        }
        *pCurActionNode = pBreakNode;
        return CXFA_ContentLayoutProcessor::Stage::kBreakBefore;
      default:
        break;
    }
  }
  return std::nullopt;
}

std::optional<CXFA_ContentLayoutProcessor::Stage> FindBreakAfterNode(
    CXFA_Node* pContainerNode,
    CXFA_Node** pCurActionNode) {
  for (CXFA_Node* pBreakNode = pContainerNode; pBreakNode;
       pBreakNode = pBreakNode->GetNextSibling()) {
    switch (pBreakNode->GetElementType()) {
      case XFA_Element::BreakAfter:
        *pCurActionNode = pBreakNode;
        return CXFA_ContentLayoutProcessor::Stage::kBreakAfter;
      case XFA_Element::Break:
        if (pBreakNode->JSObject()->GetEnum(XFA_Attribute::After) ==
            XFA_AttributeValue::Auto) {
          break;
        }
        *pCurActionNode = pBreakNode;
        return CXFA_ContentLayoutProcessor::Stage::kBreakAfter;
      default:
        break;
    }
  }
  return std::nullopt;
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
      *fProposedSplitPos > fCurVerticalOffset + pLayoutItem->s_size_.height -
                               kXFALayoutPrecision) {
    return false;
  }

  switch (pFormNode->GetIntact()) {
    case XFA_AttributeValue::None: {
      bool bAnyChanged = false;
      CXFA_Document* pDocument = pFormNode->GetDocument();
      CXFA_FFNotify* pNotify = pDocument->GetNotify();
      float fCurTopMargin = 0;
      float fCurBottomMargin = 0;
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
          std::optional<float> fRelSplitPos = pFormNode->FindSplitPos(
              pNotify->GetFFDoc()->GetDocView(), pLayoutItem->GetIndex(),
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
              fCurVerticalOffset + fCurTopMargin + pChildItem->s_pos_.y;
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
  CFX_PointF pos(
      pNode->JSObject()->GetMeasureInUnit(XFA_Attribute::X, XFA_Unit::Pt),
      pNode->JSObject()->GetMeasureInUnit(XFA_Attribute::Y, XFA_Unit::Pt));
  int32_t nRotate =
      XFA_MapRotation(pNode->JSObject()->GetInteger(XFA_Attribute::Rotate)) /
      90;
  int32_t nAbsoluteAnchorType = kNextPosTable[nRotate][nAnchorType];
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
    cppgc::Heap* pHeap,
    CXFA_Node* pNode,
    CXFA_ViewLayoutProcessor* pViewLayoutProcessor)
    : heap_(pHeap),
      form_node_(pNode),
      view_layout_processor_(pViewLayoutProcessor) {
  DCHECK(GetFormNode());
  DCHECK(GetFormNode()->IsContainerNode() ||
         GetFormNode()->GetElementType() == XFA_Element::Form);
  old_layout_item_ =
      ToContentLayoutItem(GetFormNode()->JSObject()->GetLayoutItem());
}

CXFA_ContentLayoutProcessor::~CXFA_ContentLayoutProcessor() = default;

void CXFA_ContentLayoutProcessor::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(form_node_);
  visitor->Trace(cur_child_node_);
  visitor->Trace(keep_head_node_);
  visitor->Trace(keep_tail_node_);
  visitor->Trace(layout_item_);
  visitor->Trace(old_layout_item_);
  visitor->Trace(view_layout_processor_);
  visitor->Trace(cur_child_preprocessor_);
  ContainerTrace(visitor, array_keep_items_);
  ContainerTrace(visitor, pending_nodes_);
  ContainerTrace(visitor, pending_nodes_count_);
}

CXFA_ContentLayoutItem* CXFA_ContentLayoutProcessor::CreateContentLayoutItem(
    CXFA_Node* pFormNode) {
  if (!pFormNode)
    return nullptr;

  if (old_layout_item_) {
    CXFA_ContentLayoutItem* pLayoutItem = old_layout_item_;
    old_layout_item_ = old_layout_item_->GetNext();
    return pLayoutItem;
  }
  CXFA_FFNotify* pNotify = pFormNode->GetDocument()->GetNotify();
  auto* pNewLayoutItem = cppgc::MakeGarbageCollected<CXFA_ContentLayoutItem>(
      GetHeap()->GetAllocationHandle(), pFormNode,
      pNotify->OnCreateContentLayoutItem(pFormNode));

  CXFA_ContentLayoutItem* pPrevLayoutItem =
      ToContentLayoutItem(pFormNode->JSObject()->GetLayoutItem());
  if (pPrevLayoutItem) {
    pPrevLayoutItem->GetLast()->InsertAfter(pNewLayoutItem);
  } else {
    pFormNode->JSObject()->SetLayoutItem(pNewLayoutItem);
  }
  return pNewLayoutItem;
}

float CXFA_ContentLayoutProcessor::FindSplitPos(float fProposedSplitPos) {
  DCHECK(layout_item_);
  auto value = GetFormNode()->JSObject()->TryEnum(XFA_Attribute::Layout, true);
  XFA_AttributeValue eLayout = value.value_or(XFA_AttributeValue::Position);
  bool bCalculateMargin = eLayout != XFA_AttributeValue::Position;
  while (fProposedSplitPos > kXFALayoutPrecision) {
    bool bAppChange = false;
    if (!FindLayoutItemSplitPos(layout_item_.Get(), 0, &fProposedSplitPos,
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
  if (cur_child_preprocessor_ &&
      cur_child_preprocessor_->GetFormNode() == pLayoutItem->GetFormNode()) {
    pSecondLayoutItem = cur_child_preprocessor_->CreateContentLayoutItem(
        pLayoutItem->GetFormNode());
  } else {
    pSecondLayoutItem = CreateContentLayoutItem(pLayoutItem->GetFormNode());
  }
  pSecondLayoutItem->s_pos_.x = pLayoutItem->s_pos_.x;
  pSecondLayoutItem->s_size_.width = pLayoutItem->s_size_.width;
  pSecondLayoutItem->s_pos_.y = 0;
  pSecondLayoutItem->s_size_.height = pLayoutItem->s_size_.height - fSplitPos;
  pLayoutItem->s_size_.height -= pSecondLayoutItem->s_size_.height;
  if (pLayoutItem->GetFirstChild())
    pSecondLayoutItem->s_size_.height += fCurTopMargin;

  bool bOrphanedItem = false;
  if (pSecondParent) {
    pSecondParent->AppendLastChild(pSecondLayoutItem);
    if (fCurTopMargin > 0 && pLayoutItem->GetFirstChild()) {
      pSecondParent->s_size_.height += fCurTopMargin;
      for (CXFA_LayoutItem* pParentIter = pSecondParent->GetParent();
           pParentIter; pParentIter = pParentIter->GetParent()) {
        CXFA_ContentLayoutItem* pContentItem =
            pParentIter->AsContentLayoutItem();
        if (!pContentItem)
          continue;

        pContentItem->s_size_.height += fCurTopMargin;
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
    pLayoutItem->RemoveChild(children.back());
  }

  float lHeightForKeep = 0;
  float fAddMarginHeight = 0;
  std::vector<CXFA_ContentLayoutItem*> keepLayoutItems;
  for (CXFA_ContentLayoutItem* pChildItem : children) {
    if (fSplitPos <= fCurTopMargin + pChildItem->s_pos_.y + fCurBottomMargin +
                         kXFALayoutPrecision) {
      if (!ExistContainerKeep(pChildItem->GetFormNode(), true)) {
        pChildItem->s_pos_.y -= fSplitPos - fCurBottomMargin;
        pChildItem->s_pos_.y += lHeightForKeep;
        pChildItem->s_pos_.y += fAddMarginHeight;
        pSecondLayoutItem->AppendLastChild(pChildItem);
        continue;
      }
      if (lHeightForKeep < kXFALayoutPrecision) {
        for (CXFA_ContentLayoutItem* pPreItem : keepLayoutItems) {
          pLayoutItem->RemoveChild(pPreItem);
          pPreItem->s_pos_.y -= fSplitPos;
          if (pPreItem->s_pos_.y < 0) {
            pPreItem->s_pos_.y = 0;
          }
          if (pPreItem->s_pos_.y + pPreItem->s_size_.height > lHeightForKeep) {
            pPreItem->s_pos_.y = lHeightForKeep;
            lHeightForKeep += pPreItem->s_size_.height;
            pSecondLayoutItem->s_size_.height += pPreItem->s_size_.height;
            if (pSecondParent)
              pSecondParent->s_size_.height += pPreItem->s_size_.height;
          }
          pSecondLayoutItem->AppendLastChild(pPreItem);
        }
      }
      pChildItem->s_pos_.y -= fSplitPos;
      pChildItem->s_pos_.y += lHeightForKeep;
      pChildItem->s_pos_.y += fAddMarginHeight;
      pSecondLayoutItem->AppendLastChild(pChildItem);
      continue;
    }
    if (fSplitPos + kXFALayoutPrecision >= fCurTopMargin + fCurBottomMargin +
                                               pChildItem->s_pos_.y +
                                               pChildItem->s_size_.height) {
      pLayoutItem->AppendLastChild(pChildItem);
      if (ExistContainerKeep(pChildItem->GetFormNode(), false))
        keepLayoutItems.push_back(pChildItem);
      else
        keepLayoutItems.clear();
      continue;
    }

    float fOldHeight = pSecondLayoutItem->s_size_.height;
    SplitLayoutItem(
        pChildItem, pSecondLayoutItem,
        fSplitPos - fCurTopMargin - fCurBottomMargin - pChildItem->s_pos_.y);
    fAddMarginHeight = pSecondLayoutItem->s_size_.height - fOldHeight;
    pLayoutItem->AppendLastChild(pChildItem);
  }
  if (bOrphanedItem)
    pLayoutItem->AppendLastChild(pSecondLayoutItem);
}

void CXFA_ContentLayoutProcessor::SplitLayoutItem(float fSplitPos) {
  DCHECK(layout_item_);
  SplitLayoutItem(layout_item_.Get(), nullptr, fSplitPos);
}

CXFA_ContentLayoutItem* CXFA_ContentLayoutProcessor::ExtractLayoutItem() {
  CXFA_ContentLayoutItem* pLayoutItem = layout_item_;
  if (pLayoutItem) {
    layout_item_ = ToContentLayoutItem(pLayoutItem->GetNextSibling());
    pLayoutItem->RemoveSelfIfParented();
  }
  if (cur_child_node_stage_ != Stage::kDone || !old_layout_item_) {
    return pLayoutItem;
  }

  CXFA_FFNotify* pNotify =
      old_layout_item_->GetFormNode()->GetDocument()->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(
      old_layout_item_->GetFormNode()->GetDocument());

  while (old_layout_item_) {
    CXFA_ContentLayoutItem* pToDeleteItem = old_layout_item_;
    old_layout_item_ = pToDeleteItem->GetNext();
    if (pToDeleteItem == pLayoutItem)
      break;
    pNotify->OnLayoutItemRemoving(pDocLayout, pToDeleteItem);
    pToDeleteItem->RemoveSelfIfParented();
  }
  return pLayoutItem;
}

void CXFA_ContentLayoutProcessor::GotoNextContainerNodeSimple() {
  std::tie(cur_child_node_stage_, cur_child_node_) = GotoNextContainerNode(
      cur_child_node_stage_, GetFormNode(), cur_child_node_);
}

std::pair<CXFA_ContentLayoutProcessor::Stage, CXFA_Node*>
CXFA_ContentLayoutProcessor::GotoNextContainerNode(Stage nCurStage,
                                                   CXFA_Node* pParentContainer,
                                                   CXFA_Node* pCurActionNode) {
  CXFA_Node* pChildContainer = nullptr;
  switch (nCurStage) {
    case Stage::kBreakBefore:
    case Stage::kBreakAfter: {
      pChildContainer = pCurActionNode->GetParent();
      break;
    }
    case Stage::kKeep:
    case Stage::kContainer:
      pChildContainer = pCurActionNode;
      break;
    default:
      pChildContainer = nullptr;
      break;
  }

  std::optional<Stage> ret;
  switch (nCurStage) {
    case Stage::kKeep:
      ret = HandleKeep(pChildContainer->GetFirstChild(), &pCurActionNode);
      if (ret.has_value())
        return {ret.value(), pCurActionNode};
      break;

    case Stage::kNone:
      pCurActionNode = nullptr;
      [[fallthrough]];

    case Stage::kBookendLeader:
      ret = HandleBookendLeader(pParentContainer, &pCurActionNode);
      if (ret.has_value())
        return {ret.value(), pCurActionNode};
      pCurActionNode = nullptr;
      [[fallthrough]];

    case Stage::kBreakBefore:
      ret = HandleBreakBefore(pChildContainer, &pCurActionNode);
      if (ret.has_value())
        return {ret.value(), pCurActionNode};
      break;

    case Stage::kContainer:
      pCurActionNode = nullptr;
      [[fallthrough]];

    case Stage::kBreakAfter:
      ret = HandleBreakAfter(pChildContainer, &pCurActionNode);
      if (ret.has_value())
        return {ret.value(), pCurActionNode};
      break;

    case Stage::kBookendTrailer:
      ret = HandleBookendTrailer(pParentContainer, &pCurActionNode);
      if (ret.has_value())
        return {ret.value(), pCurActionNode};
      [[fallthrough]];

    default:
      return {Stage::kDone, nullptr};
  }

  ret = HandleCheckNextChildContainer(pParentContainer, pChildContainer,
                                      &pCurActionNode);
  if (ret.has_value())
    return {ret.value(), pCurActionNode};

  pCurActionNode = nullptr;
  ret = HandleBookendTrailer(pParentContainer, &pCurActionNode);
  if (ret.has_value())
    return {ret.value(), pCurActionNode};

  return {Stage::kDone, nullptr};
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::ProcessKeepNodesForCheckNext(
    CXFA_Node** pCurActionNode,
    CXFA_Node** pNextContainer,
    bool* pLastKeepNode) {
  const bool bCanSplit =
      (*pNextContainer)->GetIntact() == XFA_AttributeValue::None;
  const bool bNextKeep = ExistContainerKeep(*pNextContainer, false);

  if (bNextKeep && !bCanSplit) {
    if (!is_process_keep_ && !keep_break_finish_) {
      keep_head_node_ = *pNextContainer;
      is_process_keep_ = true;
    }
    return std::nullopt;
  }

  if (!is_process_keep_ || !keep_head_node_) {
    if (keep_break_finish_) {
      *pLastKeepNode = true;
    }
    keep_break_finish_ = false;
    return std::nullopt;
  }

  keep_tail_node_ = *pNextContainer;
  if (keep_break_finish_) {
    *pNextContainer = keep_head_node_;
    ProcessKeepNodesEnd();
    return std::nullopt;
  }

  std::optional<Stage> ret =
      FindBreakBeforeNode((*pNextContainer)->GetFirstChild(), pCurActionNode);
  if (!ret.has_value()) {
    *pNextContainer = keep_head_node_;
    ProcessKeepNodesEnd();
    return std::nullopt;
  }

  return ret;
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::ProcessKeepNodesForBreakBefore(
    CXFA_Node** pCurActionNode,
    CXFA_Node* pContainerNode) {
  if (keep_tail_node_ == pContainerNode) {
    *pCurActionNode = keep_head_node_;
    ProcessKeepNodesEnd();
    return Stage::kContainer;
  }

  CXFA_Node* pBreakAfterNode = pContainerNode->GetFirstChild();
  return FindBreakAfterNode(pBreakAfterNode, pCurActionNode);
}

void CXFA_ContentLayoutProcessor::DoLayoutPageArea(
    CXFA_ViewLayoutItem* pPageAreaLayoutItem) {
  CXFA_Node* pFormNode = pPageAreaLayoutItem->GetFormNode();
  CXFA_Node* pCurChildNode = nullptr;
  CXFA_LayoutItem* pBeforeItem = nullptr;
  Stage nCurChildNodeStage = Stage::kNone;
  while (true) {
    std::tie(nCurChildNodeStage, pCurChildNode) =
        GotoNextContainerNode(nCurChildNodeStage, pFormNode, pCurChildNode);
    if (!pCurChildNode)
      break;

    if (nCurChildNodeStage != Stage::kContainer ||
        pCurChildNode->GetElementType() == XFA_Element::Variables)
      continue;

    auto* pProcessor = cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
        GetHeap()->GetAllocationHandle(), GetHeap(), pCurChildNode, nullptr);
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
  if (layout_item_) {
    return;
  }

  layout_item_ = CreateContentLayoutItem(GetFormNode());
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
  if (!cur_child_node_) {
    GotoNextContainerNodeSimple();
  }

  int32_t iColIndex = 0;
  for (; cur_child_node_; GotoNextContainerNodeSimple()) {
    if (cur_child_node_stage_ != Stage::kContainer) {
      continue;
    }
    if (cur_child_node_->GetElementType() == XFA_Element::Variables) {
      continue;
    }

    auto* pProcessor = cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
        GetHeap()->GetAllocationHandle(), GetHeap(), cur_child_node_,
        view_layout_processor_);

    if (pContext && pContext->prg_specified_column_widths_) {
      int32_t iColSpan =
          cur_child_node_->JSObject()->GetInteger(XFA_Attribute::ColSpan);
      if (iColSpan <= fxcrt::CollectionSize<int32_t>(
                          *pContext->prg_specified_column_widths_) -
                          iColIndex) {
        pContext->cur_column_width_ = 0.0f;
        if (iColSpan == -1) {
          iColSpan = fxcrt::CollectionSize<int32_t>(
              *pContext->prg_specified_column_widths_);
        }
        for (int32_t i = 0; iColIndex + i < iColSpan; ++i) {
          pContext->cur_column_width_.value() +=
              (*pContext->prg_specified_column_widths_)[iColIndex + i];
        }
        if (pContext->cur_column_width_.value() == 0) {
          pContext->cur_column_width_.reset();
        }

        iColIndex += iColSpan >= 0 ? iColSpan : 0;
      }
    }

    pProcessor->DoLayoutInternal(false, FLT_MAX, FLT_MAX, pContext);
    if (!pProcessor->HasLayoutItem())
      continue;

    CFX_SizeF size = pProcessor->GetCurrentComponentSize();
    bool bChangeParentSize = false;
    if (cur_child_node_->PresenceRequiresSpace()) {
      bChangeParentSize = true;
    }

    CFX_PointF absolutePos;
    if (!bIgnoreXY)
      absolutePos = CalculatePositionedContainerPos(cur_child_node_, size);

    pProcessor->SetCurrentComponentPos(absolutePos);
    if (bContainerWidthAutoSize) {
      float fChildSuppliedWidth = absolutePos.x + size.width;
      if (bChangeParentSize) {
        fContentCalculatedWidth =
            std::max(fContentCalculatedWidth, fChildSuppliedWidth);
      } else {
        if (fHiddenContentCalculatedWidth < fChildSuppliedWidth &&
            cur_child_node_->GetElementType() != XFA_Element::Subform) {
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
            cur_child_node_->GetElementType() != XFA_Element::Subform) {
          fHiddenContentCalculatedHeight = fChildSuppliedHeight;
        }
      }
    }
    layout_item_->AppendLastChild(pProcessor->ExtractLayoutItem());
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
  if (layout_item_) {
    return;
  }
  if (!pLayoutNode)
    pLayoutNode = GetFormNode();

  DCHECK(!cur_child_node_);

  layout_item_ = CreateContentLayoutItem(GetFormNode());
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
      width.TrimFront(L' ');
      if (width.IsEmpty())
        continue;

      rg_specified_column_widths_.push_back(
          CXFA_Measurement(width.AsStringView()).ToUnit(XFA_Unit::Pt));
    }
  }

  int32_t iSpecifiedColumnCount =
      fxcrt::CollectionSize<int32_t>(rg_specified_column_widths_);
  Context layoutContext;
  layoutContext.prg_specified_column_widths_ = &rg_specified_column_widths_;
  Context* pLayoutContext =
      iSpecifiedColumnCount > 0 ? &layoutContext : nullptr;
  if (!cur_child_node_) {
    GotoNextContainerNodeSimple();
  }

  for (; cur_child_node_; GotoNextContainerNodeSimple()) {
    layoutContext.cur_column_width_.reset();
    if (cur_child_node_stage_ != Stage::kContainer) {
      continue;
    }

    auto* pProcessor = cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
        GetHeap()->GetAllocationHandle(), GetHeap(), cur_child_node_,
        view_layout_processor_);

    pProcessor->DoLayoutInternal(false, FLT_MAX, FLT_MAX, pLayoutContext);
    if (!pProcessor->HasLayoutItem())
      continue;

    layout_item_->AppendLastChild(pProcessor->ExtractLayoutItem());
  }

  int32_t iRowCount = 0;
  int32_t iColCount = 0;
  {
    std::vector<CXFA_ContentLayoutItem*> rgRowItems;
    std::vector<int32_t> rgRowItemsSpan;
    std::vector<float> rgRowItemsWidth;
    for (CXFA_LayoutItem* pIter = layout_item_->GetFirstChild(); pIter;
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
        rgRowItemsWidth.push_back(pRowLayoutCell->s_size_.width);
      }
    }

    iRowCount = fxcrt::CollectionSize<int32_t>(rgRowItems);
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
          rgRowItemsWidth[i] = pNewCell ? pNewCell->s_size_.width : 0;
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
              fxcrt::CollectionSize<int32_t>(rg_specified_column_widths_);
          for (int32_t j = 0; j < c; j++)
            rg_specified_column_widths_.push_back(0);
        }
        if (rg_specified_column_widths_[iColCount] < kXFALayoutPrecision) {
          bAutoCol = true;
        }
        if (bAutoCol &&
            rg_specified_column_widths_[iColCount] < rgRowItemsWidth[i]) {
          rg_specified_column_widths_[iColCount] = rgRowItemsWidth[i];
        }
      }

      if (!bMoreColumns)
        continue;

      float fFinalColumnWidth = 0.0f;
      if (fxcrt::IndexInBounds(rg_specified_column_widths_, iColCount)) {
        fFinalColumnWidth = rg_specified_column_widths_[iColCount];
      }

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
  for (CXFA_LayoutItem* pIter = layout_item_->GetFirstChild(); pIter;
       pIter = pIter->GetNextSibling()) {
    CXFA_ContentLayoutItem* pLayoutChild = pIter->AsContentLayoutItem();
    if (!pLayoutChild || !pLayoutChild->GetFormNode()->PresenceRequiresSpace())
      continue;

    if (pLayoutChild->GetFormNode()->GetElementType() == XFA_Element::Subform) {
      XFA_AttributeValue eSubformLayout =
          pLayoutChild->GetFormNode()->JSObject()->GetEnum(
              XFA_Attribute::Layout);
      if (eSubformLayout == XFA_AttributeValue::Row ||
          eSubformLayout == XFA_AttributeValue::Rl_row) {
        RelocateTableRowCells(pLayoutChild, rg_specified_column_widths_,
                              eSubformLayout);
      }
    }

    pLayoutChild->s_pos_.y = fCurrentRowY;
    if (bContainerWidthAutoSize) {
      pLayoutChild->s_pos_.x = 0;
    } else {
      switch (pLayoutChild->GetFormNode()->JSObject()->GetEnum(
          XFA_Attribute::HAlign)) {
        case XFA_AttributeValue::Center:
          pLayoutChild->s_pos_.x =
              (fContentWidthLimit - pLayoutChild->s_size_.width) / 2;
          break;
        case XFA_AttributeValue::Right:
          pLayoutChild->s_pos_.x =
              fContentWidthLimit - pLayoutChild->s_size_.width;
          break;
        case XFA_AttributeValue::Left:
        default:
          pLayoutChild->s_pos_.x = 0;
          break;
      }
    }

    if (bContainerWidthAutoSize) {
      float fChildSuppliedWidth =
          pLayoutChild->s_pos_.x + pLayoutChild->s_size_.width;
      if (fContentWidthLimit < FLT_MAX &&
          fContentWidthLimit > fChildSuppliedWidth) {
        fChildSuppliedWidth = fContentWidthLimit;
      }
      fContentCalculatedWidth =
          std::max(fContentCalculatedWidth, fChildSuppliedWidth);
    }
    fCurrentRowY += pLayoutChild->s_size_.height;
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

  float fWidth = pTrailerItem->s_size_.width;
  XFA_AttributeValue eLayout =
      GetFormNode()->JSObject()->GetEnum(XFA_Attribute::Layout);
  return eLayout == XFA_AttributeValue::Tb || width_limit_ <= fWidth;
}

float CXFA_ContentLayoutProcessor::InsertKeepLayoutItems() {
  if (array_keep_items_.empty()) {
    return 0;
  }

  if (!layout_item_) {
    layout_item_ = CreateContentLayoutItem(GetFormNode());
    layout_item_->s_size_.clear();
  }

  float fTotalHeight = 0;
  for (const auto& item : pdfium::Reversed(array_keep_items_)) {
    AddLeaderAfterSplit(item);
    fTotalHeight += item->s_size_.height;
  }
  array_keep_items_.clear();

  return fTotalHeight;
}

bool CXFA_ContentLayoutProcessor::ProcessKeepForSplit(
    CXFA_ContentLayoutProcessor* pChildProcessor,
    Result eRetValue,
    ContentLayoutItemVector& rgCurLineLayoutItem,
    float* fContentCurRowAvailWidth,
    float* fContentCurRowHeight,
    float* fContentCurRowY,
    bool* bAddedItemInRow,
    bool* bForceEndPage,
    Result* result) {
  if (!pChildProcessor)
    return false;

  if (cur_child_node_->GetIntact() == XFA_AttributeValue::None &&
      pChildProcessor->has_avail_height_) {
    return false;
  }

  if (!ExistContainerKeep(cur_child_node_, true)) {
    return false;
  }

  CFX_SizeF childSize = pChildProcessor->GetCurrentComponentSize();
  std::vector<CXFA_ContentLayoutItem*> keepLayoutItems;
  if (JudgePutNextPage(layout_item_.Get(), childSize.height,
                       &keepLayoutItems)) {
    array_keep_items_.clear();
    for (CXFA_ContentLayoutItem* item : keepLayoutItems) {
      layout_item_->RemoveChild(item);
      *fContentCurRowY -= item->s_size_.height;
      array_keep_items_.push_back(item);
    }
    *bAddedItemInRow = true;
    *bForceEndPage = true;
    *result = Result::kPageFullBreak;
    return true;
  }

  rgCurLineLayoutItem.push_back(pChildProcessor->ExtractLayoutItem());
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
      fItemsHeight += pChildLayoutItem->s_size_.height;
    } else {
      pKeepItems->clear();
      fItemsHeight = 0;
    }
  }
  fItemsHeight += fChildHeight;
  return view_layout_processor_->GetNextAvailContentHeight(fItemsHeight);
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
    pNode->SetFlag(XFA_NodeFlag::kUnusedNode);
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
  has_avail_height_ = true;
  if (cur_child_preprocessor_) {
    cur_child_preprocessor_->pre_process_rs_ = Result::kDone;
  }

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
    has_avail_height_ = false;

  fRealHeight = fRealHeight - inset.top - inset.bottom;
  CFX_SizeF calculated_size;
  float fContentCurRowY = 0;
  CXFA_ContentLayoutItem* pLastChild = nullptr;
  if (layout_item_) {
    pLastChild = FindLastContentLayoutItem(eFlowStrategy);
    calculated_size = CalculateLayoutItemSize(pLastChild);
    fContentCurRowY =
        pLastChild ? pLastChild->s_pos_.y : calculated_size.height;
  }

  fContentCurRowY += InsertKeepLayoutItems();
  if (cur_child_node_stage_ == Stage::kNone) {
    GotoNextContainerNodeSimple();
  }

  fContentCurRowY += InsertPendingItems(GetFormNode());
  if (cur_child_preprocessor_ && cur_child_node_stage_ == Stage::kContainer) {
    if (ExistContainerKeep(cur_child_preprocessor_->GetFormNode(), false)) {
      keep_head_node_ = cur_child_node_;
      is_process_keep_ = true;
      cur_child_node_stage_ = Stage::kKeep;
    }
  }

  bool bForceEndPage = false;
  bool bBreakDone = false;
  bool bIsManualBreak = false;
  while (cur_child_node_stage_ != Stage::kDone) {
    float fContentCurRowHeight = 0;
    float fContentCurRowAvailWidth = fContentWidthLimit;
    width_limit_ = fContentCurRowAvailWidth;
    std::array<ContentLayoutItemVector, 3> rgCurLineLayoutItems;
    uint8_t uCurHAlignState =
        (eFlowStrategy != XFA_AttributeValue::Rl_tb ? 0 : 2);
    if (pLastChild) {
      for (CXFA_LayoutItem* pNext = pLastChild; pNext;
           pNext = pNext->GetNextSibling()) {
        CXFA_ContentLayoutItem* pLayoutNext = pNext->AsContentLayoutItem();
        if (!pLayoutNext)
          continue;
        if (!pLayoutNext->GetNextSibling() && cur_child_preprocessor_ &&
            cur_child_preprocessor_->GetFormNode() ==
                pLayoutNext->GetFormNode()) {
          if (cur_child_preprocessor_->layout_item_ &&
              cur_child_preprocessor_->layout_item_ != pLayoutNext) {
            pLayoutNext->InsertAfter(
                cur_child_preprocessor_->layout_item_.Get());
          }
          cur_child_preprocessor_->layout_item_ = pLayoutNext;
          break;
        }
        uint8_t uHAlign =
            HAlignEnumToInt(pLayoutNext->GetFormNode()->JSObject()->GetEnum(
                XFA_Attribute::HAlign));
        rgCurLineLayoutItems[uHAlign].emplace_back(pLayoutNext);
        if (eFlowStrategy == XFA_AttributeValue::Lr_tb) {
          if (uHAlign > uCurHAlignState)
            uCurHAlignState = uHAlign;
        } else if (uHAlign < uCurHAlignState) {
          uCurHAlignState = uHAlign;
        }
        if (pLayoutNext->GetFormNode()->PresenceRequiresSpace()) {
          if (pLayoutNext->s_size_.height > fContentCurRowHeight) {
            fContentCurRowHeight = pLayoutNext->s_size_.height;
          }
          fContentCurRowAvailWidth -= pLayoutNext->s_size_.width;
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

    while (cur_child_node_) {
      CXFA_ContentLayoutProcessor* pProcessor = nullptr;
      bool bAddedItemInRow = false;
      fContentCurRowY += InsertPendingItems(GetFormNode());
      switch (cur_child_node_stage_) {
        case Stage::kKeep:
        case Stage::kNone:
          break;
        case Stage::kBreakBefore: {
          for (auto& item : array_keep_items_) {
            layout_item_->RemoveChild(item);
            calculated_size.height -= item->s_size_.height;
          }

          if (!bUseBreakControl || !view_layout_processor_) {
            break;
          }

          std::optional<CXFA_ViewLayoutProcessor::BreakData> break_data =
              view_layout_processor_->ProcessBreakBefore(cur_child_node_);
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
                !layout_item_) {
              AddPendingNode(pTrailerNode, true);
            } else {
              auto* pTempProcessor =
                  cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
                      GetHeap()->GetAllocationHandle(), GetHeap(), pTrailerNode,
                      nullptr);

              InsertFlowedItem(
                  pTempProcessor, bContainerWidthAutoSize,
                  bContainerHeightAutoSize, container_size.height,
                  eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems, false,
                  FLT_MAX, FLT_MAX, fContentWidthLimit, &fContentCurRowY,
                  &fContentCurRowAvailWidth, &fContentCurRowHeight,
                  &bAddedItemInRow, &bForceEndPage, pContext, false);
            }
          }
          GotoNextContainerNodeSimple();
          bForceEndPage = true;
          bIsManualBreak = true;
          goto SuspendAndCreateNewRow;
        }
        case Stage::kBreakAfter: {
          if (!bUseBreakControl || !view_layout_processor_) {
            break;
          }

          std::optional<CXFA_ViewLayoutProcessor::BreakData> break_data =
              view_layout_processor_->ProcessBreakAfter(cur_child_node_);
          if (!break_data.has_value() ||
              GetFormNode()->GetElementType() == XFA_Element::Form) {
            break;
          }

          CXFA_Node* pLeaderNode = break_data.value().pLeader;
          CXFA_Node* pTrailerNode = break_data.value().pTrailer;
          bool bCreatePage = break_data.value().bCreatePage;
          if (JudgeLeaderOrTrailerForOccur(pTrailerNode)) {
            auto* pTempProcessor =
                cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
                    GetHeap()->GetAllocationHandle(), GetHeap(), pTrailerNode,
                    nullptr);

            InsertFlowedItem(pTempProcessor, bContainerWidthAutoSize,
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
              rgCurLineLayoutItems.front().clear();
              auto* pTempProcessor =
                  cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
                      GetHeap()->GetAllocationHandle(), GetHeap(), pLeaderNode,
                      nullptr);
              InsertFlowedItem(
                  pTempProcessor, bContainerWidthAutoSize,
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

          GotoNextContainerNodeSimple();
          if (bCreatePage) {
            bForceEndPage = true;
            bIsManualBreak = true;
            if (cur_child_node_stage_ == Stage::kDone) {
              bBreakDone = true;
            }
          }
          goto SuspendAndCreateNewRow;
        }
        case Stage::kBookendLeader: {
          if (cur_child_preprocessor_) {
            pProcessor = cur_child_preprocessor_.Get();
            cur_child_preprocessor_ = nullptr;
          } else if (view_layout_processor_) {
            CXFA_Node* pLeaderNode =
                view_layout_processor_->ProcessBookendLeader(cur_child_node_);
            if (pLeaderNode) {
              pProcessor =
                  cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
                      GetHeap()->GetAllocationHandle(), GetHeap(), pLeaderNode,
                      view_layout_processor_);
            }
          }

          if (pProcessor) {
            if (InsertFlowedItem(
                    pProcessor, bContainerWidthAutoSize,
                    bContainerHeightAutoSize, container_size.height,
                    eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems,
                    bUseBreakControl, fAvailHeight, fRealHeight,
                    fContentWidthLimit, &fContentCurRowY,
                    &fContentCurRowAvailWidth, &fContentCurRowHeight,
                    &bAddedItemInRow, &bForceEndPage, pContext,
                    false) != Result::kDone) {
              goto SuspendAndCreateNewRow;
            }
            pProcessor = nullptr;
          }
          break;
        }
        case Stage::kBookendTrailer: {
          if (cur_child_preprocessor_) {
            pProcessor = cur_child_preprocessor_;
            cur_child_preprocessor_.Clear();
          } else if (view_layout_processor_) {
            CXFA_Node* pTrailerNode =
                view_layout_processor_->ProcessBookendTrailer(cur_child_node_);
            if (pTrailerNode) {
              pProcessor =
                  cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
                      GetHeap()->GetAllocationHandle(), GetHeap(), pTrailerNode,
                      view_layout_processor_);
            }
          }
          if (pProcessor) {
            if (InsertFlowedItem(
                    pProcessor, bContainerWidthAutoSize,
                    bContainerHeightAutoSize, container_size.height,
                    eFlowStrategy, &uCurHAlignState, rgCurLineLayoutItems,
                    bUseBreakControl, fAvailHeight, fRealHeight,
                    fContentWidthLimit, &fContentCurRowY,
                    &fContentCurRowAvailWidth, &fContentCurRowHeight,
                    &bAddedItemInRow, &bForceEndPage, pContext,
                    false) != Result::kDone) {
              goto SuspendAndCreateNewRow;
            }
            pProcessor = nullptr;
          }
          break;
        }
        case Stage::kContainer: {
          DCHECK(cur_child_node_->IsContainerNode());
          if (cur_child_node_->GetElementType() == XFA_Element::Variables) {
            break;
          }
          if (fContentCurRowY >= fHeightLimit + kXFALayoutPrecision &&
              cur_child_node_->PresenceRequiresSpace()) {
            bForceEndPage = true;
            goto SuspendAndCreateNewRow;
          }
          if (!cur_child_node_->IsContainerNode()) {
            break;
          }

          bool bNewRow = false;
          if (cur_child_preprocessor_) {
            pProcessor = cur_child_preprocessor_;
            cur_child_preprocessor_.Clear();
            bNewRow = true;
          } else {
            pProcessor =
                cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
                    GetHeap()->GetAllocationHandle(), GetHeap(),
                    cur_child_node_, view_layout_processor_);
          }

          pProcessor->InsertPendingItems(cur_child_node_);
          Result rs = InsertFlowedItem(
              pProcessor, bContainerWidthAutoSize, bContainerHeightAutoSize,
              container_size.height, eFlowStrategy, &uCurHAlignState,
              rgCurLineLayoutItems, bUseBreakControl, fAvailHeight, fRealHeight,
              fContentWidthLimit, &fContentCurRowY, &fContentCurRowAvailWidth,
              &fContentCurRowHeight, &bAddedItemInRow, &bForceEndPage, pContext,
              bNewRow);
          switch (rs) {
            case Result::kManualBreak:
              bIsManualBreak = true;
              [[fallthrough]];
            case Result::kPageFullBreak:
              bForceEndPage = true;
              [[fallthrough]];
            case Result::kRowFullBreak:
              goto SuspendAndCreateNewRow;
            case Result::kDone:
              fContentCurRowY +=
                  pProcessor->InsertPendingItems(cur_child_node_);
              pProcessor = nullptr;
              break;
          }
          break;
        }
        case Stage::kDone:
          break;
      }
      GotoNextContainerNodeSimple();
      if (bAddedItemInRow && eFlowStrategy == XFA_AttributeValue::Tb)
        break;
      continue;
    SuspendAndCreateNewRow:
      if (pProcessor) {
        cur_child_preprocessor_ = pProcessor;
        pProcessor = nullptr;
      }
      break;
    }

    CalculateRowChildPosition(rgCurLineLayoutItems, eFlowStrategy,
                              bContainerHeightAutoSize, bContainerWidthAutoSize,
                              &calculated_size.width, &calculated_size.height,
                              &fContentCurRowY, fContentCurRowHeight,
                              fContentWidthLimit, bRootForceTb);
    width_limit_ = fContentCurRowAvailWidth;
    if (bForceEndPage)
      break;
  }

  bool bRetValue =
      cur_child_node_stage_ == Stage::kDone && pending_nodes_.empty();
  if (bBreakDone)
    bRetValue = false;

  container_size = CalculateContainerComponentSizeFromContentSize(
      GetFormNode(), bContainerWidthAutoSize, calculated_size.width,
      bContainerHeightAutoSize, calculated_size.height, container_size);

  if (container_size.height >= kXFALayoutPrecision || layout_item_ ||
      bRetValue) {
    if (!layout_item_) {
      layout_item_ = CreateContentLayoutItem(GetFormNode());
    }
    container_size.height = std::max(container_size.height, 0.f);

    SetCurrentComponentSize(container_size);
    if (bForceEndPage)
      used_size_ = 0;
    else
      used_size_ += layout_item_->s_size_.height;
  }

  if (bRetValue)
    return Result::kDone;
  return bIsManualBreak ? Result::kManualBreak : Result::kPageFullBreak;
}

bool CXFA_ContentLayoutProcessor::CalculateRowChildPosition(
    std::array<ContentLayoutItemVector, 3>& rgCurLineLayoutItems,
    XFA_AttributeValue eFlowStrategy,
    bool bContainerHeightAutoSize,
    bool bContainerWidthAutoSize,
    float* fContentCalculatedWidth,
    float* fContentCalculatedHeight,
    float* fContentCurRowY,
    float fContentCurRowHeight,
    float fContentWidthLimit,
    bool bRootForceTb) {
  std::array<int32_t, 3> nGroupLengths = {};
  std::array<float, 3> fGroupWidths = {};
  int32_t nTotalLength = 0;
  for (int32_t i = 0; i < 3; i++) {
    nGroupLengths[i] = fxcrt::CollectionSize<int32_t>(rgCurLineLayoutItems[i]);
    for (int32_t c = nGroupLengths[i], j = 0; j < c; j++) {
      nTotalLength++;
      if (rgCurLineLayoutItems[i][j]->GetFormNode()->PresenceRequiresSpace())
        fGroupWidths[i] += rgCurLineLayoutItems[i][j]->s_size_.width;
    }
  }
  if (!nTotalLength) {
    if (bContainerHeightAutoSize) {
      *fContentCalculatedHeight =
          std::min(*fContentCalculatedHeight, *fContentCurRowY);
    }
    return false;
  }
  if (!layout_item_) {
    layout_item_ = CreateContentLayoutItem(GetFormNode());
  }

  if (eFlowStrategy != XFA_AttributeValue::Rl_tb) {
    float fCurPos;
    fCurPos = 0;
    for (int32_t c = nGroupLengths[0], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[0][j]->s_pos_ = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[0][j]->GetFormNode(),
            rgCurLineLayoutItems[0][j]->s_size_);
      } else {
        rgCurLineLayoutItems[0][j]->s_pos_ =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (rgCurLineLayoutItems[0][j]->GetFormNode()->PresenceRequiresSpace())
          fCurPos += rgCurLineLayoutItems[0][j]->s_size_.width;
      }
      layout_item_->AppendLastChild(rgCurLineLayoutItems[0][j]);
      last_row_width_ = fCurPos;
    }
    fCurPos = (fContentWidthLimit + fGroupWidths[0] - fGroupWidths[1] -
               fGroupWidths[2]) /
              2;
    for (int32_t c = nGroupLengths[1], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[1][j]->s_pos_ = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[1][j]->GetFormNode(),
            rgCurLineLayoutItems[1][j]->s_size_);
      } else {
        rgCurLineLayoutItems[1][j]->s_pos_ =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (rgCurLineLayoutItems[1][j]->GetFormNode()->PresenceRequiresSpace())
          fCurPos += rgCurLineLayoutItems[1][j]->s_size_.width;
      }
      layout_item_->AppendLastChild(rgCurLineLayoutItems[1][j]);
      last_row_width_ = fCurPos;
    }
    fCurPos = fContentWidthLimit - fGroupWidths[2];
    for (int32_t c = nGroupLengths[2], j = 0; j < c; j++) {
      if (bRootForceTb) {
        rgCurLineLayoutItems[2][j]->s_pos_ = CalculatePositionedContainerPos(
            rgCurLineLayoutItems[2][j]->GetFormNode(),
            rgCurLineLayoutItems[2][j]->s_size_);
      } else {
        rgCurLineLayoutItems[2][j]->s_pos_ =
            CFX_PointF(fCurPos, *fContentCurRowY);
        if (rgCurLineLayoutItems[2][j]->GetFormNode()->PresenceRequiresSpace())
          fCurPos += rgCurLineLayoutItems[2][j]->s_size_.width;
      }
      layout_item_->AppendLastChild(rgCurLineLayoutItems[2][j]);
      last_row_width_ = fCurPos;
    }
  } else {
    float fCurPos;
    fCurPos = fGroupWidths[0];
    for (int32_t c = nGroupLengths[0], j = 0; j < c; j++) {
      if (rgCurLineLayoutItems[0][j]->GetFormNode()->PresenceRequiresSpace())
        fCurPos -= rgCurLineLayoutItems[0][j]->s_size_.width;

      rgCurLineLayoutItems[0][j]->s_pos_ =
          CFX_PointF(fCurPos, *fContentCurRowY);
      layout_item_->AppendLastChild(rgCurLineLayoutItems[0][j]);
      last_row_width_ = fCurPos;
    }
    fCurPos = (fContentWidthLimit + fGroupWidths[0] + fGroupWidths[1] -
               fGroupWidths[2]) /
              2;
    for (int32_t c = nGroupLengths[1], j = 0; j < c; j++) {
      if (rgCurLineLayoutItems[1][j]->GetFormNode()->PresenceRequiresSpace())
        fCurPos -= rgCurLineLayoutItems[1][j]->s_size_.width;

      rgCurLineLayoutItems[1][j]->s_pos_ =
          CFX_PointF(fCurPos, *fContentCurRowY);
      layout_item_->AppendLastChild(rgCurLineLayoutItems[1][j]);
      last_row_width_ = fCurPos;
    }
    fCurPos = fContentWidthLimit;
    for (int32_t c = nGroupLengths[2], j = 0; j < c; j++) {
      if (rgCurLineLayoutItems[2][j]->GetFormNode()->PresenceRequiresSpace())
        fCurPos -= rgCurLineLayoutItems[2][j]->s_size_.width;

      rgCurLineLayoutItems[2][j]->s_pos_ =
          CFX_PointF(fCurPos, *fContentCurRowY);
      layout_item_->AppendLastChild(rgCurLineLayoutItems[2][j]);
      last_row_width_ = fCurPos;
    }
  }
  last_row_y_ = *fContentCurRowY;
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
  if (layout_item_) {
    return;
  }

  DCHECK(!cur_child_node_);
  layout_item_ = CreateContentLayoutItem(GetFormNode());
  if (!layout_item_) {
    return;
  }

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
          cur_child_node_stage_ = Stage::kDone;
          return Result::kDone;
        case XFA_AttributeValue::Table:
          DoLayoutTableContainer(pLayoutNode);
          cur_child_node_stage_ = Stage::kDone;
          return Result::kDone;
      }
    }
    case XFA_Element::Draw:
    case XFA_Element::Field:
      DoLayoutField();
      cur_child_node_stage_ = Stage::kDone;
      return Result::kDone;
    case XFA_Element::ContentArea:
    default:
      return Result::kDone;
  }
}

CFX_SizeF CXFA_ContentLayoutProcessor::GetCurrentComponentSize() {
  return CFX_SizeF(layout_item_->s_size_.width, layout_item_->s_size_.height);
}

void CXFA_ContentLayoutProcessor::SetCurrentComponentPos(
    const CFX_PointF& pos) {
  layout_item_->s_pos_ = pos;
}

void CXFA_ContentLayoutProcessor::SetCurrentComponentSize(
    const CFX_SizeF& size) {
  layout_item_->s_size_ = size;
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

  int32_t iCount = pending_nodes_count_[pTemplate];
  if (iCount >= iMax)
    return false;

  pending_nodes_count_[pTemplate] = iCount + 1;
  return true;
}

void CXFA_ContentLayoutProcessor::UpdatePendingItemLayout(
    CXFA_ContentLayoutItem* pLayoutItem) {
  XFA_AttributeValue eLayout =
      pLayoutItem->GetFormNode()->JSObject()->GetEnum(XFA_Attribute::Layout);
  switch (eLayout) {
    case XFA_AttributeValue::Row:
    case XFA_AttributeValue::Rl_row:
      RelocateTableRowCells(pLayoutItem, rg_specified_column_widths_, eLayout);
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

  float fHeight = pTrailerLayoutItem->s_size_.height;
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
    pTrailerLayoutItem->s_pos_.y = last_row_y_;
    pTrailerLayoutItem->s_pos_.x = last_row_width_;
    layout_item_->s_size_.width += pTrailerLayoutItem->s_size_.width;
    layout_item_->AppendLastChild(pTrailerLayoutItem);
    return;
  }

  float fNewSplitPos = 0;
  if (fSplitPos - fHeight > kXFALayoutPrecision)
    fNewSplitPos = FindSplitPos(fSplitPos - fHeight);

  if (fNewSplitPos > kXFALayoutPrecision) {
    SplitLayoutItem(fNewSplitPos);
    pTrailerLayoutItem->s_pos_.y = fNewSplitPos - inset.top - inset.bottom;
  } else {
    pTrailerLayoutItem->s_pos_.y = fSplitPos - inset.top - inset.bottom;
  }

  switch (pTrailerLayoutItem->GetFormNode()->JSObject()->GetEnum(
      XFA_Attribute::HAlign)) {
    case XFA_AttributeValue::Right:
      pTrailerLayoutItem->s_pos_.x = layout_item_->s_size_.width - inset.right -
                                     pTrailerLayoutItem->s_size_.width;
      break;
    case XFA_AttributeValue::Center:
      pTrailerLayoutItem->s_pos_.x =
          (layout_item_->s_size_.width - inset.left - inset.right -
           pTrailerLayoutItem->s_size_.width) /
          2;
      break;
    case XFA_AttributeValue::Left:
    default:
      pTrailerLayoutItem->s_pos_.x = inset.left;
      break;
  }
  layout_item_->s_size_.height += fHeight;
  layout_item_->AppendLastChild(pTrailerLayoutItem);
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

  float fHeight = pLeaderLayoutItem->s_size_.height;
  for (CXFA_LayoutItem* pIter = layout_item_->GetFirstChild(); pIter;
       pIter = pIter->GetNextSibling()) {
    CXFA_ContentLayoutItem* pContentItem = pIter->AsContentLayoutItem();
    if (!pContentItem)
      continue;

    pContentItem->s_pos_.y += fHeight;
  }
  pLeaderLayoutItem->s_pos_.y = 0;

  switch (pLeaderLayoutItem->GetFormNode()->JSObject()->GetEnum(
      XFA_Attribute::HAlign)) {
    case XFA_AttributeValue::Right:
      pLeaderLayoutItem->s_pos_.x = layout_item_->s_size_.width - fRightInset -
                                    pLeaderLayoutItem->s_size_.width;
      break;
    case XFA_AttributeValue::Center:
      pLeaderLayoutItem->s_pos_.x =
          (layout_item_->s_size_.width - fLeftInset - fRightInset -
           pLeaderLayoutItem->s_size_.width) /
          2;
      break;
    case XFA_AttributeValue::Left:
    default:
      pLeaderLayoutItem->s_pos_.x = fLeftInset;
      break;
  }
  layout_item_->s_size_.height += fHeight;
  layout_item_->AppendLastChild(pLeaderLayoutItem);
}

void CXFA_ContentLayoutProcessor::AddPendingNode(CXFA_Node* pPendingNode,
                                                 bool bBreakPending) {
  pending_nodes_.push_back(pPendingNode);
  break_pending_ = bBreakPending;
}

float CXFA_ContentLayoutProcessor::InsertPendingItems(
    CXFA_Node* pCurChildNode) {
  float fTotalHeight = 0;
  if (pending_nodes_.empty()) {
    return fTotalHeight;
  }

  if (!layout_item_) {
    layout_item_ = CreateContentLayoutItem(pCurChildNode);
    layout_item_->s_size_.clear();
  }

  while (!pending_nodes_.empty()) {
    auto* pPendingProcessor =
        cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
            GetHeap()->GetAllocationHandle(), GetHeap(), pending_nodes_.front(),
            nullptr);
    pending_nodes_.pop_front();
    pPendingProcessor->DoLayout(false, FLT_MAX, FLT_MAX);
    CXFA_ContentLayoutItem* pPendingLayoutItem = nullptr;
    if (pPendingProcessor->HasLayoutItem())
      pPendingLayoutItem = pPendingProcessor->ExtractLayoutItem();
    if (pPendingLayoutItem) {
      AddLeaderAfterSplit(pPendingLayoutItem);
      if (break_pending_) {
        fTotalHeight += pPendingLayoutItem->s_size_.height;
      }
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
    std::array<ContentLayoutItemVector, 3>& rgCurLineLayoutItems,
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
      cur_child_node_->JSObject()->GetEnum(XFA_Attribute::HAlign));
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
  if (view_layout_processor_) {
    CXFA_Node* pOverflowNode =
        view_layout_processor_->QueryOverflow(GetFormNode());
    if (pOverflowNode) {
      layoutContext.overflow_node_ = pOverflowNode;
      layoutContext.overflow_processor_ = this;
      pLayoutContext = &layoutContext;
    }
  }

  Result eRetValue = Result::kDone;
  if (!bNewRow || pProcessor->pre_process_rs_ == Result::kDone) {
    eRetValue = pProcessor->DoLayoutInternal(
        bTakeSpace && bUseBreakControl,
        bUseRealHeight ? fRealHeight - *fContentCurRowY : FLT_MAX,
        bIsTransHeight ? fRealHeight - *fContentCurRowY : FLT_MAX,
        pLayoutContext);
    pProcessor->pre_process_rs_ = eRetValue;
  } else {
    eRetValue = pProcessor->pre_process_rs_;
    pProcessor->pre_process_rs_ = Result::kDone;
  }
  if (!pProcessor->HasLayoutItem())
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
  if (view_layout_processor_ &&
      pProcessor->GetFormNode()->GetIntact() == XFA_AttributeValue::None) {
    pFormNode =
        view_layout_processor_->QueryOverflow(pProcessor->GetFormNode());
    if (!pFormNode && pLayoutContext && pLayoutContext->overflow_processor_) {
      pFormNode = pLayoutContext->overflow_node_;
      bUseInherited = true;
    }
    std::optional<CXFA_ViewLayoutProcessor::OverflowData> overflow_data =
        view_layout_processor_->ProcessOverflow(pFormNode, false);
    if (overflow_data.has_value()) {
      pOverflowLeaderNode = overflow_data.value().pLeader;
      pOverflowTrailerNode = overflow_data.value().pTrailer;
      if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowTrailerNode)) {
        if (pOverflowTrailerNode) {
          auto* pOverflowLeaderProcessor =
              cppgc::MakeGarbageCollected<CXFA_ContentLayoutProcessor>(
                  GetHeap()->GetAllocationHandle(), GetHeap(),
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
          childSize.height += pTrailerLayoutItem->s_size_.height;
          bIsAddTrailerHeight = true;
        }
      }
    }
  }

  if (!bTakeSpace ||
      *fContentCurRowY + childSize.height <=
          fAvailHeight + kXFALayoutPrecision ||
      (!bContainerHeightAutoSize &&
       used_size_ + fAvailHeight + kXFALayoutPrecision >= fContainerHeight)) {
    if (!bTakeSpace || eRetValue == Result::kDone) {
      if (pProcessor->use_inherited_) {
        if (pTrailerLayoutItem)
          pProcessor->AddTrailerBeforeSplit(childSize.height,
                                            pTrailerLayoutItem, false);
        if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
          pProcessor->AddPendingNode(pOverflowLeaderNode, false);

        pProcessor->use_inherited_ = false;
      } else {
        if (bIsAddTrailerHeight)
          childSize.height -= pTrailerLayoutItem->s_size_.height;

        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
      }

      CXFA_ContentLayoutItem* pChildLayoutItem =
          pProcessor->ExtractLayoutItem();
      if (ExistContainerKeep(pProcessor->GetFormNode(), false) &&
          pProcessor->GetFormNode()->GetIntact() == XFA_AttributeValue::None) {
        array_keep_items_.push_back(pChildLayoutItem);
      } else {
        array_keep_items_.clear();
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
      if (pProcessor->use_inherited_) {
        if (pTrailerLayoutItem) {
          pProcessor->AddTrailerBeforeSplit(childSize.height,
                                            pTrailerLayoutItem, false);
        }
        if (pProcessor->JudgeLeaderOrTrailerForOccur(pOverflowLeaderNode))
          pProcessor->AddPendingNode(pOverflowLeaderNode, false);

        pProcessor->use_inherited_ = false;
      } else {
        if (bIsAddTrailerHeight)
          childSize.height -= pTrailerLayoutItem->s_size_.height;

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
  if (ProcessKeepForSplit(pProcessor, eRetValue, rgCurLineLayoutItems[uHAlign],
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

    if (view_layout_processor_ && !pProcessor->use_inherited_ &&
        eRetValue != Result::kPageFullBreak) {
      view_layout_processor_->ProcessOverflow(pFormNode, true);
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
      use_inherited_ = true;
    } else {
      CXFA_LayoutItem* firstChild = pProcessor->layout_item_->GetFirstChild();
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

    if (pProcessor->layout_item_->GetNextSibling()) {
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
    if (pProcessor->view_layout_processor_->GetNextAvailContentHeight(
            childSize.height)) {
      if (view_layout_processor_) {
        if (!pFormNode && pLayoutContext)
          pFormNode = pLayoutContext->overflow_processor_->GetFormNode();

        view_layout_processor_->ProcessOverflow(pFormNode, true);
      }
      if (bUseInherited) {
        pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode,
                                         pOverflowTrailerNode,
                                         pTrailerLayoutItem, pFormNode);
        use_inherited_ = true;
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
    if (view_layout_processor_) {
      std::optional<CXFA_ViewLayoutProcessor::OverflowData> overflow_data =
          view_layout_processor_->ProcessOverflow(pFormNode, true);
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
    pFormNode = pLayoutContext->overflow_processor_->GetFormNode();
  if (view_layout_processor_) {
    std::optional<CXFA_ViewLayoutProcessor::OverflowData> overflow_data =
        view_layout_processor_->ProcessOverflow(pFormNode, true);
    if (overflow_data.has_value()) {
      pOverflowLeaderNode = overflow_data.value().pLeader;
      pOverflowTrailerNode = overflow_data.value().pTrailer;
    }
  }
  if (bUseInherited) {
    pProcessor->ProcessUnUseOverFlow(pOverflowLeaderNode, pOverflowTrailerNode,
                                     pTrailerLayoutItem, pFormNode);
    use_inherited_ = true;
  }
  return Result::kPageFullBreak;
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleKeep(CXFA_Node* pBreakAfterNode,
                                        CXFA_Node** pCurActionNode) {
  if (keep_break_finish_) {
    return std::nullopt;
  }
  return FindBreakAfterNode(pBreakAfterNode, pCurActionNode);
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
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
  return std::nullopt;
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleBreakBefore(CXFA_Node* pChildContainer,
                                               CXFA_Node** pCurActionNode) {
  if (!*pCurActionNode)
    return std::nullopt;

  CXFA_Node* pBreakBeforeNode = (*pCurActionNode)->GetNextSibling();
  if (!keep_break_finish_) {
    std::optional<Stage> ret =
        FindBreakBeforeNode(pBreakBeforeNode, pCurActionNode);
    if (ret.has_value())
      return ret.value();
  }
  if (is_process_keep_) {
    return ProcessKeepNodesForBreakBefore(pCurActionNode, pChildContainer);
  }

  *pCurActionNode = pChildContainer;
  return Stage::kContainer;
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
CXFA_ContentLayoutProcessor::HandleBreakAfter(CXFA_Node* pChildContainer,
                                              CXFA_Node** pCurActionNode) {
  if (*pCurActionNode) {
    CXFA_Node* pBreakAfterNode = (*pCurActionNode)->GetNextSibling();
    return FindBreakAfterNode(pBreakAfterNode, pCurActionNode);
  }

  CXFA_Node* pBreakAfterNode = pChildContainer->GetFirstChild();
  return HandleKeep(pBreakAfterNode, pCurActionNode);
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
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
    return std::nullopt;

  bool bLastKeep = false;
  std::optional<Stage> ret = ProcessKeepNodesForCheckNext(
      pCurActionNode, &pNextChildContainer, &bLastKeep);
  if (ret.has_value())
    return ret.value();

  if (!keep_break_finish_ && !bLastKeep) {
    ret = FindBreakBeforeNode(pNextChildContainer->GetFirstChild(),
                              pCurActionNode);
    if (ret.has_value())
      return ret.value();
  }
  *pCurActionNode = pNextChildContainer;
  return is_process_keep_ ? Stage::kKeep : Stage::kContainer;
}

std::optional<CXFA_ContentLayoutProcessor::Stage>
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
  return std::nullopt;
}

void CXFA_ContentLayoutProcessor::ProcessKeepNodesEnd() {
  keep_break_finish_ = true;
  keep_head_node_ = nullptr;
  keep_tail_node_ = nullptr;
  is_process_keep_ = false;
}

void CXFA_ContentLayoutProcessor::AdjustContainerSpecifiedSize(
    Context* pContext,
    CFX_SizeF* pSize,
    bool* pContainerWidthAutoSize,
    bool* pContainerHeightAutoSize) {
  if (pContext && pContext->cur_column_width_.has_value()) {
    pSize->width = pContext->cur_column_width_.value();
    *pContainerWidthAutoSize = false;
  }
  if (*pContainerHeightAutoSize)
    return;

  pSize->height -= used_size_;
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
  if (cur_child_node_stage_ == Stage::kDone ||
      eFlowStrategy == XFA_AttributeValue::Tb) {
    return nullptr;
  }

  CXFA_ContentLayoutItem* pLastChild =
      ToContentLayoutItem(layout_item_->GetFirstChild());
  for (CXFA_LayoutItem* pNext = pLastChild; pNext;
       pNext = pNext->GetNextSibling()) {
    CXFA_ContentLayoutItem* pContentNext = pNext->AsContentLayoutItem();
    if (pContentNext && pContentNext->s_pos_.y != pLastChild->s_pos_.y) {
      pLastChild = pContentNext;
    }
  }
  return pLastChild;
}

CFX_SizeF CXFA_ContentLayoutProcessor::CalculateLayoutItemSize(
    const CXFA_ContentLayoutItem* pLastChild) {
  CFX_SizeF size;
  for (CXFA_LayoutItem* pChild = layout_item_->GetFirstChild();
       pChild != pLastChild; pChild = pChild->GetNextSibling()) {
    CXFA_ContentLayoutItem* pLayout = pChild->AsContentLayoutItem();
    if (!pLayout || !pLayout->GetFormNode()->PresenceRequiresSpace())
      continue;

    float fWidth = pLayout->s_pos_.x + pLayout->s_size_.width;
    float fHeight = pLayout->s_pos_.y + pLayout->s_size_.height;
    size.width = std::max(size.width, fWidth);
    size.height = std::max(size.height, fHeight);
  }
  return size;
}

CXFA_ContentLayoutProcessor::Context::Context() = default;

CXFA_ContentLayoutProcessor::Context::~Context() = default;
