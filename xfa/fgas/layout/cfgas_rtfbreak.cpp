// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_rtfbreak.h"

#include <algorithm>

#include "build/build_config.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/containers/adapters.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/safe_math.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/text_char_pos.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_char.h"
#include "xfa/fgas/layout/cfgas_textpiece.h"
#include "xfa/fgas/layout/cfgas_textuserdata.h"
#include "xfa/fgas/layout/fgas_arabic.h"
#include "xfa/fgas/layout/fgas_linebreak.h"

CFGAS_RTFBreak::CFGAS_RTFBreak(Mask<LayoutStyle> dwLayoutStyles)
    : CFGAS_Break(dwLayoutStyles) {
  SetBreakStatus();
  pagination_ = !!(layout_styles_ & LayoutStyle::kPagination);
}

CFGAS_RTFBreak::~CFGAS_RTFBreak() = default;

void CFGAS_RTFBreak::SetLineStartPos(float fLinePos) {
  int32_t iLinePos = FXSYS_roundf(fLinePos * kConversionFactor);
  iLinePos = std::min(iLinePos, line_width_);
  iLinePos = std::max(iLinePos, line_start_);
  cur_line_->start_ = iLinePos;
}

void CFGAS_RTFBreak::AddPositionedTab(float fTabPos) {
  int32_t iTabPos = std::min(
      FXSYS_roundf(fTabPos * kConversionFactor) + line_start_, line_width_);
  auto it = std::ranges::lower_bound(positioned_tabs_, iTabPos);
  if (it != positioned_tabs_.end() && *it == iTabPos) {
    return;
  }
  positioned_tabs_.insert(it, iTabPos);
}

void CFGAS_RTFBreak::SetUserData(
    const RetainPtr<CFGAS_TextUserData>& pUserData) {
  if (user_data_ == pUserData) {
    return;
  }

  SetBreakStatus();
  user_data_ = pUserData;
}

