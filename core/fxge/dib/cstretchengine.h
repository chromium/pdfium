// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CSTRETCHENGINE_H_
#define CORE_FXGE_DIB_CSTRETCHENGINE_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check_op.h"
#include "third_party/base/span.h"

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

  static inline uint32_t FixedFromFloat(float f) {
    return static_cast<uint32_t>(FXSYS_roundf(f * kFixedPointOne));
  }

  static inline uint8_t PixelFromFixed(uint32_t fixed) {
    return static_cast<uint8_t>(fixed >> kFixedPointBits);
  }

  struct PixelWeight {
    static size_t TotalBytesForWeightCount(size_t weight_count);

    void SetStartEnd(int src_start, int src_end, size_t weight_count) {
      CHECK_LT(src_end - src_start, static_cast<int>(weight_count));
      m_SrcStart = src_start;
      m_SrcEnd = src_end;
    }

    uint32_t GetWeightForPosition(int position) const {
      CHECK_GE(position, m_SrcStart);
      CHECK_LE(position, m_SrcEnd);
      return m_Weights[position - m_SrcStart];
    }

    void SetWeightForPosition(int position, uint32_t weight) {
      CHECK_GE(position, m_SrcStart);
      CHECK_LE(position, m_SrcEnd);
      m_Weights[position - m_SrcStart] = weight;
    }

    // NOTE: relies on defined behaviour for unsigned overflow to
    // decrement the previous position, as needed.
    void RemoveLastWeightAndAdjust(uint32_t weight_change) {
      CHECK_GT(m_SrcEnd, m_SrcStart);
      --m_SrcEnd;
      m_Weights[m_SrcEnd - m_SrcStart] += weight_change;
    }

    int m_SrcStart;
    int m_SrcEnd;           // Note: inclusive, [0, -1] for empty range at 0.
    uint32_t m_Weights[1];  // Not really 1, variable size.
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
    PixelWeight* GetPixelWeight(int pixel) {
      return const_cast<PixelWeight*>(
          static_cast<const WeightTable*>(this)->GetPixelWeight(pixel));
    }

   private:
    int m_DestMin = 0;
    size_t m_ItemSizeBytes = 0;
    size_t m_WeightTablesSizeBytes = 0;
    std::vector<uint8_t, FxAllocAllocator<uint8_t>> m_WeightTables;
  };

  CStretchEngine(ScanlineComposerIface* pDestBitmap,
                 FXDIB_Format dest_format,
                 int dest_width,
                 int dest_height,
                 const FX_RECT& clip_rect,
                 const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                 const FXDIB_ResampleOptions& options);
  ~CStretchEngine();

  bool Continue(PauseIndicatorIface* pPause);
  bool StartStretchHorz();
  bool ContinueStretchHorz(PauseIndicatorIface* pPause);
  void StretchVert();

  const FXDIB_ResampleOptions& GetResampleOptionsForTest() const {
    return m_ResampleOptions;
  }

 private:
  enum class State : uint8_t { kInitial, kHorizontal, kVertical };

  enum class TransformMethod : uint8_t {
    k1BppTo8Bpp,
    k1BppToManyBpp,
    k8BppTo8Bpp,
    k8BppTo8BppWithAlpha,
    k8BppToManyBpp,
    k8BppToManyBppWithAlpha,
    kManyBpptoManyBpp,
    kManyBpptoManyBppWithAlpha
  };

  const FXDIB_Format m_DestFormat;
  const int m_DestBpp;
  const int m_SrcBpp;
  const int m_bHasAlpha;
  RetainPtr<CFX_DIBBase> const m_pSource;
  pdfium::span<const uint32_t> m_pSrcPalette;
  const int m_SrcWidth;
  const int m_SrcHeight;
  UnownedPtr<ScanlineComposerIface> const m_pDestBitmap;
  const int m_DestWidth;
  const int m_DestHeight;
  const FX_RECT m_DestClip;
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> m_DestScanline;
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> m_DestMaskScanline;
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> m_InterBuf;
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> m_ExtraAlphaBuf;
  FX_RECT m_SrcClip;
  int m_InterPitch;
  int m_ExtraMaskPitch;
  FXDIB_ResampleOptions m_ResampleOptions;
  TransformMethod m_TransMethod;
  State m_State = State::kInitial;
  int m_CurRow;
  WeightTable m_WeightTable;
};

#endif  // CORE_FXGE_DIB_CSTRETCHENGINE_H_
