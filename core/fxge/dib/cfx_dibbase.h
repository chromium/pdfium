// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_DIBBASE_H_
#define CORE_FXGE_DIB_CFX_DIBBASE_H_

#include <vector>

#include "core/fxcrt/fx_memory_wrappers.h"
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
#if defined(OS_APPLE)
  // Matches Apple's kCGBitmapByteOrder32Little in fx_quartz_device.cpp.
  static constexpr FXDIB_Format kPlatformRGBFormat = FXDIB_Format::kRgb32;
#else   // defined(OS_APPLE)
  static constexpr FXDIB_Format kPlatformRGBFormat = FXDIB_Format::kRgb;
#endif  // defined(OS_APPLE)

  ~CFX_DIBBase() override;

  virtual uint8_t* GetBuffer() const;
  virtual const uint8_t* GetScanline(int line) const = 0;
  virtual bool SkipToScanline(int line, PauseIndicatorIface* pPause) const;

  uint8_t* GetWritableScanline(int line) {
    return const_cast<uint8_t*>(GetScanline(line));
  }
  int GetWidth() const { return m_Width; }
  int GetHeight() const { return m_Height; }
  uint32_t GetPitch() const { return m_Pitch; }

  FXDIB_Format GetFormat() const { return m_Format; }
  int GetBPP() const { return GetBppFromFormat(m_Format); }
  bool IsMaskFormat() const { return GetIsMaskFromFormat(m_Format); }
  bool IsAlphaFormat() const { return GetIsAlphaFromFormat(m_Format); }
  bool IsOpaqueImage() const { return !IsMaskFormat() && !IsAlphaFormat(); }

  bool HasPalette() const { return !m_palette.empty(); }
  pdfium::span<const uint32_t> GetPaletteSpan() const { return m_palette; }
  size_t GetRequiredPaletteSize() const;
  uint32_t GetPaletteArgb(int index) const;
  void SetPaletteArgb(int index, uint32_t color);

  // Copies into internally-owned palette.
  void SetPalette(pdfium::span<const uint32_t> src_palette);

  RetainPtr<CFX_DIBitmap> Clone(const FX_RECT* pClip) const;
  RetainPtr<CFX_DIBitmap> CloneConvert(FXDIB_Format format);
  RetainPtr<CFX_DIBitmap> StretchTo(int dest_width,
                                    int dest_height,
                                    const FXDIB_ResampleOptions& options,
                                    const FX_RECT* pClip);
  RetainPtr<CFX_DIBitmap> TransformTo(const CFX_Matrix& mtDest,
                                      int* left,
                                      int* top);
  RetainPtr<CFX_DIBitmap> SwapXY(bool bXFlip, bool bYFlip) const;
  RetainPtr<CFX_DIBitmap> FlipImage(bool bXFlip, bool bYFlip) const;

  bool HasAlphaMask() const { return !!m_pAlphaMask; }
  uint32_t GetAlphaMaskPitch() const;
  const uint8_t* GetAlphaMaskScanline(int line) const;
  uint8_t* GetWritableAlphaMaskScanline(int line);
  uint8_t* GetAlphaMaskBuffer();
  RetainPtr<CFX_DIBitmap> GetAlphaMask();
  RetainPtr<CFX_DIBitmap> CloneAlphaMask() const;

  // Copies into internally-owned mask.
  bool SetAlphaMask(const RetainPtr<CFX_DIBBase>& pAlphaMask,
                    const FX_RECT* pClip);

  bool GetOverlapRect(int& dest_left,
                      int& dest_top,
                      int& width,
                      int& height,
                      int src_width,
                      int src_height,
                      int& src_left,
                      int& src_top,
                      const CFX_ClipRgn* pClipRgn) const;

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  void DebugVerifyBitmapIsPreMultiplied(void* buffer) const;
#endif

 protected:
  CFX_DIBBase();

  static bool ConvertBuffer(
      FXDIB_Format dest_format,
      uint8_t* dest_buf,
      int dest_pitch,
      int width,
      int height,
      const RetainPtr<CFX_DIBBase>& pSrcBitmap,
      int src_left,
      int src_top,
      std::vector<uint32_t, FxAllocAllocator<uint32_t>>* pal);

  void BuildPalette();
  bool BuildAlphaMask();
  int FindPalette(uint32_t color) const;
  void GetPalette(uint32_t* pal, int alpha) const;

  FXDIB_Format m_Format = FXDIB_Format::kInvalid;
  int m_Width = 0;
  int m_Height = 0;
  uint32_t m_Pitch = 0;
  RetainPtr<CFX_DIBitmap> m_pAlphaMask;
  std::vector<uint32_t, FxAllocAllocator<uint32_t>> m_palette;
};

#endif  // CORE_FXGE_DIB_CFX_DIBBASE_H_
