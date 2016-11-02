// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_combobox.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_widget.h"

namespace {

IFWL_ComboBox* ToComboBox(IFWL_Widget* widget) {
  return static_cast<IFWL_ComboBox*>(widget);
}

const IFWL_ComboBox* ToComboBox(const IFWL_Widget* widget) {
  return static_cast<const IFWL_ComboBox*>(widget);
}

}  // namespace

CFWL_ComboBox::CFWL_ComboBox(const IFWL_App* app) : CFWL_Widget(app) {}

CFWL_ComboBox::~CFWL_ComboBox() {}

void CFWL_ComboBox::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_ComboBox>(
      m_pApp, m_pProperties->MakeWidgetImpProperties(&m_comboBoxData));

  CFWL_Widget::Initialize();
}

int32_t CFWL_ComboBox::AddString(const CFX_WideStringC& wsText) {
  std::unique_ptr<CFWL_ComboBoxItem> pItem(new CFWL_ComboBoxItem);
  pItem->m_wsText = wsText;
  pItem->m_dwStyles = 0;
  m_comboBoxData.m_ItemArray.push_back(std::move(pItem));
  return m_comboBoxData.m_ItemArray.size() - 1;
}

int32_t CFWL_ComboBox::AddString(const CFX_WideStringC& wsText,
                                 CFX_DIBitmap* pIcon) {
  std::unique_ptr<CFWL_ComboBoxItem> pItem(new CFWL_ComboBoxItem);
  pItem->m_wsText = wsText;
  pItem->m_dwStyles = 0;
  pItem->m_pDIB = pIcon;
  m_comboBoxData.m_ItemArray.push_back(std::move(pItem));
  return m_comboBoxData.m_ItemArray.size() - 1;
}

bool CFWL_ComboBox::RemoveAt(int32_t iIndex) {
  if (iIndex < 0 ||
      static_cast<size_t>(iIndex) >= m_comboBoxData.m_ItemArray.size()) {
    return false;
  }
  m_comboBoxData.m_ItemArray.erase(m_comboBoxData.m_ItemArray.begin() + iIndex);
  return true;
}

void CFWL_ComboBox::RemoveAll() {
  m_comboBoxData.m_ItemArray.clear();
}

int32_t CFWL_ComboBox::CountItems() {
  return m_comboBoxData.CountItems(GetWidget());
}

FWL_Error CFWL_ComboBox::GetTextByIndex(int32_t iIndex,
                                        CFX_WideString& wsText) {
  CFWL_ComboBoxItem* pItem = static_cast<CFWL_ComboBoxItem*>(
      m_comboBoxData.GetItem(m_pIface.get(), iIndex));
  if (!pItem)
    return FWL_Error::Indefinite;
  wsText = pItem->m_wsText;
  return FWL_Error::Succeeded;
}

int32_t CFWL_ComboBox::GetCurSel() {
  return GetWidget() ? ToComboBox(GetWidget())->GetCurSel() : -1;
}

FWL_Error CFWL_ComboBox::SetCurSel(int32_t iSel) {
  return GetWidget() ? ToComboBox(GetWidget())->SetCurSel(iSel)
                     : FWL_Error::Indefinite;
}

FWL_Error CFWL_ComboBox::SetEditText(const CFX_WideString& wsText) {
  return GetWidget() ? ToComboBox(GetWidget())->SetEditText(wsText)
                     : FWL_Error::Indefinite;
}

int32_t CFWL_ComboBox::GetEditTextLength() const {
  return GetWidget() ? ToComboBox(GetWidget())->GetEditTextLength() : 0;
}

FWL_Error CFWL_ComboBox::GetEditText(CFX_WideString& wsText,
                                     int32_t nStart,
                                     int32_t nCount) const {
  return GetWidget()
             ? ToComboBox(GetWidget())->GetEditText(wsText, nStart, nCount)
             : FWL_Error::Indefinite;
}

