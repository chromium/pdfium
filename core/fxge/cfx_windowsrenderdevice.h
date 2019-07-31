// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_WINDOWSRENDERDEVICE_H_
#define CORE_FXGE_CFX_WINDOWSRENDERDEVICE_H_

#include <windows.h>

#include "core/fxge/cfx_renderdevice.h"

enum WindowsPrintMode {
  kModeEmf = 0,
  kModeTextOnly = 1,
  kModePostScript2 = 2,
  kModePostScript3 = 3,
  kModePostScript2PassThrough = 4,
  kModePostScript3PassThrough = 5,
};

class RenderDeviceDriverIface;
struct EncoderIface;

#if defined(PDFIUM_PRINT_TEXT_WITH_GDI)
typedef void (*PDFiumEnsureTypefaceCharactersAccessible)(const LOGFONT* font,
                                                         const wchar_t* text,
                                                         size_t text_length);

extern bool g_pdfium_print_text_with_gdi;
extern PDFiumEnsureTypefaceCharactersAccessible
    g_pdfium_typeface_accessible_func;
#endif
extern WindowsPrintMode g_pdfium_print_mode;

class CFX_WindowsRenderDevice : public CFX_RenderDevice {
 public:
  CFX_WindowsRenderDevice(HDC hDC, const EncoderIface* pEncoderIface);
  ~CFX_WindowsRenderDevice() override;

 private:
  static RenderDeviceDriverIface* CreateDriver(
      HDC hDC,
      const EncoderIface* pEncoderIface);
};

#endif  // CORE_FXGE_CFX_WINDOWSRENDERDEVICE_H_
