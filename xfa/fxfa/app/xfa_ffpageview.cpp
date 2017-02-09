// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/xfa_ffpageview.h"

#include <memory>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/fde_render.h"
#include "xfa/fxfa/app/xfa_ffcheckbutton.h"
#include "xfa/fxfa/app/xfa_ffchoicelist.h"
#include "xfa/fxfa/app/xfa_fffield.h"
#include "xfa/fxfa/app/xfa_ffimageedit.h"
#include "xfa/fxfa/app/xfa_ffpushbutton.h"
#include "xfa/fxfa/app/xfa_fftextedit.h"
#include "xfa/fxfa/app/xfa_fwladapter.h"
#include "xfa/fxfa/xfa_ffdoc.h"
#include "xfa/fxfa/xfa_ffdocview.h"
#include "xfa/fxfa/xfa_ffwidget.h"

namespace {

CFX_Matrix GetPageMatrix(const CFX_RectF& docPageRect,
                         const CFX_Rect& devicePageRect,
                         int32_t iRotate,
                         uint32_t dwCoordinatesType) {
  ASSERT(iRotate >= 0 && iRotate <= 3);

  bool bFlipX = (dwCoordinatesType & 0x01) != 0;
  bool bFlipY = (dwCoordinatesType & 0x02) != 0;
  CFX_Matrix m((bFlipX ? -1.0f : 1.0f), 0, 0, (bFlipY ? -1.0f : 1.0f), 0, 0);
  if (iRotate == 0 || iRotate == 2) {
    m.a *= (FX_FLOAT)devicePageRect.width / docPageRect.width;
    m.d *= (FX_FLOAT)devicePageRect.height / docPageRect.height;
  } else {
    m.a *= (FX_FLOAT)devicePageRect.height / docPageRect.width;
    m.d *= (FX_FLOAT)devicePageRect.width / docPageRect.height;
  }
  m.Rotate(iRotate * 1.57079632675f);
  switch (iRotate) {
    case 0:
      m.e = bFlipX ? (FX_FLOAT)devicePageRect.right()
                   : (FX_FLOAT)devicePageRect.left;
      m.f = bFlipY ? (FX_FLOAT)devicePageRect.bottom()
                   : (FX_FLOAT)devicePageRect.top;
      break;
    case 1:
      m.e = bFlipY ? (FX_FLOAT)devicePageRect.left
                   : (FX_FLOAT)devicePageRect.right();
      m.f = bFlipX ? (FX_FLOAT)devicePageRect.bottom()
                   : (FX_FLOAT)devicePageRect.top;
      break;
    case 2:
      m.e = bFlipX ? (FX_FLOAT)devicePageRect.left
                   : (FX_FLOAT)devicePageRect.right();
      m.f = bFlipY ? (FX_FLOAT)devicePageRect.top
                   : (FX_FLOAT)devicePageRect.bottom();
      break;
    case 3:
      m.e = bFlipY ? (FX_FLOAT)devicePageRect.right()
                   : (FX_FLOAT)devicePageRect.left;
      m.f = bFlipX ? (FX_FLOAT)devicePageRect.top
                   : (FX_FLOAT)devicePageRect.bottom();
      break;
    default:
      break;
  }
  return m;
}

bool PageWidgetFilter(CXFA_FFWidget* pWidget,
                      uint32_t dwFilter,
                      bool bTraversal,
                      bool bIgnorerelevant) {
  CXFA_WidgetAcc* pWidgetAcc = pWidget->GetDataAcc();

  if (!!(dwFilter & XFA_WidgetStatus_Focused) &&
      pWidgetAcc->GetElementType() != XFA_Element::Field) {
    return false;
  }

  uint32_t dwStatus = pWidget->GetStatus();
  if (bTraversal && (dwStatus & XFA_WidgetStatus_Disabled))
    return false;
  if (bIgnorerelevant)
    return !!(dwStatus & XFA_WidgetStatus_Visible);

  dwFilter &= (XFA_WidgetStatus_Visible | XFA_WidgetStatus_Viewable |
               XFA_WidgetStatus_Printable);
  return (dwFilter & dwStatus) == dwFilter;
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

}  // namespace

CXFA_FFPageView::CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea)
    : CXFA_ContainerLayoutItem(pPageArea), m_pDocView(pDocView) {}

CXFA_FFPageView::~CXFA_FFPageView() {}

CXFA_FFDocView* CXFA_FFPageView::GetDocView() const {
  return m_pDocView;
}

CFX_RectF CXFA_FFPageView::GetPageViewRect() const {
  return CFX_RectF(0, 0, GetPageSize());
}

CFX_Matrix CXFA_FFPageView::GetDisplayMatrix(const CFX_Rect& rtDisp,
                                             int32_t iRotate) const {
  return GetPageMatrix(CFX_RectF(0, 0, GetPageSize()), rtDisp, iRotate, 0);
}

