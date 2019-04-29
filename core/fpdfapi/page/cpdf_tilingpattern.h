// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_TILINGPATTERN_H_
#define CORE_FPDFAPI_PAGE_CPDF_TILINGPATTERN_H_

#include <memory>

#include "core/fpdfapi/page/cpdf_pattern.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"

class CPDF_Document;
class CPDF_Form;
class CPDF_Object;
class CPDF_PageObject;

class CPDF_TilingPattern final : public CPDF_Pattern {
 public:
  class Unloader {
   public:
    explicit Unloader(CPDF_TilingPattern* pTilingPattern);
    ~Unloader();

   private:
    UnownedPtr<CPDF_TilingPattern> const m_pTilingPattern;
  };

  CPDF_TilingPattern(CPDF_Document* pDoc,
                     CPDF_Object* pPatternObj,
                     const CFX_Matrix& parentMatrix);
  ~CPDF_TilingPattern() override;

  CPDF_TilingPattern* AsTilingPattern() override;
  CPDF_ShadingPattern* AsShadingPattern() override;

  bool Load(CPDF_PageObject* pPageObj);
  void Unload();

  bool colored() const { return m_bColored; }
  const CFX_FloatRect& bbox() const { return m_BBox; }
  float x_step() const { return m_XStep; }
  float y_step() const { return m_YStep; }
  CPDF_Form* form() const { return m_pForm.get(); }

 private:
  bool m_bColored;
  CFX_FloatRect m_BBox;
  float m_XStep;
  float m_YStep;
  std::unique_ptr<CPDF_Form> m_pForm;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_TILINGPATTERN_H_
