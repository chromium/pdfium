// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_BASIC_H_
#include "../../../include/fxcrt/fx_basic.h"
#endif
extern FX_WORD _GetWord_LSBFirst(FX_LPBYTE p);
extern void _SetWord_LSBFirst(FX_LPBYTE p, FX_WORD v);
extern void _BpcConvert(FX_LPCBYTE src_buf, FX_DWORD src_len, FX_INT32 src_bpc, FX_INT32 dst_bpc,
                        FX_LPBYTE& dst_buf, FX_DWORD& dst_len);
#define GIF_SUPPORT_COMMENT_EXTENSION
#define GIF_SUPPORT_GRAPHIC_CONTROL_EXTENSION
#define GIF_SUPPORT_PLAIN_TEXT_EXTENSION
#define GIF_SIGNATURE		"GIF"
#define GIF_SIG_EXTENSION	0x21
#define GIF_SIG_IMAGE		0x2C
#define GIF_SIG_TRAILER		0x3B
#define GIF_BLOCK_GCE		0xF9
#define GIF_BLOCK_PTE		0x01
#define GIF_BLOCK_CE		0xFE
#define GIF_BLOCK_AE		0xFF
#define GIF_BLOCK_TERMINAL	0x00
#define GIF_MAX_LZW_CODE	4096
#define GIF_DATA_BLOCK		255
#define GIF_MAX_ERROR_SIZE	256
#define GIF_D_STATUS_SIG		0x01
#define GIF_D_STATUS_TAIL		0x02
#define GIF_D_STATUS_EXT		0x03
#define GIF_D_STATUS_EXT_AE		0x04
#define GIF_D_STATUS_EXT_CE		0x05
#define GIF_D_STATUS_EXT_GCE	0x06
#define GIF_D_STATUS_EXT_PTE	0x07
#define GIF_D_STATUS_EXT_UNE	0x08
#define GIF_D_STATUS_IMG_INFO	0x09
#define GIF_D_STATUS_IMG_DATA	0x0A
#pragma pack(1)
typedef struct tagGifGF {
    FX_BYTE pal_bits : 3;
    FX_BYTE sort_flag : 1;
    FX_BYTE color_resolution : 3;
    FX_BYTE global_pal : 1;
} GifGF;
typedef struct tagGifLF {
    FX_BYTE pal_bits : 3;
    FX_BYTE reserved : 2;
    FX_BYTE sort_flag : 1;
    FX_BYTE interlace : 1;
    FX_BYTE local_pal : 1;
} GifLF;
typedef struct tagGifHeader {
    char signature[3];
    char version[3];
} GifHeader;
typedef struct tagGifLSD {
    FX_WORD	width;
    FX_WORD height;
    FX_BYTE	global_flag;
    FX_BYTE bc_index;
    FX_BYTE	pixel_aspect;
} GifLSD;
typedef struct tagGifImageInfo {
    FX_WORD left;
    FX_WORD top;
    FX_WORD width;
    FX_WORD height;

    FX_BYTE	local_flag;
} GifImageInfo;
typedef struct tagGifCEF {
    FX_BYTE transparency : 1;
    FX_BYTE user_input : 1;
    FX_BYTE disposal_method : 3;
    FX_BYTE reserved : 3;
} GifCEF;
typedef struct tagGifGCE {
    FX_BYTE block_size;
    FX_BYTE	gce_flag;
    FX_WORD delay_time;
    FX_BYTE	trans_index;
} GifGCE;
typedef struct tagGifPTE {
    FX_BYTE block_size;
    FX_WORD	grid_left;
    FX_WORD	grid_top;
    FX_WORD grid_width;
    FX_WORD	grid_height;

    FX_BYTE char_width;
    FX_BYTE char_height;

    FX_BYTE fc_index;
    FX_BYTE bc_index;
} GifPTE;
typedef struct tagGifAE {
    FX_BYTE block_size;
    FX_BYTE app_identify[8];
    FX_BYTE	app_authentication[3];
} GifAE;
typedef struct tagGifPalette {
    FX_BYTE r, g, b;
} GifPalette;
#pragma pack()
typedef struct tagGifImage {
    GifGCE*			image_gce_ptr;
    GifPalette*		local_pal_ptr;
    GifImageInfo*	image_info_ptr;
    FX_BYTE			image_code_size;
    FX_DWORD		image_data_pos;
    FX_LPBYTE		image_row_buf;
    FX_INT32		image_row_num;
} GifImage;
typedef struct tagGifPlainText {
    GifGCE*			gce_ptr;
    GifPTE*			pte_ptr;
    CFX_ByteString* string_ptr;
} GifPlainText;
class CGifLZWDecoder : public CFX_Object
{
public:
    struct tag_Table {
        FX_WORD prefix;
        FX_BYTE suffix;
    };
    CGifLZWDecoder(FX_LPSTR error_ptr = NULL)
    {
        err_msg_ptr = error_ptr;
    }
    void		InitTable(FX_BYTE code_len);

