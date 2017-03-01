// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/fgas_rtfbreak.h"

#include <algorithm>

#include "core/fxcrt/fx_arabic.h"
#include "core/fxcrt/fx_arb.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/fgas_linebreak.h"

namespace {

typedef CFX_RTFBreakType (CFX_RTFBreak::*FX_RTFBreak_LPFAppendChar)(
    CFX_RTFChar* pCurChar);
const FX_RTFBreak_LPFAppendChar g_FX_RTFBreak_lpfAppendChar[16] = {
    &CFX_RTFBreak::AppendChar_Others,      &CFX_RTFBreak::AppendChar_Tab,
    &CFX_RTFBreak::AppendChar_Others,      &CFX_RTFBreak::AppendChar_Control,
    &CFX_RTFBreak::AppendChar_Combination, &CFX_RTFBreak::AppendChar_Others,
    &CFX_RTFBreak::AppendChar_Others,      &CFX_RTFBreak::AppendChar_Arabic,
    &CFX_RTFBreak::AppendChar_Arabic,      &CFX_RTFBreak::AppendChar_Arabic,
    &CFX_RTFBreak::AppendChar_Arabic,      &CFX_RTFBreak::AppendChar_Arabic,
    &CFX_RTFBreak::AppendChar_Arabic,      &CFX_RTFBreak::AppendChar_Others,
    &CFX_RTFBreak::AppendChar_Others,      &CFX_RTFBreak::AppendChar_Others,
};

}  // namespace

CFX_RTFBreak::CFX_RTFBreak(uint32_t dwLayoutStyles)
    : m_iBoundaryStart(0),
      m_iBoundaryEnd(2000000),
      m_dwLayoutStyles(dwLayoutStyles),
      m_bPagination(false),
      m_pFont(nullptr),
      m_iFontHeight(240),
      m_iFontSize(240),
      m_iTabWidth(720000),
      m_wDefChar(0xFEFF),
      m_iDefChar(0),
      m_wLineBreakChar(L'\n'),
      m_iHorizontalScale(100),
      m_iVerticalScale(100),
      m_iCharSpace(0),
      m_iAlignment(CFX_RTFLineAlignment::Left),
      m_pUserData(nullptr),
      m_eCharType(FX_CHARTYPE_Unknown),
      m_dwIdentity(0),
      m_RTFLine1(),
      m_RTFLine2(),
      m_pCurLine(nullptr),
      m_iReady(0),
      m_iTolerance(0) {
  m_pCurLine = &m_RTFLine1;

  SetBreakStatus();
  m_bPagination = (m_dwLayoutStyles & FX_RTFLAYOUTSTYLE_Pagination) != 0;
}

CFX_RTFBreak::~CFX_RTFBreak() {
  Reset();
}

void CFX_RTFBreak::SetLineBoundary(FX_FLOAT fLineStart, FX_FLOAT fLineEnd) {
  if (fLineStart > fLineEnd)
    return;

  m_iBoundaryStart = FXSYS_round(fLineStart * 20000.0f);
  m_iBoundaryEnd = FXSYS_round(fLineEnd * 20000.0f);
  m_pCurLine->m_iStart = std::min(m_pCurLine->m_iStart, m_iBoundaryEnd);
  m_pCurLine->m_iStart = std::max(m_pCurLine->m_iStart, m_iBoundaryStart);
}

void CFX_RTFBreak::SetLineStartPos(FX_FLOAT fLinePos) {
  int32_t iLinePos = FXSYS_round(fLinePos * 20000.0f);
  iLinePos = std::min(iLinePos, m_iBoundaryEnd);
  iLinePos = std::max(iLinePos, m_iBoundaryStart);
  m_pCurLine->m_iStart = iLinePos;
}

void CFX_RTFBreak::SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont) {
  if (!pFont || pFont == m_pFont)
    return;

  SetBreakStatus();
  m_pFont = pFont;
  FontChanged();
}

void CFX_RTFBreak::SetFontSize(FX_FLOAT fFontSize) {
  int32_t iFontSize = FXSYS_round(fFontSize * 20.0f);
  if (m_iFontSize == iFontSize)
    return;

  SetBreakStatus();
  m_iFontSize = iFontSize;
  FontChanged();
}

void CFX_RTFBreak::FontChanged() {
  m_iDefChar = 0;
  if (!m_pFont)
    return;

  m_iFontHeight = m_iFontSize;
  if (m_wDefChar == 0xFEFF)
    return;

  m_pFont->GetCharWidth(m_wDefChar, m_iDefChar, false);
  m_iDefChar *= m_iFontSize;
}

void CFX_RTFBreak::SetTabWidth(FX_FLOAT fTabWidth) {
  m_iTabWidth = FXSYS_round(fTabWidth * 20000.0f);
}

void CFX_RTFBreak::AddPositionedTab(FX_FLOAT fTabPos) {
  int32_t iTabPos = std::min(FXSYS_round(fTabPos * 20000.0f) + m_iBoundaryStart,
                             m_iBoundaryEnd);
  auto it = std::lower_bound(m_PositionedTabs.begin(), m_PositionedTabs.end(),
                             iTabPos);
  if (it != m_PositionedTabs.end() && *it == iTabPos)
    return;
  m_PositionedTabs.insert(it, iTabPos);
}

