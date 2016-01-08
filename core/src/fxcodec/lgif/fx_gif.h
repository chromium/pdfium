// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <setjmp.h>

#include "core/include/fxcrt/fx_basic.h"

extern FX_WORD _GetWord_LSBFirst(uint8_t* p);
extern void _SetWord_LSBFirst(uint8_t* p, FX_WORD v);
extern void _BpcConvert(const uint8_t* src_buf,
                        FX_DWORD src_len,
                        int32_t src_bpc,
                        int32_t dst_bpc,
                        uint8_t*& dst_buf,
                        FX_DWORD& dst_len);
#define GIF_SUPPORT_COMMENT_EXTENSION
#define GIF_SUPPORT_GRAPHIC_CONTROL_EXTENSION
#define GIF_SUPPORT_PLAIN_TEXT_EXTENSION
#define GIF_SIGNATURE "GIF"
#define GIF_SIG_EXTENSION 0x21
#define GIF_SIG_IMAGE 0x2C
#define GIF_SIG_TRAILER 0x3B
#define GIF_BLOCK_GCE 0xF9
#define GIF_BLOCK_PTE 0x01
#define GIF_BLOCK_CE 0xFE
#define GIF_BLOCK_AE 0xFF
#define GIF_BLOCK_TERMINAL 0x00
#define GIF_MAX_LZW_CODE 4096
#define GIF_DATA_BLOCK 255
#define GIF_MAX_ERROR_SIZE 256
#define GIF_D_STATUS_SIG 0x01
#define GIF_D_STATUS_TAIL 0x02
#define GIF_D_STATUS_EXT 0x03
#define GIF_D_STATUS_EXT_AE 0x04
#define GIF_D_STATUS_EXT_CE 0x05
#define GIF_D_STATUS_EXT_GCE 0x06
#define GIF_D_STATUS_EXT_PTE 0x07
#define GIF_D_STATUS_EXT_UNE 0x08
#define GIF_D_STATUS_IMG_INFO 0x09
#define GIF_D_STATUS_IMG_DATA 0x0A
#pragma pack(1)
typedef struct tagGifGF {
  uint8_t pal_bits : 3;
  uint8_t sort_flag : 1;
  uint8_t color_resolution : 3;
  uint8_t global_pal : 1;
} GifGF;
typedef struct tagGifLF {
  uint8_t pal_bits : 3;
  uint8_t reserved : 2;
  uint8_t sort_flag : 1;
  uint8_t interlace : 1;
  uint8_t local_pal : 1;
} GifLF;
typedef struct tagGifHeader {
  char signature[3];
  char version[3];
} GifHeader;
typedef struct tagGifLSD {
  FX_WORD width;
  FX_WORD height;
  uint8_t global_flag;
  uint8_t bc_index;
  uint8_t pixel_aspect;
} GifLSD;
typedef struct tagGifImageInfo {
  FX_WORD left;
  FX_WORD top;
  FX_WORD width;
  FX_WORD height;

  uint8_t local_flag;
} GifImageInfo;
typedef struct tagGifCEF {
  uint8_t transparency : 1;
  uint8_t user_input : 1;
  uint8_t disposal_method : 3;
  uint8_t reserved : 3;
} GifCEF;
typedef struct tagGifGCE {
  uint8_t block_size;
  uint8_t gce_flag;
  FX_WORD delay_time;
  uint8_t trans_index;
} GifGCE;
typedef struct tagGifPTE {
  uint8_t block_size;
  FX_WORD grid_left;
  FX_WORD grid_top;
  FX_WORD grid_width;
  FX_WORD grid_height;

  uint8_t char_width;
  uint8_t char_height;

  uint8_t fc_index;
  uint8_t bc_index;
} GifPTE;
typedef struct tagGifAE {
  uint8_t block_size;
  uint8_t app_identify[8];
  uint8_t app_authentication[3];
} GifAE;
typedef struct tagGifPalette { uint8_t r, g, b; } GifPalette;
#pragma pack()
typedef struct tagGifImage {
  GifGCE* image_gce_ptr;
  GifPalette* local_pal_ptr;
  GifImageInfo* image_info_ptr;
  uint8_t image_code_size;
  FX_DWORD image_data_pos;
  uint8_t* image_row_buf;
  int32_t image_row_num;
} GifImage;
typedef struct tagGifPlainText {
  GifGCE* gce_ptr;
  GifPTE* pte_ptr;
  CFX_ByteString* string_ptr;
} GifPlainText;
class CGifLZWDecoder {
 public:
  struct tag_Table {
    FX_WORD prefix;
    uint8_t suffix;
  };
  CGifLZWDecoder(FX_CHAR* error_ptr = NULL) { err_msg_ptr = error_ptr; }
  void InitTable(uint8_t code_len);

