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
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/notreached.h"
#include "third_party/base/numerics/safe_conversions.h"
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
    if (format == FXDIB_Format::kArgb) {
      *dest32 = ArgbEncode(func(3), red_y, green_m, blue_c);
    } else {
      *dest32 = FXCMYK_TODIB(CmykEncode(blue_c, green_m, red_y, func(3)));
    }
    return;
  }

  *dest32 = ArgbEncode(kOpaqueAlpha, red_y, green_m, blue_c);
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
    m_Stretcher = std::make_unique<CFX_ImageStretcher>(
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
    m_Stretcher = std::make_unique<CFX_ImageStretcher>(
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
  m_Stretcher = std::make_unique<CFX_ImageStretcher>(
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
  FXDIB_Format format = m_Stretcher->source()->IsMask()
                            ? FXDIB_Format::k8bppMask
                            : FXDIB_Format::kArgb;
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
  if (m_Storer.GetBitmap()->IsMask()) {
    CalcAlpha(cdata);
  } else {
    int Bpp = m_Storer.GetBitmap()->GetBPP() / 8;
    if (Bpp == 1)
      CalcMono(cdata);
    else
      CalcColor(cdata, format, Bpp);
  }
  m_Storer.Replace(std::move(pTransformed));
}

RetainPtr<CFX_DIBitmap> CFX_ImageTransformer::DetachBitmap() {
  return m_Storer.Detach();
}

void CFX_ImageTransformer::CalcMask(const CalcData& cdata) {
  auto func = [&cdata](const BilinearData& data, uint8_t* dest) {
    *dest = bilinear_interpol(cdata.buf, data.row_offset_l, data.row_offset_r,
                              data.src_col_l, data.src_col_r, data.res_x,
                              data.res_y, 1, 0);
  };
  DoBilinearLoop(cdata, m_result, m_StretchClip, 1, func);
}

void CFX_ImageTransformer::CalcAlpha(const CalcData& cdata) {
  auto func = [&cdata](const BilinearData& data, uint8_t* dest) {
    *dest = bilinear_interpol(cdata.buf, data.row_offset_l, data.row_offset_r,
                              data.src_col_l, data.src_col_r, data.res_x,
                              data.res_y, 1, 0);
  };
  DoBilinearLoop(cdata, m_result, m_StretchClip, 1, func);
}

void CFX_ImageTransformer::CalcMono(const CalcData& cdata) {
  uint32_t argb[256];
  if (m_Storer.GetBitmap()->HasPalette()) {
    pdfium::span<const uint32_t> palette =
        m_Storer.GetBitmap()->GetPaletteSpan();
    for (size_t i = 0; i < pdfium::size(argb); i++)
      argb[i] = palette[i];
  } else {
    for (size_t i = 0; i < pdfium::size(argb); i++)
      argb[i] = 0xff000000 | (i * 0x010101);
  }
  int destBpp = cdata.bitmap->GetBPP() / 8;
  auto func = [&cdata, &argb](const BilinearData& data, uint8_t* dest) {
    uint8_t idx = bilinear_interpol(
        cdata.buf, data.row_offset_l, data.row_offset_r, data.src_col_l,
        data.src_col_r, data.res_x, data.res_y, 1, 0);
    *reinterpret_cast<uint32_t*>(dest) = argb[idx];
  };
  DoBilinearLoop(cdata, m_result, m_StretchClip, destBpp, func);
}

void CFX_ImageTransformer::CalcColor(const CalcData& cdata,
                                     FXDIB_Format format,
                                     int Bpp) {
  DCHECK(format == FXDIB_Format::k8bppMask || format == FXDIB_Format::kArgb);
  bool bHasAlpha = m_Storer.GetBitmap()->HasAlpha();
  int destBpp = cdata.bitmap->GetBPP() / 8;
  auto func = [&cdata, format, Bpp, bHasAlpha](const BilinearData& data,
                                               uint8_t* dest) {
    auto bilinear_interpol_func = [&cdata, &data, Bpp](int offset) {
      return bilinear_interpol(cdata.buf, data.row_offset_l, data.row_offset_r,
                               data.src_col_l, data.src_col_r, data.res_x,
                               data.res_y, Bpp, offset);
    };
    WriteColorResult(bilinear_interpol_func, bHasAlpha, format, dest);
  };
  DoBilinearLoop(cdata, m_result, m_StretchClip, destBpp, func);
}
