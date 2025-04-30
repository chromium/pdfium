// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpvt_variabletext.h"

#include <algorithm>
#include <array>
#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfdoc/cpvt_section.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fpdfdoc/cpvt_wordinfo.h"
#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"

namespace {

constexpr float kFontScale = 0.001f;
constexpr uint8_t kReturnLength = 1;

constexpr auto kFontSizeSteps = std::to_array<const uint8_t>(
    {4,  6,  8,  9,  10, 12, 14, 18,  20,  25,  30,  35, 40,
     45, 50, 55, 60, 70, 80, 90, 100, 110, 120, 130, 144});

}  // namespace

CPVT_VariableText::Provider::Provider(IPVT_FontMap* pFontMap)
    : font_map_(pFontMap) {
  DCHECK(font_map_);
}

CPVT_VariableText::Provider::~Provider() = default;

int CPVT_VariableText::Provider::GetCharWidth(int32_t nFontIndex,
                                              uint16_t word) {
  RetainPtr<CPDF_Font> pPDFFont = font_map_->GetPDFFont(nFontIndex);
  if (!pPDFFont) {
    return 0;
  }

  uint32_t charcode = pPDFFont->CharCodeFromUnicode(word);
  if (charcode == CPDF_Font::kInvalidCharCode) {
    return 0;
  }

  return pPDFFont->GetCharWidthF(charcode);
}

int32_t CPVT_VariableText::Provider::GetTypeAscent(int32_t nFontIndex) {
  RetainPtr<CPDF_Font> pPDFFont = font_map_->GetPDFFont(nFontIndex);
  return pPDFFont ? pPDFFont->GetTypeAscent() : 0;
}

int32_t CPVT_VariableText::Provider::GetTypeDescent(int32_t nFontIndex) {
  RetainPtr<CPDF_Font> pPDFFont = font_map_->GetPDFFont(nFontIndex);
  return pPDFFont ? pPDFFont->GetTypeDescent() : 0;
}

int32_t CPVT_VariableText::Provider::GetWordFontIndex(uint16_t word,
                                                      FX_Charset charset,
                                                      int32_t nFontIndex) {
  if (RetainPtr<CPDF_Font> pDefFont = font_map_->GetPDFFont(0)) {
    if (pDefFont->CharCodeFromUnicode(word) != CPDF_Font::kInvalidCharCode) {
      return 0;
    }
  }
  if (RetainPtr<CPDF_Font> pSysFont = font_map_->GetPDFFont(1)) {
    if (pSysFont->CharCodeFromUnicode(word) != CPDF_Font::kInvalidCharCode) {
      return 1;
    }
  }
  return -1;
}

int32_t CPVT_VariableText::Provider::GetDefaultFontIndex() {
  return 0;
}

CPVT_VariableText::Iterator::Iterator(CPVT_VariableText* pVT) : vt_(pVT) {
  DCHECK(vt_);
}

CPVT_VariableText::Iterator::~Iterator() = default;

void CPVT_VariableText::Iterator::SetAt(int32_t nWordIndex) {
  cur_pos_ = vt_->WordIndexToWordPlace(nWordIndex);
}

void CPVT_VariableText::Iterator::SetAt(const CPVT_WordPlace& place) {
  cur_pos_ = place;
}

bool CPVT_VariableText::Iterator::NextWord() {
  if (cur_pos_ == vt_->GetEndWordPlace()) {
    return false;
  }

  cur_pos_ = vt_->GetNextWordPlace(cur_pos_);
  return true;
}

bool CPVT_VariableText::Iterator::NextLine() {
  if (!fxcrt::IndexInBounds(vt_->section_array_, cur_pos_.nSecIndex)) {
    return false;
  }

  CPVT_Section* pSection = vt_->section_array_[cur_pos_.nSecIndex].get();
  if (cur_pos_.nLineIndex < pSection->GetLineArraySize() - 1) {
    cur_pos_ = CPVT_WordPlace(cur_pos_.nSecIndex, cur_pos_.nLineIndex + 1, -1);
    return true;
  }
  if (cur_pos_.nSecIndex <
      fxcrt::CollectionSize<int32_t>(vt_->section_array_) - 1) {
    cur_pos_ = CPVT_WordPlace(cur_pos_.nSecIndex + 1, 0, -1);
    return true;
  }
  return false;
}