FWL_Error CFWL_ComboBox::SetEditSelRange(int32_t nStart, int32_t nCount) {
  return GetWidget() ? ToComboBox(GetWidget())->SetEditSelRange(nStart, nCount)
                     : FWL_Error::Indefinite;
}

int32_t CFWL_ComboBox::GetEditSelRange(int32_t nIndex, int32_t& nStart) {
  return GetWidget() ? ToComboBox(GetWidget())->GetEditSelRange(nIndex, nStart)
                     : 0;
}

int32_t CFWL_ComboBox::GetEditLimit() {
  return GetWidget() ? ToComboBox(GetWidget())->GetEditLimit() : 0;
}

FWL_Error CFWL_ComboBox::SetEditLimit(int32_t nLimit) {
  return GetWidget() ? ToComboBox(GetWidget())->SetEditLimit(nLimit)
                     : FWL_Error::Indefinite;
}

FWL_Error CFWL_ComboBox::EditDoClipboard(int32_t iCmd) {
  return GetWidget() ? ToComboBox(GetWidget())->EditDoClipboard(iCmd)
                     : FWL_Error::Indefinite;
}

bool CFWL_ComboBox::EditRedo(const IFDE_TxtEdtDoRecord* pRecord) {
  return GetWidget() ? ToComboBox(GetWidget())->EditRedo(pRecord) : false;
}

bool CFWL_ComboBox::EditUndo(const IFDE_TxtEdtDoRecord* pRecord) {
  return GetWidget() ? ToComboBox(GetWidget())->EditUndo(pRecord) : false;
}

FWL_Error CFWL_ComboBox::SetMaxListHeight(FX_FLOAT fMaxHeight) {
  m_comboBoxData.m_fMaxListHeight = fMaxHeight;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ComboBox::SetItemData(int32_t iIndex, void* pData) {
  CFWL_ComboBoxItem* pItem = static_cast<CFWL_ComboBoxItem*>(
      m_comboBoxData.GetItem(m_pIface.get(), iIndex));
  if (!pItem)
    return FWL_Error::Indefinite;
  pItem->m_pData = pData;
  return FWL_Error::Succeeded;
}

void* CFWL_ComboBox::GetItemData(int32_t iIndex) {
  CFWL_ComboBoxItem* pItem = static_cast<CFWL_ComboBoxItem*>(
      m_comboBoxData.GetItem(m_pIface.get(), iIndex));
  return pItem ? pItem->m_pData : nullptr;
}

FWL_Error CFWL_ComboBox::SetListTheme(IFWL_ThemeProvider* pTheme) {
  return ToComboBox(GetWidget())->GetListBoxt()->SetThemeProvider(pTheme);
}

bool CFWL_ComboBox::AfterFocusShowDropList() {
  return ToComboBox(GetWidget())->AfterFocusShowDropList();
}

FWL_Error CFWL_ComboBox::OpenDropDownList(bool bActivate) {
  return ToComboBox(GetWidget())->OpenDropDownList(bActivate);
}

bool CFWL_ComboBox::EditCanUndo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanUndo() : false;
}

bool CFWL_ComboBox::EditCanRedo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanRedo() : false;
}

bool CFWL_ComboBox::EditUndo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditUndo() : false;
}

bool CFWL_ComboBox::EditRedo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditRedo() : false;
}

bool CFWL_ComboBox::EditCanCopy() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanCopy() : false;
}

bool CFWL_ComboBox::EditCanCut() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanCut() : false;
}

bool CFWL_ComboBox::EditCanSelectAll() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanSelectAll() : false;
}

bool CFWL_ComboBox::EditCopy(CFX_WideString& wsCopy) {
  return GetWidget() ? ToComboBox(GetWidget())->EditCopy(wsCopy) : false;
}

bool CFWL_ComboBox::EditCut(CFX_WideString& wsCut) {
  return GetWidget() ? ToComboBox(GetWidget())->EditCut(wsCut) : false;
}

