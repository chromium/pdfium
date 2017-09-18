// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_
#define CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_

#include <map>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_cmap.h"
#include "core/fxcrt/cfx_unowned_ptr.h"

class CPDF_CMapParser {
 public:
  explicit CPDF_CMapParser(CPDF_CMap* pMap);
  ~CPDF_CMapParser();

  void ParseWord(const ByteStringView& str);
  bool HasAdditionalMappings() const {
    return !m_AdditionalCharcodeToCIDMappings.empty();
  }
  std::vector<CPDF_CMap::CIDRange> TakeAdditionalMappings() {
    return std::move(m_AdditionalCharcodeToCIDMappings);
  }

  static CIDSet CharsetFromOrdering(const ByteStringView& ordering);

 private:
  friend class cpdf_cmapparser_CMap_GetCode_Test;
  friend class cpdf_cmapparser_CMap_GetCodeRange_Test;

  static uint32_t CMap_GetCode(const ByteStringView& word);
  static bool CMap_GetCodeRange(CPDF_CMap::CodeRange& range,
                                const ByteStringView& first,
                                const ByteStringView& second);

  CFX_UnownedPtr<CPDF_CMap> const m_pCMap;
  int m_Status;
  int m_CodeSeq;
  uint32_t m_CodePoints[4];
  std::vector<CPDF_CMap::CodeRange> m_CodeRanges;
  std::vector<CPDF_CMap::CIDRange> m_AdditionalCharcodeToCIDMappings;
  ByteString m_LastWord;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_
