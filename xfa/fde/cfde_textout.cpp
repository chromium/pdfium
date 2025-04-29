// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_textout.h"

#include <algorithm>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_textrenderoptions.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/text_char_pos.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_txtbreak.h"

namespace pdfium {

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
                              span<TextCharPos> pCharPos,
                              float fFontSize,
                              const CFX_Matrix& matrix) {
  DCHECK(pFont);
  DCHECK(!pCharPos.empty());

  CFX_Font* pFxFont = pFont->GetDevFont();
  if (FontStyleIsItalic(pFont->GetFontStyles()) && !pFxFont->IsItalic()) {
    for (auto& pos : pCharPos) {
      static constexpr float mc = 0.267949f;
      pos.adjust_matrix_[2] += mc * pos.adjust_matrix_[0];
      pos.adjust_matrix_[3] += mc * pos.adjust_matrix_[1];
    }
  }

#if !BUILDFLAG(IS_WIN)
  uint32_t dwFontStyle = pFont->GetFontStyles();
  CFX_Font FxFont;
  auto SubstFxFont = std::make_unique<CFX_SubstFont>();
  SubstFxFont->weight_ = FontStyleIsForceBold(dwFontStyle) ? 700 : 400;
  SubstFxFont->italic_angle_ = FontStyleIsItalic(dwFontStyle) ? -12 : 0;
  SubstFxFont->weight_cjk_ = SubstFxFont->weight_;
  SubstFxFont->italic_cjk_ = FontStyleIsItalic(dwFontStyle);
  FxFont.SetSubstFont(std::move(SubstFxFont));
#endif

  RetainPtr<CFGAS_GEFont> pCurFont;
  TextCharPos* pCurCP = nullptr;
  size_t count = 0;
  static constexpr CFX_TextRenderOptions kOptions(CFX_TextRenderOptions::kLcd);
  for (auto& pos : pCharPos) {
    RetainPtr<CFGAS_GEFont> pSTFont =
        pFont->GetSubstFont(static_cast<int32_t>(pos.glyph_index_));
    pos.glyph_index_ &= 0x00FFFFFF;
    pos.font_style_ = false;
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
        device->DrawNormalText(UNSAFE_TODO(pdfium::span(pCurCP, count)), font,
                               -fFontSize, matrix, color, kOptions);
      }
      pCurFont = pSTFont;
      pCurCP = &pos;
      count = 1;
    } else {
      ++count;
    }
  }
  if (pCurFont && count) {
    pFxFont = pCurFont->GetDevFont();
    CFX_Font* font;
#if !BUILDFLAG(IS_WIN)
    FxFont.SetFace(pFxFont->GetFace());
    FxFont.SetFontSpan(pFxFont->GetFontSpan());
    font = &FxFont;
#else
    font = pFxFont;
#endif
    return device->DrawNormalText(UNSAFE_TODO(pdfium::span(pCurCP, count)),
                                  font, -fFontSize, matrix, color, kOptions);
  }
  return true;
}

CFDE_TextOut::Piece::Piece() = default;

CFDE_TextOut::Piece::Piece(const Piece& that) = default;

CFDE_TextOut::Piece::~Piece() = default;

CFDE_TextOut::CFDE_TextOut() : txt_break_(std::make_unique<CFGAS_TxtBreak>()) {}

CFDE_TextOut::~CFDE_TextOut() = default;

void CFDE_TextOut::SetFont(RetainPtr<CFGAS_GEFont> pFont) {
  DCHECK(pFont);
  font_ = std::move(pFont);
  txt_break_->SetFont(font_);
}

void CFDE_TextOut::SetFontSize(float fFontSize) {
  DCHECK(fFontSize > 0);
  font_size_ = fFontSize;
  txt_break_->SetFontSize(fFontSize);
}

void CFDE_TextOut::SetStyles(const FDE_TextStyle& dwStyles) {
  styles_ = dwStyles;
  txt_bk_styles_ = styles_.single_line_ ? CFGAS_Break::LayoutStyle::kSingleLine
                                        : CFGAS_Break::LayoutStyle::kNone;

  txt_break_->SetLayoutStyles(txt_bk_styles_);
}

