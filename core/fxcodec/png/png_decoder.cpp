// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/png/png_decoder.h"

#include <setjmp.h>
#include <string.h>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/unowned_ptr.h"

#ifdef USE_SYSTEM_LIBPNG
#include <png.h>
#else
#include "third_party/libpng/png.h"
#endif

#define PNG_ERROR_SIZE 256

class CPngContext final : public ProgressiveDecoderIface::Context {
 public:
  explicit CPngContext(PngDecoder::Delegate* pDelegate);
  ~CPngContext() override;

  png_structp png_ = nullptr;
  png_infop info_ = nullptr;
  UnownedPtr<PngDecoder::Delegate> const delegate_;
  char last_error_[PNG_ERROR_SIZE] = {};
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

void _png_load_bmp_attribute(png_structp png_ptr,
                             png_infop info_ptr,
                             CFX_DIBAttribute* pAttribute) {
  if (pAttribute) {
#if defined(PNG_pHYs_SUPPORTED)
    pAttribute->x_dpi_ = png_get_x_pixels_per_meter(png_ptr, info_ptr);
    pAttribute->y_dpi_ = png_get_y_pixels_per_meter(png_ptr, info_ptr);
    png_uint_32 res_x, res_y;
    int unit_type;
    png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type);
    switch (unit_type) {
      case PNG_RESOLUTION_METER:
        pAttribute->dpi_unit_ = CFX_DIBAttribute::kResUnitMeter;
        break;
      default:
        pAttribute->dpi_unit_ = CFX_DIBAttribute::kResUnitNone;
    }
#endif
#if defined(PNG_iCCP_SUPPORTED)
    png_charp icc_name;
    png_bytep icc_profile;
    png_uint_32 icc_proflen;
    int compress_type;
    png_get_iCCP(png_ptr, info_ptr, &icc_name, &compress_type, &icc_profile,
                 &icc_proflen);
#endif
#if defined(PNG_TEXT_SUPPORTED)
    int num_text;
    png_textp text = nullptr;
    png_get_text(png_ptr, info_ptr, &text, &num_text);
#endif
  }
}

void _png_get_header_func(png_structp png_ptr, png_infop info_ptr) {
  auto* pContext =
      reinterpret_cast<CPngContext*>(png_get_progressive_ptr(png_ptr));
  if (!pContext) {
    return;
  }

  png_uint_32 width = 0;
  png_uint_32 height = 0;
  int bpc = 0;
  int color_type = 0;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpc, &color_type, nullptr,
               nullptr, nullptr);
  int color_type1 = color_type;
  if (bpc > 8) {
    png_set_strip_16(png_ptr);
  } else if (bpc < 8) {
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  }

  bpc = 8;
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  }

  int pass = png_set_interlace_handling(png_ptr);
  double gamma = 1.0;
  if (!pContext->delegate_->PngReadHeader(width, height, bpc, pass, &color_type,
                                          &gamma)) {
    png_error(pContext->png_, "Read Header Callback Error");
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
  switch (color_type) {
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA: {
      if (color_type1 & PNG_COLOR_MASK_COLOR) {
        png_set_rgb_to_gray(png_ptr, 1, 0.299, 0.587);
      }
    } break;
    case PNG_COLOR_TYPE_PALETTE:
      if (color_type1 != PNG_COLOR_TYPE_PALETTE) {
        png_error(pContext->png_, "Not Support Output Palette Now");
      }
      [[fallthrough]];
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      if (!(color_type1 & PNG_COLOR_MASK_COLOR)) {
        png_set_gray_to_rgb(png_ptr);
      }
      png_set_bgr(png_ptr);
      break;
  }
  if (!(color_type & PNG_COLOR_MASK_ALPHA)) {
    png_set_strip_alpha(png_ptr);
  }

  if (color_type & PNG_COLOR_MASK_ALPHA &&
      !(color_type1 & PNG_COLOR_MASK_ALPHA)) {
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
  }
  png_read_update_info(png_ptr, info_ptr);
}

void _png_get_end_func(png_structp png_ptr, png_infop info_ptr) {}

void _png_get_row_func(png_structp png_ptr,
                       png_bytep new_row,
                       png_uint_32 row_num,
                       int pass) {
  auto* pContext =
      reinterpret_cast<CPngContext*>(png_get_progressive_ptr(png_ptr));
  if (!pContext) {
    return;
  }

  uint8_t* src_buf = pContext->delegate_->PngAskScanlineBuf(row_num);
  CHECK(src_buf);
  png_progressive_combine_row(png_ptr, src_buf, new_row);

  pContext->delegate_->PngFillScanlineBufCompleted(pass, row_num);
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

CPngContext::CPngContext(PngDecoder::Delegate* pDelegate)
    : delegate_(pDelegate) {}

CPngContext::~CPngContext() {
  png_destroy_read_struct(png_ ? &png_ : nullptr, info_ ? &info_ : nullptr,
                          nullptr);
}

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderIface::Context> PngDecoder::StartDecode(
    Delegate* pDelegate) {
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
bool PngDecoder::ContinueDecode(ProgressiveDecoderIface::Context* pContext,
                                RetainPtr<CFX_CodecMemory> codec_memory,
                                CFX_DIBAttribute* pAttribute) {
  auto* ctx = static_cast<CPngContext*>(pContext);
  pdfium::span<uint8_t> src_buf = codec_memory->GetUnconsumedSpan();
  if (!_png_continue_decode(ctx->png_, ctx->info_, src_buf.data(),
                            src_buf.size())) {
    if (pAttribute && UNSAFE_TODO(strcmp(ctx->last_error_,
                                         "Read Header Callback Error")) == 0) {
      _png_load_bmp_attribute(ctx->png_, ctx->info_, pAttribute);
    }
    return false;
  }
  return true;
}

}  // namespace fxcodec
