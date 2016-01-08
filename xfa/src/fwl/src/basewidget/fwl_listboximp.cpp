// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_scrollbarimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_listboximp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_comboboximp.h"

#define FWL_LISTBOX_ItemTextMargin 2

// static
IFWL_ListBox* IFWL_ListBox::Create(const CFWL_WidgetImpProperties& properties,
                                   IFWL_Widget* pOuter) {
  IFWL_ListBox* pListBox = new IFWL_ListBox;
  CFWL_ListBoxImp* pListBoxImpl = new CFWL_ListBoxImp(properties, pOuter);
  pListBox->SetImpl(pListBoxImpl);
  pListBoxImpl->SetInterface(pListBox);
  return pListBox;
}
// static
IFWL_ListBox* IFWL_ListBox::CreateComboList(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_ListBox* pListBox = new IFWL_ListBox;
  CFWL_ListBoxImp* pComboListImpl = new CFWL_ComboListImp(properties, pOuter);
  pListBox->SetImpl(pComboListImpl);
  pComboListImpl->SetInterface(pListBox);
  return pListBox;
}
IFWL_ListBox::IFWL_ListBox() {}
int32_t IFWL_ListBox::CountSelItems() {
  return static_cast<CFWL_ListBoxImp*>(GetImpl())->CountSelItems();
}
FWL_HLISTITEM IFWL_ListBox::GetSelItem(int32_t nIndexSel) {
  return static_cast<CFWL_ListBoxImp*>(GetImpl())->GetSelItem(nIndexSel);
}
int32_t IFWL_ListBox::GetSelIndex(int32_t nIndex) {
  return static_cast<CFWL_ListBoxImp*>(GetImpl())->GetSelIndex(nIndex);
}
FWL_ERR IFWL_ListBox::SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect) {
  return static_cast<CFWL_ListBoxImp*>(GetImpl())->SetSelItem(hItem, bSelect);
}
FWL_ERR IFWL_ListBox::GetItemText(FWL_HLISTITEM hItem, CFX_WideString& wsText) {
  return static_cast<CFWL_ListBoxImp*>(GetImpl())->GetItemText(hItem, wsText);
}
FWL_ERR IFWL_ListBox::GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert) {
  return static_cast<CFWL_ListBoxImp*>(GetImpl())->GetScrollPos(fPos, bVert);
}
FWL_ERR* IFWL_ListBox::Sort(IFWL_ListBoxCompare* pCom) {
  return static_cast<CFWL_ListBoxImp*>(GetImpl())->Sort(pCom);
}

