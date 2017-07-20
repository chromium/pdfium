// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_edit.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/xml/cxml_content.h"
#include "core/fxcrt/xml/cxml_element.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_font.h"
#include "fpdfsdk/fxedit/fxet_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_caret.h"
#include "fpdfsdk/pdfwindow/cpwl_edit_ctrl.h"
#include "fpdfsdk/pdfwindow/cpwl_font_map.h"
#include "fpdfsdk/pdfwindow/cpwl_scroll_bar.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"
#include "third_party/base/stl_util.h"

CPWL_Edit::CPWL_Edit() : m_bFocus(false) {}

CPWL_Edit::~CPWL_Edit() {
  ASSERT(!m_bFocus);
}

CFX_ByteString CPWL_Edit::GetClassName() const {
  return PWL_CLASSNAME_EDIT;
}

void CPWL_Edit::SetText(const CFX_WideString& csText) {
  CFX_WideString swText = csText;
  if (!HasFlag(PES_RICH)) {
    m_pEdit->SetText(swText);
    return;
  }

  CFX_ByteString sValue = CFX_ByteString::FromUnicode(swText);
  std::unique_ptr<CXML_Element> pXML(
      CXML_Element::Parse(sValue.c_str(), sValue.GetLength()));
  if (!pXML) {
    m_pEdit->SetText(swText);
    return;
  }
  swText.clear();

  bool bFirst = true;
  int32_t nCount = pXML->CountChildren();
  for (int32_t i = 0; i < nCount; i++) {
    CXML_Element* pSubElement = ToElement(pXML->GetChild(i));
    if (!pSubElement || !pSubElement->GetTagName().EqualNoCase("p"))
      continue;

    CFX_WideString swSection;
    int nSubChild = pSubElement->CountChildren();
    for (int32_t j = 0; j < nSubChild; j++) {
      CXML_Content* pSubContent = ToContent(pSubElement->GetChild(j));
      if (pSubContent)
        swSection += pSubContent->m_Content;
    }
    if (bFirst)
      bFirst = false;
    else
      swText += FWL_VKEY_Return;
    swText += swSection;
  }

  m_pEdit->SetText(swText);
}

void CPWL_Edit::RePosChildWnd() {
  if (CPWL_ScrollBar* pVSB = GetVScrollBar()) {
    CFX_FloatRect rcWindow = m_rcOldWindow;
    CFX_FloatRect rcVScroll =
        CFX_FloatRect(rcWindow.right, rcWindow.bottom,
                      rcWindow.right + PWL_SCROLLBAR_WIDTH, rcWindow.top);
    pVSB->Move(rcVScroll, true, false);
  }

  if (m_pEditCaret && !HasFlag(PES_TEXTOVERFLOW)) {
    CFX_FloatRect rect = GetClientRect();
    if (!rect.IsEmpty()) {
      // +1 for caret beside border
      rect.Inflate(1.0f, 1.0f);
      rect.Normalize();
    }
    m_pEditCaret->SetClipRect(rect);
  }

  CPWL_EditCtrl::RePosChildWnd();
}

CFX_FloatRect CPWL_Edit::GetClientRect() const {
  float width = static_cast<float>(GetBorderWidth() + GetInnerBorderWidth());
  CFX_FloatRect rcClient = GetWindowRect().GetDeflated(width, width);
  if (CPWL_ScrollBar* pVSB = GetVScrollBar()) {
    if (pVSB->IsVisible()) {
      rcClient.right -= PWL_SCROLLBAR_WIDTH;
    }
  }

  return rcClient;
}

void CPWL_Edit::SetAlignFormatV(PWL_EDIT_ALIGNFORMAT_V nFormat, bool bPaint) {
  m_pEdit->SetAlignmentV((int32_t)nFormat, bPaint);
}

bool CPWL_Edit::CanSelectAll() const {
  return GetSelectWordRange() != m_pEdit->GetWholeWordRange();
}

bool CPWL_Edit::CanClear() const {
  return !IsReadOnly() && m_pEdit->IsSelected();
}

bool CPWL_Edit::CanCopy() const {
  return !HasFlag(PES_PASSWORD) && !HasFlag(PES_NOREAD) &&
         m_pEdit->IsSelected();
}

bool CPWL_Edit::CanCut() const {
  return CanCopy() && !IsReadOnly();
}
void CPWL_Edit::CutText() {
  if (!CanCut())
    return;
  m_pEdit->Clear();
}

