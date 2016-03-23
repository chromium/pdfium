// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PAGE_H_
#define CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PAGE_H_

#include "core/fpdfapi/fpdf_page/include/cpdf_pageobjectholder.h"
#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Document;
class CPDF_Dictionary;
class CPDF_Object;
class CPDF_PageRenderCache;
class CPDF_ParseOptions;

CPDF_Object* FPDFAPI_GetPageAttr(CPDF_Dictionary* pPageDict,
                                 const CFX_ByteStringC& name);

class CPDF_Page : public CPDF_PageObjectHolder, public CFX_PrivateData {
 public:
  CPDF_Page();
  ~CPDF_Page();

  void Load(CPDF_Document* pDocument,
            CPDF_Dictionary* pPageDict,
            FX_BOOL bPageCache = TRUE);

  void ParseContent(CPDF_ParseOptions* pOptions);

  void GetDisplayMatrix(CFX_Matrix& matrix,
                        int xPos,
                        int yPos,
                        int xSize,
                        int ySize,
                        int iRotate) const;

  FX_FLOAT GetPageWidth() const { return m_PageWidth; }
  FX_FLOAT GetPageHeight() const { return m_PageHeight; }
  CFX_FloatRect GetPageBBox() const { return m_BBox; }
  const CFX_Matrix& GetPageMatrix() const { return m_PageMatrix; }
  CPDF_Object* GetPageAttr(const CFX_ByteStringC& name) const;
  CPDF_PageRenderCache* GetRenderCache() const { return m_pPageRender; }

 protected:
  friend class CPDF_ContentParser;

  void StartParse(CPDF_ParseOptions* pOptions);

  FX_FLOAT m_PageWidth;
  FX_FLOAT m_PageHeight;
  CFX_Matrix m_PageMatrix;
  CPDF_PageRenderCache* m_pPageRender;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PAGE_H_
