// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fxedit/fxet_list.h"

#include <algorithm>
#include <utility>

#include "core/fpdfdoc/cpvt_word.h"
#include "core/fxcrt/fx_extension.h"
#include "fpdfsdk/fxedit/fxet_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_list_box.h"
#include "third_party/base/stl_util.h"

CFX_ListItem::CFX_ListItem()
    : m_pEdit(new CFX_Edit),
      m_bSelected(false),
      m_rcListItem(0.0f, 0.0f, 0.0f, 0.0f) {
  m_pEdit->SetAlignmentV(1, true);
  m_pEdit->Initialize();
}

CFX_ListItem::~CFX_ListItem() {
}

void CFX_ListItem::SetFontMap(IPVT_FontMap* pFontMap) {
  m_pEdit->SetFontMap(pFontMap);
}

CFX_Edit* CFX_ListItem::GetEdit() const {
  return m_pEdit.get();
}

CFX_Edit_Iterator* CFX_ListItem::GetIterator() const {
  return m_pEdit->GetIterator();
}

void CFX_ListItem::SetRect(const CLST_Rect& rect) {
  m_rcListItem = rect;
}

CLST_Rect CFX_ListItem::GetRect() const {
  return m_rcListItem;
}

bool CFX_ListItem::IsSelected() const {
  return m_bSelected;
}

void CFX_ListItem::SetSelect(bool bSelected) {
  m_bSelected = bSelected;
}

void CFX_ListItem::SetText(const CFX_WideString& text) {
  m_pEdit->SetText(text);
}

void CFX_ListItem::SetFontSize(float fFontSize) {
  m_pEdit->SetFontSize(fFontSize);
}

float CFX_ListItem::GetItemHeight() const {
  return m_pEdit->GetContentRect().Height();
}

uint16_t CFX_ListItem::GetFirstChar() const {
  CPVT_Word word;
  CFX_Edit_Iterator* pIterator = GetIterator();
  pIterator->SetAt(1);
  pIterator->GetWord(word);
  return word.Word;
}

CFX_WideString CFX_ListItem::GetText() const {
  return m_pEdit->GetText();
}

CFX_ListContainer::CFX_ListContainer() {}

CFX_ListContainer::~CFX_ListContainer() {}

void CFX_ListContainer::SetPlateRect(const CFX_FloatRect& rect) {
  m_rcPlate = rect;
}

CPLST_Select::CPLST_Select() {}

CPLST_Select::~CPLST_Select() {}

void CPLST_Select::Add(int32_t nItemIndex) {
  m_Items[nItemIndex] = SELECTING;
}

void CPLST_Select::Add(int32_t nBeginIndex, int32_t nEndIndex) {
  if (nBeginIndex > nEndIndex)
    std::swap(nBeginIndex, nEndIndex);

  for (int32_t i = nBeginIndex; i <= nEndIndex; ++i)
    Add(i);
}

void CPLST_Select::Sub(int32_t nItemIndex) {
  auto it = m_Items.find(nItemIndex);
  if (it != m_Items.end())
    it->second = DESELECTING;
}

void CPLST_Select::Sub(int32_t nBeginIndex, int32_t nEndIndex) {
  if (nBeginIndex > nEndIndex)
    std::swap(nBeginIndex, nEndIndex);

  for (int32_t i = nBeginIndex; i <= nEndIndex; ++i)
    Sub(i);
}

void CPLST_Select::DeselectAll() {
  for (auto& item : m_Items)
    item.second = DESELECTING;
}

void CPLST_Select::Done() {
  auto it = m_Items.begin();
  while (it != m_Items.end()) {
    if (it->second == DESELECTING)
      it = m_Items.erase(it);
    else
      (it++)->second = NORMAL;
  }
}

