// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_FX_DIB_H_
#define CORE_FXGE_FX_DIB_H_

#include <memory>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_dibsource.h"
#include "core/fxge/dib/cfx_scanlinecompositor.h"
#include "third_party/base/stl_util.h"

typedef uint32_t FX_ARGB;
typedef uint32_t FX_COLORREF;
typedef uint32_t FX_CMYK;
class CFX_ClipRgn;
class CStretchEngine;

#define FXSYS_RGB(r, g, b) ((r) | ((g) << 8) | ((b) << 16))
#define FXSYS_GetRValue(rgb) ((rgb)&0xff)
#define FXSYS_GetGValue(rgb) (((rgb) >> 8) & 0xff)
#define FXSYS_GetBValue(rgb) (((rgb) >> 16) & 0xff)
#define FX_CCOLOR(val) (255 - (val))
#define FXSYS_CMYK(c, m, y, k) (((c) << 24) | ((m) << 16) | ((y) << 8) | (k))
#define FXSYS_GetCValue(cmyk) ((uint8_t)((cmyk) >> 24) & 0xff)
#define FXSYS_GetMValue(cmyk) ((uint8_t)((cmyk) >> 16) & 0xff)
#define FXSYS_GetYValue(cmyk) ((uint8_t)((cmyk) >> 8) & 0xff)
#define FXSYS_GetKValue(cmyk) ((uint8_t)(cmyk)&0xff)
void CmykDecode(FX_CMYK cmyk, int& c, int& m, int& y, int& k);
inline FX_CMYK CmykEncode(int c, int m, int y, int k) {
  return (c << 24) | (m << 16) | (y << 8) | k;
}
void ArgbDecode(FX_ARGB argb, int& a, int& r, int& g, int& b);
void ArgbDecode(FX_ARGB argb, int& a, FX_COLORREF& rgb);
inline FX_ARGB ArgbEncode(int a, int r, int g, int b) {
  return (a << 24) | (r << 16) | (g << 8) | b;
}
FX_ARGB ArgbEncode(int a, FX_COLORREF rgb);
#define FXARGB_A(argb) ((uint8_t)((argb) >> 24))
#define FXARGB_R(argb) ((uint8_t)((argb) >> 16))
#define FXARGB_G(argb) ((uint8_t)((argb) >> 8))
#define FXARGB_B(argb) ((uint8_t)(argb))
#define FXARGB_MAKE(a, r, g, b) \
  (((uint32_t)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define FXARGB_MUL_ALPHA(argb, alpha) \
  (((((argb) >> 24) * (alpha) / 255) << 24) | ((argb)&0xffffff))
#define FXRGB2GRAY(r, g, b) (((b)*11 + (g)*59 + (r)*30) / 100)
#define FXCMYK2GRAY(c, m, y, k)                                       \
  (((255 - (c)) * (255 - (k)) * 30 + (255 - (m)) * (255 - (k)) * 59 + \
    (255 - (y)) * (255 - (k)) * 11) /                                 \
   25500)
#define FXDIB_ALPHA_MERGE(backdrop, source, source_alpha) \
  (((backdrop) * (255 - (source_alpha)) + (source) * (source_alpha)) / 255)
#define FXDIB_ALPHA_UNION(dest, src) ((dest) + (src) - (dest) * (src) / 255)
#define FXCMYK_GETDIB(p)                                    \
  ((((uint8_t*)(p))[0] << 24 | (((uint8_t*)(p))[1] << 16) | \
    (((uint8_t*)(p))[2] << 8) | ((uint8_t*)(p))[3]))
#define FXCMYK_SETDIB(p, cmyk)  ((uint8_t*)(p))[0] = (uint8_t)((cmyk) >> 24), \
        ((uint8_t*)(p))[1] = (uint8_t)((cmyk) >> 16), \
                              ((uint8_t*)(p))[2] = (uint8_t)((cmyk) >> 8), \
                                      ((uint8_t*)(p))[3] = (uint8_t)(cmyk))
#define FXARGB_GETDIB(p)                              \
  ((((uint8_t*)(p))[0]) | (((uint8_t*)(p))[1] << 8) | \
   (((uint8_t*)(p))[2] << 16) | (((uint8_t*)(p))[3] << 24))
#define FXARGB_SETDIB(p, argb)                  \
  ((uint8_t*)(p))[0] = (uint8_t)(argb),         \
  ((uint8_t*)(p))[1] = (uint8_t)((argb) >> 8),  \
  ((uint8_t*)(p))[2] = (uint8_t)((argb) >> 16), \
  ((uint8_t*)(p))[3] = (uint8_t)((argb) >> 24)
#define FXARGB_COPY(dest, src)                      \
  *(uint8_t*)(dest) = *(uint8_t*)(src),             \
  *((uint8_t*)(dest) + 1) = *((uint8_t*)(src) + 1), \
  *((uint8_t*)(dest) + 2) = *((uint8_t*)(src) + 2), \
  *((uint8_t*)(dest) + 3) = *((uint8_t*)(src) + 3)
#define FXCMYK_COPY(dest, src)                      \
  *(uint8_t*)(dest) = *(uint8_t*)(src),             \
  *((uint8_t*)(dest) + 1) = *((uint8_t*)(src) + 1), \
  *((uint8_t*)(dest) + 2) = *((uint8_t*)(src) + 2), \
  *((uint8_t*)(dest) + 3) = *((uint8_t*)(src) + 3)
#define FXARGB_SETRGBORDERDIB(p, argb)          \
  ((uint8_t*)(p))[3] = (uint8_t)(argb >> 24),   \
  ((uint8_t*)(p))[0] = (uint8_t)((argb) >> 16), \
  ((uint8_t*)(p))[1] = (uint8_t)((argb) >> 8),  \
  ((uint8_t*)(p))[2] = (uint8_t)(argb)
#define FXARGB_GETRGBORDERDIB(p)                     \
  (((uint8_t*)(p))[2]) | (((uint8_t*)(p))[1] << 8) | \
      (((uint8_t*)(p))[0] << 16) | (((uint8_t*)(p))[3] << 24)
#define FXARGB_RGBORDERCOPY(dest, src)                                   \
  *((uint8_t*)(dest) + 3) = *((uint8_t*)(src) + 3),                      \
                       *(uint8_t*)(dest) = *((uint8_t*)(src) + 2),       \
                       *((uint8_t*)(dest) + 1) = *((uint8_t*)(src) + 1), \
                       *((uint8_t*)(dest) + 2) = *((uint8_t*)(src))
#define FXARGB_TODIB(argb) (argb)
#define FXCMYK_TODIB(cmyk)                                    \
  ((uint8_t)((cmyk) >> 24) | ((uint8_t)((cmyk) >> 16)) << 8 | \
   ((uint8_t)((cmyk) >> 8)) << 16 | ((uint8_t)(cmyk) << 24))
#define FXARGB_TOBGRORDERDIB(argb)                       \
  ((uint8_t)(argb >> 16) | ((uint8_t)(argb >> 8)) << 8 | \
   ((uint8_t)(argb)) << 16 | ((uint8_t)(argb >> 24) << 24))
#define FXGETFLAG_COLORTYPE(flag) (uint8_t)((flag) >> 8)
#define FXGETFLAG_ALPHA_FILL(flag) (uint8_t)(flag)

bool ConvertBuffer(FXDIB_Format dest_format,
                   uint8_t* dest_buf,
                   int dest_pitch,
                   int width,
                   int height,
                   const CFX_RetainPtr<CFX_DIBSource>& pSrcBitmap,
                   int src_left,
                   int src_top,
                   std::unique_ptr<uint32_t, FxFreeDeleter>* pal);

class IFX_ScanlineComposer {
 public:
  virtual ~IFX_ScanlineComposer() {}

  virtual void ComposeScanline(int line,
                               const uint8_t* scanline,
                               const uint8_t* scan_extra_alpha = nullptr) = 0;

  virtual bool SetInfo(int width,
                       int height,
                       FXDIB_Format src_format,
                       uint32_t* pSrcPalette) = 0;
};

class CFX_BitmapComposer : public IFX_ScanlineComposer {
 public:
  CFX_BitmapComposer();
  ~CFX_BitmapComposer() override;

  void Compose(const CFX_RetainPtr<CFX_DIBitmap>& pDest,
               const CFX_ClipRgn* pClipRgn,
               int bitmap_alpha,
               uint32_t mask_color,
               FX_RECT& dest_rect,
               bool bVertical,
               bool bFlipX,
               bool bFlipY,
               bool bRgbByteOrder = false,
               int alpha_flag = 0,
               void* pIccTransform = nullptr,
               int blend_type = FXDIB_BLEND_NORMAL);

  // IFX_ScanlineComposer
  bool SetInfo(int width,
               int height,
               FXDIB_Format src_format,
               uint32_t* pSrcPalette) override;

  void ComposeScanline(int line,
                       const uint8_t* scanline,
                       const uint8_t* scan_extra_alpha) override;

 protected:
  void DoCompose(uint8_t* dest_scan,
                 const uint8_t* src_scan,
                 int dest_width,
                 const uint8_t* clip_scan,
                 const uint8_t* src_extra_alpha = nullptr,
                 uint8_t* dst_extra_alpha = nullptr);
  void ComposeScanlineV(int line,
                        const uint8_t* scanline,
                        const uint8_t* scan_extra_alpha = nullptr);

  CFX_RetainPtr<CFX_DIBitmap> m_pBitmap;
  const CFX_ClipRgn* m_pClipRgn;
  FXDIB_Format m_SrcFormat;
  int m_DestLeft;
  int m_DestTop;
  int m_DestWidth;
  int m_DestHeight;
  int m_BitmapAlpha;
  uint32_t m_MaskColor;
  CFX_RetainPtr<CFX_DIBitmap> m_pClipMask;
  CFX_ScanlineCompositor m_Compositor;
  bool m_bVertical;
  bool m_bFlipX;
  bool m_bFlipY;
  int m_AlphaFlag;
  void* m_pIccTransform;
  bool m_bRgbByteOrder;
  int m_BlendType;
  uint8_t* m_pScanlineV;
  uint8_t* m_pClipScanV;
  uint8_t* m_pAddClipScan;
  uint8_t* m_pScanlineAlphaV;
};

class CFX_BitmapStorer : public IFX_ScanlineComposer {
 public:
  CFX_BitmapStorer();
  ~CFX_BitmapStorer() override;

  // IFX_ScanlineComposer
  void ComposeScanline(int line,
                       const uint8_t* scanline,
                       const uint8_t* scan_extra_alpha) override;
  bool SetInfo(int width,
               int height,
               FXDIB_Format src_format,
               uint32_t* pSrcPalette) override;

  CFX_RetainPtr<CFX_DIBitmap> GetBitmap() { return m_pBitmap; }
  CFX_RetainPtr<CFX_DIBitmap> Detach();
  void Replace(CFX_RetainPtr<CFX_DIBitmap>&& pBitmap);

 private:
  CFX_RetainPtr<CFX_DIBitmap> m_pBitmap;
};

class CFX_ImageStretcher {
 public:
  CFX_ImageStretcher(IFX_ScanlineComposer* pDest,
                     const CFX_RetainPtr<CFX_DIBSource>& pSource,
                     int dest_width,
                     int dest_height,
                     const FX_RECT& bitmap_rect,
                     uint32_t flags);
  ~CFX_ImageStretcher();

  bool Start();
  bool Continue(IFX_Pause* pPause);

  CFX_RetainPtr<CFX_DIBSource> source() { return m_pSource; }

 private:
  bool StartQuickStretch();
  bool StartStretch();
  bool ContinueQuickStretch(IFX_Pause* pPause);
  bool ContinueStretch(IFX_Pause* pPause);

  IFX_ScanlineComposer* const m_pDest;
  CFX_RetainPtr<CFX_DIBSource> m_pSource;
  std::unique_ptr<CStretchEngine> m_pStretchEngine;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pScanline;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pMaskScanline;
  const uint32_t m_Flags;
  bool m_bFlipX;
  bool m_bFlipY;
  int m_DestWidth;
  int m_DestHeight;
  FX_RECT m_ClipRect;
  const FXDIB_Format m_DestFormat;
  const int m_DestBPP;
  int m_LineIndex;
};

#endif  // CORE_FXGE_FX_DIB_H_