bool CPVT_VariableText::Iterator::GetWord(CPVT_Word& word) const {
  word.WordPlace = cur_pos_;
  if (!fxcrt::IndexInBounds(vt_->section_array_, cur_pos_.nSecIndex)) {
    return false;
  }

  CPVT_Section* pSection = vt_->section_array_[cur_pos_.nSecIndex].get();
  if (!pSection->GetLineFromArray(cur_pos_.nLineIndex)) {
    return false;
  }

  const CPVT_WordInfo* pInfo = pSection->GetWordFromArray(cur_pos_.nWordIndex);
  if (!pInfo) {
    return false;
  }

  word.Word = pInfo->Word;
  word.nCharset = pInfo->nCharset;
  word.fWidth = vt_->GetWordWidth(*pInfo);
  word.ptWord =
      vt_->InToOut(CFX_PointF(pInfo->fWordX + pSection->GetRect().left,
                              pInfo->fWordY + pSection->GetRect().top));
  word.fAscent = vt_->GetWordAscent(*pInfo);
  word.fDescent = vt_->GetWordDescent(*pInfo);
  word.nFontIndex = pInfo->nFontIndex;
  word.fFontSize = vt_->GetWordFontSize();
  return true;
}

bool CPVT_VariableText::Iterator::GetLine(CPVT_Line& line) const {
  DCHECK(vt_);
  line.lineplace = CPVT_WordPlace(cur_pos_.nSecIndex, cur_pos_.nLineIndex, -1);
  if (!fxcrt::IndexInBounds(vt_->section_array_, cur_pos_.nSecIndex)) {
    return false;
  }

  CPVT_Section* pSection = vt_->section_array_[cur_pos_.nSecIndex].get();
  const CPVT_Section::Line* pLine =
      pSection->GetLineFromArray(cur_pos_.nLineIndex);
  if (!pLine) {
    return false;
  }

  line.ptLine = vt_->InToOut(
      CFX_PointF(pLine->line_info_.fLineX + pSection->GetRect().left,
                 pLine->line_info_.fLineY + pSection->GetRect().top));
  line.fLineWidth = pLine->line_info_.fLineWidth;
  line.fLineAscent = pLine->line_info_.fLineAscent;
  line.fLineDescent = pLine->line_info_.fLineDescent;
  line.lineEnd = pLine->GetEndWordPlace();
  return true;
}

CPVT_VariableText::CPVT_VariableText(Provider* pProvider)
    : vt_provider_(pProvider) {}

CPVT_VariableText::~CPVT_VariableText() = default;

void CPVT_VariableText::Initialize() {
  if (initialized_) {
    return;
  }

  CPVT_WordPlace place;
  place.nSecIndex = 0;
  AddSection(place);

  CPVT_LineInfo lineinfo;
  lineinfo.fLineAscent = GetFontAscent(GetDefaultFontIndex(), GetFontSize());
  lineinfo.fLineDescent = GetFontDescent(GetDefaultFontIndex(), GetFontSize());
  AddLine(place, lineinfo);

  if (!section_array_.empty()) {
    section_array_.front()->ResetLinePlace();
  }

  initialized_ = true;
}

CPVT_WordPlace CPVT_VariableText::InsertWord(const CPVT_WordPlace& place,
                                             uint16_t word,
                                             FX_Charset charset) {
  int32_t nTotalWords = GetTotalWords();
  if (limit_char_ > 0 && nTotalWords >= limit_char_) {
    return place;
  }
  if (char_array_ > 0 && nTotalWords >= char_array_) {
    return place;
  }

  CPVT_WordPlace newplace = place;
  newplace.nWordIndex++;
  int32_t nFontIndex =
      GetSubWord() > 0 ? GetDefaultFontIndex()
                       : GetWordFontIndex(word, charset, GetDefaultFontIndex());
  return AddWord(newplace, CPVT_WordInfo(word, charset, nFontIndex));
}

