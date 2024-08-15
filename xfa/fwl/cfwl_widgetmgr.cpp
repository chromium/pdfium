// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_widgetmgr.h"

#include "build/build_config.h"
#include "core/fxcrt/check.h"
#include "fxjs/gc/container_trace.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_message.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_pushbutton.h"

namespace pdfium {

CFWL_WidgetMgr::CFWL_WidgetMgr(AdapterIface* pAdapter, CFWL_App* pApp)
    : m_pAdapter(pAdapter), m_pApp(pApp) {
  DCHECK(m_pAdapter);
  m_mapWidgetItem[nullptr] = cppgc::MakeGarbageCollected<Item>(
      pApp->GetHeap()->GetAllocationHandle(), nullptr);
}

CFWL_WidgetMgr::~CFWL_WidgetMgr() = default;

void CFWL_WidgetMgr::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pApp);
  visitor->Trace(m_pAdapter);
  ContainerTrace(visitor, m_mapWidgetItem);
}

CFWL_Widget* CFWL_WidgetMgr::GetParentWidget(const CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return nullptr;

  Item* pParent = pItem->GetParent();
  return pParent ? pParent->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetPriorSiblingWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return nullptr;

  Item* pSibling = pItem->GetPrevSibling();
  return pSibling ? pSibling->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetNextSiblingWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return nullptr;

  Item* pSibling = pItem->GetNextSibling();
  return pSibling ? pSibling->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetFirstChildWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return nullptr;

  Item* pChild = pItem->GetFirstChild();
  return pChild ? pChild->pWidget : nullptr;
}

CFWL_Widget* CFWL_WidgetMgr::GetLastChildWidget(CFWL_Widget* pWidget) const {
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return nullptr;

  Item* pChild = pItem->GetLastChild();
  return pChild ? pChild->pWidget : nullptr;
}

void CFWL_WidgetMgr::RepaintWidget(CFWL_Widget* pWidget,
                                   const CFX_RectF& rect) {
  CFWL_Widget* pNative = pWidget;
  CFX_RectF transformedRect = rect;
  CFWL_Widget* pOuter = pWidget->GetOuter();
  while (pOuter) {
    CFX_RectF rtTemp = pNative->GetWidgetRect();
    transformedRect.left += rtTemp.left;
    transformedRect.top += rtTemp.top;
    pNative = pOuter;
    pOuter = pOuter->GetOuter();
  }
  m_pAdapter->RepaintWidget(pNative);
}

void CFWL_WidgetMgr::InsertWidget(CFWL_Widget* pParent, CFWL_Widget* pChild) {
  Item* pParentItem = GetWidgetMgrItem(pParent);
  if (!pParentItem) {
    pParentItem = CreateWidgetMgrItem(pParent);
    GetWidgetMgrRootItem()->AppendLastChild(pParentItem);
  }
  Item* pChildItem = GetWidgetMgrItem(pChild);
  if (!pChildItem)
    pChildItem = CreateWidgetMgrItem(pChild);
  pParentItem->AppendLastChild(pChildItem);
}

void CFWL_WidgetMgr::RemoveWidget(CFWL_Widget* pWidget) {
  DCHECK(pWidget);
  Item* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return;

  while (pItem->GetFirstChild())
    RemoveWidget(pItem->GetFirstChild()->pWidget);

  pItem->RemoveSelfIfParented();
  m_mapWidgetItem.erase(pWidget);
}

CFWL_Widget* CFWL_WidgetMgr::GetWidgetAtPoint(CFWL_Widget* parent,
                                              const CFX_PointF& point) const {
  if (!parent)
    return nullptr;

  CFWL_Widget* child = GetLastChildWidget(parent);
  while (child) {
    if (child->IsVisible()) {
      CFX_PointF pos = parent->GetMatrix().GetInverse().Transform(point);
      CFX_RectF bounds = child->GetWidgetRect();
      if (bounds.Contains(pos)) {
        pos -= bounds.TopLeft();
        return GetWidgetAtPoint(child, pos);
      }
    }
    child = GetPriorSiblingWidget(child);
  }
  return parent;
}

