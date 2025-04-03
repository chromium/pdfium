// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_BREAK_H_
#define XFA_FGAS_LAYOUT_CFGAS_BREAK_H_

#include <stdint.h>

#include <array>

#include "core/fxcrt/mask.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fgas/layout/cfgas_breakline.h"

class CFGAS_GEFont;

class CFGAS_Break {
 public:
  enum class LayoutStyle : uint8_t {
    kNone = 0,
    kPagination = 1 << 0,
    kExpandTab = 1 << 1,
    kSingleLine = 1 << 2,
    kCombText = 1 << 3,
  };

  virtual ~CFGAS_Break();

  void Reset();

  void SetLayoutStyles(Mask<LayoutStyle> dwLayoutStyles);
  Mask<LayoutStyle> GetLayoutStyles() const { return layout_styles_; }

  void SetFont(RetainPtr<CFGAS_GEFont> pFont);
  void SetFontSize(float fFontSize);
  void SetTabWidth(float fTabWidth);
  int32_t GetTabWidth() const { return tab_width_; }

  void SetHorizontalScale(int32_t iScale);
  void SetVerticalScale(int32_t iScale);
  void SetLineBreakTolerance(float fTolerance);
  void SetLineBoundary(float fLineStart, float fLineEnd);

  void SetCharSpace(float fCharSpace);
  void SetParagraphBreakChar(wchar_t wch);

  int32_t CountBreakPieces() const;
  const CFGAS_BreakPiece* GetBreakPieceUnstable(int32_t index) const;
  void ClearBreakPieces();

  CFGAS_Char* GetLastChar(int32_t index, bool bOmitChar, bool bRichText) const;
  const CFGAS_BreakLine* GetCurrentLineForTesting() const { return cur_line_; }

 protected:
  struct TPO {
    bool operator<(const TPO& that) const { return pos < that.pos; }

    int32_t index;
    int32_t pos;
  };

  static const int kMinimumTabWidth;
  static const float kConversionFactor;

  explicit CFGAS_Break(Mask<LayoutStyle> dwLayoutStyles);

  void SetBreakStatus();
  bool HasLine() const { return ready_line_index_ >= 0; }
  bool IsGreaterThanLineWidth(int32_t width) const;
  FX_CHARTYPE GetUnifiedCharType(FX_CHARTYPE dwType) const;

  FX_CHARTYPE char_type_ = FX_CHARTYPE::kUnknown;
  bool single_line_ = false;
  bool comb_text_ = false;
  Mask<LayoutStyle> layout_styles_ = LayoutStyle::kNone;
  uint32_t identity_ = 0;
  int32_t line_start_ = 0;
  int32_t line_width_ = 2000000;
  wchar_t w_paragraph_break_char_ = L'\n';
  int32_t font_size_ = 240;
  int32_t tab_width_ = 720000;
  int32_t horizontal_scale_ = 100;
  int32_t vertical_scale_ = 100;
  int32_t tolerance_ = 0;
  int32_t char_space_ = 0;
  RetainPtr<CFGAS_GEFont> font_;
  int8_t ready_line_index_ = -1;
  std::array<CFGAS_BreakLine, 2> lines_;
  UnownedPtr<CFGAS_BreakLine> cur_line_;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_BREAK_H_
