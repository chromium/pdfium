// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_
#define CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_

#include <array>
#include <optional>
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

  static CIDSet CharsetFromOrdering(ByteStringView ordering);

 private:
  friend class CPDFCMapParserTest_GetCode_Test;
  friend class CPDFCMapParserTest_GetCodeRange_Test;

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

  void HandleCid(ByteStringView word);
  void HandleCodeSpaceRange(ByteStringView word);

  static uint32_t GetCode(ByteStringView word);
  static std::optional<CPDF_CMap::CodeRange> GetCodeRange(
      ByteStringView first,
      ByteStringView second);

  Status status_ = kStart;
  int code_seq_ = 0;
  UnownedPtr<CPDF_CMap> const cmap_;
  std::vector<CPDF_CMap::CodeRange> ranges_;
  std::vector<CPDF_CMap::CodeRange> pending_ranges_;
  std::vector<CPDF_CMap::CIDRange> additional_charcode_to_cidmappings_;
  ByteString last_word_;
  std::array<uint32_t, 4> code_points_ = {};
};

#endif  // CORE_FPDFAPI_FONT_CPDF_CMAPPARSER_H_
