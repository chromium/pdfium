// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "core/include/fxcodec/fx_codec.h"
#include "core/include/fxge/fx_dib.h"
#include "codec_int.h"

extern "C" {
#undef FAR
#include "third_party/lpng_v163/png.h"
}

static void _png_error_data(png_structp png_ptr, png_const_charp error_msg) {
  if (png_get_error_ptr(png_ptr)) {
    FXSYS_strncpy((char*)png_get_error_ptr(png_ptr), error_msg,
                  PNG_ERROR_SIZE - 1);
  }
  longjmp(png_jmpbuf(png_ptr), 1);
}
static void _png_warning_data(png_structp png_ptr, png_const_charp error_msg) {}
static void _png_load_bmp_attribute(png_structp png_ptr,
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
        pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_METER;
        break;
      default:
        pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_NONE;
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
    int bTime = 0;
#if defined(PNG_tIME_SUPPORTED)
    png_timep t = NULL;
    png_get_tIME(png_ptr, info_ptr, &t);
    if (t) {
      FXSYS_memset(pAttribute->m_strTime, 0, sizeof(pAttribute->m_strTime));
      FXSYS_snprintf((FX_CHAR*)pAttribute->m_strTime,
                     sizeof(pAttribute->m_strTime), "%4d:%2d:%2d %2d:%2d:%2d",
                     t->year, t->month, t->day, t->hour, t->minute, t->second);
      pAttribute->m_strTime[sizeof(pAttribute->m_strTime) - 1] = 0;
      bTime = 1;
    }
#endif
#if defined(PNG_TEXT_SUPPORTED)
    int i;
    FX_STRSIZE len;
    const FX_CHAR* buf;
    int num_text;
    png_textp text = NULL;
    png_get_text(png_ptr, info_ptr, &text, &num_text);
    for (i = 0; i < num_text; i++) {
      len = FXSYS_strlen(text[i].key);
      buf = "Time";
      if (!FXSYS_memcmp(buf, text[i].key, std::min(len, FXSYS_strlen(buf)))) {
        if (!bTime) {
          FXSYS_memset(pAttribute->m_strTime, 0, sizeof(pAttribute->m_strTime));
          FXSYS_memcpy(
              pAttribute->m_strTime, text[i].text,
              std::min(sizeof(pAttribute->m_strTime) - 1, text[i].text_length));
        }
      } else {
        buf = "Author";
        if (!FXSYS_memcmp(buf, text[i].key, std::min(len, FXSYS_strlen(buf)))) {
          pAttribute->m_strAuthor.Empty();
          pAttribute->m_strAuthor.Load((uint8_t*)text[i].text,
                                       (FX_STRSIZE)text[i].text_length);
        }
      }
    }
#endif
  }
}
struct FXPNG_Context {
  png_structp png_ptr;
  png_infop info_ptr;
  void* parent_ptr;
  void* child_ptr;

  void* (*m_AllocFunc)(unsigned int);
  void (*m_FreeFunc)(void*);
};
extern "C" {
static void* _png_alloc_func(unsigned int size) {
  return FX_Alloc(char, size);
}
static void _png_free_func(void* p) {
  if (p != NULL) {
    FX_Free(p);
  }
}
};
static void _png_get_header_func(png_structp png_ptr, png_infop info_ptr) {
  FXPNG_Context* p = (FXPNG_Context*)png_get_progressive_ptr(png_ptr);
  if (p == NULL) {
    return;
  }
  CCodec_PngModule* pModule = (CCodec_PngModule*)p->parent_ptr;
  if (pModule == NULL) {
    return;
  }
  png_uint_32 width = 0, height = 0;
  int bpc = 0, color_type = 0, color_type1 = 0, pass = 0;
  double gamma = 1.0;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpc, &color_type, NULL,
               NULL, NULL);
  color_type1 = color_type;
  if (bpc > 8) {
    png_set_strip_16(png_ptr);
  } else if (bpc < 8) {
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  }
  bpc = 8;
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  }
  pass = png_set_interlace_handling(png_ptr);
  if (!pModule->ReadHeaderCallback(p->child_ptr, width, height, bpc, pass,
                                   &color_type, &gamma)) {
    png_error(p->png_ptr, "Read Header Callback Error");
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
        png_error(p->png_ptr, "Not Support Output Palette Now");
      }
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
static void _png_get_end_func(png_structp png_ptr, png_infop info_ptr) {}
static void _png_get_row_func(png_structp png_ptr,
                              png_bytep new_row,
                              png_uint_32 row_num,
                              int pass) {
  FXPNG_Context* p = (FXPNG_Context*)png_get_progressive_ptr(png_ptr);
  if (p == NULL) {
    return;
  }
  CCodec_PngModule* pModule = (CCodec_PngModule*)p->parent_ptr;
  uint8_t* src_buf = NULL;
  if (!pModule->AskScanlineBufCallback(p->child_ptr, row_num, src_buf)) {
    png_error(png_ptr, "Ask Scanline buffer Callback Error");
  }
  if (src_buf != NULL) {
    png_progressive_combine_row(png_ptr, src_buf, new_row);
  }
  pModule->FillScanlineBufCompletedCallback(p->child_ptr, pass, row_num);
}
void* CCodec_PngModule::Start(void* pModule) {
  FXPNG_Context* p = (FXPNG_Context*)FX_Alloc(uint8_t, sizeof(FXPNG_Context));
  if (p == NULL) {
    return NULL;
  }
  p->m_AllocFunc = _png_alloc_func;
  p->m_FreeFunc = _png_free_func;
  p->png_ptr = NULL;
  p->info_ptr = NULL;
  p->parent_ptr = (void*)this;
  p->child_ptr = pModule;
  p->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (p->png_ptr == NULL) {
    FX_Free(p);
    return NULL;
  }
  p->info_ptr = png_create_info_struct(p->png_ptr);
  if (p->info_ptr == NULL) {
    png_destroy_read_struct(&(p->png_ptr), (png_infopp)NULL, (png_infopp)NULL);
    FX_Free(p);
    return NULL;
  }
  if (setjmp(png_jmpbuf(p->png_ptr))) {
    if (p != NULL) {
      png_destroy_read_struct(&(p->png_ptr), &(p->info_ptr), (png_infopp)NULL);
      FX_Free(p);
    }
    return NULL;
  }
  png_set_progressive_read_fn(p->png_ptr, p, _png_get_header_func,
                              _png_get_row_func, _png_get_end_func);
  png_set_error_fn(p->png_ptr, m_szLastError, (png_error_ptr)_png_error_data,
                   (png_error_ptr)_png_warning_data);
  return p;
}
void CCodec_PngModule::Finish(void* pContext) {
  FXPNG_Context* p = (FXPNG_Context*)pContext;
  if (p != NULL) {
    png_destroy_read_struct(&(p->png_ptr), &(p->info_ptr), (png_infopp)NULL);
    p->m_FreeFunc(p);
  }
}
FX_BOOL CCodec_PngModule::Input(void* pContext,
                                const uint8_t* src_buf,
                                FX_DWORD src_size,
                                CFX_DIBAttribute* pAttribute) {
  FXPNG_Context* p = (FXPNG_Context*)pContext;
  if (setjmp(png_jmpbuf(p->png_ptr))) {
    if (pAttribute &&
        0 == FXSYS_strcmp(m_szLastError, "Read Header Callback Error")) {
      _png_load_bmp_attribute(p->png_ptr, p->info_ptr, pAttribute);
    }
    return FALSE;
  }
  png_process_data(p->png_ptr, p->info_ptr, (uint8_t*)src_buf, src_size);
  return TRUE;
}
