// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_CHARPOSLIST_H_
#define CORE_FPDFAPI_RENDER_CPDF_CHARPOSLIST_H_

#include <vector>

#include "core/fxcrt/fx_system.h"

class CPDF_Font;
class TextCharPos;

std::vector<TextCharPos> GetCharPosList(const std::vector<uint32_t>& charCodes,
                                        const std::vector<float>& charPos,
                                        CPDF_Font* pFont,
                                        float font_size);

#endif  // CORE_FPDFAPI_RENDER_CPDF_CHARPOSLIST_H_
