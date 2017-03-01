// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_FGAS_RTFBREAK_H_
#define XFA_FGAS_LAYOUT_FGAS_RTFBREAK_H_

#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_ucd.h"
#include "xfa/fgas/crt/fgas_utils.h"
#include "xfa/fgas/layout/fgas_textbreak.h"
#include "xfa/fgas/layout/fgas_unicode.h"

class CFGAS_GEFont;

#define FX_RTFLAYOUTSTYLE_Pagination 0x01
#define FX_RTFLAYOUTSTYLE_ExpandTab 0x10

enum class CFX_RTFLineAlignment {
  Left = 0,
  Center,
  Right,
  Justified,
  Distributed
};

struct FX_RTFTEXTOBJ {
  FX_RTFTEXTOBJ();
  ~FX_RTFTEXTOBJ();

  const FX_WCHAR* pStr;
  int32_t* pWidths;
  int32_t iLength;
  CFX_RetainPtr<CFGAS_GEFont> pFont;
  FX_FLOAT fFontSize;
  int32_t iBidiLevel;
  const CFX_RectF* pRect;
  FX_WCHAR wLineBreakChar;
  int32_t iHorizontalScale;
  int32_t iVerticalScale;
};

class CFX_RTFPiece {
 public:
  CFX_RTFPiece();
  ~CFX_RTFPiece();

  void AppendChar(const CFX_RTFChar& tc) {
    ASSERT(m_pChars);
    m_pChars->push_back(tc);
    if (m_iWidth < 0)
      m_iWidth = tc.m_iCharWidth;
    else
      m_iWidth += tc.m_iCharWidth;
    m_iChars++;
  }

  int32_t GetEndPos() const {
    return m_iWidth < 0 ? m_iStartPos : m_iStartPos + m_iWidth;
  }

  int32_t GetLength() const { return m_iChars; }
  int32_t GetEndChar() const { return m_iStartChar + m_iChars; }

  CFX_RTFChar& GetChar(int32_t index) {
    ASSERT(index > -1 && index < m_iChars && m_pChars);
    return (*m_pChars)[m_iStartChar + index];
  }

  CFX_RTFChar* GetCharPtr(int32_t index) const {
    ASSERT(index > -1 && index < m_iChars && m_pChars);
    return &(*m_pChars)[m_iStartChar + index];
  }

  void GetString(FX_WCHAR* pText) const {
    ASSERT(pText);
    int32_t iEndChar = m_iStartChar + m_iChars;
    for (int32_t i = m_iStartChar; i < iEndChar; i++)
      *pText++ = static_cast<FX_WCHAR>((*m_pChars)[i].m_wCharCode);
  }

  void GetString(CFX_WideString& wsText) const {
    FX_WCHAR* pText = wsText.GetBuffer(m_iChars);
    GetString(pText);
    wsText.ReleaseBuffer(m_iChars);
  }

  void GetWidths(int32_t* pWidths) const {
    ASSERT(pWidths);
    int32_t iEndChar = m_iStartChar + m_iChars;
    for (int32_t i = m_iStartChar; i < iEndChar; i++)
      *pWidths++ = (*m_pChars)[i].m_iCharWidth;
  }

  void Reset() {
    m_dwStatus = CFX_RTFBreakType::Piece;
    if (m_iWidth > -1)
      m_iStartPos += m_iWidth;

    m_iWidth = -1;
    m_iStartChar += m_iChars;
    m_iChars = 0;
    m_iBidiLevel = 0;
    m_iBidiPos = 0;
    m_iHorizontalScale = 100;
    m_iVerticalScale = 100;
  }

  CFX_RTFBreakType m_dwStatus;
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
  uint32_t m_dwIdentity;
  std::vector<CFX_RTFChar>* m_pChars;  // not owned.
  CFX_RetainPtr<CFX_Retainable> m_pUserData;
};

typedef CFX_BaseArrayTemplate<CFX_RTFPiece> CFX_RTFPieceArray;

class CFX_RTFLine {
 public:
  CFX_RTFLine();
  ~CFX_RTFLine();

  int32_t CountChars() const {
    return pdfium::CollectionSize<int32_t>(m_LineChars);
  }

  CFX_RTFChar& GetChar(int32_t index) {
    ASSERT(index >= 0 && index < pdfium::CollectionSize<int32_t>(m_LineChars));
    return m_LineChars[index];
  }

  CFX_RTFChar* GetCharPtr(int32_t index) {
    ASSERT(index > -1 && index < pdfium::CollectionSize<int32_t>(m_LineChars));
    return &m_LineChars[index];
  }

  int32_t CountPieces() const { return m_LinePieces.GetSize(); }
  CFX_RTFPiece& GetPiece(int32_t index) const {
    ASSERT(index > -1 && index < m_LinePieces.GetSize());
    return m_LinePieces.GetAt(index);
  }

  CFX_RTFPiece* GetPiecePtr(int32_t index) const {
    ASSERT(index > -1 && index < m_LinePieces.GetSize());
    return m_LinePieces.GetPtrAt(index);
  }

