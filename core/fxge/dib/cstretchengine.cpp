// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cstretchengine.h"

#include <math.h>

#include <algorithm>
#include <type_traits>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/pauseindicator_iface.h"
#include "core/fxge/calculate_pitch.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/dib/scanlinecomposer_iface.h"

static_assert(
    std::is_trivially_destructible<CStretchEngine::PixelWeight>::value,
    "PixelWeight storage may be re-used without invoking its destructor");

namespace {

size_t TotalBytesForWeightCount(size_t weight_count) {
  // Always room for one weight even for empty ranges due to declaration
  // of weights_[1] in the header. Don't shrink below this since
  // CalculateWeights() relies on this later.
  const size_t extra_weights = weight_count > 0 ? weight_count - 1 : 0;
  FX_SAFE_SIZE_T total_bytes = extra_weights;
  total_bytes *= sizeof(CStretchEngine::PixelWeight::weights_[0]);
  total_bytes += sizeof(CStretchEngine::PixelWeight);
  return total_bytes.ValueOrDie();
}

}  // namespace

// static
bool CStretchEngine::UseInterpolateBilinear(
    const FXDIB_ResampleOptions& options,
    int dest_width,
    int dest_height,
    int src_width,
    int src_height) {
  return !options.bInterpolateBilinear && !options.bNoSmoothing &&
         abs(dest_width) != 0 &&
         abs(dest_height) / 8 <
             static_cast<long long>(src_width) * src_height / abs(dest_width);
}

CStretchEngine::WeightTable::WeightTable() = default;

CStretchEngine::WeightTable::~WeightTable() = default;

bool CStretchEngine::WeightTable::CalculateWeights(
    int dest_len,
    int dest_min,
    int dest_max,
    int src_len,
    int src_min,
    int src_max,
    const FXDIB_ResampleOptions& options) {
  // 512MB should be large enough for this while preventing OOM.
  static constexpr size_t kMaxTableBytesAllowed = 512 * 1024 * 1024;

  // Help the compiler realize that these can't change during a loop iteration:
  const bool bilinear = options.bInterpolateBilinear;

  dest_min_ = 0;
  item_size_bytes_ = 0;
  weight_tables_size_bytes_ = 0;
  weight_tables_.clear();
  if (dest_len == 0) {
    return true;
  }

  if (dest_min > dest_max) {
    return false;
  }

  dest_min_ = dest_min;

  const double scale = static_cast<double>(src_len) / dest_len;
  const double base = dest_len < 0 ? src_len : 0;
  const size_t weight_count = static_cast<size_t>(ceil(fabs(scale))) + 1;
  item_size_bytes_ = TotalBytesForWeightCount(weight_count);

  const size_t dest_range = static_cast<size_t>(dest_max - dest_min);
  const size_t kMaxTableItemsAllowed = kMaxTableBytesAllowed / item_size_bytes_;
  if (dest_range > kMaxTableItemsAllowed) {
    return false;
  }

  weight_tables_size_bytes_ = dest_range * item_size_bytes_;
  weight_tables_.resize(weight_tables_size_bytes_);
  UNSAFE_TODO({
    if (options.bNoSmoothing || fabs(scale) < 1.0f) {
      for (int dest_pixel = dest_min; dest_pixel < dest_max; ++dest_pixel) {
        PixelWeight& pixel_weights = *GetPixelWeight(dest_pixel);
        double src_pos = dest_pixel * scale + scale / 2 + base;
        if (bilinear) {
          int src_start = static_cast<int>(floor(src_pos - 0.5));
          int src_end = static_cast<int>(floor(src_pos + 0.5));
          src_start = std::max(src_start, src_min);
          src_end = std::min(src_end, src_max - 1);
          pixel_weights.SetStartEnd(src_start, src_end, weight_count);
          if (pixel_weights.src_start_ >= pixel_weights.src_end_) {
            // Always room for one weight per size calculation.
            pixel_weights.weights_[0] = kFixedPointOne;
          } else {
            pixel_weights.weights_[1] =
                FixedFromDouble(src_pos - pixel_weights.src_start_ - 0.5f);
            pixel_weights.weights_[0] =
                kFixedPointOne - pixel_weights.weights_[1];
          }
        } else {
          int pixel_pos = static_cast<int>(floor(src_pos));
          int src_start = std::max(pixel_pos, src_min);
          int src_end = std::min(pixel_pos, src_max - 1);
          pixel_weights.SetStartEnd(src_start, src_end, weight_count);
          pixel_weights.weights_[0] = kFixedPointOne;
        }
      }
      return true;
    }

    for (int dest_pixel = dest_min; dest_pixel < dest_max; ++dest_pixel) {
      PixelWeight& pixel_weights = *GetPixelWeight(dest_pixel);
      double src_start = dest_pixel * scale + base;
      double src_end = src_start + scale;
      int start_i = floor(std::min(src_start, src_end));
      int end_i = floor(std::max(src_start, src_end));
      start_i = std::max(start_i, src_min);
      end_i = std::min(end_i, src_max - 1);
      if (start_i > end_i) {
        start_i = std::min(start_i, src_max - 1);
        pixel_weights.SetStartEnd(start_i, start_i, weight_count);
        continue;
      }
      pixel_weights.SetStartEnd(start_i, end_i, weight_count);
      uint32_t remaining = kFixedPointOne;
      double rounding_error = 0.0;
      for (int j = start_i; j < end_i; ++j) {
        double dest_start = (j - base) / scale;
        double dest_end = (j + 1 - base) / scale;
        if (dest_start > dest_end) {
          std::swap(dest_start, dest_end);
        }
        double area_start =
            std::max(dest_start, static_cast<double>(dest_pixel));
        double area_end =
            std::min(dest_end, static_cast<double>(dest_pixel + 1));
        double weight = std::max(0.0, area_end - area_start);
        uint32_t fixed_weight = FixedFromDouble(weight + rounding_error);
        pixel_weights.SetWeightForPosition(j, fixed_weight);
        remaining -= fixed_weight;
        rounding_error =
            weight - static_cast<double>(fixed_weight) / kFixedPointOne;
      }
      // Note: underflow is defined behaviour for unsigned types and will
      // result in an out-of-range value.
      if (remaining && remaining <= kFixedPointOne) {
        pixel_weights.SetWeightForPosition(end_i, remaining);
      } else {
        pixel_weights.RemoveLastWeightAndAdjust(remaining);
      }
    }
  });
  return true;
}

