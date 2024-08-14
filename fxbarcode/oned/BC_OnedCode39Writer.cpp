// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2010 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fxbarcode/oned/BC_OnedCode39Writer.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/stl_util.h"
#include "fxbarcode/BC_Writer.h"
#include "fxbarcode/oned/BC_OneDimWriter.h"

namespace {

constexpr auto kOnedCode39Alphabet = fxcrt::ToArray<const char>(
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
     'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
     'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.', ' ', '*', '$', '/', '+', '%'});
constexpr size_t kOnedCode39AlphabetLen = std::size(kOnedCode39Alphabet);

constexpr auto kOnedCode39Checksum = fxcrt::ToArray<const char>(
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
     'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
     'U', 'V', 'W', 'X', 'Y', 'Z', '-', '.', ' ', '$', '/', '+', '%'});
static_assert(std::size(kOnedCode39Checksum) == 43, "Wrong size");

constexpr auto kOnedCode39CharacterEncoding = fxcrt::ToArray<const uint16_t>(
    {0x0034, 0x0121, 0x0061, 0x0160, 0x0031, 0x0130, 0x0070, 0x0025, 0x0124,
     0x0064, 0x0109, 0x0049, 0x0148, 0x0019, 0x0118, 0x0058, 0x000D, 0x010C,
     0x004C, 0x001C, 0x0103, 0x0043, 0x0142, 0x0013, 0x0112, 0x0052, 0x0007,
     0x0106, 0x0046, 0x0016, 0x0181, 0x00C1, 0x01C0, 0x0091, 0x0190, 0x00D0,
     0x0085, 0x0184, 0x00C4, 0x0094, 0x00A8, 0x00A2, 0x008A, 0x002A});
static_assert(std::size(kOnedCode39CharacterEncoding) == 44, "Wrong size");

bool IsInOnedCode39Alphabet(wchar_t ch) {
  return FXSYS_IsDecimalDigit(ch) || (ch >= L'A' && ch <= L'Z') || ch == L'-' ||
         ch == L'.' || ch == L' ' || ch == L'*' || ch == L'$' || ch == L'/' ||
         ch == L'+' || ch == L'%';
}

char CalcCheckSum(const ByteString& contents) {
  if (contents.GetLength() > 80)
    return '*';

  int32_t checksum = 0;
  for (const auto& c : contents) {
    size_t j = 0;
    for (; j < kOnedCode39AlphabetLen; j++) {
      if (kOnedCode39Alphabet[j] == c) {
        if (c != '*')
          checksum += j;
        break;
      }
    }
    if (j >= kOnedCode39AlphabetLen)
      return '*';
  }
  return kOnedCode39Checksum[checksum % std::size(kOnedCode39Checksum)];
}

void ToIntArray(int16_t value, uint8_t ratio, pdfium::span<uint8_t> span) {
  for (size_t i = 0; i < span.size(); i++) {
    span[i] = (value & (1 << i)) == 0 ? 1 : ratio;
  }
}

}  // namespace

CBC_OnedCode39Writer::CBC_OnedCode39Writer() = default;

CBC_OnedCode39Writer::~CBC_OnedCode39Writer() = default;

bool CBC_OnedCode39Writer::CheckContentValidity(WideStringView contents) {
  return HasValidContentSize(contents) &&
         std::all_of(contents.begin(), contents.end(), IsInOnedCode39Alphabet);
}

WideString CBC_OnedCode39Writer::FilterContents(WideStringView contents) {
  WideString filtercontents;
  filtercontents.Reserve(contents.GetLength());
  for (size_t i = 0; i < contents.GetLength(); i++) {
    wchar_t ch = contents[i];
    if (ch == L'*' && (i == 0 || i == contents.GetLength() - 1)) {
      continue;
    }
    if (ch > 175) {
      i++;
      continue;
    }
    ch = FXSYS_ToUpperASCII(ch);
    if (IsInOnedCode39Alphabet(ch))
      filtercontents += ch;
  }
  return filtercontents;
}

