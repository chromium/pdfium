// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_listbox.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcrt/stl_util.h"
#include "third_party/base/numerics/safe_conversions.h"
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
  visitor->Trace(m_pHorzScrollBar);
  visitor->Trace(m_pVertScrollBar);
}

FWL_Type CFWL_ListBox::GetClassID() const {
  return FWL_Type::ListBox;
}

void CFWL_ListBox::Update() {
  if (IsLocked())
    return;

  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_LTB_AlignMask) {
    case FWL_STYLEEXT_LTB_LeftAlign:
      m_iTTOAligns = FDE_TextAlignment::kCenterLeft;
      break;
    case FWL_STYLEEXT_LTB_RightAlign:
      m_iTTOAligns = FDE_TextAlignment::kCenterRight;
      break;
    case FWL_STYLEEXT_LTB_CenterAlign:
    default:
      m_iTTOAligns = FDE_TextAlignment::kCenter;
      break;
  }
  m_TTOStyles.single_line_ = true;
  m_fScorllBarWidth = GetScrollWidth();
  CalcSize();
}

FWL_WidgetHit CFWL_ListBox::HitTest(const CFX_PointF& point) {
  if (IsShowHorzScrollBar()) {
    CFX_RectF rect = m_pHorzScrollBar->GetWidgetRect();
    if (rect.Contains(point))
      return FWL_WidgetHit::HScrollBar;
  }
  if (IsShowVertScrollBar()) {
    CFX_RectF rect = m_pVertScrollBar->GetWidgetRect();
    if (rect.Contains(point))
      return FWL_WidgetHit::VScrollBar;
  }
  if (m_ClientRect.Contains(point))
    return FWL_WidgetHit::Client;
  return FWL_WidgetHit::Unknown;
}

void CFWL_ListBox::DrawWidget(CFGAS_GEGraphics* pGraphics,
                              const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);

  CFX_RectF rtClip(m_ContentRect);
  if (IsShowHorzScrollBar())
    rtClip.height -= m_fScorllBarWidth;
  if (IsShowVertScrollBar())
    rtClip.width -= m_fScorllBarWidth;

  pGraphics->SetClipRect(matrix.TransformRect(rtClip));
  if ((m_Properties.m_dwStyles & FWL_STYLE_WGT_NoBackground) == 0)
    DrawBkground(pGraphics, matrix);

  DrawItems(pGraphics, matrix);
}

int32_t CFWL_ListBox::CountSelItems() {
  int32_t iRet = 0;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (pItem && pItem->IsSelected())
      iRet++;
  }
  return iRet;
}

CFWL_ListBox::Item* CFWL_ListBox::GetSelItem(int32_t nIndexSel) {
  int32_t idx = GetSelIndex(nIndexSel);
  if (idx < 0)
    return nullptr;
  return GetItem(this, idx);
}

int32_t CFWL_ListBox::GetSelIndex(int32_t nIndex) {
  int32_t index = 0;
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem)
      return -1;
    if (pItem->IsSelected()) {
      if (index == nIndex)
        return i;
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
  if (IsMultiSelection())
    pItem->SetSelected(bSelect);
  else
    SetSelection(pItem, pItem, bSelect);
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
  if (iStart > iEnd)
    std::swap(iStart, iEnd);
  if (bSelected) {
    int32_t iCount = CountItems(this);
    for (int32_t i = 0; i < iCount; i++) {
      Item* pItem = GetItem(this, i);
      if (pItem)
        pItem->SetSelected(false);
    }
  }
  while (iStart <= iEnd) {
    Item* pItem = GetItem(this, iStart);
    if (pItem)
      pItem->SetSelected(bSelected);
    ++iStart;
  }
}

bool CFWL_ListBox::IsMultiSelection() const {
  return m_Properties.m_dwStyleExts & FWL_STYLEEXT_LTB_MultiSelection;
}

void CFWL_ListBox::ClearSelection() {
  bool bMulti = IsMultiSelection();
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem)
      continue;
    if (!pItem->IsSelected())
      continue;
    pItem->SetSelected(false);
    if (!bMulti)
      return;
  }
}