CPVT_WordPlace CPVT_VariableText::InsertSection(const CPVT_WordPlace& place) {
  int32_t nTotalWords = GetTotalWords();
  if (limit_char_ > 0 && nTotalWords >= limit_char_) {
    return place;
  }
  if (char_array_ > 0 && nTotalWords >= char_array_) {
    return place;
  }
  if (!multi_line_) {
    return place;
  }

  CPVT_WordPlace wordplace = place;
  UpdateWordPlace(wordplace);
  if (!fxcrt::IndexInBounds(section_array_, wordplace.nSecIndex)) {
    return place;
  }

  CPVT_Section* pSection = section_array_[wordplace.nSecIndex].get();
  CPVT_WordPlace NewPlace(wordplace.nSecIndex + 1, 0, -1);
  AddSection(NewPlace);
  CPVT_WordPlace result = NewPlace;
  if (fxcrt::IndexInBounds(section_array_, NewPlace.nSecIndex)) {
    CPVT_Section* pNewSection = section_array_[NewPlace.nSecIndex].get();
    for (int32_t w = wordplace.nWordIndex + 1; w < pSection->GetWordArraySize();
         ++w) {
      NewPlace.nWordIndex++;
      pNewSection->AddWord(NewPlace, *pSection->GetWordFromArray(w));
    }
  }
  ClearSectionRightWords(wordplace);
  return result;
}

CPVT_WordPlace CPVT_VariableText::DeleteWords(
    const CPVT_WordRange& PlaceRange) {
  bool bLastSecPos =
      fxcrt::IndexInBounds(section_array_, PlaceRange.EndPos.nSecIndex) &&
      PlaceRange.EndPos ==
          section_array_[PlaceRange.EndPos.nSecIndex]->GetEndWordPlace();

  ClearWords(PlaceRange);
  if (PlaceRange.BeginPos.nSecIndex != PlaceRange.EndPos.nSecIndex) {
    ClearEmptySections(PlaceRange);
    if (!bLastSecPos) {
      LinkLatterSection(PlaceRange.BeginPos);
    }
  }
  return PlaceRange.BeginPos;
}

CPVT_WordPlace CPVT_VariableText::DeleteWord(const CPVT_WordPlace& place) {
  return ClearRightWord(PrevLineHeaderPlace(place));
}

CPVT_WordPlace CPVT_VariableText::BackSpaceWord(const CPVT_WordPlace& place) {
  return ClearLeftWord(PrevLineHeaderPlace(place));
}

