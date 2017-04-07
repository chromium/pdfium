// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_SCANLINECOMPOSITOR_H_
#define CORE_FXGE_DIB_CFX_SCANLINECOMPOSITOR_H_

#include "core/fxge/dib/cfx_dibsource.h"

class CFX_ScanlineCompositor {
 public:
  CFX_ScanlineCompositor();

  ~CFX_ScanlineCompositor();

  bool Init(FXDIB_Format dest_format,
            FXDIB_Format src_format,
            int32_t width,
            uint32_t* pSrcPalette,
            uint32_t mask_color,
            int blend_type,
            bool bClip,
            bool bRgbByteOrder = false,
            int alpha_flag = 0);

  void CompositeRgbBitmapLine(uint8_t* dest_scan,
                              const uint8_t* src_scan,
                              int width,
                              const uint8_t* clip_scan,
                              const uint8_t* src_extra_alpha = nullptr,
                              uint8_t* dst_extra_alpha = nullptr);

  void CompositePalBitmapLine(uint8_t* dest_scan,
                              const uint8_t* src_scan,
                              int src_left,
                              int width,
                              const uint8_t* clip_scan,
                              const uint8_t* src_extra_alpha = nullptr,
                              uint8_t* dst_extra_alpha = nullptr);

  void CompositeByteMaskLine(uint8_t* dest_scan,
                             const uint8_t* src_scan,
                             int width,
                             const uint8_t* clip_scan,
                             uint8_t* dst_extra_alpha = nullptr);

  void CompositeBitMaskLine(uint8_t* dest_scan,
                            const uint8_t* src_scan,
                            int src_left,
                            int width,
                            const uint8_t* clip_scan,
                            uint8_t* dst_extra_alpha = nullptr);

 private:
  int m_Transparency;
  FXDIB_Format m_SrcFormat;
  FXDIB_Format m_DestFormat;
  uint32_t* m_pSrcPalette;
  int m_MaskAlpha;
  int m_MaskRed;
  int m_MaskGreen;
  int m_MaskBlue;
  int m_MaskBlack;
  int m_BlendType;
  uint8_t* m_pCacheScanline;
  int m_CacheSize;
  bool m_bRgbByteOrder;
};

#endif  // CORE_FXGE_DIB_CFX_SCANLINECOMPOSITOR_H_
