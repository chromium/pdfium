// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfdoc/fpdf_doc.h"

CPDF_ViewerPreferences::CPDF_ViewerPreferences(CPDF_Document* pDoc)
    : m_pDoc(pDoc) {}
CPDF_ViewerPreferences::~CPDF_ViewerPreferences() {}
FX_BOOL CPDF_ViewerPreferences::IsDirectionR2L() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDict("ViewerPreferences");
  if (!pDict) {
    return FALSE;
  }
  return "R2L" == pDict->GetString("Direction");
}
FX_BOOL CPDF_ViewerPreferences::PrintScaling() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDict("ViewerPreferences");
  if (!pDict) {
    return TRUE;
  }
  return "None" != pDict->GetString("PrintScaling");
}
int32_t CPDF_ViewerPreferences::NumCopies() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDict("ViewerPreferences");
  if (!pDict) {
    return 1;
  }
  return pDict->GetInteger("NumCopies");
}
CPDF_Array* CPDF_ViewerPreferences::PrintPageRange() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  CPDF_Array* pRange = NULL;
  pDict = pDict->GetDict("ViewerPreferences");
  if (!pDict) {
    return pRange;
  }
  pRange = pDict->GetArray("PrintPageRange");
  return pRange;
}
CFX_ByteString CPDF_ViewerPreferences::Duplex() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  pDict = pDict->GetDict("ViewerPreferences");
  if (!pDict) {
    return "None";
  }
  return pDict->GetString("Duplex");
}