const CStretchEngine::PixelWeight* CStretchEngine::WeightTable::GetPixelWeight(
    int pixel) const {
  DCHECK(pixel >= dest_min_);
  return reinterpret_cast<const PixelWeight*>(
      &weight_tables_[(pixel - dest_min_) * item_size_bytes_]);
}

CStretchEngine::PixelWeight* CStretchEngine::WeightTable::GetPixelWeight(
    int pixel) {
  return const_cast<PixelWeight*>(std::as_const(*this).GetPixelWeight(pixel));
}

CStretchEngine::CStretchEngine(ScanlineComposerIface* pDestBitmap,
                               FXDIB_Format dest_format,
                               int dest_width,
                               int dest_height,
                               const FX_RECT& clip_rect,
                               const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                               const FXDIB_ResampleOptions& options)
    : dest_format_(dest_format),
      dest_bpp_(GetBppFromFormat(dest_format)),
      src_bpp_(pSrcBitmap->GetBPP()),
      has_alpha_(pSrcBitmap->IsAlphaFormat()),
      source_(pSrcBitmap),
      src_palette_(pSrcBitmap->GetPaletteSpan()),
      src_width_(pSrcBitmap->GetWidth()),
      src_height_(pSrcBitmap->GetHeight()),
      dest_bitmap_(pDestBitmap),
      dest_width_(dest_width),
      dest_height_(dest_height),
      dest_clip_(clip_rect) {
  if (has_alpha_) {
    // TODO(crbug.com/42271020): Consider adding support for
    // `FXDIB_Format::kBgraPremul`
    DCHECK_EQ(dest_format_, FXDIB_Format::kBgra);
    DCHECK_EQ(dest_bpp_, GetBppFromFormat(FXDIB_Format::kBgra));
    DCHECK_EQ(source_->GetFormat(), FXDIB_Format::kBgra);
    DCHECK_EQ(src_bpp_, GetBppFromFormat(FXDIB_Format::kBgra));
  }

  std::optional<uint32_t> maybe_size =
      fxge::CalculatePitch32(dest_bpp_, clip_rect.Width());
  if (!maybe_size.has_value()) {
    return;
  }

  dest_scanline_.resize(maybe_size.value());
  if (dest_format == FXDIB_Format::kBgrx) {
    std::fill(dest_scanline_.begin(), dest_scanline_.end(), 255);
  }
  inter_pitch_ = fxge::CalculatePitch32OrDie(dest_bpp_, dest_clip_.Width());
  extra_mask_pitch_ = fxge::CalculatePitch32OrDie(8, dest_clip_.Width());
  if (options.bNoSmoothing) {
    resample_options_.bNoSmoothing = true;
  } else {
    if (UseInterpolateBilinear(options, dest_width, dest_height, src_width_,
                               src_height_)) {
      resample_options_.bInterpolateBilinear = true;
    } else {
      resample_options_ = options;
    }
  }
  double scale_x = static_cast<float>(src_width_) / dest_width_;
  double scale_y = static_cast<float>(src_height_) / dest_height_;
  double base_x = dest_width_ > 0 ? 0.0f : dest_width_;
  double base_y = dest_height_ > 0 ? 0.0f : dest_height_;
  double src_left = scale_x * (clip_rect.left + base_x);
  double src_right = scale_x * (clip_rect.right + base_x);
  double src_top = scale_y * (clip_rect.top + base_y);
  double src_bottom = scale_y * (clip_rect.bottom + base_y);
  if (src_left > src_right) {
    std::swap(src_left, src_right);
  }
  if (src_top > src_bottom) {
    std::swap(src_top, src_bottom);
  }
  src_clip_.left = static_cast<int>(floor(src_left));
  src_clip_.right = static_cast<int>(ceil(src_right));
  src_clip_.top = static_cast<int>(floor(src_top));
  src_clip_.bottom = static_cast<int>(ceil(src_bottom));
  FX_RECT src_rect(0, 0, src_width_, src_height_);
  src_clip_.Intersect(src_rect);

  switch (src_bpp_) {
    case 1:
      trans_method_ = dest_bpp_ == 8 ? TransformMethod::k1BppTo8Bpp
                                     : TransformMethod::k1BppToManyBpp;
      break;
    case 8:
      trans_method_ = dest_bpp_ == 8 ? TransformMethod::k8BppTo8Bpp
                                     : TransformMethod::k8BppToManyBpp;
      break;
    default:
      trans_method_ = has_alpha_ ? TransformMethod::kManyBpptoManyBppWithAlpha
                                 : TransformMethod::kManyBpptoManyBpp;
      break;
  }
}

