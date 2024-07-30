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

  png_structp m_pPng = nullptr;
  png_infop m_pInfo = nullptr;
  UnownedPtr<PngDecoder::Delegate> const m_pDelegate;
  char m_szLastError[PNG_ERROR_SIZE] = {};
};

extern "C" {

void _png_error_data(png_structp png_ptr, png_const_charp error_msg) {
  if (png_get_error_ptr(png_ptr)) {
    strncpy(static_cast<char*>(png_get_error_ptr(png_ptr)), error_msg,
            PNG_ERROR_SIZE - 1);
  }

  longjmp(png_jmpbuf(png_ptr), 1);
}

void _png_warning_data(png_structp png_ptr, png_const_charp error_msg) {}

void _png_load_bmp_attribute(png_structp png_ptr,
                             png_infop info_ptr,
                             CFX_DIBAttribute* pAttribute) {
  if (pAttribute) {
#if defined(PNG_pHYs_SUPPORTED)
    pAttribute->m_nXDPI = png_get_x_pixels_per_meter(png_ptr, info_ptr);
    pAttribute->m_nYDPI = png_get_y_pixels_per_meter(png_ptr, info_ptr);
    png_uint_32 res_x, res_y;
    int unit_type;
    png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type);
    switch (unit_type) {
      case PNG_RESOLUTION_METER:
        pAttribute->m_wDPIUnit = CFX_DIBAttribute::kResUnitMeter;
        break;
      default:
        pAttribute->m_wDPIUnit = CFX_DIBAttribute::kResUnitNone;
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
  if (!pContext)
    return;

  png_uint_32 width = 0;
  png_uint_32 height = 0;
  int bpc = 0;
  int color_type = 0;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpc, &color_type, nullptr,
               nullptr, nullptr);
  int color_type1 = color_type;
  if (bpc > 8)
    png_set_strip_16(png_ptr);
  else if (bpc < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr);

  bpc = 8;
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_ptr);

  int pass = png_set_interlace_handling(png_ptr);
  double gamma = 1.0;
  if (!pContext->m_pDelegate->PngReadHeader(width, height, bpc, pass,
                                            &color_type, &gamma)) {
    png_error(pContext->m_pPng, "Read Header Callback Error");
  }
  int intent;
  if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
    png_set_gamma(png_ptr, gamma, 0.45455);
  } else {
    double image_gamma;
    if (png_get_gAMA(png_ptr, info_ptr, &image_gamma))
      png_set_gamma(png_ptr, gamma, image_gamma);
    else
      png_set_gamma(png_ptr, gamma, 0.45455);
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
        png_error(pContext->m_pPng, "Not Support Output Palette Now");
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
  if (!(color_type & PNG_COLOR_MASK_ALPHA))
    png_set_strip_alpha(png_ptr);

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
  if (!pContext)
    return;

  uint8_t* src_buf = pContext->m_pDelegate->PngAskScanlineBuf(row_num);
  CHECK(src_buf);
  png_progressive_combine_row(png_ptr, src_buf, new_row);

  pContext->m_pDelegate->PngFillScanlineBufCompleted(pass, row_num);
}

}  // extern "C"

CPngContext::CPngContext(PngDecoder::Delegate* pDelegate)
    : m_pDelegate(pDelegate) {}

CPngContext::~CPngContext() {
  png_destroy_read_struct(m_pPng ? &m_pPng : nullptr,
                          m_pInfo ? &m_pInfo : nullptr, nullptr);
}

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderIface::Context> PngDecoder::StartDecode(
    Delegate* pDelegate) {
  auto p = std::make_unique<CPngContext>(pDelegate);
  p->m_pPng =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!p->m_pPng)
    return nullptr;

  p->m_pInfo = png_create_info_struct(p->m_pPng);
  if (!p->m_pInfo)
    return nullptr;

  if (setjmp(png_jmpbuf(p->m_pPng)))
    return nullptr;

  png_set_progressive_read_fn(p->m_pPng, p.get(), _png_get_header_func,
                              _png_get_row_func, _png_get_end_func);
  png_set_error_fn(p->m_pPng, p->m_szLastError, _png_error_data,
                   _png_warning_data);
  return p;
}

// static
bool PngDecoder::ContinueDecode(ProgressiveDecoderIface::Context* pContext,
                                RetainPtr<CFX_CodecMemory> codec_memory,
                                CFX_DIBAttribute* pAttribute) {
  auto* ctx = static_cast<CPngContext*>(pContext);
  if (setjmp(png_jmpbuf(ctx->m_pPng))) {
    if (pAttribute &&
        strcmp(ctx->m_szLastError, "Read Header Callback Error") == 0) {
      _png_load_bmp_attribute(ctx->m_pPng, ctx->m_pInfo, pAttribute);
    }
    return false;
  }
  pdfium::span<uint8_t> src_buf = codec_memory->GetUnconsumedSpan();
  png_process_data(ctx->m_pPng, ctx->m_pInfo, src_buf.data(), src_buf.size());
  return true;
}

}  // namespace fxcodec
