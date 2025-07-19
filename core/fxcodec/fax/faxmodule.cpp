// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/fax/faxmodule.h"

#include <stdint.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcodec/scanlinedecoder.h"
#include "core/fxcrt/binary_buffer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/calculate_pitch.h"

#if BUILDFLAG(IS_WIN)
#include "core/fxge/dib/cfx_dibbase.h"
#endif

namespace fxcodec {

namespace {

constexpr std::array<const uint8_t, 256> kOneLeadPos = {{
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
}};

// Limit of image dimension. Use the same limit as the JBIG2 codecs.
constexpr int kFaxMaxImageDimension = 65535;

constexpr int kFaxBpc = 1;
constexpr int kFaxComps = 1;

int FindBit(pdfium::span<const uint8_t> data_buf,
            int max_pos,
            int start_pos,
            bool bit) {
  DCHECK(start_pos >= 0);
  if (start_pos >= max_pos) {
    return max_pos;
  }

  const uint8_t bit_xor = bit ? 0x00 : 0xff;
  int bit_offset = start_pos % 8;
  if (bit_offset) {
    const int byte_pos = start_pos / 8;
    uint8_t data = (data_buf[byte_pos] ^ bit_xor) & (0xff >> bit_offset);
    if (data) {
      return byte_pos * 8 + kOneLeadPos[data];
    }
    start_pos += 7;
  }

  const int max_byte = (max_pos + 7) / 8;
  int byte_pos = start_pos / 8;

  // Try reading in bigger chunks in case there are long runs to be skipped.
  static constexpr int kBulkReadSize = 8;
  if (max_byte >= kBulkReadSize && byte_pos < max_byte - kBulkReadSize) {
    static constexpr uint8_t skip_block_0[kBulkReadSize] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static constexpr uint8_t skip_block_1[kBulkReadSize] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    const uint8_t* skip_block = bit ? skip_block_0 : skip_block_1;
    while (byte_pos < max_byte - kBulkReadSize &&
           UNSAFE_TODO(
               memcmp(data_buf.subspan(static_cast<size_t>(byte_pos)).data(),
                      skip_block, kBulkReadSize)) == 0) {
      byte_pos += kBulkReadSize;
    }
  }

  while (byte_pos < max_byte) {
    uint8_t data = data_buf[byte_pos] ^ bit_xor;
    if (data) {
      return std::min(byte_pos * 8 + kOneLeadPos[data], max_pos);
    }
    ++byte_pos;
  }
  return max_pos;
}

void FaxG4FindB1B2(pdfium::span<const uint8_t> ref_buf,
                   int columns,
                   int a0,
                   bool a0color,
                   int* b1,
                   int* b2) {
  bool first_bit = a0 < 0 || (ref_buf[a0 / 8] & (1 << (7 - a0 % 8))) != 0;
  *b1 = FindBit(ref_buf, columns, a0 + 1, !first_bit);
  if (*b1 >= columns) {
    *b1 = *b2 = columns;
    return;
  }
  if (first_bit == !a0color) {
    *b1 = FindBit(ref_buf, columns, *b1 + 1, first_bit);
    first_bit = !first_bit;
  }
  if (*b1 >= columns) {
    *b1 = *b2 = columns;
    return;
  }
  *b2 = FindBit(ref_buf, columns, *b1 + 1, first_bit);
}

void FaxFillBits(uint8_t* dest_buf, int columns, int startpos, int endpos) {
  startpos = std::max(startpos, 0);
  endpos = std::clamp(endpos, 0, columns);
  if (startpos >= endpos) {
    return;
  }
  int first_byte = startpos / 8;
  int last_byte = (endpos - 1) / 8;
  if (first_byte == last_byte) {
    for (int i = startpos % 8; i <= (endpos - 1) % 8; ++i) {
      UNSAFE_TODO(dest_buf[first_byte] -= 1 << (7 - i));
    }
    return;
  }
  for (int i = startpos % 8; i < 8; ++i) {
    UNSAFE_TODO(dest_buf[first_byte] -= 1 << (7 - i));
  }
  for (int i = 0; i <= (endpos - 1) % 8; ++i) {
    UNSAFE_TODO(dest_buf[last_byte] -= 1 << (7 - i));
  }
  if (last_byte > first_byte + 1) {
    UNSAFE_TODO(
        FXSYS_memset(dest_buf + first_byte + 1, 0, last_byte - first_byte - 1));
  }
}

inline bool NextBit(const uint8_t* src_buf, int* bitpos) {
  int pos = (*bitpos)++;
  return !!UNSAFE_TODO((src_buf[pos / 8] & (1 << (7 - pos % 8))));
}

const uint8_t kFaxBlackRunIns[] = {
    0,          2,          0x02,       3,          0,          0x03,
    2,          0,          2,          0x02,       1,          0,
    0x03,       4,          0,          2,          0x02,       6,
    0,          0x03,       5,          0,          1,          0x03,
    7,          0,          2,          0x04,       9,          0,
    0x05,       8,          0,          3,          0x04,       10,
    0,          0x05,       11,         0,          0x07,       12,
    0,          2,          0x04,       13,         0,          0x07,
    14,         0,          1,          0x18,       15,         0,
    5,          0x08,       18,         0,          0x0f,       64,
    0,          0x17,       16,         0,          0x18,       17,
    0,          0x37,       0,          0,          10,         0x08,
    0x00,       0x07,       0x0c,       0x40,       0x07,       0x0d,
    0x80,       0x07,       0x17,       24,         0,          0x18,
    25,         0,          0x28,       23,         0,          0x37,
    22,         0,          0x67,       19,         0,          0x68,
    20,         0,          0x6c,       21,         0,          54,
    0x12,       1984 % 256, 1984 / 256, 0x13,       2048 % 256, 2048 / 256,
    0x14,       2112 % 256, 2112 / 256, 0x15,       2176 % 256, 2176 / 256,
    0x16,       2240 % 256, 2240 / 256, 0x17,       2304 % 256, 2304 / 256,
    0x1c,       2368 % 256, 2368 / 256, 0x1d,       2432 % 256, 2432 / 256,
    0x1e,       2496 % 256, 2496 / 256, 0x1f,       2560 % 256, 2560 / 256,
    0x24,       52,         0,          0x27,       55,         0,
    0x28,       56,         0,          0x2b,       59,         0,
    0x2c,       60,         0,          0x33,       320 % 256,  320 / 256,
    0x34,       384 % 256,  384 / 256,  0x35,       448 % 256,  448 / 256,
    0x37,       53,         0,          0x38,       54,         0,
    0x52,       50,         0,          0x53,       51,         0,
    0x54,       44,         0,          0x55,       45,         0,
    0x56,       46,         0,          0x57,       47,         0,
    0x58,       57,         0,          0x59,       58,         0,
    0x5a,       61,         0,          0x5b,       256 % 256,  256 / 256,
    0x64,       48,         0,          0x65,       49,         0,
    0x66,       62,         0,          0x67,       63,         0,
    0x68,       30,         0,          0x69,       31,         0,
    0x6a,       32,         0,          0x6b,       33,         0,
    0x6c,       40,         0,          0x6d,       41,         0,
    0xc8,       128,        0,          0xc9,       192,        0,
    0xca,       26,         0,          0xcb,       27,         0,
    0xcc,       28,         0,          0xcd,       29,         0,
    0xd2,       34,         0,          0xd3,       35,         0,
    0xd4,       36,         0,          0xd5,       37,         0,
    0xd6,       38,         0,          0xd7,       39,         0,
    0xda,       42,         0,          0xdb,       43,         0,
    20,         0x4a,       640 % 256,  640 / 256,  0x4b,       704 % 256,
    704 / 256,  0x4c,       768 % 256,  768 / 256,  0x4d,       832 % 256,
    832 / 256,  0x52,       1280 % 256, 1280 / 256, 0x53,       1344 % 256,
    1344 / 256, 0x54,       1408 % 256, 1408 / 256, 0x55,       1472 % 256,
    1472 / 256, 0x5a,       1536 % 256, 1536 / 256, 0x5b,       1600 % 256,
    1600 / 256, 0x64,       1664 % 256, 1664 / 256, 0x65,       1728 % 256,
    1728 / 256, 0x6c,       512 % 256,  512 / 256,  0x6d,       576 % 256,
    576 / 256,  0x72,       896 % 256,  896 / 256,  0x73,       960 % 256,
    960 / 256,  0x74,       1024 % 256, 1024 / 256, 0x75,       1088 % 256,
    1088 / 256, 0x76,       1152 % 256, 1152 / 256, 0x77,       1216 % 256,
    1216 / 256, 0xff};

const uint8_t kFaxWhiteRunIns[] = {
    0,          0,          0,          6,          0x07,       2,
    0,          0x08,       3,          0,          0x0B,       4,
    0,          0x0C,       5,          0,          0x0E,       6,
    0,          0x0F,       7,          0,          6,          0x07,
    10,         0,          0x08,       11,         0,          0x12,
    128,        0,          0x13,       8,          0,          0x14,
    9,          0,          0x1b,       64,         0,          9,
    0x03,       13,         0,          0x07,       1,          0,
    0x08,       12,         0,          0x17,       192,        0,
    0x18,       1664 % 256, 1664 / 256, 0x2a,       16,         0,
    0x2B,       17,         0,          0x34,       14,         0,
    0x35,       15,         0,          12,         0x03,       22,
    0,          0x04,       23,         0,          0x08,       20,
    0,          0x0c,       19,         0,          0x13,       26,
    0,          0x17,       21,         0,          0x18,       28,
    0,          0x24,       27,         0,          0x27,       18,
    0,          0x28,       24,         0,          0x2B,       25,
    0,          0x37,       256 % 256,  256 / 256,  42,         0x02,
    29,         0,          0x03,       30,         0,          0x04,
    45,         0,          0x05,       46,         0,          0x0a,
    47,         0,          0x0b,       48,         0,          0x12,
    33,         0,          0x13,       34,         0,          0x14,
    35,         0,          0x15,       36,         0,          0x16,
    37,         0,          0x17,       38,         0,          0x1a,
    31,         0,          0x1b,       32,         0,          0x24,
    53,         0,          0x25,       54,         0,          0x28,
    39,         0,          0x29,       40,         0,          0x2a,
    41,         0,          0x2b,       42,         0,          0x2c,
    43,         0,          0x2d,       44,         0,          0x32,
    61,         0,          0x33,       62,         0,          0x34,
    63,         0,          0x35,       0,          0,          0x36,
    320 % 256,  320 / 256,  0x37,       384 % 256,  384 / 256,  0x4a,
    59,         0,          0x4b,       60,         0,          0x52,
    49,         0,          0x53,       50,         0,          0x54,
    51,         0,          0x55,       52,         0,          0x58,
    55,         0,          0x59,       56,         0,          0x5a,
    57,         0,          0x5b,       58,         0,          0x64,
    448 % 256,  448 / 256,  0x65,       512 % 256,  512 / 256,  0x67,
    640 % 256,  640 / 256,  0x68,       576 % 256,  576 / 256,  16,
    0x98,       1472 % 256, 1472 / 256, 0x99,       1536 % 256, 1536 / 256,
    0x9a,       1600 % 256, 1600 / 256, 0x9b,       1728 % 256, 1728 / 256,
    0xcc,       704 % 256,  704 / 256,  0xcd,       768 % 256,  768 / 256,
    0xd2,       832 % 256,  832 / 256,  0xd3,       896 % 256,  896 / 256,
    0xd4,       960 % 256,  960 / 256,  0xd5,       1024 % 256, 1024 / 256,
    0xd6,       1088 % 256, 1088 / 256, 0xd7,       1152 % 256, 1152 / 256,
    0xd8,       1216 % 256, 1216 / 256, 0xd9,       1280 % 256, 1280 / 256,
    0xda,       1344 % 256, 1344 / 256, 0xdb,       1408 % 256, 1408 / 256,
    0,          3,          0x08,       1792 % 256, 1792 / 256, 0x0c,
    1856 % 256, 1856 / 256, 0x0d,       1920 % 256, 1920 / 256, 10,
    0x12,       1984 % 256, 1984 / 256, 0x13,       2048 % 256, 2048 / 256,
    0x14,       2112 % 256, 2112 / 256, 0x15,       2176 % 256, 2176 / 256,
    0x16,       2240 % 256, 2240 / 256, 0x17,       2304 % 256, 2304 / 256,
    0x1c,       2368 % 256, 2368 / 256, 0x1d,       2432 % 256, 2432 / 256,
    0x1e,       2496 % 256, 2496 / 256, 0x1f,       2560 % 256, 2560 / 256,
    0xff,
};

int FaxGetRun(pdfium::span<const uint8_t> ins_array,
              const uint8_t* src_buf,
              int* bitpos,
              int bitsize) {
  uint32_t code = 0;
  int ins_off = 0;
  while (true) {
    uint8_t ins = ins_array[ins_off++];
    if (ins == 0xff) {
      return -1;
    }

    if (*bitpos >= bitsize) {
      return -1;
    }

    code <<= 1;
    UNSAFE_TODO({
      if (src_buf[*bitpos / 8] & (1 << (7 - *bitpos % 8))) {
        ++code;
      }
    });
    ++(*bitpos);
    int next_off = ins_off + ins * 3;
    for (; ins_off < next_off; ins_off += 3) {
      if (ins_array[ins_off] == code) {
        return ins_array[ins_off + 1] + ins_array[ins_off + 2] * 256;
      }
    }
  }
}

void FaxG4GetRow(const uint8_t* src_buf,
                 int bitsize,
                 int* bitpos,
                 uint8_t* dest_buf,
                 pdfium::span<const uint8_t> ref_buf,
                 int columns) {
  // See TABLE 1/T.6 "Code table" in ITU-T T.6.
  int a0 = -1;
  bool a0color = true;
  while (true) {
    if (*bitpos >= bitsize) {
      return;
    }

    int a1;
    int a2;
    int b1;
    int b2;
    FaxG4FindB1B2(ref_buf, columns, a0, a0color, &b1, &b2);

    int v_delta = 0;
    if (!NextBit(src_buf, bitpos)) {
      if (*bitpos >= bitsize) {
        return;
      }

      bool bit1 = NextBit(src_buf, bitpos);
      if (*bitpos >= bitsize) {
        return;
      }

      bool bit2 = NextBit(src_buf, bitpos);
      if (bit1) {
        // Mode "Vertical", VR(1), VL(1).
        v_delta = bit2 ? 1 : -1;
      } else if (bit2) {
        // Mode "Horizontal".
        int run_len1 = 0;
        while (true) {
          int run = FaxGetRun(
              a0color ? pdfium::span<const uint8_t, pdfium::dynamic_extent>(
                            kFaxWhiteRunIns)
                      : pdfium::span<const uint8_t, pdfium::dynamic_extent>(
                            kFaxBlackRunIns),
              src_buf, bitpos, bitsize);
          run_len1 += run;
          if (run < 64) {
            break;
          }
        }
        if (a0 < 0) {
          ++run_len1;
        }
        if (run_len1 < 0) {
          return;
        }

        a1 = a0 + run_len1;
        if (!a0color) {
          FaxFillBits(dest_buf, columns, a0, a1);
        }

        int run_len2 = 0;
        while (true) {
          int run = FaxGetRun(
              a0color ? pdfium::span<const uint8_t, pdfium::dynamic_extent>(
                            kFaxBlackRunIns)
                      : pdfium::span<const uint8_t, pdfium::dynamic_extent>(
                            kFaxWhiteRunIns),
              src_buf, bitpos, bitsize);
          run_len2 += run;
          if (run < 64) {
            break;
          }
        }
        if (run_len2 < 0) {
          return;
        }
        a2 = a1 + run_len2;
        if (a0color) {
          FaxFillBits(dest_buf, columns, a1, a2);
        }

        a0 = a2;
        if (a0 < columns) {
          continue;
        }

        return;
      } else {
        if (*bitpos >= bitsize) {
          return;
        }

        if (NextBit(src_buf, bitpos)) {
          // Mode "Pass".
          if (!a0color) {
            FaxFillBits(dest_buf, columns, a0, b2);
          }

          if (b2 >= columns) {
            return;
          }

          a0 = b2;
          continue;
        }

        if (*bitpos >= bitsize) {
          return;
        }

        bool next_bit1 = NextBit(src_buf, bitpos);
        if (*bitpos >= bitsize) {
          return;
        }

        bool next_bit2 = NextBit(src_buf, bitpos);
        if (next_bit1) {
          // Mode "Vertical", VR(2), VL(2).
          v_delta = next_bit2 ? 2 : -2;
        } else if (next_bit2) {
          if (*bitpos >= bitsize) {
            return;
          }

          // Mode "Vertical", VR(3), VL(3).
          v_delta = NextBit(src_buf, bitpos) ? 3 : -3;
        } else {
          if (*bitpos >= bitsize) {
            return;
          }

          // Extension
          if (NextBit(src_buf, bitpos)) {
            *bitpos += 3;
            continue;
          }
          *bitpos += 5;
          return;
        }
      }
    } else {
      // Mode "Vertical", V(0).
    }
    a1 = b1 + v_delta;
    if (!a0color) {
      FaxFillBits(dest_buf, columns, a0, a1);
    }

    if (a1 >= columns) {
      return;
    }

    // The position of picture element must be monotonic increasing.
    if (a0 >= a1) {
      return;
    }

    a0 = a1;
    a0color = !a0color;
  }
}

void FaxSkipEOL(const uint8_t* src_buf, int bitsize, int* bitpos) {
  int startbit = *bitpos;
  while (*bitpos < bitsize) {
    if (!NextBit(src_buf, bitpos)) {
      continue;
    }
    if (*bitpos - startbit <= 11) {
      *bitpos = startbit;
    }
    return;
  }
}

void FaxGet1DLine(const uint8_t* src_buf,
                  int bitsize,
                  int* bitpos,
                  uint8_t* dest_buf,
                  int columns) {
  bool color = true;
  int startpos = 0;
  while (true) {
    if (*bitpos >= bitsize) {
      return;
    }

    int run_len = 0;
    while (true) {
      int run =
          FaxGetRun(color ? pdfium::span<const uint8_t, pdfium::dynamic_extent>(
                                kFaxWhiteRunIns)
                          : pdfium::span<const uint8_t, pdfium::dynamic_extent>(
                                kFaxBlackRunIns),
                    src_buf, bitpos, bitsize);
      if (run < 0) {
        while (*bitpos < bitsize) {
          if (NextBit(src_buf, bitpos)) {
            return;
          }
        }
        return;
      }
      run_len += run;
      if (run < 64) {
        break;
      }
    }
    if (!color) {
      FaxFillBits(dest_buf, columns, startpos, startpos + run_len);
    }

    startpos += run_len;
    if (startpos >= columns) {
      break;
    }

    color = !color;
  }
}

class FaxDecoder final : public ScanlineDecoder {
 public:
  FaxDecoder(pdfium::span<const uint8_t> src_span,
             int width,
             int height,
             int K,
             bool EndOfLine,
             bool EncodedByteAlign,
             bool BlackIs1);
  ~FaxDecoder() override;

