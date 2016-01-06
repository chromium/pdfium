// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfwindow/PWL_IconList.h"

#include "fpdfsdk/include/pdfwindow/PWL_Label.h"
#include "fpdfsdk/include/pdfwindow/PWL_ListCtrl.h"
#include "fpdfsdk/include/pdfwindow/PWL_ScrollBar.h"
#include "fpdfsdk/include/pdfwindow/PWL_Utils.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"
#include "public/fpdf_fwlevent.h"

#define PWL_IconList_ITEM_ICON_LEFTMARGIN 10.0f
#define PWL_IconList_ITEM_WIDTH 20.0f
#define PWL_IconList_ITEM_HEIGHT 20.0f
#define PWL_IconList_ITEM_SPACE 4.0f

CPWL_IconList_Item::CPWL_IconList_Item()
    : m_nIconIndex(-1), m_pData(NULL), m_bSelected(FALSE), m_pText(NULL) {}

CPWL_IconList_Item::~CPWL_IconList_Item() {}

CFX_ByteString CPWL_IconList_Item::GetClassName() const {
  return "CPWL_IconList_Item";
}

FX_FLOAT CPWL_IconList_Item::GetItemHeight(FX_FLOAT fLimitWidth) {
  return PWL_IconList_ITEM_HEIGHT;
}

void CPWL_IconList_Item::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                            CFX_Matrix* pUser2Device) {
  CPDF_Rect rcClient = GetClientRect();

  if (m_bSelected) {
    if (IsEnabled()) {
      CPWL_Utils::DrawFillRect(
          pDevice, pUser2Device, rcClient,
          CPWL_Utils::PWLColorToFXColor(PWL_DEFAULT_SELBACKCOLOR,
                                        GetTransparency()));
    } else {
      CPWL_Utils::DrawFillRect(
          pDevice, pUser2Device, rcClient,
          CPWL_Utils::PWLColorToFXColor(PWL_DEFAULT_LIGHTGRAYCOLOR,
                                        GetTransparency()));
    }
  }

  CPDF_Rect rcIcon = rcClient;
  rcIcon.left += PWL_IconList_ITEM_ICON_LEFTMARGIN;
  rcIcon.right = rcIcon.left + PWL_IconList_ITEM_WIDTH;

  CPWL_Utils::DrawIconAppStream(pDevice, pUser2Device, m_nIconIndex, rcIcon,
                                m_crIcon, m_pText->GetTextColor(),
                                GetTransparency());
}

void CPWL_IconList_Item::SetSelect(FX_BOOL bSelected) {
  m_bSelected = bSelected;

  if (bSelected)
    m_pText->SetTextColor(PWL_DEFAULT_WHITECOLOR);
  else
    m_pText->SetTextColor(PWL_DEFAULT_BLACKCOLOR);
}

FX_BOOL CPWL_IconList_Item::IsSelected() const {
  return m_bSelected;
}

void CPWL_IconList_Item::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  m_pText = new CPWL_Label;

  PWL_CREATEPARAM lcp = cp;
  lcp.pParentWnd = this;
  lcp.dwFlags = PWS_CHILD | PWS_VISIBLE | PES_LEFT | PES_CENTER;
  lcp.sTextColor = PWL_DEFAULT_BLACKCOLOR;
  lcp.fFontSize = 12;
  m_pText->Create(lcp);
}

void CPWL_IconList_Item::SetData(void* pData) {
  m_pData = pData;
}

void CPWL_IconList_Item::SetIcon(int32_t nIconIndex) {
  m_nIconIndex = nIconIndex;
}

void CPWL_IconList_Item::SetText(const CFX_WideString& str) {
  m_pText->SetText(str.c_str());
}

CFX_WideString CPWL_IconList_Item::GetText() const {
  return m_pText->GetText();
}

void CPWL_IconList_Item::RePosChildWnd() {
  CPDF_Rect rcClient = GetClientRect();

  rcClient.left +=
      (PWL_IconList_ITEM_ICON_LEFTMARGIN + PWL_IconList_ITEM_WIDTH +
       PWL_IconList_ITEM_ICON_LEFTMARGIN);

  m_pText->Move(rcClient, TRUE, FALSE);
}

void CPWL_IconList_Item::SetIconFillColor(const CPWL_Color& color) {
  m_crIcon = color;
}

void CPWL_IconList_Item::OnEnabled() {
  if (m_bSelected)
    m_pText->SetTextColor(PWL_DEFAULT_WHITECOLOR);
  else
    m_pText->SetTextColor(PWL_DEFAULT_BLACKCOLOR);

  InvalidateRect();
}

