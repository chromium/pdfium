// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/fx_bmp.h"

#include <algorithm>
#include <limits>

#include "core/fxcrt/fx_system.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"

static_assert(sizeof(BmpFileHeader) == 14,
              "BmpFileHeader should have a size of 14");

namespace {

const size_t kBmpCoreHeaderSize = 12;
const size_t kBmpInfoHeaderSize = 40;

uint8_t HalfRoundUp(uint8_t value) {
  uint16_t value16 = value;
  return static_cast<uint8_t>((value16 + 1) / 2);
}

}  // namespace

BMPDecompressor::BMPDecompressor()
    : context_ptr_(nullptr),
      next_in_(nullptr),
      header_offset_(0),
      width_(0),
      height_(0),
      compress_flag_(0),
      components_(0),
      src_row_bytes_(0),
      out_row_bytes_(0),
      bit_counts_(0),
      color_used_(0),
      imgTB_flag_(false),
      pal_num_(0),
      pal_type_(0),
      data_size_(0),
      img_data_offset_(0),
      img_ifh_size_(0),
      row_num_(0),
      col_num_(0),
      dpi_x_(0),
      dpi_y_(0),
      mask_red_(0),
      mask_green_(0),
      mask_blue_(0),
      avail_in_(0),
      skip_size_(0),
      decode_status_(BMP_D_STATUS_HEADER) {}

BMPDecompressor::~BMPDecompressor() {}

void BMPDecompressor::Error() {
  longjmp(jmpbuf_, 1);
}

void BMPDecompressor::ReadScanline(uint32_t row_num_,
                                   const std::vector<uint8_t>& row_buf) {
  auto* p = reinterpret_cast<CBmpContext*>(context_ptr_);
  p->m_pDelegate->BmpReadScanline(row_num_, row_buf);
}

bool BMPDecompressor::GetDataPosition(uint32_t rcd_pos) {
  auto* p = reinterpret_cast<CBmpContext*>(context_ptr_);
  return p->m_pDelegate->BmpInputImagePositionBuf(rcd_pos);
}

