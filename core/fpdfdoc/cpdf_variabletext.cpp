// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_variabletext.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfdoc/cline.h"
#include "core/fpdfdoc/cpvt_section.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fpdfdoc/cpvt_wordinfo.h"
#include "core/fpdfdoc/csection.h"
#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/fx_codepage.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

const float kFontScale = 0.001f;
const uint8_t kReturnLength = 1;
const float kScalePercent = 0.01f;

const uint8_t gFontSizeSteps[] = {4,  6,  8,   9,   10,  12,  14, 18, 20,
                                  25, 30, 35,  40,  45,  50,  55, 60, 70,
                                  80, 90, 100, 110, 120, 130, 144};

}  // namespace

CPDF_VariableText::Provider::Provider(IPVT_FontMap* pFontMap)
    : m_pFontMap(pFontMap) {
  ASSERT(m_pFontMap);
}

CPDF_VariableText::Provider::~Provider() {}

int32_t CPDF_VariableText::Provider::GetCharWidth(int32_t nFontIndex,
                                                  uint16_t word) {
  if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex)) {
    uint32_t charcode = pPDFFont->CharCodeFromUnicode(word);
    if (charcode != CPDF_Font::kInvalidCharCode)
      return pPDFFont->GetCharWidthF(charcode);
  }
  return 0;
}

int32_t CPDF_VariableText::Provider::GetTypeAscent(int32_t nFontIndex) {
  if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex))
    return pPDFFont->GetTypeAscent();
  return 0;
}

int32_t CPDF_VariableText::Provider::GetTypeDescent(int32_t nFontIndex) {
  if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex))
    return pPDFFont->GetTypeDescent();
  return 0;
}

int32_t CPDF_VariableText::Provider::GetWordFontIndex(uint16_t word,
                                                      int32_t charset,
                                                      int32_t nFontIndex) {
  if (CPDF_Font* pDefFont = m_pFontMap->GetPDFFont(0)) {
    if (pDefFont->CharCodeFromUnicode(word) != CPDF_Font::kInvalidCharCode)
      return 0;
  }
  if (CPDF_Font* pSysFont = m_pFontMap->GetPDFFont(1)) {
    if (pSysFont->CharCodeFromUnicode(word) != CPDF_Font::kInvalidCharCode)
      return 1;
  }
  return -1;
}

bool CPDF_VariableText::Provider::IsLatinWord(uint16_t word) {
  return (word >= 0x61 && word <= 0x7A) || (word >= 0x41 && word <= 0x5A) ||
         word == 0x2D || word == 0x27;
}

int32_t CPDF_VariableText::Provider::GetDefaultFontIndex() {
  return 0;
}

CPDF_VariableText::Iterator::Iterator(CPDF_VariableText* pVT)
    : m_CurPos(-1, -1, -1), m_pVT(pVT) {}

CPDF_VariableText::Iterator::~Iterator() {}

void CPDF_VariableText::Iterator::SetAt(int32_t nWordIndex) {
  m_CurPos = m_pVT->WordIndexToWordPlace(nWordIndex);
}

void CPDF_VariableText::Iterator::SetAt(const CPVT_WordPlace& place) {
  ASSERT(m_pVT);
  m_CurPos = place;
}

bool CPDF_VariableText::Iterator::NextWord() {
  if (m_CurPos == m_pVT->GetEndWordPlace())
    return false;

  m_CurPos = m_pVT->GetNextWordPlace(m_CurPos);
  return true;
}

bool CPDF_VariableText::Iterator::PrevWord() {
  if (m_CurPos == m_pVT->GetBeginWordPlace())
    return false;

  m_CurPos = m_pVT->GetPrevWordPlace(m_CurPos);
  return true;
}

bool CPDF_VariableText::Iterator::NextLine() {
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex))
    return false;

  CSection* pSection = m_pVT->m_SectionArray[m_CurPos.nSecIndex].get();
  if (m_CurPos.nLineIndex <
      pdfium::CollectionSize<int32_t>(pSection->m_LineArray) - 1) {
    m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex, m_CurPos.nLineIndex + 1, -1);
    return true;
  }
  if (m_CurPos.nSecIndex <
      pdfium::CollectionSize<int32_t>(m_pVT->m_SectionArray) - 1) {
    m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex + 1, 0, -1);
    return true;
  }
  return false;
}

bool CPDF_VariableText::Iterator::PrevLine() {
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex))
    return false;

  if (m_CurPos.nLineIndex > 0) {
    m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex, m_CurPos.nLineIndex - 1, -1);
    return true;
  }
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex - 1))
    return false;

  CSection* pLastSection = m_pVT->m_SectionArray[m_CurPos.nSecIndex - 1].get();
  m_CurPos = CPVT_WordPlace(
      m_CurPos.nSecIndex - 1,
      pdfium::CollectionSize<int32_t>(pLastSection->m_LineArray) - 1, -1);
  return true;
}

bool CPDF_VariableText::Iterator::NextSection() {
  if (m_CurPos.nSecIndex <
      pdfium::CollectionSize<int32_t>(m_pVT->m_SectionArray) - 1) {
    m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex + 1, 0, -1);
    return true;
  }
  return false;
}

bool CPDF_VariableText::Iterator::PrevSection() {
  ASSERT(m_pVT);
  if (m_CurPos.nSecIndex > 0) {
    m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex - 1, 0, -1);
    return true;
  }
  return false;
}

