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
    if (cur_position_ >= data_.size()) {
      return ByteStringView();
    }

    cur_char = data_[cur_position_++];
    while (PDFCharIsWhitespace(cur_char)) {
      if (cur_position_ >= data_.size()) {
        return ByteStringView();
      }
      cur_char = data_[cur_position_++];
    }

    if (cur_char != '%') {
      break;
    }

    while (true) {
      if (cur_position_ >= data_.size()) {
        return ByteStringView();
      }

      cur_char = data_[cur_position_++];
      if (PDFCharIsLineEnding(cur_char)) {
        break;
      }
    }
  }

  uint32_t start_position = cur_position_ - 1;
  if (PDFCharIsDelimiter(cur_char)) {
    // Find names
    if (cur_char == '/') {
      while (cur_position_ < data_.size()) {
        cur_char = data_[cur_position_];
        // Stop parsing after encountering a whitespace or delimiter.
        if (PDFCharIsWhitespace(cur_char) || PDFCharIsDelimiter(cur_char)) {
          return ByteStringView(
              data_.subspan(start_position, cur_position_ - start_position));
        }
        ++cur_position_;
      }
      return ByteStringView();
    }

    if (cur_char == '<') {
      if (cur_position_ >= data_.size()) {
        return ByteStringView(
            data_.subspan(start_position, cur_position_ - start_position));
      }
      cur_char = data_[cur_position_++];
      if (cur_char != '<') {
        while (cur_position_ < data_.size() && data_[cur_position_] != '>') {
          cur_position_++;
        }

        if (cur_position_ < data_.size()) {
          cur_position_++;
        }
      }
    } else if (cur_char == '>') {
      if (cur_position_ >= data_.size()) {
        return ByteStringView(
            data_.subspan(start_position, cur_position_ - start_position));
      }
      if (data_[cur_position_] == '>') {
        ++cur_position_;
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
          if (cur_position_ >= data_.size()) {
            break;
          }

          cur_position_++;
        } else if (data_[cur_position_] == '(') {
          level++;
        }
        if (cur_position_ >= data_.size()) {
          break;
        }

        cur_position_++;
      }
      if (cur_position_ < data_.size()) {
        cur_position_++;
      }
    }
    return ByteStringView(
        data_.subspan(start_position, cur_position_ - start_position));
  }

  while (cur_position_ < data_.size()) {
    cur_char = data_[cur_position_];
    if (PDFCharIsDelimiter(cur_char) || PDFCharIsWhitespace(cur_char)) {
      break;
    }
    ++cur_position_;
  }
  return ByteStringView(
      data_.subspan(start_position, cur_position_ - start_position));
}
