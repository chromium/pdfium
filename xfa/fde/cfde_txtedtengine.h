// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTENGINE_H_
#define XFA_FDE_CFDE_TXTEDTENGINE_H_

#include <memory>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fde/cfde_txtedtbuf.h"
#include "xfa/fde/cfde_txtedtpage.h"
#include "xfa/fde/cfde_txtedtparag.h"
#include "xfa/fgas/layout/cfx_txtbreak.h"

class CFGAS_GEFont;
class CFWL_Edit;
class IFDE_TxtEdtDoRecord;
class IFX_CharIter;

#define FDE_TEXTEDITMODE_MultiLines (1L << 0)
#define FDE_TEXTEDITMODE_AutoLineWrap (1L << 1)
#define FDE_TEXTEDITMODE_LimitArea_Vert (1L << 3)
#define FDE_TEXTEDITMODE_LimitArea_Horz (1L << 4)
#define FDE_TEXTEDITMODE_Validate (1L << 8)
#define FDE_TEXTEDITMODE_Password (1L << 9)

#define FDE_TEXTEDITALIGN_Left 0
#define FDE_TEXTEDITALIGN_Center (1L << 0)
#define FDE_TEXTEDITALIGN_Right (1L << 1)
#define FDE_TEXTEDITALIGN_Justified (1L << 4)

#define FDE_TEXTEDITLAYOUT_CombText (1L << 4)
#define FDE_TEXTEDITLAYOUT_LastLineHeight (1L << 8)

enum class FDE_CaretMove {
  Left,
  Right,
  Up,
  Down,
  LineStart,
  LineEnd,
  Home,
  End,
};

enum class FDE_EditResult {
  kLocked = -5,
  kInvalidate = -4,
  kFull = -2,
  kSuccess = 0,
};

struct FDE_TXTEDTPARAMS {
  FDE_TXTEDTPARAMS();
  ~FDE_TXTEDTPARAMS();

  float fPlateWidth;
  float fPlateHeight;

  int32_t nLineCount;
  uint32_t dwLayoutStyles;
  uint32_t dwAlignment;
  uint32_t dwMode;

  CFX_RetainPtr<CFGAS_GEFont> pFont;
  float fFontSize;
  FX_ARGB dwFontColor;

  float fLineSpace;
  float fTabWidth;

  CFWL_Edit* pEventSink;
};

class CFDE_TxtEdtEngine {
 public:
  CFDE_TxtEdtEngine();
  ~CFDE_TxtEdtEngine();

  void SetEditParams(const FDE_TXTEDTPARAMS& params);
  FDE_TXTEDTPARAMS* GetEditParams() { return &m_Param; }

  CFDE_TxtEdtPage* GetPage(int32_t nIndex);

  void SetText(const CFX_WideString& wsText);
  int32_t GetTextLength() const { return m_pTxtBuf->GetTextLength() - 1; }
  CFX_WideString GetText(int32_t nStart, int32_t nCount) const;
  void ClearText();

  CFX_RectF GetCaretRect() const { return m_rtCaret; }
  int32_t GetCaretPos() const {
    return IsLocked() ? 0 : m_nCaret + (m_bBefore ? 0 : 1);
  }
  int32_t SetCaretPos(int32_t nIndex, bool bBefore);
  int32_t MoveCaretPos(FDE_CaretMove eMoveCaret, bool bShift);

  FDE_EditResult Insert(const CFX_WideString& str);
  void Delete(bool bBackspace);

  void SetLimit(int32_t nLimit) { m_nLimit = nLimit; }
  int32_t GetLimit() const { return m_nLimit; }
  void SetAliasChar(wchar_t wcAlias) { m_wcAliasChar = wcAlias; }

  void RemoveSelRange(int32_t nStart, int32_t nCount);
  void AddSelRange(int32_t nStart, int32_t nCount);
  int32_t CountSelRanges() const {
    return pdfium::CollectionSize<int32_t>(m_SelRangePtrArr);
  }
  int32_t GetSelRange(int32_t nIndex, int32_t* nStart) const;
  void ClearSelection();

  bool Redo(const IFDE_TxtEdtDoRecord* pRecord);
  bool Undo(const IFDE_TxtEdtDoRecord* pRecord);

  void Layout();

