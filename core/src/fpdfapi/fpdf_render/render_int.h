// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_RENDER_RENDER_INT_H_
#define CORE_SRC_FPDFAPI_FPDF_RENDER_RENDER_INT_H_

#include <map>
#include <memory>

#include "core/include/fpdfapi/fpdf_pageobj.h"
#include "core/include/fpdfapi/fpdf_render.h"

class CFX_GlyphBitmap;
class CFX_ImageTransformer;
class CPDF_ImageCacheEntry;
class CPDF_ImageLoaderHandle;
class ICodec_ScanlineDecoder;

#define TYPE3_MAX_BLUES 16

class CPDF_Type3Glyphs {
 public:
  CPDF_Type3Glyphs() : m_TopBlueCount(0), m_BottomBlueCount(0) {}
  ~CPDF_Type3Glyphs();
  void AdjustBlue(FX_FLOAT top,
                  FX_FLOAT bottom,
                  int& top_line,
                  int& bottom_line);

  std::map<FX_DWORD, CFX_GlyphBitmap*> m_GlyphMap;
  int m_TopBlue[TYPE3_MAX_BLUES];
  int m_BottomBlue[TYPE3_MAX_BLUES];
  int m_TopBlueCount;
  int m_BottomBlueCount;
};
class CPDF_Type3Cache {
 public:
  explicit CPDF_Type3Cache(CPDF_Type3Font* pFont) : m_pFont(pFont) {}
  ~CPDF_Type3Cache();

  CFX_GlyphBitmap* LoadGlyph(FX_DWORD charcode,
                             const CFX_Matrix* pMatrix,
                             FX_FLOAT retinaScaleX = 1.0f,
                             FX_FLOAT retinaScaleY = 1.0f);

 protected:
  CFX_GlyphBitmap* RenderGlyph(CPDF_Type3Glyphs* pSize,
                               FX_DWORD charcode,
                               const CFX_Matrix* pMatrix,
                               FX_FLOAT retinaScaleX = 1.0f,
                               FX_FLOAT retinaScaleY = 1.0f);
  CPDF_Type3Font* const m_pFont;
  std::map<CFX_ByteString, CPDF_Type3Glyphs*> m_SizeMap;
};

class CPDF_TransferFunc {
 public:
  explicit CPDF_TransferFunc(CPDF_Document* pDoc);

  FX_COLORREF TranslateColor(FX_COLORREF src) const;
  CFX_DIBSource* TranslateImage(const CFX_DIBSource* pSrc,
                                FX_BOOL bAutoDropSrc);

  CPDF_Document* const m_pPDFDoc;
  FX_BOOL m_bIdentity;
  uint8_t m_Samples[256 * 3];
};

class CPDF_DocRenderData {
 public:
  CPDF_DocRenderData(CPDF_Document* pPDFDoc = NULL);
  ~CPDF_DocRenderData();
  CPDF_Type3Cache* GetCachedType3(CPDF_Type3Font* pFont);
  CPDF_TransferFunc* GetTransferFunc(CPDF_Object* pObj);
  CFX_FontCache* GetFontCache() { return m_pFontCache; }
  void Clear(FX_BOOL bRelease = FALSE);
  void ReleaseCachedType3(CPDF_Type3Font* pFont);
  void ReleaseTransferFunc(CPDF_Object* pObj);

 private:
  using CPDF_Type3CacheMap =
      std::map<CPDF_Font*, CPDF_CountedObject<CPDF_Type3Cache>*>;
  using CPDF_TransferFuncMap =
      std::map<CPDF_Object*, CPDF_CountedObject<CPDF_TransferFunc>*>;

  CPDF_Document* m_pPDFDoc;
  CFX_FontCache* m_pFontCache;
  CPDF_Type3CacheMap m_Type3FaceMap;
  CPDF_TransferFuncMap m_TransferFuncMap;
};
struct _PDF_RenderItem {
 public:
  CPDF_PageObjects* m_pObjectList;
  CFX_Matrix m_Matrix;
};