void CPWL_Edit::OnCreated() {
  CPWL_EditCtrl::OnCreated();

  if (CPWL_ScrollBar* pScroll = GetVScrollBar()) {
    pScroll->RemoveFlag(PWS_AUTOTRANSPARENT);
    pScroll->SetTransparency(255);
  }

  SetParamByFlag();

  m_rcOldWindow = GetWindowRect();

  m_pEdit->SetOprNotify(this);
  m_pEdit->EnableOprNotify(true);
}

void CPWL_Edit::SetParamByFlag() {
  if (HasFlag(PES_RIGHT)) {
    m_pEdit->SetAlignmentH(2, false);
  } else if (HasFlag(PES_MIDDLE)) {
    m_pEdit->SetAlignmentH(1, false);
  } else {
    m_pEdit->SetAlignmentH(0, false);
  }

  if (HasFlag(PES_BOTTOM)) {
    m_pEdit->SetAlignmentV(2, false);
  } else if (HasFlag(PES_CENTER)) {
    m_pEdit->SetAlignmentV(1, false);
  } else {
    m_pEdit->SetAlignmentV(0, false);
  }

  if (HasFlag(PES_PASSWORD)) {
    m_pEdit->SetPasswordChar('*', false);
  }

  m_pEdit->SetMultiLine(HasFlag(PES_MULTILINE), false);
  m_pEdit->SetAutoReturn(HasFlag(PES_AUTORETURN), false);
  m_pEdit->SetAutoFontSize(HasFlag(PWS_AUTOFONTSIZE), false);
  m_pEdit->SetAutoScroll(HasFlag(PES_AUTOSCROLL), false);
  m_pEdit->EnableUndo(HasFlag(PES_UNDO));

  if (HasFlag(PES_TEXTOVERFLOW)) {
    SetClipRect(CFX_FloatRect());
    m_pEdit->SetTextOverflow(true, false);
  } else {
    if (m_pEditCaret) {
      CFX_FloatRect rect = GetClientRect();
      if (!rect.IsEmpty()) {
        // +1 for caret beside border
        rect.Inflate(1.0f, 1.0f);
        rect.Normalize();
      }
      m_pEditCaret->SetClipRect(rect);
    }
  }
}

void CPWL_Edit::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                   CFX_Matrix* pUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

  CFX_FloatRect rcClient = GetClientRect();

  int32_t nCharArray = m_pEdit->GetCharArray();
  FX_SAFE_INT32 nCharArraySafe = nCharArray;
  nCharArraySafe -= 1;
  nCharArraySafe *= 2;

  if (nCharArray > 0 && nCharArraySafe.IsValid()) {
    switch (GetBorderStyle()) {
      case BorderStyle::SOLID: {
        CFX_GraphStateData gsd;
        gsd.m_LineWidth = (float)GetBorderWidth();

        CFX_PathData path;

        for (int32_t i = 0; i < nCharArray - 1; i++) {
          path.AppendPoint(
              CFX_PointF(
                  rcClient.left +
                      ((rcClient.right - rcClient.left) / nCharArray) * (i + 1),
                  rcClient.bottom),
              FXPT_TYPE::MoveTo, false);
          path.AppendPoint(
              CFX_PointF(
                  rcClient.left +
                      ((rcClient.right - rcClient.left) / nCharArray) * (i + 1),
                  rcClient.top),
              FXPT_TYPE::LineTo, false);
        }
        if (!path.GetPoints().empty()) {
          pDevice->DrawPath(&path, pUser2Device, &gsd, 0,
                            GetBorderColor().ToFXColor(255), FXFILL_ALTERNATE);
        }
        break;
      }
      case BorderStyle::DASH: {
        CFX_GraphStateData gsd;
        gsd.m_LineWidth = (float)GetBorderWidth();

        gsd.SetDashCount(2);
        gsd.m_DashArray[0] = (float)GetBorderDash().nDash;
        gsd.m_DashArray[1] = (float)GetBorderDash().nGap;
        gsd.m_DashPhase = (float)GetBorderDash().nPhase;

        CFX_PathData path;
        for (int32_t i = 0; i < nCharArray - 1; i++) {
          path.AppendPoint(
              CFX_PointF(
                  rcClient.left +
                      ((rcClient.right - rcClient.left) / nCharArray) * (i + 1),
                  rcClient.bottom),
              FXPT_TYPE::MoveTo, false);
          path.AppendPoint(
              CFX_PointF(
                  rcClient.left +
                      ((rcClient.right - rcClient.left) / nCharArray) * (i + 1),
                  rcClient.top),
              FXPT_TYPE::LineTo, false);
        }
        if (!path.GetPoints().empty()) {
          pDevice->DrawPath(&path, pUser2Device, &gsd, 0,
                            GetBorderColor().ToFXColor(255), FXFILL_ALTERNATE);
        }
        break;
      }
      default:
        break;
    }
  }

  CFX_FloatRect rcClip;
  CPVT_WordRange wrRange = m_pEdit->GetVisibleWordRange();
  CPVT_WordRange* pRange = nullptr;
  if (!HasFlag(PES_TEXTOVERFLOW)) {
    rcClip = GetClientRect();
    pRange = &wrRange;
  }

  CFX_SystemHandler* pSysHandler = GetSystemHandler();
  CFX_Edit::DrawEdit(pDevice, pUser2Device, m_pEdit.get(),
                     GetTextColor().ToFXColor(GetTransparency()), rcClip,
                     CFX_PointF(), pRange, pSysHandler, m_pFormFiller.Get());
}

