// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_TEXTBREAK
#define _FX_TEXTBREAK

#include "core/include/fxcrt/fx_ucd.h"

class IFX_Font;
class CFX_Char;
class IFX_TxtAccess;
class CFX_TxtChar;
class CFX_TxtPiece;
class IFX_TxtBreak;
#define FX_TXTBREAKPOLICY_None 0x00
#define FX_TXTBREAKPOLICY_Pagination 0x01
#define FX_TXTBREAKPOLICY_SpaceBreak 0x02
#define FX_TXTBREAKPOLICY_NumberBreak 0x04
#define FX_TXTBREAK_None 0x00
#define FX_TXTBREAK_PieceBreak 0x01
#define FX_TXTBREAK_LineBreak 0x02
#define FX_TXTBREAK_ParagraphBreak 0x03
#define FX_TXTBREAK_PageBreak 0x04
#define FX_TXTBREAK_ControlChar 0x10
#define FX_TXTBREAK_BreakChar 0x20
#define FX_TXTBREAK_UnknownChar 0x40
#define FX_TXTBREAK_RemoveChar 0x80
#define FX_TXTLAYOUTSTYLE_MutipleFormat 0x0001
#define FX_TXTLAYOUTSTYLE_VerticalLayout 0x0002
#define FX_TXTLAYOUTSTYLE_VerticalChars 0x0004
#define FX_TXTLAYOUTSTYLE_ReverseLine 0x0008
#define FX_TXTLAYOUTSTYLE_ArabicContext 0x0010
#define FX_TXTLAYOUTSTYLE_ArabicShapes 0x0020
#define FX_TXTLAYOUTSTYLE_RTLReadingOrder 0x0040
#define FX_TXTLAYOUTSTYLE_ExpandTab 0x0100
#define FX_TXTLAYOUTSTYLE_SingleLine 0x0200
#define FX_TXTLAYOUTSTYLE_CombText 0x0400
#define FX_TXTCHARSTYLE_Alignment 0x000F
#define FX_TXTCHARSTYLE_ArabicNumber 0x0010
#define FX_TXTCHARSTYLE_ArabicShadda 0x0020
#define FX_TXTCHARSTYLE_OddBidiLevel 0x0040
#define FX_TXTCHARSTYLE_RTLReadingOrder 0x0080
#define FX_TXTCHARSTYLE_ArabicContext 0x0300
#define FX_TXTCHARSTYLE_ArabicIndic 0x0400
#define FX_TXTCHARSTYLE_ArabicComma 0x0800
#define FX_TXTLINEALIGNMENT_Left 0
#define FX_TXTLINEALIGNMENT_Center 1
#define FX_TXTLINEALIGNMENT_Right 2
#define FX_TXTLINEALIGNMENT_Justified (1 << 2)
#define FX_TXTLINEALIGNMENT_Distributed (2 << 2)
#define FX_TXTLINEALIGNMENT_JustifiedLeft \
  (FX_TXTLINEALIGNMENT_Left | FX_TXTLINEALIGNMENT_Justified)
#define FX_TXTLINEALIGNMENT_JustifiedCenter \
  (FX_TXTLINEALIGNMENT_Center | FX_TXTLINEALIGNMENT_Justified)
#define FX_TXTLINEALIGNMENT_JustifiedRight \
  (FX_TXTLINEALIGNMENT_Right | FX_TXTLINEALIGNMENT_Justified)
#define FX_TXTLINEALIGNMENT_DistributedLeft \
  (FX_TXTLINEALIGNMENT_Left | FX_TXTLINEALIGNMENT_Distributed)
#define FX_TXTLINEALIGNMENT_DistributedCenter \
  (FX_TXTLINEALIGNMENT_Center | FX_TXTLINEALIGNMENT_Distributed)
#define FX_TXTLINEALIGNMENT_DistributedRight \
  (FX_TXTLINEALIGNMENT_Right | FX_TXTLINEALIGNMENT_Distributed)
#define FX_TXTLINEALIGNMENT_LowerMask 0x03
#define FX_TXTLINEALIGNMENT_HigherMask 0x0C
#define FX_TXTBREAK_MinimumTabWidth 160000