bool CPDF_VariableText::Iterator::GetWord(CPVT_Word& word) const {
  word.WordPlace = m_CurPos;
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex))
    return false;

  CSection* pSection = m_pVT->m_SectionArray[m_CurPos.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_LineArray, m_CurPos.nLineIndex) ||
      !pdfium::IndexInBounds(pSection->m_WordArray, m_CurPos.nWordIndex)) {
    return false;
  }

  CPVT_WordInfo* pWord = pSection->m_WordArray[m_CurPos.nWordIndex].get();
  word.Word = pWord->Word;
  word.nCharset = pWord->nCharset;
  word.fWidth = m_pVT->GetWordWidth(*pWord);
  word.ptWord = m_pVT->InToOut(
      CFX_PointF(pWord->fWordX + pSection->m_SecInfo.rcSection.left,
                 pWord->fWordY + pSection->m_SecInfo.rcSection.top));
  word.fAscent = m_pVT->GetWordAscent(*pWord);
  word.fDescent = m_pVT->GetWordDescent(*pWord);
  if (pWord->pWordProps)
    word.WordProps = *pWord->pWordProps;
  word.nFontIndex = m_pVT->GetWordFontIndex(*pWord);
  word.fFontSize = m_pVT->GetWordFontSize(*pWord);
  return true;
}

bool CPDF_VariableText::Iterator::SetWord(const CPVT_Word& word) {
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex))
    return false;

  CSection* pSection = m_pVT->m_SectionArray[m_CurPos.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_WordArray, m_CurPos.nWordIndex))
    return false;

  CPVT_WordInfo* pWord = pSection->m_WordArray[m_CurPos.nWordIndex].get();
  if (pWord->pWordProps)
    *pWord->pWordProps = word.WordProps;
  return true;
}

bool CPDF_VariableText::Iterator::GetLine(CPVT_Line& line) const {
  ASSERT(m_pVT);
  line.lineplace = CPVT_WordPlace(m_CurPos.nSecIndex, m_CurPos.nLineIndex, -1);
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex))
    return false;

  CSection* pSection = m_pVT->m_SectionArray[m_CurPos.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_LineArray, m_CurPos.nLineIndex))
    return false;

  CLine* pLine = pSection->m_LineArray[m_CurPos.nLineIndex].get();
  line.ptLine = m_pVT->InToOut(
      CFX_PointF(pLine->m_LineInfo.fLineX + pSection->m_SecInfo.rcSection.left,
                 pLine->m_LineInfo.fLineY + pSection->m_SecInfo.rcSection.top));
  line.fLineWidth = pLine->m_LineInfo.fLineWidth;
  line.fLineAscent = pLine->m_LineInfo.fLineAscent;
  line.fLineDescent = pLine->m_LineInfo.fLineDescent;
  line.lineEnd = pLine->GetEndWordPlace();
  return true;
}

bool CPDF_VariableText::Iterator::GetSection(CPVT_Section& section) const {
  section.secplace = CPVT_WordPlace(m_CurPos.nSecIndex, 0, -1);
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex))
    return false;

  CSection* pSection = m_pVT->m_SectionArray[m_CurPos.nSecIndex].get();
  section.rcSection = m_pVT->InToOut(pSection->m_SecInfo.rcSection);
  if (pSection->m_SecInfo.pSecProps)
    section.SecProps = *pSection->m_SecInfo.pSecProps;
  if (pSection->m_SecInfo.pWordProps)
    section.WordProps = *pSection->m_SecInfo.pWordProps;
  return true;
}

bool CPDF_VariableText::Iterator::SetSection(const CPVT_Section& section) {
  if (!pdfium::IndexInBounds(m_pVT->m_SectionArray, m_CurPos.nSecIndex))
    return false;

  CSection* pSection = m_pVT->m_SectionArray[m_CurPos.nSecIndex].get();
  if (pSection->m_SecInfo.pSecProps)
    *pSection->m_SecInfo.pSecProps = section.SecProps;
  if (pSection->m_SecInfo.pWordProps)
    *pSection->m_SecInfo.pWordProps = section.WordProps;
  return true;
}

CPDF_VariableText::CPDF_VariableText()
    : m_nLimitChar(0),
      m_nCharArray(0),
      m_bMultiLine(false),
      m_bLimitWidth(false),
      m_bAutoFontSize(false),
      m_nAlignment(0),
      m_fLineLeading(0.0f),
      m_fCharSpace(0.0f),
      m_nHorzScale(100),
      m_wSubWord(0),
      m_fFontSize(0.0f),
      m_bInitialized(false),
      m_pVTProvider(nullptr) {}

CPDF_VariableText::~CPDF_VariableText() {
  ResetAll();
}

void CPDF_VariableText::Initialize() {
  if (m_bInitialized)
    return;

  CPVT_SectionInfo secinfo;
  CPVT_WordPlace place;
  place.nSecIndex = 0;
  AddSection(place, secinfo);

  CPVT_LineInfo lineinfo;
  lineinfo.fLineAscent = GetFontAscent(GetDefaultFontIndex(), GetFontSize());
  lineinfo.fLineDescent = GetFontDescent(GetDefaultFontIndex(), GetFontSize());
  AddLine(place, lineinfo);

  if (!m_SectionArray.empty())
    m_SectionArray.front()->ResetLinePlace();

  m_bInitialized = true;
}

