// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_combobox.h"

#include "v8/include/cppgc/visitor.h"
#include "xfa/fde/cfde_texteditengine.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_eventselectchanged.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fwl/ifwl_themeprovider.h"

CFWL_ComboBox::CFWL_ComboBox(CFWL_App* app)
    : CFWL_Widget(app, Properties(), nullptr),
      m_pEdit(cppgc::MakeGarbageCollected<CFWL_ComboEdit>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties(),
          this)),
      m_pListBox(cppgc::MakeGarbageCollected<CFWL_ComboList>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties{FWL_WGTSTYLE_Border | FWL_WGTSTYLE_VScroll, 0,
                     FWL_WGTSTATE_Invisible},
          this)) {}

CFWL_ComboBox::~CFWL_ComboBox() = default;

void CFWL_ComboBox::Trace(cppgc::Visitor* visitor) const {
  CFWL_Widget::Trace(visitor);
  visitor->Trace(m_pEdit);
  visitor->Trace(m_pListBox);
}

FWL_Type CFWL_ComboBox::GetClassID() const {
  return FWL_Type::ComboBox;
}

void CFWL_ComboBox::AddString(const WideString& wsText) {
  m_pListBox->AddString(wsText);
}

void CFWL_ComboBox::RemoveAt(int32_t iIndex) {
  m_pListBox->RemoveAt(iIndex);
}

void CFWL_ComboBox::RemoveAll() {
  m_pListBox->DeleteAll();
}

