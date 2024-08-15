// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_UTILS_H_
#define XFA_FWL_THEME_CFWL_UTILS_H_

#include <stdint.h>

#include "core/fxge/dib/fx_dib.h"

namespace pdfium {

// Values matter, used for indexing.
enum class FWLTHEME_STATE : uint8_t { kNormal = 1, kHover, kPressed, kDisable };

enum class FWLTHEME_DIRECTION : uint8_t { kUp = 0, kDown, kLeft, kRight };

#define FWLTHEME_COLOR_EDGERB1 (ArgbEncode(255, 241, 239, 226))
#define FWLTHEME_COLOR_Background (ArgbEncode(255, 236, 233, 216))
#define FWLTHEME_COLOR_BKSelected (ArgbEncode(255, 153, 193, 218))

#define FWLTHEME_CAPACITY_FontSize 12.0f
#define FWLTHEME_CAPACITY_TextColor (ArgbEncode(255, 0, 0, 0))
#define FWLTHEME_CAPACITY_TextDisColor (ArgbEncode(255, 172, 168, 153))

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::FWLTHEME_STATE;

#endif  // XFA_FWL_THEME_CFWL_UTILS_H_
