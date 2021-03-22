// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFX_RTFBREAK_H_
#define XFA_FGAS_LAYOUT_CFX_RTFBREAK_H_

#include <deque>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_unicode.h"
#include "core/fxcrt/retain_ptr.h"
#include "xfa/fgas/layout/cfx_break.h"

class CFX_TextUserData;
class CFX_TextPiece;
class TextCharPos;

enum class CFX_RTFLineAlignment {
  Left = 0,
  Center,
  Right,
  Justified,
  Distributed
};

class CFX_RTFBreak final : public CFX_Break {
 public:
  explicit CFX_RTFBreak(uint32_t dwLayoutStyles);
  ~CFX_RTFBreak() override;

  void SetLineStartPos(float fLinePos);

  void SetAlignment(CFX_RTFLineAlignment align) { m_iAlignment = align; }
  void SetUserData(const RetainPtr<CFX_TextUserData>& pUserData);

  void AddPositionedTab(float fTabPos);

  CFGAS_Char::BreakType EndBreak(CFGAS_Char::BreakType dwStatus);

  size_t GetDisplayPos(const CFX_TextPiece* pPiece,
                       std::vector<TextCharPos>* pCharPos) const;

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
  void SplitTextLine(CFX_BreakLine* pCurLine,
                     CFX_BreakLine* pNextLine,
                     bool bAllChars);
  bool EndBreak_SplitLine(CFX_BreakLine* pNextLine,
                          bool bAllChars,
                          CFGAS_Char::BreakType dwStatus);
  void EndBreak_BidiLine(std::deque<FX_TPO>* tpos,
                         CFGAS_Char::BreakType dwStatus);
  void EndBreak_Alignment(const std::deque<FX_TPO>& tpos,
                          bool bAllChars,
                          CFGAS_Char::BreakType dwStatus);

  bool m_bPagination;
  std::vector<int32_t> m_PositionedTabs;
  CFX_RTFLineAlignment m_iAlignment;
  RetainPtr<CFX_TextUserData> m_pUserData;
};

#endif  // XFA_FGAS_LAYOUT_CFX_RTFBREAK_H_
