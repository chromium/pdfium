// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfdoc/include/fpdf_doc.h"

CPDF_ViewerPreferences::CPDF_ViewerPreferences(CPDF_Document* pDoc)
    : m_pDoc(pDoc) {}
CPDF_ViewerPreferences::~CPDF_ViewerPreferences() {}
FX_BOOL CPDF_ViewerPreferences::IsDirectionR2L() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDictBy("ViewerPreferences");
  if (!pDict) {
    return FALSE;
  }
  return "R2L" == pDict->GetStringBy("Direction");
}
FX_BOOL CPDF_ViewerPreferences::PrintScaling() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDictBy("ViewerPreferences");
  if (!pDict) {
    return TRUE;
  }
  return "None" != pDict->GetStringBy("PrintScaling");
}
int32_t CPDF_ViewerPreferences::NumCopies() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDictBy("ViewerPreferences");
  if (!pDict) {
    return 1;
  }
  return pDict->GetIntegerBy("NumCopies");
}
CPDF_Array* CPDF_ViewerPreferences::PrintPageRange() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  CPDF_Array* pRange = NULL;
  pDict = pDict->GetDictBy("ViewerPreferences");
  if (!pDict) {
    return pRange;
  }
  pRange = pDict->GetArrayBy("PrintPageRange");
  return pRange;
}
CFX_ByteString CPDF_ViewerPreferences::Duplex() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDictBy("ViewerPreferences");
  if (!pDict) {
    return "None";
  }
  return pDict->GetStringBy("Duplex");
}
