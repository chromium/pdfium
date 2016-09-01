// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_contentmark.h"

CPDF_ContentMark::CPDF_ContentMark() {}

CPDF_ContentMark::CPDF_ContentMark(const CPDF_ContentMark& that)
    : m_Ref(that.m_Ref) {}

CPDF_ContentMark::~CPDF_ContentMark() {}

void CPDF_ContentMark::SetNull() {
  m_Ref.SetNull();
}

int CPDF_ContentMark::CountItems() const {
  return m_Ref.GetObject()->CountItems();
}

const CPDF_ContentMarkItem& CPDF_ContentMark::GetItem(int i) const {
  return m_Ref.GetObject()->GetItem(i);
}

int CPDF_ContentMark::GetMCID() const {
  const CPDF_ContentMarkData* pData = m_Ref.GetObject();
  return pData ? pData->GetMCID() : -1;
}

void CPDF_ContentMark::AddMark(const CFX_ByteString& name,
                               CPDF_Dictionary* pDict,
                               FX_BOOL bDirect) {
  m_Ref.GetPrivateCopy()->AddMark(name, pDict, bDirect);
}

void CPDF_ContentMark::DeleteLastMark() {
  m_Ref.GetPrivateCopy()->DeleteLastMark();
  if (CountItems() == 0)
    m_Ref.SetNull();
}

bool CPDF_ContentMark::HasMark(const CFX_ByteStringC& mark) const {
  const CPDF_ContentMarkData* pData = m_Ref.GetObject();
  if (!pData)
    return false;

  for (int i = 0; i < pData->CountItems(); i++) {
    if (pData->GetItem(i).GetName() == mark)
      return true;
  }
  return false;
}

bool CPDF_ContentMark::LookupMark(const CFX_ByteStringC& mark,
                                  CPDF_Dictionary*& pDict) const {
  const CPDF_ContentMarkData* pData = m_Ref.GetObject();
  if (!pData)
    return false;

  for (int i = 0; i < pData->CountItems(); i++) {
    const CPDF_ContentMarkItem& item = pData->GetItem(i);
    if (item.GetName() == mark) {
      pDict = nullptr;
      if (item.GetParamType() == CPDF_ContentMarkItem::PropertiesDict ||
          item.GetParamType() == CPDF_ContentMarkItem::DirectDict) {
        pDict = item.GetParam();
      }
      return true;
    }
  }
  return false;
}
