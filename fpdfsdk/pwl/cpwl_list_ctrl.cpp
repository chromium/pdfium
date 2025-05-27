// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_list_ctrl.h"

#include <algorithm>
#include <utility>

#include "core/fpdfdoc/cpvt_word.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"
#include "fpdfsdk/pwl/cpwl_edit_impl.h"
#include "fpdfsdk/pwl/cpwl_list_box.h"

CPWL_ListCtrl::NotifyIface::~NotifyIface() = default;

CPWL_ListCtrl::Item::Item() : edit_(std::make_unique<CPWL_EditImpl>()) {
  edit_->SetAlignmentV(1);
  edit_->Initialize();
}

CPWL_ListCtrl::Item::~Item() = default;

void CPWL_ListCtrl::Item::SetFontMap(IPVT_FontMap* font_map) {
  edit_->SetFontMap(font_map);
}

void CPWL_ListCtrl::Item::SetText(const WideString& text) {
  edit_->SetText(text);
  edit_->Paint();
}

void CPWL_ListCtrl::Item::SetFontSize(float fFontSize) {
  edit_->SetFontSize(fFontSize);
  edit_->Paint();
}

float CPWL_ListCtrl::Item::GetItemHeight() const {
  return edit_->GetContentRect().Height();
}

uint16_t CPWL_ListCtrl::Item::GetFirstChar() const {
  CPVT_Word word;
  CPWL_EditImpl::Iterator* pIterator = edit_->GetIterator();
  pIterator->SetAt(1);
  pIterator->GetWord(word);
  return word.Word;
}

WideString CPWL_ListCtrl::Item::GetText() const {
  return edit_->GetText();
}

CPWL_ListCtrl::SelectState::SelectState() = default;

CPWL_ListCtrl::SelectState::~SelectState() = default;

void CPWL_ListCtrl::SelectState::Add(int32_t nItemIndex) {
  items_[nItemIndex] = SELECTING;
}

void CPWL_ListCtrl::SelectState::Add(int32_t nBeginIndex, int32_t nEndIndex) {
  if (nBeginIndex > nEndIndex) {
    std::swap(nBeginIndex, nEndIndex);
  }

  for (int32_t i = nBeginIndex; i <= nEndIndex; ++i) {
    Add(i);
  }
}

void CPWL_ListCtrl::SelectState::Sub(int32_t nItemIndex) {
  auto it = items_.find(nItemIndex);
  if (it != items_.end()) {
    it->second = DESELECTING;
  }
}

void CPWL_ListCtrl::SelectState::Sub(int32_t nBeginIndex, int32_t nEndIndex) {
  if (nBeginIndex > nEndIndex) {
    std::swap(nBeginIndex, nEndIndex);
  }

  for (int32_t i = nBeginIndex; i <= nEndIndex; ++i) {
    Sub(i);
  }
}

void CPWL_ListCtrl::SelectState::DeselectAll() {
  for (auto& item : items_) {
    item.second = DESELECTING;
  }
}

void CPWL_ListCtrl::SelectState::Done() {
  auto it = items_.begin();
  while (it != items_.end()) {
    if (it->second == DESELECTING) {
      it = items_.erase(it);
    } else {
      (it++)->second = NORMAL;
    }
  }
}

CPWL_ListCtrl::CPWL_ListCtrl() = default;

CPWL_ListCtrl::~CPWL_ListCtrl() {
  list_items_.clear();
  InvalidateItem(-1);
}

CFX_PointF CPWL_ListCtrl::InToOut(const CFX_PointF& point) const {
  CFX_FloatRect rcPlate = plate_rect_;
  return CFX_PointF(point.x - (scroll_pos_point_.x - rcPlate.left),
                    point.y - (scroll_pos_point_.y - rcPlate.top));
}

CFX_PointF CPWL_ListCtrl::OutToIn(const CFX_PointF& point) const {
  CFX_FloatRect rcPlate = plate_rect_;
  return CFX_PointF(point.x + (scroll_pos_point_.x - rcPlate.left),
                    point.y + (scroll_pos_point_.y - rcPlate.top));
}

CFX_FloatRect CPWL_ListCtrl::InToOut(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftBottom = InToOut(CFX_PointF(rect.left, rect.bottom));
  CFX_PointF ptRightTop = InToOut(CFX_PointF(rect.right, rect.top));
  return CFX_FloatRect(ptLeftBottom.x, ptLeftBottom.y, ptRightTop.x,
                       ptRightTop.y);
}

