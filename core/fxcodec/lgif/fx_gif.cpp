// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/fx_gif.h"

#include <algorithm>
#include <utility>

#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcodec/lgif/cgifcontext.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

static_assert(sizeof(GifImageInfo) == 9,
              "GifImageInfo should have a size of 9");
static_assert(sizeof(GifPalette) == 3, "GifPalette should have a size of 3");
static_assert(sizeof(GifPTE) == 13, "GifPTE should have a size of 13");
static_assert(sizeof(GifGCE) == 5, "GifGCE should have a size of 5");
static_assert(sizeof(GifHeader) == 6, "GifHeader should have a size of 6");
static_assert(sizeof(GifLSD) == 7, "GifLSD should have a size of 7");

GifImage::GifImage() {}

GifImage::~GifImage() {}

void CGifLZWDecoder::Input(uint8_t* src_buf, uint32_t src_size) {
  next_in = src_buf;
  avail_in = src_size;
}

uint32_t CGifLZWDecoder::GetAvailInput() {
  return avail_in;
}

CGifLZWDecoder::CGifLZWDecoder(char* error_ptr)
    : code_size(0),
      code_size_cur(0),
      code_clear(0),
      code_end(0),
      code_next(0),
      code_first(0),
      stack_size(0),
      code_old(0),
      next_in(nullptr),
      avail_in(0),
      bits_left(0),
      code_store(0),
      err_msg_ptr(error_ptr) {}

CGifLZWDecoder::~CGifLZWDecoder() {}

void CGifLZWDecoder::InitTable(uint8_t color_exp, uint8_t code_exp) {
  // TODO(rharrison): Refactor all of this, so that initializing the table with
  // bad values will kill the decompress.
  ASSERT(code_exp <= GIF_MAX_LZW_EXP);
  code_color_end = std::min(2 << color_exp, 1 << code_exp);
  code_size = code_exp;
  code_clear = 1 << code_exp;
  code_end = code_clear + 1;
  bits_left = 0;
  code_store = 0;
  next_in = nullptr;
  avail_in = 0;
  stack_size = 0;
  code_first = 0;
  ClearTable();
}

void CGifLZWDecoder::ClearTable() {
  code_size_cur = code_size + 1;
  code_next = code_end + 1;
  code_old = static_cast<uint16_t>(-1);
  memset(code_table, 0, sizeof(tag_Table) * GIF_MAX_LZW_CODE);
  memset(stack, 0, GIF_MAX_LZW_CODE);
  for (uint16_t i = 0; i < code_clear; i++)
    code_table[i].suffix = static_cast<uint8_t>(i);
}

bool CGifLZWDecoder::DecodeString(uint16_t code) {
  stack_size = 0;
  while (code >= code_clear && code <= code_next) {
    if (code == code_table[code].prefix || stack_size == GIF_MAX_LZW_CODE - 1)
      return false;

    stack[GIF_MAX_LZW_CODE - 1 - stack_size++] = code_table[code].suffix;
    code = code_table[code].prefix;
  }
  if (code >= code_color_end)
    return false;

  stack[GIF_MAX_LZW_CODE - 1 - stack_size++] = static_cast<uint8_t>(code);
  code_first = static_cast<uint8_t>(code);
  return true;
}

void CGifLZWDecoder::AddCode(uint16_t prefix_code, uint8_t append_char) {
  if (code_next == GIF_MAX_LZW_CODE)
    return;

  code_table[code_next].prefix = prefix_code;
  code_table[code_next].suffix = append_char;
  if (++code_next < GIF_MAX_LZW_CODE) {
    if (code_next >> code_size_cur)
      code_size_cur++;
  }
}

GifDecodeStatus CGifLZWDecoder::Decode(uint8_t* des_buf, uint32_t* des_size) {
  if (*des_size == 0)
    return GifDecodeStatus::InsufficientDestSize;

  uint32_t i = 0;
  if (stack_size != 0) {
    if (*des_size < stack_size) {
      memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], *des_size);
      stack_size -= static_cast<uint16_t>(*des_size);
      return GifDecodeStatus::InsufficientDestSize;
    }
    memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], stack_size);
    des_buf += stack_size;
    i += stack_size;
    stack_size = 0;
  }
  ASSERT(err_msg_ptr);
  while (i <= *des_size && (avail_in > 0 || bits_left >= code_size_cur)) {
    if (code_size_cur > GIF_MAX_LZW_EXP) {
      strncpy(err_msg_ptr, "Code Length Out Of Range", GIF_MAX_ERROR_SIZE - 1);
      return GifDecodeStatus::Error;
    }
    if (avail_in > 0) {
      if (bits_left > 31) {
        strncpy(err_msg_ptr, "Decode Error", GIF_MAX_ERROR_SIZE - 1);
        return GifDecodeStatus::Error;
      }
      pdfium::base::CheckedNumeric<uint32_t> safe_code = *next_in++;
      safe_code <<= bits_left;
      safe_code |= code_store;
      if (!safe_code.IsValid()) {
        strncpy(err_msg_ptr, "Code Store Out Of Range", GIF_MAX_ERROR_SIZE - 1);
        return GifDecodeStatus::Error;
      }
      code_store = safe_code.ValueOrDie();
      --avail_in;
      bits_left += 8;
    }
    while (bits_left >= code_size_cur) {
      uint16_t code =
          static_cast<uint16_t>(code_store) & ((1 << code_size_cur) - 1);
      code_store >>= code_size_cur;
      bits_left -= code_size_cur;
      if (code == code_clear) {
        ClearTable();
        continue;
      }
      if (code == code_end) {
        *des_size = i;
        return GifDecodeStatus::Success;
      }
      if (code_old != static_cast<uint16_t>(-1)) {
        if (code_next < GIF_MAX_LZW_CODE) {
          if (code == code_next) {
            AddCode(code_old, code_first);
            if (!DecodeString(code)) {
              strncpy(err_msg_ptr, "String Decoding Error",
                      GIF_MAX_ERROR_SIZE - 1);
              return GifDecodeStatus::Error;
            }
          } else if (code > code_next) {
            strncpy(err_msg_ptr, "Decode Error, Out Of Range",
                    GIF_MAX_ERROR_SIZE - 1);
            return GifDecodeStatus::Error;
          } else {
            if (!DecodeString(code)) {
              strncpy(err_msg_ptr, "String Decoding Error",
                      GIF_MAX_ERROR_SIZE - 1);
              return GifDecodeStatus::Error;
            }
            uint8_t append_char = stack[GIF_MAX_LZW_CODE - stack_size];
            AddCode(code_old, append_char);
          }
        }
      } else {
        if (!DecodeString(code)) {
          strncpy(err_msg_ptr, "String Decoding Error", GIF_MAX_ERROR_SIZE - 1);
          return GifDecodeStatus::Error;
        }
      }
      code_old = code;
      if (i + stack_size > *des_size) {
        memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], *des_size - i);
        stack_size -= static_cast<uint16_t>(*des_size - i);
        return GifDecodeStatus::InsufficientDestSize;
      }
      memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], stack_size);
      des_buf += stack_size;
      i += stack_size;
      stack_size = 0;
    }
  }
  if (avail_in == 0) {
    *des_size = i;
    return GifDecodeStatus::Unfinished;
  }
  return GifDecodeStatus::Error;
}