IXFA_WidgetIterator* CXFA_FFPageView::CreateWidgetIterator(
    uint32_t dwTraverseWay,
    uint32_t dwWidgetFilter) {
  switch (dwTraverseWay) {
    case XFA_TRAVERSEWAY_Tranvalse:
      return new CXFA_FFTabOrderPageWidgetIterator(this, dwWidgetFilter);
    case XFA_TRAVERSEWAY_Form:
      return new CXFA_FFPageWidgetIterator(this, dwWidgetFilter);
  }
  return nullptr;
}

CXFA_FFPageWidgetIterator::CXFA_FFPageWidgetIterator(CXFA_FFPageView* pPageView,
                                                     uint32_t dwFilter) {
  m_pPageView = pPageView;
  m_dwFilter = dwFilter;
  m_sIterator.Init(pPageView);
  m_bIgnorerelevant =
      m_pPageView->GetDocView()->GetDoc()->GetXFADoc()->GetCurVersionMode() <
      XFA_VERSION_205;
}
CXFA_FFPageWidgetIterator::~CXFA_FFPageWidgetIterator() {}
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
bool CXFA_FFPageWidgetIterator::SetCurrentWidget(CXFA_FFWidget* hWidget) {
  return hWidget && m_sIterator.SetCurrent(hWidget);
}
CXFA_FFWidget* CXFA_FFPageWidgetIterator::GetWidget(
    CXFA_LayoutItem* pLayoutItem) {
  if (CXFA_FFWidget* pWidget = XFA_GetWidgetFromLayoutItem(pLayoutItem)) {
    if (!PageWidgetFilter(pWidget, m_dwFilter, false, m_bIgnorerelevant)) {
      return nullptr;
    }
    if (!pWidget->IsLoaded() &&
        (pWidget->GetStatus() & XFA_WidgetStatus_Visible) != 0) {
      pWidget->LoadWidget();
    }
    return pWidget;
  }
  return nullptr;
}

CXFA_FFTabOrderPageWidgetIterator::CXFA_FFTabOrderPageWidgetIterator(
    CXFA_FFPageView* pPageView,
    uint32_t dwFilter)
    : m_pPageView(pPageView), m_dwFilter(dwFilter), m_iCurWidget(-1) {
  m_bIgnorerelevant =
      m_pPageView->GetDocView()->GetDoc()->GetXFADoc()->GetCurVersionMode() <
      XFA_VERSION_205;
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
    if (PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, true,
                         m_bIgnorerelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget];
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToLast() {
  for (int32_t i = pdfium::CollectionSize<int32_t>(m_TabOrderWidgetArray) - 1;
       i >= 0; i--) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, true,
                         m_bIgnorerelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget];
    }
  }
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToNext() {
  for (int32_t i = m_iCurWidget + 1;
       i < pdfium::CollectionSize<int32_t>(m_TabOrderWidgetArray); i++) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, true,
                         m_bIgnorerelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget];
    }
  }
  m_iCurWidget = -1;
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToPrevious() {
  for (int32_t i = m_iCurWidget - 1; i >= 0; i--) {
    if (PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, true,
                         m_bIgnorerelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget];
    }
  }
  m_iCurWidget = -1;
  return nullptr;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetCurrentWidget() {
  return m_iCurWidget >= 0 ? m_TabOrderWidgetArray[m_iCurWidget] : nullptr;
}