void CPDF_VariableText::ResetAll() {
  m_bInitialized = false;
  m_SectionArray.clear();
}

CPVT_WordPlace CPDF_VariableText::InsertWord(const CPVT_WordPlace& place,
                                             uint16_t word,
                                             int32_t charset,
                                             const CPVT_WordProps* pWordProps) {
  int32_t nTotalWords = GetTotalWords();
  if (m_nLimitChar > 0 && nTotalWords >= m_nLimitChar)
    return place;
  if (m_nCharArray > 0 && nTotalWords >= m_nCharArray)
    return place;

  CPVT_WordPlace newplace = place;
  newplace.nWordIndex++;
  int32_t nFontIndex =
      GetSubWord() > 0 ? GetDefaultFontIndex()
                       : GetWordFontIndex(word, charset, GetDefaultFontIndex());
  return AddWord(newplace, CPVT_WordInfo(word, charset, nFontIndex, nullptr));
}

CPVT_WordPlace CPDF_VariableText::InsertSection(
    const CPVT_WordPlace& place,
    const CPVT_SecProps* pSecProps,
    const CPVT_WordProps* pWordProps) {
  int32_t nTotalWords = GetTotalWords();
  if (m_nLimitChar > 0 && nTotalWords >= m_nLimitChar)
    return place;
  if (m_nCharArray > 0 && nTotalWords >= m_nCharArray)
    return place;
  if (!m_bMultiLine)
    return place;

  CPVT_WordPlace wordplace = place;
  UpdateWordPlace(wordplace);
  if (!pdfium::IndexInBounds(m_SectionArray, wordplace.nSecIndex))
    return place;

  CSection* pSection = m_SectionArray[wordplace.nSecIndex].get();
  CPVT_WordPlace NewPlace(wordplace.nSecIndex + 1, 0, -1);
  CPVT_SectionInfo secinfo;
  AddSection(NewPlace, secinfo);
  CPVT_WordPlace result = NewPlace;
  if (pdfium::IndexInBounds(m_SectionArray, NewPlace.nSecIndex)) {
    CSection* pNewSection = m_SectionArray[NewPlace.nSecIndex].get();
    for (int32_t w = wordplace.nWordIndex + 1;
         w < pdfium::CollectionSize<int32_t>(pSection->m_WordArray); ++w) {
      NewPlace.nWordIndex++;
      pNewSection->AddWord(NewPlace, *pSection->m_WordArray[w]);
    }
  }
  ClearSectionRightWords(wordplace);
  return result;
}

CPVT_WordPlace CPDF_VariableText::InsertText(const CPVT_WordPlace& place,
                                             const wchar_t* text) {
  CFX_WideString swText = text;
  CPVT_WordPlace wp = place;
  for (int32_t i = 0, sz = swText.GetLength(); i < sz; i++) {
    CPVT_WordPlace oldwp = wp;
    uint16_t word = swText.GetAt(i);
    switch (word) {
      case 0x0D:
        if (m_bMultiLine) {
          if (swText.GetAt(i + 1) == 0x0A)
            i += 1;

          wp = InsertSection(wp, nullptr, nullptr);
        }
        break;
      case 0x0A:
        if (m_bMultiLine) {
          if (swText.GetAt(i + 1) == 0x0D)
            i += 1;

          wp = InsertSection(wp, nullptr, nullptr);
        }
        break;
      case 0x09:
        word = 0x20;
      default:
        wp = InsertWord(wp, word, FX_CHARSET_Default, nullptr);
        break;
    }
    if (wp == oldwp)
      break;
  }
  return wp;
}

CPVT_WordPlace CPDF_VariableText::DeleteWords(
    const CPVT_WordRange& PlaceRange) {
  bool bLastSecPos =
      pdfium::IndexInBounds(m_SectionArray, PlaceRange.EndPos.nSecIndex) &&
      PlaceRange.EndPos ==
          m_SectionArray[PlaceRange.EndPos.nSecIndex]->GetEndWordPlace();

  ClearWords(PlaceRange);
  if (PlaceRange.BeginPos.nSecIndex != PlaceRange.EndPos.nSecIndex) {
    ClearEmptySections(PlaceRange);
    if (!bLastSecPos)
      LinkLatterSection(PlaceRange.BeginPos);
  }
  return PlaceRange.BeginPos;
}

CPVT_WordPlace CPDF_VariableText::DeleteWord(const CPVT_WordPlace& place) {
  return ClearRightWord(AdjustLineHeader(place, true));
}

CPVT_WordPlace CPDF_VariableText::BackSpaceWord(const CPVT_WordPlace& place) {
  return ClearLeftWord(AdjustLineHeader(place, true));
}

