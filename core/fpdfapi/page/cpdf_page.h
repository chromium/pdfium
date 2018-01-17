// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PAGE_H_
#define CORE_FPDFAPI_PAGE_CPDF_PAGE_H_

#include <memory>

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"

class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Object;
class CPDF_PageRenderCache;
class CPDF_PageRenderContext;

class CPDF_Page : public CPDF_PageObjectHolder {
 public:
  class View {};  // Caller implements as desired, empty here due to layering.

  CPDF_Page(CPDF_Document* pDocument,
            CPDF_Dictionary* pPageDict,
            bool bPageCache);
  ~CPDF_Page() override;

  // CPDF_PageObjectHolder
  bool IsPage() const override;

  void ParseContent();

  CFX_Matrix GetDisplayMatrix(int xPos,
                              int yPos,
                              int xSize,
                              int ySize,
                              int iRotate) const;

  float GetPageWidth() const { return m_PageWidth; }
  float GetPageHeight() const { return m_PageHeight; }
  CFX_FloatRect GetPageBBox() const { return m_BBox; }
  int GetPageRotation() const;
  CPDF_PageRenderCache* GetRenderCache() const { return m_pPageRender.get(); }

  CPDF_PageRenderContext* GetRenderContext() const {
    return m_pRenderContext.get();
  }
  void SetRenderContext(std::unique_ptr<CPDF_PageRenderContext> pContext);

  View* GetView() const { return m_pView; }
  void SetView(View* pView) { m_pView = pView; }

 private:
  void StartParse();

  CPDF_Object* GetPageAttr(const ByteString& name) const;
  CFX_FloatRect GetBox(const ByteString& name) const;

  float m_PageWidth;
  float m_PageHeight;
  CFX_Matrix m_PageMatrix;
  View* m_pView;
  std::unique_ptr<CPDF_PageRenderCache> m_pPageRender;
  std::unique_ptr<CPDF_PageRenderContext> m_pRenderContext;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PAGE_H_
