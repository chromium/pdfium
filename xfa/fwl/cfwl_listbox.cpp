// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_listbox.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcrt/containers/unique_ptr_adapters.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagemousewheel.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

namespace {

const int kItemTextMargin = 2;

}  // namespace

CFWL_ListBox::CFWL_ListBox(CFWL_App* app,
                           const Properties& properties,
                           CFWL_Widget* pOuter)
    : CFWL_Widget(app, properties, pOuter) {}

CFWL_ListBox::~CFWL_ListBox() = default;

void CFWL_ListBox::Trace(cppgc::Visitor* visitor) const {
  CFWL_Widget::Trace(visitor);
  visitor->Trace(horz_scroll_bar_);
  visitor->Trace(vert_scroll_bar_);
}

FWL_Type CFWL_ListBox::GetClassID() const {
  return FWL_Type::ListBox;
}

void CFWL_ListBox::Update() {
  if (IsLocked()) {
    return;
  }

  switch (properties_.style_exts_ & FWL_STYLEEXT_LTB_AlignMask) {
    case FWL_STYLEEXT_LTB_LeftAlign:
      ttoaligns_ = FDE_TextAlignment::kCenterLeft;
      break;
    case FWL_STYLEEXT_LTB_RightAlign:
      ttoaligns_ = FDE_TextAlignment::kCenterRight;
      break;
    case FWL_STYLEEXT_LTB_CenterAlign:
    default:
      ttoaligns_ = FDE_TextAlignment::kCenter;
      break;
  }
  tto_styles_.single_line_ = true;
  scorll_bar_width_ = GetScrollWidth();
  CalcSize();
}

FWL_WidgetHit CFWL_ListBox::HitTest(const CFX_PointF& point) {
  if (IsShowHorzScrollBar()) {
    CFX_RectF rect = horz_scroll_bar_->GetWidgetRect();
    if (rect.Contains(point)) {
      return FWL_WidgetHit::HScrollBar;
    }
  }
  if (IsShowVertScrollBar()) {
    CFX_RectF rect = vert_scroll_bar_->GetWidgetRect();
    if (rect.Contains(point)) {
      return FWL_WidgetHit::VScrollBar;
    }
  }
  if (client_rect_.Contains(point)) {
    return FWL_WidgetHit::Client;
  }
  return FWL_WidgetHit::Unknown;
}

void CFWL_ListBox::DrawWidget(CFGAS_GEGraphics* pGraphics,
                              const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);
  }

  CFX_RectF rtClip(content_rect_);
  if (IsShowHorzScrollBar()) {
    rtClip.height -= scorll_bar_width_;
  }
  if (IsShowVertScrollBar()) {
    rtClip.width -= scorll_bar_width_;
  }

  pGraphics->SetClipRect(matrix.TransformRect(rtClip));
  if ((properties_.styles_ & FWL_STYLE_WGT_NoBackground) == 0) {
    DrawBkground(pGraphics, matrix);
  }

  DrawItems(pGraphics, matrix);
}

int32_t CFWL_ListBox::CountSelItems() {
  int32_t iRet = 0;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (pItem && pItem->IsSelected()) {
      iRet++;
    }
  }
  return iRet;
}

CFWL_ListBox::Item* CFWL_ListBox::GetSelItem(int32_t nIndexSel) {
  int32_t idx = GetSelIndex(nIndexSel);
  if (idx < 0) {
    return nullptr;
  }
  return GetItem(this, idx);
}

int32_t CFWL_ListBox::GetSelIndex(int32_t nIndex) {
  int32_t index = 0;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem) {
      return -1;
    }
    if (pItem->IsSelected()) {
      if (index == nIndex) {
        return i;
      }
      index++;
    }
  }
  return -1;
}

void CFWL_ListBox::SetSelItem(Item* pItem, bool bSelect) {
  if (!pItem) {
    if (bSelect) {
      SelectAll();
    } else {
      ClearSelection();
      SetFocusItem(nullptr);
    }
    return;
  }
  if (IsMultiSelection()) {
    pItem->SetSelected(bSelect);
  } else {
    SetSelection(pItem, pItem, bSelect);
  }
}