CFX_ListCtrl::CFX_ListCtrl()
    : m_pNotify(nullptr),
      m_bNotifyFlag(false),
      m_nSelItem(-1),
      m_nFootIndex(-1),
      m_bCtrlSel(false),
      m_nCaretIndex(-1),
      m_fFontSize(0.0f),
      m_pFontMap(nullptr),
      m_bMultiple(false) {}

CFX_ListCtrl::~CFX_ListCtrl() {
  Empty();
}

void CFX_ListCtrl::SetNotify(CPWL_List_Notify* pNotify) {
  m_pNotify = pNotify;
}

CFX_PointF CFX_ListCtrl::InToOut(const CFX_PointF& point) const {
  CFX_FloatRect rcPlate = GetPlateRect();
  return CFX_PointF(point.x - (m_ptScrollPos.x - rcPlate.left),
                    point.y - (m_ptScrollPos.y - rcPlate.top));
}

CFX_PointF CFX_ListCtrl::OutToIn(const CFX_PointF& point) const {
  CFX_FloatRect rcPlate = GetPlateRect();
  return CFX_PointF(point.x + (m_ptScrollPos.x - rcPlate.left),
                    point.y + (m_ptScrollPos.y - rcPlate.top));
}

CFX_FloatRect CFX_ListCtrl::InToOut(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftBottom = InToOut(CFX_PointF(rect.left, rect.bottom));
  CFX_PointF ptRightTop = InToOut(CFX_PointF(rect.right, rect.top));
  return CFX_FloatRect(ptLeftBottom.x, ptLeftBottom.y, ptRightTop.x,
                       ptRightTop.y);
}

CFX_FloatRect CFX_ListCtrl::OutToIn(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftBottom = OutToIn(CFX_PointF(rect.left, rect.bottom));
  CFX_PointF ptRightTop = OutToIn(CFX_PointF(rect.right, rect.top));
  return CFX_FloatRect(ptLeftBottom.x, ptLeftBottom.y, ptRightTop.x,
                       ptRightTop.y);
}

void CFX_ListCtrl::OnMouseDown(const CFX_PointF& point,
                               bool bShift,
                               bool bCtrl) {
  int32_t nHitIndex = GetItemIndex(point);

  if (IsMultipleSel()) {
    if (bCtrl) {
      if (IsItemSelected(nHitIndex)) {
        m_aSelItems.Sub(nHitIndex);
        SelectItems();
        m_bCtrlSel = false;
      } else {
        m_aSelItems.Add(nHitIndex);
        SelectItems();
        m_bCtrlSel = true;
      }

      m_nFootIndex = nHitIndex;
    } else if (bShift) {
      m_aSelItems.DeselectAll();
      m_aSelItems.Add(m_nFootIndex, nHitIndex);
      SelectItems();
    } else {
      m_aSelItems.DeselectAll();
      m_aSelItems.Add(nHitIndex);
      SelectItems();

      m_nFootIndex = nHitIndex;
    }

    SetCaret(nHitIndex);
  } else {
    SetSingleSelect(nHitIndex);
  }

  if (!IsItemVisible(nHitIndex))
    ScrollToListItem(nHitIndex);
}

void CFX_ListCtrl::OnMouseMove(const CFX_PointF& point,
                               bool bShift,
                               bool bCtrl) {
  int32_t nHitIndex = GetItemIndex(point);

  if (IsMultipleSel()) {
    if (bCtrl) {
      if (m_bCtrlSel)
        m_aSelItems.Add(m_nFootIndex, nHitIndex);
      else
        m_aSelItems.Sub(m_nFootIndex, nHitIndex);

      SelectItems();
    } else {
      m_aSelItems.DeselectAll();
      m_aSelItems.Add(m_nFootIndex, nHitIndex);
      SelectItems();
    }

    SetCaret(nHitIndex);
  } else {
    SetSingleSelect(nHitIndex);
  }

  if (!IsItemVisible(nHitIndex))
    ScrollToListItem(nHitIndex);
}