void CFX_RTFBreak::SetLineBreakTolerance(FX_FLOAT fTolerance) {
  m_iTolerance = FXSYS_round(fTolerance * 20000.0f);
}

void CFX_RTFBreak::SetHorizontalScale(int32_t iScale) {
  if (iScale < 0)
    iScale = 0;
  if (m_iHorizontalScale == iScale)
    return;

  SetBreakStatus();
  m_iHorizontalScale = iScale;
}

void CFX_RTFBreak::SetVerticalScale(int32_t iScale) {
  if (iScale < 0)
    iScale = 0;
  if (m_iVerticalScale == iScale)
    return;

  SetBreakStatus();
  m_iVerticalScale = iScale;
}

void CFX_RTFBreak::SetCharSpace(FX_FLOAT fCharSpace) {
  m_iCharSpace = FXSYS_round(fCharSpace * 20000.0f);
}

void CFX_RTFBreak::SetUserData(const CFX_RetainPtr<CFX_Retainable>& pUserData) {
  if (m_pUserData == pUserData)
    return;

  SetBreakStatus();
  m_pUserData = pUserData;
}

void CFX_RTFBreak::SetBreakStatus() {
  m_dwIdentity++;
  int32_t iCount = m_pCurLine->CountChars();
  if (iCount < 1)
    return;

  CFX_RTFChar& tc = m_pCurLine->GetChar(iCount - 1);
  if (tc.m_dwStatus == CFX_RTFBreakType::None)
    tc.m_dwStatus = CFX_RTFBreakType::Piece;
}

CFX_RTFChar* CFX_RTFBreak::GetLastChar(int32_t index) const {
  std::vector<CFX_RTFChar>& tca = m_pCurLine->m_LineChars;
  int32_t iCount = pdfium::CollectionSize<int32_t>(tca);
  if (index < 0 || index >= iCount)
    return nullptr;

  int32_t iStart = iCount - 1;
  while (iStart > -1) {
    CFX_RTFChar* pTC = &tca[iStart--];
    if (pTC->m_iCharWidth >= 0 ||
        pTC->GetCharType() != FX_CHARTYPE_Combination) {
      if (--index < 0)
        return pTC;
    }
  }
  return nullptr;
}

const CFX_RTFLine* CFX_RTFBreak::GetRTFLine() const {
  if (m_iReady == 1)
    return &m_RTFLine1;
  if (m_iReady == 2)
    return &m_RTFLine2;
  return nullptr;
}

const CFX_RTFPieceArray* CFX_RTFBreak::GetRTFPieces() const {
  const CFX_RTFLine* pRTFLine = GetRTFLine();
  return pRTFLine ? &pRTFLine->m_LinePieces : nullptr;
}

inline FX_CHARTYPE CFX_RTFBreak::GetUnifiedCharType(
    FX_CHARTYPE chartype) const {
  return chartype >= FX_CHARTYPE_ArabicAlef ? FX_CHARTYPE_Arabic : chartype;
}

int32_t CFX_RTFBreak::GetLastPositionedTab() const {
  return m_PositionedTabs.empty() ? m_iBoundaryStart : m_PositionedTabs.back();
}

bool CFX_RTFBreak::GetPositionedTab(int32_t* iTabPos) const {
  auto it = std::upper_bound(m_PositionedTabs.begin(), m_PositionedTabs.end(),
                             *iTabPos);
  if (it == m_PositionedTabs.end())
    return false;

  *iTabPos = *it;
  return true;
}

CFX_RTFBreakType CFX_RTFBreak::AppendChar(FX_WCHAR wch) {
  ASSERT(m_pFont && m_pCurLine);

  uint32_t dwProps = kTextLayoutCodeProperties[static_cast<uint16_t>(wch)];
  FX_CHARTYPE chartype = GetCharTypeFromProp(dwProps);
  m_pCurLine->m_LineChars.emplace_back();

  CFX_RTFChar* pCurChar = &m_pCurLine->m_LineChars.back();
  pCurChar->m_dwStatus = CFX_RTFBreakType::None;
  pCurChar->m_wCharCode = wch;
  pCurChar->m_dwCharProps = dwProps;
  pCurChar->m_iFontSize = m_iFontSize;
  pCurChar->m_iFontHeight = m_iFontHeight;
  pCurChar->m_iHorizontalScale = m_iHorizontalScale;
  pCurChar->m_iVerticalScale = m_iVerticalScale;
  pCurChar->m_iCharWidth = 0;
  pCurChar->m_dwIdentity = m_dwIdentity;
  pCurChar->m_pUserData = m_pUserData;

  CFX_RTFBreakType dwRet1 = CFX_RTFBreakType::None;
  if (chartype != FX_CHARTYPE_Combination &&
      GetUnifiedCharType(m_eCharType) != GetUnifiedCharType(chartype) &&
      m_eCharType != FX_CHARTYPE_Unknown &&
      m_pCurLine->GetLineEnd() > m_iBoundaryEnd + m_iTolerance &&
      (m_eCharType != FX_CHARTYPE_Space || chartype != FX_CHARTYPE_Control)) {
    dwRet1 = EndBreak(CFX_RTFBreakType::Line);
    int32_t iCount = m_pCurLine->CountChars();
    if (iCount > 0)
      pCurChar = &m_pCurLine->m_LineChars[iCount - 1];
  }

  CFX_RTFBreakType dwRet2 =
      (this->*g_FX_RTFBreak_lpfAppendChar[chartype >> FX_CHARTYPEBITS])(
          pCurChar);
  m_eCharType = chartype;
  return std::max(dwRet1, dwRet2);
}

