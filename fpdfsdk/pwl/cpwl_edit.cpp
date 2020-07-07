// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_edit.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_font.h"
#include "fpdfsdk/pwl/cpwl_caret.h"
#include "fpdfsdk/pwl/cpwl_edit_ctrl.h"
#include "fpdfsdk/pwl/cpwl_edit_impl.h"
#include "fpdfsdk/pwl/cpwl_scroll_bar.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"

CPWL_Edit::CPWL_Edit(
    const CreateParams& cp,
    std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData)
    : CPWL_EditCtrl(cp, std::move(pAttachedData)) {}

CPWL_Edit::~CPWL_Edit() {
  ASSERT(!m_bFocus);
}

void CPWL_Edit::SetText(const WideString& csText) {
  m_pEdit->SetText(csText);
}

bool CPWL_Edit::RePosChildWnd() {
  if (CPWL_ScrollBar* pVSB = GetVScrollBar()) {
    CFX_FloatRect rcWindow = m_rcOldWindow;
    CFX_FloatRect rcVScroll =
        CFX_FloatRect(rcWindow.right, rcWindow.bottom,
                      rcWindow.right + PWL_SCROLLBAR_WIDTH, rcWindow.top);

    ObservedPtr<CPWL_Edit> thisObserved(this);
    pVSB->Move(rcVScroll, true, false);
    if (!thisObserved)
      return false;
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

  return CPWL_EditCtrl::RePosChildWnd();
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

void CPWL_Edit::SetAlignFormatVerticalCenter() {
  m_pEdit->SetAlignmentV(static_cast<int32_t>(PEAV_CENTER), true);
}

bool CPWL_Edit::CanSelectAll() const {
  return GetSelectWordRange() != m_pEdit->GetWholeWordRange();
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
  m_pEdit->ClearSelection();
}

void CPWL_Edit::OnCreated() {
  CPWL_EditCtrl::OnCreated();

  if (CPWL_ScrollBar* pScroll = GetVScrollBar()) {
    pScroll->RemoveFlag(PWS_AUTOTRANSPARENT);
    pScroll->SetTransparency(255);
  }

  SetParamByFlag();

  m_rcOldWindow = GetWindowRect();

  m_pEdit->SetOperationNotify(this);
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
                                   const CFX_Matrix& mtUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, mtUser2Device);

  const CFX_FloatRect rcClient = GetClientRect();
  const BorderStyle border_style = GetBorderStyle();
  const int32_t nCharArray = m_pEdit->GetCharArray();
  bool draw_border = nCharArray > 0 && (border_style == BorderStyle::kSolid ||
                                        border_style == BorderStyle::kDash);
  if (draw_border) {
    FX_SAFE_INT32 nCharArraySafe = nCharArray;
    nCharArraySafe -= 1;
    nCharArraySafe *= 2;
    draw_border = nCharArraySafe.IsValid();
  }

  if (draw_border) {
    CFX_GraphStateData gsd;
    gsd.m_LineWidth = GetBorderWidth();
    if (border_style == BorderStyle::kDash) {
      gsd.m_DashArray = {static_cast<float>(GetBorderDash().nDash),
                         static_cast<float>(GetBorderDash().nGap)};
      gsd.m_DashPhase = GetBorderDash().nPhase;
    }

    const float width = (rcClient.right - rcClient.left) / nCharArray;
    CFX_PathData path;
    CFX_PointF bottom(0, rcClient.bottom);
    CFX_PointF top(0, rcClient.top);
    for (int32_t i = 0; i < nCharArray - 1; ++i) {
      bottom.x = rcClient.left + width * (i + 1);
      top.x = bottom.x;
      path.AppendPoint(bottom, FXPT_TYPE::MoveTo);
      path.AppendPoint(top, FXPT_TYPE::LineTo);
    }
    if (!path.GetPoints().empty()) {
      pDevice->DrawPath(&path, &mtUser2Device, &gsd, 0,
                        GetBorderColor().ToFXColor(255),
                        CFX_FillRenderOptions::EvenOddOptions());
    }
  }

  CFX_FloatRect rcClip;
  CPVT_WordRange wrRange = m_pEdit->GetVisibleWordRange();
  CPVT_WordRange* pRange = nullptr;
  if (!HasFlag(PES_TEXTOVERFLOW)) {
    rcClip = GetClientRect();
    pRange = &wrRange;
  }

  CPWL_EditImpl::DrawEdit(pDevice, mtUser2Device, m_pEdit.get(),
                          GetTextColor().ToFXColor(GetTransparency()), rcClip,
                          CFX_PointF(), pRange, GetSystemHandler(),
                          m_pFormFiller.Get());
}

bool CPWL_Edit::OnLButtonDown(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDown(nFlag, point);

  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point)) {
    if (m_bMouseDown && !InvalidateRect(nullptr))
      return true;

    m_bMouseDown = true;
    SetCapture();

    m_pEdit->OnMouseDown(point, IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
  }

  return true;
}