CFWL_ListBox::Item* CFWL_ListBox::GetListItem(Item* pItem,
                                              XFA_FWL_VKEYCODE dwKeyCode) {
  Item* hRet = nullptr;
  switch (dwKeyCode) {
    case XFA_FWL_VKEY_Up:
    case XFA_FWL_VKEY_Down:
    case XFA_FWL_VKEY_Home:
    case XFA_FWL_VKEY_End: {
      const bool bUp = dwKeyCode == XFA_FWL_VKEY_Up;
      const bool bDown = dwKeyCode == XFA_FWL_VKEY_Down;
      const bool bHome = dwKeyCode == XFA_FWL_VKEY_Home;
      int32_t iDstItem = -1;
      if (bUp || bDown) {
        int32_t index = GetItemIndex(this, pItem);
        iDstItem = dwKeyCode == XFA_FWL_VKEY_Up ? index - 1 : index + 1;
      } else if (bHome) {
        iDstItem = 0;
      } else {
        int32_t iCount = CountItems(this);
        iDstItem = iCount - 1;
      }
      hRet = GetItem(this, iDstItem);
      break;
    }
    default:
      break;
  }
  return hRet;
}

void CFWL_ListBox::SetSelection(Item* hStart, Item* hEnd, bool bSelected) {
  int32_t iStart = GetItemIndex(this, hStart);
  int32_t iEnd = GetItemIndex(this, hEnd);
  if (iStart > iEnd) {
    std::swap(iStart, iEnd);
  }
  if (bSelected) {
    int32_t iCount = CountItems(this);
    for (int32_t i = 0; i < iCount; i++) {
      Item* pItem = GetItem(this, i);
      if (pItem) {
        pItem->SetSelected(false);
      }
    }
  }
  while (iStart <= iEnd) {
    Item* pItem = GetItem(this, iStart);
    if (pItem) {
      pItem->SetSelected(bSelected);
    }
    ++iStart;
  }
}

bool CFWL_ListBox::IsMultiSelection() const {
  return properties_.style_exts_ & FWL_STYLEEXT_LTB_MultiSelection;
}

void CFWL_ListBox::ClearSelection() {
  bool bMulti = IsMultiSelection();
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem) {
      continue;
    }
    if (!pItem->IsSelected()) {
      continue;
    }
    pItem->SetSelected(false);
    if (!bMulti) {
      return;
    }
  }
}

void CFWL_ListBox::SelectAll() {
  if (!IsMultiSelection()) {
    return;
  }

  int32_t iCount = CountItems(this);
  if (iCount <= 0) {
    return;
  }

  Item* pItemStart = GetItem(this, 0);
  Item* pItemEnd = GetItem(this, iCount - 1);
  SetSelection(pItemStart, pItemEnd, false);
}

CFWL_ListBox::Item* CFWL_ListBox::GetFocusedItem() {
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem) {
      break;
    }
    if (pItem->IsFocused()) {
      return pItem;
    }
  }
  return nullptr;
}

void CFWL_ListBox::SetFocusItem(Item* pItem) {
  Item* hFocus = GetFocusedItem();
  if (pItem == hFocus) {
    return;
  }

  if (hFocus) {
    hFocus->SetFocused(false);
  }
  if (pItem) {
    pItem->SetFocused(true);
  }
}

CFWL_ListBox::Item* CFWL_ListBox::GetItemAtPoint(const CFX_PointF& point) {
  CFX_PointF pos = point - content_rect_.TopLeft();
  float fPosX = 0.0f;
  if (horz_scroll_bar_) {
    fPosX = horz_scroll_bar_->GetPos();
  }

  float fPosY = 0.0;
  if (vert_scroll_bar_) {
    fPosY = vert_scroll_bar_->GetPos();
  }

  int32_t nCount = CountItems(this);
  for (int32_t i = 0; i < nCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem) {
      continue;
    }

    CFX_RectF rtItem = pItem->GetRect();
    rtItem.Offset(-fPosX, -fPosY);
    if (rtItem.Contains(pos)) {
      return pItem;
    }
  }
  return nullptr;
}

