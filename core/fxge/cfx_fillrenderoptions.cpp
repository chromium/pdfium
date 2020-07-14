// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_fillrenderoptions.h"

#include "core/fxcrt/fx_system.h"
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
