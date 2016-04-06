// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/PWL_Note.h"

#include "core/fxge/include/fx_ge.h"
#include "fpdfsdk/pdfwindow/PWL_Button.h"
#include "fpdfsdk/pdfwindow/PWL_Caret.h"
#include "fpdfsdk/pdfwindow/PWL_Edit.h"
#include "fpdfsdk/pdfwindow/PWL_EditCtrl.h"
#include "fpdfsdk/pdfwindow/PWL_Label.h"
#include "fpdfsdk/pdfwindow/PWL_ListCtrl.h"
#include "fpdfsdk/pdfwindow/PWL_ScrollBar.h"
#include "fpdfsdk/pdfwindow/PWL_Utils.h"
#include "fpdfsdk/pdfwindow/PWL_Wnd.h"

#define POPUP_ITEM_HEAD_BOTTOM 3.0f
#define POPUP_ITEM_BOTTOMWIDTH 1.0f
#define POPUP_ITEM_SIDEMARGIN 3.0f
#define POPUP_ITEM_SPACE 4.0f
#define POPUP_ITEM_TEXT_INDENT 2.0f
#define POPUP_ITEM_BORDERCOLOR \
  CPWL_Color(COLORTYPE_RGB, 80 / 255.0f, 80 / 255.0f, 80 / 255.0f)

#define IsFloatZero(f) ((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa, fb) ((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa, fb) ((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa, fb) IsFloatZero((fa) - (fb))

CPWL_Note_Options::CPWL_Note_Options() : m_pText(NULL) {}

CPWL_Note_Options::~CPWL_Note_Options() {}

void CPWL_Note_Options::SetTextColor(const CPWL_Color& color) {
  CPWL_Wnd::SetTextColor(color);

  if (m_pText)
    m_pText->SetTextColor(color);
}

void CPWL_Note_Options::RePosChildWnd() {
  if (IsValid()) {
    CFX_FloatRect rcClient = GetClientRect();

    if (rcClient.Width() > 15.0f) {
      rcClient.right -= 15.0f;
      m_pText->Move(rcClient, TRUE, FALSE);
      m_pText->SetVisible(TRUE);
    } else {
      m_pText->Move(CFX_FloatRect(0, 0, 0, 0), TRUE, FALSE);
      m_pText->SetVisible(FALSE);
    }
  }
}

void CPWL_Note_Options::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  m_pText = new CPWL_Label;
  PWL_CREATEPARAM tcp = cp;
  tcp.pParentWnd = this;
  tcp.dwFlags = PWS_CHILD | PWS_VISIBLE;
  m_pText->Create(tcp);
}

void CPWL_Note_Options::SetText(const CFX_WideString& sText) {
  m_pText->SetText(sText.c_str());
}

void CPWL_Note_Options::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                           CFX_Matrix* pUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

  CFX_FloatRect rcClient = GetClientRect();
  rcClient.left = rcClient.right - 15.0f;

  CFX_FloatPoint ptCenter =
      CFX_FloatPoint((rcClient.left + rcClient.right) * 0.5f,
                     (rcClient.top + rcClient.bottom) * 0.5f);

  CFX_FloatPoint pt1(ptCenter.x - 2.0f, ptCenter.y + 2.0f * 0.5f);
  CFX_FloatPoint pt2(ptCenter.x + 2.0f, ptCenter.y + 2.0f * 0.5f);
  CFX_FloatPoint pt3(ptCenter.x, ptCenter.y - 3.0f * 0.5f);

  CFX_PathData path;

  path.SetPointCount(4);
  path.SetPoint(0, pt1.x, pt1.y, FXPT_MOVETO);
  path.SetPoint(1, pt2.x, pt2.y, FXPT_LINETO);
  path.SetPoint(2, pt3.x, pt3.y, FXPT_LINETO);
  path.SetPoint(3, pt1.x, pt1.y, FXPT_LINETO);

  pDevice->DrawPath(
      &path, pUser2Device, NULL,
      CPWL_Utils::PWLColorToFXColor(GetTextColor(), GetTransparency()), 0,
      FXFILL_ALTERNATE);
}

