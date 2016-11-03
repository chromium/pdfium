// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_combolist.h"

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/ifwl_combobox.h"
#include "xfa/fwl/core/ifwl_comboedit.h"

IFWL_ComboList::IFWL_ComboList(
    const IFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties,
    IFWL_Widget* pOuter)
    : IFWL_ListBox(app, std::move(properties), pOuter), m_bNotifyOwner(true) {
  ASSERT(pOuter);
}

int32_t IFWL_ComboList::MatchItem(const CFX_WideString& wsMatch) {
  if (wsMatch.IsEmpty()) {
    return -1;
  }
  if (!m_pProperties->m_pDataProvider)
    return -1;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    IFWL_ListItem* hItem = pData->GetItem(this, i);
    CFX_WideString wsText;
    pData->GetItemText(this, hItem, wsText);
    FX_STRSIZE pos = wsText.Find(wsMatch.c_str());
    if (!pos) {
      return i;
    }
  }
  return -1;
}

void IFWL_ComboList::ChangeSelected(int32_t iSel) {
  if (!m_pProperties->m_pDataProvider)
    return;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  IFWL_ListItem* hItem = pData->GetItem(this, iSel);
  CFX_RectF rtInvalidate;
  rtInvalidate.Reset();
  IFWL_ListItem* hOld = GetSelItem(0);
  int32_t iOld = pData->GetItemIndex(this, hOld);
  if (iOld == iSel) {
    return;
  } else if (iOld > -1) {
    GetItemRect(iOld, rtInvalidate);
    SetSelItem(hOld, false);
  }
  if (hItem) {
    CFX_RectF rect;
    GetItemRect(iSel, rect);
    rtInvalidate.Union(rect);
    IFWL_ListItem* hSel = pData->GetItem(this, iSel);
    SetSelItem(hSel, true);
  }
  if (!rtInvalidate.IsEmpty()) {
    Repaint(&rtInvalidate);
  }
}

int32_t IFWL_ComboList::CountItems() {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  return pData ? pData->CountItems(this) : 0;
}

void IFWL_ComboList::GetItemRect(int32_t nIndex, CFX_RectF& rtItem) {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  IFWL_ListItem* hItem = pData->GetItem(this, nIndex);
  pData->GetItemRect(this, hItem, rtItem);
}

void IFWL_ComboList::ClientToOuter(FX_FLOAT& fx, FX_FLOAT& fy) {
  fx += m_pProperties->m_rtWidget.left, fy += m_pProperties->m_rtWidget.top;
  IFWL_Widget* pOwner = GetOwner();
  if (!pOwner)
    return;
  pOwner->TransformTo(m_pOuter, fx, fy);
}

void IFWL_ComboList::SetFocus(bool bSet) {
  IFWL_Widget::SetFocus(bSet);
}

void IFWL_ComboList::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  CFWL_MessageType dwHashCode = pMessage->GetClassID();
  bool backDefault = true;
  if (dwHashCode == CFWL_MessageType::SetFocus ||
      dwHashCode == CFWL_MessageType::KillFocus) {
    OnDropListFocusChanged(pMessage, dwHashCode == CFWL_MessageType::SetFocus);
  } else if (dwHashCode == CFWL_MessageType::Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (IsShowScrollBar(true) && m_pVertScrollBar) {
      CFX_RectF rect;
      m_pVertScrollBar->GetWidgetRect(rect);
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        pMsg->m_fx -= rect.left;
        pMsg->m_fy -= rect.top;
        m_pVertScrollBar->GetDelegate()->OnProcessMessage(pMsg);
        return;
      }
    }
    switch (pMsg->m_dwCmd) {
      case FWL_MouseCommand::Move: {
        backDefault = false;
        OnDropListMouseMove(pMsg);
        break;
      }
      case FWL_MouseCommand::LeftButtonDown: {
        backDefault = false;
        OnDropListLButtonDown(pMsg);
        break;
      }
      case FWL_MouseCommand::LeftButtonUp: {
        backDefault = false;
        OnDropListLButtonUp(pMsg);
        break;
      }
      default:
        break;
    }
  } else if (dwHashCode == CFWL_MessageType::Key) {
    backDefault = !OnDropListKey(static_cast<CFWL_MsgKey*>(pMessage));
  }
  if (backDefault)
    IFWL_ListBox::OnProcessMessage(pMessage);
}

