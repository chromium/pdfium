// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_SHADINGOBJECT_H_
#define CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_SHADINGOBJECT_H_

#include "core/fpdfapi/fpdf_page/include/cpdf_pageobject.h"
#include "core/fxcrt/include/fx_coordinates.h"

class CPDF_ShadingPattern;

class CPDF_ShadingObject : public CPDF_PageObject {
 public:
  CPDF_ShadingObject();
  ~CPDF_ShadingObject() override;

  // CPDF_PageObject:
  CPDF_ShadingObject* Clone() const override;
  Type GetType() const override { return SHADING; };
  void Transform(const CFX_Matrix& matrix) override;
  bool IsShading() const override { return true; };
  CPDF_ShadingObject* AsShading() override { return this; };
  const CPDF_ShadingObject* AsShading() const override { return this; };

  void CalcBoundingBox();

  CPDF_ShadingPattern* m_pShading;
  CFX_Matrix m_Matrix;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_SHADINGOBJECT_H_
