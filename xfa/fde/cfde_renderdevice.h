// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_RENDERDEVICE_H_
#define XFA_FDE_CFDE_RENDERDEVICE_H_

#include <vector>

#include "core/fxge/cfx_renderdevice.h"
#include "xfa/fgas/font/cfgas_gefont.h"

class CFDE_Path;
class CFX_GraphStateData;

class CFDE_RenderDevice {
 public:
  explicit CFDE_RenderDevice(CFX_RenderDevice* pDevice);
  ~CFDE_RenderDevice();

  int32_t GetWidth() const;
  int32_t GetHeight() const;

  void SaveState();
  void RestoreState();

  bool SetClipRect(const CFX_RectF& rtClip);
  const CFX_RectF& GetClipRect();

  bool DrawString(FX_ARGB color,
                  const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                  const FXTEXT_CHARPOS* pCharPos,
                  int32_t iCount,
                  float fFontSize,
                  const CFX_Matrix* pMatrix);
  bool DrawPath(FX_ARGB color,
                float fPenWidth,
                const CFDE_Path* pPath,
                const CFX_Matrix* pMatrix);

 private:
  CFX_RenderDevice* const m_pDevice;
  CFX_RectF m_rtClip;
};

#endif  // XFA_FDE_CFDE_RENDERDEVICE_H_