CFX_FloatRect CPWL_Note_Options::GetContentRect() const {
  CFX_FloatRect rcText = m_pText->GetContentRect();
  rcText.right += 15.0f;
  return rcText;
}

CPWL_Note_Edit::CPWL_Note_Edit()
    : m_bEnableNotify(TRUE),
      m_fOldItemHeight(0.0f),
      m_bSizeChanged(FALSE),
      m_fOldMin(0.0f),
      m_fOldMax(0.0f) {}

CPWL_Note_Edit::~CPWL_Note_Edit() {}

void CPWL_Note_Edit::RePosChildWnd() {
  m_bEnableNotify = FALSE;
  CPWL_Edit::RePosChildWnd();
  m_bEnableNotify = TRUE;

  m_fOldItemHeight = GetContentRect().Height();
}

void CPWL_Note_Edit::SetText(const FX_WCHAR* csText) {
  m_bEnableNotify = FALSE;
  CPWL_Edit::SetText(csText);
  m_bEnableNotify = TRUE;
  m_fOldItemHeight = GetContentRect().Height();
}

void CPWL_Note_Edit::OnSetFocus() {
  m_bEnableNotify = FALSE;
  CPWL_Edit::OnSetFocus();
  m_bEnableNotify = TRUE;

  EnableSpellCheck(TRUE);
}

void CPWL_Note_Edit::OnKillFocus() {
  EnableSpellCheck(FALSE);
  CPWL_Edit::OnKillFocus();
}

void CPWL_Note_Edit::OnNotify(CPWL_Wnd* pWnd,
                              uint32_t msg,
                              intptr_t wParam,
                              intptr_t lParam) {
  if (m_bEnableNotify) {
    if (wParam == SBT_VSCROLL) {
      switch (msg) {
        case PNM_SETSCROLLINFO:
          if (PWL_SCROLL_INFO* pInfo = (PWL_SCROLL_INFO*)lParam) {
            if (!IsFloatEqual(pInfo->fContentMax, m_fOldMax) ||
                !IsFloatEqual(pInfo->fContentMin, m_fOldMin)) {
              m_bSizeChanged = TRUE;
              if (CPWL_Wnd* pParent = GetParentWindow()) {
                pParent->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
              }

              m_fOldMax = pInfo->fContentMax;
              m_fOldMin = pInfo->fContentMin;
              return;
            }
          }
      }
    }
  }

  CPWL_Edit::OnNotify(pWnd, msg, wParam, lParam);

  if (m_bEnableNotify) {
    switch (msg) {
      case PNM_SETCARETINFO:
        if (PWL_CARET_INFO* pInfo = (PWL_CARET_INFO*)wParam) {
          PWL_CARET_INFO newInfo = *pInfo;
          newInfo.bVisible = TRUE;
          newInfo.ptHead = ChildToParent(pInfo->ptHead);
          newInfo.ptFoot = ChildToParent(pInfo->ptFoot);

          if (CPWL_Wnd* pParent = GetParentWindow()) {
            pParent->OnNotify(this, PNM_SETCARETINFO, (intptr_t)&newInfo, 0);
          }
        }
        break;
    }
  }
}

FX_FLOAT CPWL_Note_Edit::GetItemHeight(FX_FLOAT fLimitWidth) {
  if (fLimitWidth > 0) {
    if (!m_bSizeChanged)
      return m_fOldItemHeight;

    m_bSizeChanged = FALSE;

    EnableNotify(FALSE);
    EnableRefresh(FALSE);
    m_pEdit->EnableNotify(FALSE);

    Move(CFX_FloatRect(0, 0, fLimitWidth, 0), TRUE, FALSE);
    FX_FLOAT fRet = GetContentRect().Height();

    m_pEdit->EnableNotify(TRUE);
    EnableNotify(TRUE);
    EnableRefresh(TRUE);

    return fRet;
  }

  return 0;
}

FX_FLOAT CPWL_Note_Edit::GetItemLeftMargin() {
  return POPUP_ITEM_TEXT_INDENT;
}

