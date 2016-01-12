// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_RTFBREAK
#define _FX_RTFBREAK
class IFX_Unknown;
class IFX_Font;
class CFX_Char;
class CFX_RTFChar;
class CFX_RTFBreakPiece;
class IFX_RTFBreak;
#define FX_RTFBREAKPOLICY_None 0x00
#define FX_RTFBREAKPOLICY_SpaceBreak 0x01
#define FX_RTFBREAKPOLICY_NumberBreak 0x02
#define FX_RTFBREAKPOLICY_InfixBreak 0x04
#define FX_RTFBREAKPOLICY_TabBreak 0x08
#define FX_RTFBREAKPOLICY_OrphanPositionedTab 0x10
#define FX_RTFBREAK_None 0x00
#define FX_RTFBREAK_PieceBreak 0x01
#define FX_RTFBREAK_LineBreak 0x02
#define FX_RTFBREAK_ParagraphBreak 0x03
#define FX_RTFBREAK_PageBreak 0x04
#define FX_RTFLAYOUTSTYLE_Pagination 0x01
#define FX_RTFLAYOUTSTYLE_VerticalLayout 0x02
#define FX_RTFLAYOUTSTYLE_VerticalChars 0x04
#define FX_RTFLAYOUTSTYLE_LineDirection 0x08
#define FX_RTFLAYOUTSTYLE_ExpandTab 0x10
#define FX_RTFLAYOUTSTYLE_ArabicNumber 0x20
#define FX_RTFLAYOUTSTYLE_SingleLine 0x40
#define FX_RTFLAYOUTSTYLE_MBCSCode 0x80
#define FX_RTFCHARSTYLE_Alignment 0x000F
#define FX_RTFCHARSTYLE_ArabicNumber 0x0010
#define FX_RTFCHARSTYLE_ArabicShadda 0x0020
#define FX_RTFCHARSTYLE_OddBidiLevel 0x0040
#define FX_RTFCHARSTYLE_RTLReadingOrder 0x0080
#define FX_RTFCHARSTYLE_ArabicContext 0x0300
#define FX_RTFCHARSTYLE_ArabicIndic 0x0400
#define FX_RTFCHARSTYLE_ArabicComma 0x0800
#define FX_RTFLINEALIGNMENT_Left 0
#define FX_RTFLINEALIGNMENT_Center 1
#define FX_RTFLINEALIGNMENT_Right 2
#define FX_RTFLINEALIGNMENT_Justified (1 << 2)
#define FX_RTFLINEALIGNMENT_Distributed (2 << 2)
#define FX_RTFLINEALIGNMENT_JustifiedLeft \
  (FX_RTFLINEALIGNMENT_Left | FX_RTFLINEALIGNMENT_Justified)
#define FX_RTFLINEALIGNMENT_JustifiedCenter \
  (FX_RTFLINEALIGNMENT_Center | FX_RTFLINEALIGNMENT_Justified)
#define FX_RTFLINEALIGNMENT_JustifiedRight \
  (FX_RTFLINEALIGNMENT_Right | FX_RTFLINEALIGNMENT_Justified)
#define FX_RTFLINEALIGNMENT_DistributedLeft \
  (FX_RTFLINEALIGNMENT_Left | FX_RTFLINEALIGNMENT_Distributed)
#define FX_RTFLINEALIGNMENT_DistributedCenter \
  (FX_RTFLINEALIGNMENT_Center | FX_RTFLINEALIGNMENT_Distributed)
#define FX_RTFLINEALIGNMENT_DistributedRight \
  (FX_RTFLINEALIGNMENT_Right | FX_RTFLINEALIGNMENT_Distributed)
