// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_type3char.h"

#include <utility>

#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fxge/fx_dib.h"

CPDF_Type3Char::CPDF_Type3Char(std::unique_ptr<CPDF_Form> pForm)
    : m_pForm(std::move(pForm)), m_bColored(false) {}

CPDF_Type3Char::~CPDF_Type3Char() {}

bool CPDF_Type3Char::LoadBitmap(CPDF_RenderContext* pContext) {
  if (m_pBitmap || !m_pForm)
    return true;

  if (m_pForm->GetPageObjectList()->size() != 1 || m_bColored)
    return false;

  auto& pPageObj = m_pForm->GetPageObjectList()->front();
  if (!pPageObj->IsImage())
    return false;

  m_ImageMatrix = pPageObj->AsImage()->matrix();
  {
    // |pSource| actually gets assigned a CPDF_DIBSource, which has pointers
    // into objects owned by |m_pForm|. Make sure it is out of scope before
    // clearing the form.
    CFX_RetainPtr<CFX_DIBSource> pSource =
        pPageObj->AsImage()->GetImage()->LoadDIBSource();

    // Clone() is non-virtual, and can't be overloaded by CPDF_DIBSource to
    // return a clone of the subclass as one would typically expect from a
    // such a method. Instead, it only clones the CFX_DIBSource, none of whose
    // members point to objects owned by the form. As a result, |m_pBitmap|
    // may outlive |m_pForm|.
    if (pSource)
      m_pBitmap = pSource->Clone(nullptr);
  }
  m_pForm.reset();
  return true;
}
