// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_
#define CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_

#include <utility>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

class CPDF_SimpleParser {
 public:
  CPDF_SimpleParser(const uint8_t* pData, uint32_t dwSize);
  explicit CPDF_SimpleParser(const ByteStringView& str);

  ByteStringView GetWord();

  // Find the token and its |nParams| parameters from the start of data,
  // and move the current position to the start of those parameters.
  bool FindTagParamFromStart(const ByteStringView& token, int nParams);

  // For testing only.
  uint32_t GetCurPos() const { return m_dwCurPos; }

 private:
  std::pair<const uint8_t*, uint32_t> ParseWord();

  const uint8_t* m_pData;
  uint32_t m_dwSize;
  uint32_t m_dwCurPos;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_
