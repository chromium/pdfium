// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "xfa/src/fee/include/ifde_txtedtengine.h"
#include "xfa/src/fee/include/ifde_txtedtbuf.h"
#include "xfa/src/fee/include/ifde_txtedtpage.h"
#include "fde_txtedtengine.h"
#include "fde_txtedtparag.h"
#include "fde_txtedtbuf.h"
#ifdef FDE_USEFORMATBLOCK
#include "fde_txtedtblock.h"
#endif
#define FDE_PAGEWIDTH_MAX 0xFFFF
#define FDE_TXTPLATESIZE (1024 * 12)
#define FDE_UNICODE_PARAGRAPH_SPERATOR (0x2029)
#define FDE_TXTEDT_DORECORD_INS 0
#define FDE_TXTEDT_DORECORD_DEL 1
#ifdef FDE_USEFORMATBLOCK
#define FDE_TXTEDT_DORECORD_FORMATINS 3
#define FDE_TXTEDT_DORECORD_FORMATDEL 4
#define FDE_TXTEDT_DORECORD_FORMATREP 5
#define FDE_TXTEDT_FORMATBLOCK_BGN 0xFFF9
#define FDE_TXTEDT_FORMATBLOCK_END 0xFFFB
#endif
IFDE_TxtEdtEngine* IFDE_TxtEdtEngine::Create() {
  return new CFDE_TxtEdtEngine();
}
CFDE_TxtEdtEngine::CFDE_TxtEdtEngine()
    : m_pTextBreak(nullptr),
      m_nPageLineCount(20),
      m_nLineCount(0),
      m_nAnchorPos(-1),
      m_nLayoutPos(0),
      m_fCaretPosReserve(0.0),
      m_nCaret(0),
      m_bBefore(TRUE),
      m_nCaretPage(0),
      m_dwFindFlags(0),
      m_bLock(FALSE),
      m_nLimit(0),
      m_wcAliasChar(L'*'),
#ifdef FDE_USEFORMATBLOCK
      m_nFixLength(-1),  // FIXME: no such member => USEFORMATBLOCK can't work.
#endif
      m_nFirstLineEnd(FDE_TXTEDIT_LINEEND_Auto),
      m_bAutoLineEnd(TRUE),
      m_wLineEnd(FDE_UNICODE_PARAGRAPH_SPERATOR) {
  FXSYS_memset(&m_rtCaret, 0, sizeof(CFX_RectF));
  m_pTxtBuf = new CFDE_TxtEdtBuf();
  m_bAutoLineEnd = (m_Param.nLineEnd == FDE_TXTEDIT_LINEEND_Auto);
}
CFDE_TxtEdtEngine::~CFDE_TxtEdtEngine() {
  if (m_pTxtBuf) {
    m_pTxtBuf->Release();
    m_pTxtBuf = NULL;
  }
  if (m_pTextBreak) {
    m_pTextBreak->Release();
    m_pTextBreak = NULL;
  }
#ifdef FDE_USEFORMATBLOCK
  int32_t nBlockCount = m_BlockArray.GetSize();
  if (nBlockCount > 0) {
    int32_t i = 0;
    for (; i < nBlockCount; i++) {
      CFDE_TxtEdtBlock* pBlock = m_BlockArray[i];
      delete pBlock;
    }
    m_BlockArray.RemoveAll();
  }
#endif
  RemoveAllParags();
  RemoveAllPages();
  m_Param.pEventSink = NULL;
  ClearSelection();
}
void CFDE_TxtEdtEngine::Release() {
  delete this;
}
void CFDE_TxtEdtEngine::SetEditParams(const FDE_TXTEDTPARAMS& params) {
  if (m_pTextBreak == NULL) {
    m_pTextBreak = IFX_TxtBreak::Create(FX_TXTBREAKPOLICY_None);
  }
  FXSYS_memcpy(&m_Param, &params, sizeof(FDE_TXTEDTPARAMS));
  m_wLineEnd = params.wLineBreakChar;
  m_bAutoLineEnd = (m_Param.nLineEnd == FDE_TXTEDIT_LINEEND_Auto);
  UpdateTxtBreak();
}
const FDE_TXTEDTPARAMS* CFDE_TxtEdtEngine::GetEditParams() const {
  return &m_Param;
}
int32_t CFDE_TxtEdtEngine::CountPages() const {
  if (m_nLineCount == 0) {
    return 0;
  }
  return ((m_nLineCount - 1) / m_nPageLineCount) + 1;
}
IFDE_TxtEdtPage* CFDE_TxtEdtEngine::GetPage(int32_t nIndex) {
  if (m_PagePtrArray.GetSize() <= nIndex) {
    return NULL;
  }
  return (IFDE_TxtEdtPage*)m_PagePtrArray[nIndex];
}
FX_BOOL CFDE_TxtEdtEngine::SetBufChunkSize(int32_t nChunkSize) {
  return m_pTxtBuf->SetChunkSize(nChunkSize);
}
void CFDE_TxtEdtEngine::SetTextByStream(IFX_Stream* pStream) {
  ResetEngine();
  int32_t nIndex = 0;
  if (pStream != NULL && pStream->GetLength()) {
    int32_t nStreamLength = pStream->GetLength();
    FX_BOOL bValid = TRUE;
    if (m_nLimit > 0 && nStreamLength > m_nLimit) {
      bValid = FALSE;
    }
    FX_BOOL bPreIsCR = FALSE;
    if (bValid) {
      uint8_t bom[4];
      int32_t nPos = pStream->GetBOM(bom);
      pStream->Seek(FX_STREAMSEEK_Begin, nPos);
      int32_t nPlateSize = std::min(nStreamLength, m_pTxtBuf->GetChunkSize());
      FX_WCHAR* lpwstr = FX_Alloc(FX_WCHAR, nPlateSize);
      FX_BOOL bEos = false;
      while (!bEos) {
        int32_t nRead = pStream->ReadString(lpwstr, nPlateSize, bEos);
        bPreIsCR = ReplaceParagEnd(lpwstr, nRead, bPreIsCR);
        m_pTxtBuf->Insert(nIndex, lpwstr, nRead);
        nIndex += nRead;
      }
      FX_Free(lpwstr);
    }
  }
  m_pTxtBuf->Insert(nIndex, &m_wLineEnd, 1);
  RebuildParagraphs();
}
void CFDE_TxtEdtEngine::SetText(const CFX_WideString& wsText) {
  ResetEngine();
  int32_t nLength = wsText.GetLength();
  if (nLength > 0) {
    CFX_WideString wsTemp;
    FX_WCHAR* lpBuffer = wsTemp.GetBuffer(nLength);
    FXSYS_memcpy(lpBuffer, wsText.c_str(), nLength * sizeof(FX_WCHAR));
    ReplaceParagEnd(lpBuffer, nLength, FALSE);
    wsTemp.ReleaseBuffer(nLength);
    if (m_nLimit > 0 && nLength > m_nLimit) {
      wsTemp.Delete(m_nLimit, nLength - m_nLimit);
      nLength = m_nLimit;
    }
    m_pTxtBuf->SetText(wsTemp);
  }
  m_pTxtBuf->Insert(nLength, &m_wLineEnd, 1);
  RebuildParagraphs();
}
int32_t CFDE_TxtEdtEngine::GetTextLength() const {
  return GetTextBufLength();
}
void CFDE_TxtEdtEngine::GetText(CFX_WideString& wsText,
                                int32_t nStart,
                                int32_t nCount) {
  int32_t nTextBufLength = GetTextBufLength();
  if (nCount == -1) {
    nCount = nTextBufLength - nStart;
  }
#ifdef FDE_USEFORMATBLOCK
  int32_t nBlockCount = m_BlockArray.GetSize();
  if (nBlockCount == 0 || m_wsFixText.IsEmpty()) {
    m_pTxtBuf->GetRange(wsText, nStart, nCount);
    return;
  }
  CFX_WideString wsTemp;
  const FX_WCHAR* lpFixBuffer = const FX_WCHAR * (m_wsFixText);
  FX_WCHAR* lpBuffer = wsTemp.GetBuffer(nTextBufLength);
  int32_t nRealLength = 0;
  int32_t nPrePos = 0;
  for (int32_t i = 0; i < nBlockCount; i++) {
    CFDE_TxtEdtBlock* pBlock = m_BlockArray[i];
    int32_t nPos = pBlock->GetPos();
    int32_t nCopyLength = nPos - nPrePos;
    FXSYS_memcpy(lpBuffer + nRealLength, lpFixBuffer + nPrePos,
                 nCopyLength * sizeof(FX_WCHAR));
    nRealLength += nCopyLength;
    nPrePos = nPos;
    CFX_WideString wsBlock;
    pBlock->GetRealText(wsBlock);
    nCopyLength = wsBlock.GetLength();
    FXSYS_memcpy(lpBuffer + nRealLength, const FX_WCHAR*(wsBlock),
                 nCopyLength * sizeof(FX_WCHAR));
    nRealLength += nCopyLength;
  }
  int32_t nLeftLength = m_wsFixText.GetLength() - nPrePos;
  if (nLeftLength > 0) {
    FXSYS_memcpy(lpBuffer + nRealLength, lpFixBuffer + nPrePos,
                 nLeftLength * sizeof(FX_WCHAR));
    nRealLength += nLeftLength;
  }
  wsTemp.ReleaseBuffer(nRealLength);
  int32_t nRealBgn = GetRealIndex(nStart);
  int32_t nRealEnd = GetRealIndex(nStart + nCount - 1);
  int32_t nRealCount = nRealEnd - nRealBgn;
  FX_WCHAR* lpDestBuf = wsText.GetBuffer(nRealCount);
  FXSYS_memcpy(lpDestBuf, const FX_WCHAR*(wsTemp) + nRealBgn,
               nRealCount * sizeof(FX_WCHAR));
  wsText.ReleaseBuffer();
#else
  m_pTxtBuf->GetRange(wsText, nStart, nCount);
  RecoverParagEnd(wsText);
  return;
#endif
}
void CFDE_TxtEdtEngine::ClearText() {
  DeleteRange(0, -1);
}
int32_t CFDE_TxtEdtEngine::GetCaretRect(CFX_RectF& rtCaret) const {
  rtCaret = m_rtCaret;
  return m_nCaret;
}
int32_t CFDE_TxtEdtEngine::GetCaretPos() const {
  if (IsLocked()) {
    return 0;
  }
  return m_nCaret + (m_bBefore ? 0 : 1);
}
int32_t CFDE_TxtEdtEngine::SetCaretPos(int32_t nIndex, FX_BOOL bBefore) {
  if (IsLocked()) {
    return 0;
  }
  FXSYS_assert(nIndex >= 0 && nIndex <= GetTextBufLength());
  if (m_PagePtrArray.GetSize() <= m_nCaretPage) {
    return 0;
  }
#ifdef FDE_USEFORMATBLOCK
  if (m_BlockArray.GetSize() > 0) {
    nIndex = NormalizeCaretPos(nIndex, FDE_FORMAT_CARET_MIDDLE, bBefore);
  }
#endif
  m_bBefore = bBefore;
  m_nCaret = nIndex;
  MovePage2Char(m_nCaret);
  GetCaretRect(m_rtCaret, m_nCaretPage, m_nCaret, m_bBefore);
  if (!m_bBefore) {
    m_nCaret++;
    m_bBefore = TRUE;
  }
  m_fCaretPosReserve = (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical)
                           ? m_rtCaret.top
                           : m_rtCaret.left;
  m_Param.pEventSink->On_CaretChanged(this, m_nCaretPage, 0);
  m_nAnchorPos = -1;
  return m_nCaret;
}
int32_t CFDE_TxtEdtEngine::MoveCaretPos(FDE_TXTEDTMOVECARET eMoveCaret,
                                        FX_BOOL bShift,
                                        FX_BOOL bCtrl) {
  if (IsLocked()) {
    return 0;
  }
  if (m_PagePtrArray.GetSize() <= m_nCaretPage) {
    return 0;
  }
  FX_BOOL bSelChange = FALSE;
  if (IsSelect()) {
    ClearSelection();
    bSelChange = TRUE;
  }
  if (bShift) {
    if (m_nAnchorPos == -1) {
      m_nAnchorPos = m_nCaret;
    }
  } else {
    m_nAnchorPos = -1;
  }
  FX_BOOL bVertical = m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical;
  switch (eMoveCaret) {
    case MC_Left: {
      if (bVertical) {
        CFX_PointF ptCaret;
        if (MoveUp(ptCaret)) {
          UpdateCaretIndex(ptCaret);
        }
      } else {
        FX_BOOL bBefore = TRUE;
        int32_t nIndex = MoveBackward(bBefore);
#ifdef FDE_USEFORMATBLOCK
        if (m_BlockArray.GetSize()) {
          nIndex =
              NormalizeCaretPos(nIndex, FDE_FORMAT_CARET_BACKWARD, bBefore);
          if (nIndex < 0) {
            return m_nCaret;
          }
        }
#endif
        if (nIndex >= 0) {
          UpdateCaretRect(nIndex, bBefore);
        }
      }
    } break;
    case MC_Right: {
      if (bVertical) {
        CFX_PointF ptCaret;
        if (MoveDown(ptCaret)) {
          UpdateCaretIndex(ptCaret);
        }
      } else {
        FX_BOOL bBefore = TRUE;
        int32_t nIndex = MoveForward(bBefore);
#ifdef FDE_USEFORMATBLOCK
        if (m_BlockArray.GetSize()) {
          if (nIndex == -1) {
            nIndex = GetTextBufLength();
          }
          nIndex = NormalizeCaretPos(nIndex, FDE_FORMAT_CARET_FORWARD, bBefore);
        }
#endif
        if (nIndex >= 0) {
          UpdateCaretRect(nIndex, bBefore);
        }
      }
    } break;
    case MC_Up: {
      if (bVertical) {
        FX_BOOL bBefore = TRUE;
        int32_t nIndex = MoveBackward(bBefore);
#ifdef FDE_USEFORMATBLOCK
        if (m_BlockArray.GetSize()) {
          nIndex =
              NormalizeCaretPos(nIndex, FDE_FORMAT_CARET_BACKWARD, bBefore);
        }
#endif
        if (nIndex >= 0) {
          UpdateCaretRect(nIndex, bBefore);
        }
      } else {
        CFX_PointF ptCaret;
        if (MoveUp(ptCaret)) {
          UpdateCaretIndex(ptCaret);
        }
      }
    } break;
    case MC_Down: {
      if (bVertical) {
        FX_BOOL bBefore = TRUE;
        int32_t nIndex = MoveForward(bBefore);
#ifdef FDE_USEFORMATBLOCK
        if (m_BlockArray.GetSize()) {
          nIndex = NormalizeCaretPos(nIndex, FDE_FORMAT_CARET_FORWARD, bBefore);
        }
#endif
        if (nIndex >= 0) {
          UpdateCaretRect(nIndex, bBefore);
        }
      } else {
        CFX_PointF ptCaret;
        if (MoveDown(ptCaret)) {
          UpdateCaretIndex(ptCaret);
        }
      }
    } break;
    case MC_WordBackward:
      break;
    case MC_WordForward:
      break;
    case MC_LineStart:
      MoveLineStart();
      break;
    case MC_LineEnd:
      MoveLineEnd();
      break;
    case MC_ParagStart:
      MoveParagStart();
      break;
    case MC_ParagEnd:
      MoveParagEnd();
      break;
    case MC_PageDown:
      break;
    case MC_PageUp:
      break;
    case MC_Home:
      MoveHome();
      break;
    case MC_End:
      MoveEnd();
      break;
    default:
      break;
  }
  if (bShift && m_nAnchorPos != -1 && (m_nAnchorPos != m_nCaret)) {
    AddSelRange(std::min(m_nAnchorPos, m_nCaret),
                FXSYS_abs(m_nAnchorPos - m_nCaret));
    m_Param.pEventSink->On_SelChanged(this);
  }
  if (bSelChange) {
    m_Param.pEventSink->On_SelChanged(this);
  }
  return m_nCaret;
}
void CFDE_TxtEdtEngine::Lock() {
  m_bLock = TRUE;
}
void CFDE_TxtEdtEngine::Unlock() {
  m_bLock = FALSE;
}
FX_BOOL CFDE_TxtEdtEngine::IsLocked() const {
  return m_bLock;
}
int32_t CFDE_TxtEdtEngine::Insert(int32_t nStart,
                                  const FX_WCHAR* lpText,
                                  int32_t nLength) {
  if (IsLocked()) {
    return FDE_TXTEDT_MODIFY_RET_F_Locked;
  }
#ifdef FDE_USEFORMATBLOCK
  int32_t nBlockCount = m_BlockArray.GetSize();
  if (nBlockCount) {
    if (m_Param.dwMode & FDE_TEXTEDITMODE_FIELD_TAB && nLength == 1 &&
        lpText[0] == L'\t') {
      return Move2NextEditableField(nStart) ? FDE_TXTEDT_MODIFY_RET_T_Tab
                                            : FDE_TXTEDT_MODIFY_RET_F_Tab;
    }
    int32_t nSelRangeCount = CountSelRanges();
    if (nSelRangeCount > 0) {
      if (nSelRangeCount > 1) {
        return FDE_TXTEDT_MODIFY_RET_F_Boundary;
      }
      int32_t nSelStart;
      int32_t nSelCount;
      nSelCount = GetSelRange(0, nSelStart);
      int32_t nSelEnd = nSelStart + nSelCount;
      int32_t nBgn = 0;
      int32_t nEnd = 0;
      CFDE_TxtEdtField* pField = NULL;
      FX_BOOL bInField = GetFieldBoundary(nSelStart, nBgn, nEnd, pField);
      if (nSelEnd > nEnd) {
        return FDE_TXTEDT_MODIFY_RET_F_Boundary;
      }
      if (bInField) {
        pField->Backup();
        FX_BOOL bBefore = FALSE;
        CFX_WideString wsDel;
        int32_t nCaret;
        int32_t nIndexInField = nSelStart - nBgn;
        int32_t nRet = pField->Replace(nSelStart - nBgn, nSelCount,
                                       CFX_WideStringC(lpText, nLength), wsDel,
                                       nCaret, bBefore);
        switch (nRet) {
          case FDE_FORMAT_FIELD_INSERT_RET_F_FULL:
            pField->Restore();
            return FDE_TXTEDT_MODIFY_RET_F_Full;
          case FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE:
            pField->Restore();
            return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
          default:
            break;
        }
        CFX_WideString wsField;
        pField->GetFieldText(wsField);
        if (!m_Param.pEventSink->On_ValidateField(this, pField->GetBlockIndex(),
                                                  pField->GetIndex(), wsField,
                                                  0)) {
          pField->Restore();
          return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
        }
        CFX_WideString wsDisplay;
        pField->GetDisplayText(wsDisplay);
        if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Vert) ||
            (m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Horz)) {
          CFX_WideString wsText;
          GetPreReplaceText(wsText, nBgn, nEnd - nBgn + 1,
                            const FX_WCHAR*(wsDisplay), wsDisplay.GetLength());
          if (!IsFitArea(wsText)) {
            pField->Restore();
            return FDE_TXTEDT_MODIFY_RET_F_Full;
          }
        }
        Replace(nBgn, nEnd - nBgn + 1, wsDisplay);
        int32_t nNewCaret = nBgn + nCaret;
        if (!(m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo)) {
          IFDE_TxtEdtDoRecord* pRecord = new CFDE_TxtEdtDoRecord_FieldReplace(
              this, m_nCaret, nNewCaret, pField, nIndexInField, nBgn,
              wsDisplay.GetLength(), wsDel, CFX_WideStringC(lpText, nLength),
              TRUE);
          CFX_ByteString bsDoRecord;
          pRecord->Serialize(bsDoRecord);
          m_Param.pEventSink->On_AddDoRecord(this, bsDoRecord);
          pRecord->Release();
        }
        SetCaretPos(nBgn + nCaret, bBefore);
        return FDE_TXTEDT_MODIFY_RET_S_Normal;
      }
    }
    int32_t nBgn = 0;
    int32_t nEnd = 0;
    CFDE_TxtEdtField* pField = NULL;
    FX_BOOL bInField = GetFieldBoundary(m_nCaret, nBgn, nEnd, pField);
    int32_t nCaretInField = m_nCaret - nBgn;
    FX_BOOL bBefore = FALSE;
    if (bInField) {
      pField->Backup();
      CFX_WideStringC wsIns(lpText, nLength);
      int32_t nRet =
          pField->Insert(nCaretInField, wsIns, nCaretInField, bBefore);
      FX_BOOL bFull = FALSE;
      switch (nRet) {
        case FDE_FORMAT_FIELD_INSERT_RET_S_NORMAL:
          break;
        case FDE_FORMAT_FIELD_INSERT_RET_S_FULL:
          bFull = TRUE;
          break;
        case FDE_FORMAT_FIELD_INSERT_RET_F_FULL:
          return FDE_TXTEDT_MODIFY_RET_F_Full;
        case FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE:
          return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
        default:
          return FDE_TXTEDT_MODIFY_RET_F_Normal;
      }
      CFX_WideString wsField;
      pField->GetFieldText(wsField);
      if (!m_Param.pEventSink->On_ValidateField(
              this, pField->GetBlockIndex(), pField->GetIndex(), wsField, 0)) {
        pField->Restore();
        return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
      }
      CFX_WideString wsDisplay;
      pField->GetDisplayText(wsDisplay);
      if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Vert) ||
          (m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Horz)) {
        CFX_WideString wsText;
        GetPreReplaceText(wsText, nBgn, nEnd - nBgn + 1,
                          const FX_WCHAR*(wsDisplay), wsDisplay.GetLength());
        if (!IsFitArea(wsText)) {
          pField->Restore();
          return FDE_TXTEDT_MODIFY_RET_F_Full;
        }
      }
      Replace(nBgn, nEnd - nBgn + 1, wsDisplay);
      if (!(m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo)) {
        IFDE_TxtEdtDoRecord* pRecord = new CFDE_TxtEdtDoRecord_FieldInsert(
            this, m_nCaret, pField, m_nCaret - nBgn, nBgn, nEnd - nBgn + 1,
            wsDisplay.GetLength(), CFX_WideStringC(lpText, nLength), FALSE);
        CFX_ByteString bsDoRecord;
        pRecord->Serialize(bsDoRecord);
        m_Param.pEventSink->On_AddDoRecord(this, bsDoRecord);
        pRecord->Release();
      }
      int32_t nCaretPos = nBgn + nCaretInField;
      if (m_Param.dwMode & FDE_TEXTEDITMODE_FIELD_AUTO && bFull &&
          nCaretPos == nEnd) {
        if (Move2NextEditableField(nEnd, TRUE, FALSE)) {
          return TRUE;
        }
      }
      SetCaretPos(nCaretPos, bBefore);
      return bFull ? FDE_TXTEDT_MODIFY_RET_S_Full
                   : FDE_TXTEDT_MODIFY_RET_S_Normal;
    }
    FXSYS_assert(0);
    return FDE_TXTEDT_MODIFY_RET_F_Normal;
  }
