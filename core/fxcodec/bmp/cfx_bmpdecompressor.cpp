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
#include "core/fxcodec/fx_codec.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/logging.h"
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
constexpr int32_t kBmpPalOld = 1;
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

CFX_BmpDecompressor::CFX_BmpDecompressor(CFX_BmpContext* context)
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

BmpModule::Status CFX_BmpDecompressor::ReadHeader() {
  if (decode_status_ == DecodeStatus::kHeader) {
    BmpModule::Status status = ReadBmpHeader();
    if (status != BmpModule::Status::kSuccess)
      return status;
  }

  if (decode_status_ != DecodeStatus::kPal)
    return BmpModule::Status::kSuccess;

  if (compress_flag_ == kBmpBitfields)
    return ReadBmpBitfields();

  return ReadBmpPalette();
}

BmpModule::Status CFX_BmpDecompressor::ReadBmpHeader() {
  BmpFileHeader bmp_header;
  if (!ReadData(reinterpret_cast<uint8_t*>(&bmp_header),
                sizeof(BmpFileHeader))) {
    return BmpModule::Status::kContinue;
  }

  bmp_header.bfType =
      FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&bmp_header.bfType));
  bmp_header.bfOffBits =
      FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&bmp_header.bfOffBits));
  data_size_ =
      FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&bmp_header.bfSize));
  if (bmp_header.bfType != kBmpSignature)
    return BmpModule::Status::kFail;

  size_t pos = input_buffer_->GetPosition();
  if (!ReadData(reinterpret_cast<uint8_t*>(&img_ifh_size_),
                sizeof(img_ifh_size_))) {
    return BmpModule::Status::kContinue;
  }
  if (!input_buffer_->Seek(pos))
    return BmpModule::Status::kFail;

  img_ifh_size_ =
      FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&img_ifh_size_));
  pal_type_ = 0;
  BmpModule::Status status = ReadBmpHeaderIfh();
  if (status != BmpModule::Status::kSuccess)
    return status;

  return ReadBmpHeaderDimensions();
}

BmpModule::Status CFX_BmpDecompressor::ReadBmpHeaderIfh() {
  if (img_ifh_size_ == kBmpCoreHeaderSize) {
    pal_type_ = 1;
    BmpCoreHeader bmp_core_header;
    if (!ReadData(reinterpret_cast<uint8_t*>(&bmp_core_header),
                  sizeof(BmpCoreHeader))) {
      return BmpModule::Status::kContinue;
    }

    width_ = FXWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_core_header.bcWidth));
    height_ = FXWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_core_header.bcHeight));
    bit_counts_ = FXWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_core_header.bcBitCount));
    compress_flag_ = kBmpRgb;
    img_tb_flag_ = false;
    return BmpModule::Status::kSuccess;
  }

  if (img_ifh_size_ == kBmpInfoHeaderSize) {
    BmpInfoHeader bmp_info_header;
    if (!ReadData(reinterpret_cast<uint8_t*>(&bmp_info_header),
                  sizeof(BmpInfoHeader))) {
      return BmpModule::Status::kContinue;
    }

    width_ = FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biWidth));
    int32_t signed_height = FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biHeight));
    bit_counts_ = FXWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biBitCount));
    compress_flag_ = FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biCompression));
    color_used_ = FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biClrUsed));
    dpi_x_ = static_cast<int32_t>(FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biXPelsPerMeter)));
    dpi_y_ = static_cast<int32_t>(FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&bmp_info_header.biYPelsPerMeter)));
    if (!SetHeight(signed_height))
      return BmpModule::Status::kFail;
    return BmpModule::Status::kSuccess;
  }

  if (img_ifh_size_ <= sizeof(BmpInfoHeader))
    return BmpModule::Status::kFail;

  FX_SAFE_SIZE_T new_pos = input_buffer_->GetPosition();
  BmpInfoHeader bmp_info_header;
  if (!ReadData(reinterpret_cast<uint8_t*>(&bmp_info_header),
                sizeof(bmp_info_header))) {
    return BmpModule::Status::kContinue;
  }

  new_pos += img_ifh_size_;
  if (!new_pos.IsValid())
    return BmpModule::Status::kFail;

  if (!input_buffer_->Seek(new_pos.ValueOrDie()))
    return BmpModule::Status::kContinue;

  uint16_t bi_planes;
  width_ = FXDWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biWidth));
  int32_t signed_height = FXDWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biHeight));
  bit_counts_ = FXWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biBitCount));
  compress_flag_ = FXDWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biCompression));
  color_used_ = FXDWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biClrUsed));
  bi_planes = FXWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biPlanes));
  dpi_x_ = FXDWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biXPelsPerMeter));
  dpi_y_ = FXDWORD_GET_LSBFIRST(
      reinterpret_cast<uint8_t*>(&bmp_info_header.biYPelsPerMeter));
  if (!SetHeight(signed_height))
    return BmpModule::Status::kFail;
  if (compress_flag_ != kBmpRgb || bi_planes != 1 || color_used_ != 0)
    return BmpModule::Status::kFail;
  return BmpModule::Status::kSuccess;
}

