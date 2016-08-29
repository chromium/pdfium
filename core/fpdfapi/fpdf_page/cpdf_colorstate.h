// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_

#include "core/fpdfapi/fpdf_page/cpdf_colorstatedata.h"
#include "core/fxcrt/include/cfx_count_ref.h"

class CPDF_ColorState : public CFX_CountRef<CPDF_ColorStateData> {
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_
