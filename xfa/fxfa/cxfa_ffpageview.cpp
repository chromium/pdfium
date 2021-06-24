// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffpageview.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fxcrt/stl_util.h"
#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "xfa/fxfa/cxfa_ffcheckbutton.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffimageedit.h"
#include "xfa/fxfa/cxfa_ffpushbutton.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_traversal.h"
#include "xfa/fxfa/parser/cxfa_traverse.h"

namespace {

CFX_Matrix GetPageMatrix(const CFX_RectF& docPageRect,
                         const FX_RECT& devicePageRect,
                         int32_t iRotate) {
  DCHECK(iRotate >= 0);
  DCHECK(iRotate <= 3);

  CFX_Matrix m;
  if (iRotate == 0 || iRotate == 2) {
    m.a *= (float)devicePageRect.Width() / docPageRect.width;
    m.d *= (float)devicePageRect.Height() / docPageRect.height;
  } else {
    m.a *= (float)devicePageRect.Height() / docPageRect.width;
    m.d *= (float)devicePageRect.Width() / docPageRect.height;
  }
  m.Rotate(iRotate * 1.57079632675f);
  switch (iRotate) {
    case 0:
      m.e = devicePageRect.left;
      m.f = devicePageRect.top;
      break;
    case 1:
      m.e = devicePageRect.right;
      m.f = devicePageRect.top;
      break;
    case 2:
      m.e = devicePageRect.right;
      m.f = devicePageRect.bottom;
      break;
    case 3:
      m.e = devicePageRect.left;
      m.f = devicePageRect.bottom;
      break;
    default:
      break;
  }
  return m;
}

bool PageWidgetFilter(CXFA_FFWidget* pWidget,
                      XFA_WidgetStatusMask dwFilter,
                      bool bTraversal,
                      bool bIgnoreRelevant) {
  CXFA_Node* pNode = pWidget->GetNode();

  if ((dwFilter & XFA_WidgetStatus_Focused) &&
      (!pNode || pNode->GetElementType() != XFA_Element::Field)) {
    return false;
  }

  CXFA_ContentLayoutItem* pItem = pWidget->GetLayoutItem();
  if (bTraversal && pItem->TestStatusBits(XFA_WidgetStatus_Disabled))
    return false;
  if (bIgnoreRelevant)
    return pItem->TestStatusBits(XFA_WidgetStatus_Visible);

  dwFilter &= (XFA_WidgetStatus_Visible | XFA_WidgetStatus_Viewable |
               XFA_WidgetStatus_Printable);
  return pItem->TestStatusBits(dwFilter);
}

bool IsLayoutElement(XFA_Element eElement, bool bLayoutContainer) {
  switch (eElement) {
    case XFA_Element::Draw:
    case XFA_Element::Field:
    case XFA_Element::InstanceManager:
      return !bLayoutContainer;
    case XFA_Element::Area:
    case XFA_Element::Subform:
    case XFA_Element::ExclGroup:
    case XFA_Element::SubformSet:
    case XFA_Element::PageArea:
    case XFA_Element::Form:
      return true;
    default:
      return false;
  }
}

CXFA_Document* GetDocForPageView(const CXFA_FFPageView* view) {
  return view->GetDocView()->GetDoc()->GetXFADoc();
}

bool IsDocVersionBelow205(const CXFA_Document* doc) {
  return doc->GetCurVersionMode() < XFA_VERSION_205;
}

bool EnsureWidgetLoadedIfVisible(CXFA_FFWidget* pWidget) {
  if (!pWidget->IsLoaded() &&
      pWidget->GetLayoutItem()->TestStatusBits(XFA_WidgetStatus_Visible)) {
    if (!pWidget->LoadWidget())
      return false;
  }
  return true;
}

CXFA_FFWidget* LoadedWidgetFromLayoutItem(CXFA_LayoutItem* pLayoutItem) {
  CXFA_FFWidget* pWidget = CXFA_FFWidget::FromLayoutItem(pLayoutItem);
  if (!pWidget)
    return nullptr;

  EnsureWidgetLoadedIfVisible(pWidget);
  return pWidget;
}

CXFA_FFWidget* FilteredLoadedWidgetFromLayoutItem(CXFA_LayoutItem* pLayoutItem,
                                                  XFA_WidgetStatusMask dwFilter,
                                                  bool bIgnoreRelevant) {
  CXFA_FFWidget* pWidget = CXFA_FFWidget::FromLayoutItem(pLayoutItem);
  if (!pWidget)
    return nullptr;

  if (!PageWidgetFilter(pWidget, dwFilter, false, bIgnoreRelevant))
    return nullptr;

  if (!EnsureWidgetLoadedIfVisible(pWidget))
    return nullptr;

  return pWidget;
}

class CXFA_TabParam {
 public:
  CXFA_TabParam() = default;
  explicit CXFA_TabParam(CXFA_FFWidget* pWidget)
      : m_pItem(pWidget->GetLayoutItem()) {}
  CXFA_TabParam(const CXFA_TabParam&) = delete;
  CXFA_TabParam(CXFA_TabParam&&) noexcept = default;
  ~CXFA_TabParam() = default;

