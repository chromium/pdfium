// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_RTFBREAK_IMP
#define _FX_RTFBREAK_IMP

#include "core/include/fxcrt/fx_arb.h"

class CFX_RTFLine;
class CFX_RTFBreak;
class CFX_RTFLine {
 public:
  CFX_RTFLine()
      : m_LinePieces(16),
        m_iStart(0),
        m_iWidth(0),
        m_iArabicChars(0),
        m_iMBCSChars(0) {}
  ~CFX_RTFLine() { RemoveAll(); }
  int32_t CountChars() const { return m_LineChars.GetSize(); }
  CFX_RTFChar& GetChar(int32_t index) {
    FXSYS_assert(index > -1 && index < m_LineChars.GetSize());
    return *m_LineChars.GetDataPtr(index);
  }
  CFX_RTFChar* GetCharPtr(int32_t index) {
    FXSYS_assert(index > -1 && index < m_LineChars.GetSize());
    return m_LineChars.GetDataPtr(index);
  }
  int32_t CountPieces() const { return m_LinePieces.GetSize(); }
  CFX_RTFPiece& GetPiece(int32_t index) const {
    FXSYS_assert(index > -1 && index < m_LinePieces.GetSize());
    return m_LinePieces.GetAt(index);
  }
  CFX_RTFPiece* GetPiecePtr(int32_t index) const {
    FXSYS_assert(index > -1 && index < m_LinePieces.GetSize());
    return m_LinePieces.GetPtrAt(index);
  }
  int32_t GetLineEnd() const { return m_iStart + m_iWidth; }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    CFX_RTFChar* pChar;
    IFX_Unknown* pUnknown;
    int32_t iCount = m_LineChars.GetSize();
    for (int32_t i = 0; i < iCount; i++) {
      pChar = m_LineChars.GetDataPtr(i);
      if ((pUnknown = pChar->m_pUserData) != NULL) {
        pUnknown->Release();
      }
    }
    m_LineChars.RemoveAll();
    m_LinePieces.RemoveAll(bLeaveMemory);
    m_iWidth = 0;
    m_iArabicChars = 0;
    m_iMBCSChars = 0;
  }
  CFX_RTFCharArray m_LineChars;
  CFX_RTFPieceArray m_LinePieces;
  int32_t m_iStart;
  int32_t m_iWidth;
  int32_t m_iArabicChars;
  int32_t m_iMBCSChars;
};
class CFX_RTFBreak : public IFX_RTFBreak {
 public:
  CFX_RTFBreak(FX_DWORD dwPolicies);
  ~CFX_RTFBreak();
  void Release() override { delete this; }
  void SetLineBoundary(FX_FLOAT fLineStart, FX_FLOAT fLineEnd) override final;
  void SetLineStartPos(FX_FLOAT fLinePos) override final;
  FX_DWORD GetLayoutStyles() const override { return m_dwLayoutStyles; }
  void SetLayoutStyles(FX_DWORD dwLayoutStyles) override;
  void SetFont(IFX_Font* pFont) override;
  void SetFontSize(FX_FLOAT fFontSize) override;
  void SetTabWidth(FX_FLOAT fTabWidth) override;
  void AddPositionedTab(FX_FLOAT fTabPos) override;
  void SetPositionedTabs(const CFX_FloatArray& tabs) override;
  void ClearPositionedTabs() override;
  void SetDefaultChar(FX_WCHAR wch) override;
  void SetLineBreakChar(FX_WCHAR wch) override;
  void SetLineBreakTolerance(FX_FLOAT fTolerance) override;
  void SetHorizontalScale(int32_t iScale) override;
  void SetVerticalScale(int32_t iScale) override;
  void SetCharRotation(int32_t iCharRotation) override;
  void SetCharSpace(FX_FLOAT fCharSpace) override;
  void SetWordSpace(FX_BOOL bDefault, FX_FLOAT fWordSpace) override;
  void SetReadingOrder(FX_BOOL bRTL = FALSE) override;
  void SetAlignment(int32_t iAlignment = FX_RTFLINEALIGNMENT_Left) override;
  void SetUserData(IFX_Unknown* pUserData) override;
  FX_DWORD AppendChar(FX_WCHAR wch) override;
  FX_DWORD EndBreak(FX_DWORD dwStatus = FX_RTFBREAK_PieceBreak) override;
  int32_t CountBreakPieces() const override;
  const CFX_RTFPiece* GetBreakPiece(int32_t index) const override;
  void GetLineRect(CFX_RectF& rect) const override;
  void ClearBreakPieces() override;
  void Reset() override;
  int32_t GetDisplayPos(
      FX_LPCRTFTEXTOBJ pText,
      FXTEXT_CHARPOS* pCharPos,
      FX_BOOL bCharCode = FALSE,
      CFX_WideString* pWSForms = NULL,
      FX_AdjustCharDisplayPos pAdjustPos = NULL) const override;
  int32_t GetCharRects(FX_LPCRTFTEXTOBJ pText,
                       CFX_RectFArray& rtArray,
                       FX_BOOL bCharBBox = FALSE) const override;
  FX_DWORD AppendChar_CharCode(FX_WCHAR wch);
  FX_DWORD AppendChar_Combination(CFX_RTFChar* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Tab(CFX_RTFChar* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Control(CFX_RTFChar* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Arabic(CFX_RTFChar* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Others(CFX_RTFChar* pCurChar, int32_t iRotation);

 protected:
  FX_DWORD m_dwPolicies;
  IFX_ArabicChar* m_pArabicChar;
  int32_t m_iBoundaryStart;
  int32_t m_iBoundaryEnd;
  FX_DWORD m_dwLayoutStyles;
  FX_BOOL m_bPagination;
  FX_BOOL m_bVertical;
  FX_BOOL m_bSingleLine;
  FX_BOOL m_bCharCode;
  IFX_Font* m_pFont;
  int32_t m_iFontHeight;
  int32_t m_iFontSize;
  int32_t m_iTabWidth;
  CFX_Int32Array m_PositionedTabs;
  FX_BOOL m_bOrphanLine;
  FX_WCHAR m_wDefChar;
  int32_t m_iDefChar;
  FX_WCHAR m_wLineBreakChar;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
  int32_t m_iLineRotation;
  int32_t m_iCharRotation;
  int32_t m_iRotation;
  int32_t m_iCharSpace;
  FX_BOOL m_bWordSpace;
  int32_t m_iWordSpace;
  FX_BOOL m_bRTL;
  int32_t m_iAlignment;
  IFX_Unknown* m_pUserData;
  FX_DWORD m_dwCharType;
  FX_DWORD m_dwIdentity;
  CFX_RTFLine m_RTFLine1;
  CFX_RTFLine m_RTFLine2;
  CFX_RTFLine* m_pCurLine;
  int32_t m_iReady;
  int32_t m_iTolerance;
  int32_t GetLineRotation(FX_DWORD dwStyles) const;
  void SetBreakStatus();
  CFX_RTFChar* GetLastChar(int32_t index) const;
  CFX_RTFLine* GetRTFLine(FX_BOOL bReady) const;
  CFX_RTFPieceArray* GetRTFPieces(FX_BOOL bReady) const;
  FX_DWORD GetUnifiedCharType(FX_DWORD dwType) const;
  int32_t GetLastPositionedTab() const;
  FX_BOOL GetPositionedTab(int32_t& iTabPos) const;
  int32_t GetBreakPos(CFX_RTFCharArray& tca,
                      int32_t& iEndPos,
                      FX_BOOL bAllChars = FALSE,
                      FX_BOOL bOnlyBrk = FALSE);
  void SplitTextLine(CFX_RTFLine* pCurLine,
                     CFX_RTFLine* pNextLine,
                     FX_BOOL bAllChars = FALSE);
  FX_BOOL EndBreak_SplitLine(CFX_RTFLine* pNextLine,
                             FX_BOOL bAllChars,
                             FX_DWORD dwStatus);
  void EndBreak_BidiLine(CFX_TPOArray& tpos, FX_DWORD dwStatus);
  void EndBreak_Alignment(CFX_TPOArray& tpos,
                          FX_BOOL bAllChars,
                          FX_DWORD dwStatus);
};
#endif
