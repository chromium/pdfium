// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/include/cpdf_viewerpreferences.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"

CPDF_ViewerPreferences::CPDF_ViewerPreferences(CPDF_Document* pDoc)
    : m_pDoc(pDoc) {}

CPDF_ViewerPreferences::~CPDF_ViewerPreferences() {}

FX_BOOL CPDF_ViewerPreferences::IsDirectionR2L() const {
  CPDF_Dictionary* pDict = GetViewerPreferences();
  return pDict ? pDict->GetStringFor("Direction") == "R2L" : FALSE;
}

FX_BOOL CPDF_ViewerPreferences::PrintScaling() const {
  CPDF_Dictionary* pDict = GetViewerPreferences();
  return pDict ? pDict->GetStringFor("PrintScaling") != "None" : TRUE;
}

int32_t CPDF_ViewerPreferences::NumCopies() const {
  CPDF_Dictionary* pDict = GetViewerPreferences();
  return pDict ? pDict->GetIntegerFor("NumCopies") : 1;
}

CPDF_Array* CPDF_ViewerPreferences::PrintPageRange() const {
  CPDF_Dictionary* pDict = GetViewerPreferences();
  return pDict ? pDict->GetArrayFor("PrintPageRange") : nullptr;
}

CFX_ByteString CPDF_ViewerPreferences::Duplex() const {
  CPDF_Dictionary* pDict = GetViewerPreferences();
  return pDict ? pDict->GetStringFor("Duplex") : CFX_ByteString("None");
}

CPDF_Dictionary* CPDF_ViewerPreferences::GetViewerPreferences() const {
  CPDF_Dictionary* pDict = m_pDoc->GetRoot();
  return pDict ? pDict->GetDictFor("ViewerPreferences") : nullptr;
}
