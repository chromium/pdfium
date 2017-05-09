// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_LGIF_CGIFDECOMPRESSOR_H_
#define CORE_FXCODEC_LGIF_CGIFDECOMPRESSOR_H_

#include <setjmp.h>
#include <memory>
#include <vector>

#include "core/fxcodec/lgif/fx_gif.h"
#include "core/fxcrt/fx_basic.h"

class CCodec_GifModule;

// TODO(npm): Get rid of this, maybe rename CGifDecompressor->GifContext
class FXGIF_Context {
 public:
  FXGIF_Context();
  ~FXGIF_Context();

  std::unique_ptr<CGifDecompressor> m_Gif;
  CCodec_GifModule* parent_ptr;
};

class CGifDecompressor {
 public:
  CGifDecompressor(FXGIF_Context* p, char* error_string);
  ~CGifDecompressor();

  void ErrorData(const char* err_msg);
  uint8_t* AskBufForPal(int32_t pal_size);
  void RecordCurrentPosition(uint32_t* cur_pos_ptr);
  void ReadScanline(int32_t row_num, uint8_t* row_buf);
  bool GetRecordPosition(uint32_t cur_pos,
                         int32_t left,
                         int32_t top,
                         int32_t width,
                         int32_t height,
                         int32_t pal_num,
                         void* pal_ptr,
                         int32_t delay_time,
                         bool user_input,
                         int32_t trans_index,
                         int32_t disposal_method,
                         bool interlace);

  jmp_buf jmpbuf;
  std::vector<GifPalette> m_GlobalPalette;
  int32_t global_pal_num;
  uint32_t img_row_offset;
  uint32_t img_row_avail_size;
  uint32_t avail_in;
  int32_t decode_status;
  uint32_t skip_size;

  char* err_ptr;
  FXGIF_Context* context_ptr;
  CFX_ByteString* cmt_data_ptr;
  GifGCE* gce_ptr;
  std::vector<GifPlainText*>* pt_ptr_arr_ptr;
  uint8_t* next_in;
  std::vector<GifImage*>* img_ptr_arr_ptr;
  std::unique_ptr<CGifLZWDecoder> m_ImgDecoder;

  int width;
  int height;

  uint8_t bc_index;
  uint8_t pixel_aspect;
  uint8_t global_sort_flag;
  uint8_t global_color_resolution;
  uint8_t img_pass_num;
};

#endif  // CORE_FXCODEC_LGIF_CGIFDECOMPRESSOR_H_