#define FX_RTFLINEALIGNMENT_LowerMask 0x03
#define FX_RTFLINEALIGNMENT_HigherMask 0x0C
typedef struct _FX_RTFTEXTOBJ {
  _FX_RTFTEXTOBJ() {
    pStr = NULL;
    pWidths = NULL;
    iLength = 0;
    pFont = NULL;
    fFontSize = 12.0f;
    dwLayoutStyles = 0;
    iCharRotation = 0;
    iBidiLevel = 0;
    pRect = NULL;
    wLineBreakChar = L'\n';
    iHorizontalScale = 100;
    iVerticalScale = 100;
  }
  const FX_WCHAR* pStr;
  int32_t* pWidths;
  int32_t iLength;
  IFX_Font* pFont;
  FX_FLOAT fFontSize;
  FX_DWORD dwLayoutStyles;
  int32_t iCharRotation;
  int32_t iBidiLevel;
  FX_LPCRECTF pRect;
  FX_WCHAR wLineBreakChar;
  int32_t iHorizontalScale;
  int32_t iVerticalScale;
} FX_RTFTEXTOBJ, *FX_LPRTFTEXTOBJ;
typedef FX_RTFTEXTOBJ const* FX_LPCRTFTEXTOBJ;
class CFX_RTFPiece : public CFX_Target {
 public:
  CFX_RTFPiece()
      : m_dwStatus(FX_RTFBREAK_PieceBreak),
        m_iStartPos(0),
        m_iWidth(-1),
        m_iStartChar(0),
        m_iChars(0),
        m_iBidiLevel(0),
        m_iBidiPos(0),
        m_iFontSize(0),
        m_iFontHeight(0),
        m_iHorizontalScale(100),
        m_iVerticalScale(100),
        m_dwLayoutStyles(0),
        m_dwIdentity(0),
        m_pChars(NULL),
        m_pUserData(NULL) {}
  ~CFX_RTFPiece() { Reset(); }
  void AppendChar(const CFX_RTFChar& tc) {
    FXSYS_assert(m_pChars != NULL);
    m_pChars->Add(tc);
    if (m_iWidth < 0) {
      m_iWidth = tc.m_iCharWidth;
    } else {
      m_iWidth += tc.m_iCharWidth;
    }
    m_iChars++;
  }
  int32_t GetEndPos() const {
    return m_iWidth < 0 ? m_iStartPos : m_iStartPos + m_iWidth;
  }
  int32_t GetLength() const { return m_iChars; }
  int32_t GetEndChar() const { return m_iStartChar + m_iChars; }
  CFX_RTFChar& GetChar(int32_t index) {
    FXSYS_assert(index > -1 && index < m_iChars && m_pChars != NULL);
    return *m_pChars->GetDataPtr(m_iStartChar + index);
  }
  CFX_RTFChar* GetCharPtr(int32_t index) const {
    FXSYS_assert(index > -1 && index < m_iChars && m_pChars != NULL);
    return m_pChars->GetDataPtr(m_iStartChar + index);
  }
  void GetString(FX_WCHAR* pText) const {
    FXSYS_assert(pText != NULL);
    int32_t iEndChar = m_iStartChar + m_iChars;
    CFX_RTFChar* pChar;
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
    CFX_RTFChar* pChar;
    for (int32_t i = m_iStartChar; i < iEndChar; i++) {
      pChar = m_pChars->GetDataPtr(i);
      *pWidths++ = pChar->m_iCharWidth;
    }
  }
  void Reset() {
    m_dwStatus = FX_RTFBREAK_PieceBreak;
    if (m_iWidth > -1) {
      m_iStartPos += m_iWidth;
    }
    m_iWidth = -1;
    m_iStartChar += m_iChars;
    m_iChars = 0;
    m_iBidiLevel = 0;
    m_iBidiPos = 0;
    m_iHorizontalScale = 100;
    m_iVerticalScale = 100;
  }
  FX_DWORD m_dwStatus;
  int32_t m_iStartPos;
  int32_t m_iWidth;
  int32_t m_iStartChar;
  int32_t m_iChars;
  int32_t m_iBidiLevel;
  int32_t m_iBidiPos;
  int32_t m_iFontSize;
  int32_t m_iFontHeight;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
  FX_DWORD m_dwLayoutStyles;
  FX_DWORD m_dwIdentity;
  CFX_RTFCharArray* m_pChars;
  IFX_Unknown* m_pUserData;
};
typedef CFX_BaseArrayTemplate<CFX_RTFPiece> CFX_RTFPieceArray;
class IFX_RTFBreak {
 public:
  static IFX_RTFBreak* Create(FX_DWORD dwPolicies);
  virtual ~IFX_RTFBreak() {}
  virtual void Release() = 0;
  virtual void SetLineBoundary(FX_FLOAT fLineStart, FX_FLOAT fLineEnd) = 0;
  virtual void SetLineStartPos(FX_FLOAT fLinePos) = 0;
  virtual FX_DWORD GetLayoutStyles() const = 0;
  virtual void SetLayoutStyles(FX_DWORD dwLayoutStyles) = 0;
  virtual void SetFont(IFX_Font* pFont) = 0;
  virtual void SetFontSize(FX_FLOAT fFontSize) = 0;
  virtual void SetTabWidth(FX_FLOAT fTabWidth) = 0;
  virtual void AddPositionedTab(FX_FLOAT fTabPos) = 0;
  virtual void SetPositionedTabs(const CFX_FloatArray& tabs) = 0;
  virtual void ClearPositionedTabs() = 0;
  virtual void SetDefaultChar(FX_WCHAR wch) = 0;
  virtual void SetLineBreakChar(FX_WCHAR wch) = 0;
  virtual void SetLineBreakTolerance(FX_FLOAT fTolerance) = 0;
  virtual void SetHorizontalScale(int32_t iScale) = 0;
  virtual void SetVerticalScale(int32_t iScale) = 0;
  virtual void SetCharRotation(int32_t iCharRotation) = 0;
  virtual void SetCharSpace(FX_FLOAT fCharSpace) = 0;
  virtual void SetWordSpace(FX_BOOL bDefault, FX_FLOAT fWordSpace) = 0;
  virtual void SetReadingOrder(FX_BOOL bRTL = FALSE) = 0;
  virtual void SetAlignment(int32_t iAlignment = FX_RTFLINEALIGNMENT_Left) = 0;
  virtual void SetUserData(IFX_Unknown* pUserData) = 0;
  virtual FX_DWORD AppendChar(FX_WCHAR wch) = 0;
  virtual FX_DWORD EndBreak(FX_DWORD dwStatus = FX_RTFBREAK_PieceBreak) = 0;
  virtual int32_t CountBreakPieces() const = 0;
  virtual const CFX_RTFPiece* GetBreakPiece(int32_t index) const = 0;
  virtual void GetLineRect(CFX_RectF& rect) const = 0;
  virtual void ClearBreakPieces() = 0;
  virtual void Reset() = 0;
  virtual int32_t GetDisplayPos(
      FX_LPCRTFTEXTOBJ pText,
      FXTEXT_CHARPOS* pCharPos,
      FX_BOOL bCharCode = FALSE,
      CFX_WideString* pWSForms = NULL,
      FX_AdjustCharDisplayPos pAdjustPos = NULL) const = 0;
  virtual int32_t GetCharRects(FX_LPCRTFTEXTOBJ pText,
                               CFX_RectFArray& rtArray,
                               FX_BOOL bCharBBox = FALSE) const = 0;
};
#endif