  CXFA_TabParam& operator=(const CXFA_TabParam&) = delete;
  CXFA_TabParam& operator=(CXFA_TabParam&&) noexcept = default;

  CXFA_FFWidget* GetWidget() const { return m_pItem->GetFFWidget(); }
  const std::vector<cppgc::Persistent<CXFA_ContentLayoutItem>>& GetChildren()
      const {
    return m_Children;
  }
  void ClearChildren() { m_Children.clear(); }
  void AppendTabParam(const CXFA_TabParam* pParam) {
    m_Children.push_back(pParam->m_pItem);
    m_Children.insert(m_Children.end(), pParam->m_Children.begin(),
                      pParam->m_Children.end());
  }

 private:
  cppgc::Persistent<CXFA_ContentLayoutItem> m_pItem;
  std::vector<cppgc::Persistent<CXFA_ContentLayoutItem>> m_Children;
};

void OrderContainer(CXFA_LayoutItemIterator* sIterator,
                    CXFA_LayoutItem* pViewItem,
                    CXFA_TabParam* pContainer,
                    bool* bCurrentItem,
                    bool* bContentArea,
                    bool bMasterPage) {
  std::vector<CXFA_TabParam> tabParams;
  CXFA_LayoutItem* pSearchItem = sIterator->MoveToNext();
  while (pSearchItem) {
    if (!pSearchItem->IsContentLayoutItem()) {
      *bContentArea = true;
      pSearchItem = sIterator->MoveToNext();
      continue;
    }
    if (bMasterPage && *bContentArea) {
      break;
    }
    if (bMasterPage || *bContentArea) {
      CXFA_FFWidget* hWidget = LoadedWidgetFromLayoutItem(pSearchItem);
      if (!hWidget) {
        pSearchItem = sIterator->MoveToNext();
        continue;
      }
      if (pViewItem && (pSearchItem->GetParent() != pViewItem)) {
        *bCurrentItem = true;
        break;
      }
      tabParams.emplace_back(hWidget);
      if (IsLayoutElement(pSearchItem->GetFormNode()->GetElementType(), true)) {
        OrderContainer(sIterator, pSearchItem, &tabParams.back(), bCurrentItem,
                       bContentArea, bMasterPage);
      }
    }
    if (*bCurrentItem) {
      pSearchItem = sIterator->GetCurrent();
      *bCurrentItem = false;
    } else {
      pSearchItem = sIterator->MoveToNext();
    }
  }
  std::sort(tabParams.begin(), tabParams.end(),
            [](const CXFA_TabParam& arg1, const CXFA_TabParam& arg2) {
              const CFX_RectF& rt1 = arg1.GetWidget()->GetWidgetRect();
              const CFX_RectF& rt2 = arg2.GetWidget()->GetWidgetRect();
              if (rt1.top - rt2.top >= kXFAWidgetPrecision)
                return rt1.top < rt2.top;
              return rt1.left < rt2.left;
            });
  for (const auto& param : tabParams)
    pContainer->AppendTabParam(&param);
}

}  // namespace

CXFA_FFPageView::CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea)
    : m_pPageArea(pPageArea), m_pDocView(pDocView) {}

CXFA_FFPageView::~CXFA_FFPageView() = default;

void CXFA_FFPageView::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pPageArea);
  visitor->Trace(m_pDocView);
  visitor->Trace(m_pLayoutItem);
}

CXFA_FFDocView* CXFA_FFPageView::GetDocView() const {
  return m_pDocView;
}

CFX_RectF CXFA_FFPageView::GetPageViewRect() const {
  auto* pItem = GetLayoutItem();
  if (!pItem)
    return CFX_RectF();

  return CFX_RectF(0, 0, pItem->GetPageSize());
}

CFX_Matrix CXFA_FFPageView::GetDisplayMatrix(const FX_RECT& rtDisp,
                                             int32_t iRotate) const {
  auto* pItem = GetLayoutItem();
  if (!pItem)
    return CFX_Matrix();

  return GetPageMatrix(CFX_RectF(0, 0, pItem->GetPageSize()), rtDisp, iRotate);
}

