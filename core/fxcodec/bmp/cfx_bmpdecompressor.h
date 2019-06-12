// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_
#define CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_

#include <setjmp.h>

#include <vector>

#include "core/fxcodec/bmp/fx_bmp.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_BmpContext;
class CFX_CodecMemory;

class CFX_BmpDecompressor {
 public:
  CFX_BmpDecompressor();
  ~CFX_BmpDecompressor();

  void Error();
  int32_t DecodeImage();
  int32_t ReadHeader();
  void SetInputBuffer(RetainPtr<CFX_CodecMemory> codec_memory);
  FX_FILESIZE GetAvailInput() const;

  jmp_buf jmpbuf_;
  UnownedPtr<CFX_BmpContext> context_;
  std::vector<uint8_t> out_row_buffer_;
  std::vector<uint32_t> palette_;
  uint32_t header_offset_ = 0;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t compress_flag_ = 0;
  int32_t components_ = 0;
  size_t src_row_bytes_ = 0;
  size_t out_row_bytes_ = 0;
  bool img_tb_flag_ = false;
  uint16_t bit_counts_ = 0;
  uint32_t color_used_ = 0;
  int32_t pal_num_ = 0;
  int32_t pal_type_ = 0;
  uint32_t data_size_ = 0;
  uint32_t img_ifh_size_ = 0;
  size_t row_num_ = 0;
  size_t col_num_ = 0;
  int32_t dpi_x_ = 0;
  int32_t dpi_y_ = 0;
  uint32_t mask_red_ = 0;
  uint32_t mask_green_ = 0;
  uint32_t mask_blue_ = 0;
  int32_t decode_status_ = BMP_D_STATUS_HEADER;

 private:
  bool GetDataPosition(uint32_t cur_pos);
  void ReadScanline(uint32_t row_num, const std::vector<uint8_t>& row_buf);
  int32_t DecodeRGB();
  int32_t DecodeRLE8();
  int32_t DecodeRLE4();
  bool ReadData(uint8_t* destination, uint32_t size);
  void SaveDecodingStatus(int32_t status);
  bool ValidateColorIndex(uint8_t val);
  bool ValidateFlag() const;
  void SetHeight(int32_t signed_height);

  RetainPtr<CFX_CodecMemory> input_buffer_;
};

#endif  // CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_
