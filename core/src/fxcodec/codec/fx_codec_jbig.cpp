// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxcodec/fx_codec.h"
#include "codec_int.h"

CCodec_Jbig2Context::CCodec_Jbig2Context() {
  FXSYS_memset(this, 0, sizeof(CCodec_Jbig2Context));
}

CCodec_Jbig2Module::~CCodec_Jbig2Module() {
  for (auto it : m_SymbolDictCache) {
    delete it.second;
  }
}

void* CCodec_Jbig2Module::CreateJbig2Context() {
  return new CCodec_Jbig2Context();
}
void CCodec_Jbig2Module::DestroyJbig2Context(void* pJbig2Content) {
  if (pJbig2Content) {
    CJBig2_Context::DestroyContext(
        ((CCodec_Jbig2Context*)pJbig2Content)->m_pContext);
    delete (CCodec_Jbig2Context*)pJbig2Content;
  }
  pJbig2Content = NULL;
}
FXCODEC_STATUS CCodec_Jbig2Module::StartDecode(void* pJbig2Context,
                                               FX_DWORD width,
                                               FX_DWORD height,
                                               const uint8_t* src_buf,
                                               FX_DWORD src_size,
                                               const uint8_t* global_data,
                                               FX_DWORD global_size,
                                               uint8_t* dest_buf,
                                               FX_DWORD dest_pitch,
                                               IFX_Pause* pPause) {
  if (!pJbig2Context) {
    return FXCODEC_STATUS_ERR_PARAMS;
  }
  CCodec_Jbig2Context* m_pJbig2Context = (CCodec_Jbig2Context*)pJbig2Context;
  m_pJbig2Context->m_width = width;
  m_pJbig2Context->m_height = height;
  m_pJbig2Context->m_src_buf = (unsigned char*)src_buf;
  m_pJbig2Context->m_src_size = src_size;
  m_pJbig2Context->m_global_data = global_data;
  m_pJbig2Context->m_global_size = global_size;
  m_pJbig2Context->m_dest_buf = dest_buf;
  m_pJbig2Context->m_dest_pitch = dest_pitch;
  m_pJbig2Context->m_pPause = pPause;
  m_pJbig2Context->m_bFileReader = FALSE;
  FXSYS_memset(dest_buf, 0, height * dest_pitch);
  m_pJbig2Context->m_pContext = CJBig2_Context::CreateContext(
      global_data, global_size, src_buf, src_size, &m_SymbolDictCache, pPause);
  if (!m_pJbig2Context->m_pContext) {
    return FXCODEC_STATUS_ERROR;
  }
  int ret = m_pJbig2Context->m_pContext->getFirstPage(dest_buf, width, height,
                                                      dest_pitch, pPause);
  if (m_pJbig2Context->m_pContext->GetProcessingStatus() ==
      FXCODEC_STATUS_DECODE_FINISH) {
    CJBig2_Context::DestroyContext(m_pJbig2Context->m_pContext);
    m_pJbig2Context->m_pContext = NULL;
    if (ret != JBIG2_SUCCESS) {
      return FXCODEC_STATUS_ERROR;
    }
    int dword_size = height * dest_pitch / 4;
    FX_DWORD* dword_buf = (FX_DWORD*)dest_buf;
    for (int i = 0; i < dword_size; i++) {
      dword_buf[i] = ~dword_buf[i];
    }
    return FXCODEC_STATUS_DECODE_FINISH;
  }
  return m_pJbig2Context->m_pContext->GetProcessingStatus();
}
FXCODEC_STATUS CCodec_Jbig2Module::ContinueDecode(void* pJbig2Context,
                                                  IFX_Pause* pPause) {
  CCodec_Jbig2Context* m_pJbig2Context = (CCodec_Jbig2Context*)pJbig2Context;
  int ret = m_pJbig2Context->m_pContext->Continue(pPause);
  if (m_pJbig2Context->m_pContext->GetProcessingStatus() !=
      FXCODEC_STATUS_DECODE_FINISH) {
    return m_pJbig2Context->m_pContext->GetProcessingStatus();
  }
  if (m_pJbig2Context->m_bFileReader) {
    CJBig2_Context::DestroyContext(m_pJbig2Context->m_pContext);
    m_pJbig2Context->m_pContext = NULL;
    if (ret != JBIG2_SUCCESS) {
      FX_Free(m_pJbig2Context->m_src_buf);
      m_pJbig2Context->m_src_buf = NULL;
      return FXCODEC_STATUS_ERROR;
    }
    delete m_pJbig2Context->m_dest_image;
    FX_Free(m_pJbig2Context->m_src_buf);
    return FXCODEC_STATUS_DECODE_FINISH;
  }
  CJBig2_Context::DestroyContext(m_pJbig2Context->m_pContext);
  m_pJbig2Context->m_pContext = NULL;
  if (ret != JBIG2_SUCCESS) {
    return FXCODEC_STATUS_ERROR;
  }
  int dword_size =
      m_pJbig2Context->m_height * m_pJbig2Context->m_dest_pitch / 4;
  FX_DWORD* dword_buf = (FX_DWORD*)m_pJbig2Context->m_dest_buf;
  for (int i = 0; i < dword_size; i++) {
    dword_buf[i] = ~dword_buf[i];
  }
  return FXCODEC_STATUS_DECODE_FINISH;
}
