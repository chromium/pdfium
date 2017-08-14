// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_textout.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_pathdata.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_renderdevice.h"
#include "xfa/fgas/layout/cfx_txtbreak.h"

namespace {

bool TextAlignmentVerticallyCentered(const FDE_TextAlignment align) {
  return align == FDE_TextAlignment::kCenterLeft ||
         align == FDE_TextAlignment::kCenter ||
         align == FDE_TextAlignment::kCenterRight;
}

bool IsTextAlignmentTop(const FDE_TextAlignment align) {
  return align == FDE_TextAlignment::kTopLeft;
}

}  // namespace

FDE_TTOPIECE::FDE_TTOPIECE() = default;

FDE_TTOPIECE::FDE_TTOPIECE(const FDE_TTOPIECE& that) = default;

FDE_TTOPIECE::~FDE_TTOPIECE() = default;

CFDE_TextOut::CFDE_TextOut()
    : m_pTxtBreak(pdfium::MakeUnique<CFX_TxtBreak>()),
      m_pFont(nullptr),
      m_fFontSize(12.0f),
      m_fLineSpace(m_fFontSize),
      m_fLinePos(0.0f),
      m_fTolerance(0.0f),
      m_iAlignment(FDE_TextAlignment::kTopLeft),
      m_TxtColor(0xFF000000),
      m_dwTxtBkStyles(0),
      m_ttoLines(5),
      m_iCurLine(0),
      m_iCurPiece(0),
      m_iTotalLines(0) {
  m_Matrix.SetIdentity();
}

CFDE_TextOut::~CFDE_TextOut() {}

void CFDE_TextOut::SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont) {
  ASSERT(pFont);
  m_pFont = pFont;
  m_pTxtBreak->SetFont(pFont);
}

void CFDE_TextOut::SetFontSize(float fFontSize) {
  ASSERT(fFontSize > 0);
  m_fFontSize = fFontSize;
  m_pTxtBreak->SetFontSize(fFontSize);
}

void CFDE_TextOut::SetStyles(const FDE_TextStyle& dwStyles) {
  m_Styles = dwStyles;

  m_dwTxtBkStyles = 0;
  if (m_Styles.single_line_)
    m_dwTxtBkStyles |= FX_LAYOUTSTYLE_SingleLine;

  m_pTxtBreak->SetLayoutStyles(m_dwTxtBkStyles);
}

void CFDE_TextOut::SetAlignment(FDE_TextAlignment iAlignment) {
  m_iAlignment = iAlignment;

  int32_t txtBreakAlignment = 0;
  switch (m_iAlignment) {
    case FDE_TextAlignment::kCenter:
      txtBreakAlignment = CFX_TxtLineAlignment_Center;
      break;
    case FDE_TextAlignment::kCenterRight:
      txtBreakAlignment = CFX_TxtLineAlignment_Right;
      break;
    case FDE_TextAlignment::kCenterLeft:
    case FDE_TextAlignment::kTopLeft:
      txtBreakAlignment = CFX_TxtLineAlignment_Left;
      break;
  }
  m_pTxtBreak->SetAlignment(txtBreakAlignment);
}

void CFDE_TextOut::SetLineSpace(float fLineSpace) {
  ASSERT(fLineSpace > 1.0f);
  m_fLineSpace = fLineSpace;
}

void CFDE_TextOut::SetRenderDevice(CFX_RenderDevice* pDevice) {
  ASSERT(pDevice);
  m_pRenderDevice = pdfium::MakeUnique<CFDE_RenderDevice>(pDevice);
}

void CFDE_TextOut::SetLineBreakTolerance(float fTolerance) {
  m_fTolerance = fTolerance;
  m_pTxtBreak->SetLineBreakTolerance(m_fTolerance);
}

void CFDE_TextOut::CalcLogicSize(const wchar_t* pwsStr,
                                 int32_t iLength,
                                 CFX_SizeF& size) {
  CFX_RectF rtText(0.0f, 0.0f, size.width, size.height);
  CalcLogicSize(pwsStr, iLength, rtText);
  size = rtText.Size();
}

