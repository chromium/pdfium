// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_annotcontext.h"

#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "third_party/base/check.h"

CPDF_AnnotContext::CPDF_AnnotContext(CPDF_Dictionary* pAnnotDict,
                                     IPDF_Page* pPage)
    : m_pAnnotDict(pAnnotDict), m_pPage(pPage) {
  DCHECK(m_pAnnotDict);
  DCHECK(m_pPage);
  DCHECK(m_pPage->AsPDFPage());
}

CPDF_AnnotContext::~CPDF_AnnotContext() = default;

void CPDF_AnnotContext::SetForm(CPDF_Stream* pStream) {
  if (!pStream)
    return;

  // Reset the annotation matrix to be the identity matrix, since the
  // appearance stream already takes matrix into account.
  pStream->GetDict()->SetMatrixFor("Matrix", CFX_Matrix());

  m_pAnnotForm = std::make_unique<CPDF_Form>(
      m_pPage->GetDocument(), m_pPage->AsPDFPage()->GetResources(), pStream);
  m_pAnnotForm->ParseContent();
}
