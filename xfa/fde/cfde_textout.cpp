// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_textout.h"

#include <algorithm>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_txtbreak.h"

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

// static
bool CFDE_TextOut::DrawString(CFX_RenderDevice* device,
                              FX_ARGB color,
                              const RetainPtr<CFGAS_GEFont>& pFont,
                              pdfium::span<TextCharPos> pCharPos,
                              float fFontSize,
                              const CFX_Matrix& matrix) {
  DCHECK(pFont);
  DCHECK(!pCharPos.empty());

  CFX_Font* pFxFont = pFont->GetDevFont();
  if (FontStyleIsItalic(pFont->GetFontStyles()) && !pFxFont->IsItalic()) {
    for (auto& pos : pCharPos) {
      static constexpr float mc = 0.267949f;
      pos.m_AdjustMatrix[2] += mc * pos.m_AdjustMatrix[0];
      pos.m_AdjustMatrix[3] += mc * pos.m_AdjustMatrix[1];
    }
  }

#if !BUILDFLAG(IS_WIN)
  uint32_t dwFontStyle = pFont->GetFontStyles();
  CFX_Font FxFont;
  auto SubstFxFont = std::make_unique<CFX_SubstFont>();
  SubstFxFont->m_Weight = FontStyleIsForceBold(dwFontStyle) ? 700 : 400;
  SubstFxFont->m_ItalicAngle = FontStyleIsItalic(dwFontStyle) ? -12 : 0;
  SubstFxFont->m_WeightCJK = SubstFxFont->m_Weight;
  SubstFxFont->m_bItalicCJK = FontStyleIsItalic(dwFontStyle);
  FxFont.SetSubstFont(std::move(SubstFxFont));
#endif

  RetainPtr<CFGAS_GEFont> pCurFont;
  TextCharPos* pCurCP = nullptr;
  int32_t iCurCount = 0;
  static constexpr CFX_TextRenderOptions kOptions(CFX_TextRenderOptions::kLcd);
  for (auto& pos : pCharPos) {
    RetainPtr<CFGAS_GEFont> pSTFont =
        pFont->GetSubstFont(static_cast<int32_t>(pos.m_GlyphIndex));
    pos.m_GlyphIndex &= 0x00FFFFFF;
    pos.m_bFontStyle = false;
    if (pCurFont != pSTFont) {
      if (pCurFont) {
        pFxFont = pCurFont->GetDevFont();

        CFX_Font* font;
#if !BUILDFLAG(IS_WIN)
        FxFont.SetFace(pFxFont->GetFace());
        FxFont.SetFontSpan(pFxFont->GetFontSpan());
        font = &FxFont;
#else
        font = pFxFont;
#endif

        device->DrawNormalText(pdfium::make_span(pCurCP, iCurCount), font,
                               -fFontSize, matrix, color, kOptions);
      }
      pCurFont = pSTFont;
      pCurCP = &pos;
      iCurCount = 1;
    } else {
      ++iCurCount;
    }
  }

  bool bRet = true;
  if (pCurFont && iCurCount) {
    pFxFont = pCurFont->GetDevFont();
    CFX_Font* font;
#if !BUILDFLAG(IS_WIN)
    FxFont.SetFace(pFxFont->GetFace());
    FxFont.SetFontSpan(pFxFont->GetFontSpan());
    font = &FxFont;
#else
    font = pFxFont;
#endif

    bRet = device->DrawNormalText(pdfium::make_span(pCurCP, iCurCount), font,
                                  -fFontSize, matrix, color, kOptions);
  }
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  device->Flush(false);
#endif

  return bRet;
}

CFDE_TextOut::Piece::Piece() = default;

CFDE_TextOut::Piece::Piece(const Piece& that) = default;

CFDE_TextOut::Piece::~Piece() = default;

CFDE_TextOut::CFDE_TextOut()
    : m_pTxtBreak(std::make_unique<CFGAS_TxtBreak>()), m_ttoLines(5) {}

CFDE_TextOut::~CFDE_TextOut() = default;

