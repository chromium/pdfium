// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxcodec/fx_codec.h"
#include "codec_int.h"
const FX_BYTE OneLeadPos[256] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
const FX_BYTE ZeroLeadPos[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8,
};

int _FindBit(const FX_BYTE* data_buf, int max_pos, int start_pos, int bit)
{
    if (start_pos >= max_pos) {
        return max_pos;
    }
    FX_LPCBYTE leading_pos = bit ? OneLeadPos : ZeroLeadPos;
    if (start_pos % 8) {
        FX_BYTE data = data_buf[start_pos / 8];
        if (bit) {
            data &= 0xff >> (start_pos % 8);
        } else {
            data |= 0xff << (8 - start_pos % 8);
        }
        if (leading_pos[data] < 8) {
            return start_pos / 8 * 8 + leading_pos[data];
        }
        start_pos += 7;
    }
    FX_BYTE skip = bit ? 0x00 : 0xff;
    int byte_pos = start_pos / 8;
    int max_byte = (max_pos + 7) / 8;
    while (byte_pos < max_byte) {
        if (data_buf[byte_pos] != skip) {
            break;
        }
        byte_pos ++;
    }
    if (byte_pos == max_byte) {
        return max_pos;
    }
    int pos = leading_pos[data_buf[byte_pos]] + byte_pos * 8;
    if (pos > max_pos) {
        pos = max_pos;
    }
    return pos;
}
void _FaxG4FindB1B2(const FX_BYTE* ref_buf, int columns, int a0, FX_BOOL a0color, int& b1, int& b2)
{
    if (a0color) {
        a0color = 1;
    }
    FX_BYTE first_bit = (a0 < 0) ? 1 : ((ref_buf[a0 / 8] & (1 << (7 - a0 % 8))) != 0);
    b1 = _FindBit(ref_buf, columns, a0 + 1, !first_bit);
    if (b1 >= columns) {
        b1 = b2 = columns;
        return;
    }
    if (first_bit == !a0color) {
        b1 = _FindBit(ref_buf, columns, b1 + 1, first_bit);
        first_bit = !first_bit;
    }
    if (b1 >= columns) {
        b1 = b2 = columns;
        return;
    }
    b2 = _FindBit(ref_buf, columns, b1 + 1, first_bit);
}
void _FaxFillBits(FX_LPBYTE dest_buf, int columns, int startpos, int endpos)
{
    if (startpos < 0) {
        startpos = 0;
    }
    if (endpos < 0) {
        endpos = 0;
    }
    if (endpos >= columns) {
        endpos = columns;
    }
    if (startpos >= endpos) {
        return;
    }
    int first_byte = startpos / 8;
    int last_byte = (endpos - 1) / 8;
    if (first_byte == last_byte) {
        for (int i = startpos % 8; i <= (endpos - 1) % 8; i ++) {
            dest_buf[first_byte] -= 1 << (7 - i);
        }
        return;
    }
    int i;
    for (i = startpos % 8; i < 8; i ++) {
        dest_buf[first_byte] -= 1 << (7 - i);
    }
    for (i = 0; i <= (endpos - 1) % 8; i ++) {
        dest_buf[last_byte] -= 1 << (7 - i);
    }
    if (last_byte > first_byte + 1) {
        FXSYS_memset32(dest_buf + first_byte + 1, 0, last_byte - first_byte - 1);
    }
}
#define NEXTBIT src_buf[bitpos/8] & (1 << (7-bitpos%8)); bitpos ++;
#define ADDBIT(code, bit) code = code << 1; if (bit) code ++;
#define GETBIT(bitpos) src_buf[bitpos/8] & (1 << (7-bitpos%8))
static const FX_BYTE FaxBlackRunIns[] = {
    0,
    2,
    0x02, 3, 0,
    0x03, 2, 0,
    2,
    0x02, 1, 0,
    0x03, 4, 0,
    2,
    0x02, 6, 0,
    0x03, 5, 0,
    1,
    0x03, 7, 0,
    2,
    0x04, 9, 0,
    0x05, 8, 0,
    3,
    0x04, 10, 0,
    0x05, 11, 0,
    0x07, 12, 0,
    2,
    0x04, 13, 0,
    0x07, 14, 0,
    1,
    0x18, 15, 0,
    5,
    0x08, 18, 0,
    0x0f, 64, 0,
    0x17, 16, 0,
    0x18, 17, 0,
    0x37, 0, 0,
    10,
    0x08, 0x00, 0x07,
    0x0c, 0x40, 0x07,
    0x0d, 0x80, 0x07,
    0x17, 24, 0,
    0x18, 25, 0,
    0x28, 23, 0,
    0x37, 22, 0,
    0x67, 19, 0,
    0x68, 20, 0,
    0x6c, 21, 0,
    54,
    0x12, 1984 % 256, 1984 / 256,
    0x13, 2048 % 256, 2048 / 256,
    0x14, 2112 % 256, 2112 / 256,
    0x15, 2176 % 256, 2176 / 256,
    0x16, 2240 % 256, 2240 / 256,
    0x17, 2304 % 256, 2304 / 256,
    0x1c, 2368 % 256, 2368 / 256,
    0x1d, 2432 % 256, 2432 / 256,
    0x1e, 2496 % 256, 2496 / 256,
    0x1f, 2560 % 256, 2560 / 256,
    0x24, 52, 0,
    0x27, 55, 0,
    0x28, 56, 0,
    0x2b, 59, 0,
    0x2c, 60, 0,
    0x33, 320 % 256, 320 / 256,
    0x34, 384 % 256, 384 / 256,
    0x35, 448 % 256, 448 / 256,
    0x37, 53, 0,
    0x38, 54, 0,
    0x52, 50, 0,
    0x53, 51, 0,
    0x54, 44, 0,
    0x55, 45, 0,
    0x56, 46, 0,
    0x57, 47, 0,
    0x58, 57, 0,
    0x59, 58, 0,
    0x5a, 61, 0,
    0x5b, 256 % 256, 256 / 256,
    0x64, 48, 0,
    0x65, 49, 0,
    0x66, 62, 0,
    0x67, 63, 0,
    0x68, 30, 0,
    0x69, 31, 0,
    0x6a, 32, 0,
    0x6b, 33, 0,
    0x6c, 40, 0,
    0x6d, 41, 0,
    0xc8, 128, 0,
    0xc9, 192, 0,
    0xca, 26, 0,
    0xcb, 27, 0,
    0xcc, 28, 0,
    0xcd, 29, 0,
    0xd2, 34, 0,
    0xd3, 35, 0,
    0xd4, 36, 0,
    0xd5, 37, 0,
    0xd6, 38, 0,
    0xd7, 39, 0,
    0xda, 42, 0,
    0xdb, 43, 0,
    20,
    0x4a, 640 % 256, 640 / 256,
    0x4b, 704 % 256, 704 / 256,
    0x4c, 768 % 256, 768 / 256,
    0x4d, 832 % 256, 832 / 256,
    0x52, 1280 % 256, 1280 / 256,
    0x53, 1344 % 256, 1344 / 256,
    0x54, 1408 % 256, 1408 / 256,
    0x55, 1472 % 256, 1472 / 256,
    0x5a, 1536 % 256, 1536 / 256,
    0x5b, 1600 % 256, 1600 / 256,
    0x64, 1664 % 256, 1664 / 256,
    0x65, 1728 % 256, 1728 / 256,
    0x6c, 512 % 256, 512 / 256,
    0x6d, 576 % 256, 576 / 256,
    0x72, 896 % 256, 896 / 256,
    0x73, 960 % 256, 960 / 256,
    0x74, 1024 % 256, 1024 / 256,
    0x75, 1088 % 256, 1088 / 256,
    0x76, 1152 % 256, 1152 / 256,
    0x77, 1216 % 256, 1216 / 256,
    0xff
};
static const FX_BYTE FaxWhiteRunIns[] = {
    0,
    0,
    0,
    6,
    0x07, 2, 0,
    0x08, 3, 0,
    0x0B, 4, 0,
    0x0C, 5, 0,
    0x0E, 6, 0,
    0x0F, 7, 0,
    6,
    0x07, 10, 0,
    0x08, 11, 0,
    0x12, 128, 0,
    0x13, 8, 0,
    0x14, 9, 0,
    0x1b, 64, 0,
    9,
    0x03, 13, 0,
    0x07, 1, 0,
    0x08, 12, 0,
    0x17, 192, 0,
    0x18, 1664 % 256, 1664 / 256,
    0x2a, 16, 0,
    0x2B, 17, 0,
    0x34, 14, 0,
    0x35, 15, 0,
    12,
    0x03, 22, 0,
    0x04, 23, 0,
    0x08, 20, 0,
    0x0c, 19, 0,
    0x13, 26, 0,
    0x17, 21, 0,
    0x18, 28, 0,
    0x24, 27, 0,
    0x27, 18, 0,
    0x28, 24, 0,
    0x2B, 25, 0,
    0x37, 256 % 256, 256 / 256,
    42,
    0x02, 29, 0,
    0x03, 30, 0,
    0x04, 45, 0,
    0x05, 46, 0,
    0x0a, 47, 0,
    0x0b, 48, 0,
    0x12, 33, 0,
    0x13, 34, 0,
    0x14, 35, 0,
    0x15, 36, 0,
    0x16, 37, 0,
    0x17, 38, 0,
    0x1a, 31, 0,
    0x1b, 32, 0,
    0x24, 53, 0,
    0x25, 54, 0,
    0x28, 39, 0,
    0x29, 40, 0,
    0x2a, 41, 0,
    0x2b, 42, 0,
    0x2c, 43, 0,
    0x2d, 44, 0,
    0x32, 61, 0,
    0x33, 62, 0,
    0x34, 63, 0,
    0x35, 0, 0,
    0x36, 320 % 256, 320 / 256,
    0x37, 384 % 256, 384 / 256,
    0x4a, 59, 0,
    0x4b, 60, 0,
    0x52, 49, 0,
    0x53, 50, 0,
    0x54, 51, 0,
    0x55, 52, 0,
    0x58, 55, 0,
    0x59, 56, 0,
    0x5a, 57, 0,
    0x5b, 58, 0,
    0x64, 448 % 256, 448 / 256,
    0x65, 512 % 256, 512 / 256,
    0x67, 640 % 256, 640 / 256,
    0x68, 576 % 256, 576 / 256,
    16,
    0x98, 1472 % 256, 1472 / 256,
    0x99, 1536 % 256, 1536 / 256,
    0x9a, 1600 % 256, 1600 / 256,
    0x9b, 1728 % 256, 1728 / 256,
    0xcc, 704 % 256, 704 / 256,
    0xcd, 768 % 256, 768 / 256,
    0xd2, 832 % 256, 832 / 256,
    0xd3, 896 % 256, 896 / 256,
    0xd4, 960 % 256, 960 / 256,
    0xd5, 1024 % 256, 1024 / 256,
    0xd6, 1088 % 256, 1088 / 256,
    0xd7, 1152 % 256, 1152 / 256,
    0xd8, 1216 % 256, 1216 / 256,
    0xd9, 1280 % 256, 1280 / 256,
    0xda, 1344 % 256, 1344 / 256,
    0xdb, 1408 % 256, 1408 / 256,
    0,
    3,
    0x08, 1792 % 256, 1792 / 256,
    0x0c, 1856 % 256, 1856 / 256,
    0x0d, 1920 % 256, 1920 / 256,
    10,
    0x12, 1984 % 256, 1984 / 256,
    0x13, 2048 % 256, 2048 / 256,
    0x14, 2112 % 256, 2112 / 256,
    0x15, 2176 % 256, 2176 / 256,
    0x16, 2240 % 256, 2240 / 256,
    0x17, 2304 % 256, 2304 / 256,
    0x1c, 2368 % 256, 2368 / 256,
    0x1d, 2432 % 256, 2432 / 256,
    0x1e, 2496 % 256, 2496 / 256,
    0x1f, 2560 % 256, 2560 / 256,
    0xff,
};
int _FaxGetRun(FX_LPCBYTE ins_array, const FX_BYTE* src_buf, int& bitpos, int bitsize)
{
    FX_DWORD code = 0;
    int ins_off = 0;
    while (1) {
        FX_BYTE ins = ins_array[ins_off++];
        if (ins == 0xff) {
            return -1;
        }
        if (bitpos >= bitsize) {
            return -1;
        }
        code <<= 1;
        if (src_buf[bitpos / 8] & (1 << (7 - bitpos % 8))) {
            code ++;
        }
        bitpos ++;
        int next_off = ins_off + ins * 3;
        for (; ins_off < next_off; ins_off += 3) {
            if (ins_array[ins_off] == code) {
                return ins_array[ins_off + 1] + ins_array[ins_off + 2] * 256;
            }
        }
    }
}
FX_BOOL _FaxG4GetRow(const FX_BYTE* src_buf, int bitsize, int& bitpos, FX_LPBYTE dest_buf, const FX_BYTE* ref_buf, int columns)
{
    int a0 = -1, a0color = 1;
    while (1) {
        if (bitpos >= bitsize) {
            return FALSE;
        }
        int a1, a2, b1, b2;
        _FaxG4FindB1B2(ref_buf, columns, a0, a0color, b1, b2);
        FX_BOOL bit = NEXTBIT;
        int v_delta = 0;
        if (bit) {
        } else {
            if (bitpos >= bitsize) {
                return FALSE;
            }
            FX_BOOL bit1 = NEXTBIT;
            if (bitpos >= bitsize) {
                return FALSE;
            }
            FX_BOOL bit2 = NEXTBIT;
            if (bit1 && bit2) {
                v_delta = 1;
            } else if (bit1) {
                v_delta = -1;
            } else if (bit2) {
                int run_len1 = 0;
                while (1) {
                    int run = _FaxGetRun(a0color ? FaxWhiteRunIns : FaxBlackRunIns, src_buf, bitpos, bitsize);
                    run_len1 += run;
                    if (run < 64) {
                        break;
                    }
                }
                if (a0 < 0) {
                    run_len1 ++;
                }
                a1 = a0 + run_len1;
                if (!a0color) {
                    _FaxFillBits(dest_buf, columns, a0, a1);
                }
                int run_len2 = 0;
                while (1) {
                    int run = _FaxGetRun(a0color ? FaxBlackRunIns : FaxWhiteRunIns, src_buf, bitpos, bitsize);
                    run_len2 += run;
                    if (run < 64) {
                        break;
                    }
                }
                a2 = a1 + run_len2;
                if (a0color) {
                    _FaxFillBits(dest_buf, columns, a1, a2);
                }
                a0 = a2;
                if (a0 < columns) {
                    continue;
                }
                return TRUE;
            } else {
                if (bitpos >= bitsize) {
                    return FALSE;
                }
                bit = NEXTBIT;
                if (bit) {
                    if (!a0color) {
                        _FaxFillBits(dest_buf, columns, a0, b2);
                    }
                    if (b2 >= columns) {
                        return TRUE;
                    }
                    a0 = b2;
                    continue;
                } else {
                    if (bitpos >= bitsize) {
                        return FALSE;
                    }
                    FX_BOOL bit1 = NEXTBIT;
                    if (bitpos >= bitsize) {
                        return FALSE;
                    }
                    FX_BOOL bit2 = NEXTBIT;
                    if (bit1 && bit2) {
                        v_delta = 2;
                    } else if (bit1) {
                        v_delta = -2;
                    } else if (bit2) {
                        if (bitpos >= bitsize) {
                            return FALSE;
                        }
                        bit = NEXTBIT;
                        if (bit) {
                            v_delta = 3;
                        } else {
                            v_delta = -3;
                        }
                    } else {
                        if (bitpos >= bitsize) {
                            return FALSE;
                        }
                        bit = NEXTBIT;
                        if (bit) {
                            bitpos += 3;
                            continue;
                        } else {
                            bitpos += 5;
                            return TRUE;
                        }
                    }
                }
            }
        }
        a1 = b1 + v_delta;
        if (!a0color) {
            _FaxFillBits(dest_buf, columns, a0, a1);
        }
        if (a1 >= columns) {
            return TRUE;
        }
        a0 = a1;
        a0color = !a0color;
    }
}
FX_BOOL _FaxSkipEOL(const FX_BYTE* src_buf, int bitsize, int& bitpos)
{
    int startbit = bitpos;
    while (bitpos < bitsize) {
        int bit = NEXTBIT;
        if (bit) {
            if (bitpos - startbit <= 11) {
                bitpos = startbit;
            }
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL _FaxGet1DLine(const FX_BYTE* src_buf, int bitsize, int& bitpos, FX_LPBYTE dest_buf, int columns)
{
    int color = TRUE;
    int startpos = 0;
    while (1) {
        if (bitpos >= bitsize) {
            return FALSE;
        }
        int run_len = 0;
        while (1) {
            int run = _FaxGetRun(color ? FaxWhiteRunIns : FaxBlackRunIns, src_buf, bitpos, bitsize);
            if (run < 0) {
                while (bitpos < bitsize) {
                    int bit = NEXTBIT;
                    if (bit) {
                        return TRUE;
                    }
                }
                return FALSE;
            }
            run_len += run;
            if (run < 64) {
                break;
            }
        }
        if (!color) {
            _FaxFillBits(dest_buf, columns, startpos, startpos + run_len);
        }
        startpos += run_len;
        if (startpos >= columns) {
            break;
        }
        color = !color;
    }
    return TRUE;
}
class CCodec_FaxDecoder : public CCodec_ScanlineDecoder
{
public:
    CCodec_FaxDecoder();
    virtual ~CCodec_FaxDecoder();
    FX_BOOL				Create(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
                               int K, FX_BOOL EndOfLine, FX_BOOL EncodedByteAlign, FX_BOOL BlackIs1, int Columns, int Rows);
    virtual void		v_DownScale(int dest_width, int dest_height) {}
    virtual FX_BOOL		v_Rewind();
    virtual FX_LPBYTE	v_GetNextLine();
    virtual FX_DWORD	GetSrcOffset();
    int			m_Encoding, m_bEndOfLine, m_bByteAlign, m_bBlack;
    int			bitpos;
    FX_LPCBYTE	m_pSrcBuf;
    FX_DWORD	m_SrcSize;
    FX_LPBYTE	m_pScanlineBuf, m_pRefBuf;
};
CCodec_FaxDecoder::CCodec_FaxDecoder()
{
    m_pScanlineBuf = NULL;
    m_pRefBuf = NULL;
}
CCodec_FaxDecoder::~CCodec_FaxDecoder()
{
    if (m_pScanlineBuf) {
        FX_Free(m_pScanlineBuf);
    }
    if (m_pRefBuf) {
        FX_Free(m_pRefBuf);
    }
}
FX_BOOL CCodec_FaxDecoder::Create(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
                                  int K, FX_BOOL EndOfLine, FX_BOOL EncodedByteAlign, FX_BOOL BlackIs1, int Columns, int Rows)
{
    m_Encoding = K;
    m_bEndOfLine = EndOfLine;
    m_bByteAlign = EncodedByteAlign;
    m_bBlack = BlackIs1;
    m_OrigWidth = Columns;
    m_OrigHeight = Rows;
    if (m_OrigWidth == 0) {
        m_OrigWidth = width;
    }
    if (m_OrigHeight == 0) {
        m_OrigHeight = height;
    }
    m_Pitch = (m_OrigWidth + 31) / 32 * 4;
    m_OutputWidth = m_OrigWidth;
    m_OutputHeight = m_OrigHeight;
    m_pScanlineBuf = FX_Alloc(FX_BYTE, m_Pitch);
    m_pRefBuf = FX_Alloc(FX_BYTE, m_Pitch);
    m_pSrcBuf = src_buf;
    m_SrcSize = src_size;
    m_nComps = 1;
    m_bpc = 1;
    m_bColorTransformed = FALSE;
    return TRUE;
}
FX_BOOL CCodec_FaxDecoder::v_Rewind()
{
    FXSYS_memset8(m_pRefBuf, 0xff, m_Pitch);
    bitpos = 0;
    return TRUE;
}
FX_LPBYTE CCodec_FaxDecoder::v_GetNextLine()
{
    int bitsize = m_SrcSize * 8;
    _FaxSkipEOL(m_pSrcBuf, bitsize, bitpos);
    if (bitpos >= bitsize) {
        return NULL;
    }
    FXSYS_memset8(m_pScanlineBuf, 0xff, m_Pitch);
    if (m_Encoding < 0) {
        _FaxG4GetRow(m_pSrcBuf, bitsize, bitpos, m_pScanlineBuf, m_pRefBuf, m_OrigWidth);
        FXSYS_memcpy32(m_pRefBuf, m_pScanlineBuf, m_Pitch);
    } else if (m_Encoding == 0) {
        _FaxGet1DLine(m_pSrcBuf, bitsize, bitpos, m_pScanlineBuf, m_OrigWidth);
    } else {
        FX_BOOL bNext1D = m_pSrcBuf[bitpos / 8] & (1 << (7 - bitpos % 8));
        bitpos ++;
        if (bNext1D) {
            _FaxGet1DLine(m_pSrcBuf, bitsize, bitpos, m_pScanlineBuf, m_OrigWidth);
        } else {
            _FaxG4GetRow(m_pSrcBuf, bitsize, bitpos, m_pScanlineBuf, m_pRefBuf, m_OrigWidth);
        }
        FXSYS_memcpy32(m_pRefBuf, m_pScanlineBuf, m_Pitch);
    }
    if (m_bEndOfLine) {
        _FaxSkipEOL(m_pSrcBuf, bitsize, bitpos);
    }
    if (m_bByteAlign && bitpos < bitsize) {
        int bitpos0 = bitpos;
        int bitpos1 = (bitpos + 7) / 8 * 8;
        while (m_bByteAlign && bitpos0 < bitpos1) {
            int bit = m_pSrcBuf[bitpos0 / 8] & (1 << (7 - bitpos0 % 8));
            if (bit != 0) {
                m_bByteAlign = FALSE;
            } else {
                bitpos0 ++;
            }
        }
        if (m_bByteAlign) {
            bitpos = bitpos1;
        }
    }
    if (m_bBlack) {
        for (int i = 0; i < m_Pitch; i ++) {
            m_pScanlineBuf[i] = ~m_pScanlineBuf[i];
        }
    }
    return m_pScanlineBuf;
}
FX_DWORD CCodec_FaxDecoder::GetSrcOffset()
{
    FX_DWORD ret = (bitpos + 7) / 8;
    if (ret > m_SrcSize) {
        ret = m_SrcSize;
    }
    return ret;
}
extern "C" {
    void _FaxG4Decode(void*, FX_LPCBYTE src_buf, FX_DWORD src_size, int* pbitpos, FX_LPBYTE dest_buf, int width, int height, int pitch)
    {
        if (pitch == 0) {
            pitch = (width + 7) / 8;
        }
        FX_LPBYTE ref_buf = FX_Alloc(FX_BYTE, pitch);
        FXSYS_memset8(ref_buf, 0xff, pitch);
        int bitpos = *pbitpos;
        for (int iRow = 0; iRow < height; iRow ++) {
            FX_LPBYTE line_buf = dest_buf + iRow * pitch;
            FXSYS_memset8(line_buf, 0xff, pitch);
            _FaxG4GetRow(src_buf, src_size << 3, bitpos, line_buf, ref_buf, width);
            FXSYS_memcpy32(ref_buf, line_buf, pitch);
        }
        FX_Free(ref_buf);
        *pbitpos = bitpos;
    }
};
static const FX_BYTE BlackRunTerminator[128] = {
    0x37, 10, 0x02, 3, 0x03, 2, 0x02, 2, 0x03, 3, 0x03, 4, 0x02, 4, 0x03, 5,
    0x05, 6, 0x04, 6, 0x04, 7, 0x05, 7, 0x07, 7, 0x04, 8, 0x07, 8, 0x18, 9,
    0x17, 10, 0x18, 10, 0x08, 10, 0x67, 11, 0x68, 11, 0x6c, 11, 0x37, 11, 0x28, 11,
    0x17, 11, 0x18, 11, 0xca, 12, 0xcb, 12, 0xcc, 12, 0xcd, 12, 0x68, 12, 0x69, 12,
    0x6a, 12, 0x6b, 12, 0xd2, 12, 0xd3, 12, 0xd4, 12, 0xd5, 12, 0xd6, 12, 0xd7, 12,
    0x6c, 12, 0x6d, 12, 0xda, 12, 0xdb, 12, 0x54, 12, 0x55, 12, 0x56, 12, 0x57, 12,
    0x64, 12, 0x65, 12, 0x52, 12, 0x53, 12, 0x24, 12, 0x37, 12, 0x38, 12, 0x27, 12,
    0x28, 12, 0x58, 12, 0x59, 12, 0x2b, 12, 0x2c, 12, 0x5a, 12, 0x66, 12, 0x67, 12,
};
static const FX_BYTE BlackRunMarkup[80] = {
    0x0f, 10, 0xc8, 12, 0xc9, 12, 0x5b, 12, 0x33, 12, 0x34, 12, 0x35, 12, 0x6c, 13,
    0x6d, 13, 0x4a, 13, 0x4b, 13, 0x4c, 13, 0x4d, 13, 0x72, 13, 0x73, 13, 0x74, 13,
    0x75, 13, 0x76, 13, 0x77, 13, 0x52, 13, 0x53, 13, 0x54, 13, 0x55, 13, 0x5a, 13,
    0x5b, 13, 0x64, 13, 0x65, 13, 0x08, 11, 0x0c, 11, 0x0d, 11, 0x12, 12, 0x13, 12,
    0x14, 12, 0x15, 12, 0x16, 12, 0x17, 12, 0x1c, 12, 0x1d, 12, 0x1e, 12, 0x1f, 12,
};
static const FX_BYTE WhiteRunTerminator[128] = {
    0x35, 8,
    0x07, 6,
    0x07, 4,
    0x08, 4,
    0x0B, 4,
    0x0C, 4,
    0x0E, 4,
    0x0F, 4,
    0x13, 5,
    0x14, 5,
    0x07, 5,
    0x08, 5,
    0x08, 6,
    0x03, 6,
    0x34, 6,
    0x35, 6,
    0x2a, 6,
    0x2B, 6,
    0x27, 7,
    0x0c, 7,
    0x08, 7,
    0x17, 7,
    0x03, 7,
    0x04, 7,
    0x28, 7,
    0x2B, 7,
    0x13, 7,
    0x24, 7,
    0x18, 7,
    0x02, 8,
    0x03, 8,
    0x1a, 8,
    0x1b, 8,
    0x12, 8,
    0x13, 8,
    0x14, 8,
    0x15, 8,
    0x16, 8,
    0x17, 8,
    0x28, 8,
    0x29, 8,
    0x2a, 8,
    0x2b, 8,
    0x2c, 8,
    0x2d, 8,
    0x04, 8,
    0x05, 8,
    0x0a, 8,
    0x0b, 8,
    0x52, 8,
    0x53, 8,
    0x54, 8,
    0x55, 8,
    0x24, 8,
    0x25, 8,
    0x58, 8,
    0x59, 8,
    0x5a, 8,
    0x5b, 8,
    0x4a, 8,
    0x4b, 8,
    0x32, 8,
    0x33, 8,
    0x34, 8,
};
static const FX_BYTE WhiteRunMarkup[80] = {
    0x1b, 5,
    0x12, 5,
    0x17, 6,
    0x37, 7,
    0x36, 8,
    0x37, 8,
    0x64, 8,
    0x65, 8,
    0x68, 8,
    0x67, 8,
    0xcc, 9,
    0xcd, 9,
    0xd2, 9,
    0xd3, 9,
    0xd4, 9,
    0xd5, 9,
    0xd6, 9,
    0xd7, 9,
    0xd8, 9,
    0xd9, 9,
    0xda, 9,
    0xdb, 9,
    0x98, 9,
    0x99, 9,
    0x9a, 9,
    0x18, 6,
    0x9b, 9,
    0x08, 11,
    0x0c, 11,
    0x0d, 11,
    0x12, 12,
    0x13, 12,
    0x14, 12,
    0x15, 12,
    0x16, 12,
    0x17, 12,
    0x1c, 12,
    0x1d, 12,
    0x1e, 12,
    0x1f, 12,
};
static void _AddBitStream(FX_LPBYTE dest_buf, int& dest_bitpos, int data, int bitlen)
{
    for (int i = bitlen - 1; i >= 0; i --) {
        if (data & (1 << i)) {
            dest_buf[dest_bitpos / 8] |= 1 << (7 - dest_bitpos % 8);
        }
        dest_bitpos ++;
    }
}
static void _FaxEncodeRun(FX_LPBYTE dest_buf, int& dest_bitpos, int run, FX_BOOL bWhite)
{
    while (run >= 2560) {
        _AddBitStream(dest_buf, dest_bitpos, 0x1f, 12);
        run -= 2560;
    }
    if (run >= 64) {
        int markup = run - run % 64;
        FX_LPCBYTE p = bWhite ? WhiteRunMarkup : BlackRunMarkup;
        p += (markup / 64 - 1) * 2;
        _AddBitStream(dest_buf, dest_bitpos, *p, p[1]);
    }
    run %= 64;
    FX_LPCBYTE p = bWhite ? WhiteRunTerminator : BlackRunTerminator;
    p += run * 2;
    _AddBitStream(dest_buf, dest_bitpos, *p, p[1]);
}
static void _FaxEncode2DLine(FX_LPBYTE dest_buf, int& dest_bitpos, FX_LPCBYTE src_buf, FX_LPCBYTE ref_buf, int cols)
{
    int a0 = -1, a0color = 1;
    while (1) {
        int a1 = _FindBit(src_buf, cols, a0 + 1, 1 - a0color);
        int b1, b2;
        _FaxG4FindB1B2(ref_buf, cols, a0, a0color, b1, b2);
        if (b2 < a1) {
            dest_bitpos += 3;
            dest_buf[dest_bitpos / 8] |= 1 << (7 - dest_bitpos % 8);
            dest_bitpos ++;
            a0 = b2;
        } else if (a1 - b1 <= 3 && b1 - a1 <= 3) {
            int delta = a1 - b1;
            switch (delta) {
                case 0:
                    dest_buf[dest_bitpos / 8] |= 1 << (7 - dest_bitpos % 8);
                    break;
                case 1:
                case 2:
                case 3:
                    dest_bitpos += delta == 1 ? 1 : delta + 2;
                    dest_buf[dest_bitpos / 8] |= 1 << (7 - dest_bitpos % 8);
                    dest_bitpos ++;
                    dest_buf[dest_bitpos / 8] |= 1 << (7 - dest_bitpos % 8);
                    break;
                case -1:
                case -2:
                case -3:
                    dest_bitpos += delta == -1 ? 1 : -delta + 2;
                    dest_buf[dest_bitpos / 8] |= 1 << (7 - dest_bitpos % 8);
                    dest_bitpos ++;
                    break;
            }
            dest_bitpos ++;
            a0 = a1;
            a0color = 1 - a0color;
        } else {
            int a2 = _FindBit(src_buf, cols, a1 + 1, a0color);
            dest_bitpos ++;
            dest_bitpos ++;
            dest_buf[dest_bitpos / 8] |= 1 << (7 - dest_bitpos % 8);
            dest_bitpos ++;
            if (a0 < 0) {
                a0 = 0;
            }
            _FaxEncodeRun(dest_buf, dest_bitpos, a1 - a0, a0color);
            _FaxEncodeRun(dest_buf, dest_bitpos, a2 - a1, 1 - a0color);
            a0 = a2;
        }
        if (a0 >= cols) {
            return;
        }
    }
}
class CCodec_FaxEncoder 
{
public:
    CCodec_FaxEncoder(FX_LPCBYTE src_buf, int width, int height, int pitch);
    ~CCodec_FaxEncoder();
    void			Encode(FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
    void			Encode2DLine(FX_LPCBYTE scan_line);
    CFX_BinaryBuf	m_DestBuf;
    FX_LPBYTE		m_pRefLine, m_pLineBuf;
    int				m_Cols, m_Rows, m_Pitch;
    FX_LPCBYTE		m_pSrcBuf;
};
CCodec_FaxEncoder::CCodec_FaxEncoder(FX_LPCBYTE src_buf, int width, int height, int pitch)
{
    m_pSrcBuf = src_buf;
    m_Cols = width;
    m_Rows = height;
    m_Pitch = pitch;
    m_pRefLine = FX_Alloc(FX_BYTE, m_Pitch);
    FXSYS_memset8(m_pRefLine, 0xff, m_Pitch);
    m_pLineBuf = FX_Alloc2D(FX_BYTE, m_Pitch, 8);
    m_DestBuf.EstimateSize(0, 10240);
}
CCodec_FaxEncoder::~CCodec_FaxEncoder()
{
    if (m_pRefLine) {
        FX_Free(m_pRefLine);
    }
    if (m_pLineBuf) {
        FX_Free(m_pLineBuf);
    }
}
void CCodec_FaxEncoder::Encode(FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    int dest_bitpos = 0;
    FX_BYTE last_byte = 0;
    for (int i = 0; i < m_Rows; i ++) {
        FX_LPCBYTE scan_line = m_pSrcBuf + i * m_Pitch;
        FXSYS_memset32(m_pLineBuf, 0, m_Pitch * 8);
        m_pLineBuf[0] = last_byte;
        _FaxEncode2DLine(m_pLineBuf, dest_bitpos, scan_line, m_pRefLine, m_Cols);
        m_DestBuf.AppendBlock(m_pLineBuf, dest_bitpos / 8);
        last_byte = m_pLineBuf[dest_bitpos / 8];
        dest_bitpos %= 8;
        FXSYS_memcpy32(m_pRefLine, scan_line, m_Pitch);
    }
    if (dest_bitpos) {
        m_DestBuf.AppendByte(last_byte);
    }
    dest_buf = m_DestBuf.GetBuffer();
    dest_size = m_DestBuf.GetSize();
    m_DestBuf.DetachBuffer();
}
FX_BOOL	CCodec_FaxModule::Encode(FX_LPCBYTE src_buf, int width, int height, int pitch, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    CCodec_FaxEncoder encoder(src_buf, width, height, pitch);
    encoder.Encode(dest_buf, dest_size);
    return TRUE;
}
ICodec_ScanlineDecoder*	CCodec_FaxModule::CreateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        int K, FX_BOOL EndOfLine, FX_BOOL EncodedByteAlign, FX_BOOL BlackIs1, int Columns, int Rows)
{
    CCodec_FaxDecoder* pDecoder = new CCodec_FaxDecoder;
    pDecoder->Create(src_buf, src_size, width, height, K, EndOfLine, EncodedByteAlign, BlackIs1, Columns, Rows);
    return pDecoder;
}
