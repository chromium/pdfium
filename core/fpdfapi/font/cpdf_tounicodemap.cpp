// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_tounicodemap.h"

#include <set>
#include <utility>
#include <variant>

#include "core/fpdfapi/font/cpdf_cid2unicodemap.h"
#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"

namespace {

constexpr uint32_t kCidLimit = 0xffff;

// Per spec, bfchar and bfrange sections should have at most 100 entries. Some
// PDFs violate this part of the spec and other PDF parsers tolerate it. So set
// an artificially high limit that should be good enough for PDFs in the wild,
// but not too high to prevent fuzzers from slowing down fuzzing.
constexpr int kOutOfSpecBFLimit = 160000;

WideString StringDataAdd(WideString str) {
  WideString ret;
  wchar_t value = 1;
  for (size_t i = str.GetLength(); i > 0; --i) {
    wchar_t ch = str[i - 1] + value;
    if (ch < str[i - 1]) {
      ret.InsertAtFront(0);
    } else {
      ret.InsertAtFront(ch);
      value = 0;
    }
  }
  if (value)
    ret.InsertAtFront(value);
  return ret;
}

}  // namespace

CPDF_ToUnicodeMap::CPDF_ToUnicodeMap(RetainPtr<const CPDF_Stream> pStream) {
  Load(std::move(pStream));
}

CPDF_ToUnicodeMap::~CPDF_ToUnicodeMap() = default;

WideString CPDF_ToUnicodeMap::Lookup(uint32_t charcode) const {
  auto it = m_Multimap.find(charcode);
  if (it == m_Multimap.end()) {
    if (!m_pBaseMap)
      return WideString();
    return WideString(
        m_pBaseMap->UnicodeFromCID(static_cast<uint16_t>(charcode)));
  }

  uint32_t value = *it->second.begin();
  wchar_t unicode = static_cast<wchar_t>(value & 0xffff);
  if (unicode != 0xffff)
    return WideString(unicode);

  size_t index = value >> 16;
  return index < m_MultiCharVec.size() ? m_MultiCharVec[index] : WideString();
}

uint32_t CPDF_ToUnicodeMap::ReverseLookup(wchar_t unicode) const {
  for (const auto& pair : m_Multimap) {
    if (pdfium::Contains(pair.second, static_cast<uint32_t>(unicode)))
      return pair.first;
  }
  return 0;
}

size_t CPDF_ToUnicodeMap::GetUnicodeCountByCharcodeForTesting(
    uint32_t charcode) const {
  auto it = m_Multimap.find(charcode);
  return it != m_Multimap.end() ? it->second.size() : 0u;
}

// static
std::optional<uint32_t> CPDF_ToUnicodeMap::StringToCode(ByteStringView input) {
  // Ignore whitespaces within `input`. See https://crbug.com/pdfium/2065.
  std::set<char> seen_whitespace_chars;
  for (char c : input) {
    if (PDFCharIsWhitespace(c)) {
      seen_whitespace_chars.insert(c);
    }
  }
  ByteString str_without_whitespace_chars;  // Must outlive `str`.
  ByteStringView str;
  if (seen_whitespace_chars.empty()) {
    str = input;
  } else {
    str_without_whitespace_chars.Reserve(input.GetLength());
    for (char c : input) {
      if (!pdfium::Contains(seen_whitespace_chars, c)) {
        str_without_whitespace_chars += c;
      }
    }
    str = str_without_whitespace_chars.AsStringView();
  }

  size_t len = str.GetLength();
  if (len <= 2 || str[0] != '<' || str[len - 1] != '>')
    return std::nullopt;

  FX_SAFE_UINT32 code = 0;
  for (char c : str.Substr(1, len - 2)) {
    if (!FXSYS_IsHexDigit(c))
      return std::nullopt;

    code = code * 16 + FXSYS_HexCharToInt(c);
    if (!code.IsValid())
      return std::nullopt;
  }
  return std::optional<uint32_t>(code.ValueOrDie());
}

