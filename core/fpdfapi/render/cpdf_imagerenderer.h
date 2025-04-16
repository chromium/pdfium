// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_IMAGERENDERER_H_
#define CORE_FPDFAPI_RENDER_CPDF_IMAGERENDERER_H_

#include <memory>
#include <optional>

#include "build/build_config.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CFX_AggImageRenderer;
class CFX_DIBBase;
class CFX_DIBitmap;
class CFX_DefaultRenderDevice;
class CPDF_ImageLoader;
class CPDF_ImageObject;
class CPDF_Pattern;
class CPDF_RenderOptions;
class CPDF_RenderStatus;
class PauseIndicatorIface;

#if BUILDFLAG(IS_WIN)
class CFX_ImageTransformer;
#endif

class CPDF_ImageRenderer {
 public:
  explicit CPDF_ImageRenderer(CPDF_RenderStatus* pStatus);
  ~CPDF_ImageRenderer();

  bool Start(CPDF_ImageObject* pImageObject,
             const CFX_Matrix& mtObj2Device,
             bool bStdCS);

  bool Start(RetainPtr<CFX_DIBBase> pDIBBase,
             FX_ARGB bitmap_argb,
             const CFX_Matrix& mtImage2Device,
             const FXDIB_ResampleOptions& options,
             bool bStdCS);

  bool Continue(PauseIndicatorIface* pPause);
  bool GetResult() const { return result_; }

 private:
  enum class Mode {
    kNone = 0,
    kDefault,
    kBlend,  // AGG-specific
#if BUILDFLAG(IS_WIN)
    kTransform,
#endif
  };

  bool StartBitmapAlpha();
  bool StartDIBBase();
  bool StartRenderDIBBase();
  bool StartLoadDIBBase();
  bool ContinueDefault(PauseIndicatorIface* pPause);
  bool ContinueBlend(PauseIndicatorIface* pPause);
  bool DrawMaskedImage();
  bool DrawPatternImage();
#if BUILDFLAG(IS_WIN)
  bool StartDIBBaseFallback();
  bool ContinueTransform(PauseIndicatorIface* pPause);
  bool IsPrinting() const;
  void HandleFilters();
#endif
  FX_RECT GetDrawRect() const;
  CFX_Matrix GetDrawMatrix(const FX_RECT& rect) const;
  // Returns the mask, or nullptr if the mask could not be created.
  RetainPtr<const CFX_DIBitmap> CalculateDrawImage(
      CFX_DefaultRenderDevice& bitmap_device,
      RetainPtr<CFX_DIBBase> pDIBBase,
      const CFX_Matrix& mtNewMatrix,
      const FX_RECT& rect) const;
  const CPDF_RenderOptions& GetRenderOptions() const;
  std::optional<FX_RECT> GetUnitRect() const;
  bool GetDimensionsFromUnitRect(const FX_RECT& rect,
                                 int* left,
                                 int* top,
                                 int* width,
                                 int* height) const;

  UnownedPtr<CPDF_RenderStatus> const render_status_;
  UnownedPtr<CPDF_ImageObject> image_object_;
  RetainPtr<CPDF_Pattern> pattern_;
  RetainPtr<CFX_DIBBase> dibbase_;
  CFX_Matrix obj_to_device_;
  CFX_Matrix image_matrix_;
  std::unique_ptr<CPDF_ImageLoader> const loader_;
#if BUILDFLAG(IS_WIN)
  std::unique_ptr<CFX_ImageTransformer> transformer_;
#endif
  std::unique_ptr<CFX_AggImageRenderer> device_handle_;
  Mode mode_ = Mode::kNone;
  float alpha_ = 0.0f;
  BlendMode blend_type_ = BlendMode::kNormal;
  FX_ARGB fill_argb_ = 0;
  FXDIB_ResampleOptions resample_options_;
  bool pattern_color_ = false;
  bool std_cs_ = false;
  bool result_ = true;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_IMAGERENDERER_H_
