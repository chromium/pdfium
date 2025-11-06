// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/png/libpng_png_decoder.h"

#include <setjmp.h>
#include <string.h>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcodec/png/png_decoder_delegate.h"
#include "core/fxcodec/progressive_decoder_context.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/unowned_ptr.h"

#ifdef USE_SYSTEM_LIBPNG
#include <png.h>
#else
#include "third_party/libpng/png.h"
#endif

#ifdef PDF_ENABLE_RUST_PNG
#error "If Rust PNG is enabled, then `libpng` should not be used."
#endif

#define PNG_ERROR_SIZE 256

using PngDecoderDelegate = fxcodec::PngDecoderDelegate;

class CPngContext final : public ProgressiveDecoderContext {
 public:
  explicit CPngContext(PngDecoderDelegate* pDelegate);
  ~CPngContext() override;

  png_structp png_ = nullptr;
  png_infop info_ = nullptr;
  UnownedPtr<PngDecoderDelegate> const delegate_;
  char last_error_[PNG_ERROR_SIZE] = {};
  png_uint_32 height_ = 0;
  int number_of_passes_ = 0;
};

extern "C" {

void _png_error_data(png_structp png_ptr, png_const_charp error_msg) {
  if (png_get_error_ptr(png_ptr)) {
    UNSAFE_TODO(strncpy(static_cast<char*>(png_get_error_ptr(png_ptr)),
                        error_msg, PNG_ERROR_SIZE - 1));
  }

  longjmp(png_jmpbuf(png_ptr), 1);
}

void _png_warning_data(png_structp png_ptr, png_const_charp error_msg) {}

void _png_get_header_func(png_structp png_ptr, png_infop info_ptr) {
  auto* context =
      reinterpret_cast<CPngContext*>(png_get_progressive_ptr(png_ptr));
  if (!context) {
    return;
  }

  png_uint_32 width = 0;
  png_uint_32 height = 0;
  int bits_per_component = 0;
  int libpng_color_type = 0;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bits_per_component,
               &libpng_color_type, nullptr, nullptr, nullptr);
  if (bits_per_component > 8) {
    png_set_strip_16(png_ptr);
  } else if (bits_per_component < 8) {
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  }

  if (libpng_color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  }

  context->number_of_passes_ = png_set_interlace_handling(png_ptr);
  context->height_ = height;

  double gamma = 1.0;
  if (!context->delegate_->PngReadHeader(width, height, &gamma)) {
    // Note that `png_error` function is marked as `PNG_NORETURN`.
    png_error(context->png_, "Read Header Callback Error");
  }
  int intent;
  if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
    png_set_gamma(png_ptr, gamma, 0.45455);
  } else {
    double image_gamma;
    if (png_get_gAMA(png_ptr, info_ptr, &image_gamma)) {
      png_set_gamma(png_ptr, gamma, image_gamma);
    } else {
      png_set_gamma(png_ptr, gamma, 0.45455);
    }
  }
  if (!(libpng_color_type & PNG_COLOR_MASK_COLOR)) {
    png_set_gray_to_rgb(png_ptr);
  }
  png_set_bgr(png_ptr);
  if (!(libpng_color_type & PNG_COLOR_MASK_ALPHA)) {
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
  }
  png_read_update_info(png_ptr, info_ptr);
}

void _png_get_end_func(png_structp png_ptr, png_infop info_ptr) {}

void _png_get_row_func(png_structp png_ptr,
                       png_bytep new_row,
                       png_uint_32 row_num,
                       int pass) {
  auto* context =
      reinterpret_cast<CPngContext*>(png_get_progressive_ptr(png_ptr));
  if (!context) {
    return;
  }

  pdfium::span<uint8_t> dst_buf =
      context->delegate_->PngAskScanlineBuf(row_num);
  CHECK(!dst_buf.empty());
  png_progressive_combine_row(png_ptr, dst_buf.data(), new_row);

  if ((pass == (context->number_of_passes_ - 1)) &&
      (row_num == (context->height_ - 1))) {
    context->delegate_->PngFinishedDecoding();
  }
}

int _png_set_read_and_error_fns(png_structrp png_ptr,
                                void* user_ctx,
                                char* error_buf) {
  if (setjmp(png_jmpbuf(png_ptr))) {
    return 0;
  }
  png_set_progressive_read_fn(png_ptr, user_ctx, _png_get_header_func,
                              _png_get_row_func, _png_get_end_func);
  png_set_error_fn(png_ptr, error_buf, _png_error_data, _png_warning_data);
  return 1;
}

int _png_continue_decode(png_structrp png_ptr,
                         png_inforp info_ptr,
                         uint8_t* buffer,
                         size_t size) {
  if (setjmp(png_jmpbuf(png_ptr))) {
    return 0;
  }
  png_process_data(png_ptr, info_ptr, buffer, size);
  return 1;
}

}  // extern "C"

CPngContext::CPngContext(PngDecoderDelegate* pDelegate)
    : delegate_(pDelegate) {}

CPngContext::~CPngContext() {
  png_destroy_read_struct(png_ ? &png_ : nullptr, info_ ? &info_ : nullptr,
                          nullptr);
}

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderContext> LibpngPngDecoder::StartDecode(
    PngDecoderDelegate* pDelegate) {
  auto p = std::make_unique<CPngContext>(pDelegate);
  p->png_ =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!p->png_) {
    return nullptr;
  }
  p->info_ = png_create_info_struct(p->png_);
  if (!p->info_) {
    return nullptr;
  }
  if (!_png_set_read_and_error_fns(p->png_, p.get(), p->last_error_)) {
    return nullptr;
  }
  return p;
}

// static
bool LibpngPngDecoder::ContinueDecode(ProgressiveDecoderContext* context,
                                      RetainPtr<CFX_CodecMemory> codec_memory) {
  auto* ctx = static_cast<CPngContext*>(context);
  pdfium::span<uint8_t> src_buf = codec_memory->GetUnconsumedSpan();
  bool result = _png_continue_decode(ctx->png_, ctx->info_, src_buf.data(),
                                     src_buf.size());

  // `libpng` always consumes all the data from `src_buf`, so
  // advance/seek `codec_memory` to the end of the buffer.
  codec_memory->Seek(codec_memory->GetSize());
  CHECK(codec_memory->GetUnconsumedSpan().empty());

  return result;
}

}  // namespace fxcodec
