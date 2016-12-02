// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_RENDER_INT_H_
#define CORE_FPDFAPI_RENDER_RENDER_INT_H_

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
#include "core/fxge/cfx_fxgedevice.h"
#include "core/fxge/cfx_renderdevice.h"

class CCodec_Jbig2Context;
class CCodec_ScanlineDecoder;
class CFX_GlyphBitmap;
class CFX_ImageTransformer;
class CFX_PathData;
class CPDF_Color;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Font;
class CPDF_FormObject;
class CPDF_ImageCacheEntry;
class CPDF_ImageObject;
class CPDF_Object;
class CPDF_PageObject;
class CPDF_PageObjectHolder;
class CPDF_PageRenderCache;
class CPDF_PathObject;
class CPDF_RenderStatus;
class CPDF_ShadingObject;
class CPDF_ShadingPattern;
class CPDF_Stream;
class CPDF_TilingPattern;
class CPDF_TransferFunc;
class CPDF_Type3Cache;
class CPDF_Type3Glyphs;
class CPDF_Type3Char;
class CPDF_Type3Font;

typedef struct {
  FX_FLOAT m_DecodeMin;
  FX_FLOAT m_DecodeStep;
  int m_ColorKeyMin;
  int m_ColorKeyMax;
} DIB_COMP_DATA;

#define FPDF_HUGE_IMAGE_SIZE 60000000

class CPDF_DIBSource : public CFX_DIBSource {
 public:
  CPDF_DIBSource();
  ~CPDF_DIBSource() override;

  bool Load(CPDF_Document* pDoc,
            const CPDF_Stream* pStream,
            CPDF_DIBSource** ppMask,
            uint32_t* pMatteColor,
            CPDF_Dictionary* pFormResources,
            CPDF_Dictionary* pPageResources,
            bool bStdCS = false,
            uint32_t GroupFamily = 0,
            bool bLoadMask = false);

  // CFX_DIBSource
  bool SkipToScanline(int line, IFX_Pause* pPause) const override;
  uint8_t* GetBuffer() const override;
  const uint8_t* GetScanline(int line) const override;
  void DownSampleScanline(int line,
                          uint8_t* dest_scan,
                          int dest_bpp,
                          int dest_width,
                          bool bFlipX,
                          int clip_left,
                          int clip_width) const override;

  CFX_DIBitmap* GetBitmap() const;
  void ReleaseBitmap(CFX_DIBitmap* pBitmap) const;
  uint32_t GetMatteColor() const { return m_MatteColor; }

  int StartLoadDIBSource(CPDF_Document* pDoc,
                         const CPDF_Stream* pStream,
                         bool bHasMask,
                         CPDF_Dictionary* pFormResources,
                         CPDF_Dictionary* pPageResources,
                         bool bStdCS = false,
                         uint32_t GroupFamily = 0,
                         bool bLoadMask = false);
  int ContinueLoadDIBSource(IFX_Pause* pPause);
  int StratLoadMask();
  int StartLoadMaskDIB();
  int ContinueLoadMaskDIB(IFX_Pause* pPause);
  int ContinueToLoadMask();
  CPDF_DIBSource* DetachMask();

 private:
  bool LoadColorInfo(const CPDF_Dictionary* pFormResources,
                     const CPDF_Dictionary* pPageResources);
  DIB_COMP_DATA* GetDecodeAndMaskArray(bool& bDefaultDecode, bool& bColorKey);
  CPDF_DIBSource* LoadMask(uint32_t& MatteColor);
  CPDF_DIBSource* LoadMaskDIB(CPDF_Stream* pMask);
  void LoadJpxBitmap();
  void LoadPalette();
  int CreateDecoder();
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

  CPDF_Document* m_pDocument;
  const CPDF_Stream* m_pStream;
  std::unique_ptr<CPDF_StreamAcc> m_pStreamAcc;
  const CPDF_Dictionary* m_pDict;
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
  std::unique_ptr<CFX_DIBitmap> m_pCachedBitmap;
  std::unique_ptr<CCodec_ScanlineDecoder> m_pDecoder;
  CPDF_DIBSource* m_pMask;
  std::unique_ptr<CPDF_StreamAcc> m_pGlobalStream;
  std::unique_ptr<CCodec_Jbig2Context> m_pJbig2Context;
  CPDF_Stream* m_pMaskStream;
  int m_Status;
};

CCodec_ScanlineDecoder* FPDFAPI_CreateFlateDecoder(
    const uint8_t* src_buf,
    uint32_t src_size,
    int width,
    int height,
    int nComps,
    int bpc,
    const CPDF_Dictionary* pParams);

#endif  // CORE_FPDFAPI_RENDER_RENDER_INT_H_