typedef CFX_ArrayTemplate<_PDF_RenderItem> CPDF_RenderLayer;

class IPDF_ObjectRenderer {
 public:
  static IPDF_ObjectRenderer* Create(int type);
  virtual ~IPDF_ObjectRenderer() {}
  virtual FX_BOOL Start(CPDF_RenderStatus* pRenderStatus,
                        const CPDF_PageObject* pObj,
                        const CFX_Matrix* pObj2Device,
                        FX_BOOL bStdCS,
                        int blendType = FXDIB_BLEND_NORMAL) = 0;
  virtual FX_BOOL Continue(IFX_Pause* pPause) = 0;
  FX_BOOL m_Result;
};

class CPDF_RenderStatus {
 public:
  CPDF_RenderStatus();
  ~CPDF_RenderStatus();
  FX_BOOL Initialize(class CPDF_RenderContext* pContext,
                     CFX_RenderDevice* pDevice,
                     const CFX_Matrix* pDeviceMatrix,
                     const CPDF_PageObject* pStopObj,
                     const CPDF_RenderStatus* pParentStatus,
                     const CPDF_GraphicStates* pInitialStates,
                     const CPDF_RenderOptions* pOptions,
                     int transparency,
                     FX_BOOL bDropObjects,
                     CPDF_Dictionary* pFormResource = NULL,
                     FX_BOOL bStdCS = FALSE,
                     CPDF_Type3Char* pType3Char = NULL,
                     FX_ARGB fill_color = 0,
                     FX_DWORD GroupFamily = 0,
                     FX_BOOL bLoadMask = FALSE);
  void RenderObjectList(const CPDF_PageObjects* pObjs,
                        const CFX_Matrix* pObj2Device);
  void RenderSingleObject(const CPDF_PageObject* pObj,
                          const CFX_Matrix* pObj2Device);
  FX_BOOL ContinueSingleObject(const CPDF_PageObject* pObj,
                               const CFX_Matrix* pObj2Device,
                               IFX_Pause* pPause);
  CPDF_RenderContext* GetContext() { return m_pContext; }

  CPDF_RenderOptions m_Options;
  CPDF_Dictionary* m_pFormResource;
  CPDF_Dictionary* m_pPageResource;
  CFX_ArrayTemplate<CPDF_Type3Font*> m_Type3FontCache;