bool CFWL_ListBox::ScrollToVisible(Item* pItem) {
  if (!vert_scroll_bar_) {
    return false;
  }

  CFX_RectF rtItem = pItem ? pItem->GetRect() : CFX_RectF();
  bool bScroll = false;
  float fPosY = vert_scroll_bar_->GetPos();
  rtItem.Offset(0, -fPosY + content_rect_.top);
  if (rtItem.top < content_rect_.top) {
    fPosY += rtItem.top - content_rect_.top;
    bScroll = true;
  } else if (rtItem.bottom() > content_rect_.bottom()) {
    fPosY += rtItem.bottom() - content_rect_.bottom();
    bScroll = true;
  }
  if (!bScroll) {
    return false;
  }

  vert_scroll_bar_->SetPos(fPosY);
  vert_scroll_bar_->SetTrackPos(fPosY);
  RepaintRect(client_rect_);
  return true;
}

void CFWL_ListBox::DrawBkground(CFGAS_GEGraphics* pGraphics,
                                const CFX_Matrix& mtMatrix) {
  if (!pGraphics) {
    return;
  }

  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackground, this,
                             pGraphics);
  param.matrix_ = mtMatrix;
  param.part_rect_ = client_rect_;
  if (IsShowHorzScrollBar() && IsShowVertScrollBar()) {
    param.data_rect_ = &static_rect_;
  }
  if (!IsEnabled()) {
    param.states_ = CFWL_PartState::kDisabled;
  }
  GetThemeProvider()->DrawBackground(param);
}

void CFWL_ListBox::DrawItems(CFGAS_GEGraphics* pGraphics,
                             const CFX_Matrix& mtMatrix) {
  float fPosX = 0.0f;
  if (horz_scroll_bar_) {
    fPosX = horz_scroll_bar_->GetPos();
  }

  float fPosY = 0.0f;
  if (vert_scroll_bar_) {
    fPosY = vert_scroll_bar_->GetPos();
  }

  CFX_RectF rtView(content_rect_);
  if (horz_scroll_bar_) {
    rtView.height -= scorll_bar_width_;
  }
  if (vert_scroll_bar_) {
    rtView.width -= scorll_bar_width_;
  }

  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListBox::Item* pItem = GetItem(this, i);
    if (!pItem) {
      continue;
    }

    CFX_RectF rtItem = pItem->GetRect();
    rtItem.Offset(content_rect_.left - fPosX, content_rect_.top - fPosY);
    if (rtItem.bottom() < content_rect_.top) {
      continue;
    }
    if (rtItem.top >= content_rect_.bottom()) {
      break;
    }
    DrawItem(pGraphics, pItem, i, rtItem, mtMatrix);
  }
}

void CFWL_ListBox::DrawItem(CFGAS_GEGraphics* pGraphics,
                            Item* pItem,
                            int32_t Index,
                            const CFX_RectF& rtItem,
                            const CFX_Matrix& mtMatrix) {
  Mask<CFWL_PartState> dwPartStates = CFWL_PartState::kNormal;
  if (properties_.states_ & FWL_STATE_WGT_Disabled) {
    dwPartStates = CFWL_PartState::kDisabled;
  } else if (pItem && pItem->IsSelected()) {
    dwPartStates = CFWL_PartState::kSelected;
  }

  if ((properties_.states_ & FWL_STATE_WGT_Focused) && pItem &&
      pItem->IsFocused()) {
    dwPartStates |= CFWL_PartState::kFocused;
  }

  CFX_RectF rtFocus(rtItem);  // Must outlive |bg_param|.
  CFWL_ThemeBackground bg_param(CFWL_ThemePart::Part::kListItem, this,
                                pGraphics);
  bg_param.states_ = dwPartStates;
  bg_param.matrix_ = mtMatrix;
  bg_param.part_rect_ = rtItem;
  bg_param.maximize_ = true;
  bg_param.data_rect_ = &rtFocus;
  if (vert_scroll_bar_ && !horz_scroll_bar_ &&
      (dwPartStates & CFWL_PartState::kFocused)) {
    bg_param.part_rect_.left += 1;
    bg_param.part_rect_.width -= (scorll_bar_width_ + 1);
    rtFocus.Deflate(0.5, 0.5, 1 + scorll_bar_width_, 1);
  }

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  pTheme->DrawBackground(bg_param);
  if (!pItem) {
    return;
  }

  WideString wsText = pItem->GetText();
  if (wsText.GetLength() <= 0) {
    return;
  }

  CFX_RectF rtText(rtItem);
  rtText.Deflate(kItemTextMargin, kItemTextMargin);

  CFWL_ThemeText textParam(CFWL_ThemePart::Part::kListItem, this, pGraphics);
  textParam.states_ = dwPartStates;
  textParam.matrix_ = mtMatrix;
  textParam.part_rect_ = rtText;
  textParam.text_ = std::move(wsText);
  textParam.tto_styles_ = tto_styles_;
  textParam.tto_align_ = ttoaligns_;
  textParam.maximize_ = true;
  pTheme->DrawText(textParam);
}

