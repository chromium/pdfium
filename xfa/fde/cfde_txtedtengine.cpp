// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_txtedtengine.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/ifx_chariter.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fde/cfde_txtedtbuf.h"
#include "xfa/fde/cfde_txtedtpage.h"
#include "xfa/fde/cfde_txtedtparag.h"
#include "xfa/fgas/layout/cfx_txtbreak.h"
#include "xfa/fwl/cfwl_edit.h"

namespace {

const uint32_t kPageWidthMax = 0xffff;

enum FDE_TXTEDT_MODIFY_RET {
  FDE_TXTEDT_MODIFY_RET_F_Locked = -5,
  FDE_TXTEDT_MODIFY_RET_F_Invalidate = -4,
  FDE_TXTEDT_MODIFY_RET_F_Full = -2,
  FDE_TXTEDT_MODIFY_RET_S_Normal = 0,
};

class InsertOperation : public IFDE_TxtEdtDoRecord {
 public:
  InsertOperation(CFDE_TxtEdtEngine* pEngine,
                  int32_t nCaret,
                  const CFX_WideString& str)
      : m_pEngine(pEngine), m_nCaret(nCaret), m_wsInsert(str) {
    ASSERT(m_pEngine);
  }

  ~InsertOperation() override {}

  void Undo() const override {
    CFX_WideString prev = m_pEngine->GetText(0, -1);

    if (m_pEngine->IsSelect())
      m_pEngine->ClearSelection();

    m_pEngine->Inner_DeleteRange(m_nCaret, m_wsInsert.GetLength());
    m_pEngine->GetParams()->pEventSink->OnTextChanged(prev);
    m_pEngine->SetCaretPos(m_nCaret, true);
  }

  void Redo() const override {
    CFX_WideString prev = m_pEngine->GetText(0, -1);
    m_pEngine->Inner_Insert(m_nCaret, m_wsInsert);
    m_pEngine->GetParams()->pEventSink->OnTextChanged(prev);
    m_pEngine->SetCaretPos(m_nCaret, false);
  }

 private:
  CFDE_TxtEdtEngine* m_pEngine;
  int32_t m_nCaret;
  CFX_WideString m_wsInsert;
};

class DeleteOperation : public IFDE_TxtEdtDoRecord {
 public:
  DeleteOperation(CFDE_TxtEdtEngine* pEngine,
                  int32_t nIndex,
                  int32_t nCaret,
                  const CFX_WideString& wsRange,
                  bool bSel)
      : m_pEngine(pEngine),
        m_bSel(bSel),
        m_nIndex(nIndex),
        m_nCaret(nCaret),
        m_wsRange(wsRange) {
    ASSERT(m_pEngine);
  }

  ~DeleteOperation() override {}

  void Undo() const override {
    CFX_WideString prev = m_pEngine->GetText(0, -1);
    if (m_pEngine->IsSelect())
      m_pEngine->ClearSelection();

    m_pEngine->Inner_Insert(m_nIndex, m_wsRange);
    if (m_bSel)
      m_pEngine->AddSelRange(m_nIndex, m_wsRange.GetLength());

    m_pEngine->GetParams()->pEventSink->OnTextChanged(prev);
    m_pEngine->SetCaretPos(m_nCaret, true);
  }

  void Redo() const override {
    CFX_WideString prev = m_pEngine->GetText(0, -1);
    m_pEngine->Inner_DeleteRange(m_nIndex, m_wsRange.GetLength());
    if (m_bSel)
      m_pEngine->RemoveSelRange(m_nIndex, m_wsRange.GetLength());

    m_pEngine->GetParams()->pEventSink->OnTextChanged(prev);
    m_pEngine->SetCaretPos(m_nIndex, true);
  }

 private:
  CFDE_TxtEdtEngine* m_pEngine;
  bool m_bSel;
  int32_t m_nIndex;
  int32_t m_nCaret;
  CFX_WideString m_wsRange;
};

}  // namespace

FDE_TXTEDTPARAMS::FDE_TXTEDTPARAMS()
    : fPlateWidth(0),
      fPlateHeight(0),
      nLineCount(0),
      dwLayoutStyles(0),
      dwAlignment(0),
      dwMode(0),
      fFontSize(10.0f),
      dwFontColor(0xff000000),
      fLineSpace(10.0f),
      fTabWidth(36),
      pEventSink(nullptr) {}

FDE_TXTEDTPARAMS::~FDE_TXTEDTPARAMS() {}

CFDE_TxtEdtEngine::CFDE_TxtEdtEngine()
    : m_pTxtBuf(pdfium::MakeUnique<CFDE_TxtEdtBuf>()),
      m_nPageLineCount(20),
      m_nLineCount(0),
      m_nAnchorPos(-1),
      m_fCaretPosReserve(0.0),
      m_nCaret(0),
      m_nCaretPage(0),
      m_nLimit(0),
      m_wcAliasChar(L'*'),
      m_FirstLineEnding(LineEnding::kAuto),
      m_bBefore(true),
      m_bLock(false),
      m_bAutoLineEnd(true) {}

CFDE_TxtEdtEngine::~CFDE_TxtEdtEngine() {
  RemoveAllParags();
  RemoveAllPages();
  m_Param.pEventSink = nullptr;
  ClearSelection();
}

void CFDE_TxtEdtEngine::SetEditParams(const FDE_TXTEDTPARAMS& params) {
  if (!m_pTextBreak)
    m_pTextBreak = pdfium::MakeUnique<CFX_TxtBreak>();

  m_Param = params;
  m_bAutoLineEnd = true;
  UpdateTxtBreak();
}

CFDE_TxtEdtPage* CFDE_TxtEdtEngine::GetPage(int32_t nIndex) {
  if (!pdfium::IndexInBounds(m_PagePtrArray, nIndex))
    return nullptr;
  return m_PagePtrArray[nIndex].get();
}

void CFDE_TxtEdtEngine::SetText(const CFX_WideString& wsText) {
  ResetEngine();
  int32_t nLength = wsText.GetLength();
  if (nLength > 0) {
    CFX_WideString wsTemp;
    wchar_t* lpBuffer = wsTemp.GetBuffer(nLength);
    memcpy(lpBuffer, wsText.c_str(), nLength * sizeof(wchar_t));
    ReplaceParagEnd(lpBuffer, nLength, false);
    wsTemp.ReleaseBuffer(nLength);
    if (m_nLimit > 0 && nLength > m_nLimit) {
      wsTemp.Delete(m_nLimit, nLength - m_nLimit);
      nLength = m_nLimit;
    }
    m_pTxtBuf->SetText(wsTemp);
  }
  m_pTxtBuf->Insert(nLength, L"\n");
  RebuildParagraphs();
}

