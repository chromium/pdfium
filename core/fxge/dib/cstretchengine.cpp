// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cstretchengine.h"

#include <limits.h>
#include <math.h>

#include <algorithm>
#include <type_traits>
#include <utility>

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/pauseindicator_iface.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/dib/scanlinecomposer_iface.h"
#include "third_party/base/check.h"
#include "third_party/base/cxx17_backports.h"

static_assert(
    std::is_trivially_destructible<CStretchEngine::PixelWeight>::value,
    "PixelWeight storage may be re-used without invoking its destructor");

namespace {

int GetPitchRoundUpTo4Bytes(int bits_per_pixel) {
  return (bits_per_pixel + 31) / 32 * 4;
}

}  // namespace

// static
size_t CStretchEngine::PixelWeight::TotalBytesForWeightCount(
    size_t weight_count) {
  // Always room for one weight even for empty ranges due to declaration
  // of m_Weights[1] in the header. Don't shrink below this since
  // CalculateWeights() relies on this later.
  const size_t extra_weights = weight_count > 0 ? weight_count - 1 : 0;
  FX_SAFE_SIZE_T total_bytes = extra_weights;
  total_bytes *= sizeof(m_Weights[0]);
  total_bytes += sizeof(PixelWeight);
  return total_bytes.ValueOrDie();
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

  m_DestMin = 0;
  m_ItemSizeBytes = 0;
  m_WeightTablesSizeBytes = 0;
  m_WeightTables.clear();
  if (dest_len == 0)
    return true;

  if (dest_min > dest_max)
    return false;

  m_DestMin = dest_min;

  const double scale = static_cast<double>(src_len) / dest_len;
  const double base = dest_len < 0 ? src_len : 0;
  const size_t weight_count = static_cast<size_t>(ceil(fabs(scale))) + 1;
  m_ItemSizeBytes = PixelWeight::TotalBytesForWeightCount(weight_count);

  const size_t dest_range = static_cast<size_t>(dest_max - dest_min);
  const size_t kMaxTableItemsAllowed = kMaxTableBytesAllowed / m_ItemSizeBytes;
  if (dest_range > kMaxTableItemsAllowed)
    return false;

  m_WeightTablesSizeBytes = dest_range * m_ItemSizeBytes;
  m_WeightTables.resize(m_WeightTablesSizeBytes);
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
        if (pixel_weights.m_SrcStart >= pixel_weights.m_SrcEnd) {
          // Always room for one weight per size calculation.
          pixel_weights.m_Weights[0] = kFixedPointOne;
        } else {
          pixel_weights.m_Weights[1] =
              FixedFromDouble(src_pos - pixel_weights.m_SrcStart - 0.5f);
          pixel_weights.m_Weights[0] =
              kFixedPointOne - pixel_weights.m_Weights[1];
        }
      } else {
        int pixel_pos = static_cast<int>(floor(src_pos));
        int src_start = std::max(pixel_pos, src_min);
        int src_end = std::min(pixel_pos, src_max - 1);
        pixel_weights.SetStartEnd(src_start, src_end, weight_count);
        pixel_weights.m_Weights[0] = kFixedPointOne;
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
      if (dest_start > dest_end)
        std::swap(dest_start, dest_end);
      double area_start = std::max(dest_start, static_cast<double>(dest_pixel));
      double area_end = std::min(dest_end, static_cast<double>(dest_pixel + 1));
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
  return true;
}

const CStretchEngine::PixelWeight* CStretchEngine::WeightTable::GetPixelWeight(
    int pixel) const {
  DCHECK(pixel >= m_DestMin);
  return reinterpret_cast<const PixelWeight*>(
      &m_WeightTables[(pixel - m_DestMin) * m_ItemSizeBytes]);
}