int32_t BMPDecompressor::ReadHeader() {
  uint32_t skip_size_org = skip_size_;
  if (decode_status_ == BMP_D_STATUS_HEADER) {
    BmpFileHeader* pBmp_header = nullptr;
    if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_header),
                  sizeof(BmpFileHeader))) {
      return 2;
    }

    pBmp_header->bfType =
        FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&pBmp_header->bfType));
    pBmp_header->bfOffBits = FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&pBmp_header->bfOffBits));
    data_size_ =
        FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&pBmp_header->bfSize));
    if (pBmp_header->bfType != BMP_SIGNATURE) {
      Error();
      NOTREACHED();
    }
    if (avail_in_ < sizeof(uint32_t)) {
      skip_size_ = skip_size_org;
      return 2;
    }
    img_ifh_size_ =
        FXDWORD_GET_LSBFIRST(static_cast<uint8_t*>(next_in_ + skip_size_));
    pal_type_ = 0;
    static_assert(sizeof(BmpCoreHeader) == kBmpCoreHeaderSize,
                  "BmpCoreHeader has wrong size");
    static_assert(sizeof(BmpInfoHeader) == kBmpInfoHeaderSize,
                  "BmpInfoHeader has wrong size");
    switch (img_ifh_size_) {
      case kBmpCoreHeaderSize: {
        pal_type_ = 1;
        BmpCoreHeader* pBmp_core_header = nullptr;
        if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_core_header),
                      img_ifh_size_)) {
          skip_size_ = skip_size_org;
          return 2;
        }
        width_ = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_core_header->bcWidth));
        height_ = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_core_header->bcHeight));
        bit_counts_ = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_core_header->bcBitCount));
        compress_flag_ = BMP_RGB;
        imgTB_flag_ = false;
      } break;
      case kBmpInfoHeaderSize: {
        BmpInfoHeader* pBmp_info_header = nullptr;
        if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_info_header),
                      img_ifh_size_)) {
          skip_size_ = skip_size_org;
          return 2;
        }
        width_ = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biWidth));
        int32_t signed_height = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biHeight));
        bit_counts_ = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biBitCount));
        compress_flag_ = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biCompression));
        color_used_ = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biClrUsed));
        dpi_x_ = static_cast<int32_t>(FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biXPelsPerMeter)));
        dpi_y_ = static_cast<int32_t>(FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biYPelsPerMeter)));
        SetHeight(signed_height);
      } break;
      default: {
        if (img_ifh_size_ >
            std::min(kBmpInfoHeaderSize, sizeof(BmpInfoHeader))) {
          BmpInfoHeader* pBmp_info_header = nullptr;
          if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_info_header),
                        img_ifh_size_)) {
            skip_size_ = skip_size_org;
            return 2;
          }
          uint16_t biPlanes;
          width_ = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biWidth));
          int32_t signed_height = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biHeight));
          bit_counts_ = FXWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biBitCount));
          compress_flag_ = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biCompression));
          color_used_ = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biClrUsed));
          biPlanes = FXWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biPlanes));
          dpi_x_ = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biXPelsPerMeter));
          dpi_y_ = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biYPelsPerMeter));
          SetHeight(signed_height);
          if (compress_flag_ == BMP_RGB && biPlanes == 1 && color_used_ == 0)
            break;
        }
        Error();
        NOTREACHED();
      }
    }
    if (width_ > BMP_MAX_WIDTH || compress_flag_ > BMP_BITFIELDS) {
      Error();
      NOTREACHED();
    }
    switch (bit_counts_) {
      case 1:
      case 4:
      case 8:
      case 16:
      case 24: {
        if (color_used_ > 1U << bit_counts_) {
          Error();
          NOTREACHED();
        }
      }
      case 32:
        break;
      default:
        Error();
        NOTREACHED();
    }
    src_row_bytes_ = BMP_WIDTHBYTES(width_, bit_counts_);
    switch (bit_counts_) {
      case 1:
      case 4:
      case 8:
        out_row_bytes_ = BMP_WIDTHBYTES(width_, 8);
        components_ = 1;
        break;
      case 16:
      case 24:
        out_row_bytes_ = BMP_WIDTHBYTES(width_, 24);
        components_ = 3;
        break;
      case 32:
        out_row_bytes_ = src_row_bytes_;
        components_ = 4;
        break;
    }
    out_row_buffer_.clear();

    if (out_row_bytes_ <= 0) {
      Error();
      NOTREACHED();
    }

    out_row_buffer_.resize(out_row_bytes_);
    SaveDecodingStatus(BMP_D_STATUS_PAL);
  }
  if (decode_status_ == BMP_D_STATUS_PAL) {
    skip_size_org = skip_size_;
    if (compress_flag_ == BMP_BITFIELDS) {
      if (bit_counts_ != 16 && bit_counts_ != 32) {
        Error();
        NOTREACHED();
      }
      uint32_t* mask;
      if (ReadData(reinterpret_cast<uint8_t**>(&mask), 3 * sizeof(uint32_t)) ==
          nullptr) {
        skip_size_ = skip_size_org;
        return 2;
      }
      mask_red_ = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&mask[0]));
      mask_green_ = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&mask[1]));
      mask_blue_ = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&mask[2]));
      if (mask_red_ & mask_green_ || mask_red_ & mask_blue_ ||
          mask_green_ & mask_blue_) {
        Error();
        NOTREACHED();
      }
      header_offset_ = std::max(header_offset_, 26 + img_ifh_size_);
      SaveDecodingStatus(BMP_D_STATUS_DATA_PRE);
      return 1;
    } else if (bit_counts_ == 16) {
      mask_red_ = 0x7C00;
      mask_green_ = 0x03E0;
      mask_blue_ = 0x001F;
    }
    pal_num_ = 0;
    if (bit_counts_ < 16) {
      pal_num_ = 1 << bit_counts_;
      if (color_used_ != 0)
        pal_num_ = color_used_;
      uint8_t* src_pal_ptr = nullptr;
      uint32_t src_pal_size = pal_num_ * (pal_type_ ? 3 : 4);
      if (ReadData(&src_pal_ptr, src_pal_size) == nullptr) {
        skip_size_ = skip_size_org;
        return 2;
      }
      palette_.resize(pal_num_);
      int32_t src_pal_index = 0;
      if (pal_type_ == BMP_PAL_OLD) {
        while (src_pal_index < pal_num_) {
          palette_[src_pal_index++] = BMP_PAL_ENCODE(
              0x00, src_pal_ptr[2], src_pal_ptr[1], src_pal_ptr[0]);
          src_pal_ptr += 3;
        }
      } else {
        while (src_pal_index < pal_num_) {
          palette_[src_pal_index++] = BMP_PAL_ENCODE(
              src_pal_ptr[3], src_pal_ptr[2], src_pal_ptr[1], src_pal_ptr[0]);
          src_pal_ptr += 4;
        }
      }
    }
    header_offset_ = std::max(
        header_offset_, 14 + img_ifh_size_ + pal_num_ * (pal_type_ ? 3 : 4));
    SaveDecodingStatus(BMP_D_STATUS_DATA_PRE);
  }
  return 1;
}