void CFDE_TextOut::CalcLogicSize(const wchar_t* pwsStr,
                                 int32_t iLength,
                                 CFX_RectF& rect) {
  if (!pwsStr || iLength < 1) {
    rect.width = 0.0f;
    rect.height = 0.0f;
    return;
  }

  ASSERT(m_pFont && m_fFontSize >= 1.0f);

  if (!m_Styles.single_line_) {
    if (rect.Width() < 1.0f)
      rect.width = m_fFontSize * 1000.0f;

    m_pTxtBreak->SetLineWidth(rect.Width());
  }

  m_iTotalLines = 0;
  const wchar_t* pStr = pwsStr;
  float fWidth = 0.0f;
  float fHeight = 0.0f;
  float fStartPos = rect.right();
  CFX_BreakType dwBreakStatus = CFX_BreakType::None;
  wchar_t wPreChar = 0;
  wchar_t wch;
  wchar_t wBreak = 0;
  while (iLength-- > 0) {
    wch = *pStr++;
    if (wBreak == 0 && (wch == L'\n' || wch == L'\r')) {
      wBreak = wch;
      m_pTxtBreak->SetParagraphBreakChar(wch);
    }
    dwBreakStatus = m_pTxtBreak->AppendChar(wch);
    if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus))
      RetrieveLineWidth(dwBreakStatus, fStartPos, fWidth, fHeight);

    wPreChar = 0;
  }
  dwBreakStatus = m_pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
  if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus))
    RetrieveLineWidth(dwBreakStatus, fStartPos, fWidth, fHeight);

  m_pTxtBreak->Reset();
  float fInc = rect.Height() - fHeight;
  if (TextAlignmentVerticallyCentered(m_iAlignment))
    fInc /= 2.0f;
  else if (IsTextAlignmentTop(m_iAlignment))
    fInc = 0.0f;

  rect.left += fStartPos;
  rect.top += fInc;
  rect.width = std::min(fWidth, rect.Width());
  rect.height = fHeight;
  if (m_Styles.last_line_height_)
    rect.height -= m_fLineSpace - m_fFontSize;
}

bool CFDE_TextOut::RetrieveLineWidth(CFX_BreakType dwBreakStatus,
                                     float& fStartPos,
                                     float& fWidth,
                                     float& fHeight) {
  if (CFX_BreakTypeNoneOrPiece(dwBreakStatus))
    return false;

  float fLineStep = (m_fLineSpace > m_fFontSize) ? m_fLineSpace : m_fFontSize;
  float fLineWidth = 0.0f;
  int32_t iCount = m_pTxtBreak->CountBreakPieces();
  for (int32_t i = 0; i < iCount; i++) {
    const CFX_BreakPiece* pPiece = m_pTxtBreak->GetBreakPieceUnstable(i);
    fLineWidth += static_cast<float>(pPiece->m_iWidth) / 20000.0f;
    fStartPos =
        std::min(fStartPos, static_cast<float>(pPiece->m_iStartPos) / 20000.0f);
  }
  m_pTxtBreak->ClearBreakPieces();
  if (dwBreakStatus == CFX_BreakType::Paragraph) {
    m_pTxtBreak->Reset();
  }
  if (!m_Styles.line_wrap_ && dwBreakStatus == CFX_BreakType::Line) {
    fWidth += fLineWidth;
  } else {
    fWidth = std::max(fWidth, fLineWidth);
    fHeight += fLineStep;
  }
  m_iTotalLines++;
  return true;
}

