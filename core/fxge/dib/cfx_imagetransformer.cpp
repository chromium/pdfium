// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_imagetransformer.h"

#include <math.h>

#include <array>
#include <iterator>
#include <memory>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/dib/fx_dib.h"

namespace {

constexpr int kBase = 256;
constexpr float kFix16 = 0.05f;
constexpr uint8_t kOpaqueAlpha = 0xff;

uint8_t BilinearInterpolate(const uint8_t* buf,
                            const CFX_ImageTransformer::BilinearData& data,
                            int bytes_per_pixel,
                            int c_offset) {
  const int i_resx = 255 - data.res_x;
  const int col_bpp_l = data.src_col_l * bytes_per_pixel;
  const int col_bpp_r = data.src_col_r * bytes_per_pixel;
  UNSAFE_TODO({
    const uint8_t* buf_u = buf + data.row_offset_l + c_offset;
    const uint8_t* buf_d = buf + data.row_offset_r + c_offset;
    const uint8_t* src_pos0 = buf_u + col_bpp_l;
    const uint8_t* src_pos1 = buf_u + col_bpp_r;
    const uint8_t* src_pos2 = buf_d + col_bpp_l;
    const uint8_t* src_pos3 = buf_d + col_bpp_r;
    uint8_t r_pos_0 = (*src_pos0 * i_resx + *src_pos1 * data.res_x) >> 8;
    uint8_t r_pos_1 = (*src_pos2 * i_resx + *src_pos3 * data.res_x) >> 8;
    return (r_pos_0 * (255 - data.res_y) + r_pos_1 * data.res_y) >> 8;
  });
}

class CFX_BilinearMatrix {
 public:
  explicit CFX_BilinearMatrix(const CFX_Matrix& src)
      : a(FXSYS_roundf(src.a * kBase)),
        b(FXSYS_roundf(src.b * kBase)),
        c(FXSYS_roundf(src.c * kBase)),
        d(FXSYS_roundf(src.d * kBase)),
        e(FXSYS_roundf(src.e * kBase)),
        f(FXSYS_roundf(src.f * kBase)) {}

  void Transform(int x, int y, int* x1, int* y1, int* res_x, int* res_y) const {
    CFX_PointF val = TransformInternal(CFX_PointF(x, y));
    *x1 = pdfium::saturated_cast<int>(val.x / kBase);
    *y1 = pdfium::saturated_cast<int>(val.y / kBase);
    *res_x = static_cast<int>(val.x) % kBase;
    *res_y = static_cast<int>(val.y) % kBase;
    if (*res_x < 0 && *res_x > -kBase) {
      *res_x = kBase + *res_x;
    }
    if (*res_y < 0 && *res_y > -kBase) {
      *res_y = kBase + *res_y;
    }
  }

 private:
  CFX_PointF TransformInternal(CFX_PointF pt) const {
    return CFX_PointF(a * pt.x + c * pt.y + e + kBase / 2,
                      b * pt.x + d * pt.y + f + kBase / 2);
  }

  const int a;
  const int b;
  const int c;
  const int d;
  const int e;
  const int f;
};

bool InStretchBounds(const FX_RECT& clip_rect, int col, int row) {
  return col >= 0 && col <= clip_rect.Width() && row >= 0 &&
         row <= clip_rect.Height();
}

void AdjustCoords(const FX_RECT& clip_rect, int* col, int* row) {
  int& src_col = *col;
  int& src_row = *row;
  if (src_col == clip_rect.Width()) {
    src_col--;
  }
  if (src_row == clip_rect.Height()) {
    src_row--;
  }
}

// Let the compiler deduce the type for |func|, which cheaper than specifying it
// with std::function.
template <typename F>
void DoBilinearLoop(const CFX_ImageTransformer::CalcData& calc_data,
                    const FX_RECT& result_rect,
                    const FX_RECT& clip_rect,
                    int increment,
                    const F& func) {
  CFX_BilinearMatrix matrix_fix(calc_data.matrix);
  for (int row = 0; row < result_rect.Height(); row++) {
    uint8_t* dest = calc_data.bitmap->GetWritableScanline(row).data();
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
        d.row_offset_l = d.src_row_l * calc_data.pitch;
        d.row_offset_r = d.src_row_r * calc_data.pitch;
        func(d, dest);
      }
      UNSAFE_TODO(dest += increment);
    }
  }
}

}  // namespace