void CPVT_VariableText::SetText(const WideString& swText) {
  DeleteWords(CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
  CPVT_WordPlace wp(0, 0, -1);
  if (!section_array_.empty()) {
    section_array_.front()->SetRect(CPVT_FloatRect());
  }

  FX_SAFE_INT32 nCharCount = 0;
  for (size_t i = 0, sz = swText.GetLength(); i < sz; i++) {
    if (limit_char_ > 0 && nCharCount.ValueOrDie() >= limit_char_) {
      break;
    }
    if (char_array_ > 0 && nCharCount.ValueOrDie() >= char_array_) {
      break;
    }

    uint16_t word = swText[i];
    switch (word) {
      case 0x0D:
        if (multi_line_) {
          if (i + 1 < sz && swText[i + 1] == 0x0A) {
            i++;
          }
          wp.AdvanceSection();
          AddSection(wp);
        }
        break;
      case 0x0A:
        if (multi_line_) {
          if (i + 1 < sz && swText[i + 1] == 0x0D) {
            i++;
          }
          wp.AdvanceSection();
          AddSection(wp);
        }
        break;
      case 0x09:
        word = 0x20;
        [[fallthrough]];
      default:
        wp = InsertWord(wp, word, FX_Charset::kDefault);
        break;
    }
    nCharCount++;
  }
}

void CPVT_VariableText::UpdateWordPlace(CPVT_WordPlace& place) const {
  if (place.nSecIndex < 0) {
    place = GetBeginWordPlace();
  }
  if (static_cast<size_t>(place.nSecIndex) >= section_array_.size()) {
    place = GetEndWordPlace();
  }

  place = PrevLineHeaderPlace(place);
  if (fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    section_array_[place.nSecIndex]->UpdateWordPlace(place);
  }
}

int32_t CPVT_VariableText::WordPlaceToWordIndex(
    const CPVT_WordPlace& place) const {
  CPVT_WordPlace newplace = place;
  UpdateWordPlace(newplace);
  int32_t nIndex = 0;
  int32_t i = 0;
  int32_t sz = 0;
  for (i = 0, sz = fxcrt::CollectionSize<int32_t>(section_array_);
       i < sz && i < newplace.nSecIndex; i++) {
    CPVT_Section* pSection = section_array_[i].get();
    nIndex += pSection->GetWordArraySize();
    if (i != sz - 1) {
      nIndex += kReturnLength;
    }
  }
  if (fxcrt::IndexInBounds(section_array_, i)) {
    nIndex += newplace.nWordIndex + kReturnLength;
  }
  return nIndex;
}

CPVT_WordPlace CPVT_VariableText::WordIndexToWordPlace(int32_t index) const {
  CPVT_WordPlace place = GetBeginWordPlace();
  int32_t nOldIndex = 0;
  int32_t nIndex = 0;
  bool bFound = false;
  for (size_t i = 0; i < section_array_.size(); ++i) {
    CPVT_Section* pSection = section_array_[i].get();
    nIndex += pSection->GetWordArraySize();
    if (nIndex == index) {
      place = pSection->GetEndWordPlace();
      bFound = true;
      break;
    }
    if (nIndex > index) {
      place.nSecIndex = pdfium::checked_cast<int32_t>(i);
      place.nWordIndex = index - nOldIndex - 1;
      pSection->UpdateWordPlace(place);
      bFound = true;
      break;
    }
    if (i != section_array_.size() - 1) {
      nIndex += kReturnLength;
    }
    nOldIndex = nIndex;
  }
  if (!bFound) {
    place = GetEndWordPlace();
  }
  return place;
}

CPVT_WordPlace CPVT_VariableText::GetBeginWordPlace() const {
  return initialized_ ? CPVT_WordPlace(0, 0, -1) : CPVT_WordPlace();
}

CPVT_WordPlace CPVT_VariableText::GetEndWordPlace() const {
  if (section_array_.empty()) {
    return CPVT_WordPlace();
  }
  return section_array_.back()->GetEndWordPlace();
}

CPVT_WordPlace CPVT_VariableText::GetPrevWordPlace(
    const CPVT_WordPlace& place) const {
  if (place.nSecIndex < 0) {
    return GetBeginWordPlace();
  }
  if (static_cast<size_t>(place.nSecIndex) >= section_array_.size()) {
    return GetEndWordPlace();
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  if (place > pSection->GetBeginWordPlace()) {
    return pSection->GetPrevWordPlace(place);
  }
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex - 1)) {
    return GetBeginWordPlace();
  }
  return section_array_[place.nSecIndex - 1]->GetEndWordPlace();
}

CPVT_WordPlace CPVT_VariableText::GetNextWordPlace(
    const CPVT_WordPlace& place) const {
  if (place.nSecIndex < 0) {
    return GetBeginWordPlace();
  }
  if (static_cast<size_t>(place.nSecIndex) >= section_array_.size()) {
    return GetEndWordPlace();
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  if (place < pSection->GetEndWordPlace()) {
    return pSection->GetNextWordPlace(place);
  }
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex + 1)) {
    return GetEndWordPlace();
  }
  return section_array_[place.nSecIndex + 1]->GetBeginWordPlace();
}

CPVT_WordPlace CPVT_VariableText::SearchWordPlace(
    const CFX_PointF& point) const {
  CFX_PointF pt = OutToIn(point);
  CPVT_WordPlace place = GetBeginWordPlace();
  int32_t nLeft = 0;
  int32_t nRight = fxcrt::CollectionSize<int32_t>(section_array_) - 1;
  int32_t nMid = fxcrt::CollectionSize<int32_t>(section_array_) / 2;
  bool bUp = true;
  bool bDown = true;
  while (nLeft <= nRight) {
    if (!fxcrt::IndexInBounds(section_array_, nMid)) {
      break;
    }
    CPVT_Section* pSection = section_array_[nMid].get();
    if (FXSYS_IsFloatBigger(pt.y, pSection->GetRect().top)) {
      bUp = false;
    }
    if (FXSYS_IsFloatBigger(pSection->GetRect().bottom, pt.y)) {
      bDown = false;
    }
    if (FXSYS_IsFloatSmaller(pt.y, pSection->GetRect().top)) {
      nRight = nMid - 1;
      nMid = (nLeft + nRight) / 2;
      continue;
    }
    if (FXSYS_IsFloatBigger(pt.y, pSection->GetRect().bottom)) {
      nLeft = nMid + 1;
      nMid = (nLeft + nRight) / 2;
      continue;
    }
    place = pSection->SearchWordPlace(CFX_PointF(
        pt.x - pSection->GetRect().left, pt.y - pSection->GetRect().top));
    place.nSecIndex = nMid;
    return place;
  }
  if (bUp) {
    place = GetBeginWordPlace();
  }
  if (bDown) {
    place = GetEndWordPlace();
  }
  return place;
}