FX_FLOAT CPWL_Note_Edit::GetItemRightMargin() {
  return POPUP_ITEM_TEXT_INDENT;
}

CPWL_Note_LBBox::CPWL_Note_LBBox() {}

CPWL_Note_LBBox::~CPWL_Note_LBBox() {}

void CPWL_Note_LBBox::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                         CFX_Matrix* pUser2Device) {
  CFX_FloatRect rcClient = GetClientRect();

  CFX_GraphStateData gsd;
  gsd.m_LineWidth = 1.0f;

  CFX_PathData pathCross;

  pathCross.SetPointCount(4);
  pathCross.SetPoint(0, rcClient.left, rcClient.top, FXPT_MOVETO);
  pathCross.SetPoint(1, rcClient.right, rcClient.bottom, FXPT_LINETO);
  pathCross.SetPoint(2, rcClient.left,
                     rcClient.bottom + rcClient.Height() * 0.5f, FXPT_MOVETO);
  pathCross.SetPoint(3, rcClient.left + rcClient.Width() * 0.5f,
                     rcClient.bottom, FXPT_LINETO);

  pDevice->DrawPath(
      &pathCross, pUser2Device, &gsd, 0,
      CPWL_Utils::PWLColorToFXColor(GetTextColor(), GetTransparency()),
      FXFILL_ALTERNATE);
}

CPWL_Note_RBBox::CPWL_Note_RBBox() {}

CPWL_Note_RBBox::~CPWL_Note_RBBox() {}

void CPWL_Note_RBBox::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                         CFX_Matrix* pUser2Device) {
  CFX_FloatRect rcClient = GetClientRect();

  CFX_GraphStateData gsd;
  gsd.m_LineWidth = 1.0f;

  CFX_PathData pathCross;

  pathCross.SetPointCount(4);
  pathCross.SetPoint(0, rcClient.right, rcClient.top, FXPT_MOVETO);
  pathCross.SetPoint(1, rcClient.left, rcClient.bottom, FXPT_LINETO);
  pathCross.SetPoint(2, rcClient.right,
                     rcClient.bottom + rcClient.Height() * 0.5f, FXPT_MOVETO);
  pathCross.SetPoint(3, rcClient.left + rcClient.Width() * 0.5f,
                     rcClient.bottom, FXPT_LINETO);

  pDevice->DrawPath(
      &pathCross, pUser2Device, &gsd, 0,
      CPWL_Utils::PWLColorToFXColor(GetTextColor(), GetTransparency()),
      FXFILL_ALTERNATE);
}

CPWL_Note_Icon::CPWL_Note_Icon() : m_nType(0) {}

CPWL_Note_Icon::~CPWL_Note_Icon() {}

void CPWL_Note_Icon::SetIconType(int32_t nType) {
  m_nType = nType;
}

void CPWL_Note_Icon::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                        CFX_Matrix* pUser2Device) {
  CPWL_Utils::DrawIconAppStream(pDevice, pUser2Device, m_nType, GetClientRect(),
                                GetBackgroundColor(), PWL_DEFAULT_BLACKCOLOR,
                                GetTransparency());
}

CPWL_Note_CloseBox::CPWL_Note_CloseBox() : m_bMouseDown(FALSE) {}

CPWL_Note_CloseBox::~CPWL_Note_CloseBox() {}

