// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_combolist.h"

#include "xfa/fwl/core/ifwl_combobox.h"
#include "xfa/fwl/core/ifwl_comboedit.h"

IFWL_ComboList::IFWL_ComboList(const IFWL_App* app,
                               const CFWL_WidgetImpProperties& properties,
                               IFWL_Widget* pOuter)
    : IFWL_ListBox(app, properties, pOuter), m_bNotifyOwner(TRUE) {
  ASSERT(pOuter);
}

void IFWL_ComboList::Initialize() {
  IFWL_ListBox::Initialize();

  // Delete the delegate that was created by IFWL_ListBox::Initialize ...
  delete m_pDelegate;
  m_pDelegate = new CFWL_ComboListImpDelegate(this);
}

void IFWL_ComboList::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  IFWL_ListBox::Finalize();
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
    SetSelItem(hOld, FALSE);
  }
  if (hItem) {
    CFX_RectF rect;
    GetItemRect(iSel, rect);
    rtInvalidate.Union(rect);
    IFWL_ListItem* hSel = pData->GetItem(this, iSel);
    SetSelItem(hSel, TRUE);
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

void IFWL_ComboList::SetFocus(FX_BOOL bSet) {
  IFWL_Widget::SetFocus(bSet);
}

CFWL_ComboListImpDelegate::CFWL_ComboListImpDelegate(IFWL_ComboList* pOwner)
    : CFWL_ListBoxImpDelegate(pOwner), m_pOwner(pOwner) {}

void CFWL_ComboListImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  CFWL_MessageType dwHashCode = pMessage->GetClassID();
  FX_BOOL backDefault = TRUE;
  if (dwHashCode == CFWL_MessageType::SetFocus ||
      dwHashCode == CFWL_MessageType::KillFocus) {
    OnDropListFocusChanged(pMessage, dwHashCode == CFWL_MessageType::SetFocus);
  } else if (dwHashCode == CFWL_MessageType::Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (m_pOwner->IsShowScrollBar(TRUE) && m_pOwner->m_pVertScrollBar) {
      CFX_RectF rect;
      m_pOwner->m_pVertScrollBar->GetWidgetRect(rect);
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        pMsg->m_fx -= rect.left;
        pMsg->m_fy -= rect.top;
        IFWL_WidgetDelegate* pDelegate =
            m_pOwner->m_pVertScrollBar->SetDelegate(nullptr);
        pDelegate->OnProcessMessage(pMsg);
        return;
      }
    }
    switch (pMsg->m_dwCmd) {
      case FWL_MouseCommand::Move: {
        backDefault = FALSE;
        OnDropListMouseMove(pMsg);
        break;
      }
      case FWL_MouseCommand::LeftButtonDown: {
        backDefault = FALSE;
        OnDropListLButtonDown(pMsg);
        break;
      }
      case FWL_MouseCommand::LeftButtonUp: {
        backDefault = FALSE;
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
    CFWL_ListBoxImpDelegate::OnProcessMessage(pMessage);
}

void CFWL_ComboListImpDelegate::OnDropListFocusChanged(CFWL_Message* pMsg,
                                                       FX_BOOL bSet) {
  if (!bSet) {
    CFWL_MsgKillFocus* pKill = static_cast<CFWL_MsgKillFocus*>(pMsg);
    IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOwner->m_pOuter);
    if (pKill->m_pSetFocus == m_pOwner->m_pOuter ||
        pKill->m_pSetFocus == pOuter->m_pEdit.get()) {
      pOuter->ShowDropList(FALSE);
    }
  }
}

int32_t CFWL_ComboListImpDelegate::OnDropListMouseMove(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if (m_pOwner->m_bNotifyOwner) {
      m_pOwner->m_bNotifyOwner = FALSE;
    }
    if (m_pOwner->IsShowScrollBar(TRUE) && m_pOwner->m_pVertScrollBar) {
      CFX_RectF rect;
      m_pOwner->m_pVertScrollBar->GetWidgetRect(rect);
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        return 1;
      }
    }
    IFWL_ListItem* hItem = m_pOwner->GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (hItem) {
      if (!m_pOwner->m_pProperties->m_pDataProvider)
        return 0;
      IFWL_ListBoxDP* pData = static_cast<IFWL_ListBoxDP*>(
          m_pOwner->m_pProperties->m_pDataProvider);
      int32_t iSel = pData->GetItemIndex(m_pOwner, hItem);
      CFWL_EvtCmbHoverChanged event;
      event.m_pSrcTarget = m_pOwner->m_pOuter;
      event.m_iCurHover = iSel;
      m_pOwner->DispatchEvent(&event);
      m_pOwner->ChangeSelected(iSel);
    }
  } else if (m_pOwner->m_bNotifyOwner) {
    m_pOwner->ClientToOuter(pMsg->m_fx, pMsg->m_fy);
    IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOwner->m_pOuter);
    pOuter->m_pDelegate->OnProcessMessage(pMsg);
  }
  return 1;
}

