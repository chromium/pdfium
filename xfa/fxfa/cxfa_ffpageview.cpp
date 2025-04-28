// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffpageview.h"

#include <algorithm>
#include <vector>

#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/stl_util.h"
#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cjx_object.h"
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
                      Mask<XFA_WidgetStatus> dwFilter,
                      bool bTraversal,
                      bool bIgnoreRelevant) {
  CXFA_Node* pNode = pWidget->GetNode();

  if ((dwFilter & XFA_WidgetStatus::kFocused) &&
      (!pNode || pNode->GetElementType() != XFA_Element::Field)) {
    return false;
  }

  CXFA_ContentLayoutItem* pItem = pWidget->GetLayoutItem();
  if (bTraversal && pItem->TestStatusBits(XFA_WidgetStatus::kDisabled)) {
    return false;
  }
  if (bIgnoreRelevant) {
    return pItem->TestStatusBits(XFA_WidgetStatus::kVisible);
  }

  dwFilter &= Mask<XFA_WidgetStatus>{XFA_WidgetStatus::kVisible,
                                     XFA_WidgetStatus::kViewable,
                                     XFA_WidgetStatus::kPrintable};
  return pItem->TestStatusBits(dwFilter);
}

bool IsLayoutElement(XFA_Element eElement) {
  switch (eElement) {
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
      pWidget->GetLayoutItem()->TestStatusBits(XFA_WidgetStatus::kVisible)) {
    if (!pWidget->LoadWidget()) {
      return false;
    }
  }
  return true;
}

CXFA_FFWidget* LoadedWidgetFromLayoutItem(CXFA_LayoutItem* pLayoutItem) {
  CXFA_FFWidget* pWidget = CXFA_FFWidget::FromLayoutItem(pLayoutItem);
  if (!pWidget) {
    return nullptr;
  }

  EnsureWidgetLoadedIfVisible(pWidget);
  return pWidget;
}

CXFA_FFWidget* FilteredLoadedWidgetFromLayoutItem(
    CXFA_LayoutItem* pLayoutItem,
    Mask<XFA_WidgetStatus> dwFilter,
    bool bIgnoreRelevant) {
  CXFA_FFWidget* pWidget = CXFA_FFWidget::FromLayoutItem(pLayoutItem);
  if (!pWidget) {
    return nullptr;
  }

  if (!PageWidgetFilter(pWidget, dwFilter, false, bIgnoreRelevant)) {
    return nullptr;
  }

  if (!EnsureWidgetLoadedIfVisible(pWidget)) {
    return nullptr;
  }

  return pWidget;
}

class CXFA_TabParam {
 public:
  CXFA_TabParam() = default;
  explicit CXFA_TabParam(CXFA_FFWidget* pWidget)
      : item_(pWidget->GetLayoutItem()) {}
  CXFA_TabParam(const CXFA_TabParam&) = delete;
  CXFA_TabParam(CXFA_TabParam&&) noexcept = default;
  ~CXFA_TabParam() = default;

  CXFA_TabParam& operator=(const CXFA_TabParam&) = delete;
  CXFA_TabParam& operator=(CXFA_TabParam&&) noexcept = default;

  CXFA_FFWidget* GetWidget() const { return item_->GetFFWidget(); }
  const std::vector<cppgc::Persistent<CXFA_ContentLayoutItem>>& GetChildren()
      const {
    return children_;
  }
  void ClearChildren() { children_.clear(); }
  void AppendTabParam(const CXFA_TabParam* pParam) {
    children_.push_back(pParam->item_);
    children_.insert(children_.end(), pParam->children_.begin(),
                     pParam->children_.end());
  }

 private:
  cppgc::Persistent<CXFA_ContentLayoutItem> item_;
  std::vector<cppgc::Persistent<CXFA_ContentLayoutItem>> children_;
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
      if (IsLayoutElement(pSearchItem->GetFormNode()->GetElementType())) {
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
              if (rt1.top - rt2.top >= kXFAWidgetPrecision) {
                return rt1.top < rt2.top;
              }
              return rt1.left < rt2.left;
            });
  for (const auto& param : tabParams) {
    pContainer->AppendTabParam(&param);
  }
}

}  // namespace

CXFA_FFPageView::CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea)
    : page_area_(pPageArea), doc_view_(pDocView) {}

CXFA_FFPageView::~CXFA_FFPageView() = default;

void CXFA_FFPageView::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(page_area_);
  visitor->Trace(doc_view_);
  visitor->Trace(layout_item_);
}

CXFA_FFDocView* CXFA_FFPageView::GetDocView() const {
  return doc_view_;
}

