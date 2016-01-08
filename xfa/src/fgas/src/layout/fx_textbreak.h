// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_TEXTBREAK_IMP
#define _FX_TEXTBREAK_IMP

#include "core/include/fxcrt/fx_memory.h"

class IFX_ArabicChar;
class CFX_Txtbreak;

class CFX_TxtLine {
 public:
  CFX_TxtLine(int32_t iBlockSize)
      : m_iStart(0), m_iWidth(0), m_iArabicChars(0) {
    m_pLineChars = new CFX_TxtCharArray;
    m_pLinePieces = new CFX_TxtPieceArray(16);
  }
  ~CFX_TxtLine() {
    RemoveAll();
    delete m_pLineChars;
    delete m_pLinePieces;
  }
  int32_t CountChars() const { return m_pLineChars->GetSize(); }
  CFX_TxtChar* GetCharPtr(int32_t index) const {
    FXSYS_assert(index > -1 && index < m_pLineChars->GetSize());
    return m_pLineChars->GetDataPtr(index);
  }
  int32_t CountPieces() const { return m_pLinePieces->GetSize(); }
  CFX_TxtPiece* GetPiecePtr(int32_t index) const {
    FXSYS_assert(index > -1 && index < m_pLinePieces->GetSize());
    return m_pLinePieces->GetPtrAt(index);
  }
  void GetString(CFX_WideString& wsStr) const {
    int32_t iCount = m_pLineChars->GetSize();
    FX_WCHAR* pBuf = wsStr.GetBuffer(iCount);
    CFX_Char* pChar;
    for (int32_t i = 0; i < iCount; i++) {
      pChar = m_pLineChars->GetDataPtr(i);
      *pBuf++ = (FX_WCHAR)pChar->m_wCharCode;
    }
    wsStr.ReleaseBuffer(iCount);
  }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    m_pLineChars->RemoveAll();
    m_pLinePieces->RemoveAll(bLeaveMemory);
    m_iWidth = 0;
    m_iArabicChars = 0;
  }
  CFX_TxtCharArray* m_pLineChars;
  CFX_TxtPieceArray* m_pLinePieces;
  int32_t m_iStart;
  int32_t m_iWidth;
  int32_t m_iArabicChars;
};
class CFX_TxtBreak : public IFX_TxtBreak {
 public:
  CFX_TxtBreak(FX_DWORD dwPolicies);
  ~CFX_TxtBreak();
  virtual void Release() { delete this; }
  virtual void SetLineWidth(FX_FLOAT fLineWidth);
  virtual void SetLinePos(FX_FLOAT fLinePos);
  virtual FX_DWORD GetLayoutStyles() const { return m_dwLayoutStyles; }
  virtual void SetLayoutStyles(FX_DWORD dwLayoutStyles);
  virtual void SetFont(IFX_Font* pFont);
  virtual void SetFontSize(FX_FLOAT fFontSize);
  virtual void SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant);
  virtual void SetDefaultChar(FX_WCHAR wch);
  virtual void SetParagraphBreakChar(FX_WCHAR wch);
  virtual void SetLineBreakTolerance(FX_FLOAT fTolerance);
  virtual void SetHorizontalScale(int32_t iScale);
  virtual void SetVerticalScale(int32_t iScale);
  virtual void SetCharRotation(int32_t iCharRotation);
  virtual void SetCharSpace(FX_FLOAT fCharSpace);
  virtual void SetAlignment(int32_t iAlignment);
  virtual FX_DWORD GetContextCharStyles() const;
  virtual void SetContextCharStyles(FX_DWORD dwCharStyles);
  virtual void SetCombWidth(FX_FLOAT fCombWidth);
  virtual void SetUserData(void* pUserData);
  virtual FX_DWORD AppendChar(FX_WCHAR wch);
  virtual FX_DWORD EndBreak(FX_DWORD dwStatus = FX_TXTBREAK_PieceBreak);
  virtual int32_t CountBreakChars() const;
  virtual int32_t CountBreakPieces() const;
  virtual const CFX_TxtPiece* GetBreakPiece(int32_t index) const;
  virtual void ClearBreakPieces();
  virtual void Reset();
  virtual int32_t GetDisplayPos(
      FX_LPCTXTRUN pTxtRun,
      FXTEXT_CHARPOS* pCharPos,
      FX_BOOL bCharCode = FALSE,
      CFX_WideString* pWSForms = NULL,
      FX_AdjustCharDisplayPos pAdjustPos = NULL) const;
  virtual int32_t GetCharRects(FX_LPCTXTRUN pTxtRun,
                               CFX_RectFArray& rtArray,
                               FX_BOOL bCharBBox = FALSE) const;
  void AppendChar_PageLoad(CFX_Char* pCurChar, FX_DWORD dwProps);
  FX_DWORD AppendChar_Combination(CFX_Char* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Tab(CFX_Char* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Control(CFX_Char* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Arabic(CFX_Char* pCurChar, int32_t iRotation);
  FX_DWORD AppendChar_Others(CFX_Char* pCurChar, int32_t iRotation);

 protected:
  FX_DWORD m_dwPolicies;
  FX_BOOL m_bPagination;
  IFX_ArabicChar* m_pArabicChar;
  int32_t m_iLineWidth;
  FX_DWORD m_dwLayoutStyles;
  FX_BOOL m_bVertical;
  FX_BOOL m_bArabicContext;
  FX_BOOL m_bArabicShapes;
  FX_BOOL m_bRTL;
  FX_BOOL m_bSingleLine;
  FX_BOOL m_bCombText;
  int32_t m_iArabicContext;
  int32_t m_iCurArabicContext;
  IFX_Font* m_pFont;
  int32_t m_iFontSize;
  FX_BOOL m_bEquidistant;
  int32_t m_iTabWidth;
  FX_WCHAR m_wDefChar;
  FX_WCHAR m_wParagBreakChar;
  int32_t m_iDefChar;
  int32_t m_iLineRotation;
  int32_t m_iCharRotation;
  int32_t m_iRotation;
  int32_t m_iAlignment;
  FX_DWORD m_dwContextCharStyles;
  int32_t m_iCombWidth;
  void* m_pUserData;
  FX_DWORD m_dwCharType;
  FX_BOOL m_bCurRTL;
  int32_t m_iCurAlignment;
  FX_BOOL m_bArabicNumber;
  FX_BOOL m_bArabicComma;
  CFX_TxtLine* m_pTxtLine1;
  CFX_TxtLine* m_pTxtLine2;
  CFX_TxtLine* m_pCurLine;
  int32_t m_iReady;
  int32_t m_iTolerance;
  int32_t m_iHorScale;
  int32_t m_iVerScale;
  int32_t m_iCharSpace;
  void SetBreakStatus();
  int32_t GetLineRotation(FX_DWORD dwStyles) const;
  CFX_TxtChar* GetLastChar(int32_t index, FX_BOOL bOmitChar = TRUE) const;
  CFX_TxtLine* GetTxtLine(FX_BOOL bReady) const;
  CFX_TxtPieceArray* GetTxtPieces(FX_BOOL bReady) const;
  FX_DWORD GetUnifiedCharType(FX_DWORD dwType) const;
  void ResetArabicContext();
  void ResetContextCharStyles();
  void EndBreak_UpdateArabicShapes();
  FX_BOOL EndBreak_SplitLine(CFX_TxtLine* pNextLine,
                             FX_BOOL bAllChars,
                             FX_DWORD dwStatus);
  void EndBreak_BidiLine(CFX_TPOArray& tpos, FX_DWORD dwStatus);
  void EndBreak_Alignment(CFX_TPOArray& tpos,
                          FX_BOOL bAllChars,
                          FX_DWORD dwStatus);
  int32_t GetBreakPos(CFX_TxtCharArray& ca,
                      int32_t& iEndPos,
                      FX_BOOL bAllChars = FALSE,
                      FX_BOOL bOnlyBrk = FALSE);
  void SplitTextLine(CFX_TxtLine* pCurLine,
                     CFX_TxtLine* pNextLine,
                     FX_BOOL bAllChars = FALSE);
};
#endif
