// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_WIN32_INT_H_
#define CORE_FXGE_WIN32_WIN32_INT_H_

#include <windows.h>

#include <memory>
#include <vector>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_windowsrenderdevice.h"
#include "core/fxge/renderdevicedriver_iface.h"
#include "third_party/base/optional.h"

class CFX_ImageRenderer;
class TextCharPos;
struct CFX_FillRenderOptions;
struct WINDIB_Open_Args_;

RetainPtr<CFX_DIBitmap> FX_WindowsDIB_LoadFromBuf(BITMAPINFO* pbmi,
                                                  LPVOID pData,
                                                  bool bAlpha);
class CGdiplusExt {
 public:
  CGdiplusExt();
  ~CGdiplusExt();

  void Load();
  bool IsAvailable() { return !!m_hModule; }
  bool StretchDIBits(HDC hDC,
                     const RetainPtr<CFX_DIBitmap>& pBitmap,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     const FXDIB_ResampleOptions& options);
  bool DrawPath(HDC hDC,
                const CFX_PathData* pPathData,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_argb,
                uint32_t stroke_argb,
                const CFX_FillRenderOptions& fill_options);

  RetainPtr<CFX_DIBitmap> LoadDIBitmap(WINDIB_Open_Args_ args);

  std::vector<FARPROC> m_Functions;

 protected:
  HMODULE m_hModule = nullptr;
  HMODULE m_GdiModule = nullptr;
};

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

class CGdiDeviceDriver : public RenderDeviceDriverIface {
 protected:
  CGdiDeviceDriver(HDC hDC, DeviceType device_type);
  ~CGdiDeviceDriver() override;

  // RenderDeviceDriverIface:
  DeviceType GetDeviceType() const override;
  int GetDeviceCaps(int caps_id) const override;
  void SaveState() override;
  void RestoreState(bool bKeepSaved) override;
  void SetBaseClip(const FX_RECT& rect) override;
  bool SetClip_PathFill(const CFX_PathData* pPathData,
                        const CFX_Matrix* pObject2Device,
                        const CFX_FillRenderOptions& fill_options) override;
  bool SetClip_PathStroke(const CFX_PathData* pPathData,
                          const CFX_Matrix* pObject2Device,
                          const CFX_GraphStateData* pGraphState) override;
  bool DrawPath(const CFX_PathData* pPathData,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_color,
                uint32_t stroke_color,
                const CFX_FillRenderOptions& fill_options,
                BlendMode blend_type) override;
  bool FillRectWithBlend(const FX_RECT& rect,
                         uint32_t fill_color,
                         BlendMode blend_type) override;
  bool DrawCosmeticLine(const CFX_PointF& ptMoveTo,
                        const CFX_PointF& ptLineTo,
                        uint32_t color,
                        BlendMode blend_type) override;
  bool GetClipBox(FX_RECT* pRect) override;

  void DrawLine(float x1, float y1, float x2, float y2);

  bool GDI_SetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                     const FX_RECT& src_rect,
                     int left,
                     int top);
  bool GDI_StretchDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                         int dest_left,
                         int dest_top,
                         int dest_width,
                         int dest_height,
                         const FXDIB_ResampleOptions& options);
  bool GDI_StretchBitMask(const RetainPtr<CFX_DIBitmap>& pBitmap,
                          int dest_left,
                          int dest_top,
                          int dest_width,
                          int dest_height,
                          uint32_t bitmap_color);

  const HDC m_hDC;
  bool m_bMetafileDCType;
  int m_Width;
  int m_Height;
  int m_nBitsPerPixel;
  const DeviceType m_DeviceType;
  int m_RenderCaps;
  pdfium::Optional<FX_RECT> m_BaseClipBox;
};

#endif  // CORE_FXGE_WIN32_WIN32_INT_H_