CFX_FloatRect CPWL_ListCtrl::OutToIn(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftBottom = OutToIn(CFX_PointF(rect.left, rect.bottom));
  CFX_PointF ptRightTop = OutToIn(CFX_PointF(rect.right, rect.top));
  return CFX_FloatRect(ptLeftBottom.x, ptLeftBottom.y, ptRightTop.x,
                       ptRightTop.y);
}

CFX_PointF CPWL_ListCtrl::InnerToOuter(const CFX_PointF& point) const {
  return CFX_PointF(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
}

CFX_PointF CPWL_ListCtrl::OuterToInner(const CFX_PointF& point) const {
  return CFX_PointF(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
}

CFX_FloatRect CPWL_ListCtrl::InnerToOuter(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftTop = InnerToOuter(CFX_PointF(rect.left, rect.top));
  CFX_PointF ptRightBottom = InnerToOuter(CFX_PointF(rect.right, rect.bottom));
  return CFX_FloatRect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                       ptLeftTop.y);
}

CFX_FloatRect CPWL_ListCtrl::OuterToInner(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftTop = OuterToInner(CFX_PointF(rect.left, rect.top));
  CFX_PointF ptRightBottom = OuterToInner(CFX_PointF(rect.right, rect.bottom));
  return CFX_FloatRect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                       ptLeftTop.y);
}

void CPWL_ListCtrl::OnMouseDown(const CFX_PointF& point,
                                bool bShift,
                                bool bCtrl) {
  int32_t nHitIndex = GetItemIndex(point);

  if (IsMultipleSel()) {
    if (bCtrl) {
      if (IsItemSelected(nHitIndex)) {
        select_state_.Sub(nHitIndex);
        SelectItems();
        ctrl_sel_ = false;
      } else {
        select_state_.Add(nHitIndex);
        SelectItems();
        ctrl_sel_ = true;
      }

      foot_index_ = nHitIndex;
    } else if (bShift) {
      select_state_.DeselectAll();
      select_state_.Add(foot_index_, nHitIndex);
      SelectItems();
    } else {
      select_state_.DeselectAll();
      select_state_.Add(nHitIndex);
      SelectItems();

      foot_index_ = nHitIndex;
    }

    SetCaret(nHitIndex);
  } else {
    SetSingleSelect(nHitIndex);
  }

  if (!IsItemVisible(nHitIndex)) {
    ScrollToListItem(nHitIndex);
  }
}

void CPWL_ListCtrl::OnMouseMove(const CFX_PointF& point,
                                bool bShift,
                                bool bCtrl) {
  int32_t nHitIndex = GetItemIndex(point);

  if (IsMultipleSel()) {
    if (bCtrl) {
      if (ctrl_sel_) {
        select_state_.Add(foot_index_, nHitIndex);
      } else {
        select_state_.Sub(foot_index_, nHitIndex);
      }

      SelectItems();
    } else {
      select_state_.DeselectAll();
      select_state_.Add(foot_index_, nHitIndex);
      SelectItems();
    }

    SetCaret(nHitIndex);
  } else {
    SetSingleSelect(nHitIndex);
  }

  if (!IsItemVisible(nHitIndex)) {
    ScrollToListItem(nHitIndex);
  }
}

void CPWL_ListCtrl::OnVK(int32_t nItemIndex, bool bShift, bool bCtrl) {
  if (IsMultipleSel()) {
    if (nItemIndex >= 0 && nItemIndex < GetCount()) {
      if (bCtrl) {
      } else if (bShift) {
        select_state_.DeselectAll();
        select_state_.Add(foot_index_, nItemIndex);
        SelectItems();
      } else {
        select_state_.DeselectAll();
        select_state_.Add(nItemIndex);
        SelectItems();
        foot_index_ = nItemIndex;
      }

      SetCaret(nItemIndex);
    }
  } else {
    SetSingleSelect(nItemIndex);
  }

  if (!IsItemVisible(nItemIndex)) {
    ScrollToListItem(nItemIndex);
  }
}

void CPWL_ListCtrl::OnVK_UP(bool bShift, bool bCtrl) {
  OnVK(IsMultipleSel() ? GetCaret() - 1 : GetSelect() - 1, bShift, bCtrl);
}

void CPWL_ListCtrl::OnVK_DOWN(bool bShift, bool bCtrl) {
  OnVK(IsMultipleSel() ? GetCaret() + 1 : GetSelect() + 1, bShift, bCtrl);
}

