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
      edit_(cppgc::MakeGarbageCollected<CFWL_ComboEdit>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties(),
          this)),
      list_box_(cppgc::MakeGarbageCollected<CFWL_ComboList>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties{FWL_STYLE_WGT_Border | FWL_STYLE_WGT_VScroll, 0,
                     FWL_STATE_WGT_Invisible},
          this)) {}

CFWL_ComboBox::~CFWL_ComboBox() = default;

void CFWL_ComboBox::Trace(cppgc::Visitor* visitor) const {
  CFWL_Widget::Trace(visitor);
  visitor->Trace(edit_);
  visitor->Trace(list_box_);
}

FWL_Type CFWL_ComboBox::GetClassID() const {
  return FWL_Type::ComboBox;
}

void CFWL_ComboBox::AddString(const WideString& wsText) {
  list_box_->AddString(wsText);
}

void CFWL_ComboBox::RemoveAt(int32_t iIndex) {
  list_box_->RemoveAt(iIndex);
}

void CFWL_ComboBox::RemoveAll() {
  list_box_->DeleteAll();
}

void CFWL_ComboBox::ModifyStyleExts(uint32_t dwStyleExtsAdded,
                                    uint32_t dwStyleExtsRemoved) {
  bool bAddDropDown = !!(dwStyleExtsAdded & FWL_STYLEEXT_CMB_DropDown);
  bool bDelDropDown = !!(dwStyleExtsRemoved & FWL_STYLEEXT_CMB_DropDown);
  dwStyleExtsRemoved &= ~FWL_STYLEEXT_CMB_DropDown;
  properties_.style_exts_ |= FWL_STYLEEXT_CMB_DropDown;
  if (bAddDropDown) {
    edit_->ModifyStyleExts(0, FWL_STYLEEXT_EDT_ReadOnly);
  } else if (bDelDropDown) {
    edit_->ModifyStyleExts(FWL_STYLEEXT_EDT_ReadOnly, 0);
  }

  CFWL_Widget::ModifyStyleExts(dwStyleExtsAdded, dwStyleExtsRemoved);
}

void CFWL_ComboBox::Update() {
  if (IsLocked()) {
    return;
  }

  if (edit_) {
    ResetEditAlignment();
  }
  Layout();
}

FWL_WidgetHit CFWL_ComboBox::HitTest(const CFX_PointF& point) {
  CFX_RectF rect(0, 0, widget_rect_.width - btn_rect_.width,
                 widget_rect_.height);
  if (rect.Contains(point)) {
    return FWL_WidgetHit::Edit;
  }
  if (btn_rect_.Contains(point)) {
    return FWL_WidgetHit::Client;
  }
  if (IsDropListVisible()) {
    rect = list_box_->GetWidgetRect();
    if (rect.Contains(point)) {
      return FWL_WidgetHit::Client;
    }
  }
  return FWL_WidgetHit::Unknown;
}

void CFWL_ComboBox::DrawWidget(CFGAS_GEGraphics* pGraphics,
                               const CFX_Matrix& matrix) {
  if (!btn_rect_.IsEmpty(0.1f)) {
    CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
    pGraphics->ConcatMatrix(matrix);
    CFWL_ThemeBackground param(CFWL_ThemePart::Part::kDropDownButton, this,
                               pGraphics);
    param.states_ = btn_state_;
    param.part_rect_ = btn_rect_;
    GetThemeProvider()->DrawBackground(param);
  }
  if (edit_) {
    CFX_RectF rtEdit = edit_->GetWidgetRect();
    CFX_Matrix mt(1, 0, 0, 1, rtEdit.left, rtEdit.top);
    mt.Concat(matrix);
    edit_->DrawWidget(pGraphics, mt);
  }
  if (list_box_ && IsDropListVisible()) {
    CFX_RectF rtList = list_box_->GetWidgetRect();
    CFX_Matrix mt(1, 0, 0, 1, rtList.left, rtList.top);
    mt.Concat(matrix);
    list_box_->DrawWidget(pGraphics, mt);
  }
}