class IFX_TxtAccess {
 public:
  virtual ~IFX_TxtAccess() {}
  virtual FX_WCHAR GetChar(void* pIdentity, int32_t index) const = 0;
  virtual int32_t GetWidth(void* pIdentity, int32_t index) const = 0;
};
typedef struct _FX_TXTRUN {
  _FX_TXTRUN() {
    pAccess = NULL;
    pIdentity = NULL;
    pStr = NULL;
    pWidths = NULL;
    iLength = 0;
    pFont = NULL;
    fFontSize = 12;
    dwStyles = 0;
    iHorizontalScale = 100;
    iVerticalScale = 100;
    iCharRotation = 0;
    dwCharStyles = 0;
    pRect = NULL;
    wLineBreakChar = L'\n';
    bSkipSpace = TRUE;
  }
  IFX_TxtAccess* pAccess;
  void* pIdentity;
  const FX_WCHAR* pStr;
  int32_t* pWidths;
  int32_t iLength;
  IFX_Font* pFont;
  FX_FLOAT fFontSize;
  FX_DWORD dwStyles;
  int32_t iHorizontalScale;
  int32_t iVerticalScale;
  int32_t iCharRotation;
  FX_DWORD dwCharStyles;
  FX_LPCRECTF pRect;
  FX_WCHAR wLineBreakChar;
  FX_BOOL bSkipSpace;
} FX_TXTRUN, *FX_LPTXTRUN;
typedef FX_TXTRUN const* FX_LPCTXTRUN;
class CFX_TxtPiece : public CFX_Target {
 public:
  CFX_TxtPiece()
      : m_dwStatus(FX_TXTBREAK_PieceBreak),
        m_iStartPos(0),
        m_iWidth(-1),
        m_iStartChar(0),
        m_iChars(0),
        m_iBidiLevel(0),
        m_iBidiPos(0),
        m_iHorizontalScale(100),
        m_iVerticalScale(100),
        m_dwCharStyles(0),
        m_pChars(NULL),
        m_pUserData(NULL) {}
  int32_t GetEndPos() const {
    return m_iWidth < 0 ? m_iStartPos : m_iStartPos + m_iWidth;
  }
  int32_t GetLength() const { return m_iChars; }
  int32_t GetEndChar() const { return m_iStartChar + m_iChars; }
  CFX_TxtChar* GetCharPtr(int32_t index) const {
    FXSYS_assert(index > -1 && index < m_iChars && m_pChars != NULL);
    return m_pChars->GetDataPtr(m_iStartChar + index);
  }
  void GetString(FX_WCHAR* pText) const {
    FXSYS_assert(pText != NULL);
    int32_t iEndChar = m_iStartChar + m_iChars;
    CFX_Char* pChar;
    for (int32_t i = m_iStartChar; i < iEndChar; i++) {
      pChar = m_pChars->GetDataPtr(i);
      *pText++ = (FX_WCHAR)pChar->m_wCharCode;
    }
  }

  void GetString(CFX_WideString& wsText) const {
    FX_WCHAR* pText = wsText.GetBuffer(m_iChars);
    GetString(pText);
    wsText.ReleaseBuffer(m_iChars);
  }
  void GetWidths(int32_t* pWidths) const {
    FXSYS_assert(pWidths != NULL);
    int32_t iEndChar = m_iStartChar + m_iChars;
    CFX_Char* pChar;
    for (int32_t i = m_iStartChar; i < iEndChar; i++) {
      pChar = m_pChars->GetDataPtr(i);
      *pWidths++ = pChar->m_iCharWidth;
    }
  }
  FX_DWORD m_dwStatus;
  int32_t m_iStartPos;
  int32_t m_iWidth;
  int32_t m_iStartChar;
  int32_t m_iChars;
  int32_t m_iBidiLevel;
  int32_t m_iBidiPos;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
  FX_DWORD m_dwCharStyles;
  CFX_TxtCharArray* m_pChars;
  void* m_pUserData;
};
typedef CFX_BaseArrayTemplate<CFX_TxtPiece> CFX_TxtPieceArray;
class IFX_TxtBreak {
 public:
  static IFX_TxtBreak* Create(FX_DWORD dwPolicies);
  virtual ~IFX_TxtBreak() {}
  virtual void Release() = 0;
  virtual void SetLineWidth(FX_FLOAT fLineWidth) = 0;
  virtual void SetLinePos(FX_FLOAT fLinePos) = 0;
  virtual FX_DWORD GetLayoutStyles() const = 0;
  virtual void SetLayoutStyles(FX_DWORD dwLayoutStyles) = 0;
  virtual void SetFont(IFX_Font* pFont) = 0;
  virtual void SetFontSize(FX_FLOAT fFontSize) = 0;
  virtual void SetTabWidth(FX_FLOAT fTabWidth, FX_BOOL bEquidistant) = 0;
  virtual void SetDefaultChar(FX_WCHAR wch) = 0;
  virtual void SetParagraphBreakChar(FX_WCHAR wch) = 0;
  virtual void SetLineBreakTolerance(FX_FLOAT fTolerance) = 0;
  virtual void SetHorizontalScale(int32_t iScale) = 0;
  virtual void SetVerticalScale(int32_t iScale) = 0;
  virtual void SetCharRotation(int32_t iCharRotation) = 0;
  virtual void SetCharSpace(FX_FLOAT fCharSpace) = 0;
  virtual void SetAlignment(int32_t iAlignment) = 0;
  virtual FX_DWORD GetContextCharStyles() const = 0;
  virtual void SetContextCharStyles(FX_DWORD dwCharStyles) = 0;
  virtual void SetCombWidth(FX_FLOAT fCombWidth) = 0;
  virtual void SetUserData(void* pUserData) = 0;
  virtual FX_DWORD AppendChar(FX_WCHAR wch) = 0;
  virtual FX_DWORD EndBreak(FX_DWORD dwStatus = FX_TXTBREAK_PieceBreak) = 0;
  virtual int32_t CountBreakChars() const = 0;
  virtual int32_t CountBreakPieces() const = 0;
  virtual const CFX_TxtPiece* GetBreakPiece(int32_t index) const = 0;
  virtual void ClearBreakPieces() = 0;
  virtual void Reset() = 0;
  virtual int32_t GetDisplayPos(
      FX_LPCTXTRUN pTxtRun,
      FXTEXT_CHARPOS* pCharPos,
      FX_BOOL bCharCode = FALSE,
      CFX_WideString* pWSForms = NULL,
      FX_AdjustCharDisplayPos pAdjustPos = NULL) const = 0;
  virtual int32_t GetCharRects(FX_LPCTXTRUN pTxtRun,
                               CFX_RectFArray& rtArray,
                               FX_BOOL bCharBBox = FALSE) const = 0;
};
#endif
