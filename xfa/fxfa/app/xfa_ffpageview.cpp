// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/include/fxfa/xfa_ffpageview.h"

#include "xfa/fde/fde_render.h"
#include "xfa/fxfa/app/xfa_ffcheckbutton.h"
#include "xfa/fxfa/app/xfa_ffchoicelist.h"
#include "xfa/fxfa/app/xfa_fffield.h"
#include "xfa/fxfa/app/xfa_ffimageedit.h"
#include "xfa/fxfa/app/xfa_ffpushbutton.h"
#include "xfa/fxfa/app/xfa_fftextedit.h"
#include "xfa/fxfa/app/xfa_fwladapter.h"
#include "xfa/include/fxfa/xfa_ffdoc.h"
#include "xfa/include/fxfa/xfa_ffdocview.h"
#include "xfa/include/fxfa/xfa_ffwidget.h"

CXFA_FFPageView::CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea)
    : CXFA_ContainerLayoutItem(pPageArea),
      m_pDocView(pDocView),
      m_bLoaded(FALSE) {}
CXFA_FFPageView::~CXFA_FFPageView() {}
CXFA_FFDocView* CXFA_FFPageView::GetDocView() {
  return m_pDocView;
}
int32_t CXFA_FFPageView::GetPageViewIndex() {
  return GetPageIndex();
}
void CXFA_FFPageView::GetPageViewRect(CFX_RectF& rtPage) {
  CFX_SizeF sz;
  GetPageSize(sz);
  rtPage.Set(0, 0, sz);
}
void CXFA_FFPageView::GetDisplayMatrix(CFX_Matrix& mt,
                                       const CFX_Rect& rtDisp,
                                       int32_t iRotate) {
  CFX_SizeF sz;
  GetPageSize(sz);
  CFX_RectF fdePage;
  fdePage.Set(0, 0, sz.x, sz.y);
  FDE_GetPageMatrix(mt, fdePage, rtDisp, iRotate, 0);
}
int32_t CXFA_FFPageView::LoadPageView(IFX_Pause* pPause) {
  if (m_bLoaded) {
    return 100;
  }
  m_bLoaded = TRUE;
  return 100;
}
void CXFA_FFPageView::UnloadPageView() {
  if (!m_bLoaded) {
    return;
  }
}
FX_BOOL CXFA_FFPageView::IsPageViewLoaded() {
  return m_bLoaded;
}
CXFA_FFWidget* CXFA_FFPageView::GetWidgetByPos(FX_FLOAT fx, FX_FLOAT fy) {
  if (!m_bLoaded) {
    return nullptr;
  }
  IXFA_WidgetIterator* pIterator = CreateWidgetIterator();
  CXFA_FFWidget* pWidget = nullptr;
  while ((pWidget = static_cast<CXFA_FFWidget*>(pIterator->MoveToNext()))) {
    if (!(pWidget->GetStatus() & XFA_WIDGETSTATUS_Visible)) {
      continue;
    }
    CXFA_WidgetAcc* pAcc = pWidget->GetDataAcc();
    int32_t type = pAcc->GetClassID();
    if (type != XFA_ELEMENT_Field && type != XFA_ELEMENT_Draw) {
      continue;
    }
    FX_FLOAT fWidgetx = fx;
    FX_FLOAT fWidgety = fy;
    pWidget->Rotate2Normal(fWidgetx, fWidgety);
    uint32_t dwFlag = pWidget->OnHitTest(fWidgetx, fWidgety);
    if ((FWL_WGTHITTEST_Client == dwFlag ||
         FWL_WGTHITTEST_HyperLink == dwFlag)) {
      break;
    }
  }
  pIterator->Release();
  return pWidget;
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
  return NULL;
}
static FX_BOOL XFA_PageWidgetFilter(CXFA_FFWidget* pWidget,
                                    uint32_t dwFilter,
                                    FX_BOOL bTraversal,
                                    FX_BOOL bIgnorerelevant) {
  CXFA_WidgetAcc* pWidgetAcc = pWidget->GetDataAcc();
  uint32_t dwType = dwFilter & XFA_WIDGETFILTER_AllType;
  if ((dwType == XFA_WIDGETFILTER_Field) &&
      (pWidgetAcc->GetClassID() != XFA_ELEMENT_Field)) {
    return FALSE;
  }
  uint32_t dwStatus = pWidget->GetStatus();
  if (bTraversal && (dwStatus & XFA_WIDGETSTATUS_Disabled)) {
    return FALSE;
  }
  if (bIgnorerelevant) {
    return (dwStatus & XFA_WIDGETFILTER_Visible) != 0;
  }
  dwFilter &= (XFA_WIDGETFILTER_Visible | XFA_WIDGETFILTER_Viewable |
               XFA_WIDGETFILTER_Printable);
  return (dwFilter & dwStatus) == dwFilter;
}
CXFA_FFPageWidgetIterator::CXFA_FFPageWidgetIterator(CXFA_FFPageView* pPageView,
                                                     uint32_t dwFilter) {
  m_pPageView = pPageView;
  m_dwFilter = dwFilter;
  m_sIterator.Init(pPageView);
  m_bIgnorerelevant = ((CXFA_FFDoc*)m_pPageView->GetDocView()->GetDoc())
                          ->GetXFADoc()
                          ->GetCurVersionMode() < XFA_VERSION_205;
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
  return NULL;
}
CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToLast() {
  m_sIterator.SetCurrent(NULL);
  return MoveToPrevious();
}
CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToNext() {
  for (CXFA_LayoutItem* pLayoutItem = m_sIterator.MoveToNext(); pLayoutItem;
       pLayoutItem = m_sIterator.MoveToNext()) {
    if (CXFA_FFWidget* hWidget = GetWidget(pLayoutItem)) {
      return hWidget;
    }
  }
  return NULL;
}
CXFA_FFWidget* CXFA_FFPageWidgetIterator::MoveToPrevious() {
  for (CXFA_LayoutItem* pLayoutItem = m_sIterator.MoveToPrev(); pLayoutItem;
       pLayoutItem = m_sIterator.MoveToPrev()) {
    if (CXFA_FFWidget* hWidget = GetWidget(pLayoutItem)) {
      return hWidget;
    }
  }
  return NULL;
}
CXFA_FFWidget* CXFA_FFPageWidgetIterator::GetCurrentWidget() {
  CXFA_LayoutItem* pLayoutItem = m_sIterator.GetCurrent();
  return pLayoutItem ? XFA_GetWidgetFromLayoutItem(pLayoutItem) : NULL;
}
FX_BOOL CXFA_FFPageWidgetIterator::SetCurrentWidget(CXFA_FFWidget* hWidget) {
  return hWidget && m_sIterator.SetCurrent(hWidget);
}
CXFA_FFWidget* CXFA_FFPageWidgetIterator::GetWidget(
    CXFA_LayoutItem* pLayoutItem) {
  if (CXFA_FFWidget* pWidget = XFA_GetWidgetFromLayoutItem(pLayoutItem)) {
    if (!XFA_PageWidgetFilter(pWidget, m_dwFilter, FALSE, m_bIgnorerelevant)) {
      return NULL;
    }
    if (!pWidget->IsLoaded() &&
        (pWidget->GetStatus() & XFA_WIDGETSTATUS_Visible) != 0) {
      pWidget->LoadWidget();
    }
    return pWidget;
  }
  return NULL;
}
CXFA_FFTabOrderPageWidgetIterator::CXFA_FFTabOrderPageWidgetIterator(
    CXFA_FFPageView* pPageView,
    uint32_t dwFilter)
    : m_pPageView(pPageView), m_dwFilter(dwFilter), m_iCurWidget(-1) {
  m_bIgnorerelevant = ((CXFA_FFDoc*)m_pPageView->GetDocView()->GetDoc())
                          ->GetXFADoc()
                          ->GetCurVersionMode() < XFA_VERSION_205;
  Reset();
}
CXFA_FFTabOrderPageWidgetIterator::~CXFA_FFTabOrderPageWidgetIterator() {}
void CXFA_FFTabOrderPageWidgetIterator::Release() {
  delete this;
}
void CXFA_FFTabOrderPageWidgetIterator::Reset() {
  CreateTabOrderWidgetArray();
  m_iCurWidget = -1;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToFirst() {
  if (m_TabOrderWidgetArray.GetSize() > 0) {
    for (int32_t i = 0; i < m_TabOrderWidgetArray.GetSize(); i++) {
      if (XFA_PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, TRUE,
                               m_bIgnorerelevant)) {
        m_iCurWidget = i;
        return m_TabOrderWidgetArray[m_iCurWidget];
      }
    }
  }
  return NULL;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToLast() {
  if (m_TabOrderWidgetArray.GetSize() > 0) {
    for (int32_t i = m_TabOrderWidgetArray.GetSize() - 1; i >= 0; i--) {
      if (XFA_PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, TRUE,
                               m_bIgnorerelevant)) {
        m_iCurWidget = i;
        return m_TabOrderWidgetArray[m_iCurWidget];
      }
    }
  }
  return NULL;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToNext() {
  for (int32_t i = m_iCurWidget + 1; i < m_TabOrderWidgetArray.GetSize(); i++) {
    if (XFA_PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, TRUE,
                             m_bIgnorerelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget];
    }
  }
  m_iCurWidget = -1;
  return NULL;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::MoveToPrevious() {
  for (int32_t i = m_iCurWidget - 1; i >= 0; i--) {
    if (XFA_PageWidgetFilter(m_TabOrderWidgetArray[i], m_dwFilter, TRUE,
                             m_bIgnorerelevant)) {
      m_iCurWidget = i;
      return m_TabOrderWidgetArray[m_iCurWidget];
    }
  }
  m_iCurWidget = -1;
  return NULL;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetCurrentWidget() {
  if (m_iCurWidget >= 0) {
    return m_TabOrderWidgetArray[m_iCurWidget];
  }
  return NULL;
}
FX_BOOL CXFA_FFTabOrderPageWidgetIterator::SetCurrentWidget(
    CXFA_FFWidget* hWidget) {
  int32_t iWidgetIndex = m_TabOrderWidgetArray.Find(hWidget);
  if (iWidgetIndex >= 0) {
    m_iCurWidget = iWidgetIndex;
    return TRUE;
  }
  return FALSE;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetTraverseWidget(
    CXFA_FFWidget* pWidget) {
  CXFA_WidgetAcc* pAcc = pWidget->GetDataAcc();
  CXFA_Node* pTraversal = pAcc->GetNode()->GetChild(0, XFA_ELEMENT_Traversal);
  if (pTraversal) {
    CXFA_Node* pTraverse = pTraversal->GetChild(0, XFA_ELEMENT_Traverse);
    if (pTraverse) {
      CFX_WideString wsTraverseWidgetName;
      if (pTraverse->GetAttribute(XFA_ATTRIBUTE_Ref, wsTraverseWidgetName)) {
        return FindWidgetByName(wsTraverseWidgetName.AsWideStringC(), pWidget);
      }
    }
  }
  return NULL;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::FindWidgetByName(
    const CFX_WideStringC& wsWidgetName,
    CXFA_FFWidget* pRefWidget) {
  return pRefWidget->GetDocView()->GetWidgetByName(wsWidgetName, pRefWidget);
}
void CXFA_FFTabOrderPageWidgetIterator::CreateTabOrderWidgetArray() {
  m_TabOrderWidgetArray.RemoveAll();
  CXFA_WidgetArray SpaceOrderWidgetArray;
  CreateSpaceOrderWidgetArray(SpaceOrderWidgetArray);
  int32_t nWidgetCount = SpaceOrderWidgetArray.GetSize();
  if (nWidgetCount < 1) {
    return;
  }
  CXFA_FFWidget* hWidget = SpaceOrderWidgetArray[0];
  for (; m_TabOrderWidgetArray.GetSize() < nWidgetCount;) {
    if (m_TabOrderWidgetArray.Find(hWidget) < 0) {
      m_TabOrderWidgetArray.Add(hWidget);
      CXFA_WidgetAcc* pWidgetAcc = hWidget->GetDataAcc();
      if (pWidgetAcc->GetUIType() == XFA_ELEMENT_ExclGroup) {
        int32_t iWidgetIndex = SpaceOrderWidgetArray.Find(hWidget) + 1;
        while (TRUE) {
          CXFA_FFWidget* pRadio =
              SpaceOrderWidgetArray[(iWidgetIndex) % nWidgetCount];
          if (pRadio->GetDataAcc()->GetExclGroup() != pWidgetAcc) {
            break;
          }
          if (m_TabOrderWidgetArray.Find(hWidget) < 0) {
            m_TabOrderWidgetArray.Add(pRadio);
          }
          iWidgetIndex++;
        }
      }
      if (CXFA_FFWidget* hTraverseWidget = GetTraverseWidget(hWidget)) {
        hWidget = hTraverseWidget;
        continue;
      }
    }
    int32_t iWidgetIndex = SpaceOrderWidgetArray.Find(hWidget);
    hWidget = SpaceOrderWidgetArray[(iWidgetIndex + 1) % nWidgetCount];
  }
}
static int32_t XFA_TabOrderWidgetComparator(const void* phWidget1,
                                            const void* phWidget2) {
  CXFA_FFWidget* pWidget1 = (*(CXFA_TabParam**)phWidget1)->m_pWidget;
  CXFA_FFWidget* pWidget2 = (*(CXFA_TabParam**)phWidget2)->m_pWidget;
  CFX_RectF rt1, rt2;
  pWidget1->GetWidgetRect(rt1);
  pWidget2->GetWidgetRect(rt2);
  FX_FLOAT x1 = rt1.left, y1 = rt1.top, x2 = rt2.left, y2 = rt2.top;
  if (y1 < y2 || (y1 - y2 < XFA_FLOAT_PERCISION && x1 < x2)) {
    return -1;
  }
  return 1;
}
void CXFA_FFTabOrderPageWidgetIterator::OrderContainer(
    CXFA_LayoutItemIterator* sIterator,
    CXFA_LayoutItem* pContainerItem,
    CXFA_TabParam* pContainer,
    FX_BOOL& bCurrentItem,
    FX_BOOL& bContentArea,
    FX_BOOL bMarsterPage) {
  CFX_PtrArray tabParams;
  CXFA_LayoutItem* pSearchItem = sIterator->MoveToNext();
  while (pSearchItem) {
    if (!pSearchItem->IsContentLayoutItem()) {
      bContentArea = TRUE;
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
        bCurrentItem = TRUE;
        break;
      }
      CXFA_TabParam* pParam = new CXFA_TabParam;
      pParam->m_pWidget = hWidget;
      tabParams.Add(pParam);
      if (XFA_IsLayoutElement(pSearchItem->GetFormNode()->GetClassID(), TRUE)) {
        OrderContainer(sIterator, pSearchItem, pParam, bCurrentItem,
                       bContentArea, bMarsterPage);
      }
    }
    if (bCurrentItem) {
      pSearchItem = sIterator->GetCurrent();
      bCurrentItem = FALSE;
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
    CXFA_TabParam* pParam = (CXFA_TabParam*)tabParams[iStart];
    pContainer->m_Children.Add(pParam->m_pWidget);
    if (pParam->m_Children.GetSize() > 0) {
      pContainer->m_Children.Append(pParam->m_Children);
    }
    delete pParam;
  }
  tabParams.RemoveAll();
}
void CXFA_FFTabOrderPageWidgetIterator::CreateSpaceOrderWidgetArray(
    CXFA_WidgetArray& WidgetArray) {
  CXFA_LayoutItemIterator sIterator;
  sIterator.Init(m_pPageView);
  CXFA_TabParam* pParam = new CXFA_TabParam;
  FX_BOOL bCurrentItem = FALSE;
  FX_BOOL bContentArea = FALSE;
  OrderContainer(&sIterator, NULL, pParam, bCurrentItem, bContentArea);
  if (pParam->m_Children.GetSize() > 0) {
    WidgetArray.Append(pParam->m_Children);
  }
  sIterator.Reset();
  bCurrentItem = FALSE;
  bContentArea = FALSE;
  pParam->m_Children.RemoveAll();
  OrderContainer(&sIterator, NULL, pParam, bCurrentItem, bContentArea, TRUE);
  if (pParam->m_Children.GetSize() > 0) {
    WidgetArray.Append(pParam->m_Children);
  }
  delete pParam;
}
CXFA_FFWidget* CXFA_FFTabOrderPageWidgetIterator::GetWidget(
    CXFA_LayoutItem* pLayoutItem) {
  if (CXFA_FFWidget* pWidget = XFA_GetWidgetFromLayoutItem(pLayoutItem)) {
    if (!pWidget->IsLoaded() &&
        (pWidget->GetStatus() & XFA_WIDGETSTATUS_Visible)) {
      pWidget->LoadWidget();
    }
    return pWidget;
  }
  return NULL;
}