void CFX_ListCtrl::OnVK(int32_t nItemIndex, bool bShift, bool bCtrl) {
  if (IsMultipleSel()) {
    if (nItemIndex >= 0 && nItemIndex < GetCount()) {
      if (bCtrl) {
      } else if (bShift) {
        m_aSelItems.DeselectAll();
        m_aSelItems.Add(m_nFootIndex, nItemIndex);
        SelectItems();
      } else {
        m_aSelItems.DeselectAll();
        m_aSelItems.Add(nItemIndex);
        SelectItems();
        m_nFootIndex = nItemIndex;
      }

      SetCaret(nItemIndex);
    }
  } else {
    SetSingleSelect(nItemIndex);
  }

  if (!IsItemVisible(nItemIndex))
    ScrollToListItem(nItemIndex);
}

void CFX_ListCtrl::OnVK_UP(bool bShift, bool bCtrl) {
  OnVK(IsMultipleSel() ? GetCaret() - 1 : GetSelect() - 1, bShift, bCtrl);
}

void CFX_ListCtrl::OnVK_DOWN(bool bShift, bool bCtrl) {
  OnVK(IsMultipleSel() ? GetCaret() + 1 : GetSelect() + 1, bShift, bCtrl);
}

void CFX_ListCtrl::OnVK_LEFT(bool bShift, bool bCtrl) {
  OnVK(0, bShift, bCtrl);
}

void CFX_ListCtrl::OnVK_RIGHT(bool bShift, bool bCtrl) {
  OnVK(GetCount() - 1, bShift, bCtrl);
}

void CFX_ListCtrl::OnVK_HOME(bool bShift, bool bCtrl) {
  OnVK(0, bShift, bCtrl);
}

void CFX_ListCtrl::OnVK_END(bool bShift, bool bCtrl) {
  OnVK(GetCount() - 1, bShift, bCtrl);
}

bool CFX_ListCtrl::OnChar(uint16_t nChar, bool bShift, bool bCtrl) {
  int32_t nIndex = GetLastSelected();
  int32_t nFindIndex = FindNext(nIndex, nChar);

  if (nFindIndex != nIndex) {
    OnVK(nFindIndex, bShift, bCtrl);
    return true;
  }
  return false;
}

void CFX_ListCtrl::SetPlateRect(const CFX_FloatRect& rect) {
  CFX_ListContainer::SetPlateRect(rect);
  m_ptScrollPos.x = rect.left;
  SetScrollPos(CFX_PointF(rect.left, rect.top));
  ReArrange(0);
  InvalidateItem(-1);
}

CFX_FloatRect CFX_ListCtrl::GetItemRect(int32_t nIndex) const {
  return InToOut(GetItemRectInternal(nIndex));
}

CFX_FloatRect CFX_ListCtrl::GetItemRectInternal(int32_t nIndex) const {
  if (!pdfium::IndexInBounds(m_ListItems, nIndex) || !m_ListItems[nIndex])
    return CFX_FloatRect();

  CFX_FloatRect rcItem = m_ListItems[nIndex]->GetRect();
  rcItem.left = 0.0f;
  rcItem.right = GetPlateRect().Width();
  return InnerToOuter(CLST_Rect(rcItem));
}

int32_t CFX_ListCtrl::GetCaret() const {
  return m_nCaretIndex;
}

int32_t CFX_ListCtrl::GetSelect() const {
  return m_nSelItem;
}

void CFX_ListCtrl::AddString(const CFX_WideString& str) {
  AddItem(str);
  ReArrange(GetCount() - 1);
}