void CPWL_Note_CloseBox::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                            CFX_Matrix* pUser2Device) {
  CPWL_Button::DrawThisAppearance(pDevice, pUser2Device);

  CFX_FloatRect rcClient = GetClientRect();
  rcClient = CPWL_Utils::DeflateRect(rcClient, 2.0f);

  CFX_GraphStateData gsd;
  gsd.m_LineWidth = 1.0f;

  CFX_PathData pathCross;

  if (m_bMouseDown) {
    rcClient.left += 0.5f;
    rcClient.right += 0.5f;
    rcClient.top -= 0.5f;
    rcClient.bottom -= 0.5f;
  }

  pathCross.SetPointCount(4);
  pathCross.SetPoint(0, rcClient.left, rcClient.bottom, FXPT_MOVETO);
  pathCross.SetPoint(1, rcClient.right, rcClient.top, FXPT_LINETO);
  pathCross.SetPoint(2, rcClient.left, rcClient.top, FXPT_MOVETO);
  pathCross.SetPoint(3, rcClient.right, rcClient.bottom, FXPT_LINETO);

  pDevice->DrawPath(
      &pathCross, pUser2Device, &gsd, 0,
      CPWL_Utils::PWLColorToFXColor(GetTextColor(), GetTransparency()),
      FXFILL_ALTERNATE);
}

FX_BOOL CPWL_Note_CloseBox::OnLButtonDown(const CFX_FloatPoint& point,
                                          uint32_t nFlag) {
  SetBorderStyle(PBS_INSET);
  InvalidateRect(NULL);

  m_bMouseDown = TRUE;

  return CPWL_Button::OnLButtonDown(point, nFlag);
}

FX_BOOL CPWL_Note_CloseBox::OnLButtonUp(const CFX_FloatPoint& point,
                                        uint32_t nFlag) {
  m_bMouseDown = FALSE;

  SetBorderStyle(PBS_BEVELED);
  InvalidateRect(NULL);

  return CPWL_Button::OnLButtonUp(point, nFlag);
}

CPWL_Note_Contents::CPWL_Note_Contents() : m_pEdit(NULL) {}

CPWL_Note_Contents::~CPWL_Note_Contents() {}

CFX_ByteString CPWL_Note_Contents::GetClassName() const {
  return "CPWL_Note_Contents";
}

void CPWL_Note_Contents::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  m_pEdit = new CPWL_Note_Edit;
  PWL_CREATEPARAM ecp = cp;
  ecp.pParentWnd = this;
  ecp.dwFlags = PWS_VISIBLE | PWS_CHILD | PES_MULTILINE | PES_AUTORETURN |
                PES_TEXTOVERFLOW | PES_UNDO | PES_SPELLCHECK;

  m_pEdit->EnableNotify(FALSE);
  m_pEdit->Create(ecp);
  m_pEdit->EnableNotify(TRUE);
}

void CPWL_Note_Contents::SetText(const CFX_WideString& sText) {
  if (m_pEdit) {
    m_pEdit->EnableNotify(FALSE);
    m_pEdit->SetText(sText.c_str());
    m_pEdit->EnableNotify(TRUE);
    OnNotify(m_pEdit, PNM_NOTEEDITCHANGED, 0, 0);
  }
}

CFX_WideString CPWL_Note_Contents::GetText() const {
  if (m_pEdit)
    return m_pEdit->GetText();

  return L"";
}

int32_t CPWL_Note_Contents::CountSubItems() const {
  return m_aChildren.GetSize() - 1;
}

IPWL_NoteItem* CPWL_Note_Contents::GetSubItems(int32_t index) const {
  int32_t nIndex = index + 1;

  if (nIndex > 0 && nIndex < m_aChildren.GetSize()) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(nIndex)) {
      ASSERT(pChild->GetClassName() == "CPWL_NoteItem");
      CPWL_NoteItem* pItem = (CPWL_NoteItem*)pChild;
      return pItem;
    }
  }
  return NULL;
}

IPWL_NoteItem* CPWL_Note_Contents::GetHitNoteItem(const CFX_FloatPoint& point) {
  CFX_FloatPoint pt = ParentToChild(point);

  for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
      if (pChild->GetClassName() == "CPWL_NoteItem") {
        CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pChild;
        if (IPWL_NoteItem* pRet = pNoteItem->GetHitNoteItem(pt))
          return pRet;
      }
    }
  }
  return NULL;
}

