// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_cmapparser.h"

#include <vector>

#include "core/fpdfapi/cmaps/cmap_int.h"
#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxge/fx_freetype.h"
#include "third_party/base/logging.h"
#include "third_party/base/stl_util.h"

namespace {

const char* const g_CharsetNames[CIDSET_NUM_SETS] = {nullptr,  "GB1",    "CNS1",
                                                     "Japan1", "Korea1", "UCS"};

CIDSet CIDSetFromSizeT(size_t index) {
  if (index >= CIDSET_NUM_SETS) {
    NOTREACHED();
    return CIDSET_UNKNOWN;
  }
  return static_cast<CIDSet>(index);
}

CFX_ByteStringC CMap_GetString(const CFX_ByteStringC& word) {
  if (word.GetLength() <= 2)
    return CFX_ByteStringC();
  return CFX_ByteStringC(&word[1], word.GetLength() - 2);
}

}  // namespace

CPDF_CMapParser::CPDF_CMapParser(CPDF_CMap* pCMap)
    : m_pCMap(pCMap), m_Status(0), m_CodeSeq(0) {}

CPDF_CMapParser::~CPDF_CMapParser() {}

void CPDF_CMapParser::ParseWord(const CFX_ByteStringC& word) {
  if (word.IsEmpty()) {
    return;
  }
  if (word == "begincidchar") {
    m_Status = 1;
    m_CodeSeq = 0;
  } else if (word == "begincidrange") {
    m_Status = 2;
    m_CodeSeq = 0;
  } else if (word == "endcidrange" || word == "endcidchar") {
    m_Status = 0;
  } else if (word == "/WMode") {
    m_Status = 6;
  } else if (word == "/Registry") {
    m_Status = 3;
  } else if (word == "/Ordering") {
    m_Status = 4;
  } else if (word == "/Supplement") {
    m_Status = 5;
  } else if (word == "begincodespacerange") {
    m_Status = 7;
    m_CodeSeq = 0;
  } else if (word == "usecmap") {
  } else if (m_Status == 1 || m_Status == 2) {
    m_CodePoints[m_CodeSeq] = CMap_GetCode(word);
    m_CodeSeq++;
    uint32_t StartCode, EndCode;
    uint16_t StartCID;
    if (m_Status == 1) {
      if (m_CodeSeq < 2) {
        return;
      }
      EndCode = StartCode = m_CodePoints[0];
      StartCID = (uint16_t)m_CodePoints[1];
    } else {
      if (m_CodeSeq < 3) {
        return;
      }
      StartCode = m_CodePoints[0];
      EndCode = m_CodePoints[1];
      StartCID = (uint16_t)m_CodePoints[2];
    }
    if (EndCode < 0x10000) {
      for (uint32_t code = StartCode; code <= EndCode; code++) {
        m_pCMap->m_DirectCharcodeToCIDTable[code] =
            static_cast<uint16_t>(StartCID + code - StartCode);
      }
    } else {
      m_AdditionalCharcodeToCIDMappings.push_back(
          {StartCode, EndCode, StartCID});
    }
    m_CodeSeq = 0;
  } else if (m_Status == 3) {
    m_Status = 0;
  } else if (m_Status == 4) {
    m_pCMap->m_Charset = CharsetFromOrdering(CMap_GetString(word));
    m_Status = 0;
  } else if (m_Status == 5) {
    m_Status = 0;
  } else if (m_Status == 6) {
    m_pCMap->m_bVertical = CMap_GetCode(word) != 0;
    m_Status = 0;
  } else if (m_Status == 7) {
    if (word == "endcodespacerange") {
      uint32_t nSegs = pdfium::CollectionSize<uint32_t>(m_CodeRanges);
      if (nSegs > 1) {
        m_pCMap->m_CodingScheme = CPDF_CMap::MixedFourBytes;
        m_pCMap->m_MixedFourByteLeadingRanges = m_CodeRanges;
      } else if (nSegs == 1) {
        m_pCMap->m_CodingScheme = (m_CodeRanges[0].m_CharSize == 2)
                                      ? CPDF_CMap::TwoBytes
                                      : CPDF_CMap::OneByte;
      }
      m_Status = 0;
    } else {
      if (word.GetLength() == 0 || word.GetAt(0) != '<') {
        return;
      }
      if (m_CodeSeq % 2) {
        CPDF_CMap::CodeRange range;
        if (CMap_GetCodeRange(range, m_LastWord.AsStringC(), word))
          m_CodeRanges.push_back(range);
      }
      m_CodeSeq++;
    }
  }
  m_LastWord = word;
}

// Static.
uint32_t CPDF_CMapParser::CMap_GetCode(const CFX_ByteStringC& word) {
  pdfium::base::CheckedNumeric<uint32_t> num = 0;
  if (word.GetAt(0) == '<') {
    for (int i = 1; i < word.GetLength() && std::isxdigit(word.GetAt(i)); ++i) {
      num = num * 16 + FXSYS_HexCharToInt(word.GetAt(i));
      if (!num.IsValid())
        return 0;
    }
    return num.ValueOrDie();
  }

  for (int i = 0; i < word.GetLength() && std::isdigit(word.GetAt(i)); ++i) {
    num =
        num * 10 + FXSYS_DecimalCharToInt(static_cast<wchar_t>(word.GetAt(i)));
    if (!num.IsValid())
      return 0;
  }
  return num.ValueOrDie();
}

// Static.
bool CPDF_CMapParser::CMap_GetCodeRange(CPDF_CMap::CodeRange& range,
                                        const CFX_ByteStringC& first,
                                        const CFX_ByteStringC& second) {
  if (first.GetLength() == 0 || first.GetAt(0) != '<')
    return false;

  int i;
  for (i = 1; i < first.GetLength(); ++i) {
    if (first.GetAt(i) == '>') {
      break;
    }
  }
  range.m_CharSize = (i - 1) / 2;
  if (range.m_CharSize > 4)
    return false;

  for (i = 0; i < range.m_CharSize; ++i) {
    uint8_t digit1 = first.GetAt(i * 2 + 1);
    uint8_t digit2 = first.GetAt(i * 2 + 2);
    range.m_Lower[i] =
        FXSYS_HexCharToInt(digit1) * 16 + FXSYS_HexCharToInt(digit2);
  }

  uint32_t size = second.GetLength();
  for (i = 0; i < range.m_CharSize; ++i) {
    uint8_t digit1 = ((uint32_t)i * 2 + 1 < size)
                         ? second.GetAt((FX_STRSIZE)i * 2 + 1)
                         : '0';
    uint8_t digit2 = ((uint32_t)i * 2 + 2 < size)
                         ? second.GetAt((FX_STRSIZE)i * 2 + 2)
                         : '0';
    range.m_Upper[i] =
        FXSYS_HexCharToInt(digit1) * 16 + FXSYS_HexCharToInt(digit2);
  }
  return true;
}

// static
CIDSet CPDF_CMapParser::CharsetFromOrdering(const CFX_ByteStringC& ordering) {
  for (size_t charset = 1; charset < FX_ArraySize(g_CharsetNames); ++charset) {
    if (ordering == g_CharsetNames[charset])
      return CIDSetFromSizeT(charset);
  }
  return CIDSET_UNKNOWN;
}
