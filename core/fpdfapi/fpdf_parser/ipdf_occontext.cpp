// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/ipdf_occontext.h"

#include "core/fpdfapi/fpdf_page/cpdf_contentmarkitem.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_pageobject.h"

IPDF_OCContext::~IPDF_OCContext() {}

FX_BOOL IPDF_OCContext::CheckObjectVisible(const CPDF_PageObject* pObj) {
  const CPDF_ContentMarkData* pData = pObj->m_ContentMark;
  int nItems = pData->CountItems();
  for (int i = 0; i < nItems; i++) {
    const CPDF_ContentMarkItem& item = pData->GetItem(i);
    if (item.GetName() == "OC" &&
        item.GetParamType() == CPDF_ContentMarkItem::PropertiesDict &&
        !CheckOCGVisible(item.GetParam())) {
      return FALSE;
    }
  }
  return TRUE;
}
