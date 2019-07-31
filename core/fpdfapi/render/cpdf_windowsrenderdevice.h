// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_WINDOWSRENDERDEVICE_H_
#define CORE_FPDFAPI_RENDER_CPDF_WINDOWSRENDERDEVICE_H_

#include "core/fxge/cfx_windowsrenderdevice.h"

class CPDF_WindowsRenderDevice final : public CFX_WindowsRenderDevice {
 public:
  explicit CPDF_WindowsRenderDevice(HDC hDC);
  ~CPDF_WindowsRenderDevice() override;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_WINDOWSRENDERDEVICE_H_