void CFWL_ListBox::SelectAll() {
  if (!IsMultiSelection())
    return;

  int32_t iCount = CountItems(this);
  if (iCount <= 0)
    return;

  Item* pItemStart = GetItem(this, 0);
  Item* pItemEnd = GetItem(this, iCount - 1);
  SetSelection(pItemStart, pItemEnd, false);
}

CFWL_ListBox::Item* CFWL_ListBox::GetFocusedItem() {
  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem)
      break;
    if (pItem->IsFocused())
      return pItem;
  }
  return nullptr;
}

void CFWL_ListBox::SetFocusItem(Item* pItem) {
  Item* hFocus = GetFocusedItem();
  if (pItem == hFocus)
    return;

  if (hFocus)
    hFocus->SetFocused(false);
  if (pItem)
    pItem->SetFocused(true);
}

CFWL_ListBox::Item* CFWL_ListBox::GetItemAtPoint(const CFX_PointF& point) {
  CFX_PointF pos = point - m_ContentRect.TopLeft();
  float fPosX = 0.0f;
  if (m_pHorzScrollBar)
    fPosX = m_pHorzScrollBar->GetPos();

  float fPosY = 0.0;
  if (m_pVertScrollBar)
    fPosY = m_pVertScrollBar->GetPos();

  int32_t nCount = CountItems(this);
  for (int32_t i = 0; i < nCount; i++) {
    Item* pItem = GetItem(this, i);
    if (!pItem)
      continue;

    CFX_RectF rtItem = pItem->GetRect();
    rtItem.Offset(-fPosX, -fPosY);
    if (rtItem.Contains(pos))
      return pItem;
  }
  return nullptr;
}

bool CFWL_ListBox::ScrollToVisible(Item* pItem) {
  if (!m_pVertScrollBar)
    return false;

  CFX_RectF rtItem = pItem ? pItem->GetRect() : CFX_RectF();
  bool bScroll = false;
  float fPosY = m_pVertScrollBar->GetPos();
  rtItem.Offset(0, -fPosY + m_ContentRect.top);
  if (rtItem.top < m_ContentRect.top) {
    fPosY += rtItem.top - m_ContentRect.top;
    bScroll = true;
  } else if (rtItem.bottom() > m_ContentRect.bottom()) {
    fPosY += rtItem.bottom() - m_ContentRect.bottom();
    bScroll = true;
  }
  if (!bScroll)
    return false;

  m_pVertScrollBar->SetPos(fPosY);
  m_pVertScrollBar->SetTrackPos(fPosY);
  RepaintRect(m_ClientRect);
  return true;
}

void CFWL_ListBox::DrawBkground(CFGAS_GEGraphics* pGraphics,
                                const CFX_Matrix& mtMatrix) {
  if (!pGraphics)
    return;

  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackground, this,
                             pGraphics);
  param.m_matrix = mtMatrix;
  param.m_PartRect = m_ClientRect;
  if (IsShowHorzScrollBar() && IsShowVertScrollBar())
    param.m_pRtData = &m_StaticRect;
  if (!IsEnabled())
    param.m_dwStates = CFWL_PartState::kDisabled;
  GetThemeProvider()->DrawBackground(param);
}

void CFWL_ListBox::DrawItems(CFGAS_GEGraphics* pGraphics,
                             const CFX_Matrix& mtMatrix) {
  float fPosX = 0.0f;
  if (m_pHorzScrollBar)
    fPosX = m_pHorzScrollBar->GetPos();

  float fPosY = 0.0f;
  if (m_pVertScrollBar)
    fPosY = m_pVertScrollBar->GetPos();

  CFX_RectF rtView(m_ContentRect);
  if (m_pHorzScrollBar)
    rtView.height -= m_fScorllBarWidth;
  if (m_pVertScrollBar)
    rtView.width -= m_fScorllBarWidth;

  int32_t iCount = CountItems(this);
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_ListBox::Item* pItem = GetItem(this, i);
    if (!pItem)
      continue;

    CFX_RectF rtItem = pItem->GetRect();
    rtItem.Offset(m_ContentRect.left - fPosX, m_ContentRect.top - fPosY);
    if (rtItem.bottom() < m_ContentRect.top)
      continue;
    if (rtItem.top >= m_ContentRect.bottom())
      break;
    DrawItem(pGraphics, pItem, i, rtItem, mtMatrix);
  }
}