void CFDE_TextOut::SetFont(const RetainPtr<CFGAS_GEFont>& pFont) {
  DCHECK(pFont);
  m_pFont = pFont;
  m_pTxtBreak->SetFont(pFont);
}

void CFDE_TextOut::SetFontSize(float fFontSize) {
  DCHECK(fFontSize > 0);
  m_fFontSize = fFontSize;
  m_pTxtBreak->SetFontSize(fFontSize);
}

void CFDE_TextOut::SetStyles(const FDE_TextStyle& dwStyles) {
  m_Styles = dwStyles;
  m_dwTxtBkStyles = m_Styles.single_line_
                        ? CFGAS_Break::LayoutStyle::kSingleLine
                        : CFGAS_Break::LayoutStyle::kNone;

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
  DCHECK(fLineSpace > 1.0f);
  m_fLineSpace = fLineSpace;
}

void CFDE_TextOut::SetLineBreakTolerance(float fTolerance) {
  m_fTolerance = fTolerance;
  m_pTxtBreak->SetLineBreakTolerance(m_fTolerance);
}

void CFDE_TextOut::CalcLogicSize(WideStringView str, CFX_SizeF* pSize) {
  CFX_RectF rtText(0.0f, 0.0f, pSize->width, pSize->height);
  CalcLogicSize(str, &rtText);
  *pSize = rtText.Size();
}

void CFDE_TextOut::CalcLogicSize(WideStringView str, CFX_RectF* pRect) {
  if (str.IsEmpty()) {
    pRect->width = 0.0f;
    pRect->height = 0.0f;
    return;
  }

  DCHECK(m_pFont);
  DCHECK(m_fFontSize >= 1.0f);

  if (!m_Styles.single_line_) {
    if (pRect->Width() < 1.0f)
      pRect->width = m_fFontSize * 1000.0f;

    m_pTxtBreak->SetLineWidth(pRect->Width());
  }

  m_iTotalLines = 0;
  float fWidth = 0.0f;
  float fHeight = 0.0f;
  float fStartPos = pRect->right();
  CFGAS_Char::BreakType dwBreakStatus = CFGAS_Char::BreakType::kNone;
  bool break_char_is_set = false;
  for (const wchar_t& wch : str) {
    if (!break_char_is_set && (wch == L'\n' || wch == L'\r')) {
      break_char_is_set = true;
      m_pTxtBreak->SetParagraphBreakChar(wch);
    }
    dwBreakStatus = m_pTxtBreak->AppendChar(wch);
    if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus))
      RetrieveLineWidth(dwBreakStatus, &fStartPos, &fWidth, &fHeight);
  }

  dwBreakStatus = m_pTxtBreak->EndBreak(CFGAS_Char::BreakType::kParagraph);
  if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus))
    RetrieveLineWidth(dwBreakStatus, &fStartPos, &fWidth, &fHeight);

  m_pTxtBreak->Reset();
  float fInc = pRect->Height() - fHeight;
  if (TextAlignmentVerticallyCentered(m_iAlignment))
    fInc /= 2.0f;
  else if (IsTextAlignmentTop(m_iAlignment))
    fInc = 0.0f;

  pRect->left += fStartPos;
  pRect->top += fInc;
  pRect->width = std::min(fWidth, pRect->Width());
  pRect->height = fHeight;
  if (m_Styles.last_line_height_)
    pRect->height -= m_fLineSpace - m_fFontSize;
}

