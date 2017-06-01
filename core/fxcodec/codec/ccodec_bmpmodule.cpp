// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/ccodec_bmpmodule.h"

#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

class CBmpContext : public CCodec_BmpModule::Context {
 public:
  CBmpContext(bmp_decompress_struct_p pBmp,
              CCodec_BmpModule* pModule,
              CCodec_BmpModule::Delegate* pDelegate);
  ~CBmpContext() override;

  bmp_decompress_struct_p m_pBmp;
  CFX_UnownedPtr<CCodec_BmpModule> const m_pModule;
  CFX_UnownedPtr<CCodec_BmpModule::Delegate> const m_pDelegate;
  char m_szLastError[256];
};

extern "C" {

static void bmp_error_data(bmp_decompress_struct_p pBmp, const char* err_msg) {
  strncpy(pBmp->err_ptr, err_msg, BMP_MAX_ERROR_SIZE - 1);
  longjmp(pBmp->jmpbuf, 1);
}

static void bmp_read_scanline(bmp_decompress_struct_p pBmp,
                              int32_t row_num,
                              uint8_t* row_buf) {
  auto* p = reinterpret_cast<CBmpContext*>(pBmp->context_ptr);
  p->m_pDelegate->BmpReadScanline(row_num, row_buf);
}

static bool bmp_get_data_position(bmp_decompress_struct_p pBmp,
                                  uint32_t rcd_pos) {
  auto* p = reinterpret_cast<CBmpContext*>(pBmp->context_ptr);
  return p->m_pDelegate->BmpInputImagePositionBuf(rcd_pos);
}

}  // extern "C"

CBmpContext::CBmpContext(bmp_decompress_struct_p pBmp,
                         CCodec_BmpModule* pModule,
                         CCodec_BmpModule::Delegate* pDelegate)
    : m_pBmp(pBmp), m_pModule(pModule), m_pDelegate(pDelegate) {
  memset(m_szLastError, 0, sizeof(m_szLastError));
}

CBmpContext::~CBmpContext() {
  bmp_destroy_decompress(&m_pBmp);
}

CCodec_BmpModule::CCodec_BmpModule() {}

CCodec_BmpModule::~CCodec_BmpModule() {}

std::unique_ptr<CCodec_BmpModule::Context> CCodec_BmpModule::Start(
    Delegate* pDelegate) {
  bmp_decompress_struct_p pBmp = bmp_create_decompress();
  if (!pBmp)
    return nullptr;

  auto p = pdfium::MakeUnique<CBmpContext>(pBmp, this, pDelegate);
  p->m_pBmp->context_ptr = p.get();
  p->m_pBmp->err_ptr = p->m_szLastError;
  p->m_pBmp->bmp_error_fn = bmp_error_data;
  p->m_pBmp->bmp_get_row_fn = bmp_read_scanline;
  p->m_pBmp->bmp_get_data_position_fn = bmp_get_data_position;
  return p;
}

int32_t CCodec_BmpModule::ReadHeader(Context* pContext,
                                     int32_t* width,
                                     int32_t* height,
                                     bool* tb_flag,
                                     int32_t* components,
                                     int32_t* pal_num,
                                     uint32_t** pal_pp,
                                     CFX_DIBAttribute* pAttribute) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  if (setjmp(ctx->m_pBmp->jmpbuf)) {
    return 0;
  }
  int32_t ret = bmp_read_header(ctx->m_pBmp);
  if (ret != 1) {
    return ret;
  }
  *width = ctx->m_pBmp->width;
  *height = ctx->m_pBmp->height;
  *tb_flag = ctx->m_pBmp->imgTB_flag;
  *components = ctx->m_pBmp->components;
  *pal_num = ctx->m_pBmp->pal_num;
  *pal_pp = ctx->m_pBmp->pal_ptr;
  if (pAttribute) {
    pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_METER;
    pAttribute->m_nXDPI = ctx->m_pBmp->dpi_x;
    pAttribute->m_nYDPI = ctx->m_pBmp->dpi_y;
    pAttribute->m_nBmpCompressType = ctx->m_pBmp->compress_flag;
  }
  return 1;
}

int32_t CCodec_BmpModule::LoadImage(Context* pContext) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  if (setjmp(ctx->m_pBmp->jmpbuf))
    return 0;
  return bmp_decode_image(ctx->m_pBmp);
}

uint32_t CCodec_BmpModule::GetAvailInput(Context* pContext,
                                         uint8_t** avail_buf_ptr) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  return bmp_get_avail_input(ctx->m_pBmp, avail_buf_ptr);
}

void CCodec_BmpModule::Input(Context* pContext,
                             const uint8_t* src_buf,
                             uint32_t src_size) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  bmp_input_buffer(ctx->m_pBmp, (uint8_t*)src_buf, src_size);
}