CPVT_WordPlace CPVT_VariableText::GetUpWordPlace(
    const CPVT_WordPlace& place,
    const CFX_PointF& point) const {
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return place;
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  CPVT_WordPlace temp = place;
  CFX_PointF pt = OutToIn(point);
  if (temp.nLineIndex-- > 0) {
    return pSection->SearchWordPlace(pt.x - pSection->GetRect().left, temp);
  }
  if (temp.nSecIndex-- > 0) {
    if (fxcrt::IndexInBounds(section_array_, temp.nSecIndex)) {
      CPVT_Section* pLastSection = section_array_[temp.nSecIndex].get();
      temp.nLineIndex = pLastSection->GetLineArraySize() - 1;
      return pLastSection->SearchWordPlace(pt.x - pLastSection->GetRect().left,
                                           temp);
    }
  }
  return place;
}

CPVT_WordPlace CPVT_VariableText::GetDownWordPlace(
    const CPVT_WordPlace& place,
    const CFX_PointF& point) const {
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return place;
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  CPVT_WordPlace temp = place;
  CFX_PointF pt = OutToIn(point);
  if (temp.nLineIndex++ < pSection->GetLineArraySize() - 1) {
    return pSection->SearchWordPlace(pt.x - pSection->GetRect().left, temp);
  }
  temp.AdvanceSection();
  if (!fxcrt::IndexInBounds(section_array_, temp.nSecIndex)) {
    return place;
  }

  return section_array_[temp.nSecIndex]->SearchWordPlace(
      pt.x - pSection->GetRect().left, temp);
}

CPVT_WordPlace CPVT_VariableText::GetLineBeginPlace(
    const CPVT_WordPlace& place) const {
  return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, -1);
}

CPVT_WordPlace CPVT_VariableText::GetLineEndPlace(
    const CPVT_WordPlace& place) const {
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return place;
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  const CPVT_Section::Line* pLine =
      pSection->GetLineFromArray(place.nLineIndex);
  if (!pLine) {
    return place;
  }

  return pLine->GetEndWordPlace();
}

CPVT_WordPlace CPVT_VariableText::GetSectionBeginPlace(
    const CPVT_WordPlace& place) const {
  return CPVT_WordPlace(place.nSecIndex, 0, -1);
}

CPVT_WordPlace CPVT_VariableText::GetSectionEndPlace(
    const CPVT_WordPlace& place) const {
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return place;
  }

  return section_array_[place.nSecIndex]->GetEndWordPlace();
}

int32_t CPVT_VariableText::GetTotalWords() const {
  int32_t nTotal = 0;
  for (const auto& pSection : section_array_) {
    nTotal += pSection->GetWordArraySize() + kReturnLength;
  }
  return nTotal - kReturnLength;
}

CPVT_WordPlace CPVT_VariableText::AddSection(const CPVT_WordPlace& place) {
  if (IsValid() && !multi_line_) {
    return place;
  }

  int32_t nSecIndex = std::clamp(
      place.nSecIndex, 0, fxcrt::CollectionSize<int32_t>(section_array_));

  auto pSection = std::make_unique<CPVT_Section>(this);
  pSection->SetRect(CPVT_FloatRect());
  pSection->SetPlaceIndex(nSecIndex);
  section_array_.insert(section_array_.begin() + nSecIndex,
                        std::move(pSection));
  return place;
}

CPVT_WordPlace CPVT_VariableText::AddLine(const CPVT_WordPlace& place,
                                          const CPVT_LineInfo& lineinfo) {
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return place;
  }

  return section_array_[place.nSecIndex]->AddLine(lineinfo);
}

