// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_txtedtparag.h"

#include <memory>
#include <vector>

#include "core/fxcrt/ifx_chariter.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_txtedtbuf.h"
#include "xfa/fde/cfde_txtedtengine.h"
#include "xfa/fde/ifde_txtedtengine.h"
#include "xfa/fgas/layout/cfx_txtbreak.h"

CFDE_TxtEdtParag::CFDE_TxtEdtParag(CFDE_TxtEdtEngine* pEngine)
    : m_nCharStart(0),
      m_nCharCount(0),
      m_nLineCount(0),
      m_lpData(nullptr),
      m_pEngine(pEngine) {
  ASSERT(m_pEngine);
}

CFDE_TxtEdtParag::~CFDE_TxtEdtParag() {
  if (m_lpData)
    FX_Free(m_lpData);
}

void CFDE_TxtEdtParag::LoadParag() {
  if (m_lpData) {
    m_lpData[0]++;
    return;
  }
  CFX_TxtBreak* pTxtBreak = m_pEngine->GetTextBreak();
  CFDE_TxtEdtBuf* pTxtBuf = m_pEngine->GetTextBuf();
  const FDE_TXTEDTPARAMS* pParam = m_pEngine->GetEditParams();
  wchar_t wcAlias = 0;
  if (pParam->dwMode & FDE_TEXTEDITMODE_Password)
    wcAlias = m_pEngine->GetAliasChar();

  auto pIter = pdfium::MakeUnique<CFDE_TxtEdtBuf::Iterator>(
      static_cast<CFDE_TxtEdtBuf*>(pTxtBuf), wcAlias);
  pIter->SetAt(m_nCharStart);
  int32_t nEndIndex = m_nCharStart + m_nCharCount;
  std::vector<int32_t> LineBaseArr;
  bool bReload = false;
  CFX_BreakType dwBreakStatus = CFX_BreakType::None;
  do {
    if (bReload) {
      dwBreakStatus = pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
    } else {
      wchar_t wAppend = pIter->GetChar();
      dwBreakStatus = pTxtBreak->AppendChar(wAppend);
    }
    if (pIter->GetAt() + 1 == nEndIndex &&
        CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
      dwBreakStatus = pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
    }
    if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
      int32_t nCount = pTxtBreak->CountBreakPieces();
      int32_t nTotal = 0;
      for (int32_t j = 0; j < nCount; j++) {
        const CFX_BreakPiece* Piece = pTxtBreak->GetBreakPieceUnstable(j);
        nTotal += Piece->GetLength();
      }
      LineBaseArr.push_back(nTotal);
      pTxtBreak->ClearBreakPieces();
    }
    if (pIter->GetAt() + 1 == nEndIndex &&
        dwBreakStatus == CFX_BreakType::Line) {
      bReload = true;
      pIter->Next(true);
    }
  } while (pIter->Next(false) && (pIter->GetAt() < nEndIndex));
  pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
  pTxtBreak->ClearBreakPieces();
  int32_t nLineCount = pdfium::CollectionSize<int32_t>(LineBaseArr);
  m_nLineCount = nLineCount;
  if (m_lpData)
    m_lpData = FX_Realloc(int32_t, m_lpData, nLineCount + 1);
  else
    m_lpData = FX_Alloc(int32_t, nLineCount + 1);

  int32_t* pIntArr = m_lpData;
  pIntArr[0] = 1;
  m_nLineCount = nLineCount;
  pIntArr++;
  for (int32_t j = 0; j < nLineCount; j++, pIntArr++)
    *pIntArr = LineBaseArr[j];
}

void CFDE_TxtEdtParag::UnloadParag() {
  m_lpData[0]--;
  ASSERT(m_lpData[0] >= 0);
  if (m_lpData[0] == 0) {
    FX_Free(m_lpData);
    m_lpData = nullptr;
  }
}

void CFDE_TxtEdtParag::CalcLines() {
  CFX_TxtBreak* pTxtBreak = m_pEngine->GetTextBreak();
  CFDE_TxtEdtBuf* pTxtBuf = m_pEngine->GetTextBuf();
  int32_t nCount = 0;
  CFX_BreakType dwBreakStatus = CFX_BreakType::None;
  int32_t nEndIndex = m_nCharStart + m_nCharCount;
  auto pIter = pdfium::MakeUnique<CFDE_TxtEdtBuf::Iterator>(
      static_cast<CFDE_TxtEdtBuf*>(pTxtBuf));
  pIter->SetAt(m_nCharStart);
  bool bReload = false;
  do {
    if (bReload) {
      dwBreakStatus = pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
    } else {
      wchar_t wAppend = pIter->GetChar();
      dwBreakStatus = pTxtBreak->AppendChar(wAppend);
    }
    if (pIter->GetAt() + 1 == nEndIndex &&
        CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
      dwBreakStatus = pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
    }
    if (!CFX_BreakTypeNoneOrPiece(dwBreakStatus)) {
      nCount++;
      pTxtBreak->ClearBreakPieces();
    }
    if (pIter->GetAt() + 1 == nEndIndex &&
        dwBreakStatus == CFX_BreakType::Line) {
      bReload = true;
      pIter->Next(true);
    }
  } while (pIter->Next(false) && (pIter->GetAt() < nEndIndex));
  pTxtBreak->EndBreak(CFX_BreakType::Paragraph);
  pTxtBreak->ClearBreakPieces();
  m_nLineCount = nCount;
}

void CFDE_TxtEdtParag::GetLineRange(int32_t nLineIndex,
                                    int32_t& nStart,
                                    int32_t& nCount) const {
  int32_t* pLineBaseArr = m_lpData;
  ASSERT(nLineIndex < m_nLineCount);
  nStart = m_nCharStart;
  pLineBaseArr++;
  for (int32_t i = 0; i < nLineIndex; i++) {
    nStart += *pLineBaseArr;
    pLineBaseArr++;
  }
  nCount = *pLineBaseArr;
}
