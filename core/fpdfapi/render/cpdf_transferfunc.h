// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_TRANSFERFUNC_H_
#define CORE_FPDFAPI_RENDER_CPDF_TRANSFERFUNC_H_

#include "core/fxge/fx_dib.h"

class CPDF_Document;

class CPDF_TransferFunc {
 public:
  explicit CPDF_TransferFunc(CPDF_Document* pDoc);

  FX_COLORREF TranslateColor(FX_COLORREF src) const;
  CFX_RetainPtr<CFX_DIBSource> TranslateImage(
      const CFX_RetainPtr<CFX_DIBSource>& pSrc);

  CPDF_Document* const m_pPDFDoc;
  bool m_bIdentity;
  uint8_t m_Samples[256 * 3];
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_TRANSFERFUNC_H_
