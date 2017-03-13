// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_FGAS_TEXTBREAK_H_
#define XFA_FGAS_LAYOUT_FGAS_TEXTBREAK_H_

#include <deque>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_char.h"
#include "core/fxge/cfx_renderdevice.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_txtedtpage.h"
#include "xfa/fgas/layout/cfx_breakline.h"

class CFGAS_GEFont;
struct FDE_TEXTEDITPIECE;

#define FX_TXTLAYOUTSTYLE_SingleLine 0x0200
#define FX_TXTLAYOUTSTYLE_CombText 0x0400

#define FX_TXTCHARSTYLE_ArabicShadda 0x0020
#define FX_TXTCHARSTYLE_OddBidiLevel 0x0040

enum CFX_TxtLineAlignment {
  CFX_TxtLineAlignment_Left = 0,
  CFX_TxtLineAlignment_Center = 1 << 0,
  CFX_TxtLineAlignment_Right = 1 << 1,
  CFX_TxtLineAlignment_Justified = 1 << 2
};

struct FX_TPO {
  int32_t index;
  int32_t pos;

  bool operator<(const FX_TPO& that) const { return pos < that.pos; }
};

inline bool CFX_BreakTypeNoneOrPiece(CFX_BreakType type) {
  return type == CFX_BreakType::None || type == CFX_BreakType::Piece;
}

struct FX_TXTRUN {
  FX_TXTRUN();
  FX_TXTRUN(const FX_TXTRUN& other);
  ~FX_TXTRUN();

  CFDE_TxtEdtPage* pAccess;
  const FDE_TEXTEDITPIECE* pIdentity;
  CFX_WideString wsStr;
  int32_t* pWidths;
  int32_t iLength;
  CFX_RetainPtr<CFGAS_GEFont> pFont;
  FX_FLOAT fFontSize;
  uint32_t dwStyles;
  int32_t iHorizontalScale;
  int32_t iVerticalScale;
  uint32_t dwCharStyles;
  const CFX_RectF* pRect;
  wchar_t wLineBreakChar;
  bool bSkipSpace;
};

class CFX_TxtBreak {
 public:
  CFX_TxtBreak();
  ~CFX_TxtBreak();

  void SetLineWidth(FX_FLOAT fLineWidth);
  uint32_t GetLayoutStyles() const { return m_dwLayoutStyles; }
  void SetLayoutStyles(uint32_t dwLayoutStyles);
  void SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont);
  void SetFontSize(FX_FLOAT fFontSize);
  void SetTabWidth(FX_FLOAT fTabWidth, bool bEquidistant);
  void SetDefaultChar(wchar_t wch);
  void SetParagraphBreakChar(wchar_t wch);
  void SetLineBreakTolerance(FX_FLOAT fTolerance);
  void SetHorizontalScale(int32_t iScale);
  void SetCharSpace(FX_FLOAT fCharSpace);
  void SetAlignment(int32_t iAlignment);
  void SetCombWidth(FX_FLOAT fCombWidth);
  CFX_BreakType EndBreak(CFX_BreakType dwStatus);
  int32_t CountBreakPieces() const;
  const CFX_BreakPiece* GetBreakPiece(int32_t index) const;
  void ClearBreakPieces();
  void Reset();
  int32_t GetDisplayPos(const FX_TXTRUN* pTxtRun,
                        FXTEXT_CHARPOS* pCharPos,
                        bool bCharCode = false,
                        CFX_WideString* pWSForms = nullptr) const;
  std::vector<CFX_RectF> GetCharRects(const FX_TXTRUN* pTxtRun,
                                      bool bCharBBox = false) const;
  void AppendChar_PageLoad(CFX_Char* pCurChar, uint32_t dwProps);
  CFX_BreakType AppendChar(wchar_t wch);
  CFX_BreakType AppendChar_Combination(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Tab(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Control(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Arabic(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Others(CFX_Char* pCurChar);

 private:
  void FontChanged();
  void SetBreakStatus();
  CFX_Char* GetLastChar(int32_t index, bool bOmitChar = true) const;
  bool HasTxtLine() const { return m_iReadyLineIndex >= 0; }
  FX_CHARTYPE GetUnifiedCharType(FX_CHARTYPE dwType) const;
  void ResetArabicContext();
  void ResetContextCharStyles();
  bool EndBreak_SplitLine(CFX_BreakLine* pNextLine, bool bAllChars);
  void EndBreak_BidiLine(std::deque<FX_TPO>* tpos, CFX_BreakType dwStatus);
  void EndBreak_Alignment(const std::deque<FX_TPO>& tpos,
                          bool bAllChars,
                          CFX_BreakType dwStatus);
  int32_t GetBreakPos(std::vector<CFX_Char>& ca,
                      int32_t& iEndPos,
                      bool bAllChars = false,
                      bool bOnlyBrk = false);
  void SplitTextLine(CFX_BreakLine* pCurLine,
                     CFX_BreakLine* pNextLine,
                     bool bAllChars = false);

  int32_t m_iLineWidth;
  uint32_t m_dwLayoutStyles;
  bool m_bSingleLine;
  bool m_bCombText;
  bool m_bEquidistant;
  int32_t m_iArabicContext;
  int32_t m_iCurArabicContext;
  CFX_RetainPtr<CFGAS_GEFont> m_pFont;
  int32_t m_iFontSize;
  int32_t m_iTabWidth;
  wchar_t m_wDefChar;
  wchar_t m_wParagBreakChar;
  int32_t m_iDefChar;
  int32_t m_iAlignment;
  uint32_t m_dwContextCharStyles;
  int32_t m_iCombWidth;
  FX_CHARTYPE m_eCharType;
  int32_t m_iCurAlignment;
  CFX_BreakLine m_TxtLine[2];
  CFX_BreakLine* m_pCurLine;
  int32_t m_iTolerance;
  int32_t m_iHorScale;
  int32_t m_iCharSpace;
  int8_t m_iReadyLineIndex;
};

#endif  // XFA_FGAS_LAYOUT_FGAS_TEXTBREAK_H_
