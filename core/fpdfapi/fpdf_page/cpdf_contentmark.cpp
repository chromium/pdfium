// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_contentmark.h"

bool CPDF_ContentMark::HasMark(const CFX_ByteStringC& mark) const {
  const CPDF_ContentMarkData* pData = GetObject();
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
  const CPDF_ContentMarkData* pData = GetObject();
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