void CPDF_VariableText::SetText(const CFX_WideString& swText) {
  DeleteWords(CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
  CPVT_WordPlace wp(0, 0, -1);
  CPVT_SectionInfo secinfo;
  if (!m_SectionArray.empty())
    m_SectionArray.front()->m_SecInfo = secinfo;

  int32_t nCharCount = 0;
  for (int32_t i = 0, sz = swText.GetLength(); i < sz; i++) {
    if (m_nLimitChar > 0 && nCharCount >= m_nLimitChar)
      break;
    if (m_nCharArray > 0 && nCharCount >= m_nCharArray)
      break;

    uint16_t word = swText.GetAt(i);
    switch (word) {
      case 0x0D:
        if (m_bMultiLine) {
          if (swText.GetAt(i + 1) == 0x0A)
            i += 1;

          wp.AdvanceSection();
          AddSection(wp, secinfo);
        }
        break;
      case 0x0A:
        if (m_bMultiLine) {
          if (swText.GetAt(i + 1) == 0x0D)
            i += 1;

          wp.AdvanceSection();
          AddSection(wp, secinfo);
        }
        break;
      case 0x09:
        word = 0x20;
      default:
        wp = InsertWord(wp, word, FX_CHARSET_Default, nullptr);
        break;
    }
    nCharCount++;
  }
}

void CPDF_VariableText::UpdateWordPlace(CPVT_WordPlace& place) const {
  if (place.nSecIndex < 0)
    place = GetBeginWordPlace();
  if (place.nSecIndex >= pdfium::CollectionSize<int32_t>(m_SectionArray))
    place = GetEndWordPlace();

  place = AdjustLineHeader(place, true);
  if (pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    m_SectionArray[place.nSecIndex]->UpdateWordPlace(place);
}

int32_t CPDF_VariableText::WordPlaceToWordIndex(
    const CPVT_WordPlace& place) const {
  CPVT_WordPlace newplace = place;
  UpdateWordPlace(newplace);
  int32_t nIndex = 0;
  int32_t i = 0;
  int32_t sz = 0;
  for (i = 0, sz = pdfium::CollectionSize<int32_t>(m_SectionArray);
       i < sz && i < newplace.nSecIndex; i++) {
    CSection* pSection = m_SectionArray[i].get();
    nIndex += pdfium::CollectionSize<int32_t>(pSection->m_WordArray);
    if (i != sz - 1)
      nIndex += kReturnLength;
  }
  if (pdfium::IndexInBounds(m_SectionArray, i))
    nIndex += newplace.nWordIndex + kReturnLength;
  return nIndex;
}

CPVT_WordPlace CPDF_VariableText::WordIndexToWordPlace(int32_t index) const {
  CPVT_WordPlace place = GetBeginWordPlace();
  int32_t nOldIndex = 0;
  int32_t nIndex = 0;
  bool bFound = false;
  for (int32_t i = 0, sz = pdfium::CollectionSize<int32_t>(m_SectionArray);
       i < sz; i++) {
    CSection* pSection = m_SectionArray[i].get();
    nIndex += pdfium::CollectionSize<int32_t>(pSection->m_WordArray);
    if (nIndex == index) {
      place = pSection->GetEndWordPlace();
      bFound = true;
      break;
    }
    if (nIndex > index) {
      place.nSecIndex = i;
      place.nWordIndex = index - nOldIndex - 1;
      pSection->UpdateWordPlace(place);
      bFound = true;
      break;
    }
    if (i != sz - 1)
      nIndex += kReturnLength;
    nOldIndex = nIndex;
  }
  if (!bFound)
    place = GetEndWordPlace();
  return place;
}

CPVT_WordPlace CPDF_VariableText::GetBeginWordPlace() const {
  return m_bInitialized ? CPVT_WordPlace(0, 0, -1) : CPVT_WordPlace();
}

CPVT_WordPlace CPDF_VariableText::GetEndWordPlace() const {
  if (m_SectionArray.empty())
    return CPVT_WordPlace();
  return m_SectionArray.back()->GetEndWordPlace();
}

CPVT_WordPlace CPDF_VariableText::GetPrevWordPlace(
    const CPVT_WordPlace& place) const {
  if (place.nSecIndex < 0)
    return GetBeginWordPlace();
  if (place.nSecIndex >= pdfium::CollectionSize<int32_t>(m_SectionArray))
    return GetEndWordPlace();

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  if (place > pSection->GetBeginWordPlace())
    return pSection->GetPrevWordPlace(place);
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex - 1))
    return GetBeginWordPlace();
  return m_SectionArray[place.nSecIndex - 1]->GetEndWordPlace();
}

CPVT_WordPlace CPDF_VariableText::GetNextWordPlace(
    const CPVT_WordPlace& place) const {
  if (place.nSecIndex < 0)
    return GetBeginWordPlace();
  if (place.nSecIndex >= pdfium::CollectionSize<int32_t>(m_SectionArray))
    return GetEndWordPlace();

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  if (place < pSection->GetEndWordPlace())
    return pSection->GetNextWordPlace(place);
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex + 1))
    return GetEndWordPlace();
  return m_SectionArray[place.nSecIndex + 1]->GetBeginWordPlace();
}

