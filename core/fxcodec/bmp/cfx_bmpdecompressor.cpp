// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/cfx_bmpdecompressor.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "core/fxcodec/bmp/cfx_bmpcontext.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/calculate_pitch.h"
#include "third_party/base/numerics/safe_math.h"

namespace fxcodec {

namespace {

#define BMP_PAL_ENCODE(a, r, g, b) \
  (((uint32_t)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))

constexpr size_t kBmpCoreHeaderSize = 12;
constexpr size_t kBmpInfoHeaderSize = 40;

static_assert(sizeof(BmpCoreHeader) == kBmpCoreHeaderSize,
              "BmpCoreHeader has wrong size");
static_assert(sizeof(BmpInfoHeader) == kBmpInfoHeaderSize,
              "BmpInfoHeader has wrong size");

constexpr uint16_t kBmpSignature = 0x4D42;
constexpr uint8_t kRleMarker = 0;
constexpr uint8_t kRleEol = 0;
constexpr uint8_t kRleEoi = 1;
constexpr uint8_t kRleDelta = 2;
constexpr uint32_t kBmpRgb = 0L;
constexpr uint32_t kBmpRle8 = 1L;
constexpr uint32_t kBmpRle4 = 2L;
constexpr uint32_t kBmpBitfields = 3L;

// Limit of image dimension. Use the same limit as the JBIG2 codecs.
constexpr uint32_t kBmpMaxImageDimension = 65535;

uint8_t HalfRoundUp(uint8_t value) {
  uint16_t value16 = value;
  return static_cast<uint8_t>((value16 + 1) / 2);
}

}  // namespace

CFX_BmpDecompressor::CFX_BmpDecompressor(const CFX_BmpContext* context)
    : context_(context) {}

CFX_BmpDecompressor::~CFX_BmpDecompressor() = default;

void CFX_BmpDecompressor::ReadNextScanline() {
  uint32_t row = img_tb_flag_ ? row_num_ : (height_ - 1 - row_num_);
  context_->m_pDelegate->BmpReadScanline(row, out_row_buffer_);
  ++row_num_;
}

bool CFX_BmpDecompressor::GetDataPosition(uint32_t rcd_pos) {
  return context_->m_pDelegate->BmpInputImagePositionBuf(rcd_pos);
}

BmpDecoder::Status CFX_BmpDecompressor::ReadHeader() {
  if (decode_status_ == DecodeStatus::kHeader) {
    BmpDecoder::Status status = ReadBmpHeader();
    if (status != BmpDecoder::Status::kSuccess)
      return status;
  }

  if (decode_status_ != DecodeStatus::kPal)
    return BmpDecoder::Status::kSuccess;

  if (compress_flag_ == kBmpBitfields)
    return ReadBmpBitfields();

  return ReadBmpPalette();
}

BmpDecoder::Status CFX_BmpDecompressor::ReadBmpHeader() {
  BmpFileHeader bmp_header;
  if (!ReadData(pdfium::as_writable_bytes(pdfium::make_span(&bmp_header, 1))))
    return BmpDecoder::Status::kContinue;

  bmp_header.bfType =
      FXSYS_UINT16_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&bmp_header.bfType));
  bmp_header.bfOffBits = FXSYS_UINT32_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_header.bfOffBits));
  data_size_ =
      FXSYS_UINT32_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&bmp_header.bfSize));
  if (bmp_header.bfType != kBmpSignature)
    return BmpDecoder::Status::kFail;

  size_t pos = input_buffer_->GetPosition();
  if (!ReadData(
          pdfium::as_writable_bytes(pdfium::make_span(&img_ifh_size_, 1)))) {
    return BmpDecoder::Status::kContinue;
  }
  if (!input_buffer_->Seek(pos))
    return BmpDecoder::Status::kFail;

  img_ifh_size_ =
      FXSYS_UINT32_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&img_ifh_size_));
  pal_type_ = PalType::kNew;
  BmpDecoder::Status status = ReadBmpHeaderIfh();
  if (status != BmpDecoder::Status::kSuccess)
    return status;

  return ReadBmpHeaderDimensions();
}