CFX_SizeF CFWL_ListBox::CalcSize() {
  client_rect_ = GetClientRect();
  content_rect_ = client_rect_;
  CFX_RectF rtUIMargin;
  if (!GetOuter()) {
    CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
    CFX_RectF pUIMargin = GetThemeProvider()->GetUIMargin(part);
    content_rect_.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                          pUIMargin.height);
  }

  float fWidth = GetMaxTextWidth();
  fWidth += 2 * kItemTextMargin;

  float fActualWidth = client_rect_.width - rtUIMargin.left - rtUIMargin.width;
  fWidth = std::max(fWidth, fActualWidth);
  item_height_ = CalcItemHeight();

  int32_t iCount = CountItems(this);
  CFX_SizeF fs;
  for (int32_t i = 0; i < iCount; i++) {
    Item* htem = GetItem(this, i);
    UpdateItemSize(htem, fs, fWidth, item_height_);
  }

  float iHeight = client_rect_.height;
  bool bShowVertScr = false;
  bool bShowHorzScr = false;
  if (!bShowVertScr && (properties_.styles_ & FWL_STYLE_WGT_VScroll)) {
    bShowVertScr = (fs.height > iHeight);
  }

  float fMax = 0.0f;
  if (bShowVertScr) {
    if (!vert_scroll_bar_) {
      InitVerticalScrollBar();
    }

    CFX_RectF rtScrollBar(client_rect_.right() - scorll_bar_width_,
                          client_rect_.top, scorll_bar_width_,
                          client_rect_.height - 1);
    if (bShowHorzScr) {
      rtScrollBar.height -= scorll_bar_width_;
    }

    vert_scroll_bar_->SetWidgetRect(rtScrollBar);
    fMax = std::max(fs.height - content_rect_.height, item_height_);

    vert_scroll_bar_->SetRange(0.0f, fMax);
    vert_scroll_bar_->SetPageSize(rtScrollBar.height * 9 / 10);
    vert_scroll_bar_->SetStepSize(item_height_);

    float fPos = std::clamp(vert_scroll_bar_->GetPos(), 0.0f, fMax);
    vert_scroll_bar_->SetPos(fPos);
    vert_scroll_bar_->SetTrackPos(fPos);
    if ((properties_.style_exts_ & FWL_STYLEEXT_LTB_ShowScrollBarFocus) == 0 ||
        (properties_.states_ & FWL_STATE_WGT_Focused)) {
      vert_scroll_bar_->RemoveStates(FWL_STATE_WGT_Invisible);
    }
    vert_scroll_bar_->Update();
  } else if (vert_scroll_bar_) {
    vert_scroll_bar_->SetPos(0);
    vert_scroll_bar_->SetTrackPos(0);
    vert_scroll_bar_->SetStates(FWL_STATE_WGT_Invisible);
  }
  if (bShowHorzScr) {
    if (!horz_scroll_bar_) {
      InitHorizontalScrollBar();
    }

    CFX_RectF rtScrollBar(client_rect_.left,
                          client_rect_.bottom() - scorll_bar_width_,
                          client_rect_.width, scorll_bar_width_);
    if (bShowVertScr) {
      rtScrollBar.width -= scorll_bar_width_;
    }

    horz_scroll_bar_->SetWidgetRect(rtScrollBar);
    fMax = fs.width - rtScrollBar.width;
    horz_scroll_bar_->SetRange(0.0f, fMax);
    horz_scroll_bar_->SetPageSize(fWidth * 9 / 10);
    horz_scroll_bar_->SetStepSize(fWidth / 10);

    float fPos = std::clamp(horz_scroll_bar_->GetPos(), 0.0f, fMax);
    horz_scroll_bar_->SetPos(fPos);
    horz_scroll_bar_->SetTrackPos(fPos);
    if ((properties_.style_exts_ & FWL_STYLEEXT_LTB_ShowScrollBarFocus) == 0 ||
        (properties_.states_ & FWL_STATE_WGT_Focused)) {
      horz_scroll_bar_->RemoveStates(FWL_STATE_WGT_Invisible);
    }
    horz_scroll_bar_->Update();
  } else if (horz_scroll_bar_) {
    horz_scroll_bar_->SetPos(0);
    horz_scroll_bar_->SetTrackPos(0);
    horz_scroll_bar_->SetStates(FWL_STATE_WGT_Invisible);
  }
  if (bShowVertScr && bShowHorzScr) {
    static_rect_ = CFX_RectF(client_rect_.right() - scorll_bar_width_,
                             client_rect_.bottom() - scorll_bar_width_,
                             scorll_bar_width_, scorll_bar_width_);
  }
  return fs;
}

