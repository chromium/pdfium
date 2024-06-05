// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_
#define CORE_FXCODEC_BMP_CFX_BMPDECOMPRESSOR_H_

#include <stdint.h>

#include <vector>

#include "core/fxcodec/bmp/bmp_decoder.h"
#include "core/fxcodec/bmp/fx_bmp.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CFX_CodecMemory;

namespace fxcodec {

class CFX_BmpContext;

class CFX_BmpDecompressor {
 public:
  explicit CFX_BmpDecompressor(const CFX_BmpContext* context);
  ~CFX_BmpDecompressor();

  BmpDecoder::Status DecodeImage();
  BmpDecoder::Status ReadHeader();
  void SetInputBuffer(RetainPtr<CFX_CodecMemory> codec_memory);
  FX_FILESIZE GetAvailInput() const;

  pdfium::span<const FX_ARGB> palette() const { return palette_; }
  uint32_t width() const { return width_; }
  uint32_t height() const { return height_; }
  int32_t components() const { return components_; }
  bool img_tb_flag() const { return img_tb_flag_; }
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

  enum class PalType : bool { kNew, kOld };

  BmpDecoder::Status ReadBmpHeader();
  BmpDecoder::Status ReadBmpHeaderIfh();
  BmpDecoder::Status ReadBmpHeaderDimensions();
  BmpDecoder::Status ReadBmpBitfields();
  BmpDecoder::Status ReadBmpPalette();
  bool GetDataPosition(uint32_t cur_pos);
  void ReadNextScanline();
  BmpDecoder::Status DecodeRGB();
  BmpDecoder::Status DecodeRLE8();
  BmpDecoder::Status DecodeRLE4();
  bool ReadAllOrNone(pdfium::span<uint8_t> buf);
  void SaveDecodingStatus(DecodeStatus status);
  bool ValidateColorIndex(uint8_t val) const;
  bool ValidateFlag() const;
  bool SetHeight(int32_t signed_height);
  int PaletteChannelCount() const { return pal_type_ == PalType::kNew ? 4 : 3; }

  UnownedPtr<const CFX_BmpContext> const context_;
  DataVector<uint8_t> out_row_buffer_;
  std::vector<FX_ARGB> palette_;
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
  PalType pal_type_ = PalType::kNew;
  uint32_t data_offset_ = 0;
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