#endif
  CFX_WideString wsTemp;
  FX_WCHAR* lpBuffer = wsTemp.GetBuffer(nLength);
  FXSYS_memcpy(lpBuffer, lpText, nLength * sizeof(FX_WCHAR));
  ReplaceParagEnd(lpBuffer, nLength, FALSE);
  wsTemp.ReleaseBuffer(nLength);
  FX_BOOL bPart = FALSE;
  if (m_nLimit > 0) {
    int32_t nTotalLength = GetTextBufLength();
    int32_t nCount = m_SelRangePtrArr.GetSize();
    for (int32_t i = 0; i < nCount; i++) {
      FDE_LPTXTEDTSELRANGE lpSelRange = m_SelRangePtrArr.GetAt(i);
      nTotalLength -= lpSelRange->nCount;
    }
    int32_t nExpectLength = nTotalLength + nLength;
    if (nTotalLength == m_nLimit) {
      return FDE_TXTEDT_MODIFY_RET_F_Full;
    }
    if (nExpectLength > m_nLimit) {
      nLength -= (nExpectLength - m_nLimit);
      bPart = TRUE;
    }
  }
  if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Vert) ||
      (m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Horz)) {
    int32_t nTemp = nLength;
    if (m_Param.dwMode & FDE_TEXTEDITMODE_Password) {
      CFX_WideString wsText;
      while (nLength > 0) {
        GetPreInsertText(wsText, m_nCaret, lpBuffer, nLength);
        int32_t nTotal = wsText.GetLength();
        FX_WCHAR* lpBuf = wsText.GetBuffer(nTotal);
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
      CFX_WideString wsText;
      while (nLength > 0) {
        GetPreInsertText(wsText, m_nCaret, lpBuffer, nLength);
        if (IsFitArea(wsText)) {
          break;
        }
        nLength--;
      }
    }
    if (nLength == 0) {
      return FDE_TXTEDT_MODIFY_RET_F_Full;
    }
    if (nLength < nTemp) {
      bPart = TRUE;
    }
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_Validate) {
    CFX_WideString wsText;
    GetPreInsertText(wsText, m_nCaret, lpBuffer, nLength);
    if (!m_Param.pEventSink->On_Validate(this, wsText)) {
      return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
    }
  }
  if (IsSelect()) {
    DeleteSelect();
  }
  if (!(m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo)) {
    IFDE_TxtEdtDoRecord* pRecord =
        new CFDE_TxtEdtDoRecord_Insert(this, m_nCaret, lpBuffer, nLength);
    CFX_ByteString bsDoRecord;
    pRecord->Serialize(bsDoRecord);
    m_Param.pEventSink->On_AddDoRecord(this, bsDoRecord);
    pRecord->Release();
  }
  GetText(m_ChangeInfo.wsPrevText, 0);
  Inner_Insert(m_nCaret, lpBuffer, nLength);
  m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Insert;
  m_ChangeInfo.wsInsert = CFX_WideString(lpBuffer, nLength);
  nStart = m_nCaret;
  nStart += nLength;
  FX_WCHAR wChar = m_pTxtBuf->GetCharByIndex(nStart - 1);
  FX_BOOL bBefore = TRUE;
  if (wChar != L'\n' && wChar != L'\r') {
    nStart--;
    bBefore = FALSE;
  }
  SetCaretPos(nStart, bBefore);
  m_Param.pEventSink->On_TextChanged(this, m_ChangeInfo);
  return bPart ? FDE_TXTEDT_MODIFY_RET_S_Part : FDE_TXTEDT_MODIFY_RET_S_Normal;
}
int32_t CFDE_TxtEdtEngine::Delete(int32_t nStart, FX_BOOL bBackspace) {
  if (IsLocked()) {
    return FDE_TXTEDT_MODIFY_RET_F_Locked;
  }
  if (IsSelect()) {
    DeleteSelect();
    return FDE_TXTEDT_MODIFY_RET_S_Normal;
  }
#ifdef FDE_USEFORMATBLOCK
  int32_t nBlockCount = m_BlockArray.GetSize();
  if (nBlockCount > 0) {
    if (bBackspace) {
      nStart--;
    }
    int32_t nCount = 1;
    int32_t nBgn = 0;
    int32_t nEnd = 0;
    CFDE_TxtEdtField* pField = NULL;
    FX_BOOL bInField = GetFieldBoundary(nStart, nBgn, nEnd, pField);
    int32_t nCaretInField = nStart - nBgn;
    FX_BOOL bBefore = FALSE;
    if (bInField && !pField->IsFix()) {
      pField->Backup();
      CFX_WideString wsDel;
      int32_t nCaret = 0;
      int32_t nRet =
          pField->Delete(nCaretInField, nCount, wsDel, nCaret, bBefore);
      nCaret += nBgn;
      switch (nRet) {
        case FDE_FORMAT_FIELD_DELETE_RET_S:
          break;
        case FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE:
          return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
        case FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY:
          return FDE_TXTEDT_MODIFY_RET_F_Boundary;
        default:
          FXSYS_assert(0);
          break;
      }
      CFX_WideString wsField;
      pField->GetFieldText(wsField);
      if (!m_Param.pEventSink->On_ValidateField(
              this, pField->GetBlockIndex(), pField->GetIndex(), wsField, 0)) {
        pField->Restore();
        return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
      }
      CFX_WideString wsDisplay;
      pField->GetDisplayText(wsDisplay);
      Replace(nBgn, nEnd - nBgn + 1, wsDisplay);
      if (!(m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo)) {
        IFDE_TxtEdtDoRecord* pRecord = new CFDE_TxtEdtDoRecord_FieldDelete(
            this, nStart, pField, nCaretInField, nBgn, nEnd - nBgn + 1,
            wsDisplay.GetLength(), wsDel, FALSE);
        CFX_ByteString bsDoRecord;
        pRecord->Serialize(bsDoRecord);
        m_Param.pEventSink->On_AddDoRecord(this, bsDoRecord);
        pRecord->Release();
      }
      SetCaretPos(nStart, bBefore);
      return FDE_TXTEDT_MODIFY_RET_S_Normal;
    }
    return FDE_TXTEDT_MODIFY_RET_F_Boundary;
  }
#endif
  int32_t nCount = 1;
  if (bBackspace) {
    if (nStart == 0) {
      return FDE_TXTEDT_MODIFY_RET_F_Boundary;
    }
    if (nStart > 2 && m_pTxtBuf->GetCharByIndex(nStart - 1) == L'\n' &&
        m_pTxtBuf->GetCharByIndex(nStart - 2) == L'\r') {
      nStart--;
      nCount++;
    }
    nStart--;
  } else {
    if (nStart == GetTextBufLength()) {
      return FDE_TXTEDT_MODIFY_RET_F_Full;
    }
    if ((nStart + 1 < GetTextBufLength()) &&
        (m_pTxtBuf->GetCharByIndex(nStart) == L'\r') &&
        (m_pTxtBuf->GetCharByIndex(nStart + 1) == L'\n')) {
      nCount++;
    }
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_Validate) {
    CFX_WideString wsText;
    GetPreDeleteText(wsText, nStart, nCount);
    if (!m_Param.pEventSink->On_Validate(this, wsText)) {
      return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
    }
  }
  if (!(m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo)) {
    CFX_WideString wsRange;
    m_pTxtBuf->GetRange(wsRange, nStart, nCount);
    IFDE_TxtEdtDoRecord* pRecord =
        new CFDE_TxtEdtDoRecord_DeleteRange(this, nStart, m_nCaret, wsRange);
    CFX_ByteString bsDoRecord;
    pRecord->Serialize(bsDoRecord);
    m_Param.pEventSink->On_AddDoRecord(this, bsDoRecord);
    pRecord->Release();
  }
  m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Delete;
  GetText(m_ChangeInfo.wsDelete, nStart, nCount);
  Inner_DeleteRange(nStart, nCount);
  SetCaretPos(nStart + ((!bBackspace && nStart > 0) ? -1 : 0),
              (bBackspace || nStart == 0));
  m_Param.pEventSink->On_TextChanged(this, m_ChangeInfo);
  return FDE_TXTEDT_MODIFY_RET_S_Normal;
}
int32_t CFDE_TxtEdtEngine::DeleteRange(int32_t nStart, int32_t nCount) {
  if (IsLocked()) {
    return FDE_TXTEDT_MODIFY_RET_F_Locked;
  }
  if (nCount == -1) {
    nCount = GetTextBufLength();
  }
  if (nCount == 0) {
    return FDE_TXTEDT_MODIFY_RET_S_Normal;
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_Validate) {
    CFX_WideString wsText;
    GetPreDeleteText(wsText, nStart, nCount);
    if (!m_Param.pEventSink->On_Validate(this, wsText)) {
      return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
    }
  }
  DeleteRange_DoRecord(nStart, nCount);
  m_Param.pEventSink->On_TextChanged(this, m_ChangeInfo);
  SetCaretPos(nStart, TRUE);
  return FDE_TXTEDT_MODIFY_RET_S_Normal;
}
int32_t CFDE_TxtEdtEngine::Replace(int32_t nStart,
                                   int32_t nLength,
                                   const CFX_WideString& wsReplace) {
  if (IsLocked()) {
    return FDE_TXTEDT_MODIFY_RET_F_Locked;
  }
  if (nStart < 0 || (nStart + nLength > GetTextBufLength())) {
    return FDE_TXTEDT_MODIFY_RET_F_Boundary;
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_Validate) {
    CFX_WideString wsText;
    GetPreReplaceText(wsText, nStart, nLength, wsReplace.c_str(),
                      wsReplace.GetLength());
    if (!m_Param.pEventSink->On_Validate(this, wsText)) {
      return FDE_TXTEDT_MODIFY_RET_F_Invalidate;
    }
  }
  if (IsSelect()) {
    ClearSelection();
  }
  m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Replace;
  GetText(m_ChangeInfo.wsDelete, nStart, nLength);
  if (nLength > 0) {
    Inner_DeleteRange(nStart, nLength);
  }
  int32_t nTextLength = wsReplace.GetLength();
  if (nTextLength > 0) {
    Inner_Insert(nStart, wsReplace.c_str(), nTextLength);
  }
  m_ChangeInfo.wsInsert = CFX_WideString(wsReplace.c_str(), nTextLength);
  nStart += nTextLength;
  FX_WCHAR wChar = m_pTxtBuf->GetCharByIndex(nStart - 1);
  FX_BOOL bBefore = TRUE;
  if (wChar != L'\n' && wChar != L'\r') {
    nStart--;
    bBefore = FALSE;
  }
  SetCaretPos(nStart, bBefore);
  m_Param.pEventSink->On_PageUnload(this, m_nCaretPage, 0);
  m_Param.pEventSink->On_PageLoad(this, m_nCaretPage, 0);
  m_Param.pEventSink->On_TextChanged(this, m_ChangeInfo);
  return FDE_TXTEDT_MODIFY_RET_S_Normal;
}
void CFDE_TxtEdtEngine::SetLimit(int32_t nLimit) {
  m_nLimit = nLimit;
}
void CFDE_TxtEdtEngine::SetAliasChar(FX_WCHAR wcAlias) {
  m_wcAliasChar = wcAlias;
}
void CFDE_TxtEdtEngine::SetFormatBlock(int32_t nIndex,
                                       const CFX_WideString& wsBlockFormat) {
#ifdef FDE_USEFORMATBLOCK
  if (m_nFixLength == -1) {
    m_nFixLength = GetTextLength();
    FXSYS_assert(m_wsFixText.IsEmpty());
    GetText(m_wsFixText, 0, -1);
  }
  FX_BOOL bInBlock = FALSE;
  int32_t nCharIndex = 0;
  int32_t nBlockIndex = 0;
  int32_t nBlockPos = -1;
  FX_WCHAR wc;
  CFDE_TxtEdtBufIter* pIter =
      new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)m_pTxtBuf, FALSE);
  pIter->SetAt(0);
  do {
    wc = pIter->GetChar();
    if (bInBlock) {
      if (wc == FDE_TXTEDT_FORMATBLOCK_END) {
        nBlockIndex++;
        bInBlock = FALSE;
      }
    } else {
      if (wc == FDE_TXTEDT_FORMATBLOCK_BGN) {
        bInBlock = TRUE;
      } else {
        if (nCharIndex++ == nIndex) {
          nBlockPos = pIter->GetAt();
          break;
        }
      }
    }
  } while (pIter->Next());
  pIter->Release();
  if (nBlockPos == -1) {
    nBlockPos = GetTextBufLength();
  }
  CFDE_TxtEdtBlock* pEditBlock =
      new CFDE_TxtEdtBlock(this, wsBlockFormat, nIndex);
  m_BlockArray.InsertAt(m_BlockArray.GetSize(), pEditBlock);
  CFX_WideString wsDisplay;
  pEditBlock->GetDisplayText(wsDisplay);
  m_nCaret = nBlockPos;
  if (wsDisplay.GetLength() > 0) {
    RawInsert(nBlockPos, const FX_WCHAR*(wsDisplay), wsDisplay.GetLength());
  }