void CFWL_ListBox::UpdateItemSize(Item* pItem,
                                  CFX_SizeF& size,
                                  float fWidth,
                                  float fItemHeight) const {
  if (pItem) {
    CFX_RectF rtItem(0, size.height, fWidth, fItemHeight);
    pItem->SetRect(rtItem);
  }
  size.width = fWidth;
  size.height += fItemHeight;
}

float CFWL_ListBox::GetMaxTextWidth() {
  float fRet = 0.0f;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem) {
      continue;
    }

    CFX_SizeF sz = CalcTextSize(pItem->GetText(), false);
    fRet = std::max(fRet, sz.width);
  }
  return fRet;
}

float CFWL_ListBox::GetScrollWidth() {
  return GetThemeProvider()->GetScrollBarWidth();
}

float CFWL_ListBox::CalcItemHeight() {
  CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
  return GetThemeProvider()->GetFontSize(part) + 2 * kItemTextMargin;
}

void CFWL_ListBox::InitVerticalScrollBar() {
  if (vert_scroll_bar_) {
    return;
  }

  vert_scroll_bar_ = cppgc::MakeGarbageCollected<CFWL_ScrollBar>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      Properties{0, FWL_STYLEEXT_SCB_Vert, FWL_STATE_WGT_Invisible}, this);
}

void CFWL_ListBox::InitHorizontalScrollBar() {
  if (horz_scroll_bar_) {
    return;
  }

  horz_scroll_bar_ = cppgc::MakeGarbageCollected<CFWL_ScrollBar>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      Properties{0, FWL_STYLEEXT_SCB_Horz, FWL_STATE_WGT_Invisible}, this);
}

bool CFWL_ListBox::IsShowVertScrollBar() const {
  return vert_scroll_bar_ && vert_scroll_bar_->IsVisible() &&
         ScrollBarPropertiesPresent();
}

bool CFWL_ListBox::IsShowHorzScrollBar() const {
  return horz_scroll_bar_ && horz_scroll_bar_->IsVisible() &&
         ScrollBarPropertiesPresent();
}

bool CFWL_ListBox::ScrollBarPropertiesPresent() const {
  return !(properties_.style_exts_ & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ||
         (properties_.states_ & FWL_STATE_WGT_Focused);
}

void CFWL_ListBox::OnProcessMessage(CFWL_Message* pMessage) {
  if (!IsEnabled()) {
    return;
  }

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
      OnFocusGained();
      break;
    case CFWL_Message::Type::kKillFocus:
      OnFocusLost();
      break;
    case CFWL_Message::Type::kMouse: {
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
    case CFWL_Message::Type::kMouseWheel:
      OnMouseWheel(static_cast<CFWL_MessageMouseWheel*>(pMessage));
      break;
    case CFWL_Message::Type::kKey: {
      CFWL_MessageKey* pMsg = static_cast<CFWL_MessageKey*>(pMessage);
      if (pMsg->cmd_ == CFWL_MessageKey::KeyCommand::kKeyDown) {
        OnKeyDown(pMsg);
      }
      break;
    }
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget()) {
    CFWL_Widget::OnProcessMessage(pMessage);
  }
}

void CFWL_ListBox::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent) {
    return;
  }
  if (pEvent->GetType() != CFWL_Event::Type::Scroll) {
    return;
  }

  CFWL_Widget* pSrcTarget = pEvent->GetSrcTarget();
  if ((pSrcTarget == vert_scroll_bar_ && vert_scroll_bar_) ||
      (pSrcTarget == horz_scroll_bar_ && horz_scroll_bar_)) {
    CFWL_EventScroll* pScrollEvent = static_cast<CFWL_EventScroll*>(pEvent);
    OnScroll(static_cast<CFWL_ScrollBar*>(pSrcTarget),
             pScrollEvent->GetScrollCode(), pScrollEvent->GetPos());
  }
}

