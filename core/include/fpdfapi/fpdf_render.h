// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_RENDER_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_RENDER_H_

#include <map>
#include <memory>

#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fxge/fx_ge.h"

class CFX_GraphStateData;
class CFX_PathData;
class CFX_RenderDevice;
class CPDF_FormObject;
class CPDF_ImageCacheEntry;
class CPDF_ImageObject;
class CPDF_PathObject;
class CPDF_RenderContext;
class CPDF_RenderOptions;
class CPDF_RenderStatus;
class CPDF_ShadingObject;
class CPDF_TextObject;
class IFX_Pause;

class IPDF_OCContext {
 public:
  virtual ~IPDF_OCContext() {}

  virtual FX_BOOL CheckOCGVisible(const CPDF_Dictionary* pOCG) = 0;

  FX_BOOL CheckObjectVisible(const CPDF_PageObject* pObj);
};
#define RENDER_COLOR_NORMAL 0
#define RENDER_COLOR_GRAY 1
#define RENDER_COLOR_TWOCOLOR 2
#define RENDER_COLOR_ALPHA 3
#define RENDER_CLEARTYPE 0x00000001
#define RENDER_PRINTGRAPHICTEXT 0x00000002
#define RENDER_FORCE_DOWNSAMPLE 0x00000004
#define RENDER_PRINTPREVIEW 0x00000008
#define RENDER_BGR_STRIPE 0x00000010
#define RENDER_NO_NATIVETEXT 0x00000020
#define RENDER_FORCE_HALFTONE 0x00000040
#define RENDER_RECT_AA 0x00000080
#define RENDER_FILL_FULLCOVER 0x00000100
#define RENDER_PRINTIMAGETEXT 0x00000200
#define RENDER_OVERPRINT 0x00000400
#define RENDER_THINLINE 0x00000800
#define RENDER_NOTEXTSMOOTH 0x10000000
#define RENDER_NOPATHSMOOTH 0x20000000
#define RENDER_NOIMAGESMOOTH 0x40000000
#define RENDER_LIMITEDIMAGECACHE 0x80000000
class CPDF_RenderOptions {
 public:
  CPDF_RenderOptions();

  int m_ColorMode;

  FX_COLORREF m_BackColor;

  FX_COLORREF m_ForeColor;

  FX_DWORD m_Flags;

  int m_Interpolation;

  FX_DWORD m_AddFlags;

  IPDF_OCContext* m_pOCContext;

  FX_DWORD m_dwLimitCacheSize;

  int m_HalftoneLimit;

  FX_ARGB TranslateColor(FX_ARGB argb) const;
};
class CPDF_RenderContext {
 public:
  explicit CPDF_RenderContext(CPDF_Page* pPage);
  CPDF_RenderContext(CPDF_Document* pDoc, CPDF_PageRenderCache* pPageCache);
  ~CPDF_RenderContext();

  void AppendObjectList(CPDF_PageObjects* pObjs,
                        const CFX_Matrix* pObject2Device);

  void Render(CFX_RenderDevice* pDevice,
              const CPDF_RenderOptions* pOptions = NULL,
              const CFX_Matrix* pFinalMatrix = NULL);

  void DrawObjectList(CFX_RenderDevice* pDevice,
                      CPDF_PageObjects* pObjs,
                      const CFX_Matrix* pObject2Device,
                      const CPDF_RenderOptions* pOptions);

  void GetBackground(CFX_DIBitmap* pBuffer,
                     const CPDF_PageObject* pObj,
                     const CPDF_RenderOptions* pOptions,
                     CFX_Matrix* pFinalMatrix);

  CPDF_PageRenderCache* GetPageCache() const { return m_pPageCache; }

 protected:
  void Render(CFX_RenderDevice* pDevice,
              const CPDF_PageObject* pStopObj,
              const CPDF_RenderOptions* pOptions,
              const CFX_Matrix* pFinalMatrix);

  CPDF_Document* const m_pDocument;
  CPDF_Dictionary* m_pPageResources;
  CPDF_PageRenderCache* m_pPageCache;
  CFX_ArrayTemplate<struct _PDF_RenderItem> m_ContentList;
  FX_BOOL m_bFirstLayer;

  friend class CPDF_RenderStatus;
  friend class CPDF_ProgressiveRenderer;
};

class CPDF_ProgressiveRenderer {
 public:
  // Must match FDF_RENDER_* definitions in public/fpdf_progressive.h, but
  // cannot #include that header. fpdfsdk/src/fpdf_progressive.cpp has
  // static_asserts to make sure the two sets of values match.
  enum Status {
    Ready,          // FPDF_RENDER_READER
    ToBeContinued,  // FPDF_RENDER_TOBECOUNTINUED
    Done,           // FPDF_RENDER_DONE
    Failed          // FPDF_RENDER_FAILED
  };

  static int ToFPDFStatus(Status status) { return static_cast<int>(status); }

  CPDF_ProgressiveRenderer(CPDF_RenderContext* pContext,
                           CFX_RenderDevice* pDevice,
                           const CPDF_RenderOptions* pOptions);
  ~CPDF_ProgressiveRenderer();

  Status GetStatus() const { return m_Status; }
  void Start(IFX_Pause* pPause);
  void Continue(IFX_Pause* pPause);
  int EstimateProgress();

 private:
  void RenderStep();