BmpDecoder::Status CFX_BmpDecompressor::ReadBmpHeaderIfh() {
  if (img_ifh_size_ == kBmpCoreHeaderSize) {
    pal_type_ = PalType::kOld;
    BmpCoreHeader bmp_core_header;
    if (!ReadData(pdfium::as_writable_bytes(
            pdfium::make_span(&bmp_core_header, 1)))) {
      return BmpDecoder::Status::kContinue;
    }

    width_ = FXSYS_UINT16_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_core_header.bcWidth));
    height_ = FXSYS_UINT16_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_core_header.bcHeight));
    bit_counts_ = FXSYS_UINT16_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_core_header.bcBitCount));
    compress_flag_ = kBmpRgb;
    img_tb_flag_ = false;
    return BmpDecoder::Status::kSuccess;
  }

  if (img_ifh_size_ == kBmpInfoHeaderSize) {
    BmpInfoHeader bmp_info_header;
    if (!ReadData(pdfium::as_writable_bytes(
            pdfium::make_span(&bmp_info_header, 1)))) {
      return BmpDecoder::Status::kContinue;
    }

    width_ = FXSYS_UINT32_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biWidth));
    int32_t signed_height = FXSYS_UINT32_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biHeight));
    bit_counts_ = FXSYS_UINT16_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biBitCount));
    compress_flag_ = FXSYS_UINT32_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biCompression));
    color_used_ = FXSYS_UINT32_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biClrUsed));
    dpi_x_ = static_cast<int32_t>(FXSYS_UINT32_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biXPelsPerMeter)));
    dpi_y_ = static_cast<int32_t>(FXSYS_UINT32_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biYPelsPerMeter)));
    if (!SetHeight(signed_height))
      return BmpDecoder::Status::kFail;
    return BmpDecoder::Status::kSuccess;
  }

  if (img_ifh_size_ <= sizeof(BmpInfoHeader))
    return BmpDecoder::Status::kFail;

  FX_SAFE_SIZE_T new_pos = input_buffer_->GetPosition();
  BmpInfoHeader bmp_info_header;
  if (!ReadData(
          pdfium::as_writable_bytes(pdfium::make_span(&bmp_info_header, 1)))) {
    return BmpDecoder::Status::kContinue;
  }

  new_pos += img_ifh_size_;
  if (!new_pos.IsValid())
    return BmpDecoder::Status::kFail;

  if (!input_buffer_->Seek(new_pos.ValueOrDie()))
    return BmpDecoder::Status::kContinue;

  uint16_t bi_planes;
  width_ = FXSYS_UINT32_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biWidth));
  int32_t signed_height = FXSYS_UINT32_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biHeight));
  bit_counts_ = FXSYS_UINT16_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biBitCount));
  compress_flag_ = FXSYS_UINT32_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biCompression));
  color_used_ = FXSYS_UINT32_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biClrUsed));
  bi_planes = FXSYS_UINT16_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biPlanes));
  dpi_x_ = FXSYS_UINT32_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biXPelsPerMeter));
  dpi_y_ = FXSYS_UINT32_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biYPelsPerMeter));
  if (!SetHeight(signed_height))
    return BmpDecoder::Status::kFail;
  if (compress_flag_ != kBmpRgb || bi_planes != 1 || color_used_ != 0)
    return BmpDecoder::Status::kFail;
  return BmpDecoder::Status::kSuccess;
}

