// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_
#define CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_

#include <vector>

#include "core/fxcodec/bmp/bmpmodule.h"
#include "core/fxcodec/bmp/fx_bmp.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_CodecMemory;

namespace fxcodec {

class CFX_BmpContext;

class CFX_BmpDecompressor {
 public:
  explicit CFX_BmpDecompressor(CFX_BmpContext* context);
  ~CFX_BmpDecompressor();

  BmpModule::Status DecodeImage();
  BmpModule::Status ReadHeader();
  void SetInputBuffer(RetainPtr<CFX_CodecMemory> codec_memory);
  FX_FILESIZE GetAvailInput() const;

  const std::vector<uint32_t>* palette() const { return &palette_; }
  uint32_t width() const { return width_; }
  uint32_t height() const { return height_; }
  int32_t components() const { return components_; }
  bool img_tb_flag() const { return img_tb_flag_; }
  int32_t pal_num() const { return pal_num_; }
  int32_t dpi_x() const { return dpi_x_; }
  int32_t dpi_y() const { return dpi_y_; }

 private:
  enum class DecodeStatus : uint8_t {
    kHeader,
    kPal,
    kDataPre,
    kData,
    kTail,
  };

  BmpModule::Status ReadBmpHeader();
  BmpModule::Status ReadBmpHeaderIfh();
  BmpModule::Status ReadBmpHeaderDimensions();
  BmpModule::Status ReadBmpBitfields();
  BmpModule::Status ReadBmpPalette();
  bool GetDataPosition(uint32_t cur_pos);
  void ReadNextScanline();
  BmpModule::Status DecodeRGB();
  BmpModule::Status DecodeRLE8();
  BmpModule::Status DecodeRLE4();
  bool ReadData(uint8_t* destination, uint32_t size);
  void SaveDecodingStatus(DecodeStatus status);
  bool ValidateColorIndex(uint8_t val) const;
  bool ValidateFlag() const;
  bool SetHeight(int32_t signed_height);

  UnownedPtr<CFX_BmpContext> const context_;
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
  uint32_t row_num_ = 0;
  uint32_t col_num_ = 0;
  int32_t dpi_x_ = 0;
  int32_t dpi_y_ = 0;
  uint32_t mask_red_ = 0;
  uint32_t mask_green_ = 0;
  uint32_t mask_blue_ = 0;
  DecodeStatus decode_status_ = DecodeStatus::kHeader;
  RetainPtr<CFX_CodecMemory> input_buffer_;
};

}  // namespace fxcodec

#endif  // CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_