void CPWL_ListCtrl::OnVK_LEFT(bool bShift, bool bCtrl) {
  OnVK(0, bShift, bCtrl);
}

void CPWL_ListCtrl::OnVK_RIGHT(bool bShift, bool bCtrl) {
  OnVK(GetCount() - 1, bShift, bCtrl);
}

void CPWL_ListCtrl::OnVK_HOME(bool bShift, bool bCtrl) {
  OnVK(0, bShift, bCtrl);
}

void CPWL_ListCtrl::OnVK_END(bool bShift, bool bCtrl) {
  OnVK(GetCount() - 1, bShift, bCtrl);
}

bool CPWL_ListCtrl::OnChar(uint16_t nChar, bool bShift, bool bCtrl) {
  int32_t nIndex = GetLastSelected();
  int32_t nFindIndex = FindNext(nIndex, nChar);

  if (nFindIndex != nIndex) {
    OnVK(nFindIndex, bShift, bCtrl);
    return true;
  }
  return false;
}

void CPWL_ListCtrl::SetPlateRect(const CFX_FloatRect& rect) {
  plate_rect_ = rect;
  scroll_pos_point_.x = rect.left;
  SetScrollPos(CFX_PointF(rect.left, rect.top));
  ReArrange(0);
  InvalidateItem(-1);
}

CFX_FloatRect CPWL_ListCtrl::GetItemRect(int32_t nIndex) const {
  return InToOut(GetItemRectInternal(nIndex));
}

CFX_FloatRect CPWL_ListCtrl::GetItemRectInternal(int32_t nIndex) const {
  if (!IsValid(nIndex)) {
    return CFX_FloatRect();
  }

  CFX_FloatRect rcItem = list_items_[nIndex]->GetRect();
  rcItem.left = 0.0f;
  rcItem.right = plate_rect_.Width();
  return InnerToOuter(rcItem);
}

void CPWL_ListCtrl::AddString(const WideString& str) {
  AddItem(str);
  ReArrange(GetCount() - 1);
}

void CPWL_ListCtrl::SetMultipleSelect(int32_t nItemIndex, bool bSelected) {
  if (!IsValid(nItemIndex)) {
    return;
  }

  if (bSelected != IsItemSelected(nItemIndex)) {
    if (bSelected) {
      SetItemSelect(nItemIndex, true);
      InvalidateItem(nItemIndex);
    } else {
      SetItemSelect(nItemIndex, false);
      InvalidateItem(nItemIndex);
    }
  }
}

void CPWL_ListCtrl::SetSingleSelect(int32_t nItemIndex) {
  if (!IsValid(nItemIndex)) {
    return;
  }

  if (sel_item_ != nItemIndex) {
    if (sel_item_ >= 0) {
      SetItemSelect(sel_item_, false);
      InvalidateItem(sel_item_);
    }

    SetItemSelect(nItemIndex, true);
    InvalidateItem(nItemIndex);
    sel_item_ = nItemIndex;
  }
}

void CPWL_ListCtrl::SetCaret(int32_t nItemIndex) {
  if (!IsValid(nItemIndex)) {
    return;
  }

  if (IsMultipleSel()) {
    int32_t nOldIndex = caret_index_;

    if (nOldIndex != nItemIndex) {
      caret_index_ = nItemIndex;
      InvalidateItem(nOldIndex);
      InvalidateItem(nItemIndex);
    }
  }
}

void CPWL_ListCtrl::InvalidateItem(int32_t nItemIndex) {
  if (!notify_) {
    return;
  }
  if (nItemIndex == -1) {
    if (!notify_flag_) {
      notify_flag_ = true;
      CFX_FloatRect rcRefresh = plate_rect_;
      if (!notify_->OnInvalidateRect(rcRefresh)) {
        notify_ = nullptr;  // Gone, dangling even.
      }
      notify_flag_ = false;
    }
  } else {
    if (!notify_flag_) {
      notify_flag_ = true;
      CFX_FloatRect rcRefresh = GetItemRect(nItemIndex);
      rcRefresh.left -= 1.0f;
      rcRefresh.right += 1.0f;
      rcRefresh.bottom -= 1.0f;
      rcRefresh.top += 1.0f;
      if (!notify_->OnInvalidateRect(rcRefresh)) {
        notify_ = nullptr;  // Gone, dangling even.
      }
      notify_flag_ = false;
    }
  }
}

void CPWL_ListCtrl::SelectItems() {
  for (const auto& item : select_state_) {
    if (item.second != SelectState::NORMAL) {
      SetMultipleSelect(item.first, item.second == SelectState::SELECTING);
    }
  }
  select_state_.Done();
}