void CPWL_IconList_Item::OnDisabled() {
  m_pText->SetTextColor(PWL_DEFAULT_HEAVYGRAYCOLOR);

  InvalidateRect();
}

CPWL_IconList_Content::CPWL_IconList_Content(int32_t nListCount)
    : m_nSelectIndex(-1),
      m_pNotify(NULL),
      m_bEnableNotify(TRUE),
      m_bMouseDown(FALSE),
      m_nListCount(nListCount) {}

CPWL_IconList_Content::~CPWL_IconList_Content() {}

void CPWL_IconList_Content::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  for (int32_t i = 0; i < m_nListCount; i++) {
    CPWL_IconList_Item* pNewItem = new CPWL_IconList_Item();

    PWL_CREATEPARAM icp = cp;
    icp.pParentWnd = this;
    icp.dwFlags = PWS_CHILD | PWS_VISIBLE | PWS_NOREFRESHCLIP;
    pNewItem->Create(icp);
  }

  SetItemSpace(PWL_IconList_ITEM_SPACE);
  ResetContent(0);

  if (CPWL_Wnd* pParent = GetParentWindow()) {
    CPDF_Rect rcScroll = GetScrollArea();
    GetScrollPos();

    PWL_SCROLL_INFO sInfo;
    sInfo.fContentMin = rcScroll.bottom;
    sInfo.fContentMax = rcScroll.top;
    sInfo.fPlateWidth = GetClientRect().Height();
    sInfo.fSmallStep = 13.0f;
    sInfo.fBigStep = sInfo.fPlateWidth;

    pParent->OnNotify(this, PNM_SETSCROLLINFO, SBT_VSCROLL, (intptr_t)&sInfo);
  }
}

FX_BOOL CPWL_IconList_Content::OnLButtonDown(const CPDF_Point& point,
                                             FX_DWORD nFlag) {
  SetFocus();

  SetCapture();
  m_bMouseDown = TRUE;

  int32_t nItemIndex = FindItemIndex(point);
  SetSelect(nItemIndex);
  ScrollToItem(nItemIndex);

  return TRUE;
}

FX_BOOL CPWL_IconList_Content::OnLButtonUp(const CPDF_Point& point,
                                           FX_DWORD nFlag) {
  m_bMouseDown = FALSE;
  ReleaseCapture();

  return TRUE;
}

FX_BOOL CPWL_IconList_Content::OnMouseMove(const CPDF_Point& point,
                                           FX_DWORD nFlag) {
  if (m_bMouseDown) {
    int32_t nItemIndex = FindItemIndex(point);
    SetSelect(nItemIndex);
    ScrollToItem(nItemIndex);
  }

  return TRUE;
}

FX_BOOL CPWL_IconList_Content::OnKeyDown(FX_WORD nChar, FX_DWORD nFlag) {
  switch (nChar) {
    case FWL_VKEY_Up:
      if (m_nSelectIndex > 0) {
        int32_t nItemIndex = m_nSelectIndex - 1;
        SetSelect(nItemIndex);
        ScrollToItem(nItemIndex);
      }
      return TRUE;
    case FWL_VKEY_Down:
      if (m_nSelectIndex < m_nListCount - 1) {
        int32_t nItemIndex = m_nSelectIndex + 1;
        SetSelect(nItemIndex);
        ScrollToItem(nItemIndex);
      }
      return TRUE;
  }

  return FALSE;
}

int32_t CPWL_IconList_Content::FindItemIndex(const CPDF_Point& point) {
  int32_t nIndex = 0;
  for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
      CPDF_Rect rcWnd = pChild->ChildToParent(pChild->GetWindowRect());

      if (point.y < rcWnd.top) {
        nIndex = i;
      }
    }
  }

  return nIndex;
}

void CPWL_IconList_Content::ScrollToItem(int32_t nItemIndex) {
  CPDF_Rect rcClient = GetClientRect();

  if (CPWL_IconList_Item* pItem = GetListItem(nItemIndex)) {
    CPDF_Rect rcOrigin = pItem->GetWindowRect();
    CPDF_Rect rcWnd = pItem->ChildToParent(rcOrigin);

    if (!(rcWnd.bottom > rcClient.bottom && rcWnd.top < rcClient.top)) {
      CPDF_Point ptScroll = GetScrollPos();

      if (rcWnd.top > rcClient.top) {
        ptScroll.y = rcOrigin.top;
      } else if (rcWnd.bottom < rcClient.bottom) {
        ptScroll.y = rcOrigin.bottom + rcClient.Height();
      }

      SetScrollPos(ptScroll);
      ResetFace();
      InvalidateRect();
      if (CPWL_Wnd* pParent = GetParentWindow()) {
        pParent->OnNotify(this, PNM_SETSCROLLPOS, SBT_VSCROLL,
                          (intptr_t)&ptScroll.y);
      }
    }
  }
}

