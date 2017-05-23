// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_IMAGECACHEENTRY_H_
#define CORE_FPDFAPI_RENDER_CPDF_IMAGECACHEENTRY_H_

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_system.h"

class CFX_DIBSource;
class CFX_DIBitmap;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Image;
class CPDF_RenderStatus;
class IFX_Pause;

class CPDF_ImageCacheEntry {
 public:
  CPDF_ImageCacheEntry(CPDF_Document* pDoc,
                       const CFX_RetainPtr<CPDF_Image>& pImage);
  ~CPDF_ImageCacheEntry();

  void Reset(const CFX_RetainPtr<CFX_DIBitmap>& pBitmap);
  uint32_t EstimateSize() const { return m_dwCacheSize; }
  uint32_t GetTimeCount() const { return m_dwTimeCount; }
  CPDF_Image* GetImage() const { return m_pImage.Get(); }
  int StartGetCachedBitmap(CPDF_Dictionary* pFormResources,
                           CPDF_Dictionary* pPageResources,
                           bool bStdCS,
                           uint32_t GroupFamily,
                           bool bLoadMask,
                           CPDF_RenderStatus* pRenderStatus);
  int Continue(IFX_Pause* pPause, CPDF_RenderStatus* pRenderStatus);
  CFX_RetainPtr<CFX_DIBSource> DetachBitmap();
  CFX_RetainPtr<CFX_DIBSource> DetachMask();

  int m_dwTimeCount;
  uint32_t m_MatteColor;

 private:
  void ContinueGetCachedBitmap(CPDF_RenderStatus* pRenderStatus);
  void CalcSize();

  CFX_UnownedPtr<CPDF_Document> const m_pDocument;
  CFX_RetainPtr<CPDF_Image> const m_pImage;
  CFX_RetainPtr<CFX_DIBSource> m_pCurBitmap;
  CFX_RetainPtr<CFX_DIBSource> m_pCurMask;
  CFX_RetainPtr<CFX_DIBSource> m_pCachedBitmap;
  CFX_RetainPtr<CFX_DIBSource> m_pCachedMask;
  uint32_t m_dwCacheSize;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_IMAGECACHEENTRY_H_