CStretchEngine::CStretchEngine(ScanlineComposerIface* pDestBitmap,
                               FXDIB_Format dest_format,
                               int dest_width,
                               int dest_height,
                               const FX_RECT& clip_rect,
                               const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                               const FXDIB_ResampleOptions& options)
    : m_DestFormat(dest_format),
      m_DestBpp(GetBppFromFormat(dest_format)),
      m_SrcBpp(pSrcBitmap->GetBPP()),
      m_bHasAlpha(pSrcBitmap->IsAlphaFormat()),
      m_pSource(pSrcBitmap),
      m_pSrcPalette(pSrcBitmap->GetPaletteSpan()),
      m_SrcWidth(pSrcBitmap->GetWidth()),
      m_SrcHeight(pSrcBitmap->GetHeight()),
      m_pDestBitmap(pDestBitmap),
      m_DestWidth(dest_width),
      m_DestHeight(dest_height),
      m_DestClip(clip_rect) {
  uint32_t size = clip_rect.Width();
  if (size && m_DestBpp > static_cast<int>(INT_MAX / size))
    return;

  size *= m_DestBpp;
  if (size > INT_MAX - 31)
    return;

  size = GetPitchRoundUpTo4Bytes(size);
  m_DestScanline.resize(size);
  if (dest_format == FXDIB_Format::kRgb32)
    std::fill(m_DestScanline.begin(), m_DestScanline.end(), 255);
  m_InterPitch = GetPitchRoundUpTo4Bytes(m_DestClip.Width() * m_DestBpp);
  m_ExtraMaskPitch = GetPitchRoundUpTo4Bytes(m_DestClip.Width() * 8);
  if (options.bNoSmoothing) {
    m_ResampleOptions.bNoSmoothing = true;
  } else {
    if (!options.bInterpolateBilinear && abs(dest_width) != 0 &&
        abs(dest_height) / 8 < static_cast<long long>(m_SrcWidth) *
                                   m_SrcHeight / abs(dest_width)) {
      m_ResampleOptions.bInterpolateBilinear = true;
    } else {
      m_ResampleOptions = options;
    }
  }
  double scale_x = static_cast<float>(m_SrcWidth) / m_DestWidth;
  double scale_y = static_cast<float>(m_SrcHeight) / m_DestHeight;
  double base_x = m_DestWidth > 0 ? 0.0f : m_DestWidth;
  double base_y = m_DestHeight > 0 ? 0.0f : m_DestHeight;
  double src_left = scale_x * (clip_rect.left + base_x);
  double src_right = scale_x * (clip_rect.right + base_x);
  double src_top = scale_y * (clip_rect.top + base_y);
  double src_bottom = scale_y * (clip_rect.bottom + base_y);
  if (src_left > src_right)
    std::swap(src_left, src_right);
  if (src_top > src_bottom)
    std::swap(src_top, src_bottom);
  m_SrcClip.left = static_cast<int>(floor(src_left));
  m_SrcClip.right = static_cast<int>(ceil(src_right));
  m_SrcClip.top = static_cast<int>(floor(src_top));
  m_SrcClip.bottom = static_cast<int>(ceil(src_bottom));
  FX_RECT src_rect(0, 0, m_SrcWidth, m_SrcHeight);
  m_SrcClip.Intersect(src_rect);

  switch (m_SrcBpp) {
    case 1:
      m_TransMethod = m_DestBpp == 8 ? TransformMethod::k1BppTo8Bpp
                                     : TransformMethod::k1BppToManyBpp;
      break;
    case 8:
      if (m_DestBpp == 8) {
        m_TransMethod = m_bHasAlpha ? TransformMethod::k8BppTo8BppWithAlpha
                                    : TransformMethod::k8BppTo8Bpp;
      } else {
        m_TransMethod = m_bHasAlpha ? TransformMethod::k8BppToManyBppWithAlpha
                                    : TransformMethod::k8BppToManyBpp;
      }
      break;
    default:
      m_TransMethod = m_bHasAlpha ? TransformMethod::kManyBpptoManyBppWithAlpha
                                  : TransformMethod::kManyBpptoManyBpp;
      break;
  }
}

CStretchEngine::~CStretchEngine() = default;

bool CStretchEngine::Continue(PauseIndicatorIface* pPause) {
  while (m_State == State::kHorizontal) {
    if (ContinueStretchHorz(pPause))
      return true;

    m_State = State::kVertical;
    StretchVert();
  }
  return false;
}

