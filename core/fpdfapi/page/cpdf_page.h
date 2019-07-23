// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PAGE_H_
#define CORE_FPDFAPI_PAGE_CPDF_PAGE_H_

#include <memory>
#include <utility>

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fpdfapi/page/ipdf_page.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/optional.h"

class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Image;
class CPDF_Object;

class CPDF_Page final : public IPDF_Page, public CPDF_PageObjectHolder {
 public:
  class View {};  // Caller implements as desired, empty here due to layering.

  // Data for the render layer to attach to this page.
  class RenderContextIface {
   public:
    virtual ~RenderContextIface() {}
  };

  // Cache for the render layer to attach to this page.
  class RenderCacheIface {
   public:
    virtual ~RenderCacheIface() {}
    virtual void ResetBitmapForImage(const RetainPtr<CPDF_Image>& pImage) = 0;
  };

  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IPDF_Page:
  CPDF_Page* AsPDFPage() override;
  CPDFXFA_Page* AsXFAPage() override;
  CPDF_Document* GetDocument() const override;
  float GetPageWidth() const override;
  float GetPageHeight() const override;
  CFX_Matrix GetDisplayMatrix(const FX_RECT& rect, int iRotate) const override;
  Optional<CFX_PointF> DeviceToPage(
      const FX_RECT& rect,
      int rotate,
      const CFX_PointF& device_point) const override;
  Optional<CFX_PointF> PageToDevice(
      const FX_RECT& rect,
      int rotate,
      const CFX_PointF& page_point) const override;

  // CPDF_PageObjectHolder:
  bool IsPage() const override;

  void ParseContent();
  const CFX_SizeF& GetPageSize() const { return m_PageSize; }
  int GetPageRotation() const;
  RenderCacheIface* GetRenderCache() const { return m_pRenderCache.get(); }
  void SetRenderCache(std::unique_ptr<RenderCacheIface> pCache) {
    m_pRenderCache = std::move(pCache);
  }

  RenderContextIface* GetRenderContext() const {
    return m_pRenderContext.get();
  }
  void SetRenderContext(std::unique_ptr<RenderContextIface> pContext) {
    m_pRenderContext = std::move(pContext);
  }

  CPDF_Document* GetPDFDocument() const { return m_pPDFDocument.Get(); }
  View* GetView() const { return m_pView.Get(); }
  void SetView(View* pView) { m_pView = pView; }
  void UpdateDimensions();

 private:
  CPDF_Page(CPDF_Document* pDocument, CPDF_Dictionary* pPageDict);
  ~CPDF_Page() override;

  CPDF_Object* GetPageAttr(const ByteString& name) const;
  CFX_FloatRect GetBox(const ByteString& name) const;

  CFX_SizeF m_PageSize;
  CFX_Matrix m_PageMatrix;
  UnownedPtr<CPDF_Document> m_pPDFDocument;
  std::unique_ptr<RenderCacheIface> m_pRenderCache;
  std::unique_ptr<RenderContextIface> m_pRenderContext;
  UnownedPtr<View> m_pView;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PAGE_H_
