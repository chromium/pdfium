// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_txtbreak.h"

#include <algorithm>

#include "build/build_config.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/adapters.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_char.h"
#include "xfa/fgas/layout/fgas_arabic.h"
#include "xfa/fgas/layout/fgas_linebreak.h"

namespace {

struct FX_FORMCHAR {
  uint16_t wch;
  uint16_t wForm;
  int32_t iWidth;
};

bool IsCtrlCode(wchar_t wch) {
  FX_CHARTYPE dwRet = pdfium::unicode::GetCharType(wch);
  return dwRet == FX_CHARTYPE::kTab || dwRet == FX_CHARTYPE::kControl;
}

}  // namespace

CFGAS_TxtBreak::CFGAS_TxtBreak() : CFGAS_Break(LayoutStyle::kNone) {}

CFGAS_TxtBreak::~CFGAS_TxtBreak() = default;

void CFGAS_TxtBreak::SetLineWidth(float fLineWidth) {
  m_iLineWidth = FXSYS_roundf(fLineWidth * kConversionFactor);
  DCHECK(m_iLineWidth >= 20000);
}

void CFGAS_TxtBreak::SetAlignment(int32_t iAlignment) {
  DCHECK(iAlignment >= CFX_TxtLineAlignment_Left);
  DCHECK(iAlignment <= CFX_TxtLineAlignment_Justified);
  m_iAlignment = iAlignment;
}

void CFGAS_TxtBreak::SetCombWidth(float fCombWidth) {
  m_iCombWidth = FXSYS_roundf(fCombWidth * kConversionFactor);
}

void CFGAS_TxtBreak::AppendChar_Combination(CFGAS_Char* pCurChar) {
  FX_SAFE_INT32 iCharWidth = m_iCombWidth;
  pCurChar->m_iCharWidth = -1;
  if (!m_bCombText) {
    wchar_t wch = pCurChar->char_code();
    CFGAS_Char* pLastChar = GetLastChar(0, false, false);
    if (pLastChar &&
        (pLastChar->m_dwCharStyles & FX_TXTCHARSTYLE_ArabicShadda) == 0) {
      wchar_t wLast = pLastChar->char_code();
      absl::optional<uint16_t> maybe_shadda;
      if (wch == pdfium::arabic::kArabicShadda) {
        maybe_shadda = pdfium::arabic::GetArabicFromShaddaTable(wLast);
      } else if (wLast == pdfium::arabic::kArabicShadda) {
        maybe_shadda = pdfium::arabic::GetArabicFromShaddaTable(wch);
      }
      if (maybe_shadda.has_value()) {
        wch = maybe_shadda.value();
        pCurChar->m_dwCharStyles |= FX_TXTCHARSTYLE_ArabicShadda;
        pLastChar->m_dwCharStyles |= FX_TXTCHARSTYLE_ArabicShadda;
        pLastChar->m_iCharWidth = 0;
      }
    }
    absl::optional<uint16_t> iCharWidthRet;
    if (m_pFont) {
      iCharWidthRet = m_pFont->GetCharWidth(wch);
    }
    iCharWidth = iCharWidthRet.value_or(0);
    iCharWidth *= m_iFontSize;
    iCharWidth *= m_iHorizontalScale;
    iCharWidth /= 100;
  }
  iCharWidth *= -1;
  pCurChar->m_iCharWidth = iCharWidth.ValueOrDefault(0);
}

