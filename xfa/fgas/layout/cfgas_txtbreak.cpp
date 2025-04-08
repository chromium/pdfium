// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_txtbreak.h"

#include <algorithm>
#include <array>

#include "build/build_config.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/containers/adapters.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/text_char_pos.h"
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
  line_width_ = FXSYS_roundf(fLineWidth * kConversionFactor);
  DCHECK(line_width_ >= 20000);
}

void CFGAS_TxtBreak::SetAlignment(int32_t iAlignment) {
  DCHECK(iAlignment >= CFX_TxtLineAlignment_Left);
  DCHECK(iAlignment <= CFX_TxtLineAlignment_Justified);
  alignment_ = iAlignment;
}

void CFGAS_TxtBreak::SetCombWidth(float fCombWidth) {
  comb_width_ = FXSYS_roundf(fCombWidth * kConversionFactor);
}

void CFGAS_TxtBreak::AppendChar_Combination(CFGAS_Char* pCurChar) {
  FX_SAFE_INT32 iCharWidth = comb_width_;
  pCurChar->char_width_ = -1;
  if (!comb_text_) {
    wchar_t wch = pCurChar->char_code();
    CFGAS_Char* pLastChar = GetLastChar(0, false, false);
    if (pLastChar &&
        (pLastChar->char_styles_ & FX_TXTCHARSTYLE_ArabicShadda) == 0) {
      wchar_t wLast = pLastChar->char_code();
      std::optional<uint16_t> maybe_shadda;
      if (wch == pdfium::kArabicShadda) {
        maybe_shadda = pdfium::GetArabicFromShaddaTable(wLast);
      } else if (wLast == pdfium::kArabicShadda) {
        maybe_shadda = pdfium::GetArabicFromShaddaTable(wch);
      }
      if (maybe_shadda.has_value()) {
        wch = maybe_shadda.value();
        pCurChar->char_styles_ |= FX_TXTCHARSTYLE_ArabicShadda;
        pLastChar->char_styles_ |= FX_TXTCHARSTYLE_ArabicShadda;
        pLastChar->char_width_ = 0;
      }
    }
    std::optional<uint16_t> iCharWidthRet;
    if (font_) {
      iCharWidthRet = font_->GetCharWidth(wch);
    }
    iCharWidth = iCharWidthRet.value_or(0);
    iCharWidth *= font_size_;
    iCharWidth *= horizontal_scale_;
    iCharWidth /= 100;
  }
  iCharWidth *= -1;
  pCurChar->char_width_ = iCharWidth.ValueOrDefault(0);
}