 protected:
  friend class CPDF_ImageRenderer;
  friend class CPDF_RenderContext;
  void ProcessClipPath(CPDF_ClipPath ClipPath, const CFX_Matrix* pObj2Device);
  void DrawClipPath(CPDF_ClipPath ClipPath, const CFX_Matrix* pObj2Device);
  FX_BOOL ProcessTransparency(const CPDF_PageObject* PageObj,
                              const CFX_Matrix* pObj2Device);
  void ProcessObjectNoClip(const CPDF_PageObject* PageObj,
                           const CFX_Matrix* pObj2Device);
  void DrawObjWithBackground(const CPDF_PageObject* pObj,
                             const CFX_Matrix* pObj2Device);
  FX_BOOL DrawObjWithBlend(const CPDF_PageObject* pObj,
                           const CFX_Matrix* pObj2Device);
  FX_BOOL ProcessPath(CPDF_PathObject* pPathObj, const CFX_Matrix* pObj2Device);
  void ProcessPathPattern(CPDF_PathObject* pPathObj,
                          const CFX_Matrix* pObj2Device,
                          int& filltype,
                          FX_BOOL& bStroke);
  void DrawPathWithPattern(CPDF_PathObject* pPathObj,
                           const CFX_Matrix* pObj2Device,
                           CPDF_Color* pColor,
                           FX_BOOL bStroke);
  void DrawTilingPattern(CPDF_TilingPattern* pPattern,
                         CPDF_PageObject* pPageObj,
                         const CFX_Matrix* pObj2Device,
                         FX_BOOL bStroke);
  void DrawShadingPattern(CPDF_ShadingPattern* pPattern,
                          CPDF_PageObject* pPageObj,
                          const CFX_Matrix* pObj2Device,
                          FX_BOOL bStroke);
  FX_BOOL SelectClipPath(CPDF_PathObject* pPathObj,
                         const CFX_Matrix* pObj2Device,
                         FX_BOOL bStroke);
  FX_BOOL ProcessImage(CPDF_ImageObject* pImageObj,
                       const CFX_Matrix* pObj2Device);
  FX_BOOL OutputBitmapAlpha(CPDF_ImageObject* pImageObj,
                            const CFX_Matrix* pImage2Device);
  FX_BOOL OutputImage(CPDF_ImageObject* pImageObj,
                      const CFX_Matrix* pImage2Device);
  FX_BOOL OutputDIBSource(const CFX_DIBSource* pOutputBitmap,
                          FX_ARGB fill_argb,
                          int bitmap_alpha,
                          const CFX_Matrix* pImage2Device,
                          CPDF_ImageCacheEntry* pImageCache,
                          FX_DWORD flags);
  void CompositeDIBitmap(CFX_DIBitmap* pDIBitmap,
                         int left,
                         int top,
                         FX_ARGB mask_argb,
                         int bitmap_alpha,
                         int blend_mode,
                         int bIsolated);
  FX_BOOL ProcessShading(CPDF_ShadingObject* pShadingObj,
                         const CFX_Matrix* pObj2Device);
  void DrawShading(CPDF_ShadingPattern* pPattern,
                   CFX_Matrix* pMatrix,
                   FX_RECT& clip_rect,
                   int alpha,
                   FX_BOOL bAlphaMode);
  FX_BOOL ProcessType3Text(const CPDF_TextObject* textobj,
                           const CFX_Matrix* pObj2Device);
  FX_BOOL ProcessText(const CPDF_TextObject* textobj,
                      const CFX_Matrix* pObj2Device,
                      CFX_PathData* pClippingPath);
  void DrawTextPathWithPattern(const CPDF_TextObject* textobj,
                               const CFX_Matrix* pObj2Device,
                               CPDF_Font* pFont,
                               FX_FLOAT font_size,
                               const CFX_Matrix* pTextMatrix,
                               FX_BOOL bFill,
                               FX_BOOL bStroke);
  FX_BOOL ProcessForm(CPDF_FormObject* pFormObj, const CFX_Matrix* pObj2Device);
  CFX_DIBitmap* GetBackdrop(const CPDF_PageObject* pObj,
                            const FX_RECT& rect,
                            int& left,
                            int& top,
                            FX_BOOL bBackAlphaRequired);
  CFX_DIBitmap* LoadSMask(CPDF_Dictionary* pSMaskDict,
                          FX_RECT* pClipRect,
                          const CFX_Matrix* pMatrix);
  void Init(CPDF_RenderContext* pParent);
  static class CPDF_Type3Cache* GetCachedType3(CPDF_Type3Font* pFont);
  static CPDF_GraphicStates* CloneObjStates(const CPDF_GraphicStates* pPathObj,
                                            FX_BOOL bStroke);
  CPDF_TransferFunc* GetTransferFunc(CPDF_Object* pObject) const;
  FX_ARGB GetFillArgb(const CPDF_PageObject* pObj,
                      FX_BOOL bType3 = FALSE) const;
  FX_ARGB GetStrokeArgb(const CPDF_PageObject* pObj) const;
  CPDF_RenderContext* m_pContext;
  FX_BOOL m_bStopped;
  void DitherObjectArea(const CPDF_PageObject* pObj,
                        const CFX_Matrix* pObj2Device);
  FX_BOOL GetObjectClippedRect(const CPDF_PageObject* pObj,
                               const CFX_Matrix* pObj2Device,
                               FX_BOOL bLogical,
                               FX_RECT& rect) const;
  void GetScaledMatrix(CFX_Matrix& matrix) const;