bool BMPDecompressor::ValidateFlag() const {
  switch (compress_flag_) {
    case BMP_RGB:
    case BMP_BITFIELDS:
    case BMP_RLE8:
    case BMP_RLE4:
      return true;
    default:
      return false;
  }
}

int32_t BMPDecompressor::DecodeImage() {
  if (decode_status_ == BMP_D_STATUS_DATA_PRE) {
    avail_in_ = 0;
    if (!GetDataPosition(header_offset_)) {
      decode_status_ = BMP_D_STATUS_TAIL;
      Error();
      NOTREACHED();
    }
    row_num_ = 0;
    SaveDecodingStatus(BMP_D_STATUS_DATA);
  }
  if (decode_status_ != BMP_D_STATUS_DATA || !ValidateFlag()) {
    Error();
    NOTREACHED();
  }
  switch (compress_flag_) {
    case BMP_RGB:
    case BMP_BITFIELDS:
      return DecodeRGB();
    case BMP_RLE8:
      return DecodeRLE8();
    case BMP_RLE4:
      return DecodeRLE4();
    default:
      return 0;
  }
}

bool BMPDecompressor::ValidateColorIndex(uint8_t val) {
  if (val >= pal_num_) {
    Error();
    NOTREACHED();
  }
  return true;
}

int32_t BMPDecompressor::DecodeRGB() {
  uint8_t* des_buf = nullptr;
  while (row_num_ < height_) {
    size_t idx = 0;
    if (!ReadData(&des_buf, src_row_bytes_))
      return 2;

    SaveDecodingStatus(BMP_D_STATUS_DATA);
    switch (bit_counts_) {
      case 1: {
        for (uint32_t col = 0; col < width_; ++col)
          out_row_buffer_[idx++] =
              des_buf[col >> 3] & (0x80 >> (col % 8)) ? 0x01 : 0x00;
      } break;
      case 4: {
        for (uint32_t col = 0; col < width_; ++col) {
          out_row_buffer_[idx++] = (col & 0x01)
                                       ? (des_buf[col >> 1] & 0x0F)
                                       : ((des_buf[col >> 1] & 0xF0) >> 4);
        }
      } break;
      case 16: {
        uint16_t* buf = (uint16_t*)des_buf;
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
          return 2;
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
      } break;
      case 8:
      case 24:
      case 32:
        std::copy(des_buf, des_buf + src_row_bytes_, out_row_buffer_.begin());
        idx += src_row_bytes_;
        break;
    }
    for (uint8_t byte : out_row_buffer_) {
      if (!ValidateColorIndex(byte))
        return 0;
    }
    ReadScanline(imgTB_flag_ ? row_num_++ : (height_ - 1 - row_num_++),
                 out_row_buffer_);
  }
  SaveDecodingStatus(BMP_D_STATUS_TAIL);
  return 1;
}

