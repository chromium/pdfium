// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "build/build_config.h"
#include "core/fxge/systemfontinfo_iface.h"

#ifdef PDF_ENABLE_XFA
void* SystemFontInfoIface::MapFontByUnicode(uint32_t dwUnicode,
                                            int weight,
                                            bool bItalic,
                                            int pitch_family) {
  return nullptr;
}
#endif  // PDF_ENABLE_XFA

int SystemFontInfoIface::GetFaceIndex(void* hFont) {
  return 0;
}

#if defined(OS_ANDROID)
std::unique_ptr<SystemFontInfoIface> SystemFontInfoIface::CreateDefault(
    const char** pUnused) {
  return nullptr;
}
#endif
