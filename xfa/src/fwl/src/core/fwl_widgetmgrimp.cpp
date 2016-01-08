// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetmgrimp.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_appimp.h"

FX_BOOL FWL_UseOffscreen(IFWL_Widget* pWidget) {
#if (_FX_OS_ == _FX_MACOSX_)
  return FALSE;
#else
  return pWidget->GetStyles() & FWL_WGTSTYLE_Offscreen;
#endif
}
IFWL_WidgetMgr* FWL_GetWidgetMgr() {
  IFWL_App* pApp = FWL_GetApp();
  if (!pApp)
    return NULL;
  return pApp->GetWidgetMgr();
}
CFWL_WidgetMgr::CFWL_WidgetMgr(IFWL_AdapterNative* pAdapterNative)
    : m_dwCapability(0) {
  m_pDelegate = new CFWL_WidgetMgrDelegate(this);
  m_pAdapter = pAdapterNative->GetWidgetMgr(m_pDelegate);
  FXSYS_assert(m_pAdapter);
  CFWL_WidgetMgrItem* pRoot = new CFWL_WidgetMgrItem;
  m_mapWidgetItem.SetAt(NULL, pRoot);
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
  m_rtScreen.Reset();
  IFWL_AdapterMonitorMgr* pMonitorMgr = pAdapterNative->GetMonitorMgr();
  if (pMonitorMgr) {
    FWL_HMONITOR monitor = pMonitorMgr->GetCurrentMonitor();
    if (monitor) {
      pMonitorMgr->GetMonitorSize(monitor, m_rtScreen.width, m_rtScreen.height);
    }
  }
#endif
}
CFWL_WidgetMgr::~CFWL_WidgetMgr() {
  FX_POSITION ps = m_mapWidgetItem.GetStartPosition();
  while (ps) {
    void* pWidget;
    CFWL_WidgetMgrItem* pItem;
    m_mapWidgetItem.GetNextAssoc(ps, pWidget, (void*&)pItem);
    delete pItem;
  }
  m_mapWidgetItem.RemoveAll();
  if (m_pDelegate) {
    delete m_pDelegate;
    m_pDelegate = NULL;
  }
}
int32_t CFWL_WidgetMgr::CountWidgets(IFWL_Widget* pParent) {
  CFWL_WidgetMgrItem* pParentItem = GetWidgetMgrItem(pParent);
  return TravelWidgetMgr(pParentItem, NULL, NULL);
}
IFWL_Widget* CFWL_WidgetMgr::GetWidget(int32_t nIndex, IFWL_Widget* pParent) {
  CFWL_WidgetMgrItem* pParentItem = GetWidgetMgrItem(pParent);
  IFWL_Widget* pWidget = NULL;
  TravelWidgetMgr(pParentItem, &nIndex, NULL, &pWidget);
  return pWidget;
}
IFWL_Widget* CFWL_WidgetMgr::GetWidget(IFWL_Widget* pWidget,
                                       FWL_WGTRELATION eRelation) {
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem) {
    return NULL;
  }
  IFWL_Widget* pRet = NULL;
  switch (eRelation) {
    case FWL_WGTRELATION_Parent: {
      pRet = pItem->pParent ? pItem->pParent->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_Owner: {
      pRet = pItem->pOwner ? pItem->pOwner->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_FirstSibling: {
      pItem = pItem->pPrevious;
      while (pItem && pItem->pPrevious) {
        pItem = pItem->pPrevious;
      }
      pRet = pItem ? pItem->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_PriorSibling: {
      pRet = pItem->pPrevious ? pItem->pPrevious->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_NextSibling: {
      pRet = pItem->pNext ? pItem->pNext->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_LastSibling: {
      pItem = pItem->pNext;
      while (pItem && pItem->pNext) {
        pItem = pItem->pNext;
      }
      pRet = pItem ? pItem->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_FirstChild: {
      pRet = pItem->pChild ? pItem->pChild->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_LastChild: {
      pItem = pItem->pChild;
      while (pItem && pItem->pNext) {
        pItem = pItem->pNext;
      }
      pRet = pItem ? pItem->pWidget : NULL;
      break;
    }
    case FWL_WGTRELATION_SystemForm: {
      while (pItem) {
        if (IsAbleNative(pItem->pWidget)) {
          pRet = pItem->pWidget;
          break;
        }
        pItem = pItem->pParent;
      }
      break;
    }
    default: {}
  }
  return pRet;
}
int32_t CFWL_WidgetMgr::GetWidgetIndex(IFWL_Widget* pWidget) {
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return -1;
  return TravelWidgetMgr(pItem->pParent, NULL, pItem);
}
FX_BOOL CFWL_WidgetMgr::SetWidgetIndex(IFWL_Widget* pWidget, int32_t nIndex) {
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem)
    return FALSE;
  if (!pItem->pParent)
    return FALSE;
  CFWL_WidgetMgrItem* pChild = pItem->pParent->pChild;
  int32_t i = 0;
  while (pChild) {
    if (pChild == pItem) {
      if (i == nIndex) {
        return TRUE;
      }
      if (pChild->pPrevious) {
        pChild->pPrevious->pNext = pChild->pNext;
      }
      if (pChild->pNext) {
        pChild->pNext->pPrevious = pChild->pPrevious;
      }
      if (pItem->pParent->pChild == pItem) {
        pItem->pParent->pChild = pItem->pNext;
      }
      pItem->pNext = NULL;
      pItem->pPrevious = NULL;
      break;
    }
    if (!pChild->pNext) {
      break;
    }
    pChild = pChild->pNext;
    ++i;
  }
  pChild = pItem->pParent->pChild;
  if (pChild) {
    if (nIndex < 0) {
      while (pChild->pNext) {
        pChild = pChild->pNext;
      }
      pChild->pNext = pItem;
      pItem->pPrevious = pChild;
      pItem->pNext = NULL;
      return TRUE;
    }
    i = 0;
    while (i < nIndex && pChild->pNext) {
      pChild = pChild->pNext;
      ++i;
    }
    if (!pChild->pNext) {
      pChild->pNext = pItem;
      pItem->pPrevious = pChild;
      pItem->pNext = NULL;
      return TRUE;
    }
    if (pChild->pPrevious) {
      pItem->pPrevious = pChild->pPrevious;
      pChild->pPrevious->pNext = pItem;
    }
    pChild->pPrevious = pItem;
    pItem->pNext = pChild;
    if (pItem->pParent->pChild == pChild) {
      pItem->pParent->pChild = pItem;
    }
  } else {
    pItem->pParent->pChild = pItem;
    pItem->pPrevious = NULL;
    pItem->pNext = NULL;
  }
  return TRUE;
}
FWL_ERR CFWL_WidgetMgr::RepaintWidget(IFWL_Widget* pWidget,
                                      const CFX_RectF* pRect) {
  if (!m_pAdapter)
    return FWL_ERR_Indefinite;
  IFWL_Widget* pNative = pWidget;
  CFX_RectF rect(*pRect);
  if (IsFormDisabled()) {
    IFWL_Widget* pOuter = pWidget->GetOuter();
    while (pOuter) {
      CFX_RectF rtTemp;
      pNative->GetWidgetRect(rtTemp);
      rect.left += rtTemp.left;
      rect.top += rtTemp.top;
      pNative = pOuter;
      pOuter = pOuter->GetOuter();
    }
  } else if (!IsAbleNative(pWidget)) {
    pNative = GetWidget(pWidget, FWL_WGTRELATION_SystemForm);
    if (!pNative)
      return FWL_ERR_Indefinite;
    pWidget->TransformTo(pNative, rect.left, rect.top);
  }
  AddRedrawCounts(pNative);
  return m_pAdapter->RepaintWidget(pNative, &rect);
}
void CFWL_WidgetMgr::AddWidget(IFWL_Widget* pWidget) {
  CFWL_WidgetMgrItem* pParentItem = GetWidgetMgrItem(NULL);
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem) {
    pItem = new CFWL_WidgetMgrItem;
    pItem->pWidget = pWidget;
    m_mapWidgetItem.SetAt(pWidget, pItem);
  }
  if (pItem->pParent && pItem->pParent != pParentItem) {
    if (pItem->pPrevious) {
      pItem->pPrevious->pNext = pItem->pNext;
    }
    if (pItem->pNext) {
      pItem->pNext->pPrevious = pItem->pPrevious;
    }
    if (pItem->pParent->pChild == pItem) {
      pItem->pParent->pChild = pItem->pNext;
    }
  }
  pItem->pParent = pParentItem;
  SetWidgetIndex(pWidget, -1);
}
void CFWL_WidgetMgr::InsertWidget(IFWL_Widget* pParent,
                                  IFWL_Widget* pChild,
                                  int32_t nIndex) {
  CFWL_WidgetMgrItem* pParentItem = GetWidgetMgrItem(pParent);
  if (!pParentItem) {
    pParentItem = new CFWL_WidgetMgrItem;
    pParentItem->pWidget = pParent;
    m_mapWidgetItem.SetAt(pParent, pParentItem);
    CFWL_WidgetMgrItem* pRoot = GetWidgetMgrItem(NULL);
    pParentItem->pParent = pRoot;
    SetWidgetIndex(pParent, -1);
  }
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pChild);
  if (!pItem) {
    pItem = new CFWL_WidgetMgrItem;
    pItem->pWidget = pChild;
    m_mapWidgetItem.SetAt(pChild, pItem);
  }
  if (pItem->pParent && pItem->pParent != pParentItem) {
    if (pItem->pPrevious) {
      pItem->pPrevious->pNext = pItem->pNext;
    }
    if (pItem->pNext) {
      pItem->pNext->pPrevious = pItem->pPrevious;
    }
    if (pItem->pParent->pChild == pItem) {
      pItem->pParent->pChild = pItem->pNext;
    }
  }
  pItem->pParent = pParentItem;
  SetWidgetIndex(pChild, nIndex);
}
void CFWL_WidgetMgr::RemoveWidget(IFWL_Widget* pWidget) {
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
  if (!pItem) {
    return;
  }
  if (pItem->pPrevious) {
    pItem->pPrevious->pNext = pItem->pNext;
  }
  if (pItem->pNext) {
    pItem->pNext->pPrevious = pItem->pPrevious;
  }
  if (pItem->pParent && pItem->pParent->pChild == pItem) {
    pItem->pParent->pChild = pItem->pNext;
  }
  CFWL_WidgetMgrItem* pChild = pItem->pChild;
  while (pChild) {
    CFWL_WidgetMgrItem* pNext = pChild->pNext;
    RemoveWidget(pChild->pWidget);
    pChild = pNext;
  }
  m_mapWidgetItem.RemoveKey(pWidget);
  delete pItem;
}
void CFWL_WidgetMgr::SetOwner(IFWL_Widget* pOwner, IFWL_Widget* pOwned) {
  CFWL_WidgetMgrItem* pParentItem = GetWidgetMgrItem(pOwner);
  if (!pParentItem) {
    pParentItem = new CFWL_WidgetMgrItem;
    pParentItem->pWidget = pOwner;
    m_mapWidgetItem.SetAt(pOwner, pParentItem);
    CFWL_WidgetMgrItem* pRoot = GetWidgetMgrItem(NULL);
    pParentItem->pParent = pRoot;
    SetWidgetIndex(pOwner, -1);
  }
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pOwned);
  if (!pItem) {
    pItem = new CFWL_WidgetMgrItem;
    pItem->pWidget = pOwned;
    m_mapWidgetItem.SetAt(pOwned, pItem);
  }
  pItem->pOwner = pParentItem;
}
void CFWL_WidgetMgr::SetParent(IFWL_Widget* pParent, IFWL_Widget* pChild) {
  CFWL_WidgetMgrItem* pParentItem = GetWidgetMgrItem(pParent);
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pChild);
  if (!pItem)
    return;
  if (pItem->pParent && pItem->pParent != pParentItem) {
    if (pItem->pPrevious) {
      pItem->pPrevious->pNext = pItem->pNext;
    }
    if (pItem->pNext) {
      pItem->pNext->pPrevious = pItem->pPrevious;
    }
    if (pItem->pParent->pChild == pItem) {
      pItem->pParent->pChild = pItem->pNext;
    }
    pItem->pNext = NULL;
    pItem->pPrevious = NULL;
  }
  pItem->pParent = pParentItem;
  SetWidgetIndex(pChild, -1);
  if (!m_pAdapter)
    return;
  m_pAdapter->SetParentWidget(pChild, pParent);
}
FX_BOOL CFWL_WidgetMgr::IsChild(IFWL_Widget* pChild, IFWL_Widget* pParent) {
  IFWL_Widget* pTemp = pChild;
  do {
    if (pTemp == pParent) {
      return TRUE;
    }
    pTemp = GetWidget(pTemp, FWL_WGTRELATION_Parent);
  } while (pTemp);
  return FALSE;
}
FWL_ERR CFWL_WidgetMgr::CreateWidget_Native(IFWL_Widget* pWidget) {
  if (!IsAbleNative(pWidget)) {
    return FWL_ERR_Succeeded;
  }
  return m_pAdapter->CreateWidget(pWidget, pWidget->GetOwner());
}
FWL_ERR CFWL_WidgetMgr::DestroyWidget_Native(IFWL_Widget* pWidget) {
  if (!IsAbleNative(pWidget)) {
    return FWL_ERR_Succeeded;
  }
  return m_pAdapter->DestroyWidget(pWidget);
}
FWL_ERR CFWL_WidgetMgr::GetWidgetRect_Native(IFWL_Widget* pWidget,
                                             CFX_RectF& rect) {
  if (!IsAbleNative(pWidget)) {
    return FWL_ERR_Succeeded;
  }
  return m_pAdapter->GetWidgetRect(pWidget, rect);
}
FWL_ERR CFWL_WidgetMgr::SetWidgetRect_Native(IFWL_Widget* pWidget,
                                             const CFX_RectF& rect) {
  if (FWL_UseOffscreen(pWidget)) {
    CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
    pItem->iRedrawCounter++;
    if (pItem->pOffscreen) {
      CFX_RenderDevice* pDevice = pItem->pOffscreen->GetRenderDevice();
      if (pDevice && pDevice->GetBitmap()) {
        CFX_DIBitmap* pBitmap = pDevice->GetBitmap();
        if (pBitmap->GetWidth() - rect.width > 1 ||
            pBitmap->GetHeight() - rect.height > 1) {
          delete pItem->pOffscreen;
          pItem->pOffscreen = NULL;
        }
      }
    }
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
    pItem->bOutsideChanged = !m_rtScreen.Contains(rect);
#endif
  }
  return m_pAdapter->SetWidgetRect(pWidget, rect);
}
FWL_ERR CFWL_WidgetMgr::SetWidgetPosition_Native(IFWL_Widget* pWidget,
                                                 FX_FLOAT fx,
                                                 FX_FLOAT fy) {
  return m_pAdapter->SetWidgetPosition(pWidget, fx, fy);
}
FWL_ERR CFWL_WidgetMgr::SetWidgetIcon_Native(IFWL_Widget* pWidget,
                                             const CFX_DIBitmap* pIcon,
                                             FX_BOOL bBig) {
  return m_pAdapter->SetWidgetIcon(pWidget, pIcon, bBig);
}
FWL_ERR CFWL_WidgetMgr::SetWidgetCaption_Native(
    IFWL_Widget* pWidget,
    const CFX_WideStringC& wsCaption) {
  return m_pAdapter->SetWidgetCaption(pWidget, wsCaption);
}
FWL_ERR CFWL_WidgetMgr::SetBorderRegion_Native(IFWL_Widget* pWidget,
                                               CFX_Path* pPath) {
  return m_pAdapter->SetBorderRegion(pWidget, pPath);
}
FWL_ERR CFWL_WidgetMgr::ShowWidget_Native(IFWL_Widget* pWidget) {
  return m_pAdapter->ShowWidget(pWidget);
}
FWL_ERR CFWL_WidgetMgr::HideWidget_Native(IFWL_Widget* pWidget) {
  return m_pAdapter->HideWidget(pWidget);
}
FWL_ERR CFWL_WidgetMgr::SetNormal_Native(IFWL_Widget* pWidget) {
  return m_pAdapter->SetNormal(pWidget);
}
FWL_ERR CFWL_WidgetMgr::SetMaximize_Native(IFWL_Widget* pWidget) {
  return m_pAdapter->SetMaximize(pWidget);
}
FWL_ERR CFWL_WidgetMgr::SetMinimize_Native(IFWL_Widget* pWidget) {
  return m_pAdapter->SetMinimize(pWidget);
}
FX_BOOL CFWL_WidgetMgr::CheckMessage_Native() {
  return m_pAdapter->CheckMessage();
}
FWL_ERR CFWL_WidgetMgr::DispatchMessage_Native() {
  return m_pAdapter->DispatchMessage();
}
FX_BOOL CFWL_WidgetMgr::IsIdleMessage_Native() {
  return m_pAdapter->IsIdleMessage();
}
FWL_ERR CFWL_WidgetMgr::Exit_Native(int32_t iExitCode) {
  return m_pAdapter->Exit(iExitCode);
}
FWL_ERR CFWL_WidgetMgr::CreateWidgetWithNativeId_Native(IFWL_Widget* pWidget,
                                                        void* vp) {
  return m_pAdapter->CreateWidgetWithNativeId(pWidget, vp);
}
IFWL_Widget* CFWL_WidgetMgr::GetWidgetAtPoint(IFWL_Widget* parent,
                                              FX_FLOAT x,
                                              FX_FLOAT y) {
  if (!parent)
    return NULL;
  FX_FLOAT x1;
  FX_FLOAT y1;
  IFWL_Widget* child = GetWidget(parent, FWL_WGTRELATION_LastChild);
  while (child) {
    if ((child->GetStates() & FWL_WGTSTATE_Invisible) == 0) {
      x1 = x;
      y1 = y;
      CFX_Matrix matrixOnParent;
      child->GetMatrix(matrixOnParent);
      CFX_Matrix m;
      m.SetIdentity();
      m.SetReverse(matrixOnParent);
      m.TransformPoint(x1, y1);
      CFX_RectF bounds;
      child->GetWidgetRect(bounds);
      if (bounds.Contains(x1, y1)) {
        x1 -= bounds.left;
        y1 -= bounds.top;
        return GetWidgetAtPoint(child, x1, y1);
      }
    }
    child = GetWidget(child, FWL_WGTRELATION_PriorSibling);
  }
  return parent;
}
void CFWL_WidgetMgr::NotifySizeChanged(IFWL_Widget* pForm,
                                       FX_FLOAT fx,
                                       FX_FLOAT fy) {
  if (!FWL_UseOffscreen(pForm)) {
    return;
  }
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pForm);
  if (pItem->pOffscreen) {
    delete pItem->pOffscreen;
    pItem->pOffscreen = NULL;
  }
}
IFWL_Widget* CFWL_WidgetMgr::nextTab(IFWL_Widget* parent,
                                     IFWL_Widget* focus,
                                     FX_BOOL& bFind) {
  IFWL_Widget* child =
      FWL_GetWidgetMgr()->GetWidget(parent, FWL_WGTRELATION_FirstChild);
  while (child) {
    if (focus == child) {
      bFind = TRUE;
    }
    if ((child->GetStyles() & FWL_WGTSTYLE_TabStop) &&
        (!focus || (focus != child && bFind))) {
      return child;
    }
    IFWL_Widget* bRet = nextTab(child, focus, bFind);
    if (bRet) {
      return bRet;
    }
    child = FWL_GetWidgetMgr()->GetWidget(child, FWL_WGTRELATION_NextSibling);
  }
  return NULL;
}
int32_t CFWL_WidgetMgr::CountRadioButtonGroup(IFWL_Widget* pFirst) {
  int32_t iRet = 0;
  IFWL_Widget* pChild = pFirst;
  while (pChild) {
    if ((pChild->GetStyles() & FWL_WGTSTYLE_Group) &&
        pChild->GetClassID() == 3811304691) {
      iRet++;
    }
    pChild = GetWidget(pChild, FWL_WGTRELATION_NextSibling);
  }
  return iRet;
}
IFWL_Widget* CFWL_WidgetMgr::GetSiblingRadioButton(IFWL_Widget* pWidget,
                                                   FX_BOOL bNext) {
  while ((pWidget = GetWidget(pWidget, bNext ? FWL_WGTRELATION_NextSibling
                                             : FWL_WGTRELATION_PriorSibling)) !=
         NULL) {
    if (pWidget->GetClassID() == 3811304691) {
      return pWidget;
    }
  }
  return NULL;
}
IFWL_Widget* CFWL_WidgetMgr::GetRadioButtonGroupHeader(
    IFWL_Widget* pRadioButton) {
  if (pRadioButton->GetStyles() & FWL_WGTSTYLE_Group) {
    return pRadioButton;
  }
  IFWL_Widget* pNext = pRadioButton;
  while ((pNext = GetSiblingRadioButton(pNext, FALSE)) != NULL) {
    if (pNext->GetStyles() & FWL_WGTSTYLE_Group) {
      return pNext;
    }
  }
  pNext = GetWidget(pRadioButton, FWL_WGTRELATION_LastSibling);
  if ((pNext->GetStyles() & FWL_WGTSTYLE_Group) &&
      pNext->GetClassID() == 3811304691) {
    return pNext;
  }
  while ((pNext = GetSiblingRadioButton(pNext, FALSE)) && pNext &&
         pNext != pRadioButton) {
    if (pNext->GetStyles() & FWL_WGTSTYLE_Group) {
      return pNext;
    }
  }
  pNext = GetWidget(pRadioButton, FWL_WGTRELATION_FirstSibling);
  if (pNext && (pNext->GetStyles() == FWL_WGTSTYLE_Group) &&
      pNext->GetClassID() == 3811304691) {
    return pNext;
  }
  return GetSiblingRadioButton(pNext, TRUE);
}
void CFWL_WidgetMgr::GetSameGroupRadioButton(IFWL_Widget* pRadioButton,
                                             CFX_PtrArray& group) {
  IFWL_Widget* pFirst = GetWidget(pRadioButton, FWL_WGTRELATION_FirstSibling);
  if (!pFirst) {
    pFirst = pRadioButton;
  }
  int32_t iGroup = CountRadioButtonGroup(pFirst);
  if (iGroup < 2) {
    if (pFirst->GetClassID() == 3811304691) {
      group.Add(pFirst);
    }
    IFWL_Widget* pNext = pFirst;
    while ((pNext = GetSiblingRadioButton(pNext, TRUE)) != NULL) {
      group.Add(pNext);
    }
    return;
  }
  IFWL_Widget* pNext = GetRadioButtonGroupHeader(pRadioButton);
  do {
    group.Add(pNext);
    pNext = GetSiblingRadioButton(pNext, TRUE);
    if (!pNext) {
      if (pFirst->GetClassID() == 3811304691) {
        pNext = pFirst;
      } else {
        pNext = GetSiblingRadioButton(pFirst, TRUE);
      }
    }
  } while (pNext && ((pNext->GetStyles() & FWL_WGTSTYLE_Group) == 0));
}
IFWL_Widget* CFWL_WidgetMgr::GetDefaultButton(IFWL_Widget* pParent) {
  if ((pParent->GetClassID() == 3521614244) &&
      (pParent->GetStates() & (1 << (FWL_WGTSTATE_MAX + 2)))) {
    return pParent;
  }
  IFWL_Widget* child =
      FWL_GetWidgetMgr()->GetWidget(pParent, FWL_WGTRELATION_FirstChild);
  while (child) {
    if ((child->GetClassID() == 3521614244) &&
        (child->GetStates() & (1 << (FWL_WGTSTATE_MAX + 2)))) {
      return child;
    }
    IFWL_Widget* find = GetDefaultButton(child);
    if (find) {
      return find;
    }
    child = FWL_GetWidgetMgr()->GetWidget(child, FWL_WGTRELATION_NextSibling);
  }
  return NULL;
}
void CFWL_WidgetMgr::AddRedrawCounts(IFWL_Widget* pWidget) {
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
  (pItem->iRedrawCounter)++;
}
void CFWL_WidgetMgr::ResetRedrawCounts(IFWL_Widget* pWidget) {
  CFWL_WidgetMgrItem* pItem = GetWidgetMgrItem(pWidget);
  pItem->iRedrawCounter = 0;
}
CFWL_WidgetMgrItem* CFWL_WidgetMgr::GetWidgetMgrItem(IFWL_Widget* pWidget) {
  return static_cast<CFWL_WidgetMgrItem*>(m_mapWidgetItem.GetValueAt(pWidget));
}
int32_t CFWL_WidgetMgr::TravelWidgetMgr(CFWL_WidgetMgrItem* pParent,
                                        int32_t* pIndex,
                                        CFWL_WidgetMgrItem* pItem,
                                        IFWL_Widget** pWidget) {
  if (!pParent) {
    return 0;
  }
  int32_t iCount = 0;
  CFWL_WidgetMgrItem* pChild = pParent->pChild;
  while (pChild) {
    iCount++;
    if (pIndex) {
      if (*pIndex == 0) {
        *pWidget = pChild->pWidget;
        return iCount;
      }
      pIndex--;
    }
    if (pItem && pItem == pChild) {
      return iCount - 1;
    }
    pChild = pChild->pNext;
  }
  if (pIndex) {
    return 0;
  } else if (pItem) {
    return -1;
  }
  return iCount - 1;
}
FX_BOOL CFWL_WidgetMgr::IsAbleNative(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  if (!pWidget->IsInstance(FX_WSTRC(FWL_CLASS_Form))) {
    return FALSE;
  }
  FX_DWORD dwStyles = pWidget->GetStyles();
  return ((dwStyles & FWL_WGTSTYLE_WindowTypeMask) ==
          FWL_WGTSTYLE_OverLapper) ||
         (dwStyles & FWL_WGTSTYLE_Popup);
}
FX_BOOL CFWL_WidgetMgr::IsThreadEnabled() {
  return !(m_dwCapability & FWL_WGTMGR_DisableThread);
}
FX_BOOL CFWL_WidgetMgr::IsFormDisabled() {
  return m_dwCapability & FWL_WGTMGR_DisableForm;
}
FX_BOOL CFWL_WidgetMgr::GetAdapterPopupPos(IFWL_Widget* pWidget,
                                           FX_FLOAT fMinHeight,
                                           FX_FLOAT fMaxHeight,
                                           const CFX_RectF& rtAnchor,
                                           CFX_RectF& rtPopup) {
  IFWL_AdapterWidgetMgr* pSDApapter = GetAdapterWidgetMgr();
  return pSDApapter->GetPopupPos(pWidget, fMinHeight, fMaxHeight, rtAnchor,
                                 rtPopup);
}
CFWL_WidgetMgrDelegate::CFWL_WidgetMgrDelegate(CFWL_WidgetMgr* pWidgetMgr)
    : m_pWidgetMgr(pWidgetMgr) {}
