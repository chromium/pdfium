// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CSTRETCHENGINE_H_
#define CORE_FXGE_DIB_CSTRETCHENGINE_H_

#include <stdint.h>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CFX_DIBBase;
class PauseIndicatorIface;
class ScanlineComposerIface;

class CStretchEngine {
 public:
  static constexpr uint32_t kFixedPointBits = 16;
  static constexpr uint32_t kFixedPointOne = 1 << kFixedPointBits;

  static inline uint32_t FixedFromDouble(double d) {
    return static_cast<uint32_t>(FXSYS_round(d * kFixedPointOne));
  }

  static inline uint8_t PixelFromFixed(uint32_t fixed) {
    return static_cast<uint8_t>(fixed >> kFixedPointBits);
  }

  // Indicates whether to manually set interpolate bilinear option to true to
  // achieve a smoother rendering results.
  static bool UseInterpolateBilinear(const FXDIB_ResampleOptions& options,
                                     int dest_width,
                                     int dest_height,
                                     int src_width,
                                     int src_height);

  struct PixelWeight {
    void SetStartEnd(int src_start, int src_end, size_t weight_count) {
      CHECK_LT(src_end - src_start, static_cast<int>(weight_count));
      src_start_ = src_start;
      src_end_ = src_end;
    }

    uint32_t GetWeightForPosition(int position) const {
      CHECK_GE(position, src_start_);
      CHECK_LE(position, src_end_);
      // SAFETY: enforced by checks above.
      return UNSAFE_BUFFERS(weights_[position - src_start_]);
    }

    void SetWeightForPosition(int position, uint32_t weight) {
      CHECK_GE(position, src_start_);
      CHECK_LE(position, src_end_);
      // SAFETY: enforced by checks above.
      UNSAFE_BUFFERS(weights_[position - src_start_] = weight);
    }

    // NOTE: relies on defined behaviour for unsigned overflow to
    // decrement the previous position, as needed.
    void RemoveLastWeightAndAdjust(uint32_t weight_change) {
      CHECK_GT(src_end_, src_start_);
      --src_end_;
      // SAFETY: enforced by checks above.
      UNSAFE_BUFFERS(weights_[src_end_ - src_start_] += weight_change);
    }

    int src_start_;
    int src_end_;          // Note: inclusive, [0, -1] for empty range at 0.
    uint32_t weights_[1];  // Not really 1, variable size.
  };

  class WeightTable {
   public:
    WeightTable();
    ~WeightTable();

    // Accepts a negative `dest_len` argument, producing a "mirror
    // image" of the result if `dest_len` is negative.
    bool CalculateWeights(int dest_len,
                          int dest_min,
                          int dest_max,
                          int src_len,
                          int src_min,
                          int src_max,
                          const FXDIB_ResampleOptions& options);

    const PixelWeight* GetPixelWeight(int pixel) const;
    PixelWeight* GetPixelWeight(int pixel);

   private:
    int dest_min_ = 0;
    size_t item_size_bytes_ = 0;
    size_t weight_tables_size_bytes_ = 0;
    DataVector<uint8_t> weight_tables_;
  };

  CStretchEngine(ScanlineComposerIface* pDestBitmap,
                 FXDIB_Format dest_format,
                 int dest_width,
                 int dest_height,
                 const FX_RECT& clip_rect,
                 const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                 const FXDIB_ResampleOptions& options);
  ~CStretchEngine();

  bool Continue(PauseIndicatorIface* pPause);
  bool StartStretchHorz();
  bool ContinueStretchHorz(PauseIndicatorIface* pPause);
  void StretchVert();

  const FXDIB_ResampleOptions& GetResampleOptionsForTest() const {
    return resample_options_;
  }

 private:
  enum class State : uint8_t { kInitial, kHorizontal, kVertical };

  enum class TransformMethod : uint8_t {
    k1BppTo8Bpp,
    k1BppToManyBpp,
    k8BppTo8Bpp,
    k8BppToManyBpp,
    kManyBpptoManyBpp,
    kManyBpptoManyBppWithAlpha
  };

  const FXDIB_Format dest_format_;
  const int dest_bpp_;
  const int src_bpp_;
  const bool has_alpha_;
  RetainPtr<const CFX_DIBBase> const source_;
  pdfium::raw_span<const uint32_t> src_palette_;
  const int src_width_;
  const int src_height_;
  UnownedPtr<ScanlineComposerIface> const dest_bitmap_;
  const int dest_width_;
  const int dest_height_;
  const FX_RECT dest_clip_;
  DataVector<uint8_t> dest_scanline_;
  FixedSizeDataVector<uint8_t> inter_buf_;
  FX_RECT src_clip_;
  int inter_pitch_;
  int extra_mask_pitch_;
  FXDIB_ResampleOptions resample_options_;
  TransformMethod trans_method_;
  State state_ = State::kInitial;
  int cur_row_ = 0;
  WeightTable weight_table_;
};

#endif  // CORE_FXGE_DIB_CSTRETCHENGINE_H_