CFX_RTFBreakType CFX_RTFBreak::AppendChar_Combination(CFX_RTFChar* pCurChar) {
  int32_t iCharWidth = 0;
  if (!m_pFont->GetCharWidth(pCurChar->m_wCharCode, iCharWidth, false))
    iCharWidth = 0;

  iCharWidth *= m_iFontSize;
  iCharWidth = iCharWidth * m_iHorizontalScale / 100;
  CFX_RTFChar* pLastChar = GetLastChar(0);
  if (pLastChar && pLastChar->GetCharType() > FX_CHARTYPE_Combination)
    iCharWidth = -iCharWidth;
  else
    m_eCharType = FX_CHARTYPE_Combination;

  pCurChar->m_iCharWidth = iCharWidth;
  if (iCharWidth > 0)
    m_pCurLine->m_iWidth += iCharWidth;

  return CFX_RTFBreakType::None;
}

CFX_RTFBreakType CFX_RTFBreak::AppendChar_Tab(CFX_RTFChar* pCurChar) {
  if (!(m_dwLayoutStyles & FX_RTFLAYOUTSTYLE_ExpandTab))
    return CFX_RTFBreakType::None;

  int32_t& iLineWidth = m_pCurLine->m_iWidth;
  int32_t iCharWidth = iLineWidth;
  if (GetPositionedTab(&iCharWidth))
    iCharWidth -= iLineWidth;
  else
    iCharWidth = m_iTabWidth * (iLineWidth / m_iTabWidth + 1) - iLineWidth;

  pCurChar->m_iCharWidth = iCharWidth;
  iLineWidth += iCharWidth;
  return CFX_RTFBreakType::None;
}

CFX_RTFBreakType CFX_RTFBreak::AppendChar_Control(CFX_RTFChar* pCurChar) {
  CFX_RTFBreakType dwRet2 = CFX_RTFBreakType::None;
  switch (pCurChar->m_wCharCode) {
    case L'\v':
    case 0x2028:
      dwRet2 = CFX_RTFBreakType::Line;
      break;
    case L'\f':
      dwRet2 = CFX_RTFBreakType::Page;
      break;
    case 0x2029:
      dwRet2 = CFX_RTFBreakType::Paragraph;
      break;
    default:
      if (pCurChar->m_wCharCode == m_wLineBreakChar)
        dwRet2 = CFX_RTFBreakType::Paragraph;
      break;
  }
  if (dwRet2 != CFX_RTFBreakType::None)
    dwRet2 = EndBreak(dwRet2);

  return dwRet2;
}

CFX_RTFBreakType CFX_RTFBreak::AppendChar_Arabic(CFX_RTFChar* pCurChar) {
  CFX_RTFChar* pLastChar = nullptr;
  int32_t iCharWidth = 0;
  FX_WCHAR wForm;
  bool bAlef = false;
  if (m_eCharType >= FX_CHARTYPE_ArabicAlef &&
      m_eCharType <= FX_CHARTYPE_ArabicDistortion) {
    pLastChar = GetLastChar(1);
    if (pLastChar) {
      m_pCurLine->m_iWidth -= pLastChar->m_iCharWidth;
      CFX_RTFChar* pPrevChar = GetLastChar(2);
      wForm = pdfium::arabic::GetFormChar(pLastChar, pPrevChar, pCurChar);
      bAlef = (wForm == 0xFEFF &&
               pLastChar->GetCharType() == FX_CHARTYPE_ArabicAlef);
      if (!m_pFont->GetCharWidth(wForm, iCharWidth, false) &&
          !m_pFont->GetCharWidth(pLastChar->m_wCharCode, iCharWidth, false)) {
        iCharWidth = m_iDefChar;
      }

      iCharWidth *= m_iFontSize;
      iCharWidth = iCharWidth * m_iHorizontalScale / 100;
      pLastChar->m_iCharWidth = iCharWidth;
      m_pCurLine->m_iWidth += iCharWidth;
      iCharWidth = 0;
    }
  }

  wForm = pdfium::arabic::GetFormChar(pCurChar, bAlef ? nullptr : pLastChar,
                                      nullptr);
  if (!m_pFont->GetCharWidth(wForm, iCharWidth, false) &&
      !m_pFont->GetCharWidth(pCurChar->m_wCharCode, iCharWidth, false)) {
    iCharWidth = m_iDefChar;
  }

  iCharWidth *= m_iFontSize;
  iCharWidth = iCharWidth * m_iHorizontalScale / 100;
  pCurChar->m_iCharWidth = iCharWidth;
  m_pCurLine->m_iWidth += iCharWidth;
  m_pCurLine->m_iArabicChars++;

  if (m_pCurLine->GetLineEnd() > m_iBoundaryEnd + m_iTolerance)
    return EndBreak(CFX_RTFBreakType::Line);
  return CFX_RTFBreakType::None;
}

