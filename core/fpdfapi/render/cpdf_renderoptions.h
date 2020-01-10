// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_
#define CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_

#include "core/fpdfapi/page/cpdf_occontext.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/fx_dib.h"

class CPDF_RenderOptions {
 public:
  enum Type : uint8_t { kNormal = 0, kGray, kAlpha };

  struct Options {
    Options();
    Options(const Options& rhs);

    bool bClearType = false;
    bool bPrintGraphicText = false;
    bool bPrintPreview = false;
    bool bBGRStripe = false;
    bool bNoNativeText = false;
    bool bForceHalftone = false;
    bool bRectAA = false;
    bool bFillFullcover = false;
    bool bPrintImageText = false;
    bool bOverprint = false;
    bool bThinLine = false;
    bool bBreakForMasks = false;
    bool bNoTextSmooth = false;
    bool bNoPathSmooth = false;
    bool bNoImageSmooth = false;
    bool bLimitedImageCache = false;
  };

  CPDF_RenderOptions();
  CPDF_RenderOptions(const CPDF_RenderOptions& rhs);
  ~CPDF_RenderOptions();

  FX_ARGB TranslateColor(FX_ARGB argb) const;

  void SetColorMode(Type mode) { m_ColorMode = mode; }
  bool ColorModeIs(Type mode) const { return m_ColorMode == mode; }

  const Options& GetOptions() const { return m_Options; }
  Options& GetOptions() { return m_Options; }

  uint32_t GetCacheSizeLimit() const;

  void SetDrawAnnots(bool draw) { m_bDrawAnnots = draw; }
  bool GetDrawAnnots() const { return m_bDrawAnnots; }

  void SetOCContext(RetainPtr<CPDF_OCContext> context) {
    m_pOCContext = context;
  }
  const CPDF_OCContext* GetOCContext() const { return m_pOCContext.Get(); }

 private:
  Type m_ColorMode = kNormal;
  bool m_bDrawAnnots = false;
  Options m_Options;
  RetainPtr<CPDF_OCContext> m_pOCContext;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_