void CFWL_ComboBox::ModifyStylesEx(uint32_t dwStylesExAdded,
                                   uint32_t dwStylesExRemoved) {
  bool bAddDropDown = !!(dwStylesExAdded & FWL_STYLEEXT_CMB_DropDown);
  bool bDelDropDown = !!(dwStylesExRemoved & FWL_STYLEEXT_CMB_DropDown);
  dwStylesExRemoved &= ~FWL_STYLEEXT_CMB_DropDown;
  m_Properties.m_dwStyleExes |= FWL_STYLEEXT_CMB_DropDown;
  if (bAddDropDown)
    m_pEdit->ModifyStylesEx(0, FWL_STYLEEXT_EDT_ReadOnly);
  else if (bDelDropDown)
    m_pEdit->ModifyStylesEx(FWL_STYLEEXT_EDT_ReadOnly, 0);

  CFWL_Widget::ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

void CFWL_ComboBox::Update() {
  if (IsLocked())
    return;

  if (m_pEdit)
    ResetEditAlignment();
  Layout();
}

FWL_WidgetHit CFWL_ComboBox::HitTest(const CFX_PointF& point) {
  CFX_RectF rect(0, 0, m_WidgetRect.width - m_BtnRect.width,
                 m_WidgetRect.height);
  if (rect.Contains(point))
    return FWL_WidgetHit::Edit;
  if (m_BtnRect.Contains(point))
    return FWL_WidgetHit::Client;
  if (IsDropListVisible()) {
    rect = m_pListBox->GetWidgetRect();
    if (rect.Contains(point))
      return FWL_WidgetHit::Client;
  }
  return FWL_WidgetHit::Unknown;
}

void CFWL_ComboBox::DrawWidget(CXFA_Graphics* pGraphics,
                               const CFX_Matrix& matrix) {
  pGraphics->SaveGraphState();
  pGraphics->ConcatMatrix(&matrix);
  if (!m_BtnRect.IsEmpty(0.1f)) {
    CFWL_ThemeBackground param;
    param.m_pWidget = this;
    param.m_iPart = CFWL_Part::DropDownButton;
    param.m_dwStates = m_iBtnState;
    param.m_pGraphics = pGraphics;
    param.m_PartRect = m_BtnRect;
    GetThemeProvider()->DrawBackground(param);
  }
  pGraphics->RestoreGraphState();

  if (m_pEdit) {
    CFX_RectF rtEdit = m_pEdit->GetWidgetRect();
    CFX_Matrix mt(1, 0, 0, 1, rtEdit.left, rtEdit.top);
    mt.Concat(matrix);
    m_pEdit->DrawWidget(pGraphics, mt);
  }
  if (m_pListBox && IsDropListVisible()) {
    CFX_RectF rtList = m_pListBox->GetWidgetRect();
    CFX_Matrix mt(1, 0, 0, 1, rtList.left, rtList.top);
    mt.Concat(matrix);
    m_pListBox->DrawWidget(pGraphics, mt);
  }
}

WideString CFWL_ComboBox::GetTextByIndex(int32_t iIndex) const {
  CFWL_ListBox::Item* pItem = m_pListBox->GetItem(m_pListBox, iIndex);
  return pItem ? pItem->GetText() : WideString();
}

void CFWL_ComboBox::SetCurSel(int32_t iSel) {
  int32_t iCount = m_pListBox->CountItems(nullptr);
  bool bClearSel = iSel < 0 || iSel >= iCount;
  if (IsDropDownStyle() && m_pEdit) {
    if (bClearSel) {
      m_pEdit->SetText(WideString());
    } else {
      CFWL_ListBox::Item* hItem = m_pListBox->GetItem(this, iSel);
      m_pEdit->SetText(hItem ? hItem->GetText() : WideString());
    }
    m_pEdit->Update();
  }
  m_iCurSel = bClearSel ? -1 : iSel;
}

void CFWL_ComboBox::SetStates(uint32_t dwStates) {
  if (IsDropDownStyle() && m_pEdit)
    m_pEdit->SetStates(dwStates);
  if (m_pListBox)
    m_pListBox->SetStates(dwStates);
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_ComboBox::RemoveStates(uint32_t dwStates) {
  if (IsDropDownStyle() && m_pEdit)
    m_pEdit->RemoveStates(dwStates);
  if (m_pListBox)
    m_pListBox->RemoveStates(dwStates);
  CFWL_Widget::RemoveStates(dwStates);
}

void CFWL_ComboBox::SetEditText(const WideString& wsText) {
  if (!m_pEdit)
    return;

  m_pEdit->SetText(wsText);
  m_pEdit->Update();
}

WideString CFWL_ComboBox::GetEditText() const {
  if (m_pEdit)
    return m_pEdit->GetText();
  if (!m_pListBox)
    return WideString();

  CFWL_ListBox::Item* hItem = m_pListBox->GetItem(this, m_iCurSel);
  return hItem ? hItem->GetText() : WideString();
}

void CFWL_ComboBox::OpenDropDownList(bool bActivate) {
  ShowDropList(bActivate);
}

CFX_RectF CFWL_ComboBox::GetBBox() const {
  CFX_RectF rect = m_WidgetRect;
  if (!m_pListBox || !IsDropListVisible())
    return rect;

  CFX_RectF rtList = m_pListBox->GetWidgetRect();
  rtList.Offset(rect.left, rect.top);
  rect.Union(rtList);
  return rect;
}

void CFWL_ComboBox::EditModifyStylesEx(uint32_t dwStylesExAdded,
                                       uint32_t dwStylesExRemoved) {
  if (m_pEdit)
    m_pEdit->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

void CFWL_ComboBox::ShowDropList(bool bActivate) {
  if (IsDropListVisible() == bActivate)
    return;

  if (bActivate) {
    CFWL_Event preEvent(CFWL_Event::Type::PreDropDown, this);
    DispatchEvent(&preEvent);
    if (!preEvent.GetSrcTarget())
      return;

    CFWL_ComboList* pComboList = m_pListBox;
    int32_t iItems = pComboList->CountItems(nullptr);
    if (iItems < 1)
      return;

    ResetListItemAlignment();
    pComboList->ChangeSelected(m_iCurSel);

    float fItemHeight = pComboList->CalcItemHeight();
    float fBorder = GetCXBorderSize();
    float fPopupMin = 0.0f;
    if (iItems > 3)
      fPopupMin = fItemHeight * 3 + fBorder * 2;

    float fPopupMax = fItemHeight * iItems + fBorder * 2;
    CFX_RectF rtList(m_ClientRect.left, 0, m_WidgetRect.width, 0);
    GetPopupPos(fPopupMin, fPopupMax, m_WidgetRect, &rtList);
    m_pListBox->SetWidgetRect(rtList);
    m_pListBox->Update();
  }

  if (bActivate) {
    m_pListBox->RemoveStates(FWL_WGTSTATE_Invisible);
    CFWL_Event postEvent(CFWL_Event::Type::PostDropDown, this);
    DispatchEvent(&postEvent);
  } else {
    m_pListBox->SetStates(FWL_WGTSTATE_Invisible);
  }

  CFX_RectF rect = m_pListBox->GetWidgetRect();
  rect.Inflate(2, 2);
  RepaintRect(rect);
}

void CFWL_ComboBox::MatchEditText() {
  WideString wsText = m_pEdit->GetText();
  int32_t iMatch = m_pListBox->MatchItem(wsText.AsStringView());
  if (iMatch != m_iCurSel) {
    m_pListBox->ChangeSelected(iMatch);
    if (iMatch >= 0)
      SyncEditText(iMatch);
  } else if (iMatch >= 0) {
    m_pEdit->SetSelected();
  }
  m_iCurSel = iMatch;
}

void CFWL_ComboBox::SyncEditText(int32_t iListItem) {
  CFWL_ListBox::Item* hItem = m_pListBox->GetItem(this, iListItem);
  m_pEdit->SetText(hItem ? hItem->GetText() : WideString());
  m_pEdit->Update();
  m_pEdit->SetSelected();
}

void CFWL_ComboBox::Layout() {
  m_ClientRect = GetClientRect();
  m_ContentRect = m_ClientRect;

  IFWL_ThemeProvider* theme = GetThemeProvider();
  float borderWidth = 1;
  float fBtn = theme->GetScrollBarWidth();
  if (!(GetStylesEx() & FWL_STYLEEXT_CMB_ReadOnly)) {
    m_BtnRect =
        CFX_RectF(m_ClientRect.right() - fBtn, m_ClientRect.top + borderWidth,
                  fBtn - borderWidth, m_ClientRect.height - 2 * borderWidth);
  }

  CFWL_ThemePart part;
  part.m_pWidget = this;
  CFX_RectF pUIMargin = theme->GetUIMargin(part);
  m_ContentRect.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                        pUIMargin.height);

  if (!IsDropDownStyle() || !m_pEdit)
    return;

  CFX_RectF rtEdit(m_ContentRect.left, m_ContentRect.top,
                   m_ContentRect.width - fBtn, m_ContentRect.height);
  m_pEdit->SetWidgetRect(rtEdit);

  if (m_iCurSel >= 0) {
    CFWL_ListBox::Item* hItem = m_pListBox->GetItem(this, m_iCurSel);
    ScopedUpdateLock update_lock(m_pEdit);
    m_pEdit->SetText(hItem ? hItem->GetText() : WideString());
  }
  m_pEdit->Update();
}

void CFWL_ComboBox::ResetEditAlignment() {
  if (!m_pEdit)
    return;

  uint32_t dwAdd = 0;
  switch (m_Properties.m_dwStyleExes & FWL_STYLEEXT_CMB_EditHAlignMask) {
    case FWL_STYLEEXT_CMB_EditHCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_HCenter;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_EDT_HNear;
      break;
    }
  }
  switch (m_Properties.m_dwStyleExes & FWL_STYLEEXT_CMB_EditVAlignMask) {
    case FWL_STYLEEXT_CMB_EditVCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_VCenter;
      break;
    }
    case FWL_STYLEEXT_CMB_EditVFar: {
      dwAdd |= FWL_STYLEEXT_EDT_VFar;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_EDT_VNear;
      break;
    }
  }
  if (m_Properties.m_dwStyleExes & FWL_STYLEEXT_CMB_EditJustified)
    dwAdd |= FWL_STYLEEXT_EDT_Justified;

  m_pEdit->ModifyStylesEx(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                     FWL_STYLEEXT_EDT_HAlignModeMask |
                                     FWL_STYLEEXT_EDT_VAlignMask);
}

