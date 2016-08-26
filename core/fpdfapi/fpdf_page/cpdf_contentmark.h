// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_CONTENTMARK_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_CONTENTMARK_H_

#include "core/fpdfapi/fpdf_page/cpdf_contentmarkdata.h"
#include "core/fxcrt/include/cfx_count_ref.h"
#include "core/fxcrt/include/fx_basic.h"

class CPDF_ContentMark : public CFX_CountRef<CPDF_ContentMarkData> {
 public:
  int GetMCID() const {
    const CPDF_ContentMarkData* pData = GetObject();
    return pData ? pData->GetMCID() : -1;
  }

  bool HasMark(const CFX_ByteStringC& mark) const;
  bool LookupMark(const CFX_ByteStringC& mark, CPDF_Dictionary*& pDict) const;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_CONTENTMARK_H_