CFX_WideString CFDE_TxtEdtEngine::GetText(int32_t nStart,
                                          int32_t nCount) const {
  int32_t nTextBufLength = GetTextLength();
  if (nCount == -1)
    nCount = nTextBufLength - nStart;

  CFX_WideString wsText = m_pTxtBuf->GetRange(nStart, nCount);
  RecoverParagEnd(wsText);
  return wsText;
}

void CFDE_TxtEdtEngine::ClearText() {
  if (IsLocked())
    return;

  int32_t len = GetTextLength();
  if (len == 0)
    return;
  if (m_Param.dwMode & FDE_TEXTEDITMODE_Validate) {
    CFX_WideString wsText = GetPreDeleteText(0, len);
    if (!m_Param.pEventSink->OnValidate(wsText))
      return;
  }

  CFX_WideString prev = GetText(0, -1);

  DeleteRange_DoRecord(0, len, false);
  m_Param.pEventSink->OnTextChanged(prev);
  SetCaretPos(0, true);
}

int32_t CFDE_TxtEdtEngine::SetCaretPos(int32_t nIndex, bool bBefore) {
  if (IsLocked())
    return 0;

  ASSERT(nIndex >= 0 && nIndex <= GetTextLength());
  if (!pdfium::IndexInBounds(m_PagePtrArray, m_nCaretPage))
    return 0;

  m_bBefore = bBefore;
  m_nCaret = nIndex;
  MovePage2Char(m_nCaret);
  GetCaretRect(m_rtCaret, m_nCaretPage, m_nCaret, m_bBefore);
  if (!m_bBefore) {
    m_nCaret++;
    m_bBefore = true;
  }
  m_fCaretPosReserve = m_rtCaret.left;
  m_Param.pEventSink->OnCaretChanged();
  m_nAnchorPos = -1;
  return m_nCaret;
}

int32_t CFDE_TxtEdtEngine::MoveCaretPos(FDE_CaretMove eMoveCaret, bool bShift) {
  if (IsLocked() || !pdfium::IndexInBounds(m_PagePtrArray, m_nCaretPage))
    return 0;

  bool bSelChange = false;
  if (IsSelect()) {
    ClearSelection();
    bSelChange = true;
  }
  if (bShift) {
    if (m_nAnchorPos == -1)
      m_nAnchorPos = m_nCaret;
  } else {
    m_nAnchorPos = -1;
  }

  switch (eMoveCaret) {
    case FDE_CaretMove::Left: {
      bool bBefore = true;
      int32_t nIndex = MoveBackward(bBefore);
      if (nIndex >= 0)
        UpdateCaretRect(nIndex, bBefore);
      break;
    }
    case FDE_CaretMove::Right: {
      bool bBefore = true;
      int32_t nIndex = MoveForward(bBefore);
      if (nIndex >= 0)
        UpdateCaretRect(nIndex, bBefore);
      break;
    }
    case FDE_CaretMove::Up: {
      CFX_PointF ptCaret;
      if (MoveUp(ptCaret))
        UpdateCaretIndex(ptCaret);
      break;
    }
    case FDE_CaretMove::Down: {
      CFX_PointF ptCaret;
      if (MoveDown(ptCaret))
        UpdateCaretIndex(ptCaret);
      break;
    }
    case FDE_CaretMove::LineStart:
      MoveLineStart();
      break;
    case FDE_CaretMove::LineEnd:
      MoveLineEnd();
      break;
    case FDE_CaretMove::Home:
      MoveHome();
      break;
    case FDE_CaretMove::End:
      MoveEnd();
      break;
  }
  if (bShift && m_nAnchorPos != -1 && (m_nAnchorPos != m_nCaret)) {
    AddSelRange(std::min(m_nAnchorPos, m_nCaret), abs(m_nAnchorPos - m_nCaret));
    m_Param.pEventSink->OnSelChanged();
  }
  if (bSelChange)
    m_Param.pEventSink->OnSelChanged();

  return m_nCaret;
}

int32_t CFDE_TxtEdtEngine::Insert(const CFX_WideString& str) {
  if (IsLocked())
    return FDE_TXTEDT_MODIFY_RET_F_Locked;

  int32_t nLength = str.GetLength();
  CFX_WideString wsTemp;
  wchar_t* lpBuffer = wsTemp.GetBuffer(nLength);
  memcpy(lpBuffer, str.c_str(), nLength * sizeof(wchar_t));
  ReplaceParagEnd(lpBuffer, nLength, false);
  wsTemp.ReleaseBuffer(nLength);

  if (m_nLimit > 0) {
    int32_t nTotalLength = GetTextLength();
    for (const auto& lpSelRange : m_SelRangePtrArr)
      nTotalLength -= lpSelRange->nCount;

    int32_t nExpectLength = nTotalLength + nLength;
    if (nTotalLength == m_nLimit)
      return FDE_TXTEDT_MODIFY_RET_F_Full;

    if (nExpectLength > m_nLimit)
      nLength -= (nExpectLength - m_nLimit);
  }
  if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Vert) ||
      (m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Horz)) {
    if (m_Param.dwMode & FDE_TEXTEDITMODE_Password) {
      while (nLength > 0) {
        CFX_WideString wsText = GetPreInsertText(m_nCaret, lpBuffer, nLength);
        int32_t nTotal = wsText.GetLength();
        wchar_t* lpBuf = wsText.GetBuffer(nTotal);
        for (int32_t i = 0; i < nTotal; i++) {
          lpBuf[i] = m_wcAliasChar;
        }
        wsText.ReleaseBuffer(nTotal);
        if (IsFitArea(wsText)) {
          break;
        }
        nLength--;
      }
    } else {
      while (nLength > 0) {
        CFX_WideString wsText = GetPreInsertText(m_nCaret, lpBuffer, nLength);
        if (IsFitArea(wsText)) {
          break;
        }
        nLength--;
      }
    }
    if (nLength == 0)
      return FDE_TXTEDT_MODIFY_RET_F_Full;
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_Validate) {
    CFX_WideString wsText = GetPreInsertText(m_nCaret, lpBuffer, nLength);
    if (!m_Param.pEventSink->OnValidate(wsText))
      return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
  }
  if (IsSelect()) {
    DeleteSelect();
  }
  m_Param.pEventSink->OnAddDoRecord(pdfium::MakeUnique<InsertOperation>(
      this, m_nCaret, CFX_WideString(lpBuffer, nLength)));

  CFX_WideString prev = GetText(0, -1);
  Inner_Insert(m_nCaret, CFX_WideString(lpBuffer, nLength));

  int32_t nStart = m_nCaret;
  nStart += nLength;
  wchar_t wChar = m_pTxtBuf->GetCharByIndex(nStart - 1);
  bool bBefore = true;
  if (wChar != L'\n' && wChar != L'\r') {
    nStart--;
    bBefore = false;
  }
  SetCaretPos(nStart, bBefore);
  m_Param.pEventSink->OnTextChanged(prev);
  return FDE_TXTEDT_MODIFY_RET_S_Normal;
}

