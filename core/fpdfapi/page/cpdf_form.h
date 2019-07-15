// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_FORM_H_
#define CORE_FPDFAPI_PAGE_CPDF_FORM_H_

#include <memory>
#include <set>

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"

class CFX_Matrix;
class CPDF_AllStates;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_ImageObject;
class CPDF_Stream;
class CPDF_Type3Char;

class CPDF_Form final : public CPDF_PageObjectHolder {
 public:
  // Helper method to choose the first non-null resources dictionary.
  static CPDF_Dictionary* ChooseResourcesDict(CPDF_Dictionary* pResources,
                                              CPDF_Dictionary* pParentResources,
                                              CPDF_Dictionary* pPageResources);

  CPDF_Form(CPDF_Document* pDocument,
            CPDF_Dictionary* pPageResources,
            CPDF_Stream* pFormStream);
  CPDF_Form(CPDF_Document* pDocument,
            CPDF_Dictionary* pPageResources,
            CPDF_Stream* pFormStream,
            CPDF_Dictionary* pParentResources);
  ~CPDF_Form() override;

  void ParseContent();
  void ParseContent(const CPDF_AllStates* pGraphicStates,
                    const CFX_Matrix* pParentMatrix,
                    std::set<const uint8_t*>* pParsedSet);
  void ParseContentForType3Char(CPDF_Type3Char* pType3Char);

  CFX_FloatRect CalcBoundingBox() const;
  const CPDF_Stream* GetStream() const;

  // Fast path helper for avoiding a full rendering of form.
  const CPDF_ImageObject* GetSoleImageOfForm() const;

 private:
  void ParseContentInternal(const CPDF_AllStates* pGraphicStates,
                            const CFX_Matrix* pParentMatrix,
                            CPDF_Type3Char* pType3Char,
                            std::set<const uint8_t*>* pParsedSet);

  std::unique_ptr<std::set<const uint8_t*>> m_ParsedSet;
  RetainPtr<CPDF_Stream> const m_pFormStream;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_FORM_H_