BmpDecoder::Status CFX_BmpDecompressor::ReadBmpHeaderDimensions() {
  if (width_ > kBmpMaxImageDimension || height_ > kBmpMaxImageDimension ||
      compress_flag_ > kBmpBitfields) {
    return BmpDecoder::Status::kFail;
  }

  switch (bit_counts_) {
    case 1:
    case 4:
    case 8:
    case 16:
    case 24: {
      if (color_used_ > 1U << bit_counts_)
        return BmpDecoder::Status::kFail;
      break;
    }
    case 32:
      break;
    default:
      return BmpDecoder::Status::kFail;
  }
  absl::optional<uint32_t> stride = fxge::CalculatePitch32(bit_counts_, width_);
  if (!stride.has_value())
    return BmpDecoder::Status::kFail;

  src_row_bytes_ = stride.value();
  switch (bit_counts_) {
    case 1:
    case 4:
    case 8:
      stride = fxge::CalculatePitch32(8, width_);
      if (!stride.has_value())
        return BmpDecoder::Status::kFail;
      out_row_bytes_ = stride.value();
      components_ = 1;
      break;
    case 16:
    case 24:
      stride = fxge::CalculatePitch32(24, width_);
      if (!stride.has_value())
        return BmpDecoder::Status::kFail;
      out_row_bytes_ = stride.value();
      components_ = 3;
      break;
    case 32:
      out_row_bytes_ = src_row_bytes_;
      components_ = 4;
      break;
  }
  out_row_buffer_.clear();

  if (out_row_bytes_ <= 0)
    return BmpDecoder::Status::kFail;

  out_row_buffer_.resize(out_row_bytes_);
  SaveDecodingStatus(DecodeStatus::kPal);
  return BmpDecoder::Status::kSuccess;
}

BmpDecoder::Status CFX_BmpDecompressor::ReadBmpBitfields() {
  if (bit_counts_ != 16 && bit_counts_ != 32)
    return BmpDecoder::Status::kFail;

  uint32_t masks[3];
  if (!ReadData(pdfium::as_writable_bytes(pdfium::make_span(masks))))
    return BmpDecoder::Status::kContinue;

  mask_red_ = FXSYS_UINT32_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&masks[0]));
  mask_green_ =
      FXSYS_UINT32_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&masks[1]));
  mask_blue_ = FXSYS_UINT32_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&masks[2]));
  if (mask_red_ & mask_green_ || mask_red_ & mask_blue_ ||
      mask_green_ & mask_blue_) {
    return BmpDecoder::Status::kFail;
  }
  header_offset_ = std::max(header_offset_, 26 + img_ifh_size_);
  SaveDecodingStatus(DecodeStatus::kDataPre);
  return BmpDecoder::Status::kSuccess;
}

BmpDecoder::Status CFX_BmpDecompressor::ReadBmpPalette() {
  if (bit_counts_ == 16) {
    mask_red_ = 0x7C00;
    mask_green_ = 0x03E0;
    mask_blue_ = 0x001F;
  }
  pal_num_ = 0;
  if (bit_counts_ < 16) {
    pal_num_ = 1 << bit_counts_;
    if (color_used_ != 0)
      pal_num_ = color_used_;
    size_t src_pal_size = pal_num_ * PaletteChannelCount();
    std::vector<uint8_t, FxAllocAllocator<uint8_t>> src_pal(src_pal_size);
    uint8_t* src_pal_data = src_pal.data();
    if (!ReadData(src_pal))
      return BmpDecoder::Status::kContinue;

    palette_.resize(pal_num_);
    int32_t src_pal_index = 0;
    if (pal_type_ == PalType::kOld) {
      while (src_pal_index < pal_num_) {
        palette_[src_pal_index++] = BMP_PAL_ENCODE(
            0x00, src_pal_data[2], src_pal_data[1], src_pal_data[0]);
        src_pal_data += 3;
      }
    } else {
      while (src_pal_index < pal_num_) {
        palette_[src_pal_index++] = BMP_PAL_ENCODE(
            src_pal_data[3], src_pal_data[2], src_pal_data[1], src_pal_data[0]);
        src_pal_data += 4;
      }
    }
  }
  header_offset_ = std::max(
      header_offset_, 14 + img_ifh_size_ + pal_num_ * PaletteChannelCount());
  SaveDecodingStatus(DecodeStatus::kDataPre);
  return BmpDecoder::Status::kSuccess;
}

