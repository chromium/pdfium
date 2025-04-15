// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_cmapparser.h"

#include <array>
#include <iterator>

#include "core/fpdfapi/cmaps/fpdf_cmaps.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"

namespace {

ByteStringView CMap_GetString(ByteStringView word) {
  if (word.GetLength() <= 2) {
    return ByteStringView();
  }
  return word.Last(word.GetLength() - 2);
}

}  // namespace

CPDF_CMapParser::CPDF_CMapParser(CPDF_CMap* pCMap) : cmap_(pCMap) {}

CPDF_CMapParser::~CPDF_CMapParser() {
  cmap_->SetAdditionalMappings(std::move(additional_charcode_to_cidmappings_));
  cmap_->SetMixedFourByteLeadingRanges(std::move(ranges_));
}

void CPDF_CMapParser::ParseWord(ByteStringView word) {
  DCHECK(!word.IsEmpty());

  if (word == "begincidchar") {
    status_ = kProcessingCidChar;
    code_seq_ = 0;
  } else if (word == "begincidrange") {
    status_ = kProcessingCidRange;
    code_seq_ = 0;
  } else if (word == "endcidrange" || word == "endcidchar") {
    status_ = kStart;
  } else if (word == "/WMode") {
    status_ = kProcessingWMode;
  } else if (word == "/Registry") {
    status_ = kProcessingRegistry;
  } else if (word == "/Ordering") {
    status_ = kProcessingOrdering;
  } else if (word == "/Supplement") {
    status_ = kProcessingSupplement;
  } else if (word == "begincodespacerange") {
    status_ = kProcessingCodeSpaceRange;
    code_seq_ = 0;
  } else if (word == "usecmap") {
  } else if (status_ == kProcessingCidChar) {
    HandleCid(word);
  } else if (status_ == kProcessingCidRange) {
    HandleCid(word);
  } else if (status_ == kProcessingRegistry) {
    status_ = kStart;
  } else if (status_ == kProcessingOrdering) {
    cmap_->SetCharset(CharsetFromOrdering(CMap_GetString(word)));
    status_ = kStart;
  } else if (status_ == kProcessingSupplement) {
    status_ = kStart;
  } else if (status_ == kProcessingWMode) {
    cmap_->SetVertical(GetCode(word) != 0);
    status_ = kStart;
  } else if (status_ == kProcessingCodeSpaceRange) {
    HandleCodeSpaceRange(word);
  }
  last_word_ = word;
}

void CPDF_CMapParser::HandleCid(ByteStringView word) {
  DCHECK(status_ == kProcessingCidChar || status_ == kProcessingCidRange);
  bool bChar = status_ == kProcessingCidChar;

  code_points_[code_seq_] = GetCode(word);
  code_seq_++;
  int nRequiredCodePoints = bChar ? 2 : 3;
  if (code_seq_ < nRequiredCodePoints) {
    return;
  }

  uint32_t StartCode = code_points_[0];
  uint32_t EndCode;
  uint16_t StartCID;
  if (bChar) {
    EndCode = StartCode;
    StartCID = static_cast<uint16_t>(code_points_[1]);
  } else {
    EndCode = code_points_[1];
    StartCID = static_cast<uint16_t>(code_points_[2]);
  }
  if (EndCode < CPDF_CMap::kDirectMapTableSize) {
    cmap_->SetDirectCharcodeToCIDTableRange(StartCode, EndCode, StartCID);
  } else {
    additional_charcode_to_cidmappings_.push_back(
        {StartCode, EndCode, StartCID});
  }
  code_seq_ = 0;
}

void CPDF_CMapParser::HandleCodeSpaceRange(ByteStringView word) {
  if (word != "endcodespacerange") {
    if (word.IsEmpty() || word[0] != '<') {
      return;
    }

    if (code_seq_ % 2) {
      std::optional<CPDF_CMap::CodeRange> range =
          GetCodeRange(last_word_.AsStringView(), word);
      if (range.has_value()) {
        pending_ranges_.push_back(range.value());
      }
    }
    code_seq_++;
    return;
  }

  size_t nSegs = ranges_.size() + pending_ranges_.size();
  if (nSegs == 1) {
    const auto& first_range =
        !ranges_.empty() ? ranges_[0] : pending_ranges_[0];
    cmap_->SetCodingScheme(first_range.char_size_ == 2 ? CPDF_CMap::TwoBytes
                                                       : CPDF_CMap::OneByte);
  } else if (nSegs > 1) {
    cmap_->SetCodingScheme(CPDF_CMap::MixedFourBytes);
    ranges_.reserve(nSegs);
    std::move(pending_ranges_.begin(), pending_ranges_.end(),
              std::back_inserter(ranges_));
    pending_ranges_.clear();
  }
  status_ = kStart;
}

// static
uint32_t CPDF_CMapParser::GetCode(ByteStringView word) {
  if (word.IsEmpty()) {
    return 0;
  }

  FX_SAFE_UINT32 num = 0;
  if (word[0] == '<') {
    for (size_t i = 1; i < word.GetLength() && FXSYS_IsHexDigit(word[i]); ++i) {
      num = num * 16 + FXSYS_HexCharToInt(word[i]);
      if (!num.IsValid()) {
        return 0;
      }
    }
    return num.ValueOrDie();
  }

  for (size_t i = 0;
       i < word.GetLength() && FXSYS_IsDecimalDigit(word.CharAt(i)); ++i) {
    num = num * 10 + FXSYS_DecimalCharToInt(word.CharAt(i));
    if (!num.IsValid()) {
      return 0;
    }
  }
  return num.ValueOrDie();
}

// static
std::optional<CPDF_CMap::CodeRange> CPDF_CMapParser::GetCodeRange(
    ByteStringView first,
    ByteStringView second) {
  if (first.IsEmpty() || first[0] != '<') {
    return std::nullopt;
  }

  size_t i;
  for (i = 1; i < first.GetLength(); ++i) {
    if (first[i] == '>') {
      break;
    }
  }
  size_t char_size = (i - 1) / 2;
  if (char_size > 4) {
    return std::nullopt;
  }

  CPDF_CMap::CodeRange range;
  range.char_size_ = char_size;
  for (i = 0; i < range.char_size_; ++i) {
    uint8_t digit1 = first[i * 2 + 1];
    uint8_t digit2 = first[i * 2 + 2];
    range.lower_[i] =
        FXSYS_HexCharToInt(digit1) * 16 + FXSYS_HexCharToInt(digit2);
  }

  size_t size = second.GetLength();
  for (i = 0; i < range.char_size_; ++i) {
    size_t i1 = i * 2 + 1;
    size_t i2 = i1 + 1;
    uint8_t digit1 = i1 < size ? second[i1] : '0';
    uint8_t digit2 = i2 < size ? second[i2] : '0';
    range.upper_[i] =
        FXSYS_HexCharToInt(digit1) * 16 + FXSYS_HexCharToInt(digit2);
  }
  return range;
}

// static
CIDSet CPDF_CMapParser::CharsetFromOrdering(ByteStringView ordering) {
  static const std::array<const char*, CIDSET_NUM_SETS> kCharsetNames = {
      {nullptr, "GB1", "CNS1", "Japan1", "Korea1", "UCS"}};

  for (size_t charset = 1; charset < std::size(kCharsetNames); ++charset) {
    if (ordering == kCharsetNames[charset]) {
      return static_cast<CIDSet>(charset);
    }
  }
  return CIDSET_UNKNOWN;
}
