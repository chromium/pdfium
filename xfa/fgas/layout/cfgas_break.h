// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_BREAK_H_
#define XFA_FGAS_LAYOUT_CFGAS_BREAK_H_

#include <stdint.h>

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
  Mask<LayoutStyle> GetLayoutStyles() const { return m_dwLayoutStyles; }

  void SetFont(RetainPtr<CFGAS_GEFont> pFont);
  void SetFontSize(float fFontSize);
  void SetTabWidth(float fTabWidth);
  int32_t GetTabWidth() const { return m_iTabWidth; }

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
  const CFGAS_BreakLine* GetCurrentLineForTesting() const { return m_pCurLine; }

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
  bool HasLine() const { return m_iReadyLineIndex >= 0; }
  bool IsGreaterThanLineWidth(int32_t width) const;
  FX_CHARTYPE GetUnifiedCharType(FX_CHARTYPE dwType) const;

  FX_CHARTYPE m_eCharType = FX_CHARTYPE::kUnknown;
  bool m_bSingleLine = false;
  bool m_bCombText = false;
  Mask<LayoutStyle> m_dwLayoutStyles = LayoutStyle::kNone;
  uint32_t m_dwIdentity = 0;
  int32_t m_iLineStart = 0;
  int32_t m_iLineWidth = 2000000;
  wchar_t m_wParagraphBreakChar = L'\n';
  int32_t m_iFontSize = 240;
  int32_t m_iTabWidth = 720000;
  int32_t m_iHorizontalScale = 100;
  int32_t m_iVerticalScale = 100;
  int32_t m_iTolerance = 0;
  int32_t m_iCharSpace = 0;
  RetainPtr<CFGAS_GEFont> m_pFont;
  UnownedPtr<CFGAS_BreakLine> m_pCurLine;
  int8_t m_iReadyLineIndex = -1;
  CFGAS_BreakLine m_Lines[2];
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_BREAK_H_
