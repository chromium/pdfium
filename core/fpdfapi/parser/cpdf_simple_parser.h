// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_
#define CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_

#include <stdint.h>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/span.h"

class CPDF_SimpleParser {
 public:
  explicit CPDF_SimpleParser(pdfium::span<const uint8_t> input);
  ~CPDF_SimpleParser();

  ByteStringView GetWord();

  void SetCurrentPosition(uint32_t position) { cur_position_ = position; }
  uint32_t GetCurrentPosition() const { return cur_position_; }

 private:
  const pdfium::span<const uint8_t> data_;

  // The current unread position.
  uint32_t cur_position_ = 0;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_