void CFDE_TxtEdtEngine::Delete(bool bBackspace) {
  if (IsLocked())
    return;
  if (IsSelect()) {
    DeleteSelect();
    return;
  }

  int32_t nCount = 1;
  int32_t nStart = m_nCaret;
  if (bBackspace) {
    if (nStart == 0)
      return;

    if (nStart > 2 && m_pTxtBuf->GetCharByIndex(nStart - 1) == L'\n' &&
        m_pTxtBuf->GetCharByIndex(nStart - 2) == L'\r') {
      nStart--;
      nCount++;
    }
    nStart--;
  } else {
    if (nStart == GetTextLength())
      return;

    if ((nStart + 1 < GetTextLength()) &&
        (m_pTxtBuf->GetCharByIndex(nStart) == L'\r') &&
        (m_pTxtBuf->GetCharByIndex(nStart + 1) == L'\n')) {
      nCount++;
    }
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_Validate) {
    CFX_WideString wsText = GetPreDeleteText(nStart, nCount);
    if (!m_Param.pEventSink->OnValidate(wsText))
      return;
  }

  CFX_WideString wsRange = m_pTxtBuf->GetRange(nStart, nCount);
  m_Param.pEventSink->OnAddDoRecord(pdfium::MakeUnique<DeleteOperation>(
      this, nStart, m_nCaret, wsRange, false));

  CFX_WideString prev = GetText(0, -1);
  Inner_DeleteRange(nStart, nCount);
  SetCaretPos(nStart + ((!bBackspace && nStart > 0) ? -1 : 0),
              (bBackspace || nStart == 0));
  m_Param.pEventSink->OnTextChanged(prev);
}

void CFDE_TxtEdtEngine::RemoveSelRange(int32_t nStart, int32_t nCount) {
  int32_t nRangeCount = pdfium::CollectionSize<int32_t>(m_SelRangePtrArr);
  for (int32_t i = 0; i < nRangeCount; i++) {
    FDE_TXTEDTSELRANGE* lpTemp = m_SelRangePtrArr[i].get();
    if (lpTemp->nStart == nStart && lpTemp->nCount == nCount) {
      m_SelRangePtrArr.erase(m_SelRangePtrArr.begin() + i);
      return;
    }
  }
}

void CFDE_TxtEdtEngine::AddSelRange(int32_t nStart, int32_t nCount) {
  if (nCount == -1)
    nCount = GetTextLength() - nStart;

  if (m_SelRangePtrArr.empty()) {
    auto lpSelRange = pdfium::MakeUnique<FDE_TXTEDTSELRANGE>();
    lpSelRange->nStart = nStart;
    lpSelRange->nCount = nCount;
    m_SelRangePtrArr.push_back(std::move(lpSelRange));
    m_Param.pEventSink->OnSelChanged();
    return;
  }
  auto* lpTemp = m_SelRangePtrArr.back().get();
  if (nStart >= lpTemp->nStart + lpTemp->nCount) {
    auto lpSelRange = pdfium::MakeUnique<FDE_TXTEDTSELRANGE>();
    lpSelRange->nStart = nStart;
    lpSelRange->nCount = nCount;
    m_SelRangePtrArr.push_back(std::move(lpSelRange));
    m_Param.pEventSink->OnSelChanged();
    return;
  }
  int32_t nEnd = nStart + nCount - 1;
  bool bBegin = false;
  int32_t nRangeBgn = 0;
  int32_t nRangeCnt = 0;
  for (int32_t i = 0, nSize = pdfium::CollectionSize<int32_t>(m_SelRangePtrArr);
       i < nSize; i++) {
    lpTemp = m_SelRangePtrArr[i].get();
    int32_t nTempBgn = lpTemp->nStart;
    int32_t nTempEnd = nTempBgn + lpTemp->nCount - 1;
    if (bBegin) {
      if (nEnd < nTempBgn) {
        break;
      } else if (nStart >= nTempBgn && nStart <= nTempEnd) {
        nRangeCnt++;
        break;
      }
      nRangeCnt++;
    } else {
      if (nStart <= nTempEnd) {
        nRangeBgn = i;
        if (nEnd < nTempBgn) {
          break;
        }
        nRangeCnt = 1;
        bBegin = true;
      }
    }
  }
  if (nRangeCnt == 0) {
    auto lpSelRange = pdfium::MakeUnique<FDE_TXTEDTSELRANGE>();
    lpSelRange->nStart = nStart;
    lpSelRange->nCount = nCount;
    m_SelRangePtrArr.insert(m_SelRangePtrArr.begin() + nRangeBgn,
                            std::move(lpSelRange));
  } else {
    lpTemp = m_SelRangePtrArr[nRangeBgn].get();
    lpTemp->nStart = nStart;
    lpTemp->nCount = nCount;
    nRangeCnt--;
    nRangeBgn++;
    m_SelRangePtrArr.erase(m_SelRangePtrArr.begin() + nRangeBgn,
                           m_SelRangePtrArr.begin() + nRangeBgn + nRangeCnt);
  }
  m_Param.pEventSink->OnSelChanged();
}

