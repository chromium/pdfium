// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_WIN32_INT_H_
#define CORE_FXGE_WIN32_WIN32_INT_H_

#include <windows.h>

#include <memory>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/win32/cgdi_plus_ext.h"

RetainPtr<CFX_DIBitmap> FX_WindowsDIB_LoadFromBuf(BITMAPINFO* pbmi,
                                                  LPVOID pData,
                                                  bool bAlpha);

class CWin32Platform : public CFX_GEModule::PlatformIface {
 public:
  CWin32Platform();
  ~CWin32Platform() override;

  // CFX_GEModule::PlatformIface:
  void Init() override;
  std::unique_ptr<SystemFontInfoIface> CreateDefaultSystemFontInfo() override;

  bool m_bHalfTone = false;
  CGdiplusExt m_GdiplusExt;
};

#endif  // CORE_FXGE_WIN32_WIN32_INT_H_
