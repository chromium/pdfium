// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_listbox.h"

#include <memory>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

IFWL_ListBox* ToListBox(IFWL_Widget* widget) {
  return static_cast<IFWL_ListBox*>(widget);
}

}  // namespace

CFWL_ListBox::CFWL_ListBox(const IFWL_App* app) : CFWL_Widget(app) {}

CFWL_ListBox::~CFWL_ListBox() {}

void CFWL_ListBox::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_ListBox>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>(this), nullptr);

  CFWL_Widget::Initialize();
}

FWL_Error CFWL_ListBox::AddDIBitmap(CFX_DIBitmap* pDIB, CFWL_ListItem* pItem) {
  static_cast<CFWL_ListItem*>(pItem)->m_pDIB = pDIB;
  return FWL_Error::Succeeded;
}

CFWL_ListItem* CFWL_ListBox::AddString(const CFX_WideStringC& wsAdd,
                                       bool bSelect) {
  std::unique_ptr<CFWL_ListItem> pItem(new CFWL_ListItem);
  pItem->m_dwStates = 0;
  pItem->m_wsText = wsAdd;
  pItem->m_dwStates = bSelect ? FWL_ITEMSTATE_LTB_Selected : 0;
  m_ItemArray.push_back(std::move(pItem));
  return m_ItemArray.back().get();
}

bool CFWL_ListBox::DeleteString(CFWL_ListItem* pItem) {
  int32_t nIndex = GetItemIndex(GetWidget(), pItem);
  if (nIndex < 0 || static_cast<size_t>(nIndex) >= m_ItemArray.size()) {
    return false;
  }
  int32_t iCount = CountItems(m_pIface.get());
  int32_t iSel = nIndex + 1;
  if (iSel >= iCount) {
    iSel = nIndex - 1;
    if (iSel < 0) {
      iSel = -1;
    }
  }
  if (iSel >= 0) {
    CFWL_ListItem* pSel =
        static_cast<CFWL_ListItem*>(GetItem(m_pIface.get(), iSel));
    pSel->m_dwStates |= FWL_ITEMSTATE_LTB_Selected;
  }
  m_ItemArray.erase(m_ItemArray.begin() + nIndex);
  return true;
}

void CFWL_ListBox::DeleteAll() {
  m_ItemArray.clear();
}

int32_t CFWL_ListBox::CountSelItems() {
  if (!GetWidget())
    return 0;
  return ToListBox(GetWidget())->CountSelItems();
}

CFWL_ListItem* CFWL_ListBox::GetSelItem(int32_t nIndexSel) {
  if (!GetWidget())
    return nullptr;
  return ToListBox(GetWidget())->GetSelItem(nIndexSel);
}

int32_t CFWL_ListBox::GetSelIndex(int32_t nIndex) {
  if (!GetWidget())
    return 0;
  return ToListBox(GetWidget())->GetSelIndex(nIndex);
}

void CFWL_ListBox::SetSelItem(CFWL_ListItem* pItem, bool bSelect) {
  if (GetWidget())
    ToListBox(GetWidget())->SetSelItem(pItem, bSelect);
}

void CFWL_ListBox::GetItemText(CFWL_ListItem* pItem, CFX_WideString& wsText) {
  if (GetWidget())
    ToListBox(GetWidget())->GetItemText(pItem, wsText);
}

void CFWL_ListBox::GetScrollPos(FX_FLOAT& fPos, bool bVert) {
  if (GetWidget())
    ToListBox(GetWidget())->GetScrollPos(fPos, bVert);
}

FWL_Error CFWL_ListBox::SetItemHeight(FX_FLOAT fItemHeight) {
  m_fItemHeight = fItemHeight;
  return FWL_Error::Succeeded;
}

CFWL_ListItem* CFWL_ListBox::GetFocusItem() {
  for (const auto& pItem : m_ItemArray) {
    if (pItem->m_dwStates & FWL_ITEMSTATE_LTB_Focused)
      return pItem.get();
  }
  return nullptr;
}

