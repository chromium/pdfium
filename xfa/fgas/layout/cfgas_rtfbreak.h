// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_RTFBREAK_H_
#define XFA_FGAS_LAYOUT_CFGAS_RTFBREAK_H_

#include <deque>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_unicode.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "xfa/fgas/layout/cfgas_break.h"

class CFGAS_TextPiece;
class CFGAS_TextUserData;
class TextCharPos;

class CFGAS_RTFBreak final : public CFGAS_Break {
 public:
  enum class LineAlignment : uint8_t {
    Left = 0,
    Center,
    Right,
    Justified,
    Distributed
  };

  explicit CFGAS_RTFBreak(Mask<LayoutStyle> dwLayoutStyles);
  ~CFGAS_RTFBreak() override;

  void SetLineStartPos(float fLinePos);

  void SetAlignment(LineAlignment align) { m_iAlignment = align; }
  void SetUserData(const RetainPtr<CFGAS_TextUserData>& pUserData);

  void AddPositionedTab(float fTabPos);

  CFGAS_Char::BreakType EndBreak(CFGAS_Char::BreakType dwStatus);

  size_t GetDisplayPos(const CFGAS_TextPiece* pPiece,
                       pdfium::span<TextCharPos> pCharPos) const;

  CFGAS_Char::BreakType AppendChar(wchar_t wch);

 private:
  void AppendChar_Combination(CFGAS_Char* pCurChar);
  void AppendChar_Tab(CFGAS_Char* pCurChar);
  CFGAS_Char::BreakType AppendChar_Control(CFGAS_Char* pCurChar);
  CFGAS_Char::BreakType AppendChar_Arabic(CFGAS_Char* pCurChar);
  CFGAS_Char::BreakType AppendChar_Others(CFGAS_Char* pCurChar);
  bool GetPositionedTab(int32_t* iTabPos) const;

  int32_t GetBreakPos(std::vector<CFGAS_Char>& tca,
                      bool bAllChars,
                      bool bOnlyBrk,
                      int32_t* pEndPos);
  void SplitTextLine(CFGAS_BreakLine* pCurLine,
                     CFGAS_BreakLine* pNextLine,
                     bool bAllChars);
  bool EndBreakSplitLine(CFGAS_BreakLine* pNextLine,
                         bool bAllChars,
                         CFGAS_Char::BreakType dwStatus);
  std::deque<TPO> EndBreakBidiLine(CFGAS_Char::BreakType dwStatus);
  void EndBreakAlignment(const std::deque<TPO>& tpos,
                         bool bAllChars,
                         CFGAS_Char::BreakType dwStatus);

  bool m_bPagination = false;
  LineAlignment m_iAlignment = LineAlignment::Left;
  std::vector<int32_t> m_PositionedTabs;
  RetainPtr<CFGAS_TextUserData> m_pUserData;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_RTFBREAK_H_