int32_t CFDE_TxtEdtEngine::GetSelRange(int32_t nIndex, int32_t* nStart) const {
  if (nStart)
    *nStart = m_SelRangePtrArr[nIndex]->nStart;
  return m_SelRangePtrArr[nIndex]->nCount;
}

void CFDE_TxtEdtEngine::ClearSelection() {
  if (m_SelRangePtrArr.empty())
    return;
  m_SelRangePtrArr.clear();
  if (m_Param.pEventSink)
    m_Param.pEventSink->OnSelChanged();
}

bool CFDE_TxtEdtEngine::Redo(const IFDE_TxtEdtDoRecord* pDoRecord) {
  if (IsLocked())
    return false;
  pDoRecord->Redo();
  return true;
}

bool CFDE_TxtEdtEngine::Undo(const IFDE_TxtEdtDoRecord* pDoRecord) {
  if (IsLocked())
    return false;
  pDoRecord->Undo();
  return true;
}

void CFDE_TxtEdtEngine::Layout() {
  CFX_AutoRestorer<bool> lock(&m_bLock);
  m_bLock = true;

  RemoveAllPages();
  UpdateLineCounts();
  UpdatePages();

  m_nCaret = std::min(m_nCaret, GetTextLength());
  m_rtCaret = CFX_RectF(0, 0, 1, m_Param.fFontSize);
}

int32_t CFDE_TxtEdtEngine::Line2Parag(int32_t nStartParag,
                                      int32_t nStartLineofParag,
                                      int32_t nLineIndex,
                                      int32_t& nStartLine) const {
  int32_t nLineTotal = nStartLineofParag;
  int32_t nCount = pdfium::CollectionSize<int32_t>(m_ParagPtrArray);
  CFDE_TxtEdtParag* pParag = nullptr;
  int32_t i = nStartParag;
  for (; i < nCount; i++) {
    pParag = m_ParagPtrArray[i].get();
    nLineTotal += pParag->GetLineCount();
    if (nLineTotal > nLineIndex) {
      break;
    }
  }
  nStartLine = nLineTotal - pParag->GetLineCount();
  return i;
}

CFX_WideString CFDE_TxtEdtEngine::GetPreDeleteText(int32_t nIndex,
                                                   int32_t nLength) {
  CFX_WideString wsText = GetText(0, GetTextLength());
  wsText.Delete(nIndex, nLength);
  return wsText;
}

CFX_WideString CFDE_TxtEdtEngine::GetPreInsertText(int32_t nIndex,
                                                   const wchar_t* lpText,
                                                   int32_t nLength) {
  CFX_WideString wsText = GetText(0, GetTextLength());
  int32_t nSelIndex = 0;
  int32_t nSelLength = 0;
  int32_t nSelCount = CountSelRanges();
  while (nSelCount--) {
    nSelLength = GetSelRange(nSelCount, &nSelIndex);
    wsText.Delete(nSelIndex, nSelLength);
    nIndex = nSelIndex;
  }
  CFX_WideString wsTemp;
  int32_t nOldLength = wsText.GetLength();
  const wchar_t* pOldBuffer = wsText.c_str();
  wchar_t* lpBuffer = wsTemp.GetBuffer(nOldLength + nLength);
  memcpy(lpBuffer, pOldBuffer, (nIndex) * sizeof(wchar_t));
  memcpy(lpBuffer + nIndex, lpText, nLength * sizeof(wchar_t));
  memcpy(lpBuffer + nIndex + nLength, pOldBuffer + nIndex,
         (nOldLength - nIndex) * sizeof(wchar_t));
  wsTemp.ReleaseBuffer(nOldLength + nLength);
  wsText = wsTemp;
  return wsText;
}

CFX_WideString CFDE_TxtEdtEngine::GetPreReplaceText(int32_t nIndex,
                                                    int32_t nOriginLength,
                                                    const wchar_t* lpText,
                                                    int32_t nLength) {
  CFX_WideString wsText = GetText(0, GetTextLength());
  int32_t nSelIndex = 0;
  int32_t nSelLength = 0;
  int32_t nSelCount = CountSelRanges();
  while (nSelCount--) {
    nSelLength = GetSelRange(nSelCount, &nSelIndex);
    wsText.Delete(nSelIndex, nSelLength);
  }
  wsText.Delete(nIndex, nOriginLength);
  int32_t i = 0;
  for (i = 0; i < nLength; i++)
    wsText.Insert(nIndex++, lpText[i]);

  return wsText;
}

void CFDE_TxtEdtEngine::Inner_Insert(int32_t nStart,
                                     const CFX_WideString& wsText) {
  const int32_t nLength = wsText.GetLength();
  ASSERT(nLength > 0);
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nStart, ParagPos);
  m_Param.pEventSink->OnPageUnload(m_nCaretPage);
  int32_t nParagCount = pdfium::CollectionSize<int32_t>(m_ParagPtrArray);
  for (int32_t i = ParagPos.nParagIndex + 1; i < nParagCount; i++)
    m_ParagPtrArray[i]->IncrementStartIndex(nLength);

  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex].get();
  int32_t nReserveLineCount = pParag->GetLineCount();
  int32_t nReserveCharStart = pParag->GetStartIndex();
  int32_t nLeavePart = ParagPos.nCharIndex;
  int32_t nCutPart = pParag->GetTextLength() - ParagPos.nCharIndex;
  int32_t nTextStart = 0;
  int32_t nCur = 0;
  bool bFirst = true;
  int32_t nParagIndex = ParagPos.nParagIndex;
  for (const auto& wCurChar : wsText) {
    ++nCur;
    if (wCurChar == L'\n') {
      if (bFirst) {
        pParag->SetTextLength(nLeavePart + (nCur - nTextStart + 1));
        pParag->SetLineCount(-1);
        nReserveCharStart += pParag->GetTextLength();
        bFirst = false;
      } else {
        auto pParag2 = pdfium::MakeUnique<CFDE_TxtEdtParag>(this);
        pParag2->SetLineCount(-1);
        pParag2->SetTextLength(nCur - nTextStart + 1);
        pParag2->SetStartIndex(nReserveCharStart);
        nReserveCharStart += pParag2->GetTextLength();
        m_ParagPtrArray.insert(m_ParagPtrArray.begin() + ++nParagIndex,
                               std::move(pParag2));
      }
      nTextStart = nCur + 1;
    }
  }
  if (bFirst) {
    pParag->IncrementTextLength(nLength);
    pParag->SetLineCount(-1);
    bFirst = false;
  } else {
    auto pParag2 = pdfium::MakeUnique<CFDE_TxtEdtParag>(this);
    pParag2->SetLineCount(-1);
    pParag2->SetTextLength(nLength - nTextStart + nCutPart);
    pParag2->SetStartIndex(nReserveCharStart);
    m_ParagPtrArray.insert(m_ParagPtrArray.begin() + ++nParagIndex,
                           std::move(pParag2));
  }
  m_pTxtBuf->Insert(nStart, wsText);
  int32_t nTotalLineCount = 0;
  for (int32_t i = ParagPos.nParagIndex; i <= nParagIndex; i++) {
    pParag = m_ParagPtrArray[i].get();
    pParag->CalcLines();
    nTotalLineCount += pParag->GetLineCount();
  }
  m_nLineCount += nTotalLineCount - nReserveLineCount;
  m_Param.pEventSink->OnPageLoad(m_nCaretPage);
  UpdatePages();
}

