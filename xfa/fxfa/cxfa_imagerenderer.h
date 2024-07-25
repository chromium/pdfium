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

class CFX_AggImageRenderer;
class CFX_DIBitmap;
class CFX_RenderDevice;

class CXFA_ImageRenderer {
 public:
  CXFA_ImageRenderer(CFX_RenderDevice* device,
                     RetainPtr<CFX_DIBitmap> bitmap,
                     const CFX_Matrix& image_to_device);
  ~CXFA_ImageRenderer();

  // Returns whether to continue or not.
  bool Start();
  bool Continue();

 private:
  enum class State : bool { kInitial = 0, kStarted };

  State m_State = State::kInitial;
  const CFX_Matrix m_ImageMatrix;
  UnownedPtr<CFX_RenderDevice> const m_pDevice;
  RetainPtr<CFX_DIBitmap> const m_pBitmap;
  std::unique_ptr<CFX_AggImageRenderer> m_DeviceHandle;
};

#endif  // XFA_FXFA_CXFA_IMAGERENDERER_H_