CFX_RectF CXFA_FFPageView::GetPageViewRect() const {
  auto* pItem = GetLayoutItem();
  if (!pItem) {
    return CFX_RectF();
  }

  return CFX_RectF(0, 0, pItem->GetPageSize());
}

CFX_Matrix CXFA_FFPageView::GetDisplayMatrix(const FX_RECT& rtDisp,
                                             int32_t iRotate) const {
  auto* pItem = GetLayoutItem();
  if (!pItem) {
    return CFX_Matrix();
  }

  return GetPageMatrix(CFX_RectF(0, 0, pItem->GetPageSize()), rtDisp, iRotate);
}

CXFA_FFWidget::IteratorIface* CXFA_FFPageView::CreateGCedTraverseWidgetIterator(
    Mask<XFA_WidgetStatus> dwWidgetFilter) {
  return cppgc::MakeGarbageCollected<CXFA_FFTabOrderPageWidgetIterator>(
      GetDocView()->GetDoc()->GetHeap()->GetAllocationHandle(), this,
      dwWidgetFilter);
}

CXFA_FFPageWidgetIterator::CXFA_FFPageWidgetIterator(
    CXFA_FFPageView* pPageView,
    Mask<XFA_WidgetStatus> dwFilter)
    : s_iterator_(pPageView->GetLayoutItem()),
      filter_(dwFilter),
      ignore_relevant_(IsDocVersionBelow205(GetDocForPageView(pPageView))) {}

