// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jpeg/jpeg_progressive_decoder.h"

#include <optional>
#include <utility>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/jpeg/jpeg_common.h"
#include "core/fxcodec/scanlinedecoder.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/fx_dib.h"

class CJpegContext final : public ProgressiveDecoderIface::Context {
 public:
  CJpegContext();
  ~CJpegContext() override;

  jmp_buf& GetJumpMark() { return m_Common.jmpbuf; }

  JpegCommon m_Common = {};
};

static void JpegLoadAttribute(const jpeg_decompress_struct& info,
                              CFX_DIBAttribute* pAttribute) {
  pAttribute->m_nXDPI = info.X_density;
  pAttribute->m_nYDPI = info.Y_density;
  pAttribute->m_wDPIUnit =
      static_cast<CFX_DIBAttribute::ResUnit>(info.density_unit);
}

CJpegContext::CJpegContext() {
  m_Common.cinfo.client_data = &m_Common;
  m_Common.cinfo.err = &m_Common.error_mgr;

  m_Common.error_mgr.error_exit = jpeg_common_error_fatal;
  m_Common.error_mgr.emit_message = jpeg_common_error_do_nothing_int;
  m_Common.error_mgr.output_message = jpeg_common_error_do_nothing;
  m_Common.error_mgr.format_message = jpeg_common_error_do_nothing_char;
  m_Common.error_mgr.reset_error_mgr = jpeg_common_error_do_nothing;

  m_Common.source_mgr.init_source = jpeg_common_src_do_nothing;
  m_Common.source_mgr.term_source = jpeg_common_src_do_nothing;
  m_Common.source_mgr.skip_input_data = jpeg_common_src_skip_data_or_record;
  m_Common.source_mgr.fill_input_buffer = jpeg_common_src_fill_buffer;
  m_Common.source_mgr.resync_to_restart = jpeg_common_src_resync;
}

CJpegContext::~CJpegContext() {
  jpeg_destroy_decompress(&m_Common.cinfo);
}

namespace fxcodec {

namespace {

JpegProgressiveDecoder* g_jpeg_decoder = nullptr;

}  // namespace

// static
void JpegProgressiveDecoder::InitializeGlobals() {
  CHECK(!g_jpeg_decoder);
  g_jpeg_decoder = new JpegProgressiveDecoder();
}

// static
void JpegProgressiveDecoder::DestroyGlobals() {
  delete g_jpeg_decoder;
  g_jpeg_decoder = nullptr;
}

// static
JpegProgressiveDecoder* JpegProgressiveDecoder::GetInstance() {
  return g_jpeg_decoder;
}

// static
std::unique_ptr<ProgressiveDecoderIface::Context>
JpegProgressiveDecoder::Start() {
  auto pContext = std::make_unique<CJpegContext>();
  if (!jpeg_common_create_decompress(&pContext->m_Common)) {
    return nullptr;
  }
  pContext->m_Common.cinfo.src = &pContext->m_Common.source_mgr;
  pContext->m_Common.skip_size = 0;
  return pContext;
}

// static
jmp_buf& JpegProgressiveDecoder::GetJumpMark(Context* pContext) {
  return static_cast<CJpegContext*>(pContext)->GetJumpMark();
}

// static
int JpegProgressiveDecoder::ReadHeader(Context* pContext,
                                       int* width,
                                       int* height,
                                       int* nComps,
                                       CFX_DIBAttribute* pAttribute) {
  DCHECK(pAttribute);

  auto* ctx = static_cast<CJpegContext*>(pContext);
  int ret = jpeg_common_read_header(&ctx->m_Common, TRUE);
  if (ret == -1) {
    return kFatal;
  }
  if (ret == JPEG_SUSPENDED) {
    return kNeedsMoreInput;
  }
  if (ret != JPEG_HEADER_OK) {
    return kError;
  }
  *width = ctx->m_Common.cinfo.image_width;
  *height = ctx->m_Common.cinfo.image_height;
  *nComps = ctx->m_Common.cinfo.num_components;
  JpegLoadAttribute(ctx->m_Common.cinfo, pAttribute);
  return kOk;
}

// static
bool JpegProgressiveDecoder::StartScanline(Context* pContext) {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  ctx->m_Common.cinfo.scale_denom = 1;
  return !!jpeg_start_decompress(&ctx->m_Common.cinfo);
}

// static
bool JpegProgressiveDecoder::ReadScanline(Context* pContext,
                                          unsigned char* dest_buf) {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  unsigned int nlines = jpeg_read_scanlines(&ctx->m_Common.cinfo, &dest_buf, 1);
  return nlines == 1;
}

FX_FILESIZE JpegProgressiveDecoder::GetAvailInput(Context* pContext) const {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  return static_cast<FX_FILESIZE>(ctx->m_Common.source_mgr.bytes_in_buffer);
}

bool JpegProgressiveDecoder::Input(Context* pContext,
                                   RetainPtr<CFX_CodecMemory> codec_memory) {
  pdfium::span<uint8_t> src_buf = codec_memory->GetUnconsumedSpan();
  auto* ctx = static_cast<CJpegContext*>(pContext);
  if (ctx->m_Common.skip_size) {
    if (ctx->m_Common.skip_size > src_buf.size()) {
      ctx->m_Common.source_mgr.bytes_in_buffer = 0;
      ctx->m_Common.skip_size -= src_buf.size();
      return true;
    }
    src_buf = src_buf.subspan(ctx->m_Common.skip_size);
    ctx->m_Common.skip_size = 0;
  }
  ctx->m_Common.source_mgr.next_input_byte = src_buf.data();
  ctx->m_Common.source_mgr.bytes_in_buffer = src_buf.size();
  return true;
}

JpegProgressiveDecoder::JpegProgressiveDecoder() = default;

JpegProgressiveDecoder::~JpegProgressiveDecoder() = default;

}  // namespace fxcodec