void CFDE_TextOut::SetAlignment(FDE_TextAlignment iAlignment) {
  alignment_ = iAlignment;

  int32_t txtBreakAlignment = 0;
  switch (alignment_) {
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
  txt_break_->SetAlignment(txtBreakAlignment);
}

void CFDE_TextOut::SetLineSpace(float fLineSpace) {
  DCHECK(fLineSpace > 1.0f);
  line_space_ = fLineSpace;
}

void CFDE_TextOut::SetLineBreakTolerance(float fTolerance) {
  tolerance_ = fTolerance;
  txt_break_->SetLineBreakTolerance(tolerance_);
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

  DCHECK(font_);
  DCHECK(font_size_ >= 1.0f);

  if (!styles_.single_line_) {
    if (pRect->Width() < 1.0f) {
      pRect->width = font_size_ * 1000.0f;
    }

    txt_break_->SetLineWidth(pRect->Width());
  }

  total_lines_ = 0;
  float fWidth = 0.0f;
  float fHeight = 0.0f;
  float fStartPos = pRect->right();
  CFGAS_Char::BreakType dwBreakStatus = CFGAS_Char::BreakType::kNone;
  bool break_char_is_set = false;
  for (const wchar_t& wch : str) {
    if (!break_char_is_set && (wch == L'\n' || wch == L'\r')) {
      break_char_is_set = true;
      txt_break_->SetParagraphBreakChar(wch);
    }
    dwBreakStatus = txt_break_->AppendChar(wch);
    if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
      RetrieveLineWidth(dwBreakStatus, &fStartPos, &fWidth, &fHeight);
    }
  }

  dwBreakStatus = txt_break_->EndBreak(CFGAS_Char::BreakType::kParagraph);
  if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
    RetrieveLineWidth(dwBreakStatus, &fStartPos, &fWidth, &fHeight);
  }

  txt_break_->Reset();
  float fInc = pRect->Height() - fHeight;
  if (TextAlignmentVerticallyCentered(alignment_)) {
    fInc /= 2.0f;
  } else if (IsTextAlignmentTop(alignment_)) {
    fInc = 0.0f;
  }

  pRect->left += fStartPos;
  pRect->top += fInc;
  pRect->width = std::min(fWidth, pRect->Width());
  pRect->height = fHeight;
  if (styles_.last_line_height_) {
    pRect->height -= line_space_ - font_size_;
  }
}

bool CFDE_TextOut::RetrieveLineWidth(CFGAS_Char::BreakType dwBreakStatus,
                                     float* pStartPos,
                                     float* pWidth,
                                     float* pHeight) {
  if (CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
    return false;
  }

  float fLineStep = std::max(line_space_, font_size_);
  float fLineWidth = 0.0f;
  for (int32_t i = 0; i < txt_break_->CountBreakPieces(); i++) {
    const CFGAS_BreakPiece* pPiece = txt_break_->GetBreakPieceUnstable(i);
    fLineWidth += static_cast<float>(pPiece->GetWidth()) / 20000.0f;
    *pStartPos = std::min(*pStartPos,
                          static_cast<float>(pPiece->GetStartPos()) / 20000.0f);
  }
  txt_break_->ClearBreakPieces();

  if (dwBreakStatus == CFGAS_Char::BreakType::kParagraph) {
    txt_break_->Reset();
  }
  if (!styles_.line_wrap_ && dwBreakStatus == CFGAS_Char::BreakType::kLine) {
    *pWidth += fLineWidth;
  } else {
    *pWidth = std::max(*pWidth, fLineWidth);
    *pHeight += fLineStep;
  }
  ++total_lines_;
  return true;
}

void CFDE_TextOut::DrawLogicText(CFX_RenderDevice* device,
                                 const WideString& str,
                                 const CFX_RectF& rect) {
  DCHECK(font_);
  DCHECK(font_size_ >= 1.0f);

  if (str.IsEmpty()) {
    return;
  }
  if (rect.width < font_size_ || rect.height < font_size_) {
    return;
  }

  float fLineWidth = rect.width;
  txt_break_->SetLineWidth(fLineWidth);
  tto_lines_.clear();
  text_.clear();

  LoadText(str, rect);
  Reload(rect);
  DoAlignment(rect);

  if (!device || tto_lines_.empty()) {
    return;
  }

  CFX_RectF rtClip = matrix_.TransformRect(CFX_RectF());
  device->SaveState();
  if (rtClip.Width() > 0.0f && rtClip.Height() > 0.0f) {
    device->SetClip_Rect(rtClip.GetOuterRect());
  }

  for (auto& line : tto_lines_) {
    for (size_t i = 0; i < line.GetSize(); ++i) {
      const Piece* pPiece = line.GetPieceAtIndex(i);
      size_t szCount = GetDisplayPos(pPiece);
      if (szCount == 0) {
        continue;
      }
      CFDE_TextOut::DrawString(device, txt_color_, font_,
                               pdfium::span(char_pos_).first(szCount),
                               font_size_, matrix_);
    }
  }
  device->RestoreState(false);
}