void CFDE_TxtEdtEngine::Inner_DeleteRange(int32_t nStart, int32_t nCount) {
  if (nCount == -1) {
    nCount = m_pTxtBuf->GetTextLength() - nStart;
  }
  int32_t nEnd = nStart + nCount - 1;
  ASSERT(nStart >= 0 && nEnd < m_pTxtBuf->GetTextLength());
  m_Param.pEventSink->OnPageUnload(m_nCaretPage);
  FDE_TXTEDTPARAGPOS ParagPosBgn, ParagPosEnd;
  TextPos2ParagPos(nStart, ParagPosBgn);
  TextPos2ParagPos(nEnd, ParagPosEnd);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPosEnd.nParagIndex].get();
  bool bLastParag = false;
  if (ParagPosEnd.nCharIndex == pParag->GetTextLength() - 1) {
    if (ParagPosEnd.nParagIndex <
        pdfium::CollectionSize<int32_t>(m_ParagPtrArray) - 1) {
      ParagPosEnd.nParagIndex++;
    } else {
      bLastParag = true;
    }
  }
  int32_t nTotalLineCount = 0;
  int32_t nTotalCharCount = 0;
  int32_t i = 0;
  for (i = ParagPosBgn.nParagIndex; i <= ParagPosEnd.nParagIndex; i++) {
    CFDE_TxtEdtParag* pTextParag = m_ParagPtrArray[i].get();
    pTextParag->CalcLines();
    nTotalLineCount += pTextParag->GetLineCount();
    nTotalCharCount += pTextParag->GetTextLength();
  }
  m_pTxtBuf->Delete(nStart, nCount);
  int32_t nNextParagIndex = (ParagPosBgn.nCharIndex == 0 && bLastParag)
                                ? ParagPosBgn.nParagIndex
                                : (ParagPosBgn.nParagIndex + 1);
  m_ParagPtrArray.erase(m_ParagPtrArray.begin() + nNextParagIndex,
                        m_ParagPtrArray.begin() + ParagPosEnd.nParagIndex + 1);

  if (!(bLastParag && ParagPosBgn.nCharIndex == 0)) {
    pParag = m_ParagPtrArray[ParagPosBgn.nParagIndex].get();
    pParag->SetTextLength(nTotalCharCount - nCount);
    pParag->CalcLines();
    nTotalLineCount -= pParag->GetLineCount();
  }
  int32_t nParagCount = pdfium::CollectionSize<int32_t>(m_ParagPtrArray);
  for (i = nNextParagIndex; i < nParagCount; i++)
    m_ParagPtrArray[i]->DecrementStartIndex(nCount);

  m_nLineCount -= nTotalLineCount;
  UpdatePages();
  int32_t nPageCount = CountPages();
  if (m_nCaretPage >= nPageCount) {
    m_nCaretPage = nPageCount - 1;
  }
  m_Param.pEventSink->OnPageLoad(m_nCaretPage);
}

void CFDE_TxtEdtEngine::DeleteRange_DoRecord(int32_t nStart,
                                             int32_t nCount,
                                             bool bSel) {
  ASSERT(nStart >= 0);
  if (nCount == -1) {
    nCount = GetTextLength() - nStart;
  }
  ASSERT((nStart + nCount) <= m_pTxtBuf->GetTextLength());

  CFX_WideString wsRange = m_pTxtBuf->GetRange(nStart, nCount);
  m_Param.pEventSink->OnAddDoRecord(pdfium::MakeUnique<DeleteOperation>(
      this, nStart, m_nCaret, wsRange, bSel));

  Inner_DeleteRange(nStart, nCount);
}

void CFDE_TxtEdtEngine::ResetEngine() {
  RemoveAllPages();
  RemoveAllParags();
  ClearSelection();
  m_nCaret = 0;
  m_pTxtBuf->Clear();
  m_nCaret = 0;
}

void CFDE_TxtEdtEngine::RebuildParagraphs() {
  RemoveAllParags();
  wchar_t wChar = L' ';
  int32_t nParagStart = 0;
  int32_t nIndex = 0;
  auto pIter = pdfium::MakeUnique<CFDE_TxtEdtBuf::Iterator>(m_pTxtBuf.get(), 0);
  pIter->SetAt(0);
  do {
    wChar = pIter->GetChar();
    nIndex = pIter->GetAt();
    if (wChar == L'\n') {
      auto pParag = pdfium::MakeUnique<CFDE_TxtEdtParag>(this);
      pParag->SetStartIndex(nParagStart);
      pParag->SetTextLength(nIndex - nParagStart + 1);
      pParag->SetLineCount(-1);
      m_ParagPtrArray.push_back(std::move(pParag));
      nParagStart = nIndex + 1;
    }
  } while (pIter->Next());
}

void CFDE_TxtEdtEngine::UpdateLineCounts() {
  if (m_ParagPtrArray.empty())
    return;

  int32_t nLineCount = 0;
  for (auto& pParag : m_ParagPtrArray) {
    pParag->CalcLines();
    nLineCount += pParag->GetLineCount();
  }
  m_nLineCount = nLineCount;
}