    FX_INT32	Decode(FX_LPBYTE des_buf, FX_DWORD& des_size);

    void		Input(FX_LPBYTE src_buf, FX_DWORD src_size);
    FX_DWORD	GetAvailInput();

private:
    void		ClearTable();
    void		AddCode(FX_WORD prefix_code, FX_BYTE append_char);
    void		DecodeString(FX_WORD code);
    FX_BYTE		code_size;
    FX_BYTE		code_size_cur;
    FX_WORD		code_clear;
    FX_WORD		code_end;
    FX_WORD		code_next;
    FX_BYTE		code_first;
    FX_BYTE		stack[GIF_MAX_LZW_CODE];
    FX_WORD		stack_size;
    tag_Table	code_table[GIF_MAX_LZW_CODE];
    FX_WORD		code_old;

    FX_LPBYTE	next_in;
    FX_DWORD	avail_in;

    FX_BYTE		bits_left;
    FX_DWORD	code_store;

    FX_LPSTR	err_msg_ptr;
};
class CGifLZWEncoder : public CFX_Object
{
public:
    struct tag_Table {
        FX_WORD		prefix;
        FX_BYTE		suffix;
    };
    CGifLZWEncoder();
    ~CGifLZWEncoder();
    void		Start(FX_BYTE code_len, FX_LPCBYTE src_buf, FX_LPBYTE& dst_buf, FX_DWORD& offset);
    FX_BOOL		Encode(FX_LPCBYTE src_buf, FX_DWORD src_len, FX_LPBYTE& dst_buf, FX_DWORD& dst_len, FX_DWORD& offset);
    void		Finish(FX_LPBYTE& dst_buf, FX_DWORD& dst_len, FX_DWORD& offset);
private:
    void		ClearTable();
    FX_BOOL		LookUpInTable(FX_LPCBYTE buf, FX_DWORD& offset, FX_BYTE& bit_offset);
    void		EncodeString(FX_DWORD index, FX_LPBYTE& dst_buf, FX_DWORD& dst_len, FX_DWORD& offset);
    void		WriteBlock(FX_LPBYTE& dst_buf, FX_DWORD& dst_len, FX_DWORD& offset);
    jmp_buf		jmp;
    FX_DWORD	src_offset;
    FX_BYTE		src_bit_offset;
    FX_BYTE		src_bit_cut;
    FX_DWORD	src_bit_num;
    FX_BYTE		code_size;
    FX_WORD		code_clear;
    FX_WORD		code_end;
    FX_WORD		index_num;
    FX_BYTE		bit_offset;
    FX_BYTE		index_bit_cur;
    FX_BYTE		index_buf[GIF_DATA_BLOCK];
    FX_BYTE		index_buf_len;
    tag_Table	code_table[GIF_MAX_LZW_CODE];
    FX_WORD		table_cur;
};
typedef struct tag_gif_decompress_struct gif_decompress_struct;
typedef gif_decompress_struct *gif_decompress_struct_p;
typedef gif_decompress_struct_p *gif_decompress_struct_pp;
static FX_INT32 s_gif_interlace_step[4] = {8, 8, 4, 2};
struct tag_gif_decompress_struct {
    jmp_buf			jmpbuf;
    FX_LPSTR		err_ptr;
    void			(*_gif_error_fn)(gif_decompress_struct_p gif_ptr, FX_LPCSTR err_msg);
    void*			context_ptr;
    int				width;
    int				height;
    GifPalette*		global_pal_ptr;
    FX_INT32		global_pal_num;
    FX_BYTE			global_sort_flag;
    FX_BYTE			global_color_resolution;

