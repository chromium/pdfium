// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"

#include "core/fxcrt/check_op.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ContentLayoutItem::CXFA_ContentLayoutItem(CXFA_Node* pNode,
                                               CXFA_FFWidget* pWidget)
    : CXFA_LayoutItem(pNode, kContentItem), ffwidget_(pWidget) {
  if (ffwidget_) {
    ffwidget_->SetLayoutItem(this);
  }
}

CXFA_ContentLayoutItem::~CXFA_ContentLayoutItem() = default;

void CXFA_ContentLayoutItem::Trace(cppgc::Visitor* visitor) const {
  CXFA_LayoutItem::Trace(visitor);
  visitor->Trace(prev_);
  visitor->Trace(next_);
  visitor->Trace(ffwidget_);
}

CXFA_ContentLayoutItem* CXFA_ContentLayoutItem::GetFirst() {
  CXFA_ContentLayoutItem* pCurNode = this;
  while (auto* pPrev = pCurNode->GetPrev()) {
    pCurNode = pPrev;
  }

  return pCurNode;
}

CXFA_ContentLayoutItem* CXFA_ContentLayoutItem::GetLast() {
  CXFA_ContentLayoutItem* pCurNode = this;
  while (auto* pNext = pCurNode->GetNext()) {
    pCurNode = pNext;
  }

  return pCurNode;
}

void CXFA_ContentLayoutItem::InsertAfter(CXFA_ContentLayoutItem* pItem) {
  CHECK_NE(this, pItem);
  pItem->RemoveSelf();
  pItem->next_ = next_;
  pItem->prev_ = this;
  next_ = pItem;
  if (pItem->next_) {
    pItem->next_->prev_ = pItem;
  }
}

void CXFA_ContentLayoutItem::RemoveSelf() {
  if (next_) {
    next_->prev_ = prev_;
  }
  if (prev_) {
    prev_->next_ = next_;
  }
}

CFX_RectF CXFA_ContentLayoutItem::GetRelativeRect() const {
  return CFX_RectF(s_pos_, s_size_);
}

CFX_RectF CXFA_ContentLayoutItem::GetAbsoluteRect() const {
  CFX_PointF sPos = s_pos_;
  CFX_SizeF sSize = s_size_;

  for (CXFA_LayoutItem* pLayoutItem = GetParent(); pLayoutItem;
       pLayoutItem = pLayoutItem->GetParent()) {
    if (CXFA_ContentLayoutItem* pContent = pLayoutItem->AsContentLayoutItem()) {
      sPos += pContent->s_pos_;
      CXFA_Margin* pMarginNode =
          pContent->GetFormNode()->GetFirstChildByClass<CXFA_Margin>(
              XFA_Element::Margin);
      if (pMarginNode) {
        sPos += CFX_PointF(pMarginNode->JSObject()->GetMeasureInUnit(
                               XFA_Attribute::LeftInset, XFA_Unit::Pt),
                           pMarginNode->JSObject()->GetMeasureInUnit(
                               XFA_Attribute::TopInset, XFA_Unit::Pt));
      }
      continue;
    }

    if (pLayoutItem->GetFormNode()->GetElementType() ==
        XFA_Element::ContentArea) {
      sPos +=
          CFX_PointF(pLayoutItem->GetFormNode()->JSObject()->GetMeasureInUnit(
                         XFA_Attribute::X, XFA_Unit::Pt),
                     pLayoutItem->GetFormNode()->JSObject()->GetMeasureInUnit(
                         XFA_Attribute::Y, XFA_Unit::Pt));
      break;
    }
    if (pLayoutItem->GetFormNode()->GetElementType() == XFA_Element::PageArea) {
      break;
    }
  }
  return CFX_RectF(sPos, sSize);
}

size_t CXFA_ContentLayoutItem::GetIndex() const {
  size_t szIndex = 0;
  const CXFA_ContentLayoutItem* pCurNode = this;
  while (auto* pPrev = pCurNode->GetPrev()) {
    pCurNode = pPrev;
    ++szIndex;
  }
  return szIndex;
}