void CFX_ListCtrl::SetMultipleSelect(int32_t nItemIndex, bool bSelected) {
  if (!IsValid(nItemIndex))
    return;

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

void CFX_ListCtrl::SetSingleSelect(int32_t nItemIndex) {
  if (!IsValid(nItemIndex))
    return;

  if (m_nSelItem != nItemIndex) {
    if (m_nSelItem >= 0) {
      SetItemSelect(m_nSelItem, false);
      InvalidateItem(m_nSelItem);
    }

    SetItemSelect(nItemIndex, true);
    InvalidateItem(nItemIndex);
    m_nSelItem = nItemIndex;
  }
}

void CFX_ListCtrl::SetCaret(int32_t nItemIndex) {
  if (!IsValid(nItemIndex))
    return;

  if (IsMultipleSel()) {
    int32_t nOldIndex = m_nCaretIndex;

    if (nOldIndex != nItemIndex) {
      m_nCaretIndex = nItemIndex;
      InvalidateItem(nOldIndex);
      InvalidateItem(nItemIndex);
    }
  }
}

void CFX_ListCtrl::InvalidateItem(int32_t nItemIndex) {
  if (m_pNotify) {
    if (nItemIndex == -1) {
      if (!m_bNotifyFlag) {
        m_bNotifyFlag = true;
        CFX_FloatRect rcRefresh = GetPlateRect();
        m_pNotify->IOnInvalidateRect(&rcRefresh);
        m_bNotifyFlag = false;
      }
    } else {
      if (!m_bNotifyFlag) {
        m_bNotifyFlag = true;
        CFX_FloatRect rcRefresh = GetItemRect(nItemIndex);
        rcRefresh.left -= 1.0f;
        rcRefresh.right += 1.0f;
        rcRefresh.bottom -= 1.0f;
        rcRefresh.top += 1.0f;

        m_pNotify->IOnInvalidateRect(&rcRefresh);
        m_bNotifyFlag = false;
      }
    }
  }
}

void CFX_ListCtrl::SelectItems() {
  for (const auto& item : m_aSelItems) {
    if (item.second != CPLST_Select::NORMAL)
      SetMultipleSelect(item.first, item.second == CPLST_Select::SELECTING);
  }
  m_aSelItems.Done();
}

void CFX_ListCtrl::Select(int32_t nItemIndex) {
  if (!IsValid(nItemIndex))
    return;

  if (IsMultipleSel()) {
    m_aSelItems.Add(nItemIndex);
    SelectItems();
  } else {
    SetSingleSelect(nItemIndex);
  }
}

bool CFX_ListCtrl::IsItemVisible(int32_t nItemIndex) const {
  CFX_FloatRect rcPlate = GetPlateRect();
  CFX_FloatRect rcItem = GetItemRect(nItemIndex);

  return rcItem.bottom >= rcPlate.bottom && rcItem.top <= rcPlate.top;
}

void CFX_ListCtrl::ScrollToListItem(int32_t nItemIndex) {
  if (!IsValid(nItemIndex))
    return;

  CFX_FloatRect rcPlate = GetPlateRect();
  CFX_FloatRect rcItem = GetItemRectInternal(nItemIndex);
  CFX_FloatRect rcItemCtrl = GetItemRect(nItemIndex);

  if (IsFloatSmaller(rcItemCtrl.bottom, rcPlate.bottom)) {
    if (IsFloatSmaller(rcItemCtrl.top, rcPlate.top)) {
      SetScrollPosY(rcItem.bottom + rcPlate.Height());
    }
  } else if (IsFloatBigger(rcItemCtrl.top, rcPlate.top)) {
    if (IsFloatBigger(rcItemCtrl.bottom, rcPlate.bottom)) {
      SetScrollPosY(rcItem.top);
    }
  }
}

void CFX_ListCtrl::SetScrollInfo() {
  if (m_pNotify) {
    CFX_FloatRect rcPlate = GetPlateRect();
    CFX_FloatRect rcContent = GetContentRectInternal();

    if (!m_bNotifyFlag) {
      m_bNotifyFlag = true;
      m_pNotify->IOnSetScrollInfoY(rcPlate.bottom, rcPlate.top,
                                   rcContent.bottom, rcContent.top,
                                   GetFirstHeight(), rcPlate.Height());
      m_bNotifyFlag = false;
    }
  }
}

void CFX_ListCtrl::SetScrollPos(const CFX_PointF& point) {
  SetScrollPosY(point.y);
}

void CFX_ListCtrl::SetScrollPosY(float fy) {
  if (!IsFloatEqual(m_ptScrollPos.y, fy)) {
    CFX_FloatRect rcPlate = GetPlateRect();
    CFX_FloatRect rcContent = GetContentRectInternal();

    if (rcPlate.Height() > rcContent.Height()) {
      fy = rcPlate.top;
    } else {
      if (IsFloatSmaller(fy - rcPlate.Height(), rcContent.bottom)) {
        fy = rcContent.bottom + rcPlate.Height();
      } else if (IsFloatBigger(fy, rcContent.top)) {
        fy = rcContent.top;
      }
    }

    m_ptScrollPos.y = fy;
    InvalidateItem(-1);

    if (m_pNotify) {
      if (!m_bNotifyFlag) {
        m_bNotifyFlag = true;
        m_pNotify->IOnSetScrollPosY(fy);
        m_bNotifyFlag = false;
      }
    }
  }
}

CFX_FloatRect CFX_ListCtrl::GetContentRectInternal() const {
  return InnerToOuter(CFX_ListContainer::GetContentRect());
}

CFX_FloatRect CFX_ListCtrl::GetContentRect() const {
  return InToOut(GetContentRectInternal());
}

void CFX_ListCtrl::ReArrange(int32_t nItemIndex) {
  float fPosY = 0.0f;
  if (pdfium::IndexInBounds(m_ListItems, nItemIndex - 1) &&
      m_ListItems[nItemIndex - 1]) {
    fPosY = m_ListItems[nItemIndex - 1]->GetRect().bottom;
  }
  for (const auto& pListItem : m_ListItems) {
    if (pListItem) {
      float fListItemHeight = pListItem->GetItemHeight();
      pListItem->SetRect(CLST_Rect(0.0f, fPosY, 0.0f, fPosY + fListItemHeight));
      fPosY += fListItemHeight;
    }
  }
  SetContentRect(CLST_Rect(0.0f, 0.0f, 0.0f, fPosY));
  SetScrollInfo();
}

void CFX_ListCtrl::SetTopItem(int32_t nIndex) {
  if (IsValid(nIndex)) {
    GetPlateRect();
    CFX_FloatRect rcItem = GetItemRectInternal(nIndex);
    SetScrollPosY(rcItem.top);
  }
}

int32_t CFX_ListCtrl::GetTopItem() const {
  int32_t nItemIndex = GetItemIndex(GetBTPoint());
  if (!IsItemVisible(nItemIndex) && IsItemVisible(nItemIndex + 1))
    nItemIndex += 1;

  return nItemIndex;
}

void CFX_ListCtrl::Empty() {
  m_ListItems.clear();
  InvalidateItem(-1);
}

void CFX_ListCtrl::Cancel() {
  m_aSelItems.DeselectAll();
}

int32_t CFX_ListCtrl::GetItemIndex(const CFX_PointF& point) const {
  CFX_PointF pt = OuterToInner(OutToIn(point));
  bool bFirst = true;
  bool bLast = true;
  for (const auto& pListItem : m_ListItems) {
    if (!pListItem)
      continue;
    CLST_Rect rcListItem = pListItem->GetRect();
    if (IsFloatBigger(pt.y, rcListItem.top))
      bFirst = false;
    if (IsFloatSmaller(pt.y, rcListItem.bottom))
      bLast = false;
    if (pt.y >= rcListItem.top && pt.y < rcListItem.bottom)
      return &pListItem - &m_ListItems.front();
  }
  if (bFirst)
    return 0;
  if (bLast)
    return pdfium::CollectionSize<int32_t>(m_ListItems) - 1;
  return -1;
}

CFX_WideString CFX_ListCtrl::GetText() const {
  if (IsMultipleSel())
    return GetItemText(m_nCaretIndex);
  return GetItemText(m_nSelItem);
}

void CFX_ListCtrl::SetFontMap(IPVT_FontMap* pFontMap) {
  m_pFontMap = pFontMap;
}

void CFX_ListCtrl::SetFontSize(float fFontSize) {
  m_fFontSize = fFontSize;
}

void CFX_ListCtrl::AddItem(const CFX_WideString& str) {
  auto pListItem = pdfium::MakeUnique<CFX_ListItem>();
  pListItem->SetFontMap(m_pFontMap.Get());
  pListItem->SetFontSize(m_fFontSize);
  pListItem->SetText(str);
  m_ListItems.push_back(std::move(pListItem));
}

CFX_Edit* CFX_ListCtrl::GetItemEdit(int32_t nIndex) const {
  if (!pdfium::IndexInBounds(m_ListItems, nIndex) || !m_ListItems[nIndex])
    return nullptr;
  return m_ListItems[nIndex]->GetEdit();
}

int32_t CFX_ListCtrl::GetCount() const {
  return pdfium::CollectionSize<int32_t>(m_ListItems);
}

CFX_FloatRect CFX_ListCtrl::GetPlateRect() const {
  return CFX_ListContainer::GetPlateRect();
}

float CFX_ListCtrl::GetFontSize() const {
  return m_fFontSize;
}

float CFX_ListCtrl::GetFirstHeight() const {
  if (m_ListItems.empty() || !m_ListItems.front())
    return 1.0f;

  return m_ListItems.front()->GetItemHeight();
}

int32_t CFX_ListCtrl::GetFirstSelected() const {
  int32_t i = 0;
  for (const auto& pListItem : m_ListItems) {
    if (pListItem && pListItem->IsSelected())
      return i;
    ++i;
  }
  return -1;
}

int32_t CFX_ListCtrl::GetLastSelected() const {
  for (auto iter = m_ListItems.rbegin(); iter != m_ListItems.rend(); ++iter) {
    if (*iter && (*iter)->IsSelected())
      return &*iter - &m_ListItems.front();
  }
  return -1;
}

int32_t CFX_ListCtrl::FindNext(int32_t nIndex, wchar_t nChar) const {
  int32_t nCircleIndex = nIndex;
  int32_t sz = pdfium::CollectionSize<int32_t>(m_ListItems);
  for (int32_t i = 0; i < sz; i++) {
    nCircleIndex++;
    if (nCircleIndex >= sz)
      nCircleIndex = 0;

    if (CFX_ListItem* pListItem = m_ListItems[nCircleIndex].get()) {
      if (FXSYS_toupper(pListItem->GetFirstChar()) == FXSYS_toupper(nChar))
        return nCircleIndex;
    }
  }

  return nCircleIndex;
}

bool CFX_ListCtrl::IsItemSelected(int32_t nIndex) const {
  return pdfium::IndexInBounds(m_ListItems, nIndex) && m_ListItems[nIndex] &&
         m_ListItems[nIndex]->IsSelected();
}

void CFX_ListCtrl::SetItemSelect(int32_t nIndex, bool bSelected) {
  if (pdfium::IndexInBounds(m_ListItems, nIndex) && m_ListItems[nIndex])
    m_ListItems[nIndex]->SetSelect(bSelected);
}

void CFX_ListCtrl::SetMultipleSel(bool bMultiple) {
  m_bMultiple = bMultiple;
}

bool CFX_ListCtrl::IsMultipleSel() const {
  return m_bMultiple;
}

bool CFX_ListCtrl::IsValid(int32_t nItemIndex) const {
  return pdfium::IndexInBounds(m_ListItems, nItemIndex);
}

CFX_WideString CFX_ListCtrl::GetItemText(int32_t nIndex) const {
  if (pdfium::IndexInBounds(m_ListItems, nIndex) && m_ListItems[nIndex])
    return m_ListItems[nIndex]->GetText();

  return L"";
}
