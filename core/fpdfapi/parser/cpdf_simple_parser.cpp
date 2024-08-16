// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_simple_parser.h"

#include <stdint.h>

#include "core/fpdfapi/parser/fpdf_parser_utility.h"

CPDF_SimpleParser::CPDF_SimpleParser(pdfium::span<const uint8_t> input)
    : data_(input) {}

CPDF_SimpleParser::~CPDF_SimpleParser() = default;

ByteStringView CPDF_SimpleParser::GetWord() {
  uint8_t cur_char;

  // Skip whitespace and comment lines.
  while (true) {
    if (data_.size() <= cur_position_) {
      return ByteStringView();
    }

    cur_char = data_[cur_position_++];
    while (PDFCharIsWhitespace(cur_char)) {
      if (data_.size() <= cur_position_) {
        return ByteStringView();
      }
      cur_char = data_[cur_position_++];
    }

    if (cur_char != '%') {
      break;
    }

    while (true) {
      if (data_.size() <= cur_position_) {
        return ByteStringView();
      }

      cur_char = data_[cur_position_++];
      if (PDFCharIsLineEnding(cur_char)) {
        break;
      }
    }
  }

  uint8_t size = 0;
  uint32_t start_position = cur_position_ - 1;
  if (PDFCharIsDelimiter(cur_char)) {
    // Find names
    if (cur_char == '/') {
      while (true) {
        if (data_.size() <= cur_position_) {
          break;
        }

        cur_char = data_[cur_position_++];
        if (!PDFCharIsOther(cur_char) && !PDFCharIsNumeric(cur_char)) {
          cur_position_--;
          size = cur_position_ - start_position;
          break;
        }
      }
      return ByteStringView(data_.subspan(start_position, size));
    }

    size = 1;
    if (cur_char == '<') {
      if (data_.size() <= cur_position_) {
        return ByteStringView(data_.subspan(start_position, size));
      }
      cur_char = data_[cur_position_++];
      if (cur_char == '<') {
        size = 2;
      } else {
        while (cur_position_ < data_.size() && data_[cur_position_] != '>') {
          cur_position_++;
        }

        if (cur_position_ < data_.size()) {
          cur_position_++;
        }

        size = cur_position_ - start_position;
      }
    } else if (cur_char == '>') {
      if (data_.size() <= cur_position_) {
        return ByteStringView(data_.subspan(start_position, size));
      }
      cur_char = data_[cur_position_++];
      if (cur_char == '>') {
        size = 2;
      } else {
        cur_position_--;
      }
    } else if (cur_char == '(') {
      int level = 1;
      while (cur_position_ < data_.size()) {
        if (data_[cur_position_] == ')') {
          level--;
          if (level == 0)
            break;
        }

        if (data_[cur_position_] == '\\') {
          if (data_.size() <= cur_position_) {
            break;
          }

          cur_position_++;
        } else if (data_[cur_position_] == '(') {
          level++;
        }
        if (data_.size() <= cur_position_) {
          break;
        }

        cur_position_++;
      }
      if (cur_position_ < data_.size()) {
        cur_position_++;
      }

      size = cur_position_ - start_position;
    }
    return ByteStringView(data_.subspan(start_position, size));
  }

  size = 1;
  while (cur_position_ < data_.size()) {
    cur_char = data_[cur_position_++];

    if (PDFCharIsDelimiter(cur_char) || PDFCharIsWhitespace(cur_char)) {
      cur_position_--;
      break;
    }
    size++;
  }
  return ByteStringView(data_.subspan(start_position, size));
}
