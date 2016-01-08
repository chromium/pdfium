// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcodec/fx_codec.h"
#include "core/include/fxge/fx_dib.h"
#include "codec_int.h"
#include "core/src/fxcodec/lgif/fx_gif.h"
struct FXGIF_Context {
  gif_decompress_struct_p gif_ptr;
  void* parent_ptr;
  void* child_ptr;

  void* (*m_AllocFunc)(unsigned int);
  void (*m_FreeFunc)(void*);
};
extern "C" {
static void* _gif_alloc_func(unsigned int size) {
  return FX_Alloc(char, size);
}
static void _gif_free_func(void* p) {
  if (p != NULL) {
    FX_Free(p);
  }
}
};
static void _gif_error_data(gif_decompress_struct_p gif_ptr,
                            const FX_CHAR* err_msg) {
  FXSYS_strncpy((char*)gif_ptr->err_ptr, err_msg, GIF_MAX_ERROR_SIZE - 1);
  longjmp(gif_ptr->jmpbuf, 1);
}
static uint8_t* _gif_ask_buf_for_pal(gif_decompress_struct_p gif_ptr,
                                     int32_t pal_size) {
  FXGIF_Context* p = (FXGIF_Context*)gif_ptr->context_ptr;
  CCodec_GifModule* pModule = (CCodec_GifModule*)p->parent_ptr;
  return pModule->AskLocalPaletteBufCallback(
      p->child_ptr, _gif_get_frame_num(gif_ptr), pal_size);
}
static void _gif_record_current_position(gif_decompress_struct_p gif_ptr,
                                         FX_DWORD* cur_pos_ptr) {
  FXGIF_Context* p = (FXGIF_Context*)gif_ptr->context_ptr;
  CCodec_GifModule* pModule = (CCodec_GifModule*)p->parent_ptr;
  pModule->RecordCurrentPositionCallback(p->child_ptr, *cur_pos_ptr);
}
static void _gif_read_scanline(gif_decompress_struct_p gif_ptr,
                               int32_t row_num,
                               uint8_t* row_buf) {
  FXGIF_Context* p = (FXGIF_Context*)gif_ptr->context_ptr;
  CCodec_GifModule* pModule = (CCodec_GifModule*)p->parent_ptr;
  pModule->ReadScanlineCallback(p->child_ptr, row_num, row_buf);
}
static FX_BOOL _gif_get_record_position(gif_decompress_struct_p gif_ptr,
                                        FX_DWORD cur_pos,
                                        int32_t left,
                                        int32_t top,
                                        int32_t width,
                                        int32_t height,
                                        int32_t pal_num,
                                        void* pal_ptr,
                                        int32_t delay_time,
                                        FX_BOOL user_input,
                                        int32_t trans_index,
                                        int32_t disposal_method,
                                        FX_BOOL interlace) {
  FXGIF_Context* p = (FXGIF_Context*)gif_ptr->context_ptr;
  CCodec_GifModule* pModule = (CCodec_GifModule*)p->parent_ptr;
  return pModule->InputRecordPositionBufCallback(
      p->child_ptr, cur_pos, FX_RECT(left, top, left + width, top + height),
      pal_num, pal_ptr, delay_time, user_input, trans_index, disposal_method,
      interlace);
}
void* CCodec_GifModule::Start(void* pModule) {
  FXGIF_Context* p = (FXGIF_Context*)FX_Alloc(uint8_t, sizeof(FXGIF_Context));
  if (p == NULL) {
    return NULL;
  }
  FXSYS_memset(p, 0, sizeof(FXGIF_Context));
  p->m_AllocFunc = _gif_alloc_func;
  p->m_FreeFunc = _gif_free_func;
  p->gif_ptr = NULL;
  p->parent_ptr = (void*)this;
  p->child_ptr = pModule;
  p->gif_ptr = _gif_create_decompress();
  if (p->gif_ptr == NULL) {
    FX_Free(p);
    return NULL;
  }
  p->gif_ptr->context_ptr = (void*)p;
  p->gif_ptr->err_ptr = m_szLastError;
  p->gif_ptr->_gif_error_fn = _gif_error_data;
  p->gif_ptr->_gif_ask_buf_for_pal_fn = _gif_ask_buf_for_pal;
  p->gif_ptr->_gif_record_current_position_fn = _gif_record_current_position;
  p->gif_ptr->_gif_get_row_fn = _gif_read_scanline;
  p->gif_ptr->_gif_get_record_position_fn = _gif_get_record_position;
  return p;
}
void CCodec_GifModule::Finish(void* pContext) {
  FXGIF_Context* p = (FXGIF_Context*)pContext;
  if (p != NULL) {
    _gif_destroy_decompress(&p->gif_ptr);
    p->m_FreeFunc(p);
  }
}
int32_t CCodec_GifModule::ReadHeader(void* pContext,
                                     int* width,
                                     int* height,
                                     int* pal_num,
                                     void** pal_pp,
                                     int* bg_index,
                                     CFX_DIBAttribute* pAttribute) {
  FXGIF_Context* p = (FXGIF_Context*)pContext;
  if (setjmp(p->gif_ptr->jmpbuf)) {
    return 0;
  }
  int32_t ret = _gif_read_header(p->gif_ptr);
  if (ret != 1) {
    return ret;
  }
  if (pAttribute) {
  }
  *width = p->gif_ptr->width;
  *height = p->gif_ptr->height;
  *pal_num = p->gif_ptr->global_pal_num;
  *pal_pp = p->gif_ptr->global_pal_ptr;
  *bg_index = p->gif_ptr->bc_index;
  return 1;
}
int32_t CCodec_GifModule::LoadFrameInfo(void* pContext, int* frame_num) {
  FXGIF_Context* p = (FXGIF_Context*)pContext;
  if (setjmp(p->gif_ptr->jmpbuf)) {
    return 0;
  }
  int32_t ret = _gif_get_frame(p->gif_ptr);
  if (ret != 1) {
    return ret;
  }
  *frame_num = _gif_get_frame_num(p->gif_ptr);
  return 1;
}
int32_t CCodec_GifModule::LoadFrame(void* pContext,
                                    int frame_num,
                                    CFX_DIBAttribute* pAttribute) {
  FXGIF_Context* p = (FXGIF_Context*)pContext;
  if (setjmp(p->gif_ptr->jmpbuf)) {
    return 0;
  }
  int32_t ret = _gif_load_frame(p->gif_ptr, frame_num);
  if (ret == 1) {
    if (pAttribute) {
      pAttribute->m_nGifLeft =
          p->gif_ptr->img_ptr_arr_ptr->GetAt(frame_num)->image_info_ptr->left;
      pAttribute->m_nGifTop =
          p->gif_ptr->img_ptr_arr_ptr->GetAt(frame_num)->image_info_ptr->top;
      pAttribute->m_fAspectRatio = p->gif_ptr->pixel_aspect;
      if (p->gif_ptr->cmt_data_ptr) {
        const uint8_t* buf =
            (const uint8_t*)p->gif_ptr->cmt_data_ptr->GetBuffer(0);
        FX_DWORD len = p->gif_ptr->cmt_data_ptr->GetLength();
        if (len > 21) {
          uint8_t size = *buf++;
          if (size) {
            pAttribute->m_strAuthor = CFX_ByteString(buf, size);
          } else {
            pAttribute->m_strAuthor.Empty();
          }
          buf += size;
          size = *buf++;
          if (size == 20) {
            FXSYS_memcpy(pAttribute->m_strTime, buf, size);
          }
        }
      }
    }
  }
  return ret;
}
FX_DWORD CCodec_GifModule::GetAvailInput(void* pContext,
                                         uint8_t** avial_buf_ptr) {
  FXGIF_Context* p = (FXGIF_Context*)pContext;
  return _gif_get_avail_input(p->gif_ptr, avial_buf_ptr);
}
void CCodec_GifModule::Input(void* pContext,
                             const uint8_t* src_buf,
                             FX_DWORD src_size) {
  FXGIF_Context* p = (FXGIF_Context*)pContext;
  _gif_input_buffer(p->gif_ptr, (uint8_t*)src_buf, src_size);
}
