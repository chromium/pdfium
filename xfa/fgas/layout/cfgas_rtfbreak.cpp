// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_rtfbreak.h"

#include <algorithm>

#include "build/build_config.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/adapters.h"
#include "third_party/base/numerics/safe_math.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_char.h"
#include "xfa/fgas/layout/cfgas_textpiece.h"
#include "xfa/fgas/layout/cfgas_textuserdata.h"
#include "xfa/fgas/layout/fgas_arabic.h"
#include "xfa/fgas/layout/fgas_linebreak.h"

CFGAS_RTFBreak::CFGAS_RTFBreak(Mask<LayoutStyle> dwLayoutStyles)
    : CFGAS_Break(dwLayoutStyles) {
  SetBreakStatus();
  m_bPagination = !!(m_dwLayoutStyles & LayoutStyle::kPagination);
}

CFGAS_RTFBreak::~CFGAS_RTFBreak() = default;

void CFGAS_RTFBreak::SetLineStartPos(float fLinePos) {
  int32_t iLinePos = FXSYS_roundf(fLinePos * kConversionFactor);
  iLinePos = std::min(iLinePos, m_iLineWidth);
  iLinePos = std::max(iLinePos, m_iLineStart);
  m_pCurLine->m_iStart = iLinePos;
}

void CFGAS_RTFBreak::AddPositionedTab(float fTabPos) {
  int32_t iTabPos = std::min(
      FXSYS_roundf(fTabPos * kConversionFactor) + m_iLineStart, m_iLineWidth);
  auto it = std::lower_bound(m_PositionedTabs.begin(), m_PositionedTabs.end(),
                             iTabPos);
  if (it != m_PositionedTabs.end() && *it == iTabPos)
    return;
  m_PositionedTabs.insert(it, iTabPos);
}

void CFGAS_RTFBreak::SetUserData(
    const RetainPtr<CFGAS_TextUserData>& pUserData) {
  if (m_pUserData == pUserData)
    return;

  SetBreakStatus();
  m_pUserData = pUserData;
}