bool CFDE_TextOut::RetrieveLineWidth(CFGAS_Char::BreakType dwBreakStatus,
                                     float* pStartPos,
                                     float* pWidth,
                                     float* pHeight) {
  if (CFX_BreakTypeNoneOrPiece(dwBreakStatus))
    return false;

  float fLineStep = std::max(m_fLineSpace, m_fFontSize);
  float fLineWidth = 0.0f;
  for (int32_t i = 0; i < m_pTxtBreak->CountBreakPieces(); i++) {
    const CFGAS_BreakPiece* pPiece = m_pTxtBreak->GetBreakPieceUnstable(i);
    fLineWidth += static_cast<float>(pPiece->m_iWidth) / 20000.0f;
    *pStartPos = std::min(*pStartPos,
                          static_cast<float>(pPiece->m_iStartPos) / 20000.0f);
  }
  m_pTxtBreak->ClearBreakPieces();

  if (dwBreakStatus == CFGAS_Char::BreakType::kParagraph)
    m_pTxtBreak->Reset();
  if (!m_Styles.line_wrap_ && dwBreakStatus == CFGAS_Char::BreakType::kLine) {
    *pWidth += fLineWidth;
  } else {
    *pWidth = std::max(*pWidth, fLineWidth);
    *pHeight += fLineStep;
  }
  ++m_iTotalLines;
  return true;
}

void CFDE_TextOut::DrawLogicText(CFX_RenderDevice* device,
                                 WideStringView str,
                                 const CFX_RectF& rect) {
  DCHECK(m_pFont);
  DCHECK(m_fFontSize >= 1.0f);

  if (str.IsEmpty())
    return;
  if (rect.width < m_fFontSize || rect.height < m_fFontSize)
    return;

  float fLineWidth = rect.width;
  m_pTxtBreak->SetLineWidth(fLineWidth);
  m_ttoLines.clear();
  m_wsText.clear();

  LoadText(WideString(str), rect);
  Reload(rect);
  DoAlignment(rect);

  if (!device || m_ttoLines.empty())
    return;

  CFX_RectF rtClip = m_Matrix.TransformRect(CFX_RectF());
  device->SaveState();
  if (rtClip.Width() > 0.0f && rtClip.Height() > 0.0f)
    device->SetClip_Rect(rtClip.GetOuterRect());

  for (auto& line : m_ttoLines) {
    for (size_t i = 0; i < line.GetSize(); ++i) {
      const Piece* pPiece = line.GetPieceAtIndex(i);
      size_t szCount = GetDisplayPos(pPiece);
      if (szCount == 0)
        continue;

      CFDE_TextOut::DrawString(device, m_TxtColor, m_pFont,
                               {m_CharPos.data(), szCount}, m_fFontSize,
                               m_Matrix);
    }
  }
  device->RestoreState(false);
}

void CFDE_TextOut::LoadText(const WideString& str, const CFX_RectF& rect) {
  DCHECK(!str.IsEmpty());

  m_wsText = str;

  if (m_CharWidths.size() < str.GetLength())
    m_CharWidths.resize(str.GetLength(), 0);

  float fLineStep = std::max(m_fLineSpace, m_fFontSize);
  float fLineStop = rect.bottom();
  m_fLinePos = rect.top;
  size_t start_char = 0;
  int32_t iPieceWidths = 0;
  CFGAS_Char::BreakType dwBreakStatus;
  bool bRet = false;
  for (const auto& wch : str) {
    dwBreakStatus = m_pTxtBreak->AppendChar(wch);
    if (CFX_BreakTypeNoneOrPiece(dwBreakStatus))
      continue;

    bool bEndofLine =
        RetrievePieces(dwBreakStatus, false, rect, &start_char, &iPieceWidths);
    if (bEndofLine && (m_Styles.line_wrap_ ||
                       dwBreakStatus == CFGAS_Char::BreakType::kParagraph ||
                       dwBreakStatus == CFGAS_Char::BreakType::kPage)) {
      iPieceWidths = 0;
      ++m_iCurLine;
      m_fLinePos += fLineStep;
    }
    if (m_fLinePos + fLineStep > fLineStop) {
      int32_t iCurLine = bEndofLine ? m_iCurLine - 1 : m_iCurLine;
      m_ttoLines[iCurLine].set_new_reload(true);
      bRet = true;
      break;
    }
  }

  dwBreakStatus = m_pTxtBreak->EndBreak(CFGAS_Char::BreakType::kParagraph);
  if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus) && !bRet)
    RetrievePieces(dwBreakStatus, false, rect, &start_char, &iPieceWidths);

  m_pTxtBreak->ClearBreakPieces();
  m_pTxtBreak->Reset();
}