int32_t BMPDecompressor::DecodeRLE8() {
  uint8_t* first_byte_ptr = nullptr;
  uint8_t* second_byte_ptr = nullptr;
  col_num_ = 0;
  while (true) {
    uint32_t skip_size_org = skip_size_;
    if (!ReadData(&first_byte_ptr, 1))
      return 2;

    switch (*first_byte_ptr) {
      case RLE_MARKER: {
        if (!ReadData(&first_byte_ptr, 1)) {
          skip_size_ = skip_size_org;
          return 2;
        }
        switch (*first_byte_ptr) {
          case RLE_EOL: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(BMP_D_STATUS_TAIL);
              Error();
              NOTREACHED();
            }
            ReadScanline(imgTB_flag_ ? row_num_++ : (height_ - 1 - row_num_++),
                         out_row_buffer_);
            col_num_ = 0;
            std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
            SaveDecodingStatus(BMP_D_STATUS_DATA);
            continue;
          }
          case RLE_EOI: {
            if (row_num_ < height_) {
              ReadScanline(
                  imgTB_flag_ ? row_num_++ : (height_ - 1 - row_num_++),
                  out_row_buffer_);
            }
            SaveDecodingStatus(BMP_D_STATUS_TAIL);
            return 1;
          }
          case RLE_DELTA: {
            uint8_t* delta_ptr;
            if (!ReadData(&delta_ptr, 2)) {
              skip_size_ = skip_size_org;
              return 2;
            }
            col_num_ += delta_ptr[0];
            size_t bmp_row_num__next = row_num_ + delta_ptr[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_) {
              Error();
              NOTREACHED();
            }
            while (row_num_ < bmp_row_num__next) {
              std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
              ReadScanline(
                  imgTB_flag_ ? row_num_++ : (height_ - 1 - row_num_++),
                  out_row_buffer_);
            }
          } break;
          default: {
            int32_t avail_size = out_row_bytes_ - col_num_;
            if (!avail_size ||
                static_cast<int32_t>(*first_byte_ptr) > avail_size) {
              Error();
              NOTREACHED();
            }
            if (!ReadData(&second_byte_ptr, *first_byte_ptr & 1
                                                ? *first_byte_ptr + 1
                                                : *first_byte_ptr)) {
              skip_size_ = skip_size_org;
              return 2;
            }
            std::copy(second_byte_ptr, second_byte_ptr + *first_byte_ptr,
                      out_row_buffer_.begin() + col_num_);
            for (size_t i = col_num_; i < col_num_ + *first_byte_ptr; ++i) {
              if (!ValidateColorIndex(out_row_buffer_[i]))
                return 0;
            }
            col_num_ += *first_byte_ptr;
          }
        }
      } break;
      default: {
        int32_t avail_size = out_row_bytes_ - col_num_;
        if (!avail_size || static_cast<int32_t>(*first_byte_ptr) > avail_size) {
          Error();
          NOTREACHED();
        }
        if (!ReadData(&second_byte_ptr, 1)) {
          skip_size_ = skip_size_org;
          return 2;
        }
        std::fill(out_row_buffer_.begin() + col_num_,
                  out_row_buffer_.begin() + col_num_ + *first_byte_ptr,
                  *second_byte_ptr);
        if (!ValidateColorIndex(out_row_buffer_[col_num_]))
          return 0;
        col_num_ += *first_byte_ptr;
      }
    }
  }
  Error();
  NOTREACHED();
}