CFX_RTFBreakType CFX_RTFBreak::AppendChar_Others(CFX_RTFChar* pCurChar) {
  FX_CHARTYPE chartype = pCurChar->GetCharType();
  FX_WCHAR wForm = pCurChar->m_wCharCode;
  int32_t iCharWidth = 0;
  if (!m_pFont->GetCharWidth(wForm, iCharWidth, false))
    iCharWidth = m_iDefChar;

  iCharWidth *= m_iFontSize;
  iCharWidth *= m_iHorizontalScale / 100;
  iCharWidth += m_iCharSpace;

  pCurChar->m_iCharWidth = iCharWidth;
  m_pCurLine->m_iWidth += iCharWidth;
  if (chartype != FX_CHARTYPE_Space &&
      m_pCurLine->GetLineEnd() > m_iBoundaryEnd + m_iTolerance) {
    return EndBreak(CFX_RTFBreakType::Line);
  }
  return CFX_RTFBreakType::None;
}

CFX_RTFBreakType CFX_RTFBreak::EndBreak(CFX_RTFBreakType dwStatus) {
  ASSERT(dwStatus != CFX_RTFBreakType::None);

  m_dwIdentity++;
  const CFX_RTFPieceArray* pCurPieces = &m_pCurLine->m_LinePieces;
  int32_t iCount = pCurPieces->GetSize();
  if (iCount > 0) {
    CFX_RTFPiece* pLastPiece = pCurPieces->GetPtrAt(--iCount);
    if (dwStatus != CFX_RTFBreakType::Piece)
      pLastPiece->m_dwStatus = dwStatus;
    else
      dwStatus = pLastPiece->m_dwStatus;
    return dwStatus;
  }

  const CFX_RTFLine* pLastLine = GetRTFLine();
  if (pLastLine) {
    pCurPieces = &pLastLine->m_LinePieces;
    iCount = pCurPieces->GetSize();
    if (iCount-- > 0) {
      CFX_RTFPiece* pLastPiece = pCurPieces->GetPtrAt(iCount);
      if (dwStatus != CFX_RTFBreakType::Piece)
        pLastPiece->m_dwStatus = dwStatus;
      else
        dwStatus = pLastPiece->m_dwStatus;
      return dwStatus;
    }
    return CFX_RTFBreakType::None;
  }

  iCount = m_pCurLine->CountChars();
  if (iCount < 1)
    return CFX_RTFBreakType::None;

  CFX_RTFChar& tc = m_pCurLine->GetChar(iCount - 1);
  tc.m_dwStatus = dwStatus;
  if (dwStatus == CFX_RTFBreakType::Piece)
    return dwStatus;

  m_iReady = m_pCurLine == &m_RTFLine1 ? 1 : 2;
  CFX_RTFLine* pNextLine =
      m_pCurLine == &m_RTFLine1 ? &m_RTFLine2 : &m_RTFLine1;
  bool bAllChars = m_iAlignment == CFX_RTFLineAlignment::Justified ||
                   m_iAlignment == CFX_RTFLineAlignment::Distributed;

  if (!EndBreak_SplitLine(pNextLine, bAllChars, dwStatus)) {
    std::deque<FX_TPO> tpos;
    EndBreak_BidiLine(&tpos, dwStatus);
    if (!m_bPagination && m_iAlignment != CFX_RTFLineAlignment::Left)
      EndBreak_Alignment(tpos, bAllChars, dwStatus);
  }
  m_pCurLine = pNextLine;
  m_pCurLine->m_iStart = m_iBoundaryStart;

  CFX_RTFChar* pTC = GetLastChar(0);
  m_eCharType = pTC ? pTC->GetCharType() : FX_CHARTYPE_Unknown;
  return dwStatus;
}