bool CStretchEngine::StartStretchHorz() {
  if (m_DestWidth == 0 || m_InterPitch == 0 || m_DestScanline.empty())
    return false;

  if (m_SrcClip.Height() == 0 ||
      m_SrcClip.Height() > (1 << 29) / m_InterPitch) {
    return false;
  }
  m_InterBuf.resize(m_SrcClip.Height() * m_InterPitch);
  if (m_pSource && m_bHasAlpha && m_pSource->HasAlphaMask()) {
    m_ExtraAlphaBuf.resize(m_SrcClip.Height(), m_ExtraMaskPitch);
    m_DestMaskScanline.resize(m_ExtraMaskPitch);
  }
  if (!m_WeightTable.CalculateWeights(
          m_DestWidth, m_DestClip.left, m_DestClip.right, m_SrcWidth,
          m_SrcClip.left, m_SrcClip.right, m_ResampleOptions)) {
    return false;
  }
  m_CurRow = m_SrcClip.top;
  m_State = State::kHorizontal;
  return true;
}

bool CStretchEngine::ContinueStretchHorz(PauseIndicatorIface* pPause) {
  if (!m_DestWidth)
    return false;
  if (m_pSource->SkipToScanline(m_CurRow, pPause))
    return true;

  int Bpp = m_DestBpp / 8;
  static const int kStrechPauseRows = 10;
  int rows_to_go = kStrechPauseRows;
  for (; m_CurRow < m_SrcClip.bottom; ++m_CurRow) {
    if (rows_to_go == 0) {
      if (pPause && pPause->NeedToPauseNow())
        return true;

      rows_to_go = kStrechPauseRows;
    }

    const uint8_t* src_scan = m_pSource->GetScanline(m_CurRow).data();
    uint8_t* dest_scan =
        m_InterBuf.data() + (m_CurRow - m_SrcClip.top) * m_InterPitch;
    const uint8_t* src_scan_mask = nullptr;
    uint8_t* dest_scan_mask = nullptr;
    if (!m_ExtraAlphaBuf.empty()) {
      src_scan_mask = m_pSource->GetAlphaMaskScanline(m_CurRow).data();
      dest_scan_mask = m_ExtraAlphaBuf.data() +
                       (m_CurRow - m_SrcClip.top) * m_ExtraMaskPitch;
    }
    // TODO(npm): reduce duplicated code here
    switch (m_TransMethod) {
      case TransformMethod::k1BppTo8Bpp:
      case TransformMethod::k1BppToManyBpp: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          PixelWeight* pWeights = m_WeightTable.GetPixelWeight(col);
          uint32_t dest_a = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            if (src_scan[j / 8] & (1 << (7 - j % 8)))
              dest_a += pixel_weight * 255;
          }
          *dest_scan++ = PixelFromFixed(dest_a);
        }
        break;
      }
      case TransformMethod::k8BppTo8Bpp: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          PixelWeight* pWeights = m_WeightTable.GetPixelWeight(col);
          uint32_t dest_a = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            dest_a += pixel_weight * src_scan[j];
          }
          *dest_scan++ = PixelFromFixed(dest_a);
        }
        break;
      }
      case TransformMethod::k8BppTo8BppWithAlpha: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          PixelWeight* pWeights = m_WeightTable.GetPixelWeight(col);
          uint32_t dest_a = 0;
          uint32_t dest_r = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            pixel_weight = pixel_weight * src_scan_mask[j] / 255;
            dest_r += pixel_weight * src_scan[j];
            dest_a += pixel_weight;
          }
          *dest_scan++ = PixelFromFixed(dest_r);
          *dest_scan_mask++ = PixelFromFixed(255 * dest_a);
        }
        break;
      }
      case TransformMethod::k8BppToManyBpp: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          PixelWeight* pWeights = m_WeightTable.GetPixelWeight(col);
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            unsigned long argb = m_pSrcPalette[src_scan[j]];
            if (m_DestFormat == FXDIB_Format::kRgb) {
              dest_r += pixel_weight * static_cast<uint8_t>(argb >> 16);
              dest_g += pixel_weight * static_cast<uint8_t>(argb >> 8);
              dest_b += pixel_weight * static_cast<uint8_t>(argb);
            } else {
              dest_b += pixel_weight * static_cast<uint8_t>(argb >> 24);
              dest_g += pixel_weight * static_cast<uint8_t>(argb >> 16);
              dest_r += pixel_weight * static_cast<uint8_t>(argb >> 8);
            }
          }
          *dest_scan++ = PixelFromFixed(dest_b);
          *dest_scan++ = PixelFromFixed(dest_g);
          *dest_scan++ = PixelFromFixed(dest_r);
        }
        break;
      }
      case TransformMethod::k8BppToManyBppWithAlpha: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          PixelWeight* pWeights = m_WeightTable.GetPixelWeight(col);
          uint32_t dest_a = 0;
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            pixel_weight = pixel_weight * src_scan_mask[j] / 255;
            unsigned long argb = m_pSrcPalette[src_scan[j]];
            dest_b += pixel_weight * static_cast<uint8_t>(argb >> 24);
            dest_g += pixel_weight * static_cast<uint8_t>(argb >> 16);
            dest_r += pixel_weight * static_cast<uint8_t>(argb >> 8);
            dest_a += pixel_weight;
          }
          *dest_scan++ = PixelFromFixed(dest_b);
          *dest_scan++ = PixelFromFixed(dest_g);
          *dest_scan++ = PixelFromFixed(dest_r);
          *dest_scan_mask++ = PixelFromFixed(255 * dest_a);
        }
        break;
      }
      case TransformMethod::kManyBpptoManyBpp: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          PixelWeight* pWeights = m_WeightTable.GetPixelWeight(col);
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            const uint8_t* src_pixel = src_scan + j * Bpp;
            dest_b += pixel_weight * (*src_pixel++);
            dest_g += pixel_weight * (*src_pixel++);
            dest_r += pixel_weight * (*src_pixel);
          }
          *dest_scan++ = PixelFromFixed(dest_b);
          *dest_scan++ = PixelFromFixed(dest_g);
          *dest_scan++ = PixelFromFixed(dest_r);
          dest_scan += Bpp - 3;
        }
        break;
      }
      case TransformMethod::kManyBpptoManyBppWithAlpha: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          PixelWeight* pWeights = m_WeightTable.GetPixelWeight(col);
          uint32_t dest_a = 0;
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            const uint8_t* src_pixel = src_scan + j * Bpp;
            if (m_DestFormat == FXDIB_Format::kArgb) {
              pixel_weight = pixel_weight * src_pixel[3] / 255;
            } else {
              pixel_weight = pixel_weight * src_scan_mask[j] / 255;
            }
            dest_b += pixel_weight * (*src_pixel++);
            dest_g += pixel_weight * (*src_pixel++);
            dest_r += pixel_weight * (*src_pixel);
            dest_a += pixel_weight;
          }
          *dest_scan++ = PixelFromFixed(dest_b);
          *dest_scan++ = PixelFromFixed(dest_g);
          *dest_scan++ = PixelFromFixed(dest_r);
          if (m_DestFormat == FXDIB_Format::kArgb)
            *dest_scan = PixelFromFixed(255 * dest_a);
          if (dest_scan_mask)
            *dest_scan_mask++ = PixelFromFixed(255 * dest_a);
          dest_scan += Bpp - 3;
        }
        break;
      }
    }
    rows_to_go--;
  }
  return false;
}