void CPWL_Note_Contents::OnNotify(CPWL_Wnd* pWnd,
                                  uint32_t msg,
                                  intptr_t wParam,
                                  intptr_t lParam) {
  switch (msg) {
    case PNM_NOTEEDITCHANGED: {
      int32_t nIndex = GetItemIndex(pWnd);
      if (nIndex < 0)
        nIndex = 0;

      m_pEdit->EnableNotify(FALSE);
      ResetContent(nIndex);
      m_pEdit->EnableNotify(TRUE);

      for (int32_t i = nIndex + 1, sz = m_aChildren.GetSize(); i < sz; i++) {
        if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
          pChild->OnNotify(this, PNM_NOTERESET, 0, 0);
      }

      if (CPWL_Wnd* pParent = GetParentWindow()) {
        pParent->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
      }
    }
      return;
    case PNM_SCROLLWINDOW:
      SetScrollPos(CFX_FloatPoint(0.0f, *(FX_FLOAT*)lParam));
      ResetFace();
      InvalidateRect(NULL);
      return;
    case PNM_SETCARETINFO:
      if (PWL_CARET_INFO* pInfo = (PWL_CARET_INFO*)wParam) {
        PWL_CARET_INFO newInfo = *pInfo;
        newInfo.bVisible = TRUE;
        newInfo.ptHead = ChildToParent(pInfo->ptHead);
        newInfo.ptFoot = ChildToParent(pInfo->ptFoot);

        if (CPWL_Wnd* pParent = GetParentWindow()) {
          pParent->OnNotify(this, PNM_SETCARETINFO, (intptr_t)&newInfo, 0);
        }
      }
      return;
    case PNM_NOTERESET: {
      m_pEdit->EnableNotify(FALSE);
      ResetContent(0);
      m_pEdit->EnableNotify(TRUE);

      for (int32_t i = 1, sz = m_aChildren.GetSize(); i < sz; i++) {
        if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
          pChild->OnNotify(this, PNM_NOTERESET, 0, 0);
      }

      m_pEdit->EnableNotify(FALSE);
      ResetContent(0);
      m_pEdit->EnableNotify(TRUE);
    }
      return;
  }

  CPWL_Wnd::OnNotify(pWnd, msg, wParam, lParam);
}

FX_BOOL CPWL_Note_Contents::OnLButtonDown(const CFX_FloatPoint& point,
                                          uint32_t nFlag) {
  if (CPWL_Wnd::OnLButtonDown(point, nFlag))
    return TRUE;

  if (!m_pEdit->IsFocused()) {
    m_pEdit->SetFocus();
  }

  return TRUE;
}

void CPWL_Note_Contents::SetEditFocus(FX_BOOL bLast) {
  if (!m_pEdit->IsFocused()) {
    m_pEdit->SetFocus();
    m_pEdit->SetCaret(bLast ? m_pEdit->GetTotalWords() : 0);
  }
}

CPWL_Edit* CPWL_Note_Contents::GetEdit() const {
  return m_pEdit;
}

void CPWL_Note_Contents::EnableModify(FX_BOOL bEnabled) {
  if (!bEnabled)
    m_pEdit->AddFlag(PWS_READONLY);
  else
    m_pEdit->RemoveFlag(PWS_READONLY);

  for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
      if (pChild->GetClassName() == "CPWL_NoteItem") {
        CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pChild;
        pNoteItem->EnableModify(bEnabled);
      }
    }
  }
}

void CPWL_Note_Contents::EnableRead(FX_BOOL bEnabled) {
  if (!bEnabled)
    m_pEdit->AddFlag(PES_NOREAD);
  else
    m_pEdit->RemoveFlag(PES_NOREAD);

  for (int32_t i = 0, sz = m_aChildren.GetSize(); i < sz; i++) {
    if (CPWL_Wnd* pChild = m_aChildren.GetAt(i)) {
      if (pChild->GetClassName() == "CPWL_NoteItem") {
        CPWL_NoteItem* pNoteItem = (CPWL_NoteItem*)pChild;
        pNoteItem->EnableRead(bEnabled);
      }
    }
  }
}

CPWL_NoteItem::CPWL_NoteItem()
    : m_pSubject(NULL),
      m_pDateTime(NULL),
      m_pContents(NULL),
      m_pPrivateData(NULL),
      m_sAuthor(L""),
      m_fOldItemHeight(0.0f),
      m_bSizeChanged(FALSE),
      m_bAllowModify(TRUE) {}