void CFDE_TextOut::LoadText(const WideString& str, const CFX_RectF& rect) {
  DCHECK(!str.IsEmpty());

  text_ = str;

  if (char_widths_.size() < str.GetLength()) {
    char_widths_.resize(str.GetLength(), 0);
  }

  float fLineStep = std::max(line_space_, font_size_);
  float fLineStop = rect.bottom();
  line_pos_ = rect.top;
  size_t start_char = 0;
  int32_t iPieceWidths = 0;
  CFGAS_Char::BreakType dwBreakStatus;
  bool bRet = false;
  for (const auto& wch : str) {
    dwBreakStatus = txt_break_->AppendChar(wch);
    if (CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
      continue;
    }

    bool bEndofLine =
        RetrievePieces(dwBreakStatus, false, rect, &start_char, &iPieceWidths);
    if (bEndofLine && (styles_.line_wrap_ ||
                       dwBreakStatus == CFGAS_Char::BreakType::kParagraph ||
                       dwBreakStatus == CFGAS_Char::BreakType::kPage)) {
      iPieceWidths = 0;
      ++cur_line_;
      line_pos_ += fLineStep;
    }
    if (line_pos_ + fLineStep > fLineStop) {
      size_t iCurLine = bEndofLine ? cur_line_ - 1 : cur_line_;
      CHECK_LT(cur_line_, tto_lines_.size());
      tto_lines_[iCurLine].set_new_reload(true);
      bRet = true;
      break;
    }
  }

  dwBreakStatus = txt_break_->EndBreak(CFGAS_Char::BreakType::kParagraph);
  if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus) && !bRet) {
    RetrievePieces(dwBreakStatus, false, rect, &start_char, &iPieceWidths);
  }

  txt_break_->ClearBreakPieces();
  txt_break_->Reset();
}

bool CFDE_TextOut::RetrievePieces(CFGAS_Char::BreakType dwBreakStatus,
                                  bool bReload,
                                  const CFX_RectF& rect,
                                  size_t* pStartChar,
                                  int32_t* pPieceWidths) {
  float fLineStep = std::max(line_space_, font_size_);
  bool bNeedReload = false;
  int32_t iLineWidth = FXSYS_roundf(rect.Width() * 20000.0f);
  int32_t iCount = txt_break_->CountBreakPieces();

  size_t chars_to_skip = *pStartChar;
  for (int32_t i = 0; i < iCount; i++) {
    const CFGAS_BreakPiece* pPiece = txt_break_->GetBreakPieceUnstable(i);
    size_t iPieceChars = pPiece->GetLength();
    if (chars_to_skip > iPieceChars) {
      chars_to_skip -= iPieceChars;
      continue;
    }

    size_t iChar = *pStartChar;
    int32_t iWidth = 0;
    size_t j = chars_to_skip;
    for (; j < iPieceChars; j++) {
      const CFGAS_Char* pTC = pPiece->GetChar(j);
      int32_t iCurCharWidth = std::max(pTC->char_width_, 0);
      if (styles_.single_line_ || !styles_.line_wrap_) {
        if (iLineWidth - *pPieceWidths - iWidth < iCurCharWidth) {
          bNeedReload = true;
          break;
        }
      }
      iWidth += iCurCharWidth;
      char_widths_[iChar++] = iCurCharWidth;
    }

    if (j == chars_to_skip && !bReload) {
      CHECK_LT(cur_line_, tto_lines_.size());
      tto_lines_[cur_line_].set_new_reload(true);
    } else if (j > chars_to_skip) {
      Piece piece;
      piece.start_char = *pStartChar;
      piece.char_count = j - chars_to_skip;
      piece.char_styles = pPiece->GetCharStyles();
      piece.bounds = CFX_RectF(
          rect.left + static_cast<float>(pPiece->GetStartPos()) / 20000.0f,
          line_pos_, iWidth / 20000.0f, fLineStep);

      if (FX_IsOdd(pPiece->GetBidiLevel())) {
        piece.char_styles |= FX_TXTCHARSTYLE_OddBidiLevel;
      }

      AppendPiece(piece, bNeedReload, (bReload && i == iCount - 1));
    }
    *pStartChar += iPieceChars;
    *pPieceWidths += iWidth;
  }
  txt_break_->ClearBreakPieces();

  return styles_.single_line_ || styles_.line_wrap_ || bNeedReload ||
         dwBreakStatus == CFGAS_Char::BreakType::kParagraph;
}

