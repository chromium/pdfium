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

class CFGAS_GEFont;
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

  CFX_BreakType EndBreak(CFX_BreakType dwStatus);

  size_t GetDisplayPos(const CFX_TextPiece* pPiece,
                       std::vector<TextCharPos>* pCharPos) const;

  CFX_BreakType AppendChar(wchar_t wch);

 private:
  void AppendChar_Combination(CFX_Char* pCurChar);
  void AppendChar_Tab(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Control(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Arabic(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Others(CFX_Char* pCurChar);
  bool GetPositionedTab(int32_t* iTabPos) const;

  int32_t GetBreakPos(std::vector<CFX_Char>& tca,
                      bool bAllChars,
                      bool bOnlyBrk,
                      int32_t* pEndPos);
  void SplitTextLine(CFX_BreakLine* pCurLine,
                     CFX_BreakLine* pNextLine,
                     bool bAllChars);
  bool EndBreak_SplitLine(CFX_BreakLine* pNextLine,
                          bool bAllChars,
                          CFX_BreakType dwStatus);
  void EndBreak_BidiLine(std::deque<FX_TPO>* tpos, CFX_BreakType dwStatus);
  void EndBreak_Alignment(const std::deque<FX_TPO>& tpos,
                          bool bAllChars,
                          CFX_BreakType dwStatus);

  bool m_bPagination;
  std::vector<int32_t> m_PositionedTabs;
  CFX_RTFLineAlignment m_iAlignment;
  RetainPtr<CFX_TextUserData> m_pUserData;
};

#endif  // XFA_FGAS_LAYOUT_CFX_RTFBREAK_H_
