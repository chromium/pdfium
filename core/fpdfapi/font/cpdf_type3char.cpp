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
    : m_pForm(std::move(pForm)) {}

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
    RetainPtr<CFX_DIBSource> pSource =
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

void CPDF_Type3Char::InitializeFromStreamData(bool bColored,
                                              const float* pData) {
  m_bColored = bColored;
  m_Width = FXSYS_round(pData[0] * 1000);
  m_BBox.left = FXSYS_round(pData[2] * 1000);
  m_BBox.bottom = FXSYS_round(pData[3] * 1000);
  m_BBox.right = FXSYS_round(pData[4] * 1000);
  m_BBox.top = FXSYS_round(pData[5] * 1000);
}

void CPDF_Type3Char::Transform(const CFX_Matrix& matrix) {
  m_Width = m_Width * matrix.GetXUnit() + 0.5f;
  CFX_FloatRect char_rect(static_cast<float>(m_BBox.left) / 1000.0f,
                          static_cast<float>(m_BBox.bottom) / 1000.0f,
                          static_cast<float>(m_BBox.right) / 1000.0f,
                          static_cast<float>(m_BBox.top) / 1000.0f);
  if (m_BBox.right <= m_BBox.left || m_BBox.bottom >= m_BBox.top)
    char_rect = form()->CalcBoundingBox();

  char_rect = matrix.TransformRect(char_rect);
  m_BBox.left = FXSYS_round(char_rect.left * 1000);
  m_BBox.right = FXSYS_round(char_rect.right * 1000);
  m_BBox.top = FXSYS_round(char_rect.top * 1000);
  m_BBox.bottom = FXSYS_round(char_rect.bottom * 1000);
}

void CPDF_Type3Char::ResetForm() {
  m_pForm.reset();
}