FWL_ERR CFWL_WidgetMgrDelegate::OnSetCapability(FX_DWORD dwCapability) {
  m_pWidgetMgr->m_dwCapability = dwCapability;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_WidgetMgrDelegate::OnProcessMessageToForm(CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  if (!pMessage->m_pDstTarget)
    return 0;
  IFWL_Widget* pDstWidget = pMessage->m_pDstTarget;
  IFWL_NoteThread* pNoteThread = pDstWidget->GetOwnerThread();
  if (!pNoteThread)
    return 0;
  CFWL_NoteDriver* pNoteDriver =
      static_cast<CFWL_NoteDriver*>(pNoteThread->GetNoteDriver());
  if (!pNoteDriver)
    return 0;
  if (m_pWidgetMgr->IsThreadEnabled()) {
    pMessage = static_cast<CFWL_Message*>(pMessage->Clone());
  }
  if (m_pWidgetMgr->IsFormDisabled()) {
    pNoteDriver->ProcessMessage(pMessage);
  } else {
    pNoteDriver->QueueMessage(pMessage);
  }
#if (_FX_OS_ == _FX_MACOSX_)
  CFWL_NoteLoop* pTopLoop = pNoteDriver->GetTopLoop();
  if (pTopLoop) {
    pNoteDriver->UnqueueMessage(pTopLoop);
  }
#endif
  if (m_pWidgetMgr->IsThreadEnabled()) {
    pMessage->Release();
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetMgrDelegate::OnDrawWidget(IFWL_Widget* pWidget,
                                             CFX_Graphics* pGraphics,
                                             const CFX_Matrix* pMatrix) {
  if (!pWidget)
    return FWL_ERR_Indefinite;
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  CFX_Graphics* pTemp = DrawWidgetBefore(pWidget, pGraphics, pMatrix);
  CFX_RectF clipCopy;
  pWidget->GetWidgetRect(clipCopy);
  clipCopy.left = clipCopy.top = 0;
  if (bUseOffscreenDirect(pWidget)) {
    DrawWidgetAfter(pWidget, pGraphics, clipCopy, pMatrix);
    return FWL_ERR_Succeeded;
  }
  CFX_RectF clipBounds;
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_) || \
    (_FX_OS_ == _FX_LINUX_DESKTOP_) || (_FX_OS_ == _FX_ANDROID_)
  IFWL_WidgetDelegate* pDelegate = pWidget->SetDelegate(NULL);
  pDelegate->OnDrawWidget(pTemp, pMatrix);
  pGraphics->GetClipRect(clipBounds);
  clipCopy = clipBounds;
#elif(_FX_OS_ == _FX_MACOSX_)
  if (m_pWidgetMgr->IsFormDisabled()) {
    IFWL_WidgetDelegate* pDelegate = pWidget->SetDelegate(NULL);
    pDelegate->OnDrawWidget(pTemp, pMatrix);
    pGraphics->GetClipRect(clipBounds);
    clipCopy = clipBounds;
  } else {
    clipBounds.Set(pMatrix->a, pMatrix->b, pMatrix->c, pMatrix->d);
    const_cast<CFX_Matrix*>(pMatrix)->SetIdentity();  // FIXME: const cast.
#ifdef FWL_UseMacSystemBorder
#else
#endif
    {
      IFWL_WidgetDelegate* pDelegate = pWidget->SetDelegate(NULL);
      pDelegate->OnDrawWidget(pTemp, pMatrix);
    }
  }
#endif
  if (!m_pWidgetMgr->IsFormDisabled()) {
    CFX_RectF rtClient;
    pWidget->GetClientRect(rtClient);
    clipBounds.Intersect(rtClient);
  }
  if (!clipBounds.IsEmpty()) {
    DrawChild(pWidget, clipBounds, pTemp, pMatrix);
  }
  DrawWidgetAfter(pWidget, pGraphics, clipCopy, pMatrix);
  m_pWidgetMgr->ResetRedrawCounts(pWidget);
  return FWL_ERR_Succeeded;
}
void CFWL_WidgetMgrDelegate::DrawChild(IFWL_Widget* parent,
                                       const CFX_RectF& rtClip,
                                       CFX_Graphics* pGraphics,
                                       const CFX_Matrix* pMatrix) {
  if (!parent)
    return;
  FX_BOOL bFormDisable = m_pWidgetMgr->IsFormDisabled();
  IFWL_Widget* pNextChild =
      m_pWidgetMgr->GetWidget(parent, FWL_WGTRELATION_FirstChild);
  while (pNextChild) {
    IFWL_Widget* child = pNextChild;
    pNextChild = m_pWidgetMgr->GetWidget(child, FWL_WGTRELATION_NextSibling);
    if (child->GetStates() & FWL_WGTSTATE_Invisible) {
      continue;
    }
    CFX_RectF rtWidget;
    child->GetWidgetRect(rtWidget);
    if (rtWidget.IsEmpty()) {
      continue;
    }
    CFX_Matrix widgetMatrix;
    CFX_RectF clipBounds(rtWidget);
    if (!bFormDisable) {
      child->GetMatrix(widgetMatrix, TRUE);
    }
    if (pMatrix) {
      widgetMatrix.Concat(*pMatrix);
    }
    if (!bFormDisable) {
      widgetMatrix.TransformPoint(clipBounds.left, clipBounds.top);
      clipBounds.Intersect(rtClip);
      if (clipBounds.IsEmpty()) {
        continue;
      }
      pGraphics->SaveGraphState();
      pGraphics->SetClipRect(clipBounds);
    }
    widgetMatrix.Translate(rtWidget.left, rtWidget.top, TRUE);
    IFWL_WidgetDelegate* pDelegate = child->SetDelegate(NULL);
    if (pDelegate) {
      if (m_pWidgetMgr->IsFormDisabled() ||
          IsNeedRepaint(child, &widgetMatrix, rtClip)) {
        pDelegate->OnDrawWidget(pGraphics, &widgetMatrix);
      }
    }
    if (!bFormDisable) {
      pGraphics->RestoreGraphState();
    }
    DrawChild(child, clipBounds, pGraphics,
              bFormDisable ? &widgetMatrix : pMatrix);
    child = m_pWidgetMgr->GetWidget(child, FWL_WGTRELATION_NextSibling);
  }
}
CFX_Graphics* CFWL_WidgetMgrDelegate::DrawWidgetBefore(
    IFWL_Widget* pWidget,
    CFX_Graphics* pGraphics,
    const CFX_Matrix* pMatrix) {
  if (!FWL_UseOffscreen(pWidget)) {
    return pGraphics;
  }
  CFWL_WidgetMgrItem* pItem = m_pWidgetMgr->GetWidgetMgrItem(pWidget);
  if (!pItem->pOffscreen) {
    pItem->pOffscreen = new CFX_Graphics;
    CFX_RectF rect;
    pWidget->GetWidgetRect(rect);
    pItem->pOffscreen->Create((int32_t)rect.width, (int32_t)rect.height,
                              FXDIB_Argb);
  }
  CFX_RectF rect;
  pGraphics->GetClipRect(rect);
  pItem->pOffscreen->SetClipRect(rect);
  return pItem->pOffscreen;
}
void CFWL_WidgetMgrDelegate::DrawWidgetAfter(IFWL_Widget* pWidget,
                                             CFX_Graphics* pGraphics,
                                             CFX_RectF& rtClip,
                                             const CFX_Matrix* pMatrix) {
  if (FWL_UseOffscreen(pWidget)) {
    CFWL_WidgetMgrItem* pItem = m_pWidgetMgr->GetWidgetMgrItem(pWidget);
    pGraphics->Transfer(pItem->pOffscreen, rtClip.left, rtClip.top, rtClip,
                        pMatrix);
#ifdef _WIN32
    pItem->pOffscreen->ClearClip();
#endif
  }
  CFWL_WidgetMgrItem* pItem = m_pWidgetMgr->GetWidgetMgrItem(pWidget);
  pItem->iRedrawCounter = 0;
}
#define FWL_NEEDREPAINTHIT_Point 12
#define FWL_NEEDREPAINTHIT_Piece 3
typedef struct _FWL_NeedRepaintHitData {
  CFX_PointF hitPoint;
  FX_BOOL bNotNeedRepaint;
  FX_BOOL bNotContainByDirty;
} FWL_NeedRepaintHitData;
FX_BOOL CFWL_WidgetMgrDelegate::IsNeedRepaint(IFWL_Widget* pWidget,
                                              CFX_Matrix* pMatrix,
                                              const CFX_RectF& rtDirty) {
  CFWL_WidgetMgrItem* pItem = m_pWidgetMgr->GetWidgetMgrItem(pWidget);
  if (pItem && pItem->iRedrawCounter > 0) {
    pItem->iRedrawCounter = 0;
    return TRUE;
  }
  CFX_RectF rtWidget;
  pWidget->GetWidgetRect(rtWidget);
  rtWidget.left = rtWidget.top = 0;
  pMatrix->TransformRect(rtWidget);
  if (!rtWidget.IntersectWith(rtDirty)) {
    return FALSE;
  }
  IFWL_Widget* pChild =
      FWL_GetWidgetMgr()->GetWidget(pWidget, FWL_WGTRELATION_FirstChild);
  if (!pChild) {
    return TRUE;
  }
  if (pChild->GetClassID() == 3150298670) {
    CFX_RectF rtTemp;
    pChild->GetWidgetRect(rtTemp);
    if (rtTemp.width >= rtWidget.width && rtTemp.height >= rtWidget.height) {
      pChild =
          FWL_GetWidgetMgr()->GetWidget(pChild, FWL_WGTRELATION_FirstChild);
      if (!pChild) {
        return TRUE;
      }
    }
  }
  CFX_RectF rtChilds;
  rtChilds.Empty();
  FX_BOOL bChildIntersectWithDirty = FALSE;
  FX_BOOL bOrginPtIntersectWidthChild = FALSE;
  FX_BOOL bOrginPtIntersectWidthDirty =
      rtDirty.Contains(rtWidget.left, rtWidget.top);
  static FWL_NeedRepaintHitData hitPoint[FWL_NEEDREPAINTHIT_Point];
  static int32_t iSize = sizeof(FWL_NeedRepaintHitData);
  FXSYS_memset(hitPoint, 0, iSize);
  FX_FLOAT fxPiece = rtWidget.width / FWL_NEEDREPAINTHIT_Piece;
  FX_FLOAT fyPiece = rtWidget.height / FWL_NEEDREPAINTHIT_Piece;
  hitPoint[2].hitPoint.x = hitPoint[6].hitPoint.x = rtWidget.left;
  hitPoint[0].hitPoint.x = hitPoint[3].hitPoint.x = hitPoint[7].hitPoint.x =
      hitPoint[10].hitPoint.x = fxPiece + rtWidget.left;
  hitPoint[1].hitPoint.x = hitPoint[4].hitPoint.x = hitPoint[8].hitPoint.x =
      hitPoint[11].hitPoint.x = fxPiece * 2 + rtWidget.left;
  hitPoint[5].hitPoint.x = hitPoint[9].hitPoint.x =
      rtWidget.width + rtWidget.left;
  hitPoint[0].hitPoint.y = hitPoint[1].hitPoint.y = rtWidget.top;
  hitPoint[2].hitPoint.y = hitPoint[3].hitPoint.y = hitPoint[4].hitPoint.y =
      hitPoint[5].hitPoint.y = fyPiece + rtWidget.top;
  hitPoint[6].hitPoint.y = hitPoint[7].hitPoint.y = hitPoint[8].hitPoint.y =
      hitPoint[9].hitPoint.y = fyPiece * 2 + rtWidget.top;
  hitPoint[10].hitPoint.y = hitPoint[11].hitPoint.y =
      rtWidget.height + rtWidget.top;
  do {
    CFX_RectF rect;
    pChild->GetWidgetRect(rect);
    CFX_RectF r = rect;
    r.left += rtWidget.left;
    r.top += rtWidget.top;
    if (r.IsEmpty()) {
      continue;
    }
    if (r.Contains(rtDirty)) {
      return FALSE;
    }
    if (!bChildIntersectWithDirty && r.IntersectWith(rtDirty)) {
      bChildIntersectWithDirty = TRUE;
    }
    if (bOrginPtIntersectWidthDirty && !bOrginPtIntersectWidthChild) {
      bOrginPtIntersectWidthChild = rect.Contains(0, 0);
    }
    if (rtChilds.IsEmpty()) {
      rtChilds = rect;
    } else if (!(pChild->GetStates() & FWL_WGTSTATE_Invisible)) {
      rtChilds.Union(rect);
    }
    for (int32_t i = 0; i < FWL_NEEDREPAINTHIT_Point; i++) {
      if (hitPoint[i].bNotContainByDirty || hitPoint[i].bNotNeedRepaint) {
        continue;
      }
      if (!rtDirty.Contains(hitPoint[i].hitPoint)) {
        hitPoint[i].bNotContainByDirty = TRUE;
        continue;
      }
      if (r.Contains(hitPoint[i].hitPoint)) {
        hitPoint[i].bNotNeedRepaint = TRUE;
      }
    }
  } while ((pChild = FWL_GetWidgetMgr()->GetWidget(
                pChild, FWL_WGTRELATION_NextSibling)) != NULL);
  if (!bChildIntersectWithDirty) {
    return TRUE;
  }
  if (bOrginPtIntersectWidthDirty && !bOrginPtIntersectWidthChild) {
    return TRUE;
  }
  if (rtChilds.IsEmpty()) {
    return TRUE;
  }
  int32_t repaintPoint = FWL_NEEDREPAINTHIT_Point;
  for (int32_t i = 0; i < FWL_NEEDREPAINTHIT_Point; i++) {
    if (hitPoint[i].bNotNeedRepaint) {
      repaintPoint--;
    }
  }
  if (repaintPoint > 0) {
    return TRUE;
  }
  pMatrix->TransformRect(rtChilds);
  if (rtChilds.Contains(rtDirty) || rtChilds.Contains(rtWidget)) {
    return FALSE;
  }
  return TRUE;
}
FX_BOOL CFWL_WidgetMgrDelegate::bUseOffscreenDirect(IFWL_Widget* pWidget) {
  CFWL_WidgetMgrItem* pItem = m_pWidgetMgr->GetWidgetMgrItem(pWidget);
  if (!FWL_UseOffscreen(pWidget) || !(pItem->pOffscreen)) {
    return FALSE;
  }
#if (_FX_OS_ == _FX_WIN32_DESKTOP_) || (_FX_OS_ == _FX_WIN64_)
  if (pItem->bOutsideChanged) {
    CFX_RectF r;
    pWidget->GetWidgetRect(r);
    CFX_RectF temp(m_pWidgetMgr->m_rtScreen);
    temp.Deflate(50, 50);
    if (!temp.Contains(r)) {
      return FALSE;
    }
    pItem->bOutsideChanged = FALSE;
  }
#endif
  return pItem->iRedrawCounter == 0;
}
static void FWL_WriteBMP(CFX_DIBitmap* pBitmap, const FX_CHAR* filename) {
  FILE* file = fopen(filename, "wb");
  if (file == NULL) {
    return;
  }
  int size = 14 + 40 + pBitmap->GetPitch() * pBitmap->GetHeight();
  unsigned char buffer[40];
  buffer[0] = 'B';
  buffer[1] = 'M';
  buffer[2] = (unsigned char)size;
  buffer[3] = (unsigned char)(size >> 8);
  buffer[4] = (unsigned char)(size >> 16);
  buffer[5] = (unsigned char)(size >> 24);
  buffer[6] = buffer[7] = buffer[8] = buffer[9] = 0;
  buffer[10] = 54;
  buffer[11] = buffer[12] = buffer[13] = 0;
  fwrite(buffer, 14, 1, file);
  memset(buffer, 0, 40);
  buffer[0] = 40;
  buffer[4] = (unsigned char)pBitmap->GetWidth();
  buffer[5] = (unsigned char)(pBitmap->GetWidth() >> 8);
  buffer[6] = (unsigned char)(pBitmap->GetWidth() >> 16);
  buffer[7] = (unsigned char)(pBitmap->GetWidth() >> 24);
  buffer[8] = (unsigned char)(-pBitmap->GetHeight());
  buffer[9] = (unsigned char)((-pBitmap->GetHeight()) >> 8);
  buffer[10] = (unsigned char)((-pBitmap->GetHeight()) >> 16);
  buffer[11] = (unsigned char)((-pBitmap->GetHeight()) >> 24);
  buffer[12] = 1;
  buffer[14] = pBitmap->GetBPP();
  fwrite(buffer, 40, 1, file);
  for (int row = 0; row < pBitmap->GetHeight(); row++) {
    uint8_t* scan_line = pBitmap->GetBuffer() + row * pBitmap->GetPitch();
    fwrite(scan_line, pBitmap->GetPitch(), 1, file);
  }
  fclose(file);
}
FWL_ERR FWL_WidgetMgrSnapshot(IFWL_Widget* pWidget,
                              const CFX_WideString* saveFile,
                              const CFX_Matrix* pMatrix) {
  CFX_RectF r;
  pWidget->GetWidgetRect(r);
  CFX_Graphics gs;
  gs.Create((int32_t)r.width, (int32_t)r.height, FXDIB_Argb);
  CFWL_WidgetMgr* widgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  CFWL_WidgetMgrDelegate* delegate = widgetMgr->GetDelegate();
  delegate->OnDrawWidget(pWidget, &gs, pMatrix);
  CFX_DIBitmap* dib = gs.GetRenderDevice()->GetBitmap();
  FWL_WriteBMP(dib, saveFile->UTF8Encode());
  return FWL_ERR_Succeeded;
}
FX_BOOL FWL_WidgetIsChild(IFWL_Widget* parent, IFWL_Widget* find) {
  if (!find) {
    return FALSE;
  }
  IFWL_Widget* child =
      FWL_GetWidgetMgr()->GetWidget(parent, FWL_WGTRELATION_FirstChild);
  while (child) {
    if (child == find) {
      return TRUE;
    }
    if (FWL_WidgetIsChild(child, find)) {
      return TRUE;
    }
    child = FWL_GetWidgetMgr()->GetWidget(child, FWL_WGTRELATION_NextSibling);
  }
  return FALSE;
}
