// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/lightwidget/cfwl_listbox.h"

#include <memory>

#include "third_party/base/stl_util.h"

CFWL_ListBox* CFWL_ListBox::Create() {
  return new CFWL_ListBox;
}

FWL_Error CFWL_ListBox::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_Error::Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_ListBox> pListBox(IFWL_ListBox::Create(
      m_pProperties->MakeWidgetImpProperties(&m_ListBoxDP), nullptr));
  FWL_Error ret = pListBox->Initialize();
  if (ret != FWL_Error::Succeeded) {
    return ret;
  }
  m_pIface = pListBox.release();
  CFWL_Widget::Initialize();
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::AddDIBitmap(CFX_DIBitmap* pDIB, FWL_HLISTITEM hItem) {
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_pDIB = pDIB;
  return FWL_Error::Succeeded;
}

FWL_HLISTITEM CFWL_ListBox::AddString(const CFX_WideStringC& wsAdd,
                                      FX_BOOL bSelect) {
  std::unique_ptr<CFWL_ListItem> pItem(new CFWL_ListItem);
  pItem->m_dwStates = 0;
  pItem->m_wsText = wsAdd;
  pItem->m_dwStates = bSelect ? FWL_ITEMSTATE_LTB_Selected : 0;
  m_ListBoxDP.m_ItemArray.push_back(std::move(pItem));
  return (FWL_HLISTITEM)m_ListBoxDP.m_ItemArray.back().get();
}

FX_BOOL CFWL_ListBox::DeleteString(FWL_HLISTITEM hItem) {
  int32_t nIndex = m_ListBoxDP.GetItemIndex(GetWidget(), hItem);
  if (nIndex < 0 ||
      static_cast<size_t>(nIndex) >= m_ListBoxDP.m_ItemArray.size()) {
    return FALSE;
  }
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
  m_ListBoxDP.m_ItemArray.erase(m_ListBoxDP.m_ItemArray.begin() + nIndex);
  return TRUE;
}

void CFWL_ListBox::DeleteAll() {
  m_ListBoxDP.m_ItemArray.clear();
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

FWL_Error CFWL_ListBox::SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return static_cast<IFWL_ListBox*>(m_pIface)->SetSelItem(hItem, bSelect);
}

FWL_Error CFWL_ListBox::GetItemText(FWL_HLISTITEM hItem,
                                    CFX_WideString& wsText) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return static_cast<IFWL_ListBox*>(m_pIface)->GetItemText(hItem, wsText);
}

FWL_Error CFWL_ListBox::GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert) {
  if (!m_pIface)
    return FWL_Error::Indefinite;
  return static_cast<IFWL_ListBox*>(m_pIface)->GetScrollPos(fPos, bVert);
}

FWL_Error CFWL_ListBox::SetItemHeight(FX_FLOAT fItemHeight) {
  m_ListBoxDP.m_fItemHeight = fItemHeight;
  return FWL_Error::Succeeded;
}

FWL_HLISTITEM CFWL_ListBox::GetFocusItem() {
  for (const auto& hItem : m_ListBoxDP.m_ItemArray) {
    if (hItem->m_dwStates & FWL_ITEMSTATE_LTB_Focused)
      return (FWL_HLISTITEM)hItem.get();
  }
  return nullptr;
}

FWL_Error CFWL_ListBox::SetFocusItem(FWL_HLISTITEM hItem) {
  int32_t nIndex = m_ListBoxDP.GetItemIndex(GetWidget(), hItem);
  m_ListBoxDP.m_ItemArray[nIndex]->m_dwStates |= FWL_ITEMSTATE_LTB_Focused;
  return FWL_Error::Succeeded;
}

int32_t CFWL_ListBox::CountItems() {
  return pdfium::CollectionSize<int32_t>(m_ListBoxDP.m_ItemArray);
}

FWL_HLISTITEM CFWL_ListBox::GetItem(int32_t nIndex) {
  if (nIndex < 0 || nIndex >= CountItems())
    return nullptr;

  return (FWL_HLISTITEM)m_ListBoxDP.m_ItemArray[nIndex].get();
}