CPWL_NoteItem::~CPWL_NoteItem() {}

CFX_ByteString CPWL_NoteItem::GetClassName() const {
  return "CPWL_NoteItem";
}

void CPWL_NoteItem::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  CPWL_Color sTextColor;

  if (CPWL_Utils::IsBlackOrWhite(GetBackgroundColor()))
    sTextColor = PWL_DEFAULT_WHITECOLOR;
  else
    sTextColor = PWL_DEFAULT_BLACKCOLOR;

  m_pSubject = new CPWL_Label;
  PWL_CREATEPARAM scp = cp;
  scp.pParentWnd = this;
  scp.dwFlags = PWS_VISIBLE | PWS_CHILD | PES_LEFT | PES_TOP;
  scp.sTextColor = sTextColor;
  m_pSubject->Create(scp);

  m_pDateTime = new CPWL_Label;
  PWL_CREATEPARAM dcp = cp;
  dcp.pParentWnd = this;
  dcp.dwFlags = PWS_VISIBLE | PWS_CHILD | PES_RIGHT | PES_TOP;
  dcp.sTextColor = sTextColor;
  m_pDateTime->Create(dcp);

  m_pContents = new CPWL_Note_Contents;
  PWL_CREATEPARAM ccp = cp;
  ccp.pParentWnd = this;
  ccp.sBackgroundColor =
      CPWL_Color(COLORTYPE_RGB, 240 / 255.0f, 240 / 255.0f, 240 / 255.0f);
  ccp.dwFlags = PWS_VISIBLE | PWS_CHILD | PWS_BACKGROUND;
  m_pContents->Create(ccp);
  m_pContents->SetItemSpace(POPUP_ITEM_SPACE);
  m_pContents->SetTopSpace(POPUP_ITEM_SPACE);
  m_pContents->SetBottomSpace(POPUP_ITEM_SPACE);
}

void CPWL_NoteItem::RePosChildWnd() {
  if (IsValid()) {
    CFX_FloatRect rcClient = GetClientRect();

    CFX_FloatRect rcSubject = rcClient;
    rcSubject.left += POPUP_ITEM_TEXT_INDENT;
    rcSubject.top = rcClient.top;
    rcSubject.right =
        PWL_MIN(rcSubject.left + m_pSubject->GetContentRect().Width() + 1.0f,
                rcClient.right);
    rcSubject.bottom = rcSubject.top - m_pSubject->GetContentRect().Height();
    rcSubject.Normalize();
    m_pSubject->Move(rcSubject, TRUE, FALSE);
    m_pSubject->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcSubject));

    CFX_FloatRect rcDate = rcClient;
    rcDate.right -= POPUP_ITEM_TEXT_INDENT;
    rcDate.left =
        PWL_MAX(rcDate.right - m_pDateTime->GetContentRect().Width() - 1.0f,
                rcSubject.right);
    rcDate.bottom = rcDate.top - m_pDateTime->GetContentRect().Height();
    rcDate.Normalize();
    m_pDateTime->Move(rcDate, TRUE, FALSE);
    m_pDateTime->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcDate));

    CFX_FloatRect rcContents = rcClient;
    rcContents.left += 1.0f;
    rcContents.right -= 1.0f;
    rcContents.top = rcDate.bottom - POPUP_ITEM_HEAD_BOTTOM;
    rcContents.bottom += POPUP_ITEM_BOTTOMWIDTH;
    rcContents.Normalize();
    m_pContents->Move(rcContents, TRUE, FALSE);
    m_pContents->SetVisible(CPWL_Utils::ContainsRect(rcClient, rcContents));
  }

  SetClipRect(CPWL_Utils::InflateRect(GetWindowRect(), 1.0f));
}

void CPWL_NoteItem::SetPrivateData(void* pData) {
  m_pPrivateData = pData;
}