CStretchEngine::~CStretchEngine() = default;

bool CStretchEngine::Continue(PauseIndicatorIface* pPause) {
  while (state_ == State::kHorizontal) {
    if (ContinueStretchHorz(pPause)) {
      return true;
    }

    state_ = State::kVertical;
    StretchVert();
  }
  return false;
}

bool CStretchEngine::StartStretchHorz() {
  if (dest_width_ == 0 || inter_pitch_ == 0 || dest_scanline_.empty()) {
    return false;
  }

  FX_SAFE_SIZE_T safe_size = src_clip_.Height();
  safe_size *= inter_pitch_;
  const size_t size = safe_size.ValueOrDefault(0);
  if (size == 0) {
    return false;
  }
  inter_buf_ = FixedSizeDataVector<uint8_t>::TryZeroed(size);
  if (inter_buf_.empty()) {
    return false;
  }
  if (!weight_table_.CalculateWeights(
          dest_width_, dest_clip_.left, dest_clip_.right, src_width_,
          src_clip_.left, src_clip_.right, resample_options_)) {
    return false;
  }
  cur_row_ = src_clip_.top;
  state_ = State::kHorizontal;
  return true;
}

bool CStretchEngine::ContinueStretchHorz(PauseIndicatorIface* pPause) {
  if (!dest_width_) {
    return false;
  }
  if (source_->SkipToScanline(cur_row_, pPause)) {
    return true;
  }

  int Bpp = dest_bpp_ / 8;
  static const int kStrechPauseRows = 10;
  int rows_to_go = kStrechPauseRows;
  for (; cur_row_ < src_clip_.bottom; ++cur_row_) {
    if (rows_to_go == 0) {
      if (pPause && pPause->NeedToPauseNow()) {
        return true;
      }

      rows_to_go = kStrechPauseRows;
    }

    const uint8_t* src_scan = source_->GetScanline(cur_row_).data();
    pdfium::span<uint8_t> dest_span = inter_buf_.subspan(
        (cur_row_ - src_clip_.top) * inter_pitch_, inter_pitch_);
    size_t dest_span_index = 0;
    // TODO(npm): reduce duplicated code here
    UNSAFE_TODO({
      switch (trans_method_) {
        case TransformMethod::k1BppTo8Bpp:
        case TransformMethod::k1BppToManyBpp: {
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            PixelWeight* pWeights = weight_table_.GetPixelWeight(col);
            uint32_t dest_a = 0;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
              if (src_scan[j / 8] & (1 << (7 - j % 8))) {
                dest_a += pixel_weight * 255;
              }
            }
            dest_span[dest_span_index++] = PixelFromFixed(dest_a);
          }
          break;
        }
        case TransformMethod::k8BppTo8Bpp: {
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            PixelWeight* pWeights = weight_table_.GetPixelWeight(col);
            uint32_t dest_a = 0;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
              dest_a += pixel_weight * src_scan[j];
            }
            dest_span[dest_span_index++] = PixelFromFixed(dest_a);
          }
          break;
        }
        case TransformMethod::k8BppToManyBpp: {
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            PixelWeight* pWeights = weight_table_.GetPixelWeight(col);
            uint32_t dest_r = 0;
            uint32_t dest_g = 0;
            uint32_t dest_b = 0;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
              FX_ARGB argb = src_palette_[src_scan[j]];
              if (dest_format_ == FXDIB_Format::kBgr) {
                dest_r += pixel_weight * static_cast<uint8_t>(argb >> 16);
                dest_g += pixel_weight * static_cast<uint8_t>(argb >> 8);
                dest_b += pixel_weight * static_cast<uint8_t>(argb);
              } else {
                dest_b += pixel_weight * static_cast<uint8_t>(argb >> 24);
                dest_g += pixel_weight * static_cast<uint8_t>(argb >> 16);
                dest_r += pixel_weight * static_cast<uint8_t>(argb >> 8);
              }
            }
            dest_span[dest_span_index++] = PixelFromFixed(dest_b);
            dest_span[dest_span_index++] = PixelFromFixed(dest_g);
            dest_span[dest_span_index++] = PixelFromFixed(dest_r);
          }
          break;
        }
        case TransformMethod::kManyBpptoManyBpp: {
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            PixelWeight* pWeights = weight_table_.GetPixelWeight(col);
            uint32_t dest_r = 0;
            uint32_t dest_g = 0;
            uint32_t dest_b = 0;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
              const uint8_t* src_pixel = src_scan + j * Bpp;
              dest_b += pixel_weight * (*src_pixel++);
              dest_g += pixel_weight * (*src_pixel++);
              dest_r += pixel_weight * (*src_pixel);
            }
            dest_span[dest_span_index++] = PixelFromFixed(dest_b);
            dest_span[dest_span_index++] = PixelFromFixed(dest_g);
            dest_span[dest_span_index++] = PixelFromFixed(dest_r);
            dest_span_index += Bpp - 3;
          }
          break;
        }
        case TransformMethod::kManyBpptoManyBppWithAlpha: {
          DCHECK(has_alpha_);
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            PixelWeight* pWeights = weight_table_.GetPixelWeight(col);
            uint32_t dest_a = 0;
            uint32_t dest_r = 0;
            uint32_t dest_g = 0;
            uint32_t dest_b = 0;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              const uint8_t* src_pixel = src_scan + j * Bpp;
              uint32_t pixel_weight =
                  pWeights->GetWeightForPosition(j) * src_pixel[3] / 255;
              dest_b += pixel_weight * (*src_pixel++);
              dest_g += pixel_weight * (*src_pixel++);
              dest_r += pixel_weight * (*src_pixel);
              dest_a += pixel_weight;
            }
            dest_span[dest_span_index++] = PixelFromFixed(dest_b);
            dest_span[dest_span_index++] = PixelFromFixed(dest_g);
            dest_span[dest_span_index++] = PixelFromFixed(dest_r);
            dest_span[dest_span_index] = PixelFromFixed(255 * dest_a);
            dest_span_index += Bpp - 3;
          }
          break;
        }
      }
    });
    rows_to_go--;
  }
  return false;
}

