// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_imagetransformer.h"

#include <memory>
#include <utility>

#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

namespace {

uint8_t bilinear_interpol(const uint8_t* buf,
                          int row_offset_l,
                          int row_offset_r,
                          int src_col_l,
                          int src_col_r,
                          int res_x,
                          int res_y,
                          int bpp,
                          int c_offset) {
  int i_resx = 255 - res_x;
  int col_bpp_l = src_col_l * bpp;
  int col_bpp_r = src_col_r * bpp;
  const uint8_t* buf_u = buf + row_offset_l + c_offset;
  const uint8_t* buf_d = buf + row_offset_r + c_offset;
  const uint8_t* src_pos0 = buf_u + col_bpp_l;
  const uint8_t* src_pos1 = buf_u + col_bpp_r;
  const uint8_t* src_pos2 = buf_d + col_bpp_l;
  const uint8_t* src_pos3 = buf_d + col_bpp_r;
  uint8_t r_pos_0 = (*src_pos0 * i_resx + *src_pos1 * res_x) >> 8;
  uint8_t r_pos_1 = (*src_pos2 * i_resx + *src_pos3 * res_x) >> 8;
  return (r_pos_0 * (255 - res_y) + r_pos_1 * res_y) >> 8;
}

uint8_t bicubic_interpol(const uint8_t* buf,
                         int pitch,
                         int pos_pixel[],
                         int u_w[],
                         int v_w[],
                         int res_x,
                         int res_y,
                         int bpp,
                         int c_offset) {
  int s_result = 0;
  for (int i = 0; i < 4; i++) {
    int a_result = 0;
    for (int j = 0; j < 4; j++) {
      a_result += u_w[j] * (*(uint8_t*)(buf + pos_pixel[i + 4] * pitch +
                                        pos_pixel[j] * bpp + c_offset));
    }
    s_result += a_result * v_w[i];
  }
  s_result >>= 16;
  return static_cast<uint8_t>(pdfium::clamp(s_result, 0, 255));
}

void bicubic_get_pos_weight(int pos_pixel[],
                            int u_w[],
                            int v_w[],
                            int src_col_l,
                            int src_row_l,
                            int res_x,
                            int res_y,
                            int stretch_width,
                            int stretch_height) {
  pos_pixel[0] = src_col_l - 1;
  pos_pixel[1] = src_col_l;
  pos_pixel[2] = src_col_l + 1;
  pos_pixel[3] = src_col_l + 2;
  pos_pixel[4] = src_row_l - 1;
  pos_pixel[5] = src_row_l;
  pos_pixel[6] = src_row_l + 1;
  pos_pixel[7] = src_row_l + 2;
  for (int i = 0; i < 4; i++) {
    if (pos_pixel[i] < 0) {
      pos_pixel[i] = 0;
    }
    if (pos_pixel[i] >= stretch_width) {
      pos_pixel[i] = stretch_width - 1;
    }
    if (pos_pixel[i + 4] < 0) {
      pos_pixel[i + 4] = 0;
    }
    if (pos_pixel[i + 4] >= stretch_height) {
      pos_pixel[i + 4] = stretch_height - 1;
    }
  }
  u_w[0] = SDP_Table[256 + res_x];
  u_w[1] = SDP_Table[res_x];
  u_w[2] = SDP_Table[256 - res_x];
  u_w[3] = SDP_Table[512 - res_x];
  v_w[0] = SDP_Table[256 + res_y];
  v_w[1] = SDP_Table[res_y];
  v_w[2] = SDP_Table[256 - res_y];
  v_w[3] = SDP_Table[512 - res_y];
}

FXDIB_Format GetTransformedFormat(const CFX_RetainPtr<CFX_DIBSource>& pDrc) {
  FXDIB_Format format = pDrc->GetFormat();
  if (pDrc->IsAlphaMask()) {
    format = FXDIB_8bppMask;
  } else if (format >= 1025) {
    format = FXDIB_Cmyka;
  } else if (format <= 32 || format == FXDIB_Argb) {
    format = FXDIB_Argb;
  } else {
    format = FXDIB_Rgba;
  }
  return format;
}

class CPDF_FixedMatrix {
 public:
  CPDF_FixedMatrix(const CFX_Matrix& src, int bits) {
    base = 1 << bits;
    a = FXSYS_round(src.a * base);
    b = FXSYS_round(src.b * base);
    c = FXSYS_round(src.c * base);
    d = FXSYS_round(src.d * base);
    e = FXSYS_round(src.e * base);
    f = FXSYS_round(src.f * base);
  }

