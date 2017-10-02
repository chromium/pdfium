// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lbmp/fx_bmp.h"

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
    : context_ptr(nullptr),
      next_in(nullptr),
      header_offset(0),
      width(0),
      height(0),
      compress_flag(0),
      components(0),
      src_row_bytes(0),
      out_row_bytes(0),
      bitCounts(0),
      color_used(0),
      imgTB_flag(false),
      pal_num(0),
      pal_type(0),
      data_size(0),
      img_data_offset(0),
      img_ifh_size(0),
      row_num(0),
      col_num(0),
      dpi_x(0),
      dpi_y(0),
      mask_red(0),
      mask_green(0),
      mask_blue(0),
      avail_in(0),
      skip_size(0),
      decode_status(BMP_D_STATUS_HEADER) {}

BMPDecompressor::~BMPDecompressor() {}

void BMPDecompressor::Error() {
  longjmp(jmpbuf, 1);
}

void BMPDecompressor::ReadScanline(uint32_t row_num,
                                   const std::vector<uint8_t>& row_buf) {
  auto* p = reinterpret_cast<CBmpContext*>(context_ptr);
  p->m_pDelegate->BmpReadScanline(row_num, row_buf);
}

bool BMPDecompressor::GetDataPosition(uint32_t rcd_pos) {
  auto* p = reinterpret_cast<CBmpContext*>(context_ptr);
  return p->m_pDelegate->BmpInputImagePositionBuf(rcd_pos);
}