void CStretchEngine::StretchVert() {
  if (dest_height_ == 0) {
    return;
  }

  WeightTable table;
  if (!table.CalculateWeights(dest_height_, dest_clip_.top, dest_clip_.bottom,
                              src_height_, src_clip_.top, src_clip_.bottom,
                              resample_options_)) {
    return;
  }

  const int DestBpp = dest_bpp_ / 8;
  UNSAFE_TODO({
    for (int row = dest_clip_.top; row < dest_clip_.bottom; ++row) {
      unsigned char* dest_scan = dest_scanline_.data();
      PixelWeight* pWeights = table.GetPixelWeight(row);
      switch (trans_method_) {
        case TransformMethod::k1BppTo8Bpp:
        case TransformMethod::k1BppToManyBpp:
        case TransformMethod::k8BppTo8Bpp: {
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            pdfium::span<const uint8_t> src_span =
                inter_buf_.subspan((col - dest_clip_.left) * DestBpp);
            uint32_t dest_a = 0;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
              dest_a +=
                  pixel_weight * src_span[(j - src_clip_.top) * inter_pitch_];
            }
            *dest_scan = PixelFromFixed(dest_a);
            dest_scan += DestBpp;
          }
          break;
        }
        case TransformMethod::k8BppToManyBpp:
        case TransformMethod::kManyBpptoManyBpp: {
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            pdfium::span<const uint8_t> src_span =
                inter_buf_.subspan((col - dest_clip_.left) * DestBpp);
            uint32_t dest_r = 0;
            uint32_t dest_g = 0;
            uint32_t dest_b = 0;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
              pdfium::span<const uint8_t> src_pixel = src_span.subspan(
                  static_cast<size_t>((j - src_clip_.top) * inter_pitch_), 3u);
              dest_b += pixel_weight * src_pixel[0];
              dest_g += pixel_weight * src_pixel[1];
              dest_r += pixel_weight * src_pixel[2];
            }
            dest_scan[0] = PixelFromFixed(dest_b);
            dest_scan[1] = PixelFromFixed(dest_g);
            dest_scan[2] = PixelFromFixed(dest_r);
            dest_scan += DestBpp;
          }
          break;
        }
        case TransformMethod::kManyBpptoManyBppWithAlpha: {
          DCHECK(has_alpha_);
          for (int col = dest_clip_.left; col < dest_clip_.right; ++col) {
            pdfium::span<const uint8_t> src_span =
                inter_buf_.subspan((col - dest_clip_.left) * DestBpp);
            uint32_t dest_a = 0;
            uint32_t dest_r = 0;
            uint32_t dest_g = 0;
            uint32_t dest_b = 0;
            static constexpr size_t kPixelBytes = 4;
            for (int j = pWeights->src_start_; j <= pWeights->src_end_; ++j) {
              uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
              pdfium::span<const uint8_t> src_pixel = src_span.subspan(
                  static_cast<size_t>((j - src_clip_.top) * inter_pitch_),
                  kPixelBytes);
              dest_b += pixel_weight * src_pixel[0];
              dest_g += pixel_weight * src_pixel[1];
              dest_r += pixel_weight * src_pixel[2];
              dest_a += pixel_weight * src_pixel[3];
            }
            if (dest_a) {
              int r = static_cast<uint32_t>(dest_r) * 255 / dest_a;
              int g = static_cast<uint32_t>(dest_g) * 255 / dest_a;
              int b = static_cast<uint32_t>(dest_b) * 255 / dest_a;
              dest_scan[0] = std::clamp(b, 0, 255);
              dest_scan[1] = std::clamp(g, 0, 255);
              dest_scan[2] = std::clamp(r, 0, 255);
            }
            dest_scan[3] = PixelFromFixed(dest_a);
            dest_scan += DestBpp;
          }
          break;
        }
      }
      dest_bitmap_->ComposeScanline(row - dest_clip_.top, dest_scanline_);
    }
  });
}
