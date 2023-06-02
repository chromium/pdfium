// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_CGDI_PLUS_EXT_H_
#define CORE_FXGE_WIN32_CGDI_PLUS_EXT_H_

#include <stdint.h>
#include <windows.h>

#include <vector>

#include "core/fxcrt/retain_ptr.h"

class CFX_DIBBase;
class CFX_GraphStateData;
class CFX_Matrix;
class CFX_Path;
struct CFX_FillRenderOptions;
struct FXDIB_ResampleOptions;
struct FX_RECT;

class CGdiplusExt {
 public:
  CGdiplusExt();
  ~CGdiplusExt();

  void Load();
  bool IsAvailable() { return !!m_hModule; }
  bool StretchDIBits(HDC hDC,
                     const RetainPtr<CFX_DIBBase>& source,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     const FXDIB_ResampleOptions& options);
  bool DrawPath(HDC hDC,
                const CFX_Path& path,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_argb,
                uint32_t stroke_argb,
                const CFX_FillRenderOptions& fill_options);

  std::vector<FARPROC> m_Functions;

 private:
  HMODULE m_hModule = nullptr;
  HMODULE m_GdiModule = nullptr;
};

#endif  // CORE_FXGE_WIN32_CGDI_PLUS_EXT_H_