void CFDE_TxtEdtEngine::UpdatePages() {
  if (m_nLineCount == 0)
    return;

  int32_t nPageCount = (m_nLineCount - 1) / (m_nPageLineCount) + 1;
  int32_t nSize = pdfium::CollectionSize<int32_t>(m_PagePtrArray);
  if (nSize == nPageCount)
    return;

  if (nSize > nPageCount) {
    m_PagePtrArray.erase(m_PagePtrArray.begin() + nPageCount,
                         m_PagePtrArray.end());
    return;
  }
  for (int32_t i = nSize; i < nPageCount; i++)
    m_PagePtrArray.push_back(pdfium::MakeUnique<CFDE_TxtEdtPage>(this, i));
}

void CFDE_TxtEdtEngine::UpdateTxtBreak() {
  uint32_t dwStyle = m_pTextBreak->GetLayoutStyles();
  if (m_Param.dwMode & FDE_TEXTEDITMODE_MultiLines)
    dwStyle &= ~FX_LAYOUTSTYLE_SingleLine;
  else
    dwStyle |= FX_LAYOUTSTYLE_SingleLine;

  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_CombText)
    dwStyle |= FX_LAYOUTSTYLE_CombText;
  else
    dwStyle &= ~FX_LAYOUTSTYLE_CombText;

  m_pTextBreak->SetLayoutStyles(dwStyle);
  uint32_t dwAligment = 0;
  if (m_Param.dwAlignment & FDE_TEXTEDITALIGN_Justified)
    dwAligment |= CFX_TxtLineAlignment_Justified;

  if (m_Param.dwAlignment & FDE_TEXTEDITALIGN_Center)
    dwAligment |= CFX_TxtLineAlignment_Center;
  else if (m_Param.dwAlignment & FDE_TEXTEDITALIGN_Right)
    dwAligment |= CFX_TxtLineAlignment_Right;

  m_pTextBreak->SetAlignment(dwAligment);

  if (m_Param.dwMode & FDE_TEXTEDITMODE_AutoLineWrap)
    m_pTextBreak->SetLineWidth(m_Param.fPlateWidth);
  else
    m_pTextBreak->SetLineWidth(kPageWidthMax);

  m_nPageLineCount = m_Param.nLineCount;
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_CombText) {
    float fCombWidth = m_Param.fPlateWidth;
    if (m_nLimit > 0)
      fCombWidth /= m_nLimit;

    m_pTextBreak->SetCombWidth(fCombWidth);
  }
  m_pTextBreak->SetFont(m_Param.pFont);
  m_pTextBreak->SetFontSize(m_Param.fFontSize);
  m_pTextBreak->SetTabWidth(m_Param.fTabWidth);
  m_pTextBreak->SetDefaultChar(0xFEFF);
  m_pTextBreak->SetParagraphBreakChar(L'\n');
  m_pTextBreak->SetLineBreakTolerance(m_Param.fFontSize * 0.2f);
  m_pTextBreak->SetHorizontalScale(100);
  m_pTextBreak->SetCharSpace(0);
}

bool CFDE_TxtEdtEngine::ReplaceParagEnd(wchar_t*& lpText,
                                        int32_t& nLength,
                                        bool bPreIsCR) {
  for (int32_t i = 0; i < nLength; i++) {
    wchar_t wc = lpText[i];
    switch (wc) {
      case L'\r': {
        lpText[i] = L'\n';
        bPreIsCR = true;
      } break;
      case L'\n': {
        if (bPreIsCR == true) {
          int32_t nNext = i + 1;
          if (nNext < nLength) {
            memmove(lpText + i, lpText + nNext,
                    (nLength - nNext) * sizeof(wchar_t));
          }
          i--;
          nLength--;
          bPreIsCR = false;
          if (m_bAutoLineEnd) {
            m_FirstLineEnding = LineEnding::kCRLF;
            m_bAutoLineEnd = false;
          }
        } else {
          lpText[i] = L'\n';
          if (m_bAutoLineEnd) {
            m_FirstLineEnding = LineEnding::kLF;
            m_bAutoLineEnd = false;
          }
        }
      } break;
      default: {
        if (bPreIsCR && m_bAutoLineEnd) {
          m_FirstLineEnding = LineEnding::kCR;
          m_bAutoLineEnd = false;
        }
        bPreIsCR = false;
      } break;
    }
  }
  return bPreIsCR;
}

void CFDE_TxtEdtEngine::RecoverParagEnd(CFX_WideString& wsText) const {
  if (m_FirstLineEnding == LineEnding::kCR)
    return;

  if (m_FirstLineEnding == LineEnding::kCRLF) {
    std::vector<int32_t> PosArr;
    int32_t nLength = wsText.GetLength();
    wchar_t* lpPos = const_cast<wchar_t*>(wsText.c_str());
    for (int32_t i = 0; i < nLength; i++, lpPos++) {
      if (*lpPos == L'\n') {
        *lpPos = L'\r';
        PosArr.push_back(i);
      }
    }
    const wchar_t* lpSrcBuf = wsText.c_str();
    CFX_WideString wsTemp;
    int32_t nCount = pdfium::CollectionSize<int32_t>(PosArr);
    wchar_t* lpDstBuf = wsTemp.GetBuffer(nLength + nCount);
    int32_t nDstPos = 0;
    int32_t nSrcPos = 0;
    for (int32_t i = 0; i < nCount; i++) {
      int32_t nPos = PosArr[i];
      int32_t nCopyLen = nPos - nSrcPos + 1;
      memcpy(lpDstBuf + nDstPos, lpSrcBuf + nSrcPos,
             nCopyLen * sizeof(wchar_t));
      nDstPos += nCopyLen;
      nSrcPos += nCopyLen;
      lpDstBuf[nDstPos] = L'\n';
      nDstPos++;
    }
    if (nSrcPos < nLength) {
      memcpy(lpDstBuf + nDstPos, lpSrcBuf + nSrcPos,
             (nLength - nSrcPos) * sizeof(wchar_t));
    }
    wsTemp.ReleaseBuffer(nLength + nCount);
    wsText = wsTemp;
    return;
  }

  int32_t nLength = wsText.GetLength();
  wchar_t* lpBuf = const_cast<wchar_t*>(wsText.c_str());
  for (int32_t i = 0; i < nLength; i++, lpBuf++) {
    if (*lpBuf == L'\n')
      *lpBuf = L'\r';
  }
}

