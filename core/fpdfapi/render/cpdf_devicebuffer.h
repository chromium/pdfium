// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_DEVICEBUFFER_H_
#define CORE_FPDFAPI_RENDER_CPDF_DEVICEBUFFER_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_DIBitmap;
class CFX_RenderDevice;
class CPDF_PageObject;
class CPDF_RenderContext;

class CPDF_DeviceBuffer {
 public:
  static CFX_Matrix CalculateMatrix(CFX_RenderDevice* pDevice,
                                    const FX_RECT& rect,
                                    int max_dpi,
                                    bool scale);

  CPDF_DeviceBuffer(CPDF_RenderContext* pContext,
                    CFX_RenderDevice* pDevice,
                    const FX_RECT& rect,
                    const CPDF_PageObject* pObj,
                    int max_dpi);
  ~CPDF_DeviceBuffer();

  bool Initialize();
  void OutputToDevice();
  RetainPtr<CFX_DIBitmap> GetBitmap() const { return m_pBitmap; }
  const CFX_Matrix& GetMatrix() const { return m_Matrix; }

 private:
  UnownedPtr<CFX_RenderDevice> const m_pDevice;
  UnownedPtr<CPDF_RenderContext> const m_pContext;
  UnownedPtr<const CPDF_PageObject> const m_pObject;
  RetainPtr<CFX_DIBitmap> const m_pBitmap;
  const FX_RECT m_Rect;
  const CFX_Matrix m_Matrix;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_DEVICEBUFFER_H_