void CFWL_ListBox::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_ListBox::OnFocusGained() {
  if (GetStyleExts() & FWL_STYLEEXT_LTB_ShowScrollBarFocus) {
    if (vert_scroll_bar_) {
      vert_scroll_bar_->RemoveStates(FWL_STATE_WGT_Invisible);
    }
    if (horz_scroll_bar_) {
      horz_scroll_bar_->RemoveStates(FWL_STATE_WGT_Invisible);
    }
  }
  properties_.states_ |= FWL_STATE_WGT_Focused;
  RepaintRect(client_rect_);
}

void CFWL_ListBox::OnFocusLost() {
  if (GetStyleExts() & FWL_STYLEEXT_LTB_ShowScrollBarFocus) {
    if (vert_scroll_bar_) {
      vert_scroll_bar_->SetStates(FWL_STATE_WGT_Invisible);
    }
    if (horz_scroll_bar_) {
      horz_scroll_bar_->SetStates(FWL_STATE_WGT_Invisible);
    }
  }
  properties_.states_ &= ~FWL_STATE_WGT_Focused;
  RepaintRect(client_rect_);
}

void CFWL_ListBox::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  lbutton_down_ = true;

  Item* pItem = GetItemAtPoint(pMsg->pos_);
  if (!pItem) {
    return;
  }

  if (IsMultiSelection()) {
    if (pMsg->flags_ & XFA_FWL_KeyFlag::kCtrl) {
      pItem->SetSelected(!pItem->IsSelected());
      h_anchor_ = pItem;
    } else if (pMsg->flags_ & XFA_FWL_KeyFlag::kShift) {
      if (h_anchor_) {
        SetSelection(h_anchor_, pItem, true);
      } else {
        pItem->SetSelected(true);
      }
    } else {
      SetSelection(pItem, pItem, true);
      h_anchor_ = pItem;
    }
  } else {
    SetSelection(pItem, pItem, true);
  }

  SetFocusItem(pItem);
  ScrollToVisible(pItem);
  SetGrab(true);
  RepaintRect(client_rect_);
}

void CFWL_ListBox::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (!lbutton_down_) {
    return;
  }

  lbutton_down_ = false;
  SetGrab(false);
}

void CFWL_ListBox::OnMouseWheel(CFWL_MessageMouseWheel* pMsg) {
  if (IsShowVertScrollBar()) {
    vert_scroll_bar_->GetDelegate()->OnProcessMessage(pMsg);
  }
}

void CFWL_ListBox::OnKeyDown(CFWL_MessageKey* pMsg) {
  auto dwKeyCode = static_cast<XFA_FWL_VKEYCODE>(pMsg->key_code_or_char_);
  switch (dwKeyCode) {
    case XFA_FWL_VKEY_Tab:
    case XFA_FWL_VKEY_Up:
    case XFA_FWL_VKEY_Down:
    case XFA_FWL_VKEY_Home:
    case XFA_FWL_VKEY_End: {
      Item* pItem = GetListItem(GetFocusedItem(), dwKeyCode);
      bool bShift = !!(pMsg->flags_ & XFA_FWL_KeyFlag::kShift);
      bool bCtrl = !!(pMsg->flags_ & XFA_FWL_KeyFlag::kCtrl);
      OnVK(pItem, bShift, bCtrl);
      break;
    }
    default:
      break;
  }
}

