// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_fillrenderoptions.h"

#include "core/fxcrt/fx_system.h"
#include "core/fxge/render_defines.h"
#include "third_party/base/no_destructor.h"

// static
const CFX_FillRenderOptions& CFX_FillRenderOptions::EvenOddOptions() {
  static const pdfium::base::NoDestructor<CFX_FillRenderOptions>
      alternate_options(CFX_FillRenderOptions::FillType::kEvenOdd);
  return *alternate_options;
}

// static
const CFX_FillRenderOptions& CFX_FillRenderOptions::WindingOptions() {
  static const pdfium::base::NoDestructor<CFX_FillRenderOptions>
      winding_options(CFX_FillRenderOptions::FillType::kWinding);
  return *winding_options;
}

CFX_FillRenderOptions::CFX_FillRenderOptions() = default;

CFX_FillRenderOptions::CFX_FillRenderOptions(
    CFX_FillRenderOptions::FillType fill_type)
    : fill_type(fill_type) {}

CFX_FillRenderOptions::FillType GetFillType(int fill_type) {
  ASSERT(fill_type >= 0);
  ASSERT(fill_type <= 2);
  return static_cast<CFX_FillRenderOptions::FillType>(fill_type);
}

CFX_FillRenderOptions GetFillOptionsFromIntegerFlags(int flags) {
  CFX_FillRenderOptions options(GetFillType(flags & 3));
  if (flags & FX_STROKE_ADJUST)
    options.adjust_stroke = true;
  if (flags & FXFILL_NOPATHSMOOTH)
    options.aliased_path = true;
  if (flags & FXFILL_FULLCOVER)
    options.full_cover = true;
  if (flags & FXFILL_RECT_AA)
    options.rect_aa = true;
  if (flags & FX_FILL_STROKE)
    options.stroke = true;
  if (flags & FX_STROKE_TEXT_MODE)
    options.stroke_text_mode = true;
  if (flags & FX_FILL_TEXT_MODE)
    options.text_mode = true;
  if (flags & FX_ZEROAREA_FILL)
    options.zero_area = true;
  return options;
}

int GetIntegerFlagsFromFillOptions(const CFX_FillRenderOptions& options) {
  int flags = static_cast<int>(options.fill_type);
  if (options.adjust_stroke)
    flags |= FX_STROKE_ADJUST;
  if (options.aliased_path)
    flags |= FXFILL_NOPATHSMOOTH;
  if (options.full_cover)
    flags |= FXFILL_FULLCOVER;
  if (options.rect_aa)
    flags |= FXFILL_RECT_AA;
  if (options.stroke)
    flags |= FX_FILL_STROKE;
  if (options.stroke_text_mode)
    flags |= FX_STROKE_TEXT_MODE;
  if (options.text_mode)
    flags |= FX_FILL_TEXT_MODE;
  if (options.zero_area)
    flags |= FX_ZEROAREA_FILL;
  return flags;
}