int32_t CFDE_TxtEdtEngine::MovePage2Char(int32_t nIndex) {
  ASSERT(nIndex >= 0);
  ASSERT(nIndex <= m_pTxtBuf->GetTextLength());
  if (m_nCaretPage >= 0) {
    CFDE_TxtEdtPage* pPage = m_PagePtrArray[m_nCaretPage].get();
    m_Param.pEventSink->OnPageLoad(m_nCaretPage);
    int32_t nPageCharStart = pPage->GetCharStart();
    int32_t nPageCharCount = pPage->GetCharCount();
    if (nIndex >= nPageCharStart && nIndex < nPageCharStart + nPageCharCount) {
      m_Param.pEventSink->OnPageUnload(m_nCaretPage);
      return m_nCaretPage;
    }
    m_Param.pEventSink->OnPageUnload(m_nCaretPage);
  }
  CFDE_TxtEdtParag* pParag = nullptr;
  int32_t nLineCount = 0;
  int32_t nParagCount = pdfium::CollectionSize<int32_t>(m_ParagPtrArray);
  int32_t i = 0;
  for (i = 0; i < nParagCount; i++) {
    pParag = m_ParagPtrArray[i].get();
    if (pParag->GetStartIndex() <= nIndex &&
        nIndex < (pParag->GetStartIndex() + pParag->GetTextLength())) {
      break;
    }
    nLineCount += pParag->GetLineCount();
  }
  pParag->LoadParag();
  int32_t nLineStart = -1;
  int32_t nLineCharCount = -1;
  for (i = 0; i < pParag->GetLineCount(); i++) {
    pParag->GetLineRange(i, nLineStart, nLineCharCount);
    if (nLineStart <= nIndex && nIndex < (nLineStart + nLineCharCount))
      break;
  }
  ASSERT(i < pParag->GetLineCount());
  nLineCount += (i + 1);
  m_nCaretPage = (nLineCount - 1) / m_nPageLineCount + 1 - 1;
  pParag->UnloadParag();
  return m_nCaretPage;
}

void CFDE_TxtEdtEngine::TextPos2ParagPos(int32_t nIndex,
                                         FDE_TXTEDTPARAGPOS& ParagPos) const {
  ASSERT(nIndex >= 0 && nIndex < m_pTxtBuf->GetTextLength());
  int32_t nCount = pdfium::CollectionSize<int32_t>(m_ParagPtrArray);
  int32_t nBgn = 0;
  int32_t nMid = 0;
  int32_t nEnd = nCount - 1;
  while (nEnd > nBgn) {
    nMid = (nBgn + nEnd) / 2;
    CFDE_TxtEdtParag* pParag = m_ParagPtrArray[nMid].get();
    if (nIndex < pParag->GetStartIndex())
      nEnd = nMid - 1;
    else if (nIndex >= (pParag->GetStartIndex() + pParag->GetTextLength()))
      nBgn = nMid + 1;
    else
      break;
  }
  if (nBgn == nEnd)
    nMid = nBgn;

  ASSERT(nIndex >= m_ParagPtrArray[nMid]->GetStartIndex() &&
         (nIndex < m_ParagPtrArray[nMid]->GetStartIndex() +
                       m_ParagPtrArray[nMid]->GetTextLength()));
  ParagPos.nParagIndex = nMid;
  ParagPos.nCharIndex = nIndex - m_ParagPtrArray[nMid]->GetStartIndex();
}

int32_t CFDE_TxtEdtEngine::MoveForward(bool& bBefore) {
  if (m_nCaret == m_pTxtBuf->GetTextLength() - 1)
    return -1;

  int32_t nCaret = m_nCaret;
  if ((nCaret + 1 < m_pTxtBuf->GetTextLength()) &&
      (m_pTxtBuf->GetCharByIndex(nCaret) == L'\r') &&
      (m_pTxtBuf->GetCharByIndex(nCaret + 1) == L'\n')) {
    nCaret++;
  }
  nCaret++;
  bBefore = true;
  return nCaret;
}

int32_t CFDE_TxtEdtEngine::MoveBackward(bool& bBefore) {
  if (m_nCaret == 0)
    return false;

  int32_t nCaret = m_nCaret;
  if (nCaret > 2 && m_pTxtBuf->GetCharByIndex(nCaret - 1) == L'\n' &&
      m_pTxtBuf->GetCharByIndex(nCaret - 2) == L'\r') {
    nCaret--;
  }
  nCaret--;
  bBefore = true;
  return nCaret;
}

bool CFDE_TxtEdtEngine::MoveUp(CFX_PointF& ptCaret) {
  CFDE_TxtEdtPage* pPage = GetPage(m_nCaretPage);
  const CFX_RectF& rtContent = pPage->GetContentsBox();
  ptCaret.x = m_fCaretPosReserve;
  ptCaret.y = m_rtCaret.top + m_rtCaret.height / 2 - m_Param.fLineSpace;
  if (ptCaret.y < rtContent.top) {
    if (m_nCaretPage == 0) {
      return false;
    }
    ptCaret.y -= rtContent.top;
    m_nCaretPage--;
    CFDE_TxtEdtPage* pCurPage = GetPage(m_nCaretPage);
    ptCaret.y += pCurPage->GetContentsBox().bottom();
  }

  return true;
}

bool CFDE_TxtEdtEngine::MoveDown(CFX_PointF& ptCaret) {
  CFDE_TxtEdtPage* pPage = GetPage(m_nCaretPage);
  const CFX_RectF& rtContent = pPage->GetContentsBox();
  ptCaret.x = m_fCaretPosReserve;
  ptCaret.y = m_rtCaret.top + m_rtCaret.height / 2 + m_Param.fLineSpace;
  if (ptCaret.y >= rtContent.bottom()) {
    if (m_nCaretPage == CountPages() - 1) {
      return false;
    }
    ptCaret.y -= rtContent.bottom();
    m_nCaretPage++;
    CFDE_TxtEdtPage* pCurPage = GetPage(m_nCaretPage);
    ptCaret.y += pCurPage->GetContentsBox().top;
  }
  return true;
}