  CFDE_TxtEdtParag* GetParag(int32_t nParagIndex) const {
    return m_ParagPtrArray[nParagIndex].get();
  }
  CFDE_TxtEdtBuf* GetTextBuf() const { return m_pTxtBuf.get(); }

  CFX_TxtBreak* GetTextBreak() { return &m_TextBreak; }
  int32_t GetLineCount() const { return m_nLineCount; }
  int32_t GetPageLineCount() const { return m_nPageLineCount; }

  int32_t Line2Parag(int32_t nStartParag,
                     int32_t nStartLineofParag,
                     int32_t nLineIndex,
                     int32_t& nStartLine) const;
  wchar_t GetAliasChar() const { return m_wcAliasChar; }

  bool IsSelect() const { return !m_SelRangePtrArr.empty(); }
  void Inner_DeleteRange(int32_t nStart, int32_t nCount);
  void Inner_Insert(int32_t nStart, const CFX_WideString& wsText);
  const FDE_TXTEDTPARAMS* GetParams() const { return &m_Param; }

 private:
  struct FDE_TXTEDTSELRANGE {
    int32_t nStart;
    int32_t nCount;
  };

  struct FDE_TXTEDTPARAGPOS {
    int32_t nParagIndex;
    int32_t nCharIndex;
  };

  enum class LineEnding : uint8_t {
    kAuto,
    kCRLF,
    kCR,
    kLF,
  };

  int32_t CountPages() const {
    return m_nLineCount == 0 ? 0 : ((m_nLineCount - 1) / m_nPageLineCount) + 1;
  }

  bool IsLocked() const { return m_bLock; }

  CFX_WideString InsertIntoTextCopy(int32_t nIndex,
                                    const wchar_t* lpText,
                                    int32_t nLength);

  void DeleteRange_DoRecord(int32_t nStart, int32_t nCount, bool bSel);
  void ResetEngine();
  void RebuildParagraphs();
  void RemoveAllParags() { m_ParagPtrArray.clear(); }
  void RemoveAllPages() { m_PagePtrArray.clear(); }
  void UpdateLineCounts();
  void UpdatePages();
  void UpdateTxtBreak();

  bool ReplaceParagEnd(wchar_t*& lpText, int32_t& nLength, bool bPreIsCR);
  void RecoverParagEnd(CFX_WideString& wsText) const;
  int32_t MovePage2Char(int32_t nIndex);
  void TextPos2ParagPos(int32_t nIndex, FDE_TXTEDTPARAGPOS& ParagPos) const;
  int32_t MoveForward(bool& bBefore);
  int32_t MoveBackward(bool& bBefore);
  bool MoveUp(CFX_PointF& ptCaret);
  bool MoveDown(CFX_PointF& ptCaret);
  bool MoveLineStart();
  bool MoveLineEnd();
  bool MoveHome();
  bool MoveEnd();
  bool IsFitArea(CFX_WideString& wsText);
  void UpdateCaretRect(int32_t nIndex, bool bBefore);
  void GetCaretRect(CFX_RectF& rtCaret,
                    int32_t nPageIndex,
                    int32_t nCaret,
                    bool bBefore);
  void UpdateCaretIndex(const CFX_PointF& ptCaret);

  void DeleteSelect();

  std::unique_ptr<CFDE_TxtEdtBuf> m_pTxtBuf;
  CFX_TxtBreak m_TextBreak;
  FDE_TXTEDTPARAMS m_Param;
  std::vector<std::unique_ptr<CFDE_TxtEdtPage>> m_PagePtrArray;
  std::vector<std::unique_ptr<CFDE_TxtEdtParag>> m_ParagPtrArray;
  std::vector<std::unique_ptr<FDE_TXTEDTSELRANGE>> m_SelRangePtrArr;
  int32_t m_nPageLineCount;
  int32_t m_nLineCount;
  int32_t m_nAnchorPos;
  float m_fCaretPosReserve;
  int32_t m_nCaret;
  int32_t m_nCaretPage;
  CFX_RectF m_rtCaret;
  int32_t m_nLimit;
  wchar_t m_wcAliasChar;
  LineEnding m_FirstLineEnding;
  bool m_bBefore;
  bool m_bLock;
  bool m_bAutoLineEnd;
};

#endif  // XFA_FDE_CFDE_TXTEDTENGINE_H_