    FX_BYTE			bc_index;
    FX_BYTE			pixel_aspect;
    CGifLZWDecoder*	img_decoder_ptr;
    FX_DWORD		img_row_offset;
    FX_DWORD		img_row_avail_size;
    FX_BYTE			img_pass_num;
    CFX_ArrayTemplate<GifImage*>* img_ptr_arr_ptr;
    FX_LPBYTE		(*_gif_ask_buf_for_pal_fn)(gif_decompress_struct_p gif_ptr, FX_INT32 pal_size);
    FX_LPBYTE		next_in;
    FX_DWORD		avail_in;
    FX_INT32		decode_status;
    FX_DWORD		skip_size;
    void			(*_gif_record_current_position_fn)(gif_decompress_struct_p gif_ptr, FX_DWORD* cur_pos_ptr);
    void			(*_gif_get_row_fn)(gif_decompress_struct_p gif_ptr, FX_INT32 row_num, FX_LPBYTE row_buf);
    FX_BOOL			(*_gif_get_record_position_fn)(gif_decompress_struct_p gif_ptr, FX_DWORD cur_pos,
            FX_INT32 left, FX_INT32 top, FX_INT32 width, FX_INT32 height,
            FX_INT32 pal_num, void* pal_ptr,
            FX_INT32 delay_time, FX_BOOL user_input,
            FX_INT32 trans_index, FX_INT32 disposal_method, FX_BOOL interlace);
#ifdef GIF_SUPPORT_APPLICATION_EXTENSION
    FX_BYTE			app_identify[8];
    FX_BYTE			app_authentication[3];
    FX_DWORD		app_data_size;
    FX_LPBYTE		app_data;
#endif
#ifdef GIF_SUPPORT_COMMENT_EXTENSION
    CFX_ByteString*	cmt_data_ptr;
#endif
#ifdef GIF_SUPPORT_GRAPHIC_CONTROL_EXTENSION
    GifGCE*			gce_ptr;
#endif
#ifdef GIF_SUPPORT_PLAIN_TEXT_EXTENSION
    CFX_ArrayTemplate<GifPlainText*>* pt_ptr_arr_ptr;
#endif
};
typedef struct tag_gif_compress_struct gif_compress_struct;
typedef gif_compress_struct *gif_compress_struct_p;
typedef gif_compress_struct_p *gif_compress_struct_pp;
struct tag_gif_compress_struct {
    FX_LPCBYTE		src_buf;
    FX_DWORD		src_pitch;
    FX_DWORD		src_width;
    FX_DWORD		src_row;
    FX_DWORD		cur_offset;
    FX_DWORD		frames;
    GifHeader*		header_ptr;
    GifLSD*			lsd_ptr;
    GifPalette*		global_pal;
    FX_WORD			gpal_num;
    GifPalette*		local_pal;
    FX_WORD			lpal_num;
    GifImageInfo*	image_info_ptr;
    CGifLZWEncoder* img_encoder_ptr;
#ifdef GIF_SUPPORT_APPLICATION_EXTENSION
    FX_BYTE			app_identify[8];
    FX_BYTE			app_authentication[3];
    FX_DWORD		app_data_size;
    FX_LPBYTE		app_data;
#endif

#ifdef GIF_SUPPORT_COMMENT_EXTENSION
    FX_LPBYTE		cmt_data_ptr;
    FX_DWORD		cmt_data_len;
#endif

#ifdef GIF_SUPPORT_GRAPHIC_CONTROL_EXTENSION
    GifGCE*			gce_ptr;
#endif

#ifdef GIF_SUPPORT_PLAIN_TEXT_EXTENSION
    GifPTE*			pte_ptr;
    FX_LPCBYTE		pte_data_ptr;
    FX_DWORD		pte_data_len;
#endif
};
void _gif_error(gif_decompress_struct_p gif_ptr, FX_LPCSTR err_msg);
void _gif_warn(gif_decompress_struct_p gif_ptr, FX_LPCSTR err_msg);
gif_decompress_struct_p _gif_create_decompress();
void _gif_destroy_decompress(gif_decompress_struct_pp gif_ptr_ptr);
gif_compress_struct_p _gif_create_compress();
void _gif_destroy_compress(gif_compress_struct_pp gif_ptr_ptr);
FX_INT32 _gif_read_header(gif_decompress_struct_p gif_ptr);
FX_INT32 _gif_get_frame(gif_decompress_struct_p gif_ptr);
FX_INT32 _gif_get_frame_num(gif_decompress_struct_p gif_ptr);
FX_INT32 _gif_decode_extension(gif_decompress_struct_p gif_ptr);
FX_INT32 _gif_decode_image_info(gif_decompress_struct_p gif_ptr);
void _gif_takeover_gce_ptr(gif_decompress_struct_p gif_ptr, GifGCE** gce_ptr_ptr);
FX_INT32 _gif_load_frame(gif_decompress_struct_p gif_ptr, FX_INT32 frame_num);
FX_LPBYTE _gif_read_data(gif_decompress_struct_p gif_ptr, FX_LPBYTE* des_buf_pp, FX_DWORD data_size);
void _gif_save_decoding_status(gif_decompress_struct_p gif_ptr, FX_INT32 status);
void _gif_input_buffer(gif_decompress_struct_p gif_ptr, FX_LPBYTE src_buf, FX_DWORD src_size);
FX_DWORD _gif_get_avail_input(gif_decompress_struct_p gif_ptr, FX_LPBYTE* avial_buf_ptr);
void interlace_buf(FX_LPCBYTE buf, FX_DWORD width, FX_DWORD height);
FX_BOOL _gif_encode( gif_compress_struct_p gif_ptr, FX_LPBYTE& dst_buf, FX_DWORD& dst_len );
#define GIF_PTR_NOT_NULL(ptr,gif_ptr)	if(ptr == NULL){						\
        _gif_error(gif_ptr,"Out Of Memory");\
        return 0;							\
    }