WideString CFWL_ComboBox::GetTextByIndex(int32_t iIndex) const {
  CFWL_ListBox::Item* pItem = list_box_->GetItem(list_box_, iIndex);
  return pItem ? pItem->GetText() : WideString();
}

void CFWL_ComboBox::SetCurSel(int32_t iSel) {
  int32_t iCount = list_box_->CountItems(nullptr);
  bool bClearSel = iSel < 0 || iSel >= iCount;
  if (IsDropDownStyle() && edit_) {
    if (bClearSel) {
      edit_->SetText(WideString());
    } else {
      CFWL_ListBox::Item* hItem = list_box_->GetItem(this, iSel);
      edit_->SetText(hItem ? hItem->GetText() : WideString());
    }
    edit_->Update();
  }
  cur_sel_ = bClearSel ? -1 : iSel;
}

void CFWL_ComboBox::SetStates(uint32_t dwStates) {
  if (IsDropDownStyle() && edit_) {
    edit_->SetStates(dwStates);
  }
  if (list_box_) {
    list_box_->SetStates(dwStates);
  }
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_ComboBox::RemoveStates(uint32_t dwStates) {
  if (IsDropDownStyle() && edit_) {
    edit_->RemoveStates(dwStates);
  }
  if (list_box_) {
    list_box_->RemoveStates(dwStates);
  }
  CFWL_Widget::RemoveStates(dwStates);
}

void CFWL_ComboBox::SetEditText(const WideString& wsText) {
  if (!edit_) {
    return;
  }

  edit_->SetText(wsText);
  edit_->Update();
}

WideString CFWL_ComboBox::GetEditText() const {
  if (edit_) {
    return edit_->GetText();
  }
  if (!list_box_) {
    return WideString();
  }

  CFWL_ListBox::Item* hItem = list_box_->GetItem(this, cur_sel_);
  return hItem ? hItem->GetText() : WideString();
}

CFX_RectF CFWL_ComboBox::GetBBox() const {
  CFX_RectF rect = widget_rect_;
  if (!list_box_ || !IsDropListVisible()) {
    return rect;
  }

  CFX_RectF rtList = list_box_->GetWidgetRect();
  rtList.Offset(rect.left, rect.top);
  rect.Union(rtList);
  return rect;
}

void CFWL_ComboBox::EditModifyStyleExts(uint32_t dwStyleExtsAdded,
                                        uint32_t dwStyleExtsRemoved) {
  if (edit_) {
    edit_->ModifyStyleExts(dwStyleExtsAdded, dwStyleExtsRemoved);
  }
}

void CFWL_ComboBox::ShowDropDownList() {
  if (IsDropListVisible()) {
    return;
  }

  CFWL_Event preEvent(CFWL_Event::Type::PreDropDown, this);
  DispatchEvent(&preEvent);
  if (!preEvent.GetSrcTarget()) {
    return;
  }

  CFWL_ComboList* pComboList = list_box_;
  int32_t iItems = pComboList->CountItems(nullptr);
  if (iItems < 1) {
    return;
  }

  ResetListItemAlignment();
  pComboList->ChangeSelected(cur_sel_);

  float fItemHeight = pComboList->CalcItemHeight();
  float fBorder = GetCXBorderSize();
  float fPopupMin = 0.0f;
  if (iItems > 3) {
    fPopupMin = fItemHeight * 3 + fBorder * 2;
  }

  float fPopupMax = fItemHeight * iItems + fBorder * 2;
  CFX_RectF rtList(client_rect_.left, 0, widget_rect_.width, 0);
  GetPopupPos(fPopupMin, fPopupMax, widget_rect_, &rtList);
  list_box_->SetWidgetRect(rtList);
  list_box_->Update();
  list_box_->RemoveStates(FWL_STATE_WGT_Invisible);

  CFWL_Event postEvent(CFWL_Event::Type::PostDropDown, this);
  DispatchEvent(&postEvent);
  RepaintInflatedListBoxRect();
}

void CFWL_ComboBox::HideDropDownList() {
  if (!IsDropListVisible()) {
    return;
  }

  list_box_->SetStates(FWL_STATE_WGT_Invisible);
  RepaintInflatedListBoxRect();
}

void CFWL_ComboBox::RepaintInflatedListBoxRect() {
  CFX_RectF rect = list_box_->GetWidgetRect();
  rect.Inflate(2, 2);
  RepaintRect(rect);
}

void CFWL_ComboBox::MatchEditText() {
  WideString wsText = edit_->GetText();
  int32_t iMatch = list_box_->MatchItem(wsText.AsStringView());
  if (iMatch != cur_sel_) {
    list_box_->ChangeSelected(iMatch);
    if (iMatch >= 0) {
      SyncEditText(iMatch);
    }
  } else if (iMatch >= 0) {
    edit_->SetSelected();
  }
  cur_sel_ = iMatch;
}

void CFWL_ComboBox::SyncEditText(int32_t iListItem) {
  CFWL_ListBox::Item* hItem = list_box_->GetItem(this, iListItem);
  edit_->SetText(hItem ? hItem->GetText() : WideString());
  edit_->Update();
  edit_->SetSelected();
}

void CFWL_ComboBox::Layout() {
  client_rect_ = GetClientRect();
  content_rect_ = client_rect_;

  IFWL_ThemeProvider* theme = GetThemeProvider();
  float borderWidth = 1;
  float fBtn = theme->GetScrollBarWidth();
  if (!(GetStyleExts() & FWL_STYLEEXT_CMB_ReadOnly)) {
    btn_rect_ =
        CFX_RectF(client_rect_.right() - fBtn, client_rect_.top + borderWidth,
                  fBtn - borderWidth, client_rect_.height - 2 * borderWidth);
  }

  CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
  CFX_RectF pUIMargin = theme->GetUIMargin(part);
  content_rect_.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                        pUIMargin.height);

  if (!IsDropDownStyle() || !edit_) {
    return;
  }

  CFX_RectF rtEdit(content_rect_.left, content_rect_.top,
                   content_rect_.width - fBtn, content_rect_.height);
  edit_->SetWidgetRect(rtEdit);

  if (cur_sel_ >= 0) {
    CFWL_ListBox::Item* hItem = list_box_->GetItem(this, cur_sel_);
    ScopedUpdateLock update_lock(edit_);
    edit_->SetText(hItem ? hItem->GetText() : WideString());
  }
  edit_->Update();
}