bool CFWL_ComboBox::EditPaste(const CFX_WideString& wsPaste) {
  return GetWidget() ? ToComboBox(GetWidget())->EditPaste(wsPaste) : false;
}

bool CFWL_ComboBox::EditSelectAll() {
  return GetWidget() ? ToComboBox(GetWidget())->EditSelectAll() : false;
}

bool CFWL_ComboBox::EditDelete() {
  return GetWidget() ? ToComboBox(GetWidget())->EditDelete() : false;
}

bool CFWL_ComboBox::EditDeSelect() {
  return GetWidget() ? ToComboBox(GetWidget())->EditDeSelect() : false;
}

FWL_Error CFWL_ComboBox::GetBBox(CFX_RectF& rect) {
  return GetWidget() ? ToComboBox(GetWidget())->GetBBox(rect)
                     : FWL_Error::Indefinite;
}

FWL_Error CFWL_ComboBox::EditModifyStylesEx(uint32_t dwStylesExAdded,
                                            uint32_t dwStylesExRemoved) {
  return GetWidget()
             ? ToComboBox(GetWidget())
                   ->EditModifyStylesEx(dwStylesExAdded, dwStylesExRemoved)
             : FWL_Error::Indefinite;
}

CFWL_ComboBox::CFWL_ComboBoxDP::CFWL_ComboBoxDP() {
  m_fItemHeight = 0;
  m_fMaxListHeight = 0;
}

CFWL_ComboBox::CFWL_ComboBoxDP::~CFWL_ComboBoxDP() {}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::GetCaption(
    IFWL_Widget* pWidget,
    CFX_WideString& wsCaption) {
  return FWL_Error::Succeeded;
}

int32_t CFWL_ComboBox::CFWL_ComboBoxDP::CountItems(const IFWL_Widget* pWidget) {
  return m_ItemArray.size();
}

IFWL_ListItem* CFWL_ComboBox::CFWL_ComboBoxDP::GetItem(
    const IFWL_Widget* pWidget,
    int32_t nIndex) {
  if (nIndex < 0 || static_cast<size_t>(nIndex) >= m_ItemArray.size())
    return nullptr;

  return m_ItemArray[nIndex].get();
}

int32_t CFWL_ComboBox::CFWL_ComboBoxDP::GetItemIndex(IFWL_Widget* pWidget,
                                                     IFWL_ListItem* pItem) {
  auto it = std::find_if(
      m_ItemArray.begin(), m_ItemArray.end(),
      [pItem](const std::unique_ptr<CFWL_ComboBoxItem>& candidate) {
        return candidate.get() == static_cast<CFWL_ComboBoxItem*>(pItem);
      });
  return it != m_ItemArray.end() ? it - m_ItemArray.begin() : -1;
}

bool CFWL_ComboBox::CFWL_ComboBoxDP::SetItemIndex(IFWL_Widget* pWidget,
                                                  IFWL_ListItem* pItem,
                                                  int32_t nIndex) {
  if (nIndex < 0 || static_cast<size_t>(nIndex) >= m_ItemArray.size())
    return false;

  m_ItemArray[nIndex].reset(static_cast<CFWL_ComboBoxItem*>(pItem));
  return true;
}