#endif
}
int32_t CFDE_TxtEdtEngine::CountEditBlocks() const {
#ifdef FDE_USEFORMATBLOCK
  return m_BlockArray.GetSize();
#else
  return 0;
#endif
}
void CFDE_TxtEdtEngine::GetEditBlockText(int32_t nIndex,
                                         CFX_WideString& wsBlockText) const {
#ifdef FDE_USEFORMATBLOCK
  CFDE_TxtEdtBlock* pBlock = m_BlockArray[nIndex];
  pBlock->GetBlockText(wsBlockText);
#endif
}
int32_t CFDE_TxtEdtEngine::CountEditFields(int32_t nBlockIndex) const {
#ifdef FDE_USEFORMATBLOCK
  CFDE_TxtEdtBlock* pBlock = m_BlockArray[nBlockIndex];
  return pBlock->CountField();
#else
  return 0;
#endif
}
void CFDE_TxtEdtEngine::GetEditFieldText(int32_t nBlockIndex,
                                         int32_t nFieldIndex,
                                         CFX_WideString& wsFieldText) const {
#ifdef FDE_USEFORMATBLOCK
  CFDE_TxtEdtBlock* pBlock = m_BlockArray[nBlockIndex];
  pBlock->GetFieldText(nFieldIndex, wsFieldText);
#endif
}
void CFDE_TxtEdtEngine::StartEdit() {
#ifdef FDE_USEFORMATBLOCK
#endif
}
void CFDE_TxtEdtEngine::EndEdit() {
#ifdef FDE_USEFORMATBLOCK
#endif
}
void CFDE_TxtEdtEngine::RemoveSelRange(int32_t nStart, int32_t nCount) {
  FDE_LPTXTEDTSELRANGE lpTemp = NULL;
  int32_t nRangeCount = m_SelRangePtrArr.GetSize();
  int32_t i = 0;
  for (i = 0; i < nRangeCount; i++) {
    lpTemp = m_SelRangePtrArr[i];
    if (lpTemp->nStart == nStart && lpTemp->nCount == nCount) {
      delete lpTemp;
      m_SelRangePtrArr.RemoveAt(i);
      return;
    }
  }
  return;
}
void CFDE_TxtEdtEngine::AddSelRange(int32_t nStart, int32_t nCount) {
  if (nCount == -1) {
    nCount = GetTextLength() - nStart;
  }
  int32_t nSize = m_SelRangePtrArr.GetSize();
  if (nSize <= 0) {
    FDE_LPTXTEDTSELRANGE lpSelRange = new FDE_TXTEDTSELRANGE;
    lpSelRange->nStart = nStart;
    lpSelRange->nCount = nCount;
    m_SelRangePtrArr.Add(lpSelRange);
    m_Param.pEventSink->On_SelChanged(this);
    return;
  }
  FDE_LPTXTEDTSELRANGE lpTemp = NULL;
  lpTemp = m_SelRangePtrArr[nSize - 1];
  if (nStart >= lpTemp->nStart + lpTemp->nCount) {
    FDE_LPTXTEDTSELRANGE lpSelRange = new FDE_TXTEDTSELRANGE;
    lpSelRange->nStart = nStart;
    lpSelRange->nCount = nCount;
    m_SelRangePtrArr.Add(lpSelRange);
    m_Param.pEventSink->On_SelChanged(this);
    return;
  }
  int32_t nEnd = nStart + nCount - 1;
  FX_BOOL bBegin = FALSE;
  int32_t nRangeBgn = 0;
  int32_t nRangeCnt = 0;
  for (int32_t i = 0; i < nSize; i++) {
    lpTemp = m_SelRangePtrArr[i];
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
        bBegin = TRUE;
      }
    }
  }
  if (nRangeCnt == 0) {
    FDE_LPTXTEDTSELRANGE lpSelRange = new FDE_TXTEDTSELRANGE;
    lpSelRange->nStart = nStart;
    lpSelRange->nCount = nCount;
    m_SelRangePtrArr.InsertAt(nRangeBgn, lpSelRange);
  } else {
    lpTemp = m_SelRangePtrArr[nRangeBgn];
    lpTemp->nStart = nStart;
    lpTemp->nCount = nCount;
    nRangeCnt--;
    nRangeBgn++;
    while (nRangeCnt--) {
      delete m_SelRangePtrArr[nRangeBgn];
      m_SelRangePtrArr.RemoveAt(nRangeBgn);
    }
  }
  m_Param.pEventSink->On_SelChanged(this);
  return;
}
int32_t CFDE_TxtEdtEngine::CountSelRanges() {
  return m_SelRangePtrArr.GetSize();
}
int32_t CFDE_TxtEdtEngine::GetSelRange(int32_t nIndex, int32_t& nStart) {
  nStart = m_SelRangePtrArr[nIndex]->nStart;
  return m_SelRangePtrArr[nIndex]->nCount;
}
void CFDE_TxtEdtEngine::ClearSelection() {
  int32_t nCount = m_SelRangePtrArr.GetSize();
  FDE_LPTXTEDTSELRANGE lpRange = NULL;
  int32_t i = 0;
  for (i = 0; i < nCount; i++) {
    lpRange = m_SelRangePtrArr[i];
    if (lpRange != NULL) {
      delete lpRange;
      lpRange = NULL;
    }
  }
  m_SelRangePtrArr.RemoveAll();
  if (nCount && m_Param.pEventSink) {
    m_Param.pEventSink->On_SelChanged(this);
  }
}
FX_BOOL CFDE_TxtEdtEngine::Redo(const CFX_ByteStringC& bsRedo) {
  if (IsLocked()) {
    return FALSE;
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo) {
    return FALSE;
  }
  IFDE_TxtEdtDoRecord* pDoRecord = IFDE_TxtEdtDoRecord::Create(bsRedo);
  FXSYS_assert(pDoRecord);
  if (pDoRecord == NULL) {
    return FALSE;
  }
  FX_BOOL bOK = pDoRecord->Redo();
  pDoRecord->Release();
  return bOK;
}
FX_BOOL CFDE_TxtEdtEngine::Undo(const CFX_ByteStringC& bsUndo) {
  if (IsLocked()) {
    return FALSE;
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo) {
    return FALSE;
  }
  IFDE_TxtEdtDoRecord* pDoRecord = IFDE_TxtEdtDoRecord::Create(bsUndo);
  FXSYS_assert(pDoRecord);
  if (pDoRecord == NULL) {
    return FALSE;
  }
  FX_BOOL bOK = pDoRecord->Undo();
  pDoRecord->Release();
  return bOK;
}
int32_t CFDE_TxtEdtEngine::StartLayout() {
  Lock();
  RemoveAllPages();
  m_nLayoutPos = 0;
  m_nLineCount = 0;
  return 0;
}
int32_t CFDE_TxtEdtEngine::DoLayout(IFX_Pause* pPause) {
  int32_t nCount = m_ParagPtrArray.GetSize();
  CFDE_TxtEdtParag* pParag = NULL;
  int32_t nLineCount = 0;
  for (; m_nLayoutPos < nCount; m_nLayoutPos++) {
    pParag = m_ParagPtrArray[m_nLayoutPos];
    pParag->CalcLines();
    nLineCount += pParag->m_nLineCount;
    if ((pPause != NULL) && (nLineCount > m_nPageLineCount) &&
        pPause->NeedToPauseNow()) {
      m_nLineCount += nLineCount;
      return (++m_nLayoutPos * 100) / nCount;
    }
  }
  m_nLineCount += nLineCount;
  return 100;
}
void CFDE_TxtEdtEngine::EndLayout() {
  UpdatePages();
  int32_t nLength = GetTextLength();
  if (m_nCaret > nLength) {
    m_nCaret = nLength;
  }
  int32_t nIndex = m_nCaret;
  if (!m_bBefore) {
    nIndex--;
  }
  m_rtCaret.Set(0, 0, 1, m_Param.fFontSize);
  Unlock();
}
FX_BOOL CFDE_TxtEdtEngine::Optimize(IFX_Pause* pPause) {
  return m_pTxtBuf->Optimize(pPause);
}
IFDE_TxtEdtBuf* CFDE_TxtEdtEngine::GetTextBuf() const {
  return (IFDE_TxtEdtBuf*)m_pTxtBuf;
}
int32_t CFDE_TxtEdtEngine::GetTextBufLength() const {
  return m_pTxtBuf->GetTextLength() - 1;
}
IFX_TxtBreak* CFDE_TxtEdtEngine::GetTextBreak() const {
  return m_pTextBreak;
}
int32_t CFDE_TxtEdtEngine::GetLineCount() const {
  return m_nLineCount;
}
int32_t CFDE_TxtEdtEngine::GetPageLineCount() const {
  return m_nPageLineCount;
}
int32_t CFDE_TxtEdtEngine::CountParags() const {
  return m_ParagPtrArray.GetSize();
}
IFDE_TxtEdtParag* CFDE_TxtEdtEngine::GetParag(int32_t nParagIndex) const {
  return m_ParagPtrArray[nParagIndex];
}
IFX_CharIter* CFDE_TxtEdtEngine::CreateCharIter() {
  if (!m_pTxtBuf) {
    return NULL;
  }
  return new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)m_pTxtBuf);
}
int32_t CFDE_TxtEdtEngine::Line2Parag(int32_t nStartParag,
                                      int32_t nStartLineofParag,
                                      int32_t nLineIndex,
                                      int32_t& nStartLine) const {
  int32_t nLineTotal = nStartLineofParag;
  int32_t nCount = m_ParagPtrArray.GetSize();
  CFDE_TxtEdtParag* pParag = NULL;
  int32_t i = nStartParag;
  for (; i < nCount; i++) {
    pParag = m_ParagPtrArray[i];
    nLineTotal += pParag->m_nLineCount;
    if (nLineTotal > nLineIndex) {
      break;
    }
  }
  nStartLine = nLineTotal - pParag->m_nLineCount;
  return i;
}
void CFDE_TxtEdtEngine::GetPreDeleteText(CFX_WideString& wsText,
                                         int32_t nIndex,
                                         int32_t nLength) {
  GetText(wsText, 0, GetTextBufLength());
  wsText.Delete(nIndex, nLength);
}
void CFDE_TxtEdtEngine::GetPreInsertText(CFX_WideString& wsText,
                                         int32_t nIndex,
                                         const FX_WCHAR* lpText,
                                         int32_t nLength) {
  GetText(wsText, 0, GetTextBufLength());
  int32_t nSelIndex = 0;
  int32_t nSelLength = 0;
  int32_t nSelCount = CountSelRanges();
  while (nSelCount--) {
    nSelLength = GetSelRange(nSelCount, nSelIndex);
    wsText.Delete(nSelIndex, nSelLength);
    nIndex = nSelIndex;
  }
  CFX_WideString wsTemp;
  int32_t nOldLength = wsText.GetLength();
  const FX_WCHAR* pOldBuffer = wsText.c_str();
  FX_WCHAR* lpBuffer = wsTemp.GetBuffer(nOldLength + nLength);
  FXSYS_memcpy(lpBuffer, pOldBuffer, (nIndex) * sizeof(FX_WCHAR));
  FXSYS_memcpy(lpBuffer + nIndex, lpText, nLength * sizeof(FX_WCHAR));
  FXSYS_memcpy(lpBuffer + nIndex + nLength, pOldBuffer + nIndex,
               (nOldLength - nIndex) * sizeof(FX_WCHAR));
  wsTemp.ReleaseBuffer(nOldLength + nLength);
  wsText = wsTemp;
}
void CFDE_TxtEdtEngine::GetPreReplaceText(CFX_WideString& wsText,
                                          int32_t nIndex,
                                          int32_t nOriginLength,
                                          const FX_WCHAR* lpText,
                                          int32_t nLength) {
  GetText(wsText, 0, GetTextBufLength());
  int32_t nSelIndex = 0;
  int32_t nSelLength = 0;
  int32_t nSelCount = CountSelRanges();
  while (nSelCount--) {
    nSelLength = GetSelRange(nSelCount, nSelIndex);
    wsText.Delete(nSelIndex, nSelLength);
  }
  wsText.Delete(nIndex, nOriginLength);
  int32_t i = 0;
  for (i = 0; i < nLength; i++) {
    wsText.Insert(nIndex++, lpText[i]);
  }
}
void CFDE_TxtEdtEngine::Inner_Insert(int32_t nStart,
                                     const FX_WCHAR* lpText,
                                     int32_t nLength) {
  FXSYS_assert(nLength > 0);
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nStart, ParagPos);
  m_Param.pEventSink->On_PageUnload(this, m_nCaretPage, 0);
  int32_t nParagCount = m_ParagPtrArray.GetSize();
  int32_t i = 0;
  for (i = ParagPos.nParagIndex + 1; i < nParagCount; i++) {
    m_ParagPtrArray[i]->m_nCharStart += nLength;
  }
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex];
  int32_t nReserveLineCount = pParag->m_nLineCount;
  int32_t nReserveCharStart = pParag->m_nCharStart;
  int32_t nLeavePart = ParagPos.nCharIndex;
  int32_t nCutPart = pParag->m_nCharCount - ParagPos.nCharIndex;
  int32_t nTextStart = 0;
  FX_WCHAR wCurChar = L' ';
  const FX_WCHAR* lpPos = lpText;
  FX_BOOL bFirst = TRUE;
  int32_t nParagIndex = ParagPos.nParagIndex;
  for (i = 0; i < nLength; i++, lpPos++) {
    wCurChar = *lpPos;
    if (wCurChar == m_wLineEnd) {
      if (bFirst) {
        pParag->m_nCharCount = nLeavePart + (i - nTextStart + 1);
        pParag->m_nLineCount = -1;
        nReserveCharStart += pParag->m_nCharCount;
        bFirst = FALSE;
      } else {
        pParag = new CFDE_TxtEdtParag(this);
        pParag->m_nLineCount = -1;
        pParag->m_nCharCount = i - nTextStart + 1;
        pParag->m_nCharStart = nReserveCharStart;
        m_ParagPtrArray.InsertAt(++nParagIndex, pParag);
        nReserveCharStart += pParag->m_nCharCount;
      }
      nTextStart = i + 1;
    }
  }
  if (bFirst) {
    pParag->m_nCharCount += nLength;
    pParag->m_nLineCount = -1;
    bFirst = FALSE;
  } else {
    pParag = new CFDE_TxtEdtParag(this);
    pParag->m_nLineCount = -1;
    pParag->m_nCharCount = nLength - nTextStart + nCutPart;
    pParag->m_nCharStart = nReserveCharStart;
    m_ParagPtrArray.InsertAt(++nParagIndex, pParag);
  }
  m_pTxtBuf->Insert(nStart, lpText, nLength);
  int32_t nTotalLineCount = 0;
  for (i = ParagPos.nParagIndex; i <= nParagIndex; i++) {
    pParag = m_ParagPtrArray[i];
    pParag->CalcLines();
    nTotalLineCount += pParag->m_nLineCount;
  }
  m_nLineCount += nTotalLineCount - nReserveLineCount;
  m_Param.pEventSink->On_PageLoad(this, m_nCaretPage, 0);
  UpdatePages();
}
#ifdef FDE_USEFORMATBLOCK
void CFDE_TxtEdtEngine::RawInsert(int32_t nStart,
                                  const FX_WCHAR* lpText,
                                  int32_t nLength) {
  FXSYS_assert(nLength > 0);
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nStart, ParagPos);
  int32_t nParagCount = m_ParagPtrArray.GetSize();
  int32_t i = 0;
  for (i = ParagPos.nParagIndex + 1; i < nParagCount; i++) {
    m_ParagPtrArray[i]->m_nCharStart += nLength;
  }
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex];
  int32_t nReserveLineCount = pParag->m_nLineCount;
  int32_t nReserveCharStart = pParag->m_nCharStart;
  int32_t nLeavePart = ParagPos.nCharIndex;
  int32_t nCutPart = pParag->m_nCharCount - ParagPos.nCharIndex;
  int32_t nTextStart = 0;
  FX_WCHAR wCurChar = L' ';
  const FX_WCHAR* lpPos = lpText;
  FX_BOOL bFirst = TRUE;
  int32_t nParagIndex = ParagPos.nParagIndex;
  for (i = 0; i < nLength; i++, lpPos++) {
    wCurChar = *lpPos;
    if (wCurChar == m_wLineEnd) {
      if (bFirst) {
        pParag->m_nCharCount = nLeavePart + (i - nTextStart + 1);
        pParag->m_nLineCount = -1;
        nReserveCharStart += pParag->m_nCharCount;
        bFirst = FALSE;
      } else {
        pParag = new CFDE_TxtEdtParag(this);
        pParag->m_nLineCount = -1;
        pParag->m_nCharCount = i - nTextStart + 1;
        pParag->m_nCharStart = nReserveCharStart;
        m_ParagPtrArray.InsertAt(++nParagIndex, pParag);
        nReserveCharStart += pParag->m_nCharCount;
      }
      nTextStart = i + 1;
    }
  }
  if (bFirst) {
    pParag->m_nCharCount += nLength;
    pParag->m_nLineCount = -1;
    bFirst = FALSE;
  } else {
    pParag = new CFDE_TxtEdtParag(this);
    pParag->m_nLineCount = -1;
    pParag->m_nCharCount = nLength - nTextStart + nCutPart;
    pParag->m_nCharStart = nReserveCharStart;
    m_ParagPtrArray.InsertAt(++nParagIndex, pParag);
  }
  m_pTxtBuf->Insert(nStart, lpText, nLength);
}
#endif
void CFDE_TxtEdtEngine::Inner_DeleteRange(int32_t nStart, int32_t nCount) {
  if (nCount == -1) {
    nCount = m_pTxtBuf->GetTextLength() - nStart;
  }
  int32_t nEnd = nStart + nCount - 1;
  FXSYS_assert(nStart >= 0 && nEnd < m_pTxtBuf->GetTextLength());
  m_Param.pEventSink->On_PageUnload(this, m_nCaretPage, 0);
  FDE_TXTEDTPARAGPOS ParagPosBgn, ParagPosEnd;
  TextPos2ParagPos(nStart, ParagPosBgn);
  TextPos2ParagPos(nEnd, ParagPosEnd);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPosEnd.nParagIndex];
  FX_BOOL bLastParag = FALSE;
  if (ParagPosEnd.nCharIndex == pParag->m_nCharCount - 1) {
    if (ParagPosEnd.nParagIndex < m_ParagPtrArray.GetSize() - 1) {
      ParagPosEnd.nParagIndex++;
    } else {
      bLastParag = TRUE;
    }
  }
  int32_t nTotalLineCount = 0;
  int32_t nTotalCharCount = 0;
  int32_t i = 0;
  for (i = ParagPosBgn.nParagIndex; i <= ParagPosEnd.nParagIndex; i++) {
    CFDE_TxtEdtParag* pParag = m_ParagPtrArray[i];
    pParag->CalcLines();
    nTotalLineCount += pParag->m_nLineCount;
    nTotalCharCount += pParag->m_nCharCount;
  }
  m_pTxtBuf->Delete(nStart, nCount);
  int32_t nNextParagIndex = (ParagPosBgn.nCharIndex == 0 && bLastParag)
                                ? ParagPosBgn.nParagIndex
                                : (ParagPosBgn.nParagIndex + 1);
  for (i = nNextParagIndex; i <= ParagPosEnd.nParagIndex; i++) {
    CFDE_TxtEdtParag* pParag = m_ParagPtrArray[nNextParagIndex];
    delete pParag;
    m_ParagPtrArray.RemoveAt(nNextParagIndex);
  }
  if (!(bLastParag && ParagPosBgn.nCharIndex == 0)) {
    pParag = m_ParagPtrArray[ParagPosBgn.nParagIndex];
    pParag->m_nCharCount = nTotalCharCount - nCount;
    pParag->CalcLines();
    nTotalLineCount -= pParag->m_nLineCount;
  }
  int32_t nParagCount = m_ParagPtrArray.GetSize();
  for (i = nNextParagIndex; i < nParagCount; i++) {
    m_ParagPtrArray[i]->m_nCharStart -= nCount;
  }
  m_nLineCount -= nTotalLineCount;
  UpdatePages();
  int32_t nPageCount = CountPages();
  if (m_nCaretPage >= nPageCount) {
    m_nCaretPage = nPageCount - 1;
  }
  m_Param.pEventSink->On_PageLoad(this, m_nCaretPage, 0);
}
void CFDE_TxtEdtEngine::DeleteRange_DoRecord(int32_t nStart,
                                             int32_t nCount,
                                             FX_BOOL bSel) {
  FXSYS_assert(nStart >= 0);
  if (nCount == -1) {
    nCount = GetTextLength() - nStart;
  }
  FXSYS_assert((nStart + nCount) <= m_pTxtBuf->GetTextLength());
#ifdef FDE_USEFORMATBLOCK
  int32_t nBlockCount = m_BlockArray.GetSize();
  if (nBlockCount > 0) {
  }
#endif
  if (!(m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo)) {
    CFX_WideString wsRange;
    m_pTxtBuf->GetRange(wsRange, nStart, nCount);
    IFDE_TxtEdtDoRecord* pRecord = new CFDE_TxtEdtDoRecord_DeleteRange(
        this, nStart, m_nCaret, wsRange, bSel);
    CFX_ByteString bsDoRecord;
    pRecord->Serialize(bsDoRecord);
    m_Param.pEventSink->On_AddDoRecord(this, bsDoRecord);
    pRecord->Release();
  }
  m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Delete;
  GetText(m_ChangeInfo.wsDelete, nStart, nCount);
  Inner_DeleteRange(nStart, nCount);
}
void CFDE_TxtEdtEngine::ResetEngine() {
  RemoveAllPages();
  RemoveAllParags();
  ClearSelection();
  m_nCaret = 0;
  m_pTxtBuf->Clear(FALSE);
  m_nCaret = 0;
}
void CFDE_TxtEdtEngine::RebuildParagraphs() {
  RemoveAllParags();
  FX_WCHAR wChar = L' ';
  int32_t nParagStart = 0;
  int32_t nIndex = 0;
  IFX_CharIter* pIter = new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)m_pTxtBuf);
  pIter->SetAt(0);
  do {
    wChar = pIter->GetChar();
    nIndex = pIter->GetAt();
    if (wChar == m_wLineEnd) {
      CFDE_TxtEdtParag* pParag = new CFDE_TxtEdtParag(this);
      pParag->m_nCharStart = nParagStart;
      pParag->m_nCharCount = nIndex - nParagStart + 1;
      pParag->m_nLineCount = -1;
      m_ParagPtrArray.Add(pParag);
      nParagStart = nIndex + 1;
    }
  } while (pIter->Next());
  pIter->Release();
}
void CFDE_TxtEdtEngine::RemoveAllParags() {
  int32_t nCount = m_ParagPtrArray.GetSize();
  int32_t i = 0;
  for (i = 0; i < nCount; i++) {
    CFDE_TxtEdtParag* pParag = m_ParagPtrArray[i];
    if (pParag) {
      delete pParag;
    }
  }
  m_ParagPtrArray.RemoveAll();
}
void CFDE_TxtEdtEngine::RemoveAllPages() {
  int32_t nCount = m_PagePtrArray.GetSize();
  int32_t i = 0;
  for (i = 0; i < nCount; i++) {
    IFDE_TxtEdtPage* pPage = m_PagePtrArray[i];
    if (pPage) {
      pPage->Release();
    }
  }
  m_PagePtrArray.RemoveAll();
}
void CFDE_TxtEdtEngine::UpdateParags() {
  int32_t nCount = m_ParagPtrArray.GetSize();
  if (nCount == 0) {
    return;
  }
  CFDE_TxtEdtParag* pParag = NULL;
  int32_t nLineCount = 0;
  int32_t i = 0;
  for (i = 0; i < nCount; i++) {
    pParag = m_ParagPtrArray[i];
    if (pParag->m_nLineCount == -1) {
      pParag->CalcLines();
    }
    nLineCount += pParag->m_nLineCount;
  }
  m_nLineCount = nLineCount;
}
void CFDE_TxtEdtEngine::UpdatePages() {
  if (m_nLineCount == 0) {
    return;
  }
  int32_t nPageCount = (m_nLineCount - 1) / (m_nPageLineCount) + 1;
  int32_t nSize = m_PagePtrArray.GetSize();
  if (nSize == nPageCount) {
    return;
  }
  if (nSize > nPageCount) {
    IFDE_TxtEdtPage* pPage = NULL;
    int32_t i = 0;
    for (i = nSize - 1; i >= nPageCount; i--) {
      pPage = m_PagePtrArray[i];
      if (pPage) {
        pPage->Release();
      }
      m_PagePtrArray.RemoveAt(i);
    }
    m_Param.pEventSink->On_PageCountChanged(this);
    return;
  }
  if (nSize < nPageCount) {
    IFDE_TxtEdtPage* pPage = NULL;
    int32_t i = 0;
    for (i = nSize; i < nPageCount; i++) {
      pPage = IFDE_TxtEdtPage::Create(this, i);
      m_PagePtrArray.Add(pPage);
    }
    m_Param.pEventSink->On_PageCountChanged(this);
    return;
  }
}
void CFDE_TxtEdtEngine::UpdateTxtBreak() {
  FX_DWORD dwStyle = m_pTextBreak->GetLayoutStyles();
  if (m_Param.dwMode & FDE_TEXTEDITMODE_MultiLines) {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_SingleLine;
  } else {
    dwStyle |= FX_TXTLAYOUTSTYLE_SingleLine;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical) {
    dwStyle |= FX_TXTLAYOUTSTYLE_VerticalLayout;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_VerticalLayout;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_LineReserve) {
    dwStyle |= FX_TXTLAYOUTSTYLE_ReverseLine;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_ReverseLine;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_RTL) {
    dwStyle |= FX_TXTLAYOUTSTYLE_RTLReadingOrder;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_RTLReadingOrder;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_CombText) {
    dwStyle |= FX_TXTLAYOUTSTYLE_CombText;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_CombText;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_CharVertial) {
    dwStyle |= FX_TXTLAYOUTSTYLE_VerticalChars;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_VerticalChars;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_ExpandTab) {
    dwStyle |= FX_TXTLAYOUTSTYLE_ExpandTab;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_ExpandTab;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_ArabicContext) {
    dwStyle |= FX_TXTLAYOUTSTYLE_ArabicContext;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_ArabicContext;
  }
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_ArabicShapes) {
    dwStyle |= FX_TXTLAYOUTSTYLE_ArabicShapes;
  } else {
    dwStyle &= ~FX_TXTLAYOUTSTYLE_ArabicShapes;
  }
  m_pTextBreak->SetLayoutStyles(dwStyle);
  FX_DWORD dwAligment = 0;
  if (m_Param.dwAlignment & FDE_TEXTEDITALIGN_Justified) {
    dwAligment |= FX_TXTLINEALIGNMENT_Justified;
  } else if (m_Param.dwAlignment & FDE_TEXTEDITALIGN_Distributed) {
    dwAligment |= FX_TXTLINEALIGNMENT_Distributed;
  }
  if (m_Param.dwAlignment & FDE_TEXTEDITALIGN_Center) {
    dwAligment |= FX_TXTLINEALIGNMENT_Center;
  } else if (m_Param.dwAlignment & FDE_TEXTEDITALIGN_Right) {
    dwAligment |= FX_TXTLINEALIGNMENT_Right;
  }
  m_pTextBreak->SetAlignment(dwAligment);
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical) {
    if (m_Param.dwMode & FDE_TEXTEDITMODE_AutoLineWrap) {
      m_pTextBreak->SetLineWidth(m_Param.fPlateHeight);
    } else {
      m_pTextBreak->SetLineWidth(FDE_PAGEWIDTH_MAX);
    }
  } else {
    if (m_Param.dwMode & FDE_TEXTEDITMODE_AutoLineWrap) {
      m_pTextBreak->SetLineWidth(m_Param.fPlateWidth);
    } else {
      m_pTextBreak->SetLineWidth(FDE_PAGEWIDTH_MAX);
    }
  }
  m_nPageLineCount = m_Param.nLineCount;
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_CombText) {
    FX_FLOAT fCombWidth =
        m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical
            ? m_Param.fPlateHeight
            : m_Param.fPlateWidth;
    if (m_nLimit > 0) {
      fCombWidth /= m_nLimit;
    }
    m_pTextBreak->SetCombWidth(fCombWidth);
  }
  m_pTextBreak->SetFont(m_Param.pFont);
  m_pTextBreak->SetFontSize(m_Param.fFontSize);
  m_pTextBreak->SetTabWidth(m_Param.fTabWidth, m_Param.bTabEquidistant);
  m_pTextBreak->SetDefaultChar(m_Param.wDefChar);
  m_pTextBreak->SetParagraphBreakChar(m_Param.wLineBreakChar);
  m_pTextBreak->SetCharRotation(m_Param.nCharRotation);
  m_pTextBreak->SetLineBreakTolerance(m_Param.fFontSize * 0.2f);
  m_pTextBreak->SetHorizontalScale(m_Param.nHorzScale);
  m_pTextBreak->SetCharSpace(m_Param.fCharSpace);
}
FX_BOOL CFDE_TxtEdtEngine::ReplaceParagEnd(FX_WCHAR*& lpText,
                                           int32_t& nLength,
                                           FX_BOOL bPreIsCR) {
  for (int32_t i = 0; i < nLength; i++) {
    FX_WCHAR wc = lpText[i];
    switch (wc) {
      case L'\r': {
        lpText[i] = m_wLineEnd;
        bPreIsCR = TRUE;
      } break;
      case L'\n': {
        if (bPreIsCR == TRUE) {
          int32_t nNext = i + 1;
          if (nNext < nLength) {
            FXSYS_memmove(lpText + i, lpText + nNext,
                          (nLength - nNext) * sizeof(FX_WCHAR));
          }
          i--;
          nLength--;
          bPreIsCR = FALSE;
          if (m_bAutoLineEnd) {
            m_nFirstLineEnd = FDE_TXTEDIT_LINEEND_CRLF;
            m_bAutoLineEnd = FALSE;
          }
        } else {
          lpText[i] = m_wLineEnd;
          if (m_bAutoLineEnd) {
            m_nFirstLineEnd = FDE_TXTEDIT_LINEEND_LF;
            m_bAutoLineEnd = FALSE;
          }
        }
      } break;
      default: {
        if (bPreIsCR && m_bAutoLineEnd) {
          m_nFirstLineEnd = FDE_TXTEDIT_LINEEND_CR;
          m_bAutoLineEnd = FALSE;
        }
        bPreIsCR = FALSE;
      } break;
    }
  }
  return bPreIsCR;
}
void CFDE_TxtEdtEngine::RecoverParagEnd(CFX_WideString& wsText) {
  FX_WCHAR wc = (m_nFirstLineEnd == FDE_TXTEDIT_LINEEND_CR) ? L'\n' : L'\r';
  if (m_nFirstLineEnd == FDE_TXTEDIT_LINEEND_CRLF) {
    CFX_ArrayTemplate<int32_t> PosArr;
    int32_t nLength = wsText.GetLength();
    int32_t i = 0;
    FX_WCHAR* lpPos = (FX_WCHAR*)(const FX_WCHAR*)wsText;
    for (i = 0; i < nLength; i++, lpPos++) {
      if (*lpPos == m_wLineEnd) {
        *lpPos = wc;
        PosArr.Add(i);
      }
    }
    const FX_WCHAR* lpSrcBuf = wsText.c_str();
    CFX_WideString wsTemp;
    int32_t nCount = PosArr.GetSize();
    FX_WCHAR* lpDstBuf = wsTemp.GetBuffer(nLength + nCount);
    int32_t nDstPos = 0;
    int32_t nSrcPos = 0;
    for (i = 0; i < nCount; i++) {
      int32_t nPos = PosArr[i];
      int32_t nCopyLen = nPos - nSrcPos + 1;
      FXSYS_memcpy(lpDstBuf + nDstPos, lpSrcBuf + nSrcPos,
                   nCopyLen * sizeof(FX_WCHAR));
      nDstPos += nCopyLen;
      nSrcPos += nCopyLen;
      lpDstBuf[nDstPos] = L'\n';
      nDstPos++;
    }
    if (nSrcPos < nLength) {
      FXSYS_memcpy(lpDstBuf + nDstPos, lpSrcBuf + nSrcPos,
                   (nLength - nSrcPos) * sizeof(FX_WCHAR));
    }
    wsTemp.ReleaseBuffer(nLength + nCount);
    wsText = wsTemp;
  } else {
    int32_t nLength = wsText.GetLength();
    FX_WCHAR* lpBuf = (FX_WCHAR*)(const FX_WCHAR*)wsText;
    for (int32_t i = 0; i < nLength; i++, lpBuf++) {
      if (*lpBuf == m_wLineEnd) {
        *lpBuf = wc;
      }
    }
  }
}
int32_t CFDE_TxtEdtEngine::MovePage2Char(int32_t nIndex) {
  FXSYS_assert(nIndex >= 0);
  FXSYS_assert(nIndex <= m_pTxtBuf->GetTextLength());
  if (m_nCaretPage >= 0) {
    IFDE_TxtEdtPage* pPage = m_PagePtrArray[m_nCaretPage];
    m_Param.pEventSink->On_PageLoad(this, m_nCaretPage, 0);
    int32_t nPageCharStart = pPage->GetCharStart();
    int32_t nPageCharCount = pPage->GetCharCount();
    if (nIndex >= nPageCharStart && nIndex < nPageCharStart + nPageCharCount) {
      m_Param.pEventSink->On_PageUnload(this, m_nCaretPage, 0);
      return m_nCaretPage;
    }
    m_Param.pEventSink->On_PageUnload(this, m_nCaretPage, 0);
  }
  CFDE_TxtEdtParag* pParag = NULL;
  int32_t nLineCount = 0;
  int32_t nParagCount = m_ParagPtrArray.GetSize();
  int32_t i = 0;
  for (i = 0; i < nParagCount; i++) {
    pParag = m_ParagPtrArray[i];
    if (pParag->m_nCharStart <= nIndex &&
        nIndex < (pParag->m_nCharStart + pParag->m_nCharCount)) {
      break;
    }
    nLineCount += pParag->m_nLineCount;
  }
  pParag->LoadParag();
  int32_t nLineStart = -1;
  int32_t nLineCharCount = -1;
  for (i = 0; i < pParag->m_nLineCount; i++) {
    pParag->GetLineRange(i, nLineStart, nLineCharCount);
    if (nLineStart <= nIndex && nIndex < (nLineStart + nLineCharCount)) {
      break;
    }
  }
  FXSYS_assert(i < pParag->m_nLineCount);
  nLineCount += (i + 1);
  m_nCaretPage = (nLineCount - 1) / m_nPageLineCount + 1 - 1;
  m_Param.pEventSink->On_PageChange(this, m_nCaretPage);
  pParag->UnloadParag();
  return m_nCaretPage;
}
void CFDE_TxtEdtEngine::TextPos2ParagPos(int32_t nIndex,
                                         FDE_TXTEDTPARAGPOS& ParagPos) const {
  FXSYS_assert(nIndex >= 0 && nIndex < m_pTxtBuf->GetTextLength());
  int32_t nCount = m_ParagPtrArray.GetSize();
  int32_t nBgn = 0;
  int32_t nMid = 0;
  int32_t nEnd = nCount - 1;
  while (nEnd > nBgn) {
    nMid = (nBgn + nEnd) / 2;
    CFDE_TxtEdtParag* pParag = m_ParagPtrArray[nMid];
    if (nIndex < pParag->m_nCharStart) {
      nEnd = nMid - 1;
    } else if (nIndex >= (pParag->m_nCharStart + pParag->m_nCharCount)) {
      nBgn = nMid + 1;
    } else {
      break;
    }
  }
  if (nBgn == nEnd) {
    nMid = nBgn;
  }
  FXSYS_assert(nIndex >= m_ParagPtrArray[nMid]->m_nCharStart &&
               (nIndex < m_ParagPtrArray[nMid]->m_nCharStart +
                             m_ParagPtrArray[nMid]->m_nCharCount));
  ParagPos.nParagIndex = nMid;
  ParagPos.nCharIndex = nIndex - m_ParagPtrArray[nMid]->m_nCharStart;
}
int32_t CFDE_TxtEdtEngine::MoveForward(FX_BOOL& bBefore) {
  if (m_nCaret == m_pTxtBuf->GetTextLength() - 1) {
    return -1;
  }
  int32_t nCaret = m_nCaret;
  if ((nCaret + 1 < m_pTxtBuf->GetTextLength()) &&
      (m_pTxtBuf->GetCharByIndex(nCaret) == L'\r') &&
      (m_pTxtBuf->GetCharByIndex(nCaret + 1) == L'\n')) {
    nCaret++;
  }
  nCaret++;
  bBefore = TRUE;
  return nCaret;
}
int32_t CFDE_TxtEdtEngine::MoveBackward(FX_BOOL& bBefore) {
  if (m_nCaret == 0) {
    return FALSE;
  }
  int32_t nCaret = m_nCaret;
  if (nCaret > 2 && m_pTxtBuf->GetCharByIndex(nCaret - 1) == L'\n' &&
      m_pTxtBuf->GetCharByIndex(nCaret - 2) == L'\r') {
    nCaret--;
  }
  nCaret--;
  bBefore = TRUE;
  return nCaret;
}
FX_BOOL CFDE_TxtEdtEngine::MoveUp(CFX_PointF& ptCaret) {
  IFDE_TxtEdtPage* pPage = GetPage(m_nCaretPage);
  const CFX_RectF& rtContent = pPage->GetContentsBox();
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical) {
    ptCaret.x = m_rtCaret.left + m_rtCaret.width / 2 - m_Param.fLineSpace;
    ptCaret.y = m_fCaretPosReserve;
    FX_BOOL bLineReserve =
        m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_LineReserve;
    if (ptCaret.x < rtContent.left) {
      if (bLineReserve) {
        if (m_nCaretPage == CountPages() - 1) {
          return FALSE;
        }
      } else {
        if (m_nCaretPage == 0) {
          return FALSE;
        }
      }
      if (bLineReserve) {
        m_nCaretPage++;
      } else {
        m_nCaretPage--;
      }
      m_Param.pEventSink->On_PageChange(this, m_nCaretPage);
      ptCaret.x -= rtContent.left;
      IFDE_TxtEdtPage* pCurPage = GetPage(m_nCaretPage);
      ptCaret.x += pCurPage->GetContentsBox().right();
    }
  } else {
    ptCaret.x = m_fCaretPosReserve;
    ptCaret.y = m_rtCaret.top + m_rtCaret.height / 2 - m_Param.fLineSpace;
    if (ptCaret.y < rtContent.top) {
      if (m_nCaretPage == 0) {
        return FALSE;
      }
      ptCaret.y -= rtContent.top;
      m_nCaretPage--;
      m_Param.pEventSink->On_PageChange(this, m_nCaretPage);
      IFDE_TxtEdtPage* pCurPage = GetPage(m_nCaretPage);
      ptCaret.y += pCurPage->GetContentsBox().bottom();
    }
  }
  return TRUE;
}
FX_BOOL CFDE_TxtEdtEngine::MoveDown(CFX_PointF& ptCaret) {
  IFDE_TxtEdtPage* pPage = GetPage(m_nCaretPage);
  const CFX_RectF& rtContent = pPage->GetContentsBox();
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical) {
    ptCaret.x = m_rtCaret.left + m_rtCaret.width / 2 + m_Param.fLineSpace;
    ptCaret.y = m_fCaretPosReserve;
    if (ptCaret.x >= rtContent.right()) {
      FX_BOOL bLineReserve =
          m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_LineReserve;
      if (bLineReserve) {
        if (m_nCaretPage == 0) {
          return FALSE;
        }
      } else {
        if (m_nCaretPage == CountPages() - 1) {
          return FALSE;
        }
      }
      if (bLineReserve) {
        m_nCaretPage--;
      } else {
        m_nCaretPage++;
      }
      m_Param.pEventSink->On_PageChange(this, m_nCaretPage);
      ptCaret.x -= rtContent.right();
      IFDE_TxtEdtPage* pCurPage = GetPage(m_nCaretPage);
      ptCaret.x += pCurPage->GetContentsBox().left;
    }
  } else {
    ptCaret.x = m_fCaretPosReserve;
    ptCaret.y = m_rtCaret.top + m_rtCaret.height / 2 + m_Param.fLineSpace;
    if (ptCaret.y >= rtContent.bottom()) {
      if (m_nCaretPage == CountPages() - 1) {
        return FALSE;
      }
      ptCaret.y -= rtContent.bottom();
      m_nCaretPage++;
      m_Param.pEventSink->On_PageChange(this, m_nCaretPage);
      IFDE_TxtEdtPage* pCurPage = GetPage(m_nCaretPage);
      ptCaret.y += pCurPage->GetContentsBox().top;
    }
  }
  return TRUE;
}
FX_BOOL CFDE_TxtEdtEngine::MoveLineStart() {
  int32_t nIndex = m_bBefore ? m_nCaret : m_nCaret - 1;
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nIndex, ParagPos);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex];
  pParag->LoadParag();
  int32_t nLineCount = pParag->m_nLineCount;
  int32_t i = 0;
  int32_t nStart = 0;
  int32_t nCount = 0;
  for (; i < nLineCount; i++) {
    pParag->GetLineRange(i, nStart, nCount);
    if (nIndex >= nStart && nIndex < nStart + nCount) {
      break;
    }
  }
  UpdateCaretRect(nStart, TRUE);
  pParag->UnloadParag();
  return TRUE;
}
FX_BOOL CFDE_TxtEdtEngine::MoveLineEnd() {
  int32_t nIndex = m_bBefore ? m_nCaret : m_nCaret - 1;
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nIndex, ParagPos);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex];
  pParag->LoadParag();
  int32_t nLineCount = pParag->m_nLineCount;
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
  FXSYS_assert(nIndex <= GetTextBufLength());
  FX_WCHAR wChar = m_pTxtBuf->GetCharByIndex(nIndex);
  FX_BOOL bBefore = FALSE;
  if (nIndex <= GetTextBufLength()) {
    if (wChar == L'\r') {
      bBefore = TRUE;
    } else if (wChar == L'\n' && nIndex > nStart) {
      bBefore = TRUE;
      nIndex--;
      wChar = m_pTxtBuf->GetCharByIndex(nIndex);
      if (wChar != L'\r') {
        nIndex++;
      }
    }
  }
  UpdateCaretRect(nIndex, bBefore);
  pParag->UnloadParag();
  return TRUE;
}
FX_BOOL CFDE_TxtEdtEngine::MoveParagStart() {
  int32_t nIndex = m_bBefore ? m_nCaret : m_nCaret - 1;
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nIndex, ParagPos);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex];
  UpdateCaretRect(pParag->m_nCharStart, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtEngine::MoveParagEnd() {
  int32_t nIndex = m_bBefore ? m_nCaret : m_nCaret - 1;
  FDE_TXTEDTPARAGPOS ParagPos;
  TextPos2ParagPos(nIndex, ParagPos);
  CFDE_TxtEdtParag* pParag = m_ParagPtrArray[ParagPos.nParagIndex];
  nIndex = pParag->m_nCharStart + pParag->m_nCharCount - 1;
  FX_WCHAR wChar = m_pTxtBuf->GetCharByIndex(nIndex);
  if (wChar == L'\n' && nIndex > 0) {
    nIndex--;
    wChar = m_pTxtBuf->GetCharByIndex(nIndex);
    if (wChar != L'\r') {
      nIndex++;
    }
  }
  UpdateCaretRect(nIndex, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtEngine::MoveHome() {
  UpdateCaretRect(0, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtEngine::MoveEnd() {
  UpdateCaretRect(GetTextBufLength(), TRUE);
  return TRUE;
}
#ifdef FDE_USEFORMATBLOCK
int32_t CFDE_TxtEdtEngine::NormalizeCaretPos(int32_t nIndex,
                                             int32_t nFlags,
                                             FX_BOOL& bBefore) {
  bBefore = TRUE;
  int32_t nBgn = 0, nEnd = 0;
  int32_t nRecord = -1;
  CFDE_TxtEdtField* pField = NULL;
  FX_BOOL bRet = GetFieldBoundary(nIndex, nBgn, nEnd, pField);
  int32_t nDelta = 0;
  if (bRet && !pField->IsFix()) {
    if (nIndex - nBgn < FDE_FORMAT_EDIT_FIELD_HADERSIZE) {
      if (nFlags == FDE_FORMAT_CARET_BACKWARD) {
        CFDE_TxtEdtField* pEditableField = NULL;
        if (FindEditableField(nIndex, nBgn, nEnd, pEditableField, FALSE)) {
          return pEditableField->NormalizeCaretPos(nEnd - nBgn,
                                                   FDE_FORMAT_CARET_BACKWARD) +
                 nBgn;
        }
      }
      nIndex = nBgn + FDE_FORMAT_EDIT_FIELD_HADERSIZE;
    }
    int32_t nRet = pField->NormalizeCaretPos(
        nIndex - nBgn, (FDE_FORMAT_CARET_DIRECTION)nFlags);
    if (nRet >= 0) {
      return nRet + nBgn;
    }
    if (nRet == -2) {
      int32_t nEditablePosBgn = 0, nEditablePosEnd = 0;
      pField->GetEditableRange(nEditablePosBgn, nEditablePosEnd);
      nRecord = nBgn + nEditablePosBgn;
      nFlags = FDE_FORMAT_CARET_BACKWARD;
    } else {
      FXSYS_assert(nRet == -1);
      int32_t nEditablePosBgn = 0, nEditablePosEnd = 0;
      pField->GetEditableRange(nEditablePosBgn, nEditablePosEnd);
      nRecord = nBgn + nEditablePosEnd;
      nFlags = FDE_FORMAT_CARET_FORWARD;
    }
  } else if (!bRet) {
    nDelta = FDE_FORMAT_EDIT_FIELD_HADERSIZE - FDE_FORMAT_EDIT_FIELD_TAILSIZE;
  }
  switch (nFlags) {
    case FDE_FORMAT_CARET_FORWARD: {
      if (FindEditableField(nIndex, nBgn, nEnd, pField)) {
        return pField->NormalizeCaretPos(FDE_FORMAT_EDIT_FIELD_HADERSIZE,
                                         FDE_FORMAT_CARET_FORWARD) +
               nBgn;
      } else {
        if (nRecord != -1) {
          return nRecord;
        }
        bRet = FindEditableField(nIndex, nBgn, nEnd, pField, FALSE);
        FXSYS_assert(bRet);
        return pField->NormalizeCaretPos(nEnd - nBgn,
                                         FDE_FORMAT_CARET_BACKWARD) +
               nBgn;
      }
    } break;
    case FDE_FORMAT_CARET_MIDDLE: {
      int32_t nBgn1 = 0, nEnd1 = 0, nBgn2 = 0, nEnd2 = 0;
      CFDE_TxtEdtField* pEditableField1 = NULL;
      CFDE_TxtEdtField* pEditableField2 = NULL;
      FX_BOOL bRet1 =
          FindEditableField(nIndex, nBgn1, nEnd1, pEditableField1, FALSE);
      FX_BOOL bRet2 = FindEditableField(nIndex, nBgn2, nEnd2, pEditableField2);
      if (bRet1 == FALSE) {
        FXSYS_assert(bRet2);
        return pEditableField2->NormalizeCaretPos(
                   FDE_FORMAT_EDIT_FIELD_HADERSIZE, FDE_FORMAT_CARET_FORWARD) +
               nBgn2;
      } else if (bRet2 == FALSE) {
        FXSYS_assert(bRet1);
        return pEditableField1->NormalizeCaretPos(nEnd1 - nBgn1,
                                                  FDE_FORMAT_CARET_BACKWARD) +
               nBgn1;
      } else {
        int32_t nEditablePosBgn = 0, nEditablePosEnd = 0;
        if (nIndex - nEnd1 < nBgn2 + nDelta - nIndex) {
          pEditableField1->GetEditableRange(nEditablePosBgn, nEditablePosEnd);
          return nEditablePosEnd + nBgn1;
        } else {
          pEditableField2->GetEditableRange(nEditablePosBgn, nEditablePosEnd);
          return nEditablePosBgn + nBgn2;
        }
      }
    } break;
    case FDE_FORMAT_CARET_BACKWARD: {
      if (FindEditableField(nIndex, nBgn, nEnd, pField, FALSE)) {
        return pField->NormalizeCaretPos(nEnd - nBgn,
                                         FDE_FORMAT_CARET_BACKWARD) +
               nBgn;
      } else {
        if (nRecord != -1) {
          return nRecord;
        }
        bRet = FindEditableField(nIndex, nBgn, nEnd, pField);
        FXSYS_assert(bRet);
        return pField->NormalizeCaretPos(FDE_FORMAT_EDIT_FIELD_HADERSIZE,
                                         FDE_FORMAT_CARET_FORWARD) +
               nBgn;
      }
    } break;
    default:
      FXSYS_assert(0);
      return nIndex;
  }
}
FX_BOOL CFDE_TxtEdtEngine::GetFieldBoundary(int32_t nIndex,
                                            int32_t& nBgn,
                                            int32_t& nEnd,
                                            CFDE_TxtEdtField*& pField) {
  CFDE_TxtEdtBufIter* pIter =
      new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)m_pTxtBuf, FALSE);
  pIter->SetAt(nIndex);
  FX_BOOL bFind = FALSE;
  do {
    FX_WCHAR wc = pIter->GetChar();
    if (wc == FDE_TXTEDT_FORMATBLOCK_END) {
      nEnd = pIter->GetAt();
      bFind = TRUE;
      nIndex--;
      break;
    }
    if (wc == FDE_TXTEDT_FORMATBLOCK_BGN) {
      pIter->Release();
      return FALSE;
    }
  } while (pIter->Next());
  if (!bFind) {
    pIter->Release();
    return FALSE;
  }
  pIter->SetAt(nIndex);
  do {
    FX_WCHAR wc = pIter->GetChar();
    if (wc == FDE_TXTEDT_FORMATBLOCK_BGN) {
      nBgn = pIter->GetAt();
      pIter->Next();
      FX_DWORD dwPre = (FX_DWORD)pIter->GetChar();
      pIter->Next();
      FX_DWORD dwCur = (FX_DWORD)pIter->GetChar();
      pField = (CFDE_TxtEdtField*)((dwCur << 16) | dwPre);
      pIter->Release();
      return TRUE;
    }
    if (wc == FDE_TXTEDT_FORMATBLOCK_END) {
      pIter->Release();
      return FALSE;
    }
  } while (pIter->Next(TRUE));
  pIter->Release();
  return FALSE;
}
FX_BOOL CFDE_TxtEdtEngine::FindEditableField(int32_t nIndex,
                                             int32_t& nBgn,
                                             int32_t& nEnd,
                                             CFDE_TxtEdtField*& pField,
                                             FX_BOOL bForward) {
  FX_WCHAR wcFirst = FDE_TXTEDT_FORMATBLOCK_BGN;
  FX_WCHAR wcSecond = FDE_TXTEDT_FORMATBLOCK_END;
  if (!bForward) {
    wcFirst = FDE_TXTEDT_FORMATBLOCK_END;
    wcSecond = FDE_TXTEDT_FORMATBLOCK_BGN;
  }
  CFDE_TxtEdtBufIter* pIter =
      new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)m_pTxtBuf, FALSE);
  pIter->SetAt(nIndex);
  int32_t bFind = FALSE;
  do {
    FX_WCHAR wc = pIter->GetChar();
    if (wc == wcFirst) {
      nBgn = pIter->GetAt();
      bFind = TRUE;
      break;
    }
  } while (pIter->Next(!bForward));
  if (!bFind) {
    pIter->Release();
    return FALSE;
  }
  bFind = FALSE;
  do {
    FX_WCHAR wc = pIter->GetChar();
    if (wc == wcSecond) {
      nEnd = pIter->GetAt();
      bFind = TRUE;
      break;
    }
  } while (pIter->Next(!bForward));
  FXSYS_assert(bFind);
  if (!bForward) {
    int32_t nTemp = nBgn;
    nBgn = nEnd;
    nEnd = nTemp;
  }
  pIter->SetAt(nBgn + 1);
  FX_DWORD dwPre = (FX_DWORD)pIter->GetChar();
  pIter->Next();
  FX_DWORD dwCur = (FX_DWORD)pIter->GetChar();
  pField = (CFDE_TxtEdtField*)((dwCur << 16) | dwPre);
  pIter->Release();
  if (!pField->IsFix()) {
    return TRUE;
  }
  return FindEditableField((bForward ? nEnd : nBgn), nBgn, nEnd, pField,
                           bForward);
}
FX_BOOL CFDE_TxtEdtEngine::Move2NextEditableField(int32_t nIndex,
                                                  FX_BOOL bForward,
                                                  FX_BOOL bSelect) {
  if (m_SelRangePtrArr.GetSize() > 0) {
    ClearSelection();
    m_Param.pEventSink->On_SelChanged(this);
  }
  int32_t nBgn = 0, nEnd = 0;
  CFDE_TxtEdtField* pField = NULL;
  FX_BOOL bRet = FindEditableField(nIndex, nBgn, nEnd, pField, bForward);
  if (!bRet) {
    return FALSE;
  }
  int32_t nEditableBgn = 0, nEditableEnd = 0;
  pField->GetEditableRange(nEditableBgn, nEditableEnd);
  nEditableBgn += nBgn;
  nEditableEnd += nBgn;
  if (bSelect) {
    int32_t nRangeCount = nEditableEnd - nEditableBgn;
    if (nRangeCount > 0) {
      AddSelRange(nEditableBgn, nEditableEnd - nEditableBgn);
    }
  }
  SetCaretPos(nEditableEnd, TRUE);
  return TRUE;
}
int32_t CFDE_TxtEdtEngine::GetRealIndex(int32_t nIndex) const {
  CFDE_TxtEdtBufIter* pIter =
      new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)m_pTxtBuf, FALSE);
  pIter->SetAt(0);
  FX_BOOL bInField = FALSE;
  int32_t nFieldBgn = 0;
  int32_t nRealIndex = 0;
  for (int32_t i = 0; i <= nIndex; i++) {
    FX_WCHAR wc = pIter->GetChar();
    if (bInField) {
      if (wc == FDE_TXTEDT_FORMATBLOCK_END) {
        FX_DWORD dwPre = (FX_DWORD)m_pTxtBuf->GetCharByIndex(nFieldBgn + 1);
        FX_DWORD dwCur = (FX_DWORD)m_pTxtBuf->GetCharByIndex(nFieldBgn + 2);
        CFDE_TxtEdtField* pField = (CFDE_TxtEdtField*)((dwCur << 16) | dwPre);
        nRealIndex += pField->GetFieldTextLength();
        bInField = FALSE;
      }
    } else {
      if (wc == FDE_TXTEDT_FORMATBLOCK_BGN) {
        bInField = TRUE;
        nFieldBgn = pIter->GetAt();
      } else {
        nRealIndex++;
      }
    }
    pIter->Next();
  }
  if (!bInField) {
    pIter->Release();
    return nRealIndex;
  }
  pIter->SetAt(nFieldBgn + 1);
  FX_DWORD dwPre = (FX_DWORD)pIter->GetChar();
  pIter->Next();
  FX_DWORD dwCur = (FX_DWORD)pIter->GetChar();
  CFDE_TxtEdtField* pField = (CFDE_TxtEdtField*)((dwCur << 16) | dwPre);
  pIter->Release();
  if (pField->IsFix()) {
    int32_t nDelta = nIndex - nFieldBgn - FDE_FORMAT_EDIT_FIELD_HADERSIZE + 1;
    return nRealIndex + (nDelta > 0 ? nDelta : 0);
  } else {
    return nRealIndex + pField->GetRealIndex(nIndex - nFieldBgn);
  }
}
#endif
FX_BOOL CFDE_TxtEdtEngine::IsFitArea(CFX_WideString& wsText) {
  IFDE_TextOut* pTextOut = IFDE_TextOut::Create();
  pTextOut->SetLineSpace(m_Param.fLineSpace);
  pTextOut->SetFont(m_Param.pFont);
  pTextOut->SetFontSize(m_Param.fFontSize);
  CFX_RectF rcText;
  FXSYS_memset(&rcText, 0, sizeof(rcText));
  FX_DWORD dwStyle = 0;
  if (!(m_Param.dwMode & FDE_TEXTEDITMODE_MultiLines)) {
    dwStyle |= FDE_TTOSTYLE_SingleLine;
  }
  if (m_Param.dwMode & FDE_TEXTEDITMODE_AutoLineWrap) {
    dwStyle |= FDE_TTOSTYLE_LineWrap;
    rcText.width = m_Param.fPlateWidth;
  } else {
    rcText.width = 65535;
  }
  pTextOut->SetStyles(dwStyle);
  wsText += L"\n";
  pTextOut->CalcLogicSize(wsText, wsText.GetLength(), rcText);
  pTextOut->Release();
  wsText.Delete(wsText.GetLength() - 1);
  if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Horz) &&
      (rcText.width > m_Param.fPlateWidth)) {
    return FALSE;
  }
  if ((m_Param.dwMode & FDE_TEXTEDITMODE_LimitArea_Vert) &&
      (rcText.height > m_Param.fLineSpace * m_Param.nLineCount)) {
    return FALSE;
  }
  return TRUE;
}
void CFDE_TxtEdtEngine::UpdateCaretRect(int32_t nIndex, FX_BOOL bBefore) {
  MovePage2Char(nIndex);
  GetCaretRect(m_rtCaret, m_nCaretPage, nIndex, bBefore);
  m_nCaret = nIndex;
  m_bBefore = bBefore;
  if (!m_bBefore) {
    m_nCaret++;
    m_bBefore = TRUE;
  }
  m_fCaretPosReserve = (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical)
                           ? m_rtCaret.top
                           : m_rtCaret.left;
  m_Param.pEventSink->On_CaretChanged(this, m_nCaretPage, 0);
}
void CFDE_TxtEdtEngine::GetCaretRect(CFX_RectF& rtCaret,
                                     int32_t nPageIndex,
                                     int32_t nCaret,
                                     FX_BOOL bBefore) {
  IFDE_TxtEdtPage* pPage = m_PagePtrArray[m_nCaretPage];
  m_Param.pEventSink->On_PageLoad(this, m_nCaretPage, 0);
  FX_BOOL bCombText = m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_CombText;
  int32_t nIndexInpage = nCaret - pPage->GetCharStart();
  if (bBefore && bCombText && nIndexInpage > 0) {
    nIndexInpage--;
    bBefore = FALSE;
  }
  int32_t nBIDILevel = pPage->GetCharRect(nIndexInpage, rtCaret, bCombText);
  if (m_Param.dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical) {
    if ((!FX_IsOdd(nBIDILevel) && !bBefore) ||
        (FX_IsOdd(nBIDILevel) && bBefore)) {
      rtCaret.Offset(0, rtCaret.height - 1.0f);
    }
    if (rtCaret.height == 0 && rtCaret.top > 1.0f) {
      rtCaret.top -= 1.0f;
    }
    rtCaret.height = 1.0f;
  } else {
    if ((!FX_IsOdd(nBIDILevel) && !bBefore) ||
        (FX_IsOdd(nBIDILevel) && bBefore)) {
      rtCaret.Offset(rtCaret.width - 1.0f, 0);
    }
    if (rtCaret.width == 0 && rtCaret.left > 1.0f) {
      rtCaret.left -= 1.0f;
    }
    rtCaret.width = 1.0f;
  }
  m_Param.pEventSink->On_PageUnload(this, m_nCaretPage, 0);
}
void CFDE_TxtEdtEngine::UpdateCaretIndex(const CFX_PointF& ptCaret) {
  IFDE_TxtEdtPage* pPage = m_PagePtrArray[m_nCaretPage];
  m_Param.pEventSink->On_PageLoad(this, m_nCaretPage, 0);
  m_nCaret = pPage->GetCharIndex(ptCaret, m_bBefore);
  GetCaretRect(m_rtCaret, m_nCaretPage, m_nCaret, m_bBefore);
  if (!m_bBefore) {
    m_nCaret++;
    m_bBefore = TRUE;
  }
  m_Param.pEventSink->On_CaretChanged(this, m_nCaretPage);
  m_Param.pEventSink->On_PageUnload(this, m_nCaretPage, 0);
}
FX_BOOL CFDE_TxtEdtEngine::IsSelect() {
  return m_SelRangePtrArr.GetSize() > 0;
}
void CFDE_TxtEdtEngine::DeleteSelect() {
  int32_t nCountRange = CountSelRanges();
  if (nCountRange > 0) {
#ifdef FDE_USEFORMATBLOCK
    int32_t nBlockCount = m_BlockArray.GetSize();
    if (nBlockCount > 0) {
      if (nCountRange > 1) {
        return;
      }
      int32_t nSelStart;
      int32_t nSelCount;
      nSelCount = GetSelRange(0, nSelStart);
      int32_t nSelEnd = nSelStart + nSelCount;
      int32_t nBgn = 0;
      int32_t nEnd = 0;
      CFDE_TxtEdtField* pField = NULL;
      FX_BOOL bInField = GetFieldBoundary(nSelStart, nBgn, nEnd, pField);
      int32_t nCaretInField = nSelStart - nBgn;
      FX_BOOL bBefore = FALSE;
      if (!bInField || pField->IsFix() || nSelEnd > nEnd) {
        return;
      }
      pField->Backup();
      CFX_WideString wsDel;
      int32_t nCaret = 0;
      int32_t nRet =
          pField->Delete(nCaretInField, nSelCount, wsDel, nCaret, bBefore);
      nCaret += nBgn;
      switch (nRet) {
        case FDE_FORMAT_FIELD_DELETE_RET_S:
          break;
        case FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE:
        case FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY:
          return;
        default:
          FXSYS_assert(0);
          break;
      }
      CFX_WideString wsField;
      pField->GetFieldText(wsField);
      if (!m_Param.pEventSink->On_ValidateField(
              this, pField->GetBlockIndex(), pField->GetIndex(), wsField, 0)) {
        pField->Restore();
        return;
      }
      CFX_WideString wsDisplay;
      pField->GetDisplayText(wsDisplay);
      Replace(nBgn, nEnd - nBgn + 1, wsDisplay);
      if (!(m_Param.dwMode & FDE_TEXTEDITMODE_NoRedoUndo)) {
        IFDE_TxtEdtDoRecord* pRecord = new CFDE_TxtEdtDoRecord_FieldDelete(
            this, nSelStart, pField, nCaretInField, nBgn, nEnd - nBgn + 1,
            wsDisplay.GetLength(), wsDel, FALSE);
        CFX_ByteString bsDoRecord;
        pRecord->Serialize(bsDoRecord);
        m_Param.pEventSink->On_AddDoRecord(this, bsDoRecord);
        pRecord->Release();
      }
      SetCaretPos(nSelStart, bBefore);
      return;
    }
#endif
    int32_t nSelStart;
    int32_t nSelCount;
    while (nCountRange > 0) {
      nSelCount = GetSelRange(--nCountRange, nSelStart);
      FDE_LPTXTEDTSELRANGE lpTemp = m_SelRangePtrArr[nCountRange];
      delete lpTemp;
      m_SelRangePtrArr.RemoveAt(nCountRange);
      DeleteRange_DoRecord(nSelStart, nSelCount, TRUE);
    }
    ClearSelection();
    m_Param.pEventSink->On_TextChanged(this, m_ChangeInfo);
    m_Param.pEventSink->On_SelChanged(this);
    SetCaretPos(nSelStart, TRUE);
    return;
  }
}
IFDE_TxtEdtDoRecord* IFDE_TxtEdtDoRecord::Create(
    const CFX_ByteStringC& bsDoRecord) {
  const FX_CHAR* lpBuf = bsDoRecord.GetCStr();
  int32_t nType = *((int32_t*)lpBuf);
  switch (nType) {
    case FDE_TXTEDT_DORECORD_INS:
      return new CFDE_TxtEdtDoRecord_Insert(bsDoRecord);
    case FDE_TXTEDT_DORECORD_DEL:
      return new CFDE_TxtEdtDoRecord_DeleteRange(bsDoRecord);
#ifdef FDE_USEFORMATBLOCK
    case FDE_TXTEDT_DORECORD_FORMATINS:
      return new CFDE_TxtEdtDoRecord_FieldInsert(bsDoRecord);
    case FDE_TXTEDT_DORECORD_FORMATDEL:
      return new CFDE_TxtEdtDoRecord_FieldDelete(bsDoRecord);
    case FDE_TXTEDT_DORECORD_FORMATREP:
      return new CFDE_TxtEdtDoRecord_FieldReplace(bsDoRecord);
#endif
    default:
      break;
  }
  return NULL;
}
CFDE_TxtEdtDoRecord_Insert::CFDE_TxtEdtDoRecord_Insert(
    const CFX_ByteStringC& bsDoRecord) {
  Deserialize(bsDoRecord);
}
CFDE_TxtEdtDoRecord_Insert::CFDE_TxtEdtDoRecord_Insert(
    CFDE_TxtEdtEngine* pEngine,
    int32_t nCaret,
    const FX_WCHAR* lpText,
    int32_t nLength)
    : m_pEngine(pEngine), m_nCaret(nCaret) {
  FXSYS_assert(pEngine);
  FX_WCHAR* lpBuffer = m_wsInsert.GetBuffer(nLength);
  FXSYS_memcpy(lpBuffer, lpText, nLength * sizeof(FX_WCHAR));
  m_wsInsert.ReleaseBuffer();
}
CFDE_TxtEdtDoRecord_Insert::~CFDE_TxtEdtDoRecord_Insert() {}
void CFDE_TxtEdtDoRecord_Insert::Release() {
  delete this;
}
FX_BOOL CFDE_TxtEdtDoRecord_Insert::Undo() {
  if (m_pEngine->IsSelect()) {
    m_pEngine->ClearSelection();
  }
  m_pEngine->Inner_DeleteRange(m_nCaret, m_wsInsert.GetLength());
  FDE_TXTEDTPARAMS& Param = m_pEngine->m_Param;
  m_pEngine->m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Delete;
  m_pEngine->m_ChangeInfo.wsDelete = m_wsInsert;
  Param.pEventSink->On_TextChanged(m_pEngine, m_pEngine->m_ChangeInfo);
  m_pEngine->SetCaretPos(m_nCaret, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtDoRecord_Insert::Redo() {
  m_pEngine->Inner_Insert(m_nCaret, m_wsInsert.c_str(), m_wsInsert.GetLength());
  FDE_TXTEDTPARAMS& Param = m_pEngine->m_Param;
  m_pEngine->m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Insert;
  m_pEngine->m_ChangeInfo.wsDelete = m_wsInsert;
  Param.pEventSink->On_TextChanged(m_pEngine, m_pEngine->m_ChangeInfo);
  m_pEngine->SetCaretPos(m_nCaret, FALSE);
  return TRUE;
}
void CFDE_TxtEdtDoRecord_Insert::Serialize(CFX_ByteString& bsDoRecord) const {
  CFX_ArchiveSaver ArchiveSaver;
  ArchiveSaver << int32_t(FDE_TXTEDT_DORECORD_INS);
  ArchiveSaver << (int32_t)(uintptr_t)m_pEngine;
  ArchiveSaver << m_nCaret;
  ArchiveSaver << m_wsInsert;
  int32_t nLength = ArchiveSaver.GetLength();
  const uint8_t* lpSrcBuf = ArchiveSaver.GetBuffer();
  FX_CHAR* lpDstBuf = bsDoRecord.GetBuffer(nLength);
  FXSYS_memcpy(lpDstBuf, lpSrcBuf, nLength);
  bsDoRecord.ReleaseBuffer(nLength);
}
void CFDE_TxtEdtDoRecord_Insert::Deserialize(
    const CFX_ByteStringC& bsDoRecord) {
  CFX_ArchiveLoader ArchiveLoader((const uint8_t*)bsDoRecord.GetCStr(),
                                  bsDoRecord.GetLength());
  int32_t nType = 0;
  ArchiveLoader >> nType;
  FXSYS_assert(nType == FDE_TXTEDT_DORECORD_INS);
  int32_t nEngine = 0;
  ArchiveLoader >> nEngine;
  m_pEngine = (CFDE_TxtEdtEngine*)(uintptr_t)nEngine;
  ArchiveLoader >> m_nCaret;
  ArchiveLoader >> m_wsInsert;
}
CFDE_TxtEdtDoRecord_DeleteRange::CFDE_TxtEdtDoRecord_DeleteRange(
    const CFX_ByteStringC& bsDoRecord) {
  Deserialize(bsDoRecord);
}
CFDE_TxtEdtDoRecord_DeleteRange::CFDE_TxtEdtDoRecord_DeleteRange(
    CFDE_TxtEdtEngine* pEngine,
    int32_t nIndex,
    int32_t nCaret,
    const CFX_WideString& wsRange,
    FX_BOOL bSel)
    : m_pEngine(pEngine),
      m_bSel(bSel),
      m_nIndex(nIndex),
      m_nCaret(nCaret),
      m_wsRange(wsRange) {
  FXSYS_assert(pEngine);
}
CFDE_TxtEdtDoRecord_DeleteRange::~CFDE_TxtEdtDoRecord_DeleteRange() {}
void CFDE_TxtEdtDoRecord_DeleteRange::Release() {
  delete this;
}
FX_BOOL CFDE_TxtEdtDoRecord_DeleteRange::Undo() {
  if (m_pEngine->IsSelect()) {
    m_pEngine->ClearSelection();
  }
  m_pEngine->Inner_Insert(m_nIndex, m_wsRange.c_str(), m_wsRange.GetLength());
  if (m_bSel) {
    m_pEngine->AddSelRange(m_nIndex, m_wsRange.GetLength());
  }
  FDE_TXTEDTPARAMS& Param = m_pEngine->m_Param;
  m_pEngine->m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Insert;
  m_pEngine->m_ChangeInfo.wsDelete = m_wsRange;
  Param.pEventSink->On_TextChanged(m_pEngine, m_pEngine->m_ChangeInfo);
  m_pEngine->SetCaretPos(m_nCaret, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtDoRecord_DeleteRange::Redo() {
  m_pEngine->Inner_DeleteRange(m_nIndex, m_wsRange.GetLength());
  if (m_bSel) {
    m_pEngine->RemoveSelRange(m_nIndex, m_wsRange.GetLength());
  }
  FDE_TXTEDTPARAMS& Param = m_pEngine->m_Param;
  m_pEngine->m_ChangeInfo.nChangeType = FDE_TXTEDT_TEXTCHANGE_TYPE_Insert;
  m_pEngine->m_ChangeInfo.wsDelete = m_wsRange;
  Param.pEventSink->On_TextChanged(m_pEngine, m_pEngine->m_ChangeInfo);
  m_pEngine->SetCaretPos(m_nIndex, TRUE);
  return TRUE;
}
void CFDE_TxtEdtDoRecord_DeleteRange::Serialize(
    CFX_ByteString& bsDoRecord) const {
  CFX_ArchiveSaver ArchiveSaver;
  ArchiveSaver << int32_t(FDE_TXTEDT_DORECORD_DEL);
  ArchiveSaver << (int32_t)(uintptr_t)m_pEngine;
  ArchiveSaver << (int32_t)m_bSel;
  ArchiveSaver << m_nIndex;
  ArchiveSaver << m_nCaret;
  ArchiveSaver << m_wsRange;
  int32_t nLength = ArchiveSaver.GetLength();
  const uint8_t* lpSrcBuf = ArchiveSaver.GetBuffer();
  FX_CHAR* lpDstBuf = bsDoRecord.GetBuffer(nLength);
  FXSYS_memcpy(lpDstBuf, lpSrcBuf, nLength);
  bsDoRecord.ReleaseBuffer(nLength);
}
void CFDE_TxtEdtDoRecord_DeleteRange::Deserialize(
    const CFX_ByteStringC& bsDoRecord) {
  CFX_ArchiveLoader ArchiveLoader((const uint8_t*)bsDoRecord.GetCStr(),
                                  bsDoRecord.GetLength());
  int32_t nType = 0;
  ArchiveLoader >> nType;
  FXSYS_assert(nType == FDE_TXTEDT_DORECORD_DEL);
  int32_t nEngine = 0;
  ArchiveLoader >> nEngine;
  m_pEngine = (CFDE_TxtEdtEngine*)(uintptr_t)nEngine;
  int32_t iSel = 0;
  ArchiveLoader >> iSel;
  m_bSel = !!iSel;
  ArchiveLoader >> m_nIndex;
  ArchiveLoader >> m_nCaret;
  ArchiveLoader >> m_wsRange;
}
#ifdef FDE_USEFORMATBLOCK
CFDE_TxtEdtDoRecord_FieldInsert::CFDE_TxtEdtDoRecord_FieldInsert(
    const CFX_ByteStringC& bsDoRecord) {
  Deserialize(bsDoRecord);
}
CFDE_TxtEdtDoRecord_FieldInsert::CFDE_TxtEdtDoRecord_FieldInsert(
    CFDE_TxtEdtEngine* pEngine,
    int32_t nCaret,
    CFDE_TxtEdtField* pField,
    int32_t nIndexInField,
    int32_t nFieldBgn,
    int32_t nOldFieldLength,
    int32_t nNewFieldLength,
    const CFX_WideString& wsIns,
    FX_BOOL bSel)
    : m_pEngine(pEngine),
      m_nCaret(nCaret),
      m_pField(pField),
      m_nIndexInField(nIndexInField),
      m_nFieldBgn(nFieldBgn),
      m_nOldFieldLength(nOldFieldLength),
      m_nNewFieldLength(nNewFieldLength),
      m_wsIns(wsIns),
      m_bSel(bSel) {
  FXSYS_assert(pEngine);
  FXSYS_assert(pField);
}
CFDE_TxtEdtDoRecord_FieldInsert::~CFDE_TxtEdtDoRecord_FieldInsert() {}
void CFDE_TxtEdtDoRecord_FieldInsert::Release() {
  delete this;
}
FX_BOOL CFDE_TxtEdtDoRecord_FieldInsert::Undo() {
  CFX_WideString wsDel;
  int32_t nCaret = 0;
  FX_BOOL bBefore = FALSE;
  int32_t nRet = m_pField->Delete(m_nIndexInField, m_wsIns.GetLength(), wsDel,
                                  nCaret, bBefore);
  FXSYS_assert(nRet != FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE &&
               nRet != FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY);
  CFX_WideString wsDisplay;
  m_pField->GetDisplayText(wsDisplay);
  m_pEngine->Replace(m_nFieldBgn, m_nNewFieldLength, wsDisplay);
  m_pEngine->SetCaretPos(m_nCaret, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtDoRecord_FieldInsert::Redo() {
  int32_t nCaret = 0;
  FX_BOOL bBefore = FALSE;
  int32_t nRet = m_pField->Insert(m_nIndexInField, m_wsIns, nCaret, bBefore);
  FXSYS_assert(nRet != FDE_FORMAT_FIELD_INSERT_RET_F_FULL &&
               nRet != FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE);
  CFX_WideString wsDisplay;
  m_pField->GetDisplayText(wsDisplay);
  m_pEngine->Replace(m_nFieldBgn, m_nOldFieldLength, wsDisplay);
  m_pEngine->SetCaretPos(m_nCaret + m_wsIns.GetLength(), TRUE);
  return TRUE;
}
void CFDE_TxtEdtDoRecord_FieldInsert::Serialize(
    CFX_ByteString& bsDoRecord) const {
  CFX_ArchiveSaver ArchiveSaver;
  ArchiveSaver << int32_t(FDE_TXTEDT_DORECORD_FORMATINS);
  ArchiveSaver << int32_t(m_pEngine);
  ArchiveSaver << m_nCaret;
  ArchiveSaver << int32_t(m_pField);
  ArchiveSaver << m_nIndexInField;
  ArchiveSaver << m_nFieldBgn;
  ArchiveSaver << m_nOldFieldLength;
  ArchiveSaver << m_nNewFieldLength;
  ArchiveSaver << m_wsIns;
  ArchiveSaver << m_bSel;
  int32_t nLength = ArchiveSaver.GetLength();
  const uint8_t* lpSrcBuf = ArchiveSaver.GetBuffer();
  FX_CHAR* lpDstBuf = bsDoRecord.GetBuffer(nLength);
  FXSYS_memcpy(lpDstBuf, lpSrcBuf, nLength);
  bsDoRecord.ReleaseBuffer(nLength);
}
void CFDE_TxtEdtDoRecord_FieldInsert::Deserialize(
    const CFX_ByteStringC& bsDoRecord) {
  CFX_ArchiveLoader ArchiveLoader((const uint8_t*)bsDoRecord.GetCStr(),
                                  bsDoRecord.GetLength());
  int32_t nType = 0;
  ArchiveLoader >> nType;
  FXSYS_assert(nType == FDE_TXTEDT_DORECORD_FORMATINS);
  int32_t nEngine = 0;
  ArchiveLoader >> nEngine;
  m_pEngine = (CFDE_TxtEdtEngine*)(uintptr_t)nEngine;
  ArchiveLoader >> m_nCaret;
  int32_t nField = 0;
  ArchiveLoader >> nField;
  m_pField = (CFDE_TxtEdtField*)nField;
  ArchiveLoader >> m_nIndexInField;
  ArchiveLoader >> m_nFieldBgn;
  ArchiveLoader >> m_nOldFieldLength;
  ArchiveLoader >> m_nNewFieldLength;
  ArchiveLoader >> m_wsIns;
  ArchiveLoader >> m_bSel;
}
CFDE_TxtEdtDoRecord_FieldDelete::CFDE_TxtEdtDoRecord_FieldDelete(
    const CFX_ByteStringC& bsDoRecord) {
  Deserialize(bsDoRecord);
}
CFDE_TxtEdtDoRecord_FieldDelete::CFDE_TxtEdtDoRecord_FieldDelete(
    CFDE_TxtEdtEngine* pEngine,
    int32_t nCaret,
    CFDE_TxtEdtField* pField,
    int32_t nIndexInField,
    int32_t nFieldBgn,
    int32_t nOldLength,
    int32_t nNewLength,
    const CFX_WideString& wsDel,
    FX_BOOL bSel)
    : m_pEngine(pEngine),
      m_nCaret(nCaret),
      m_pField(pField),
      m_nIndexInField(nIndexInField),
      m_nFieldBgn(nFieldBgn),
      m_nOldFieldLength(nOldLength),
      m_nNewFieldLength(nNewLength),
      m_wsDel(wsDel),
      m_bSel(bSel) {
  FXSYS_assert(m_pEngine);
  FXSYS_assert(m_pField);
}
CFDE_TxtEdtDoRecord_FieldDelete::~CFDE_TxtEdtDoRecord_FieldDelete() {}
void CFDE_TxtEdtDoRecord_FieldDelete::Release() {
  delete this;
}
FX_BOOL CFDE_TxtEdtDoRecord_FieldDelete::Undo() {
  int32_t nCaret = 0;
  FX_BOOL bBefore = FALSE;
  int32_t nRet = m_pField->Insert(m_nIndexInField, m_wsDel, nCaret, bBefore);
  FXSYS_assert(nRet != FDE_FORMAT_FIELD_INSERT_RET_F_FULL &&
               nRet != FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE);
  CFX_WideString wsDisplay;
  m_pField->GetDisplayText(wsDisplay);
  m_pEngine->Replace(m_nFieldBgn, m_nNewFieldLength, wsDisplay);
  m_pEngine->SetCaretPos(m_nCaret, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtDoRecord_FieldDelete::Redo() {
  int32_t nCaret = 0;
  FX_BOOL bBefore = 0;
  CFX_WideString wsDel;
  int32_t nRet = m_pField->Delete(m_nIndexInField, m_wsDel.GetLength(), wsDel,
                                  nCaret, bBefore);
  FXSYS_assert(nRet != FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE &&
               nRet != FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY);
  CFX_WideString wsDisplay;
  m_pField->GetDisplayText(wsDisplay);
  m_pEngine->Replace(m_nFieldBgn, m_nOldFieldLength, wsDisplay);
  m_pEngine->SetCaretPos(m_nCaret - m_wsDel.GetLength(), TRUE);
  return TRUE;
}
void CFDE_TxtEdtDoRecord_FieldDelete::Serialize(
    CFX_ByteString& bsDoRecord) const {
  CFX_ArchiveSaver ArchiveSaver;
  ArchiveSaver << int32_t(FDE_TXTEDT_DORECORD_FORMATDEL);
  ArchiveSaver << int32_t(m_pEngine);
  ArchiveSaver << m_nCaret;
  ArchiveSaver << int32_t(m_pField);
  ArchiveSaver << m_nIndexInField;
  ArchiveSaver << m_nFieldBgn;
  ArchiveSaver << m_nOldFieldLength;
  ArchiveSaver << m_nNewFieldLength;
  ArchiveSaver << m_wsDel;
  ArchiveSaver << m_bSel;
  int32_t nLength = ArchiveSaver.GetLength();
  const uint8_t* lpSrcBuf = ArchiveSaver.GetBuffer();
  FX_CHAR* lpDstBuf = bsDoRecord.GetBuffer(nLength);
  FXSYS_memcpy(lpDstBuf, lpSrcBuf, nLength);
  bsDoRecord.ReleaseBuffer(nLength);
}
void CFDE_TxtEdtDoRecord_FieldDelete::Deserialize(
    const CFX_ByteStringC& bsDoRecord) {
  CFX_ArchiveLoader ArchiveLoader((const uint8_t*)bsDoRecord.GetCStr(),
                                  bsDoRecord.GetLength());
  int32_t nType = 0;
  ArchiveLoader >> nType;
  FXSYS_assert(nType == FDE_TXTEDT_DORECORD_FORMATDEL);
  int32_t nEngine = 0;
  ArchiveLoader >> nEngine;
  m_pEngine = (CFDE_TxtEdtEngine*)(uintptr_t)nEngine;
  ArchiveLoader >> m_nCaret;
  int32_t nField = 0;
  ArchiveLoader >> nField;
  m_pField = (CFDE_TxtEdtField*)nField;
  ArchiveLoader >> m_nIndexInField;
  ArchiveLoader >> m_nFieldBgn;
  ArchiveLoader >> m_nOldFieldLength;
  ArchiveLoader >> m_nNewFieldLength;
  ArchiveLoader >> m_wsDel;
  ArchiveLoader >> m_bSel;
}
CFDE_TxtEdtDoRecord_FieldReplace::CFDE_TxtEdtDoRecord_FieldReplace(
    const CFX_ByteStringC& bsDoRecord) {
  Deserialize(bsDoRecord);
}
CFDE_TxtEdtDoRecord_FieldReplace::CFDE_TxtEdtDoRecord_FieldReplace(
    CFDE_TxtEdtEngine* pEngine,
    int32_t nCaret,
    int32_t nNewCaret,
    CFDE_TxtEdtField* pField,
    int32_t nIndexInField,
    int32_t nFieldBgn,
    int32_t nFieldNewLength,
    const CFX_WideString& wsDel,
    const CFX_WideString& wsIns,
    FX_BOOL bSel)
    : m_pEngine(pEngine),
      m_nCaret(nCaret),
      m_nNewCaret(nNewCaret),
      m_pField(pField),
      m_nIndexInField(nIndexInField),
      m_nFieldBgn(nFieldBgn),
      m_nFieldNewLength(nFieldNewLength),
      m_wsDel(wsDel),
      m_wsIns(wsIns),
      m_bSel(bSel) {
  FXSYS_assert(m_pEngine);
  FXSYS_assert(m_pField);
}
CFDE_TxtEdtDoRecord_FieldReplace::~CFDE_TxtEdtDoRecord_FieldReplace() {}
void CFDE_TxtEdtDoRecord_FieldReplace::Release() {
  delete this;
}
FX_BOOL CFDE_TxtEdtDoRecord_FieldReplace::Undo() {
  CFX_WideString wsDel;
  int32_t nCaret = 0;
  FX_BOOL bBefore = FALSE;
  int32_t nRet = m_pField->Replace(m_nIndexInField, m_wsIns.GetLength(),
                                   m_wsDel, wsDel, nCaret, bBefore);
  FXSYS_assert(nRet != FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE &&
               nRet != FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY);
  CFX_WideString wsDisplay;
  m_pField->GetDisplayText(wsDisplay);
  m_pEngine->Replace(m_nFieldBgn, m_nFieldNewLength, wsDisplay);
  m_pEngine->SetCaretPos(m_nCaret, TRUE);
  return TRUE;
}
FX_BOOL CFDE_TxtEdtDoRecord_FieldReplace::Redo() {
  CFX_WideString wsDel;
  int32_t nCaret = 0;
  FX_BOOL bBefore = FALSE;
  int32_t nRet = m_pField->Replace(m_nIndexInField, m_wsDel.GetLength(),
                                   m_wsIns, wsDel, nCaret, bBefore);
  FXSYS_assert(nRet != FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE &&
               nRet != FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY);
  CFX_WideString wsDisplay;
  m_pField->GetDisplayText(wsDisplay);
  m_pEngine->Replace(m_nFieldBgn, m_nFieldNewLength, wsDisplay);
  m_pEngine->SetCaretPos(m_nNewCaret, TRUE);
  return TRUE;
}
void CFDE_TxtEdtDoRecord_FieldReplace::Serialize(
    CFX_ByteString& bsDoRecord) const {
  CFX_ArchiveSaver ArchiveSaver;
  ArchiveSaver << int32_t(FDE_TXTEDT_DORECORD_FORMATREP);
  ArchiveSaver << int32_t(m_pEngine);
  ArchiveSaver << m_nCaret;
  ArchiveSaver << m_nNewCaret;
  ArchiveSaver << int32_t(m_pField);
  ArchiveSaver << m_nIndexInField;
  ArchiveSaver << m_nFieldBgn;
  ArchiveSaver << m_nFieldNewLength;
  ArchiveSaver << m_wsDel;
  ArchiveSaver << m_wsIns;
  ArchiveSaver << m_bSel;
  int32_t nLength = ArchiveSaver.GetLength();
  const uint8_t* lpSrcBuf = ArchiveSaver.GetBuffer();
  FX_CHAR* lpDstBuf = bsDoRecord.GetBuffer(nLength);
  FXSYS_memcpy(lpDstBuf, lpSrcBuf, nLength);
  bsDoRecord.ReleaseBuffer(nLength);
}
void CFDE_TxtEdtDoRecord_FieldReplace::Deserialize(
    const CFX_ByteStringC& bsDoRecord) {
  CFX_ArchiveLoader ArchiveLoader((const uint8_t*)bsDoRecord.GetCStr(),
                                  bsDoRecord.GetLength());
  int32_t nType = 0;
  ArchiveLoader >> nType;
  FXSYS_assert(nType == FDE_TXTEDT_DORECORD_FORMATREP);
  int32_t nEngine = 0;
  ArchiveLoader >> nEngine;
  m_pEngine = (CFDE_TxtEdtEngine*)(uintptr_t)nEngine;
  ArchiveLoader >> m_nCaret;
  ArchiveLoader >> m_nNewCaret;
  int32_t nField = 0;
  ArchiveLoader >> nField;
  m_pField = (CFDE_TxtEdtField*)nField;
  ArchiveLoader >> m_nIndexInField;
  ArchiveLoader >> m_nFieldBgn;
  ArchiveLoader >> m_nFieldNewLength;
  ArchiveLoader >> m_wsDel;
  ArchiveLoader >> m_wsIns;
  ArchiveLoader >> m_bSel;
}
#endif
