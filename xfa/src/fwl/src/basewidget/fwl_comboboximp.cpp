// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_appimp.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_panelimp.h"
#include "xfa/src/fwl/src/core/include/fwl_formimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetmgrimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_scrollbarimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_editimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_listboximp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_formproxyimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_comboboximp.h"

// static
IFWL_ComboBox* IFWL_ComboBox::Create(
    const CFWL_WidgetImpProperties& properties) {
  IFWL_ComboBox* pComboBox = new IFWL_ComboBox;
  CFWL_ComboBoxImp* pComboBoxImpl = new CFWL_ComboBoxImp(properties, nullptr);
  pComboBox->SetImpl(pComboBoxImpl);
  pComboBoxImpl->SetInterface(pComboBox);
  return pComboBox;
}
IFWL_ComboBox::IFWL_ComboBox() {}
int32_t IFWL_ComboBox::GetCurSel() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->GetCurSel();
}
FWL_ERR IFWL_ComboBox::SetCurSel(int32_t iSel) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->SetCurSel(iSel);
}
FWL_ERR IFWL_ComboBox::SetEditText(const CFX_WideString& wsText) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->SetEditText(wsText);
}
int32_t IFWL_ComboBox::GetEditTextLength() const {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->GetEditTextLength();
}
FWL_ERR IFWL_ComboBox::GetEditText(CFX_WideString& wsText,
                                   int32_t nStart,
                                   int32_t nCount) const {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())
      ->GetEditText(wsText, nStart, nCount);
}
FWL_ERR IFWL_ComboBox::SetEditSelRange(int32_t nStart, int32_t nCount) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())
      ->SetEditSelRange(nStart, nCount);
}
int32_t IFWL_ComboBox::GetEditSelRange(int32_t nIndex, int32_t& nStart) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())
      ->GetEditSelRange(nIndex, nStart);
}
int32_t IFWL_ComboBox::GetEditLimit() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->GetEditLimit();
}
FWL_ERR IFWL_ComboBox::SetEditLimit(int32_t nLimit) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->SetEditLimit(nLimit);
}
FWL_ERR IFWL_ComboBox::EditDoClipboard(int32_t iCmd) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditDoClipboard(iCmd);
}
FX_BOOL IFWL_ComboBox::EditRedo(const CFX_ByteStringC& bsRecord) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditRedo(bsRecord);
}
FX_BOOL IFWL_ComboBox::EditUndo(const CFX_ByteStringC& bsRecord) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditUndo(bsRecord);
}
IFWL_ListBox* IFWL_ComboBox::GetListBoxt() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->GetListBoxt();
}
FX_BOOL IFWL_ComboBox::AfterFocusShowDropList() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->AfterFocusShowDropList();
}
FX_ERR IFWL_ComboBox::OpenDropDownList(FX_BOOL bActivate) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->OpenDropDownList(bActivate);
}
FX_BOOL IFWL_ComboBox::EditCanUndo() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditCanUndo();
}
FX_BOOL IFWL_ComboBox::EditCanRedo() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditCanRedo();
}
FX_BOOL IFWL_ComboBox::EditUndo() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditUndo();
}
FX_BOOL IFWL_ComboBox::EditRedo() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditRedo();
}
FX_BOOL IFWL_ComboBox::EditCanCopy() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditCanCopy();
}
FX_BOOL IFWL_ComboBox::EditCanCut() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditCanCut();
}
FX_BOOL IFWL_ComboBox::EditCanSelectAll() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditCanSelectAll();
}
FX_BOOL IFWL_ComboBox::EditCopy(CFX_WideString& wsCopy) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditCopy(wsCopy);
}
FX_BOOL IFWL_ComboBox::EditCut(CFX_WideString& wsCut) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditCut(wsCut);
}
FX_BOOL IFWL_ComboBox::EditPaste(const CFX_WideString& wsPaste) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditPaste(wsPaste);
}
FX_BOOL IFWL_ComboBox::EditSelectAll() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditSelectAll();
}
FX_BOOL IFWL_ComboBox::EditDelete() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditDelete();
}
FX_BOOL IFWL_ComboBox::EditDeSelect() {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->EditDeSelect();
}
FWL_ERR IFWL_ComboBox::GetBBox(CFX_RectF& rect) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())->GetBBox(rect);
}
FWL_ERR IFWL_ComboBox::EditModifyStylesEx(FX_DWORD dwStylesExAdded,
                                          FX_DWORD dwStylesExRemoved) {
  return static_cast<CFWL_ComboBoxImp*>(GetImpl())
      ->EditModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

CFWL_ComboEditImp::CFWL_ComboEditImp(const CFWL_WidgetImpProperties& properties,
                                     IFWL_Widget* pOuter)
    : CFWL_EditImp(properties, pOuter) {
  m_pOuter = static_cast<CFWL_ComboBoxImp*>(pOuter->GetImpl());
}

CFWL_ComboEditImpDelegate::CFWL_ComboEditImpDelegate(CFWL_ComboEditImp* pOwner)
    : CFWL_EditImpDelegate(pOwner), m_pOwner(pOwner) {
}
int32_t CFWL_ComboEditImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  FX_BOOL backDefault = TRUE;
  switch (dwMsgCode) {
    case FWL_MSGHASH_SetFocus:
    case FWL_MSGHASH_KillFocus: {
      if (dwMsgCode == FWL_MSGHASH_SetFocus) {
        m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
      } else {
        m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
      }
      backDefault = FALSE;
      break;
    }
    case FWL_MSGHASH_Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      if ((pMsg->m_dwCmd == FWL_MSGMOUSECMD_LButtonDown) &&
          ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)) {
        m_pOwner->SetSelected();
        m_pOwner->SetComboBoxFocus(TRUE);
      }
      break;
    }
    default: {}
  }
  if (!backDefault) {
    return 1;
  }
  return CFWL_EditImpDelegate::OnProcessMessage(pMessage);
}
void CFWL_ComboEditImp::ClearSelected() {
  ClearSelections();
  Repaint(&m_rtClient);
}
void CFWL_ComboEditImp::SetSelected() {
  FlagFocus(TRUE);
  EndCaret();
  AddSelRange(0);
}
void CFWL_ComboEditImp::EndCaret() {
  m_pEdtEngine->MoveCaretPos(MC_End);
}
void CFWL_ComboEditImp::FlagFocus(FX_BOOL bSet) {
  if (bSet) {
    m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
  } else {
    m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    ShowCaret(FALSE);
  }
};
void CFWL_ComboEditImp::SetComboBoxFocus(FX_BOOL bSet) {
  m_pOuter->SetFocus(bSet);
}
CFWL_ComboListImp::CFWL_ComboListImp(const CFWL_WidgetImpProperties& properties,
                                     IFWL_Widget* pOuter)
    : CFWL_ListBoxImp(properties, pOuter), m_bNotifyOwner(TRUE) {
  FXSYS_assert(pOuter != NULL);
}
FWL_ERR CFWL_ComboListImp::Initialize() {
  if (CFWL_ListBoxImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  delete m_pDelegate;
  m_pDelegate = new CFWL_ComboListImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboListImp::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_ListBoxImp::Finalize();
}
int32_t CFWL_ComboListImp::MatchItem(const CFX_WideString& wsMatch) {
  if (wsMatch.IsEmpty()) {
    return -1;
  }
  if (!m_pProperties->m_pDataProvider)
    return -1;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  int32_t iCount = pData->CountItems(m_pInterface);
  for (int32_t i = 0; i < iCount; i++) {
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, i);
    CFX_WideString wsText;
    pData->GetItemText(m_pInterface, hItem, wsText);
    FX_STRSIZE pos = wsText.Find(wsMatch.c_str());
    if (!pos) {
      return i;
    }
  }
  return -1;
}
void CFWL_ComboListImp::ChangeSelected(int32_t iSel) {
  if (!m_pProperties->m_pDataProvider)
    return;
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, iSel);
  CFX_RectF rtInvalidate;
  rtInvalidate.Reset();
  FWL_HLISTITEM hOld = GetSelItem(0);
  int32_t iOld = pData->GetItemIndex(m_pInterface, hOld);
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
    FWL_HLISTITEM hSel = pData->GetItem(m_pInterface, iSel);
    SetSelItem(hSel, TRUE);
  }
  if (!rtInvalidate.IsEmpty()) {
    Repaint(&rtInvalidate);
  }
}
int32_t CFWL_ComboListImp::CountItems() {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  return pData ? pData->CountItems(m_pInterface) : 0;
}
void CFWL_ComboListImp::GetItemRect(int32_t nIndex, CFX_RectF& rtItem) {
  IFWL_ListBoxDP* pData =
      static_cast<IFWL_ListBoxDP*>(m_pProperties->m_pDataProvider);
  FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, nIndex);
  pData->GetItemRect(m_pInterface, hItem, rtItem);
}
void CFWL_ComboListImp::ClientToOuter(FX_FLOAT& fx, FX_FLOAT& fy) {
  fx += m_pProperties->m_rtWidget.left, fy += m_pProperties->m_rtWidget.top;
  IFWL_Widget* pOwner = GetOwner();
  if (!pOwner)
    return;
  pOwner->TransformTo(m_pOuter, fx, fy);
}
void CFWL_ComboListImp::SetFocus(FX_BOOL bSet) {
  CFWL_WidgetImp::SetFocus(bSet);
}
CFWL_ComboListImpDelegate::CFWL_ComboListImpDelegate(CFWL_ComboListImp* pOwner)
    : CFWL_ListBoxImpDelegate(pOwner), m_pOwner(pOwner) {
}
int32_t CFWL_ComboListImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  FX_DWORD dwHashCode = pMessage->GetClassID();
  FX_BOOL backDefault = TRUE;
  if (dwHashCode == FWL_MSGHASH_SetFocus ||
      dwHashCode == FWL_MSGHASH_KillFocus) {
    OnDropListFocusChanged(pMessage, dwHashCode == FWL_MSGHASH_SetFocus);
  } else if (dwHashCode == FWL_MSGHASH_Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (m_pOwner->IsShowScrollBar(TRUE) && m_pOwner->m_pVertScrollBar) {
      CFX_RectF rect;
      m_pOwner->m_pVertScrollBar->GetWidgetRect(rect);
      if (rect.Contains(pMsg->m_fx, pMsg->m_fy)) {
        pMsg->m_fx -= rect.left;
        pMsg->m_fy -= rect.top;
        IFWL_WidgetDelegate* pDelegate =
            m_pOwner->m_pVertScrollBar->SetDelegate(NULL);
        return pDelegate->OnProcessMessage(pMsg);
      }
    }
    FX_DWORD dwCmd = pMsg->m_dwCmd;
    switch (dwCmd) {
      case FWL_MSGMOUSECMD_MouseMove: {
        backDefault = FALSE;
        OnDropListMouseMove(pMsg);
        break;
      }
      case FWL_MSGMOUSECMD_LButtonDown: {
        backDefault = FALSE;
        OnDropListLButtonDown(pMsg);
        break;
      }
      case FWL_MSGMOUSECMD_LButtonUp: {
        backDefault = FALSE;
        OnDropListLButtonUp(pMsg);
        break;
      }
      default: {}
    }
  } else if (dwHashCode == FWL_MSGHASH_Key) {
    backDefault = !OnDropListKey(static_cast<CFWL_MsgKey*>(pMessage));
  }
  if (!backDefault) {
    return 1;
  }
  return CFWL_ListBoxImpDelegate::OnProcessMessage(pMessage);
}
void CFWL_ComboListImpDelegate::OnDropListFocusChanged(CFWL_Message* pMsg,
                                                       FX_BOOL bSet) {
  if (!bSet) {
    CFWL_MsgKillFocus* pKill = static_cast<CFWL_MsgKillFocus*>(pMsg);
    CFWL_ComboBoxImp* pOuter =
        static_cast<CFWL_ComboBoxImp*>(m_pOwner->m_pOuter->GetImpl());
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
    FWL_HLISTITEM hItem = m_pOwner->GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (hItem) {
      if (!m_pOwner->m_pProperties->m_pDataProvider)
        return 0;
      IFWL_ListBoxDP* pData = static_cast<IFWL_ListBoxDP*>(
          m_pOwner->m_pProperties->m_pDataProvider);
      int32_t iSel = pData->GetItemIndex(m_pOwner->m_pInterface, hItem);
      CFWL_EvtCmbHoverChanged event;
      event.m_pSrcTarget = m_pOwner->m_pOuter;
      event.m_iCurHover = iSel;
      m_pOwner->DispatchEvent(&event);
      m_pOwner->ChangeSelected(iSel);
    }
  } else if (m_pOwner->m_bNotifyOwner) {
    m_pOwner->ClientToOuter(pMsg->m_fx, pMsg->m_fy);
    CFWL_ComboBoxImp* pOuter =
        static_cast<CFWL_ComboBoxImp*>(m_pOwner->m_pOuter->GetImpl());
    pOuter->m_pDelegate->OnProcessMessage(pMsg);
  }
  return 1;
}
int32_t CFWL_ComboListImpDelegate::OnDropListLButtonDown(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    return 0;
  }
  CFWL_ComboBoxImp* pOuter =
      static_cast<CFWL_ComboBoxImp*>(m_pOwner->m_pOuter->GetImpl());
  pOuter->ShowDropList(FALSE);
  return 1;
}
int32_t CFWL_ComboListImpDelegate::OnDropListLButtonUp(CFWL_MsgMouse* pMsg) {
  CFWL_ComboBoxImp* pOuter =
      static_cast<CFWL_ComboBoxImp*>(m_pOwner->m_pOuter->GetImpl());
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
    FWL_HLISTITEM hItem = m_pOwner->GetItemAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (hItem) {
      pOuter->ProcessSelChanged(TRUE);
    }
  }
  return 1;
}
int32_t CFWL_ComboListImpDelegate::OnDropListKey(CFWL_MsgKey* pKey) {
  CFWL_ComboBoxImp* pOuter =
      static_cast<CFWL_ComboBoxImp*>(m_pOwner->m_pOuter->GetImpl());
  FX_BOOL bPropagate = FALSE;
  if (pKey->m_dwCmd == FWL_MSGKEYCMD_KeyDown) {
    FX_DWORD dwKeyCode = pKey->m_dwKeyCode;
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
  } else if (pKey->m_dwCmd == FWL_MSGKEYCMD_Char) {
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
  FX_DWORD dwKeyCode = pKey->m_dwKeyCode;
  switch (dwKeyCode) {
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_End: {
      CFWL_ComboBoxImp* pOuter =
          static_cast<CFWL_ComboBoxImp*>(m_pOwner->m_pOuter->GetImpl());
      IFWL_ListBoxDP* pData = static_cast<IFWL_ListBoxDP*>(
          m_pOwner->m_pProperties->m_pDataProvider);
      FWL_HLISTITEM hItem =
          pData->GetItem(m_pOwner->m_pInterface, pOuter->m_iCurSel);
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
    default: {}
  }
}
CFWL_ComboBoxImp::CFWL_ComboBoxImp(const CFWL_WidgetImpProperties& properties,
                                   IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_pForm(NULL),
      m_bLButtonDown(FALSE),
      m_iCurSel(-1),
      m_iBtnState(FWL_PARTSTATE_CMB_Normal),
      m_fComboFormHandler(0),
      m_bNeedShowList(FALSE) {
  m_rtClient.Reset();
  m_rtBtn.Reset();
  m_rtHandler.Reset();
}
CFWL_ComboBoxImp::~CFWL_ComboBoxImp() {
}
FWL_ERR CFWL_ComboBoxImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_ComboBox;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ComboBoxImp::GetClassID() const {
  return FWL_CLASSHASH_ComboBox;
}
FWL_ERR CFWL_ComboBoxImp::Initialize() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_Initialize();
  }
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_WGTSTATE_Invisible;  // Probably a bug; not a FWL_ERR_ value.
  m_pDelegate = new CFWL_ComboBoxImpDelegate(this);
  CFWL_WidgetImpProperties prop;
  prop.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  prop.m_dwStyles |= FWL_WGTSTYLE_Border | FWL_WGTSTYLE_VScroll;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_ListItemIconText) {
    prop.m_dwStyleExes |= FWL_STYLEEXT_LTB_Icon;
  }
  prop.m_pDataProvider = m_pProperties->m_pDataProvider;
  m_pListBox.reset(IFWL_ListBox::CreateComboList(prop, m_pInterface));
  m_pListBox->Initialize();
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_DropDown) && !m_pEdit) {
    CFWL_WidgetImpProperties prop2;
    m_pEdit.reset(IFWL_Edit::CreateComboEdit(prop2, m_pInterface));
    m_pEdit->Initialize();
    static_cast<CFWL_EditImp*>(m_pEdit->GetImpl())->SetOuter(m_pInterface);
  }
  if (m_pEdit) {
    m_pEdit->SetParent(m_pInterface);
  }
  SetStates(m_pProperties->m_dwStates);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImp::Finalize() {
  if (m_pEdit) {
    m_pEdit->Finalize();
  }
  m_pListBox->Finalize();
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_ComboBoxImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Reset();
    FX_BOOL bIsDropDown = IsDropDownStyle();
    if (bIsDropDown && m_pEdit) {
      m_pEdit->GetWidgetRect(rect, TRUE);
    } else {
      rect.width = 100;
      rect.height = 16;
    }
    if (!m_pProperties->m_pThemeProvider) {
      ReSetTheme();
    }
    FX_FLOAT* pFWidth = static_cast<FX_FLOAT*>(
        GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth));
    if (!pFWidth)
      return FWL_ERR_Indefinite;
    rect.Inflate(0, 0, *pFWidth, 0);
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImp::ModifyStylesEx(FX_DWORD dwStylesExAdded,
                                         FX_DWORD dwStylesExRemoved) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
  }
  FX_BOOL bAddDropDown = dwStylesExAdded & FWL_STYLEEXT_CMB_DropDown;
  FX_BOOL bRemoveDropDown = dwStylesExRemoved & FWL_STYLEEXT_CMB_DropDown;
  if (bAddDropDown && !m_pEdit) {
    CFWL_WidgetImpProperties prop;
    m_pEdit.reset(IFWL_Edit::CreateComboEdit(prop, nullptr));
    m_pEdit->Initialize();
    static_cast<CFWL_EditImp*>(m_pEdit->GetImpl())->SetOuter(m_pInterface);
    m_pEdit->SetParent(m_pInterface);
  } else if (bRemoveDropDown && m_pEdit) {
    m_pEdit->SetStates(FWL_WGTSTATE_Invisible, TRUE);
  }
  return CFWL_WidgetImp::ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