CPVT_WordPlace CPVT_VariableText::AddWord(const CPVT_WordPlace& place,
                                          const CPVT_WordInfo& wordinfo) {
  if (section_array_.empty()) {
    return place;
  }

  CPVT_WordPlace newplace = place;
  newplace.nSecIndex =
      std::clamp(newplace.nSecIndex, 0,
                 fxcrt::CollectionSize<int32_t>(section_array_) - 1);
  return section_array_[newplace.nSecIndex]->AddWord(newplace, wordinfo);
}

void CPVT_VariableText::SetPlateRect(const CFX_FloatRect& rect) {
  plate_rect_ = rect;
}

CFX_FloatRect CPVT_VariableText::GetContentRect() const {
  return InToOut(content_rect_);
}

const CFX_FloatRect& CPVT_VariableText::GetPlateRect() const {
  return plate_rect_;
}

float CPVT_VariableText::GetWordFontSize() const {
  return GetFontSize();
}

float CPVT_VariableText::GetWordWidth(int32_t nFontIndex,
                                      uint16_t Word,
                                      uint16_t SubWord,
                                      float fFontSize,
                                      float fWordTail) const {
  return GetCharWidth(nFontIndex, Word, SubWord) * fFontSize * kFontScale +
         fWordTail;
}

float CPVT_VariableText::GetWordWidth(const CPVT_WordInfo& WordInfo) const {
  return GetWordWidth(WordInfo.nFontIndex, WordInfo.Word, GetSubWord(),
                      GetWordFontSize(), WordInfo.fWordTail);
}

float CPVT_VariableText::GetLineAscent() {
  return GetFontAscent(GetDefaultFontIndex(), GetFontSize());
}

float CPVT_VariableText::GetLineDescent() {
  return GetFontDescent(GetDefaultFontIndex(), GetFontSize());
}

float CPVT_VariableText::GetFontAscent(int32_t nFontIndex,
                                       float fFontSize) const {
  float ascent = vt_provider_ ? vt_provider_->GetTypeAscent(nFontIndex) : 0;
  return ascent * fFontSize * kFontScale;
}

float CPVT_VariableText::GetFontDescent(int32_t nFontIndex,
                                        float fFontSize) const {
  float descent = vt_provider_ ? vt_provider_->GetTypeDescent(nFontIndex) : 0;
  return descent * fFontSize * kFontScale;
}

float CPVT_VariableText::GetWordAscent(const CPVT_WordInfo& WordInfo,
                                       float fFontSize) const {
  return GetFontAscent(WordInfo.nFontIndex, fFontSize);
}

float CPVT_VariableText::GetWordDescent(const CPVT_WordInfo& WordInfo,
                                        float fFontSize) const {
  return GetFontDescent(WordInfo.nFontIndex, fFontSize);
}

float CPVT_VariableText::GetWordAscent(const CPVT_WordInfo& WordInfo) const {
  return GetFontAscent(WordInfo.nFontIndex, GetWordFontSize());
}

float CPVT_VariableText::GetWordDescent(const CPVT_WordInfo& WordInfo) const {
  return GetFontDescent(WordInfo.nFontIndex, GetWordFontSize());
}

float CPVT_VariableText::GetLineLeading() {
  return line_leading_;
}

float CPVT_VariableText::GetLineIndent() {
  return 0.0f;
}

void CPVT_VariableText::ClearSectionRightWords(const CPVT_WordPlace& place) {
  CPVT_WordPlace wordplace = PrevLineHeaderPlace(place);
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return;
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  pSection->EraseWordsFrom(wordplace.nWordIndex + 1);
}

CPVT_WordPlace CPVT_VariableText::PrevLineHeaderPlace(
    const CPVT_WordPlace& place) const {
  if (place.nWordIndex < 0 && place.nLineIndex > 0) {
    return GetPrevWordPlace(place);
  }
  return place;
}

CPVT_WordPlace CPVT_VariableText::NextLineHeaderPlace(
    const CPVT_WordPlace& place) const {
  if (place.nWordIndex < 0 && place.nLineIndex > 0) {
    return GetNextWordPlace(place);
  }
  return place;
}

void CPVT_VariableText::ClearEmptySection(const CPVT_WordPlace& place) {
  if (place.nSecIndex == 0 && section_array_.size() == 1) {
    return;
  }

  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return;
  }

  if (section_array_[place.nSecIndex]->GetWordArraySize() != 0) {
    return;
  }

  section_array_.erase(section_array_.begin() + place.nSecIndex);
}