  void Transform(int x, int y, int* x1, int* y1) {
    *x1 = (a * x + c * y + e + base / 2) / base;
    *y1 = (b * x + d * y + f + base / 2) / base;
  }

  int a;
  int b;
  int c;
  int d;
  int e;
  int f;
  int base;
};

class CFX_BilinearMatrix : public CPDF_FixedMatrix {
 public:
  CFX_BilinearMatrix(const CFX_Matrix& src, int bits)
      : CPDF_FixedMatrix(src, bits) {}
  void Transform(int x, int y, int* x1, int* y1, int* res_x, int* res_y) {
    pdfium::base::CheckedNumeric<int> val = a;
    pdfium::base::CheckedNumeric<int> val2 = c;
    val *= x;
    val2 *= y;
    val += val2 + e + (base >> 1);
    *x1 = val.ValueOrDefault(0);

    val = b;
    val2 = d;
    val *= x;
    val2 *= y;
    val += val2 + f + (base >> 1);
    *y1 = val.ValueOrDefault(0);

    *res_x = *x1 % base;
    *res_y = *y1 % base;

    if (*res_x < 0 && *res_x > -base)
      *res_x = base + *res_x;
    if (*res_y < 0 && *res_x > -base)
      *res_y = base + *res_y;

    *x1 /= base;
    *y1 /= base;
  }
};

#define FIX16_005 0.05f

}  // namespace

CFX_ImageTransformer::CFX_ImageTransformer(
    const CFX_RetainPtr<CFX_DIBSource>& pSrc,
    const CFX_Matrix* pMatrix,
    int flags,
    const FX_RECT* pClip)
    : m_pSrc(pSrc),
      m_pMatrix(pMatrix),
      m_pClip(pClip),
      m_Flags(flags),
      m_Status(0) {
  FX_RECT result_rect = m_pMatrix->GetUnitRect().GetClosestRect();
  FX_RECT result_clip = result_rect;
  if (m_pClip)
    result_clip.Intersect(*m_pClip);

  if (result_clip.IsEmpty())
    return;

  m_result = result_clip;
  if (fabs(m_pMatrix->a) < fabs(m_pMatrix->b) / 20 &&
      fabs(m_pMatrix->d) < fabs(m_pMatrix->c) / 20 &&
      fabs(m_pMatrix->a) < 0.5f && fabs(m_pMatrix->d) < 0.5f) {
    int dest_width = result_rect.Width();
    int dest_height = result_rect.Height();
    result_clip.Offset(-result_rect.left, -result_rect.top);
    result_clip = FXDIB_SwapClipBox(result_clip, dest_width, dest_height,
                                    m_pMatrix->c > 0, m_pMatrix->b < 0);
    m_Stretcher = pdfium::MakeUnique<CFX_ImageStretcher>(
        &m_Storer, m_pSrc, dest_height, dest_width, result_clip, m_Flags);
    m_Stretcher->Start();
    m_Status = 1;
    return;
  }
  if (fabs(m_pMatrix->b) < FIX16_005 && fabs(m_pMatrix->c) < FIX16_005) {
    int dest_width = static_cast<int>(m_pMatrix->a > 0 ? ceil(m_pMatrix->a)
                                                       : floor(m_pMatrix->a));
    int dest_height = static_cast<int>(m_pMatrix->d > 0 ? -ceil(m_pMatrix->d)
                                                        : -floor(m_pMatrix->d));
    result_clip.Offset(-result_rect.left, -result_rect.top);
    m_Stretcher = pdfium::MakeUnique<CFX_ImageStretcher>(
        &m_Storer, m_pSrc, dest_width, dest_height, result_clip, m_Flags);
    m_Stretcher->Start();
    m_Status = 2;
    return;
  }
  int stretch_width =
      static_cast<int>(ceil(FXSYS_sqrt2(m_pMatrix->a, m_pMatrix->b)));
  int stretch_height =
      static_cast<int>(ceil(FXSYS_sqrt2(m_pMatrix->c, m_pMatrix->d)));
  CFX_Matrix stretch2dest(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, stretch_height);
  stretch2dest.Concat(
      CFX_Matrix(m_pMatrix->a / stretch_width, m_pMatrix->b / stretch_width,
                 m_pMatrix->c / stretch_height, m_pMatrix->d / stretch_height,
                 m_pMatrix->e, m_pMatrix->f));
  m_dest2stretch = stretch2dest.GetInverse();

  CFX_FloatRect clip_rect_f(result_clip);
  m_dest2stretch.TransformRect(clip_rect_f);
  m_StretchClip = clip_rect_f.GetOuterRect();
  m_StretchClip.Intersect(0, 0, stretch_width, stretch_height);
  m_Stretcher = pdfium::MakeUnique<CFX_ImageStretcher>(
      &m_Storer, m_pSrc, stretch_width, stretch_height, m_StretchClip, m_Flags);
  m_Stretcher->Start();
  m_Status = 3;
}

