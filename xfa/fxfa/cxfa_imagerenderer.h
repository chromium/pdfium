// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_IMAGERENDERER_H_
#define XFA_FXFA_CXFA_IMAGERENDERER_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_RenderDevice;
class CFX_DIBBase;
class CFX_DIBitmap;
class CFX_ImageTransformer;
class CFX_ImageRenderer;

class CXFA_ImageRenderer {
 public:
  CXFA_ImageRenderer(CFX_RenderDevice* pDevice,
                     const RetainPtr<CFX_DIBBase>& pDIBBase,
                     const CFX_Matrix& mtImage2Device);
  ~CXFA_ImageRenderer();

  bool Start();
  bool Continue();

 private:
  enum class State : uint8_t { kInitial = 0, kTransforming, kStarted };

  void CompositeDIBitmap(const RetainPtr<CFX_DIBitmap>& pDIBitmap,
                         int left,
                         int top);

  State m_State = State::kInitial;
  CFX_Matrix m_ImageMatrix;
  UnownedPtr<CFX_RenderDevice> m_pDevice;
  RetainPtr<CFX_DIBBase> m_pDIBBase;
  RetainPtr<CFX_DIBitmap> m_pCloneConvert;
  std::unique_ptr<CFX_ImageTransformer> m_pTransformer;
  std::unique_ptr<CFX_ImageRenderer> m_DeviceHandle;
};

#endif  // XFA_FXFA_CXFA_IMAGERENDERER_H_
