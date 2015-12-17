// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_AGG_INCLUDE_FX_AGG_DRIVER_H_
#define CORE_SRC_FXGE_AGG_INCLUDE_FX_AGG_DRIVER_H_

#include "core/include/fxge/fx_ge.h"
#include "third_party/agg23/agg_clip_liang_barsky.h"
#include "third_party/agg23/agg_path_storage.h"
#include "third_party/agg23/agg_rasterizer_scanline_aa.h"

class CFX_Matrix;
class CFX_PathData;

class CAgg_PathData {
 public:
  CAgg_PathData() {}
  ~CAgg_PathData() {}
  void BuildPath(const CFX_PathData* pPathData,
                 const CFX_Matrix* pObject2Device);

  agg::path_storage m_PathData;
};
class CFX_AggDeviceDriver : public IFX_RenderDeviceDriver {
 public:
  CFX_AggDeviceDriver(CFX_DIBitmap* pBitmap,
                      int dither_bits,
                      FX_BOOL bRgbByteOrder,
                      CFX_DIBitmap* pOriDevice,
                      FX_BOOL bGroupKnockout);
  ~CFX_AggDeviceDriver() override;

  void InitPlatform();
  void DestroyPlatform();

  // IFX_RenderDeviceDriver
  int GetDeviceCaps(int caps_id) override;
  void SaveState() override;
  void RestoreState(FX_BOOL bKeepSaved) override;
  FX_BOOL SetClip_PathFill(const CFX_PathData* pPathData,
                           const CFX_Matrix* pObject2Device,
                           int fill_mode) override;
  FX_BOOL SetClip_PathStroke(const CFX_PathData* pPathData,
                             const CFX_Matrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState) override;
  FX_BOOL DrawPath(const CFX_PathData* pPathData,
                   const CFX_Matrix* pObject2Device,
                   const CFX_GraphStateData* pGraphState,
                   FX_DWORD fill_color,
                   FX_DWORD stroke_color,
                   int fill_mode,
                   int alpha_flag,
                   void* pIccTransform,
                   int blend_type) override;
  FX_BOOL SetPixel(int x,
                   int y,
                   FX_DWORD color,
                   int alpha_flag,
                   void* pIccTransform) override;
  FX_BOOL FillRect(const FX_RECT* pRect,
                   FX_DWORD fill_color,
                   int alpha_flag,
                   void* pIccTransform,
                   int blend_type) override;
  FX_BOOL DrawCosmeticLine(FX_FLOAT x1,
                           FX_FLOAT y1,
                           FX_FLOAT x2,
                           FX_FLOAT y2,
                           FX_DWORD color,
                           int alpha_flag,
                           void* pIccTransform,
                           int blend_type) override {
    return FALSE;
  }
  FX_BOOL GetClipBox(FX_RECT* pRect) override;
  FX_BOOL GetDIBits(CFX_DIBitmap* pBitmap,
                    int left,
                    int top,
                    void* pIccTransform = NULL,
                    FX_BOOL bDEdge = FALSE) override;
  CFX_DIBitmap* GetBackDrop() override { return m_pOriDevice; }
  FX_BOOL SetDIBits(const CFX_DIBSource* pBitmap,
                    FX_DWORD color,
                    const FX_RECT* pSrcRect,
                    int left,
                    int top,
                    int blend_type,
                    int alpha_flag,
                    void* pIccTransform) override;
  FX_BOOL StretchDIBits(const CFX_DIBSource* pBitmap,
                        FX_DWORD color,
                        int dest_left,
                        int dest_top,
                        int dest_width,
                        int dest_height,
                        const FX_RECT* pClipRect,
                        FX_DWORD flags,
                        int alpha_flag,
                        void* pIccTransform,
                        int blend_type) override;
  FX_BOOL StartDIBits(const CFX_DIBSource* pBitmap,
                      int bitmap_alpha,
                      FX_DWORD color,
                      const CFX_Matrix* pMatrix,
                      FX_DWORD flags,
                      void*& handle,
                      int alpha_flag,
                      void* pIccTransform,
                      int blend_type) override;
  FX_BOOL ContinueDIBits(void* handle, IFX_Pause* pPause) override;
  void CancelDIBits(void* handle) override;
  FX_BOOL DrawDeviceText(int nChars,
                         const FXTEXT_CHARPOS* pCharPos,
                         CFX_Font* pFont,
                         CFX_FontCache* pCache,
                         const CFX_Matrix* pObject2Device,
                         FX_FLOAT font_size,
                         FX_DWORD color,
                         int alpha_flag,
                         void* pIccTransform) override;
  int GetDriverType() override { return 1; }

  FX_BOOL RenderRasterizer(agg::rasterizer_scanline_aa& rasterizer,
                           FX_DWORD color,
                           FX_BOOL bFullCover,
                           FX_BOOL bGroupKnockout,
                           int alpha_flag,
                           void* pIccTransform);

  void SetClipMask(agg::rasterizer_scanline_aa& rasterizer);

  virtual uint8_t* GetBuffer() const { return m_pBitmap->GetBuffer(); }

  CFX_DIBitmap* m_pBitmap;
  CFX_ClipRgn* m_pClipRgn;
  CFX_ArrayTemplate<CFX_ClipRgn*> m_StateStack;
  void* m_pPlatformGraphics;
  void* m_pPlatformBitmap;
  void* m_pDwRenderTartget;
  int m_FillFlags;
  int m_DitherBits;
  FX_BOOL m_bRgbByteOrder;
  CFX_DIBitmap* m_pOriDevice;
  FX_BOOL m_bGroupKnockout;
};

#endif  // CORE_SRC_FXGE_AGG_INCLUDE_FX_AGG_DRIVER_H_