int32_t BMPDecompressor::ReadHeader() {
  uint32_t skip_size_org = skip_size;
  if (decode_status == BMP_D_STATUS_HEADER) {
    BmpFileHeader* pBmp_header = nullptr;
    if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_header),
                  sizeof(BmpFileHeader))) {
      return 2;
    }

    pBmp_header->bfType =
        FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&pBmp_header->bfType));
    pBmp_header->bfOffBits = FXDWORD_GET_LSBFIRST(
        reinterpret_cast<uint8_t*>(&pBmp_header->bfOffBits));
    data_size =
        FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&pBmp_header->bfSize));
    if (pBmp_header->bfType != BMP_SIGNATURE) {
      Error();
      NOTREACHED();
    }
    if (avail_in < sizeof(uint32_t)) {
      skip_size = skip_size_org;
      return 2;
    }
    img_ifh_size =
        FXDWORD_GET_LSBFIRST(static_cast<uint8_t*>(next_in + skip_size));
    pal_type = 0;
    static_assert(sizeof(BmpCoreHeader) == kBmpCoreHeaderSize,
                  "BmpCoreHeader has wrong size");
    static_assert(sizeof(BmpInfoHeader) == kBmpInfoHeaderSize,
                  "BmpInfoHeader has wrong size");
    switch (img_ifh_size) {
      case kBmpCoreHeaderSize: {
        pal_type = 1;
        BmpCoreHeader* pBmp_core_header = nullptr;
        if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_core_header),
                      img_ifh_size)) {
          skip_size = skip_size_org;
          return 2;
        }
        width = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_core_header->bcWidth));
        height = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_core_header->bcHeight));
        bitCounts = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_core_header->bcBitCount));
        compress_flag = BMP_RGB;
        imgTB_flag = false;
      } break;
      case kBmpInfoHeaderSize: {
        BmpInfoHeader* pBmp_info_header = nullptr;
        if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_info_header),
                      img_ifh_size)) {
          skip_size = skip_size_org;
          return 2;
        }
        width = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biWidth));
        int32_t signed_height = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biHeight));
        bitCounts = FXWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biBitCount));
        compress_flag = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biCompression));
        color_used = FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biClrUsed));
        dpi_x = static_cast<int32_t>(FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biXPelsPerMeter)));
        dpi_y = static_cast<int32_t>(FXDWORD_GET_LSBFIRST(
            reinterpret_cast<uint8_t*>(&pBmp_info_header->biYPelsPerMeter)));
        SetHeight(signed_height);
      } break;
      default: {
        if (img_ifh_size >
            std::min(kBmpInfoHeaderSize, sizeof(BmpInfoHeader))) {
          BmpInfoHeader* pBmp_info_header = nullptr;
          if (!ReadData(reinterpret_cast<uint8_t**>(&pBmp_info_header),
                        img_ifh_size)) {
            skip_size = skip_size_org;
            return 2;
          }
          uint16_t biPlanes;
          width = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biWidth));
          int32_t signed_height = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biHeight));
          bitCounts = FXWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biBitCount));
          compress_flag = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biCompression));
          color_used = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biClrUsed));
          biPlanes = FXWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biPlanes));
          dpi_x = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biXPelsPerMeter));
          dpi_y = FXDWORD_GET_LSBFIRST(
              reinterpret_cast<uint8_t*>(&pBmp_info_header->biYPelsPerMeter));
          SetHeight(signed_height);
          if (compress_flag == BMP_RGB && biPlanes == 1 && color_used == 0)
            break;
        }
        Error();
        NOTREACHED();
      }
    }
    if (width > BMP_MAX_WIDTH || compress_flag > BMP_BITFIELDS) {
      Error();
      NOTREACHED();
    }
    switch (bitCounts) {
      case 1:
      case 4:
      case 8:
      case 16:
      case 24: {
        if (color_used > 1U << bitCounts) {
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
    src_row_bytes = BMP_WIDTHBYTES(width, bitCounts);
    switch (bitCounts) {
      case 1:
      case 4:
      case 8:
        out_row_bytes = BMP_WIDTHBYTES(width, 8);
        components = 1;
        break;
      case 16:
      case 24:
        out_row_bytes = BMP_WIDTHBYTES(width, 24);
        components = 3;
        break;
      case 32:
        out_row_bytes = src_row_bytes;
        components = 4;
        break;
    }
    out_row_buffer.clear();

    if (out_row_bytes <= 0) {
      Error();
      NOTREACHED();
    }

    out_row_buffer.resize(out_row_bytes);
    SaveDecodingStatus(BMP_D_STATUS_PAL);
  }
  if (decode_status == BMP_D_STATUS_PAL) {
    skip_size_org = skip_size;
    if (compress_flag == BMP_BITFIELDS) {
      if (bitCounts != 16 && bitCounts != 32) {
        Error();
        NOTREACHED();
      }
      uint32_t* mask;
      if (ReadData(reinterpret_cast<uint8_t**>(&mask), 3 * sizeof(uint32_t)) ==
          nullptr) {
        skip_size = skip_size_org;
        return 2;
      }
      mask_red = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&mask[0]));
      mask_green = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&mask[1]));
      mask_blue = FXDWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&mask[2]));
      if (mask_red & mask_green || mask_red & mask_blue ||
          mask_green & mask_blue) {
        Error();
        NOTREACHED();
      }
      header_offset = std::max(header_offset, 26 + img_ifh_size);
      SaveDecodingStatus(BMP_D_STATUS_DATA_PRE);
      return 1;
    } else if (bitCounts == 16) {
      mask_red = 0x7C00;
      mask_green = 0x03E0;
      mask_blue = 0x001F;
    }
    pal_num = 0;
    if (bitCounts < 16) {
      pal_num = 1 << bitCounts;
      if (color_used != 0)
        pal_num = color_used;
      uint8_t* src_pal_ptr = nullptr;
      uint32_t src_pal_size = pal_num * (pal_type ? 3 : 4);
      if (ReadData(&src_pal_ptr, src_pal_size) == nullptr) {
        skip_size = skip_size_org;
        return 2;
      }
      palette.resize(pal_num);
      int32_t src_pal_index = 0;
      if (pal_type == BMP_PAL_OLD) {
        while (src_pal_index < pal_num) {
          palette[src_pal_index++] = BMP_PAL_ENCODE(
              0x00, src_pal_ptr[2], src_pal_ptr[1], src_pal_ptr[0]);
          src_pal_ptr += 3;
        }
      } else {
        while (src_pal_index < pal_num) {
          palette[src_pal_index++] = BMP_PAL_ENCODE(
              src_pal_ptr[3], src_pal_ptr[2], src_pal_ptr[1], src_pal_ptr[0]);
          src_pal_ptr += 4;
        }
      }
    }
    header_offset = std::max(header_offset,
                             14 + img_ifh_size + pal_num * (pal_type ? 3 : 4));
    SaveDecodingStatus(BMP_D_STATUS_DATA_PRE);
  }
  return 1;
}