void CPWL_ListCtrl::Select(int32_t nItemIndex) {
  if (!IsValid(nItemIndex)) {
    return;
  }

  if (IsMultipleSel()) {
    select_state_.Add(nItemIndex);
    SelectItems();
  } else {
    SetSingleSelect(nItemIndex);
  }
}

void CPWL_ListCtrl::Deselect(int32_t nItemIndex) {
  if (!IsItemSelected(nItemIndex)) {
    return;
  }

  SetMultipleSelect(nItemIndex, false);

  if (!IsMultipleSel()) {
    sel_item_ = -1;
  }
}

bool CPWL_ListCtrl::IsItemVisible(int32_t nItemIndex) const {
  CFX_FloatRect rcPlate = plate_rect_;
  CFX_FloatRect rcItem = GetItemRect(nItemIndex);

  return rcItem.bottom >= rcPlate.bottom && rcItem.top <= rcPlate.top;
}

void CPWL_ListCtrl::ScrollToListItem(int32_t nItemIndex) {
  if (!IsValid(nItemIndex)) {
    return;
  }

  CFX_FloatRect rcPlate = plate_rect_;
  CFX_FloatRect rcItem = GetItemRectInternal(nItemIndex);
  CFX_FloatRect rcItemCtrl = GetItemRect(nItemIndex);

  if (FXSYS_IsFloatSmaller(rcItemCtrl.bottom, rcPlate.bottom)) {
    if (FXSYS_IsFloatSmaller(rcItemCtrl.top, rcPlate.top)) {
      SetScrollPosY(rcItem.bottom + rcPlate.Height());
    }
  } else if (FXSYS_IsFloatBigger(rcItemCtrl.top, rcPlate.top)) {
    if (FXSYS_IsFloatBigger(rcItemCtrl.bottom, rcPlate.bottom)) {
      SetScrollPosY(rcItem.top);
    }
  }
}

void CPWL_ListCtrl::SetScrollInfo() {
  if (notify_) {
    CFX_FloatRect rcPlate = plate_rect_;
    CFX_FloatRect rcContent = GetContentRectInternal();

    if (!notify_flag_) {
      notify_flag_ = true;
      notify_->OnSetScrollInfoY(rcPlate.bottom, rcPlate.top, rcContent.bottom,
                                rcContent.top, GetFirstHeight(),
                                rcPlate.Height());
      notify_flag_ = false;
    }
  }
}

void CPWL_ListCtrl::SetScrollPos(const CFX_PointF& point) {
  SetScrollPosY(point.y);
}

void CPWL_ListCtrl::SetScrollPosY(float fy) {
  if (!FXSYS_IsFloatEqual(scroll_pos_point_.y, fy)) {
    CFX_FloatRect rcPlate = plate_rect_;
    CFX_FloatRect rcContent = GetContentRectInternal();

    if (rcPlate.Height() > rcContent.Height()) {
      fy = rcPlate.top;
    } else {
      if (FXSYS_IsFloatSmaller(fy - rcPlate.Height(), rcContent.bottom)) {
        fy = rcContent.bottom + rcPlate.Height();
      } else if (FXSYS_IsFloatBigger(fy, rcContent.top)) {
        fy = rcContent.top;
      }
    }

    scroll_pos_point_.y = fy;
    InvalidateItem(-1);

    if (notify_) {
      if (!notify_flag_) {
        notify_flag_ = true;
        notify_->OnSetScrollPosY(fy);
        notify_flag_ = false;
      }
    }
  }
}

CFX_FloatRect CPWL_ListCtrl::GetContentRectInternal() const {
  return InnerToOuter(content_rect_);
}

CFX_FloatRect CPWL_ListCtrl::GetContentRect() const {
  return InToOut(GetContentRectInternal());
}

void CPWL_ListCtrl::ReArrange(int32_t nItemIndex) {
  float fPosY = 0.0f;
  if (IsValid(nItemIndex - 1)) {
    fPosY = list_items_[nItemIndex - 1]->GetRect().bottom;
  }

  for (const auto& pListItem : list_items_) {
    float fListItemHeight = pListItem->GetItemHeight();
    pListItem->SetRect(
        CFX_FloatRect(0.0f, fPosY + fListItemHeight, 0.0f, fPosY));
    fPosY += fListItemHeight;
  }
  content_rect_ = CFX_FloatRect(0.0f, fPosY, 0.0f, 0.0f);
  SetScrollInfo();
}