bool CPWL_Edit::OnLButtonDblClk(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDblClk(nFlag, point);

  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point)) {
    m_pEdit->SelectAll();
  }

  return true;
}

bool CPWL_Edit::OnRButtonUp(uint32_t nFlag, const CFX_PointF& point) {
  if (m_bMouseDown)
    return false;

  CPWL_Wnd::OnRButtonUp(nFlag, point);

  if (!HasFlag(PES_TEXTOVERFLOW) && !ClientHitTest(point))
    return true;

  SetFocus();

  return false;
}

void CPWL_Edit::OnSetFocus() {
  ObservedPtr<CPWL_Edit> observed_ptr(this);
  SetEditCaret(true);
  if (!observed_ptr)
    return;

  if (!IsReadOnly()) {
    if (CPWL_Wnd::FocusHandlerIface* pFocusHandler = GetFocusHandler()) {
      pFocusHandler->OnSetFocus(this);
      if (!observed_ptr)
        return;
    }
  }
  m_bFocus = true;
}

void CPWL_Edit::OnKillFocus() {
  ObservedPtr<CPWL_Edit> observed_ptr(this);
  CPWL_ScrollBar* pScroll = GetVScrollBar();
  if (pScroll && pScroll->IsVisible()) {
    pScroll->SetVisible(false);
    if (!observed_ptr)
      return;

    if (!Move(m_rcOldWindow, true, true))
      return;
  }

  m_pEdit->SelectNone();
  if (!observed_ptr)
    return;

  if (!SetCaret(false, CFX_PointF(), CFX_PointF()))
    return;

  SetCharSet(FX_CHARSET_ANSI);
  m_bFocus = false;
}

void CPWL_Edit::SetCharSpace(float fCharSpace) {
  m_pEdit->SetCharSpace(fCharSpace);
}

CPVT_WordRange CPWL_Edit::GetSelectWordRange() const {
  if (!m_pEdit->IsSelected())
    return CPVT_WordRange();

  int32_t nStart;
  int32_t nEnd;
  std::tie(nStart, nEnd) = m_pEdit->GetSelection();

  CPVT_WordPlace wpStart = m_pEdit->WordIndexToWordPlace(nStart);
  CPVT_WordPlace wpEnd = m_pEdit->WordIndexToWordPlace(nEnd);
  return CPVT_WordRange(wpStart, wpEnd);
}

CFX_PointF CPWL_Edit::GetWordRightBottomPoint(const CPVT_WordPlace& wpWord) {
  CPWL_EditImpl_Iterator* pIterator = m_pEdit->GetIterator();
  CPVT_WordPlace wpOld = pIterator->GetAt();
  pIterator->SetAt(wpWord);

  CFX_PointF pt;
  CPVT_Word word;
  if (pIterator->GetWord(word))
    pt = CFX_PointF(word.ptWord.x + word.fWidth, word.ptWord.y + word.fDescent);
  pIterator->SetAt(wpOld);
  return pt;
}

bool CPWL_Edit::IsTextFull() const {
  return m_pEdit->IsTextFull();
}