  int32_t GetLineEnd() const { return m_iStart + m_iWidth; }
  void RemoveAll(bool bLeaveMemory = false) {
    m_LineChars.clear();
    m_LinePieces.RemoveAll(bLeaveMemory);
    m_iWidth = 0;
    m_iArabicChars = 0;
    m_iMBCSChars = 0;
  }

  std::vector<CFX_RTFChar> m_LineChars;
  CFX_RTFPieceArray m_LinePieces;
  int32_t m_iStart;
  int32_t m_iWidth;
  int32_t m_iArabicChars;
  int32_t m_iMBCSChars;
};

class CFX_RTFBreak {
 public:
  explicit CFX_RTFBreak(uint32_t dwLayoutStyles);
  ~CFX_RTFBreak();

  void SetLineBoundary(FX_FLOAT fLineStart, FX_FLOAT fLineEnd);
  void SetLineStartPos(FX_FLOAT fLinePos);
  void SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont);
  void SetFontSize(FX_FLOAT fFontSize);
  void SetTabWidth(FX_FLOAT fTabWidth);
  void AddPositionedTab(FX_FLOAT fTabPos);
  void SetLineBreakTolerance(FX_FLOAT fTolerance);
  void SetHorizontalScale(int32_t iScale);
  void SetVerticalScale(int32_t iScale);
  void SetCharSpace(FX_FLOAT fCharSpace);
  void SetAlignment(CFX_RTFLineAlignment align) { m_iAlignment = align; }
  void SetUserData(const CFX_RetainPtr<CFX_Retainable>& pUserData);
  CFX_RTFBreakType AppendChar(FX_WCHAR wch);
  CFX_RTFBreakType EndBreak(CFX_RTFBreakType dwStatus);
  int32_t CountBreakPieces() const;
  const CFX_RTFPiece* GetBreakPiece(int32_t index) const;
  void GetLineRect(CFX_RectF& rect) const;
  void ClearBreakPieces();
  void Reset();
  int32_t GetDisplayPos(const FX_RTFTEXTOBJ* pText,
                        FXTEXT_CHARPOS* pCharPos,
                        bool bCharCode = false,
                        CFX_WideString* pWSForms = nullptr,
                        FX_AdjustCharDisplayPos pAdjustPos = nullptr) const;
  CFX_RTFBreakType AppendChar_Combination(CFX_RTFChar* pCurChar);
  CFX_RTFBreakType AppendChar_Tab(CFX_RTFChar* pCurChar);
  CFX_RTFBreakType AppendChar_Control(CFX_RTFChar* pCurChar);
  CFX_RTFBreakType AppendChar_Arabic(CFX_RTFChar* pCurChar);
  CFX_RTFBreakType AppendChar_Others(CFX_RTFChar* pCurChar);

 protected:
  void FontChanged();
  void SetBreakStatus();
  CFX_RTFChar* GetLastChar(int32_t index) const;
  const CFX_RTFLine* GetRTFLine() const;
  const CFX_RTFPieceArray* GetRTFPieces() const;
  FX_CHARTYPE GetUnifiedCharType(FX_CHARTYPE chartype) const;
  int32_t GetLastPositionedTab() const;
  bool GetPositionedTab(int32_t* iTabPos) const;

  int32_t GetBreakPos(std::vector<CFX_RTFChar>& tca,
                      int32_t& iEndPos,
                      bool bAllChars = false,
                      bool bOnlyBrk = false);
  void SplitTextLine(CFX_RTFLine* pCurLine,
                     CFX_RTFLine* pNextLine,
                     bool bAllChars = false);
  bool EndBreak_SplitLine(CFX_RTFLine* pNextLine,
                          bool bAllChars,
                          CFX_RTFBreakType dwStatus);
  void EndBreak_BidiLine(CFX_TPOArray* tpos, CFX_RTFBreakType dwStatus);
  void EndBreak_Alignment(const CFX_TPOArray& tpos,
                          bool bAllChars,
                          CFX_RTFBreakType dwStatus);

  int32_t m_iBoundaryStart;
  int32_t m_iBoundaryEnd;
  uint32_t m_dwLayoutStyles;
  bool m_bPagination;
  CFX_RetainPtr<CFGAS_GEFont> m_pFont;
  int32_t m_iFontHeight;
  int32_t m_iFontSize;
  int32_t m_iTabWidth;
  CFX_ArrayTemplate<int32_t> m_PositionedTabs;
  FX_WCHAR m_wDefChar;
  int32_t m_iDefChar;
  FX_WCHAR m_wLineBreakChar;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
  int32_t m_iCharSpace;
  CFX_RTFLineAlignment m_iAlignment;
  CFX_RetainPtr<CFX_Retainable> m_pUserData;
  FX_CHARTYPE m_eCharType;
  uint32_t m_dwIdentity;
  CFX_RTFLine m_RTFLine1;
  CFX_RTFLine m_RTFLine2;
  CFX_RTFLine* m_pCurLine;
  int32_t m_iReady;
  int32_t m_iTolerance;
};

#endif  // XFA_FGAS_LAYOUT_FGAS_RTFBREAK_H_