bool CFX_RTFBreak::EndBreak_SplitLine(CFX_RTFLine* pNextLine,
                                      bool bAllChars,
                                      CFX_RTFBreakType dwStatus) {
  bool bDone = false;
  if (m_pCurLine->GetLineEnd() > m_iBoundaryEnd + m_iTolerance) {
    const CFX_RTFChar& tc = m_pCurLine->GetChar(m_pCurLine->CountChars() - 1);
    switch (tc.GetCharType()) {
      case FX_CHARTYPE_Tab:
      case FX_CHARTYPE_Control:
      case FX_CHARTYPE_Space:
        break;
      default:
        SplitTextLine(m_pCurLine, pNextLine, !m_bPagination && bAllChars);
        bDone = true;
        break;
    }
  }

  if (!m_bPagination && m_pCurLine->m_iMBCSChars <= 0) {
    if (bAllChars && !bDone) {
      int32_t endPos = m_pCurLine->GetLineEnd();
      GetBreakPos(m_pCurLine->m_LineChars, endPos, bAllChars, true);
    }
    return false;
  }

  const CFX_RTFChar* pCurChars = m_pCurLine->m_LineChars.data();
  const CFX_RTFChar* pTC;
  CFX_RTFPieceArray* pCurPieces = &m_pCurLine->m_LinePieces;
  CFX_RTFPiece tp;
  tp.m_pChars = &m_pCurLine->m_LineChars;
  bool bNew = true;
  uint32_t dwIdentity = static_cast<uint32_t>(-1);
  int32_t iLast = m_pCurLine->CountChars() - 1;
  int32_t j = 0;
  for (int32_t i = 0; i <= iLast;) {
    pTC = pCurChars + i;
    if (bNew) {
      tp.m_iStartChar = i;
      tp.m_iStartPos += tp.m_iWidth;
      tp.m_iWidth = 0;
      tp.m_dwStatus = pTC->m_dwStatus;
      tp.m_iFontSize = pTC->m_iFontSize;
      tp.m_iFontHeight = pTC->m_iFontHeight;
      tp.m_iHorizontalScale = pTC->m_iHorizontalScale;
      tp.m_iVerticalScale = pTC->m_iVerticalScale;
      dwIdentity = pTC->m_dwIdentity;
      tp.m_dwIdentity = dwIdentity;
      tp.m_pUserData = pTC->m_pUserData;
      j = i;
      bNew = false;
    }

    if (i == iLast || pTC->m_dwStatus != CFX_RTFBreakType::None ||
        pTC->m_dwIdentity != dwIdentity) {
      tp.m_iChars = i - j;
      if (pTC->m_dwIdentity == dwIdentity) {
        tp.m_dwStatus = pTC->m_dwStatus;
        tp.m_iWidth += pTC->m_iCharWidth;
        tp.m_iChars += 1;
        i++;
      }
      pCurPieces->Add(tp);
      bNew = true;
    } else {
      tp.m_iWidth += pTC->m_iCharWidth;
      i++;
    }
  }
  return true;
}