void CFDE_TextOut::DrawLogicText(const wchar_t* pwsStr,
                                 int32_t iLength,
                                 const CFX_RectF& rect) {
  ASSERT(m_pFont && m_fFontSize >= 1.0f);
  if (!pwsStr || iLength < 1)
    return;
  if (rect.width < m_fFontSize || rect.height < m_fFontSize)
    return;

  float fLineWidth = rect.width;
  m_pTxtBreak->SetLineWidth(fLineWidth);
  m_ttoLines.clear();
  m_wsText.clear();

  LoadText(pwsStr, iLength, rect);
  Reload(rect);
  DoAlignment(rect);

  if (!m_pRenderDevice || m_ttoLines.empty())
    return;

  CFX_RectF rtClip;
  m_Matrix.TransformRect(rtClip);

  m_pRenderDevice->SaveState();
  if (rtClip.Width() > 0.0f && rtClip.Height() > 0.0f)
    m_pRenderDevice->SetClipRect(rtClip);

  for (auto& line : m_ttoLines) {
    int32_t iPieces = line.GetSize();
    for (int32_t j = 0; j < iPieces; j++) {
      FDE_TTOPIECE* pPiece = line.GetPtrAt(j);
      if (!pPiece)
        continue;

      int32_t iCount = GetDisplayPos(pPiece);
      if (iCount > 0) {
        m_pRenderDevice->DrawString(m_TxtColor, m_pFont, m_CharPos.data(),
                                    iCount, m_fFontSize, &m_Matrix);
      }
    }
  }
  m_pRenderDevice->RestoreState();
}

void CFDE_TextOut::LoadText(const wchar_t* pwsStr,
                            int32_t iLength,
                            const CFX_RectF& rect) {
  wchar_t* pStr = m_wsText.GetBuffer(iLength);
  int32_t iTxtLength = iLength;

  ASSERT(iTxtLength >= 0);
  if (pdfium::CollectionSize<int32_t>(m_CharWidths) < iTxtLength)
    m_CharWidths.resize(iTxtLength, 0);

  float fLineStep = (m_fLineSpace > m_fFontSize) ? m_fLineSpace : m_fFontSize;
  float fLineStop = rect.bottom();
  m_fLinePos = rect.top;
  int32_t iStartChar = 0;
  int32_t iChars = 0;
  int32_t iPieceWidths = 0;
  CFX_BreakType dwBreakStatus;
  wchar_t wch;
  bool bRet = false;
  while (iTxtLength-- > 0) {
    wch = *pwsStr++;
    *pStr++ = wch;
    iChars++;
    dwBreakStatus = m_pTxtBreak->AppendChar(wch);
    if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
      bool bEndofLine =
          RetrievePieces(dwBreakStatus, iStartChar, iPieceWidths, false, rect);
      if (bEndofLine &&
          (m_Styles.line_wrap_ || dwBreakStatus == CFX_BreakType::Paragraph ||
           dwBreakStatus == CFX_BreakType::Page)) {
        iPieceWidths = 0;
        m_iCurLine++;
        m_fLinePos += fLineStep;
      }
      if (m_fLinePos + fLineStep > fLineStop) {
        int32_t iCurLine = bEndofLine ? m_iCurLine - 1 : m_iCurLine;
        m_ttoLines[iCurLine].SetNewReload(true);
        bRet = true;
        break;
      }
    }
  }
  dwBreakStatus = m_pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
  if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus) && !bRet)
    RetrievePieces(dwBreakStatus, iStartChar, iPieceWidths, false, rect);

  m_pTxtBreak->ClearBreakPieces();
  m_pTxtBreak->Reset();
  m_wsText.ReleaseBuffer(iLength);
}