void CFWL_ComboBox::ResetEditAlignment() {
  if (!edit_) {
    return;
  }

  uint32_t dwAdd = 0;
  switch (properties_.style_exts_ & FWL_STYLEEXT_CMB_EditHAlignMask) {
    case FWL_STYLEEXT_CMB_EditHCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_HCenter;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_EDT_HNear;
      break;
    }
  }
  switch (properties_.style_exts_ & FWL_STYLEEXT_CMB_EditVAlignMask) {
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
  if (properties_.style_exts_ & FWL_STYLEEXT_CMB_EditJustified) {
    dwAdd |= FWL_STYLEEXT_EDT_Justified;
  }

  edit_->ModifyStyleExts(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                    FWL_STYLEEXT_EDT_HAlignModeMask |
                                    FWL_STYLEEXT_EDT_VAlignMask);
}

void CFWL_ComboBox::ResetListItemAlignment() {
  if (!list_box_) {
    return;
  }

  uint32_t dwAdd = 0;
  switch (properties_.style_exts_ & FWL_STYLEEXT_CMB_ListItemAlignMask) {
    case FWL_STYLEEXT_CMB_ListItemCenterAlign: {
      dwAdd |= FWL_STYLEEXT_LTB_CenterAlign;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_LTB_LeftAlign;
      break;
    }
  }
  list_box_->ModifyStyleExts(dwAdd, FWL_STYLEEXT_CMB_ListItemAlignMask);
}