bool BMPDecompressor::ValidateFlag() const {
  switch (compress_flag) {
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
  if (decode_status == BMP_D_STATUS_DATA_PRE) {
    avail_in = 0;
    if (!GetDataPosition(header_offset)) {
      decode_status = BMP_D_STATUS_TAIL;
      Error();
      NOTREACHED();
    }
    row_num = 0;
    SaveDecodingStatus(BMP_D_STATUS_DATA);
  }
  if (decode_status != BMP_D_STATUS_DATA || !ValidateFlag()) {
    Error();
    NOTREACHED();
  }
  switch (compress_flag) {
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
  if (val >= pal_num) {
    Error();
    NOTREACHED();
  }
  return true;
}

int32_t BMPDecompressor::DecodeRGB() {
  uint8_t* des_buf = nullptr;
  while (row_num < height) {
    size_t idx = 0;
    if (!ReadData(&des_buf, src_row_bytes))
      return 2;

    SaveDecodingStatus(BMP_D_STATUS_DATA);
    switch (bitCounts) {
      case 1: {
        for (uint32_t col = 0; col < width; ++col)
          out_row_buffer[idx++] =
              des_buf[col >> 3] & (0x80 >> (col % 8)) ? 0x01 : 0x00;
      } break;
      case 4: {
        for (uint32_t col = 0; col < width; ++col) {
          out_row_buffer[idx++] = (col & 0x01)
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
          if ((mask_blue >> i) & 0x01)
            blue_bits++;
          if ((mask_green >> i) & 0x01)
            green_bits++;
          if ((mask_red >> i) & 0x01)
            red_bits++;
        }
        green_bits += blue_bits;
        red_bits += green_bits;
        if (blue_bits > 8 || green_bits < 8 || red_bits < 8)
          return 2;
        blue_bits = 8 - blue_bits;
        green_bits -= 8;
        red_bits -= 8;
        for (uint32_t col = 0; col < width; ++col) {
          *buf = FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(buf));
          out_row_buffer[idx++] =
              static_cast<uint8_t>((*buf & mask_blue) << blue_bits);
          out_row_buffer[idx++] =
              static_cast<uint8_t>((*buf & mask_green) >> green_bits);
          out_row_buffer[idx++] =
              static_cast<uint8_t>((*buf++ & mask_red) >> red_bits);
        }
      } break;
      case 8:
      case 24:
      case 32:
        std::copy(des_buf, des_buf + src_row_bytes, out_row_buffer.begin());
        idx += src_row_bytes;
        break;
    }
    for (uint8_t byte : out_row_buffer) {
      if (!ValidateColorIndex(byte))
        return 0;
    }
    ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                 out_row_buffer);
  }
  SaveDecodingStatus(BMP_D_STATUS_TAIL);
  return 1;
}