CFWL_ListBoxImp::CFWL_ListBoxImp(const CFWL_WidgetImpProperties& properties,
                                 IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_dwTTOStyles(0),
      m_iTTOAligns(0),
      m_hAnchor(NULL),
      m_fScorllBarWidth(0),
      m_bLButtonDown(FALSE),
      m_pScrollBarTP(NULL) {
  m_rtClient.Reset();
  m_rtConent.Reset();
  m_rtStatic.Reset();
}
CFWL_ListBoxImp::~CFWL_ListBoxImp() {
}
FWL_ERR CFWL_ListBoxImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_ListBox;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ListBoxImp::GetClassID() const {
  return FWL_CLASSHASH_ListBox;
}
FWL_ERR CFWL_ListBoxImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_ListBoxImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBoxImp::Finalize() {
  if (m_pVertScrollBar) {
    m_pVertScrollBar->Finalize();
  }
  if (m_pHorzScrollBar) {
    m_pHorzScrollBar->Finalize();
  }
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_ListBoxImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, 0, 0);
    if (!m_pProperties->m_pThemeProvider) {
      m_pProperties->m_pThemeProvider = GetAvailableTheme();
    }
    CFX_SizeF fs = CalcSize(TRUE);
    rect.Set(0, 0, fs.x, fs.y);
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBoxImp::Update() {
  if (IsLocked()) {
    return FWL_ERR_Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  m_iTTOAligns = FDE_TTOALIGNMENT_Center;
  switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_AlignMask) {
    case FWL_STYLEEXT_LTB_LeftAlign: {
      m_iTTOAligns = FDE_TTOALIGNMENT_CenterLeft;
      break;
    }
    case FWL_STYLEEXT_LTB_RightAlign: {
      m_iTTOAligns = FDE_TTOALIGNMENT_CenterRight;
      break;
    }
    case FWL_STYLEEXT_LTB_CenterAlign:
    default: { m_iTTOAligns = FDE_TTOALIGNMENT_Center; }
  }
  if (m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_RTLReading) {
    m_dwTTOStyles |= FDE_TTOSTYLE_RTL;
  }
  m_dwTTOStyles |= FDE_TTOSTYLE_SingleLine;
  m_fScorllBarWidth = GetScrollWidth();
  SortItem();
  CalcSize();
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ListBoxImp::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (IsShowScrollBar(FALSE)) {
    CFX_RectF rect;
    m_pHorzScrollBar->GetWidgetRect(rect);
    if (rect.Contains(fx, fy)) {
      return FWL_WGTHITTEST_HScrollBar;
    }
  }
  if (IsShowScrollBar(TRUE)) {
    CFX_RectF rect;
    m_pVertScrollBar->GetWidgetRect(rect);
    if (rect.Contains(fx, fy)) {
      return FWL_WGTHITTEST_VScrollBar;
    }
  }
  if (m_rtClient.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Client;
  }
  return FWL_WGTHITTEST_Unknown;
}
FWL_ERR CFWL_ListBoxImp::DrawWidget(CFX_Graphics* pGraphics,
                                    const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  pGraphics->SaveGraphState();
  if (HasBorder()) {
    DrawBorder(pGraphics, FWL_PART_LTB_Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, FWL_PART_LTB_Edge, pTheme, pMatrix);
  }
  CFX_RectF rtClip(m_rtConent);
  if (IsShowScrollBar(FALSE)) {
    rtClip.height -= m_fScorllBarWidth;
  }
  if (IsShowScrollBar(TRUE)) {
    rtClip.width -= m_fScorllBarWidth;
  }
  if (pMatrix) {
    pMatrix->TransformRect(rtClip);
  }
  pGraphics->SetClipRect(rtClip);
  if ((m_pProperties->m_dwStyles & FWL_WGTSTYLE_NoBackground) == 0) {
    DrawBkground(pGraphics, pTheme, pMatrix);
  }
  DrawItems(pGraphics, pTheme, pMatrix);
  pGraphics->RestoreGraphState();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBoxImp::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  if (!pThemeProvider)
    return FWL_ERR_Indefinite;
  if (!pThemeProvider->IsValidWidget(m_pInterface)) {
    m_pScrollBarTP = pThemeProvider;
    return FWL_ERR_Succeeded;
  }
  m_pProperties->m_pThemeProvider = pThemeProvider;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_ListBoxImp::CountSelItems() {
  if (!m_pProperties->m_pDataProvider)
    return 0;
  int32_t iRet = 0;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    if (!hItem) {
      continue;
    }
    FX_DWORD dwStyle = pData->GetItemStyles(m_pInterface, hItem);
    if (dwStyle & FWL_ITEMSTATE_LTB_Selected) {
      iRet++;
    }
  }
  return iRet;
}
FWL_HLISTITEM CFWL_ListBoxImp::GetSelItem(int32_t nIndexSel) {
  if (!m_pProperties->m_pDataProvider)
    return NULL;
  int32_t index = 0;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    if (!hItem) {
      return NULL;
    }
    FX_DWORD dwStyle = pData->GetItemStyles(m_pInterface, hItem);
    if (dwStyle & FWL_ITEMSTATE_LTB_Selected) {
      if (index == nIndexSel) {
        return hItem;
      } else {
        index++;
      }
    }
  }
  return NULL;
}
int32_t CFWL_ListBoxImp::GetSelIndex(int32_t nIndex) {
  if (!m_pProperties->m_pDataProvider)
    return -1;
  int32_t index = 0;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    if (!hItem) {
      return -1;
    }
    FX_DWORD dwStyle = pData->GetItemStyles(m_pInterface, hItem);
    if (dwStyle & FWL_ITEMSTATE_LTB_Selected) {
      if (index == nIndex) {
        return i;
      } else {
        index++;
      }
    }
  }
  return -1;
}
FWL_ERR CFWL_ListBoxImp::SetSelItem(FWL_HLISTITEM hItem, FX_BOOL bSelect) {
  if (!m_pProperties->m_pDataProvider)
    return FWL_ERR_Indefinite;
  if (!hItem) {
    if (bSelect) {
      SelectAll();
    } else {
      ClearSelection();
      SetFocusItem(NULL);
    }
    return FWL_ERR_Indefinite;
  }
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiSelection) {
    SetSelectionDirect(hItem, bSelect);
  } else {
    SetSelection(hItem, hItem, bSelect);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBoxImp::GetItemText(FWL_HLISTITEM hItem,
                                     CFX_WideString& wsText) {
  if (!m_pProperties->m_pDataProvider)
    return FWL_ERR_Indefinite;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  if (!hItem)
    return FWL_ERR_Indefinite;
  pData->GetItemText(m_pInterface, hItem, wsText);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBoxImp::GetScrollPos(FX_FLOAT& fPos, FX_BOOL bVert) {
  if ((bVert && IsShowScrollBar(TRUE)) || (!bVert && IsShowScrollBar(FALSE))) {
    IFWL_ScrollBar* pScrollBar =
        bVert ? m_pVertScrollBar.get() : m_pHorzScrollBar.get();
    fPos = pScrollBar->GetPos();
    return FWL_ERR_Succeeded;
  }
  return FWL_ERR_Indefinite;
}
FWL_ERR* CFWL_ListBoxImp::Sort(IFWL_ListBoxCompare* pCom) {
  FWL_HLISTITEM hTemp;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t sz = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < sz - 1; i++) {
    for (int32_t j = i + 1; j < sz; j++) {
      if (pCom->Compare(pData->GetItem(m_pInterface, i),
                        pData->GetItem(m_pInterface, j)) > 0) {
        hTemp = pData->GetItem(m_pInterface, i);
        pData->SetItemIndex(m_pInterface, pData->GetItem(m_pInterface, j), i);
        pData->SetItemIndex(m_pInterface, hTemp, j);
      }
    }
  }
  return FWL_ERR_Succeeded;
}
FWL_HLISTITEM CFWL_ListBoxImp::GetItem(FWL_HLISTITEM hItem,
                                       FX_DWORD dwKeyCode) {
  FWL_HLISTITEM hRet = NULL;
  switch (dwKeyCode) {
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      FX_BOOL bUp = dwKeyCode == FWL_VKEY_Up;
      FX_BOOL bDown = dwKeyCode == FWL_VKEY_Down;
      FX_BOOL bHome = dwKeyCode == FWL_VKEY_Home;
      IFWL_ListBoxDP* pData =
          static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
      int32_t iDstItem = -1;
      if (bUp || bDown) {
        int32_t index = pData->GetItemIndex(m_pInterface, hItem);
        iDstItem = dwKeyCode == FWL_VKEY_Up ? index - 1 : index + 1;
      } else if (bHome) {
        iDstItem = 0;
      } else {
        int32_t iCount = pData->CountItems(m_pInterface);
        iDstItem = iCount - 1;
      }
      hRet = pData->GetItem(m_pInterface, iDstItem);
      break;
    }
    default: {}
  }
  return hRet;
}
void CFWL_ListBoxImp::SetSelection(FWL_HLISTITEM hStart,
                                   FWL_HLISTITEM hEnd,
                                   FX_BOOL bSelected) {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iStart = pData->GetItemIndex(m_pInterface, hStart);
  int32_t iEnd = pData->GetItemIndex(m_pInterface, hEnd);
  if (iStart > iEnd) {
    int32_t iTemp = iStart;
    iStart = iEnd;
    iEnd = iTemp;
  }
  if (bSelected) {
    int32_t iCount = pData->CountItems(m_pInterface);
    for (int32_t i = 0; i < iCount; i++) {
      FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
      SetSelectionDirect(hItem, FALSE);
    }
  }
  for (; iStart <= iEnd; iStart++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, iStart);
    SetSelectionDirect(hItem, bSelected);
  }
}
void CFWL_ListBoxImp::SetSelectionDirect(FWL_HLISTITEM hItem, FX_BOOL bSelect) {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  FX_DWORD dwOldStyle = pData->GetItemStyles(m_pInterface, hItem);
  bSelect ? dwOldStyle |= FWL_ITEMSTATE_LTB_Selected
          : dwOldStyle &= ~FWL_ITEMSTATE_LTB_Selected;
  pData->SetItemStyles(m_pInterface, hItem, dwOldStyle);
}
FX_BOOL CFWL_ListBoxImp::IsItemSelected(FWL_HLISTITEM hItem) {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  FX_DWORD dwState = pData->GetItemStyles(m_pInterface, hItem);
  return (dwState & FWL_ITEMSTATE_LTB_Selected) != 0;
}
void CFWL_ListBoxImp::ClearSelection() {
  FX_BOOL bMulti =
      m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiSelection;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    FX_DWORD dwState = pData->GetItemStyles(m_pInterface, hItem);
    FX_BOOL bFindSel = dwState & FWL_ITEMSTATE_LTB_Selected;
    if (!bFindSel) {
      continue;
    }
    SetSelectionDirect(hItem, FALSE);
    if (!bMulti) {
      return;
    }
  }
}
void CFWL_ListBoxImp::SelectAll() {
  FX_BOOL bMulti =
      m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiSelection;
  if (!bMulti) {
    return;
  }
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  if (iCount > 0) {
    FWL_HLISTITEM hItemStart = pData->GetItem(m_pInterface, 0);
    FWL_HLISTITEM hItemEnd = pData->GetItem(m_pInterface, iCount - 1);
    SetSelection(hItemStart, hItemEnd, FALSE);
  }
}
FWL_HLISTITEM CFWL_ListBoxImp::GetFocusedItem() {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    if (!hItem)
      return NULL;
    if (pData->GetItemStyles(m_pInterface, hItem) & FWL_ITEMSTATE_LTB_Focused) {
      return hItem;
    }
  }
  return NULL;
}
void CFWL_ListBoxImp::SetFocusItem(FWL_HLISTITEM hItem) {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  FWL_HLISTITEM hFocus = GetFocusedItem();
  if (hItem != hFocus) {
    if (hFocus) {
      FX_DWORD dwStyle = pData->GetItemStyles(m_pInterface, hFocus);
      dwStyle &= ~FWL_ITEMSTATE_LTB_Focused;
      pData->SetItemStyles(m_pInterface, hFocus, dwStyle);
    }
    if (hItem) {
      FX_DWORD dwStyle = pData->GetItemStyles(m_pInterface, hItem);
      dwStyle |= FWL_ITEMSTATE_LTB_Focused;
      pData->SetItemStyles(m_pInterface, hItem, dwStyle);
    }
  }
}
FWL_HLISTITEM CFWL_ListBoxImp::GetItemAtPoint(FX_FLOAT fx, FX_FLOAT fy) {
  fx -= m_rtConent.left, fy -= m_rtConent.top;
  FX_FLOAT fPosX = 0.0f;
  if (m_pHorzScrollBar) {
    fPosX = m_pHorzScrollBar->GetPos();
  }
  FX_FLOAT fPosY = 0.0;
  if (m_pVertScrollBar) {
    fPosY = m_pVertScrollBar->GetPos();
  }
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t nCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < nCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    if (!hItem) {
      continue;
    }
    CFX_RectF rtItem;
    pData->GetItemRect(m_pInterface, hItem, rtItem);
    rtItem.Offset(-fPosX, -fPosY);
    if (rtItem.Contains(fx, fy)) {
      return hItem;
    }
  }
  return NULL;
}
FX_BOOL CFWL_ListBoxImp::GetItemCheckRect(FWL_HLISTITEM hItem,
                                          CFX_RectF& rtCheck) {
  if (!m_pProperties->m_pDataProvider)
    return FALSE;
  if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check)) {
    return FALSE;
  }
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  pData->GetItemCheckRect(m_pInterface, hItem, rtCheck);
  return TRUE;
}
FX_BOOL CFWL_ListBoxImp::GetItemChecked(FWL_HLISTITEM hItem) {
  if (!m_pProperties->m_pDataProvider)
    return FALSE;
  if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check)) {
    return FALSE;
  }
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  return (pData->GetItemCheckState(m_pInterface, hItem) &
          FWL_ITEMSTATE_LTB_Checked);
}
FX_BOOL CFWL_ListBoxImp::SetItemChecked(FWL_HLISTITEM hItem, FX_BOOL bChecked) {
  if (!m_pProperties->m_pDataProvider)
    return FALSE;
  if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check)) {
    return FALSE;
  }
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  pData->SetItemCheckState(m_pInterface, hItem,
                           bChecked ? FWL_ITEMSTATE_LTB_Checked : 0);
  return TRUE;
}
FX_BOOL CFWL_ListBoxImp::ScrollToVisible(FWL_HLISTITEM hItem) {
  if (!m_pVertScrollBar)
    return FALSE;
  CFX_RectF rtItem;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  pData->GetItemRect(m_pInterface, hItem, rtItem);
  FX_BOOL bScroll = FALSE;
  FX_FLOAT fPosY = m_pVertScrollBar->GetPos();
  rtItem.Offset(0, -fPosY + m_rtConent.top);
  if (rtItem.top < m_rtConent.top) {
    fPosY += rtItem.top - m_rtConent.top;
    bScroll = TRUE;
  } else if (rtItem.bottom() > m_rtConent.bottom()) {
    fPosY += rtItem.bottom() - m_rtConent.bottom();
    bScroll = TRUE;
  }
  if (!bScroll) {
    return FALSE;
  }
  m_pVertScrollBar->SetPos(fPosY);
  m_pVertScrollBar->SetTrackPos(fPosY);
  Repaint(&m_rtClient);
  return TRUE;
}
void CFWL_ListBoxImp::DrawBkground(CFX_Graphics* pGraphics,
                                   IFWL_ThemeProvider* pTheme,
                                   const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!pTheme)
    return;
  CFWL_ThemeBackground param;
  param.m_pWidget = m_pInterface;
  param.m_iPart = FWL_PART_LTB_Background;
  param.m_dwStates = 0;
  param.m_pGraphics = pGraphics;
  param.m_matrix.Concat(*pMatrix);
  param.m_rtPart = m_rtClient;
  if (IsShowScrollBar(FALSE) && IsShowScrollBar(TRUE)) {
    param.m_pData = &m_rtStatic;
  }
  if (!IsEnabled()) {
    param.m_dwStates = FWL_PARTSTATE_LTB_Disabled;
  }
  pTheme->DrawBackground(&param);
}
void CFWL_ListBoxImp::DrawItems(CFX_Graphics* pGraphics,
                                IFWL_ThemeProvider* pTheme,
                                const CFX_Matrix* pMatrix) {
  FX_FLOAT fPosX = 0.0f;
  if (m_pHorzScrollBar) {
    fPosX = m_pHorzScrollBar->GetPos();
  }
  FX_FLOAT fPosY = 0.0f;
  if (m_pVertScrollBar) {
    fPosY = m_pVertScrollBar->GetPos();
  }
  CFX_RectF rtView(m_rtConent);
  if (m_pHorzScrollBar) {
    rtView.height -= m_fScorllBarWidth;
  }
  if (m_pVertScrollBar) {
    rtView.width -= m_fScorllBarWidth;
  }
  FX_BOOL bMultiCol =
      m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiColumn;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    if (!hItem) {
      continue;
    }
    CFX_RectF rtItem;
    pData->GetItemRect(m_pInterface, hItem, rtItem);
    rtItem.Offset(m_rtConent.left - fPosX, m_rtConent.top - fPosY);
    if (rtItem.bottom() < m_rtConent.top) {
      continue;
    }
    if (rtItem.top >= m_rtConent.bottom()) {
      break;
    }
    if (bMultiCol && rtItem.left > m_rtConent.right()) {
      break;
    }
    if (GetStylesEx() & FWL_STYLEEXT_LTB_OwnerDraw) {
      CFWL_EvtLtbDrawItem ev;
      ev.m_pSrcTarget = m_pInterface;
      ev.m_pGraphics = pGraphics;
      ev.m_matrix = *pMatrix;
      ev.m_index = i;
      ev.m_rect = rtItem;
      DispatchEvent(&ev);
    } else {
      DrawItem(pGraphics, pTheme, hItem, i, rtItem, pMatrix);
    }
  }
}
void CFWL_ListBoxImp::DrawItem(CFX_Graphics* pGraphics,
                               IFWL_ThemeProvider* pTheme,
                               FWL_HLISTITEM hItem,
                               int32_t Index,
                               const CFX_RectF& rtItem,
                               const CFX_Matrix* pMatrix) {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  FX_DWORD dwItemStyles = pData->GetItemStyles(m_pInterface, hItem);
  FX_DWORD dwPartStates = FWL_PARTSTATE_LTB_Normal;
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
    dwPartStates = FWL_PARTSTATE_LTB_Disabled;
  } else if (dwItemStyles & FWL_ITEMSTATE_LTB_Selected) {
    dwPartStates = FWL_PARTSTATE_LTB_Selected;
  }
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused &&
      dwItemStyles & FWL_ITEMSTATE_LTB_Focused) {
    dwPartStates |= FWL_PARTSTATE_LTB_Focused;
  }
  FWL_ListBoxItemData itemData;
  itemData.pDataProvider = pData;
  itemData.iIndex = Index;
  {
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = FWL_PART_LTB_ListItem;
    param.m_dwStates = dwPartStates;
    param.m_pGraphics = pGraphics;
    param.m_matrix.Concat(*pMatrix);
    param.m_rtPart = rtItem;
    param.m_dwData = (FX_DWORD)(uintptr_t)(&itemData);
    CFX_RectF rtFocus(rtItem);
    param.m_pData = &rtFocus;
    if (m_pVertScrollBar && !m_pHorzScrollBar &&
        (dwPartStates & FWL_PARTSTATE_LTB_Focused)) {
      param.m_rtPart.left += 1;
      param.m_rtPart.width -= (m_fScorllBarWidth + 1);
      rtFocus.Deflate(0.5, 0.5, 1 + m_fScorllBarWidth, 1);
    }
    pTheme->DrawBackground(&param);
  }
  {
    FX_BOOL bHasIcon = GetStylesEx() & FWL_STYLEEXT_LTB_Icon;
    if (bHasIcon) {
      CFX_RectF rtDIB;
      CFX_DIBitmap* pDib = pData->GetItemIcon(m_pInterface, hItem);
      rtDIB.Set(rtItem.left, rtItem.top, rtItem.height, rtItem.height);
      if (pDib) {
        CFWL_ThemeBackground param;
        param.m_pWidget = m_pInterface;
        param.m_iPart = FWL_PART_LTB_Icon;
        param.m_pGraphics = pGraphics;
        param.m_matrix.Concat(*pMatrix);
        param.m_rtPart = rtDIB;
        param.m_dwData = (FX_DWORD)(uintptr_t)(&itemData);
        param.m_pImage = pDib;
        pTheme->DrawBackground(&param);
      }
    }
    FX_BOOL bHasCheck = GetStylesEx() & FWL_STYLEEXT_LTB_Check;
    if (bHasCheck) {
      CFX_RectF rtCheck;
      rtCheck.Set(rtItem.left, rtItem.top, rtItem.height, rtItem.height);
      rtCheck.Deflate(2, 2, 2, 2);
      pData->SetItemCheckRect(m_pInterface, hItem, rtCheck);
      CFWL_ThemeBackground param;
      param.m_pWidget = m_pInterface;
      param.m_iPart = FWL_PART_LTB_Check;
      param.m_pGraphics = pGraphics;
      if (GetItemChecked(hItem)) {
        param.m_dwStates = FWL_PARTSTATE_LTB_Checked;
      } else {
        param.m_dwStates = FWL_PARTSTATE_LTB_UnChecked;
      }
      param.m_matrix.Concat(*pMatrix);
      param.m_rtPart = rtCheck;
      param.m_dwData = (FX_DWORD)(uintptr_t)(&itemData);
      pTheme->DrawBackground(&param);
    }
    CFX_WideString wsText;
    pData->GetItemText(m_pInterface, hItem, wsText);
    if (wsText.GetLength() <= 0) {
      return;
    }
    CFX_RectF rtText(rtItem);
    rtText.Deflate(FWL_LISTBOX_ItemTextMargin, FWL_LISTBOX_ItemTextMargin);
    if (bHasIcon || bHasCheck) {
      rtText.Deflate(rtItem.height, 0, 0, 0);
    }
    CFWL_ThemeText textParam;
    textParam.m_pWidget = m_pInterface;
    textParam.m_iPart = FWL_PART_LTB_ListItem;
    textParam.m_dwStates = dwPartStates;
    textParam.m_pGraphics = pGraphics;
    textParam.m_matrix.Concat(*pMatrix);
    textParam.m_rtPart = rtText;
    textParam.m_wsText = wsText;
    textParam.m_dwTTOStyles = m_dwTTOStyles;
    textParam.m_iTTOAlign = m_iTTOAligns;
    textParam.m_dwData = (FX_DWORD)(uintptr_t)(&itemData);
    pTheme->DrawText(&textParam);
  }
}
CFX_SizeF CFWL_ListBoxImp::CalcSize(FX_BOOL bAutoSize) {
  CFX_SizeF fs;
  fs.Set(0, 0);
  if (!m_pProperties->m_pThemeProvider)
    return fs;
  GetClientRect(m_rtClient);
  m_rtConent = m_rtClient;
  CFX_RectF rtUIMargin;
  rtUIMargin.Set(0, 0, 0, 0);
  if (!m_pOuter) {
    CFX_RectF* pUIMargin =
        static_cast<CFX_RectF*>(GetThemeCapacity(FWL_WGTCAPACITY_UIMargin));
    if (pUIMargin) {
      m_rtConent.Deflate(pUIMargin->left, pUIMargin->top, pUIMargin->width,
                         pUIMargin->height);
    }
  }
  FX_FLOAT fWidth = 0;
  if (m_pProperties->m_pThemeProvider->IsCustomizedLayout(m_pInterface)) {
    IFWL_ListBoxDP* pData =
        static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
    if (!bAutoSize) {
    }
    int32_t iCount = pData->CountItems(m_pInterface);
    for (int32_t i = 0; i < iCount; i++) {
      FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
      CFWL_ThemePart itemPart;
      itemPart.m_pWidget = m_pInterface;
      itemPart.m_iPart = FWL_PART_LTB_ListItem;
      itemPart.m_pData = m_pProperties->m_pDataProvider;
      itemPart.m_dwData = i;
      CFX_RectF r;
      m_pProperties->m_pThemeProvider->GetPartRect(&itemPart, r);
      if (!bAutoSize) {
        CFX_RectF rtItem;
        rtItem.Set(m_rtClient.left, m_rtClient.top + fs.y, r.width, r.height);
        IFWL_ListBoxDP* pData =
            static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
        pData->SetItemRect(m_pInterface, hItem, rtItem);
      }
      fs.y += r.height;
      if (fs.x < r.width) {
        fs.x = r.width;
        fWidth = r.width;
      }
    }
  } else {
    fWidth = GetMaxTextWidth();
    fWidth += 2 * FWL_LISTBOX_ItemTextMargin;
    if (!bAutoSize) {
      FX_FLOAT fActualWidth =
          m_rtClient.width - rtUIMargin.left - rtUIMargin.width;
      if (fWidth < fActualWidth) {
        fWidth = fActualWidth;
      }
    }
    IFWL_ListBoxDP* pData =
        static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
    m_fItemHeight = GetItemHeigt();
    FX_BOOL bHasIcon;
    bHasIcon = GetStylesEx() & FWL_STYLEEXT_LTB_Icon;
    if (bHasIcon) {
      fWidth += m_fItemHeight;
    }
    int32_t iCount = pData->CountItems(m_pInterface);
    for (int32_t i = 0; i < iCount; i++) {
      FWL_HLISTITEM htem = pData->GetItem(m_pInterface, i);
      GetItemSize(fs, htem, fWidth, m_fItemHeight, bAutoSize);
    }
  }
  if (bAutoSize) {
    return fs;
  }
  FX_FLOAT iWidth = m_rtClient.width - rtUIMargin.left - rtUIMargin.width;
  FX_FLOAT iHeight = m_rtClient.height;
  FX_BOOL bShowVertScr =
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarAlaways) &&
      (m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll);
  FX_BOOL bShowHorzScr =
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarAlaways) &&
      (m_pProperties->m_dwStyles & FWL_WGTSTYLE_HScroll);
  if (!bShowVertScr && m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll &&
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiColumn) == 0) {
    bShowVertScr = (fs.y > iHeight);
  }
  if (!bShowHorzScr && m_pProperties->m_dwStyles & FWL_WGTSTYLE_HScroll) {
    bShowHorzScr = (fs.x > iWidth);
  }
  CFX_SizeF szRange;
  if (bShowVertScr) {
    if (!m_pVertScrollBar) {
      InitScrollBar();
    }
    CFX_RectF rtScrollBar;
    rtScrollBar.Set(m_rtClient.right() - m_fScorllBarWidth, m_rtClient.top,
                    m_fScorllBarWidth, m_rtClient.height - 1);
    if (bShowHorzScr) {
      rtScrollBar.height -= m_fScorllBarWidth;
    }
    m_pVertScrollBar->SetWidgetRect(rtScrollBar);
    szRange.x = 0, szRange.y = fs.y - m_rtConent.height;
    if (szRange.y < m_fItemHeight) {
      szRange.y = m_fItemHeight;
    }
    m_pVertScrollBar->SetRange(szRange.x, szRange.y);
    m_pVertScrollBar->SetPageSize(rtScrollBar.height * 9 / 10);
    m_pVertScrollBar->SetStepSize(m_fItemHeight);
    FX_FLOAT fPos = m_pVertScrollBar->GetPos();
    if (fPos < 0) {
      fPos = 0;
    }
    if (fPos > szRange.y) {
      fPos = szRange.y;
    }
    m_pVertScrollBar->SetPos(fPos);
    m_pVertScrollBar->SetTrackPos(fPos);
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ==
            0 ||
        (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)) {
      m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible, FALSE);
    }
    m_pVertScrollBar->Update();
  } else if (m_pVertScrollBar) {
    m_pVertScrollBar->SetPos(0);
    m_pVertScrollBar->SetTrackPos(0);
    m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible, TRUE);
  }
  if (bShowHorzScr) {
    if (!m_pHorzScrollBar) {
      InitScrollBar(FALSE);
    }
    CFX_RectF rtScrollBar;
    rtScrollBar.Set(m_rtClient.left, m_rtClient.bottom() - m_fScorllBarWidth,
                    m_rtClient.width, m_fScorllBarWidth);
    if (bShowVertScr) {
      rtScrollBar.width -= m_fScorllBarWidth;
    }
    m_pHorzScrollBar->SetWidgetRect(rtScrollBar);
    szRange.x = 0, szRange.y = fs.x - rtScrollBar.width;
    m_pHorzScrollBar->SetRange(szRange.x, szRange.y);
    m_pHorzScrollBar->SetPageSize(fWidth * 9 / 10);
    m_pHorzScrollBar->SetStepSize(fWidth / 10);
    FX_FLOAT fPos = m_pHorzScrollBar->GetPos();
    if (fPos < 0) {
      fPos = 0;
    }
    if (fPos > szRange.y) {
      fPos = szRange.y;
    }
    m_pHorzScrollBar->SetPos(fPos);
    m_pHorzScrollBar->SetTrackPos(fPos);
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ==
            0 ||
        (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)) {
      m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible, FALSE);
    }
    m_pHorzScrollBar->Update();
  } else if (m_pHorzScrollBar) {
    m_pHorzScrollBar->SetPos(0);
    m_pHorzScrollBar->SetTrackPos(0);
    m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible, TRUE);
  }
  if (bShowVertScr && bShowHorzScr) {
    m_rtStatic.Set(m_rtClient.right() - m_fScorllBarWidth,
                   m_rtClient.bottom() - m_fScorllBarWidth, m_fScorllBarWidth,
                   m_fScorllBarWidth);
  }
  return fs;
}
void CFWL_ListBoxImp::GetItemSize(CFX_SizeF& size,
                                  FWL_HLISTITEM hItem,
                                  FX_FLOAT fWidth,
                                  FX_FLOAT m_fItemHeight,
                                  FX_BOOL bAutoSize) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_MultiColumn) {
  } else {
    if (!bAutoSize) {
      CFX_RectF rtItem;
      rtItem.Set(0, size.y, fWidth, m_fItemHeight);
      IFWL_ListBoxDP* pData =
          static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
      pData->SetItemRect(m_pInterface, hItem, rtItem);
    }
    size.x = fWidth;
    size.y += m_fItemHeight;
  }
}
FX_FLOAT CFWL_ListBoxImp::GetMaxTextWidth() {
  FX_FLOAT fRet = 0.0f;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    if (!hItem) {
      continue;
    }
    CFX_WideString wsText;
    pData->GetItemText(m_pInterface, hItem, wsText);
    CFX_SizeF sz = CalcTextSize(wsText, m_pProperties->m_pThemeProvider);
    if (sz.x > fRet) {
      fRet = sz.x;
    }
  }
  return fRet;
}
FX_FLOAT CFWL_ListBoxImp::GetScrollWidth() {
  FX_FLOAT* pfWidth =
      static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth));
  if (!pfWidth)
    return 0;
  return *pfWidth;
}
FX_FLOAT CFWL_ListBoxImp::GetItemHeigt() {
  FX_FLOAT* pfFont =
      static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_FontSize));
  if (!pfFont)
    return 20;
  return *pfFont + 2 * FWL_LISTBOX_ItemTextMargin;
}
void CFWL_ListBoxImp::InitScrollBar(FX_BOOL bVert) {
  if ((bVert && m_pVertScrollBar) || (!bVert && m_pHorzScrollBar)) {
    return;
  }
  CFWL_WidgetImpProperties prop;
  prop.m_dwStyleExes = bVert ? FWL_STYLEEXT_SCB_Vert : FWL_STYLEEXT_SCB_Horz;
  prop.m_dwStates = FWL_WGTSTATE_Invisible;
  prop.m_pParent = m_pInterface;
  prop.m_pThemeProvider = m_pScrollBarTP;
  IFWL_ScrollBar* pScrollBar = IFWL_ScrollBar::Create(prop, m_pInterface);
  pScrollBar->Initialize();
  (bVert ? &m_pVertScrollBar : &m_pHorzScrollBar)->reset(pScrollBar);
}
void CFWL_ListBoxImp::SortItem() {}
FX_BOOL CFWL_ListBoxImp::IsShowScrollBar(FX_BOOL bVert) {
  IFWL_ScrollBar* pScrollbar =
      bVert ? m_pVertScrollBar.get() : m_pHorzScrollBar.get();
  if (!pScrollbar || (pScrollbar->GetStates() & FWL_WGTSTATE_Invisible)) {
    return FALSE;
  }
  return !(m_pProperties->m_dwStyleExes &
           FWL_STYLEEXT_LTB_ShowScrollBarFocus) ||
         (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused);
}
void CFWL_ListBoxImp::ProcessSelChanged() {
  CFWL_EvtLtbSelChanged selEvent;
  selEvent.m_pSrcTarget = m_pInterface;
  CFX_Int32Array arrSels;
  int32_t iCount = CountSelItems();
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM item = GetSelItem(i);
    if (item == NULL) {
      continue;
    }
    selEvent.iarraySels.Add(i);
  }
  DispatchEvent(&selEvent);
}
CFWL_ListBoxImpDelegate::CFWL_ListBoxImpDelegate(CFWL_ListBoxImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_ListBoxImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  if (!m_pOwner->IsEnabled()) {
    return 1;
  }
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  int32_t iRet = 1;
  switch (dwMsgCode) {
    case FWL_MSGHASH_SetFocus:
    case FWL_MSGHASH_KillFocus: {
      OnFocusChanged(pMessage, dwMsgCode == FWL_MSGHASH_SetFocus);
      break;
    }
    case FWL_MSGHASH_Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      FX_DWORD dwCmd = pMsg->m_dwCmd;
      switch (dwCmd) {
        case FWL_MSGMOUSECMD_LButtonDown: {
          OnLButtonDown(pMsg);
          break;
        }
        case FWL_MSGMOUSECMD_LButtonUp: {
          OnLButtonUp(pMsg);
          break;
        }
        default: {}
      }
      break;
    }
    case FWL_MSGHASH_MouseWheel: {
      OnMouseWheel(static_cast<CFWL_MsgMouseWheel*>(pMessage));
      break;
    }
    case FWL_MSGHASH_Key: {
      CFWL_MsgKey* pMsg = static_cast<CFWL_MsgKey*>(pMessage);
      if (pMsg->m_dwCmd == FWL_MSGKEYCMD_KeyDown)
        OnKeyDown(pMsg);
      break;
    }
    default: { iRet = 0; }
  }
  CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
  return iRet;
}
FWL_ERR CFWL_ListBoxImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent)
    return FWL_ERR_Indefinite;
  if (pEvent->GetClassID() != FWL_EVTHASH_Scroll) {
    return FWL_ERR_Succeeded;
  }
  IFWL_Widget* pSrcTarget = pEvent->m_pSrcTarget;
  if ((pSrcTarget == m_pOwner->m_pVertScrollBar.get() &&
       m_pOwner->m_pVertScrollBar) ||
      (pSrcTarget == m_pOwner->m_pHorzScrollBar.get() &&
       m_pOwner->m_pHorzScrollBar)) {
    CFWL_EvtScroll* pScrollEvent = static_cast<CFWL_EvtScroll*>(pEvent);
    OnScroll(static_cast<IFWL_ScrollBar*>(pSrcTarget),
             pScrollEvent->m_iScrollCode, pScrollEvent->m_fPos);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ListBoxImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                              const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_ListBoxImpDelegate::OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet) {
  if (m_pOwner->GetStylesEx() & FWL_STYLEEXT_LTB_ShowScrollBarFocus) {
    if (m_pOwner->m_pVertScrollBar) {
      m_pOwner->m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible, !bSet);
    }
    if (m_pOwner->m_pHorzScrollBar) {
      m_pOwner->m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible, !bSet);
    }
  }
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= (FWL_WGTSTATE_Focused);
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~(FWL_WGTSTATE_Focused);
  }
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_ListBoxImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  m_pOwner->m_bLButtonDown = TRUE;
  if ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
    m_pOwner->SetFocus(TRUE);
  }
  FWL_HLISTITEM hItem = m_pOwner->GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
  if (!hItem) {
    return;
  }
  if (m_pOwner->m_pProperties->m_dwStyleExes &
      FWL_STYLEEXT_LTB_MultiSelection) {
    if (pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl) {
      FX_BOOL bSelected = m_pOwner->IsItemSelected(hItem);
      m_pOwner->SetSelectionDirect(hItem, !bSelected);
      m_pOwner->m_hAnchor = hItem;
    } else if (pMsg->m_dwFlags & FWL_KEYFLAG_Shift) {
      if (m_pOwner->m_hAnchor) {
        m_pOwner->SetSelection(m_pOwner->m_hAnchor, hItem, TRUE);
      } else {
        m_pOwner->SetSelectionDirect(hItem, TRUE);
      }
    } else {
      m_pOwner->SetSelection(hItem, hItem, TRUE);
      m_pOwner->m_hAnchor = hItem;
    }
  } else {
    m_pOwner->SetSelection(hItem, hItem, TRUE);
  }
  if (m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_LTB_Check) {
    FWL_HLISTITEM hSelectedItem =
        m_pOwner->GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    CFX_RectF rtCheck;
    m_pOwner->GetItemCheckRect(hSelectedItem, rtCheck);
    FX_BOOL bChecked = m_pOwner->GetItemChecked(hItem);
    if (rtCheck.Contains(pMsg->m_fx, pMsg->m_fy)) {
      if (bChecked) {
        m_pOwner->SetItemChecked(hItem, FALSE);
      } else {
        m_pOwner->SetItemChecked(hItem, TRUE);
      }
      m_pOwner->Update();
    }
  }
  m_pOwner->SetFocusItem(hItem);
  m_pOwner->ScrollToVisible(hItem);
  m_pOwner->SetGrab(TRUE);
  m_pOwner->ProcessSelChanged();
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_ListBoxImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_bLButtonDown) {
    m_pOwner->m_bLButtonDown = FALSE;
    m_pOwner->SetGrab(FALSE);
    DispatchSelChangedEv();
  }
}
void CFWL_ListBoxImpDelegate::OnMouseWheel(CFWL_MsgMouseWheel* pMsg) {
  if (!m_pOwner->IsShowScrollBar(TRUE)) {
    return;
  }
  IFWL_WidgetDelegate* pDelegate =
      m_pOwner->m_pVertScrollBar->SetDelegate(NULL);
  pDelegate->OnProcessMessage(pMsg);
}
void CFWL_ListBoxImpDelegate::OnKeyDown(CFWL_MsgKey* pMsg) {
  FX_DWORD dwKeyCode = pMsg->m_dwKeyCode;
  switch (dwKeyCode) {
    case FWL_VKEY_Tab:
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      FWL_HLISTITEM hItem = m_pOwner->GetFocusedItem();
      hItem = m_pOwner->GetItem(hItem, dwKeyCode);
      FX_BOOL bShift = pMsg->m_dwFlags & FWL_KEYFLAG_Shift;
      FX_BOOL bCtrl = pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl;
      OnVK(hItem, bShift, bCtrl);
      DispatchSelChangedEv();
      m_pOwner->ProcessSelChanged();
      break;
    }
    default: {}
  }
}
void CFWL_ListBoxImpDelegate::OnVK(FWL_HLISTITEM hItem,
                                   FX_BOOL bShift,
                                   FX_BOOL bCtrl) {
  if (!hItem) {
    return;
  }
  if (m_pOwner->m_pProperties->m_dwStyleExes &
      FWL_STYLEEXT_LTB_MultiSelection) {
    if (bCtrl) {
    } else if (bShift) {
      if (m_pOwner->m_hAnchor) {
        m_pOwner->SetSelection(m_pOwner->m_hAnchor, hItem, TRUE);
      } else {
        m_pOwner->SetSelectionDirect(hItem, TRUE);
      }
    } else {
      m_pOwner->SetSelection(hItem, hItem, TRUE);
      m_pOwner->m_hAnchor = hItem;
    }
  } else {
    m_pOwner->SetSelection(hItem, hItem, TRUE);
  }
  m_pOwner->SetFocusItem(hItem);
  m_pOwner->ScrollToVisible(hItem);
  {
    CFX_RectF rtInvalidate;
    rtInvalidate.Set(0, 0, m_pOwner->m_pProperties->m_rtWidget.width,
                     m_pOwner->m_pProperties->m_rtWidget.height);
    m_pOwner->Repaint(&rtInvalidate);
  }
}
FX_BOOL CFWL_ListBoxImpDelegate::OnScroll(IFWL_ScrollBar* pScrollBar,
                                          FX_DWORD dwCode,
                                          FX_FLOAT fPos) {
  CFX_SizeF fs;
  pScrollBar->GetRange(fs.x, fs.y);
  FX_FLOAT iCurPos = pScrollBar->GetPos();
  FX_FLOAT fStep = pScrollBar->GetStepSize();
  switch (dwCode) {
    case FWL_SCBCODE_Min: {
      fPos = fs.x;
      break;
    }
    case FWL_SCBCODE_Max: {
      fPos = fs.y;
      break;
    }
    case FWL_SCBCODE_StepBackward: {
      fPos -= fStep;
      if (fPos < fs.x + fStep / 2) {
        fPos = fs.x;
      }
      break;
    }
    case FWL_SCBCODE_StepForward: {
      fPos += fStep;
      if (fPos > fs.y - fStep / 2) {
        fPos = fs.y;
      }
      break;
    }
    case FWL_SCBCODE_PageBackward: {
      fPos -= pScrollBar->GetPageSize();
      if (fPos < fs.x) {
        fPos = fs.x;
      }
      break;
    }
    case FWL_SCBCODE_PageForward: {
      fPos += pScrollBar->GetPageSize();
      if (fPos > fs.y) {
        fPos = fs.y;
      }
      break;
    }
    case FWL_SCBCODE_Pos:
    case FWL_SCBCODE_TrackPos:
      break;
    case FWL_SCBCODE_EndScroll:
      return FALSE;
  }
  if (iCurPos != fPos) {
    pScrollBar->SetPos(fPos);
    pScrollBar->SetTrackPos(fPos);
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  }
  return TRUE;
}
void CFWL_ListBoxImpDelegate::DispatchSelChangedEv() {
  CFWL_EvtLtbSelChanged ev;
  ev.m_pSrcTarget = m_pOwner->m_pInterface;
  m_pOwner->DispatchEvent(&ev);
}