bool CFX_BmpDecompressor::ValidateFlag() const {
  switch (compress_flag_) {
    case kBmpRgb:
    case kBmpBitfields:
    case kBmpRle8:
    case kBmpRle4:
      return true;
    default:
      return false;
  }
}

BmpDecoder::Status CFX_BmpDecompressor::DecodeImage() {
  if (decode_status_ == DecodeStatus::kDataPre) {
    input_buffer_->Seek(0);
    if (!GetDataPosition(header_offset_)) {
      decode_status_ = DecodeStatus::kTail;
      return BmpDecoder::Status::kFail;
    }

    row_num_ = 0;
    SaveDecodingStatus(DecodeStatus::kData);
  }
  if (decode_status_ != DecodeStatus::kData || !ValidateFlag())
    return BmpDecoder::Status::kFail;

  switch (compress_flag_) {
    case kBmpRgb:
    case kBmpBitfields:
      return DecodeRGB();
    case kBmpRle8:
      return DecodeRLE8();
    case kBmpRle4:
      return DecodeRLE4();
    default:
      return BmpDecoder::Status::kFail;
  }
}

bool CFX_BmpDecompressor::ValidateColorIndex(uint8_t val) const {
  return val < pal_num_;
}

BmpDecoder::Status CFX_BmpDecompressor::DecodeRGB() {
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> dest_buf(src_row_bytes_);
  while (row_num_ < height_) {
    size_t idx = 0;
    if (!ReadData(dest_buf))
      return BmpDecoder::Status::kContinue;

    SaveDecodingStatus(DecodeStatus::kData);
    switch (bit_counts_) {
      case 1: {
        for (uint32_t col = 0; col < width_; ++col)
          out_row_buffer_[idx++] =
              dest_buf[col >> 3] & (0x80 >> (col % 8)) ? 0x01 : 0x00;
        break;
      }
      case 4: {
        for (uint32_t col = 0; col < width_; ++col) {
          out_row_buffer_[idx++] = (col & 0x01)
                                       ? (dest_buf[col >> 1] & 0x0F)
                                       : ((dest_buf[col >> 1] & 0xF0) >> 4);
        }
        break;
      }
      case 16: {
        uint16_t* buf = reinterpret_cast<uint16_t*>(dest_buf.data());
        uint8_t blue_bits = 0;
        uint8_t green_bits = 0;
        uint8_t red_bits = 0;
        for (int32_t i = 0; i < 16; i++) {
          if ((mask_blue_ >> i) & 0x01)
            blue_bits++;
          if ((mask_green_ >> i) & 0x01)
            green_bits++;
          if ((mask_red_ >> i) & 0x01)
            red_bits++;
        }
        green_bits += blue_bits;
        red_bits += green_bits;
        if (blue_bits > 8 || green_bits < 8 || red_bits < 8)
          return BmpDecoder::Status::kContinue;
        blue_bits = 8 - blue_bits;
        green_bits -= 8;
        red_bits -= 8;
        for (uint32_t col = 0; col < width_; ++col) {
          *buf = FXSYS_UINT16_GET_LSBFIRST(reinterpret_cast<uint8_t*>(buf));
          out_row_buffer_[idx++] =
              static_cast<uint8_t>((*buf & mask_blue_) << blue_bits);
          out_row_buffer_[idx++] =
              static_cast<uint8_t>((*buf & mask_green_) >> green_bits);
          out_row_buffer_[idx++] =
              static_cast<uint8_t>((*buf++ & mask_red_) >> red_bits);
        }
        break;
      }
      case 8:
      case 24:
      case 32:
        uint8_t* dest_buf_data = dest_buf.data();
        std::copy(dest_buf_data, dest_buf_data + src_row_bytes_,
                  out_row_buffer_.begin());
        idx += src_row_bytes_;
        break;
    }
    for (uint8_t byte : out_row_buffer_) {
      if (!ValidateColorIndex(byte))
        return BmpDecoder::Status::kFail;
    }
    ReadNextScanline();
  }
  SaveDecodingStatus(DecodeStatus::kTail);
  return BmpDecoder::Status::kSuccess;
}