bool CFDE_TextOut::RetrievePieces(CFGAS_Char::BreakType dwBreakStatus,
                                  bool bReload,
                                  const CFX_RectF& rect,
                                  size_t* pStartChar,
                                  int32_t* pPieceWidths) {
  float fLineStep = std::max(m_fLineSpace, m_fFontSize);
  bool bNeedReload = false;
  int32_t iLineWidth = FXSYS_roundf(rect.Width() * 20000.0f);
  int32_t iCount = m_pTxtBreak->CountBreakPieces();
  for (int32_t i = 0; i < iCount; i++) {
    const CFGAS_BreakPiece* pPiece = m_pTxtBreak->GetBreakPieceUnstable(i);
    int32_t iPieceChars = pPiece->GetLength();
    size_t iChar = *pStartChar;
    int32_t iWidth = 0;
    int32_t j = 0;
    for (; j < iPieceChars; j++) {
      const CFGAS_Char* pTC = pPiece->GetChar(j);
      int32_t iCurCharWidth = std::max(pTC->m_iCharWidth, 0);
      if (m_Styles.single_line_ || !m_Styles.line_wrap_) {
        if (iLineWidth - *pPieceWidths - iWidth < iCurCharWidth) {
          bNeedReload = true;
          break;
        }
      }
      iWidth += iCurCharWidth;
      m_CharWidths[iChar++] = iCurCharWidth;
    }

    if (j == 0 && !bReload) {
      m_ttoLines[m_iCurLine].set_new_reload(true);
    } else if (j > 0) {
      Piece piece;
      piece.start_char = *pStartChar;
      piece.char_count = j;
      piece.char_styles = pPiece->m_dwCharStyles;
      piece.bounds = CFX_RectF(
          rect.left + static_cast<float>(pPiece->m_iStartPos) / 20000.0f,
          m_fLinePos, iWidth / 20000.0f, fLineStep);

      if (FX_IsOdd(pPiece->m_iBidiLevel))
        piece.char_styles |= FX_TXTCHARSTYLE_OddBidiLevel;

      AppendPiece(piece, bNeedReload, (bReload && i == iCount - 1));
    }
    *pStartChar += iPieceChars;
    *pPieceWidths += iWidth;
  }
  m_pTxtBreak->ClearBreakPieces();

  return m_Styles.single_line_ || m_Styles.line_wrap_ || bNeedReload ||
         dwBreakStatus == CFGAS_Char::BreakType::kParagraph;
}

void CFDE_TextOut::AppendPiece(const Piece& piece,
                               bool bNeedReload,
                               bool bEnd) {
  if (m_iCurLine >= fxcrt::CollectionSize<int32_t>(m_ttoLines)) {
    Line ttoLine;
    ttoLine.set_new_reload(bNeedReload);

    m_iCurPiece = ttoLine.AddPiece(m_iCurPiece, piece);
    m_ttoLines.push_back(ttoLine);
    m_iCurLine = fxcrt::CollectionSize<int32_t>(m_ttoLines) - 1;
  } else {
    Line* pLine = &m_ttoLines[m_iCurLine];
    pLine->set_new_reload(bNeedReload);

    m_iCurPiece = pLine->AddPiece(m_iCurPiece, piece);
    if (bEnd) {
      size_t iPieces = pLine->GetSize();
      if (m_iCurPiece < iPieces)
        pLine->RemoveLast(iPieces - m_iCurPiece - 1);
    }
  }
  if (!bEnd && bNeedReload)
    m_iCurPiece = 0;
}

void CFDE_TextOut::Reload(const CFX_RectF& rect) {
  int i = 0;
  for (auto& line : m_ttoLines) {
    if (line.new_reload()) {
      m_iCurLine = i;
      m_iCurPiece = 0;
      ReloadLinePiece(&line, rect);
    }
    ++i;
  }
}

