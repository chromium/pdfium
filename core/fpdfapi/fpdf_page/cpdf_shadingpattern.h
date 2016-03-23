// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_SHADINGPATTERN_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_SHADINGPATTERN_H_

#include "core/fpdfapi/fpdf_page/cpdf_countedobject.h"
#include "core/fpdfapi/fpdf_page/cpdf_pattern.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fxcrt/include/fx_system.h"

typedef enum {
  kInvalidShading = 0,
  kFunctionBasedShading = 1,
  kAxialShading = 2,
  kRadialShading = 3,
  kFreeFormGouraudTriangleMeshShading = 4,
  kLatticeFormGouraudTriangleMeshShading = 5,
  kCoonsPatchMeshShading = 6,
  kTensorProductPatchMeshShading = 7,
  kMaxShading = 8
} ShadingType;

class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_Document;
class CPDF_Object;

class CPDF_ShadingPattern : public CPDF_Pattern {
 public:
  CPDF_ShadingPattern(CPDF_Document* pDoc,
                      CPDF_Object* pPatternObj,
                      FX_BOOL bShading,
                      const CFX_Matrix* parentMatrix);

  ~CPDF_ShadingPattern() override;

  bool IsMeshShading() const {
    return m_ShadingType == kFreeFormGouraudTriangleMeshShading ||
           m_ShadingType == kLatticeFormGouraudTriangleMeshShading ||
           m_ShadingType == kCoonsPatchMeshShading ||
           m_ShadingType == kTensorProductPatchMeshShading;
  }
  FX_BOOL Load();

  ShadingType m_ShadingType;
  FX_BOOL m_bShadingObj;
  CPDF_Object* m_pShadingObj;

  // Still keep |m_pCS| as some CPDF_ColorSpace (name object) are not managed
  // as counted objects. Refer to CPDF_DocPageData::GetColorSpace.
  CPDF_ColorSpace* m_pCS;

  CPDF_CountedColorSpace* m_pCountedCS;
  CPDF_Function* m_pFunctions[4];
  int m_nFuncs;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_SHADINGPATTERN_H_
