// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_SKIA_FX_SKIA_DEVICE_H_
#define CORE_FXGE_SKIA_FX_SKIA_DEVICE_H_

#if defined(_SKIA_SUPPORT_)

class SkCanvas;
class SkPaint;
class SkPath;
class SkPictureRecorder;
struct SkIRect;

class CFX_SkiaDeviceDriver : public IFX_RenderDeviceDriver {
 public:
  CFX_SkiaDeviceDriver(CFX_DIBitmap* pBitmap,
                       int dither_bits,
                       FX_BOOL bRgbByteOrder,
                       CFX_DIBitmap* pOriDevice,
                       FX_BOOL bGroupKnockout);
  CFX_SkiaDeviceDriver(SkPictureRecorder* recorder);
  CFX_SkiaDeviceDriver(int size_x, int size_y);
  ~CFX_SkiaDeviceDriver() override;

  /** Options */
  int GetDeviceCaps(int caps_id) override;

  /** Save and restore all graphic states */
  void SaveState() override;
  void RestoreState(FX_BOOL bKeepSaved) override;

  /** Set clipping path using filled region */
  FX_BOOL SetClip_PathFill(
      const CFX_PathData* pPathData,     // path info
      const CFX_Matrix* pObject2Device,  // optional transformation
      int fill_mode) override;           // fill mode, WINDING or ALTERNATE

  /** Set clipping path using stroked region */
  FX_BOOL SetClip_PathStroke(
      const CFX_PathData* pPathData,     // path info
      const CFX_Matrix* pObject2Device,  // optional transformation
      const CFX_GraphStateData*
          pGraphState)  // graphic state, for pen attributes
      override;

  /** Draw a path */
  FX_BOOL DrawPath(const CFX_PathData* pPathData,
                   const CFX_Matrix* pObject2Device,
                   const CFX_GraphStateData* pGraphState,
                   FX_DWORD fill_color,
                   FX_DWORD stroke_color,
                   int fill_mode,
                   int alpha_flag = 0,
                   void* pIccTransform = NULL,
                   int blend_type = FXDIB_BLEND_NORMAL) override;

  FX_BOOL SetPixel(int x,
                   int y,
                   FX_DWORD color,
                   int alpha_flag = 0,
                   void* pIccTransform = NULL) override;

  FX_BOOL FillRect(const FX_RECT* pRect,
                   FX_DWORD fill_color,
                   int alpha_flag = 0,
                   void* pIccTransform = NULL,
                   int blend_type = FXDIB_BLEND_NORMAL) override;

  /** Draw a single pixel (device dependant) line */
  FX_BOOL DrawCosmeticLine(FX_FLOAT x1,
                           FX_FLOAT y1,
                           FX_FLOAT x2,
                           FX_FLOAT y2,
                           FX_DWORD color,
                           int alpha_flag = 0,
                           void* pIccTransform = NULL,
                           int blend_type = FXDIB_BLEND_NORMAL) override {
    return FALSE;
  }

  FX_BOOL GetClipBox(FX_RECT* pRect) override;

  /** Load device buffer into a DIB */
  FX_BOOL GetDIBits(CFX_DIBitmap* pBitmap,
                    int left,
                    int top,
                    void* pIccTransform = NULL,
                    FX_BOOL bDEdge = FALSE) override;

  CFX_DIBitmap* GetBackDrop() override { return m_pAggDriver->GetBackDrop(); }

  FX_BOOL SetDIBits(const CFX_DIBSource* pBitmap,
                    FX_DWORD color,
                    const FX_RECT* pSrcRect,
                    int dest_left,
                    int dest_top,
                    int blend_type,
                    int alpha_flag = 0,
                    void* pIccTransform = NULL) override;
  FX_BOOL StretchDIBits(const CFX_DIBSource* pBitmap,
                        FX_DWORD color,
                        int dest_left,
                        int dest_top,
                        int dest_width,
                        int dest_height,
                        const FX_RECT* pClipRect,
                        FX_DWORD flags,
                        int alpha_flag = 0,
                        void* pIccTransform = NULL,
                        int blend_type = FXDIB_BLEND_NORMAL) override;

  FX_BOOL StartDIBits(const CFX_DIBSource* pBitmap,
                      int bitmap_alpha,
                      FX_DWORD color,
                      const CFX_Matrix* pMatrix,
                      FX_DWORD flags,
                      void*& handle,
                      int alpha_flag = 0,
                      void* pIccTransform = NULL,
                      int blend_type = FXDIB_BLEND_NORMAL) override;
  FX_BOOL ContinueDIBits(void* handle, IFX_Pause* pPause) override;
  void CancelDIBits(void* handle) override;

  FX_BOOL DrawDeviceText(int nChars,
                         const FXTEXT_CHARPOS* pCharPos,
                         CFX_Font* pFont,
                         CFX_FontCache* pCache,
                         const CFX_Matrix* pObject2Device,
                         FX_FLOAT font_size,
                         FX_DWORD color,
                         int alpha_flag = 0,
                         void* pIccTransform = NULL) override;

  FX_BOOL RenderRasterizer(agg::rasterizer_scanline_aa& rasterizer,
                           FX_DWORD color,
                           FX_BOOL bFullCover,
                           FX_BOOL bGroupKnockout,
                           int alpha_flag,
                           void* pIccTransform);
  void SetClipMask(agg::rasterizer_scanline_aa& rasterizer);
  void SetClipMask(SkPath& skPath, SkPaint* spaint);
  virtual uint8_t* GetBuffer() const { return m_pAggDriver->GetBuffer(); }
  void PaintStroke(SkPaint* spaint, const CFX_GraphStateData* pGraphState);
  SkPictureRecorder* GetRecorder() const { return m_pRecorder; }

 private:
  CFX_AggDeviceDriver* m_pAggDriver;
  SkCanvas* m_pCanvas;
  SkPictureRecorder* const m_pRecorder;
  int m_ditherBits;
};
#endif  // defined(_SKIA_SUPPORT_)

#endif  // CORE_FXGE_SKIA_FX_SKIA_DEVICE_H_