// static
WideString CPDF_ToUnicodeMap::StringToWideString(ByteStringView str) {
  size_t len = str.GetLength();
  if (len <= 2 || str[0] != '<' || str[len - 1] != '>')
    return WideString();

  WideString result;
  int byte_pos = 0;
  wchar_t ch = 0;
  for (char c : str.Substr(1, len - 2)) {
    if (!FXSYS_IsHexDigit(c))
      break;

    ch = ch * 16 + FXSYS_HexCharToInt(c);
    byte_pos++;
    if (byte_pos == 4) {
      result += ch;
      byte_pos = 0;
      ch = 0;
    }
  }
  return result;
}

void CPDF_ToUnicodeMap::Load(RetainPtr<const CPDF_Stream> pStream) {
  CIDSet cid_set = CIDSET_UNKNOWN;
  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(pStream));
  pAcc->LoadAllDataFiltered();
  CPDF_SimpleParser parser(pAcc->GetSpan());
  ByteStringView previous_word;
  while (true) {
    ByteStringView word = parser.GetWord();
    if (word.IsEmpty()) {
      break;
    }

    if (word == "beginbfchar") {
      word = HandleBeginBFChar(parser, previous_word);
    } else if (word == "beginbfrange") {
      word = HandleBeginBFRange(parser, previous_word);
    } else if (word == "/Adobe-Korea1-UCS2") {
      cid_set = CIDSET_KOREA1;
    } else if (word == "/Adobe-Japan1-UCS2") {
      cid_set = CIDSET_JAPAN1;
    } else if (word == "/Adobe-CNS1-UCS2") {
      cid_set = CIDSET_CNS1;
    } else if (word == "/Adobe-GB1-UCS2") {
      cid_set = CIDSET_GB1;
    }

    previous_word = word;
  }
  if (cid_set != CIDSET_UNKNOWN) {
    m_pBaseMap = CPDF_FontGlobals::GetInstance()->GetCID2UnicodeMap(cid_set);
  }
}

ByteStringView CPDF_ToUnicodeMap::HandleBeginBFChar(
    CPDF_SimpleParser& parser,
    ByteStringView previous_word) {
  struct CodeWord {
    uint32_t code;
    ByteStringView word;
  };
  std::vector<CodeWord> code_words;

  const int raw_count = StringToInt(previous_word);
  bool is_valid = raw_count >= 0 && raw_count <= kOutOfSpecBFLimit;
  const size_t expected_count = is_valid ? static_cast<size_t>(raw_count) : 0;
  code_words.reserve(expected_count);

  ByteStringView word;
  while (true) {
    word = parser.GetWord();
    if (word.IsEmpty() || word == "endbfchar") {
      break;
    }
    if (!is_valid) {
      continue;  // Keep consuming words. Do nothing else.
    }

    std::optional<uint32_t> code = StringToCode(word);
    if (!code.has_value() || code.value() > kCidLimit) {
      is_valid = false;
      continue;
    }

    word = parser.GetWord();
    code_words.emplace_back(CodeWord{code.value(), word});

    if (code_words.size() > expected_count) {
      is_valid = false;
    }
  }

  if (is_valid && code_words.size() == expected_count) {
    for (const auto& entry : code_words) {
      SetCode(entry.code, StringToWideString(entry.word));
    }
  }
  return word;
}