BmpDecoder::Status CFX_BmpDecompressor::DecodeRLE8() {
  uint8_t first_part;
  col_num_ = 0;
  while (true) {
    if (!ReadData(pdfium::make_span(&first_part, 1)))
      return BmpDecoder::Status::kContinue;

    switch (first_part) {
      case kRleMarker: {
        if (!ReadData(pdfium::make_span(&first_part, 1)))
          return BmpDecoder::Status::kContinue;

        switch (first_part) {
          case kRleEol: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(DecodeStatus::kTail);
              return BmpDecoder::Status::kFail;
            }

            ReadNextScanline();
            col_num_ = 0;
            std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
            SaveDecodingStatus(DecodeStatus::kData);
            continue;
          }
          case kRleEoi: {
            if (row_num_ < height_)
              ReadNextScanline();
            SaveDecodingStatus(DecodeStatus::kTail);
            return BmpDecoder::Status::kSuccess;
          }
          case kRleDelta: {
            uint8_t delta[2];
            if (!ReadData(delta))
              return BmpDecoder::Status::kContinue;

            col_num_ += delta[0];
            size_t bmp_row_num__next = row_num_ + delta[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_)
              return BmpDecoder::Status::kFail;

            while (row_num_ < bmp_row_num__next) {
              std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
              ReadNextScanline();
            }
            break;
          }
          default: {
            int32_t avail_size =
                pdfium::base::checked_cast<int32_t>(out_row_bytes_ - col_num_);
            if (!avail_size || static_cast<int32_t>(first_part) > avail_size)
              return BmpDecoder::Status::kFail;

            size_t second_part_size =
                first_part & 1 ? first_part + 1 : first_part;
            std::vector<uint8_t, FxAllocAllocator<uint8_t>> second_part(
                second_part_size);
            uint8_t* second_part_data = second_part.data();
            if (!ReadData(second_part))
              return BmpDecoder::Status::kContinue;

            std::copy(second_part_data, second_part_data + first_part,
                      out_row_buffer_.begin() + col_num_);
            for (size_t i = col_num_; i < col_num_ + first_part; ++i) {
              if (!ValidateColorIndex(out_row_buffer_[i]))
                return BmpDecoder::Status::kFail;
            }
            col_num_ += first_part;
          }
        }
        break;
      }
      default: {
        int32_t avail_size =
            pdfium::base::checked_cast<int32_t>(out_row_bytes_ - col_num_);
        if (!avail_size || static_cast<int32_t>(first_part) > avail_size)
          return BmpDecoder::Status::kFail;

        uint8_t second_part;
        if (!ReadData(pdfium::make_span(&second_part, 1)))
          return BmpDecoder::Status::kContinue;

        std::fill(out_row_buffer_.begin() + col_num_,
                  out_row_buffer_.begin() + col_num_ + first_part, second_part);
        if (!ValidateColorIndex(out_row_buffer_[col_num_]))
          return BmpDecoder::Status::kFail;
        col_num_ += first_part;
      }
    }
  }
}