  Status m_Status;
  CPDF_RenderContext* const m_pContext;
  CFX_RenderDevice* const m_pDevice;
  const CPDF_RenderOptions* const m_pOptions;
  std::unique_ptr<CPDF_RenderStatus> m_pRenderStatus;
  CFX_FloatRect m_ClipRect;
  FX_DWORD m_LayerIndex;
  FX_DWORD m_ObjectIndex;
  FX_POSITION m_ObjectPos;
  FX_POSITION m_PrevLastPos;
};

class CPDF_TextRenderer {
 public:
  static void DrawTextString(CFX_RenderDevice* pDevice,
                             int left,
                             int top,
                             CPDF_Font* pFont,
                             int height,
                             const CFX_ByteString& str,
                             FX_ARGB argb);

  static void DrawTextString(CFX_RenderDevice* pDevice,
                             FX_FLOAT origin_x,
                             FX_FLOAT origin_y,
                             CPDF_Font* pFont,
                             FX_FLOAT font_size,
                             const CFX_Matrix* matrix,
                             const CFX_ByteString& str,
                             FX_ARGB fill_argb,
                             FX_ARGB stroke_argb = 0,
                             const CFX_GraphStateData* pGraphState = NULL,
                             const CPDF_RenderOptions* pOptions = NULL);

  static FX_BOOL DrawTextPath(CFX_RenderDevice* pDevice,
                              int nChars,
                              FX_DWORD* pCharCodes,
                              FX_FLOAT* pCharPos,
                              CPDF_Font* pFont,
                              FX_FLOAT font_size,
                              const CFX_Matrix* pText2User,
                              const CFX_Matrix* pUser2Device,
                              const CFX_GraphStateData* pGraphState,
                              FX_ARGB fill_argb,
                              FX_ARGB stroke_argb,
                              CFX_PathData* pClippingPath,
                              int nFlag = 0);

  static FX_BOOL DrawNormalText(CFX_RenderDevice* pDevice,
                                int nChars,
                                FX_DWORD* pCharCodes,
                                FX_FLOAT* pCharPos,
                                CPDF_Font* pFont,
                                FX_FLOAT font_size,
                                const CFX_Matrix* pText2Device,
                                FX_ARGB fill_argb,
                                const CPDF_RenderOptions* pOptions);

  static FX_BOOL DrawType3Text(CFX_RenderDevice* pDevice,
                               int nChars,
                               FX_DWORD* pCharCodes,
                               FX_FLOAT* pCharPos,
                               CPDF_Font* pFont,
                               FX_FLOAT font_size,
                               const CFX_Matrix* pText2Device,
                               FX_ARGB fill_argb);
};
class CPDF_PageRenderCache {
 public:
  explicit CPDF_PageRenderCache(CPDF_Page* pPage)
      : m_pPage(pPage),
        m_pCurImageCacheEntry(nullptr),
        m_nTimeCount(0),
        m_nCacheSize(0),
        m_bCurFindCache(FALSE) {}
  ~CPDF_PageRenderCache();
  void ClearImageData();

  FX_DWORD EstimateSize();
  void CacheOptimization(int32_t dwLimitCacheSize);
  FX_DWORD GetTimeCount() const { return m_nTimeCount; }
  void SetTimeCount(FX_DWORD dwTimeCount) { m_nTimeCount = dwTimeCount; }

  void GetCachedBitmap(CPDF_Stream* pStream,
                       CFX_DIBSource*& pBitmap,
                       CFX_DIBSource*& pMask,
                       FX_DWORD& MatteColor,
                       FX_BOOL bStdCS = FALSE,
                       FX_DWORD GroupFamily = 0,
                       FX_BOOL bLoadMask = FALSE,
                       CPDF_RenderStatus* pRenderStatus = NULL,
                       int32_t downsampleWidth = 0,
                       int32_t downsampleHeight = 0);

  void ResetBitmap(CPDF_Stream* pStream, const CFX_DIBitmap* pBitmap);
  void ClearImageCacheEntry(CPDF_Stream* pStream);
  CPDF_Page* GetPage() const { return m_pPage; }
  CPDF_ImageCacheEntry* GetCurImageCacheEntry() const {
    return m_pCurImageCacheEntry;
  }

  FX_BOOL StartGetCachedBitmap(CPDF_Stream* pStream,
                               FX_BOOL bStdCS = FALSE,
                               FX_DWORD GroupFamily = 0,
                               FX_BOOL bLoadMask = FALSE,
                               CPDF_RenderStatus* pRenderStatus = NULL,
                               int32_t downsampleWidth = 0,
                               int32_t downsampleHeight = 0);

  FX_BOOL Continue(IFX_Pause* pPause);

 protected:
  friend class CPDF_Page;
  CPDF_Page* const m_pPage;
  CPDF_ImageCacheEntry* m_pCurImageCacheEntry;
  std::map<CPDF_Stream*, CPDF_ImageCacheEntry*> m_ImageCache;
  FX_DWORD m_nTimeCount;
  FX_DWORD m_nCacheSize;
  FX_BOOL m_bCurFindCache;
};
class CPDF_RenderConfig {
 public:
  CPDF_RenderConfig();
  ~CPDF_RenderConfig();
  int m_HalftoneLimit;
  int m_RenderStepLimit;
};

FX_BOOL IsAvailableMatrix(const CFX_Matrix& matrix);

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_RENDER_H_