int32_t CFWL_ComboListImpDelegate::OnDropListLButtonDown(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    return 0;
  }
  IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOwner->m_pOuter);
  pOuter->ShowDropList(FALSE);
  return 1;
}

int32_t CFWL_ComboListImpDelegate::OnDropListLButtonUp(CFWL_MsgMouse* pMsg) {
  IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOwner->m_pOuter);
  if (m_pOwner->m_bNotifyOwner) {
    m_pOwner->ClientToOuter(pMsg->m_fx, pMsg->m_fy);
    pOuter->m_pDelegate->OnProcessMessage(pMsg);
  } else {
    if (m_pOwner->IsShowScrollBar(TRUE) && m_pOwner->m_pVertScrollBar) {
      CFX_RectF rect;
      m_pOwner->m_pVertScrollBar->GetWidgetRect(rect);
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        return 1;
      }
    }
    pOuter->ShowDropList(FALSE);
    IFWL_ListItem* hItem = m_pOwner->GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (hItem) {
      pOuter->ProcessSelChanged(TRUE);
    }
  }
  return 1;
}

int32_t CFWL_ComboListImpDelegate::OnDropListKey(CFWL_MsgKey* pKey) {
  IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOwner->m_pOuter);
  FX_BOOL bPropagate = FALSE;
  if (pKey->m_dwCmd == FWL_KeyCommand::KeyDown) {
    uint32_t dwKeyCode = pKey->m_dwKeyCode;
    switch (dwKeyCode) {
      case FWL_VKEY_Return:
      case FWL_VKEY_Escape: {
        pOuter->ShowDropList(FALSE);
        return 1;
      }
      case FWL_VKEY_Up:
      case FWL_VKEY_Down: {
        OnDropListKeyDown(pKey);
        pOuter->SetDelegate(nullptr);
        pOuter->ProcessSelChanged(FALSE);
        return 1;
      }
      default: { bPropagate = TRUE; }
    }
  } else if (pKey->m_dwCmd == FWL_KeyCommand::Char) {
    bPropagate = TRUE;
  }
  if (bPropagate) {
    pKey->m_pDstTarget = m_pOwner->m_pOuter;
    pOuter->m_pDelegate->OnProcessMessage(pKey);
    return 1;
  }
  return 0;
}

void CFWL_ComboListImpDelegate::OnDropListKeyDown(CFWL_MsgKey* pKey) {
  uint32_t dwKeyCode = pKey->m_dwKeyCode;
  switch (dwKeyCode) {
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      IFWL_ComboBox* pOuter = static_cast<IFWL_ComboBox*>(m_pOwner->m_pOuter);
      IFWL_ListBoxDP* pData = static_cast<IFWL_ListBoxDP*>(
          m_pOwner->m_pProperties->m_pDataProvider);
      IFWL_ListItem* hItem = pData->GetItem(m_pOwner, pOuter->m_iCurSel);
      hItem = m_pOwner->GetItem(hItem, dwKeyCode);
      if (!hItem) {
        break;
      }
      m_pOwner->SetSelection(hItem, hItem, TRUE);
      m_pOwner->ScrollToVisible(hItem);
      CFX_RectF rtInvalidate;
      rtInvalidate.Set(0, 0, m_pOwner->m_pProperties->m_rtWidget.width,
                       m_pOwner->m_pProperties->m_rtWidget.height);
      m_pOwner->Repaint(&rtInvalidate);
      break;
    }
    default:
      break;
  }
}