 protected:
  static const int kRenderMaxRecursionDepth = 64;
  static int s_CurrentRecursionDepth;

  CFX_RenderDevice* m_pDevice;
  CFX_Matrix m_DeviceMatrix;
  CPDF_ClipPath m_LastClipPath;
  const CPDF_PageObject* m_pCurObj;
  const CPDF_PageObject* m_pStopObj;
  CPDF_GraphicStates m_InitialStates;
  int m_HalftoneLimit;
  std::unique_ptr<IPDF_ObjectRenderer> m_pObjectRenderer;
  FX_BOOL m_bPrint;
  int m_Transparency;
  int m_DitherBits;
  FX_BOOL m_bDropObjects;
  FX_BOOL m_bStdCS;
  FX_DWORD m_GroupFamily;
  FX_BOOL m_bLoadMask;
  CPDF_Type3Char* m_pType3Char;
  FX_ARGB m_T3FillColor;
  int m_curBlend;
};
class CPDF_ImageLoader {
 public:
  CPDF_ImageLoader()
      : m_pBitmap(nullptr),
        m_pMask(nullptr),
        m_MatteColor(0),
        m_bCached(FALSE),
        m_nDownsampleWidth(0),
        m_nDownsampleHeight(0) {}
  ~CPDF_ImageLoader();

  FX_BOOL Start(const CPDF_ImageObject* pImage,
                CPDF_PageRenderCache* pCache,
                CPDF_ImageLoaderHandle*& LoadHandle,
                FX_BOOL bStdCS = FALSE,
                FX_DWORD GroupFamily = 0,
                FX_BOOL bLoadMask = FALSE,
                CPDF_RenderStatus* pRenderStatus = NULL,
                int32_t nDownsampleWidth = 0,
                int32_t nDownsampleHeight = 0);
  FX_BOOL Continue(CPDF_ImageLoaderHandle* LoadHandle, IFX_Pause* pPause);

  CFX_DIBSource* m_pBitmap;
  CFX_DIBSource* m_pMask;
  FX_DWORD m_MatteColor;
  FX_BOOL m_bCached;

 protected:
  int32_t m_nDownsampleWidth;
  int32_t m_nDownsampleHeight;
};
class CPDF_ImageLoaderHandle {
 public:
  CPDF_ImageLoaderHandle();
  ~CPDF_ImageLoaderHandle();

  FX_BOOL Start(CPDF_ImageLoader* pImageLoader,
                const CPDF_ImageObject* pImage,
                CPDF_PageRenderCache* pCache,
                FX_BOOL bStdCS = FALSE,
                FX_DWORD GroupFamily = 0,
                FX_BOOL bLoadMask = FALSE,
                CPDF_RenderStatus* pRenderStatus = NULL,
                int32_t nDownsampleWidth = 0,
                int32_t nDownsampleHeight = 0);
  FX_BOOL Continue(IFX_Pause* pPause);

 protected:
  CPDF_ImageLoader* m_pImageLoader;
  CPDF_PageRenderCache* m_pCache;
  CPDF_ImageObject* m_pImage;
  int32_t m_nDownsampleWidth;
  int32_t m_nDownsampleHeight;
};

class CPDF_ImageRenderer : public IPDF_ObjectRenderer {
 public:
  CPDF_ImageRenderer();
  ~CPDF_ImageRenderer() override;

  // IPDF_ObjectRenderer
  FX_BOOL Start(CPDF_RenderStatus* pStatus,
                const CPDF_PageObject* pObj,
                const CFX_Matrix* pObj2Device,
                FX_BOOL bStdCS,
                int blendType = FXDIB_BLEND_NORMAL) override;
  FX_BOOL Continue(IFX_Pause* pPause) override;

  FX_BOOL Start(CPDF_RenderStatus* pStatus,
                const CFX_DIBSource* pDIBSource,
                FX_ARGB bitmap_argb,
                int bitmap_alpha,
                const CFX_Matrix* pImage2Device,
                FX_DWORD flags,
                FX_BOOL bStdCS,
                int blendType = FXDIB_BLEND_NORMAL);