CPVT_WordPlace CPDF_VariableText::SearchWordPlace(
    const CFX_PointF& point) const {
  CFX_PointF pt = OutToIn(point);
  CPVT_WordPlace place = GetBeginWordPlace();
  int32_t nLeft = 0;
  int32_t nRight = pdfium::CollectionSize<int32_t>(m_SectionArray) - 1;
  int32_t nMid = pdfium::CollectionSize<int32_t>(m_SectionArray) / 2;
  bool bUp = true;
  bool bDown = true;
  while (nLeft <= nRight) {
    if (!pdfium::IndexInBounds(m_SectionArray, nMid))
      break;
    CSection* pSection = m_SectionArray[nMid].get();
    if (IsFloatBigger(pt.y, pSection->m_SecInfo.rcSection.top))
      bUp = false;
    if (IsFloatBigger(pSection->m_SecInfo.rcSection.bottom, pt.y))
      bDown = false;
    if (IsFloatSmaller(pt.y, pSection->m_SecInfo.rcSection.top)) {
      nRight = nMid - 1;
      nMid = (nLeft + nRight) / 2;
      continue;
    }
    if (IsFloatBigger(pt.y, pSection->m_SecInfo.rcSection.bottom)) {
      nLeft = nMid + 1;
      nMid = (nLeft + nRight) / 2;
      continue;
    }
    place = pSection->SearchWordPlace(
        CFX_PointF(pt.x - pSection->m_SecInfo.rcSection.left,
                   pt.y - pSection->m_SecInfo.rcSection.top));
    place.nSecIndex = nMid;
    return place;
  }
  if (bUp)
    place = GetBeginWordPlace();
  if (bDown)
    place = GetEndWordPlace();
  return place;
}

CPVT_WordPlace CPDF_VariableText::GetUpWordPlace(
    const CPVT_WordPlace& place,
    const CFX_PointF& point) const {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return place;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  CPVT_WordPlace temp = place;
  CFX_PointF pt = OutToIn(point);
  if (temp.nLineIndex-- > 0) {
    return pSection->SearchWordPlace(pt.x - pSection->m_SecInfo.rcSection.left,
                                     temp);
  }
  if (temp.nSecIndex-- > 0) {
    if (pdfium::IndexInBounds(m_SectionArray, temp.nSecIndex)) {
      CSection* pLastSection = m_SectionArray[temp.nSecIndex].get();
      temp.nLineIndex =
          pdfium::CollectionSize<int32_t>(pLastSection->m_LineArray) - 1;
      return pLastSection->SearchWordPlace(
          pt.x - pLastSection->m_SecInfo.rcSection.left, temp);
    }
  }
  return place;
}

CPVT_WordPlace CPDF_VariableText::GetDownWordPlace(
    const CPVT_WordPlace& place,
    const CFX_PointF& point) const {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return place;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  CPVT_WordPlace temp = place;
  CFX_PointF pt = OutToIn(point);
  if (temp.nLineIndex++ <
      pdfium::CollectionSize<int32_t>(pSection->m_LineArray) - 1) {
    return pSection->SearchWordPlace(pt.x - pSection->m_SecInfo.rcSection.left,
                                     temp);
  }
  temp.AdvanceSection();
  if (!pdfium::IndexInBounds(m_SectionArray, temp.nSecIndex))
    return place;

  return m_SectionArray[temp.nSecIndex]->SearchWordPlace(
      pt.x - pSection->m_SecInfo.rcSection.left, temp);
}

CPVT_WordPlace CPDF_VariableText::GetLineBeginPlace(
    const CPVT_WordPlace& place) const {
  return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, -1);
}

CPVT_WordPlace CPDF_VariableText::GetLineEndPlace(
    const CPVT_WordPlace& place) const {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return place;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_LineArray, place.nLineIndex))
    return place;

  return pSection->m_LineArray[place.nLineIndex]->GetEndWordPlace();
}

CPVT_WordPlace CPDF_VariableText::GetSectionBeginPlace(
    const CPVT_WordPlace& place) const {
  return CPVT_WordPlace(place.nSecIndex, 0, -1);
}

CPVT_WordPlace CPDF_VariableText::GetSectionEndPlace(
    const CPVT_WordPlace& place) const {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return place;

  return m_SectionArray[place.nSecIndex]->GetEndWordPlace();
}

int32_t CPDF_VariableText::GetTotalWords() const {
  int32_t nTotal = 0;
  for (const auto& pSection : m_SectionArray) {
    nTotal +=
        pdfium::CollectionSize<int32_t>(pSection->m_WordArray) + kReturnLength;
  }
  return nTotal - kReturnLength;
}

CPVT_WordPlace CPDF_VariableText::AddSection(const CPVT_WordPlace& place,
                                             const CPVT_SectionInfo& secinfo) {
  if (IsValid() && !m_bMultiLine)
    return place;

  int32_t nSecIndex = pdfium::clamp(
      place.nSecIndex, 0, pdfium::CollectionSize<int32_t>(m_SectionArray));

  auto pSection = pdfium::MakeUnique<CSection>(this);
  pSection->m_SecInfo = secinfo;
  pSection->SecPlace.nSecIndex = nSecIndex;
  m_SectionArray.insert(m_SectionArray.begin() + nSecIndex,
                        std::move(pSection));
  return place;
}

CPVT_WordPlace CPDF_VariableText::AddLine(const CPVT_WordPlace& place,
                                          const CPVT_LineInfo& lineinfo) {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return place;

  return m_SectionArray[place.nSecIndex]->AddLine(lineinfo);
}

CPVT_WordPlace CPDF_VariableText::AddWord(const CPVT_WordPlace& place,
                                          const CPVT_WordInfo& wordinfo) {
  if (m_SectionArray.empty())
    return place;

  CPVT_WordPlace newplace = place;
  newplace.nSecIndex =
      pdfium::clamp(newplace.nSecIndex, 0,
                    pdfium::CollectionSize<int32_t>(m_SectionArray) - 1);
  return m_SectionArray[newplace.nSecIndex]->AddWord(newplace, wordinfo);
}