void CPWL_ListCtrl::SetTopItem(int32_t nIndex) {
  if (IsValid(nIndex)) {
    CFX_FloatRect rcItem = GetItemRectInternal(nIndex);
    SetScrollPosY(rcItem.top);
  }
}

int32_t CPWL_ListCtrl::GetTopItem() const {
  int32_t nItemIndex = GetItemIndex(GetBTPoint());
  if (!IsItemVisible(nItemIndex) && IsItemVisible(nItemIndex + 1)) {
    nItemIndex += 1;
  }

  return nItemIndex;
}

int32_t CPWL_ListCtrl::GetItemIndex(const CFX_PointF& point) const {
  CFX_PointF pt = OuterToInner(OutToIn(point));
  bool bFirst = true;
  bool bLast = true;
  for (const auto& pListItem : list_items_) {
    CFX_FloatRect rcListItem = pListItem->GetRect();
    if (FXSYS_IsFloatBigger(pt.y, rcListItem.top)) {
      bFirst = false;
    }
    if (FXSYS_IsFloatSmaller(pt.y, rcListItem.bottom)) {
      bLast = false;
    }
    if (pt.y >= rcListItem.top && pt.y < rcListItem.bottom) {
      return pdfium::checked_cast<int32_t>(&pListItem - &list_items_.front());
    }
  }
  if (bFirst) {
    return 0;
  }
  if (bLast) {
    return GetCount() - 1;
  }
  return -1;
}

WideString CPWL_ListCtrl::GetText() const {
  if (IsMultipleSel()) {
    return GetItemText(caret_index_);
  }
  return GetItemText(sel_item_);
}

void CPWL_ListCtrl::AddItem(const WideString& str) {
  auto pListItem = std::make_unique<Item>();
  pListItem->SetFontMap(font_map_);
  pListItem->SetFontSize(font_size_);
  pListItem->SetText(str);
  list_items_.push_back(std::move(pListItem));
}

CPWL_EditImpl* CPWL_ListCtrl::GetItemEdit(int32_t nIndex) const {
  if (!IsValid(nIndex)) {
    return nullptr;
  }
  return list_items_[nIndex]->GetEdit();
}

int32_t CPWL_ListCtrl::GetCount() const {
  return fxcrt::CollectionSize<int32_t>(list_items_);
}

float CPWL_ListCtrl::GetFirstHeight() const {
  if (list_items_.empty()) {
    return 1.0f;
  }
  return list_items_.front()->GetItemHeight();
}

int32_t CPWL_ListCtrl::GetFirstSelected() const {
  int32_t i = 0;
  for (const auto& pListItem : list_items_) {
    if (pListItem->IsSelected()) {
      return i;
    }
    ++i;
  }
  return -1;
}

int32_t CPWL_ListCtrl::GetLastSelected() const {
  for (auto iter = list_items_.rbegin(); iter != list_items_.rend(); ++iter) {
    if ((*iter)->IsSelected()) {
      return pdfium::checked_cast<int32_t>(&*iter - &list_items_.front());
    }
  }
  return -1;
}

int32_t CPWL_ListCtrl::FindNext(int32_t nIndex, wchar_t nChar) const {
  int32_t nCircleIndex = nIndex;
  int32_t sz = GetCount();
  for (int32_t i = 0; i < sz; i++) {
    nCircleIndex++;
    if (nCircleIndex >= sz) {
      nCircleIndex = 0;
    }

    if (Item* pListItem = list_items_[nCircleIndex].get()) {
      if (FXSYS_towupper(pListItem->GetFirstChar()) == FXSYS_towupper(nChar)) {
        return nCircleIndex;
      }
    }
  }

  return nCircleIndex;
}

bool CPWL_ListCtrl::IsItemSelected(int32_t nIndex) const {
  return IsValid(nIndex) && list_items_[nIndex]->IsSelected();
}

void CPWL_ListCtrl::SetItemSelect(int32_t nIndex, bool bSelected) {
  if (IsValid(nIndex)) {
    list_items_[nIndex]->SetSelect(bSelected);
  }
}

bool CPWL_ListCtrl::IsValid(int32_t nItemIndex) const {
  return fxcrt::IndexInBounds(list_items_, nItemIndex);
}

WideString CPWL_ListCtrl::GetItemText(int32_t nIndex) const {
  if (IsValid(nIndex)) {
    return list_items_[nIndex]->GetText();
  }
  return WideString();
}
