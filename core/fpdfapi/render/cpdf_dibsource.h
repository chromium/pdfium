// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_DIBSOURCE_H_
#define CORE_FPDFAPI_RENDER_CPDF_DIBSOURCE_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_clippath.h"
#include "core/fpdfapi/page/cpdf_countedobject.h"
#include "core/fpdfapi/page/cpdf_graphicstates.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/render/cpdf_imageloader.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/dib/cfx_dibsource.h"

class CCodec_Jbig2Context;
class CCodec_ScanlineDecoder;
class CPDF_Color;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Stream;

typedef struct {
  float m_DecodeMin;
  float m_DecodeStep;
  int m_ColorKeyMin;
  int m_ColorKeyMax;
} DIB_COMP_DATA;

#define FPDF_HUGE_IMAGE_SIZE 60000000

class CPDF_DIBSource : public CFX_DIBSource {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  ~CPDF_DIBSource() override;

  bool Load(CPDF_Document* pDoc, const CPDF_Stream* pStream);

  // CFX_DIBSource
  bool SkipToScanline(int line, IFX_PauseIndicator* pPause) const override;
  uint8_t* GetBuffer() const override;
  const uint8_t* GetScanline(int line) const override;
  void DownSampleScanline(int line,
                          uint8_t* dest_scan,
                          int dest_bpp,
                          int dest_width,
                          bool bFlipX,
                          int clip_left,
                          int clip_width) const override;

  const CPDF_ColorSpace* GetColorSpace() const { return m_pColorSpace; }
  uint32_t GetMatteColor() const { return m_MatteColor; }

  int StartLoadDIBSource(CPDF_Document* pDoc,
                         const CPDF_Stream* pStream,
                         bool bHasMask,
                         CPDF_Dictionary* pFormResources,
                         CPDF_Dictionary* pPageResources,
                         bool bStdCS = false,
                         uint32_t GroupFamily = 0,
                         bool bLoadMask = false);
  int ContinueLoadDIBSource(IFX_PauseIndicator* pPause);
  int StartLoadMask();
  int StartLoadMaskDIB();
  int ContinueLoadMaskDIB(IFX_PauseIndicator* pPause);
  bool ContinueToLoadMask();
  RetainPtr<CPDF_DIBSource> DetachMask();

 private:
  CPDF_DIBSource();

  bool LoadColorInfo(const CPDF_Dictionary* pFormResources,
                     const CPDF_Dictionary* pPageResources);
  DIB_COMP_DATA* GetDecodeAndMaskArray(bool* bDefaultDecode, bool* bColorKey);
  void LoadJpxBitmap();
  void LoadPalette();
  int CreateDecoder();
  bool CreateDCTDecoder(const uint8_t* src_data,
                        uint32_t src_size,
                        const CPDF_Dictionary* pParams);
  void TranslateScanline24bpp(uint8_t* dest_scan,
                              const uint8_t* src_scan) const;
  void ValidateDictParam();
  void DownSampleScanline1Bit(int orig_Bpp,
                              int dest_Bpp,
                              uint32_t src_width,
                              const uint8_t* pSrcLine,
                              uint8_t* dest_scan,
                              int dest_width,
                              bool bFlipX,
                              int clip_left,
                              int clip_width) const;
  void DownSampleScanline8Bit(int orig_Bpp,
                              int dest_Bpp,
                              uint32_t src_width,
                              const uint8_t* pSrcLine,
                              uint8_t* dest_scan,
                              int dest_width,
                              bool bFlipX,
                              int clip_left,
                              int clip_width) const;
  void DownSampleScanline32Bit(int orig_Bpp,
                               int dest_Bpp,
                               uint32_t src_width,
                               const uint8_t* pSrcLine,
                               uint8_t* dest_scan,
                               int dest_width,
                               bool bFlipX,
                               int clip_left,
                               int clip_width) const;
  bool TransMask() const;

  UnownedPtr<CPDF_Document> m_pDocument;
  UnownedPtr<const CPDF_Stream> m_pStream;
  UnownedPtr<const CPDF_Dictionary> m_pDict;
  RetainPtr<CPDF_StreamAcc> m_pStreamAcc;
  CPDF_ColorSpace* m_pColorSpace;
  uint32_t m_Family;
  uint32_t m_bpc;
  uint32_t m_bpc_orig;
  uint32_t m_nComponents;
  uint32_t m_GroupFamily;
  uint32_t m_MatteColor;
  bool m_bLoadMask;
  bool m_bDefaultDecode;
  bool m_bImageMask;
  bool m_bDoBpcCheck;
  bool m_bColorKey;
  bool m_bHasMask;
  bool m_bStdCS;
  DIB_COMP_DATA* m_pCompData;
  uint8_t* m_pLineBuf;
  uint8_t* m_pMaskedLine;
  RetainPtr<CFX_DIBitmap> m_pCachedBitmap;
  RetainPtr<CPDF_DIBSource> m_pMask;
  RetainPtr<CPDF_StreamAcc> m_pGlobalStream;
  std::unique_ptr<CCodec_ScanlineDecoder> m_pDecoder;
  std::unique_ptr<CCodec_Jbig2Context> m_pJbig2Context;
  UnownedPtr<CPDF_Stream> m_pMaskStream;
  int m_Status;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_DIBSOURCE_H_