bool CXFA_FFTabOrderPageWidgetIterator::SetCurrentWidget(
    CXFA_FFWidget* hWidget) {
  auto it = std::find(m_TabOrderWidgetArray.begin(),
                      m_TabOrderWidgetArray.end(), hWidget);
  if (it == m_TabOrderWidgetArray.end())
    return false;

  m_iCurWidget = it - m_TabOrderWidgetArray.begin();
  return true;
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetTraverseWidget(
    CXFA_FFWidget* pWidget) {
  CXFA_WidgetAcc* pAcc = pWidget->GetDataAcc();
  CXFA_Node* pTraversal = pAcc->GetNode()->GetChild(0, XFA_Element::Traversal);
  if (pTraversal) {
    CXFA_Node* pTraverse = pTraversal->GetChild(0, XFA_Element::Traverse);
    if (pTraverse) {
      CFX_WideString wsTraverseWidgetName;
      if (pTraverse->GetAttribute(XFA_ATTRIBUTE_Ref, wsTraverseWidgetName)) {
        return FindWidgetByName(wsTraverseWidgetName, pWidget);
      }
    }
  }
  return nullptr;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::FindWidgetByName(
    const CFX_WideString& wsWidgetName,
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
    if (!pdfium::ContainsValue(m_TabOrderWidgetArray, hWidget)) {
      m_TabOrderWidgetArray.push_back(hWidget);
      CXFA_WidgetAcc* pWidgetAcc = hWidget->GetDataAcc();
      if (pWidgetAcc->GetUIType() == XFA_Element::ExclGroup) {
        auto it = std::find(SpaceOrderWidgetArray.begin(),
                            SpaceOrderWidgetArray.end(), hWidget);
        int32_t iWidgetIndex = it != SpaceOrderWidgetArray.end()
                                   ? it - SpaceOrderWidgetArray.begin() + 1
                                   : 0;
        while (true) {
          CXFA_FFWidget* pRadio =
              SpaceOrderWidgetArray[iWidgetIndex % nWidgetCount];
          if (pRadio->GetDataAcc()->GetExclGroup() != pWidgetAcc) {
            break;
          }
          if (!pdfium::ContainsValue(m_TabOrderWidgetArray, hWidget)) {
            m_TabOrderWidgetArray.push_back(pRadio);
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

static int32_t XFA_TabOrderWidgetComparator(const void* phWidget1,
                                            const void* phWidget2) {
  auto param1 = *static_cast<CXFA_TabParam**>(const_cast<void*>(phWidget1));
  auto param2 = *static_cast<CXFA_TabParam**>(const_cast<void*>(phWidget2));
  CFX_RectF rt1 = param1->m_pWidget->GetWidgetRect();
  CFX_RectF rt2 = param2->m_pWidget->GetWidgetRect();
  FX_FLOAT x1 = rt1.left, y1 = rt1.top, x2 = rt2.left, y2 = rt2.top;
  if (y1 < y2 || (y1 - y2 < XFA_FLOAT_PERCISION && x1 < x2))
    return -1;
  return 1;
}

void CXFA_FFTabOrderPageWidgetIterator::OrderContainer(
    CXFA_LayoutItemIterator* sIterator,
    CXFA_LayoutItem* pContainerItem,
    CXFA_TabParam* pContainer,
    bool& bCurrentItem,
    bool& bContentArea,
    bool bMarsterPage) {
  CFX_ArrayTemplate<CXFA_TabParam*> tabParams;
  CXFA_LayoutItem* pSearchItem = sIterator->MoveToNext();
  while (pSearchItem) {
    if (!pSearchItem->IsContentLayoutItem()) {
      bContentArea = true;
      pSearchItem = sIterator->MoveToNext();
      continue;
    }
    if (bMarsterPage && bContentArea) {
      break;
    }
    if (bMarsterPage || bContentArea) {
      CXFA_FFWidget* hWidget = GetWidget(pSearchItem);
      if (!hWidget) {
        pSearchItem = sIterator->MoveToNext();
        continue;
      }
      if (pContainerItem && (pSearchItem->GetParent() != pContainerItem)) {
        bCurrentItem = true;
        break;
      }
      CXFA_TabParam* pParam = new CXFA_TabParam;
      pParam->m_pWidget = hWidget;
      tabParams.Add(pParam);
      if (IsLayoutElement(pSearchItem->GetFormNode()->GetElementType(), true)) {
        OrderContainer(sIterator, pSearchItem, pParam, bCurrentItem,
                       bContentArea, bMarsterPage);
      }
    }
    if (bCurrentItem) {
      pSearchItem = sIterator->GetCurrent();
      bCurrentItem = false;
    } else {
      pSearchItem = sIterator->MoveToNext();
    }
  }
  int32_t iChildren = tabParams.GetSize();
  if (iChildren > 1) {
    FXSYS_qsort(tabParams.GetData(), iChildren, sizeof(void*),
                XFA_TabOrderWidgetComparator);
  }
  for (int32_t iStart = 0; iStart < iChildren; iStart++) {
    std::unique_ptr<CXFA_TabParam> pParam(tabParams[iStart]);
    pContainer->m_Children.push_back(pParam->m_pWidget);
    pContainer->m_Children.insert(pContainer->m_Children.end(),
                                  pParam->m_Children.begin(),
                                  pParam->m_Children.end());
  }
  tabParams.RemoveAll();
}
void CXFA_FFTabOrderPageWidgetIterator::CreateSpaceOrderWidgetArray(
    std::vector<CXFA_FFWidget*>* WidgetArray) {
  CXFA_LayoutItemIterator sIterator;
  sIterator.Init(m_pPageView);
  auto pParam = pdfium::MakeUnique<CXFA_TabParam>();
  bool bCurrentItem = false;
  bool bContentArea = false;
  OrderContainer(&sIterator, nullptr, pParam.get(), bCurrentItem, bContentArea);
  WidgetArray->insert(WidgetArray->end(), pParam->m_Children.begin(),
                      pParam->m_Children.end());

  sIterator.Reset();
  bCurrentItem = false;
  bContentArea = false;
  pParam->m_Children.clear();
  OrderContainer(&sIterator, nullptr, pParam.get(), bCurrentItem, bContentArea,
                 true);
  WidgetArray->insert(WidgetArray->end(), pParam->m_Children.begin(),
                      pParam->m_Children.end());
}

CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetWidget(
    CXFA_LayoutItem* pLayoutItem) {
  if (CXFA_FFWidget* pWidget = XFA_GetWidgetFromLayoutItem(pLayoutItem)) {
    if (!pWidget->IsLoaded() &&
        (pWidget->GetStatus() & XFA_WidgetStatus_Visible)) {
      pWidget->LoadWidget();
    }
    return pWidget;
  }
  return nullptr;
}

CXFA_TabParam::CXFA_TabParam() : m_pWidget(nullptr) {}

CXFA_TabParam::~CXFA_TabParam() {}