void CFWL_ListBox::DrawItem(CFGAS_GEGraphics* pGraphics,
                            Item* pItem,
                            int32_t Index,
                            const CFX_RectF& rtItem,
                            const CFX_Matrix& mtMatrix) {
  Mask<CFWL_PartState> dwPartStates = CFWL_PartState::kNormal;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)
    dwPartStates = CFWL_PartState::kDisabled;
  else if (pItem && pItem->IsSelected())
    dwPartStates = CFWL_PartState::kSelected;

  if ((m_Properties.m_dwStates & FWL_STATE_WGT_Focused) && pItem &&
      pItem->IsFocused()) {
    dwPartStates |= CFWL_PartState::kFocused;
  }

  CFX_RectF rtFocus(rtItem);  // Must outlive |bg_param|.
  CFWL_ThemeBackground bg_param(CFWL_ThemePart::Part::kListItem, this,
                                pGraphics);
  bg_param.m_dwStates = dwPartStates;
  bg_param.m_matrix = mtMatrix;
  bg_param.m_PartRect = rtItem;
  bg_param.m_bMaximize = true;
  bg_param.m_pRtData = &rtFocus;
  if (m_pVertScrollBar && !m_pHorzScrollBar &&
      (dwPartStates & CFWL_PartState::kFocused)) {
    bg_param.m_PartRect.left += 1;
    bg_param.m_PartRect.width -= (m_fScorllBarWidth + 1);
    rtFocus.Deflate(0.5, 0.5, 1 + m_fScorllBarWidth, 1);
  }

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  pTheme->DrawBackground(bg_param);
  if (!pItem)
    return;

  WideString wsText = pItem->GetText();
  if (wsText.GetLength() <= 0)
    return;

  CFX_RectF rtText(rtItem);
  rtText.Deflate(kItemTextMargin, kItemTextMargin);

  CFWL_ThemeText textParam(CFWL_ThemePart::Part::kListItem, this, pGraphics);
  textParam.m_dwStates = dwPartStates;
  textParam.m_matrix = mtMatrix;
  textParam.m_PartRect = rtText;
  textParam.m_wsText = std::move(wsText);
  textParam.m_dwTTOStyles = m_TTOStyles;
  textParam.m_iTTOAlign = m_iTTOAligns;
  textParam.m_bMaximize = true;
  pTheme->DrawText(textParam);
}