BmpDecoder::Status CFX_BmpDecompressor::DecodeRLE4() {
  uint8_t first_part;
  col_num_ = 0;
  while (true) {
    if (!ReadData(pdfium::make_span(&first_part, 1)))
      return BmpDecoder::Status::kContinue;

    switch (first_part) {
      case kRleMarker: {
        if (!ReadData(pdfium::make_span(&first_part, 1)))
          return BmpDecoder::Status::kContinue;

        switch (first_part) {
          case kRleEol: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(DecodeStatus::kTail);
              return BmpDecoder::Status::kFail;
            }

            ReadNextScanline();
            col_num_ = 0;
            std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
            SaveDecodingStatus(DecodeStatus::kData);
            continue;
          }
          case kRleEoi: {
            if (row_num_ < height_)
              ReadNextScanline();
            SaveDecodingStatus(DecodeStatus::kTail);
            return BmpDecoder::Status::kSuccess;
          }
          case kRleDelta: {
            uint8_t delta[2];
            if (!ReadData(delta))
              return BmpDecoder::Status::kContinue;

            col_num_ += delta[0];
            size_t bmp_row_num__next = row_num_ + delta[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_)
              return BmpDecoder::Status::kFail;

            while (row_num_ < bmp_row_num__next) {
              std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
              ReadNextScanline();
            }
            break;
          }
          default: {
            int32_t avail_size =
                pdfium::base::checked_cast<int32_t>(out_row_bytes_ - col_num_);
            if (!avail_size)
              return BmpDecoder::Status::kFail;
            uint8_t size = HalfRoundUp(first_part);
            if (static_cast<int32_t>(first_part) > avail_size) {
              if (size + (col_num_ >> 1) > src_row_bytes_)
                return BmpDecoder::Status::kFail;

              first_part = avail_size - 1;
            }
            size_t second_part_size = size & 1 ? size + 1 : size;
            std::vector<uint8_t, FxAllocAllocator<uint8_t>> second_part(
                second_part_size);
            uint8_t* second_part_data = second_part.data();
            if (!ReadData(second_part))
              return BmpDecoder::Status::kContinue;

            for (uint8_t i = 0; i < first_part; i++) {
              uint8_t color = (i & 0x01) ? (*second_part_data++ & 0x0F)
                                         : (*second_part_data & 0xF0) >> 4;
              if (!ValidateColorIndex(color))
                return BmpDecoder::Status::kFail;

              out_row_buffer_[col_num_++] = color;
            }
          }
        }
        break;
      }
      default: {
        int32_t avail_size =
            pdfium::base::checked_cast<int32_t>(out_row_bytes_ - col_num_);
        if (!avail_size)
          return BmpDecoder::Status::kFail;

        if (static_cast<int32_t>(first_part) > avail_size) {
          uint8_t size = HalfRoundUp(first_part);
          if (size + (col_num_ >> 1) > src_row_bytes_)
            return BmpDecoder::Status::kFail;

          first_part = avail_size - 1;
        }
        uint8_t second_part;
        if (!ReadData(pdfium::make_span(&second_part, 1)))
          return BmpDecoder::Status::kContinue;

        for (uint8_t i = 0; i < first_part; i++) {
          uint8_t second_byte = second_part;
          second_byte =
              i & 0x01 ? (second_byte & 0x0F) : (second_byte & 0xF0) >> 4;
          if (!ValidateColorIndex(second_byte))
            return BmpDecoder::Status::kFail;

          out_row_buffer_[col_num_++] = second_byte;
        }
      }
    }
  }
}

bool CFX_BmpDecompressor::ReadData(pdfium::span<uint8_t> buf) {
  return input_buffer_ &&
         input_buffer_->ReadBlock(buf.data(), buf.size()) == buf.size();
}

void CFX_BmpDecompressor::SaveDecodingStatus(DecodeStatus status) {
  decode_status_ = status;
}

void CFX_BmpDecompressor::SetInputBuffer(
    RetainPtr<CFX_CodecMemory> codec_memory) {
  input_buffer_ = std::move(codec_memory);
}

FX_FILESIZE CFX_BmpDecompressor::GetAvailInput() const {
  if (!input_buffer_)
    return 0;

  return input_buffer_->GetSize() - input_buffer_->GetPosition();
}

bool CFX_BmpDecompressor::SetHeight(int32_t signed_height) {
  if (signed_height >= 0) {
    height_ = signed_height;
    return true;
  }
  if (signed_height != std::numeric_limits<int>::min()) {
    height_ = -signed_height;
    img_tb_flag_ = true;
    return true;
  }
  return false;
}

}  // namespace fxcodec
