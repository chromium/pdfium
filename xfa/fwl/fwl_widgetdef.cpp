// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fwl/fwl_widgetdef.h"

#include "build/build_config.h"
#include "core/fxcrt/mask.h"

namespace pdfium {

bool IsPlatformShortcutKey(Mask<XFA_FWL_KeyFlag> flags) {
#if BUILDFLAG(IS_APPLE)
  constexpr XFA_FWL_KeyFlag kEditingModifier = XFA_FWL_KeyFlag::kCommand;
#else
  constexpr XFA_FWL_KeyFlag kEditingModifier = XFA_FWL_KeyFlag::kCtrl;
#endif
  return !!(flags & kEditingModifier);
}

}  // namespace pdfium
