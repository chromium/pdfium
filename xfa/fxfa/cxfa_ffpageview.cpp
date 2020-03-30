// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffpageview.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
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
  ASSERT(iRotate >= 0);
  ASSERT(iRotate <= 3);

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
                      uint32_t dwFilter,
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

}  // namespace

CXFA_FFPageView::CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea)
    : m_pPageArea(pPageArea), m_pDocView(pDocView) {}

CXFA_FFPageView::~CXFA_FFPageView() {}

CXFA_FFDocView* CXFA_FFPageView::GetDocView() const {
  return m_pDocView.Get();
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

std::unique_ptr<IXFA_WidgetIterator> CXFA_FFPageView::CreateFormWidgetIterator(
    uint32_t dwWidgetFilter) {
  return pdfium::MakeUnique<CXFA_FFPageWidgetIterator>(this, dwWidgetFilter);
}

std::unique_ptr<IXFA_WidgetIterator>
CXFA_FFPageView::CreateTraverseWidgetIterator(uint32_t dwWidgetFilter) {
  return pdfium::MakeUnique<CXFA_FFTabOrderPageWidgetIterator>(this,
                                                               dwWidgetFilter);
}

CXFA_FFPageWidgetIterator::CXFA_FFPageWidgetIterator(CXFA_FFPageView* pPageView,
                                                     uint32_t dwFilter)
    : m_sIterator(pPageView->GetLayoutItem()),
      m_pPageView(pPageView),
      m_dwFilter(dwFilter),
      m_bIgnoreRelevant(IsDocVersionBelow205(GetDocForPageView(pPageView))) {}

CXFA_FFPageWidgetIterator::~CXFA_FFPageWidgetIterator() = default;

void CXFA_FFPageWidgetIterator::Reset() {
  m_sIterator.Reset();
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToFirst() {
  m_sIterator.Reset();
  for (CXFA_LayoutItem* pLayoutItem = m_sIterator.GetCurrent(); pLayoutItem;
       pLayoutItem = m_sIterator.MoveToNext()) {
    if (CXFA_FFWidget* hWidget = GetWidget(pLayoutItem)) {
      return hWidget;
    }
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
    if (CXFA_FFWidget* hWidget = GetWidget(pLayoutItem)) {
      return hWidget;
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToPrevious() {
  for (CXFA_LayoutItem* pLayoutItem = m_sIterator.MoveToPrev(); pLayoutItem;
       pLayoutItem = m_sIterator.MoveToPrev()) {
    if (CXFA_FFWidget* hWidget = GetWidget(pLayoutItem)) {
      return hWidget;
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::GetCurrentWidget() {
  CXFA_LayoutItem* pLayoutItem = m_sIterator.GetCurrent();
  return pLayoutItem ? XFA_GetWidgetFromLayoutItem(pLayoutItem) : nullptr;
}

bool CXFA_FFPageWidgetIterator::SetCurrentWidget(CXFA_FFWidget* pWidget) {
  return pWidget && m_sIterator.SetCurrent(pWidget->GetLayoutItem());
}

CXFA_FFWidget* CXFA_FFPageWidgetIterator::GetWidget(
    CXFA_LayoutItem* pLayoutItem) {
  CXFA_FFWidget* pWidget = XFA_GetWidgetFromLayoutItem(pLayoutItem);
  if (!pWidget)
    return nullptr;

  if (!PageWidgetFilter(pWidget, m_dwFilter, false, m_bIgnoreRelevant))
    return nullptr;

  if (!pWidget->IsLoaded() &&
      pWidget->GetLayoutItem()->TestStatusBits(XFA_WidgetStatus_Visible)) {
    if (!pWidget->LoadWidget())
      return nullptr;
  }
  return pWidget;
}

void CXFA_TabParam::AppendTabParam(CXFA_TabParam* pParam) {
  m_Children.push_back(pParam->GetWidget());
  m_Children.insert(m_Children.end(), pParam->GetChildren().begin(),
                    pParam->GetChildren().end());
}

void CXFA_TabParam::ClearChildren() {
  m_Children.clear();
}

CXFA_FFTabOrderPageWidgetIterator::CXFA_FFTabOrderPageWidgetIterator(
    CXFA_FFPageView* pPageView,
    uint32_t dwFilter)
    : m_pPageView(pPageView),
      m_dwFilter(dwFilter),
      m_bIgnoreRelevant(IsDocVersionBelow205(GetDocForPageView(pPageView))) {
  Reset();
}

CXFA_FFTabOrderPageWidgetIterator::~CXFA_FFTabOrderPageWidgetIterator() {}

void CXFA_FFTabOrderPageWidgetIterator::Reset() {
  CreateTabOrderWidgetArray();
  m_iCurWidget = -1;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToFirst() {
  for (int32_t i = 0;
       i < pdfium::CollectionSize<int32_t>(m_TabOrderWidgetArray); i++) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i]->GetFFWidget(), m_dwFilter,
                         true, m_bIgnoreRelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget]->GetFFWidget();
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToLast() {
  for (int32_t i = pdfium::CollectionSize<int32_t>(m_TabOrderWidgetArray) - 1;
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
       i < pdfium::CollectionSize<int32_t>(m_TabOrderWidgetArray); i++) {
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
      if (traverseWidgetName)
        return FindWidgetByName(*traverseWidgetName, pWidget);
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

  std::vector<CXFA_FFWidget*> SpaceOrderWidgetArray;
  CreateSpaceOrderWidgetArray(&SpaceOrderWidgetArray);
  if (SpaceOrderWidgetArray.empty())
    return;

  int32_t nWidgetCount = pdfium::CollectionSize<int32_t>(SpaceOrderWidgetArray);
  CXFA_FFWidget* hWidget = SpaceOrderWidgetArray[0];
  while (pdfium::CollectionSize<int32_t>(m_TabOrderWidgetArray) <
         nWidgetCount) {
    if (!pdfium::ContainsValue(m_TabOrderWidgetArray,
                               hWidget->GetLayoutItem())) {
      m_TabOrderWidgetArray.emplace_back(hWidget->GetLayoutItem());
      CXFA_Node* pNode = hWidget->GetNode();
      if (pNode->GetFFWidgetType() == XFA_FFWidgetType::kExclGroup) {
        auto it = std::find(SpaceOrderWidgetArray.begin(),
                            SpaceOrderWidgetArray.end(), hWidget);
        int32_t iWidgetIndex = it != SpaceOrderWidgetArray.end()
                                   ? it - SpaceOrderWidgetArray.begin() + 1
                                   : 0;
        while (true) {
          CXFA_FFWidget* radio =
              SpaceOrderWidgetArray[iWidgetIndex % nWidgetCount];
          if (radio->GetNode()->GetExclGroupIfExists() != pNode)
            break;
          if (!pdfium::ContainsValue(m_TabOrderWidgetArray,
                                     hWidget->GetLayoutItem())) {
            m_TabOrderWidgetArray.emplace_back(radio->GetLayoutItem());
          }
          iWidgetIndex++;
        }
      }
      if (CXFA_FFWidget* hTraverseWidget = GetTraverseWidget(hWidget)) {
        hWidget = hTraverseWidget;
        continue;
      }
    }
    auto it = std::find(SpaceOrderWidgetArray.begin(),
                        SpaceOrderWidgetArray.end(), hWidget);
    int32_t iWidgetIndex = it != SpaceOrderWidgetArray.end()
                               ? it - SpaceOrderWidgetArray.begin() + 1
                               : 0;
    hWidget = SpaceOrderWidgetArray[iWidgetIndex % nWidgetCount];
  }
}

void CXFA_FFTabOrderPageWidgetIterator::OrderContainer(
    CXFA_LayoutItemIterator* sIterator,
    CXFA_LayoutItem* pViewItem,
    CXFA_TabParam* pContainer,
    bool* bCurrentItem,
    bool* bContentArea,
    bool bMasterPage) {
  std::vector<std::unique_ptr<CXFA_TabParam>> tabParams;
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
      CXFA_FFWidget* hWidget = GetWidget(pSearchItem);
      if (!hWidget) {
        pSearchItem = sIterator->MoveToNext();
        continue;
      }
      if (pViewItem && (pSearchItem->GetParent() != pViewItem)) {
        *bCurrentItem = true;
        break;
      }
      tabParams.push_back(pdfium::MakeUnique<CXFA_TabParam>(hWidget));
      if (IsLayoutElement(pSearchItem->GetFormNode()->GetElementType(), true)) {
        OrderContainer(sIterator, pSearchItem, tabParams.back().get(),
                       bCurrentItem, bContentArea, bMasterPage);
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
            [](const std::unique_ptr<CXFA_TabParam>& arg1,
               const std::unique_ptr<CXFA_TabParam>& arg2) {
              const CFX_RectF& rt1 = arg1->GetWidget()->GetWidgetRect();
              const CFX_RectF& rt2 = arg2->GetWidget()->GetWidgetRect();
              if (rt1.top - rt2.top >= kXFAWidgetPrecision)
                return rt1.top < rt2.top;
              return rt1.left < rt2.left;
            });
  for (const auto& pParam : tabParams)
    pContainer->AppendTabParam(pParam.get());
}

void CXFA_FFTabOrderPageWidgetIterator::CreateSpaceOrderWidgetArray(
    std::vector<CXFA_FFWidget*>* WidgetArray) {
  CXFA_LayoutItemIterator sIterator(m_pPageView->GetLayoutItem());
  auto pParam = pdfium::MakeUnique<CXFA_TabParam>(nullptr);
  bool bCurrentItem = false;
  bool bContentArea = false;
  OrderContainer(&sIterator, nullptr, pParam.get(), &bCurrentItem,
                 &bContentArea, false);
  WidgetArray->insert(WidgetArray->end(), pParam->GetChildren().begin(),
                      pParam->GetChildren().end());

  sIterator.Reset();
  bCurrentItem = false;
  bContentArea = false;
  pParam->ClearChildren();
  OrderContainer(&sIterator, nullptr, pParam.get(), &bCurrentItem,
                 &bContentArea, true);
  WidgetArray->insert(WidgetArray->end(), pParam->GetChildren().begin(),
                      pParam->GetChildren().end());
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetWidget(
    CXFA_LayoutItem* pLayoutItem) {
  CXFA_FFWidget* pWidget = XFA_GetWidgetFromLayoutItem(pLayoutItem);
  if (!pWidget)
    return nullptr;

  if (!pWidget->IsLoaded() &&
      pWidget->GetLayoutItem()->TestStatusBits(XFA_WidgetStatus_Visible)) {
    pWidget->LoadWidget();
  }
  return pWidget;
}

CXFA_TabParam::CXFA_TabParam(CXFA_FFWidget* pWidget) : m_pWidget(pWidget) {}

CXFA_TabParam::~CXFA_TabParam() {}