void CPWL_NoteItem::SetBkColor(const CPWL_Color& color) {
  CPWL_Color sBK = color;
  SetBackgroundColor(sBK);

  CPWL_Color sTextColor;

  if (CPWL_Utils::IsBlackOrWhite(sBK))
    sTextColor = PWL_DEFAULT_WHITECOLOR;
  else
    sTextColor = PWL_DEFAULT_BLACKCOLOR;

  SetTextColor(sTextColor);
  if (m_pSubject)
    m_pSubject->SetTextColor(sTextColor);
  if (m_pDateTime)
    m_pDateTime->SetTextColor(sTextColor);

  InvalidateRect(nullptr);
}

void CPWL_NoteItem::SetSubjectName(const CFX_WideString& sName) {
  if (m_pSubject)
    m_pSubject->SetText(sName.c_str());
}

void CPWL_NoteItem::SetDateTime(FX_SYSTEMTIME time) {
  m_dtNote = time;

  CFX_WideString swTime;
  swTime.Format(L"%04d-%02d-%02d %02d:%02d:%02d", time.wYear, time.wMonth,
                time.wDay, time.wHour, time.wMinute, time.wSecond);
  if (m_pDateTime) {
    m_pDateTime->SetText(swTime.c_str());
  }

  RePosChildWnd();
}

void CPWL_NoteItem::SetContents(const CFX_WideString& sContents) {
  if (m_pContents) {
    m_pContents->SetText(sContents);
  }
}

CPWL_NoteItem* CPWL_NoteItem::GetParentNoteItem() const {
  if (CPWL_Wnd* pParent = GetParentWindow()) {
    if (CPWL_Wnd* pGrand = pParent->GetParentWindow()) {
      ASSERT(pGrand->GetClassName() == "CPWL_NoteItem");
      return (CPWL_NoteItem*)pGrand;
    }
  }

  return NULL;
}

IPWL_NoteItem* CPWL_NoteItem::GetParentItem() const {
  return GetParentNoteItem();
}

CPWL_Edit* CPWL_NoteItem::GetEdit() const {
  if (m_pContents)
    return m_pContents->GetEdit();
  return NULL;
}

void* CPWL_NoteItem::GetPrivateData() const {
  return m_pPrivateData;
}

CFX_WideString CPWL_NoteItem::GetAuthorName() const {
  return m_sAuthor;
}

CPWL_Color CPWL_NoteItem::GetBkColor() const {
  return GetBackgroundColor();
}

CFX_WideString CPWL_NoteItem::GetContents() const {
  if (m_pContents)
    return m_pContents->GetText();

  return L"";
}

FX_SYSTEMTIME CPWL_NoteItem::GetDateTime() const {
  return m_dtNote;
}

CFX_WideString CPWL_NoteItem::GetSubjectName() const {
  if (m_pSubject)
    return m_pSubject->GetText();

  return L"";
}

int32_t CPWL_NoteItem::CountSubItems() const {
  if (m_pContents)
    return m_pContents->CountSubItems();

  return 0;
}

IPWL_NoteItem* CPWL_NoteItem::GetSubItems(int32_t index) const {
  if (m_pContents)
    return m_pContents->GetSubItems(index);

  return NULL;
}

IPWL_NoteItem* CPWL_NoteItem::GetHitNoteItem(const CFX_FloatPoint& point) {
  CFX_FloatPoint pt = ParentToChild(point);

  if (WndHitTest(pt)) {
    if (m_pContents) {
      if (IPWL_NoteItem* pNoteItem = m_pContents->GetHitNoteItem(pt))
        return pNoteItem;
    }

    return this;
  }

  return NULL;
}

IPWL_NoteItem* CPWL_NoteItem::GetFocusedNoteItem() const {
  if (const CPWL_Wnd* pWnd = GetFocused()) {
    if (pWnd->GetClassName() == "CPWL_Edit") {
      if (CPWL_Wnd* pParent = pWnd->GetParentWindow()) {
        ASSERT(pParent->GetClassName() == "CPWL_Note_Contents");

        if (CPWL_Wnd* pGrand = pParent->GetParentWindow()) {
          ASSERT(pGrand->GetClassName() == "CPWL_NoteItem");
          return (CPWL_NoteItem*)pGrand;
        }
      }
    }
  }

  return NULL;
}