CFX_ImageTransformer::~CFX_ImageTransformer() {}

bool CFX_ImageTransformer::Continue(IFX_Pause* pPause) {
  if (m_Status == 1) {
    if (m_Stretcher->Continue(pPause))
      return true;

    if (m_Storer.GetBitmap()) {
      m_Storer.Replace(
          m_Storer.GetBitmap()->SwapXY(m_pMatrix->c > 0, m_pMatrix->b < 0));
    }
    return false;
  }

  if (m_Status == 2)
    return m_Stretcher->Continue(pPause);
  if (m_Status != 3)
    return false;
  if (m_Stretcher->Continue(pPause))
    return true;

  int stretch_width = m_StretchClip.Width();
  int stretch_height = m_StretchClip.Height();
  if (!m_Storer.GetBitmap())
    return false;

  const uint8_t* stretch_buf = m_Storer.GetBitmap()->GetBuffer();
  const uint8_t* stretch_buf_mask = nullptr;
  if (m_Storer.GetBitmap()->m_pAlphaMask)
    stretch_buf_mask = m_Storer.GetBitmap()->m_pAlphaMask->GetBuffer();

  int stretch_pitch = m_Storer.GetBitmap()->GetPitch();
  auto pTransformed = pdfium::MakeRetain<CFX_DIBitmap>();
  FXDIB_Format transformF = GetTransformedFormat(m_Stretcher->source());
  if (!pTransformed->Create(m_result.Width(), m_result.Height(), transformF))
    return false;

  pTransformed->Clear(0);
  if (pTransformed->m_pAlphaMask)
    pTransformed->m_pAlphaMask->Clear(0);

  CFX_Matrix result2stretch(1.0f, 0.0f, 0.0f, 1.0f, (float)(m_result.left),
                            (float)(m_result.top));
  result2stretch.Concat(m_dest2stretch);
  result2stretch.Translate(-m_StretchClip.left, -m_StretchClip.top);
  if (!stretch_buf_mask && pTransformed->m_pAlphaMask) {
    pTransformed->m_pAlphaMask->Clear(0xff000000);
  } else if (pTransformed->m_pAlphaMask) {
    int stretch_pitch_mask = m_Storer.GetBitmap()->m_pAlphaMask->GetPitch();
    if (!(m_Flags & FXDIB_DOWNSAMPLE) && !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
      CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
      for (int row = 0; row < m_result.Height(); row++) {
        uint8_t* dest_pos_mask =
            (uint8_t*)pTransformed->m_pAlphaMask->GetScanline(row);
        for (int col = 0; col < m_result.Width(); col++) {
          int src_col_l = 0;
          int src_row_l = 0;
          int res_x = 0;
          int res_y = 0;
          result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l, &res_x,
                                       &res_y);
          if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 &&
              src_row_l <= stretch_height) {
            if (src_col_l == stretch_width) {
              src_col_l--;
            }
            if (src_row_l == stretch_height) {
              src_row_l--;
            }
            int src_col_r = src_col_l + 1;
            int src_row_r = src_row_l + 1;
            if (src_col_r == stretch_width) {
              src_col_r--;
            }
            if (src_row_r == stretch_height) {
              src_row_r--;
            }
            int row_offset_l = src_row_l * stretch_pitch_mask;
            int row_offset_r = src_row_r * stretch_pitch_mask;
            *dest_pos_mask =
                bilinear_interpol(stretch_buf_mask, row_offset_l, row_offset_r,
                                  src_col_l, src_col_r, res_x, res_y, 1, 0);
          }
          dest_pos_mask++;
        }
      }
    } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
      CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
      for (int row = 0; row < m_result.Height(); row++) {
        uint8_t* dest_pos_mask =
            (uint8_t*)pTransformed->m_pAlphaMask->GetScanline(row);
        for (int col = 0; col < m_result.Width(); col++) {
          int src_col_l = 0;
          int src_row_l = 0;
          int res_x = 0;
          int res_y = 0;
          result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l, &res_x,
                                       &res_y);
          if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 &&
              src_row_l <= stretch_height) {
            int pos_pixel[8];
            int u_w[4], v_w[4];
            if (src_col_l == stretch_width) {
              src_col_l--;
            }
            if (src_row_l == stretch_height) {
              src_row_l--;
            }
            bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l,
                                   res_x, res_y, stretch_width, stretch_height);
            *dest_pos_mask =
                bicubic_interpol(stretch_buf_mask, stretch_pitch_mask,
                                 pos_pixel, u_w, v_w, res_x, res_y, 1, 0);
          }
          dest_pos_mask++;
        }
      }
    } else {
      CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
      for (int row = 0; row < m_result.Height(); row++) {
        uint8_t* dest_pos_mask =
            (uint8_t*)pTransformed->m_pAlphaMask->GetScanline(row);
        for (int col = 0; col < m_result.Width(); col++) {
          int src_col = 0;
          int src_row = 0;
          result2stretch_fix.Transform(col, row, &src_col, &src_row);
          if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 &&
              src_row <= stretch_height) {
            if (src_col == stretch_width) {
              src_col--;
            }
            if (src_row == stretch_height) {
              src_row--;
            }
            *dest_pos_mask =
                stretch_buf_mask[src_row * stretch_pitch_mask + src_col];
          }
          dest_pos_mask++;
        }
      }
    }
  }
  if (m_Storer.GetBitmap()->IsAlphaMask()) {
    if (!(m_Flags & FXDIB_DOWNSAMPLE) && !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
      CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
      for (int row = 0; row < m_result.Height(); row++) {
        uint8_t* dest_scan = (uint8_t*)pTransformed->GetScanline(row);
        for (int col = 0; col < m_result.Width(); col++) {
          int src_col_l = 0;
          int src_row_l = 0;
          int res_x = 0;
          int res_y = 0;
          result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l, &res_x,
                                       &res_y);
          if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 &&
              src_row_l <= stretch_height) {
            if (src_col_l == stretch_width) {
              src_col_l--;
            }
            if (src_row_l == stretch_height) {
              src_row_l--;
            }
            int src_col_r = src_col_l + 1;
            int src_row_r = src_row_l + 1;
            if (src_col_r == stretch_width) {
              src_col_r--;
            }
            if (src_row_r == stretch_height) {
              src_row_r--;
            }
            int row_offset_l = src_row_l * stretch_pitch;
            int row_offset_r = src_row_r * stretch_pitch;
            *dest_scan =
                bilinear_interpol(stretch_buf, row_offset_l, row_offset_r,
                                  src_col_l, src_col_r, res_x, res_y, 1, 0);
          }
          dest_scan++;
        }
      }
    } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
      CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
      for (int row = 0; row < m_result.Height(); row++) {
        uint8_t* dest_scan = (uint8_t*)pTransformed->GetScanline(row);
        for (int col = 0; col < m_result.Width(); col++) {
          int src_col_l = 0;
          int src_row_l = 0;
          int res_x = 0;
          int res_y = 0;
          result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l, &res_x,
                                       &res_y);
          if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 &&
              src_row_l <= stretch_height) {
            int pos_pixel[8];
            int u_w[4], v_w[4];
            if (src_col_l == stretch_width) {
              src_col_l--;
            }
            if (src_row_l == stretch_height) {
              src_row_l--;
            }
            bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l,
                                   res_x, res_y, stretch_width, stretch_height);
            *dest_scan = bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel,
                                          u_w, v_w, res_x, res_y, 1, 0);
          }
          dest_scan++;
        }
      }
    } else {
      CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
      for (int row = 0; row < m_result.Height(); row++) {
        uint8_t* dest_scan = (uint8_t*)pTransformed->GetScanline(row);
        for (int col = 0; col < m_result.Width(); col++) {
          int src_col = 0;
          int src_row = 0;
          result2stretch_fix.Transform(col, row, &src_col, &src_row);
          if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 &&
              src_row <= stretch_height) {
            if (src_col == stretch_width) {
              src_col--;
            }
            if (src_row == stretch_height) {
              src_row--;
            }
            const uint8_t* src_pixel =
                stretch_buf + stretch_pitch * src_row + src_col;
            *dest_scan = *src_pixel;
          }
          dest_scan++;
        }
      }
    }
  } else {
    int Bpp = m_Storer.GetBitmap()->GetBPP() / 8;
    if (Bpp == 1) {
      uint32_t argb[256];
      FX_ARGB* pPal = m_Storer.GetBitmap()->GetPalette();
      if (pPal) {
        for (int i = 0; i < 256; i++) {
          argb[i] = pPal[i];
        }
      } else {
        if (m_Storer.GetBitmap()->IsCmykImage()) {
          for (int i = 0; i < 256; i++) {
            argb[i] = 255 - i;
          }
        } else {
          for (int i = 0; i < 256; i++) {
            argb[i] = 0xff000000 | (i * 0x010101);
          }
        }
      }
      int destBpp = pTransformed->GetBPP() / 8;
      if (!(m_Flags & FXDIB_DOWNSAMPLE) &&
          !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
        CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
        for (int row = 0; row < m_result.Height(); row++) {
          uint8_t* dest_pos = (uint8_t*)pTransformed->GetScanline(row);
          for (int col = 0; col < m_result.Width(); col++) {
            int src_col_l = 0;
            int src_row_l = 0;
            int res_x = 0;
            int res_y = 0;
            result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l,
                                         &res_x, &res_y);
            if (src_col_l >= 0 && src_col_l <= stretch_width &&
                src_row_l >= 0 && src_row_l <= stretch_height) {
              if (src_col_l == stretch_width) {
                src_col_l--;
              }
              if (src_row_l == stretch_height) {
                src_row_l--;
              }
              int src_col_r = src_col_l + 1;
              int src_row_r = src_row_l + 1;
              if (src_col_r == stretch_width) {
                src_col_r--;
              }
              if (src_row_r == stretch_height) {
                src_row_r--;
              }
              int row_offset_l = src_row_l * stretch_pitch;
              int row_offset_r = src_row_r * stretch_pitch;
              uint32_t r_bgra_cmyk = argb[bilinear_interpol(
                  stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r,
                  res_x, res_y, 1, 0)];
              if (transformF == FXDIB_Rgba) {
                dest_pos[0] = (uint8_t)(r_bgra_cmyk >> 24);
                dest_pos[1] = (uint8_t)(r_bgra_cmyk >> 16);
                dest_pos[2] = (uint8_t)(r_bgra_cmyk >> 8);
              } else {
                *(uint32_t*)dest_pos = r_bgra_cmyk;
              }
            }
            dest_pos += destBpp;
          }
        }
      } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
        CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
        for (int row = 0; row < m_result.Height(); row++) {
          uint8_t* dest_pos = (uint8_t*)pTransformed->GetScanline(row);
          for (int col = 0; col < m_result.Width(); col++) {
            int src_col_l = 0;
            int src_row_l = 0;
            int res_x = 0;
            int res_y = 0;
            result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l,
                                         &res_x, &res_y);
            if (src_col_l >= 0 && src_col_l <= stretch_width &&
                src_row_l >= 0 && src_row_l <= stretch_height) {
              int pos_pixel[8];
              int u_w[4], v_w[4];
              if (src_col_l == stretch_width) {
                src_col_l--;
              }
              if (src_row_l == stretch_height) {
                src_row_l--;
              }
              bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l,
                                     res_x, res_y, stretch_width,
                                     stretch_height);
              uint32_t r_bgra_cmyk =
                  argb[bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel,
                                        u_w, v_w, res_x, res_y, 1, 0)];
              if (transformF == FXDIB_Rgba) {
                dest_pos[0] = (uint8_t)(r_bgra_cmyk >> 24);
                dest_pos[1] = (uint8_t)(r_bgra_cmyk >> 16);
                dest_pos[2] = (uint8_t)(r_bgra_cmyk >> 8);
              } else {
                *(uint32_t*)dest_pos = r_bgra_cmyk;
              }
            }
            dest_pos += destBpp;
          }
        }
      } else {
        CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
        for (int row = 0; row < m_result.Height(); row++) {
          uint8_t* dest_pos = (uint8_t*)pTransformed->GetScanline(row);
          for (int col = 0; col < m_result.Width(); col++) {
            int src_col = 0;
            int src_row = 0;
            result2stretch_fix.Transform(col, row, &src_col, &src_row);
            if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 &&
                src_row <= stretch_height) {
              if (src_col == stretch_width) {
                src_col--;
              }
              if (src_row == stretch_height) {
                src_row--;
              }
              uint32_t r_bgra_cmyk =
                  argb[stretch_buf[src_row * stretch_pitch + src_col]];
              if (transformF == FXDIB_Rgba) {
                dest_pos[0] = (uint8_t)(r_bgra_cmyk >> 24);
                dest_pos[1] = (uint8_t)(r_bgra_cmyk >> 16);
                dest_pos[2] = (uint8_t)(r_bgra_cmyk >> 8);
              } else {
                *(uint32_t*)dest_pos = r_bgra_cmyk;
              }
            }
            dest_pos += destBpp;
          }
        }
      }
    } else {
      bool bHasAlpha = m_Storer.GetBitmap()->HasAlpha();
      int destBpp = pTransformed->GetBPP() / 8;
      if (!(m_Flags & FXDIB_DOWNSAMPLE) &&
          !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
        CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
        for (int row = 0; row < m_result.Height(); row++) {
          uint8_t* dest_pos = (uint8_t*)pTransformed->GetScanline(row);
          for (int col = 0; col < m_result.Width(); col++) {
            int src_col_l = 0;
            int src_row_l = 0;
            int res_x = 0;
            int res_y = 0;
            int r_pos_k_r = 0;
            result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l,
                                         &res_x, &res_y);
            if (src_col_l >= 0 && src_col_l <= stretch_width &&
                src_row_l >= 0 && src_row_l <= stretch_height) {
              if (src_col_l == stretch_width) {
                src_col_l--;
              }
              if (src_row_l == stretch_height) {
                src_row_l--;
              }
              int src_col_r = src_col_l + 1;
              int src_row_r = src_row_l + 1;
              if (src_col_r == stretch_width) {
                src_col_r--;
              }
              if (src_row_r == stretch_height) {
                src_row_r--;
              }
              int row_offset_l = src_row_l * stretch_pitch;
              int row_offset_r = src_row_r * stretch_pitch;
              uint8_t r_pos_red_y_r =
                  bilinear_interpol(stretch_buf, row_offset_l, row_offset_r,
                                    src_col_l, src_col_r, res_x, res_y, Bpp, 2);
              uint8_t r_pos_green_m_r =
                  bilinear_interpol(stretch_buf, row_offset_l, row_offset_r,
                                    src_col_l, src_col_r, res_x, res_y, Bpp, 1);
              uint8_t r_pos_blue_c_r =
                  bilinear_interpol(stretch_buf, row_offset_l, row_offset_r,
                                    src_col_l, src_col_r, res_x, res_y, Bpp, 0);
              if (bHasAlpha) {
                if (transformF != FXDIB_Argb) {
                  if (transformF == FXDIB_Rgba) {
                    dest_pos[0] = r_pos_blue_c_r;
                    dest_pos[1] = r_pos_green_m_r;
                    dest_pos[2] = r_pos_red_y_r;
                  } else {
                    r_pos_k_r = bilinear_interpol(
                        stretch_buf, row_offset_l, row_offset_r, src_col_l,
                        src_col_r, res_x, res_y, Bpp, 3);
                    *(uint32_t*)dest_pos =
                        FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r,
                                                r_pos_red_y_r, r_pos_k_r));
                  }
                } else {
                  uint8_t r_pos_a_r = bilinear_interpol(
                      stretch_buf, row_offset_l, row_offset_r, src_col_l,
                      src_col_r, res_x, res_y, Bpp, 3);
                  *(uint32_t*)dest_pos = FXARGB_TODIB(
                      FXARGB_MAKE(r_pos_a_r, r_pos_red_y_r, r_pos_green_m_r,
                                  r_pos_blue_c_r));
                }
              } else {
                r_pos_k_r = 0xff;
                if (transformF == FXDIB_Cmyka) {
                  r_pos_k_r = bilinear_interpol(
                      stretch_buf, row_offset_l, row_offset_r, src_col_l,
                      src_col_r, res_x, res_y, Bpp, 3);
                  *(uint32_t*)dest_pos =
                      FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r,
                                              r_pos_red_y_r, r_pos_k_r));
                } else {
                  *(uint32_t*)dest_pos = FXARGB_TODIB(
                      FXARGB_MAKE(r_pos_k_r, r_pos_red_y_r, r_pos_green_m_r,
                                  r_pos_blue_c_r));
                }
              }
            }
            dest_pos += destBpp;
          }
        }
      } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
        CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
        for (int row = 0; row < m_result.Height(); row++) {
          uint8_t* dest_pos = (uint8_t*)pTransformed->GetScanline(row);
          for (int col = 0; col < m_result.Width(); col++) {
            int src_col_l = 0;
            int src_row_l = 0;
            int res_x = 0;
            int res_y = 0;
            int r_pos_k_r = 0;
            result2stretch_fix.Transform(col, row, &src_col_l, &src_row_l,
                                         &res_x, &res_y);
            if (src_col_l >= 0 && src_col_l <= stretch_width &&
                src_row_l >= 0 && src_row_l <= stretch_height) {
              int pos_pixel[8];
              int u_w[4], v_w[4];
              if (src_col_l == stretch_width) {
                src_col_l--;
              }
              if (src_row_l == stretch_height) {
                src_row_l--;
              }
              bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l,
                                     res_x, res_y, stretch_width,
                                     stretch_height);
              uint8_t r_pos_red_y_r =
                  bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w,
                                   v_w, res_x, res_y, Bpp, 2);
              uint8_t r_pos_green_m_r =
                  bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w,
                                   v_w, res_x, res_y, Bpp, 1);
              uint8_t r_pos_blue_c_r =
                  bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w,
                                   v_w, res_x, res_y, Bpp, 0);
              if (bHasAlpha) {
                if (transformF != FXDIB_Argb) {
                  if (transformF == FXDIB_Rgba) {
                    dest_pos[0] = r_pos_blue_c_r;
                    dest_pos[1] = r_pos_green_m_r;
                    dest_pos[2] = r_pos_red_y_r;
                  } else {
                    r_pos_k_r =
                        bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel,
                                         u_w, v_w, res_x, res_y, Bpp, 3);
                    *(uint32_t*)dest_pos =
                        FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r,
                                                r_pos_red_y_r, r_pos_k_r));
                  }
                } else {
                  uint8_t r_pos_a_r =
                      bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel,
                                       u_w, v_w, res_x, res_y, Bpp, 3);
                  *(uint32_t*)dest_pos = FXARGB_TODIB(
                      FXARGB_MAKE(r_pos_a_r, r_pos_red_y_r, r_pos_green_m_r,
                                  r_pos_blue_c_r));
                }
              } else {
                r_pos_k_r = 0xff;
                if (transformF == FXDIB_Cmyka) {
                  r_pos_k_r =
                      bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel,
                                       u_w, v_w, res_x, res_y, Bpp, 3);
                  *(uint32_t*)dest_pos =
                      FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r,
                                              r_pos_red_y_r, r_pos_k_r));
                } else {
                  *(uint32_t*)dest_pos = FXARGB_TODIB(
                      FXARGB_MAKE(r_pos_k_r, r_pos_red_y_r, r_pos_green_m_r,
                                  r_pos_blue_c_r));
                }
              }
            }
            dest_pos += destBpp;
          }
        }
      } else {
        CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
        for (int row = 0; row < m_result.Height(); row++) {
          uint8_t* dest_pos = (uint8_t*)pTransformed->GetScanline(row);
          for (int col = 0; col < m_result.Width(); col++) {
            int src_col = 0;
            int src_row = 0;
            result2stretch_fix.Transform(col, row, &src_col, &src_row);
            if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 &&
                src_row <= stretch_height) {
              if (src_col == stretch_width) {
                src_col--;
              }
              if (src_row == stretch_height) {
                src_row--;
              }
              const uint8_t* src_pos =
                  stretch_buf + src_row * stretch_pitch + src_col * Bpp;
              if (bHasAlpha) {
                if (transformF != FXDIB_Argb) {
                  if (transformF == FXDIB_Rgba) {
                    dest_pos[0] = src_pos[0];
                    dest_pos[1] = src_pos[1];
                    dest_pos[2] = src_pos[2];
                  } else {
                    *(uint32_t*)dest_pos = FXCMYK_TODIB(CmykEncode(
                        src_pos[0], src_pos[1], src_pos[2], src_pos[3]));
                  }
                } else {
                  *(uint32_t*)dest_pos = FXARGB_TODIB(FXARGB_MAKE(
                      src_pos[3], src_pos[2], src_pos[1], src_pos[0]));
                }
              } else {
                if (transformF == FXDIB_Cmyka) {
                  *(uint32_t*)dest_pos = FXCMYK_TODIB(CmykEncode(
                      src_pos[0], src_pos[1], src_pos[2], src_pos[3]));
                } else {
                  *(uint32_t*)dest_pos = FXARGB_TODIB(
                      FXARGB_MAKE(0xff, src_pos[2], src_pos[1], src_pos[0]));
                }
              }
            }
            dest_pos += destBpp;
          }
        }
      }
    }
  }
  m_Storer.Replace(std::move(pTransformed));
  return false;
}

CFX_RetainPtr<CFX_DIBitmap> CFX_ImageTransformer::DetachBitmap() {
  return m_Storer.Detach();
}