bool CPWL_Edit::OnLButtonDown(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonDown(point, nFlag);

  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point)) {
    if (m_bMouseDown)
      InvalidateRect();

    m_bMouseDown = true;
    SetCapture();

    m_pEdit->OnMouseDown(point, IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
  }

  return true;
}

bool CPWL_Edit::OnLButtonDblClk(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonDblClk(point, nFlag);

  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point)) {
    m_pEdit->SelectAll();
  }

  return true;
}

bool CPWL_Edit::OnRButtonUp(const CFX_PointF& point, uint32_t nFlag) {
  if (m_bMouseDown)
    return false;

  CPWL_Wnd::OnRButtonUp(point, nFlag);

  if (!HasFlag(PES_TEXTOVERFLOW) && !ClientHitTest(point))
    return true;

  CFX_SystemHandler* pSH = GetSystemHandler();
  if (!pSH)
    return false;

  SetFocus();

  return false;
}

void CPWL_Edit::OnSetFocus() {
  SetEditCaret(true);
  if (!IsReadOnly()) {
    if (IPWL_FocusHandler* pFocusHandler = GetFocusHandler())
      pFocusHandler->OnSetFocus(this);
  }
  m_bFocus = true;
}

void CPWL_Edit::OnKillFocus() {
  CPWL_ScrollBar* pScroll = GetVScrollBar();
  if (pScroll && pScroll->IsVisible()) {
    pScroll->SetVisible(false);
    Move(m_rcOldWindow, true, true);
  }

  m_pEdit->SelectNone();
  SetCaret(false, CFX_PointF(), CFX_PointF());
  SetCharSet(FX_CHARSET_ANSI);
  m_bFocus = false;
}

void CPWL_Edit::SetCharSpace(float fCharSpace) {
  m_pEdit->SetCharSpace(fCharSpace);
}

CPVT_WordRange CPWL_Edit::GetSelectWordRange() const {
  if (m_pEdit->IsSelected()) {
    int32_t nStart = -1;
    int32_t nEnd = -1;

    m_pEdit->GetSel(nStart, nEnd);

    CPVT_WordPlace wpStart = m_pEdit->WordIndexToWordPlace(nStart);
    CPVT_WordPlace wpEnd = m_pEdit->WordIndexToWordPlace(nEnd);

    return CPVT_WordRange(wpStart, wpEnd);
  }

  return CPVT_WordRange();
}

CFX_PointF CPWL_Edit::GetWordRightBottomPoint(const CPVT_WordPlace& wpWord) {
  CFX_Edit_Iterator* pIterator = m_pEdit->GetIterator();
  CPVT_WordPlace wpOld = pIterator->GetAt();
  pIterator->SetAt(wpWord);

  CFX_PointF pt;
  CPVT_Word word;
  if (pIterator->GetWord(word)) {
    pt = CFX_PointF(word.ptWord.x + word.fWidth, word.ptWord.y + word.fDescent);
  }
  pIterator->SetAt(wpOld);
  return pt;
}

bool CPWL_Edit::IsTextFull() const {
  return m_pEdit->IsTextFull();
}

float CPWL_Edit::GetCharArrayAutoFontSize(CPDF_Font* pFont,
                                          const CFX_FloatRect& rcPlate,
                                          int32_t nCharArray) {
  if (pFont && !pFont->IsStandardFont()) {
    FX_RECT rcBBox;
    pFont->GetFontBBox(rcBBox);

    CFX_FloatRect rcCell = rcPlate;
    float xdiv = rcCell.Width() / nCharArray * 1000.0f / rcBBox.Width();
    float ydiv = -rcCell.Height() * 1000.0f / rcBBox.Height();

    return xdiv < ydiv ? xdiv : ydiv;
  }

  return 0.0f;
}

