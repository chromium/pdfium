// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_BITMAPCOMPOSER_H_
#define CORE_FXGE_DIB_CFX_BITMAPCOMPOSER_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/cfx_scanlinecompositor.h"
#include "core/fxge/dib/ifx_scanlinecomposer.h"

class CFX_ClipRgn;
class CFX_DIBitmap;

class CFX_BitmapComposer : public IFX_ScanlineComposer {
 public:
  CFX_BitmapComposer();
  ~CFX_BitmapComposer() override;

  void Compose(const CFX_RetainPtr<CFX_DIBitmap>& pDest,
               const CFX_ClipRgn* pClipRgn,
               int bitmap_alpha,
               uint32_t mask_color,
               FX_RECT& dest_rect,
               bool bVertical,
               bool bFlipX,
               bool bFlipY,
               bool bRgbByteOrder = false,
               int alpha_flag = 0,
               void* pIccTransform = nullptr,
               int blend_type = FXDIB_BLEND_NORMAL);

  // IFX_ScanlineComposer
  bool SetInfo(int width,
               int height,
               FXDIB_Format src_format,
               uint32_t* pSrcPalette) override;

  void ComposeScanline(int line,
                       const uint8_t* scanline,
                       const uint8_t* scan_extra_alpha) override;

 private:
  void DoCompose(uint8_t* dest_scan,
                 const uint8_t* src_scan,
                 int dest_width,
                 const uint8_t* clip_scan,
                 const uint8_t* src_extra_alpha = nullptr,
                 uint8_t* dst_extra_alpha = nullptr);
  void ComposeScanlineV(int line,
                        const uint8_t* scanline,
                        const uint8_t* scan_extra_alpha = nullptr);

  CFX_RetainPtr<CFX_DIBitmap> m_pBitmap;
  const CFX_ClipRgn* m_pClipRgn;
  FXDIB_Format m_SrcFormat;
  int m_DestLeft;
  int m_DestTop;
  int m_DestWidth;
  int m_DestHeight;
  int m_BitmapAlpha;
  uint32_t m_MaskColor;
  CFX_RetainPtr<CFX_DIBitmap> m_pClipMask;
  CFX_ScanlineCompositor m_Compositor;
  bool m_bVertical;
  bool m_bFlipX;
  bool m_bFlipY;
  int m_AlphaFlag;
  void* m_pIccTransform;
  bool m_bRgbByteOrder;
  int m_BlendType;
  uint8_t* m_pScanlineV;
  uint8_t* m_pClipScanV;
  uint8_t* m_pAddClipScan;
  uint8_t* m_pScanlineAlphaV;
};

#endif  // CORE_FXGE_DIB_CFX_BITMAPCOMPOSER_H_