BmpModule::Status CFX_BmpDecompressor::ReadBmpHeaderDimensions() {
  if (width_ > kBmpMaxImageDimension || height_ > kBmpMaxImageDimension ||
      compress_flag_ > kBmpBitfields) {
    return BmpModule::Status::kFail;
  }

  switch (bit_counts_) {
    case 1:
    case 4:
    case 8:
    case 16:
    case 24: {
      if (color_used_ > 1U << bit_counts_)
        return BmpModule::Status::kFail;
      break;
    }
    case 32:
      break;
    default:
      return BmpModule::Status::kFail;
  }
  FX_SAFE_UINT32 stride = CalculatePitch32(bit_counts_, width_);
  if (!stride.IsValid())
    return BmpModule::Status::kFail;

  src_row_bytes_ = stride.ValueOrDie();
  switch (bit_counts_) {
    case 1:
    case 4:
    case 8:
      stride = CalculatePitch32(8, width_);
      if (!stride.IsValid())
        return BmpModule::Status::kFail;
      out_row_bytes_ = stride.ValueOrDie();
      components_ = 1;
      break;
    case 16:
    case 24:
      stride = CalculatePitch32(24, width_);
      if (!stride.IsValid())
        return BmpModule::Status::kFail;
      out_row_bytes_ = stride.ValueOrDie();
      components_ = 3;
      break;
    case 32:
      out_row_bytes_ = src_row_bytes_;
      components_ = 4;
      break;
  }
  out_row_buffer_.clear();

  if (out_row_bytes_ <= 0)
    return BmpModule::Status::kFail;

  out_row_buffer_.resize(out_row_bytes_);
  SaveDecodingStatus(DecodeStatus::kPal);
  return BmpModule::Status::kSuccess;
}

BmpModule::Status CFX_BmpDecompressor::ReadBmpBitfields() {
  if (bit_counts_ != 16 && bit_counts_ != 32)
    return BmpModule::Status::kFail;

  uint32_t masks[3];
  if (!ReadData(reinterpret_cast<uint8_t*>(masks), sizeof(masks)))
    return BmpModule::Status::kContinue;

  mask_red_ = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&masks[0]));
  mask_green_ = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&masks[1]));
  mask_blue_ = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&masks[2]));
  if (mask_red_ & mask_green_ || mask_red_ & mask_blue_ ||
      mask_green_ & mask_blue_) {
    return BmpModule::Status::kFail;
  }
  header_offset_ = std::max(header_offset_, 26 + img_ifh_size_);
  SaveDecodingStatus(DecodeStatus::kDataPre);
  return BmpModule::Status::kSuccess;
}