 protected:
  CPDF_RenderStatus* m_pRenderStatus;
  CPDF_ImageObject* m_pImageObject;
  int m_Status;
  const CFX_Matrix* m_pObj2Device;
  CFX_Matrix m_ImageMatrix;
  CPDF_ImageLoader m_Loader;
  const CFX_DIBSource* m_pDIBSource;
  CFX_DIBitmap* m_pClone;
  int m_BitmapAlpha;
  FX_BOOL m_bPatternColor;
  CPDF_Pattern* m_pPattern;
  FX_ARGB m_FillArgb;
  FX_DWORD m_Flags;
  CFX_ImageTransformer* m_pTransformer;
  void* m_DeviceHandle;
  CPDF_ImageLoaderHandle* m_LoadHandle;
  FX_BOOL m_bStdCS;
  int m_BlendType;
  FX_BOOL StartBitmapAlpha();
  FX_BOOL StartDIBSource();
  FX_BOOL StartRenderDIBSource();
  FX_BOOL StartLoadDIBSource();
  FX_BOOL DrawMaskedImage();
  FX_BOOL DrawPatternImage(const CFX_Matrix* pObj2Device);
};

class CPDF_ScaledRenderBuffer {
 public:
  CPDF_ScaledRenderBuffer();
  ~CPDF_ScaledRenderBuffer();

  FX_BOOL Initialize(CPDF_RenderContext* pContext,
                     CFX_RenderDevice* pDevice,
                     const FX_RECT& pRect,
                     const CPDF_PageObject* pObj,
                     const CPDF_RenderOptions* pOptions = NULL,
                     int max_dpi = 0);
  CFX_RenderDevice* GetDevice() {
    return m_pBitmapDevice ? m_pBitmapDevice.get() : m_pDevice;
  }
  CFX_Matrix* GetMatrix() { return &m_Matrix; }
  void OutputToDevice();

 private:
  CFX_RenderDevice* m_pDevice;
  CPDF_RenderContext* m_pContext;
  FX_RECT m_Rect;
  const CPDF_PageObject* m_pObject;
  std::unique_ptr<CFX_FxgeDevice> m_pBitmapDevice;
  CFX_Matrix m_Matrix;
};

class CPDF_DeviceBuffer {
 public:
  CPDF_DeviceBuffer();
  ~CPDF_DeviceBuffer();
  FX_BOOL Initialize(CPDF_RenderContext* pContext,
                     CFX_RenderDevice* pDevice,
                     FX_RECT* pRect,
                     const CPDF_PageObject* pObj,
                     int max_dpi = 0);
  void OutputToDevice();
  CFX_DIBitmap* GetBitmap() const { return m_pBitmap.get(); }
  const CFX_Matrix* GetMatrix() const { return &m_Matrix; }

 private:
  CFX_RenderDevice* m_pDevice;
  CPDF_RenderContext* m_pContext;
  FX_RECT m_Rect;
  const CPDF_PageObject* m_pObject;
  std::unique_ptr<CFX_DIBitmap> m_pBitmap;
  CFX_Matrix m_Matrix;
};

class CPDF_ImageCacheEntry {
 public:
  CPDF_ImageCacheEntry(CPDF_Document* pDoc, CPDF_Stream* pStream);
  ~CPDF_ImageCacheEntry();
  void ClearImageData();
  void Reset(const CFX_DIBitmap* pBitmap);
  FX_BOOL GetCachedBitmap(CFX_DIBSource*& pBitmap,
                          CFX_DIBSource*& pMask,
                          FX_DWORD& MatteColor,
                          CPDF_Dictionary* pPageResources,
                          FX_BOOL bStdCS = FALSE,
                          FX_DWORD GroupFamily = 0,
                          FX_BOOL bLoadMask = FALSE,
                          CPDF_RenderStatus* pRenderStatus = NULL,
                          int32_t downsampleWidth = 0,
                          int32_t downsampleHeight = 0);
  FX_DWORD EstimateSize() const { return m_dwCacheSize; }
  FX_DWORD GetTimeCount() const { return m_dwTimeCount; }
  CPDF_Stream* GetStream() const { return m_pStream; }
  void SetTimeCount(FX_DWORD dwTimeCount) { m_dwTimeCount = dwTimeCount; }
  int m_dwTimeCount;

