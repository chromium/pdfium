// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "xfa/src/foxitlib.h"

CFWL_ComboBox* CFWL_ComboBox::Create() {
  return new CFWL_ComboBox;
}
FWL_ERR CFWL_ComboBox::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_ComboBox> pComboBox(IFWL_ComboBox::Create(
      m_pProperties->MakeWidgetImpProperties(&m_comboBoxData)));
  FWL_ERR ret = pComboBox->Initialize();
  if (ret != FWL_ERR_Succeeded) {
    return ret;
  }
  m_pIface = pComboBox.release();
  CFWL_Widget::Initialize();
  return FWL_ERR_Succeeded;
}
int32_t CFWL_ComboBox::AddString(const CFX_WideStringC& wsText) {
  CFWL_ComboBoxItem* pItem = new CFWL_ComboBoxItem;
  pItem->m_wsText = wsText;
  pItem->m_dwStyles = 0;
  return m_comboBoxData.m_arrItem.Add(pItem);
}
int32_t CFWL_ComboBox::AddString(const CFX_WideStringC& wsText,
                                 CFX_DIBitmap* pIcon) {
  CFWL_ComboBoxItem* pItem = new CFWL_ComboBoxItem;
  pItem->m_wsText = wsText;
  pItem->m_dwStyles = 0;
  pItem->m_pDIB = pIcon;
  return m_comboBoxData.m_arrItem.Add(pItem);
}
int32_t CFWL_ComboBox::RemoveAt(int32_t iIndex) {
  return m_comboBoxData.m_arrItem.RemoveAt(iIndex);
}
int32_t CFWL_ComboBox::RemoveAll() {
  m_comboBoxData.m_arrItem.RemoveAll();
  return 0;
}
int32_t CFWL_ComboBox::CountItems() {
  return m_comboBoxData.CountItems(GetWidget());
}
FWL_ERR CFWL_ComboBox::GetTextByIndex(int32_t iIndex, CFX_WideString& wsText) {
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(
      m_comboBoxData.GetItem(m_pIface, iIndex));
  if (!pItem)
    return FWL_ERR_Indefinite;
  wsText = pItem->m_wsText;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_ComboBox::GetCurSel() {
  if (!m_pIface)
    return -1;
  return static_cast<IFWL_ComboBox*>(m_pIface)->GetCurSel();
}
FWL_ERR CFWL_ComboBox::SetCurSel(int32_t iSel) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ComboBox*>(m_pIface)->SetCurSel(iSel);
}
FWL_ERR CFWL_ComboBox::SetEditText(const CFX_WideStringC& wsText) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ComboBox*>(m_pIface)->SetEditText(wsText);
}
int32_t CFWL_ComboBox::GetEditTextLength() const {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_ComboBox*>(m_pIface)->GetEditTextLength();
}
FWL_ERR CFWL_ComboBox::GetEditText(CFX_WideString& wsText,
                                   int32_t nStart,
                                   int32_t nCount) const {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ComboBox*>(m_pIface)
      ->GetEditText(wsText, nStart, nCount);
}
FWL_ERR CFWL_ComboBox::SetEditSelRange(int32_t nStart, int32_t nCount) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ComboBox*>(m_pIface)->SetEditSelRange(nStart, nCount);
}
int32_t CFWL_ComboBox::GetEditSelRange(int32_t nIndex, int32_t& nStart) {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_ComboBox*>(m_pIface)->GetEditSelRange(nIndex, nStart);
}
int32_t CFWL_ComboBox::GetEditLimit() {
  if (!m_pIface)
    return 0;
  return static_cast<IFWL_ComboBox*>(m_pIface)->GetEditLimit();
}
FWL_ERR CFWL_ComboBox::SetEditLimit(int32_t nLimit) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ComboBox*>(m_pIface)->SetEditLimit(nLimit);
}
FWL_ERR CFWL_ComboBox::EditDoClipboard(int32_t iCmd) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditDoClipboard(iCmd);
}
FX_BOOL CFWL_ComboBox::EditRedo(const CFX_ByteStringC& bsRecord) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditRedo(bsRecord);
}
FX_BOOL CFWL_ComboBox::EditUndo(const CFX_ByteStringC& bsRecord) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditUndo(bsRecord);
}
FWL_ERR CFWL_ComboBox::SetMaxListHeight(FX_FLOAT fMaxHeight) {
  m_comboBoxData.m_fMaxListHeight = fMaxHeight;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::SetItemData(int32_t iIndex, void* pData) {
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(
      m_comboBoxData.GetItem(m_pIface, iIndex));
  if (!pItem)
    return FWL_ERR_Indefinite;
  pItem->m_pData = pData;
  return FWL_ERR_Succeeded;
}
void* CFWL_ComboBox::GetItemData(int32_t iIndex) {
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(
      m_comboBoxData.GetItem(m_pIface, iIndex));
  if (!pItem)
    return NULL;
  return pItem->m_pData;
}
FWL_ERR CFWL_ComboBox::SetListTheme(IFWL_ThemeProvider* pTheme) {
  return static_cast<IFWL_ComboBox*>(m_pIface)->GetListBoxt()->SetThemeProvider(
      pTheme);
}
FX_BOOL CFWL_ComboBox::AfterFocusShowDropList() {
  return static_cast<IFWL_ComboBox*>(m_pIface)->AfterFocusShowDropList();
}
FWL_ERR CFWL_ComboBox::OpenDropDownList(FX_BOOL bActivate) {
  return static_cast<IFWL_ComboBox*>(m_pIface)->OpenDropDownList(bActivate);
}
FX_BOOL CFWL_ComboBox::EditCanUndo() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditCanUndo();
}
FX_BOOL CFWL_ComboBox::EditCanRedo() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditCanRedo();
}
FX_BOOL CFWL_ComboBox::EditUndo() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditUndo();
}
FX_BOOL CFWL_ComboBox::EditRedo() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditRedo();
}
FX_BOOL CFWL_ComboBox::EditCanCopy() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditCanCopy();
}
FX_BOOL CFWL_ComboBox::EditCanCut() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditCanCut();
}
FX_BOOL CFWL_ComboBox::EditCanSelectAll() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditCanSelectAll();
}
FX_BOOL CFWL_ComboBox::EditCopy(CFX_WideString& wsCopy) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditCopy(wsCopy);
}
FX_BOOL CFWL_ComboBox::EditCut(CFX_WideString& wsCut) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditCut(wsCut);
}
FX_BOOL CFWL_ComboBox::EditPaste(const CFX_WideString& wsPaste) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditPaste(wsPaste);
}
FX_BOOL CFWL_ComboBox::EditSelectAll() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditSelectAll();
}
FX_BOOL CFWL_ComboBox::EditDelete() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditDelete();
}
FX_BOOL CFWL_ComboBox::EditDeSelect() {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->EditDeSelect();
}
FWL_ERR CFWL_ComboBox::GetBBox(CFX_RectF& rect) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)->GetBBox(rect);
}
FWL_ERR CFWL_ComboBox::EditModifyStylesEx(FX_DWORD dwStylesExAdded,
                                          FX_DWORD dwStylesExRemoved) {
  if (!m_pIface)
    return FALSE;
  return static_cast<IFWL_ComboBox*>(m_pIface)
      ->EditModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
CFWL_ComboBox::CFWL_ComboBox() {}
CFWL_ComboBox::~CFWL_ComboBox() {}
CFWL_ComboBox::CFWL_ComboBoxDP::CFWL_ComboBoxDP() {
  m_fItemHeight = 0;
  m_fMaxListHeight = 0;
}
CFWL_ComboBox::CFWL_ComboBoxDP::~CFWL_ComboBoxDP() {
  int32_t nCount = m_arrItem.GetSize();
  for (int32_t i = 0; i < nCount; i++) {
    delete static_cast<CFWL_ComboBoxItem*>(m_arrItem[i]);
  }
  m_arrItem.RemoveAll();
}
int32_t CFWL_ComboBox::CFWL_ComboBoxDP::CountItems(IFWL_Widget* pWidget) {
  return m_arrItem.GetSize();
}
FWL_HLISTITEM CFWL_ComboBox::CFWL_ComboBoxDP::GetItem(IFWL_Widget* pWidget,
                                                      int32_t nIndex) {
  int32_t iCount = m_arrItem.GetSize();
  if (nIndex >= iCount || nIndex < 0) {
    return NULL;
  }
  return (FWL_HLISTITEM)m_arrItem[nIndex];
}
int32_t CFWL_ComboBox::CFWL_ComboBoxDP::GetItemIndex(IFWL_Widget* pWidget,
                                                     FWL_HLISTITEM hItem) {
  return m_arrItem.Find(hItem);
}
FX_BOOL CFWL_ComboBox::CFWL_ComboBoxDP::SetItemIndex(IFWL_Widget* pWidget,
                                                     FWL_HLISTITEM hItem,
                                                     int32_t nIndex) {
  return m_arrItem.SetAt(nIndex, hItem);
}
FX_DWORD CFWL_ComboBox::CFWL_ComboBoxDP::GetItemStyles(IFWL_Widget* pWidget,
                                                       FWL_HLISTITEM hItem) {
  if (!hItem)
    return 0;
  return reinterpret_cast<CFWL_ComboBoxItem*>(hItem)->m_dwStyles;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::GetItemText(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    CFX_WideString& wsText) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  wsText = reinterpret_cast<CFWL_ComboBoxItem*>(hItem)->m_wsText;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::GetItemRect(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    CFX_RectF& rtItem) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(hItem);
  rtItem.Set(pItem->m_rtItem.left, pItem->m_rtItem.top, pItem->m_rtItem.width,
             pItem->m_rtItem.height);
  return FWL_ERR_Succeeded;
}
void* CFWL_ComboBox::CFWL_ComboBoxDP::GetItemData(IFWL_Widget* pWidget,
                                                  FWL_HLISTITEM hItem) {
  if (!hItem)
    return NULL;
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(hItem);
  return pItem->m_pData;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemStyles(IFWL_Widget* pWidget,
                                                      FWL_HLISTITEM hItem,
                                                      FX_DWORD dwStyle) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ComboBoxItem*>(hItem)->m_dwStyles = dwStyle;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemText(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    const FX_WCHAR* pszText) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ComboBoxItem*>(hItem)->m_wsText = pszText;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemRect(IFWL_Widget* pWidget,
                                                    FWL_HLISTITEM hItem,
                                                    const CFX_RectF& rtItem) {
  if (!hItem)
    return FWL_ERR_Indefinite;
  reinterpret_cast<CFWL_ComboBoxItem*>(hItem)->m_rtItem = rtItem;
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_ComboBox::CFWL_ComboBoxDP::GetItemHeight(IFWL_Widget* pWidget) {
  return m_fItemHeight;
}
CFX_DIBitmap* CFWL_ComboBox::CFWL_ComboBoxDP::GetItemIcon(IFWL_Widget* pWidget,
                                                          FWL_HLISTITEM hItem) {
  if (!hItem)
    return NULL;
  return reinterpret_cast<CFWL_ComboBoxItem*>(hItem)->m_pDIB;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::GetItemCheckRect(IFWL_Widget* pWidget,
                                                         FWL_HLISTITEM hItem,
                                                         CFX_RectF& rtCheck) {
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(hItem);
  rtCheck = pItem->m_rtCheckBox;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemCheckRect(
    IFWL_Widget* pWidget,
    FWL_HLISTITEM hItem,
    const CFX_RectF& rtCheck) {
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(hItem);
  pItem->m_rtCheckBox = rtCheck;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ComboBox::CFWL_ComboBoxDP::GetItemCheckState(
    IFWL_Widget* pWidget,
    FWL_HLISTITEM hItem) {
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(hItem);
  return pItem->m_dwCheckState;
}
FWL_ERR CFWL_ComboBox::CFWL_ComboBoxDP::SetItemCheckState(
    IFWL_Widget* pWidget,
    FWL_HLISTITEM hItem,
    FX_DWORD dwCheckState) {
  CFWL_ComboBoxItem* pItem = reinterpret_cast<CFWL_ComboBoxItem*>(hItem);
  pItem->m_dwCheckState = dwCheckState;
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_ComboBox::CFWL_ComboBoxDP::GetListHeight(IFWL_Widget* pWidget) {
  return m_fMaxListHeight;
}
