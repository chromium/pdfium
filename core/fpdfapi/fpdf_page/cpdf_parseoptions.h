// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_PARSEOPTIONS_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_PARSEOPTIONS_H_

#include "core/fxcrt/include/fx_system.h"

class CPDF_ParseOptions {
 public:
  CPDF_ParseOptions();

  FX_BOOL m_bTextOnly;
  FX_BOOL m_bMarkedContent;
  FX_BOOL m_bSeparateForm;
  FX_BOOL m_bDecodeInlineImage;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_PARSEOPTIONS_H_
