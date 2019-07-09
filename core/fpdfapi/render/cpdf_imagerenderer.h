// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_IMAGERENDERER_H_
#define CORE_FPDFAPI_RENDER_CPDF_IMAGERENDERER_H_

#include <memory>

#include "core/fpdfapi/render/cpdf_imageloader.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/optional.h"

class CFX_DIBitmap;
class CFX_DIBBase;
class CFX_DefaultRenderDevice;
class CFX_ImageTransformer;
class CPDF_ImageObject;
class CPDF_PageObject;
class CPDF_Pattern;
class CPDF_RenderOptions;
class CPDF_RenderStatus;

class CPDF_ImageRenderer {
 public:
  CPDF_ImageRenderer();
  ~CPDF_ImageRenderer();

  bool Start(CPDF_RenderStatus* pStatus,
             CPDF_ImageObject* pImageObject,
             const CFX_Matrix& mtObj2Device,
             bool bStdCS,
             BlendMode blendType);

  bool Start(CPDF_RenderStatus* pStatus,
             const RetainPtr<CFX_DIBBase>& pDIBBase,
             FX_ARGB bitmap_argb,
             int bitmap_alpha,
             const CFX_Matrix& mtImage2Device,
             const FXDIB_ResampleOptions& options,
             bool bStdCS,
             BlendMode blendType);

  bool Continue(PauseIndicatorIface* pPause);
  bool GetResult() const { return m_Result; }

 private:
  enum class Mode {
    kNone = 0,
    kDefault,
    kBlend,
    kTransform,
  };

  bool StartBitmapAlpha();
  bool StartDIBBase();
  bool StartRenderDIBBase();
  bool StartLoadDIBBase();
  bool ContinueDefault(PauseIndicatorIface* pPause);
  bool ContinueBlend(PauseIndicatorIface* pPause);
  bool ContinueTransform(PauseIndicatorIface* pPause);
  bool DrawMaskedImage();
  bool DrawPatternImage();
  bool NotDrawing() const;
  FX_RECT GetDrawRect() const;
  CFX_Matrix GetDrawMatrix(const FX_RECT& rect) const;
  void CalculateDrawImage(CFX_DefaultRenderDevice* pBitmapDevice1,
                          CFX_DefaultRenderDevice* pBitmapDevice2,
                          const RetainPtr<CFX_DIBBase>& pDIBBase,
                          const CFX_Matrix& mtNewMatrix,
                          const FX_RECT& rect) const;
  const CPDF_RenderOptions& GetRenderOptions() const;
  void HandleFilters();
  Optional<FX_RECT> GetUnitRect() const;
  bool GetDimensionsFromUnitRect(const FX_RECT& rect,
                                 int* left,
                                 int* top,
                                 int* width,
                                 int* height) const;

  UnownedPtr<CPDF_RenderStatus> m_pRenderStatus;
  UnownedPtr<CPDF_ImageObject> m_pImageObject;
  RetainPtr<CPDF_Pattern> m_pPattern;
  RetainPtr<CFX_DIBBase> m_pDIBBase;
  CFX_Matrix m_mtObj2Device;
  CFX_Matrix m_ImageMatrix;
  CPDF_ImageLoader m_Loader;
  std::unique_ptr<CFX_ImageTransformer> m_pTransformer;
  std::unique_ptr<CFX_ImageRenderer> m_DeviceHandle;
  Mode m_Mode = Mode::kNone;
  int m_BitmapAlpha = 0;
  BlendMode m_BlendType = BlendMode::kNormal;
  FX_ARGB m_FillArgb = 0;
  FXDIB_ResampleOptions m_ResampleOptions;
  bool m_bPatternColor = false;
  bool m_bStdCS = false;
  bool m_Result = true;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_IMAGERENDERER_H_