  // ScanlineDecoder:
  [[nodiscard]] bool Rewind() override;
  pdfium::span<uint8_t> GetNextLine() override;
  uint32_t GetSrcOffset() override;

 private:
  void InvertBuffer();

  const int encoding_;
  int bitpos_ = 0;
  bool byte_align_ = false;
  const bool end_of_line_;
  const bool black_;
  const pdfium::raw_span<const uint8_t> src_span_;
  DataVector<uint8_t> scanline_buf_;
  DataVector<uint8_t> ref_buf_;
};

FaxDecoder::FaxDecoder(pdfium::span<const uint8_t> src_span,
                       int width,
                       int height,
                       int K,
                       bool EndOfLine,
                       bool EncodedByteAlign,
                       bool BlackIs1)
    : ScanlineDecoder(width,
                      height,
                      width,
                      height,
                      kFaxComps,
                      kFaxBpc,
                      fxge::CalculatePitch32OrDie(kFaxBpc, width)),
      encoding_(K),
      byte_align_(EncodedByteAlign),
      end_of_line_(EndOfLine),
      black_(BlackIs1),
      src_span_(src_span),
      scanline_buf_(pitch_),
      ref_buf_(pitch_) {}

FaxDecoder::~FaxDecoder() {
  // Span in superclass can't outlive our buffer.
  last_scanline_ = pdfium::span<uint8_t>();
}

bool FaxDecoder::Rewind() {
  std::ranges::fill(ref_buf_, 0xff);
  bitpos_ = 0;
  return true;
}

pdfium::span<uint8_t> FaxDecoder::GetNextLine() {
  int bitsize = pdfium::checked_cast<int>(src_span_.size() * 8);
  FaxSkipEOL(src_span_.data(), bitsize, &bitpos_);
  if (bitpos_ >= bitsize) {
    return pdfium::span<uint8_t>();
  }

  std::ranges::fill(scanline_buf_, 0xff);
  if (encoding_ < 0) {
    FaxG4GetRow(src_span_.data(), bitsize, &bitpos_, scanline_buf_.data(),
                ref_buf_, orig_width_);
    ref_buf_ = scanline_buf_;
  } else if (encoding_ == 0) {
    FaxGet1DLine(src_span_.data(), bitsize, &bitpos_, scanline_buf_.data(),
                 orig_width_);
  } else {
    if (NextBit(src_span_.data(), &bitpos_)) {
      FaxGet1DLine(src_span_.data(), bitsize, &bitpos_, scanline_buf_.data(),
                   orig_width_);
    } else {
      FaxG4GetRow(src_span_.data(), bitsize, &bitpos_, scanline_buf_.data(),
                  ref_buf_, orig_width_);
    }
    ref_buf_ = scanline_buf_;
  }
  if (end_of_line_) {
    FaxSkipEOL(src_span_.data(), bitsize, &bitpos_);
  }

  if (byte_align_ && bitpos_ < bitsize) {
    int bitpos0 = bitpos_;
    int bitpos1 = FxAlignToBoundary<8>(bitpos_);
    while (byte_align_ && bitpos0 < bitpos1) {
      int bit = src_span_[bitpos0 / 8] & (1 << (7 - bitpos0 % 8));
      if (bit != 0) {
        byte_align_ = false;
      } else {
        ++bitpos0;
      }
    }
    if (byte_align_) {
      bitpos_ = bitpos1;
    }
  }
  if (black_) {
    InvertBuffer();
  }
  return scanline_buf_;
}

uint32_t FaxDecoder::GetSrcOffset() {
  return pdfium::checked_cast<uint32_t>(
      std::min<size_t>((bitpos_ + 7) / 8, src_span_.size()));
}

void FaxDecoder::InvertBuffer() {
  auto byte_span = pdfium::span(scanline_buf_);
  auto data = fxcrt::reinterpret_span<uint32_t>(byte_span);
  for (auto& datum : data) {
    datum = ~datum;
  }
}

}  // namespace

// static
std::unique_ptr<ScanlineDecoder> FaxModule::CreateDecoder(
    pdfium::span<const uint8_t> src_span,
    int width,
    int height,
    int K,
    bool EndOfLine,
    bool EncodedByteAlign,
    bool BlackIs1,
    int Columns,
    int Rows) {
  int actual_width = Columns ? Columns : width;
  int actual_height = Rows ? Rows : height;

  // Reject invalid values.
  if (actual_width <= 0 || actual_height <= 0) {
    return nullptr;
  }

  // Reject unreasonable large input.
  if (actual_width > kFaxMaxImageDimension ||
      actual_height > kFaxMaxImageDimension) {
    return nullptr;
  }

  return std::make_unique<FaxDecoder>(src_span, actual_width, actual_height, K,
                                      EndOfLine, EncodedByteAlign, BlackIs1);
}

// static
int FaxModule::FaxG4Decode(pdfium::span<const uint8_t> src_span,
                           int starting_bitpos,
                           int width,
                           int height,
                           int pitch,
                           uint8_t* dest_buf) {
  DCHECK(pitch != 0);

  const uint8_t* src_buf = src_span.data();
  uint32_t src_size = pdfium::checked_cast<uint32_t>(src_span.size());

  DataVector<uint8_t> ref_buf(pitch, 0xff);
  int bitpos = starting_bitpos;
  for (int iRow = 0; iRow < height; ++iRow) {
    uint8_t* line_buf = UNSAFE_TODO(dest_buf + iRow * pitch);
    UNSAFE_TODO(FXSYS_memset(line_buf, 0xff, pitch));
    FaxG4GetRow(src_buf, src_size << 3, &bitpos, line_buf, ref_buf, width);
    UNSAFE_TODO(FXSYS_memcpy(ref_buf.data(), line_buf, pitch));
  }
  return bitpos;
}

#if BUILDFLAG(IS_WIN)
namespace {
const uint8_t BlackRunTerminator[128] = {
    0x37, 10, 0x02, 3,  0x03, 2,  0x02, 2,  0x03, 3,  0x03, 4,  0x02, 4,
    0x03, 5,  0x05, 6,  0x04, 6,  0x04, 7,  0x05, 7,  0x07, 7,  0x04, 8,
    0x07, 8,  0x18, 9,  0x17, 10, 0x18, 10, 0x08, 10, 0x67, 11, 0x68, 11,
    0x6c, 11, 0x37, 11, 0x28, 11, 0x17, 11, 0x18, 11, 0xca, 12, 0xcb, 12,
    0xcc, 12, 0xcd, 12, 0x68, 12, 0x69, 12, 0x6a, 12, 0x6b, 12, 0xd2, 12,
    0xd3, 12, 0xd4, 12, 0xd5, 12, 0xd6, 12, 0xd7, 12, 0x6c, 12, 0x6d, 12,
    0xda, 12, 0xdb, 12, 0x54, 12, 0x55, 12, 0x56, 12, 0x57, 12, 0x64, 12,
    0x65, 12, 0x52, 12, 0x53, 12, 0x24, 12, 0x37, 12, 0x38, 12, 0x27, 12,
    0x28, 12, 0x58, 12, 0x59, 12, 0x2b, 12, 0x2c, 12, 0x5a, 12, 0x66, 12,
    0x67, 12,
};

const uint8_t BlackRunMarkup[80] = {
    0x0f, 10, 0xc8, 12, 0xc9, 12, 0x5b, 12, 0x33, 12, 0x34, 12, 0x35, 12,
    0x6c, 13, 0x6d, 13, 0x4a, 13, 0x4b, 13, 0x4c, 13, 0x4d, 13, 0x72, 13,
    0x73, 13, 0x74, 13, 0x75, 13, 0x76, 13, 0x77, 13, 0x52, 13, 0x53, 13,
    0x54, 13, 0x55, 13, 0x5a, 13, 0x5b, 13, 0x64, 13, 0x65, 13, 0x08, 11,
    0x0c, 11, 0x0d, 11, 0x12, 12, 0x13, 12, 0x14, 12, 0x15, 12, 0x16, 12,
    0x17, 12, 0x1c, 12, 0x1d, 12, 0x1e, 12, 0x1f, 12,
};

const uint8_t WhiteRunTerminator[128] = {
    0x35, 8, 0x07, 6, 0x07, 4, 0x08, 4, 0x0B, 4, 0x0C, 4, 0x0E, 4, 0x0F, 4,
    0x13, 5, 0x14, 5, 0x07, 5, 0x08, 5, 0x08, 6, 0x03, 6, 0x34, 6, 0x35, 6,
    0x2a, 6, 0x2B, 6, 0x27, 7, 0x0c, 7, 0x08, 7, 0x17, 7, 0x03, 7, 0x04, 7,
    0x28, 7, 0x2B, 7, 0x13, 7, 0x24, 7, 0x18, 7, 0x02, 8, 0x03, 8, 0x1a, 8,
    0x1b, 8, 0x12, 8, 0x13, 8, 0x14, 8, 0x15, 8, 0x16, 8, 0x17, 8, 0x28, 8,
    0x29, 8, 0x2a, 8, 0x2b, 8, 0x2c, 8, 0x2d, 8, 0x04, 8, 0x05, 8, 0x0a, 8,
    0x0b, 8, 0x52, 8, 0x53, 8, 0x54, 8, 0x55, 8, 0x24, 8, 0x25, 8, 0x58, 8,
    0x59, 8, 0x5a, 8, 0x5b, 8, 0x4a, 8, 0x4b, 8, 0x32, 8, 0x33, 8, 0x34, 8,
};

const uint8_t WhiteRunMarkup[80] = {
    0x1b, 5,  0x12, 5,  0x17, 6,  0x37, 7,  0x36, 8,  0x37, 8,  0x64, 8,
    0x65, 8,  0x68, 8,  0x67, 8,  0xcc, 9,  0xcd, 9,  0xd2, 9,  0xd3, 9,
    0xd4, 9,  0xd5, 9,  0xd6, 9,  0xd7, 9,  0xd8, 9,  0xd9, 9,  0xda, 9,
    0xdb, 9,  0x98, 9,  0x99, 9,  0x9a, 9,  0x18, 6,  0x9b, 9,  0x08, 11,
    0x0c, 11, 0x0d, 11, 0x12, 12, 0x13, 12, 0x14, 12, 0x15, 12, 0x16, 12,
    0x17, 12, 0x1c, 12, 0x1d, 12, 0x1e, 12, 0x1f, 12,
};

class FaxEncoder {
 public:
  explicit FaxEncoder(RetainPtr<const CFX_DIBBase> src);
  ~FaxEncoder();
  DataVector<uint8_t> Encode();