 public:
  int StartGetCachedBitmap(CPDF_Dictionary* pFormResources,
                           CPDF_Dictionary* pPageResources,
                           FX_BOOL bStdCS = FALSE,
                           FX_DWORD GroupFamily = 0,
                           FX_BOOL bLoadMask = FALSE,
                           CPDF_RenderStatus* pRenderStatus = NULL,
                           int32_t downsampleWidth = 0,
                           int32_t downsampleHeight = 0);
  int Continue(IFX_Pause* pPause);
  CFX_DIBSource* DetachBitmap();
  CFX_DIBSource* DetachMask();
  CFX_DIBSource* m_pCurBitmap;
  CFX_DIBSource* m_pCurMask;
  FX_DWORD m_MatteColor;
  CPDF_RenderStatus* m_pRenderStatus;

 protected:
  void ContinueGetCachedBitmap();

  CPDF_Document* m_pDocument;
  CPDF_Stream* m_pStream;
  CFX_DIBSource* m_pCachedBitmap;
  CFX_DIBSource* m_pCachedMask;
  FX_DWORD m_dwCacheSize;
  void CalcSize();
};
typedef struct {
  FX_FLOAT m_DecodeMin;
  FX_FLOAT m_DecodeStep;
  int m_ColorKeyMin;
  int m_ColorKeyMax;
} DIB_COMP_DATA;

class CPDF_DIBSource : public CFX_DIBSource {
 public:
  CPDF_DIBSource();
  ~CPDF_DIBSource() override;

  FX_BOOL Load(CPDF_Document* pDoc,
               const CPDF_Stream* pStream,
               CPDF_DIBSource** ppMask,
               FX_DWORD* pMatteColor,
               CPDF_Dictionary* pFormResources,
               CPDF_Dictionary* pPageResources,
               FX_BOOL bStdCS = FALSE,
               FX_DWORD GroupFamily = 0,
               FX_BOOL bLoadMask = FALSE);

  // CFX_DIBSource
  FX_BOOL SkipToScanline(int line, IFX_Pause* pPause) const override;
  uint8_t* GetBuffer() const override;
  const uint8_t* GetScanline(int line) const override;
  void DownSampleScanline(int line,
                          uint8_t* dest_scan,
                          int dest_bpp,
                          int dest_width,
                          FX_BOOL bFlipX,
                          int clip_left,
                          int clip_width) const override;
  void SetDownSampleSize(int dest_width, int dest_height) const override;

  CFX_DIBitmap* GetBitmap() const;
  void ReleaseBitmap(CFX_DIBitmap*) const;
  void ClearImageData();

  int StartLoadDIBSource(CPDF_Document* pDoc,
                         const CPDF_Stream* pStream,
                         FX_BOOL bHasMask,
                         CPDF_Dictionary* pFormResources,
                         CPDF_Dictionary* pPageResources,
                         FX_BOOL bStdCS = FALSE,
                         FX_DWORD GroupFamily = 0,
                         FX_BOOL bLoadMask = FALSE);
  int ContinueLoadDIBSource(IFX_Pause* pPause);
  int StratLoadMask();
  int StartLoadMaskDIB();
  int ContinueLoadMaskDIB(IFX_Pause* pPause);
  int ContinueToLoadMask();
  CPDF_DIBSource* DetachMask();
  CPDF_DIBSource* m_pMask;
  FX_DWORD m_MatteColor;
  void* m_pJbig2Context;
  CPDF_StreamAcc* m_pGlobalStream;
  FX_BOOL m_bStdCS;
  int m_Status;
  CPDF_Stream* m_pMaskStream;
  FX_BOOL m_bHasMask;