void CFDE_TextOut::ReloadLinePiece(Line* line, const CFX_RectF& rect) {
  pdfium::span<const wchar_t> text_span = m_wsText.span();
  size_t start_char = 0;
  size_t piece_count = line->GetSize();
  int32_t piece_widths = 0;
  CFGAS_Char::BreakType break_status = CFGAS_Char::BreakType::kNone;
  for (size_t piece_index = 0; piece_index < piece_count; ++piece_index) {
    const Piece* piece = line->GetPieceAtIndex(piece_index);
    if (piece_index == 0) {
      start_char = piece->start_char;
      m_fLinePos = piece->bounds.top;
    }
    size_t end = start_char + piece->char_count;
    for (size_t start = start_char; start < end; ++start) {
      break_status = m_pTxtBreak->AppendChar(text_span[start]);
      if (!CFX_BreakTypeNoneOrPiece(break_status))
        RetrievePieces(break_status, true, rect, &start_char, &piece_widths);
    }
  }

  break_status = m_pTxtBreak->EndBreak(CFGAS_Char::BreakType::kParagraph);
  if (!CFX_BreakTypeNoneOrPiece(break_status))
    RetrievePieces(break_status, true, rect, &start_char, &piece_widths);

  m_pTxtBreak->Reset();
}

void CFDE_TextOut::DoAlignment(const CFX_RectF& rect) {
  if (m_ttoLines.empty())
    return;

  const Piece* pFirstPiece = m_ttoLines.back().GetPieceAtIndex(0);
  if (!pFirstPiece)
    return;

  float fInc = rect.bottom() - pFirstPiece->bounds.bottom();
  if (TextAlignmentVerticallyCentered(m_iAlignment))
    fInc /= 2.0f;
  else if (IsTextAlignmentTop(m_iAlignment))
    fInc = 0.0f;

  if (fInc < 1.0f)
    return;

  for (auto& line : m_ttoLines) {
    for (size_t i = 0; i < line.GetSize(); ++i)
      line.GetPieceAtIndex(i)->bounds.top += fInc;
  }
}

size_t CFDE_TextOut::GetDisplayPos(const Piece* pPiece) {
  if (m_CharPos.size() < pPiece->char_count)
    m_CharPos.resize(pPiece->char_count, TextCharPos());

  CFGAS_TxtBreak::Run tr;
  tr.wsStr = m_wsText.Substr(pPiece->start_char);
  tr.pWidths = &m_CharWidths[pPiece->start_char];
  tr.iLength = pdfium::base::checked_cast<int32_t>(pPiece->char_count);
  tr.pFont = m_pFont;
  tr.fFontSize = m_fFontSize;
  tr.dwStyles = m_dwTxtBkStyles;
  tr.dwCharStyles = pPiece->char_styles;
  tr.pRect = &pPiece->bounds;

  return m_pTxtBreak->GetDisplayPos(tr, m_CharPos.data());
}

CFDE_TextOut::Line::Line() = default;

CFDE_TextOut::Line::Line(const Line& that) {
  new_reload_ = that.new_reload_;
  pieces_ = that.pieces_;
}

CFDE_TextOut::Line::~Line() = default;

size_t CFDE_TextOut::Line::AddPiece(size_t index, const Piece& piece) {
  if (index >= pieces_.size()) {
    pieces_.push_back(piece);
    return pieces_.size();
  }
  pieces_[index] = piece;
  return index;
}

size_t CFDE_TextOut::Line::GetSize() const {
  return pieces_.size();
}

const CFDE_TextOut::Piece* CFDE_TextOut::Line::GetPieceAtIndex(
    size_t index) const {
  CHECK(fxcrt::IndexInBounds(pieces_, index));
  return &pieces_[index];
}

CFDE_TextOut::Piece* CFDE_TextOut::Line::GetPieceAtIndex(size_t index) {
  CHECK(fxcrt::IndexInBounds(pieces_, index));
  return &pieces_[index];
}

void CFDE_TextOut::Line::RemoveLast(size_t count) {
  pieces_.erase(pieces_.end() - std::min(count, pieces_.size()), pieces_.end());
}
