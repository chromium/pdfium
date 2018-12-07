// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2006-2007 Jeremias Maerki.
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

#include "fxbarcode/datamatrix/BC_HighLevelEncoder.h"

#include <limits>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "fxbarcode/common/BC_CommonBitMatrix.h"
#include "fxbarcode/datamatrix/BC_ASCIIEncoder.h"
#include "fxbarcode/datamatrix/BC_Base256Encoder.h"
#include "fxbarcode/datamatrix/BC_C40Encoder.h"
#include "fxbarcode/datamatrix/BC_EdifactEncoder.h"
#include "fxbarcode/datamatrix/BC_Encoder.h"
#include "fxbarcode/datamatrix/BC_EncoderContext.h"
#include "fxbarcode/datamatrix/BC_SymbolInfo.h"
#include "fxbarcode/datamatrix/BC_TextEncoder.h"
#include "fxbarcode/datamatrix/BC_X12Encoder.h"
#include "third_party/base/ptr_util.h"

namespace {

const wchar_t kPad = 129;
const wchar_t kMacro05 = 236;
const wchar_t kMacro06 = 237;
const wchar_t kMacro05Header[] = L"[)>05";
const wchar_t kMacro06Header[] = L"[)>06";
const wchar_t kMacroTrailer = 0x0004;

wchar_t Randomize253State(wchar_t ch, int32_t codewordPosition) {
  int32_t pseudoRandom = ((149 * codewordPosition) % 253) + 1;
  int32_t tempVariable = ch + pseudoRandom;
  return tempVariable <= 254 ? static_cast<wchar_t>(tempVariable)
                             : static_cast<wchar_t>(tempVariable - 254);
}

int32_t FindMinimums(std::vector<float>& charCounts,
                     std::vector<int32_t>& intCharCounts,
                     int32_t min,
                     std::vector<uint8_t>& mins) {
  for (size_t l = 0; l < mins.size(); l++)
    mins[l] = 0;

  for (size_t i = 0; i < 6; i++) {
    intCharCounts[i] = static_cast<int32_t>(ceil(charCounts[i]));
    int32_t current = intCharCounts[i];
    if (min > current) {
      min = current;
      for (size_t j = 0; j < mins.size(); j++)
        mins[j] = 0;
    }
    if (min == current)
      mins[i]++;
  }
  return min;
}

int32_t GetMinimumCount(std::vector<uint8_t>& mins) {
  int32_t minCount = 0;
  for (int32_t i = 0; i < 6; i++) {
    minCount += mins[i];
  }
  return minCount;
}

bool IsNativeC40(wchar_t ch) {
  return (ch == ' ') || (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z');
}

bool IsNativeText(wchar_t ch) {
  return (ch == ' ') || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z');
}

bool IsX12TermSep(wchar_t ch) {
  return (ch == '\r') || (ch == '*') || (ch == '>');
}

bool IsNativeX12(wchar_t ch) {
  return IsX12TermSep(ch) || (ch == ' ') || (ch >= '0' && ch <= '9') ||
         (ch >= 'A' && ch <= 'Z');
}

bool IsNativeEDIFACT(wchar_t ch) {
  return ch >= ' ' && ch <= '^';
}

}  // namespace

// static
WideString CBC_HighLevelEncoder::EncodeHighLevel(const WideString& msg,
                                                 bool bAllowRectangular) {
  CBC_EncoderContext context(msg, bAllowRectangular);
  if (context.HasCharactersOutsideISO88591Encoding())
    return WideString();

  if (msg.Last() == kMacroTrailer) {
    WideString left = msg.Left(6);
    if (left == kMacro05Header) {
      context.writeCodeword(kMacro05);
      context.setSkipAtEnd(2);
      context.m_pos += 6;
    } else if (left == kMacro06Header) {
      context.writeCodeword(kMacro06);
      context.setSkipAtEnd(2);
      context.m_pos += 6;
    }
  }

  std::vector<std::unique_ptr<CBC_Encoder>> encoders;
  encoders.push_back(pdfium::MakeUnique<CBC_ASCIIEncoder>());
  encoders.push_back(pdfium::MakeUnique<CBC_C40Encoder>());
  encoders.push_back(pdfium::MakeUnique<CBC_TextEncoder>());
  encoders.push_back(pdfium::MakeUnique<CBC_X12Encoder>());
  encoders.push_back(pdfium::MakeUnique<CBC_EdifactEncoder>());
  encoders.push_back(pdfium::MakeUnique<CBC_Base256Encoder>());
  int32_t encodingMode = ASCII_ENCODATION;
  while (context.hasMoreCharacters()) {
    if (!encoders[encodingMode]->Encode(&context))
      return WideString();

    if (context.m_newEncoding >= 0) {
      encodingMode = context.m_newEncoding;
      context.resetEncoderSignal();
    }
  }
  size_t len = context.m_codewords.GetLength();
  if (!context.UpdateSymbolInfo())
    return WideString();

  size_t capacity = context.m_symbolInfo->dataCapacity();
  if (len < capacity) {
    if (encodingMode != ASCII_ENCODATION &&
        encodingMode != BASE256_ENCODATION) {
      context.writeCodeword(0x00fe);
    }
  }
  WideString codewords = context.m_codewords;
  if (codewords.GetLength() < capacity)
    codewords += kPad;

  while (codewords.GetLength() < capacity)
    codewords += Randomize253State(kPad, codewords.GetLength() + 1);

  ASSERT(!codewords.IsEmpty());
  return codewords;
}

// static
int32_t CBC_HighLevelEncoder::lookAheadTest(const WideString& msg,
                                            int32_t startpos,
                                            int32_t currentMode) {
  if (startpos >= pdfium::base::checked_cast<int32_t>(msg.GetLength())) {
    return currentMode;
  }
  std::vector<float> charCounts(6);
  if (currentMode == ASCII_ENCODATION) {
    charCounts = {0, 1, 1, 1, 1, 1.25f};
  } else {
    charCounts = {1, 2, 2, 2, 2, 2.25f};
    charCounts[currentMode] = 0;
  }
  int32_t charsProcessed = 0;
  while (true) {
    if ((startpos + charsProcessed) ==
        pdfium::base::checked_cast<int32_t>(msg.GetLength())) {
      int32_t min = std::numeric_limits<int32_t>::max();
      std::vector<uint8_t> mins(6);
      std::vector<int32_t> intCharCounts(6);
      min = FindMinimums(charCounts, intCharCounts, min, mins);
      int32_t minCount = GetMinimumCount(mins);
      if (intCharCounts[ASCII_ENCODATION] == min) {
        return ASCII_ENCODATION;
      }
      if (minCount == 1 && mins[BASE256_ENCODATION] > 0) {
        return BASE256_ENCODATION;
      }
      if (minCount == 1 && mins[EDIFACT_ENCODATION] > 0) {
        return EDIFACT_ENCODATION;
      }
      if (minCount == 1 && mins[TEXT_ENCODATION] > 0) {
        return TEXT_ENCODATION;
      }
      if (minCount == 1 && mins[X12_ENCODATION] > 0) {
        return X12_ENCODATION;
      }
      return C40_ENCODATION;
    }
    wchar_t c = msg[startpos + charsProcessed];
    charsProcessed++;
    if (FXSYS_IsDecimalDigit(c)) {
      charCounts[ASCII_ENCODATION] += 0.5;
    } else if (isExtendedASCII(c)) {
      charCounts[ASCII_ENCODATION] = (float)ceil(charCounts[ASCII_ENCODATION]);
      charCounts[ASCII_ENCODATION] += 2;
    } else {
      charCounts[ASCII_ENCODATION] = (float)ceil(charCounts[ASCII_ENCODATION]);
      charCounts[ASCII_ENCODATION]++;
    }
    if (IsNativeC40(c)) {
      charCounts[C40_ENCODATION] += 2.0f / 3.0f;
    } else if (isExtendedASCII(c)) {
      charCounts[C40_ENCODATION] += 8.0f / 3.0f;
    } else {
      charCounts[C40_ENCODATION] += 4.0f / 3.0f;
    }
    if (IsNativeText(c)) {
      charCounts[TEXT_ENCODATION] += 2.0f / 3.0f;
    } else if (isExtendedASCII(c)) {
      charCounts[TEXT_ENCODATION] += 8.0f / 3.0f;
    } else {
      charCounts[TEXT_ENCODATION] += 4.0f / 3.0f;
    }
    if (IsNativeX12(c)) {
      charCounts[X12_ENCODATION] += 2.0f / 3.0f;
    } else if (isExtendedASCII(c)) {
      charCounts[X12_ENCODATION] += 13.0f / 3.0f;
    } else {
      charCounts[X12_ENCODATION] += 10.0f / 3.0f;
    }
    if (IsNativeEDIFACT(c)) {
      charCounts[EDIFACT_ENCODATION] += 3.0f / 4.0f;
    } else if (isExtendedASCII(c)) {
      charCounts[EDIFACT_ENCODATION] += 17.0f / 4.0f;
    } else {
      charCounts[EDIFACT_ENCODATION] += 13.0f / 4.0f;
    }
    charCounts[BASE256_ENCODATION]++;
    if (charsProcessed >= 4) {
      std::vector<int32_t> intCharCounts(6);
      std::vector<uint8_t> mins(6);
      FindMinimums(charCounts, intCharCounts,
                   std::numeric_limits<int32_t>::max(), mins);
      int32_t minCount = GetMinimumCount(mins);
      if (intCharCounts[ASCII_ENCODATION] < intCharCounts[BASE256_ENCODATION] &&
          intCharCounts[ASCII_ENCODATION] < intCharCounts[C40_ENCODATION] &&
          intCharCounts[ASCII_ENCODATION] < intCharCounts[TEXT_ENCODATION] &&
          intCharCounts[ASCII_ENCODATION] < intCharCounts[X12_ENCODATION] &&
          intCharCounts[ASCII_ENCODATION] < intCharCounts[EDIFACT_ENCODATION]) {
        return ASCII_ENCODATION;
      }
      if (intCharCounts[BASE256_ENCODATION] < intCharCounts[ASCII_ENCODATION] ||
          (mins[C40_ENCODATION] + mins[TEXT_ENCODATION] + mins[X12_ENCODATION] +
           mins[EDIFACT_ENCODATION]) == 0) {
        return BASE256_ENCODATION;
      }
      if (minCount == 1 && mins[EDIFACT_ENCODATION] > 0) {
        return EDIFACT_ENCODATION;
      }
      if (minCount == 1 && mins[TEXT_ENCODATION] > 0) {
        return TEXT_ENCODATION;
      }
      if (minCount == 1 && mins[X12_ENCODATION] > 0) {
        return X12_ENCODATION;
      }
      if (intCharCounts[C40_ENCODATION] + 1 < intCharCounts[ASCII_ENCODATION] &&
          intCharCounts[C40_ENCODATION] + 1 <
              intCharCounts[BASE256_ENCODATION] &&
          intCharCounts[C40_ENCODATION] + 1 <
              intCharCounts[EDIFACT_ENCODATION] &&
          intCharCounts[C40_ENCODATION] + 1 < intCharCounts[TEXT_ENCODATION]) {
        if (intCharCounts[C40_ENCODATION] < intCharCounts[X12_ENCODATION]) {
          return C40_ENCODATION;
        }
        if (intCharCounts[C40_ENCODATION] == intCharCounts[X12_ENCODATION]) {
          int32_t p = startpos + charsProcessed + 1;
          int32_t checked_length =
              pdfium::base::checked_cast<int32_t>(msg.GetLength());
          while (p < checked_length) {
            wchar_t tc = msg[p];
            if (IsX12TermSep(tc)) {
              return X12_ENCODATION;
            }
            if (!IsNativeX12(tc)) {
              break;
            }
            p++;
          }
          return C40_ENCODATION;
        }
      }
    }
  }
}

// static
bool CBC_HighLevelEncoder::isExtendedASCII(wchar_t ch) {
  return ch >= 128 && ch <= 255;
}

// static
int32_t CBC_HighLevelEncoder::determineConsecutiveDigitCount(WideString msg,
                                                             int32_t startpos) {
  int32_t count = 0;
  int32_t len = pdfium::base::checked_cast<int32_t>(msg.GetLength());
  int32_t idx = startpos;
  if (idx < len) {
    wchar_t ch = msg[idx];
    while (FXSYS_IsDecimalDigit(ch) && idx < len) {
      count++;
      idx++;
      if (idx < len) {
        ch = msg[idx];
      }
    }
  }
  return count;
}