void CPWL_IconList_Content::SetSelect(int32_t nIndex) {
  if (m_nSelectIndex != nIndex) {
    SelectItem(m_nSelectIndex, FALSE);
    SelectItem(nIndex, TRUE);
    m_nSelectIndex = nIndex;

    if (IPWL_IconList_Notify* pNotify = GetNotify())
      pNotify->OnNoteListSelChanged(nIndex);
  }
}

int32_t CPWL_IconList_Content::GetSelect() const {
  return m_nSelectIndex;
}

IPWL_IconList_Notify* CPWL_IconList_Content::GetNotify() const {
  if (m_bEnableNotify)
    return m_pNotify;
  return NULL;
}

void CPWL_IconList_Content::SetNotify(IPWL_IconList_Notify* pNotify) {
  m_pNotify = pNotify;
}

void CPWL_IconList_Content::EnableNotify(FX_BOOL bNotify) {
  m_bEnableNotify = bNotify;
}

void CPWL_IconList_Content::SelectItem(int32_t nItemIndex, FX_BOOL bSelect) {
  if (CPWL_IconList_Item* pItem = GetListItem(nItemIndex)) {
    pItem->SetSelect(bSelect);
    pItem->InvalidateRect();
  }
}

CPWL_IconList_Item* CPWL_IconList_Content::GetListItem(
    int32_t nItemIndex) const {
  if (nItemIndex >= 0 && nItemIndex < m_aChildren.GetSize()) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(nItemIndex)) {
      if (pChild->GetClassName() == "CPWL_IconList_Item") {
        return (CPWL_IconList_Item*)pChild;
      }
    }
  }

  return NULL;
}

void CPWL_IconList_Content::SetListData(int32_t nItemIndex, void* pData) {
  if (CPWL_IconList_Item* pItem = GetListItem(nItemIndex))
    pItem->SetData(pData);
}

void CPWL_IconList_Content::SetListIcon(int32_t nItemIndex,
                                        int32_t nIconIndex) {
  if (CPWL_IconList_Item* pItem = GetListItem(nItemIndex))
    pItem->SetIcon(nIconIndex);
}

void CPWL_IconList_Content::SetListString(int32_t nItemIndex,
                                          const CFX_WideString& str) {
  if (CPWL_IconList_Item* pItem = GetListItem(nItemIndex))
    pItem->SetText(str);
}

CFX_WideString CPWL_IconList_Content::GetListString(int32_t nItemIndex) const {
  if (CPWL_IconList_Item* pItem = GetListItem(nItemIndex))
    return pItem->GetText();

  return L"";
}

void CPWL_IconList_Content::SetIconFillColor(const CPWL_Color& color) {
  for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
      if (pChild->GetClassName() == "CPWL_IconList_Item") {
        CPWL_IconList_Item* pItem = (CPWL_IconList_Item*)pChild;
        pItem->SetIconFillColor(color);
        pItem->InvalidateRect();
      }
    }
  }
}

CPWL_IconList::CPWL_IconList(int32_t nListCount)
    : m_pListContent(NULL), m_nListCount(nListCount) {}

CPWL_IconList::~CPWL_IconList() {}

void CPWL_IconList::RePosChildWnd() {
  CPWL_Wnd::RePosChildWnd();

  if (m_pListContent)
    m_pListContent->Move(GetClientRect(), TRUE, FALSE);
}

void CPWL_IconList::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  m_pListContent = new CPWL_IconList_Content(m_nListCount);

  PWL_CREATEPARAM ccp = cp;
  ccp.pParentWnd = this;
  ccp.dwFlags = PWS_CHILD | PWS_VISIBLE;
  m_pListContent->Create(ccp);
}

void CPWL_IconList::OnCreated() {
  if (CPWL_ScrollBar* pScrollBar = GetVScrollBar()) {
    pScrollBar->RemoveFlag(PWS_AUTOTRANSPARENT);
    pScrollBar->SetTransparency(255);
    pScrollBar->SetNotifyForever(TRUE);
  }
}