  int32_t Decode(uint8_t* des_buf, FX_DWORD& des_size);

  void Input(uint8_t* src_buf, FX_DWORD src_size);
  FX_DWORD GetAvailInput();

 private:
  void ClearTable();
  void AddCode(FX_WORD prefix_code, uint8_t append_char);
  void DecodeString(FX_WORD code);
  uint8_t code_size;
  uint8_t code_size_cur;
  FX_WORD code_clear;
  FX_WORD code_end;
  FX_WORD code_next;
  uint8_t code_first;
  uint8_t stack[GIF_MAX_LZW_CODE];
  FX_WORD stack_size;
  tag_Table code_table[GIF_MAX_LZW_CODE];
  FX_WORD code_old;

  uint8_t* next_in;
  FX_DWORD avail_in;

  uint8_t bits_left;
  FX_DWORD code_store;

  FX_CHAR* err_msg_ptr;
};
class CGifLZWEncoder {
 public:
  struct tag_Table {
    FX_WORD prefix;
    uint8_t suffix;
  };
  CGifLZWEncoder();
  ~CGifLZWEncoder();
  void Start(uint8_t code_len,
             const uint8_t* src_buf,
             uint8_t*& dst_buf,
             FX_DWORD& offset);
  FX_BOOL Encode(const uint8_t* src_buf,
                 FX_DWORD src_len,
                 uint8_t*& dst_buf,
                 FX_DWORD& dst_len,
                 FX_DWORD& offset);
  void Finish(uint8_t*& dst_buf, FX_DWORD& dst_len, FX_DWORD& offset);

 private:
  void ClearTable();
  FX_BOOL LookUpInTable(const uint8_t* buf,
                        FX_DWORD& offset,
                        uint8_t& bit_offset);
  void EncodeString(FX_DWORD index,
                    uint8_t*& dst_buf,
                    FX_DWORD& dst_len,
                    FX_DWORD& offset);
  void WriteBlock(uint8_t*& dst_buf, FX_DWORD& dst_len, FX_DWORD& offset);
  jmp_buf jmp;
  FX_DWORD src_offset;
  uint8_t src_bit_offset;
  uint8_t src_bit_cut;
  FX_DWORD src_bit_num;
  uint8_t code_size;
  FX_WORD code_clear;
  FX_WORD code_end;
  FX_WORD index_num;
  uint8_t bit_offset;
  uint8_t index_bit_cur;
  uint8_t index_buf[GIF_DATA_BLOCK];
  uint8_t index_buf_len;
  tag_Table code_table[GIF_MAX_LZW_CODE];
  FX_WORD table_cur;
};
typedef struct tag_gif_decompress_struct gif_decompress_struct;
typedef gif_decompress_struct* gif_decompress_struct_p;
typedef gif_decompress_struct_p* gif_decompress_struct_pp;
static int32_t s_gif_interlace_step[4] = {8, 8, 4, 2};
struct tag_gif_decompress_struct {
  jmp_buf jmpbuf;
  FX_CHAR* err_ptr;
  void (*_gif_error_fn)(gif_decompress_struct_p gif_ptr,
                        const FX_CHAR* err_msg);
  void* context_ptr;
  int width;
  int height;
  GifPalette* global_pal_ptr;
  int32_t global_pal_num;
  uint8_t global_sort_flag;
  uint8_t global_color_resolution;

