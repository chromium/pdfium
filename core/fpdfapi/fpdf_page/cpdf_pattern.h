// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_PATTERN_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_PATTERN_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Document;
class CPDF_Object;

class CPDF_Pattern {
 public:
  enum PatternType { TILING = 1, SHADING };

  virtual ~CPDF_Pattern();

  void SetForceClear(FX_BOOL bForceClear) { m_bForceClear = bForceClear; }

  const PatternType m_PatternType;
  CPDF_Document* const m_pDocument;
  CPDF_Object* const m_pPatternObj;
  CFX_Matrix m_Pattern2Form;
  CFX_Matrix m_ParentMatrix;

 protected:
  CPDF_Pattern(PatternType type,
               CPDF_Document* pDoc,
               CPDF_Object* pObj,
               const CFX_Matrix* pParentMatrix);

  FX_BOOL m_bForceClear;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_PATTERN_H_