uint32_t CFWL_ComboBox::CFWL_ComboBoxDP::GetItemStyles(IFWL_Widget* pWidget,
                                                       IFWL_ListItem* pItem) {
  if (!pItem)
    return 0;
  return static_cast<CFWL_ComboBoxItem*>(pItem)->m_dwStyles;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::GetItemText(IFWL_Widget* pWidget,
                                                      IFWL_ListItem* pItem,
                                                      CFX_WideString& wsText) {
  if (!pItem)
    return FWL_Error::Indefinite;
  wsText = static_cast<CFWL_ComboBoxItem*>(pItem)->m_wsText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::GetItemRect(IFWL_Widget* pWidget,
                                                      IFWL_ListItem* pItem,
                                                      CFX_RectF& rtItem) {
  if (!pItem)
    return FWL_Error::Indefinite;
  CFWL_ComboBoxItem* pComboItem = static_cast<CFWL_ComboBoxItem*>(pItem);
  rtItem.Set(pComboItem->m_rtItem.left, pComboItem->m_rtItem.top,
             pComboItem->m_rtItem.width, pComboItem->m_rtItem.height);
  return FWL_Error::Succeeded;
}

void* CFWL_ComboBox::CFWL_ComboBoxDP::GetItemData(IFWL_Widget* pWidget,
                                                  IFWL_ListItem* pItem) {
  return pItem ? static_cast<CFWL_ComboBoxItem*>(pItem)->m_pData : nullptr;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::SetItemStyles(IFWL_Widget* pWidget,
                                                        IFWL_ListItem* pItem,
                                                        uint32_t dwStyle) {
  if (!pItem)
    return FWL_Error::Indefinite;
  static_cast<CFWL_ComboBoxItem*>(pItem)->m_dwStyles = dwStyle;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::SetItemText(IFWL_Widget* pWidget,
                                                      IFWL_ListItem* pItem,
                                                      const FX_WCHAR* pszText) {
  if (!pItem)
    return FWL_Error::Indefinite;
  static_cast<CFWL_ComboBoxItem*>(pItem)->m_wsText = pszText;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::SetItemRect(IFWL_Widget* pWidget,
                                                      IFWL_ListItem* pItem,
                                                      const CFX_RectF& rtItem) {
  if (!pItem)
    return FWL_Error::Indefinite;
  static_cast<CFWL_ComboBoxItem*>(pItem)->m_rtItem = rtItem;
  return FWL_Error::Succeeded;
}

FX_FLOAT CFWL_ComboBox::CFWL_ComboBoxDP::GetItemHeight(IFWL_Widget* pWidget) {
  return m_fItemHeight;
}

CFX_DIBitmap* CFWL_ComboBox::CFWL_ComboBoxDP::GetItemIcon(
    IFWL_Widget* pWidget,
    IFWL_ListItem* pItem) {
  return pItem ? static_cast<CFWL_ComboBoxItem*>(pItem)->m_pDIB : nullptr;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::GetItemCheckRect(IFWL_Widget* pWidget,
                                                           IFWL_ListItem* pItem,
                                                           CFX_RectF& rtCheck) {
  rtCheck = static_cast<CFWL_ComboBoxItem*>(pItem)->m_rtCheckBox;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::SetItemCheckRect(
    IFWL_Widget* pWidget,
    IFWL_ListItem* pItem,
    const CFX_RectF& rtCheck) {
  static_cast<CFWL_ComboBoxItem*>(pItem)->m_rtCheckBox = rtCheck;
  return FWL_Error::Succeeded;
}

uint32_t CFWL_ComboBox::CFWL_ComboBoxDP::GetItemCheckState(
    IFWL_Widget* pWidget,
    IFWL_ListItem* pItem) {
  return static_cast<CFWL_ComboBoxItem*>(pItem)->m_dwCheckState;
}

FWL_Error CFWL_ComboBox::CFWL_ComboBoxDP::SetItemCheckState(
    IFWL_Widget* pWidget,
    IFWL_ListItem* pItem,
    uint32_t dwCheckState) {
  static_cast<CFWL_ComboBoxItem*>(pItem)->m_dwCheckState = dwCheckState;
  return FWL_Error::Succeeded;
}

FX_FLOAT CFWL_ComboBox::CFWL_ComboBoxDP::GetListHeight(IFWL_Widget* pWidget) {
  return m_fMaxListHeight;
}

CFWL_ComboBoxItem::CFWL_ComboBoxItem() : m_pDIB(nullptr), m_pData(nullptr) {}

CFWL_ComboBoxItem::~CFWL_ComboBoxItem() {}
