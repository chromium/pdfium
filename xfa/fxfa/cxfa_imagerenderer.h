// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_IMAGERENDERER_H_
#define XFA_FXFA_CXFA_IMAGERENDERER_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/fx_dib.h"

class CFX_RenderDevice;
class CFX_DIBSource;
class CFX_DIBitmap;
class CFX_ImageTransformer;
class CFX_ImageRenderer;

class CXFA_ImageRenderer {
 public:
  CXFA_ImageRenderer(CFX_RenderDevice* pDevice,
                     const RetainPtr<CFX_DIBSource>& pDIBSource,
                     const CFX_Matrix* pImage2Device,
                     uint32_t flags);
  ~CXFA_ImageRenderer();

  bool Start();
  bool Continue();

 protected:
  void CompositeDIBitmap(const RetainPtr<CFX_DIBitmap>& pDIBitmap,
                         int left,
                         int top,
                         int blend_mode,
                         int iTransparency);

  CFX_RenderDevice* m_pDevice;
  int m_Status = 0;
  CFX_Matrix m_ImageMatrix;
  RetainPtr<CFX_DIBSource> m_pDIBSource;
  RetainPtr<CFX_DIBitmap> m_pCloneConvert;
  uint32_t m_Flags;
  std::unique_ptr<CFX_ImageTransformer> m_pTransformer;
  std::unique_ptr<CFX_ImageRenderer> m_DeviceHandle;
  int32_t m_BlendType = FXDIB_BLEND_NORMAL;
  bool m_bPrint = false;
};

#endif  // XFA_FXFA_CXFA_IMAGERENDERER_H_