void IFWL_ComboList::OnDropListFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (bSet)
    return;

  CFWL_MsgKillFocus* pKill = static_cast<CFWL_MsgKillFocus*>(pMsg);
  IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOuter);
  if (pKill->m_pSetFocus == m_pOuter ||
      pKill->m_pSetFocus == pOuter->GetComboEdit()) {
    pOuter->ShowDropList(false);
  }
}

int32_t IFWL_ComboList::OnDropListMouseMove(CFWL_MsgMouse* pMsg) {
  if (m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if (m_bNotifyOwner) {
      m_bNotifyOwner = false;
    }
    if (IsShowScrollBar(true) && m_pVertScrollBar) {
      CFX_RectF rect;
      m_pVertScrollBar->GetWidgetRect(rect);
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        return 1;
      }
    }
    IFWL_ListItem* hItem = GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (hItem) {
      if (!m_pProperties->m_pDataProvider)
        return 0;
      IFWL_ListBoxDP* pData =
          static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
      int32_t iSel = pData->GetItemIndex(this, hItem);
      CFWL_EvtCmbHoverChanged event;
      event.m_pSrcTarget = m_pOuter;
      event.m_iCurHover = iSel;
      DispatchEvent(&event);
      ChangeSelected(iSel);
    }
  } else if (m_bNotifyOwner) {
    ClientToOuter(pMsg->m_fx, pMsg->m_fy);
    IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOuter);
    pOuter->GetDelegate()->OnProcessMessage(pMsg);
  }
  return 1;
}

int32_t IFWL_ComboList::OnDropListLButtonDown(CFWL_MsgMouse* pMsg) {
  if (m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy))
    return 0;

  IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOuter);
  pOuter->ShowDropList(false);
  return 1;
}

int32_t IFWL_ComboList::OnDropListLButtonUp(CFWL_MsgMouse* pMsg) {
  IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOuter);
  if (m_bNotifyOwner) {
    ClientToOuter(pMsg->m_fx, pMsg->m_fy);
    pOuter->GetDelegate()->OnProcessMessage(pMsg);
  } else {
    if (IsShowScrollBar(true) && m_pVertScrollBar) {
      CFX_RectF rect;
      m_pVertScrollBar->GetWidgetRect(rect);
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        return 1;
      }
    }
    pOuter->ShowDropList(false);
    IFWL_ListItem* hItem = GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (hItem)
      pOuter->ProcessSelChanged(true);
  }
  return 1;
}

int32_t IFWL_ComboList::OnDropListKey(CFWL_MsgKey* pKey) {
  IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOuter);
  bool bPropagate = false;
  if (pKey->m_dwCmd == FWL_KeyCommand::KeyDown) {
    uint32_t dwKeyCode = pKey->m_dwKeyCode;
    switch (dwKeyCode) {
      case FWL_VKEY_Return:
      case FWL_VKEY_Escape: {
        pOuter->ShowDropList(false);
        return 1;
      }
      case FWL_VKEY_Up:
      case FWL_VKEY_Down: {
        OnDropListKeyDown(pKey);
        pOuter->ProcessSelChanged(false);
        return 1;
      }
      default: { bPropagate = true; }
    }
  } else if (pKey->m_dwCmd == FWL_KeyCommand::Char) {
    bPropagate = true;
  }
  if (bPropagate) {
    pKey->m_pDstTarget = m_pOuter;
    pOuter->GetDelegate()->OnProcessMessage(pKey);
    return 1;
  }
  return 0;
}

void IFWL_ComboList::OnDropListKeyDown(CFWL_MsgKey* pKey) {
  uint32_t dwKeyCode = pKey->m_dwKeyCode;
  switch (dwKeyCode) {
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOuter);
      IFWL_ListBoxDP* pData =
          static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
      IFWL_ListItem* hItem =
          pData->GetItem(this, pOuter->GetCurrentSelection());
      hItem = GetItem(hItem, dwKeyCode);
      if (!hItem) {
        break;
      }
      SetSelection(hItem, hItem, true);
      ScrollToVisible(hItem);
      CFX_RectF rtInvalidate;
      rtInvalidate.Set(0, 0, m_pProperties->m_rtWidget.width,
                       m_pProperties->m_rtWidget.height);
      Repaint(&rtInvalidate);
      break;
    }
    default:
      break;
  }
}