BmpModule::Status CFX_BmpDecompressor::ReadBmpPalette() {
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
    uint32_t src_pal_size = pal_num_ * (pal_type_ ? 3 : 4);
    std::vector<uint8_t, FxAllocAllocator<uint8_t>> src_pal(src_pal_size);
    uint8_t* src_pal_data = src_pal.data();
    if (!ReadData(src_pal_data, src_pal_size))
      return BmpModule::Status::kContinue;

    palette_.resize(pal_num_);
    int32_t src_pal_index = 0;
    if (pal_type_ == kBmpPalOld) {
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
      header_offset_, 14 + img_ifh_size_ + pal_num_ * (pal_type_ ? 3 : 4));
  SaveDecodingStatus(DecodeStatus::kDataPre);
  return BmpModule::Status::kSuccess;
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

BmpModule::Status CFX_BmpDecompressor::DecodeImage() {
  if (decode_status_ == DecodeStatus::kDataPre) {
    input_buffer_->Seek(0);
    if (!GetDataPosition(header_offset_)) {
      decode_status_ = DecodeStatus::kTail;
      return BmpModule::Status::kFail;
    }

    row_num_ = 0;
    SaveDecodingStatus(DecodeStatus::kData);
  }
  if (decode_status_ != DecodeStatus::kData || !ValidateFlag())
    return BmpModule::Status::kFail;

  switch (compress_flag_) {
    case kBmpRgb:
    case kBmpBitfields:
      return DecodeRGB();
    case kBmpRle8:
      return DecodeRLE8();
    case kBmpRle4:
      return DecodeRLE4();
    default:
      return BmpModule::Status::kFail;
  }
}

bool CFX_BmpDecompressor::ValidateColorIndex(uint8_t val) const {
  return val < pal_num_;
}

BmpModule::Status CFX_BmpDecompressor::DecodeRGB() {
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> dest_buf(src_row_bytes_);
  while (row_num_ < height_) {
    size_t idx = 0;
    if (!ReadData(dest_buf.data(), src_row_bytes_))
      return BmpModule::Status::kContinue;

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
          return BmpModule::Status::kContinue;
        blue_bits = 8 - blue_bits;
        green_bits -= 8;
        red_bits -= 8;
        for (uint32_t col = 0; col < width_; ++col) {
          *buf = FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(buf));
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
        return BmpModule::Status::kFail;
    }
    ReadNextScanline();
  }
  SaveDecodingStatus(DecodeStatus::kTail);
  return BmpModule::Status::kSuccess;
}

BmpModule::Status CFX_BmpDecompressor::DecodeRLE8() {
  uint8_t first_part;
  col_num_ = 0;
  while (true) {
    if (!ReadData(&first_part, sizeof(first_part)))
      return BmpModule::Status::kContinue;

    switch (first_part) {
      case kRleMarker: {
        if (!ReadData(&first_part, sizeof(first_part)))
          return BmpModule::Status::kContinue;

        switch (first_part) {
          case kRleEol: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(DecodeStatus::kTail);
              return BmpModule::Status::kFail;
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
            return BmpModule::Status::kSuccess;
          }
          case kRleDelta: {
            uint8_t delta[2];
            if (!ReadData(delta, sizeof(delta)))
              return BmpModule::Status::kContinue;

            col_num_ += delta[0];
            size_t bmp_row_num__next = row_num_ + delta[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_)
              return BmpModule::Status::kFail;

            while (row_num_ < bmp_row_num__next) {
              std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
              ReadNextScanline();
            }
            break;
          }
          default: {
            int32_t avail_size = out_row_bytes_ - col_num_;
            if (!avail_size || static_cast<int32_t>(first_part) > avail_size)
              return BmpModule::Status::kFail;

            size_t second_part_size =
                first_part & 1 ? first_part + 1 : first_part;
            std::vector<uint8_t, FxAllocAllocator<uint8_t>> second_part(
                second_part_size);
            uint8_t* second_part_data = second_part.data();
            if (!ReadData(second_part_data, second_part_size))
              return BmpModule::Status::kContinue;

            std::copy(second_part_data, second_part_data + first_part,
                      out_row_buffer_.begin() + col_num_);
            for (size_t i = col_num_; i < col_num_ + first_part; ++i) {
              if (!ValidateColorIndex(out_row_buffer_[i]))
                return BmpModule::Status::kFail;
            }
            col_num_ += first_part;
          }
        }
        break;
      }
      default: {
        int32_t avail_size = out_row_bytes_ - col_num_;
        if (!avail_size || static_cast<int32_t>(first_part) > avail_size)
          return BmpModule::Status::kFail;

        uint8_t second_part;
        if (!ReadData(&second_part, sizeof(second_part)))
          return BmpModule::Status::kContinue;

        std::fill(out_row_buffer_.begin() + col_num_,
                  out_row_buffer_.begin() + col_num_ + first_part, second_part);
        if (!ValidateColorIndex(out_row_buffer_[col_num_]))
          return BmpModule::Status::kFail;
        col_num_ += first_part;
      }
    }
  }
  return BmpModule::Status::kFail;
}

