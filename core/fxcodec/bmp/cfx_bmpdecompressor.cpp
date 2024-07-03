// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/cfx_bmpdecompressor.h"

#include <stdint.h>

#include <algorithm>
#include <limits>
#include <utility>

#include "core/fxcodec/bmp/cfx_bmpcontext.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcrt/byteorder.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/safe_math.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/calculate_pitch.h"

namespace fxcodec {

namespace {

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
  if (!ReadAllOrNone(
          pdfium::as_writable_bytes(pdfium::span_from_ref(bmp_header)))) {
    return BmpDecoder::Status::kContinue;
  }

  bmp_header.bfType = fxcrt::FromLE16(bmp_header.bfType);
  data_offset_ = fxcrt::FromLE32(bmp_header.bfOffBits);
  data_size_ = fxcrt::FromLE32(bmp_header.bfSize);
  if (bmp_header.bfType != kBmpSignature)
    return BmpDecoder::Status::kFail;

  size_t pos = input_buffer_->GetPosition();
  if (!ReadAllOrNone(
          pdfium::as_writable_bytes(pdfium::span_from_ref(img_ifh_size_)))) {
    return BmpDecoder::Status::kContinue;
  }
  if (!input_buffer_->Seek(pos))
    return BmpDecoder::Status::kFail;

  img_ifh_size_ = fxcrt::FromLE32(img_ifh_size_);
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
    if (!ReadAllOrNone(pdfium::as_writable_bytes(
            pdfium::span_from_ref(bmp_core_header)))) {
      return BmpDecoder::Status::kContinue;
    }

    width_ = fxcrt::FromLE16(bmp_core_header.bcWidth);
    height_ = fxcrt::FromLE16(bmp_core_header.bcHeight);
    bit_counts_ = fxcrt::FromLE16(bmp_core_header.bcBitCount);
    compress_flag_ = kBmpRgb;
    img_tb_flag_ = false;
    return BmpDecoder::Status::kSuccess;
  }

  if (img_ifh_size_ == kBmpInfoHeaderSize) {
    BmpInfoHeader bmp_info_header;
    if (!ReadAllOrNone(pdfium::as_writable_bytes(
            pdfium::span_from_ref(bmp_info_header)))) {
      return BmpDecoder::Status::kContinue;
    }

    width_ = fxcrt::FromLE32(bmp_info_header.biWidth);
    bit_counts_ = fxcrt::FromLE16(bmp_info_header.biBitCount);
    compress_flag_ = fxcrt::FromLE32(bmp_info_header.biCompression);
    color_used_ = fxcrt::FromLE32(bmp_info_header.biClrUsed);
    dpi_x_ =
        static_cast<int32_t>(fxcrt::FromLE32(bmp_info_header.biXPelsPerMeter));
    dpi_y_ =
        static_cast<int32_t>(fxcrt::FromLE32(bmp_info_header.biYPelsPerMeter));

    int32_t signed_height = fxcrt::FromLE32(bmp_info_header.biHeight);
    if (!SetHeight(signed_height)) {
      return BmpDecoder::Status::kFail;
    }
    return BmpDecoder::Status::kSuccess;
  }

  if (img_ifh_size_ <= sizeof(BmpInfoHeader))
    return BmpDecoder::Status::kFail;

  FX_SAFE_SIZE_T new_pos = input_buffer_->GetPosition();
  BmpInfoHeader bmp_info_header;
  if (!ReadAllOrNone(
          pdfium::as_writable_bytes(pdfium::span_from_ref(bmp_info_header)))) {
    return BmpDecoder::Status::kContinue;
  }

  new_pos += img_ifh_size_;
  if (!new_pos.IsValid())
    return BmpDecoder::Status::kFail;

  if (!input_buffer_->Seek(new_pos.ValueOrDie()))
    return BmpDecoder::Status::kContinue;

  width_ = fxcrt::FromLE32(bmp_info_header.biWidth);
  bit_counts_ = fxcrt::FromLE16(bmp_info_header.biBitCount);
  compress_flag_ = fxcrt::FromLE32(bmp_info_header.biCompression);
  color_used_ = fxcrt::FromLE32(bmp_info_header.biClrUsed);
  dpi_x_ = fxcrt::FromLE32(bmp_info_header.biXPelsPerMeter);
  dpi_y_ = fxcrt::FromLE32(bmp_info_header.biYPelsPerMeter);

  int32_t signed_height = fxcrt::FromLE32(bmp_info_header.biHeight);
  if (!SetHeight(signed_height)) {
    return BmpDecoder::Status::kFail;
  }
  uint16_t bi_planes = fxcrt::FromLE16(bmp_info_header.biPlanes);
  if (compress_flag_ != kBmpRgb || bi_planes != 1 || color_used_ != 0) {
    return BmpDecoder::Status::kFail;
  }
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
  std::optional<uint32_t> stride = fxge::CalculatePitch32(bit_counts_, width_);
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
  if (!ReadAllOrNone(pdfium::as_writable_byte_span(masks))) {
    return BmpDecoder::Status::kContinue;
  }