void CPWL_Edit::SetCharArray(int32_t nCharArray) {
  if (HasFlag(PES_CHARARRAY) && nCharArray > 0) {
    m_pEdit->SetCharArray(nCharArray);
    m_pEdit->SetTextOverflow(true, true);

    if (HasFlag(PWS_AUTOFONTSIZE)) {
      if (IPVT_FontMap* pFontMap = GetFontMap()) {
        float fFontSize = GetCharArrayAutoFontSize(pFontMap->GetPDFFont(0),
                                                   GetClientRect(), nCharArray);
        if (fFontSize > 0.0f) {
          m_pEdit->SetAutoFontSize(false, true);
          m_pEdit->SetFontSize(fFontSize);
        }
      }
    }
  }
}

void CPWL_Edit::SetLimitChar(int32_t nLimitChar) {
  m_pEdit->SetLimitChar(nLimitChar);
}

void CPWL_Edit::ReplaceSel(const CFX_WideString& wsText) {
  m_pEdit->Clear();
  m_pEdit->InsertText(wsText, FX_CHARSET_Default);
}

CFX_FloatRect CPWL_Edit::GetFocusRect() const {
  return CFX_FloatRect();
}

bool CPWL_Edit::IsVScrollBarVisible() const {
  if (CPWL_ScrollBar* pScroll = GetVScrollBar())
    return pScroll->IsVisible();
  return false;
}

bool CPWL_Edit::OnKeyDown(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  if (nChar == FWL_VKEY_Delete) {
    if (m_pFillerNotify) {
      CFX_WideString strChange;
      CFX_WideString strChangeEx;

      int nSelStart = 0;
      int nSelEnd = 0;
      GetSel(nSelStart, nSelEnd);

      if (nSelStart == nSelEnd)
        nSelEnd = nSelStart + 1;

      bool bRC;
      bool bExit;
      std::tie(bRC, bExit) = m_pFillerNotify->OnBeforeKeyStroke(
          GetAttachedData(), strChange, strChangeEx, nSelStart, nSelEnd, true,
          nFlag);
      if (!bRC)
        return false;
      if (bExit)
        return false;
    }
  }

  bool bRet = CPWL_EditCtrl::OnKeyDown(nChar, nFlag);

  // In case of implementation swallow the OnKeyDown event.
  if (IsProceedtoOnChar(nChar, nFlag))
    return true;

  return bRet;
}

/**
 *In case of implementation swallow the OnKeyDown event.
 *If the event is swallowed, implementation may do other unexpected things,
 *which is not the control means to do.
 */
bool CPWL_Edit::IsProceedtoOnChar(uint16_t nKeyCode, uint32_t nFlag) {
  bool bCtrl = IsCTRLpressed(nFlag);
  bool bAlt = IsALTpressed(nFlag);
  if (bCtrl && !bAlt) {
    // hot keys for edit control.
    switch (nKeyCode) {
      case 'C':
      case 'V':
      case 'X':
      case 'A':
      case 'Z':
        return true;
      default:
        break;
    }
  }
  // control characters.
  switch (nKeyCode) {
    case FWL_VKEY_Escape:
    case FWL_VKEY_Back:
    case FWL_VKEY_Return:
    case FWL_VKEY_Space:
      return true;
    default:
      return false;
  }
}

bool CPWL_Edit::OnChar(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  bool bRC = true;
  bool bExit = false;

  if (!IsCTRLpressed(nFlag)) {
    if (m_pFillerNotify) {
      CFX_WideString swChange;

      int nSelStart = 0;
      int nSelEnd = 0;
      GetSel(nSelStart, nSelEnd);

      switch (nChar) {
        case FWL_VKEY_Back:
          if (nSelStart == nSelEnd)
            nSelStart = nSelEnd - 1;
          break;
        case FWL_VKEY_Return:
          break;
        default:
          swChange += nChar;
          break;
      }

      CFX_WideString strChangeEx;
      std::tie(bRC, bExit) = m_pFillerNotify->OnBeforeKeyStroke(
          GetAttachedData(), swChange, strChangeEx, nSelStart, nSelEnd, true,
          nFlag);
    }
  }

  if (!bRC)
    return true;
  if (bExit)
    return false;

  if (IPVT_FontMap* pFontMap = GetFontMap()) {
    int32_t nOldCharSet = GetCharSet();
    int32_t nNewCharSet =
        pFontMap->CharSetFromUnicode(nChar, FX_CHARSET_Default);
    if (nOldCharSet != nNewCharSet) {
      SetCharSet(nNewCharSet);
    }
  }

  return CPWL_EditCtrl::OnChar(nChar, nFlag);
}