CFWL_Widget* CFWL_WidgetMgr::GetDefaultButton(CFWL_Widget* pParent) const {
  if (pParent->GetClassID() == FWL_Type::PushButton &&
      (pParent->GetStates() & FWL_STATE_PSB_Default)) {
    return pParent;
  }

  CFWL_Widget* child = GetFirstChildWidget(pParent);
  while (child) {
    if (child->GetClassID() == FWL_Type::PushButton &&
        (child->GetStates() & FWL_STATE_PSB_Default)) {
      return child;
    }
    if (CFWL_Widget* find = GetDefaultButton(child))
      return find;

    child = GetNextSiblingWidget(child);
  }
  return nullptr;
}

CFWL_WidgetMgr::Item* CFWL_WidgetMgr::GetWidgetMgrRootItem() const {
  return GetWidgetMgrItem(nullptr);
}

CFWL_WidgetMgr::Item* CFWL_WidgetMgr::GetWidgetMgrItem(
    const CFWL_Widget* pWidget) const {
  auto it = m_mapWidgetItem.find(pWidget);
  return it != m_mapWidgetItem.end() ? it->second : nullptr;
}

CFWL_WidgetMgr::Item* CFWL_WidgetMgr::CreateWidgetMgrItem(
    CFWL_Widget* pWidget) {
  auto* pItem = cppgc::MakeGarbageCollected<Item>(
      m_pApp->GetHeap()->GetAllocationHandle(), pWidget);
  m_mapWidgetItem[pWidget] = pItem;
  return pItem;
}

void CFWL_WidgetMgr::GetAdapterPopupPos(CFWL_Widget* pWidget,
                                        float fMinHeight,
                                        float fMaxHeight,
                                        const CFX_RectF& rtAnchor,
                                        CFX_RectF* pPopupRect) const {
  m_pAdapter->GetPopupPos(pWidget, fMinHeight, fMaxHeight, rtAnchor,
                          pPopupRect);
}

void CFWL_WidgetMgr::OnProcessMessageToForm(CFWL_Message* pMessage) {
  CFWL_Widget* pDstWidget = pMessage->GetDstTarget();
  if (!pDstWidget)
    return;

  CFWL_NoteDriver* pNoteDriver = pDstWidget->GetFWLApp()->GetNoteDriver();
  pNoteDriver->ProcessMessage(pMessage);
}

void CFWL_WidgetMgr::OnDrawWidget(CFWL_Widget* pWidget,
                                  CFGAS_GEGraphics* pGraphics,
                                  const CFX_Matrix& matrix) {
  if (!pWidget || !pGraphics)
    return;

  pWidget->GetDelegate()->OnDrawWidget(pGraphics, matrix);

  CFX_RectF clipBounds = pGraphics->GetClipRect();
  if (!clipBounds.IsEmpty())
    DrawChildren(pWidget, clipBounds, pGraphics, matrix);
}

void CFWL_WidgetMgr::DrawChildren(CFWL_Widget* parent,
                                  const CFX_RectF& rtClip,
                                  CFGAS_GEGraphics* pGraphics,
                                  const CFX_Matrix& mtMatrix) {
  if (!parent)
    return;

  CFWL_Widget* pNextChild = GetFirstChildWidget(parent);
  while (pNextChild) {
    CFWL_Widget* child = pNextChild;
    pNextChild = GetNextSiblingWidget(child);
    if (!child->IsVisible())
      continue;

    CFX_RectF rtWidget = child->GetWidgetRect();
    if (rtWidget.IsEmpty())
      continue;

    CFX_Matrix widgetMatrix;
    CFX_RectF clipBounds(rtWidget);
    widgetMatrix.Concat(mtMatrix);
    widgetMatrix.TranslatePrepend(rtWidget.left, rtWidget.top);

    if (IFWL_WidgetDelegate* pDelegate = child->GetDelegate())
      pDelegate->OnDrawWidget(pGraphics, widgetMatrix);

    DrawChildren(child, clipBounds, pGraphics, widgetMatrix);
  }
}

CFWL_WidgetMgr::Item::Item(CFWL_Widget* widget) : pWidget(widget) {}

CFWL_WidgetMgr::Item::~Item() = default;

void CFWL_WidgetMgr::Item::Trace(cppgc::Visitor* visitor) const {
  GCedTreeNode<Item>::Trace(visitor);
  visitor->Trace(pWidget);
}

}  // namespace pdfium
