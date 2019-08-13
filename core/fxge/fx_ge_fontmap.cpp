// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "build/build_config.h"
#include "core/fxge/systemfontinfo_iface.h"

int SystemFontInfoIface::GetFaceIndex(void* hFont) {
  return 0;
}

#if defined(OS_ANDROID)
std::unique_ptr<SystemFontInfoIface> SystemFontInfoIface::CreateDefault(
    const char** pUnused) {
  return nullptr;
}
#endif