void CPVT_VariableText::ClearEmptySections(const CPVT_WordRange& PlaceRange) {
  CPVT_WordPlace wordplace;
  for (int32_t s = PlaceRange.EndPos.nSecIndex;
       s > PlaceRange.BeginPos.nSecIndex; s--) {
    wordplace.nSecIndex = s;
    ClearEmptySection(wordplace);
  }
}

void CPVT_VariableText::LinkLatterSection(const CPVT_WordPlace& place) {
  CPVT_WordPlace oldplace = PrevLineHeaderPlace(place);
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex + 1)) {
    return;
  }

  CPVT_Section* pNextSection = section_array_[place.nSecIndex + 1].get();
  if (fxcrt::IndexInBounds(section_array_, oldplace.nSecIndex)) {
    CPVT_Section* pSection = section_array_[oldplace.nSecIndex].get();
    for (int32_t i = 0; i < pNextSection->GetWordArraySize(); ++i) {
      oldplace.nWordIndex++;
      pSection->AddWord(oldplace, *pNextSection->GetWordFromArray(i));
    }
  }
  section_array_.erase(section_array_.begin() + place.nSecIndex + 1);
}

void CPVT_VariableText::ClearWords(const CPVT_WordRange& PlaceRange) {
  CPVT_WordRange NewRange;
  NewRange.BeginPos = PrevLineHeaderPlace(PlaceRange.BeginPos);
  NewRange.EndPos = PrevLineHeaderPlace(PlaceRange.EndPos);
  for (int32_t s = NewRange.EndPos.nSecIndex; s >= NewRange.BeginPos.nSecIndex;
       s--) {
    if (fxcrt::IndexInBounds(section_array_, s)) {
      section_array_[s]->ClearWords(NewRange);
    }
  }
}

CPVT_WordPlace CPVT_VariableText::ClearLeftWord(const CPVT_WordPlace& place) {
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return place;
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  CPVT_WordPlace leftplace = GetPrevWordPlace(place);
  if (leftplace == place) {
    return place;
  }

  if (leftplace.nSecIndex != place.nSecIndex) {
    if (pSection->GetWordArraySize() == 0) {
      ClearEmptySection(place);
    } else {
      LinkLatterSection(leftplace);
    }
  } else {
    pSection->ClearWord(place);
  }
  return leftplace;
}

CPVT_WordPlace CPVT_VariableText::ClearRightWord(const CPVT_WordPlace& place) {
  if (!fxcrt::IndexInBounds(section_array_, place.nSecIndex)) {
    return place;
  }

  CPVT_Section* pSection = section_array_[place.nSecIndex].get();
  CPVT_WordPlace rightplace = NextLineHeaderPlace(GetNextWordPlace(place));
  if (rightplace == place) {
    return place;
  }

  if (rightplace.nSecIndex != place.nSecIndex) {
    LinkLatterSection(place);
  } else {
    pSection->ClearWord(rightplace);
  }
  return place;
}