WideString CBC_OnedCode39Writer::RenderTextContents(WideStringView contents) {
  WideString renderContents;
  for (size_t i = 0; i < contents.GetLength(); i++) {
    wchar_t ch = contents[i];
    if (ch == L'*' && (i == 0 || i == contents.GetLength() - 1)) {
      continue;
    }
    if (ch > 175) {
      i++;
      continue;
    }
    if (IsInOnedCode39Alphabet(FXSYS_ToUpperASCII(ch)))
      renderContents += ch;
  }
  return renderContents;
}

void CBC_OnedCode39Writer::SetTextLocation(BC_TEXT_LOC location) {
  m_locTextLoc = location;
}

bool CBC_OnedCode39Writer::SetWideNarrowRatio(int8_t ratio) {
  if (ratio < 2 || ratio > 3)
    return false;

  m_iWideNarrRatio = ratio;
  return true;
}

DataVector<uint8_t> CBC_OnedCode39Writer::Encode(const ByteString& contents) {
  char checksum = CalcCheckSum(contents);
  if (checksum == '*')
    return DataVector<uint8_t>();

  std::array<uint8_t, kArraySize> widths = {};
  constexpr int32_t kWideStrideNum = 3;
  constexpr int32_t kNarrowStrideNum = kArraySize - kWideStrideNum;
  ByteString encodedContents = contents;
  if (m_bCalcChecksum)
    encodedContents += checksum;
  m_iContentLen = encodedContents.GetLength();
  size_t code_width =
      (kWideStrideNum * m_iWideNarrRatio + kNarrowStrideNum) * 2 + 1 +
      m_iContentLen;
  for (size_t j = 0; j < m_iContentLen; j++) {
    for (size_t i = 0; i < kOnedCode39AlphabetLen; i++) {
      if (kOnedCode39Alphabet[i] != encodedContents[j]) {
        continue;
      }
      ToIntArray(kOnedCode39CharacterEncoding[i], m_iWideNarrRatio, widths);
      for (size_t k = 0; k < kArraySize; k++)
        code_width += widths[k];
    }
  }
  DataVector<uint8_t> result(code_width);
  auto result_span = pdfium::make_span(result);
  ToIntArray(kOnedCode39CharacterEncoding[39], m_iWideNarrRatio, widths);
  result_span = AppendPattern(result_span, widths, true);

  static constexpr uint8_t kNarrowWhite[] = {1};
  result_span = AppendPattern(result_span, kNarrowWhite, false);

  for (int32_t l = m_iContentLen - 1; l >= 0; l--) {
    for (size_t i = 0; i < kOnedCode39AlphabetLen; i++) {
      if (kOnedCode39Alphabet[i] != encodedContents[l]) {
        continue;
      }
      ToIntArray(kOnedCode39CharacterEncoding[i], m_iWideNarrRatio, widths);
      result_span = AppendPattern(result_span, widths, true);
    }
    result_span = AppendPattern(result_span, kNarrowWhite, false);
  }
  ToIntArray(kOnedCode39CharacterEncoding[39], m_iWideNarrRatio, widths);
  AppendPattern(result_span, widths, true);

  for (size_t i = 0; i < code_width / 2; i++) {
    result[i] ^= result[code_width - 1 - i];
    result[code_width - 1 - i] ^= result[i];
    result[i] ^= result[code_width - 1 - i];
  }
  return result;
}

bool CBC_OnedCode39Writer::encodedContents(WideStringView contents,
                                           WideString* result) {
  *result = WideString(contents);
  if (m_bCalcChecksum && m_bPrintChecksum) {
    WideString checksumContent = FilterContents(contents);
    ByteString str = checksumContent.ToUTF8();
    char checksum;
    checksum = CalcCheckSum(str);
    if (checksum == '*')
      return false;
    str += checksum;
    *result += checksum;
  }
  return true;
}

bool CBC_OnedCode39Writer::RenderResult(WideStringView contents,
                                        pdfium::span<const uint8_t> code) {
  WideString encodedCon;
  if (!encodedContents(contents, &encodedCon))
    return false;
  return CBC_OneDimWriter::RenderResult(encodedCon.AsStringView(), code);
}
