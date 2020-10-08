// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_WIN32_INT_H_
#define CORE_FXGE_WIN32_WIN32_INT_H_

#include <windows.h>

#include "core/fxcrt/retain_ptr.h"

class CFX_DIBitmap;

RetainPtr<CFX_DIBitmap> FX_WindowsDIB_LoadFromBuf(BITMAPINFO* pbmi,
                                                  LPVOID pData,
                                                  bool bAlpha);

#endif  // CORE_FXGE_WIN32_WIN32_INT_H_
