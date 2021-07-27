// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_DIBITMAP_H_
#define CORE_FXGE_DIB_CFX_DIBITMAP_H_

#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/optional.h"

class CFX_DIBitmap : public CFX_DIBBase {
 public:
  struct PitchAndSize {
    uint32_t pitch;
    uint32_t size;
  };

  CONSTRUCT_VIA_MAKE_RETAIN;

  bool Create(int width, int height, FXDIB_Format format);
  bool Create(int width,
              int height,
              FXDIB_Format format,
              uint8_t* pBuffer,
              uint32_t pitch);

  bool Copy(const RetainPtr<CFX_DIBBase>& pSrc);

  // CFX_DIBBase
  uint8_t* GetBuffer() const override;
  const uint8_t* GetScanline(int line) const override;

  void TakeOver(RetainPtr<CFX_DIBitmap>&& pSrcBitmap);
  bool ConvertFormat(FXDIB_Format format);
  void Clear(uint32_t color);

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  uint32_t GetPixel(int x, int y) const;
#endif
#if defined(_SKIA_SUPPORT_)
  void SetPixel(int x, int y, uint32_t color);
#endif

  bool SetRedFromBitmap(const RetainPtr<CFX_DIBBase>& pSrcBitmap);
  bool SetAlphaFromBitmap(const RetainPtr<CFX_DIBBase>& pSrcBitmap);
  bool SetUniformOpaqueAlpha();

  bool MultiplyAlpha(int alpha);
  bool MultiplyAlpha(const RetainPtr<CFX_DIBBase>& pSrcBitmap);

  bool TransferBitmap(int dest_left,
                      int dest_top,
                      int width,
                      int height,
                      const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                      int src_left,
                      int src_top);

  bool CompositeBitmap(int dest_left,
                       int dest_top,
                       int width,
                       int height,
                       const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                       int src_left,
                       int src_top,
                       BlendMode blend_type,
                       const CFX_ClipRgn* pClipRgn,
                       bool bRgbByteOrder);

  bool CompositeMask(int dest_left,
                     int dest_top,
                     int width,
                     int height,
                     const RetainPtr<CFX_DIBBase>& pMask,
                     uint32_t color,
                     int src_left,
                     int src_top,
                     BlendMode blend_type,
                     const CFX_ClipRgn* pClipRgn,
                     bool bRgbByteOrder);

  bool CompositeRect(int dest_left,
                     int dest_top,
                     int width,
                     int height,
                     uint32_t color);

  bool ConvertColorScale(uint32_t forecolor, uint32_t backcolor);

  // |width| and |height| must be greater than 0.
  // |format| must have a valid bits per pixel count.
  // If |pitch| is zero, then the actual pitch will be calculated based on
  // |width| and |format|.
  // If |pitch| is non-zero, then that be used as the actual pitch.
  // The actual pitch will be used to calculate the size.
  // Returns the calculated pitch and size on success, or nullopt on failure.
  static Optional<PitchAndSize> CalculatePitchAndSize(int width,
                                                      int height,
                                                      FXDIB_Format format,
                                                      uint32_t pitch);

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  void PreMultiply();
#endif
#if defined(_SKIA_SUPPORT_PATHS_)
  void UnPreMultiply();
#endif

 protected:
  CFX_DIBitmap();
  CFX_DIBitmap(const CFX_DIBitmap& src);
  ~CFX_DIBitmap() override;

#if defined(_SKIA_SUPPORT_PATHS_)
  enum class Format { kCleared, kPreMultiplied, kUnPreMultiplied };
#endif

  MaybeOwned<uint8_t, FxFreeDeleter> m_pBuffer;
#if defined(_SKIA_SUPPORT_PATHS_)
  Format m_nFormat = Format::kCleared;
#endif

 private:
  enum class Channel : uint8_t { kRed, kAlpha };

  bool SetChannelFromBitmap(Channel destChannel,
                            const RetainPtr<CFX_DIBBase>& pSrcBitmap);
  void ConvertBGRColorScale(uint32_t forecolor, uint32_t backcolor);
  bool TransferWithUnequalFormats(FXDIB_Format dest_format,
                                  int dest_left,
                                  int dest_top,
                                  int width,
                                  int height,
                                  const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                  int src_left,
                                  int src_top);
  void TransferWithMultipleBPP(int dest_left,
                               int dest_top,
                               int width,
                               int height,
                               const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top);
  void TransferEqualFormatsOneBPP(int dest_left,
                                  int dest_top,
                                  int width,
                                  int height,
                                  const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                  int src_left,
                                  int src_top);
};

#endif  // CORE_FXGE_DIB_CFX_DIBITMAP_H_
