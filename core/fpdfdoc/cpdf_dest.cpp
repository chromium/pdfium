// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/include/cpdf_dest.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"

namespace {

const FX_CHAR* const g_sZoomModes[] = {"XYZ",  "Fit",   "FitH",  "FitV", "FitR",
                                       "FitB", "FitBH", "FitBV", nullptr};

}  // namespace

int CPDF_Dest::GetPageIndex(CPDF_Document* pDoc) {
  CPDF_Array* pArray = ToArray(m_pObj);
  if (!pArray)
    return 0;

  CPDF_Object* pPage = pArray->GetDirectObjectAt(0);
  if (!pPage)
    return 0;
  if (pPage->IsNumber())
    return pPage->GetInteger();
  if (!pPage->IsDictionary())
    return 0;
  return pDoc->GetPageIndex(pPage->GetObjNum());
}

uint32_t CPDF_Dest::GetPageObjNum() {
  CPDF_Array* pArray = ToArray(m_pObj);
  if (!pArray)
    return 0;

  CPDF_Object* pPage = pArray->GetDirectObjectAt(0);
  if (!pPage)
    return 0;
  if (pPage->IsNumber())
    return pPage->GetInteger();
  if (pPage->IsDictionary())
    return pPage->GetObjNum();
  return 0;
}

int CPDF_Dest::GetZoomMode() {
  CPDF_Array* pArray = ToArray(m_pObj);
  if (!pArray)
    return 0;

  CPDF_Object* pObj = pArray->GetDirectObjectAt(1);
  if (!pObj)
    return 0;

  CFX_ByteString mode = pObj->GetString();
  for (int i = 0; g_sZoomModes[i]; ++i) {
    if (mode == g_sZoomModes[i])
      return i + 1;
  }

  return 0;
}

FX_FLOAT CPDF_Dest::GetParam(int index) {
  CPDF_Array* pArray = ToArray(m_pObj);
  return pArray ? pArray->GetNumberAt(2 + index) : 0;
}

CFX_ByteString CPDF_Dest::GetRemoteName() {
  return m_pObj ? m_pObj->GetString() : CFX_ByteString();
}
