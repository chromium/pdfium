// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <setjmp.h>

#include "core/include/fxcrt/fx_basic.h"

#define BMP_SUPPORT_BITFIELD
#define BMP_WIDTHBYTES(width, bitCount) ((width * bitCount) + 31) / 32 * 4
#define BMP_PAL_ENCODE(a, r, g, b) \
  (((FX_DWORD)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define BMP_D_STATUS_HEADER 0x01
#define BMP_D_STATUS_PAL 0x02
#define BMP_D_STATUS_DATA_PRE 0x03
#define BMP_D_STATUS_DATA 0x04
#define BMP_D_STATUS_TAIL 0x00
#define BMP_SIGNATURE 0x4D42
#define BMP_PAL_NEW 0
#define BMP_PAL_OLD 1
#define RLE_MARKER 0
#define RLE_EOL 0
#define RLE_EOI 1
#define RLE_DELTA 2
#define BMP_RGB 0L
#define BMP_RLE8 1L
#define BMP_RLE4 2L
#define BMP_BITFIELDS 3L
#define BMP_BIT_555 0
#define BMP_BIT_565 1
#define BMP_MAX_ERROR_SIZE 256
#pragma pack(1)
typedef struct tagBmpFileHeader {
  FX_WORD bfType;
  FX_DWORD bfSize;
  FX_WORD bfReserved1;
  FX_WORD bfReserved2;
  FX_DWORD bfOffBits;
} BmpFileHeader, *BmpFileHeaderPtr;
typedef struct tagBmpCoreHeader {
  FX_DWORD bcSize;
  FX_WORD bcWidth;
  FX_WORD bcHeight;
  FX_WORD bcPlanes;
  FX_WORD bcBitCount;
} BmpCoreHeader, *BmpCoreHeaderPtr;
typedef struct tagBmpInfoHeader {
  FX_DWORD biSize;
  int32_t biWidth;
  int32_t biHeight;
  FX_WORD biPlanes;
  FX_WORD biBitCount;
  FX_DWORD biCompression;
  FX_DWORD biSizeImage;
  int32_t biXPelsPerMeter;
  int32_t biYPelsPerMeter;
  FX_DWORD biClrUsed;
  FX_DWORD biClrImportant;
} BmpInfoHeader, *BmpInfoHeaderPtr;
#pragma pack()
typedef struct tag_bmp_decompress_struct bmp_decompress_struct;
typedef bmp_decompress_struct* bmp_decompress_struct_p;
typedef bmp_decompress_struct_p* bmp_decompress_struct_pp;
struct tag_bmp_decompress_struct {
  jmp_buf jmpbuf;
  FX_CHAR* err_ptr;
  void (*_bmp_error_fn)(bmp_decompress_struct_p gif_ptr,
                        const FX_CHAR* err_msg);

  void* context_ptr;

  BmpFileHeaderPtr bmp_header_ptr;
  BmpInfoHeaderPtr bmp_infoheader_ptr;
  int32_t width;
  int32_t height;
  FX_DWORD compress_flag;
  int32_t components;
  int32_t src_row_bytes;
  int32_t out_row_bytes;
  uint8_t* out_row_buffer;
  FX_WORD bitCounts;
  FX_DWORD color_used;
  FX_BOOL imgTB_flag;
  int32_t pal_num;
  int32_t pal_type;
  FX_DWORD* pal_ptr;
  FX_DWORD data_size;
  FX_DWORD img_data_offset;
  FX_DWORD img_ifh_size;
  int32_t row_num;
  int32_t col_num;
  int32_t dpi_x;
  int32_t dpi_y;
#ifdef BMP_SUPPORT_BITFIELD
  FX_DWORD mask_red;
  FX_DWORD mask_green;
  FX_DWORD mask_blue;
#endif

  FX_BOOL			(*_bmp_get_data_position_fn)(bmp_decompress_struct_p bmp_ptr, FX_DWORD cur_pos);
  void (*_bmp_get_row_fn)(bmp_decompress_struct_p bmp_ptr,
                          int32_t row_num,
                          uint8_t* row_buf);
  uint8_t* next_in;
  FX_DWORD avail_in;
  FX_DWORD skip_size;
  int32_t decode_status;
};
void _bmp_error(bmp_decompress_struct_p bmp_ptr, const FX_CHAR* err_msg);
bmp_decompress_struct_p _bmp_create_decompress();
void _bmp_destroy_decompress(bmp_decompress_struct_pp bmp_ptr_ptr);
int32_t _bmp_read_header(bmp_decompress_struct_p bmp_ptr);
int32_t _bmp_decode_image(bmp_decompress_struct_p bmp_ptr);
int32_t _bmp_decode_rgb(bmp_decompress_struct_p bmp_ptr);
int32_t _bmp_decode_rle8(bmp_decompress_struct_p bmp_ptr);
int32_t _bmp_decode_rle4(bmp_decompress_struct_p bmp_ptr);
uint8_t* _bmp_read_data(bmp_decompress_struct_p bmp_ptr,
                        uint8_t** des_buf_pp,
                        FX_DWORD data_size);
void _bmp_save_decoding_status(bmp_decompress_struct_p bmp_ptr, int32_t status);
void _bmp_input_buffer(bmp_decompress_struct_p bmp_ptr,
                       uint8_t* src_buf,
                       FX_DWORD src_size);
FX_DWORD _bmp_get_avail_input(bmp_decompress_struct_p bmp_ptr,
                              uint8_t** avial_buf_ptr);
#define BMP_PTR_NOT_NULL(ptr, bmp_ptr)    \
  if (ptr == NULL) {                      \
    _bmp_error(bmp_ptr, "Out Of Memory"); \
    return 0;                             \
  }
typedef struct tag_bmp_compress_struct bmp_compress_struct;
typedef bmp_compress_struct* bmp_compress_struct_p;
typedef bmp_compress_struct_p* bmp_compress_struct_pp;
struct tag_bmp_compress_struct {
  BmpFileHeader file_header;
  BmpInfoHeader info_header;
  uint8_t* src_buf;
  FX_DWORD src_pitch;
  FX_DWORD src_row;
  uint8_t src_bpp;
  FX_DWORD src_width;
  FX_BOOL src_free;
  FX_DWORD* pal_ptr;
  FX_WORD pal_num;
#ifdef BMP_SUPPORT_BITFIELD
  uint8_t bit_type;
#endif
};
bmp_compress_struct_p _bmp_create_compress();
void _bmp_destroy_compress(bmp_compress_struct_p bmp_ptr);
FX_BOOL _bmp_encode_image(bmp_compress_struct_p bmp_ptr,
                          uint8_t*& dst_buf,
                          FX_DWORD& dst_size);
