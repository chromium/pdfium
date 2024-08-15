// Copyright 2014 The PDFium Authors
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

namespace pdfium {

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
          Properties{FWL_STYLE_WGT_Border | FWL_STYLE_WGT_VScroll, 0,
                     FWL_STATE_WGT_Invisible},
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

void CFWL_ComboBox::ModifyStyleExts(uint32_t dwStyleExtsAdded,
                                    uint32_t dwStyleExtsRemoved) {
  bool bAddDropDown = !!(dwStyleExtsAdded & FWL_STYLEEXT_CMB_DropDown);
  bool bDelDropDown = !!(dwStyleExtsRemoved & FWL_STYLEEXT_CMB_DropDown);
  dwStyleExtsRemoved &= ~FWL_STYLEEXT_CMB_DropDown;
  m_Properties.m_dwStyleExts |= FWL_STYLEEXT_CMB_DropDown;
  if (bAddDropDown)
    m_pEdit->ModifyStyleExts(0, FWL_STYLEEXT_EDT_ReadOnly);
  else if (bDelDropDown)
    m_pEdit->ModifyStyleExts(FWL_STYLEEXT_EDT_ReadOnly, 0);

  CFWL_Widget::ModifyStyleExts(dwStyleExtsAdded, dwStyleExtsRemoved);
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

void CFWL_ComboBox::DrawWidget(CFGAS_GEGraphics* pGraphics,
                               const CFX_Matrix& matrix) {
  if (!m_BtnRect.IsEmpty(0.1f)) {
    CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
    pGraphics->ConcatMatrix(matrix);
    CFWL_ThemeBackground param(CFWL_ThemePart::Part::kDropDownButton, this,
                               pGraphics);
    param.m_dwStates = m_iBtnState;
    param.m_PartRect = m_BtnRect;
    GetThemeProvider()->DrawBackground(param);
  }
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

CFX_RectF CFWL_ComboBox::GetBBox() const {
  CFX_RectF rect = m_WidgetRect;
  if (!m_pListBox || !IsDropListVisible())
    return rect;

  CFX_RectF rtList = m_pListBox->GetWidgetRect();
  rtList.Offset(rect.left, rect.top);
  rect.Union(rtList);
  return rect;
}

void CFWL_ComboBox::EditModifyStyleExts(uint32_t dwStyleExtsAdded,
                                        uint32_t dwStyleExtsRemoved) {
  if (m_pEdit)
    m_pEdit->ModifyStyleExts(dwStyleExtsAdded, dwStyleExtsRemoved);
}

void CFWL_ComboBox::ShowDropDownList() {
  if (IsDropListVisible())
    return;

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
  m_pListBox->RemoveStates(FWL_STATE_WGT_Invisible);

  CFWL_Event postEvent(CFWL_Event::Type::PostDropDown, this);
  DispatchEvent(&postEvent);
  RepaintInflatedListBoxRect();
}

void CFWL_ComboBox::HideDropDownList() {
  if (!IsDropListVisible())
    return;

  m_pListBox->SetStates(FWL_STATE_WGT_Invisible);
  RepaintInflatedListBoxRect();
}

void CFWL_ComboBox::RepaintInflatedListBoxRect() {
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
  if (!(GetStyleExts() & FWL_STYLEEXT_CMB_ReadOnly)) {
    m_BtnRect =
        CFX_RectF(m_ClientRect.right() - fBtn, m_ClientRect.top + borderWidth,
                  fBtn - borderWidth, m_ClientRect.height - 2 * borderWidth);
  }

  CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
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
  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CMB_EditHAlignMask) {
    case FWL_STYLEEXT_CMB_EditHCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_HCenter;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_EDT_HNear;
      break;
    }
  }
  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CMB_EditVAlignMask) {
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
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CMB_EditJustified)
    dwAdd |= FWL_STYLEEXT_EDT_Justified;

  m_pEdit->ModifyStyleExts(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                      FWL_STYLEEXT_EDT_HAlignModeMask |
                                      FWL_STYLEEXT_EDT_VAlignMask);
}

void CFWL_ComboBox::ResetListItemAlignment() {
  if (!m_pListBox)
    return;

  uint32_t dwAdd = 0;
  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_CMB_ListItemAlignMask) {
    case FWL_STYLEEXT_CMB_ListItemCenterAlign: {
      dwAdd |= FWL_STYLEEXT_LTB_CenterAlign;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_LTB_LeftAlign;
      break;
    }
  }
  m_pListBox->ModifyStyleExts(dwAdd, FWL_STYLEEXT_CMB_ListItemAlignMask);
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
  CFWL_EventSelectChanged ev(this, bLButtonUp);
  DispatchEvent(&ev);
}

void CFWL_ComboBox::OnProcessMessage(CFWL_Message* pMessage) {
  bool backDefault = true;
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus: {
      backDefault = false;
      OnFocusGained();
      break;
    }
    case CFWL_Message::Type::kKillFocus: {
      backDefault = false;
      OnFocusLost();
      break;
    }
    case CFWL_Message::Type::kMouse: {
      backDefault = false;
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
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
      if (IsDropListVisible() &&
          pKey->m_dwCmd == CFWL_MessageKey::KeyCommand::kKeyDown) {
        bool bListKey = pKey->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Up ||
                        pKey->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Down ||
                        pKey->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Return ||
                        pKey->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Escape;
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
    CFWL_EventScroll pScrollEv(this, pScrollEvent->GetScrollCode(),
                               pScrollEvent->GetPos());
    DispatchEvent(&pScrollEv);
  } else if (type == CFWL_Event::Type::TextWillChange) {
    CFWL_Event pTemp(CFWL_Event::Type::EditChanged, this);
    DispatchEvent(&pTemp);
  }
}

void CFWL_ComboBox::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                 const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_ComboBox::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (m_BtnRect.Contains(pMsg->m_pos))
    m_iBtnState = CFWL_PartState::kHovered;
  else
    m_iBtnState = CFWL_PartState::kNormal;

  RepaintRect(m_BtnRect);
}

void CFWL_ComboBox::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (IsDropListVisible()) {
    if (m_BtnRect.Contains(pMsg->m_pos))
      HideDropDownList();
    return;
  }
  if (!m_ClientRect.Contains(pMsg->m_pos))
    return;

  if (m_pEdit)
    MatchEditText();
  ShowDropDownList();
}

void CFWL_ComboBox::OnFocusGained() {
  m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
  if ((m_pEdit->GetStates() & FWL_STATE_WGT_Focused) == 0) {
    CFWL_MessageSetFocus msg(m_pEdit);
    m_pEdit->GetDelegate()->OnProcessMessage(&msg);
  }
}

void CFWL_ComboBox::OnFocusLost() {
  m_Properties.m_dwStates &= ~FWL_STATE_WGT_Focused;
  HideDropDownList();
  CFWL_MessageKillFocus msg(nullptr);
  m_pEdit->GetDelegate()->OnProcessMessage(&msg);
}

void CFWL_ComboBox::OnKey(CFWL_MessageKey* pMsg) {
  uint32_t dwKeyCode = pMsg->m_dwKeyCodeOrChar;
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

}  // namespace pdfium