CXFA_FFPageWidgetIterator::~CXFA_FFPageWidgetIterator() = default;

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToFirst() {
  s_iterator_.Reset();
  for (CXFA_LayoutItem* pLayoutItem = s_iterator_.GetCurrent(); pLayoutItem;
       pLayoutItem = s_iterator_.MoveToNext()) {
    CXFA_FFWidget* hWidget = FilteredLoadedWidgetFromLayoutItem(
        pLayoutItem, filter_, ignore_relevant_);
    if (hWidget) {
      return hWidget;
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToLast() {
  s_iterator_.SetCurrent(nullptr);
  return MoveToPrevious();
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToNext() {
  for (CXFA_LayoutItem* pLayoutItem = s_iterator_.MoveToNext(); pLayoutItem;
       pLayoutItem = s_iterator_.MoveToNext()) {
    CXFA_FFWidget* hWidget = FilteredLoadedWidgetFromLayoutItem(
        pLayoutItem, filter_, ignore_relevant_);
    if (hWidget) {
      return hWidget;
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToPrevious() {
  for (CXFA_LayoutItem* pLayoutItem = s_iterator_.MoveToPrev(); pLayoutItem;
       pLayoutItem = s_iterator_.MoveToPrev()) {
    CXFA_FFWidget* hWidget = FilteredLoadedWidgetFromLayoutItem(
        pLayoutItem, filter_, ignore_relevant_);
    if (hWidget) {
      return hWidget;
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::GetCurrentWidget() {
  CXFA_LayoutItem* pLayoutItem = s_iterator_.GetCurrent();
  return pLayoutItem ? CXFA_FFWidget::FromLayoutItem(pLayoutItem) : nullptr;
}

bool CXFA_FFPageWidgetIterator::SetCurrentWidget(CXFA_FFWidget* pWidget) {
  return pWidget && s_iterator_.SetCurrent(pWidget->GetLayoutItem());
}

CXFA_FFTabOrderPageWidgetIterator::CXFA_FFTabOrderPageWidgetIterator(
    CXFA_FFPageView* pPageView,
    Mask<XFA_WidgetStatus> dwFilter)
    : page_view_layout_(pPageView->GetLayoutItem()),
      filter_(dwFilter),
      ignore_relevant_(IsDocVersionBelow205(GetDocForPageView(pPageView))) {
  CreateTabOrderWidgetArray();
}

CXFA_FFTabOrderPageWidgetIterator::~CXFA_FFTabOrderPageWidgetIterator() =
    default;

void CXFA_FFTabOrderPageWidgetIterator::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(page_view_layout_);
  ContainerTrace(visitor, tab_order_widget_array_);
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToFirst() {
  for (int32_t i = 0;
       i < fxcrt::CollectionSize<int32_t>(tab_order_widget_array_); i++) {
    if (PageWidgetFilter(tab_order_widget_array_[i]->GetFFWidget(), filter_,
                         true, ignore_relevant_)) {
      cur_widget_ = i;
      return tab_order_widget_array_[cur_widget_]->GetFFWidget();
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToLast() {
  for (int32_t i = fxcrt::CollectionSize<int32_t>(tab_order_widget_array_) - 1;
       i >= 0; i--) {
    if (PageWidgetFilter(tab_order_widget_array_[i]->GetFFWidget(), filter_,
                         true, ignore_relevant_)) {
      cur_widget_ = i;
      return tab_order_widget_array_[cur_widget_]->GetFFWidget();
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToNext() {
  for (int32_t i = cur_widget_ + 1;
       i < fxcrt::CollectionSize<int32_t>(tab_order_widget_array_); i++) {
    if (PageWidgetFilter(tab_order_widget_array_[i]->GetFFWidget(), filter_,
                         true, ignore_relevant_)) {
      cur_widget_ = i;
      return tab_order_widget_array_[cur_widget_]->GetFFWidget();
    }
  }
  cur_widget_ = -1;
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToPrevious() {
  for (int32_t i = cur_widget_ - 1; i >= 0; i--) {
    if (PageWidgetFilter(tab_order_widget_array_[i]->GetFFWidget(), filter_,
                         true, ignore_relevant_)) {
      cur_widget_ = i;
      return tab_order_widget_array_[cur_widget_]->GetFFWidget();
    }
  }
  cur_widget_ = -1;
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetCurrentWidget() {
  return cur_widget_ >= 0 ? tab_order_widget_array_[cur_widget_]->GetFFWidget()
                          : nullptr;
}

bool CXFA_FFTabOrderPageWidgetIterator::SetCurrentWidget(
    CXFA_FFWidget* hWidget) {
  auto it =
      std::ranges::find(tab_order_widget_array_, hWidget->GetLayoutItem());
  if (it == tab_order_widget_array_.end()) {
    return false;
  }

  cur_widget_ =
      pdfium::checked_cast<int32_t>(it - tab_order_widget_array_.begin());
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
      std::optional<WideString> traverseWidgetName =
          pTraverse->JSObject()->TryAttribute(XFA_Attribute::Ref, true);
      if (traverseWidgetName.has_value()) {
        return FindWidgetByName(traverseWidgetName.value(), pWidget);
      }
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
  tab_order_widget_array_.clear();

  const std::vector<CXFA_ContentLayoutItem*> items =
      CreateSpaceOrderLayoutItems();
  if (items.empty()) {
    return;
  }

  CXFA_ContentLayoutItem* item = items[0];
  while (tab_order_widget_array_.size() < items.size()) {
    if (!pdfium::Contains(tab_order_widget_array_, item)) {
      tab_order_widget_array_.emplace_back(item);
      CXFA_Node* node = item->GetFFWidget()->GetNode();
      if (node->GetFFWidgetType() == XFA_FFWidgetType::kExclGroup) {
        auto it = std::ranges::find(items, item);
        size_t index = it != items.end() ? it - items.begin() + 1 : 0;
        while (true) {
          CXFA_FFWidget* radio = items[index % items.size()]->GetFFWidget();
          if (radio->GetNode()->GetExclGroupIfExists() != node) {
            break;
          }
          if (!pdfium::Contains(tab_order_widget_array_, item)) {
            tab_order_widget_array_.emplace_back(radio->GetLayoutItem());
          }
          ++index;
        }
      }
      CXFA_FFWidget* next_widget = GetTraverseWidget(item->GetFFWidget());
      if (next_widget) {
        item = next_widget->GetLayoutItem();
        continue;
      }
    }
    auto it = std::ranges::find(items, item);
    size_t index = it != items.end() ? it - items.begin() + 1 : 0;
    item = items[index % items.size()];
  }
}

std::vector<CXFA_ContentLayoutItem*>
CXFA_FFTabOrderPageWidgetIterator::CreateSpaceOrderLayoutItems() {
  std::vector<CXFA_ContentLayoutItem*> items;
  CXFA_LayoutItemIterator sIterator(page_view_layout_.Get());
  CXFA_TabParam tabparam;
  bool bCurrentItem = false;
  bool bContentArea = false;
  OrderContainer(&sIterator, nullptr, &tabparam, &bCurrentItem, &bContentArea,
                 false);
  items.reserve(tabparam.GetChildren().size());
  for (const auto& layout_item : tabparam.GetChildren()) {
    items.push_back(layout_item);
  }

  sIterator.Reset();
  bCurrentItem = false;
  bContentArea = false;
  tabparam.ClearChildren();
  OrderContainer(&sIterator, nullptr, &tabparam, &bCurrentItem, &bContentArea,
                 true);
  for (const auto& layout_item : tabparam.GetChildren()) {
    items.push_back(layout_item);
  }

  return items;
}