CFX_SizeF CFWL_ListBox::CalcSize() {
  m_ClientRect = GetClientRect();
  m_ContentRect = m_ClientRect;
  CFX_RectF rtUIMargin;
  if (!GetOuter()) {
    CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
    CFX_RectF pUIMargin = GetThemeProvider()->GetUIMargin(part);
    m_ContentRect.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                          pUIMargin.height);
  }

  float fWidth = GetMaxTextWidth();
  fWidth += 2 * kItemTextMargin;

  float fActualWidth = m_ClientRect.width - rtUIMargin.left - rtUIMargin.width;
  fWidth = std::max(fWidth, fActualWidth);
  m_fItemHeight = CalcItemHeight();

  int32_t iCount = CountItems(this);
  CFX_SizeF fs;
  for (int32_t i = 0; i < iCount; i++) {
    Item* htem = GetItem(this, i);
    UpdateItemSize(htem, fs, fWidth, m_fItemHeight);
  }

  float iHeight = m_ClientRect.height;
  bool bShowVertScr = false;
  bool bShowHorzScr = false;
  if (!bShowVertScr && (m_Properties.m_dwStyles & FWL_STYLE_WGT_VScroll))
    bShowVertScr = (fs.height > iHeight);

  float fMax = 0.0f;
  if (bShowVertScr) {
    if (!m_pVertScrollBar)
      InitVerticalScrollBar();

    CFX_RectF rtScrollBar(m_ClientRect.right() - m_fScorllBarWidth,
                          m_ClientRect.top, m_fScorllBarWidth,
                          m_ClientRect.height - 1);
    if (bShowHorzScr)
      rtScrollBar.height -= m_fScorllBarWidth;

    m_pVertScrollBar->SetWidgetRect(rtScrollBar);
    fMax = std::max(fs.height - m_ContentRect.height, m_fItemHeight);

    m_pVertScrollBar->SetRange(0.0f, fMax);
    m_pVertScrollBar->SetPageSize(rtScrollBar.height * 9 / 10);
    m_pVertScrollBar->SetStepSize(m_fItemHeight);

    float fPos = std::clamp(m_pVertScrollBar->GetPos(), 0.0f, fMax);
    m_pVertScrollBar->SetPos(fPos);
    m_pVertScrollBar->SetTrackPos(fPos);
    if ((m_Properties.m_dwStyleExts & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ==
            0 ||
        (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)) {
      m_pVertScrollBar->RemoveStates(FWL_STATE_WGT_Invisible);
    }
    m_pVertScrollBar->Update();
  } else if (m_pVertScrollBar) {
    m_pVertScrollBar->SetPos(0);
    m_pVertScrollBar->SetTrackPos(0);
    m_pVertScrollBar->SetStates(FWL_STATE_WGT_Invisible);
  }
  if (bShowHorzScr) {
    if (!m_pHorzScrollBar)
      InitHorizontalScrollBar();

    CFX_RectF rtScrollBar(m_ClientRect.left,
                          m_ClientRect.bottom() - m_fScorllBarWidth,
                          m_ClientRect.width, m_fScorllBarWidth);
    if (bShowVertScr)
      rtScrollBar.width -= m_fScorllBarWidth;

    m_pHorzScrollBar->SetWidgetRect(rtScrollBar);
    fMax = fs.width - rtScrollBar.width;
    m_pHorzScrollBar->SetRange(0.0f, fMax);
    m_pHorzScrollBar->SetPageSize(fWidth * 9 / 10);
    m_pHorzScrollBar->SetStepSize(fWidth / 10);

    float fPos = std::clamp(m_pHorzScrollBar->GetPos(), 0.0f, fMax);
    m_pHorzScrollBar->SetPos(fPos);
    m_pHorzScrollBar->SetTrackPos(fPos);
    if ((m_Properties.m_dwStyleExts & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ==
            0 ||
        (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)) {
      m_pHorzScrollBar->RemoveStates(FWL_STATE_WGT_Invisible);
    }
    m_pHorzScrollBar->Update();
  } else if (m_pHorzScrollBar) {
    m_pHorzScrollBar->SetPos(0);
    m_pHorzScrollBar->SetTrackPos(0);
    m_pHorzScrollBar->SetStates(FWL_STATE_WGT_Invisible);
  }
  if (bShowVertScr && bShowHorzScr) {
    m_StaticRect = CFX_RectF(m_ClientRect.right() - m_fScorllBarWidth,
                             m_ClientRect.bottom() - m_fScorllBarWidth,
                             m_fScorllBarWidth, m_fScorllBarWidth);
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
    if (!pItem)
      continue;

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
  if (m_pVertScrollBar)
    return;

  m_pVertScrollBar = cppgc::MakeGarbageCollected<CFWL_ScrollBar>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      Properties{0, FWL_STYLEEXT_SCB_Vert, FWL_STATE_WGT_Invisible}, this);
}

void CFWL_ListBox::InitHorizontalScrollBar() {
  if (m_pHorzScrollBar)
    return;

  m_pHorzScrollBar = cppgc::MakeGarbageCollected<CFWL_ScrollBar>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      Properties{0, FWL_STYLEEXT_SCB_Horz, FWL_STATE_WGT_Invisible}, this);
}

bool CFWL_ListBox::IsShowVertScrollBar() const {
  return m_pVertScrollBar && m_pVertScrollBar->IsVisible() &&
         ScrollBarPropertiesPresent();
}

bool CFWL_ListBox::IsShowHorzScrollBar() const {
  return m_pHorzScrollBar && m_pHorzScrollBar->IsVisible() &&
         ScrollBarPropertiesPresent();
}