FWL_ERR CFWL_ComboBoxImp::Update() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_Update();
  }
  if (IsLocked()) {
    return FWL_ERR_Indefinite;
  }
  ReSetTheme();
  FX_BOOL bDropDown = IsDropDownStyle();
  if (bDropDown && m_pEdit) {
    ReSetEditAlignment();
  }
  if (m_pProperties->m_pThemeProvider == NULL) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  Layout();
  CFWL_ThemePart part;
  part.m_pWidget = m_pInterface;
  m_fComboFormHandler =
      *static_cast<FX_FLOAT*>(m_pProperties->m_pThemeProvider->GetCapacity(
          &part, FWL_WGTCAPACITY_CMB_ComboFormHandler));
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ComboBoxImp::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_HitTest(fx, fy);
  }
  return CFWL_WidgetImp::HitTest(fx, fy);
}
FWL_ERR CFWL_ComboBoxImp::DrawWidget(CFX_Graphics* pGraphics,
                                     const CFX_Matrix* pMatrix) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_DrawWidget(pGraphics, pMatrix);
  }
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  FX_BOOL bIsDropDown = IsDropDownStyle();
  if (HasBorder()) {
    DrawBorder(pGraphics, FWL_PART_CMB_Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, FWL_PART_CMB_Edge, pTheme, pMatrix);
  }
  if (!bIsDropDown) {
    CFX_RectF rtTextBk(m_rtClient);
    rtTextBk.width -= m_rtBtn.width;
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = FWL_PART_CMB_Background;
    param.m_pGraphics = pGraphics;
    if (pMatrix) {
      param.m_matrix.Concat(*pMatrix);
    }
    param.m_rtPart = rtTextBk;
    if (m_iCurSel >= 0) {
      IFWL_ListBoxDP* pData = static_cast<IFWL_ListBoxDP*>(
          static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
              ->m_pProperties->m_pDataProvider);
      void* p = pData->GetItemData(m_pListBox.get(),
                                   pData->GetItem(m_pListBox.get(), m_iCurSel));
      if (p != NULL) {
        param.m_pData = p;
      }
    }
    if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
      param.m_dwStates = FWL_PARTSTATE_CMB_Disabled;
    } else if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) &&
               (m_iCurSel >= 0)) {
      param.m_dwStates = FWL_PARTSTATE_CMB_Selected;
    } else {
      param.m_dwStates = FWL_PARTSTATE_CMB_Normal;
    }
    pTheme->DrawBackground(&param);
    if (m_iCurSel >= 0) {
      if (!m_pListBox)
        return FWL_ERR_Indefinite;
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, m_iCurSel);
      static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
          ->GetItemText(hItem, wsText);
      CFWL_ThemeText param;
      param.m_pWidget = m_pInterface;
      param.m_iPart = FWL_PART_CMB_Caption;
      param.m_dwStates = m_iBtnState;
      param.m_pGraphics = pGraphics;
      param.m_matrix.Concat(*pMatrix);
      param.m_rtPart = rtTextBk;
      param.m_dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)
                             ? FWL_PARTSTATE_CMB_Selected
                             : FWL_PARTSTATE_CMB_Normal;
      param.m_wsText = wsText;
      param.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
      param.m_iTTOAlign = FDE_TTOALIGNMENT_CenterLeft;
      pTheme->DrawText(&param);
    }
  }
  {
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = FWL_PART_CMB_DropDownButton;
    param.m_dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
                           ? FWL_PARTSTATE_CMB_Disabled
                           : m_iBtnState;
    param.m_pGraphics = pGraphics;
    param.m_matrix.Concat(*pMatrix);
    param.m_rtPart = m_rtBtn;
    pTheme->DrawBackground(&param);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImp::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  if (!pThemeProvider)
    return FWL_ERR_Indefinite;
  m_pProperties->m_pThemeProvider = pThemeProvider;
  if (m_pListBox && pThemeProvider->IsValidWidget(m_pListBox.get())) {
    m_pListBox->SetThemeProvider(pThemeProvider);
  }
  if (m_pEdit && pThemeProvider->IsValidWidget(m_pEdit.get())) {
    m_pEdit->SetThemeProvider(pThemeProvider);
  }
  return FWL_ERR_Succeeded;
}
int32_t CFWL_ComboBoxImp::GetCurSel() {
  return m_iCurSel;
}
FWL_ERR CFWL_ComboBoxImp::SetCurSel(int32_t iSel) {
  int32_t iCount =
      static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())->CountItems();
  FX_BOOL bClearSel = iSel < 0 || iSel >= iCount;
  FX_BOOL bDropDown = IsDropDownStyle();
  if (bDropDown && m_pEdit) {
    if (bClearSel) {
      m_pEdit->SetText(CFX_WideString());
    } else {
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, iSel);
      static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
          ->GetItemText(hItem, wsText);
      m_pEdit->SetText(wsText);
    }
    m_pEdit->Update();
  }
  m_iCurSel = bClearSel ? -1 : iSel;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImp::SetStates(FX_DWORD dwStates, FX_BOOL bSet) {
  FX_BOOL bIsDropDown = IsDropDownStyle();
  if (bIsDropDown && m_pEdit) {
    m_pEdit->SetStates(dwStates, bSet);
  }
  if (m_pListBox) {
    m_pListBox->SetStates(dwStates, bSet);
  }
  return CFWL_WidgetImp::SetStates(dwStates, bSet);
}
FWL_ERR CFWL_ComboBoxImp::SetEditText(const CFX_WideString& wsText) {
  if (!m_pEdit)
    return FWL_ERR_Indefinite;
  m_pEdit->SetText(wsText);
  return m_pEdit->Update();
}
int32_t CFWL_ComboBoxImp::GetEditTextLength() const {
  if (!m_pEdit)
    return -1;
  return m_pEdit->GetTextLength();
}
FWL_ERR CFWL_ComboBoxImp::GetEditText(CFX_WideString& wsText,
                                      int32_t nStart,
                                      int32_t nCount) const {
  if (m_pEdit) {
    return m_pEdit->GetText(wsText, nStart, nCount);
  } else if (m_pListBox) {
    IFWL_ComboBoxDP* pData =
        static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, m_iCurSel);
    return m_pListBox->GetItemText(hItem, wsText);
  }
  return FWL_ERR_Indefinite;
}
FWL_ERR CFWL_ComboBoxImp::SetEditSelRange(int32_t nStart, int32_t nCount) {
  if (!m_pEdit)
    return FWL_ERR_Indefinite;
  static_cast<CFWL_ComboEditImp*>(m_pEdit->GetImpl())->ClearSelected();
  m_pEdit->AddSelRange(nStart, nCount);
  return FWL_ERR_Succeeded;
}
int32_t CFWL_ComboBoxImp::GetEditSelRange(int32_t nIndex, int32_t& nStart) {
  if (!m_pEdit)
    return -1;
  return m_pEdit->GetSelRange(nIndex, nStart);
}
int32_t CFWL_ComboBoxImp::GetEditLimit() {
  if (!m_pEdit)
    return -1;
  return m_pEdit->GetLimit();
}
FWL_ERR CFWL_ComboBoxImp::SetEditLimit(int32_t nLimit) {
  if (!m_pEdit)
    return FWL_ERR_Indefinite;
  return m_pEdit->SetLimit(nLimit);
}
FWL_ERR CFWL_ComboBoxImp::EditDoClipboard(int32_t iCmd) {
  if (!m_pEdit)
    return FWL_ERR_Indefinite;
  return m_pEdit->DoClipboard(iCmd);
}
FX_BOOL CFWL_ComboBoxImp::EditRedo(const CFX_ByteStringC& bsRecord) {
  if (!m_pEdit)
    return FALSE;
  return m_pEdit->Redo(bsRecord);
}
FX_BOOL CFWL_ComboBoxImp::EditUndo(const CFX_ByteStringC& bsRecord) {
  if (!m_pEdit)
    return FALSE;
  return m_pEdit->Undo(bsRecord);
}
IFWL_ListBox* CFWL_ComboBoxImp::GetListBoxt() {
  return m_pListBox.get();
}
FX_BOOL CFWL_ComboBoxImp::AfterFocusShowDropList() {
  if (!m_bNeedShowList) {
    return FALSE;
  }
  if (m_pEdit) {
    MatchEditText();
  }
  ShowDropList(TRUE);
  m_bNeedShowList = FALSE;
  return TRUE;
}
FX_ERR CFWL_ComboBoxImp::OpenDropDownList(FX_BOOL bActivate) {
  ShowDropList(bActivate);
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_ComboBoxImp::EditCanUndo() {
  return m_pEdit->CanUndo();
}
FX_BOOL CFWL_ComboBoxImp::EditCanRedo() {
  return m_pEdit->CanRedo();
}
FX_BOOL CFWL_ComboBoxImp::EditUndo() {
  return m_pEdit->Undo();
}
FX_BOOL CFWL_ComboBoxImp::EditRedo() {
  return m_pEdit->Redo();
}
FX_BOOL CFWL_ComboBoxImp::EditCanCopy() {
  return m_pEdit->CountSelRanges() > 0;
}
FX_BOOL CFWL_ComboBoxImp::EditCanCut() {
  if (m_pEdit->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly) {
    return FALSE;
  }
  return m_pEdit->CountSelRanges() > 0;
}
FX_BOOL CFWL_ComboBoxImp::EditCanSelectAll() {
  return m_pEdit->GetTextLength() > 0;
}
FX_BOOL CFWL_ComboBoxImp::EditCopy(CFX_WideString& wsCopy) {
  return m_pEdit->Copy(wsCopy);
}
FX_BOOL CFWL_ComboBoxImp::EditCut(CFX_WideString& wsCut) {
  return m_pEdit->Cut(wsCut);
}
FX_BOOL CFWL_ComboBoxImp::EditPaste(const CFX_WideString& wsPaste) {
  return m_pEdit->Paste(wsPaste);
}
FX_BOOL CFWL_ComboBoxImp::EditSelectAll() {
  return m_pEdit->AddSelRange(0) == FWL_ERR_Succeeded;
}
FX_BOOL CFWL_ComboBoxImp::EditDelete() {
  return m_pEdit->ClearText() == FWL_ERR_Succeeded;
}
FX_BOOL CFWL_ComboBoxImp::EditDeSelect() {
  return m_pEdit->ClearSelections() == FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImp::GetBBox(CFX_RectF& rect) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_GetBBox(rect);
  }
  rect = m_pProperties->m_rtWidget;
  if (m_pListBox && IsDropListShowed()) {
    CFX_RectF rtList;
    m_pListBox->GetWidgetRect(rtList);
    rtList.Offset(rect.left, rect.top);
    rect.Union(rtList);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImp::EditModifyStylesEx(FX_DWORD dwStylesExAdded,
                                             FX_DWORD dwStylesExRemoved) {
  if (m_pEdit != NULL) {
    return m_pEdit->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
  } else {
    return FWL_ERR_Parameter_Invalid;
  }
}
FX_FLOAT CFWL_ComboBoxImp::GetListHeight() {
  return static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider)
      ->GetListHeight(m_pInterface);
}
void CFWL_ComboBoxImp::DrawStretchHandler(CFX_Graphics* pGraphics,
                                          const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pGraphics = pGraphics;
  param.m_iPart = FWL_PART_CMB_StretcgHandler;
  param.m_dwStates = FWL_PARTSTATE_CMB_Normal;
  param.m_pWidget = m_pInterface;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix);
  }
  param.m_rtPart = m_rtHandler;
  m_pProperties->m_pThemeProvider->DrawBackground(&param);
}
void CFWL_ComboBoxImp::ShowDropList(FX_BOOL bActivate) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_ShowDropList(bActivate);
  }
  FX_BOOL bDropList = IsDropListShowed();
  if (bDropList == bActivate) {
    return;
  }
  if (!m_pForm) {
    InitProxyForm();
  }
  m_pListProxyDelegate->Reset();
  if (bActivate) {
    static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
        ->ChangeSelected(m_iCurSel);
    ReSetListItemAlignment();
    FX_DWORD dwStyleAdd = m_pProperties->m_dwStyleExes &
                          (FWL_STYLEEXT_CMB_Sort | FWL_STYLEEXT_CMB_OwnerDraw);
    m_pListBox->ModifyStylesEx(dwStyleAdd, 0);
    m_pListBox->GetWidgetRect(m_rtList, TRUE);
    FX_FLOAT fHeight = GetListHeight();
    if (fHeight > 0) {
      if (m_rtList.height > GetListHeight()) {
        m_rtList.height = GetListHeight();
        m_pListBox->ModifyStyles(FWL_WGTSTYLE_VScroll, 0);
      }
    }
    CFX_RectF rtAnchor;
    rtAnchor.Set(0, 0, m_pProperties->m_rtWidget.width,
                 m_pProperties->m_rtWidget.height);
    FX_FLOAT fMinHeight = 0;
    if (m_rtList.width < m_rtClient.width) {
      m_rtList.width = m_rtClient.width;
    }
    m_rtProxy = m_rtList;
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_ListDrag) {
      m_rtProxy.height += m_fComboFormHandler;
    }
    GetPopupPos(fMinHeight, m_rtProxy.height, rtAnchor, m_rtProxy);
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_ListDrag) {
      FX_FLOAT fx = 0;
      FX_FLOAT fy = m_rtClient.top + m_rtClient.height / 2;
      TransformTo(NULL, fx, fy);
      m_bUpFormHandler = fy > m_rtProxy.top;
      if (m_bUpFormHandler) {
        m_rtHandler.Set(0, 0, m_rtList.width, m_fComboFormHandler);
        m_rtList.top = m_fComboFormHandler;
      } else {
        m_rtHandler.Set(0, m_rtList.height, m_rtList.width,
                        m_fComboFormHandler);
      }
    }
    m_pForm->SetWidgetRect(m_rtProxy);
    m_pForm->Update();
    m_pListBox->SetWidgetRect(m_rtList);
    m_pListBox->Update();
    CFWL_EvtCmbPreDropDown ev;
    ev.m_pSrcTarget = m_pInterface;
    DispatchEvent(&ev);
    m_fItemHeight =
        static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())->m_fItemHeight;
    static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())->SetFocus(TRUE);
    m_pForm->DoModal();
    static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())->SetFocus(FALSE);
  } else {
    m_pForm->EndDoModal();
    CFWL_EvtCmbCloseUp ev;
    ev.m_pSrcTarget = m_pInterface;
    DispatchEvent(&ev);
    m_bLButtonDown = FALSE;
    static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())->m_bNotifyOwner =
        TRUE;
    SetFocus(TRUE);
  }
}
FX_BOOL CFWL_ComboBoxImp::IsDropListShowed() {
  return m_pForm && !(m_pForm->GetStates() & FWL_WGTSTATE_Invisible);
}
FX_BOOL CFWL_ComboBoxImp::IsDropDownStyle() const {
  return m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_DropDown;
}
void CFWL_ComboBoxImp::MatchEditText() {
  CFX_WideString wsText;
  m_pEdit->GetText(wsText);
  int32_t iMatch =
      static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())->MatchItem(wsText);
  if (iMatch != m_iCurSel) {
    static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
        ->ChangeSelected(iMatch);
    if (iMatch >= 0) {
      SynchrEditText(iMatch);
    }
  } else if (iMatch >= 0) {
    static_cast<CFWL_ComboEditImp*>(m_pEdit->GetImpl())->SetSelected();
  }
  m_iCurSel = iMatch;
}
void CFWL_ComboBoxImp::SynchrEditText(int32_t iListItem) {
  CFX_WideString wsText;
  IFWL_ComboBoxDP* pData =
      static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
  FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, iListItem);
  static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
      ->GetItemText(hItem, wsText);
  m_pEdit->SetText(wsText);
  m_pEdit->Update();
  static_cast<CFWL_ComboEditImp*>(m_pEdit->GetImpl())->SetSelected();
}
void CFWL_ComboBoxImp::Layout() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_Layout();
  }
  GetClientRect(m_rtClient);
  FX_FLOAT* pFWidth =
      static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth));
  if (!pFWidth)
    return;
  FX_FLOAT fBtn = *pFWidth;
  m_rtBtn.Set(m_rtClient.right() - fBtn, m_rtClient.top, fBtn,
              m_rtClient.height);
  FX_BOOL bIsDropDown = IsDropDownStyle();
  if (bIsDropDown && m_pEdit) {
    CFX_RectF rtEdit;
    rtEdit.Set(m_rtClient.left, m_rtClient.top, m_rtClient.width - fBtn,
               m_rtClient.height);
    m_pEdit->SetWidgetRect(rtEdit);
    if (m_iCurSel >= 0) {
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, m_iCurSel);
      static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
          ->GetItemText(hItem, wsText);
      m_pEdit->LockUpdate();
      m_pEdit->SetText(wsText);
      m_pEdit->UnlockUpdate();
    }
    m_pEdit->Update();
  }
}
void CFWL_ComboBoxImp::ReSetTheme() {
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (!pTheme) {
    pTheme = GetAvailableTheme();
    m_pProperties->m_pThemeProvider = pTheme;
  }
  if (m_pListBox) {
    if (!m_pListBox->GetThemeProvider() &&
        pTheme->IsValidWidget(m_pListBox.get())) {
      m_pListBox->SetThemeProvider(pTheme);
    }
  }
  if (m_pEdit) {
    if (!m_pEdit->GetThemeProvider() && pTheme->IsValidWidget(m_pEdit.get())) {
      m_pEdit->SetThemeProvider(pTheme);
    }
  }
}
void CFWL_ComboBoxImp::ReSetEditAlignment() {
  if (!m_pEdit)
    return;
  FX_DWORD dwStylExes = m_pProperties->m_dwStyleExes;
  FX_DWORD dwAdd = 0;
  switch (dwStylExes & FWL_STYLEEXT_CMB_EditHAlignMask) {
    case FWL_STYLEEXT_CMB_EditHCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_HCenter;
      break;
    }
    case FWL_STYLEEXT_CMB_EditHFar: {
      dwAdd |= FWL_STYLEEXT_EDT_HFar;
      break;
    }
    default: { dwAdd |= FWL_STYLEEXT_EDT_HNear; }
  }
  switch (dwStylExes & FWL_STYLEEXT_CMB_EditVAlignMask) {
    case FWL_STYLEEXT_CMB_EditVCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_VCenter;
      break;
    }
    case FWL_STYLEEXT_CMB_EditVFar: {
      dwAdd |= FWL_STYLEEXT_EDT_VFar;
      break;
    }
    default: { dwAdd |= FWL_STYLEEXT_EDT_VNear; }
  }
  if (dwStylExes & FWL_STYLEEXT_CMB_EditJustified) {
    dwAdd |= FWL_STYLEEXT_EDT_Justified;
  }
  if (dwStylExes & FWL_STYLEEXT_CMB_EditDistributed) {
    dwAdd |= FWL_STYLEEXT_EDT_Distributed;
  }
  m_pEdit->ModifyStylesEx(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                     FWL_STYLEEXT_EDT_HAlignModeMask |
                                     FWL_STYLEEXT_EDT_VAlignMask);
}
void CFWL_ComboBoxImp::ReSetListItemAlignment() {
  if (!m_pListBox)
    return;
  FX_DWORD dwStylExes = m_pProperties->m_dwStyleExes;
  FX_DWORD dwAdd = 0;
  switch (dwStylExes & FWL_STYLEEXT_CMB_ListItemAlignMask) {
    case FWL_STYLEEXT_CMB_ListItemCenterAlign: {
      dwAdd |= FWL_STYLEEXT_LTB_CenterAlign;
    }
    case FWL_STYLEEXT_CMB_ListItemRightAlign: {
      dwAdd |= FWL_STYLEEXT_LTB_RightAlign;
    }
    default: { dwAdd |= FWL_STYLEEXT_LTB_LeftAlign; }
  }
  m_pListBox->ModifyStylesEx(dwAdd, FWL_STYLEEXT_CMB_ListItemAlignMask);
}
void CFWL_ComboBoxImp::ProcessSelChanged(FX_BOOL bLButtonUp) {
  IFWL_ComboBoxDP* pDatas =
      static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
  m_iCurSel = pDatas->GetItemIndex(m_pInterface, m_pListBox->GetSelItem(0));
  FX_BOOL bDropDown = IsDropDownStyle();
  if (bDropDown) {
    IFWL_ComboBoxDP* pData =
        static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
    FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, m_iCurSel);
    if (hItem) {
      CFX_WideString wsText;
      pData->GetItemText(m_pInterface, hItem, wsText);
      if (m_pEdit) {
        m_pEdit->SetText(wsText);
        m_pEdit->Update();
        static_cast<CFWL_ComboEditImp*>(m_pEdit->GetImpl())->SetSelected();
      }
      CFWL_EvtCmbSelChanged ev;
      ev.bLButtonUp = bLButtonUp;
      ev.m_pSrcTarget = m_pInterface;
      ev.iArraySels.Add(m_iCurSel);
      DispatchEvent(&ev);
    }
  } else {
    Repaint(&m_rtClient);
  }
}
void CFWL_ComboBoxImp::InitProxyForm() {
  if (m_pForm)
    return;
  if (!m_pListBox)
    return;
  CFWL_WidgetImpProperties propForm;
  propForm.m_pOwner = m_pInterface;
  propForm.m_dwStyles = FWL_WGTSTYLE_Popup;
  propForm.m_dwStates = FWL_WGTSTATE_Invisible;
  CFX_WideString className;
  m_pForm = IFWL_Form::CreateFormProxy(propForm, &className, m_pListBox.get());
  m_pForm->Initialize();
  m_pProxy = static_cast<CFWL_FormProxyImp*>(m_pForm->GetImpl());
  m_pListBox->SetParent(m_pForm);
  m_pListProxyDelegate = new CFWL_ComboProxyImpDelegate(m_pForm, this);
  m_pProxy->SetDelegate(m_pListProxyDelegate);
}
FWL_ERR CFWL_ComboBoxImp::DisForm_Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_WGTSTATE_Invisible;  // Ditto.
  m_pDelegate = new CFWL_ComboBoxImpDelegate(this);
  DisForm_InitComboList();
  DisForm_InitComboEdit();
  return FWL_ERR_Succeeded;
}
void CFWL_ComboBoxImp::DisForm_InitComboList() {
  if (m_pListBox) {
    return;
  }
  CFWL_WidgetImpProperties prop;
  prop.m_pParent = m_pInterface;
  prop.m_dwStyles = FWL_WGTSTYLE_Border | FWL_WGTSTYLE_VScroll;
  prop.m_dwStates = FWL_WGTSTATE_Invisible;
  prop.m_pDataProvider = m_pProperties->m_pDataProvider;
  prop.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pListBox.reset(IFWL_ListBox::CreateComboList(prop, m_pInterface));
  m_pListBox->Initialize();
}
void CFWL_ComboBoxImp::DisForm_InitComboEdit() {
  if (m_pEdit) {
    return;
  }
  CFWL_WidgetImpProperties prop;
  prop.m_pParent = m_pInterface;
  prop.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pEdit.reset(IFWL_Edit::CreateComboEdit(prop, m_pInterface));
  m_pEdit->Initialize();
  static_cast<CFWL_ComboEditImp*>(m_pEdit->GetImpl())->SetOuter(m_pInterface);
}
void CFWL_ComboBoxImp::DisForm_ShowDropList(FX_BOOL bActivate) {
  FX_BOOL bDropList = DisForm_IsDropListShowed();
  if (bDropList == bActivate) {
    return;
  }
  if (bActivate) {
    CFWL_EvtCmbPreDropDown preEvent;
    preEvent.m_pSrcTarget = m_pInterface;
    DispatchEvent(&preEvent);
    CFWL_ComboListImp* pComboList =
        static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl());
    int32_t iItems = pComboList->CountItems();
    if (iItems < 1) {
      return;
    }
    ReSetListItemAlignment();
    pComboList->ChangeSelected(m_iCurSel);
    FX_FLOAT fItemHeight = pComboList->GetItemHeigt();
    FX_FLOAT fBorder = GetBorderSize();
    FX_FLOAT fPopupMin = 0.0f;
    if (iItems > 3) {
      fPopupMin = fItemHeight * 3 + fBorder * 2;
    }
    FX_FLOAT fPopupMax = fItemHeight * iItems + fBorder * 2;
    CFX_RectF rtList;
    rtList.left = m_rtClient.left;
    rtList.width = m_pProperties->m_rtWidget.width;
    rtList.top = 0;
    rtList.height = 0;
    GetPopupPos(fPopupMin, fPopupMax, m_pProperties->m_rtWidget, rtList);
    m_pListBox->SetWidgetRect(rtList);
    m_pListBox->Update();
  } else {
    SetFocus(TRUE);
  }
  m_pListBox->SetStates(FWL_WGTSTATE_Invisible, !bActivate);
  if (bActivate) {
    CFWL_EvtCmbPostDropDown postEvent;
    postEvent.m_pSrcTarget = m_pInterface;
    DispatchEvent(&postEvent);
  }
  CFX_RectF rect;
  m_pListBox->GetWidgetRect(rect);
  rect.Inflate(2, 2);
  Repaint(&rect);
}
FX_BOOL CFWL_ComboBoxImp::DisForm_IsDropListShowed() {
  return !(m_pListBox->GetStates() & FWL_WGTSTATE_Invisible);
}
FWL_ERR CFWL_ComboBoxImp::DisForm_ModifyStylesEx(FX_DWORD dwStylesExAdded,
                                                 FX_DWORD dwStylesExRemoved) {
  if (!m_pEdit) {
    DisForm_InitComboEdit();
  }
  FX_BOOL bAddDropDown = dwStylesExAdded & FWL_STYLEEXT_CMB_DropDown;
  FX_BOOL bDelDropDown = dwStylesExRemoved & FWL_STYLEEXT_CMB_DropDown;
  dwStylesExRemoved &= ~FWL_STYLEEXT_CMB_DropDown;
  m_pProperties->m_dwStyleExes |= FWL_STYLEEXT_CMB_DropDown;
  if (bAddDropDown) {
    m_pEdit->ModifyStylesEx(0, FWL_STYLEEXT_EDT_ReadOnly);
  } else if (bDelDropDown) {
    m_pEdit->ModifyStylesEx(FWL_STYLEEXT_EDT_ReadOnly, 0);
  }
  return CFWL_WidgetImp::ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
FWL_ERR CFWL_ComboBoxImp::DisForm_Update() {
  if (m_iLock) {
    return FWL_ERR_Indefinite;
  }
  if (m_pEdit) {
    ReSetEditAlignment();
  }
  ReSetTheme();
  Layout();
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_ComboBoxImp::DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  CFX_RectF rect;
  rect.Set(0, 0, m_pProperties->m_rtWidget.width - m_rtBtn.width,
           m_pProperties->m_rtWidget.height);
  if (rect.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Edit;
  }
  if (m_rtBtn.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Client;
  }
  if (DisForm_IsDropListShowed()) {
    m_pListBox->GetWidgetRect(rect);
    if (rect.Contains(fx, fy)) {
      return FWL_WGTHITTEST_Client;
    }
  }
  return FWL_WGTHITTEST_Unknown;
}
FWL_ERR CFWL_ComboBoxImp::DisForm_DrawWidget(CFX_Graphics* pGraphics,
                                             const CFX_Matrix* pMatrix) {
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  CFX_Matrix mtOrg;
  mtOrg.Set(1, 0, 0, 1, 0, 0);
  if (pMatrix) {
    mtOrg = *pMatrix;
  }
  FX_BOOL bListShowed = m_pListBox && DisForm_IsDropListShowed();
  pGraphics->SaveGraphState();
  pGraphics->ConcatMatrix(&mtOrg);
  if (!m_rtBtn.IsEmpty(0.1f)) {
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = FWL_PART_CMB_DropDownButton;
    param.m_dwStates = m_iBtnState;
    param.m_pGraphics = pGraphics;
    param.m_rtPart = m_rtBtn;
    pTheme->DrawBackground(&param);
  }
  pGraphics->RestoreGraphState();
  if (m_pEdit) {
    CFX_RectF rtEdit;
    m_pEdit->GetWidgetRect(rtEdit);
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, rtEdit.left, rtEdit.top);
    mt.Concat(mtOrg);
    m_pEdit->DrawWidget(pGraphics, &mt);
  }
  if (bListShowed) {
    CFX_RectF rtList;
    m_pListBox->GetWidgetRect(rtList);
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, rtList.left, rtList.top);
    mt.Concat(mtOrg);
    m_pListBox->DrawWidget(pGraphics, &mt);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImp::DisForm_GetBBox(CFX_RectF& rect) {
  rect = m_pProperties->m_rtWidget;
  if (m_pListBox && DisForm_IsDropListShowed()) {
    CFX_RectF rtList;
    m_pListBox->GetWidgetRect(rtList);
    rtList.Offset(rect.left, rect.top);
    rect.Union(rtList);
  }
  return FWL_ERR_Succeeded;
}
void CFWL_ComboBoxImp::DisForm_Layout() {
  GetClientRect(m_rtClient);
  m_rtContent = m_rtClient;
  FX_FLOAT* pFWidth =
      static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth));
  if (!pFWidth)
    return;
  FX_FLOAT borderWidth = 0;
  { borderWidth = FWL_PART_CMB_Border; }
  FX_FLOAT fBtn = *pFWidth;
  if (!(this->GetStylesEx() & FWL_STYLEEXT_CMB_ReadOnly)) {
    m_rtBtn.Set(m_rtClient.right() - fBtn, m_rtClient.top + borderWidth,
                fBtn - borderWidth, m_rtClient.height - 2 * borderWidth);
  }
  CFX_RectF* pUIMargin =
      static_cast<CFX_RectF*>(GetThemeCapacity(FWL_WGTCAPACITY_UIMargin));
  if (pUIMargin) {
    m_rtContent.Deflate(pUIMargin->left, pUIMargin->top, pUIMargin->width,
                        pUIMargin->height);
  }
  FX_BOOL bIsDropDown = IsDropDownStyle();
  if (bIsDropDown && m_pEdit) {
    CFX_RectF rtEdit;
    rtEdit.Set(m_rtContent.left, m_rtContent.top, m_rtContent.width - fBtn,
               m_rtContent.height);
    m_pEdit->SetWidgetRect(rtEdit);
    if (m_iCurSel >= 0) {
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      FWL_HLISTITEM hItem = pData->GetItem(m_pInterface, m_iCurSel);
      static_cast<CFWL_ComboListImp*>(m_pListBox->GetImpl())
          ->GetItemText(hItem, wsText);
      m_pEdit->LockUpdate();
      m_pEdit->SetText(wsText);
      m_pEdit->UnlockUpdate();
    }
    m_pEdit->Update();
  }
}
CFWL_ComboBoxImpDelegate::CFWL_ComboBoxImpDelegate(CFWL_ComboBoxImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_ComboBoxImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (m_pOwner->m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_OnProcessMessage(pMessage);
  }
  if (!pMessage)
    return 0;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  FX_BOOL iRet = 1;
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
        case FWL_MSGMOUSECMD_MouseMove: {
          OnMouseMove(pMsg);
          break;
        }
        case FWL_MSGMOUSECMD_MouseLeave: {
          OnMouseLeave(pMsg);
          break;
        }
        default: {}
      }
      break;
    }
    case FWL_MSGHASH_Key: {
      OnKey(static_cast<CFWL_MsgKey*>(pMessage));
      break;
    }
    default: { iRet = 0; }
  }
  CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
  return iRet;
}
FWL_ERR CFWL_ComboBoxImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {
  FX_DWORD dwFlag = pEvent->GetClassID();
  if (dwFlag == FWL_EVTHASH_LTB_DrawItem) {
    CFWL_EvtLtbDrawItem* pDrawItemEvent =
        static_cast<CFWL_EvtLtbDrawItem*>(pEvent);
    CFWL_EvtCmbDrawItem pTemp;
    pTemp.m_pSrcTarget = m_pOwner->m_pInterface;
    pTemp.m_pGraphics = pDrawItemEvent->m_pGraphics;
    pTemp.m_index = pDrawItemEvent->m_index;
    pTemp.m_rtItem = pDrawItemEvent->m_rect;
    m_pOwner->DispatchEvent(&pTemp);
  } else if (dwFlag == FWL_EVTHASH_Scroll) {
    CFWL_EvtScroll* pScrollEvent = static_cast<CFWL_EvtScroll*>(pEvent);
    CFWL_EvtScroll pScrollEv;
    pScrollEv.m_pSrcTarget = m_pOwner->m_pInterface;
    pScrollEv.m_iScrollCode = pScrollEvent->m_iScrollCode;
    pScrollEv.m_fPos = pScrollEvent->m_fPos;
    m_pOwner->DispatchEvent(&pScrollEv);
  } else if (dwFlag == FWL_EVTHASH_EDT_TextChanged) {
    CFWL_EvtEdtTextChanged* pTextChangedEvent =
        static_cast<CFWL_EvtEdtTextChanged*>(pEvent);
    CFWL_EvtCmbEditChanged pTemp;
    pTemp.m_pSrcTarget = m_pOwner->m_pInterface;
    pTemp.wsInsert = pTextChangedEvent->wsInsert;
    pTemp.wsDelete = pTextChangedEvent->wsDelete;
    pTemp.nChangeType = pTextChangedEvent->nChangeType;
    m_pOwner->DispatchEvent(&pTemp);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ComboBoxImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                               const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_ComboBoxImpDelegate::OnFocusChanged(CFWL_Message* pMsg,
                                              FX_BOOL bSet) {
  IFWL_Target* pDstTarget = pMsg->m_pDstTarget;
  IFWL_Target* pSrcTarget = pMsg->m_pSrcTarget;
  FX_BOOL bDropDown = m_pOwner->IsDropDownStyle();
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
    if (bDropDown && pSrcTarget != m_pOwner->m_pListBox.get()) {
      if (!m_pOwner->m_pEdit)
        return;
      static_cast<CFWL_ComboEditImp*>(m_pOwner->m_pEdit->GetImpl())
          ->SetSelected();
    } else {
      m_pOwner->Repaint(&m_pOwner->m_rtClient);
    }
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    if (bDropDown && pDstTarget != m_pOwner->m_pListBox.get()) {
      if (!m_pOwner->m_pEdit)
        return;
      static_cast<CFWL_ComboEditImp*>(m_pOwner->m_pEdit->GetImpl())
          ->FlagFocus(FALSE);
      static_cast<CFWL_ComboEditImp*>(m_pOwner->m_pEdit->GetImpl())
          ->ClearSelected();
    } else {
      m_pOwner->Repaint(&m_pOwner->m_rtClient);
    }
  }
}
void CFWL_ComboBoxImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
    return;
  }
  FX_BOOL bDropDown = m_pOwner->IsDropDownStyle();
  CFX_RectF& rtBtn = bDropDown ? m_pOwner->m_rtBtn : m_pOwner->m_rtClient;
  FX_BOOL bClickBtn = rtBtn.Contains(pMsg->m_fx, pMsg->m_fy);
  if (bClickBtn) {
    if (bDropDown && m_pOwner->m_pEdit) {
      m_pOwner->MatchEditText();
    }
    m_pOwner->m_bLButtonDown = TRUE;
    m_pOwner->m_iBtnState = FWL_PARTSTATE_CMB_Pressed;
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
    m_pOwner->ShowDropList(TRUE);
    m_pOwner->m_iBtnState = FWL_PARTSTATE_CMB_Normal;
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  }
}
void CFWL_ComboBoxImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  m_pOwner->m_bLButtonDown = FALSE;
  if (m_pOwner->m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iBtnState = FWL_PARTSTATE_CMB_Hovered;
  } else {
    m_pOwner->m_iBtnState = FWL_PARTSTATE_CMB_Normal;
  }
  m_pOwner->Repaint(&m_pOwner->m_rtBtn);
}
void CFWL_ComboBoxImpDelegate::OnMouseMove(CFWL_MsgMouse* pMsg) {
  int32_t iOldState = m_pOwner->m_iBtnState;
  if (m_pOwner->m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iBtnState = m_pOwner->m_bLButtonDown
                                ? FWL_PARTSTATE_CMB_Pressed
                                : FWL_PARTSTATE_CMB_Hovered;
  } else {
    m_pOwner->m_iBtnState = FWL_PARTSTATE_CMB_Normal;
  }
  if ((iOldState != m_pOwner->m_iBtnState) &&
      !((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) ==
        FWL_WGTSTATE_Disabled)) {
    m_pOwner->Repaint(&m_pOwner->m_rtBtn);
  }
}
void CFWL_ComboBoxImpDelegate::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  if (!m_pOwner->IsDropListShowed() &&
      !((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) ==
        FWL_WGTSTATE_Disabled)) {
    m_pOwner->m_iBtnState = FWL_PARTSTATE_CMB_Normal;
    m_pOwner->Repaint(&m_pOwner->m_rtBtn);
  }
}
void CFWL_ComboBoxImpDelegate::OnKey(CFWL_MsgKey* pMsg) {
  FX_DWORD dwKeyCode = pMsg->m_dwKeyCode;
  if (dwKeyCode == FWL_VKEY_Tab) {
    m_pOwner->DispatchKeyEvent(pMsg);
    return;
  }
  FX_BOOL bSubCtrlKey = pMsg->m_pDstTarget == m_pOwner->m_pInterface;
  if (bSubCtrlKey) {
    DoSubCtrlKey(pMsg);
  }
}
void CFWL_ComboBoxImpDelegate::DoSubCtrlKey(CFWL_MsgKey* pMsg) {
  FX_DWORD dwKeyCode = pMsg->m_dwKeyCode;
  FX_BOOL bUp = dwKeyCode == FWL_VKEY_Up;
  FX_BOOL bDown = dwKeyCode == FWL_VKEY_Down;
  if (bUp || bDown) {
    int32_t iCount = static_cast<CFWL_ComboListImp*>(
                         m_pOwner->m_pListBox->GetImpl())->CountItems();
    if (iCount < 1) {
      return;
    }
    FX_BOOL bMatchEqual = FALSE;
    int32_t iCurSel = m_pOwner->m_iCurSel;
    FX_BOOL bDropDown = m_pOwner->IsDropDownStyle();
    if (bDropDown && m_pOwner->m_pEdit) {
      CFX_WideString wsText;
      m_pOwner->m_pEdit->GetText(wsText);
      iCurSel = static_cast<CFWL_ComboListImp*>(m_pOwner->m_pListBox->GetImpl())
                    ->MatchItem(wsText);
      if (iCurSel >= 0) {
        CFX_WideString wsTemp;
        IFWL_ComboBoxDP* pData = static_cast<IFWL_ComboBoxDP*>(
            m_pOwner->m_pProperties->m_pDataProvider);
        FWL_HLISTITEM hItem = pData->GetItem(m_pOwner->m_pInterface, iCurSel);
        static_cast<CFWL_ComboListImp*>(m_pOwner->m_pListBox->GetImpl())
            ->GetItemText(hItem, wsTemp);
        bMatchEqual = wsText.Equal(wsTemp);
      }
    }
    if (iCurSel < 0) {
      iCurSel = 0;
    } else if (!bDropDown || bMatchEqual) {
      if ((bUp && iCurSel == 0) || (bDown && iCurSel == iCount - 1)) {
        return;
      }
      if (bUp) {
        iCurSel--;
      } else {
        iCurSel++;
      }
    }
    m_pOwner->m_iCurSel = iCurSel;
    if (bDropDown && m_pOwner->m_pEdit) {
      m_pOwner->SynchrEditText(m_pOwner->m_iCurSel);
    } else {
      m_pOwner->Repaint(&m_pOwner->m_rtClient);
    }
    return;
  }
  FX_BOOL bDropDown = m_pOwner->IsDropDownStyle();
  if (bDropDown) {
    IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pEdit->SetDelegate(NULL);
    pDelegate->OnProcessMessage(pMsg);
  }
}
int32_t CFWL_ComboBoxImpDelegate::DisForm_OnProcessMessage(
    CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  FX_BOOL backDefault = TRUE;
  switch (dwMsgCode) {
    case FWL_MSGHASH_SetFocus:
    case FWL_MSGHASH_KillFocus: {
      backDefault = FALSE;
      DisForm_OnFocusChanged(pMessage, dwMsgCode == FWL_MSGHASH_SetFocus);
      break;
    }
    case FWL_MSGHASH_Mouse: {
      backDefault = FALSE;
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      FX_DWORD dwCmd = pMsg->m_dwCmd;
      switch (dwCmd) {
        case FWL_MSGMOUSECMD_LButtonDown: {
          DisForm_OnLButtonDown(pMsg);
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
    case FWL_MSGHASH_Key: {
      backDefault = FALSE;
      CFWL_MsgKey* pKey = static_cast<CFWL_MsgKey*>(pMessage);
      if (pKey->m_dwCmd == FWL_MSGKEYCMD_KeyUp) {
        break;
      }
      if (m_pOwner->DisForm_IsDropListShowed() &&
          pKey->m_dwCmd == FWL_MSGKEYCMD_KeyDown) {
        FX_DWORD dwKeyCode = pKey->m_dwKeyCode;
        FX_BOOL bListKey =
            dwKeyCode == FWL_VKEY_Up || dwKeyCode == FWL_VKEY_Down ||
            dwKeyCode == FWL_VKEY_Return || dwKeyCode == FWL_VKEY_Escape;
        if (bListKey) {
          IFWL_WidgetDelegate* pDelegate =
              m_pOwner->m_pListBox->SetDelegate(NULL);
          pDelegate->OnProcessMessage(pMessage);
          break;
        }
      }
      DisForm_OnKey(pKey);
      break;
    }
    default: {}
  }
  if (!backDefault) {
    return 1;
  }
  return CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
}
void CFWL_ComboBoxImpDelegate::DisForm_OnLButtonDown(CFWL_MsgMouse* pMsg) {
  FX_BOOL bDropDown = m_pOwner->DisForm_IsDropListShowed();
  CFX_RectF& rtBtn = bDropDown ? m_pOwner->m_rtBtn : m_pOwner->m_rtClient;
  FX_BOOL bClickBtn = rtBtn.Contains(pMsg->m_fx, pMsg->m_fy);
  if (bClickBtn) {
    if (m_pOwner->DisForm_IsDropListShowed()) {
      m_pOwner->DisForm_ShowDropList(FALSE);
      return;
    }
    {
      if (m_pOwner->m_pEdit) {
        m_pOwner->MatchEditText();
      }
      m_pOwner->DisForm_ShowDropList(TRUE);
    }
  }
}
void CFWL_ComboBoxImpDelegate::DisForm_OnFocusChanged(CFWL_Message* pMsg,
                                                      FX_BOOL bSet) {
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
    if ((m_pOwner->m_pEdit->GetStates() & FWL_WGTSTATE_Focused) == 0) {
      CFWL_MsgSetFocus msg;
      msg.m_pDstTarget = m_pOwner->m_pEdit.get();
      msg.m_pSrcTarget = NULL;
      IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pEdit->SetDelegate(NULL);
      pDelegate->OnProcessMessage(&msg);
    }
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    m_pOwner->DisForm_ShowDropList(FALSE);
    CFWL_MsgKillFocus msg;
    msg.m_pDstTarget = NULL;
    msg.m_pSrcTarget = m_pOwner->m_pEdit.get();
    IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pEdit->SetDelegate(NULL);
    pDelegate->OnProcessMessage(&msg);
  }
}
void CFWL_ComboBoxImpDelegate::DisForm_OnKey(CFWL_MsgKey* pMsg) {
  FX_DWORD dwKeyCode = pMsg->m_dwKeyCode;
  FX_BOOL bUp = dwKeyCode == FWL_VKEY_Up;
  FX_BOOL bDown = dwKeyCode == FWL_VKEY_Down;
  if (bUp || bDown) {
    CFWL_ComboListImp* pComboList =
        static_cast<CFWL_ComboListImp*>(m_pOwner->m_pListBox->GetImpl());
    int32_t iCount = pComboList->CountItems();
    if (iCount < 1) {
      return;
    }
    FX_BOOL bMatchEqual = FALSE;
    int32_t iCurSel = m_pOwner->m_iCurSel;
    if (m_pOwner->m_pEdit) {
      CFX_WideString wsText;
      m_pOwner->m_pEdit->GetText(wsText);
      iCurSel = pComboList->MatchItem(wsText);
      if (iCurSel >= 0) {
        CFX_WideString wsTemp;
        FWL_HLISTITEM item = m_pOwner->m_pListBox->GetSelItem(iCurSel);
        m_pOwner->m_pListBox->GetItemText(item, wsTemp);
        bMatchEqual = wsText.Equal(wsTemp);
      }
    }
    if (iCurSel < 0) {
      iCurSel = 0;
    } else if (bMatchEqual) {
      if ((bUp && iCurSel == 0) || (bDown && iCurSel == iCount - 1)) {
        return;
      }
      if (bUp) {
        iCurSel--;
      } else {
        iCurSel++;
      }
    }
    m_pOwner->m_iCurSel = iCurSel;
    m_pOwner->SynchrEditText(m_pOwner->m_iCurSel);
    return;
  }
  if (m_pOwner->m_pEdit) {
    IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pEdit->SetDelegate(NULL);
    pDelegate->OnProcessMessage(pMsg);
  }
}
CFWL_ComboProxyImpDelegate::CFWL_ComboProxyImpDelegate(
    IFWL_Form* pForm,
    CFWL_ComboBoxImp* pComboBox)
    : m_bLButtonDown(FALSE),
      m_bLButtonUpSelf(FALSE),
      m_fStartPos(0),
      m_pForm(pForm),
      m_pComboBox(pComboBox) {
}
int32_t CFWL_ComboProxyImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  if (dwMsgCode == FWL_MSGHASH_Mouse) {
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
      case FWL_MSGMOUSECMD_MouseMove: {
        OnMouseMove(pMsg);
        break;
      }
      default: {}
    }
  }
  if (dwMsgCode == FWL_MSGHASH_Deactivate) {
    OnDeactive(static_cast<CFWL_MsgDeactivate*>(pMessage));
  }
  if (dwMsgCode == FWL_MSGHASH_KillFocus || dwMsgCode == FWL_MSGHASH_SetFocus) {
    OnFocusChanged(static_cast<CFWL_MsgKillFocus*>(pMessage),
                   dwMsgCode == FWL_MSGHASH_SetFocus);
  }
  return CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
}
FWL_ERR CFWL_ComboProxyImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                                 const CFX_Matrix* pMatrix) {
  m_pComboBox->DrawStretchHandler(pGraphics, pMatrix);
  return FWL_ERR_Succeeded;
}
void CFWL_ComboProxyImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  IFWL_NoteThread* pThread = m_pForm->GetOwnerThread();
  if (!pThread)
    return;
  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pThread->GetNoteDriver());
  CFX_RectF rtWidget;
  m_pForm->GetWidgetRect(rtWidget);
  rtWidget.left = rtWidget.top = 0;
  if (rtWidget.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_bLButtonDown = TRUE;
    pDriver->SetGrab(m_pForm, TRUE);
  } else {
    m_bLButtonDown = FALSE;
    pDriver->SetGrab(m_pForm, FALSE);
    m_pComboBox->ShowDropList(FALSE);
    return;
  }
  IFWL_AdapterNative* pNative = FWL_GetAdapterNative();
  IFWL_AdapterCursorMgr* pCursorMgr = pNative->GetCursorMgr();
  FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(FWL_CURSORTYPE_SizeNS);
  pCursorMgr->SetCursor(hCursor);
  pCursorMgr->ShowCursor(TRUE);
  m_pForm->TransformTo(NULL, pMsg->m_fx, pMsg->m_fy);
  m_fStartPos = pMsg->m_fy;
}
void CFWL_ComboProxyImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  m_bLButtonDown = FALSE;
  IFWL_NoteThread* pThread = m_pForm->GetOwnerThread();
  if (!pThread)
    return;
  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pThread->GetNoteDriver());
  pDriver->SetGrab(m_pForm, FALSE);
  if (m_bLButtonUpSelf) {
    CFX_RectF rect;
    m_pForm->GetWidgetRect(rect);
    rect.left = rect.top = 0;
    if (!rect.Contains(pMsg->m_fx, pMsg->m_fy) &&
        m_pComboBox->IsDropListShowed()) {
      m_pComboBox->ShowDropList(FALSE);
    }
  } else {
    m_bLButtonUpSelf = TRUE;
  }
}
void CFWL_ComboProxyImpDelegate::OnMouseMove(CFWL_MsgMouse* pMsg) {
  IFWL_AdapterNative* pNative = FWL_GetAdapterNative();
  IFWL_AdapterCursorMgr* pCursorMgr = pNative->GetCursorMgr();
  FWL_CURSORTYPE cursorType = FWL_CURSORTYPE_Arrow;
  if (m_pComboBox->m_rtHandler.Contains(pMsg->m_fx, pMsg->m_fy)) {
    cursorType = FWL_CURSORTYPE_SizeNS;
  }
  FWL_HCURSOR hCursor = pCursorMgr->GetSystemCursor(cursorType);
  pCursorMgr->SetCursor(hCursor);
  pCursorMgr->ShowCursor(TRUE);
  if (!m_bLButtonDown) {
    return;
  }
  m_pForm->TransformTo(NULL, pMsg->m_fx, pMsg->m_fy);
  FX_FLOAT fChanged = pMsg->m_fy - m_fStartPos;
  if (m_pComboBox->m_bUpFormHandler) {
    fChanged = m_fStartPos - pMsg->m_fy;
  }
  if (m_pComboBox->m_rtList.height + fChanged < m_pComboBox->m_fItemHeight) {
    return;
  }
  m_pComboBox->m_rtList.height += fChanged;
  m_pComboBox->m_rtProxy.height += fChanged;
  if (m_pComboBox->m_bUpFormHandler) {
    m_pComboBox->m_rtProxy.top -= fChanged;
    m_pComboBox->m_rtHandler.Set(0, 0, m_pComboBox->m_rtList.width,
                                 m_pComboBox->m_fComboFormHandler);
  } else {
    m_pComboBox->m_rtHandler.Set(0, m_pComboBox->m_rtList.height,
                                 m_pComboBox->m_rtList.width,
                                 m_pComboBox->m_fComboFormHandler);
  }
  m_pForm->SetWidgetRect(m_pComboBox->m_rtProxy);
  m_pComboBox->m_pListBox->SetWidgetRect(m_pComboBox->m_rtList);
  m_pComboBox->m_pListBox->Update();
  m_fStartPos = pMsg->m_fy;
}
void CFWL_ComboProxyImpDelegate::OnDeactive(CFWL_MsgDeactivate* pMsg) {
  m_pComboBox->ShowDropList(FALSE);
}
void CFWL_ComboProxyImpDelegate::OnFocusChanged(CFWL_MsgKillFocus* pMsg,
                                                FX_BOOL bSet) {
  if (!bSet) {
    if (pMsg->m_pSetFocus == NULL) {
      m_pComboBox->ShowDropList(FALSE);
    }
  }
}
