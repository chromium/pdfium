// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_PROGRESSIVERENDERER_H_
#define CORE_FPDFAPI_RENDER_CPDF_PROGRESSIVERENDERER_H_

#include <stdint.h>

#include <memory>

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_RenderOptions;
class CPDF_RenderStatus;
class CFX_RenderDevice;
class PauseIndicatorIface;

class CPDF_ProgressiveRenderer {
 public:
  // Must match FDF_RENDER_* definitions in public/fpdf_progressive.h, but
  // cannot #include that header. fpdfsdk/fpdf_progressive.cpp has
  // static_asserts to make sure the two sets of values match.
  enum Status {
    kReady,          // FPDF_RENDER_READY
    kToBeContinued,  // FPDF_RENDER_TOBECONTINUED
    kDone,           // FPDF_RENDER_DONE
    kFailed          // FPDF_RENDER_FAILED
  };

  CPDF_ProgressiveRenderer(CPDF_RenderContext* context,
                           CFX_RenderDevice* pDevice,
                           const CPDF_RenderOptions* pOptions);
  ~CPDF_ProgressiveRenderer();

  Status GetStatus() const { return status_; }
  void Start(PauseIndicatorIface* pPause);
  void Continue(PauseIndicatorIface* pPause);

 private:
  // Maximum page objects to render before checking for pause.
  static constexpr int kStepLimit = 100;

  Status status_ = kReady;
  UnownedPtr<CPDF_RenderContext> const context_;
  UnownedPtr<CFX_RenderDevice> const device_;
  UnownedPtr<const CPDF_RenderOptions> const options_;
  std::unique_ptr<CPDF_RenderStatus> render_status_;
  CFX_FloatRect clip_rect_;
  uint32_t layer_index_ = 0;
  UnownedPtr<CPDF_RenderContext::Layer> current_layer_;
  CPDF_PageObjectHolder::const_iterator last_object_rendered_;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_PROGRESSIVERENDERER_H_