FX_FLOAT CPWL_NoteItem::GetItemHeight(FX_FLOAT fLimitWidth) {
  if (fLimitWidth > 0) {
    if (!m_bSizeChanged)
      return m_fOldItemHeight;

    m_bSizeChanged = FALSE;

    FX_FLOAT fRet = m_pDateTime->GetContentRect().Height();
    FX_FLOAT fBorderWidth = (FX_FLOAT)GetBorderWidth();
    if (fLimitWidth > fBorderWidth * 2)
      fRet += m_pContents->GetContentsHeight(fLimitWidth - fBorderWidth * 2);
    fRet += POPUP_ITEM_HEAD_BOTTOM + POPUP_ITEM_BOTTOMWIDTH + fBorderWidth * 2;

    return m_fOldItemHeight = fRet;
  }

  return 0;
}

FX_FLOAT CPWL_NoteItem::GetItemLeftMargin() {
  return POPUP_ITEM_SIDEMARGIN;
}

FX_FLOAT CPWL_NoteItem::GetItemRightMargin() {
  return POPUP_ITEM_SIDEMARGIN;
}

FX_BOOL CPWL_NoteItem::OnLButtonDown(const CFX_FloatPoint& point,
                                     uint32_t nFlag) {
  if (!m_pContents->WndHitTest(m_pContents->ParentToChild(point))) {
    SetNoteFocus(FALSE);
  }

  CPWL_Wnd::OnLButtonDown(point, nFlag);

  return TRUE;
}

FX_BOOL CPWL_NoteItem::OnRButtonUp(const CFX_FloatPoint& point,
                                   uint32_t nFlag) {
  if (!m_pContents->WndHitTest(m_pContents->ParentToChild(point))) {
    SetNoteFocus(FALSE);
    PopupNoteItemMenu(point);

    return TRUE;
  }

  return CPWL_Wnd::OnRButtonUp(point, nFlag);
}

void CPWL_NoteItem::OnNotify(CPWL_Wnd* pWnd,
                             uint32_t msg,
                             intptr_t wParam,
                             intptr_t lParam) {
  switch (msg) {
    case PNM_NOTEEDITCHANGED:
      m_bSizeChanged = TRUE;

      if (CPWL_Wnd* pParent = GetParentWindow()) {
        pParent->OnNotify(this, PNM_NOTEEDITCHANGED, 0, 0);
      }
      return;
    case PNM_SETCARETINFO:
      if (PWL_CARET_INFO* pInfo = (PWL_CARET_INFO*)wParam) {
        PWL_CARET_INFO newInfo = *pInfo;
        newInfo.bVisible = TRUE;
        newInfo.ptHead = ChildToParent(pInfo->ptHead);
        newInfo.ptFoot = ChildToParent(pInfo->ptFoot);

        if (CPWL_Wnd* pParent = GetParentWindow()) {
          pParent->OnNotify(this, PNM_SETCARETINFO, (intptr_t)&newInfo, 0);
        }
      }
      return;
    case PNM_NOTERESET:
      m_bSizeChanged = TRUE;
      m_pContents->OnNotify(this, PNM_NOTERESET, 0, 0);

      return;
  }

  CPWL_Wnd::OnNotify(pWnd, msg, wParam, lParam);
}

void CPWL_NoteItem::PopupNoteItemMenu(const CFX_FloatPoint& point) {
}

void CPWL_NoteItem::SetNoteFocus(FX_BOOL bLast) {
  m_pContents->SetEditFocus(bLast);
}

void CPWL_NoteItem::EnableModify(FX_BOOL bEnabled) {
  m_pContents->EnableModify(bEnabled);
  m_bAllowModify = bEnabled;
}

void CPWL_NoteItem::EnableRead(FX_BOOL bEnabled) {
  m_pContents->EnableRead(bEnabled);
}