bool CPDF_VariableText::GetWordInfo(const CPVT_WordPlace& place,
                                    CPVT_WordInfo& wordinfo) {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return false;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_WordArray, place.nWordIndex))
    return false;

  wordinfo = *pSection->m_WordArray[place.nWordIndex];
  return true;
}

bool CPDF_VariableText::SetWordInfo(const CPVT_WordPlace& place,
                                    const CPVT_WordInfo& wordinfo) {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return false;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_WordArray, place.nWordIndex))
    return false;

  *pSection->m_WordArray[place.nWordIndex] = wordinfo;
  return true;
}

bool CPDF_VariableText::GetLineInfo(const CPVT_WordPlace& place,
                                    CPVT_LineInfo& lineinfo) {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return false;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_LineArray, place.nLineIndex))
    return false;

  lineinfo = pSection->m_LineArray[place.nLineIndex]->m_LineInfo;
  return true;
}

bool CPDF_VariableText::GetSectionInfo(const CPVT_WordPlace& place,
                                       CPVT_SectionInfo& secinfo) {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return false;

  secinfo = m_SectionArray[place.nSecIndex]->m_SecInfo;
  return true;
}

void CPDF_VariableText::SetPlateRect(const CFX_FloatRect& rect) {
  m_rcPlate = rect;
}

void CPDF_VariableText::SetContentRect(const CPVT_FloatRect& rect) {
  m_rcContent = rect;
}

CFX_FloatRect CPDF_VariableText::GetContentRect() const {
  return InToOut(CPVT_FloatRect(m_rcContent));
}

const CFX_FloatRect& CPDF_VariableText::GetPlateRect() const {
  return m_rcPlate;
}

float CPDF_VariableText::GetWordFontSize(const CPVT_WordInfo& WordInfo) {
  return GetFontSize();
}

int32_t CPDF_VariableText::GetWordFontIndex(const CPVT_WordInfo& WordInfo) {
  return WordInfo.nFontIndex;
}

float CPDF_VariableText::GetWordWidth(int32_t nFontIndex,
                                      uint16_t Word,
                                      uint16_t SubWord,
                                      float fCharSpace,
                                      int32_t nHorzScale,
                                      float fFontSize,
                                      float fWordTail) {
  return (GetCharWidth(nFontIndex, Word, SubWord) * fFontSize * kFontScale +
          fCharSpace) *
             nHorzScale * kScalePercent +
         fWordTail;
}

float CPDF_VariableText::GetWordWidth(const CPVT_WordInfo& WordInfo) {
  return GetWordWidth(GetWordFontIndex(WordInfo), WordInfo.Word, GetSubWord(),
                      GetCharSpace(WordInfo), GetHorzScale(WordInfo),
                      GetWordFontSize(WordInfo), WordInfo.fWordTail);
}

float CPDF_VariableText::GetLineAscent(const CPVT_SectionInfo& SecInfo) {
  return GetFontAscent(GetDefaultFontIndex(), GetFontSize());
}

float CPDF_VariableText::GetLineDescent(const CPVT_SectionInfo& SecInfo) {
  return GetFontDescent(GetDefaultFontIndex(), GetFontSize());
}

float CPDF_VariableText::GetFontAscent(int32_t nFontIndex, float fFontSize) {
  return (float)GetTypeAscent(nFontIndex) * fFontSize * kFontScale;
}

float CPDF_VariableText::GetFontDescent(int32_t nFontIndex, float fFontSize) {
  return (float)GetTypeDescent(nFontIndex) * fFontSize * kFontScale;
}

float CPDF_VariableText::GetWordAscent(const CPVT_WordInfo& WordInfo,
                                       float fFontSize) {
  return GetFontAscent(GetWordFontIndex(WordInfo), fFontSize);
}

float CPDF_VariableText::GetWordDescent(const CPVT_WordInfo& WordInfo,
                                        float fFontSize) {
  return GetFontDescent(GetWordFontIndex(WordInfo), fFontSize);
}

float CPDF_VariableText::GetWordAscent(const CPVT_WordInfo& WordInfo) {
  return GetFontAscent(GetWordFontIndex(WordInfo), GetWordFontSize(WordInfo));
}

float CPDF_VariableText::GetWordDescent(const CPVT_WordInfo& WordInfo) {
  return GetFontDescent(GetWordFontIndex(WordInfo), GetWordFontSize(WordInfo));
}

float CPDF_VariableText::GetLineLeading(const CPVT_SectionInfo& SecInfo) {
  return m_fLineLeading;
}

float CPDF_VariableText::GetLineIndent(const CPVT_SectionInfo& SecInfo) {
  return 0.0f;
}

int32_t CPDF_VariableText::GetAlignment(const CPVT_SectionInfo& SecInfo) {
  return m_nAlignment;
}

float CPDF_VariableText::GetCharSpace(const CPVT_WordInfo& WordInfo) {
  return m_fCharSpace;
}

int32_t CPDF_VariableText::GetHorzScale(const CPVT_WordInfo& WordInfo) {
  return m_nHorzScale;
}

void CPDF_VariableText::ClearSectionRightWords(const CPVT_WordPlace& place) {
  CPVT_WordPlace wordplace = AdjustLineHeader(place, true);
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  if (!pdfium::IndexInBounds(pSection->m_WordArray, wordplace.nWordIndex + 1))
    return;

  pSection->m_WordArray.erase(
      pSection->m_WordArray.begin() + wordplace.nWordIndex + 1,
      pSection->m_WordArray.end());
}

