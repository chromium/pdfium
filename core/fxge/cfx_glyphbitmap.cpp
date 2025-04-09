// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_glyphbitmap.h"

#include "core/fxge/dib/cfx_dibitmap.h"

CFX_GlyphBitmap::CFX_GlyphBitmap(int left, int top)
    : left_(left), top_(top), bitmap_(pdfium::MakeRetain<CFX_DIBitmap>()) {}

CFX_GlyphBitmap::~CFX_GlyphBitmap() = default;
