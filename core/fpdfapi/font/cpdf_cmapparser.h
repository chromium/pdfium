// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_
#define CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_

#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fpdfapi/font/cpdf_cmap.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_CMapParser {
 public:
  explicit CPDF_CMapParser(CPDF_CMap* pCMap);
  ~CPDF_CMapParser();

  void ParseWord(ByteStringView word);
  bool HasAdditionalMappings() const {
    return !m_AdditionalCharcodeToCIDMappings.empty();
  }
  std::vector<CPDF_CMap::CIDRange> TakeAdditionalMappings() {
    return std::move(m_AdditionalCharcodeToCIDMappings);
  }

  uint32_t GetCode(ByteStringView word) const;
  bool GetCodeRange(CPDF_CMap::CodeRange& range,
                    ByteStringView first,
                    ByteStringView second) const;

  static CIDSet CharsetFromOrdering(ByteStringView ordering);

 private:
  enum Status {
    kStart,
    kProcessingCidChar,
    kProcessingCidRange,
    kProcessingRegistry,
    kProcessingOrdering,
    kProcessingSupplement,
    kProcessingWMode,
    kProcessingCodeSpaceRange,
  };

  Status m_Status = kStart;
  int m_CodeSeq = 0;
  UnownedPtr<CPDF_CMap> const m_pCMap;
  std::vector<CPDF_CMap::CodeRange> m_PendingRanges;
  std::vector<CPDF_CMap::CIDRange> m_AdditionalCharcodeToCIDMappings;
  ByteString m_LastWord;
  uint32_t m_CodePoints[4];
};

#endif  // CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_
