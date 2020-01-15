// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_imagetransformer.h"

#include <cmath>
#include <memory>
#include <utility>

#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

constexpr int kBase = 256;
constexpr float kFix16 = 0.05f;
constexpr uint8_t kOpaqueAlpha = 0xff;

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
                         uint32_t pitch,
                         const int pos_pixel[],
                         const int u_w[],
                         const int v_w[],
                         int res_x,
                         int res_y,
                         int bpp,
                         int c_offset) {
  int s_result = 0;
  for (int i = 0; i < 4; i++) {
    int a_result = 0;
    for (int j = 0; j < 4; j++) {
      uint8_t val =
          *(buf + pos_pixel[i + 4] * pitch + pos_pixel[j] * bpp + c_offset);
      a_result += u_w[j] * val;
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
    pos_pixel[i] = pdfium::clamp(pos_pixel[i], 0, stretch_width - 1);
    pos_pixel[i + 4] = pdfium::clamp(pos_pixel[i + 4], 0, stretch_height - 1);
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

FXDIB_Format GetTransformedFormat(const RetainPtr<CFX_DIBBase>& pDrc) {
  if (pDrc->IsAlphaMask())
    return FXDIB_8bppMask;

  FXDIB_Format format = pDrc->GetFormat();
  if (format >= 1025)
    return FXDIB_Cmyka;
  if (format <= 32 || format == FXDIB_Argb)
    return FXDIB_Argb;
  return FXDIB_Rgba;
}

void WriteMonoResult(uint32_t r_bgra_cmyk, FXDIB_Format format, uint8_t* dest) {
  if (format == FXDIB_Rgba) {
    dest[0] = static_cast<uint8_t>(r_bgra_cmyk >> 24);
    dest[1] = static_cast<uint8_t>(r_bgra_cmyk >> 16);
    dest[2] = static_cast<uint8_t>(r_bgra_cmyk >> 8);
  } else {
    *reinterpret_cast<uint32_t*>(dest) = r_bgra_cmyk;
  }
}

// Let the compiler deduce the type for |func|, which cheaper than specifying it
// with std::function.
template <typename F>
void WriteColorResult(const F& func,
                      bool bHasAlpha,
                      FXDIB_Format format,
                      uint8_t* dest) {
  uint8_t blue_c = func(0);
  uint8_t green_m = func(1);
  uint8_t red_y = func(2);

  uint32_t* dest32 = reinterpret_cast<uint32_t*>(dest);
  if (bHasAlpha) {
    if (format == FXDIB_Argb) {
      *dest32 = FXARGB_TODIB(ArgbEncode(func(3), red_y, green_m, blue_c));
    } else if (format == FXDIB_Rgba) {
      dest[0] = blue_c;
      dest[1] = green_m;
      dest[2] = red_y;
    } else {
      *dest32 = FXCMYK_TODIB(CmykEncode(blue_c, green_m, red_y, func(3)));
    }
    return;
  }

  if (format == FXDIB_Cmyka) {
    *dest32 = FXCMYK_TODIB(CmykEncode(blue_c, green_m, red_y, func(3)));
  } else {
    *dest32 = FXARGB_TODIB(ArgbEncode(kOpaqueAlpha, red_y, green_m, blue_c));
  }
}

class CPDF_FixedMatrix {
 public:
  explicit CPDF_FixedMatrix(const CFX_Matrix& src)
      : a(FXSYS_roundf(src.a * kBase)),
        b(FXSYS_roundf(src.b * kBase)),
        c(FXSYS_roundf(src.c * kBase)),
        d(FXSYS_roundf(src.d * kBase)),
        e(FXSYS_roundf(src.e * kBase)),
        f(FXSYS_roundf(src.f * kBase)) {}

  void Transform(int x, int y, int* x1, int* y1) const {
    std::pair<float, float> val = TransformInternal(x, y);
    *x1 = pdfium::base::saturated_cast<int>(val.first / kBase);
    *y1 = pdfium::base::saturated_cast<int>(val.second / kBase);
  }

 protected:
  std::pair<float, float> TransformInternal(float x, float y) const {
    return std::make_pair(a * x + c * y + e + kBase / 2,
                          b * x + d * y + f + kBase / 2);
  }

  const int a;
  const int b;
  const int c;
  const int d;
  const int e;
  const int f;
};

class CFX_BilinearMatrix final : public CPDF_FixedMatrix {
 public:
  explicit CFX_BilinearMatrix(const CFX_Matrix& src) : CPDF_FixedMatrix(src) {}

  void Transform(int x, int y, int* x1, int* y1, int* res_x, int* res_y) const {
    std::pair<float, float> val = TransformInternal(x, y);
    *x1 = pdfium::base::saturated_cast<int>(val.first / kBase);
    *y1 = pdfium::base::saturated_cast<int>(val.second / kBase);

    *res_x = static_cast<int>(val.first) % kBase;
    *res_y = static_cast<int>(val.second) % kBase;
    if (*res_x < 0 && *res_x > -kBase)
      *res_x = kBase + *res_x;
    if (*res_y < 0 && *res_y > -kBase)
      *res_y = kBase + *res_y;
  }
};

bool InStretchBounds(const FX_RECT& clip_rect, int col, int row) {
  return col >= 0 && col <= clip_rect.Width() && row >= 0 &&
         row <= clip_rect.Height();
}

void AdjustCoords(const FX_RECT& clip_rect, int* col, int* row) {
  int& src_col = *col;
  int& src_row = *row;
  if (src_col == clip_rect.Width())
    src_col--;
  if (src_row == clip_rect.Height())
    src_row--;
}

// Let the compiler deduce the type for |func|, which cheaper than specifying it
// with std::function.
template <typename F>
void DoBilinearLoop(const CFX_ImageTransformer::CalcData& cdata,
                    const FX_RECT& result_rect,
                    const FX_RECT& clip_rect,
                    int increment,
                    const F& func) {
  CFX_BilinearMatrix matrix_fix(cdata.matrix);
  for (int row = 0; row < result_rect.Height(); row++) {
    uint8_t* dest = cdata.bitmap->GetWritableScanline(row);
    for (int col = 0; col < result_rect.Width(); col++) {
      CFX_ImageTransformer::BilinearData d;
      d.res_x = 0;
      d.res_y = 0;
      d.src_col_l = 0;
      d.src_row_l = 0;
      matrix_fix.Transform(col, row, &d.src_col_l, &d.src_row_l, &d.res_x,
                           &d.res_y);
      if (LIKELY(InStretchBounds(clip_rect, d.src_col_l, d.src_row_l))) {
        AdjustCoords(clip_rect, &d.src_col_l, &d.src_row_l);
        d.src_col_r = d.src_col_l + 1;
        d.src_row_r = d.src_row_l + 1;
        AdjustCoords(clip_rect, &d.src_col_r, &d.src_row_r);
        d.row_offset_l = d.src_row_l * cdata.pitch;
        d.row_offset_r = d.src_row_r * cdata.pitch;
        func(d, dest);
      }
      dest += increment;
    }
  }
}

// Let the compiler deduce the type for |func|, which cheaper than specifying it
// with std::function.
template <typename F>
void DoBicubicLoop(const CFX_ImageTransformer::CalcData& cdata,
                   const FX_RECT& result_rect,
                   const FX_RECT& clip_rect,
                   int increment,
                   const F& func) {
  CFX_BilinearMatrix matrix_fix(cdata.matrix);
  for (int row = 0; row < result_rect.Height(); row++) {
    uint8_t* dest = cdata.bitmap->GetWritableScanline(row);
    for (int col = 0; col < result_rect.Width(); col++) {
      CFX_ImageTransformer::BicubicData d;
      d.res_x = 0;
      d.res_y = 0;
      d.src_col_l = 0;
      d.src_row_l = 0;
      matrix_fix.Transform(col, row, &d.src_col_l, &d.src_row_l, &d.res_x,
                           &d.res_y);
      if (LIKELY(InStretchBounds(clip_rect, d.src_col_l, d.src_row_l))) {
        AdjustCoords(clip_rect, &d.src_col_l, &d.src_row_l);
        bicubic_get_pos_weight(d.pos_pixel, d.u_w, d.v_w, d.src_col_l,
                               d.src_row_l, d.res_x, d.res_y, clip_rect.Width(),
                               clip_rect.Height());
        func(d, dest);
      }
      dest += increment;
    }
  }
}

// Let the compiler deduce the type for |func|, which cheaper than specifying it
// with std::function.
template <typename F>
void DoDownSampleLoop(const CFX_ImageTransformer::CalcData& cdata,
                      const FX_RECT& result_rect,
                      const FX_RECT& clip_rect,
                      int increment,
                      const F& func) {
  CPDF_FixedMatrix matrix_fix(cdata.matrix);
  for (int row = 0; row < result_rect.Height(); row++) {
    uint8_t* dest = cdata.bitmap->GetWritableScanline(row);
    for (int col = 0; col < result_rect.Width(); col++) {
      CFX_ImageTransformer::DownSampleData d;
      d.src_col = 0;
      d.src_row = 0;
      matrix_fix.Transform(col, row, &d.src_col, &d.src_row);
      if (LIKELY(InStretchBounds(clip_rect, d.src_col, d.src_row))) {
        AdjustCoords(clip_rect, &d.src_col, &d.src_row);
        func(d, dest);
      }
      dest += increment;
    }
  }
}

}  // namespace

CFX_ImageTransformer::CFX_ImageTransformer(const RetainPtr<CFX_DIBBase>& pSrc,
                                           const CFX_Matrix& matrix,
                                           const FXDIB_ResampleOptions& options,
                                           const FX_RECT* pClip)
    : m_pSrc(pSrc), m_matrix(matrix), m_ResampleOptions(options) {
  FX_RECT result_rect = m_matrix.GetUnitRect().GetClosestRect();
  FX_RECT result_clip = result_rect;
  if (pClip)
    result_clip.Intersect(*pClip);

  if (result_clip.IsEmpty())
    return;

  m_result = result_clip;
  if (fabs(m_matrix.a) < fabs(m_matrix.b) / 20 &&
      fabs(m_matrix.d) < fabs(m_matrix.c) / 20 && fabs(m_matrix.a) < 0.5f &&
      fabs(m_matrix.d) < 0.5f) {
    int dest_width = result_rect.Width();
    int dest_height = result_rect.Height();
    result_clip.Offset(-result_rect.left, -result_rect.top);
    result_clip = FXDIB_SwapClipBox(result_clip, dest_width, dest_height,
                                    m_matrix.c > 0, m_matrix.b < 0);
    m_Stretcher = pdfium::MakeUnique<CFX_ImageStretcher>(
        &m_Storer, m_pSrc, dest_height, dest_width, result_clip,
        m_ResampleOptions);
    m_Stretcher->Start();
    m_type = kRotate;
    return;
  }
  if (fabs(m_matrix.b) < kFix16 && fabs(m_matrix.c) < kFix16) {
    int dest_width =
        static_cast<int>(m_matrix.a > 0 ? ceil(m_matrix.a) : floor(m_matrix.a));
    int dest_height = static_cast<int>(m_matrix.d > 0 ? -ceil(m_matrix.d)
                                                      : -floor(m_matrix.d));
    result_clip.Offset(-result_rect.left, -result_rect.top);
    m_Stretcher = pdfium::MakeUnique<CFX_ImageStretcher>(
        &m_Storer, m_pSrc, dest_width, dest_height, result_clip,
        m_ResampleOptions);
    m_Stretcher->Start();
    m_type = kNormal;
    return;
  }

  int stretch_width =
      static_cast<int>(ceil(FXSYS_sqrt2(m_matrix.a, m_matrix.b)));
  int stretch_height =
      static_cast<int>(ceil(FXSYS_sqrt2(m_matrix.c, m_matrix.d)));
  CFX_Matrix stretch_to_dest(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, stretch_height);
  stretch_to_dest.Concat(
      CFX_Matrix(m_matrix.a / stretch_width, m_matrix.b / stretch_width,
                 m_matrix.c / stretch_height, m_matrix.d / stretch_height,
                 m_matrix.e, m_matrix.f));
  CFX_Matrix dest_to_strech = stretch_to_dest.GetInverse();

  FX_RECT stretch_clip =
      dest_to_strech.TransformRect(CFX_FloatRect(result_clip)).GetOuterRect();
  if (!stretch_clip.Valid())
    return;

  stretch_clip.Intersect(0, 0, stretch_width, stretch_height);
  if (!stretch_clip.Valid())
    return;

  m_dest2stretch = dest_to_strech;
  m_StretchClip = stretch_clip;
  m_Stretcher = pdfium::MakeUnique<CFX_ImageStretcher>(
      &m_Storer, m_pSrc, stretch_width, stretch_height, m_StretchClip,
      m_ResampleOptions);
  m_Stretcher->Start();
  m_type = kOther;
}

CFX_ImageTransformer::~CFX_ImageTransformer() = default;

bool CFX_ImageTransformer::Continue(PauseIndicatorIface* pPause) {
  if (m_type == kNone)
    return false;

  if (m_Stretcher->Continue(pPause))
    return true;

  switch (m_type) {
    case kNormal:
      break;
    case kRotate:
      ContinueRotate(pPause);
      break;
    case kOther:
      ContinueOther(pPause);
      break;
    default:
      NOTREACHED();
      break;
  }
  return false;
}

void CFX_ImageTransformer::ContinueRotate(PauseIndicatorIface* pPause) {
  if (m_Storer.GetBitmap()) {
    m_Storer.Replace(
        m_Storer.GetBitmap()->SwapXY(m_matrix.c > 0, m_matrix.b < 0));
  }
}

void CFX_ImageTransformer::ContinueOther(PauseIndicatorIface* pPause) {
  if (!m_Storer.GetBitmap())
    return;

  auto pTransformed = pdfium::MakeRetain<CFX_DIBitmap>();
  FXDIB_Format format = GetTransformedFormat(m_Stretcher->source());
  if (!pTransformed->Create(m_result.Width(), m_result.Height(), format))
    return;

  const auto& pSrcMask = m_Storer.GetBitmap()->m_pAlphaMask;
  const uint8_t* pSrcMaskBuf = pSrcMask ? pSrcMask->GetBuffer() : nullptr;

  pTransformed->Clear(0);
  auto& pDestMask = pTransformed->m_pAlphaMask;
  if (pDestMask)
    pDestMask->Clear(0);

  CFX_Matrix result2stretch(1.0f, 0.0f, 0.0f, 1.0f, m_result.left,
                            m_result.top);
  result2stretch.Concat(m_dest2stretch);
  result2stretch.Translate(-m_StretchClip.left, -m_StretchClip.top);
  if (!pSrcMaskBuf && pDestMask) {
    pDestMask->Clear(0xff000000);
  } else if (pDestMask) {
    CalcData cdata = {
        pDestMask.Get(),
        result2stretch,
        pSrcMaskBuf,
        m_Storer.GetBitmap()->m_pAlphaMask->GetPitch(),
    };
    CalcMask(cdata);
  }

  CalcData cdata = {pTransformed.Get(), result2stretch,
                    m_Storer.GetBitmap()->GetBuffer(),
                    m_Storer.GetBitmap()->GetPitch()};
  if (m_Storer.GetBitmap()->IsAlphaMask()) {
    CalcAlpha(cdata);
  } else {
    int Bpp = m_Storer.GetBitmap()->GetBPP() / 8;
    if (Bpp == 1)
      CalcMono(cdata, format);
    else
      CalcColor(cdata, format, Bpp);
  }
  m_Storer.Replace(std::move(pTransformed));
}

RetainPtr<CFX_DIBitmap> CFX_ImageTransformer::DetachBitmap() {
  return m_Storer.Detach();
}

void CFX_ImageTransformer::CalcMask(const CalcData& cdata) {
  if (IsBilinear()) {
    auto func = [&cdata](const BilinearData& data, uint8_t* dest) {
      *dest = bilinear_interpol(cdata.buf, data.row_offset_l, data.row_offset_r,
                                data.src_col_l, data.src_col_r, data.res_x,
                                data.res_y, 1, 0);
    };
    DoBilinearLoop(cdata, m_result, m_StretchClip, 1, func);
  } else if (IsBiCubic()) {
    auto func = [&cdata](const BicubicData& data, uint8_t* dest) {
      *dest = bicubic_interpol(cdata.buf, cdata.pitch, data.pos_pixel, data.u_w,
                               data.v_w, data.res_x, data.res_y, 1, 0);
    };
    DoBicubicLoop(cdata, m_result, m_StretchClip, 1, func);
  } else {
    auto func = [&cdata](const DownSampleData& data, uint8_t* dest) {
      *dest = cdata.buf[data.src_row * cdata.pitch + data.src_col];
    };
    DoDownSampleLoop(cdata, m_result, m_StretchClip, 1, func);
  }
}

void CFX_ImageTransformer::CalcAlpha(const CalcData& cdata) {
  if (IsBilinear()) {
    auto func = [&cdata](const BilinearData& data, uint8_t* dest) {
      *dest = bilinear_interpol(cdata.buf, data.row_offset_l, data.row_offset_r,
                                data.src_col_l, data.src_col_r, data.res_x,
                                data.res_y, 1, 0);
    };
    DoBilinearLoop(cdata, m_result, m_StretchClip, 1, func);
  } else if (IsBiCubic()) {
    auto func = [&cdata](const BicubicData& data, uint8_t* dest) {
      *dest = bicubic_interpol(cdata.buf, cdata.pitch, data.pos_pixel, data.u_w,
                               data.v_w, data.res_x, data.res_y, 1, 0);
    };
    DoBicubicLoop(cdata, m_result, m_StretchClip, 1, func);
  } else {
    auto func = [&cdata](const DownSampleData& data, uint8_t* dest) {
      const uint8_t* src_pixel =
          cdata.buf + cdata.pitch * data.src_row + data.src_col;
      *dest = *src_pixel;
    };
    DoDownSampleLoop(cdata, m_result, m_StretchClip, 1, func);
  }
}

void CFX_ImageTransformer::CalcMono(const CalcData& cdata,
                                    FXDIB_Format format) {
  uint32_t argb[256];
  FX_ARGB* pPal = m_Storer.GetBitmap()->GetPalette();
  if (pPal) {
    for (size_t i = 0; i < FX_ArraySize(argb); i++)
      argb[i] = pPal[i];
  } else if (m_Storer.GetBitmap()->IsCmykImage()) {
    for (size_t i = 0; i < FX_ArraySize(argb); i++)
      argb[i] = 255 - i;
  } else {
    for (size_t i = 0; i < FX_ArraySize(argb); i++)
      argb[i] = 0xff000000 | (i * 0x010101);
  }
  int destBpp = cdata.bitmap->GetBPP() / 8;
  if (IsBilinear()) {
    auto func = [&cdata, format, &argb](const BilinearData& data,
                                        uint8_t* dest) {
      uint8_t idx = bilinear_interpol(
          cdata.buf, data.row_offset_l, data.row_offset_r, data.src_col_l,
          data.src_col_r, data.res_x, data.res_y, 1, 0);
      uint32_t r_bgra_cmyk = argb[idx];
      WriteMonoResult(r_bgra_cmyk, format, dest);
    };
    DoBilinearLoop(cdata, m_result, m_StretchClip, destBpp, func);
  } else if (IsBiCubic()) {
    auto func = [&cdata, format, &argb](const BicubicData& data,
                                        uint8_t* dest) {
      uint32_t r_bgra_cmyk = argb[bicubic_interpol(
          cdata.buf, cdata.pitch, data.pos_pixel, data.u_w, data.v_w,
          data.res_x, data.res_y, 1, 0)];
      WriteMonoResult(r_bgra_cmyk, format, dest);
    };
    DoBicubicLoop(cdata, m_result, m_StretchClip, destBpp, func);
  } else {
    auto func = [&cdata, format, &argb](const DownSampleData& data,
                                        uint8_t* dest) {
      uint32_t r_bgra_cmyk =
          argb[cdata.buf[data.src_row * cdata.pitch + data.src_col]];
      WriteMonoResult(r_bgra_cmyk, format, dest);
    };
    DoDownSampleLoop(cdata, m_result, m_StretchClip, destBpp, func);
  }
}

void CFX_ImageTransformer::CalcColor(const CalcData& cdata,
                                     FXDIB_Format format,
                                     int Bpp) {
  bool bHasAlpha = m_Storer.GetBitmap()->HasAlpha();
  int destBpp = cdata.bitmap->GetBPP() / 8;
  if (IsBilinear()) {
    auto func = [&cdata, format, Bpp, bHasAlpha](const BilinearData& data,
                                                 uint8_t* dest) {
      auto bilinear_interpol_func = [&cdata, &data, Bpp](int offset) {
        return bilinear_interpol(
            cdata.buf, data.row_offset_l, data.row_offset_r, data.src_col_l,
            data.src_col_r, data.res_x, data.res_y, Bpp, offset);
      };
      WriteColorResult(bilinear_interpol_func, bHasAlpha, format, dest);
    };
    DoBilinearLoop(cdata, m_result, m_StretchClip, destBpp, func);
  } else if (IsBiCubic()) {
    auto func = [&cdata, format, Bpp, bHasAlpha](const BicubicData& data,
                                                 uint8_t* dest) {
      auto bicubic_interpol_func = [&cdata, &data, Bpp](int offset) {
        return bicubic_interpol(cdata.buf, cdata.pitch, data.pos_pixel,
                                data.u_w, data.v_w, data.res_x, data.res_y, Bpp,
                                offset);
      };
      WriteColorResult(bicubic_interpol_func, bHasAlpha, format, dest);
    };
    DoBicubicLoop(cdata, m_result, m_StretchClip, destBpp, func);
  } else {
    auto func = [&cdata, format, bHasAlpha, Bpp](const DownSampleData& data,
                                                 uint8_t* dest) {
      const uint8_t* src_pos =
          cdata.buf + data.src_row * cdata.pitch + data.src_col * Bpp;
      auto sample_func = [src_pos](int offset) { return src_pos[offset]; };
      WriteColorResult(sample_func, bHasAlpha, format, dest);
    };
    DoDownSampleLoop(cdata, m_result, m_StretchClip, destBpp, func);
  }
}

bool CFX_ImageTransformer::IsBilinear() const {
  return !IsBiCubic();
}

bool CFX_ImageTransformer::IsBiCubic() const {
  return m_ResampleOptions.bInterpolateBicubic;
}
