// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_TXTBREAK_H_
#define XFA_FGAS_LAYOUT_CFGAS_TXTBREAK_H_

#include <deque>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/unowned_ptr_exclusion.h"
#include "xfa/fgas/layout/cfgas_break.h"
#include "xfa/fgas/layout/cfgas_char.h"

class CFGAS_GEFont;
class TextCharPos;

#define FX_TXTCHARSTYLE_ArabicShadda 0x0020
#define FX_TXTCHARSTYLE_OddBidiLevel 0x0040

enum CFX_TxtLineAlignment {
  CFX_TxtLineAlignment_Left = 0,
  CFX_TxtLineAlignment_Center = 1 << 0,
  CFX_TxtLineAlignment_Right = 1 << 1,
  CFX_TxtLineAlignment_Justified = 1 << 2
};

inline bool CFX_BreakTypeNoneOrPiece(CFGAS_Char::BreakType type) {
  return type == CFGAS_Char::BreakType::kNone ||
         type == CFGAS_Char::BreakType::kPiece;
}

class CFGAS_TxtBreak final : public CFGAS_Break {
 public:
  class Engine {
   public:
    virtual ~Engine();
    virtual wchar_t GetChar(size_t idx) const = 0;
    // May return negative for combining characters. Non-const so we can force
    // a layout if needed.
    virtual int32_t GetWidthOfChar(size_t idx) = 0;
  };

  struct Run {
    Run();
    Run(const Run& other);
    ~Run();

    UnownedPtr<CFGAS_TxtBreak::Engine> pEdtEngine;
    WideString wsStr;
    UNOWNED_PTR_EXCLUSION int32_t* pWidths = nullptr;
    // TODO(thestig): These 2 members probably should be size_t.
    int32_t iStart = 0;
    int32_t iLength = 0;
    RetainPtr<CFGAS_GEFont> pFont;
    float fFontSize = 12.0f;
    Mask<LayoutStyle> dwStyles = LayoutStyle::kNone;
    int32_t iHorizontalScale = 100;
    int32_t iVerticalScale = 100;
    uint32_t dwCharStyles = 0;
    UnownedPtr<const CFX_RectF> pRect;
    bool bSkipSpace = true;
  };

  CFGAS_TxtBreak();
  ~CFGAS_TxtBreak() override;

  void SetLineWidth(float fLineWidth);
  void SetAlignment(int32_t iAlignment);
  void SetCombWidth(float fCombWidth);
  CFGAS_Char::BreakType EndBreak(CFGAS_Char::BreakType dwStatus);

  size_t GetDisplayPos(const Run& run, TextCharPos* pCharPos) const;
  std::vector<CFX_RectF> GetCharRects(const Run& run) const;
  CFGAS_Char::BreakType AppendChar(wchar_t wch);

 private:
  void AppendChar_Combination(CFGAS_Char* pCurChar);
  void AppendChar_Tab(CFGAS_Char* pCurChar);
  CFGAS_Char::BreakType AppendChar_Control(CFGAS_Char* pCurChar);
  CFGAS_Char::BreakType AppendChar_Arabic(CFGAS_Char* pCurChar);
  CFGAS_Char::BreakType AppendChar_Others(CFGAS_Char* pCurChar);

  void ResetContextCharStyles();
  void EndBreakSplitLine(CFGAS_BreakLine* pNextLine, bool bAllChars);
  std::deque<TPO> EndBreakBidiLine(CFGAS_Char::BreakType dwStatus);
  void EndBreakAlignment(const std::deque<TPO>& tpos,
                         bool bAllChars,
                         CFGAS_Char::BreakType dwStatus);
  int32_t GetBreakPos(std::vector<CFGAS_Char>* pChars,
                      bool bAllChars,
                      bool bOnlyBrk,
                      int32_t* pEndPos);
  void SplitTextLine(CFGAS_BreakLine* pCurLine,
                     CFGAS_BreakLine* pNextLine,
                     bool bAllChars);

  int32_t m_iAlignment = CFX_TxtLineAlignment_Left;
  int32_t m_iCombWidth = 360000;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_TXTBREAK_H_