BmpModule::Status CFX_BmpDecompressor::DecodeRLE4() {
  uint8_t first_part;
  col_num_ = 0;
  while (true) {
    if (!ReadData(&first_part, sizeof(first_part)))
      return BmpModule::Status::kContinue;

    switch (first_part) {
      case kRleMarker: {
        if (!ReadData(&first_part, sizeof(first_part)))
          return BmpModule::Status::kContinue;

        switch (first_part) {
          case kRleEol: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(DecodeStatus::kTail);
              return BmpModule::Status::kFail;
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
            return BmpModule::Status::kSuccess;
          }
          case kRleDelta: {
            uint8_t delta[2];
            if (!ReadData(delta, sizeof(delta)))
              return BmpModule::Status::kContinue;

            col_num_ += delta[0];
            size_t bmp_row_num__next = row_num_ + delta[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_)
              return BmpModule::Status::kFail;

            while (row_num_ < bmp_row_num__next) {
              std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
              ReadNextScanline();
            }
            break;
          }
          default: {
            int32_t avail_size = out_row_bytes_ - col_num_;
            if (!avail_size)
              return BmpModule::Status::kFail;
            uint8_t size = HalfRoundUp(first_part);
            if (static_cast<int32_t>(first_part) > avail_size) {
              if (size + (col_num_ >> 1) > src_row_bytes_)
                return BmpModule::Status::kFail;

              first_part = avail_size - 1;
            }
            size_t second_part_size = size & 1 ? size + 1 : size;
            std::vector<uint8_t, FxAllocAllocator<uint8_t>> second_part(
                second_part_size);
            uint8_t* second_part_data = second_part.data();
            if (!ReadData(second_part_data, second_part_size))
              return BmpModule::Status::kContinue;

            for (uint8_t i = 0; i < first_part; i++) {
              uint8_t color = (i & 0x01) ? (*second_part_data++ & 0x0F)
                                         : (*second_part_data & 0xF0) >> 4;
              if (!ValidateColorIndex(color))
                return BmpModule::Status::kFail;

              out_row_buffer_[col_num_++] = color;
            }
          }
        }
        break;
      }
      default: {
        int32_t avail_size = out_row_bytes_ - col_num_;
        if (!avail_size)
          return BmpModule::Status::kFail;

        if (static_cast<int32_t>(first_part) > avail_size) {
          uint8_t size = HalfRoundUp(first_part);
          if (size + (col_num_ >> 1) > src_row_bytes_)
            return BmpModule::Status::kFail;

          first_part = avail_size - 1;
        }
        uint8_t second_part;
        if (!ReadData(&second_part, sizeof(second_part)))
          return BmpModule::Status::kContinue;

        for (uint8_t i = 0; i < first_part; i++) {
          uint8_t second_byte = second_part;
          second_byte =
              i & 0x01 ? (second_byte & 0x0F) : (second_byte & 0xF0) >> 4;
          if (!ValidateColorIndex(second_byte))
            return BmpModule::Status::kFail;

          out_row_buffer_[col_num_++] = second_byte;
        }
      }
    }
  }
  return BmpModule::Status::kFail;
}

bool CFX_BmpDecompressor::ReadData(uint8_t* destination, uint32_t size) {
  return input_buffer_ && input_buffer_->ReadBlock(destination, size) == size;
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