bool CFGAS_RTFBreak::GetPositionedTab(int32_t* iTabPos) const {
  auto it = std::upper_bound(m_PositionedTabs.begin(), m_PositionedTabs.end(),
                             *iTabPos);
  if (it == m_PositionedTabs.end())
    return false;

  *iTabPos = *it;
  return true;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::AppendChar(wchar_t wch) {
  DCHECK(m_pCurLine);

  FX_CHARTYPE chartype = pdfium::unicode::GetCharType(wch);
  m_pCurLine->m_LineChars.emplace_back(wch, m_iHorizontalScale,
                                       m_iVerticalScale);
  CFGAS_Char* pCurChar = &m_pCurLine->m_LineChars.back();
  pCurChar->m_iFontSize = m_iFontSize;
  pCurChar->m_dwIdentity = m_dwIdentity;
  pCurChar->m_pUserData = m_pUserData;

  CFGAS_Char::BreakType dwRet1 = CFGAS_Char::BreakType::kNone;
  if (chartype != FX_CHARTYPE::kCombination &&
      GetUnifiedCharType(m_eCharType) != GetUnifiedCharType(chartype) &&
      m_eCharType != FX_CHARTYPE::kUnknown &&
      IsGreaterThanLineWidth(m_pCurLine->GetLineEnd()) &&
      (m_eCharType != FX_CHARTYPE::kSpace ||
       chartype != FX_CHARTYPE::kControl)) {
    dwRet1 = EndBreak(CFGAS_Char::BreakType::kLine);
    if (!m_pCurLine->m_LineChars.empty())
      pCurChar = &m_pCurLine->m_LineChars.back();
  }

  CFGAS_Char::BreakType dwRet2 = CFGAS_Char::BreakType::kNone;
  switch (chartype) {
    case FX_CHARTYPE::kTab:
      AppendChar_Tab(pCurChar);
      break;
    case FX_CHARTYPE::kControl:
      dwRet2 = AppendChar_Control(pCurChar);
      break;
    case FX_CHARTYPE::kCombination:
      AppendChar_Combination(pCurChar);
      break;
    case FX_CHARTYPE::kArabicAlef:
    case FX_CHARTYPE::kArabicSpecial:
    case FX_CHARTYPE::kArabicDistortion:
    case FX_CHARTYPE::kArabicNormal:
    case FX_CHARTYPE::kArabicForm:
    case FX_CHARTYPE::kArabic:
      dwRet2 = AppendChar_Arabic(pCurChar);
      break;
    case FX_CHARTYPE::kUnknown:
    case FX_CHARTYPE::kSpace:
    case FX_CHARTYPE::kNumeric:
    case FX_CHARTYPE::kNormal:
    default:
      dwRet2 = AppendChar_Others(pCurChar);
      break;
  }

  m_eCharType = chartype;
  return std::max(dwRet1, dwRet2);
}

void CFGAS_RTFBreak::AppendChar_Combination(CFGAS_Char* pCurChar) {
  absl::optional<uint16_t> iCharWidthRet;
  if (m_pFont) {
    iCharWidthRet = m_pFont->GetCharWidth(pCurChar->char_code());
  }
  FX_SAFE_INT32 iCharWidth = iCharWidthRet.value_or(0);
  iCharWidth *= m_iFontSize;
  iCharWidth *= m_iHorizontalScale;
  iCharWidth /= 100;
  CFGAS_Char* pLastChar = GetLastChar(0, false, true);
  if (pLastChar && pLastChar->GetCharType() > FX_CHARTYPE::kCombination)
    iCharWidth *= -1;
  else
    m_eCharType = FX_CHARTYPE::kCombination;

  int32_t iCharWidthValid = iCharWidth.ValueOrDefault(0);
  pCurChar->m_iCharWidth = iCharWidthValid;
  if (iCharWidthValid > 0) {
    FX_SAFE_INT32 checked_width = m_pCurLine->m_iWidth;
    checked_width += iCharWidthValid;
    if (!checked_width.IsValid())
      return;

    m_pCurLine->m_iWidth = checked_width.ValueOrDie();
  }
}

void CFGAS_RTFBreak::AppendChar_Tab(CFGAS_Char* pCurChar) {
  if (!(m_dwLayoutStyles & LayoutStyle::kExpandTab))
    return;

  int32_t& iLineWidth = m_pCurLine->m_iWidth;
  int32_t iCharWidth = iLineWidth;
  FX_SAFE_INT32 iSafeCharWidth;
  if (GetPositionedTab(&iCharWidth)) {
    iSafeCharWidth = iCharWidth;
  } else {
    // Tab width is >= 160000, so this part does not need to be checked.
    DCHECK(m_iTabWidth >= kMinimumTabWidth);
    iSafeCharWidth = iLineWidth / m_iTabWidth + 1;
    iSafeCharWidth *= m_iTabWidth;
  }
  iSafeCharWidth -= iLineWidth;

  iCharWidth = iSafeCharWidth.ValueOrDefault(0);

  pCurChar->m_iCharWidth = iCharWidth;
  iLineWidth += iCharWidth;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::AppendChar_Control(CFGAS_Char* pCurChar) {
  CFGAS_Char::BreakType dwRet2 = CFGAS_Char::BreakType::kNone;
  switch (pCurChar->char_code()) {
    case L'\v':
    case pdfium::unicode::kLineSeparator:
      dwRet2 = CFGAS_Char::BreakType::kLine;
      break;
    case L'\f':
      dwRet2 = CFGAS_Char::BreakType::kPage;
      break;
    case pdfium::unicode::kParagraphSeparator:
      dwRet2 = CFGAS_Char::BreakType::kParagraph;
      break;
    default:
      if (pCurChar->char_code() == m_wParagraphBreakChar)
        dwRet2 = CFGAS_Char::BreakType::kParagraph;
      break;
  }
  if (dwRet2 != CFGAS_Char::BreakType::kNone)
    dwRet2 = EndBreak(dwRet2);

  return dwRet2;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::AppendChar_Arabic(CFGAS_Char* pCurChar) {
  m_pCurLine->IncrementArabicCharCount();

  CFGAS_Char* pLastChar = nullptr;
  wchar_t wForm;
  bool bAlef = false;
  if (m_eCharType >= FX_CHARTYPE::kArabicAlef &&
      m_eCharType <= FX_CHARTYPE::kArabicDistortion) {
    pLastChar = GetLastChar(1, false, true);
    if (pLastChar) {
      m_pCurLine->m_iWidth -= pLastChar->m_iCharWidth;
      CFGAS_Char* pPrevChar = GetLastChar(2, false, true);
      wForm = pdfium::arabic::GetFormChar(pLastChar, pPrevChar, pCurChar);
      bAlef = (wForm == pdfium::unicode::kZeroWidthNoBreakSpace &&
               pLastChar->GetCharType() == FX_CHARTYPE::kArabicAlef);
      FX_SAFE_INT32 iCharWidth = 0;
      if (m_pFont) {
        absl::optional<uint16_t> iCharWidthRet = m_pFont->GetCharWidth(wForm);
        if (iCharWidthRet.has_value()) {
          iCharWidth = iCharWidthRet.value();
        } else {
          iCharWidthRet = m_pFont->GetCharWidth(pLastChar->char_code());
          iCharWidth = iCharWidthRet.value_or(0);
        }
      }
      iCharWidth *= m_iFontSize;
      iCharWidth *= m_iHorizontalScale;
      iCharWidth /= 100;

      int iCharWidthValid = iCharWidth.ValueOrDefault(0);
      pLastChar->m_iCharWidth = iCharWidthValid;

      FX_SAFE_INT32 checked_width = m_pCurLine->m_iWidth;
      checked_width += iCharWidthValid;
      if (!checked_width.IsValid())
        return CFGAS_Char::BreakType::kNone;

      m_pCurLine->m_iWidth = checked_width.ValueOrDie();
      iCharWidth = 0;
    }
  }

  wForm = pdfium::arabic::GetFormChar(pCurChar, bAlef ? nullptr : pLastChar,
                                      nullptr);
  FX_SAFE_INT32 iCharWidth = 0;
  if (m_pFont) {
    absl::optional<uint16_t> iCharWidthRet = m_pFont->GetCharWidth(wForm);
    if (!iCharWidthRet.has_value())
      iCharWidthRet = m_pFont->GetCharWidth(pCurChar->char_code());
    iCharWidth = iCharWidthRet.value_or(0);
    iCharWidth *= m_iFontSize;
    iCharWidth *= m_iHorizontalScale;
    iCharWidth /= 100;
  }

  int iCharWidthValid = iCharWidth.ValueOrDefault(0);
  pCurChar->m_iCharWidth = iCharWidthValid;

  FX_SAFE_INT32 checked_width = m_pCurLine->m_iWidth;
  checked_width += iCharWidthValid;
  if (!checked_width.IsValid())
    return CFGAS_Char::BreakType::kNone;

  m_pCurLine->m_iWidth = checked_width.ValueOrDie();

  if (IsGreaterThanLineWidth(m_pCurLine->GetLineEnd()))
    return EndBreak(CFGAS_Char::BreakType::kLine);
  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::AppendChar_Others(CFGAS_Char* pCurChar) {
  FX_CHARTYPE chartype = pCurChar->GetCharType();
  wchar_t wForm = pCurChar->char_code();
  FX_SAFE_INT32 iCharWidth = 0;
  if (m_pFont) {
    iCharWidth = m_pFont->GetCharWidth(wForm).value_or(0);
  }
  iCharWidth *= m_iFontSize;
  iCharWidth *= m_iHorizontalScale;
  iCharWidth /= 100;
  iCharWidth += m_iCharSpace;

  int iValidCharWidth = iCharWidth.ValueOrDefault(0);
  pCurChar->m_iCharWidth = iValidCharWidth;

  FX_SAFE_INT32 checked_width = m_pCurLine->m_iWidth;
  checked_width += iValidCharWidth;
  if (!checked_width.IsValid())
    return CFGAS_Char::BreakType::kNone;

  m_pCurLine->m_iWidth = checked_width.ValueOrDie();
  if (chartype != FX_CHARTYPE::kSpace &&
      IsGreaterThanLineWidth(m_pCurLine->GetLineEnd())) {
    return EndBreak(CFGAS_Char::BreakType::kLine);
  }
  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::EndBreak(CFGAS_Char::BreakType dwStatus) {
  DCHECK(dwStatus != CFGAS_Char::BreakType::kNone);

  ++m_dwIdentity;
  if (!m_pCurLine->m_LinePieces.empty()) {
    if (dwStatus != CFGAS_Char::BreakType::kPiece)
      m_pCurLine->m_LinePieces.back().SetStatus(dwStatus);
    return m_pCurLine->m_LinePieces.back().GetStatus();
  }

  if (HasLine()) {
    if (m_Lines[m_iReadyLineIndex].m_LinePieces.empty())
      return CFGAS_Char::BreakType::kNone;

    if (dwStatus != CFGAS_Char::BreakType::kPiece)
      m_Lines[m_iReadyLineIndex].m_LinePieces.back().SetStatus(dwStatus);
    return m_Lines[m_iReadyLineIndex].m_LinePieces.back().GetStatus();
  }

  CFGAS_Char* tc = m_pCurLine->LastChar();
  if (!tc)
    return CFGAS_Char::BreakType::kNone;

  tc->m_dwStatus = dwStatus;
  if (dwStatus == CFGAS_Char::BreakType::kPiece)
    return dwStatus;

  m_iReadyLineIndex = m_pCurLine == &m_Lines[0] ? 0 : 1;
  CFGAS_BreakLine* pNextLine = &m_Lines[1 - m_iReadyLineIndex];
  bool bAllChars = m_iAlignment == LineAlignment::Justified ||
                   m_iAlignment == LineAlignment::Distributed;

  if (!EndBreakSplitLine(pNextLine, bAllChars, dwStatus)) {
    std::deque<TPO> tpos = EndBreakBidiLine(dwStatus);
    if (!m_bPagination && m_iAlignment != LineAlignment::Left)
      EndBreakAlignment(tpos, bAllChars, dwStatus);
  }
  m_pCurLine = pNextLine;
  m_pCurLine->m_iStart = m_iLineStart;

  CFGAS_Char* pTC = GetLastChar(0, false, true);
  m_eCharType = pTC ? pTC->GetCharType() : FX_CHARTYPE::kUnknown;
  return dwStatus;
}

bool CFGAS_RTFBreak::EndBreakSplitLine(CFGAS_BreakLine* pNextLine,
                                       bool bAllChars,
                                       CFGAS_Char::BreakType dwStatus) {
  bool bDone = false;
  if (IsGreaterThanLineWidth(m_pCurLine->GetLineEnd())) {
    const CFGAS_Char* tc = m_pCurLine->LastChar();
    switch (tc->GetCharType()) {
      case FX_CHARTYPE::kTab:
      case FX_CHARTYPE::kControl:
      case FX_CHARTYPE::kSpace:
        break;
      default:
        SplitTextLine(m_pCurLine, pNextLine, !m_bPagination && bAllChars);
        bDone = true;
        break;
    }
  }

  if (!m_bPagination) {
    if (bAllChars && !bDone) {
      int32_t endPos = m_pCurLine->GetLineEnd();
      GetBreakPos(m_pCurLine->m_LineChars, bAllChars, true, &endPos);
    }
    return false;
  }

  const CFGAS_Char* pCurChars = m_pCurLine->m_LineChars.data();
  CFGAS_BreakPiece tp;
  tp.SetChars(&m_pCurLine->m_LineChars);
  bool bNew = true;
  uint32_t dwIdentity = static_cast<uint32_t>(-1);
  int32_t iLast = fxcrt::CollectionSize<int32_t>(m_pCurLine->m_LineChars) - 1;
  int32_t j = 0;
  for (int32_t i = 0; i <= iLast;) {
    const CFGAS_Char* pTC = pCurChars + i;
    if (bNew) {
      tp.SetStartChar(i);
      tp.IncrementStartPos(tp.GetWidth());
      tp.SetWidth(0);
      tp.SetStatus(pTC->m_dwStatus);
      tp.SetFontSize(pTC->m_iFontSize);
      tp.SetHorizontalScale(pTC->horizonal_scale());
      tp.SetVerticalScale(pTC->vertical_scale());
      dwIdentity = pTC->m_dwIdentity;
      tp.SetUserData(pTC->m_pUserData);
      j = i;
      bNew = false;
    }

    if (i == iLast || pTC->m_dwStatus != CFGAS_Char::BreakType::kNone ||
        pTC->m_dwIdentity != dwIdentity) {
      if (pTC->m_dwIdentity == dwIdentity) {
        tp.SetStatus(pTC->m_dwStatus);
        tp.IncrementWidth(pTC->m_iCharWidth);
        ++i;
      }
      tp.SetCharCount(i - j);
      m_pCurLine->m_LinePieces.push_back(tp);
      bNew = true;
    } else {
      tp.IncrementWidth(pTC->m_iCharWidth);
      ++i;
    }
  }
  return true;
}

std::deque<CFGAS_Break::TPO> CFGAS_RTFBreak::EndBreakBidiLine(
    CFGAS_Char::BreakType dwStatus) {
  CFGAS_Char* pTC;
  std::vector<CFGAS_Char>& chars = m_pCurLine->m_LineChars;
  if (!m_bPagination && m_pCurLine->HasArabicChar()) {
    size_t iBidiNum = 0;
    for (size_t i = 0; i < m_pCurLine->m_LineChars.size(); ++i) {
      pTC = &chars[i];
      pTC->m_iBidiPos = static_cast<int32_t>(i);
      if (pTC->GetCharType() != FX_CHARTYPE::kControl)
        iBidiNum = i;
      if (i == 0)
        pTC->m_iBidiLevel = 1;
    }
    CFGAS_Char::BidiLine(&chars, iBidiNum + 1);
  } else {
    for (size_t i = 0; i < m_pCurLine->m_LineChars.size(); ++i) {
      pTC = &chars[i];
      pTC->m_iBidiLevel = 0;
      pTC->m_iBidiPos = 0;
      pTC->m_iBidiOrder = 0;
    }
  }

  CFGAS_BreakPiece tp;
  tp.SetStatus(CFGAS_Char::BreakType::kPiece);
  tp.SetStartPos(m_pCurLine->m_iStart);
  tp.SetChars(&chars);

  int32_t iBidiLevel = -1;
  int32_t iCharWidth;
  std::deque<TPO> tpos;
  uint32_t dwIdentity = static_cast<uint32_t>(-1);
  int32_t i = 0;
  int32_t j = 0;
  int32_t iCount = fxcrt::CollectionSize<int32_t>(m_pCurLine->m_LineChars);
  while (i < iCount) {
    pTC = &chars[i];
    if (iBidiLevel < 0) {
      iBidiLevel = pTC->m_iBidiLevel;
      iCharWidth = pTC->m_iCharWidth;
      tp.SetWidth(iCharWidth < 1 ? 0 : iCharWidth);
      tp.SetBidiLevel(iBidiLevel);
      tp.SetBidiPos(pTC->m_iBidiOrder);
      tp.SetFontSize(pTC->m_iFontSize);
      tp.SetHorizontalScale(pTC->horizonal_scale());
      tp.SetVerticalScale(pTC->vertical_scale());
      dwIdentity = pTC->m_dwIdentity;
      tp.SetUserData(pTC->m_pUserData);
      tp.SetStatus(CFGAS_Char::BreakType::kPiece);
      ++i;
    } else if (iBidiLevel != pTC->m_iBidiLevel ||
               pTC->m_dwIdentity != dwIdentity) {
      tp.SetCharCount(i - tp.GetStartChar());
      m_pCurLine->m_LinePieces.push_back(tp);
      tp.IncrementStartPos(tp.GetWidth());
      tp.SetStartChar(i);
      tpos.push_back({j++, tp.GetBidiPos()});
      iBidiLevel = -1;
    } else {
      iCharWidth = pTC->m_iCharWidth;
      if (iCharWidth > 0)
        tp.IncrementWidth(iCharWidth);
      ++i;
    }
  }

  if (i > tp.GetStartChar()) {
    tp.SetStatus(dwStatus);
    tp.SetCharCount(i - tp.GetStartChar());
    m_pCurLine->m_LinePieces.push_back(tp);
    tpos.push_back({j, tp.GetBidiPos()});
  }

  std::sort(tpos.begin(), tpos.end());
  int32_t iStartPos = m_pCurLine->m_iStart;
  for (const auto& it : tpos) {
    CFGAS_BreakPiece& ttp = m_pCurLine->m_LinePieces[it.index];
    ttp.SetStartPos(iStartPos);
    iStartPos += ttp.GetWidth();
  }
  return tpos;
}

void CFGAS_RTFBreak::EndBreakAlignment(const std::deque<TPO>& tpos,
                                       bool bAllChars,
                                       CFGAS_Char::BreakType dwStatus) {
  int32_t iNetWidth = m_pCurLine->m_iWidth;
  int32_t iGapChars = 0;
  bool bFind = false;
  for (const TPO& pos : pdfium::base::Reversed(tpos)) {
    const CFGAS_BreakPiece& ttp = m_pCurLine->m_LinePieces[pos.index];
    if (!bFind)
      iNetWidth = ttp.GetEndPos();

    bool bArabic = FX_IsOdd(ttp.GetBidiLevel());
    int32_t j = bArabic ? 0 : ttp.GetCharCount() - 1;
    while (j > -1 && j < ttp.GetCharCount()) {
      const CFGAS_Char* tc = ttp.GetChar(j);
      if (tc->m_eLineBreakType == FX_LINEBREAKTYPE::kDIRECT_BRK)
        ++iGapChars;

      if (!bFind || !bAllChars) {
        FX_CHARTYPE dwCharType = tc->GetCharType();
        if (dwCharType == FX_CHARTYPE::kSpace ||
            dwCharType == FX_CHARTYPE::kControl) {
          if (!bFind) {
            int32_t iCharWidth = tc->m_iCharWidth;
            if (bAllChars && iCharWidth > 0)
              iNetWidth -= iCharWidth;
          }
        } else {
          bFind = true;
          if (!bAllChars)
            break;
        }
      }
      j += bArabic ? 1 : -1;
    }
    if (!bAllChars && bFind)
      break;
  }

  int32_t iOffset = m_iLineWidth - iNetWidth;
  if (iGapChars > 0 && (m_iAlignment == LineAlignment::Distributed ||
                        (m_iAlignment == LineAlignment::Justified &&
                         dwStatus != CFGAS_Char::BreakType::kParagraph))) {
    int32_t iStart = -1;
    for (const auto& tpo : tpos) {
      CFGAS_BreakPiece& ttp = m_pCurLine->m_LinePieces[tpo.index];
      if (iStart < 0)
        iStart = ttp.GetStartPos();
      else
        ttp.SetStartPos(iStart);

      for (int32_t j = 0; j < ttp.GetCharCount(); ++j) {
        CFGAS_Char* tc = ttp.GetChar(j);
        if (tc->m_eLineBreakType != FX_LINEBREAKTYPE::kDIRECT_BRK ||
            tc->m_iCharWidth < 0) {
          continue;
        }
        int32_t k = iOffset / iGapChars;
        tc->m_iCharWidth += k;
        ttp.IncrementWidth(k);
        iOffset -= k;
        --iGapChars;
        if (iGapChars < 1)
          break;
      }
      iStart += ttp.GetWidth();
    }
  } else if (m_iAlignment == LineAlignment::Right ||
             m_iAlignment == LineAlignment::Center) {
    if (m_iAlignment == LineAlignment::Center)
      iOffset /= 2;
    if (iOffset > 0) {
      for (auto& ttp : m_pCurLine->m_LinePieces)
        ttp.IncrementStartPos(iOffset);
    }
  }
}

int32_t CFGAS_RTFBreak::GetBreakPos(std::vector<CFGAS_Char>& tca,
                                    bool bAllChars,
                                    bool bOnlyBrk,
                                    int32_t* pEndPos) {
  int32_t iLength = fxcrt::CollectionSize<int32_t>(tca) - 1;
  if (iLength < 1)
    return iLength;

  int32_t iBreak = -1;
  int32_t iBreakPos = -1;
  int32_t iIndirect = -1;
  int32_t iIndirectPos = -1;
  int32_t iLast = -1;
  int32_t iLastPos = -1;
  if (*pEndPos <= m_iLineWidth) {
    if (!bAllChars)
      return iLength;

    iBreak = iLength;
    iBreakPos = *pEndPos;
  }

  CFGAS_Char* pCharArray = tca.data();
  CFGAS_Char* pCur = pCharArray + iLength;
  --iLength;
  if (bAllChars)
    pCur->m_eLineBreakType = FX_LINEBREAKTYPE::kUNKNOWN;

  FX_BREAKPROPERTY nNext = pdfium::unicode::GetBreakProperty(pCur->char_code());
  int32_t iCharWidth = pCur->m_iCharWidth;
  if (iCharWidth > 0)
    *pEndPos -= iCharWidth;

  while (iLength >= 0) {
    pCur = pCharArray + iLength;
    FX_BREAKPROPERTY nCur =
        pdfium::unicode::GetBreakProperty(pCur->char_code());
    bool bNeedBreak = false;
    FX_LINEBREAKTYPE eType;
    if (nCur == FX_BREAKPROPERTY::kTB) {
      bNeedBreak = true;
      eType = nNext == FX_BREAKPROPERTY::kTB
                  ? FX_LINEBREAKTYPE::kPROHIBITED_BRK
                  : GetLineBreakTypeFromPair(nCur, nNext);
    } else {
      if (nCur == FX_BREAKPROPERTY::kSP)
        bNeedBreak = true;

      eType = nNext == FX_BREAKPROPERTY::kSP
                  ? FX_LINEBREAKTYPE::kPROHIBITED_BRK
                  : GetLineBreakTypeFromPair(nCur, nNext);
    }
    if (bAllChars)
      pCur->m_eLineBreakType = eType;

    if (!bOnlyBrk) {
      iCharWidth = pCur->m_iCharWidth;
      if (*pEndPos <= m_iLineWidth || bNeedBreak) {
        if (eType == FX_LINEBREAKTYPE::kDIRECT_BRK && iBreak < 0) {
          iBreak = iLength;
          iBreakPos = *pEndPos;
          if (!bAllChars)
            return iLength;
        } else if (eType == FX_LINEBREAKTYPE::kINDIRECT_BRK && iIndirect < 0) {
          iIndirect = iLength;
          iIndirectPos = *pEndPos;
        }
        if (iLast < 0) {
          iLast = iLength;
          iLastPos = *pEndPos;
        }
      }
      if (iCharWidth > 0)
        *pEndPos -= iCharWidth;
    }
    nNext = nCur;
    --iLength;
  }
  if (bOnlyBrk)
    return 0;

  if (iBreak > -1) {
    *pEndPos = iBreakPos;
    return iBreak;
  }
  if (iIndirect > -1) {
    *pEndPos = iIndirectPos;
    return iIndirect;
  }
  if (iLast > -1) {
    *pEndPos = iLastPos;
    return iLast;
  }
  return 0;
}

void CFGAS_RTFBreak::SplitTextLine(CFGAS_BreakLine* pCurLine,
                                   CFGAS_BreakLine* pNextLine,
                                   bool bAllChars) {
  DCHECK(pCurLine);
  DCHECK(pNextLine);

  if (pCurLine->m_LineChars.size() < 2)
    return;

  int32_t iEndPos = pCurLine->GetLineEnd();
  std::vector<CFGAS_Char>& curChars = pCurLine->m_LineChars;
  int32_t iCharPos = GetBreakPos(curChars, bAllChars, false, &iEndPos);
  if (iCharPos < 0)
    iCharPos = 0;

  ++iCharPos;
  if (iCharPos >= fxcrt::CollectionSize<int32_t>(pCurLine->m_LineChars)) {
    pNextLine->Clear();
    curChars[iCharPos - 1].m_eLineBreakType = FX_LINEBREAKTYPE::kUNKNOWN;
    return;
  }

  pNextLine->m_LineChars =
      std::vector<CFGAS_Char>(curChars.begin() + iCharPos, curChars.end());
  curChars.erase(curChars.begin() + iCharPos, curChars.end());
  pNextLine->m_iStart = pCurLine->m_iStart;
  pNextLine->m_iWidth = pCurLine->GetLineEnd() - iEndPos;
  pCurLine->m_iWidth = iEndPos;
  curChars[iCharPos - 1].m_eLineBreakType = FX_LINEBREAKTYPE::kUNKNOWN;

  for (size_t i = 0; i < pNextLine->m_LineChars.size(); ++i) {
    if (pNextLine->m_LineChars[i].GetCharType() >= FX_CHARTYPE::kArabicAlef) {
      pCurLine->DecrementArabicCharCount();
      pNextLine->IncrementArabicCharCount();
    }
    pNextLine->m_LineChars[i].m_dwStatus = CFGAS_Char::BreakType::kNone;
  }
}

size_t CFGAS_RTFBreak::GetDisplayPos(const CFGAS_TextPiece* pPiece,
                                     std::vector<TextCharPos>* pCharPos) const {
  if (pPiece->iChars == 0 || !pPiece->pFont)
    return 0;

  RetainPtr<CFGAS_GEFont> pFont = pPiece->pFont;
  CFX_RectF rtText(pPiece->rtPiece);
  const bool bRTLPiece = FX_IsOdd(pPiece->iBidiLevel);
  const float fFontSize = pPiece->fFontSize;
  const int32_t iFontSize = FXSYS_roundf(fFontSize * 20.0f);
  if (iFontSize == 0)
    return 0;

  const int32_t iAscent = pFont->GetAscent();
  const int32_t iDescent = pFont->GetDescent();
  const int32_t iMaxHeight = iAscent - iDescent;
  const float fAscent = iMaxHeight ? fFontSize * iAscent / iMaxHeight : 0;
  wchar_t wPrev = pdfium::unicode::kZeroWidthNoBreakSpace;
  wchar_t wNext;
  float fX = rtText.left;
  int32_t iHorScale = pPiece->iHorScale;
  int32_t iVerScale = pPiece->iVerScale;
  if (bRTLPiece)
    fX = rtText.right();

  float fY = rtText.top + fAscent;
  size_t szCount = 0;
  for (int32_t i = 0; i < pPiece->iChars; ++i) {
    TextCharPos& current_char_pos = (*pCharPos)[szCount];
    wchar_t wch = pPiece->szText[i];
    int32_t iWidth = pPiece->Widths[i];
    FX_CHARTYPE dwCharType = pdfium::unicode::GetCharType(wch);
    if (iWidth == 0) {
      if (dwCharType == FX_CHARTYPE::kArabicAlef)
        wPrev = pdfium::unicode::kZeroWidthNoBreakSpace;
      continue;
    }

    int iCharWidth = abs(iWidth);
    const bool bEmptyChar = (dwCharType >= FX_CHARTYPE::kTab &&
                             dwCharType <= FX_CHARTYPE::kControl);
    if (!bEmptyChar)
      ++szCount;

    iCharWidth /= iFontSize;
    wchar_t wForm = wch;
    if (dwCharType >= FX_CHARTYPE::kArabicAlef) {
      if (i + 1 < pPiece->iChars) {
        wNext = pPiece->szText[i + 1];
        if (pPiece->Widths[i + 1] < 0 && i + 2 < pPiece->iChars)
          wNext = pPiece->szText[i + 2];
      } else {
        wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
      }
      wForm = pdfium::arabic::GetFormChar(wch, wPrev, wNext);
    } else if (bRTLPiece) {
      wForm = pdfium::unicode::GetMirrorChar(wch);
    }

    if (!bEmptyChar) {
      current_char_pos.m_GlyphIndex = pFont->GetGlyphIndex(wForm);
      if (current_char_pos.m_GlyphIndex == 0xFFFF)
        current_char_pos.m_GlyphIndex = pFont->GetGlyphIndex(wch);
#if BUILDFLAG(IS_APPLE)
      current_char_pos.m_ExtGID = current_char_pos.m_GlyphIndex;
#endif
      current_char_pos.m_FontCharWidth = iCharWidth;
    }

    float fCharWidth = fFontSize * iCharWidth / 1000.0f;
    if (bRTLPiece && dwCharType != FX_CHARTYPE::kCombination)
      fX -= fCharWidth;

    if (!bEmptyChar)
      current_char_pos.m_Origin = CFX_PointF(fX, fY);
    if (!bRTLPiece && dwCharType != FX_CHARTYPE::kCombination)
      fX += fCharWidth;

    if (!bEmptyChar) {
      current_char_pos.m_bGlyphAdjust = true;
      current_char_pos.m_AdjustMatrix[0] = -1;
      current_char_pos.m_AdjustMatrix[1] = 0;
      current_char_pos.m_AdjustMatrix[2] = 0;
      current_char_pos.m_AdjustMatrix[3] = 1;
      current_char_pos.m_Origin.y += fAscent * iVerScale / 100.0f;
      current_char_pos.m_Origin.y -= fAscent;

      if (iHorScale != 100 || iVerScale != 100) {
        current_char_pos.m_AdjustMatrix[0] =
            current_char_pos.m_AdjustMatrix[0] * iHorScale / 100.0f;
        current_char_pos.m_AdjustMatrix[1] =
            current_char_pos.m_AdjustMatrix[1] * iHorScale / 100.0f;
        current_char_pos.m_AdjustMatrix[2] =
            current_char_pos.m_AdjustMatrix[2] * iVerScale / 100.0f;
        current_char_pos.m_AdjustMatrix[3] =
            current_char_pos.m_AdjustMatrix[3] * iVerScale / 100.0f;
      }
    }
    if (iWidth > 0)
      wPrev = wch;
  }
  return szCount;
}