CPVT_WordPlace CPDF_VariableText::AdjustLineHeader(const CPVT_WordPlace& place,
                                                   bool bPrevOrNext) const {
  if (place.nWordIndex < 0 && place.nLineIndex > 0)
    return bPrevOrNext ? GetPrevWordPlace(place) : GetNextWordPlace(place);
  return place;
}

bool CPDF_VariableText::ClearEmptySection(const CPVT_WordPlace& place) {
  if (place.nSecIndex == 0 && m_SectionArray.size() == 1)
    return false;

  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return false;

  if (!m_SectionArray[place.nSecIndex]->m_WordArray.empty())
    return false;

  m_SectionArray.erase(m_SectionArray.begin() + place.nSecIndex);
  return true;
}

void CPDF_VariableText::ClearEmptySections(const CPVT_WordRange& PlaceRange) {
  CPVT_WordPlace wordplace;
  for (int32_t s = PlaceRange.EndPos.nSecIndex;
       s > PlaceRange.BeginPos.nSecIndex; s--) {
    wordplace.nSecIndex = s;
    ClearEmptySection(wordplace);
  }
}

void CPDF_VariableText::LinkLatterSection(const CPVT_WordPlace& place) {
  CPVT_WordPlace oldplace = AdjustLineHeader(place, true);
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex + 1))
    return;

  CSection* pNextSection = m_SectionArray[place.nSecIndex + 1].get();
  if (pdfium::IndexInBounds(m_SectionArray, oldplace.nSecIndex)) {
    CSection* pSection = m_SectionArray[oldplace.nSecIndex].get();
    for (auto& pWord : pNextSection->m_WordArray) {
      oldplace.nWordIndex++;
      pSection->AddWord(oldplace, *pWord);
    }
  }
  m_SectionArray.erase(m_SectionArray.begin() + place.nSecIndex + 1);
}

void CPDF_VariableText::ClearWords(const CPVT_WordRange& PlaceRange) {
  CPVT_WordRange NewRange;
  NewRange.BeginPos = AdjustLineHeader(PlaceRange.BeginPos, true);
  NewRange.EndPos = AdjustLineHeader(PlaceRange.EndPos, true);
  for (int32_t s = NewRange.EndPos.nSecIndex; s >= NewRange.BeginPos.nSecIndex;
       s--) {
    if (pdfium::IndexInBounds(m_SectionArray, s))
      m_SectionArray[s]->ClearWords(NewRange);
  }
}

CPVT_WordPlace CPDF_VariableText::ClearLeftWord(const CPVT_WordPlace& place) {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return place;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  CPVT_WordPlace leftplace = GetPrevWordPlace(place);
  if (leftplace == place)
    return place;

  if (leftplace.nSecIndex != place.nSecIndex) {
    if (pSection->m_WordArray.empty())
      ClearEmptySection(place);
    else
      LinkLatterSection(leftplace);
  } else {
    pSection->ClearWord(place);
  }
  return leftplace;
}

CPVT_WordPlace CPDF_VariableText::ClearRightWord(const CPVT_WordPlace& place) {
  if (!pdfium::IndexInBounds(m_SectionArray, place.nSecIndex))
    return place;

  CSection* pSection = m_SectionArray[place.nSecIndex].get();
  CPVT_WordPlace rightplace = AdjustLineHeader(GetNextWordPlace(place), false);
  if (rightplace == place)
    return place;

  if (rightplace.nSecIndex != place.nSecIndex)
    LinkLatterSection(place);
  else
    pSection->ClearWord(rightplace);
  return place;
}

