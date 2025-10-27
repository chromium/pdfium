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

  JpegCommon common_ = {};
};

static void JpegLoadAttribute(const jpeg_decompress_struct& info,
                              CFX_DIBAttribute* pAttribute) {
  pAttribute->x_dpi_ = info.X_density;
  pAttribute->y_dpi_ = info.Y_density;
  pAttribute->dpi_unit_ =
      static_cast<CFX_DIBAttribute::ResUnit>(info.density_unit);
}

CJpegContext::CJpegContext() {
  common_.cinfo.client_data = &common_;
  common_.cinfo.err = &common_.error_mgr;

  common_.error_mgr.error_exit = jpeg_common_error_fatal;
  common_.error_mgr.emit_message = jpeg_common_error_do_nothing_int;
  common_.error_mgr.output_message = jpeg_common_error_do_nothing;
  common_.error_mgr.format_message = jpeg_common_error_do_nothing_char;
  common_.error_mgr.reset_error_mgr = jpeg_common_error_do_nothing;

  common_.source_mgr.init_source = jpeg_common_src_do_nothing;
  common_.source_mgr.term_source = jpeg_common_src_do_nothing;
  common_.source_mgr.skip_input_data = jpeg_common_src_skip_data_or_record;
  common_.source_mgr.fill_input_buffer = jpeg_common_src_fill_buffer;
  common_.source_mgr.resync_to_restart = jpeg_common_src_resync;
}

CJpegContext::~CJpegContext() {
  jpeg_destroy_decompress(&common_.cinfo);
}

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderIface::Context>
JpegProgressiveDecoder::Start() {
  auto pContext = std::make_unique<CJpegContext>();
  if (!jpeg_common_create_decompress(&pContext->common_)) {
    return nullptr;
  }
  pContext->common_.cinfo.src = &pContext->common_.source_mgr;
  pContext->common_.skip_size = 0;
  return pContext;
}

// static
int JpegProgressiveDecoder::ReadHeader(
    ProgressiveDecoderIface::Context* pContext,
    int* width,
    int* height,
    int* nComps,
    CFX_DIBAttribute* pAttribute) {
  DCHECK(pAttribute);

  auto* ctx = static_cast<CJpegContext*>(pContext);
  int ret = jpeg_common_read_header(&ctx->common_, TRUE);
  if (ret == -1) {
    return kFatal;
  }
  if (ret == JPEG_SUSPENDED) {
    return kNeedsMoreInput;
  }
  if (ret != JPEG_HEADER_OK) {
    return kError;
  }
  *width = ctx->common_.cinfo.image_width;
  *height = ctx->common_.cinfo.image_height;
  *nComps = ctx->common_.cinfo.num_components;
  JpegLoadAttribute(ctx->common_.cinfo, pAttribute);
  return kOk;
}

// static
bool JpegProgressiveDecoder::StartScanline(
    ProgressiveDecoderIface::Context* pContext) {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  ctx->common_.cinfo.scale_denom = 1;
  return !!jpeg_common_start_decompress(&ctx->common_);
}

// static
int JpegProgressiveDecoder::ReadScanline(
    ProgressiveDecoderIface::Context* pContext,
    unsigned char* dest_buf) {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  int nlines = jpeg_common_read_scanlines(&ctx->common_, &dest_buf, 1);
  if (nlines == -1) {
    return kFatal;
  }
  return nlines == 1 ? kOk : kError;
}

// static
FX_FILESIZE JpegProgressiveDecoder::GetAvailInput(
    ProgressiveDecoderIface::Context* pContext) {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  return static_cast<FX_FILESIZE>(ctx->common_.source_mgr.bytes_in_buffer);
}

// static
bool JpegProgressiveDecoder::Input(ProgressiveDecoderIface::Context* pContext,
                                   RetainPtr<CFX_CodecMemory> codec_memory) {
  pdfium::span<uint8_t> src_buf = codec_memory->GetUnconsumedSpan();
  auto* ctx = static_cast<CJpegContext*>(pContext);
  if (ctx->common_.skip_size) {
    if (ctx->common_.skip_size > src_buf.size()) {
      ctx->common_.source_mgr.bytes_in_buffer = 0;
      ctx->common_.skip_size -= src_buf.size();
      return true;
    }
    src_buf = src_buf.subspan(ctx->common_.skip_size);
    ctx->common_.skip_size = 0;
  }
  ctx->common_.source_mgr.next_input_byte = src_buf.data();
  ctx->common_.source_mgr.bytes_in_buffer = src_buf.size();
  return true;
}

}  // namespace fxcodec
