// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_SRC_FXGE_SKIA_FX_SKIA_BLITTER_NEW_H_
#define CORE_SRC_FXGE_SKIA_FX_SKIA_BLITTER_NEW_H_

#if defined(_SKIA_SUPPORT_)
class CFX_SkiaRenderer : public SkBlitter {
 protected:
  int m_Alpha,
      m_Red,         // Or the complementary-color, Cyan
      m_Green,       // Magenta
      m_Blue,        // Yellow
      m_Gray;        // Black
  FX_DWORD m_Color;  // FX_ARGB or FX_CMYK
  FX_BOOL m_bFullCover;
  int m_ProcessFilter;
  FX_BOOL m_bRgbByteOrder;

  FX_RECT m_ClipBox;
  CFX_DIBitmap* m_pDevice;
  CFX_DIBitmap* m_pOriDevice;
  const CFX_ClipRgn* m_pClipRgn;
  const CFX_DIBitmap* m_pClipMask;

  uint8_t* m_pDestScan;
  uint8_t* m_pDestExtraAlphaScan;
  uint8_t* m_pOriScan;
  uint8_t* m_pClipScan;

  void (CFX_SkiaRenderer::*composite_span)(uint8_t*,
                                           uint8_t*,
                                           int,
                                           int,
                                           int,
                                           int,
                                           uint8_t,
                                           int,
                                           int,
                                           int,
                                           uint8_t*,
                                           uint8_t*);

 public:
  //--------------------------------------------------------------------
  virtual void blitAntiH(int x,
                         int y,
                         const SkAlpha antialias[],
                         const int16_t runs[]);
  virtual void blitH(int x, int y, int width);
  virtual void blitV(int x, int y, int height, SkAlpha alpha);
  virtual void blitRect(int x, int y, int width, int height);
  virtual void blitAntiRect(int x,
                            int y,
                            int width,
                            int height,
                            SkAlpha leftAlpha,
                            SkAlpha rightAlpha);

  /*------------------------------------------------------------------------------------------------------*/
  // A general alpha merge function (with clipping mask). Gray device.
  void CompositeSpan1bpp_0(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);
  void CompositeSpan1bpp_1(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);
  void CompositeSpan1bpp_4(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);
  void CompositeSpan1bpp_5(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);
  void CompositeSpan1bpp_8(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);
  void CompositeSpan1bpp_9(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);
  void CompositeSpan1bpp_12(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpan1bpp_13(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);

  /*--------------------------------------------------------------------------------------------------------*/

  // A general alpha merge function (with clipping mask). Gray device.
  void CompositeSpanGray_2(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);

  void CompositeSpanGray_3(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);

  void CompositeSpanGray_6(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);

  void CompositeSpanGray_7(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);

  void CompositeSpanGray_10(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);

  void CompositeSpanGray_11(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);

  void CompositeSpanGray_14(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);

  void CompositeSpanGray_15(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);

  /*--------------------------------------------------------------------------------------------------------*/
  void CompositeSpanARGB_2(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);

  void CompositeSpanARGB_3(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);

  void CompositeSpanARGB_6(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);

  void CompositeSpanARGB_7(uint8_t* dest_scan,
                           uint8_t* ori_scan,
                           int Bpp,
                           int span_left,
                           int span_len,
                           int span_top,
                           uint8_t cover_scan,
                           int clip_top,
                           int clip_left,
                           int clip_right,
                           uint8_t* clip_scan,
                           uint8_t* dest_extra_alpha_scan);
  // ...
  /*--------------------------------------------------------------------------------------------------------*/
  void CompositeSpanRGB32_2(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB32_3(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB32_6(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB32_7(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);

  /*---------------------------------------------------------------------------------------------------------*/

  void CompositeSpanRGB24_2(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB24_3(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB24_6(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB24_7(uint8_t* dest_scan,
                            uint8_t* ori_scan,
                            int Bpp,
                            int span_left,
                            int span_len,
                            int span_top,
                            uint8_t cover_scan,
                            int clip_top,
                            int clip_left,
                            int clip_right,
                            uint8_t* clip_scan,
                            uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB24_10(uint8_t* dest_scan,
                             uint8_t* ori_scan,
                             int Bpp,
                             int span_left,
                             int span_len,
                             int span_top,
                             uint8_t cover_scan,
                             int clip_top,
                             int clip_left,
                             int clip_right,
                             uint8_t* clip_scan,
                             uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB24_11(uint8_t* dest_scan,
                             uint8_t* ori_scan,
                             int Bpp,
                             int span_left,
                             int span_len,
                             int span_top,
                             uint8_t cover_scan,
                             int clip_top,
                             int clip_left,
                             int clip_right,
                             uint8_t* clip_scan,
                             uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB24_14(uint8_t* dest_scan,
                             uint8_t* ori_scan,
                             int Bpp,
                             int span_left,
                             int span_len,
                             int span_top,
                             uint8_t cover_scan,
                             int clip_top,
                             int clip_left,
                             int clip_right,
                             uint8_t* clip_scan,
                             uint8_t* dest_extra_alpha_scan);
  void CompositeSpanRGB24_15(uint8_t* dest_scan,
                             uint8_t* ori_scan,
                             int Bpp,
                             int span_left,
                             int span_len,
                             int span_top,
                             uint8_t cover_scan,
                             int clip_top,
                             int clip_left,
                             int clip_right,
                             uint8_t* clip_scan,
                             uint8_t* dest_extra_alpha_scan);

  /*----------------------------------------------------------------------------------------------------------*/

  // A general alpha merge function (with clipping mask). Cmyka/Cmyk device.
  void CompositeSpanCMYK(uint8_t* dest_scan,
                         uint8_t* ori_scan,
                         int Bpp,
                         int span_left,
                         int span_len,
                         int span_top,
                         uint8_t cover_scan,
                         int clip_top,
                         int clip_left,
                         int clip_right,
                         uint8_t* clip_scan,
                         uint8_t* dest_extra_alpha_scan);

  //--------------------------------------------------------------------
  FX_BOOL Init(CFX_DIBitmap* pDevice,
               CFX_DIBitmap* pOriDevice,
               const CFX_ClipRgn* pClipRgn,
               FX_DWORD color,
               FX_BOOL bFullCover,
               FX_BOOL bRgbByteOrder,
               int alpha_flag = 0,
               void* pIccTransform =
                   NULL);  // The alpha flag must be fill_flag if exist.
};
class CFX_SkiaA8Renderer : public SkBlitter {
 public:
  //--------------------------------------------------------------------
  virtual void blitAntiH(int x,
                         int y,
                         const SkAlpha antialias[],
                         const int16_t runs[]);
  virtual void blitH(int x, int y, int width);
  virtual void blitV(int x, int y, int height, SkAlpha alpha);
  virtual void blitRect(int x, int y, int width, int height);
  virtual void blitAntiRect(int x,
                            int y,
                            int width,
                            int height,
                            SkAlpha leftAlpha,
                            SkAlpha rightAlpha);
  //--------------------------------------------------------------------
  FX_BOOL Init(CFX_DIBitmap* pDevice, int Left, int Top);
  CFX_DIBitmap* m_pDevice;
  int m_Left;
  int m_Top;
  int m_dstWidth;
  int m_dstHeight;
};
#endif

#endif  // CORE_SRC_FXGE_SKIA_FX_SKIA_BLITTER_NEW_H_
