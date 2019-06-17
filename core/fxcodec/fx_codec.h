// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_FX_CODEC_H_
#define CORE_FXCODEC_FX_CODEC_H_

#include <map>
#include <memory>
#include <utility>

#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_string.h"

#ifdef PDF_ENABLE_XFA
#ifdef PDF_ENABLE_XFA_BMP
#include "core/fxcodec/bmp/bmpmodule.h"
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
#include "core/fxcodec/gif/gifmodule.h"
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
#include "core/fxcodec/png/pngmodule.h"
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_TIFF
#include "core/fxcodec/tiff/tiffmodule.h"
#endif  // PDF_ENABLE_XFA_TIFF
#endif  // PDF_ENABLE_XFA

namespace fxcodec {

#ifdef PDF_ENABLE_XFA
class CFX_DIBAttribute {
 public:
  CFX_DIBAttribute();
  ~CFX_DIBAttribute();

  int32_t m_nXDPI = -1;
  int32_t m_nYDPI = -1;
  uint16_t m_wDPIUnit = 0;
  std::map<uint32_t, void*> m_Exif;
};
#endif  // PDF_ENABLE_XFA

class Jbig2Module;
class JpegModule;
class ProgressiveDecoder;

class ModuleMgr {
 public:
  // Per-process singleton managed by callers.
  static void Create();
  static void Destroy();
  static ModuleMgr* GetInstance();

  JpegModule* GetJpegModule() const { return m_pJpegModule.get(); }
  Jbig2Module* GetJbig2Module() const { return m_pJbig2Module.get(); }

#ifdef PDF_ENABLE_XFA
  std::unique_ptr<ProgressiveDecoder> CreateProgressiveDecoder();

#ifdef PDF_ENABLE_XFA_BMP
  BmpModule* GetBmpModule() const { return m_pBmpModule.get(); }
  void SetBmpModule(std::unique_ptr<BmpModule> module) {
    m_pBmpModule = std::move(module);
  }
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
  GifModule* GetGifModule() const { return m_pGifModule.get(); }
  void SetGifModule(std::unique_ptr<GifModule> module) {
    m_pGifModule = std::move(module);
  }
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
  PngModule* GetPngModule() const { return m_pPngModule.get(); }
  void SetPngModule(std::unique_ptr<PngModule> module) {
    m_pPngModule = std::move(module);
  }
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_TIFF
  TiffModule* GetTiffModule() const { return m_pTiffModule.get(); }
  void SetTiffModule(std::unique_ptr<TiffModule> module) {
    m_pTiffModule = std::move(module);
  }
#endif  // PDF_ENABLE_XFA_TIFF
#endif  // PDF_ENABLE_XFA

 private:
  ModuleMgr();
  ~ModuleMgr();

  std::unique_ptr<JpegModule> m_pJpegModule;
  std::unique_ptr<Jbig2Module> m_pJbig2Module;

#ifdef PDF_ENABLE_XFA
#ifdef PDF_ENABLE_XFA_BMP
  std::unique_ptr<BmpModule> m_pBmpModule;
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
  std::unique_ptr<GifModule> m_pGifModule;
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
  std::unique_ptr<PngModule> m_pPngModule;
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_TIFF
  std::unique_ptr<TiffModule> m_pTiffModule;
#endif  // PDF_ENABLE_XFA_TIFF
#endif  // PDF_ENABLE_XFA
};

void ReverseRGB(uint8_t* pDestBuf, const uint8_t* pSrcBuf, int pixels);

FX_SAFE_UINT32 CalculatePitch8(uint32_t bpc, uint32_t components, int width);
FX_SAFE_UINT32 CalculatePitch32(int bpp, int width);

}  // namespace fxcodec

#ifdef PDF_ENABLE_XFA
using CFX_DIBAttribute = fxcodec::CFX_DIBAttribute;
#endif

#endif  // CORE_FXCODEC_FX_CODEC_H_