  mask_red_ = fxcrt::FromLE32(masks[0]);
  mask_green_ = fxcrt::FromLE32(masks[1]);
  mask_blue_ = fxcrt::FromLE32(masks[2]);
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
  uint32_t palette_entries = 0;
  if (bit_counts_ < 16) {
    palette_entries = 1 << bit_counts_;
    if (color_used_ != 0) {
      palette_entries = color_used_;
    }
    size_t pal_bytes = palette_entries * PaletteChannelCount();
    DataVector<uint8_t> src_pal(pal_bytes);
    if (!ReadAllOrNone(src_pal))
      return BmpDecoder::Status::kContinue;

    palette_.resize(palette_entries);
    if (pal_type_ == PalType::kOld) {
      auto src_pal_data =
          fxcrt::reinterpret_span<FX_BGR_STRUCT<uint8_t>, uint8_t>(src_pal);
      for (auto& dest : palette_) {
        const auto& entry = src_pal_data.front();
        dest = ArgbEncode(0x00, entry.red, entry.green, entry.blue);
        src_pal_data = src_pal_data.subspan(1);
      }
    } else {
      auto src_pal_data =
          fxcrt::reinterpret_span<FX_BGRA_STRUCT<uint8_t>, uint8_t>(src_pal);
      for (auto& dest : palette_) {
        const auto& entry = src_pal_data.front();
        dest = ArgbEncode(entry.alpha, entry.red, entry.green, entry.blue);
        src_pal_data = src_pal_data.subspan(1);
      }
    }
  }
  header_offset_ =
      std::max(header_offset_,
               14 + img_ifh_size_ + palette_entries * PaletteChannelCount());
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
    // In order to tolerate certain corrupt BMP files, use the header offset if
    // the data offset would point into the header.
    data_offset_ = std::max(header_offset_, data_offset_);

