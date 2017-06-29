// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_CFX_IMAGETRANSFORMER_H_
#define CORE_FXGE_DIB_CFX_IMAGETRANSFORMER_H_

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/cfx_bitmapstorer.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_dibsource.h"

class CFX_ImageStretcher;

class CFX_ImageTransformer {
 public:
  CFX_ImageTransformer(const CFX_RetainPtr<CFX_DIBSource>& pSrc,
                       const CFX_Matrix* pMatrix,
                       int flags,
                       const FX_RECT* pClip);
  ~CFX_ImageTransformer();

  bool Continue(IFX_Pause* pPause);

  const FX_RECT& result() const { return m_result; }
  CFX_RetainPtr<CFX_DIBitmap> DetachBitmap();

 private:
  const CFX_RetainPtr<CFX_DIBSource> m_pSrc;
  CFX_UnownedPtr<const CFX_Matrix> const m_pMatrix;
  const FX_RECT* const m_pClip;
  FX_RECT m_StretchClip;
  FX_RECT m_result;
  CFX_Matrix m_dest2stretch;
  std::unique_ptr<CFX_ImageStretcher> m_Stretcher;
  CFX_BitmapStorer m_Storer;
  const uint32_t m_Flags;
  int m_Status;
};

#endif  // CORE_FXGE_DIB_CFX_IMAGETRANSFORMER_H_
