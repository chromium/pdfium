// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_IMAGERENDERER_H_
#define CORE_FXGE_DIB_CFX_IMAGERENDERER_H_

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/cfx_bitmapcomposer.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_dibsource.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/stl_util.h"

class CFX_ImageTransformer;
class CFX_ImageStretcher;

class CFX_ImageRenderer {
 public:
  CFX_ImageRenderer();
  ~CFX_ImageRenderer();

  bool Start(const CFX_RetainPtr<CFX_DIBitmap>& pDevice,
             const CFX_ClipRgn* pClipRgn,
             const CFX_RetainPtr<CFX_DIBSource>& pSource,
             int bitmap_alpha,
             uint32_t mask_color,
             const CFX_Matrix* pMatrix,
             uint32_t dib_flags,
             bool bRgbByteOrder);

  bool Continue(IFX_Pause* pPause);

 private:
  CFX_RetainPtr<CFX_DIBitmap> m_pDevice;
  const CFX_ClipRgn* m_pClipRgn;
  int m_BitmapAlpha;
  uint32_t m_MaskColor;
  CFX_Matrix m_Matrix;
  std::unique_ptr<CFX_ImageTransformer> m_pTransformer;
  std::unique_ptr<CFX_ImageStretcher> m_Stretcher;
  CFX_BitmapComposer m_Composer;
  int m_Status;
  FX_RECT m_ClipBox;
  uint32_t m_Flags;
  int m_AlphaFlag;
  bool m_bRgbByteOrder;
  int m_BlendType;
};

#endif  // CORE_FXGE_DIB_CFX_IMAGERENDERER_H_
