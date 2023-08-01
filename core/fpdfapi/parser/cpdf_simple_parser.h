// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_
#define CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_

#include <stdint.h>

#include "core/fxcrt/bytestring.h"
#include "third_party/base/containers/span.h"

class CPDF_SimpleParser {
 public:
  explicit CPDF_SimpleParser(pdfium::span<const uint8_t> input);
  ~CPDF_SimpleParser();

  ByteStringView GetWord();

  void SetCurPos(uint32_t pos) { cur_pos_ = pos; }
  uint32_t GetCurPos() const { return cur_pos_; }

 private:
  const pdfium::span<const uint8_t> data_;
  uint32_t cur_pos_ = 0;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_SIMPLE_PARSER_H_