bool CFGAS_RTFBreak::GetPositionedTab(int32_t* iTabPos) const {
  auto it = std::ranges::upper_bound(positioned_tabs_, *iTabPos);
  if (it == positioned_tabs_.end()) {
    return false;
  }

  *iTabPos = *it;
  return true;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::AppendChar(wchar_t wch) {
  DCHECK(cur_line_);

  FX_CHARTYPE chartype = pdfium::unicode::GetCharType(wch);
  cur_line_->line_chars_.emplace_back(wch, horizontal_scale_, vertical_scale_);
  CFGAS_Char* pCurChar = &cur_line_->line_chars_.back();
  pCurChar->font_size_ = font_size_;
  pCurChar->identity_ = identity_;
  pCurChar->user_data_ = user_data_;

  CFGAS_Char::BreakType dwRet1 = CFGAS_Char::BreakType::kNone;
  if (chartype != FX_CHARTYPE::kCombination &&
      GetUnifiedCharType(char_type_) != GetUnifiedCharType(chartype) &&
      char_type_ != FX_CHARTYPE::kUnknown &&
      IsGreaterThanLineWidth(cur_line_->GetLineEnd()) &&
      (char_type_ != FX_CHARTYPE::kSpace ||
       chartype != FX_CHARTYPE::kControl)) {
    dwRet1 = EndBreak(CFGAS_Char::BreakType::kLine);
    if (!cur_line_->line_chars_.empty()) {
      pCurChar = &cur_line_->line_chars_.back();
    }
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
      dwRet2 = AppendChar_Others(pCurChar);
      break;
  }

  char_type_ = chartype;
  return std::max(dwRet1, dwRet2);
}

void CFGAS_RTFBreak::AppendChar_Combination(CFGAS_Char* pCurChar) {
  std::optional<uint16_t> iCharWidthRet;
  if (font_) {
    iCharWidthRet = font_->GetCharWidth(pCurChar->char_code());
  }
  FX_SAFE_INT32 iCharWidth = iCharWidthRet.value_or(0);
  iCharWidth *= font_size_;
  iCharWidth *= horizontal_scale_;
  iCharWidth /= 100;
  CFGAS_Char* pLastChar = GetLastChar(0, false, true);
  if (pLastChar && pLastChar->GetCharType() > FX_CHARTYPE::kCombination) {
    iCharWidth *= -1;
  } else {
    char_type_ = FX_CHARTYPE::kCombination;
  }

  int32_t iCharWidthValid = iCharWidth.ValueOrDefault(0);
  pCurChar->char_width_ = iCharWidthValid;
  if (iCharWidthValid > 0) {
    FX_SAFE_INT32 checked_width = cur_line_->width_;
    checked_width += iCharWidthValid;
    if (!checked_width.IsValid()) {
      return;
    }

    cur_line_->width_ = checked_width.ValueOrDie();
  }
}

void CFGAS_RTFBreak::AppendChar_Tab(CFGAS_Char* pCurChar) {
  if (!(layout_styles_ & LayoutStyle::kExpandTab)) {
    return;
  }

  int32_t& iLineWidth = cur_line_->width_;
  int32_t iCharWidth = iLineWidth;
  FX_SAFE_INT32 iSafeCharWidth;
  if (GetPositionedTab(&iCharWidth)) {
    iSafeCharWidth = iCharWidth;
  } else {
    // Tab width is >= 160000, so this part does not need to be checked.
    DCHECK(tab_width_ >= kMinimumTabWidth);
    iSafeCharWidth = iLineWidth / tab_width_ + 1;
    iSafeCharWidth *= tab_width_;
  }
  iSafeCharWidth -= iLineWidth;

  iCharWidth = iSafeCharWidth.ValueOrDefault(0);

  pCurChar->char_width_ = iCharWidth;
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
      if (pCurChar->char_code() == w_paragraph_break_char_) {
        dwRet2 = CFGAS_Char::BreakType::kParagraph;
      }
      break;
  }
  if (dwRet2 != CFGAS_Char::BreakType::kNone) {
    dwRet2 = EndBreak(dwRet2);
  }

  return dwRet2;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::AppendChar_Arabic(CFGAS_Char* pCurChar) {
  cur_line_->IncrementArabicCharCount();

  CFGAS_Char* pLastChar = nullptr;
  wchar_t wForm;
  bool bAlef = false;
  if (char_type_ >= FX_CHARTYPE::kArabicAlef &&
      char_type_ <= FX_CHARTYPE::kArabicDistortion) {
    pLastChar = GetLastChar(1, false, true);
    if (pLastChar) {
      cur_line_->width_ -= pLastChar->char_width_;
      CFGAS_Char* pPrevChar = GetLastChar(2, false, true);
      wForm = pdfium::GetArabicFormChar(pLastChar, pPrevChar, pCurChar);
      bAlef = (wForm == pdfium::unicode::kZeroWidthNoBreakSpace &&
               pLastChar->GetCharType() == FX_CHARTYPE::kArabicAlef);
      FX_SAFE_INT32 iCharWidth = 0;
      if (font_) {
        std::optional<uint16_t> iCharWidthRet = font_->GetCharWidth(wForm);
        if (iCharWidthRet.has_value()) {
          iCharWidth = iCharWidthRet.value();
        } else {
          iCharWidthRet = font_->GetCharWidth(pLastChar->char_code());
          iCharWidth = iCharWidthRet.value_or(0);
        }
      }
      iCharWidth *= font_size_;
      iCharWidth *= horizontal_scale_;
      iCharWidth /= 100;

      int iCharWidthValid = iCharWidth.ValueOrDefault(0);
      pLastChar->char_width_ = iCharWidthValid;

      FX_SAFE_INT32 checked_width = cur_line_->width_;
      checked_width += iCharWidthValid;
      if (!checked_width.IsValid()) {
        return CFGAS_Char::BreakType::kNone;
      }

      cur_line_->width_ = checked_width.ValueOrDie();
      iCharWidth = 0;
    }
  }

  wForm =
      pdfium::GetArabicFormChar(pCurChar, bAlef ? nullptr : pLastChar, nullptr);
  FX_SAFE_INT32 iCharWidth = 0;
  if (font_) {
    std::optional<uint16_t> iCharWidthRet = font_->GetCharWidth(wForm);
    if (!iCharWidthRet.has_value()) {
      iCharWidthRet = font_->GetCharWidth(pCurChar->char_code());
    }
    iCharWidth = iCharWidthRet.value_or(0);
    iCharWidth *= font_size_;
    iCharWidth *= horizontal_scale_;
    iCharWidth /= 100;
  }

  int iCharWidthValid = iCharWidth.ValueOrDefault(0);
  pCurChar->char_width_ = iCharWidthValid;

  FX_SAFE_INT32 checked_width = cur_line_->width_;
  checked_width += iCharWidthValid;
  if (!checked_width.IsValid()) {
    return CFGAS_Char::BreakType::kNone;
  }

  cur_line_->width_ = checked_width.ValueOrDie();

  if (IsGreaterThanLineWidth(cur_line_->GetLineEnd())) {
    return EndBreak(CFGAS_Char::BreakType::kLine);
  }
  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::AppendChar_Others(CFGAS_Char* pCurChar) {
  FX_CHARTYPE chartype = pCurChar->GetCharType();
  wchar_t wForm = pCurChar->char_code();
  FX_SAFE_INT32 iCharWidth = 0;
  if (font_) {
    iCharWidth = font_->GetCharWidth(wForm).value_or(0);
  }
  iCharWidth *= font_size_;
  iCharWidth *= horizontal_scale_;
  iCharWidth /= 100;
  iCharWidth += char_space_;

  int iValidCharWidth = iCharWidth.ValueOrDefault(0);
  pCurChar->char_width_ = iValidCharWidth;

  FX_SAFE_INT32 checked_width = cur_line_->width_;
  checked_width += iValidCharWidth;
  if (!checked_width.IsValid()) {
    return CFGAS_Char::BreakType::kNone;
  }

  cur_line_->width_ = checked_width.ValueOrDie();
  if (chartype != FX_CHARTYPE::kSpace &&
      IsGreaterThanLineWidth(cur_line_->GetLineEnd())) {
    return EndBreak(CFGAS_Char::BreakType::kLine);
  }
  return CFGAS_Char::BreakType::kNone;
}

CFGAS_Char::BreakType CFGAS_RTFBreak::EndBreak(CFGAS_Char::BreakType dwStatus) {
  DCHECK(dwStatus != CFGAS_Char::BreakType::kNone);

  ++identity_;
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

  CFGAS_Char* tc = cur_line_->LastChar();
  if (!tc) {
    return CFGAS_Char::BreakType::kNone;
  }

  tc->status_ = dwStatus;
  if (dwStatus == CFGAS_Char::BreakType::kPiece) {
    return dwStatus;
  }

  ready_line_index_ = cur_line_ == &lines_[0] ? 0 : 1;
  CFGAS_BreakLine* pNextLine = &lines_[1 - ready_line_index_];
  bool bAllChars = alignment_ == LineAlignment::Justified ||
                   alignment_ == LineAlignment::Distributed;

  if (!EndBreakSplitLine(pNextLine, bAllChars, dwStatus)) {
    std::deque<TPO> tpos = EndBreakBidiLine(dwStatus);
    if (!pagination_ && alignment_ != LineAlignment::Left) {
      EndBreakAlignment(tpos, bAllChars, dwStatus);
    }
  }
  cur_line_ = pNextLine;
  cur_line_->start_ = line_start_;

  CFGAS_Char* pTC = GetLastChar(0, false, true);
  char_type_ = pTC ? pTC->GetCharType() : FX_CHARTYPE::kUnknown;
  return dwStatus;
}

bool CFGAS_RTFBreak::EndBreakSplitLine(CFGAS_BreakLine* pNextLine,
                                       bool bAllChars,
                                       CFGAS_Char::BreakType dwStatus) {
  bool bDone = false;
  if (IsGreaterThanLineWidth(cur_line_->GetLineEnd())) {
    const CFGAS_Char* tc = cur_line_->LastChar();
    switch (tc->GetCharType()) {
      case FX_CHARTYPE::kTab:
      case FX_CHARTYPE::kControl:
      case FX_CHARTYPE::kSpace:
        break;
      default:
        SplitTextLine(cur_line_, pNextLine, !pagination_ && bAllChars);
        bDone = true;
        break;
    }
  }

  if (!pagination_) {
    if (bAllChars && !bDone) {
      int32_t endPos = cur_line_->GetLineEnd();
      GetBreakPos(cur_line_->line_chars_, bAllChars, true, &endPos);
    }
    return false;
  }

  const CFGAS_Char* pCurChars = cur_line_->line_chars_.data();
  CFGAS_BreakPiece tp;
  tp.SetChars(&cur_line_->line_chars_);
  bool bNew = true;
  uint32_t dwIdentity = static_cast<uint32_t>(-1);
  int32_t iLast = fxcrt::CollectionSize<int32_t>(cur_line_->line_chars_) - 1;
  int32_t j = 0;
  for (int32_t i = 0; i <= iLast;) {
    const CFGAS_Char* pTC = UNSAFE_TODO(pCurChars + i);
    if (bNew) {
      tp.SetStartChar(i);
      tp.IncrementStartPos(tp.GetWidth());
      tp.SetWidth(0);
      tp.SetStatus(pTC->status_);
      tp.SetFontSize(pTC->font_size_);
      tp.SetHorizontalScale(pTC->horizonal_scale());
      tp.SetVerticalScale(pTC->vertical_scale());
      dwIdentity = pTC->identity_;
      tp.SetUserData(pTC->user_data_);
      j = i;
      bNew = false;
    }

    if (i == iLast || pTC->status_ != CFGAS_Char::BreakType::kNone ||
        pTC->identity_ != dwIdentity) {
      if (pTC->identity_ == dwIdentity) {
        tp.SetStatus(pTC->status_);
        tp.IncrementWidth(pTC->char_width_);
        ++i;
      }
      tp.SetCharCount(i - j);
      cur_line_->line_pieces_.push_back(tp);
      bNew = true;
    } else {
      tp.IncrementWidth(pTC->char_width_);
      ++i;
    }
  }
  return true;
}

std::deque<CFGAS_Break::TPO> CFGAS_RTFBreak::EndBreakBidiLine(
    CFGAS_Char::BreakType dwStatus) {
  CFGAS_Char* pTC;
  std::vector<CFGAS_Char>& chars = cur_line_->line_chars_;
  if (!pagination_ && cur_line_->HasArabicChar()) {
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
  } else {
    for (size_t i = 0; i < cur_line_->line_chars_.size(); ++i) {
      pTC = &chars[i];
      pTC->bidi_level_ = 0;
      pTC->bidi_pos_ = 0;
      pTC->bidi_order_ = 0;
    }
  }

  CFGAS_BreakPiece tp;
  tp.SetStatus(CFGAS_Char::BreakType::kPiece);
  tp.SetStartPos(cur_line_->start_);
  tp.SetChars(&chars);

  int32_t iBidiLevel = -1;
  int32_t iCharWidth;
  std::deque<TPO> tpos;
  uint32_t dwIdentity = static_cast<uint32_t>(-1);
  int32_t i = 0;
  int32_t j = 0;
  int32_t iCount = fxcrt::CollectionSize<int32_t>(cur_line_->line_chars_);
  while (i < iCount) {
    pTC = &chars[i];
    if (iBidiLevel < 0) {
      iBidiLevel = pTC->bidi_level_;
      iCharWidth = pTC->char_width_;
      tp.SetWidth(iCharWidth < 1 ? 0 : iCharWidth);
      tp.SetBidiLevel(iBidiLevel);
      tp.SetBidiPos(pTC->bidi_order_);
      tp.SetFontSize(pTC->font_size_);
      tp.SetHorizontalScale(pTC->horizonal_scale());
      tp.SetVerticalScale(pTC->vertical_scale());
      dwIdentity = pTC->identity_;
      tp.SetUserData(pTC->user_data_);
      tp.SetStatus(CFGAS_Char::BreakType::kPiece);
      ++i;
    } else if (iBidiLevel != pTC->bidi_level_ || pTC->identity_ != dwIdentity) {
      tp.SetCharCount(i - tp.GetStartChar());
      cur_line_->line_pieces_.push_back(tp);
      tp.IncrementStartPos(tp.GetWidth());
      tp.SetStartChar(i);
      tpos.push_back({j++, tp.GetBidiPos()});
      iBidiLevel = -1;
    } else {
      iCharWidth = pTC->char_width_;
      if (iCharWidth > 0) {
        tp.IncrementWidth(iCharWidth);
      }
      ++i;
    }
  }

  if (i > tp.GetStartChar()) {
    tp.SetStatus(dwStatus);
    tp.SetCharCount(i - tp.GetStartChar());
    cur_line_->line_pieces_.push_back(tp);
    tpos.push_back({j, tp.GetBidiPos()});
  }

  std::sort(tpos.begin(), tpos.end());
  int32_t iStartPos = cur_line_->start_;
  for (const auto& it : tpos) {
    CFGAS_BreakPiece& ttp = cur_line_->line_pieces_[it.index];
    ttp.SetStartPos(iStartPos);
    iStartPos += ttp.GetWidth();
  }
  return tpos;
}

void CFGAS_RTFBreak::EndBreakAlignment(const std::deque<TPO>& tpos,
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
      const CFGAS_Char* tc = ttp.GetChar(j);
      if (tc->line_break_type_ == FX_LINEBREAKTYPE::kDIRECT_BRK) {
        ++iGapChars;
      }

      if (!bFind || !bAllChars) {
        FX_CHARTYPE dwCharType = tc->GetCharType();
        if (dwCharType == FX_CHARTYPE::kSpace ||
            dwCharType == FX_CHARTYPE::kControl) {
          if (!bFind) {
            int32_t iCharWidth = tc->char_width_;
            if (bAllChars && iCharWidth > 0) {
              iNetWidth -= iCharWidth;
            }
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
  if (iGapChars > 0 && (alignment_ == LineAlignment::Distributed ||
                        (alignment_ == LineAlignment::Justified &&
                         dwStatus != CFGAS_Char::BreakType::kParagraph))) {
    int32_t iStart = -1;
    for (const auto& tpo : tpos) {
      CFGAS_BreakPiece& ttp = cur_line_->line_pieces_[tpo.index];
      if (iStart < 0) {
        iStart = ttp.GetStartPos();
      } else {
        ttp.SetStartPos(iStart);
      }

      for (int32_t j = 0; j < ttp.GetCharCount(); ++j) {
        CFGAS_Char* tc = ttp.GetChar(j);
        if (tc->line_break_type_ != FX_LINEBREAKTYPE::kDIRECT_BRK ||
            tc->char_width_ < 0) {
          continue;
        }
        int32_t k = iOffset / iGapChars;
        tc->char_width_ += k;
        ttp.IncrementWidth(k);
        iOffset -= k;
        --iGapChars;
        if (iGapChars < 1) {
          break;
        }
      }
      iStart += ttp.GetWidth();
    }
  } else if (alignment_ == LineAlignment::Right ||
             alignment_ == LineAlignment::Center) {
    if (alignment_ == LineAlignment::Center) {
      iOffset /= 2;
    }
    if (iOffset > 0) {
      for (auto& ttp : cur_line_->line_pieces_) {
        ttp.IncrementStartPos(iOffset);
      }
    }
  }
}

int32_t CFGAS_RTFBreak::GetBreakPos(std::vector<CFGAS_Char>& tca,
                                    bool bAllChars,
                                    bool bOnlyBrk,
                                    int32_t* pEndPos) {
  int32_t iLength = fxcrt::CollectionSize<int32_t>(tca) - 1;
  if (iLength < 1) {
    return iLength;
  }

  int32_t iBreak = -1;
  int32_t iBreakPos = -1;
  int32_t iIndirect = -1;
  int32_t iIndirectPos = -1;
  int32_t iLast = -1;
  int32_t iLastPos = -1;
  if (*pEndPos <= line_width_) {
    if (!bAllChars) {
      return iLength;
    }

    iBreak = iLength;
    iBreakPos = *pEndPos;
  }

  CFGAS_Char* pCharArray = tca.data();
  CFGAS_Char* pCur = UNSAFE_TODO(pCharArray + iLength);
  --iLength;
  if (bAllChars) {
    pCur->line_break_type_ = FX_LINEBREAKTYPE::kUNKNOWN;
  }

  FX_BREAKPROPERTY nNext = pdfium::unicode::GetBreakProperty(pCur->char_code());
  int32_t iCharWidth = pCur->char_width_;
  if (iCharWidth > 0) {
    *pEndPos -= iCharWidth;
  }

  while (iLength >= 0) {
    pCur = UNSAFE_TODO(pCharArray + iLength);
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
      if (nCur == FX_BREAKPROPERTY::kSP) {
        bNeedBreak = true;
      }

      eType = nNext == FX_BREAKPROPERTY::kSP
                  ? FX_LINEBREAKTYPE::kPROHIBITED_BRK
                  : GetLineBreakTypeFromPair(nCur, nNext);
    }
    if (bAllChars) {
      pCur->line_break_type_ = eType;
    }

    if (!bOnlyBrk) {
      iCharWidth = pCur->char_width_;
      if (*pEndPos <= line_width_ || bNeedBreak) {
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
      if (iCharWidth > 0) {
        *pEndPos -= iCharWidth;
      }
    }
    nNext = nCur;
    --iLength;
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

void CFGAS_RTFBreak::SplitTextLine(CFGAS_BreakLine* pCurLine,
                                   CFGAS_BreakLine* pNextLine,
                                   bool bAllChars) {
  DCHECK(pCurLine);
  DCHECK(pNextLine);

  if (pCurLine->line_chars_.size() < 2) {
    return;
  }

  int32_t iEndPos = pCurLine->GetLineEnd();
  std::vector<CFGAS_Char>& curChars = pCurLine->line_chars_;
  int32_t iCharPos = GetBreakPos(curChars, bAllChars, false, &iEndPos);
  if (iCharPos < 0) {
    iCharPos = 0;
  }

  ++iCharPos;
  if (iCharPos >= fxcrt::CollectionSize<int32_t>(pCurLine->line_chars_)) {
    pNextLine->Clear();
    curChars[iCharPos - 1].line_break_type_ = FX_LINEBREAKTYPE::kUNKNOWN;
    return;
  }

  pNextLine->line_chars_ =
      std::vector<CFGAS_Char>(curChars.begin() + iCharPos, curChars.end());
  curChars.erase(curChars.begin() + iCharPos, curChars.end());
  pNextLine->start_ = pCurLine->start_;
  pNextLine->width_ = pCurLine->GetLineEnd() - iEndPos;
  pCurLine->width_ = iEndPos;
  curChars[iCharPos - 1].line_break_type_ = FX_LINEBREAKTYPE::kUNKNOWN;

  for (size_t i = 0; i < pNextLine->line_chars_.size(); ++i) {
    if (pNextLine->line_chars_[i].GetCharType() >= FX_CHARTYPE::kArabicAlef) {
      pCurLine->DecrementArabicCharCount();
      pNextLine->IncrementArabicCharCount();
    }
    pNextLine->line_chars_[i].status_ = CFGAS_Char::BreakType::kNone;
  }
}

size_t CFGAS_RTFBreak::GetDisplayPos(const CFGAS_TextPiece* pPiece,
                                     pdfium::span<TextCharPos> pCharPos) const {
  if (pPiece->iChars == 0 || !pPiece->font) {
    return 0;
  }

  RetainPtr<CFGAS_GEFont> font = pPiece->font;
  CFX_RectF rtText(pPiece->rtPiece);
  const bool bRTLPiece = FX_IsOdd(pPiece->iBidiLevel);
  const float fFontSize = pPiece->fFontSize;
  const int32_t iFontSize = FXSYS_roundf(fFontSize * 20.0f);
  if (iFontSize == 0) {
    return 0;
  }

  const int32_t iAscent = font->GetAscent();
  const int32_t iDescent = font->GetDescent();
  const int32_t iMaxHeight = iAscent - iDescent;
  const float fAscent = iMaxHeight ? fFontSize * iAscent / iMaxHeight : 0;
  wchar_t wPrev = pdfium::unicode::kZeroWidthNoBreakSpace;
  wchar_t wNext;
  float fX = rtText.left;
  int32_t iHorScale = pPiece->iHorScale;
  int32_t iVerScale = pPiece->iVerScale;
  if (bRTLPiece) {
    fX = rtText.right();
  }

  float fY = rtText.top + fAscent;
  size_t szCount = 0;
  for (int32_t i = 0; i < pPiece->iChars; ++i) {
    TextCharPos& current_char_pos = pCharPos[szCount];
    wchar_t wch = pPiece->szText[i];
    int32_t iWidth = pPiece->Widths[i];
    FX_CHARTYPE dwCharType = pdfium::unicode::GetCharType(wch);
    if (iWidth == 0) {
      if (dwCharType == FX_CHARTYPE::kArabicAlef) {
        wPrev = pdfium::unicode::kZeroWidthNoBreakSpace;
      }
      continue;
    }

    int iCharWidth = abs(iWidth);
    const bool bEmptyChar = (dwCharType >= FX_CHARTYPE::kTab &&
                             dwCharType <= FX_CHARTYPE::kControl);
    if (!bEmptyChar) {
      ++szCount;
    }

    iCharWidth /= iFontSize;
    wchar_t wForm = wch;
    if (dwCharType >= FX_CHARTYPE::kArabicAlef) {
      if (i + 1 < pPiece->iChars) {
        wNext = pPiece->szText[i + 1];
        if (pPiece->Widths[i + 1] < 0 && i + 2 < pPiece->iChars) {
          wNext = pPiece->szText[i + 2];
        }
      } else {
        wNext = pdfium::unicode::kZeroWidthNoBreakSpace;
      }
      wForm = pdfium::GetArabicFormChar(wch, wPrev, wNext);
    } else if (bRTLPiece) {
      wForm = pdfium::unicode::GetMirrorChar(wch);
    }

    if (!bEmptyChar) {
      current_char_pos.glyph_index_ = font->GetGlyphIndex(wForm);
      if (current_char_pos.glyph_index_ == 0xFFFF) {
        current_char_pos.glyph_index_ = font->GetGlyphIndex(wch);
      }
#if BUILDFLAG(IS_APPLE)
      current_char_pos.ext_gid_ = current_char_pos.glyph_index_;
#endif
      current_char_pos.font_char_width_ = iCharWidth;
    }

    float fCharWidth = fFontSize * iCharWidth / 1000.0f;
    if (bRTLPiece && dwCharType != FX_CHARTYPE::kCombination) {
      fX -= fCharWidth;
    }

    if (!bEmptyChar) {
      current_char_pos.origin_ = CFX_PointF(fX, fY);
    }
    if (!bRTLPiece && dwCharType != FX_CHARTYPE::kCombination) {
      fX += fCharWidth;
    }

    if (!bEmptyChar) {
      current_char_pos.glyph_adjust_ = true;
      current_char_pos.adjust_matrix_[0] = -1;
      current_char_pos.adjust_matrix_[1] = 0;
      current_char_pos.adjust_matrix_[2] = 0;
      current_char_pos.adjust_matrix_[3] = 1;
      current_char_pos.origin_.y += fAscent * iVerScale / 100.0f;
      current_char_pos.origin_.y -= fAscent;

      if (iHorScale != 100 || iVerScale != 100) {
        current_char_pos.adjust_matrix_[0] =
            current_char_pos.adjust_matrix_[0] * iHorScale / 100.0f;
        current_char_pos.adjust_matrix_[1] =
            current_char_pos.adjust_matrix_[1] * iHorScale / 100.0f;
        current_char_pos.adjust_matrix_[2] =
            current_char_pos.adjust_matrix_[2] * iVerScale / 100.0f;
        current_char_pos.adjust_matrix_[3] =
            current_char_pos.adjust_matrix_[3] * iVerScale / 100.0f;
      }
    }
    if (iWidth > 0) {
      wPrev = wch;
    }
  }
  return szCount;
}
