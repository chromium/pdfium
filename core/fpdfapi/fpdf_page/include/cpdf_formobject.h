// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_FORMOBJECT_H_
#define CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_FORMOBJECT_H_

#include "core/fpdfapi/fpdf_page/include/cpdf_pageobject.h"
#include "core/fxcrt/include/fx_coordinates.h"

class CPDF_Form;

class CPDF_FormObject : public CPDF_PageObject {
 public:
  CPDF_FormObject();
  ~CPDF_FormObject() override;

  // CPDF_PageObject:
  CPDF_FormObject* Clone() const override;
  Type GetType() const override { return FORM; };
  void Transform(const CFX_Matrix& matrix) override;
  bool IsForm() const override { return true; };
  CPDF_FormObject* AsForm() override { return this; };
  const CPDF_FormObject* AsForm() const override { return this; };

  void CalcBoundingBox();

  CPDF_Form* m_pForm;
  CFX_Matrix m_FormMatrix;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_FORMOBJECT_H_
