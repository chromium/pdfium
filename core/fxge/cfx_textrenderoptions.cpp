// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_textrenderoptions.h"

#include "third_party/base/no_destructor.h"

// static
const CFX_TextRenderOptions& CFX_TextRenderOptions::LcdOptions() {
  static pdfium::base::NoDestructor<CFX_TextRenderOptions> instance(kLcd);
  return *instance;
}

CFX_TextRenderOptions::CFX_TextRenderOptions() = default;

CFX_TextRenderOptions::CFX_TextRenderOptions(AliasingType type)
    : aliasing_type(type) {}

CFX_TextRenderOptions::CFX_TextRenderOptions(
    const CFX_TextRenderOptions& other) = default;