    input_buffer_->Seek(input_buffer_->GetSize());
    if (!GetDataPosition(data_offset_)) {
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
  return val < palette_.size();
}

BmpDecoder::Status CFX_BmpDecompressor::DecodeRGB() {
  DataVector<uint8_t> dest_buf(src_row_bytes_);
  while (row_num_ < height_) {
    size_t idx = 0;
    if (!ReadAllOrNone(dest_buf))
      return BmpDecoder::Status::kContinue;

    SaveDecodingStatus(DecodeStatus::kData);
    switch (bit_counts_) {
      case 1: {
        for (uint32_t col = 0; col < width_; ++col) {
          uint8_t index =
              dest_buf[col >> 3] & (0x80 >> (col % 8)) ? 0x01 : 0x00;
          if (!ValidateColorIndex(index))
            return BmpDecoder::Status::kFail;
          out_row_buffer_[idx++] = index;
        }
        break;
      }
      case 4: {
        for (uint32_t col = 0; col < width_; ++col) {
          uint8_t index = (col & 0x01) ? (dest_buf[col >> 1] & 0x0F)
                                       : ((dest_buf[col >> 1] & 0xF0) >> 4);
          if (!ValidateColorIndex(index))
            return BmpDecoder::Status::kFail;
          out_row_buffer_[idx++] = index;
        }
        break;
      }
      case 8: {
        for (uint32_t col = 0; col < width_; ++col) {
          uint8_t index = dest_buf[col];
          if (!ValidateColorIndex(index))
            return BmpDecoder::Status::kFail;
          out_row_buffer_[idx++] = index;
        }
        break;
      }
      case 16: {
        auto buf =
            fxcrt::reinterpret_span<uint16_t>(pdfium::make_span(dest_buf));
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
          buf.front() = fxcrt::FromLE16(buf.front());
          out_row_buffer_[idx++] =
              static_cast<uint8_t>((buf.front() & mask_blue_) << blue_bits);
          out_row_buffer_[idx++] =
              static_cast<uint8_t>((buf.front() & mask_green_) >> green_bits);
          out_row_buffer_[idx++] =
              static_cast<uint8_t>((buf.front() & mask_red_) >> red_bits);
          buf = buf.subspan(1);
        }
        break;
      }
      case 24:
      case 32:
        // TODO(crbug.com/pdfium/1901): Apply bitfields.
        fxcrt::Copy(pdfium::make_span(dest_buf).first(src_row_bytes_),
                    out_row_buffer_);
        idx += src_row_bytes_;
        break;
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
    if (!ReadAllOrNone(pdfium::span_from_ref(first_part))) {
      return BmpDecoder::Status::kContinue;
    }

    switch (first_part) {
      case kRleMarker: {
        if (!ReadAllOrNone(pdfium::span_from_ref(first_part))) {
          return BmpDecoder::Status::kContinue;
        }

        switch (first_part) {
          case kRleEol: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(DecodeStatus::kTail);
              return BmpDecoder::Status::kFail;
            }

            ReadNextScanline();
            col_num_ = 0;
            fxcrt::Fill(out_row_buffer_, 0);
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
            if (!ReadAllOrNone(delta))
              return BmpDecoder::Status::kContinue;

            col_num_ += delta[0];
            size_t bmp_row_num__next = row_num_ + delta[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_)
              return BmpDecoder::Status::kFail;

            while (row_num_ < bmp_row_num__next) {
              fxcrt::Fill(out_row_buffer_, 0);
              ReadNextScanline();
            }
            break;
          }
          default: {
            int32_t avail_size =
                pdfium::checked_cast<int32_t>(out_row_bytes_ - col_num_);
            if (!avail_size || static_cast<int32_t>(first_part) > avail_size)
              return BmpDecoder::Status::kFail;

            size_t second_part_size =
                first_part & 1 ? first_part + 1 : first_part;
            DataVector<uint8_t> second_part(second_part_size);
            if (!ReadAllOrNone(second_part))
              return BmpDecoder::Status::kContinue;

            fxcrt::Copy(pdfium::make_span(second_part).first(first_part),
                        pdfium::make_span(out_row_buffer_).subspan(col_num_));

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
            pdfium::checked_cast<int32_t>(out_row_bytes_ - col_num_);
        if (!avail_size || static_cast<int32_t>(first_part) > avail_size)
          return BmpDecoder::Status::kFail;

        uint8_t second_part;
        if (!ReadAllOrNone(pdfium::span_from_ref(second_part))) {
          return BmpDecoder::Status::kContinue;
        }

        fxcrt::Fill(
            pdfium::make_span(out_row_buffer_).subspan(col_num_, first_part),
            second_part);

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
    if (!ReadAllOrNone(pdfium::span_from_ref(first_part))) {
      return BmpDecoder::Status::kContinue;
    }

    switch (first_part) {
      case kRleMarker: {
        if (!ReadAllOrNone(pdfium::span_from_ref(first_part))) {
          return BmpDecoder::Status::kContinue;
        }

        switch (first_part) {
          case kRleEol: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(DecodeStatus::kTail);
              return BmpDecoder::Status::kFail;
            }

            ReadNextScanline();
            col_num_ = 0;
            fxcrt::Fill(out_row_buffer_, 0);
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
            if (!ReadAllOrNone(delta))
              return BmpDecoder::Status::kContinue;

            col_num_ += delta[0];
            size_t bmp_row_num__next = row_num_ + delta[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_)
              return BmpDecoder::Status::kFail;

            while (row_num_ < bmp_row_num__next) {
              fxcrt::Fill(out_row_buffer_, 0);
              ReadNextScanline();
            }
            break;
          }
          default: {
            int32_t avail_size =
                pdfium::checked_cast<int32_t>(out_row_bytes_ - col_num_);
            if (!avail_size)
              return BmpDecoder::Status::kFail;
            uint8_t size = HalfRoundUp(first_part);
            if (static_cast<int32_t>(first_part) > avail_size) {
              if (size + (col_num_ >> 1) > src_row_bytes_)
                return BmpDecoder::Status::kFail;

              first_part = avail_size - 1;
            }
            size_t second_part_size = size & 1 ? size + 1 : size;
            DataVector<uint8_t> second_part(second_part_size);
            uint8_t* second_part_data = second_part.data();
            if (!ReadAllOrNone(second_part))
              return BmpDecoder::Status::kContinue;

            for (uint8_t i = 0; i < first_part; i++) {
              uint8_t color = (i & 0x01)
                                  ? UNSAFE_TODO((*second_part_data++ & 0x0F))
                                  : (*second_part_data & 0xF0) >> 4;
              if (!ValidateColorIndex(color)) {
                return BmpDecoder::Status::kFail;
              }
              out_row_buffer_[col_num_++] = color;
            }
          }
        }
        break;
      }
      default: {
        int32_t avail_size =
            pdfium::checked_cast<int32_t>(out_row_bytes_ - col_num_);
        if (!avail_size)
          return BmpDecoder::Status::kFail;

        if (static_cast<int32_t>(first_part) > avail_size) {
          uint8_t size = HalfRoundUp(first_part);
          if (size + (col_num_ >> 1) > src_row_bytes_)
            return BmpDecoder::Status::kFail;

          first_part = avail_size - 1;
        }
        uint8_t second_part;
        if (!ReadAllOrNone(pdfium::span_from_ref(second_part))) {
          return BmpDecoder::Status::kContinue;
        }

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

bool CFX_BmpDecompressor::ReadAllOrNone(pdfium::span<uint8_t> buf) {
  if (!input_buffer_)
    return false;

  size_t original_position = input_buffer_->GetPosition();
  size_t read = input_buffer_->ReadBlock(buf);
  if (read < buf.size()) {
    input_buffer_->Seek(original_position);
    return false;
  }

  return true;
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
