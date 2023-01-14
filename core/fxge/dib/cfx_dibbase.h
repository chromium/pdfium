// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_DIBBASE_H_
#define CORE_FXGE_DIB_CFX_DIBBASE_H_

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/span.h"

class CFX_ClipRgn;
class CFX_DIBitmap;
class CFX_Matrix;
class PauseIndicatorIface;
struct FX_RECT;

// Base class for all Device-Independent Bitmaps.
class CFX_DIBBase : public Retainable {
 public:
#if BUILDFLAG(IS_APPLE)
  // Matches Apple's kCGBitmapByteOrder32Little in fx_quartz_device.cpp.
  static constexpr FXDIB_Format kPlatformRGBFormat = FXDIB_Format::kRgb32;
#else   // BUILDFLAG(IS_APPLE)
  static constexpr FXDIB_Format kPlatformRGBFormat = FXDIB_Format::kRgb;
#endif  // BUILDFLAG(IS_APPLE)

  static constexpr uint32_t kPaletteSize = 256;

  ~CFX_DIBBase() override;

  virtual pdfium::span<uint8_t> GetBuffer() const;
  virtual pdfium::span<const uint8_t> GetScanline(int line) const = 0;
  virtual bool SkipToScanline(int line, PauseIndicatorIface* pPause) const;
  virtual size_t GetEstimatedImageMemoryBurden() const;

  pdfium::span<uint8_t> GetWritableScanline(int line) {
    pdfium::span<const uint8_t> src = GetScanline(line);
    return {const_cast<uint8_t*>(src.data()), src.size()};
  }
  int GetWidth() const { return m_Width; }
  int GetHeight() const { return m_Height; }
  uint32_t GetPitch() const { return m_Pitch; }

  FXDIB_Format GetFormat() const { return m_Format; }
  int GetBPP() const { return GetBppFromFormat(m_Format); }
  bool IsMaskFormat() const { return GetIsMaskFromFormat(m_Format); }
  bool IsAlphaFormat() const { return m_Format == FXDIB_Format::kArgb; }
  bool IsOpaqueImage() const { return !IsMaskFormat() && !IsAlphaFormat(); }

  bool HasPalette() const { return !m_palette.empty(); }
  pdfium::span<const uint32_t> GetPaletteSpan() const { return m_palette; }
  size_t GetRequiredPaletteSize() const;
  uint32_t GetPaletteArgb(int index) const;
  void SetPaletteArgb(int index, uint32_t color);

  // Copies into internally-owned palette.
  void SetPalette(pdfium::span<const uint32_t> src_palette);

  RetainPtr<CFX_DIBitmap> Realize() const;
  RetainPtr<CFX_DIBitmap> ClipTo(const FX_RECT& rect) const;
  RetainPtr<CFX_DIBitmap> ConvertTo(FXDIB_Format format) const;
  RetainPtr<CFX_DIBitmap> StretchTo(int dest_width,
                                    int dest_height,
                                    const FXDIB_ResampleOptions& options,
                                    const FX_RECT* pClip) const;
  RetainPtr<CFX_DIBitmap> TransformTo(const CFX_Matrix& mtDest,
                                      int* left,
                                      int* top) const;
  RetainPtr<CFX_DIBitmap> SwapXY(bool bXFlip, bool bYFlip) const;
  RetainPtr<CFX_DIBitmap> FlipImage(bool bXFlip, bool bYFlip) const;

  RetainPtr<CFX_DIBitmap> CloneAlphaMask() const;

  bool GetOverlapRect(int& dest_left,
                      int& dest_top,
                      int& width,
                      int& height,
                      int src_width,
                      int src_height,
                      int& src_left,
                      int& src_top,
                      const CFX_ClipRgn* pClipRgn) const;

#ifdef _SKIA_SUPPORT_
  void DebugVerifyBitmapIsPreMultiplied() const;
#endif

 protected:
  CFX_DIBBase();

  static bool ConvertBuffer(FXDIB_Format dest_format,
                            pdfium::span<uint8_t> dest_buf,
                            int dest_pitch,
                            int width,
                            int height,
                            const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                            int src_left,
                            int src_top,
                            DataVector<uint32_t>* pal);

  RetainPtr<CFX_DIBitmap> ClipToInternal(const FX_RECT* pClip) const;
  void BuildPalette();
  int FindPalette(uint32_t color) const;

  FXDIB_Format m_Format = FXDIB_Format::kInvalid;
  int m_Width = 0;
  int m_Height = 0;
  uint32_t m_Pitch = 0;
  DataVector<uint32_t> m_palette;
};

#endif  // CORE_FXGE_DIB_CFX_DIBBASE_H_
