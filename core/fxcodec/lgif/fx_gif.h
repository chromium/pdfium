// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_LGIF_FX_GIF_H_
#define CORE_FXCODEC_LGIF_FX_GIF_H_

#include <memory>
#include <vector>

class CGifContext;

#define GIF_SIGNATURE "GIF"
#define GIF_SIG_EXTENSION 0x21
#define GIF_SIG_IMAGE 0x2C
#define GIF_SIG_TRAILER 0x3B
#define GIF_BLOCK_GCE 0xF9
#define GIF_BLOCK_PTE 0x01
#define GIF_BLOCK_CE 0xFE
#define GIF_BLOCK_AE 0xFF
#define GIF_BLOCK_TERMINAL 0x00
#define GIF_MAX_LZW_EXP 12
#define GIF_MAX_LZW_CODE 4096
#define GIF_DATA_BLOCK 255
#define GIF_MAX_ERROR_SIZE 256
#define GIF_D_STATUS_SIG 0x01
#define GIF_D_STATUS_TAIL 0x02
#define GIF_D_STATUS_EXT 0x03
#define GIF_D_STATUS_EXT_AE 0x04
#define GIF_D_STATUS_EXT_CE 0x05
#define GIF_D_STATUS_EXT_GCE 0x06
#define GIF_D_STATUS_EXT_PTE 0x07
#define GIF_D_STATUS_EXT_UNE 0x08
#define GIF_D_STATUS_IMG_INFO 0x09
#define GIF_D_STATUS_IMG_DATA 0x0A

#pragma pack(1)
typedef struct {
  uint8_t pal_bits : 3;
  uint8_t sort_flag : 1;
  uint8_t color_resolution : 3;
  uint8_t global_pal : 1;
} GifGlobalFlags;

typedef struct {
  uint8_t pal_bits : 3;
  uint8_t reserved : 2;
  uint8_t sort_flag : 1;
  uint8_t interlace : 1;
  uint8_t local_pal : 1;
} GifLocalFlags;

typedef struct {
  char signature[3];
  char version[3];
} GifHeader;

typedef struct {
  uint16_t width;
  uint16_t height;
  GifGlobalFlags global_flags;
  uint8_t bc_index;
  uint8_t pixel_aspect;
} GifLocalScreenDescriptor;

typedef struct {
  uint16_t left;
  uint16_t top;
  uint16_t width;
  uint16_t height;
  GifLocalFlags local_flags;
} GifImageInfo;

typedef struct {
  uint8_t transparency : 1;
  uint8_t user_input : 1;
  uint8_t disposal_method : 3;
  uint8_t reserved : 3;
} GifControlExtensionFlags;

typedef struct {
  uint8_t block_size;
  GifControlExtensionFlags gce_flags;
  uint16_t delay_time;
  uint8_t trans_index;
} GifGraphicControlExtension;

typedef struct {
  uint8_t block_size;
  uint16_t grid_left;
  uint16_t grid_top;
  uint16_t grid_width;
  uint16_t grid_height;

  uint8_t char_width;
  uint8_t char_height;

  uint8_t fc_index;
  uint8_t bc_index;
} GifPlainTextExtension;

typedef struct {
  uint8_t block_size;
  uint8_t app_identify[8];
  uint8_t app_authentication[3];
} GifApplicationExtension;

typedef struct { uint8_t r, g, b; } GifPalette;
#pragma pack()

enum class GifDecodeStatus {
  Error,
  Success,
  Unfinished,
  InsufficientDestSize,  // Only used internally by CGifLZWDecoder::Decode()
};

typedef struct {
  std::unique_ptr<GifGraphicControlExtension> m_ImageGCE;
  std::vector<GifPalette> m_LocalPalettes;
  std::vector<uint8_t> m_ImageRowBuf;
  GifImageInfo m_ImageInfo;
  uint8_t local_pallette_exp;
  uint8_t image_code_exp;
  uint32_t image_data_pos;
  int32_t image_row_num;
} GifImage;

#endif  // CORE_FXCODEC_LGIF_FX_GIF_H_
