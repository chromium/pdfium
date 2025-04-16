// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_annotcontext.h"

#include <utility>

#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/check.h"

CPDF_AnnotContext::CPDF_AnnotContext(RetainPtr<CPDF_Dictionary> pAnnotDict,
                                     IPDF_Page* pPage)
    : annot_dict_(std::move(pAnnotDict)), page_(pPage) {
  DCHECK(annot_dict_);
  DCHECK(page_);
  DCHECK(page_->AsPDFPage());
}

CPDF_AnnotContext::~CPDF_AnnotContext() = default;

void CPDF_AnnotContext::SetForm(RetainPtr<CPDF_Stream> pStream) {
  if (!pStream) {
    return;
  }

  // Reset the annotation matrix to be the identity matrix, since the
  // appearance stream already takes matrix into account.
  pStream->GetMutableDict()->SetMatrixFor("Matrix", CFX_Matrix());

  annot_form_ = std::make_unique<CPDF_Form>(
      page_->GetDocument(), page_->AsPDFPage()->GetMutableResources(), pStream);
  annot_form_->ParseContent();
}