 private:
  bool LoadColorInfo(const CPDF_Dictionary* pFormResources,
                     const CPDF_Dictionary* pPageResources);
  DIB_COMP_DATA* GetDecodeAndMaskArray(FX_BOOL& bDefaultDecode,
                                       FX_BOOL& bColorKey);
  CPDF_DIBSource* LoadMask(FX_DWORD& MatteColor);
  CPDF_DIBSource* LoadMaskDIB(CPDF_Stream* pMask);
  void LoadJpxBitmap();
  void LoadPalette();
  int CreateDecoder();
  void TranslateScanline24bpp(uint8_t* dest_scan,
                              const uint8_t* src_scan) const;
  void ValidateDictParam();
  void DownSampleScanline1Bit(int orig_Bpp,
                              int dest_Bpp,
                              FX_DWORD src_width,
                              const uint8_t* pSrcLine,
                              uint8_t* dest_scan,
                              int dest_width,
                              FX_BOOL bFlipX,
                              int clip_left,
                              int clip_width) const;
  void DownSampleScanline8Bit(int orig_Bpp,
                              int dest_Bpp,
                              FX_DWORD src_width,
                              const uint8_t* pSrcLine,
                              uint8_t* dest_scan,
                              int dest_width,
                              FX_BOOL bFlipX,
                              int clip_left,
                              int clip_width) const;
  void DownSampleScanline32Bit(int orig_Bpp,
                               int dest_Bpp,
                               FX_DWORD src_width,
                               const uint8_t* pSrcLine,
                               uint8_t* dest_scan,
                               int dest_width,
                               FX_BOOL bFlipX,
                               int clip_left,
                               int clip_width) const;
  FX_BOOL TransMask() const;

  CPDF_Document* m_pDocument;
  const CPDF_Stream* m_pStream;
  CPDF_StreamAcc* m_pStreamAcc;
  const CPDF_Dictionary* m_pDict;
  CPDF_ColorSpace* m_pColorSpace;
  FX_DWORD m_Family;
  FX_DWORD m_bpc;
  FX_DWORD m_bpc_orig;
  FX_DWORD m_nComponents;
  FX_DWORD m_GroupFamily;
  FX_BOOL m_bLoadMask;
  FX_BOOL m_bDefaultDecode;
  FX_BOOL m_bImageMask;
  FX_BOOL m_bDoBpcCheck;
  FX_BOOL m_bColorKey;
  DIB_COMP_DATA* m_pCompData;
  uint8_t* m_pLineBuf;
  uint8_t* m_pMaskedLine;
  std::unique_ptr<CFX_DIBitmap> m_pCachedBitmap;
  ICodec_ScanlineDecoder* m_pDecoder;
};

#define FPDF_HUGE_IMAGE_SIZE 60000000
class CPDF_DIBTransferFunc : public CFX_FilteredDIB {
 public:
  CPDF_DIBTransferFunc(const CPDF_TransferFunc* pTransferFunc);
  ~CPDF_DIBTransferFunc() override;

  // CFX_FilteredDIB
  FXDIB_Format GetDestFormat() override;
  FX_ARGB* GetDestPalette() override { return NULL; }
  void TranslateScanline(uint8_t* dest_buf,
                         const uint8_t* src_buf) const override;
  void TranslateDownSamples(uint8_t* dest_buf,
                            const uint8_t* src_buf,
                            int pixels,
                            int Bpp) const override;

  const uint8_t* m_RampR;
  const uint8_t* m_RampG;
  const uint8_t* m_RampB;
};

struct _CPDF_UniqueKeyGen {
  void Generate(int count, ...);
  FX_CHAR m_Key[128];
  int m_KeyLen;
};

#endif  // CORE_SRC_FPDFAPI_FPDF_RENDER_RENDER_INT_H_