CXFA_FFWidget::IteratorIface* CXFA_FFPageView::CreateGCedFormWidgetIterator(
    XFA_WidgetStatusMask dwWidgetFilter) {
  return cppgc::MakeGarbageCollected<CXFA_FFPageWidgetIterator>(
      GetDocView()->GetDoc()->GetHeap()->GetAllocationHandle(), this,
      dwWidgetFilter);
}

CXFA_FFWidget::IteratorIface* CXFA_FFPageView::CreateGCedTraverseWidgetIterator(
    XFA_WidgetStatusMask dwWidgetFilter) {
  return cppgc::MakeGarbageCollected<CXFA_FFTabOrderPageWidgetIterator>(
      GetDocView()->GetDoc()->GetHeap()->GetAllocationHandle(), this,
      dwWidgetFilter);
}

CXFA_FFPageWidgetIterator::CXFA_FFPageWidgetIterator(
    CXFA_FFPageView* pPageView,
    XFA_WidgetStatusMask dwFilter)
    : m_sIterator(pPageView->GetLayoutItem()),
      m_dwFilter(dwFilter),
      m_bIgnoreRelevant(IsDocVersionBelow205(GetDocForPageView(pPageView))) {}

CXFA_FFPageWidgetIterator::~CXFA_FFPageWidgetIterator() = default;

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToFirst() {
  m_sIterator.Reset();
  for (CXFA_LayoutItem* pLayoutItem = m_sIterator.GetCurrent(); pLayoutItem;
       pLayoutItem = m_sIterator.MoveToNext()) {
    CXFA_FFWidget* hWidget = FilteredLoadedWidgetFromLayoutItem(
        pLayoutItem, m_dwFilter, m_bIgnoreRelevant);
    if (hWidget)
      return hWidget;
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToLast() {
  m_sIterator.SetCurrent(nullptr);
  return MoveToPrevious();
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToNext() {
  for (CXFA_LayoutItem* pLayoutItem = m_sIterator.MoveToNext(); pLayoutItem;
       pLayoutItem = m_sIterator.MoveToNext()) {
    CXFA_FFWidget* hWidget = FilteredLoadedWidgetFromLayoutItem(
        pLayoutItem, m_dwFilter, m_bIgnoreRelevant);
    if (hWidget)
      return hWidget;
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToPrevious() {
  for (CXFA_LayoutItem* pLayoutItem = m_sIterator.MoveToPrev(); pLayoutItem;
       pLayoutItem = m_sIterator.MoveToPrev()) {
    CXFA_FFWidget* hWidget = FilteredLoadedWidgetFromLayoutItem(
        pLayoutItem, m_dwFilter, m_bIgnoreRelevant);
    if (hWidget)
      return hWidget;
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::GetCurrentWidget() {
  CXFA_LayoutItem* pLayoutItem = m_sIterator.GetCurrent();
  return pLayoutItem ? CXFA_FFWidget::FromLayoutItem(pLayoutItem) : nullptr;
}

bool CXFA_FFPageWidgetIterator::SetCurrentWidget(CXFA_FFWidget* pWidget) {
  return pWidget && m_sIterator.SetCurrent(pWidget->GetLayoutItem());
}

CXFA_FFTabOrderPageWidgetIterator::CXFA_FFTabOrderPageWidgetIterator(
    CXFA_FFPageView* pPageView,
    XFA_WidgetStatusMask dwFilter)
    : m_pPageViewLayout(pPageView->GetLayoutItem()),
      m_dwFilter(dwFilter),
      m_bIgnoreRelevant(IsDocVersionBelow205(GetDocForPageView(pPageView))) {
  CreateTabOrderWidgetArray();
}

CXFA_FFTabOrderPageWidgetIterator::~CXFA_FFTabOrderPageWidgetIterator() =
    default;

void CXFA_FFTabOrderPageWidgetIterator::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pPageViewLayout);
  ContainerTrace(visitor, m_TabOrderWidgetArray);
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToFirst() {
  for (int32_t i = 0; i < fxcrt::CollectionSize<int32_t>(m_TabOrderWidgetArray);
       i++) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i]->GetFFWidget(), m_dwFilter,
                         true, m_bIgnoreRelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget]->GetFFWidget();
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToLast() {
  for (int32_t i = fxcrt::CollectionSize<int32_t>(m_TabOrderWidgetArray) - 1;
       i >= 0; i--) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i]->GetFFWidget(), m_dwFilter,
                         true, m_bIgnoreRelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget]->GetFFWidget();
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToNext() {
  for (int32_t i = m_iCurWidget + 1;
       i < fxcrt::CollectionSize<int32_t>(m_TabOrderWidgetArray); i++) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i]->GetFFWidget(), m_dwFilter,
                         true, m_bIgnoreRelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget]->GetFFWidget();
    }
  }
  m_iCurWidget = -1;
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToPrevious() {
  for (int32_t i = m_iCurWidget - 1; i >= 0; i--) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i]->GetFFWidget(), m_dwFilter,
                         true, m_bIgnoreRelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget]->GetFFWidget();
    }
  }
  m_iCurWidget = -1;
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetCurrentWidget() {
  return m_iCurWidget >= 0 ? m_TabOrderWidgetArray[m_iCurWidget]->GetFFWidget()
                           : nullptr;
}