void CFX_RTFBreak::EndBreak_BidiLine(std::deque<FX_TPO>* tpos,
                                     CFX_RTFBreakType dwStatus) {
  FX_TPO tpo;
  CFX_RTFPiece tp;
  CFX_RTFChar* pTC;
  int32_t i;
  int32_t j;
  std::vector<CFX_RTFChar>& chars = m_pCurLine->m_LineChars;
  int32_t iCount = m_pCurLine->CountChars();
  if (!m_bPagination && m_pCurLine->m_iArabicChars > 0) {
    int32_t iBidiNum = 0;
    for (i = 0; i < iCount; i++) {
      pTC = &chars[i];
      pTC->m_iBidiPos = i;
      if (pTC->GetCharType() != FX_CHARTYPE_Control)
        iBidiNum = i;
      if (i == 0)
        pTC->m_iBidiLevel = 1;
    }
    FX_BidiLine(chars, iBidiNum + 1, 0);
  } else {
    for (i = 0; i < iCount; i++) {
      pTC = &chars[i];
      pTC->m_iBidiLevel = 0;
      pTC->m_iBidiPos = 0;
      pTC->m_iBidiOrder = 0;
    }
  }

  tp.m_dwStatus = CFX_RTFBreakType::Piece;
  tp.m_iStartPos = m_pCurLine->m_iStart;
  tp.m_pChars = &chars;
  CFX_RTFPieceArray* pCurPieces = &m_pCurLine->m_LinePieces;
  int32_t iBidiLevel = -1;
  int32_t iCharWidth;
  uint32_t dwIdentity = static_cast<uint32_t>(-1);
  i = 0;
  j = 0;
  while (i < iCount) {
    pTC = &chars[i];
    if (iBidiLevel < 0) {
      iBidiLevel = pTC->m_iBidiLevel;
      iCharWidth = pTC->m_iCharWidth;
      tp.m_iWidth = iCharWidth < 1 ? 0 : iCharWidth;
      tp.m_iBidiLevel = iBidiLevel;
      tp.m_iBidiPos = pTC->m_iBidiOrder;
      tp.m_iFontSize = pTC->m_iFontSize;
      tp.m_iFontHeight = pTC->m_iFontHeight;
      tp.m_iHorizontalScale = pTC->m_iHorizontalScale;
      tp.m_iVerticalScale = pTC->m_iVerticalScale;
      dwIdentity = pTC->m_dwIdentity;
      tp.m_dwIdentity = dwIdentity;
      tp.m_pUserData = pTC->m_pUserData;
      tp.m_dwStatus = CFX_RTFBreakType::Piece;
      i++;
    } else if (iBidiLevel != pTC->m_iBidiLevel ||
               pTC->m_dwIdentity != dwIdentity) {
      tp.m_iChars = i - tp.m_iStartChar;
      pCurPieces->Add(tp);
      tp.m_iStartPos += tp.m_iWidth;
      tp.m_iStartChar = i;
      tpo.index = j++;
      tpo.pos = tp.m_iBidiPos;
      tpos->push_back(tpo);
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
    tp.m_iChars = i - tp.m_iStartChar;
    pCurPieces->Add(tp);
    tpo.index = j;
    tpo.pos = tp.m_iBidiPos;
    tpos->push_back(tpo);
  }

  std::sort(tpos->begin(), tpos->end());
  int32_t iStartPos = m_pCurLine->m_iStart;
  for (const auto& it : *tpos) {
    CFX_RTFPiece& ttp = pCurPieces->GetAt(it.index);
    ttp.m_iStartPos = iStartPos;
    iStartPos += ttp.m_iWidth;
  }
}

void CFX_RTFBreak::EndBreak_Alignment(const std::deque<FX_TPO>& tpos,
                                      bool bAllChars,
                                      CFX_RTFBreakType dwStatus) {
  CFX_RTFPieceArray* pCurPieces = &m_pCurLine->m_LinePieces;
  int32_t iNetWidth = m_pCurLine->m_iWidth;
  int32_t iGapChars = 0;
  int32_t iCharWidth;
  int32_t iCount = pCurPieces->GetSize();
  bool bFind = false;
  uint32_t dwCharType;
  int32_t i;
  int32_t j;
  FX_TPO tpo;
  for (i = iCount - 1; i > -1; i--) {
    tpo = tpos[i];
    CFX_RTFPiece& ttp = pCurPieces->GetAt(tpo.index);
    if (!bFind)
      iNetWidth = ttp.GetEndPos();

    bool bArabic = FX_IsOdd(ttp.m_iBidiLevel);
    j = bArabic ? 0 : ttp.m_iChars - 1;
    while (j > -1 && j < ttp.m_iChars) {
      const CFX_RTFChar& tc = ttp.GetChar(j);
      if (tc.m_nBreakType == FX_LBT_DIRECT_BRK)
        iGapChars++;

      if (!bFind || !bAllChars) {
        dwCharType = tc.GetCharType();
        if (dwCharType == FX_CHARTYPE_Space ||
            dwCharType == FX_CHARTYPE_Control) {
          if (!bFind) {
            iCharWidth = tc.m_iCharWidth;
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

  int32_t iOffset = m_iBoundaryEnd - iNetWidth;
  if (iGapChars > 0 && (m_iAlignment == CFX_RTFLineAlignment::Distributed ||
                        (m_iAlignment == CFX_RTFLineAlignment::Justified &&
                         dwStatus != CFX_RTFBreakType::Paragraph))) {
    int32_t iStart = -1;
    for (i = 0; i < iCount; i++) {
      tpo = tpos[i];
      CFX_RTFPiece& ttp = pCurPieces->GetAt(tpo.index);
      if (iStart < 0)
        iStart = ttp.m_iStartPos;
      else
        ttp.m_iStartPos = iStart;

      for (j = 0; j < ttp.m_iChars; j++) {
        CFX_RTFChar& tc = ttp.GetChar(j);
        if (tc.m_nBreakType != FX_LBT_DIRECT_BRK || tc.m_iCharWidth < 0)
          continue;

        int32_t k = iOffset / iGapChars;
        tc.m_iCharWidth += k;
        ttp.m_iWidth += k;
        iOffset -= k;
        iGapChars--;
        if (iGapChars < 1)
          break;
      }
      iStart += ttp.m_iWidth;
    }
  } else if (m_iAlignment == CFX_RTFLineAlignment::Right ||
             m_iAlignment == CFX_RTFLineAlignment::Center) {
    if (m_iAlignment == CFX_RTFLineAlignment::Center)
      iOffset /= 2;
    if (iOffset > 0) {
      for (i = 0; i < iCount; i++) {
        CFX_RTFPiece& ttp = pCurPieces->GetAt(i);
        ttp.m_iStartPos += iOffset;
      }
    }
  }
}

int32_t CFX_RTFBreak::GetBreakPos(std::vector<CFX_RTFChar>& tca,
                                  int32_t& iEndPos,
                                  bool bAllChars,
                                  bool bOnlyBrk) {
  int32_t iLength = pdfium::CollectionSize<int32_t>(tca) - 1;
  if (iLength < 1)
    return iLength;

  int32_t iBreak = -1;
  int32_t iBreakPos = -1;
  int32_t iIndirect = -1;
  int32_t iIndirectPos = -1;
  int32_t iLast = -1;
  int32_t iLastPos = -1;
  if (iEndPos <= m_iBoundaryEnd) {
    if (!bAllChars)
      return iLength;

    iBreak = iLength;
    iBreakPos = iEndPos;
  }

  CFX_RTFChar* pCharArray = tca.data();
  CFX_RTFChar* pCur = pCharArray + iLength;
  --iLength;
  if (bAllChars)
    pCur->m_nBreakType = FX_LBT_UNKNOWN;

  uint32_t nCodeProp = pCur->m_dwCharProps;
  uint32_t nNext = nCodeProp & 0x003F;
  int32_t iCharWidth = pCur->m_iCharWidth;
  if (iCharWidth > 0)
    iEndPos -= iCharWidth;

  while (iLength >= 0) {
    pCur = pCharArray + iLength;
    nCodeProp = pCur->m_dwCharProps;
    uint32_t nCur = nCodeProp & 0x003F;
    bool bNeedBreak = false;
    FX_LINEBREAKTYPE eType;
    if (nCur == FX_CBP_TB) {
      bNeedBreak = true;
      eType = nNext == FX_CBP_TB ? FX_LBT_PROHIBITED_BRK
                                 : gs_FX_LineBreak_PairTable[nCur][nNext];
    } else {
      if (nCur == FX_CBP_SP)
        bNeedBreak = true;

      eType = nNext == FX_CBP_SP ? FX_LBT_PROHIBITED_BRK
                                 : gs_FX_LineBreak_PairTable[nCur][nNext];
    }
    if (bAllChars)
      pCur->m_nBreakType = eType;

    if (!bOnlyBrk) {
      iCharWidth = pCur->m_iCharWidth;
      if (iEndPos <= m_iBoundaryEnd || bNeedBreak) {
        if (eType == FX_LBT_DIRECT_BRK && iBreak < 0) {
          iBreak = iLength;
          iBreakPos = iEndPos;
          if (!bAllChars)
            return iLength;
        } else if (eType == FX_LBT_INDIRECT_BRK && iIndirect < 0) {
          iIndirect = iLength;
          iIndirectPos = iEndPos;
        }
        if (iLast < 0) {
          iLast = iLength;
          iLastPos = iEndPos;
        }
      }
      if (iCharWidth > 0)
        iEndPos -= iCharWidth;
    }
    nNext = nCodeProp & 0x003F;
    iLength--;
  }
  if (bOnlyBrk)
    return 0;

  if (iBreak > -1) {
    iEndPos = iBreakPos;
    return iBreak;
  }
  if (iIndirect > -1) {
    iEndPos = iIndirectPos;
    return iIndirect;
  }
  if (iLast > -1) {
    iEndPos = iLastPos;
    return iLast;
  }
  return 0;
}

void CFX_RTFBreak::SplitTextLine(CFX_RTFLine* pCurLine,
                                 CFX_RTFLine* pNextLine,
                                 bool bAllChars) {
  ASSERT(pCurLine && pNextLine);
  int32_t iCount = pCurLine->CountChars();
  if (iCount < 2)
    return;

  int32_t iEndPos = pCurLine->GetLineEnd();
  std::vector<CFX_RTFChar>& curChars = pCurLine->m_LineChars;
  int32_t iCharPos = GetBreakPos(curChars, iEndPos, bAllChars, false);
  if (iCharPos < 0)
    iCharPos = 0;

  iCharPos++;
  if (iCharPos >= iCount) {
    pNextLine->RemoveAll(true);
    CFX_Char* pTC = &curChars[iCharPos - 1];
    pTC->m_nBreakType = FX_LBT_UNKNOWN;
    return;
  }

  pNextLine->m_LineChars =
      std::vector<CFX_RTFChar>(curChars.begin() + iCharPos, curChars.end());
  curChars.erase(curChars.begin() + iCharPos, curChars.end());
  pNextLine->m_iStart = pCurLine->m_iStart;
  pNextLine->m_iWidth = pCurLine->GetLineEnd() - iEndPos;
  pCurLine->m_iWidth = iEndPos;
  curChars[iCharPos - 1].m_nBreakType = FX_LBT_UNKNOWN;

  for (size_t i = 0; i < pNextLine->m_LineChars.size(); i++) {
    if (pNextLine->m_LineChars[i].GetCharType() >= FX_CHARTYPE_ArabicAlef) {
      pCurLine->m_iArabicChars--;
      pNextLine->m_iArabicChars++;
    }
    pNextLine->m_LineChars[i].m_dwStatus = CFX_RTFBreakType::None;
  }
}

int32_t CFX_RTFBreak::CountBreakPieces() const {
  const CFX_RTFPieceArray* pRTFPieces = GetRTFPieces();
  return pRTFPieces ? pRTFPieces->GetSize() : 0;
}

const CFX_RTFPiece* CFX_RTFBreak::GetBreakPiece(int32_t index) const {
  const CFX_RTFPieceArray* pRTFPieces = GetRTFPieces();
  if (!pRTFPieces)
    return nullptr;
  if (index < 0 || index >= pRTFPieces->GetSize())
    return nullptr;
  return pRTFPieces->GetPtrAt(index);
}

void CFX_RTFBreak::ClearBreakPieces() {
  const CFX_RTFLine* pRTFLine = GetRTFLine();
  if (pRTFLine)
    const_cast<CFX_RTFLine*>(pRTFLine)->RemoveAll(true);
  m_iReady = 0;
}

void CFX_RTFBreak::Reset() {
  m_eCharType = FX_CHARTYPE_Unknown;
  m_RTFLine1.RemoveAll(true);
  m_RTFLine2.RemoveAll(true);
}

int32_t CFX_RTFBreak::GetDisplayPos(const FX_RTFTEXTOBJ* pText,
                                    FXTEXT_CHARPOS* pCharPos,
                                    bool bCharCode) const {
  if (!pText || pText->iLength < 1)
    return 0;

  ASSERT(pText->pFont && pText->pRect);

  CFX_RetainPtr<CFGAS_GEFont> pFont = pText->pFont;
  CFX_RectF rtText(*pText->pRect);
  bool bRTLPiece = FX_IsOdd(pText->iBidiLevel);
  FX_FLOAT fFontSize = pText->fFontSize;
  int32_t iFontSize = FXSYS_round(fFontSize * 20.0f);
  int32_t iAscent = pFont->GetAscent();
  int32_t iDescent = pFont->GetDescent();
  int32_t iMaxHeight = iAscent - iDescent;
  FX_FLOAT fFontHeight = fFontSize;
  FX_FLOAT fAscent = fFontHeight * static_cast<FX_FLOAT>(iAscent) /
                     static_cast<FX_FLOAT>(iMaxHeight);
  FX_WCHAR wch;
  FX_WCHAR wPrev = 0xFEFF;
  FX_WCHAR wNext;
  FX_WCHAR wForm;
  int32_t iWidth;
  int32_t iCharWidth;
  int32_t iCharHeight;
  FX_FLOAT fX = rtText.left;
  FX_FLOAT fY = rtText.top;
  FX_FLOAT fCharWidth;
  FX_FLOAT fCharHeight;
  int32_t iHorScale = pText->iHorizontalScale;
  int32_t iVerScale = pText->iVerticalScale;
  bool bEmptyChar;
  uint32_t dwProps;
  uint32_t dwCharType;

  if (bRTLPiece)
    fX = rtText.right();

  fY += fAscent;
  int32_t iCount = 0;
  for (int32_t i = 0; i < pText->iLength; i++) {
    wch = pText->pStr[i];
    iWidth = pText->pWidths[i];
    dwProps = FX_GetUnicodeProperties(wch);
    dwCharType = (dwProps & FX_CHARTYPEBITSMASK);
    if (iWidth == 0) {
      if (dwCharType == FX_CHARTYPE_ArabicAlef)
        wPrev = 0xFEFF;
      continue;
    }

    iCharWidth = FXSYS_abs(iWidth);
    bEmptyChar =
        (dwCharType >= FX_CHARTYPE_Tab && dwCharType <= FX_CHARTYPE_Control);
    if (!bEmptyChar)
      iCount++;

    if (pCharPos) {
      iCharWidth /= iFontSize;
      wForm = wch;
      if (dwCharType >= FX_CHARTYPE_ArabicAlef) {
        if (i + 1 < pText->iLength) {
          wNext = pText->pStr[i + 1];
          if (pText->pWidths[i + 1] < 0 && i + 2 < pText->iLength)
            wNext = pText->pStr[i + 2];
        } else {
          wNext = 0xFEFF;
        }
        wForm = pdfium::arabic::GetFormChar(wch, wPrev, wNext);
      } else if (bRTLPiece) {
        wForm = FX_GetMirrorChar(wch, dwProps, bRTLPiece, false);
      }
      dwProps = FX_GetUnicodeProperties(wForm);

      if (!bEmptyChar) {
        if (bCharCode) {
          pCharPos->m_GlyphIndex = wch;
        } else {
          pCharPos->m_GlyphIndex = pFont->GetGlyphIndex(wForm, false);
          if (pCharPos->m_GlyphIndex == 0xFFFF)
            pCharPos->m_GlyphIndex = pFont->GetGlyphIndex(wch, false);
        }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
        pCharPos->m_ExtGID = pCharPos->m_GlyphIndex;
#endif
        pCharPos->m_FontCharWidth = iCharWidth;
      }
      iCharHeight = 1000;

      fCharWidth = fFontSize * iCharWidth / 1000.0f;
      fCharHeight = fFontSize * iCharHeight / 1000.0f;
      if (bRTLPiece && dwCharType != FX_CHARTYPE_Combination)
        fX -= fCharWidth;

      if (!bEmptyChar)
        pCharPos->m_Origin = CFX_PointF(fX, fY);
      if (!bRTLPiece && dwCharType != FX_CHARTYPE_Combination)
        fX += fCharWidth;

      if (!bEmptyChar) {
        pCharPos->m_bGlyphAdjust = true;
        pCharPos->m_AdjustMatrix[0] = -1;
        pCharPos->m_AdjustMatrix[1] = 0;
        pCharPos->m_AdjustMatrix[2] = 0;
        pCharPos->m_AdjustMatrix[3] = 1;
        pCharPos->m_Origin.y += fAscent * iVerScale / 100.0f;
        pCharPos->m_Origin.y -= fAscent;

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
      wPrev = wch;
  }
  return iCount;
}

CFX_RTFPiece::CFX_RTFPiece()
    : m_dwStatus(CFX_RTFBreakType::Piece),
      m_iStartPos(0),
      m_iWidth(-1),
      m_iStartChar(0),
      m_iChars(0),
      m_iBidiLevel(0),
      m_iBidiPos(0),
      m_iFontSize(0),
      m_iFontHeight(0),
      m_iHorizontalScale(100),
      m_iVerticalScale(100),
      m_dwIdentity(0),
      m_pChars(nullptr),
      m_pUserData(nullptr) {}

CFX_RTFPiece::~CFX_RTFPiece() {
  Reset();
}

CFX_RTFLine::CFX_RTFLine()
    : m_LinePieces(16),
      m_iStart(0),
      m_iWidth(0),
      m_iArabicChars(0),
      m_iMBCSChars(0) {}

CFX_RTFLine::~CFX_RTFLine() {
  RemoveAll(false);
}

FX_RTFTEXTOBJ::FX_RTFTEXTOBJ()
    : pFont(nullptr),
      pRect(nullptr),
      wLineBreakChar(L'\n'),
      fFontSize(12.0f),
      iLength(0),
      iBidiLevel(0),
      iHorizontalScale(100),
      iVerticalScale(100) {}

FX_RTFTEXTOBJ::~FX_RTFTEXTOBJ() {}