  uint8_t bc_index;
  uint8_t pixel_aspect;
  CGifLZWDecoder* img_decoder_ptr;
  FX_DWORD img_row_offset;
  FX_DWORD img_row_avail_size;
  uint8_t img_pass_num;
  CFX_ArrayTemplate<GifImage*>* img_ptr_arr_ptr;
  uint8_t* (*_gif_ask_buf_for_pal_fn)(gif_decompress_struct_p gif_ptr,
                                      int32_t pal_size);
  uint8_t* next_in;
  FX_DWORD avail_in;
  int32_t decode_status;
  FX_DWORD skip_size;
  void (*_gif_record_current_position_fn)(gif_decompress_struct_p gif_ptr,
                                          FX_DWORD* cur_pos_ptr);
  void (*_gif_get_row_fn)(gif_decompress_struct_p gif_ptr,
                          int32_t row_num,
                          uint8_t* row_buf);
  FX_BOOL			(*_gif_get_record_position_fn)(gif_decompress_struct_p gif_ptr, FX_DWORD cur_pos,
            int32_t left, int32_t top, int32_t width, int32_t height,
            int32_t pal_num, void* pal_ptr,
            int32_t delay_time, FX_BOOL user_input,
            int32_t trans_index, int32_t disposal_method, FX_BOOL interlace);
#ifdef GIF_SUPPORT_APPLICATION_EXTENSION
  uint8_t app_identify[8];
  uint8_t app_authentication[3];
  FX_DWORD app_data_size;
  uint8_t* app_data;
#endif
#ifdef GIF_SUPPORT_COMMENT_EXTENSION
  CFX_ByteString* cmt_data_ptr;
#endif
#ifdef GIF_SUPPORT_GRAPHIC_CONTROL_EXTENSION
  GifGCE* gce_ptr;
#endif
#ifdef GIF_SUPPORT_PLAIN_TEXT_EXTENSION
  CFX_ArrayTemplate<GifPlainText*>* pt_ptr_arr_ptr;
#endif
};
typedef struct tag_gif_compress_struct gif_compress_struct;
typedef gif_compress_struct* gif_compress_struct_p;
typedef gif_compress_struct_p* gif_compress_struct_pp;
struct tag_gif_compress_struct {
  const uint8_t* src_buf;
  FX_DWORD src_pitch;
  FX_DWORD src_width;
  FX_DWORD src_row;
  FX_DWORD cur_offset;
  FX_DWORD frames;
  GifHeader* header_ptr;
  GifLSD* lsd_ptr;
  GifPalette* global_pal;
  FX_WORD gpal_num;
  GifPalette* local_pal;
  FX_WORD lpal_num;
  GifImageInfo* image_info_ptr;
  CGifLZWEncoder* img_encoder_ptr;
#ifdef GIF_SUPPORT_APPLICATION_EXTENSION
  uint8_t app_identify[8];
  uint8_t app_authentication[3];
  FX_DWORD app_data_size;
  uint8_t* app_data;
#endif

#ifdef GIF_SUPPORT_COMMENT_EXTENSION
  uint8_t* cmt_data_ptr;
  FX_DWORD cmt_data_len;
#endif

#ifdef GIF_SUPPORT_GRAPHIC_CONTROL_EXTENSION
  GifGCE* gce_ptr;
#endif

#ifdef GIF_SUPPORT_PLAIN_TEXT_EXTENSION
  GifPTE* pte_ptr;
  const uint8_t* pte_data_ptr;
  FX_DWORD pte_data_len;
#endif
};
void _gif_error(gif_decompress_struct_p gif_ptr, const FX_CHAR* err_msg);
void _gif_warn(gif_decompress_struct_p gif_ptr, const FX_CHAR* err_msg);
gif_decompress_struct_p _gif_create_decompress();
void _gif_destroy_decompress(gif_decompress_struct_pp gif_ptr_ptr);
gif_compress_struct_p _gif_create_compress();
void _gif_destroy_compress(gif_compress_struct_pp gif_ptr_ptr);
int32_t _gif_read_header(gif_decompress_struct_p gif_ptr);
int32_t _gif_get_frame(gif_decompress_struct_p gif_ptr);
int32_t _gif_get_frame_num(gif_decompress_struct_p gif_ptr);
int32_t _gif_decode_extension(gif_decompress_struct_p gif_ptr);
int32_t _gif_decode_image_info(gif_decompress_struct_p gif_ptr);
void _gif_takeover_gce_ptr(gif_decompress_struct_p gif_ptr,
                           GifGCE** gce_ptr_ptr);
int32_t _gif_load_frame(gif_decompress_struct_p gif_ptr, int32_t frame_num);
uint8_t* _gif_read_data(gif_decompress_struct_p gif_ptr,
                        uint8_t** des_buf_pp,
                        FX_DWORD data_size);
void _gif_save_decoding_status(gif_decompress_struct_p gif_ptr, int32_t status);
void _gif_input_buffer(gif_decompress_struct_p gif_ptr,
                       uint8_t* src_buf,
                       FX_DWORD src_size);
FX_DWORD _gif_get_avail_input(gif_decompress_struct_p gif_ptr,
                              uint8_t** avial_buf_ptr);
void interlace_buf(const uint8_t* buf, FX_DWORD width, FX_DWORD height);
FX_BOOL _gif_encode(gif_compress_struct_p gif_ptr,
                    uint8_t*& dst_buf,
                    FX_DWORD& dst_len);
#define GIF_PTR_NOT_NULL(ptr, gif_ptr)    \
  if (ptr == NULL) {                      \
    _gif_error(gif_ptr, "Out Of Memory"); \
    return 0;                             \
  }
