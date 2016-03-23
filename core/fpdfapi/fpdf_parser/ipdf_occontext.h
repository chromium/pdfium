// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_IPDF_OCCONTEXT_H_
#define CORE_FPDFAPI_FPDF_PARSER_IPDF_OCCONTEXT_H_

#include "core/fxcrt/include/fx_system.h"

class CPDF_Dictionary;
class CPDF_PageObject;

class IPDF_OCContext {
 public:
  virtual ~IPDF_OCContext();

  virtual FX_BOOL CheckOCGVisible(const CPDF_Dictionary* pOCG) = 0;
  FX_BOOL CheckObjectVisible(const CPDF_PageObject* pObj);
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_IPDF_OCCONTEXT_H_