void CPWL_IconList::OnNotify(CPWL_Wnd* pWnd,
                             FX_DWORD msg,
                             intptr_t wParam,
                             intptr_t lParam) {
  CPWL_Wnd::OnNotify(pWnd, msg, wParam, lParam);

  if (wParam == SBT_VSCROLL) {
    switch (msg) {
      case PNM_SETSCROLLINFO:
        if (PWL_SCROLL_INFO* pInfo = (PWL_SCROLL_INFO*)lParam) {
          if (CPWL_ScrollBar* pScrollBar = GetVScrollBar()) {
            if (pInfo->fContentMax - pInfo->fContentMin > pInfo->fPlateWidth) {
              if (!pScrollBar->IsVisible()) {
                pScrollBar->SetVisible(TRUE);
                RePosChildWnd();
              } else {
              }
            } else {
              if (pScrollBar->IsVisible()) {
                pScrollBar->SetVisible(FALSE);
                RePosChildWnd();
              }

              if (m_pListContent)
                m_pListContent->SetScrollPos(CPDF_Point(0.0f, 0.0f));
            }

            pScrollBar->OnNotify(pWnd, PNM_SETSCROLLINFO, wParam, lParam);
          }
        }
        return;
      case PNM_SCROLLWINDOW:
        if (m_pListContent) {
          m_pListContent->SetScrollPos(CPDF_Point(0.0f, *(FX_FLOAT*)lParam));
          m_pListContent->ResetFace();
          m_pListContent->InvalidateRect(NULL);
        }
        return;
      case PNM_SETSCROLLPOS:
        if (CPWL_ScrollBar* pScrollBar = GetVScrollBar())
          pScrollBar->OnNotify(pWnd, PNM_SETSCROLLPOS, wParam, lParam);
        return;
    }
  }
}

void CPWL_IconList::SetSelect(int32_t nIndex) {
  m_pListContent->SetSelect(nIndex);
}

void CPWL_IconList::SetTopItem(int32_t nIndex) {
  m_pListContent->ScrollToItem(nIndex);
}

int32_t CPWL_IconList::GetSelect() const {
  return m_pListContent->GetSelect();
}

void CPWL_IconList::SetNotify(IPWL_IconList_Notify* pNotify) {
  m_pListContent->SetNotify(pNotify);
}

void CPWL_IconList::EnableNotify(FX_BOOL bNotify) {
  m_pListContent->EnableNotify(bNotify);
}

void CPWL_IconList::SetListData(int32_t nItemIndex, void* pData) {
  m_pListContent->SetListData(nItemIndex, pData);
}

void CPWL_IconList::SetListIcon(int32_t nItemIndex, int32_t nIconIndex) {
  m_pListContent->SetListIcon(nItemIndex, nIconIndex);
}

void CPWL_IconList::SetListString(int32_t nItemIndex,
                                  const CFX_WideString& str) {
  m_pListContent->SetListString(nItemIndex, str);
}

CFX_WideString CPWL_IconList::GetListString(int32_t nItemIndex) const {
  return m_pListContent->GetListString(nItemIndex);
}

void CPWL_IconList::SetIconFillColor(const CPWL_Color& color) {
  m_pListContent->SetIconFillColor(color);
}

FX_BOOL CPWL_IconList::OnMouseWheel(short zDelta,
                                    const CPDF_Point& point,
                                    FX_DWORD nFlag) {
  CPDF_Point ptScroll = m_pListContent->GetScrollPos();
  CPDF_Rect rcScroll = m_pListContent->GetScrollArea();
  CPDF_Rect rcContents = m_pListContent->GetClientRect();

  if (rcScroll.top - rcScroll.bottom > rcContents.Height()) {
    CPDF_Point ptNew = ptScroll;

    if (zDelta > 0)
      ptNew.y += 30;
    else
      ptNew.y -= 30;

    if (ptNew.y > rcScroll.top)
      ptNew.y = rcScroll.top;
    if (ptNew.y < rcScroll.bottom + rcContents.Height())
      ptNew.y = rcScroll.bottom + rcContents.Height();
    if (ptNew.y < rcScroll.bottom)
      ptNew.y = rcScroll.bottom;

    if (ptNew.y != ptScroll.y) {
      m_pListContent->SetScrollPos(ptNew);
      m_pListContent->ResetFace();
      m_pListContent->InvalidateRect(NULL);

      if (CPWL_ScrollBar* pScrollBar = GetVScrollBar())
        pScrollBar->OnNotify(this, PNM_SETSCROLLPOS, SBT_VSCROLL,
                             (intptr_t)&ptNew.y);

      return TRUE;
    }
  }

  return FALSE;
}