void CPVT_VariableText::RearrangeAll() {
  Rearrange(CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
}

void CPVT_VariableText::RearrangePart(const CPVT_WordRange& PlaceRange) {
  Rearrange(PlaceRange);
}

void CPVT_VariableText::Rearrange(const CPVT_WordRange& PlaceRange) {
  CPVT_FloatRect rcRet;
  if (IsValid()) {
    if (auto_font_size_) {
      SetFontSize(GetAutoFontSize());
      rcRet = RearrangeSections(
          CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
    } else {
      rcRet = RearrangeSections(PlaceRange);
    }
  }
  content_rect_ = rcRet;
}

float CPVT_VariableText::GetAutoFontSize() {
  constexpr size_t kFullSize = kFontSizeSteps.size();
  constexpr size_t kQuarterSize = kFullSize / 4;

  static_assert(kFullSize >= 4,
                "kFontSizeSteps.size() must be at least 4 to ensure "
                "kQuarterSize is not zero.");

  if (GetPlateWidth() <= 0) {
    return 0;
  }

  size_t span_size = IsMultiLine() ? kQuarterSize : kFullSize;
  auto font_span = pdfium::span(kFontSizeSteps).first(span_size);

  constexpr bool kUnusedValue = true;
  auto it = std::lower_bound(
      font_span.begin(), font_span.end(), kUnusedValue,
      [this](uint8_t font_size, bool) { return !IsBigger(font_size); });

  if (it == font_span.end()) {
    return static_cast<float>(font_span.back());
  }

  if (it == font_span.begin()) {
    return static_cast<float>(*it);
  }
  return static_cast<float>(font_span[it - font_span.begin() - 1]);
}

bool CPVT_VariableText::IsBigger(float fFontSize) const {
  CFX_SizeF szTotal;
  for (const auto& pSection : section_array_) {
    CFX_SizeF size = pSection->GetSectionSize(fFontSize);
    szTotal.width = std::max(size.width, szTotal.width);
    szTotal.height += size.height;
    if (FXSYS_IsFloatBigger(szTotal.width, GetPlateWidth()) ||
        FXSYS_IsFloatBigger(szTotal.height, GetPlateHeight())) {
      return true;
    }
  }
  return false;
}

CPVT_FloatRect CPVT_VariableText::RearrangeSections(
    const CPVT_WordRange& PlaceRange) {
  float fPosY = 0;
  CPVT_FloatRect rcRet;
  for (int32_t s = 0, sz = fxcrt::CollectionSize<int32_t>(section_array_);
       s < sz; s++) {
    CPVT_WordPlace place;
    place.nSecIndex = s;
    CPVT_Section* pSection = section_array_[s].get();
    pSection->SetPlace(place);
    CPVT_FloatRect rcSec = pSection->GetRect();
    if (s >= PlaceRange.BeginPos.nSecIndex) {
      if (s <= PlaceRange.EndPos.nSecIndex) {
        rcSec = pSection->Rearrange();
        rcSec.top += fPosY;
        rcSec.bottom += fPosY;
      } else {
        float fOldHeight = pSection->GetRect().bottom - pSection->GetRect().top;
        rcSec.top = fPosY;
        rcSec.bottom = fPosY + fOldHeight;
      }
      pSection->SetRect(rcSec);
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

int CPVT_VariableText::GetCharWidth(int32_t nFontIndex,
                                    uint16_t Word,
                                    uint16_t SubWord) const {
  if (!vt_provider_) {
    return 0;
  }
  uint16_t word = SubWord ? SubWord : Word;
  return vt_provider_->GetCharWidth(nFontIndex, word);
}

int32_t CPVT_VariableText::GetWordFontIndex(uint16_t word,
                                            FX_Charset charset,
                                            int32_t nFontIndex) {
  return vt_provider_
             ? vt_provider_->GetWordFontIndex(word, charset, nFontIndex)
             : -1;
}

int32_t CPVT_VariableText::GetDefaultFontIndex() {
  return vt_provider_ ? vt_provider_->GetDefaultFontIndex() : -1;
}

CPVT_VariableText::Iterator* CPVT_VariableText::GetIterator() {
  if (!vt_iterator_) {
    vt_iterator_ = std::make_unique<CPVT_VariableText::Iterator>(this);
  }
  return vt_iterator_.get();
}

void CPVT_VariableText::SetProvider(Provider* pProvider) {
  vt_provider_ = pProvider;
}

CPVT_VariableText::Provider* CPVT_VariableText::GetProvider() {
  return vt_provider_;
}

CFX_PointF CPVT_VariableText::GetBTPoint() const {
  return CFX_PointF(plate_rect_.left, plate_rect_.top);
}

CFX_PointF CPVT_VariableText::GetETPoint() const {
  return CFX_PointF(plate_rect_.right, plate_rect_.bottom);
}

CFX_PointF CPVT_VariableText::InToOut(const CFX_PointF& point) const {
  return CFX_PointF(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
}

CFX_PointF CPVT_VariableText::OutToIn(const CFX_PointF& point) const {
  return CFX_PointF(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
}

CFX_FloatRect CPVT_VariableText::InToOut(const CPVT_FloatRect& rect) const {
  CFX_PointF ptLeftTop = InToOut(CFX_PointF(rect.left, rect.top));
  CFX_PointF ptRightBottom = InToOut(CFX_PointF(rect.right, rect.bottom));
  return CFX_FloatRect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                       ptLeftTop.y);
}