bool CFDE_TxtEdtEngine::MoveLineStart() {
  int32_t nIndex = m_bBefore ? m_nCaret : m_nCaret - 1;
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nIndex, ParagPos);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex].get();
  pParag->LoadParag();
  int32_t nLineCount = pParag->GetLineCount();
  int32_t i = 0;
  int32_t nStart = 0;
  int32_t nCount = 0;
  for (; i < nLineCount; i++) {
    pParag->GetLineRange(i, nStart, nCount);
    if (nIndex >= nStart && nIndex < nStart + nCount) {
      break;
    }
  }
  UpdateCaretRect(nStart, true);
  pParag->UnloadParag();
  return true;
}

bool CFDE_TxtEdtEngine::MoveLineEnd() {
  int32_t nIndex = m_bBefore ? m_nCaret : m_nCaret - 1;
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nIndex, ParagPos);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex].get();
  pParag->LoadParag();
  int32_t nLineCount = pParag->GetLineCount();
  int32_t i = 0;
  int32_t nStart = 0;
  int32_t nCount = 0;
  for (; i < nLineCount; i++) {
    pParag->GetLineRange(i, nStart, nCount);
    if (nIndex >= nStart && nIndex < nStart + nCount) {
      break;
    }
  }
  nIndex = nStart + nCount - 1;
  ASSERT(nIndex <= GetTextLength());
  wchar_t wChar = m_pTxtBuf->GetCharByIndex(nIndex);
  bool bBefore = false;
  if (nIndex <= GetTextLength()) {
    if (wChar == L'\r') {
      bBefore = true;
    } else if (wChar == L'\n' && nIndex > nStart) {
      bBefore = true;
      nIndex--;
      wChar = m_pTxtBuf->GetCharByIndex(nIndex);
      if (wChar != L'\r') {
        nIndex++;
      }
    }
  }
  UpdateCaretRect(nIndex, bBefore);
  pParag->UnloadParag();
  return true;
}

bool CFDE_TxtEdtEngine::MoveHome() {
  UpdateCaretRect(0, true);
  return true;
}

bool CFDE_TxtEdtEngine::MoveEnd() {
  UpdateCaretRect(GetTextLength(), true);
  return true;
}

bool CFDE_TxtEdtEngine::IsFitArea(CFX_WideString& wsText) {
  auto pTextOut = pdfium::MakeUnique<CFDE_TextOut>();
  pTextOut->SetLineSpace(m_Param.fLineSpace);
  pTextOut->SetFont(m_Param.pFont);
  pTextOut->SetFontSize(m_Param.fFontSize);

  FDE_TextStyle dwStyle;
  if (!(m_Param.dwMode & FDE_TEXTEDITMODE_MultiLines))
    dwStyle.single_line_ = true;

  CFX_RectF rcText;
  if (m_Param.dwMode & FDE_TEXTEDITMODE_AutoLineWrap) {
    dwStyle.line_wrap_ = true;
    rcText.width = m_Param.fPlateWidth;
  } else {
    rcText.width = 65535;
  }
  pTextOut->SetStyles(dwStyle);
  wsText += L"\n";
  pTextOut->CalcLogicSize(wsText, rcText);
  wsText.Delete(wsText.GetLength() - 1);
  if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Horz) &&
      (rcText.width > m_Param.fPlateWidth)) {
    return false;
  }
  if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Vert) &&
      (rcText.height > m_Param.fLineSpace * m_Param.nLineCount)) {
    return false;
  }
  return true;
}

void CFDE_TxtEdtEngine::UpdateCaretRect(int32_t nIndex, bool bBefore) {
  MovePage2Char(nIndex);
  GetCaretRect(m_rtCaret, m_nCaretPage, nIndex, bBefore);
  m_nCaret = nIndex;
  m_bBefore = bBefore;
  if (!m_bBefore) {
    m_nCaret++;
    m_bBefore = true;
  }
  m_fCaretPosReserve = m_rtCaret.left;
  m_Param.pEventSink->OnCaretChanged();
}

void CFDE_TxtEdtEngine::GetCaretRect(CFX_RectF& rtCaret,
                                     int32_t nPageIndex,
                                     int32_t nCaret,
                                     bool bBefore) {
  CFDE_TxtEdtPage* pPage = m_PagePtrArray[m_nCaretPage].get();
  m_Param.pEventSink->OnPageLoad(m_nCaretPage);

  bool bCombText = !!(m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_CombText);
  int32_t nIndexInpage = nCaret - pPage->GetCharStart();
  if (bBefore && bCombText && nIndexInpage > 0) {
    nIndexInpage--;
    bBefore = false;
  }
  int32_t nBIDILevel = pPage->GetCharRect(nIndexInpage, rtCaret, bCombText);
  if ((!FX_IsOdd(nBIDILevel) && !bBefore) ||
      (FX_IsOdd(nBIDILevel) && bBefore)) {
    rtCaret.Offset(rtCaret.width - 1.0f, 0);
  }
  if (rtCaret.width == 0 && rtCaret.left > 1.0f)
    rtCaret.left -= 1.0f;

  rtCaret.width = 1.0f;

  m_Param.pEventSink->OnPageUnload(m_nCaretPage);
}

void CFDE_TxtEdtEngine::UpdateCaretIndex(const CFX_PointF& ptCaret) {
  CFDE_TxtEdtPage* pPage = m_PagePtrArray[m_nCaretPage].get();
  m_Param.pEventSink->OnPageLoad(m_nCaretPage);
  m_nCaret = pPage->GetCharIndex(ptCaret, m_bBefore);
  GetCaretRect(m_rtCaret, m_nCaretPage, m_nCaret, m_bBefore);
  if (!m_bBefore) {
    m_nCaret++;
    m_bBefore = true;
  }
  m_Param.pEventSink->OnCaretChanged();
  m_Param.pEventSink->OnPageUnload(m_nCaretPage);
}

void CFDE_TxtEdtEngine::DeleteSelect() {
  int32_t nCountRange = CountSelRanges();
  if (nCountRange <= 0)
    return;

  CFX_WideString prev = GetText(0, -1);

  int32_t nSelStart = 0;
  while (nCountRange > 0) {
    int32_t nSelCount = GetSelRange(--nCountRange, &nSelStart);
    m_SelRangePtrArr.erase(m_SelRangePtrArr.begin() + nCountRange);
    DeleteRange_DoRecord(nSelStart, nSelCount, true);
  }
  ClearSelection();
  m_Param.pEventSink->OnTextChanged(prev);
  m_Param.pEventSink->OnSelChanged();
  SetCaretPos(nSelStart, true);
}