bool CFDE_TextOut::RetrievePieces(CFX_BreakType dwBreakStatus,
                                  int32_t& iStartChar,
                                  int32_t& iPieceWidths,
                                  bool bReload,
                                  const CFX_RectF& rect) {
  float fLineStep = (m_fLineSpace > m_fFontSize) ? m_fLineSpace : m_fFontSize;
  bool bNeedReload = false;
  float fLineWidth = rect.Width();
  int32_t iLineWidth = FXSYS_round(fLineWidth * 20000.0f);
  int32_t iCount = m_pTxtBreak->CountBreakPieces();
  for (int32_t i = 0; i < iCount; i++) {
    const CFX_BreakPiece* pPiece = m_pTxtBreak->GetBreakPieceUnstable(i);
    int32_t iPieceChars = pPiece->GetLength();
    int32_t iChar = iStartChar;
    int32_t iWidth = 0;
    int32_t j = 0;
    for (; j < iPieceChars; j++) {
      const CFX_Char* pTC = pPiece->GetChar(j);
      int32_t iCurCharWidth = pTC->m_iCharWidth > 0 ? pTC->m_iCharWidth : 0;
      if (m_Styles.single_line_ || !m_Styles.line_wrap_) {
        if (iLineWidth - iPieceWidths - iWidth < iCurCharWidth) {
          bNeedReload = true;
          break;
        }
      }
      iWidth += iCurCharWidth;
      m_CharWidths[iChar++] = iCurCharWidth;
    }
    if (j == 0 && !bReload) {
      m_ttoLines[m_iCurLine].SetNewReload(true);
    } else if (j > 0) {
      CFX_RectF rtPiece;
      rtPiece.left = rect.left + (float)pPiece->m_iStartPos / 20000.0f;
      rtPiece.top = m_fLinePos;
      rtPiece.width = iWidth / 20000.0f;
      rtPiece.height = fLineStep;

      FDE_TTOPIECE ttoPiece;
      ttoPiece.iStartChar = iStartChar;
      ttoPiece.iChars = j;
      ttoPiece.rtPiece = rtPiece;
      ttoPiece.dwCharStyles = pPiece->m_dwCharStyles;
      if (FX_IsOdd(pPiece->m_iBidiLevel)) {
        ttoPiece.dwCharStyles |= FX_TXTCHARSTYLE_OddBidiLevel;
      }
      AppendPiece(ttoPiece, bNeedReload, (bReload && i == iCount - 1));
    }
    iStartChar += iPieceChars;
    iPieceWidths += iWidth;
  }
  m_pTxtBreak->ClearBreakPieces();
  return m_Styles.single_line_ || m_Styles.line_wrap_ || bNeedReload ||
         dwBreakStatus == CFX_BreakType::Paragraph;
}

void CFDE_TextOut::AppendPiece(const FDE_TTOPIECE& ttoPiece,
                               bool bNeedReload,
                               bool bEnd) {
  if (m_iCurLine >= pdfium::CollectionSize<int32_t>(m_ttoLines)) {
    CFDE_TTOLine ttoLine;
    ttoLine.SetNewReload(bNeedReload);
    m_iCurPiece = ttoLine.AddPiece(m_iCurPiece, ttoPiece);
    m_ttoLines.push_back(ttoLine);
    m_iCurLine = pdfium::CollectionSize<int32_t>(m_ttoLines) - 1;
  } else {
    CFDE_TTOLine* pLine = &m_ttoLines[m_iCurLine];
    pLine->SetNewReload(bNeedReload);
    m_iCurPiece = pLine->AddPiece(m_iCurPiece, ttoPiece);
    if (bEnd) {
      int32_t iPieces = pLine->GetSize();
      if (m_iCurPiece < iPieces) {
        pLine->RemoveLast(iPieces - m_iCurPiece - 1);
      }
    }
  }
  if (!bEnd && bNeedReload)
    m_iCurPiece = 0;
}

void CFDE_TextOut::Reload(const CFX_RectF& rect) {
  int i = 0;
  for (auto& line : m_ttoLines) {
    if (line.GetNewReload()) {
      m_iCurLine = i;
      m_iCurPiece = 0;
      ReloadLinePiece(&line, rect);
    }
    ++i;
  }
}

void CFDE_TextOut::ReloadLinePiece(CFDE_TTOLine* pLine, const CFX_RectF& rect) {
  const wchar_t* pwsStr = m_wsText.c_str();
  int32_t iPieceWidths = 0;
  FDE_TTOPIECE* pPiece = pLine->GetPtrAt(0);
  int32_t iStartChar = pPiece->iStartChar;
  m_fLinePos = pPiece->rtPiece.top;
  int32_t iPieceCount = pLine->GetSize();
  int32_t iPieceIndex = 0;
  CFX_BreakType dwBreakStatus = CFX_BreakType::None;
  wchar_t wch;
  while (iPieceIndex < iPieceCount) {
    int32_t iStar = iStartChar;
    int32_t iEnd = pPiece->iChars + iStar;
    while (iStar < iEnd) {
      wch = *(pwsStr + iStar);
      dwBreakStatus = m_pTxtBreak->AppendChar(wch);
      if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus))
        RetrievePieces(dwBreakStatus, iStartChar, iPieceWidths, true, rect);
      iStar++;
    }
    iPieceIndex++;
    pPiece = pLine->GetPtrAt(iPieceIndex);
  }
  dwBreakStatus = m_pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
  if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus))
    RetrievePieces(dwBreakStatus, iStartChar, iPieceWidths, true, rect);

  m_pTxtBreak->Reset();
}

