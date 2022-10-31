// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_CGDI_DISPLAY_DRIVER_H_
#define CORE_FXGE_WIN32_CGDI_DISPLAY_DRIVER_H_

#include <stdint.h>
#include <windows.h>

#include <memory>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/win32/cgdi_device_driver.h"

class CFX_DIBBase;
struct FXDIB_ResampleOptions;
struct FX_RECT;

class CGdiDisplayDriver final : public CGdiDeviceDriver {
 public:
  explicit CGdiDisplayDriver(HDC hDC);
  ~CGdiDisplayDriver() override;

 private:
  // CGdiDisplayDriver:
  int GetDeviceCaps(int caps_id) const override;
  bool GetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                 int left,
                 int top) override;
  bool SetDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                 uint32_t color,
                 const FX_RECT& src_rect,
                 int left,
                 int top,
                 BlendMode blend_type) override;
  bool StretchDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                     uint32_t color,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     const FXDIB_ResampleOptions& options,
                     BlendMode blend_type) override;
  bool StartDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                   int bitmap_alpha,
                   uint32_t color,
                   const CFX_Matrix& matrix,
                   const FXDIB_ResampleOptions& options,
                   std::unique_ptr<CFX_ImageRenderer>* handle,
                   BlendMode blend_type) override;

  bool UseFoxitStretchEngine(const RetainPtr<CFX_DIBBase>& pSource,
                             uint32_t color,
                             int dest_left,
                             int dest_top,
                             int dest_width,
                             int dest_height,
                             const FX_RECT* pClipRect,
                             const FXDIB_ResampleOptions& options);
};

#endif  // CORE_FXGE_WIN32_CGDI_DISPLAY_DRIVER_H_
