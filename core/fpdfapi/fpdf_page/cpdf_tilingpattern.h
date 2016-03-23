// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_TILINGPATTERN_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_TILINGPATTERN_H_

#include "core/fpdfapi/fpdf_page/cpdf_pattern.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Document;
class CPDF_Form;
class CPDF_Object;

class CPDF_TilingPattern : public CPDF_Pattern {
 public:
  CPDF_TilingPattern(CPDF_Document* pDoc,
                     CPDF_Object* pPatternObj,
                     const CFX_Matrix* parentMatrix);
  ~CPDF_TilingPattern() override;

  FX_BOOL Load();

  FX_BOOL m_bColored;
  CFX_FloatRect m_BBox;
  FX_FLOAT m_XStep;
  FX_FLOAT m_YStep;
  CPDF_Form* m_pForm;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_TILINGPATTERN_H_