void CFWL_ComboBox::ResetListItemAlignment() {
  if (!m_pListBox)
    return;

  uint32_t dwAdd = 0;
  switch (m_Properties.m_dwStyleExes & FWL_STYLEEXT_CMB_ListItemAlignMask) {
    case FWL_STYLEEXT_CMB_ListItemCenterAlign: {
      dwAdd |= FWL_STYLEEXT_LTB_CenterAlign;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_LTB_LeftAlign;
      break;
    }
  }
  m_pListBox->ModifyStylesEx(dwAdd, FWL_STYLEEXT_CMB_ListItemAlignMask);
}

void CFWL_ComboBox::ProcessSelChanged(bool bLButtonUp) {
  m_iCurSel = m_pListBox->GetItemIndex(this, m_pListBox->GetSelItem(0));
  if (!IsDropDownStyle()) {
    RepaintRect(m_ClientRect);
    return;
  }

  CFWL_ListBox::Item* hItem = m_pListBox->GetItem(this, m_iCurSel);
  if (!hItem)
    return;
  if (m_pEdit) {
    m_pEdit->SetText(hItem->GetText());
    m_pEdit->Update();
    m_pEdit->SetSelected();
  }

  CFWL_EventSelectChanged ev(this);
  ev.bLButtonUp = bLButtonUp;
  DispatchEvent(&ev);
}

