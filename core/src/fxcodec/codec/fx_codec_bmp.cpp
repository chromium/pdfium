// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcodec/fx_codec.h"
#include "core/include/fxge/fx_dib.h"
#include "codec_int.h"
#include "core/src/fxcodec/lbmp/fx_bmp.h"
struct FXBMP_Context {
  bmp_decompress_struct_p bmp_ptr;
  void* parent_ptr;
  void* child_ptr;

  void* (*m_AllocFunc)(unsigned int);
  void (*m_FreeFunc)(void*);
};
extern "C" {
static void* _bmp_alloc_func(unsigned int size) {
  return FX_Alloc(char, size);
}
static void _bmp_free_func(void* p) {
  if (p != NULL) {
    FX_Free(p);
  }
}
};
static void _bmp_error_data(bmp_decompress_struct_p bmp_ptr,
                            const FX_CHAR* err_msg) {
  FXSYS_strncpy((char*)bmp_ptr->err_ptr, err_msg, BMP_MAX_ERROR_SIZE - 1);
  longjmp(bmp_ptr->jmpbuf, 1);
}
static void _bmp_read_scanline(bmp_decompress_struct_p bmp_ptr,
                               int32_t row_num,
                               uint8_t* row_buf) {
  FXBMP_Context* p = (FXBMP_Context*)bmp_ptr->context_ptr;
  CCodec_BmpModule* pModule = (CCodec_BmpModule*)p->parent_ptr;
  pModule->ReadScanlineCallback(p->child_ptr, row_num, row_buf);
}
static FX_BOOL _bmp_get_data_position(bmp_decompress_struct_p bmp_ptr,
                                      FX_DWORD rcd_pos) {
  FXBMP_Context* p = (FXBMP_Context*)bmp_ptr->context_ptr;
  CCodec_BmpModule* pModule = (CCodec_BmpModule*)p->parent_ptr;
  return pModule->InputImagePositionBufCallback(p->child_ptr, rcd_pos);
}
void* CCodec_BmpModule::Start(void* pModule) {
  FXBMP_Context* p = (FXBMP_Context*)FX_Alloc(uint8_t, sizeof(FXBMP_Context));
  if (p == NULL) {
    return NULL;
  }
  FXSYS_memset(p, 0, sizeof(FXBMP_Context));
  if (p == NULL) {
    return NULL;
  }
  p->m_AllocFunc = _bmp_alloc_func;
  p->m_FreeFunc = _bmp_free_func;
  p->bmp_ptr = NULL;
  p->parent_ptr = (void*)this;
  p->child_ptr = pModule;
  p->bmp_ptr = _bmp_create_decompress();
  if (p->bmp_ptr == NULL) {
    FX_Free(p);
    return NULL;
  }
  p->bmp_ptr->context_ptr = (void*)p;
  p->bmp_ptr->err_ptr = m_szLastError;
  p->bmp_ptr->_bmp_error_fn = _bmp_error_data;
  p->bmp_ptr->_bmp_get_row_fn = _bmp_read_scanline;
  p->bmp_ptr->_bmp_get_data_position_fn = _bmp_get_data_position;
  return p;
}
void CCodec_BmpModule::Finish(void* pContext) {
  FXBMP_Context* p = (FXBMP_Context*)pContext;
  if (p != NULL) {
    _bmp_destroy_decompress(&p->bmp_ptr);
    p->m_FreeFunc(p);
  }
}
int32_t CCodec_BmpModule::ReadHeader(void* pContext,
                                     int32_t* width,
                                     int32_t* height,
                                     FX_BOOL* tb_flag,
                                     int32_t* components,
                                     int32_t* pal_num,
                                     FX_DWORD** pal_pp,
                                     CFX_DIBAttribute* pAttribute) {
  FXBMP_Context* p = (FXBMP_Context*)pContext;
  if (setjmp(p->bmp_ptr->jmpbuf)) {
    return 0;
  }
  int32_t ret = _bmp_read_header(p->bmp_ptr);
  if (ret != 1) {
    return ret;
  }
  *width = p->bmp_ptr->width;
  *height = p->bmp_ptr->height;
  *tb_flag = p->bmp_ptr->imgTB_flag;
  *components = p->bmp_ptr->components;
  *pal_num = p->bmp_ptr->pal_num;
  *pal_pp = p->bmp_ptr->pal_ptr;
  if (pAttribute) {
    pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_METER;
    pAttribute->m_nXDPI = p->bmp_ptr->dpi_x;
    pAttribute->m_nYDPI = p->bmp_ptr->dpi_y;
    pAttribute->m_nBmpCompressType = p->bmp_ptr->compress_flag;
  }
  return 1;
}
int32_t CCodec_BmpModule::LoadImage(void* pContext) {
  FXBMP_Context* p = (FXBMP_Context*)pContext;
  if (setjmp(p->bmp_ptr->jmpbuf)) {
    return 0;
  }
  return _bmp_decode_image(p->bmp_ptr);
}
FX_DWORD CCodec_BmpModule::GetAvailInput(void* pContext,
                                         uint8_t** avial_buf_ptr) {
  FXBMP_Context* p = (FXBMP_Context*)pContext;
  return _bmp_get_avail_input(p->bmp_ptr, avial_buf_ptr);
}
void CCodec_BmpModule::Input(void* pContext,
                             const uint8_t* src_buf,
                             FX_DWORD src_size) {
  FXBMP_Context* p = (FXBMP_Context*)pContext;
  _bmp_input_buffer(p->bmp_ptr, (uint8_t*)src_buf, src_size);
}