void CFWL_ComboBox::ProcessSelChanged(bool bLButtonUp) {
  cur_sel_ = list_box_->GetItemIndex(this, list_box_->GetSelItem(0));
  if (!IsDropDownStyle()) {
    RepaintRect(client_rect_);
    return;
  }
  CFWL_ListBox::Item* hItem = list_box_->GetItem(this, cur_sel_);
  if (!hItem) {
    return;
  }

  if (edit_) {
    edit_->SetText(hItem->GetText());
    edit_->Update();
    edit_->SetSelected();
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
      switch (pMsg->cmd_) {
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
          pKey->cmd_ == CFWL_MessageKey::KeyCommand::kKeyDown) {
        bool bListKey = pKey->key_code_or_char_ == XFA_FWL_VKEY_Up ||
                        pKey->key_code_or_char_ == XFA_FWL_VKEY_Down ||
                        pKey->key_code_or_char_ == XFA_FWL_VKEY_Return ||
                        pKey->key_code_or_char_ == XFA_FWL_VKEY_Escape;
        if (bListKey) {
          list_box_->GetDelegate()->OnProcessMessage(pMessage);
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
  if (backDefault && pMessage->GetDstTarget()) {
    CFWL_Widget::OnProcessMessage(pMessage);
  }
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
  if (btn_rect_.Contains(pMsg->pos_)) {
    btn_state_ = CFWL_PartState::kHovered;
  } else {
    btn_state_ = CFWL_PartState::kNormal;
  }

  RepaintRect(btn_rect_);
}

void CFWL_ComboBox::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (IsDropListVisible()) {
    if (btn_rect_.Contains(pMsg->pos_)) {
      HideDropDownList();
    }
    return;
  }
  if (!client_rect_.Contains(pMsg->pos_)) {
    return;
  }

  if (edit_) {
    MatchEditText();
  }
  ShowDropDownList();
}

void CFWL_ComboBox::OnFocusGained() {
  properties_.states_ |= FWL_STATE_WGT_Focused;
  if ((edit_->GetStates() & FWL_STATE_WGT_Focused) == 0) {
    CFWL_MessageSetFocus msg(edit_);
    edit_->GetDelegate()->OnProcessMessage(&msg);
  }
}

void CFWL_ComboBox::OnFocusLost() {
  properties_.states_ &= ~FWL_STATE_WGT_Focused;
  HideDropDownList();
  CFWL_MessageKillFocus msg(nullptr);
  edit_->GetDelegate()->OnProcessMessage(&msg);
}

void CFWL_ComboBox::OnKey(CFWL_MessageKey* pMsg) {
  uint32_t dwKeyCode = pMsg->key_code_or_char_;
  const bool bUp = dwKeyCode == XFA_FWL_VKEY_Up;
  const bool bDown = dwKeyCode == XFA_FWL_VKEY_Down;
  if (bUp || bDown) {
    CFWL_ComboList* pComboList = list_box_;
    int32_t iCount = pComboList->CountItems(nullptr);
    if (iCount < 1) {
      return;
    }

    bool bMatchEqual = false;
    int32_t iCurSel = cur_sel_;
    if (edit_) {
      WideString wsText = edit_->GetText();
      iCurSel = pComboList->MatchItem(wsText.AsStringView());
      if (iCurSel >= 0) {
        CFWL_ListBox::Item* item = list_box_->GetSelItem(iCurSel);
        bMatchEqual = wsText == (item ? item->GetText() : WideString());
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
    cur_sel_ = iCurSel;
    SyncEditText(cur_sel_);
    return;
  }
  if (edit_) {
    edit_->GetDelegate()->OnProcessMessage(pMsg);
  }
}

void CFWL_ComboBox::GetPopupPos(float fMinHeight,
                                float fMaxHeight,
                                const CFX_RectF& rtAnchor,
                                CFX_RectF* pPopupRect) {
  GetWidgetMgr()->GetAdapterPopupPos(this, fMinHeight, fMaxHeight, rtAnchor,
                                     pPopupRect);
}

}  // namespace pdfium