void CFWL_ComboBox::OnProcessMessage(CFWL_Message* pMessage) {
  bool backDefault = true;
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus: {
      backDefault = false;
      OnFocusChanged(pMessage, true);
      break;
    }
    case CFWL_Message::Type::kKillFocus: {
      backDefault = false;
      OnFocusChanged(pMessage, false);
      break;
    }
    case CFWL_Message::Type::kMouse: {
      backDefault = false;
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::kKey: {
      backDefault = false;
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->m_dwCmd == CFWL_MessageKey::Type::kKeyUp)
        break;
      if (IsDropListVisible() &&
          pKey->m_dwCmd == CFWL_MessageKey::Type::kKeyDown) {
        bool bListKey = pKey->m_dwKeyCode == XFA_FWL_VKEY_Up ||
                        pKey->m_dwKeyCode == XFA_FWL_VKEY_Down ||
                        pKey->m_dwKeyCode == XFA_FWL_VKEY_Return ||
                        pKey->m_dwKeyCode == XFA_FWL_VKEY_Escape;
        if (bListKey) {
          m_pListBox->GetDelegate()->OnProcessMessage(pMessage);
          break;
        }
      }
      OnKey(pKey);
      break;
    }
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (backDefault && pMessage->GetDstTarget())
    CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_ComboBox::OnProcessEvent(CFWL_Event* pEvent) {
  CFWL_Event::Type type = pEvent->GetType();
  if (type == CFWL_Event::Type::Scroll) {
    CFWL_EventScroll* pScrollEvent = static_cast<CFWL_EventScroll*>(pEvent);
    CFWL_EventScroll pScrollEv(this);
    pScrollEv.m_iScrollCode = pScrollEvent->m_iScrollCode;
    pScrollEv.m_fPos = pScrollEvent->m_fPos;
    DispatchEvent(&pScrollEv);
  } else if (type == CFWL_Event::Type::TextWillChange) {
    CFWL_Event pTemp(CFWL_Event::Type::EditChanged, this);
    DispatchEvent(&pTemp);
  }
}

void CFWL_ComboBox::OnDrawWidget(CXFA_Graphics* pGraphics,
                                 const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_ComboBox::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (m_BtnRect.Contains(pMsg->m_pos))
    m_iBtnState = CFWL_PartState_Hovered;
  else
    m_iBtnState = CFWL_PartState_Normal;

  RepaintRect(m_BtnRect);
}

void CFWL_ComboBox::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  bool bDropDown = IsDropListVisible();
  CFX_RectF& rtBtn = bDropDown ? m_BtnRect : m_ClientRect;
  if (!rtBtn.Contains(pMsg->m_pos))
    return;

  if (IsDropListVisible()) {
    ShowDropList(false);
    return;
  }
  if (m_pEdit)
    MatchEditText();
  ShowDropList(true);
}

void CFWL_ComboBox::OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (bSet) {
    m_Properties.m_dwStates |= FWL_WGTSTATE_Focused;
    if ((m_pEdit->GetStates() & FWL_WGTSTATE_Focused) == 0) {
      CFWL_MessageSetFocus msg(nullptr, m_pEdit);
      m_pEdit->GetDelegate()->OnProcessMessage(&msg);
    }
  } else {
    m_Properties.m_dwStates &= ~FWL_WGTSTATE_Focused;
    ShowDropList(false);
    CFWL_MessageKillFocus msg(m_pEdit);
    m_pEdit->GetDelegate()->OnProcessMessage(&msg);
  }
}

void CFWL_ComboBox::OnKey(CFWL_MessageKey* pMsg) {
  uint32_t dwKeyCode = pMsg->m_dwKeyCode;
  const bool bUp = dwKeyCode == XFA_FWL_VKEY_Up;
  const bool bDown = dwKeyCode == XFA_FWL_VKEY_Down;
  if (bUp || bDown) {
    CFWL_ComboList* pComboList = m_pListBox;
    int32_t iCount = pComboList->CountItems(nullptr);
    if (iCount < 1)
      return;

    bool bMatchEqual = false;
    int32_t iCurSel = m_iCurSel;
    if (m_pEdit) {
      WideString wsText = m_pEdit->GetText();
      iCurSel = pComboList->MatchItem(wsText.AsStringView());
      if (iCurSel >= 0) {
        CFWL_ListBox::Item* item = m_pListBox->GetSelItem(iCurSel);
        bMatchEqual = wsText == (item ? item->GetText() : WideString());
      }
    }
    if (iCurSel < 0) {
      iCurSel = 0;
    } else if (bMatchEqual) {
      if ((bUp && iCurSel == 0) || (bDown && iCurSel == iCount - 1))
        return;
      if (bUp)
        iCurSel--;
      else
        iCurSel++;
    }
    m_iCurSel = iCurSel;
    SyncEditText(m_iCurSel);
    return;
  }
  if (m_pEdit)
    m_pEdit->GetDelegate()->OnProcessMessage(pMsg);
}

void CFWL_ComboBox::GetPopupPos(float fMinHeight,
                                float fMaxHeight,
                                const CFX_RectF& rtAnchor,
                                CFX_RectF* pPopupRect) {
  GetWidgetMgr()->GetAdapterPopupPos(this, fMinHeight, fMaxHeight, rtAnchor,
                                     pPopupRect);
}
