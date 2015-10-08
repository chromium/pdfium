// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_SymbolDict.h"

#include "../../../include/fxcrt/fx_memory.h"
#include "JBig2_Image.h"

CJBig2_SymbolDict::CJBig2_SymbolDict() {
  m_bContextRetained = FALSE;
  m_gbContext = m_grContext = NULL;
}

CJBig2_SymbolDict::~CJBig2_SymbolDict() {
  if (m_bContextRetained) {
    FX_Free(m_gbContext);
    FX_Free(m_grContext);
  }
}

nonstd::unique_ptr<CJBig2_SymbolDict> CJBig2_SymbolDict::DeepCopy() const {
  nonstd::unique_ptr<CJBig2_SymbolDict> dst;
  const CJBig2_SymbolDict* src = this;
  if (src->m_bContextRetained || src->m_gbContext || src->m_grContext)
    return dst;

  dst.reset(new CJBig2_SymbolDict);
  for (size_t i = 0; i < src->m_SDEXSYMS.size(); ++i) {
    CJBig2_Image* image = src->m_SDEXSYMS.get(i);
    dst->m_SDEXSYMS.push_back(image ? new CJBig2_Image(*image) : nullptr);
  }
  return dst;
}
