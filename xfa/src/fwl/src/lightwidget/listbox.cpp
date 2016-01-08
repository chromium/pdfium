// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "xfa/src/foxitlib.h"

CFWL_ListBox* CFWL_ListBox::Create() {
  return new CFWL_ListBox;
}
FWL_ERR CFWL_ListBox::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_ListBox> pListBox(IFWL_ListBox::Create(
      m_pProperties->MakeWidgetImpProperties(&m_ListBoxDP), nullptr));
  FWL_ERR ret = pListBox->Initialize();
  if (ret != FWL_ERR_Succeeded) {
    return ret;
  }
  m_pIface = pListBox.release();
  CFWL_Widget::Initialize();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::AddDIBitmap(CFX_DIBitmap* pDIB, FWL_HLISTITEM hItem) {
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_pDIB = pDIB;
  return FWL_ERR_Succeeded;
}
FWL_HLISTITEM CFWL_ListBox::AddString(const CFX_WideStringC& wsAdd,
                                      FX_BOOL bSelect) {
  CFWL_ListItem* pItem = new CFWL_ListItem;
  pItem->m_dwStates = 0;
  pItem->m_wsText = wsAdd;
  pItem->m_dwStates = bSelect ? FWL_ITEMSTATE_LTB_Selected : 0;
  m_ListBoxDP.m_arrItem.Add(pItem);
  return (FWL_HLISTITEM)pItem;
}
FX_BOOL CFWL_ListBox::DeleteString(FWL_HLISTITEM hItem) {
  int32_t nIndex = m_ListBoxDP.GetItemIndex(GetWidget(), hItem);
  if (nIndex < 0 || nIndex >= m_ListBoxDP.m_arrItem.GetSize()) {
    return FALSE;
  }
  CFWL_ListItem* pDelItem =
      reinterpret_cast<CFWL_ListItem*>(m_ListBoxDP.GetItem(m_pIface, nIndex));
  int32_t iCount = m_ListBoxDP.CountItems(m_pIface);
  int32_t iSel = nIndex + 1;
  if (iSel >= iCount) {
    iSel = nIndex - 1;
    if (iSel < 0) {
      iSel = -1;
    }
  }
  if (iSel >= 0) {
    CFWL_ListItem* pSel =
        reinterpret_cast<CFWL_ListItem*>(m_ListBoxDP.GetItem(m_pIface, iSel));
    pSel->m_dwStates |= FWL_ITEMSTATE_LTB_Selected;
  }
  m_ListBoxDP.m_arrItem.RemoveAt(nIndex);
  delete pDelItem;
  return TRUE;
}
FX_BOOL CFWL_ListBox::DeleteAll() {
  int32_t iCount = m_ListBoxDP.CountItems(m_pIface);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListItem* pItem =
        reinterpret_cast<CFWL_ListItem*>(m_ListBoxDP.GetItem(m_pIface, i));
    delete pItem;
  }
  m_ListBoxDP.m_arrItem.RemoveAll();
  return TRUE;
}
int32_t CFWL_ListBox::CountSelItems() {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_ListBox*>(m_pIface)->CountSelItems();
}
FWL_HLISTITEM CFWL_ListBox::GetSelItem(int32_t nIndexSel) {
  if (!m_pIface)
    return NULL;
  return static_cast<IFWL_ListBox*>(m_pIface)->GetSelItem(nIndexSel);
}
int32_t CFWL_ListBox::GetSelIndex(int32_t nIndex) {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_ListBox*>(m_pIface)->GetSelIndex(nIndex);
}
FWL_ERR CFWL_ListBox::SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ListBox*>(m_pIface)->SetSelItem(hItem, bSelect);
}
FWL_ERR CFWL_ListBox::GetItemText(FWL_HLISTITEM hItem, CFX_WideString& wsText) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ListBox*>(m_pIface)->GetItemText(hItem, wsText);
}
FWL_ERR CFWL_ListBox::GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ListBox*>(m_pIface)->GetScrollPos(fPos, bVert);
}
FWL_ERR CFWL_ListBox::SetItemHeight(FX_FLOAT fItemHeight) {
  m_ListBoxDP.m_fItemHeight = fItemHeight;
  return FWL_ERR_Succeeded;
}
FWL_HLISTITEM CFWL_ListBox::GetFocusItem() {
  for (int32_t i = 0; i < m_ListBoxDP.m_arrItem.GetSize(); i++) {
    CFWL_ListItem* hItem =
        static_cast<CFWL_ListItem*>(m_ListBoxDP.m_arrItem[i]);
    if (hItem->m_dwStates & FWL_ITEMSTATE_LTB_Focused) {
      return (FWL_HLISTITEM)hItem;
    }
  }
  return NULL;
}
FWL_ERR CFWL_ListBox::SetFocusItem(FWL_HLISTITEM hItem) {
  int32_t nIndex = m_ListBoxDP.GetItemIndex(GetWidget(), hItem);
  static_cast<CFWL_ListItem*>(m_ListBoxDP.m_arrItem[nIndex])->m_dwStates |=
      FWL_ITEMSTATE_LTB_Focused;
  return FWL_ERR_Succeeded;
}
FWL_ERR* CFWL_ListBox::Sort(IFWL_ListBoxCompare* pCom) {
  return static_cast<IFWL_ListBox*>(m_pIface)->Sort(pCom);
}
int32_t CFWL_ListBox::CountItems() {
  return m_ListBoxDP.m_arrItem.GetSize();
}
FWL_HLISTITEM CFWL_ListBox::GetItem(int32_t nIndex) {
  int32_t nCount = m_ListBoxDP.m_arrItem.GetSize();
  if (nIndex > nCount - 1 && nIndex < 0) {
    return NULL;
  }
  return (FWL_HLISTITEM)m_ListBoxDP.m_arrItem[nIndex];
}
FWL_ERR CFWL_ListBox::SetItemString(FWL_HLISTITEM hItem,
                                    const CFX_WideStringC& wsText) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText = wsText;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::GetItemString(FWL_HLISTITEM hItem,
                                    CFX_WideString& wsText) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  wsText = reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::SetItemData(FWL_HLISTITEM hItem, void* pData) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_pData = pData;
  return FWL_ERR_Succeeded;
}
void* CFWL_ListBox::GetItemData(FWL_HLISTITEM hItem) {
  if (!hItem)
    return NULL;
  return reinterpret_cast<CFWL_ListItem*>(hItem)->m_pData;
}
FWL_HLISTITEM CFWL_ListBox::GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy) {
  CFX_RectF rtClient;
  m_pIface->GetClientRect(rtClient);
  fx -= rtClient.left;
  fy -= rtClient.top;
  FX_FLOAT fPosX = 0;
  FX_FLOAT fPosY = 0;
  static_cast<IFWL_ListBox*>(m_pIface)->GetScrollPos(fx);
  static_cast<IFWL_ListBox*>(m_pIface)->GetScrollPos(fy, FALSE);
  int32_t nCount = m_ListBoxDP.CountItems(NULL);
  for (int32_t i = 0; i < nCount; i++) {
    FWL_HLISTITEM hItem = m_ListBoxDP.GetItem(NULL, i);
    if (!hItem) {
      continue;
    }
    CFX_RectF rtItem;
    m_ListBoxDP.GetItemRect(NULL, hItem, rtItem);
    rtItem.Offset(-fPosX, -fPosY);
    if (rtItem.Contains(fx, fy)) {
      return hItem;
    }
  }
  return NULL;
}
FX_DWORD CFWL_ListBox::GetItemStates(FWL_HLISTITEM hItem) {
  if (!hItem)
    return 0;
  CFWL_ListItem* pItem = reinterpret_cast<CFWL_ListItem*>(hItem);
  return pItem->m_dwStates | pItem->m_dwCheckState;
}
CFWL_ListBox::CFWL_ListBox() {}
CFWL_ListBox::~CFWL_ListBox() {}
CFWL_ListBox::CFWL_ListBoxDP::CFWL_ListBoxDP() {}
CFWL_ListBox::CFWL_ListBoxDP::~CFWL_ListBoxDP() {
  int32_t nCount = m_arrItem.GetSize();
  for (int32_t i = 0; i < nCount; i++) {
    delete static_cast<CFWL_ListItem*>(m_arrItem[i]);
  }
  m_arrItem.RemoveAll();
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetCaption(IFWL_Widget* pWidget,
                                                 CFX_WideString& wsCaption) {
  wsCaption = m_wsData;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_ListBox::CFWL_ListBoxDP::CountItems(IFWL_Widget* pWidget) {
  return m_arrItem.GetSize();
}
FWL_HLISTITEM CFWL_ListBox::CFWL_ListBoxDP::GetItem(IFWL_Widget* pWidget,
                                                    int32_t nIndex) {
  if (nIndex >= m_arrItem.GetSize() || nIndex < 0) {
    return NULL;
  } else {
    return (FWL_HLISTITEM)m_arrItem[nIndex];
  }
}
int32_t CFWL_ListBox::CFWL_ListBoxDP::GetItemIndex(IFWL_Widget* pWidget,
                                                   FWL_HLISTITEM hItem) {
  return m_arrItem.Find(hItem);
}
FX_BOOL CFWL_ListBox::CFWL_ListBoxDP::SetItemIndex(IFWL_Widget* pWidget,
                                                   FWL_HLISTITEM hItem,
                                                   int32_t nIndex) {
  return m_arrItem.SetAt(nIndex, hItem);
}
FX_DWORD CFWL_ListBox::CFWL_ListBoxDP::GetItemStyles(IFWL_Widget* pWidget,
                                                     FWL_HLISTITEM hItem) {
  if (!hItem)
    return -1;
  return reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwStates;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetItemText(IFWL_Widget* pWidget,
                                                  FWL_HLISTITEM hItem,
                                                  CFX_WideString& wsText) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  wsText = reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetItemRect(IFWL_Widget* pWidget,
                                                  FWL_HLISTITEM hItem,
                                                  CFX_RectF& rtItem) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  CFWL_ListItem* pItem = reinterpret_cast<CFWL_ListItem*>(hItem);
  rtItem = pItem->m_rtItem;
  return FWL_ERR_Succeeded;
}
void* CFWL_ListBox::CFWL_ListBoxDP::GetItemData(IFWL_Widget* pWidget,
                                                FWL_HLISTITEM hItem) {
  if (!hItem)
    return NULL;
  CFWL_ListItem* pItem = reinterpret_cast<CFWL_ListItem*>(hItem);
  return pItem->m_pData;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemStyles(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    FX_DWORD dwStyle) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwStates = dwStyle;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemText(IFWL_Widget* pWidget,
                                                  FWL_HLISTITEM hItem,
                                                  const FX_WCHAR* pszText) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText = pszText;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemRect(IFWL_Widget* pWidget,
                                                  FWL_HLISTITEM hItem,
                                                  const CFX_RectF& rtItem) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_rtItem = rtItem;
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_ListBox::CFWL_ListBoxDP::GetItemHeight(IFWL_Widget* pWidget) {
  return m_fItemHeight;
}
CFX_DIBitmap* CFWL_ListBox::CFWL_ListBoxDP::GetItemIcon(IFWL_Widget* pWidget,
                                                        FWL_HLISTITEM hItem) {
  return reinterpret_cast<CFWL_ListItem*>(hItem)->m_pDIB;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::GetItemCheckRect(IFWL_Widget* pWidget,
                                                       FWL_HLISTITEM hItem,
                                                       CFX_RectF& rtCheck) {
  rtCheck = reinterpret_cast<CFWL_ListItem*>(hItem)->m_rtCheckBox;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemCheckRect(
    IFWL_Widget* pWidget,
    FWL_HLISTITEM hItem,
    const CFX_RectF& rtCheck) {
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_rtCheckBox = rtCheck;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ListBox::CFWL_ListBoxDP::GetItemCheckState(IFWL_Widget* pWidget,
                                                         FWL_HLISTITEM hItem) {
  return reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwCheckState;
}
FWL_ERR CFWL_ListBox::CFWL_ListBoxDP::SetItemCheckState(IFWL_Widget* pWidget,
                                                        FWL_HLISTITEM hItem,
                                                        FX_DWORD dwCheckState) {
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwCheckState = dwCheckState;
  return FWL_ERR_Succeeded;
}