float CPWL_Edit::GetCharArrayAutoFontSize(const CPDF_Font* pFont,
                                          const CFX_FloatRect& rcPlate,
                                          int32_t nCharArray) {
  if (!pFont || pFont->IsStandardFont())
    return 0.0f;

  const FX_RECT& rcBBox = pFont->GetFontBBox();

  CFX_FloatRect rcCell = rcPlate;
  float xdiv = rcCell.Width() / nCharArray * 1000.0f / rcBBox.Width();
  float ydiv = -rcCell.Height() * 1000.0f / rcBBox.Height();

  return xdiv < ydiv ? xdiv : ydiv;
}

void CPWL_Edit::SetCharArray(int32_t nCharArray) {
  if (!HasFlag(PES_CHARARRAY) || nCharArray <= 0)
    return;

  m_pEdit->SetCharArray(nCharArray);
  m_pEdit->SetTextOverflow(true, true);

  if (!HasFlag(PWS_AUTOFONTSIZE))
    return;

  IPVT_FontMap* pFontMap = GetFontMap();
  if (!pFontMap)
    return;

  float fFontSize = GetCharArrayAutoFontSize(pFontMap->GetPDFFont(0).Get(),
                                             GetClientRect(), nCharArray);
  if (fFontSize <= 0.0f)
    return;

  m_pEdit->SetAutoFontSize(false, true);
  m_pEdit->SetFontSize(fFontSize);
}

void CPWL_Edit::SetLimitChar(int32_t nLimitChar) {
  m_pEdit->SetLimitChar(nLimitChar);
}

CFX_FloatRect CPWL_Edit::GetFocusRect() const {
  return CFX_FloatRect();
}

bool CPWL_Edit::IsVScrollBarVisible() const {
  CPWL_ScrollBar* pScroll = GetVScrollBar();
  return pScroll && pScroll->IsVisible();
}

bool CPWL_Edit::OnKeyDown(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  if (nChar == FWL_VKEY_Delete) {
    if (m_pFillerNotify) {
      WideString strChange;
      WideString strChangeEx;

      int nSelStart;
      int nSelEnd;
      std::tie(nSelStart, nSelEnd) = GetSelection();

      if (nSelStart == nSelEnd)
        nSelEnd = nSelStart + 1;

      ObservedPtr<CPWL_Wnd> thisObserved(this);

      bool bRC;
      bool bExit;
      std::tie(bRC, bExit) = m_pFillerNotify->OnBeforeKeyStroke(
          GetAttachedData(), strChange, strChangeEx, nSelStart, nSelEnd, true,
          nFlag);

      if (!thisObserved)
        return false;

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

// static
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
      WideString swChange;

      int nSelStart;
      int nSelEnd;
      std::tie(nSelStart, nSelEnd) = GetSelection();

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

      ObservedPtr<CPWL_Wnd> thisObserved(this);

      WideString strChangeEx;
      std::tie(bRC, bExit) = m_pFillerNotify->OnBeforeKeyStroke(
          GetAttachedData(), swChange, strChangeEx, nSelStart, nSelEnd, true,
          nFlag);

      if (!thisObserved)
        return false;
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

bool CPWL_Edit::OnMouseWheel(uint32_t nFlag,
                             const CFX_PointF& point,
                             const CFX_Vector& delta) {
  if (!HasFlag(PES_MULTILINE))
    return false;

  CFX_PointF ptScroll = GetScrollPos();
  if (delta.y > 0)
    ptScroll.y += GetFontSize();
  else
    ptScroll.y -= GetFontSize();
  SetScrollPos(ptScroll);
  return true;
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

#define PWL_ISARABICWORD(word) \
  ((word >= 0x0600 && word <= 0x06FF) || (word >= 0xFB50 && word <= 0xFEFC))

CPVT_WordRange CPWL_Edit::GetSameWordsRange(const CPVT_WordPlace& place,
                                            bool bLatin,
                                            bool bArabic) const {
  CPWL_EditImpl_Iterator* pIterator = m_pEdit->GetIterator();
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

  return CPVT_WordRange(wpStart, wpEnd);
}