int32_t BMPDecompressor::DecodeRLE8() {
  uint8_t* first_byte_ptr = nullptr;
  uint8_t* second_byte_ptr = nullptr;
  col_num = 0;
  while (true) {
    uint32_t skip_size_org = skip_size;
    if (!ReadData(&first_byte_ptr, 1))
      return 2;

    switch (*first_byte_ptr) {
      case RLE_MARKER: {
        if (!ReadData(&first_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        switch (*first_byte_ptr) {
          case RLE_EOL: {
            if (row_num >= height) {
              SaveDecodingStatus(BMP_D_STATUS_TAIL);
              Error();
              NOTREACHED();
            }
            ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                         out_row_buffer);
            col_num = 0;
            std::fill(out_row_buffer.begin(), out_row_buffer.end(), 0);
            SaveDecodingStatus(BMP_D_STATUS_DATA);
            continue;
          }
          case RLE_EOI: {
            if (row_num < height) {
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
            SaveDecodingStatus(BMP_D_STATUS_TAIL);
            return 1;
          }
          case RLE_DELTA: {
            uint8_t* delta_ptr;
            if (!ReadData(&delta_ptr, 2)) {
              skip_size = skip_size_org;
              return 2;
            }
            col_num += delta_ptr[0];
            size_t bmp_row_num_next = row_num + delta_ptr[1];
            if (col_num >= out_row_bytes || bmp_row_num_next >= height) {
              Error();
              NOTREACHED();
            }
            while (row_num < bmp_row_num_next) {
              std::fill(out_row_buffer.begin(), out_row_buffer.end(), 0);
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
          } break;
          default: {
            int32_t avail_size = out_row_bytes - col_num;
            if (!avail_size ||
                static_cast<int32_t>(*first_byte_ptr) > avail_size) {
              Error();
              NOTREACHED();
            }
            if (!ReadData(&second_byte_ptr, *first_byte_ptr & 1
                                                ? *first_byte_ptr + 1
                                                : *first_byte_ptr)) {
              skip_size = skip_size_org;
              return 2;
            }
            std::copy(second_byte_ptr, second_byte_ptr + *first_byte_ptr,
                      out_row_buffer.begin() + col_num);
            for (size_t i = col_num; i < col_num + *first_byte_ptr; ++i) {
              if (!ValidateColorIndex(out_row_buffer[i]))
                return 0;
            }
            col_num += *first_byte_ptr;
          }
        }
      } break;
      default: {
        int32_t avail_size = out_row_bytes - col_num;
        if (!avail_size || static_cast<int32_t>(*first_byte_ptr) > avail_size) {
          Error();
          NOTREACHED();
        }
        if (!ReadData(&second_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        std::fill(out_row_buffer.begin() + col_num,
                  out_row_buffer.begin() + col_num + *first_byte_ptr,
                  *second_byte_ptr);
        if (!ValidateColorIndex(out_row_buffer[col_num]))
          return 0;
        col_num += *first_byte_ptr;
      }
    }
  }
  Error();
  NOTREACHED();
}

int32_t BMPDecompressor::DecodeRLE4() {
  uint8_t* first_byte_ptr = nullptr;
  uint8_t* second_byte_ptr = nullptr;
  col_num = 0;
  while (true) {
    uint32_t skip_size_org = skip_size;
    if (!ReadData(&first_byte_ptr, 1))
      return 2;

    switch (*first_byte_ptr) {
      case RLE_MARKER: {
        if (!ReadData(&first_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        switch (*first_byte_ptr) {
          case RLE_EOL: {
            if (row_num >= height) {
              SaveDecodingStatus(BMP_D_STATUS_TAIL);
              Error();
              NOTREACHED();
            }
            ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                         out_row_buffer);
            col_num = 0;
            std::fill(out_row_buffer.begin(), out_row_buffer.end(), 0);
            SaveDecodingStatus(BMP_D_STATUS_DATA);
            continue;
          }
          case RLE_EOI: {
            if (row_num < height) {
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
            SaveDecodingStatus(BMP_D_STATUS_TAIL);
            return 1;
          }
          case RLE_DELTA: {
            uint8_t* delta_ptr;
            if (!ReadData(&delta_ptr, 2)) {
              skip_size = skip_size_org;
              return 2;
            }
            col_num += delta_ptr[0];
            size_t bmp_row_num_next = row_num + delta_ptr[1];
            if (col_num >= out_row_bytes || bmp_row_num_next >= height) {
              Error();
              NOTREACHED();
            }
            while (row_num < bmp_row_num_next) {
              std::fill(out_row_buffer.begin(), out_row_buffer.end(), 0);
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
          } break;
          default: {
            int32_t avail_size = out_row_bytes - col_num;
            if (!avail_size) {
              Error();
              NOTREACHED();
            }
            uint8_t size = HalfRoundUp(*first_byte_ptr);
            if (static_cast<int32_t>(*first_byte_ptr) > avail_size) {
              if (size + (col_num >> 1) > src_row_bytes) {
                Error();
                NOTREACHED();
              }
              *first_byte_ptr = avail_size - 1;
            }
            if (!ReadData(&second_byte_ptr, size & 1 ? size + 1 : size)) {
              skip_size = skip_size_org;
              return 2;
            }
            for (uint8_t i = 0; i < *first_byte_ptr; i++) {
              uint8_t color = (i & 0x01) ? (*second_byte_ptr++ & 0x0F)
                                         : (*second_byte_ptr & 0xF0) >> 4;
              if (!ValidateColorIndex(color))
                return 0;

              out_row_buffer[col_num++] = color;
            }
          }
        }
      } break;
      default: {
        int32_t avail_size = out_row_bytes - col_num;
        if (!avail_size) {
          Error();
          NOTREACHED();
        }
        if (static_cast<int32_t>(*first_byte_ptr) > avail_size) {
          uint8_t size = HalfRoundUp(*first_byte_ptr);
          if (size + (col_num >> 1) > src_row_bytes) {
            Error();
            NOTREACHED();
          }
          *first_byte_ptr = avail_size - 1;
        }
        if (!ReadData(&second_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        for (uint8_t i = 0; i < *first_byte_ptr; i++) {
          uint8_t second_byte = *second_byte_ptr;
          second_byte =
              i & 0x01 ? (second_byte & 0x0F) : (second_byte & 0xF0) >> 4;
          if (!ValidateColorIndex(second_byte))
            return 0;
          out_row_buffer[col_num++] = second_byte;
        }
      }
    }
  }
  Error();
  NOTREACHED();
}

uint8_t* BMPDecompressor::ReadData(uint8_t** des_buf, uint32_t data_size) {
  if (avail_in < skip_size + data_size)
    return nullptr;

  *des_buf = next_in + skip_size;
  skip_size += data_size;
  return *des_buf;
}

void BMPDecompressor::SaveDecodingStatus(int32_t status) {
  decode_status = status;
  next_in += skip_size;
  avail_in -= skip_size;
  skip_size = 0;
}

void BMPDecompressor::SetInputBuffer(uint8_t* src_buf, uint32_t src_size) {
  next_in = src_buf;
  avail_in = src_size;
  skip_size = 0;
}

uint32_t BMPDecompressor::GetAvailInput(uint8_t** avail_buf) {
  if (avail_buf) {
    *avail_buf = nullptr;
    if (avail_in > 0)
      *avail_buf = next_in;
  }
  return avail_in;
}

void BMPDecompressor::SetHeight(int32_t signed_height) {
  if (signed_height >= 0) {
    height = signed_height;
    return;
  }
  if (signed_height == std::numeric_limits<int>::min()) {
    Error();
    NOTREACHED();
  }
  height = -signed_height;
  imgTB_flag = true;
}