void CFGAS_TxtBreak::AppendChar_Tab(CFGAS_Char* pCurChar) {
  char_type_ = FX_CHARTYPE::kTab;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar_Control(CFGAS_Char* pCurChar) {
  char_type_ = FX_CHARTYPE::kControl;
  CFGAS_Char::BreakType dwRet = CFGAS_Char::BreakType::kNone;
  if (!single_line_) {
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
        if (wch == w_paragraph_break_char_) {
          dwRet = CFGAS_Char::BreakType::kParagraph;
        }
        break;
    }
    if (dwRet != CFGAS_Char::BreakType::kNone) {
      dwRet = EndBreak(dwRet);
    }
  }
  return dwRet;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar_Arabic(CFGAS_Char* pCurChar) {
  FX_CHARTYPE chartype = pCurChar->GetCharType();
  int32_t& iLineWidth = cur_line_->width_;
  wchar_t wForm;
  CFGAS_Char* pLastChar = nullptr;
  bool bAlef = false;
  if (!comb_text_ && char_type_ >= FX_CHARTYPE::kArabicAlef &&
      char_type_ <= FX_CHARTYPE::kArabicDistortion) {
    FX_SAFE_INT32 iCharWidth = 0;
    pLastChar = GetLastChar(1, true, false);
    if (pLastChar) {
      if (pLastChar->char_width_ > 0) {
        iLineWidth -= pLastChar->char_width_;
      }
      iCharWidth = pLastChar->char_width_;

      CFGAS_Char* pPrevChar = GetLastChar(2, true, false);
      wForm = pdfium::GetArabicFormChar(pLastChar, pPrevChar, pCurChar);
      bAlef = (wForm == pdfium::unicode::kZeroWidthNoBreakSpace &&
               pLastChar->GetCharType() == FX_CHARTYPE::kArabicAlef);
      if (font_) {
        iCharWidth = font_->GetCharWidth(wForm).value_or(0);
      }
      if (wForm == pdfium::unicode::kZeroWidthNoBreakSpace) {
        iCharWidth = 0;
      }

      iCharWidth *= font_size_;
      iCharWidth *= horizontal_scale_;
      iCharWidth /= 100;

      int32_t iCharWidthValid = iCharWidth.ValueOrDefault(0);
      pLastChar->char_width_ = iCharWidthValid;
      iLineWidth += iCharWidthValid;
    }
  }

  char_type_ = chartype;
  wForm =
      pdfium::GetArabicFormChar(pCurChar, bAlef ? nullptr : pLastChar, nullptr);
  FX_SAFE_INT32 iCharWidth = 0;
  if (comb_text_) {
    iCharWidth = comb_width_;
  } else {
    if (font_ && wForm != pdfium::unicode::kZeroWidthNoBreakSpace) {
      iCharWidth = font_->GetCharWidth(wForm).value_or(0);
    }
    iCharWidth *= font_size_;
    iCharWidth *= horizontal_scale_;
    iCharWidth /= 100;
  }

  int32_t iCharWidthValid = iCharWidth.ValueOrDefault(0);
  pCurChar->char_width_ = iCharWidthValid;
  iLineWidth += iCharWidthValid;

  cur_line_->IncrementArabicCharCount();
  if (!single_line_ && IsGreaterThanLineWidth(iLineWidth)) {
    return EndBreak(CFGAS_Char::BreakType::kLine);
  }
  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar_Others(CFGAS_Char* pCurChar) {
  FX_CHARTYPE chartype = pCurChar->GetCharType();
  int32_t& iLineWidth = cur_line_->width_;
  char_type_ = chartype;
  wchar_t wch = pCurChar->char_code();
  wchar_t wForm = wch;

  FX_SAFE_INT32 iCharWidth = 0;
  if (comb_text_) {
    iCharWidth = comb_width_;
  } else if (font_) {
    iCharWidth = font_->GetCharWidth(wForm).value_or(0);
    iCharWidth *= font_size_;
    iCharWidth *= horizontal_scale_;
    iCharWidth /= 100;
  }
  iCharWidth += char_space_;

  int32_t iValidCharWidth = iCharWidth.ValueOrDefault(0);
  pCurChar->char_width_ = iValidCharWidth;
  iLineWidth += iValidCharWidth;
  if (!single_line_ && chartype != FX_CHARTYPE::kSpace &&
      IsGreaterThanLineWidth(iLineWidth)) {
    return EndBreak(CFGAS_Char::BreakType::kLine);
  }

  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_TxtBreak::AppendChar(wchar_t wch) {
  FX_CHARTYPE chartype = pdfium::unicode::GetCharType(wch);
  cur_line_->line_chars_.emplace_back(wch, horizontal_scale_, vertical_scale_);
  CFGAS_Char* pCurChar = &cur_line_->line_chars_.back();
  pCurChar->char_styles_ = alignment_ | (1 << 8);

  CFGAS_Char::BreakType dwRet1 = CFGAS_Char::BreakType::kNone;
  if (chartype != FX_CHARTYPE::kCombination &&
      GetUnifiedCharType(char_type_) != GetUnifiedCharType(chartype) &&
      char_type_ != FX_CHARTYPE::kUnknown && !single_line_ &&
      IsGreaterThanLineWidth(cur_line_->width_) &&
      (char_type_ != FX_CHARTYPE::kSpace ||
       chartype != FX_CHARTYPE::kControl)) {
    dwRet1 = EndBreak(CFGAS_Char::BreakType::kLine);
    if (!cur_line_->line_chars_.empty()) {
      pCurChar = &cur_line_->line_chars_.back();
    }
  }

  CFGAS_Char::BreakType dwRet2 = CFGAS_Char::BreakType::kNone;
  if (wch == w_paragraph_break_char_) {
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
  if (!single_line_ && IsGreaterThanLineWidth(cur_line_->width_)) {
    pTC = cur_line_->LastChar();
    switch (pTC->GetCharType()) {
      case FX_CHARTYPE::kTab:
      case FX_CHARTYPE::kControl:
      case FX_CHARTYPE::kSpace:
        break;
      default:
        SplitTextLine(cur_line_, pNextLine, bAllChars);
        bDone = true;
        break;
    }
  }
  if (bAllChars && !bDone) {
    int32_t iEndPos = cur_line_->width_;
    GetBreakPos(&cur_line_->line_chars_, bAllChars, true, &iEndPos);
  }
}

std::deque<CFGAS_Break::TPO> CFGAS_TxtBreak::EndBreakBidiLine(
    CFGAS_Char::BreakType dwStatus) {
  CFGAS_BreakPiece tp;
  std::deque<TPO> tpos;
  CFGAS_Char* pTC;
  std::vector<CFGAS_Char>& chars = cur_line_->line_chars_;
  if (!cur_line_->HasArabicChar()) {
    tp.SetStatus(dwStatus);
    tp.SetStartPos(cur_line_->start_);
    tp.SetWidth(cur_line_->width_);
    tp.SetStartChar(0);
    tp.SetCharCount(fxcrt::CollectionSize<int32_t>(cur_line_->line_chars_));
    tp.SetChars(&cur_line_->line_chars_);
    pTC = &chars[0];
    tp.SetCharStyles(pTC->char_styles_);
    tp.SetHorizontalScale(pTC->horizonal_scale());
    tp.SetVerticalScale(pTC->vertical_scale());
    cur_line_->line_pieces_.push_back(tp);
    tpos.push_back({0, 0});
    return tpos;
  }

  size_t iBidiNum = 0;
  for (size_t i = 0; i < cur_line_->line_chars_.size(); ++i) {
    pTC = &chars[i];
    pTC->bidi_pos_ = static_cast<int32_t>(i);
    if (pTC->GetCharType() != FX_CHARTYPE::kControl) {
      iBidiNum = i;
    }
    if (i == 0) {
      pTC->bidi_level_ = 1;
    }
  }
  CFGAS_Char::BidiLine(&chars, iBidiNum + 1);

  tp.SetStatus(CFGAS_Char::BreakType::kPiece);
  tp.SetStartPos(cur_line_->start_);
  tp.SetChars(&cur_line_->line_chars_);
  int32_t iBidiLevel = -1;
  int32_t iCharWidth;
  int32_t i = 0;
  int32_t j = -1;
  int32_t iCount = fxcrt::CollectionSize<int32_t>(cur_line_->line_chars_);
  while (i < iCount) {
    pTC = &chars[i];
    if (iBidiLevel < 0) {
      iBidiLevel = pTC->bidi_level_;
      tp.SetWidth(0);
      tp.SetBidiLevel(iBidiLevel);
      tp.SetBidiPos(pTC->bidi_order_);
      tp.SetCharStyles(pTC->char_styles_);
      tp.SetHorizontalScale(pTC->horizonal_scale());
      tp.SetVerticalScale(pTC->vertical_scale());
      tp.SetStatus(CFGAS_Char::BreakType::kPiece);
    }
    if (iBidiLevel != pTC->bidi_level_ ||
        pTC->status_ != CFGAS_Char::BreakType::kNone) {
      if (iBidiLevel == pTC->bidi_level_) {
        tp.SetStatus(pTC->status_);
        iCharWidth = pTC->char_width_;
        if (iCharWidth > 0) {
          tp.IncrementWidth(iCharWidth);
        }

        i++;
      }
      tp.SetCharCount(i - tp.GetStartChar());
      cur_line_->line_pieces_.push_back(tp);
      tp.IncrementStartPos(tp.GetWidth());
      tp.SetStartChar(i);
      tpos.push_back({++j, tp.GetBidiPos()});
      iBidiLevel = -1;
    } else {
      iCharWidth = pTC->char_width_;
      if (iCharWidth > 0) {
        tp.IncrementWidth(iCharWidth);
      }

      i++;
    }
  }
  if (i > tp.GetStartChar()) {
    tp.SetStatus(dwStatus);
    tp.SetCharCount(i - tp.GetStartChar());
    cur_line_->line_pieces_.push_back(tp);
    tpos.push_back({++j, tp.GetBidiPos()});
  }
  if (j > -1) {
    if (j > 0) {
      std::sort(tpos.begin(), tpos.end());
      int32_t iStartPos = 0;
      for (i = 0; i <= j; i++) {
        CFGAS_BreakPiece& ttp = cur_line_->line_pieces_[tpos[i].index];
        ttp.SetStartPos(iStartPos);
        iStartPos += ttp.GetWidth();
      }
    }
    cur_line_->line_pieces_[j].SetStatus(dwStatus);
  }
  return tpos;
}

void CFGAS_TxtBreak::EndBreakAlignment(const std::deque<TPO>& tpos,
                                       bool bAllChars,
                                       CFGAS_Char::BreakType dwStatus) {
  int32_t iNetWidth = cur_line_->width_;
  int32_t iGapChars = 0;
  bool bFind = false;
  for (const TPO& pos : pdfium::Reversed(tpos)) {
    const CFGAS_BreakPiece& ttp = cur_line_->line_pieces_[pos.index];
    if (!bFind) {
      iNetWidth = ttp.GetEndPos();
    }

    bool bArabic = FX_IsOdd(ttp.GetBidiLevel());
    int32_t j = bArabic ? 0 : ttp.GetCharCount() - 1;
    while (j > -1 && j < ttp.GetCharCount()) {
      const CFGAS_Char* pTC = ttp.GetChar(j);
      if (pTC->line_break_type_ == FX_LINEBREAKTYPE::kDIRECT_BRK) {
        iGapChars++;
      }
      if (!bFind || !bAllChars) {
        FX_CHARTYPE chartype = pTC->GetCharType();
        if (chartype == FX_CHARTYPE::kSpace ||
            chartype == FX_CHARTYPE::kControl) {
          if (!bFind && bAllChars && pTC->char_width_ > 0) {
            iNetWidth -= pTC->char_width_;
          }
        } else {
          bFind = true;
          if (!bAllChars) {
            break;
          }
        }
      }
      j += bArabic ? 1 : -1;
    }
    if (!bAllChars && bFind) {
      break;
    }
  }

  int32_t iOffset = line_width_ - iNetWidth;
  if (iGapChars > 0 && alignment_ & CFX_TxtLineAlignment_Justified &&
      dwStatus != CFGAS_Char::BreakType::kParagraph) {
    int32_t iStart = -1;
    for (auto& tpo : tpos) {
      CFGAS_BreakPiece& ttp = cur_line_->line_pieces_[tpo.index];
      if (iStart < -1) {
        iStart = ttp.GetStartPos();
      } else {
        ttp.SetStartPos(iStart);
      }

      for (int32_t j = 0; j < ttp.GetCharCount() && iGapChars > 0;
           j++, iGapChars--) {
        CFGAS_Char* pTC = ttp.GetChar(j);
        if (pTC->line_break_type_ != FX_LINEBREAKTYPE::kDIRECT_BRK ||
            pTC->char_width_ < 0) {
          continue;
        }
        int32_t k = iOffset / iGapChars;
        pTC->char_width_ += k;
        ttp.IncrementWidth(k);
        iOffset -= k;
      }
      iStart += ttp.GetWidth();
    }
  } else if (alignment_ & CFX_TxtLineAlignment_Center ||
             alignment_ & CFX_TxtLineAlignment_Right) {
    if (alignment_ & CFX_TxtLineAlignment_Center &&
        !(alignment_ & CFX_TxtLineAlignment_Right)) {
      iOffset /= 2;
    }
    if (iOffset > 0) {
      for (auto& ttp : cur_line_->line_pieces_) {
        ttp.IncrementStartPos(iOffset);
      }
    }
  }
}

CFGAS_Char::BreakType CFGAS_TxtBreak::EndBreak(CFGAS_Char::BreakType dwStatus) {
  DCHECK(dwStatus != CFGAS_Char::BreakType::kNone);

  if (!cur_line_->line_pieces_.empty()) {
    if (dwStatus != CFGAS_Char::BreakType::kPiece) {
      cur_line_->line_pieces_.back().SetStatus(dwStatus);
    }
    return cur_line_->line_pieces_.back().GetStatus();
  }

  if (HasLine()) {
    if (lines_[ready_line_index_].line_pieces_.empty()) {
      return CFGAS_Char::BreakType::kNone;
    }

    if (dwStatus != CFGAS_Char::BreakType::kPiece) {
      lines_[ready_line_index_].line_pieces_.back().SetStatus(dwStatus);
    }
    return lines_[ready_line_index_].line_pieces_.back().GetStatus();
  }

  if (cur_line_->line_chars_.empty()) {
    return CFGAS_Char::BreakType::kNone;
  }

  cur_line_->line_chars_.back().status_ = dwStatus;
  if (dwStatus == CFGAS_Char::BreakType::kPiece) {
    return dwStatus;
  }

  ready_line_index_ = cur_line_ == &lines_[0] ? 0 : 1;
  CFGAS_BreakLine* pNextLine = &lines_[1 - ready_line_index_];
  const bool bAllChars = alignment_ > CFX_TxtLineAlignment_Right;
  EndBreakSplitLine(pNextLine, bAllChars);

  std::deque<TPO> tpos = EndBreakBidiLine(dwStatus);
  if (alignment_ > CFX_TxtLineAlignment_Left) {
    EndBreakAlignment(tpos, bAllChars, dwStatus);
  }

  cur_line_ = pNextLine;
  CFGAS_Char* pTC = GetLastChar(0, false, false);
  char_type_ = pTC ? pTC->GetCharType() : FX_CHARTYPE::kUnknown;
  return dwStatus;
}

int32_t CFGAS_TxtBreak::GetBreakPos(std::vector<CFGAS_Char>* pChars,
                                    bool bAllChars,
                                    bool bOnlyBrk,
                                    int32_t* pEndPos) {
  std::vector<CFGAS_Char>& chars = *pChars;
  int32_t iLength = fxcrt::CollectionSize<int32_t>(chars) - 1;
  if (iLength < 1) {
    return iLength;
  }

  int32_t iBreak = -1;
  int32_t iBreakPos = -1;
  int32_t iIndirect = -1;
  int32_t iIndirectPos = -1;
  int32_t iLast = -1;
  int32_t iLastPos = -1;
  if (single_line_ || *pEndPos <= line_width_) {
    if (!bAllChars) {
      return iLength;
    }

    iBreak = iLength;
    iBreakPos = *pEndPos;
  }

  FX_LINEBREAKTYPE eType;
  FX_BREAKPROPERTY nCur;
  FX_BREAKPROPERTY nNext;
  CFGAS_Char* pCur = &chars[iLength--];
  if (bAllChars) {
    pCur->line_break_type_ = FX_LINEBREAKTYPE::kUNKNOWN;
  }

  nNext = pdfium::unicode::GetBreakProperty(pCur->char_code());
  int32_t iCharWidth = pCur->char_width_;
  if (iCharWidth > 0) {
    *pEndPos -= iCharWidth;
  }

  while (iLength >= 0) {
    pCur = &chars[iLength];
    nCur = pdfium::unicode::GetBreakProperty(pCur->char_code());
    if (nNext == FX_BREAKPROPERTY::kSP) {
      eType = FX_LINEBREAKTYPE::kPROHIBITED_BRK;
    } else {
      eType = GetLineBreakTypeFromPair(nCur, nNext);
    }
    if (bAllChars) {
      pCur->line_break_type_ = eType;
    }
    if (!bOnlyBrk) {
      if (single_line_ || *pEndPos <= line_width_ ||
          nCur == FX_BREAKPROPERTY::kSP) {
        if (eType == FX_LINEBREAKTYPE::kDIRECT_BRK && iBreak < 0) {
          iBreak = iLength;
          iBreakPos = *pEndPos;
          if (!bAllChars) {
            return iLength;
          }
        } else if (eType == FX_LINEBREAKTYPE::kINDIRECT_BRK && iIndirect < 0) {
          iIndirect = iLength;
          iIndirectPos = *pEndPos;
        }
        if (iLast < 0) {
          iLast = iLength;
          iLastPos = *pEndPos;
        }
      }
      iCharWidth = pCur->char_width_;
      if (iCharWidth > 0) {
        *pEndPos -= iCharWidth;
      }
    }
    nNext = nCur;
    iLength--;
  }
  if (bOnlyBrk) {
    return 0;
  }
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

  if (pCurLine->line_chars_.size() < 2) {
    return;
  }

  int32_t iEndPos = pCurLine->width_;
  std::vector<CFGAS_Char>& curChars = pCurLine->line_chars_;
  int32_t iCharPos = GetBreakPos(&curChars, bAllChars, false, &iEndPos);
  if (iCharPos < 0) {
    iCharPos = 0;
  }

  iCharPos++;
  if (iCharPos >= fxcrt::CollectionSize<int32_t>(pCurLine->line_chars_)) {
    pNextLine->Clear();
    CFGAS_Char* pTC = &curChars[iCharPos - 1];
    pTC->line_break_type_ = FX_LINEBREAKTYPE::kUNKNOWN;
    return;
  }

  pNextLine->line_chars_ =
      std::vector<CFGAS_Char>(curChars.begin() + iCharPos, curChars.end());
  curChars.erase(curChars.begin() + iCharPos, curChars.end());
  pCurLine->width_ = iEndPos;
  CFGAS_Char* pTC = &curChars[iCharPos - 1];
  pTC->line_break_type_ = FX_LINEBREAKTYPE::kUNKNOWN;
  int32_t iWidth = 0;
  for (size_t i = 0; i < pNextLine->line_chars_.size(); ++i) {
    if (pNextLine->line_chars_[i].GetCharType() >= FX_CHARTYPE::kArabicAlef) {
      pCurLine->DecrementArabicCharCount();
      pNextLine->IncrementArabicCharCount();
    }
    iWidth += std::max(0, pNextLine->line_chars_[i].char_width_);
    pNextLine->line_chars_[i].status_ = CFGAS_Char::BreakType::kNone;
  }
  pNextLine->width_ = iWidth;
}

size_t CFGAS_TxtBreak::GetDisplayPos(const Run& run,
                                     pdfium::span<TextCharPos> pCharPos) const {
  if (run.iLength < 1) {
    return 0;
  }

  Engine* pEngine = run.pEdtEngine;
  WideStringView pStr = run.wsStr.AsStringView();
  pdfium::span<int32_t> pWidths = run.pWidths;
  int32_t iLength = run.iLength - 1;
  RetainPtr<CFGAS_GEFont> pFont = run.pFont;
  Mask<LayoutStyle> dwStyles = run.dwStyles;
  CFX_RectF rtText(*run.pRect);
  const bool bRTLPiece = (run.dwCharStyles & FX_TXTCHARSTYLE_OddBidiLevel) != 0;
  const float fFontSize = run.fFontSize;
  const int32_t iFontSize = FXSYS_roundf(fFontSize * 20.0f);
  const int32_t iAscent = pFont->GetAscent();
  const int32_t iDescent = pFont->GetDescent();
  const int32_t iMaxHeight = iAscent - iDescent;
  const float fAscent = iMaxHeight ? fFontSize * iAscent / iMaxHeight : 0;
  int32_t iHorScale = run.iHorizontalScale;
  int32_t iVerScale = run.iVerticalScale;
  bool bSkipSpace = run.bSkipSpace;

  const float fYBase = rtText.top + (rtText.height - fFontSize) / 2.0f;
  float fX = bRTLPiece ? rtText.right() : rtText.left;
  float fY = fYBase + fAscent;

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
      wch = pStr.Front();
      pStr = pStr.Substr(1);
      iWidth = pWidths.front();
      pWidths = pWidths.subspan(1);
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
          if (iNext > iLength) {
            wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
          }
        } else {
          int32_t j = -1;
          do {
            j++;
            if (i + j >= iLength) {
              break;
            }
            wNext = pStr[j];
          } while (pdfium::unicode::GetCharType(wNext) ==
                   FX_CHARTYPE::kCombination);
          if (i + j >= iLength) {
            wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
          }
        }
      } else {
        wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
      }

      wForm = pdfium::GetArabicFormChar(wch, wPrev, wNext);
      bLam = (wPrev == pdfium::kArabicLetterLam &&
              wch == pdfium::kArabicLetterLam &&
              wNext == pdfium::kArabicLetterHeh);
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
            wNext = pStr.Front();
          }
          std::optional<uint16_t> maybe_shadda;
          if (wch == pdfium::kArabicShadda) {
            maybe_shadda = pdfium::GetArabicFromShaddaTable(wNext);
          } else if (wNext == pdfium::kArabicShadda) {
            maybe_shadda = pdfium::GetArabicFromShaddaTable(wch);
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
    if (chartype != FX_CHARTYPE::kCombination) {
      bShadda = false;
    }
    if (chartype < FX_CHARTYPE::kArabicAlef) {
      bLam = false;
    }

    bool bEmptyChar =
        (chartype >= FX_CHARTYPE::kTab && chartype <= FX_CHARTYPE::kControl);
    if (wForm == pdfium::unicode::kZeroWidthNoBreakSpace) {
      bEmptyChar = true;
    }

    int32_t iForms = bLam ? 3 : 1;
    szCount += (bEmptyChar && bSkipSpace) ? 0 : iForms;
    if (pCharPos.empty()) {
      if (iWidth > 0) {
        wPrev = wch;
      }
      wLast = wch;
      continue;
    }

    int32_t iCharWidth = iWidth;
    if (iCharWidth < 0) {
      iCharWidth = -iCharWidth;
    }

    iCharWidth /= iFontSize;
    std::array<FX_FORMCHAR, 3> form_chars;
    form_chars[0].wch = wch;
    form_chars[0].wForm = wForm;
    form_chars[0].iWidth = iCharWidth;
    if (bLam) {
      form_chars[1].wForm = pdfium::kArabicShadda;
      form_chars[1].iWidth =
          pFont->GetCharWidth(pdfium::kArabicShadda).value_or(0);
      form_chars[2].wForm = pdfium::kArabicLetterSuperscriptAlef;
      form_chars[2].iWidth =
          pFont->GetCharWidth(pdfium::kArabicLetterSuperscriptAlef).value_or(0);
    }

    for (int32_t j = 0; j < iForms; j++) {
      TextCharPos& front_ref = pCharPos.front();
      wForm = (wchar_t)form_chars[j].wForm;
      iCharWidth = form_chars[j].iWidth;
      if (j > 0) {
        chartype = FX_CHARTYPE::kCombination;
        wch = wForm;
        wLast = (wchar_t)form_chars[j - 1].wForm;
      }
      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        front_ref.m_GlyphIndex = pFont->GetGlyphIndex(wForm);
#if BUILDFLAG(IS_APPLE)
        front_ref.m_ExtGID = front_ref.m_GlyphIndex;
#endif
        front_ref.m_FontCharWidth = iCharWidth;
      }

      const float fCharWidth = fFontSize * iCharWidth / 1000.0f;
      if (bRTLPiece && chartype != FX_CHARTYPE::kCombination) {
        fX -= fCharWidth;
      }

      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        front_ref.m_Origin = CFX_PointF(fX, fY);

        if (!!(dwStyles & LayoutStyle::kCombText)) {
          int32_t iFormWidth = pFont->GetCharWidth(wForm).value_or(iCharWidth);
          float fOffset = fFontSize * (iCharWidth - iFormWidth) / 2000.0f;
          front_ref.m_Origin.x += fOffset;
        }
        if (chartype == FX_CHARTYPE::kCombination) {
          std::optional<FX_RECT> rtBBox = pFont->GetCharBBox(wForm);
          if (rtBBox.has_value()) {
            front_ref.m_Origin.y =
                fYBase + fFontSize -
                fFontSize * rtBBox.value().Height() / iMaxHeight;
          }
          if (wForm == wch &&
              wLast != pdfium::unicode::kZeroWidthNoBreakSpace) {
            if (pdfium::unicode::GetCharType(wLast) ==
                FX_CHARTYPE::kCombination) {
              std::optional<FX_RECT> rtOtherBox = pFont->GetCharBBox(wLast);
              if (rtOtherBox.has_value()) {
                front_ref.m_Origin.y -=
                    fFontSize * rtOtherBox.value().Height() / iMaxHeight;
              }
            }
          }
        }
      }
      if (!bRTLPiece && chartype != FX_CHARTYPE::kCombination) {
        fX += fCharWidth;
      }

      if (!bEmptyChar || (bEmptyChar && !bSkipSpace)) {
        front_ref.m_bGlyphAdjust = true;
        front_ref.m_AdjustMatrix[0] = -1;
        front_ref.m_AdjustMatrix[1] = 0;
        front_ref.m_AdjustMatrix[2] = 0;
        front_ref.m_AdjustMatrix[3] = 1;

        if (iHorScale != 100 || iVerScale != 100) {
          front_ref.m_AdjustMatrix[0] =
              front_ref.m_AdjustMatrix[0] * iHorScale / 100.0f;
          front_ref.m_AdjustMatrix[1] =
              front_ref.m_AdjustMatrix[1] * iHorScale / 100.0f;
          front_ref.m_AdjustMatrix[2] =
              front_ref.m_AdjustMatrix[2] * iVerScale / 100.0f;
          front_ref.m_AdjustMatrix[3] =
              front_ref.m_AdjustMatrix[3] * iVerScale / 100.0f;
        }
        pCharPos = pCharPos.subspan(1);
      }
    }
    if (iWidth > 0) {
      wPrev = static_cast<wchar_t>(form_chars[0].wch);
    }
    wLast = wch;
  }
  return szCount;
}

std::vector<CFX_RectF> CFGAS_TxtBreak::GetCharRects(const Run& run) const {
  if (run.iLength < 1) {
    return std::vector<CFX_RectF>();
  }

  Engine* pEngine = run.pEdtEngine;
  WideStringView pStr = run.wsStr.AsStringView();
  pdfium::span<int32_t> pWidths = run.pWidths;
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
      wch = pStr.Front();
      pStr = pStr.Substr(1);
      iCharSize = pWidths.front();
      pWidths = pWidths.subspan(1);
    }
    float fCharSize = static_cast<float>(iCharSize) / kConversionFactor;
    bool bRet = (!bSingleLine && IsCtrlCode(wch));
    if (!(wch == L'\v' || wch == L'\f' ||
          wch == pdfium::unicode::kLineSeparator ||
          wch == pdfium::unicode::kParagraphSeparator || wch == L'\n')) {
      bRet = false;
    }
    if (bRet) {
      fCharSize = fFontSize / 2.0f;
    }
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