void CFGAS_TxtBreak::AppendChar_Tab(CFGAS_Char* pCurChar) {
  m_eCharType = FX_CHARTYPE::kTab;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar_Control(CFGAS_Char* pCurChar) {
  m_eCharType = FX_CHARTYPE::kControl;
  CFGAS_Char::BreakType dwRet = CFGAS_Char::BreakType::kNone;
  if (!m_bSingleLine) {
    wchar_t wch = pCurChar->char_code();
    switch (wch) {
      case L'\v':
      case pdfium::unicode::kLineSeparator:
        dwRet = CFGAS_Char::BreakType::kLine;
        break;
      case L'\f':
        dwRet = CFGAS_Char::BreakType::kPage;
        break;
      case pdfium::unicode::kParagraphSeparator:
        dwRet = CFGAS_Char::BreakType::kParagraph;
        break;
      default:
        if (wch == m_wParagraphBreakChar)
          dwRet = CFGAS_Char::BreakType::kParagraph;
        break;
    }
    if (dwRet != CFGAS_Char::BreakType::kNone)
      dwRet = EndBreak(dwRet);
  }
  return dwRet;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar_Arabic(CFGAS_Char* pCurChar) {
  FX_CHARTYPE chartype = pCurChar->GetCharType();
  int32_t& iLineWidth = m_pCurLine->m_iWidth;
  wchar_t wForm;
  CFGAS_Char* pLastChar = nullptr;
  bool bAlef = false;
  if (!m_bCombText && m_eCharType >= FX_CHARTYPE::kArabicAlef &&
      m_eCharType <= FX_CHARTYPE::kArabicDistortion) {
    FX_SAFE_INT32 iCharWidth = 0;
    pLastChar = GetLastChar(1, true, false);
    if (pLastChar) {
      if (pLastChar->m_iCharWidth > 0)
        iLineWidth -= pLastChar->m_iCharWidth;
      iCharWidth = pLastChar->m_iCharWidth;

      CFGAS_Char* pPrevChar = GetLastChar(2, true, false);
      wForm = pdfium::arabic::GetFormChar(pLastChar, pPrevChar, pCurChar);
      bAlef = (wForm == pdfium::unicode::kZeroWidthNoBreakSpace &&
               pLastChar->GetCharType() == FX_CHARTYPE::kArabicAlef);
      if (m_pFont) {
        iCharWidth = m_pFont->GetCharWidth(wForm).value_or(0);
      }
      if (wForm == pdfium::unicode::kZeroWidthNoBreakSpace)
        iCharWidth = 0;

      iCharWidth *= m_iFontSize;
      iCharWidth *= m_iHorizontalScale;
      iCharWidth /= 100;

      int32_t iCharWidthValid = iCharWidth.ValueOrDefault(0);
      pLastChar->m_iCharWidth = iCharWidthValid;
      iLineWidth += iCharWidthValid;
    }
  }

  m_eCharType = chartype;
  wForm = pdfium::arabic::GetFormChar(pCurChar, bAlef ? nullptr : pLastChar,
                                      nullptr);
  FX_SAFE_INT32 iCharWidth = 0;
  if (m_bCombText) {
    iCharWidth = m_iCombWidth;
  } else {
    if (m_pFont && wForm != pdfium::unicode::kZeroWidthNoBreakSpace) {
      iCharWidth = m_pFont->GetCharWidth(wForm).value_or(0);
    }
    iCharWidth *= m_iFontSize;
    iCharWidth *= m_iHorizontalScale;
    iCharWidth /= 100;
  }

  int32_t iCharWidthValid = iCharWidth.ValueOrDefault(0);
  pCurChar->m_iCharWidth = iCharWidthValid;
  iLineWidth += iCharWidthValid;

  m_pCurLine->IncrementArabicCharCount();
  if (!m_bSingleLine && IsGreaterThanLineWidth(iLineWidth))
    return EndBreak(CFGAS_Char::BreakType::kLine);
  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar_Others(CFGAS_Char* pCurChar) {
  FX_CHARTYPE chartype = pCurChar->GetCharType();
  int32_t& iLineWidth = m_pCurLine->m_iWidth;
  m_eCharType = chartype;
  wchar_t wch = pCurChar->char_code();
  wchar_t wForm = wch;

  FX_SAFE_INT32 iCharWidth = 0;
  if (m_bCombText) {
    iCharWidth = m_iCombWidth;
  } else if (m_pFont) {
    iCharWidth = m_pFont->GetCharWidth(wForm).value_or(0);
    iCharWidth *= m_iFontSize;
    iCharWidth *= m_iHorizontalScale;
    iCharWidth /= 100;
  }
  iCharWidth += m_iCharSpace;

  int32_t iValidCharWidth = iCharWidth.ValueOrDefault(0);
  pCurChar->m_iCharWidth = iValidCharWidth;
  iLineWidth += iValidCharWidth;
  if (!m_bSingleLine && chartype != FX_CHARTYPE::kSpace &&
      IsGreaterThanLineWidth(iLineWidth)) {
    return EndBreak(CFGAS_Char::BreakType::kLine);
  }

  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar(wchar_t wch) {
  FX_CHARTYPE chartype = pdfium::unicode::GetCharType(wch);
  m_pCurLine->m_LineChars.emplace_back(wch, m_iHorizontalScale,
                                       m_iVerticalScale);
  CFGAS_Char* pCurChar = &m_pCurLine->m_LineChars.back();
  pCurChar->m_dwCharStyles = m_iAlignment | (1 << 8);

  CFGAS_Char::BreakType dwRet1 = CFGAS_Char::BreakType::kNone;
  if (chartype != FX_CHARTYPE::kCombination &&
      GetUnifiedCharType(m_eCharType) != GetUnifiedCharType(chartype) &&
      m_eCharType != FX_CHARTYPE::kUnknown && !m_bSingleLine &&
      IsGreaterThanLineWidth(m_pCurLine->m_iWidth) &&
      (m_eCharType != FX_CHARTYPE::kSpace ||
       chartype != FX_CHARTYPE::kControl)) {
    dwRet1 = EndBreak(CFGAS_Char::BreakType::kLine);
    if (!m_pCurLine->m_LineChars.empty())
      pCurChar = &m_pCurLine->m_LineChars.back();
  }

  CFGAS_Char::BreakType dwRet2 = CFGAS_Char::BreakType::kNone;
  if (wch == m_wParagraphBreakChar) {
    // This is handled in AppendChar_Control, but it seems like \n and \r
    // don't get matched as control characters so we go into AppendChar_other
    // and never detect the new paragraph ...
    dwRet2 = CFGAS_Char::BreakType::kParagraph;
    EndBreak(dwRet2);
  } else {
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
  }
  return std::max(dwRet1, dwRet2);
}

void CFGAS_TxtBreak::EndBreakSplitLine(CFGAS_BreakLine* pNextLine,
                                       bool bAllChars) {
  bool bDone = false;
  CFGAS_Char* pTC;
  if (!m_bSingleLine && IsGreaterThanLineWidth(m_pCurLine->m_iWidth)) {
    pTC = m_pCurLine->GetChar(m_pCurLine->m_LineChars.size() - 1);
    switch (pTC->GetCharType()) {
      case FX_CHARTYPE::kTab:
      case FX_CHARTYPE::kControl:
      case FX_CHARTYPE::kSpace:
        break;
      default:
        SplitTextLine(m_pCurLine.Get(), pNextLine, bAllChars);
        bDone = true;
        break;
    }
  }
  if (bAllChars && !bDone) {
    int32_t iEndPos = m_pCurLine->m_iWidth;
    GetBreakPos(&m_pCurLine->m_LineChars, bAllChars, true, &iEndPos);
  }
}

std::deque<CFGAS_Break::TPO> CFGAS_TxtBreak::EndBreakBidiLine(
    CFGAS_Char::BreakType dwStatus) {
  CFGAS_BreakPiece tp;
  std::deque<TPO> tpos;
  CFGAS_Char* pTC;
  std::vector<CFGAS_Char>& chars = m_pCurLine->m_LineChars;
  if (!m_pCurLine->HasArabicChar()) {
    tp.m_dwStatus = dwStatus;
    tp.m_iStartPos = m_pCurLine->m_iStart;
    tp.m_iWidth = m_pCurLine->m_iWidth;
    tp.m_iStartChar = 0;
    tp.m_iCharCount = m_pCurLine->m_LineChars.size();
    tp.m_pChars = &m_pCurLine->m_LineChars;
    pTC = &chars[0];
    tp.m_dwCharStyles = pTC->m_dwCharStyles;
    tp.m_iHorizontalScale = pTC->horizonal_scale();
    tp.m_iVerticalScale = pTC->vertical_scale();
    m_pCurLine->m_LinePieces.push_back(tp);
    tpos.push_back({0, 0});
    return tpos;
  }

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

  tp.m_dwStatus = CFGAS_Char::BreakType::kPiece;
  tp.m_iStartPos = m_pCurLine->m_iStart;
  tp.m_pChars = &m_pCurLine->m_LineChars;
  int32_t iBidiLevel = -1;
  int32_t iCharWidth;
  int32_t i = 0;
  int32_t j = -1;
  int32_t iCount = fxcrt::CollectionSize<int32_t>(m_pCurLine->m_LineChars);
  while (i < iCount) {
    pTC = &chars[i];
    if (iBidiLevel < 0) {
      iBidiLevel = pTC->m_iBidiLevel;
      tp.m_iWidth = 0;
      tp.m_iBidiLevel = iBidiLevel;
      tp.m_iBidiPos = pTC->m_iBidiOrder;
      tp.m_dwCharStyles = pTC->m_dwCharStyles;
      tp.m_iHorizontalScale = pTC->horizonal_scale();
      tp.m_iVerticalScale = pTC->vertical_scale();
      tp.m_dwStatus = CFGAS_Char::BreakType::kPiece;
    }
    if (iBidiLevel != pTC->m_iBidiLevel ||
        pTC->m_dwStatus != CFGAS_Char::BreakType::kNone) {
      if (iBidiLevel == pTC->m_iBidiLevel) {
        tp.m_dwStatus = pTC->m_dwStatus;
        iCharWidth = pTC->m_iCharWidth;
        if (iCharWidth > 0)
          tp.m_iWidth += iCharWidth;

        i++;
      }
      tp.m_iCharCount = i - tp.m_iStartChar;
      m_pCurLine->m_LinePieces.push_back(tp);
      tp.m_iStartPos += tp.m_iWidth;
      tp.m_iStartChar = i;
      tpos.push_back({++j, tp.m_iBidiPos});
      iBidiLevel = -1;
    } else {
      iCharWidth = pTC->m_iCharWidth;
      if (iCharWidth > 0)
        tp.m_iWidth += iCharWidth;

      i++;
    }
  }
  if (i > tp.m_iStartChar) {
    tp.m_dwStatus = dwStatus;
    tp.m_iCharCount = i - tp.m_iStartChar;
    m_pCurLine->m_LinePieces.push_back(tp);
    tpos.push_back({++j, tp.m_iBidiPos});
  }
  if (j > -1) {
    if (j > 0) {
      std::sort(tpos.begin(), tpos.end());
      int32_t iStartPos = 0;
      for (i = 0; i <= j; i++) {
        CFGAS_BreakPiece& ttp = m_pCurLine->m_LinePieces[tpos[i].index];
        ttp.m_iStartPos = iStartPos;
        iStartPos += ttp.m_iWidth;
      }
    }
    m_pCurLine->m_LinePieces[j].m_dwStatus = dwStatus;
  }
  return tpos;
}

void CFGAS_TxtBreak::EndBreakAlignment(const std::deque<TPO>& tpos,
                                       bool bAllChars,
                                       CFGAS_Char::BreakType dwStatus) {
  int32_t iNetWidth = m_pCurLine->m_iWidth;
  int32_t iGapChars = 0;
  bool bFind = false;
  for (const TPO& pos : pdfium::base::Reversed(tpos)) {
    const CFGAS_BreakPiece& ttp = m_pCurLine->m_LinePieces[pos.index];
    if (!bFind)
      iNetWidth = ttp.GetEndPos();

    bool bArabic = FX_IsOdd(ttp.m_iBidiLevel);
    int32_t j = bArabic ? 0 : ttp.m_iCharCount - 1;
    while (j > -1 && j < ttp.m_iCharCount) {
      const CFGAS_Char* pTC = ttp.GetChar(j);
      if (pTC->m_eLineBreakType == FX_LINEBREAKTYPE::kDIRECT_BRK)
        iGapChars++;
      if (!bFind || !bAllChars) {
        FX_CHARTYPE chartype = pTC->GetCharType();
        if (chartype == FX_CHARTYPE::kSpace ||
            chartype == FX_CHARTYPE::kControl) {
          if (!bFind && bAllChars && pTC->m_iCharWidth > 0)
            iNetWidth -= pTC->m_iCharWidth;
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
  if (iGapChars > 0 && m_iAlignment & CFX_TxtLineAlignment_Justified &&
      dwStatus != CFGAS_Char::BreakType::kParagraph) {
    int32_t iStart = -1;
    for (auto& tpo : tpos) {
      CFGAS_BreakPiece& ttp = m_pCurLine->m_LinePieces[tpo.index];
      if (iStart < -1)
        iStart = ttp.m_iStartPos;
      else
        ttp.m_iStartPos = iStart;

      for (int32_t j = 0; j < ttp.m_iCharCount && iGapChars > 0;
           j++, iGapChars--) {
        CFGAS_Char* pTC = ttp.GetChar(j);
        if (pTC->m_eLineBreakType != FX_LINEBREAKTYPE::kDIRECT_BRK ||
            pTC->m_iCharWidth < 0) {
          continue;
        }
        int32_t k = iOffset / iGapChars;
        pTC->m_iCharWidth += k;
        ttp.m_iWidth += k;
        iOffset -= k;
      }
      iStart += ttp.m_iWidth;
    }
  } else if (m_iAlignment & CFX_TxtLineAlignment_Center ||
             m_iAlignment & CFX_TxtLineAlignment_Right) {
    if (m_iAlignment & CFX_TxtLineAlignment_Center &&
        !(m_iAlignment & CFX_TxtLineAlignment_Right)) {
      iOffset /= 2;
    }
    if (iOffset > 0) {
      for (auto& ttp : m_pCurLine->m_LinePieces)
        ttp.m_iStartPos += iOffset;
    }
  }
}

CFGAS_Char::BreakType CFGAS_TxtBreak::EndBreak(CFGAS_Char::BreakType dwStatus) {
  DCHECK(dwStatus != CFGAS_Char::BreakType::kNone);

  if (!m_pCurLine->m_LinePieces.empty()) {
    if (dwStatus != CFGAS_Char::BreakType::kPiece)
      m_pCurLine->m_LinePieces.back().m_dwStatus = dwStatus;
    return m_pCurLine->m_LinePieces.back().m_dwStatus;
  }

  if (HasLine()) {
    if (m_Lines[m_iReadyLineIndex].m_LinePieces.empty())
      return CFGAS_Char::BreakType::kNone;

    if (dwStatus != CFGAS_Char::BreakType::kPiece)
      m_Lines[m_iReadyLineIndex].m_LinePieces.back().m_dwStatus = dwStatus;
    return m_Lines[m_iReadyLineIndex].m_LinePieces.back().m_dwStatus;
  }

  if (m_pCurLine->m_LineChars.empty())
    return CFGAS_Char::BreakType::kNone;

  m_pCurLine->m_LineChars.back().m_dwStatus = dwStatus;
  if (dwStatus == CFGAS_Char::BreakType::kPiece)
    return dwStatus;

  m_iReadyLineIndex = m_pCurLine == &m_Lines[0] ? 0 : 1;
  CFGAS_BreakLine* pNextLine = &m_Lines[1 - m_iReadyLineIndex];
  const bool bAllChars = m_iAlignment > CFX_TxtLineAlignment_Right;
  EndBreakSplitLine(pNextLine, bAllChars);

  std::deque<TPO> tpos = EndBreakBidiLine(dwStatus);
  if (m_iAlignment > CFX_TxtLineAlignment_Left)
    EndBreakAlignment(tpos, bAllChars, dwStatus);

  m_pCurLine = pNextLine;
  CFGAS_Char* pTC = GetLastChar(0, false, false);
  m_eCharType = pTC ? pTC->GetCharType() : FX_CHARTYPE::kUnknown;
  return dwStatus;
}

int32_t CFGAS_TxtBreak::GetBreakPos(std::vector<CFGAS_Char>* pChars,
                                    bool bAllChars,
                                    bool bOnlyBrk,
                                    int32_t* pEndPos) {
  std::vector<CFGAS_Char>& chars = *pChars;
  int32_t iLength = fxcrt::CollectionSize<int32_t>(chars) - 1;
  if (iLength < 1)
    return iLength;

  int32_t iBreak = -1;
  int32_t iBreakPos = -1;
  int32_t iIndirect = -1;
  int32_t iIndirectPos = -1;
  int32_t iLast = -1;
  int32_t iLastPos = -1;
  if (m_bSingleLine || *pEndPos <= m_iLineWidth) {
    if (!bAllChars)
      return iLength;

    iBreak = iLength;
    iBreakPos = *pEndPos;
  }

  FX_LINEBREAKTYPE eType;
  FX_BREAKPROPERTY nCur;
  FX_BREAKPROPERTY nNext;
  CFGAS_Char* pCur = &chars[iLength--];
  if (bAllChars)
    pCur->m_eLineBreakType = FX_LINEBREAKTYPE::kUNKNOWN;

  nNext = pdfium::unicode::GetBreakProperty(pCur->char_code());
  int32_t iCharWidth = pCur->m_iCharWidth;
  if (iCharWidth > 0)
    *pEndPos -= iCharWidth;

  while (iLength >= 0) {
    pCur = &chars[iLength];
    nCur = pdfium::unicode::GetBreakProperty(pCur->char_code());
    if (nNext == FX_BREAKPROPERTY::kSP)
      eType = FX_LINEBREAKTYPE::kPROHIBITED_BRK;
    else
      eType = GetLineBreakTypeFromPair(nCur, nNext);
    if (bAllChars)
      pCur->m_eLineBreakType = eType;
    if (!bOnlyBrk) {
      if (m_bSingleLine || *pEndPos <= m_iLineWidth ||
          nCur == FX_BREAKPROPERTY::kSP) {
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
      iCharWidth = pCur->m_iCharWidth;
      if (iCharWidth > 0)
        *pEndPos -= iCharWidth;
    }
    nNext = nCur;
    iLength--;
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

void CFGAS_TxtBreak::SplitTextLine(CFGAS_BreakLine* pCurLine,
                                   CFGAS_BreakLine* pNextLine,
                                   bool bAllChars) {
  DCHECK(pCurLine);
  DCHECK(pNextLine);

  if (pCurLine->m_LineChars.size() < 2)
    return;

  int32_t iEndPos = pCurLine->m_iWidth;
  std::vector<CFGAS_Char>& curChars = pCurLine->m_LineChars;
  int32_t iCharPos = GetBreakPos(&curChars, bAllChars, false, &iEndPos);
  if (iCharPos < 0)
    iCharPos = 0;

  iCharPos++;
  if (iCharPos >= fxcrt::CollectionSize<int32_t>(pCurLine->m_LineChars)) {
    pNextLine->Clear();
    CFGAS_Char* pTC = &curChars[iCharPos - 1];
    pTC->m_eLineBreakType = FX_LINEBREAKTYPE::kUNKNOWN;
    return;
  }

  pNextLine->m_LineChars =
      std::vector<CFGAS_Char>(curChars.begin() + iCharPos, curChars.end());
  curChars.erase(curChars.begin() + iCharPos, curChars.end());
  pCurLine->m_iWidth = iEndPos;
  CFGAS_Char* pTC = &curChars[iCharPos - 1];
  pTC->m_eLineBreakType = FX_LINEBREAKTYPE::kUNKNOWN;
  int32_t iWidth = 0;
  for (size_t i = 0; i < pNextLine->m_LineChars.size(); ++i) {
    if (pNextLine->m_LineChars[i].GetCharType() >= FX_CHARTYPE::kArabicAlef) {
      pCurLine->DecrementArabicCharCount();
      pNextLine->IncrementArabicCharCount();
    }
    iWidth += std::max(0, pNextLine->m_LineChars[i].m_iCharWidth);
    pNextLine->m_LineChars[i].m_dwStatus = CFGAS_Char::BreakType::kNone;
  }
  pNextLine->m_iWidth = iWidth;
}

size_t CFGAS_TxtBreak::GetDisplayPos(const Run& run,
                                     TextCharPos* pCharPos) const {
  if (run.iLength < 1)
    return 0;

  Engine* pEngine = run.pEdtEngine;
  const wchar_t* pStr = run.wsStr.c_str();
  int32_t* pWidths = run.pWidths;
  int32_t iLength = run.iLength - 1;
  RetainPtr<CFGAS_GEFont> pFont = run.pFont;
  Mask<LayoutStyle> dwStyles = run.dwStyles;
  CFX_RectF rtText(*run.pRect);
  bool bRTLPiece = (run.dwCharStyles & FX_TXTCHARSTYLE_OddBidiLevel) != 0;
  float fFontSize = run.fFontSize;
  int32_t iFontSize = FXSYS_roundf(fFontSize * 20.0f);
  int32_t iAscent = pFont->GetAscent();
  int32_t iDescent = pFont->GetDescent();
  int32_t iMaxHeight = iAscent - iDescent;
  float fFontHeight = fFontSize;
  float fAscent = fFontHeight * iAscent / iMaxHeight;
  float fX = rtText.left;
  float fY;
  float fCharWidth;
  int32_t iHorScale = run.iHorizontalScale;
  int32_t iVerScale = run.iVerticalScale;
  bool bSkipSpace = run.bSkipSpace;
  FX_FORMCHAR formChars[3];
  float fYBase;

  if (bRTLPiece)
    fX = rtText.right();

  fYBase = rtText.top + (rtText.height - fFontSize) / 2.0f;
  fY = fYBase + fAscent;

  size_t szCount = 0;
  int32_t iNext = 0;
  wchar_t wPrev = pdfium::unicode::kZeroWidthNoBreakSpace;
  wchar_t wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
  wchar_t wForm = pdfium::unicode::kZeroWidthNoBreakSpace;
  wchar_t wLast = pdfium::unicode::kZeroWidthNoBreakSpace;
  bool bShadda = false;
  bool bLam = false;
  for (int32_t i = 0; i <= iLength; i++) {
    int32_t iAbsolute = i + run.iStart;
    int32_t iWidth;
    wchar_t wch;
    if (pEngine) {
      wch = pEngine->GetChar(iAbsolute);
      iWidth = pEngine->GetWidthOfChar(iAbsolute);
    } else {
      wch = *pStr++;
      iWidth = *pWidths++;
    }

    FX_CHARTYPE chartype = pdfium::unicode::GetCharType(wch);
    if (chartype == FX_CHARTYPE::kArabicAlef && iWidth == 0) {
      wPrev = pdfium::unicode::kZeroWidthNoBreakSpace;
      wLast = wch;
      continue;
    }

    if (chartype >= FX_CHARTYPE::kArabicAlef) {
      if (i < iLength) {
        if (pEngine) {
          iNext = i + 1;
          while (iNext <= iLength) {
            int32_t iNextAbsolute = iNext + run.iStart;
            wNext = pEngine->GetChar(iNextAbsolute);
            if (pdfium::unicode::GetCharType(wNext) !=
                FX_CHARTYPE::kCombination) {
              break;
            }
            iNext++;
          }
          if (iNext > iLength)
            wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
        } else {
          int32_t j = -1;
          do {
            j++;
            if (i + j >= iLength)
              break;

            wNext = pStr[j];
          } while (pdfium::unicode::GetCharType(wNext) ==
                   FX_CHARTYPE::kCombination);
          if (i + j >= iLength)
            wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
        }
      } else {
        wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
      }

      wForm = pdfium::arabic::GetFormChar(wch, wPrev, wNext);
      bLam = (wPrev == pdfium::arabic::kArabicLetterLam &&
              wch == pdfium::arabic::kArabicLetterLam &&
              wNext == pdfium::arabic::kArabicLetterHeh);
    } else if (chartype == FX_CHARTYPE::kCombination) {
      wForm = wch;
      if (wch >= 0x064C && wch <= 0x0651) {
        if (bShadda) {
          wForm = pdfium::unicode::kZeroWidthNoBreakSpace;
          bShadda = false;
        } else {
          wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
          if (pEngine) {
            iNext = i + 1;
            if (iNext <= iLength) {
              int32_t iNextAbsolute = iNext + run.iStart;
              wNext = pEngine->GetChar(iNextAbsolute);
            }
          } else if (i < iLength) {
            wNext = *pStr;
          }
          absl::optional<uint16_t> maybe_shadda;
          if (wch == pdfium::arabic::kArabicShadda) {
            maybe_shadda = pdfium::arabic::GetArabicFromShaddaTable(wNext);
          } else if (wNext == pdfium::arabic::kArabicShadda) {
            maybe_shadda = pdfium::arabic::GetArabicFromShaddaTable(wch);
          }
          if (maybe_shadda.has_value()) {
            wForm = maybe_shadda.value();
            bShadda = true;
          }
        }
      } else {
        bShadda = false;
      }
    } else if (chartype == FX_CHARTYPE::kNumeric) {
      wForm = wch;
    } else if (wch == L'.') {
      wForm = wch;
    } else if (wch == L',') {
      wForm = wch;
    } else if (bRTLPiece) {
      wForm = pdfium::unicode::GetMirrorChar(wch);
    } else {
      wForm = wch;
    }
    if (chartype != FX_CHARTYPE::kCombination)
      bShadda = false;
    if (chartype < FX_CHARTYPE::kArabicAlef)
      bLam = false;

    bool bEmptyChar =
        (chartype >= FX_CHARTYPE::kTab && chartype <= FX_CHARTYPE::kControl);
    if (wForm == pdfium::unicode::kZeroWidthNoBreakSpace)
      bEmptyChar = true;

    int32_t iForms = bLam ? 3 : 1;
    szCount += (bEmptyChar && bSkipSpace) ? 0 : iForms;
    if (!pCharPos) {
      if (iWidth > 0)
        wPrev = wch;
      wLast = wch;
      continue;
    }

    int32_t iCharWidth = iWidth;
    if (iCharWidth < 0)
      iCharWidth = -iCharWidth;

    iCharWidth /= iFontSize;
    formChars[0].wch = wch;
    formChars[0].wForm = wForm;
    formChars[0].iWidth = iCharWidth;
    if (bLam) {
      formChars[1].wForm = pdfium::arabic::kArabicShadda;
      formChars[1].iWidth =
          pFont->GetCharWidth(pdfium::arabic::kArabicShadda).value_or(0);
      formChars[2].wForm = pdfium::arabic::kArabicLetterSuperscriptAlef;
      formChars[2].iWidth =
          pFont->GetCharWidth(pdfium::arabic::kArabicLetterSuperscriptAlef)
              .value_or(0);
    }

    for (int32_t j = 0; j < iForms; j++) {
      wForm = (wchar_t)formChars[j].wForm;
      iCharWidth = formChars[j].iWidth;
      if (j > 0) {
        chartype = FX_CHARTYPE::kCombination;
        wch = wForm;
        wLast = (wchar_t)formChars[j - 1].wForm;
      }
      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        pCharPos->m_GlyphIndex = pFont->GetGlyphIndex(wForm);
#if BUILDFLAG(IS_APPLE)
        pCharPos->m_ExtGID = pCharPos->m_GlyphIndex;
#endif
        pCharPos->m_FontCharWidth = iCharWidth;
      }

      fCharWidth = fFontSize * iCharWidth / 1000.0f;
      if (bRTLPiece && chartype != FX_CHARTYPE::kCombination)
        fX -= fCharWidth;

      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        pCharPos->m_Origin = CFX_PointF(fX, fY);

        if (!!(dwStyles & LayoutStyle::kCombText)) {
          int32_t iFormWidth = pFont->GetCharWidth(wForm).value_or(iCharWidth);
          float fOffset = fFontSize * (iCharWidth - iFormWidth) / 2000.0f;
          pCharPos->m_Origin.x += fOffset;
        }
        if (chartype == FX_CHARTYPE::kCombination) {
          absl::optional<FX_RECT> rtBBox = pFont->GetCharBBox(wForm);
          if (rtBBox.has_value()) {
            pCharPos->m_Origin.y =
                fYBase + fFontSize -
                fFontSize * rtBBox.value().Height() / iMaxHeight;
          }
          if (wForm == wch &&
              wLast != pdfium::unicode::kZeroWidthNoBreakSpace) {
            if (pdfium::unicode::GetCharType(wLast) ==
                FX_CHARTYPE::kCombination) {
              absl::optional<FX_RECT> rtOtherBox = pFont->GetCharBBox(wLast);
              if (rtOtherBox.has_value()) {
                pCharPos->m_Origin.y -=
                    fFontSize * rtOtherBox.value().Height() / iMaxHeight;
              }
            }
          }
        }
      }
      if (!bRTLPiece && chartype != FX_CHARTYPE::kCombination)
        fX += fCharWidth;

      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        pCharPos->m_bGlyphAdjust = true;
        pCharPos->m_AdjustMatrix[0] = -1;
        pCharPos->m_AdjustMatrix[1] = 0;
        pCharPos->m_AdjustMatrix[2] = 0;
        pCharPos->m_AdjustMatrix[3] = 1;

        if (iHorScale != 100 || iVerScale != 100) {
          pCharPos->m_AdjustMatrix[0] =
              pCharPos->m_AdjustMatrix[0] * iHorScale / 100.0f;
          pCharPos->m_AdjustMatrix[1] =
              pCharPos->m_AdjustMatrix[1] * iHorScale / 100.0f;
          pCharPos->m_AdjustMatrix[2] =
              pCharPos->m_AdjustMatrix[2] * iVerScale / 100.0f;
          pCharPos->m_AdjustMatrix[3] =
              pCharPos->m_AdjustMatrix[3] * iVerScale / 100.0f;
        }
        pCharPos++;
      }
    }
    if (iWidth > 0)
      wPrev = static_cast<wchar_t>(formChars[0].wch);
    wLast = wch;
  }
  return szCount;
}

std::vector<CFX_RectF> CFGAS_TxtBreak::GetCharRects(const Run& run) const {
  if (run.iLength < 1)
    return std::vector<CFX_RectF>();

  Engine* pEngine = run.pEdtEngine;
  const wchar_t* pStr = run.wsStr.c_str();
  int32_t* pWidths = run.pWidths;
  int32_t iLength = run.iLength;
  CFX_RectF rect(*run.pRect);
  float fFontSize = run.fFontSize;
  bool bRTLPiece = !!(run.dwCharStyles & FX_TXTCHARSTYLE_OddBidiLevel);
  bool bSingleLine = !!(run.dwStyles & LayoutStyle::kSingleLine);
  float fStart = bRTLPiece ? rect.right() : rect.left;

  std::vector<CFX_RectF> rtArray(iLength);
  for (int32_t i = 0; i < iLength; i++) {
    wchar_t wch;
    int32_t iCharSize;
    if (pEngine) {
      int32_t iAbsolute = i + run.iStart;
      wch = pEngine->GetChar(iAbsolute);
      iCharSize = pEngine->GetWidthOfChar(iAbsolute);
    } else {
      wch = *pStr++;
      iCharSize = *pWidths++;
    }
    float fCharSize = static_cast<float>(iCharSize) / kConversionFactor;
    bool bRet = (!bSingleLine && IsCtrlCode(wch));
    if (!(wch == L'\v' || wch == L'\f' ||
          wch == pdfium::unicode::kLineSeparator ||
          wch == pdfium::unicode::kParagraphSeparator || wch == L'\n')) {
      bRet = false;
    }
    if (bRet)
      fCharSize = fFontSize / 2.0f;
    rect.left = fStart;
    if (bRTLPiece) {
      rect.left -= fCharSize;
      fStart -= fCharSize;
    } else {
      fStart += fCharSize;
    }
    rect.width = fCharSize;
    rtArray[i] = rect;
  }
  return rtArray;
}

CFGAS_TxtBreak::Engine::~Engine() = default;

CFGAS_TxtBreak::Run::Run() = default;

CFGAS_TxtBreak::Run::~Run() = default;

CFGAS_TxtBreak::Run::Run(const CFGAS_TxtBreak::Run& other) = default;