int32_t BMPDecompressor::DecodeRLE4() {
  uint8_t* first_byte_ptr = nullptr;
  uint8_t* second_byte_ptr = nullptr;
  col_num_ = 0;
  while (true) {
    uint32_t skip_size_org = skip_size_;
    if (!ReadData(&first_byte_ptr, 1))
      return 2;

    switch (*first_byte_ptr) {
      case RLE_MARKER: {
        if (!ReadData(&first_byte_ptr, 1)) {
          skip_size_ = skip_size_org;
          return 2;
        }
        switch (*first_byte_ptr) {
          case RLE_EOL: {
            if (row_num_ >= height_) {
              SaveDecodingStatus(BMP_D_STATUS_TAIL);
              Error();
              NOTREACHED();
            }
            ReadScanline(imgTB_flag_ ? row_num_++ : (height_ - 1 - row_num_++),
                         out_row_buffer_);
            col_num_ = 0;
            std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
            SaveDecodingStatus(BMP_D_STATUS_DATA);
            continue;
          }
          case RLE_EOI: {
            if (row_num_ < height_) {
              ReadScanline(
                  imgTB_flag_ ? row_num_++ : (height_ - 1 - row_num_++),
                  out_row_buffer_);
            }
            SaveDecodingStatus(BMP_D_STATUS_TAIL);
            return 1;
          }
          case RLE_DELTA: {
            uint8_t* delta_ptr;
            if (!ReadData(&delta_ptr, 2)) {
              skip_size_ = skip_size_org;
              return 2;
            }
            col_num_ += delta_ptr[0];
            size_t bmp_row_num__next = row_num_ + delta_ptr[1];
            if (col_num_ >= out_row_bytes_ || bmp_row_num__next >= height_) {
              Error();
              NOTREACHED();
            }
            while (row_num_ < bmp_row_num__next) {
              std::fill(out_row_buffer_.begin(), out_row_buffer_.end(), 0);
              ReadScanline(
                  imgTB_flag_ ? row_num_++ : (height_ - 1 - row_num_++),
                  out_row_buffer_);
            }
          } break;
          default: {
            int32_t avail_size = out_row_bytes_ - col_num_;
            if (!avail_size) {
              Error();
              NOTREACHED();
            }
            uint8_t size = HalfRoundUp(*first_byte_ptr);
            if (static_cast<int32_t>(*first_byte_ptr) > avail_size) {
              if (size + (col_num_ >> 1) > src_row_bytes_) {
                Error();
                NOTREACHED();
              }
              *first_byte_ptr = avail_size - 1;
            }
            if (!ReadData(&second_byte_ptr, size & 1 ? size + 1 : size)) {
              skip_size_ = skip_size_org;
              return 2;
            }
            for (uint8_t i = 0; i < *first_byte_ptr; i++) {
              uint8_t color = (i & 0x01) ? (*second_byte_ptr++ & 0x0F)
                                         : (*second_byte_ptr & 0xF0) >> 4;
              if (!ValidateColorIndex(color))
                return 0;

              out_row_buffer_[col_num_++] = color;
            }
          }
        }
      } break;
      default: {
        int32_t avail_size = out_row_bytes_ - col_num_;
        if (!avail_size) {
          Error();
          NOTREACHED();
        }
        if (static_cast<int32_t>(*first_byte_ptr) > avail_size) {
          uint8_t size = HalfRoundUp(*first_byte_ptr);
          if (size + (col_num_ >> 1) > src_row_bytes_) {
            Error();
            NOTREACHED();
          }
          *first_byte_ptr = avail_size - 1;
        }
        if (!ReadData(&second_byte_ptr, 1)) {
          skip_size_ = skip_size_org;
          return 2;
        }
        for (uint8_t i = 0; i < *first_byte_ptr; i++) {
          uint8_t second_byte = *second_byte_ptr;
          second_byte =
              i & 0x01 ? (second_byte & 0x0F) : (second_byte & 0xF0) >> 4;
          if (!ValidateColorIndex(second_byte))
            return 0;
          out_row_buffer_[col_num_++] = second_byte;
        }
      }
    }
  }
  Error();
  NOTREACHED();
}

uint8_t* BMPDecompressor::ReadData(uint8_t** des_buf, uint32_t data_size_) {
  if (avail_in_ < skip_size_ + data_size_)
    return nullptr;

  *des_buf = next_in_ + skip_size_;
  skip_size_ += data_size_;
  return *des_buf;
}

void BMPDecompressor::SaveDecodingStatus(int32_t status) {
  decode_status_ = status;
  next_in_ += skip_size_;
  avail_in_ -= skip_size_;
  skip_size_ = 0;
}

void BMPDecompressor::SetInputBuffer(uint8_t* src_buf, uint32_t src_size) {
  next_in_ = src_buf;
  avail_in_ = src_size;
  skip_size_ = 0;
}

uint32_t BMPDecompressor::GetAvailInput(uint8_t** avail_buf) {
  if (avail_buf) {
    *avail_buf = nullptr;
    if (avail_in_ > 0)
      *avail_buf = next_in_;
  }
  return avail_in_;
}

void BMPDecompressor::SetHeight(int32_t signed_height) {
  if (signed_height >= 0) {
    height_ = signed_height;
    return;
  }
  if (signed_height == std::numeric_limits<int>::min()) {
    Error();
    NOTREACHED();
  }
  height_ = -signed_height;
  imgTB_flag_ = true;
}
