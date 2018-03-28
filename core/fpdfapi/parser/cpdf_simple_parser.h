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
  explicit CPDF_SimpleParser(const ByteStringView& str);
  ~CPDF_SimpleParser();

  ByteStringView GetWord();

  // Find the token and its |nParams| parameters from the start of data,
  // and move the current position to the start of those parameters.
  bool FindTagParamFromStart(const ByteStringView& token, int nParams);

  uint32_t GetCurPosForTest() const { return cur_pos_; }

 private:
  const ByteStringView data_;
  uint32_t cur_pos_ = 0;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_