void CStretchEngine::StretchVert() {
  if (m_DestHeight == 0)
    return;

  WeightTable table;
  if (!table.CalculateWeights(m_DestHeight, m_DestClip.top, m_DestClip.bottom,
                              m_SrcHeight, m_SrcClip.top, m_SrcClip.bottom,
                              m_ResampleOptions)) {
    return;
  }

  const int DestBpp = m_DestBpp / 8;
  for (int row = m_DestClip.top; row < m_DestClip.bottom; ++row) {
    unsigned char* dest_scan = m_DestScanline.data();
    unsigned char* dest_scan_mask = m_DestMaskScanline.data();
    PixelWeight* pWeights = table.GetPixelWeight(row);
    switch (m_TransMethod) {
      case TransformMethod::k1BppTo8Bpp:
      case TransformMethod::k1BppToManyBpp:
      case TransformMethod::k8BppTo8Bpp: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          unsigned char* src_scan =
              m_InterBuf.data() + (col - m_DestClip.left) * DestBpp;
          uint32_t dest_a = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            dest_a +=
                pixel_weight * src_scan[(j - m_SrcClip.top) * m_InterPitch];
          }
          *dest_scan = PixelFromFixed(dest_a);
          dest_scan += DestBpp;
        }
        break;
      }
      case TransformMethod::k8BppTo8BppWithAlpha: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          unsigned char* src_scan =
              m_InterBuf.data() + (col - m_DestClip.left) * DestBpp;
          unsigned char* src_scan_mask =
              m_ExtraAlphaBuf.data() + (col - m_DestClip.left);
          uint32_t dest_a = 0;
          uint32_t dest_k = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            dest_k +=
                pixel_weight * src_scan[(j - m_SrcClip.top) * m_InterPitch];
            dest_a += pixel_weight *
                      src_scan_mask[(j - m_SrcClip.top) * m_ExtraMaskPitch];
          }
          *dest_scan = PixelFromFixed(dest_k);
          dest_scan += DestBpp;
          *dest_scan_mask++ = PixelFromFixed(dest_a);
        }
        break;
      }
      case TransformMethod::k8BppToManyBpp:
      case TransformMethod::kManyBpptoManyBpp: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          unsigned char* src_scan =
              m_InterBuf.data() + (col - m_DestClip.left) * DestBpp;
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            const uint8_t* src_pixel =
                src_scan + (j - m_SrcClip.top) * m_InterPitch;
            dest_b += pixel_weight * (*src_pixel++);
            dest_g += pixel_weight * (*src_pixel++);
            dest_r += pixel_weight * (*src_pixel);
          }
          dest_scan[0] = PixelFromFixed(dest_b);
          dest_scan[1] = PixelFromFixed(dest_g);
          dest_scan[2] = PixelFromFixed(dest_r);
          dest_scan += DestBpp;
        }
        break;
      }
      case TransformMethod::k8BppToManyBppWithAlpha:
      case TransformMethod::kManyBpptoManyBppWithAlpha: {
        for (int col = m_DestClip.left; col < m_DestClip.right; ++col) {
          unsigned char* src_scan =
              m_InterBuf.data() + (col - m_DestClip.left) * DestBpp;
          unsigned char* src_scan_mask = nullptr;
          if (m_DestFormat != FXDIB_Format::kArgb)
            src_scan_mask = m_ExtraAlphaBuf.data() + (col - m_DestClip.left);
          uint32_t dest_a = 0;
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pWeights->m_SrcStart; j <= pWeights->m_SrcEnd; ++j) {
            uint32_t pixel_weight = pWeights->GetWeightForPosition(j);
            const uint8_t* src_pixel =
                src_scan + (j - m_SrcClip.top) * m_InterPitch;
            int mask_v = 255;
            if (src_scan_mask)
              mask_v = src_scan_mask[(j - m_SrcClip.top) * m_ExtraMaskPitch];
            dest_b += pixel_weight * (*src_pixel++);
            dest_g += pixel_weight * (*src_pixel++);
            dest_r += pixel_weight * (*src_pixel);
            if (m_DestFormat == FXDIB_Format::kArgb)
              dest_a += pixel_weight * (*(src_pixel + 1));
            else
              dest_a += pixel_weight * mask_v;
          }
          if (dest_a) {
            int r = static_cast<uint32_t>(dest_r) * 255 / dest_a;
            int g = static_cast<uint32_t>(dest_g) * 255 / dest_a;
            int b = static_cast<uint32_t>(dest_b) * 255 / dest_a;
            dest_scan[0] = pdfium::clamp(b, 0, 255);
            dest_scan[1] = pdfium::clamp(g, 0, 255);
            dest_scan[2] = pdfium::clamp(r, 0, 255);
          }
          if (m_DestFormat == FXDIB_Format::kArgb)
            dest_scan[3] = PixelFromFixed(dest_a);
          else
            *dest_scan_mask = PixelFromFixed(dest_a);
          dest_scan += DestBpp;
          if (dest_scan_mask)
            dest_scan_mask++;
        }
        break;
      }
    }
    m_pDestBitmap->ComposeScanline(row - m_DestClip.top, m_DestScanline,
                                   m_DestMaskScanline);
  }
}