bool CFWL_ListBox::ScrollBarPropertiesPresent() const {
  return !(m_Properties.m_dwStyleExts & FWL_STYLEEXT_LTB_ShowScrollBarFocus) ||
         (m_Properties.m_dwStates & FWL_STATE_WGT_Focused);
}

void CFWL_ListBox::OnProcessMessage(CFWL_Message* pMessage) {
  if (!IsEnabled())
    return;

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
      OnFocusGained();
      break;
    case CFWL_Message::Type::kKillFocus:
      OnFocusLost();
      break;
    case CFWL_Message::Type::kMouse: {
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
    case CFWL_Message::Type::kMouseWheel:
      OnMouseWheel(static_cast<CFWL_MessageMouseWheel*>(pMessage));
      break;
    case CFWL_Message::Type::kKey: {
      CFWL_MessageKey* pMsg = static_cast<CFWL_MessageKey*>(pMessage);
      if (pMsg->m_dwCmd == CFWL_MessageKey::KeyCommand::kKeyDown)
        OnKeyDown(pMsg);
      break;
    }
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget())
    CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_ListBox::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent)
    return;
  if (pEvent->GetType() != CFWL_Event::Type::Scroll)
    return;

  CFWL_Widget* pSrcTarget = pEvent->GetSrcTarget();
  if ((pSrcTarget == m_pVertScrollBar && m_pVertScrollBar) ||
      (pSrcTarget == m_pHorzScrollBar && m_pHorzScrollBar)) {
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
    if (m_pVertScrollBar)
      m_pVertScrollBar->RemoveStates(FWL_STATE_WGT_Invisible);
    if (m_pHorzScrollBar)
      m_pHorzScrollBar->RemoveStates(FWL_STATE_WGT_Invisible);
  }
  m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
  RepaintRect(m_ClientRect);
}

void CFWL_ListBox::OnFocusLost() {
  if (GetStyleExts() & FWL_STYLEEXT_LTB_ShowScrollBarFocus) {
    if (m_pVertScrollBar)
      m_pVertScrollBar->SetStates(FWL_STATE_WGT_Invisible);
    if (m_pHorzScrollBar)
      m_pHorzScrollBar->SetStates(FWL_STATE_WGT_Invisible);
  }
  m_Properties.m_dwStates &= ~FWL_STATE_WGT_Focused;
  RepaintRect(m_ClientRect);
}

void CFWL_ListBox::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  m_bLButtonDown = true;

  Item* pItem = GetItemAtPoint(pMsg->m_pos);
  if (!pItem)
    return;

  if (IsMultiSelection()) {
    if (pMsg->m_dwFlags & XFA_FWL_KeyFlag::kCtrl) {
      pItem->SetSelected(!pItem->IsSelected());
      m_hAnchor = pItem;
    } else if (pMsg->m_dwFlags & XFA_FWL_KeyFlag::kShift) {
      if (m_hAnchor)
        SetSelection(m_hAnchor, pItem, true);
      else
        pItem->SetSelected(true);
    } else {
      SetSelection(pItem, pItem, true);
      m_hAnchor = pItem;
    }
  } else {
    SetSelection(pItem, pItem, true);
  }

  SetFocusItem(pItem);
  ScrollToVisible(pItem);
  SetGrab(true);
  RepaintRect(m_ClientRect);
}

void CFWL_ListBox::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (!m_bLButtonDown)
    return;

  m_bLButtonDown = false;
  SetGrab(false);
}

void CFWL_ListBox::OnMouseWheel(CFWL_MessageMouseWheel* pMsg) {
  if (IsShowVertScrollBar())
    m_pVertScrollBar->GetDelegate()->OnProcessMessage(pMsg);
}