void CFDE_TextOut::AppendPiece(const Piece& piece,
                               bool bNeedReload,
                               bool bEnd) {
  if (cur_line_ >= tto_lines_.size()) {
    Line ttoLine;
    ttoLine.set_new_reload(bNeedReload);

    cur_piece_ = ttoLine.AddPiece(cur_piece_, piece);
    tto_lines_.push_back(ttoLine);
    cur_line_ = tto_lines_.size() - 1;
  } else {
    Line* pLine = &tto_lines_[cur_line_];
    pLine->set_new_reload(bNeedReload);

    cur_piece_ = pLine->AddPiece(cur_piece_, piece);
    if (bEnd) {
      size_t iPieces = pLine->GetSize();
      if (cur_piece_ < iPieces) {
        pLine->RemoveLast(iPieces - cur_piece_ - 1);
      }
    }
  }
  if (!bEnd && bNeedReload) {
    cur_piece_ = 0;
  }
}

void CFDE_TextOut::Reload(const CFX_RectF& rect) {
  size_t i = 0;
  for (auto& line : tto_lines_) {
    if (line.new_reload()) {
      cur_line_ = i;
      cur_piece_ = 0;
      ReloadLinePiece(&line, rect);
    }
    ++i;
  }
}

void CFDE_TextOut::ReloadLinePiece(Line* line, const CFX_RectF& rect) {
  span<const wchar_t> text_span = text_.span();
  size_t start_char = 0;
  size_t piece_count = line->GetSize();
  int32_t piece_widths = 0;
  CFGAS_Char::BreakType break_status = CFGAS_Char::BreakType::kNone;
  for (size_t piece_index = 0; piece_index < piece_count; ++piece_index) {
    const Piece* piece = line->GetPieceAtIndex(piece_index);
    if (piece_index == 0) {
      line_pos_ = piece->bounds.top;
    }

    start_char = piece->start_char;
    const size_t end = piece->start_char + piece->char_count;
    for (size_t char_index = start_char; char_index < end; ++char_index) {
      break_status = txt_break_->AppendChar(text_span[char_index]);
      if (!CFX_BreakTypeNoneOrPiece(break_status)) {
        RetrievePieces(break_status, true, rect, &start_char, &piece_widths);
      }
    }
  }

  break_status = txt_break_->EndBreak(CFGAS_Char::BreakType::kParagraph);
  if (!CFX_BreakTypeNoneOrPiece(break_status)) {
    RetrievePieces(break_status, true, rect, &start_char, &piece_widths);
  }

  txt_break_->Reset();
}

void CFDE_TextOut::DoAlignment(const CFX_RectF& rect) {
  if (tto_lines_.empty()) {
    return;
  }

  const Piece* pFirstPiece = tto_lines_.back().GetPieceAtIndex(0);
  if (!pFirstPiece) {
    return;
  }

  float fInc = rect.bottom() - pFirstPiece->bounds.bottom();
  if (TextAlignmentVerticallyCentered(alignment_)) {
    fInc /= 2.0f;
  } else if (IsTextAlignmentTop(alignment_)) {
    fInc = 0.0f;
  }

  if (fInc < 1.0f) {
    return;
  }

  for (auto& line : tto_lines_) {
    for (size_t i = 0; i < line.GetSize(); ++i) {
      line.GetPieceAtIndex(i)->bounds.top += fInc;
    }
  }
}

size_t CFDE_TextOut::GetDisplayPos(const Piece* pPiece) {
  if (char_pos_.size() < pPiece->char_count) {
    char_pos_.resize(pPiece->char_count, TextCharPos());
  }

  CFGAS_TxtBreak::Run tr;
  tr.wsStr = text_.Substr(pPiece->start_char);
  tr.pWidths = pdfium::span(char_widths_).subspan(pPiece->start_char);
  tr.iLength = checked_cast<int32_t>(pPiece->char_count);
  tr.pFont = font_;
  tr.fFontSize = font_size_;
  tr.dwStyles = txt_bk_styles_;
  tr.dwCharStyles = pPiece->char_styles;
  tr.pRect = &pPiece->bounds;

  return txt_break_->GetDisplayPos(tr, char_pos_);
}

CFDE_TextOut::Line::Line() = default;

CFDE_TextOut::Line::Line(const Line& that) = default;

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

}  // namespace pdfium