bool CPWL_Edit::OnMouseWheel(short zDelta,
                             const CFX_PointF& point,
                             uint32_t nFlag) {
  if (HasFlag(PES_MULTILINE)) {
    CFX_PointF ptScroll = GetScrollPos();

    if (zDelta > 0) {
      ptScroll.y += GetFontSize();
    } else {
      ptScroll.y -= GetFontSize();
    }
    SetScrollPos(ptScroll);

    return true;
  }

  return false;
}

void CPWL_Edit::OnInsertReturn(const CPVT_WordPlace& place,
                               const CPVT_WordPlace& oldplace) {
  if (HasFlag(PES_SPELLCHECK)) {
    m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),
                                               GetLatinWordsRange(place)));
  }
}

void CPWL_Edit::OnBackSpace(const CPVT_WordPlace& place,
                            const CPVT_WordPlace& oldplace) {
  if (HasFlag(PES_SPELLCHECK)) {
    m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),
                                               GetLatinWordsRange(place)));
  }
}

void CPWL_Edit::OnDelete(const CPVT_WordPlace& place,
                         const CPVT_WordPlace& oldplace) {
  if (HasFlag(PES_SPELLCHECK)) {
    m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),
                                               GetLatinWordsRange(place)));
  }
}

void CPWL_Edit::OnClear(const CPVT_WordPlace& place,
                        const CPVT_WordPlace& oldplace) {
  if (HasFlag(PES_SPELLCHECK)) {
    m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),
                                               GetLatinWordsRange(place)));
  }
}

void CPWL_Edit::OnInsertWord(const CPVT_WordPlace& place,
                             const CPVT_WordPlace& oldplace) {
  if (HasFlag(PES_SPELLCHECK)) {
    m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),
                                               GetLatinWordsRange(place)));
  }
}

void CPWL_Edit::OnInsertText(const CPVT_WordPlace& place,
                             const CPVT_WordPlace& oldplace) {
  if (HasFlag(PES_SPELLCHECK)) {
    m_pEdit->RefreshWordRange(CombineWordRange(GetLatinWordsRange(oldplace),
                                               GetLatinWordsRange(place)));
  }
}

CPVT_WordRange CPWL_Edit::CombineWordRange(const CPVT_WordRange& wr1,
                                           const CPVT_WordRange& wr2) {
  return CPVT_WordRange(std::min(wr1.BeginPos, wr2.BeginPos),
                        std::max(wr1.EndPos, wr2.EndPos));
}

CPVT_WordRange CPWL_Edit::GetLatinWordsRange(const CFX_PointF& point) const {
  return GetSameWordsRange(m_pEdit->SearchWordPlace(point), true, false);
}

CPVT_WordRange CPWL_Edit::GetLatinWordsRange(
    const CPVT_WordPlace& place) const {
  return GetSameWordsRange(place, true, false);
}

CPVT_WordRange CPWL_Edit::GetArabicWordsRange(
    const CPVT_WordPlace& place) const {
  return GetSameWordsRange(place, false, true);
}

#define PWL_ISARABICWORD(word) \
  ((word >= 0x0600 && word <= 0x06FF) || (word >= 0xFB50 && word <= 0xFEFC))

CPVT_WordRange CPWL_Edit::GetSameWordsRange(const CPVT_WordPlace& place,
                                            bool bLatin,
                                            bool bArabic) const {
  CPVT_WordRange range;

  CFX_Edit_Iterator* pIterator = m_pEdit->GetIterator();
  CPVT_Word wordinfo;
  CPVT_WordPlace wpStart(place), wpEnd(place);
  pIterator->SetAt(place);

  if (bLatin) {
    while (pIterator->NextWord()) {
      if (!pIterator->GetWord(wordinfo) ||
          !FX_EDIT_ISLATINWORD(wordinfo.Word)) {
        break;
      }

      wpEnd = pIterator->GetAt();
    }
  } else if (bArabic) {
    while (pIterator->NextWord()) {
      if (!pIterator->GetWord(wordinfo) || !PWL_ISARABICWORD(wordinfo.Word))
        break;

      wpEnd = pIterator->GetAt();
    }
  }

  pIterator->SetAt(place);

  if (bLatin) {
    do {
      if (!pIterator->GetWord(wordinfo) ||
          !FX_EDIT_ISLATINWORD(wordinfo.Word)) {
        break;
      }

      wpStart = pIterator->GetAt();
    } while (pIterator->PrevWord());
  } else if (bArabic) {
    do {
      if (!pIterator->GetWord(wordinfo) || !PWL_ISARABICWORD(wordinfo.Word))
        break;

      wpStart = pIterator->GetAt();
    } while (pIterator->PrevWord());
  }

  range.Set(wpStart, wpEnd);
  return range;
}