CFX_ImageTransformer::CFX_ImageTransformer(RetainPtr<const CFX_DIBBase> source,
                                           const CFX_Matrix& matrix,
                                           const FXDIB_ResampleOptions& options,
                                           const FX_RECT* pClip)
    : src_(std::move(source)), matrix_(matrix), resample_options_(options) {
  FX_RECT result_rect = matrix_.GetUnitRect().GetClosestRect();
  FX_RECT result_clip = result_rect;
  if (pClip) {
    result_clip.Intersect(*pClip);
  }

  if (result_clip.IsEmpty()) {
    return;
  }

  result_ = result_clip;
  if (fabs(matrix_.a) < fabs(matrix_.b) / 20 &&
      fabs(matrix_.d) < fabs(matrix_.c) / 20 && fabs(matrix_.a) < 0.5f &&
      fabs(matrix_.d) < 0.5f) {
    int dest_width = result_rect.Width();
    int dest_height = result_rect.Height();
    result_clip.Offset(-result_rect.left, -result_rect.top);
    result_clip = result_clip.SwappedClipBox(dest_width, dest_height,
                                             matrix_.c > 0, matrix_.b < 0);
    stretcher_ = std::make_unique<CFX_ImageStretcher>(
        &storer_, src_, dest_height, dest_width, result_clip,
        resample_options_);
    stretcher_->Start();
    type_ = StretchType::kRotate;
    return;
  }
  if (fabs(matrix_.b) < kFix16 && fabs(matrix_.c) < kFix16) {
    int dest_width =
        static_cast<int>(matrix_.a > 0 ? ceil(matrix_.a) : floor(matrix_.a));
    int dest_height =
        static_cast<int>(matrix_.d > 0 ? -ceil(matrix_.d) : -floor(matrix_.d));
    result_clip.Offset(-result_rect.left, -result_rect.top);
    stretcher_ = std::make_unique<CFX_ImageStretcher>(
        &storer_, src_, dest_width, dest_height, result_clip,
        resample_options_);
    stretcher_->Start();
    type_ = StretchType::kNormal;
    return;
  }

  int stretch_width = static_cast<int>(ceil(hypotf(matrix_.a, matrix_.b)));
  if (stretch_width == 0) {
    return;
  }

  int stretch_height = static_cast<int>(ceil(hypotf(matrix_.c, matrix_.d)));
  if (stretch_height == 0) {
    return;
  }

  CFX_Matrix stretch_to_dest(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, stretch_height);
  stretch_to_dest.Concat(
      CFX_Matrix(matrix_.a / stretch_width, matrix_.b / stretch_width,
                 matrix_.c / stretch_height, matrix_.d / stretch_height,
                 matrix_.e, matrix_.f));
  CFX_Matrix dest_to_strech = stretch_to_dest.GetInverse();

  FX_RECT stretch_clip =
      dest_to_strech.TransformRect(CFX_FloatRect(result_clip)).GetOuterRect();
  if (!stretch_clip.Valid()) {
    return;
  }

  stretch_clip.Intersect(0, 0, stretch_width, stretch_height);
  if (!stretch_clip.Valid()) {
    return;
  }

  dest_to_stretch_ = dest_to_strech;
  stretch_clip_ = stretch_clip;
  stretcher_ = std::make_unique<CFX_ImageStretcher>(
      &storer_, src_, stretch_width, stretch_height, stretch_clip_,
      resample_options_);
  stretcher_->Start();
  type_ = StretchType::kOther;
}

CFX_ImageTransformer::~CFX_ImageTransformer() = default;

bool CFX_ImageTransformer::Continue(PauseIndicatorIface* pPause) {
  if (type_ == StretchType::kNone) {
    return false;
  }

  if (stretcher_->Continue(pPause)) {
    return true;
  }

  switch (type_) {
    case StretchType::kNone:
      // Already handled separately at the beginning of this method.
      NOTREACHED();
    case StretchType::kNormal:
      return false;
    case StretchType::kRotate:
      ContinueRotate(pPause);
      return false;
    case StretchType::kOther:
      ContinueOther(pPause);
      return false;
  }
}

void CFX_ImageTransformer::ContinueRotate(PauseIndicatorIface* pPause) {
  if (storer_.GetBitmap()) {
    storer_.Replace(storer_.GetBitmap()->SwapXY(matrix_.c > 0, matrix_.b < 0));
  }
}

