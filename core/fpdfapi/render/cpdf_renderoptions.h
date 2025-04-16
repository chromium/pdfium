// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_
#define CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_

#include <stdint.h>

#include "core/fpdfapi/page/cpdf_occontext.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CPDF_Dictionary;

class CPDF_RenderOptions {
 public:
  enum Type : uint8_t { kNormal = 0, kGray, kAlpha, kForcedColor };

  struct Options {
    Options();
    Options(const Options& rhs);
    Options& operator=(const Options& rhs);

    bool bClearType = false;
    bool bNoNativeText = false;
    bool bForceHalftone = false;
    bool bRectAA = false;
    bool bBreakForMasks = false;
    bool bNoTextSmooth = false;
    bool bNoPathSmooth = false;
    bool bNoImageSmooth = false;
    bool bLimitedImageCache = false;
    bool bConvertFillToStroke = false;
  };

  struct ColorScheme {
    FX_ARGB path_fill_color;
    FX_ARGB path_stroke_color;
    FX_ARGB text_fill_color;
    FX_ARGB text_stroke_color;
  };

  CPDF_RenderOptions();
  CPDF_RenderOptions(const CPDF_RenderOptions& rhs);
  ~CPDF_RenderOptions();

  FX_ARGB TranslateColor(FX_ARGB argb) const;
  FX_ARGB TranslateObjectFillColor(FX_ARGB argb,
                                   CPDF_PageObject::Type object_type) const;
  FX_ARGB TranslateObjectStrokeColor(FX_ARGB argb,
                                     CPDF_PageObject::Type object_type) const;

  void SetColorScheme(const ColorScheme& color_scheme) {
    color_scheme_ = color_scheme;
  }

  void SetColorMode(Type mode) { color_mode_ = mode; }
  bool ColorModeIs(Type mode) const { return color_mode_ == mode; }

  const Options& GetOptions() const { return options_; }
  Options& GetOptions() { return options_; }

  uint32_t GetCacheSizeLimit() const;
  bool CheckOCGDictVisible(const CPDF_Dictionary* pOC) const;
  bool CheckPageObjectVisible(const CPDF_PageObject* pPageObj) const;

  void SetDrawAnnots(bool draw) { draw_annots_ = draw; }
  bool GetDrawAnnots() const { return draw_annots_; }

  void SetOCContext(RetainPtr<CPDF_OCContext> context) {
    oc_context_ = context;
  }

 private:
  Type color_mode_ = kNormal;
  bool draw_annots_ = false;
  Options options_;
  ColorScheme color_scheme_ = {};
  RetainPtr<CPDF_OCContext> oc_context_;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_RENDEROPTIONS_H_