void CFDE_TextOut::DoAlignment(const CFX_RectF& rect) {
  if (m_ttoLines.empty())
    return;

  float fLineStopS = rect.bottom();
  FDE_TTOPIECE* pFirstPiece = m_ttoLines.back().GetPtrAt(0);
  if (!pFirstPiece)
    return;

  float fLineStopD = pFirstPiece->rtPiece.bottom();
  float fInc = fLineStopS - fLineStopD;
  if (TextAlignmentVerticallyCentered(m_iAlignment))
    fInc /= 2.0f;
  else if (IsTextAlignmentTop(m_iAlignment))
    fInc = 0.0f;

  if (fInc < 1.0f)
    return;

  for (auto& line : m_ttoLines) {
    int32_t iPieces = line.GetSize();
    for (int32_t j = 0; j < iPieces; j++) {
      FDE_TTOPIECE* pPiece = line.GetPtrAt(j);
      pPiece->rtPiece.top += fInc;
    }
  }
}

int32_t CFDE_TextOut::GetDisplayPos(FDE_TTOPIECE* pPiece) {
  FX_TXTRUN tr = ToTextRun(pPiece);
  ASSERT(tr.iLength >= 0);
  if (pdfium::CollectionSize<int32_t>(m_CharPos) < tr.iLength)
    m_CharPos.resize(tr.iLength, FXTEXT_CHARPOS());
  return m_pTxtBreak->GetDisplayPos(&tr, m_CharPos.data());
}

FX_TXTRUN CFDE_TextOut::ToTextRun(const FDE_TTOPIECE* pPiece) {
  FX_TXTRUN tr;
  tr.wsStr = m_wsText + pPiece->iStartChar;
  tr.pWidths = &m_CharWidths[pPiece->iStartChar];
  tr.iLength = pPiece->iChars;
  tr.pFont = m_pFont;
  tr.fFontSize = m_fFontSize;
  tr.dwStyles = m_dwTxtBkStyles;
  tr.dwCharStyles = pPiece->dwCharStyles;
  tr.wLineBreakChar = L'\n';
  tr.pRect = &pPiece->rtPiece;
  return tr;
}

CFDE_TTOLine::CFDE_TTOLine() : m_bNewReload(false) {}

CFDE_TTOLine::CFDE_TTOLine(const CFDE_TTOLine& ttoLine) : m_pieces(5) {
  m_bNewReload = ttoLine.m_bNewReload;
  m_pieces = ttoLine.m_pieces;
}

CFDE_TTOLine::~CFDE_TTOLine() {}

int32_t CFDE_TTOLine::AddPiece(int32_t index, const FDE_TTOPIECE& ttoPiece) {
  if (index >= pdfium::CollectionSize<int32_t>(m_pieces)) {
    m_pieces.push_back(ttoPiece);
    return pdfium::CollectionSize<int32_t>(m_pieces);
  }
  m_pieces[index] = ttoPiece;
  return index;
}

int32_t CFDE_TTOLine::GetSize() const {
  return pdfium::CollectionSize<int32_t>(m_pieces);
}

FDE_TTOPIECE* CFDE_TTOLine::GetPtrAt(int32_t index) {
  return pdfium::IndexInBounds(m_pieces, index) ? &m_pieces[index] : nullptr;
}

void CFDE_TTOLine::RemoveLast(int32_t icount) {
  if (icount < 0)
    return;
  icount = std::min(icount, pdfium::CollectionSize<int32_t>(m_pieces));
  m_pieces.erase(m_pieces.end() - icount, m_pieces.end());
}