void CFX_ImageTransformer::ContinueOther(PauseIndicatorIface* pPause) {
  if (!storer_.GetBitmap()) {
    return;
  }

  auto pTransformed = pdfium::MakeRetain<CFX_DIBitmap>();
  // TODO(crbug.com/42271020): Consider adding support for
  // `FXDIB_Format::kBgraPremul`
  FXDIB_Format dest_format = stretcher_->source()->IsMaskFormat()
                                 ? FXDIB_Format::k8bppMask
                                 : FXDIB_Format::kBgra;
  if (!pTransformed->Create(result_.Width(), result_.Height(), dest_format)) {
    return;
  }

  CFX_Matrix result2stretch(1.0f, 0.0f, 0.0f, 1.0f, result_.left, result_.top);
  result2stretch.Concat(dest_to_stretch_);
  result2stretch.Translate(-stretch_clip_.left, -stretch_clip_.top);

  CalcData calc_data = {pTransformed.Get(), result2stretch,
                        storer_.GetBitmap()->GetBuffer().data(),
                        storer_.GetBitmap()->GetPitch()};
  if (storer_.GetBitmap()->IsMaskFormat()) {
    CalcAlpha(calc_data);
  } else {
    const int src_bytes_per_pixel = storer_.GetBitmap()->GetBPP() / 8;
    if (src_bytes_per_pixel == 1) {
      CalcMono(calc_data);
    } else {
      CalcColor(calc_data, dest_format, src_bytes_per_pixel);
    }
  }
  storer_.Replace(std::move(pTransformed));
}

RetainPtr<CFX_DIBitmap> CFX_ImageTransformer::DetachBitmap() {
  return storer_.Detach();
}

void CFX_ImageTransformer::CalcAlpha(const CalcData& calc_data) {
  auto func = [&calc_data](const BilinearData& data, uint8_t* dest) {
    *dest = BilinearInterpolate(calc_data.buf, data, 1, 0);
  };
  DoBilinearLoop(calc_data, result_, stretch_clip_, 1, func);
}

void CFX_ImageTransformer::CalcMono(const CalcData& calc_data) {
  std::array<uint32_t, 256> argb;
  if (storer_.GetBitmap()->HasPalette()) {
    pdfium::span<const uint32_t> palette =
        storer_.GetBitmap()->GetPaletteSpan();
    fxcrt::Copy(palette.first(argb.size()), argb);
  } else {
    for (uint32_t i = 0; i < argb.size(); ++i) {
      argb[i] = ArgbEncode(0xff, i, i, i);
    }
  }
  const int dest_bytes_per_pixel = calc_data.bitmap->GetBPP() / 8;
  auto func = [&calc_data, &argb](const BilinearData& data, uint8_t* dest) {
    uint8_t idx = BilinearInterpolate(calc_data.buf, data, 1, 0);
    *reinterpret_cast<uint32_t*>(dest) = argb[idx];
  };
  DoBilinearLoop(calc_data, result_, stretch_clip_, dest_bytes_per_pixel, func);
}

void CFX_ImageTransformer::CalcColor(const CalcData& calc_data,
                                     FXDIB_Format dest_format,
                                     int src_bytes_per_pixel) {
  DCHECK(dest_format == FXDIB_Format::k8bppMask ||
         dest_format == FXDIB_Format::kBgra);
  const int dest_bytes_per_pixel = calc_data.bitmap->GetBPP() / 8;
  if (!storer_.GetBitmap()->IsAlphaFormat()) {
    auto func = [&calc_data, src_bytes_per_pixel](const BilinearData& data,
                                                  uint8_t* dest) {
      uint8_t b =
          BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 0);
      uint8_t g =
          BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 1);
      uint8_t r =
          BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 2);
      *reinterpret_cast<uint32_t*>(dest) = ArgbEncode(kOpaqueAlpha, r, g, b);
    };
    DoBilinearLoop(calc_data, result_, stretch_clip_, dest_bytes_per_pixel,
                   func);
    return;
  }

  if (dest_format == FXDIB_Format::kBgra) {
    auto func = [&calc_data, src_bytes_per_pixel](const BilinearData& data,
                                                  uint8_t* dest) {
      uint8_t b =
          BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 0);
      uint8_t g =
          BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 1);
      uint8_t r =
          BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 2);
      uint8_t alpha =
          BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 3);
      *reinterpret_cast<uint32_t*>(dest) = ArgbEncode(alpha, r, g, b);
    };
    DoBilinearLoop(calc_data, result_, stretch_clip_, dest_bytes_per_pixel,
                   func);
    return;
  }

  auto func = [&calc_data, src_bytes_per_pixel](const BilinearData& data,
                                                uint8_t* dest) {
    uint8_t c =
        BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 0);
    uint8_t m =
        BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 1);
    uint8_t y =
        BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 2);
    uint8_t k =
        BilinearInterpolate(calc_data.buf, data, src_bytes_per_pixel, 3);
    *reinterpret_cast<uint32_t*>(dest) = FXCMYK_TODIB(CmykEncode(c, m, y, k));
  };
  DoBilinearLoop(calc_data, result_, stretch_clip_, dest_bytes_per_pixel, func);
}
