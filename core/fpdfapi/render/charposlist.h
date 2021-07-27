// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CHARPOSLIST_H_
#define CORE_FPDFAPI_RENDER_CHARPOSLIST_H_

#include <stdint.h>

#include <vector>

#include "third_party/base/span.h"

class CPDF_Font;
class TextCharPos;

std::vector<TextCharPos> GetCharPosList(pdfium::span<const uint32_t> char_codes,
                                        pdfium::span<const float> char_pos,
                                        CPDF_Font* font,
                                        float font_size);

#endif  // CORE_FPDFAPI_RENDER_CHARPOSLIST_H_