void CFWL_ListBox::OnKeyDown(CFWL_MessageKey* pMsg) {
  auto dwKeyCode = static_cast<XFA_FWL_VKEYCODE>(pMsg->m_dwKeyCodeOrChar);
  switch (dwKeyCode) {
    case XFA_FWL_VKEY_Tab:
    case XFA_FWL_VKEY_Up:
    case XFA_FWL_VKEY_Down:
    case XFA_FWL_VKEY_Home:
    case XFA_FWL_VKEY_End: {
      Item* pItem = GetListItem(GetFocusedItem(), dwKeyCode);
      bool bShift = !!(pMsg->m_dwFlags & XFA_FWL_KeyFlag::kShift);
      bool bCtrl = !!(pMsg->m_dwFlags & XFA_FWL_KeyFlag::kCtrl);
      OnVK(pItem, bShift, bCtrl);
      break;
    }
    default:
      break;
  }
}

void CFWL_ListBox::OnVK(Item* pItem, bool bShift, bool bCtrl) {
  if (!pItem)
    return;

  if (IsMultiSelection()) {
    if (bCtrl) {
      // Do nothing.
    } else if (bShift) {
      if (m_hAnchor)
        SetSelection(m_hAnchor, pItem, true);
      else
        pItem->SetSelected(true);
    } else {
      SetSelection(pItem, pItem, true);
      m_hAnchor = pItem;
    }
  } else {
    SetSelection(pItem, pItem, true);
  }

  SetFocusItem(pItem);
  ScrollToVisible(pItem);
  RepaintRect(CFX_RectF(0, 0, m_WidgetRect.width, m_WidgetRect.height));
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
      if (fPos < fMin + fStep / 2)
        fPos = fMin;
      break;
    }
    case CFWL_EventScroll::Code::StepForward: {
      fPos += fStep;
      if (fPos > fMax - fStep / 2)
        fPos = fMax;
      break;
    }
    case CFWL_EventScroll::Code::PageBackward: {
      fPos -= pScrollBar->GetPageSize();
      if (fPos < fMin)
        fPos = fMin;
      break;
    }
    case CFWL_EventScroll::Code::PageForward: {
      fPos += pScrollBar->GetPageSize();
      if (fPos > fMax)
        fPos = fMax;
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
    RepaintRect(m_ClientRect);
  }
  return true;
}

int32_t CFWL_ListBox::CountItems(const CFWL_Widget* pWidget) const {
  return fxcrt::CollectionSize<int32_t>(m_ItemArray);
}

CFWL_ListBox::Item* CFWL_ListBox::GetItem(const CFWL_Widget* pWidget,
                                          int32_t nIndex) const {
  if (nIndex < 0 || nIndex >= CountItems(pWidget))
    return nullptr;
  return m_ItemArray[nIndex].get();
}

int32_t CFWL_ListBox::GetItemIndex(CFWL_Widget* pWidget, Item* pItem) {
  auto it = std::find_if(m_ItemArray.begin(), m_ItemArray.end(),
                         [pItem](const std::unique_ptr<Item>& candidate) {
                           return candidate.get() == pItem;
                         });
  return it != m_ItemArray.end()
             ? pdfium::base::checked_cast<int32_t>(it - m_ItemArray.begin())
             : -1;
}

CFWL_ListBox::Item* CFWL_ListBox::AddString(const WideString& wsAdd) {
  m_ItemArray.push_back(std::make_unique<Item>(wsAdd));
  return m_ItemArray.back().get();
}

void CFWL_ListBox::RemoveAt(int32_t iIndex) {
  if (iIndex < 0 || static_cast<size_t>(iIndex) >= m_ItemArray.size())
    return;
  m_ItemArray.erase(m_ItemArray.begin() + iIndex);
}

void CFWL_ListBox::DeleteString(Item* pItem) {
  int32_t nIndex = GetItemIndex(this, pItem);
  if (nIndex < 0 || static_cast<size_t>(nIndex) >= m_ItemArray.size())
    return;

  int32_t iSel = nIndex + 1;
  if (iSel >= CountItems(this))
    iSel = nIndex - 1;
  if (iSel >= 0) {
    Item* item = GetItem(this, iSel);
    if (item)
      item->SetSelected(true);
  }
  m_ItemArray.erase(m_ItemArray.begin() + nIndex);
}

void CFWL_ListBox::DeleteAll() {
  m_ItemArray.clear();
}

CFWL_ListBox::Item::Item(const WideString& text) : m_wsText(text) {}

CFWL_ListBox::Item::~Item() = default;