 private:
  void FaxEncode2DLine(pdfium::span<const uint8_t> src_span);
  void FaxEncodeRun(int run, bool bWhite);
  void AddBitStream(int data, int bitlen);

  // Must outlive `ref_line_span_`.
  RetainPtr<const CFX_DIBBase> const src_;
  int dest_bitpos_ = 0;
  const int cols_;
  const int rows_;
  const int pitch_;
  BinaryBuffer dest_buf_;
  // Must outlive `ref_line_span_`.
  const DataVector<uint8_t> initial_ref_line_;
  DataVector<uint8_t> line_buf_;
  pdfium::raw_span<const uint8_t> ref_line_span_;
};

FaxEncoder::FaxEncoder(RetainPtr<const CFX_DIBBase> src)
    : src_(std::move(src)),
      cols_(src_->GetWidth()),
      rows_(src_->GetHeight()),
      pitch_(src_->GetPitch()),
      initial_ref_line_(pitch_, 0xff),
      line_buf_(Fx2DSizeOrDie(8, pitch_)),
      ref_line_span_(initial_ref_line_) {
  DCHECK_EQ(1, src_->GetBPP());
  dest_buf_.SetAllocStep(10240);
}

FaxEncoder::~FaxEncoder() = default;

void FaxEncoder::AddBitStream(int data, int bitlen) {
  for (int i = bitlen - 1; i >= 0; --i, ++dest_bitpos_) {
    if (data & (1 << i)) {
      line_buf_[dest_bitpos_ / 8] |= 1 << (7 - dest_bitpos_ % 8);
    }
  }
}

void FaxEncoder::FaxEncodeRun(int run, bool bWhite) {
  while (run >= 2560) {
    AddBitStream(0x1f, 12);
    run -= 2560;
  }
  UNSAFE_TODO({
    if (run >= 64) {
      int markup = run - run % 64;
      const uint8_t* p = bWhite ? WhiteRunMarkup : BlackRunMarkup;
      p += (markup / 64 - 1) * 2;
      AddBitStream(*p, p[1]);
    }
    run %= 64;
    const uint8_t* p = bWhite ? WhiteRunTerminator : BlackRunTerminator;
    p += run * 2;
    AddBitStream(*p, p[1]);
  });
}

void FaxEncoder::FaxEncode2DLine(pdfium::span<const uint8_t> src_span) {
  int a0 = -1;
  bool a0color = true;
  while (1) {
    int a1 = FindBit(src_span, cols_, a0 + 1, !a0color);
    int b1;
    int b2;
    FaxG4FindB1B2(ref_line_span_, cols_, a0, a0color, &b1, &b2);
    if (b2 < a1) {
      dest_bitpos_ += 3;
      line_buf_[dest_bitpos_ / 8] |= 1 << (7 - dest_bitpos_ % 8);
      ++dest_bitpos_;
      a0 = b2;
    } else if (a1 - b1 <= 3 && b1 - a1 <= 3) {
      int delta = a1 - b1;
      switch (delta) {
        case 0:
          line_buf_[dest_bitpos_ / 8] |= 1 << (7 - dest_bitpos_ % 8);
          break;
        case 1:
        case 2:
        case 3:
          dest_bitpos_ += delta == 1 ? 1 : delta + 2;
          line_buf_[dest_bitpos_ / 8] |= 1 << (7 - dest_bitpos_ % 8);
          ++dest_bitpos_;
          line_buf_[dest_bitpos_ / 8] |= 1 << (7 - dest_bitpos_ % 8);
          break;
        case -1:
        case -2:
        case -3:
          dest_bitpos_ += delta == -1 ? 1 : -delta + 2;
          line_buf_[dest_bitpos_ / 8] |= 1 << (7 - dest_bitpos_ % 8);
          ++dest_bitpos_;
          break;
      }
      ++dest_bitpos_;
      a0 = a1;
      a0color = !a0color;
    } else {
      int a2 = FindBit(src_span, cols_, a1 + 1, a0color);
      ++dest_bitpos_;
      ++dest_bitpos_;
      line_buf_[dest_bitpos_ / 8] |= 1 << (7 - dest_bitpos_ % 8);
      ++dest_bitpos_;
      if (a0 < 0) {
        a0 = 0;
      }
      FaxEncodeRun(a1 - a0, a0color);
      FaxEncodeRun(a2 - a1, !a0color);
      a0 = a2;
    }
    if (a0 >= cols_) {
      return;
    }
  }
}

DataVector<uint8_t> FaxEncoder::Encode() {
  dest_bitpos_ = 0;
  uint8_t last_byte = 0;
  for (int i = 0; i < rows_; ++i) {
    std::ranges::fill(line_buf_, 0);
    line_buf_[0] = last_byte;
    pdfium::span<const uint8_t> scan_line = src_->GetScanline(i);
    FaxEncode2DLine(scan_line);
    dest_buf_.AppendSpan(
        pdfium::span(line_buf_).first(static_cast<size_t>(dest_bitpos_ / 8)));
    last_byte = line_buf_[dest_bitpos_ / 8];
    dest_bitpos_ %= 8;
    ref_line_span_ = scan_line;
  }
  if (dest_bitpos_) {
    dest_buf_.AppendUint8(last_byte);
  }
  return dest_buf_.DetachBuffer();
}

}  // namespace

// static
DataVector<uint8_t> FaxModule::FaxEncode(RetainPtr<const CFX_DIBBase> src) {
  DCHECK_EQ(1, src->GetBPP());
  FaxEncoder encoder(std::move(src));
  return encoder.Encode();
}

#endif  // BUILDFLAG(IS_WIN)

}  // namespace fxcodec