FWL_Error CFWL_ListBox::SetFocusItem(CFWL_ListItem* pItem) {
  int32_t nIndex = GetItemIndex(GetWidget(), pItem);
  m_ItemArray[nIndex]->m_dwStates |= FWL_ITEMSTATE_LTB_Focused;
  return FWL_Error::Succeeded;
}

int32_t CFWL_ListBox::CountItems() {
  return pdfium::CollectionSize<int32_t>(m_ItemArray);
}

CFWL_ListItem* CFWL_ListBox::GetItem(int32_t nIndex) {
  if (nIndex < 0 || nIndex >= CountItems())
    return nullptr;

  return m_ItemArray[nIndex].get();
}

FWL_Error CFWL_ListBox::SetItemString(CFWL_ListItem* pItem,
                                      const CFX_WideStringC& wsText) {
  if (!pItem)
    return FWL_Error::Indefinite;
  static_cast<CFWL_ListItem*>(pItem)->m_wsText = wsText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::GetItemString(CFWL_ListItem* pItem,
                                      CFX_WideString& wsText) {
  if (!pItem)
    return FWL_Error::Indefinite;
  wsText = static_cast<CFWL_ListItem*>(pItem)->m_wsText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ListBox::SetItemData(CFWL_ListItem* pItem, void* pData) {
  if (!pItem)
    return FWL_Error::Indefinite;
  static_cast<CFWL_ListItem*>(pItem)->m_pData = pData;
  return FWL_Error::Succeeded;
}

void* CFWL_ListBox::GetItemData(CFWL_ListItem* pItem) {
  return pItem ? static_cast<CFWL_ListItem*>(pItem)->m_pData : nullptr;
}

CFWL_ListItem* CFWL_ListBox::GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy) {
  CFX_RectF rtClient;
  GetWidget()->GetClientRect(rtClient);
  fx -= rtClient.left;
  fy -= rtClient.top;
  FX_FLOAT fPosX = 0;
  FX_FLOAT fPosY = 0;
  ToListBox(GetWidget())->GetScrollPos(fx);
  ToListBox(GetWidget())->GetScrollPos(fy, false);
  int32_t nCount = CountItems(nullptr);
  for (int32_t i = 0; i < nCount; i++) {
    CFWL_ListItem* pItem = GetItem(nullptr, i);
    if (!pItem) {
      continue;
    }
    CFX_RectF rtItem;
    GetItemRect(nullptr, pItem, rtItem);
    rtItem.Offset(-fPosX, -fPosY);
    if (rtItem.Contains(fx, fy)) {
      return pItem;
    }
  }
  return nullptr;
}

uint32_t CFWL_ListBox::GetItemStates(CFWL_ListItem* pItem) {
  if (!pItem)
    return 0;
  CFWL_ListItem* pListItem = static_cast<CFWL_ListItem*>(pItem);
  return pListItem->m_dwStates | pListItem->m_dwCheckState;
}

FWL_Error CFWL_ListBox::GetCaption(IFWL_Widget* pWidget,
                                   CFX_WideString& wsCaption) {
  wsCaption = m_wsData;
  return FWL_Error::Succeeded;
}

int32_t CFWL_ListBox::CountItems(const IFWL_Widget* pWidget) {
  return pdfium::CollectionSize<int32_t>(m_ItemArray);
}

CFWL_ListItem* CFWL_ListBox::GetItem(const IFWL_Widget* pWidget,
                                     int32_t nIndex) {
  if (nIndex < 0 || nIndex >= CountItems(pWidget))
    return nullptr;

  return m_ItemArray[nIndex].get();
}

int32_t CFWL_ListBox::GetItemIndex(IFWL_Widget* pWidget, CFWL_ListItem* pItem) {
  auto it = std::find_if(
      m_ItemArray.begin(), m_ItemArray.end(),
      [pItem](const std::unique_ptr<CFWL_ListItem>& candidate) {
        return candidate.get() == static_cast<CFWL_ListItem*>(pItem);
      });
  return it != m_ItemArray.end() ? it - m_ItemArray.begin() : -1;
}

bool CFWL_ListBox::SetItemIndex(IFWL_Widget* pWidget,
                                CFWL_ListItem* pItem,
                                int32_t nIndex) {
  if (nIndex < 0 || nIndex >= CountItems(pWidget))
    return false;
  m_ItemArray[nIndex].reset(static_cast<CFWL_ListItem*>(pItem));
  return true;
}

uint32_t CFWL_ListBox::GetItemStyles(IFWL_Widget* pWidget,
                                     CFWL_ListItem* pItem) {
  if (!pItem)
    return 0;
  return static_cast<CFWL_ListItem*>(pItem)->m_dwStates;
}

void CFWL_ListBox::GetItemText(IFWL_Widget* pWidget,
                               CFWL_ListItem* pItem,
                               CFX_WideString& wsText) {
  if (pItem)
    wsText = static_cast<CFWL_ListItem*>(pItem)->m_wsText;
}

void CFWL_ListBox::GetItemRect(IFWL_Widget* pWidget,
                               CFWL_ListItem* pItem,
                               CFX_RectF& rtItem) {
  if (pItem)
    rtItem = static_cast<CFWL_ListItem*>(pItem)->m_rtItem;
}

void* CFWL_ListBox::GetItemData(IFWL_Widget* pWidget, CFWL_ListItem* pItem) {
  return pItem ? static_cast<CFWL_ListItem*>(pItem)->m_pData : nullptr;
}

void CFWL_ListBox::SetItemStyles(IFWL_Widget* pWidget,
                                 CFWL_ListItem* pItem,
                                 uint32_t dwStyle) {
  if (pItem)
    static_cast<CFWL_ListItem*>(pItem)->m_dwStates = dwStyle;
}

void CFWL_ListBox::SetItemText(IFWL_Widget* pWidget,
                               CFWL_ListItem* pItem,
                               const FX_WCHAR* pszText) {
  if (pItem)
    static_cast<CFWL_ListItem*>(pItem)->m_wsText = pszText;
}

void CFWL_ListBox::SetItemRect(IFWL_Widget* pWidget,
                               CFWL_ListItem* pItem,
                               const CFX_RectF& rtItem) {
  if (pItem)
    static_cast<CFWL_ListItem*>(pItem)->m_rtItem = rtItem;
}

FX_FLOAT CFWL_ListBox::GetItemHeight(IFWL_Widget* pWidget) {
  return m_fItemHeight;
}

CFX_DIBitmap* CFWL_ListBox::GetItemIcon(IFWL_Widget* pWidget,
                                        CFWL_ListItem* pItem) {
  return static_cast<CFWL_ListItem*>(pItem)->m_pDIB;
}

void CFWL_ListBox::GetItemCheckRect(IFWL_Widget* pWidget,
                                    CFWL_ListItem* pItem,
                                    CFX_RectF& rtCheck) {
  rtCheck = static_cast<CFWL_ListItem*>(pItem)->m_rtCheckBox;
}

void CFWL_ListBox::SetItemCheckRect(IFWL_Widget* pWidget,
                                    CFWL_ListItem* pItem,
                                    const CFX_RectF& rtCheck) {
  static_cast<CFWL_ListItem*>(pItem)->m_rtCheckBox = rtCheck;
}

uint32_t CFWL_ListBox::GetItemCheckState(IFWL_Widget* pWidget,
                                         CFWL_ListItem* pItem) {
  return static_cast<CFWL_ListItem*>(pItem)->m_dwCheckState;
}

void CFWL_ListBox::SetItemCheckState(IFWL_Widget* pWidget,
                                     CFWL_ListItem* pItem,
                                     uint32_t dwCheckState) {
  static_cast<CFWL_ListItem*>(pItem)->m_dwCheckState = dwCheckState;
}