bool CXFA_FFTabOrderPageWidgetIterator::SetCurrentWidget(
    CXFA_FFWidget* hWidget) {
  auto it = std::find(m_TabOrderWidgetArray.begin(),
                      m_TabOrderWidgetArray.end(), hWidget->GetLayoutItem());
  if (it == m_TabOrderWidgetArray.end())
    return false;

  m_iCurWidget = it - m_TabOrderWidgetArray.begin();
  return true;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetTraverseWidget(
    CXFA_FFWidget* pWidget) {
  CXFA_Traversal* pTraversal = pWidget->GetNode()->GetChild<CXFA_Traversal>(
      0, XFA_Element::Traversal, false);
  if (pTraversal) {
    CXFA_Traverse* pTraverse =
        pTraversal->GetChild<CXFA_Traverse>(0, XFA_Element::Traverse, false);
    if (pTraverse) {
      Optional<WideString> traverseWidgetName =
          pTraverse->JSObject()->TryAttribute(XFA_Attribute::Ref, true);
      if (traverseWidgetName.has_value())
        return FindWidgetByName(traverseWidgetName.value(), pWidget);
    }
  }
  return nullptr;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::FindWidgetByName(
    const WideString& wsWidgetName,
    CXFA_FFWidget* pRefWidget) {
  return pRefWidget->GetDocView()->GetWidgetByName(wsWidgetName, pRefWidget);
}

void CXFA_FFTabOrderPageWidgetIterator::CreateTabOrderWidgetArray() {
  m_TabOrderWidgetArray.clear();

  const std::vector<CXFA_ContentLayoutItem*> items =
      CreateSpaceOrderLayoutItems();
  if (items.empty())
    return;

  CXFA_ContentLayoutItem* item = items[0];
  while (m_TabOrderWidgetArray.size() < items.size()) {
    if (!pdfium::Contains(m_TabOrderWidgetArray, item)) {
      m_TabOrderWidgetArray.emplace_back(item);
      CXFA_Node* node = item->GetFFWidget()->GetNode();
      if (node->GetFFWidgetType() == XFA_FFWidgetType::kExclGroup) {
        auto it = std::find(items.begin(), items.end(), item);
        size_t index = it != items.end() ? it - items.begin() + 1 : 0;
        while (true) {
          CXFA_FFWidget* radio = items[index % items.size()]->GetFFWidget();
          if (radio->GetNode()->GetExclGroupIfExists() != node)
            break;
          if (!pdfium::Contains(m_TabOrderWidgetArray, item))
            m_TabOrderWidgetArray.emplace_back(radio->GetLayoutItem());
          ++index;
        }
      }
      CXFA_FFWidget* next_widget = GetTraverseWidget(item->GetFFWidget());
      if (next_widget) {
        item = next_widget->GetLayoutItem();
        continue;
      }
    }
    auto it = std::find(items.begin(), items.end(), item);
    size_t index = it != items.end() ? it - items.begin() + 1 : 0;
    item = items[index % items.size()];
  }
}

std::vector<CXFA_ContentLayoutItem*>
CXFA_FFTabOrderPageWidgetIterator::CreateSpaceOrderLayoutItems() {
  std::vector<CXFA_ContentLayoutItem*> items;
  CXFA_LayoutItemIterator sIterator(m_pPageViewLayout.Get());
  CXFA_TabParam tabparam;
  bool bCurrentItem = false;
  bool bContentArea = false;
  OrderContainer(&sIterator, nullptr, &tabparam, &bCurrentItem, &bContentArea,
                 false);
  items.reserve(tabparam.GetChildren().size());
  for (const auto& layout_item : tabparam.GetChildren())
    items.push_back(layout_item);

  sIterator.Reset();
  bCurrentItem = false;
  bContentArea = false;
  tabparam.ClearChildren();
  OrderContainer(&sIterator, nullptr, &tabparam, &bCurrentItem, &bContentArea,
                 true);
  for (const auto& layout_item : tabparam.GetChildren())
    items.push_back(layout_item);

  return items;
}
