// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/jbig2module.h"

#include "core/fxcodec/jbig2/JBig2_Context.h"
#include "core/fxcodec/jbig2/JBig2_DocumentContext.h"
#include "third_party/base/ptr_util.h"

namespace fxcodec {

JBig2_DocumentContext* GetJBig2DocumentContext(
    std::unique_ptr<JBig2_DocumentContext>* pContextHolder) {
  if (!*pContextHolder)
    *pContextHolder = pdfium::MakeUnique<JBig2_DocumentContext>();
  return pContextHolder->get();
}

Jbig2Context::Jbig2Context() = default;

Jbig2Context::~Jbig2Context() = default;

Jbig2Module::Jbig2Module() = default;

Jbig2Module::~Jbig2Module() = default;

FXCODEC_STATUS Jbig2Module::StartDecode(
    Jbig2Context* pJbig2Context,
    std::unique_ptr<JBig2_DocumentContext>* pContextHolder,
    uint32_t width,
    uint32_t height,
    pdfium::span<const uint8_t> src_span,
    uint32_t src_objnum,
    pdfium::span<const uint8_t> global_span,
    uint32_t global_objnum,
    uint8_t* dest_buf,
    uint32_t dest_pitch,
    PauseIndicatorIface* pPause) {
  if (!pJbig2Context)
    return FXCODEC_STATUS_ERR_PARAMS;

  JBig2_DocumentContext* pJBig2DocumentContext =
      GetJBig2DocumentContext(pContextHolder);
  pJbig2Context->m_width = width;
  pJbig2Context->m_height = height;
  pJbig2Context->m_pSrcSpan = src_span;
  pJbig2Context->m_nSrcObjNum = src_objnum;
  pJbig2Context->m_pGlobalSpan = global_span;
  pJbig2Context->m_nGlobalObjNum = global_objnum;
  pJbig2Context->m_dest_buf = dest_buf;
  pJbig2Context->m_dest_pitch = dest_pitch;
  memset(dest_buf, 0, height * dest_pitch);
  pJbig2Context->m_pContext =
      CJBig2_Context::Create(global_span, global_objnum, src_span, src_objnum,
                             pJBig2DocumentContext->GetSymbolDictCache());
  bool succeeded = pJbig2Context->m_pContext->GetFirstPage(
      dest_buf, width, height, dest_pitch, pPause);
  return Decode(pJbig2Context, succeeded);
}

FXCODEC_STATUS Jbig2Module::ContinueDecode(Jbig2Context* pJbig2Context,
                                           PauseIndicatorIface* pPause) {
  bool succeeded = pJbig2Context->m_pContext->Continue(pPause);
  return Decode(pJbig2Context, succeeded);
}

FXCODEC_STATUS Jbig2Module::Decode(Jbig2Context* pJbig2Context,
                                   bool decode_success) {
  FXCODEC_STATUS status = pJbig2Context->m_pContext->GetProcessingStatus();
  if (status != FXCODEC_STATUS_DECODE_FINISH)
    return status;

  pJbig2Context->m_pContext.reset();
  if (!decode_success)
    return FXCODEC_STATUS_ERROR;

  int dword_size = pJbig2Context->m_height * pJbig2Context->m_dest_pitch / 4;
  uint32_t* dword_buf = reinterpret_cast<uint32_t*>(pJbig2Context->m_dest_buf);
  for (int i = 0; i < dword_size; i++)
    dword_buf[i] = ~dword_buf[i];
  return FXCODEC_STATUS_DECODE_FINISH;
}

}  // namespace fxcodec