FWL_Error CFWL_ListBox::SetItemString(FWL_HLISTITEM hItem,
                                      const CFX_WideStringC& wsText) {
  if (!hItem)
    return FWL_Error::Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText = wsText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::GetItemString(FWL_HLISTITEM hItem,
                                      CFX_WideString& wsText) {
  if (!hItem)
    return FWL_Error::Indefinite;
  wsText = reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::SetItemData(FWL_HLISTITEM hItem, void* pData) {
  if (!hItem)
    return FWL_Error::Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_pData = pData;
  return FWL_Error::Succeeded;
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

uint32_t CFWL_ListBox::GetItemStates(FWL_HLISTITEM hItem) {
  if (!hItem)
    return 0;
  CFWL_ListItem* pItem = reinterpret_cast<CFWL_ListItem*>(hItem);
  return pItem->m_dwStates | pItem->m_dwCheckState;
}

CFWL_ListBox::CFWL_ListBox() {}

CFWL_ListBox::~CFWL_ListBox() {}

CFWL_ListBox::CFWL_ListBoxDP::CFWL_ListBoxDP() {}

CFWL_ListBox::CFWL_ListBoxDP::~CFWL_ListBoxDP() {}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::GetCaption(IFWL_Widget* pWidget,
                                                   CFX_WideString& wsCaption) {
  wsCaption = m_wsData;
  return FWL_Error::Succeeded;
}

int32_t CFWL_ListBox::CFWL_ListBoxDP::CountItems(IFWL_Widget* pWidget) {
  return pdfium::CollectionSize<int32_t>(m_ItemArray);
}

FWL_HLISTITEM CFWL_ListBox::CFWL_ListBoxDP::GetItem(IFWL_Widget* pWidget,
                                                    int32_t nIndex) {
  if (nIndex < 0 || nIndex >= CountItems(pWidget))
    return nullptr;

  return (FWL_HLISTITEM)m_ItemArray[nIndex].get();
}

int32_t CFWL_ListBox::CFWL_ListBoxDP::GetItemIndex(IFWL_Widget* pWidget,
                                                   FWL_HLISTITEM hItem) {
  auto it = std::find_if(
      m_ItemArray.begin(), m_ItemArray.end(),
      [hItem](const std::unique_ptr<CFWL_ListItem>& candidate) {
        return candidate.get() == reinterpret_cast<CFWL_ListItem*>(hItem);
      });
  return it != m_ItemArray.end() ? it - m_ItemArray.begin() : -1;
}

FX_BOOL CFWL_ListBox::CFWL_ListBoxDP::SetItemIndex(IFWL_Widget* pWidget,
                                                   FWL_HLISTITEM hItem,
                                                   int32_t nIndex) {
  if (nIndex < 0 || nIndex >= CountItems(pWidget))
    return FALSE;
  m_ItemArray[nIndex].reset(reinterpret_cast<CFWL_ListItem*>(hItem));
  return TRUE;
}

uint32_t CFWL_ListBox::CFWL_ListBoxDP::GetItemStyles(IFWL_Widget* pWidget,
                                                     FWL_HLISTITEM hItem) {
  if (!hItem)
    return -1;
  return reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwStates;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::GetItemText(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    CFX_WideString& wsText) {
  if (!hItem)
    return FWL_Error::Indefinite;
  wsText = reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::GetItemRect(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    CFX_RectF& rtItem) {
  if (!hItem)
    return FWL_Error::Indefinite;
  CFWL_ListItem* pItem = reinterpret_cast<CFWL_ListItem*>(hItem);
  rtItem = pItem->m_rtItem;
  return FWL_Error::Succeeded;
}

void* CFWL_ListBox::CFWL_ListBoxDP::GetItemData(IFWL_Widget* pWidget,
                                                FWL_HLISTITEM hItem) {
  if (!hItem)
    return NULL;
  CFWL_ListItem* pItem = reinterpret_cast<CFWL_ListItem*>(hItem);
  return pItem->m_pData;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::SetItemStyles(IFWL_Widget* pWidget,
                                                      FWL_HLISTITEM hItem,
                                                      uint32_t dwStyle) {
  if (!hItem)
    return FWL_Error::Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwStates = dwStyle;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::SetItemText(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    const FX_WCHAR* pszText) {
  if (!hItem)
    return FWL_Error::Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_wsText = pszText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::SetItemRect(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    const CFX_RectF& rtItem) {
  if (!hItem)
    return FWL_Error::Indefinite;
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_rtItem = rtItem;
  return FWL_Error::Succeeded;
}

FX_FLOAT CFWL_ListBox::CFWL_ListBoxDP::GetItemHeight(IFWL_Widget* pWidget) {
  return m_fItemHeight;
}

CFX_DIBitmap* CFWL_ListBox::CFWL_ListBoxDP::GetItemIcon(IFWL_Widget* pWidget,
                                                        FWL_HLISTITEM hItem) {
  return reinterpret_cast<CFWL_ListItem*>(hItem)->m_pDIB;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::GetItemCheckRect(IFWL_Widget* pWidget,
                                                         FWL_HLISTITEM hItem,
                                                         CFX_RectF& rtCheck) {
  rtCheck = reinterpret_cast<CFWL_ListItem*>(hItem)->m_rtCheckBox;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::SetItemCheckRect(
    IFWL_Widget* pWidget,
    FWL_HLISTITEM hItem,
    const CFX_RectF& rtCheck) {
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_rtCheckBox = rtCheck;
  return FWL_Error::Succeeded;
}

uint32_t CFWL_ListBox::CFWL_ListBoxDP::GetItemCheckState(IFWL_Widget* pWidget,
                                                         FWL_HLISTITEM hItem) {
  return reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwCheckState;
}

FWL_Error CFWL_ListBox::CFWL_ListBoxDP::SetItemCheckState(
    IFWL_Widget* pWidget,
    FWL_HLISTITEM hItem,
    uint32_t dwCheckState) {
  reinterpret_cast<CFWL_ListItem*>(hItem)->m_dwCheckState = dwCheckState;
  return FWL_Error::Succeeded;
}