void CPDF_VariableText::RearrangeAll() {
  Rearrange(CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
}

void CPDF_VariableText::RearrangePart(const CPVT_WordRange& PlaceRange) {
  Rearrange(PlaceRange);
}

CPVT_FloatRect CPDF_VariableText::Rearrange(const CPVT_WordRange& PlaceRange) {
  CPVT_FloatRect rcRet;
  if (IsValid()) {
    if (m_bAutoFontSize) {
      SetFontSize(GetAutoFontSize());
      rcRet = RearrangeSections(
          CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
    } else {
      rcRet = RearrangeSections(PlaceRange);
    }
  }
  SetContentRect(rcRet);
  return rcRet;
}

float CPDF_VariableText::GetAutoFontSize() {
  int32_t nTotal = sizeof(gFontSizeSteps) / sizeof(uint8_t);
  if (IsMultiLine())
    nTotal /= 4;
  if (nTotal <= 0)
    return 0;
  if (GetPlateWidth() <= 0)
    return 0;

  int32_t nLeft = 0;
  int32_t nRight = nTotal - 1;
  int32_t nMid = nTotal / 2;
  while (nLeft <= nRight) {
    if (IsBigger(gFontSizeSteps[nMid])) {
      nRight = nMid - 1;
      nMid = (nLeft + nRight) / 2;
      continue;
    } else {
      nLeft = nMid + 1;
      nMid = (nLeft + nRight) / 2;
      continue;
    }
  }
  return (float)gFontSizeSteps[nMid];
}

bool CPDF_VariableText::IsBigger(float fFontSize) const {
  CFX_SizeF szTotal;
  for (const auto& pSection : m_SectionArray) {
    CFX_SizeF size = pSection->GetSectionSize(fFontSize);
    szTotal.width = std::max(size.width, szTotal.width);
    szTotal.height += size.height;
    if (IsFloatBigger(szTotal.width, GetPlateWidth()) ||
        IsFloatBigger(szTotal.height, GetPlateHeight())) {
      return true;
    }
  }
  return false;
}

CPVT_FloatRect CPDF_VariableText::RearrangeSections(
    const CPVT_WordRange& PlaceRange) {
  CPVT_WordPlace place;
  float fPosY = 0;
  float fOldHeight;
  int32_t nSSecIndex = PlaceRange.BeginPos.nSecIndex;
  int32_t nESecIndex = PlaceRange.EndPos.nSecIndex;
  CPVT_FloatRect rcRet;
  for (int32_t s = 0, sz = pdfium::CollectionSize<int32_t>(m_SectionArray);
       s < sz; s++) {
    place.nSecIndex = s;
    CSection* pSection = m_SectionArray[s].get();
    pSection->SecPlace = place;
    CPVT_FloatRect rcSec = pSection->m_SecInfo.rcSection;
    if (s >= nSSecIndex) {
      if (s <= nESecIndex) {
        rcSec = pSection->Rearrange();
        rcSec.top += fPosY;
        rcSec.bottom += fPosY;
      } else {
        fOldHeight = pSection->m_SecInfo.rcSection.bottom -
                     pSection->m_SecInfo.rcSection.top;
        rcSec.top = fPosY;
        rcSec.bottom = fPosY + fOldHeight;
      }
      pSection->m_SecInfo.rcSection = rcSec;
      pSection->ResetLinePlace();
    }
    if (s == 0) {
      rcRet = rcSec;
    } else {
      rcRet.left = std::min(rcSec.left, rcRet.left);
      rcRet.top = std::min(rcSec.top, rcRet.top);
      rcRet.right = std::max(rcSec.right, rcRet.right);
      rcRet.bottom = std::max(rcSec.bottom, rcRet.bottom);
    }
    fPosY += rcSec.Height();
  }
  return rcRet;
}

int32_t CPDF_VariableText::GetCharWidth(int32_t nFontIndex,
                                        uint16_t Word,
                                        uint16_t SubWord) {
  if (!m_pVTProvider)
    return 0;
  uint16_t word = SubWord ? SubWord : Word;
  return m_pVTProvider->GetCharWidth(nFontIndex, word);
}

int32_t CPDF_VariableText::GetTypeAscent(int32_t nFontIndex) {
  return m_pVTProvider ? m_pVTProvider->GetTypeAscent(nFontIndex) : 0;
}

int32_t CPDF_VariableText::GetTypeDescent(int32_t nFontIndex) {
  return m_pVTProvider ? m_pVTProvider->GetTypeDescent(nFontIndex) : 0;
}

int32_t CPDF_VariableText::GetWordFontIndex(uint16_t word,
                                            int32_t charset,
                                            int32_t nFontIndex) {
  return m_pVTProvider
             ? m_pVTProvider->GetWordFontIndex(word, charset, nFontIndex)
             : -1;
}

int32_t CPDF_VariableText::GetDefaultFontIndex() {
  return m_pVTProvider ? m_pVTProvider->GetDefaultFontIndex() : -1;
}

bool CPDF_VariableText::IsLatinWord(uint16_t word) {
  return m_pVTProvider ? m_pVTProvider->IsLatinWord(word) : false;
}

CPDF_VariableText::Iterator* CPDF_VariableText::GetIterator() {
  if (!m_pVTIterator)
    m_pVTIterator = pdfium::MakeUnique<CPDF_VariableText::Iterator>(this);
  return m_pVTIterator.get();
}

void CPDF_VariableText::SetProvider(CPDF_VariableText::Provider* pProvider) {
  m_pVTProvider = pProvider;
}

CFX_SizeF CPDF_VariableText::GetPlateSize() const {
  return CFX_SizeF(GetPlateWidth(), GetPlateHeight());
}

CFX_PointF CPDF_VariableText::GetBTPoint() const {
  return CFX_PointF(m_rcPlate.left, m_rcPlate.top);
}

CFX_PointF CPDF_VariableText::GetETPoint() const {
  return CFX_PointF(m_rcPlate.right, m_rcPlate.bottom);
}

CFX_PointF CPDF_VariableText::InToOut(const CFX_PointF& point) const {
  return CFX_PointF(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
}

CFX_PointF CPDF_VariableText::OutToIn(const CFX_PointF& point) const {
  return CFX_PointF(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
}

CFX_FloatRect CPDF_VariableText::InToOut(const CPVT_FloatRect& rect) const {
  CFX_PointF ptLeftTop = InToOut(CFX_PointF(rect.left, rect.top));
  CFX_PointF ptRightBottom = InToOut(CFX_PointF(rect.right, rect.bottom));
  return CFX_FloatRect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                       ptLeftTop.y);
}

CPVT_FloatRect CPDF_VariableText::OutToIn(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftTop = OutToIn(CFX_PointF(rect.left, rect.top));
  CFX_PointF ptRightBottom = OutToIn(CFX_PointF(rect.right, rect.bottom));
  return CPVT_FloatRect(ptLeftTop.x, ptLeftTop.y, ptRightBottom.x,
                        ptRightBottom.y);
}