ByteStringView CPDF_ToUnicodeMap::HandleBeginBFRange(
    CPDF_SimpleParser& parser,
    ByteStringView previous_word) {
  struct CodeWordRange {
    uint32_t low_code;
    std::vector<ByteStringView> code_words;
  };
  struct MultimapSingleDestRange {
    uint32_t low_code;
    uint32_t high_code;
    uint32_t start_value;
  };
  struct MultimapMultiDestRange {
    uint32_t low_code;
    std::vector<WideString> retcodes;
  };
  using Range = std::variant<CodeWordRange, MultimapSingleDestRange,
                             MultimapMultiDestRange>;
  std::vector<Range> ranges;

  const int raw_count = StringToInt(previous_word);
  bool is_valid = raw_count >= 0 && raw_count <= kOutOfSpecBFLimit;
  const size_t expected_count = is_valid ? static_cast<size_t>(raw_count) : 0;
  ranges.reserve(expected_count);

  ByteStringView word;
  while (true) {
    word = parser.GetWord();
    if (word.IsEmpty() || word == "endbfrange") {
      break;
    }
    if (!is_valid) {
      continue;  // Keep consuming words. Do nothing else.
    }

    std::optional<uint32_t> lowcode_opt = StringToCode(word);
    if (!lowcode_opt.has_value()) {
      is_valid = false;
      continue;
    }

    word = parser.GetWord();
    std::optional<uint32_t> highcode_opt = StringToCode(word);
    if (!highcode_opt.has_value()) {
      is_valid = false;
      continue;
    }

    uint32_t lowcode = lowcode_opt.value();
    uint32_t highcode = (lowcode & 0xffffff00) | (highcode_opt.value() & 0xff);
    if (lowcode > kCidLimit || highcode > kCidLimit || lowcode > highcode) {
      is_valid = false;
      continue;
    }

    word = parser.GetWord();
    ByteStringView start = word;
    if (start == "[") {
      CodeWordRange range;
      range.low_code = lowcode;
      range.code_words.reserve(1 + highcode - lowcode);
      for (uint32_t code = lowcode; code <= highcode; ++code) {
        word = parser.GetWord();
        range.code_words.push_back(word);
      }
      ranges.push_back(std::move(range));

      if (ranges.size() > expected_count) {
        is_valid = false;
        continue;
      }

      word = parser.GetWord();
      if (word != "]") {
        is_valid = false;
      }
      continue;
    }

    WideString destcode = StringToWideString(start);
    if (destcode.GetLength() == 1) {
      std::optional<uint32_t> value_or_error = StringToCode(start);
      if (!value_or_error.has_value()) {
        is_valid = false;
        continue;
      }

      ranges.push_back(
          MultimapSingleDestRange{.low_code = lowcode,
                                  .high_code = highcode,
                                  .start_value = value_or_error.value()});
    } else {
      MultimapMultiDestRange range;
      range.low_code = lowcode;
      range.retcodes.reserve(1 + highcode - lowcode);
      range.retcodes.push_back(destcode);
      for (uint32_t code = lowcode + 1; code <= highcode; ++code) {
        WideString retcode = StringDataAdd(range.retcodes.back());
        range.retcodes.push_back(std::move(retcode));
      }
      ranges.push_back(std::move(range));
    }

    if (ranges.size() > expected_count) {
      is_valid = false;
    }
  }

  if (is_valid && ranges.size() == expected_count) {
    for (const auto& entry : ranges) {
      if (std::holds_alternative<CodeWordRange>(entry)) {
        const auto& range = std::get<CodeWordRange>(entry);
        uint32_t code = range.low_code;
        for (const auto& code_word : range.code_words) {
          SetCode(code, StringToWideString(code_word));
          ++code;
        }
      } else if (std::holds_alternative<MultimapSingleDestRange>(entry)) {
        const auto& range = std::get<MultimapSingleDestRange>(entry);
        uint32_t value = range.start_value;
        for (uint32_t code = range.low_code; code <= range.high_code; ++code) {
          InsertIntoMultimap(code, value++);
        }
      } else {
        CHECK(std::holds_alternative<MultimapMultiDestRange>(entry));
        const auto& range = std::get<MultimapMultiDestRange>(entry);
        uint32_t code = range.low_code;
        for (const auto& retcode : range.retcodes) {
          InsertIntoMultimap(code, GetMultiCharIndexIndicator());
          m_MultiCharVec.push_back(retcode);
          ++code;
        }
      }
    }
  }
  return word;
}

uint32_t CPDF_ToUnicodeMap::GetMultiCharIndexIndicator() const {
  FX_SAFE_UINT32 uni = m_MultiCharVec.size();
  uni = uni * 0x10000 + 0xffff;
  return uni.ValueOrDefault(0);
}

void CPDF_ToUnicodeMap::SetCode(uint32_t srccode, WideString destcode) {
  size_t len = destcode.GetLength();
  if (len == 0)
    return;

  if (len == 1) {
    InsertIntoMultimap(srccode, destcode[0]);
  } else {
    InsertIntoMultimap(srccode, GetMultiCharIndexIndicator());
    m_MultiCharVec.push_back(destcode);
  }
}

void CPDF_ToUnicodeMap::InsertIntoMultimap(uint32_t code, uint32_t destcode) {
  auto it = m_Multimap.find(code);
  if (it == m_Multimap.end()) {
    m_Multimap.emplace(code, std::set<uint32_t>{destcode});
    return;
  }

  it->second.emplace(destcode);
}