void CFWL_ListBox::OnVK(Item* pItem, bool bShift, bool bCtrl) {
  if (!pItem) {
    return;
  }

  if (IsMultiSelection()) {
    if (bCtrl) {
      // Do nothing.
    } else if (bShift) {
      if (h_anchor_) {
        SetSelection(h_anchor_, pItem, true);
      } else {
        pItem->SetSelected(true);
      }
    } else {
      SetSelection(pItem, pItem, true);
      h_anchor_ = pItem;
    }
  } else {
    SetSelection(pItem, pItem, true);
  }

  SetFocusItem(pItem);
  ScrollToVisible(pItem);
  RepaintRect(CFX_RectF(0, 0, widget_rect_.width, widget_rect_.height));
}

bool CFWL_ListBox::OnScroll(CFWL_ScrollBar* pScrollBar,
                            CFWL_EventScroll::Code dwCode,
                            float fPos) {
  float fMin;
  float fMax;
  pScrollBar->GetRange(&fMin, &fMax);
  float iCurPos = pScrollBar->GetPos();
  float fStep = pScrollBar->GetStepSize();
  switch (dwCode) {
    case CFWL_EventScroll::Code::Min: {
      fPos = fMin;
      break;
    }
    case CFWL_EventScroll::Code::Max: {
      fPos = fMax;
      break;
    }
    case CFWL_EventScroll::Code::StepBackward: {
      fPos -= fStep;
      if (fPos < fMin + fStep / 2) {
        fPos = fMin;
      }
      break;
    }
    case CFWL_EventScroll::Code::StepForward: {
      fPos += fStep;
      if (fPos > fMax - fStep / 2) {
        fPos = fMax;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageBackward: {
      fPos -= pScrollBar->GetPageSize();
      if (fPos < fMin) {
        fPos = fMin;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageForward: {
      fPos += pScrollBar->GetPageSize();
      if (fPos > fMax) {
        fPos = fMax;
      }
      break;
    }
    case CFWL_EventScroll::Code::Pos:
    case CFWL_EventScroll::Code::TrackPos:
    case CFWL_EventScroll::Code::None:
      break;
    case CFWL_EventScroll::Code::EndScroll:
      return false;
  }
  if (iCurPos != fPos) {
    pScrollBar->SetPos(fPos);
    pScrollBar->SetTrackPos(fPos);
    RepaintRect(client_rect_);
  }
  return true;
}

int32_t CFWL_ListBox::CountItems(const CFWL_Widget* pWidget) const {
  return fxcrt::CollectionSize<int32_t>(item_array_);
}

CFWL_ListBox::Item* CFWL_ListBox::GetItem(const CFWL_Widget* pWidget,
                                          int32_t nIndex) const {
  if (nIndex < 0 || nIndex >= CountItems(pWidget)) {
    return nullptr;
  }
  return item_array_[nIndex].get();
}

int32_t CFWL_ListBox::GetItemIndex(CFWL_Widget* pWidget, Item* pItem) {
  auto it = std::ranges::find_if(item_array_, pdfium::MatchesUniquePtr(pItem));
  return it != item_array_.end()
             ? checked_cast<int32_t>(it - item_array_.begin())
             : -1;
}

CFWL_ListBox::Item* CFWL_ListBox::AddString(const WideString& wsAdd) {
  item_array_.push_back(std::make_unique<Item>(wsAdd));
  return item_array_.back().get();
}

void CFWL_ListBox::RemoveAt(int32_t iIndex) {
  if (iIndex < 0 || static_cast<size_t>(iIndex) >= item_array_.size()) {
    return;
  }
  item_array_.erase(item_array_.begin() + iIndex);
}

void CFWL_ListBox::DeleteString(Item* pItem) {
  int32_t nIndex = GetItemIndex(this, pItem);
  if (nIndex < 0 || static_cast<size_t>(nIndex) >= item_array_.size()) {
    return;
  }

  int32_t iSel = nIndex + 1;
  if (iSel >= CountItems(this)) {
    iSel = nIndex - 1;
  }
  if (iSel >= 0) {
    Item* item = GetItem(this, iSel);
    if (item) {
      item->SetSelected(true);
    }
  }
  item_array_.erase(item_array_.begin() + nIndex);
}

void CFWL_ListBox::DeleteAll() {
  item_array_.clear();
}

CFWL_ListBox::Item::Item(const WideString& text) : text_(text) {}

CFWL_ListBox::Item::~Item() = default;

}  // namespace pdfium
