// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PATH_H_
#define CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PATH_H_

#include "core/fxcrt/include/fx_system.h"
#include "core/fxge/include/cfx_fxgedevice.h"
#include "core/fxge/include/cfx_pathdata.h"
#include "core/fxge/include/cfx_renderdevice.h"

class CPDF_Path : public CFX_CountRef<CFX_PathData> {
 public:
  void Transform(const CFX_Matrix* pMatrix) {
    MakePrivateCopy();
    GetObject()->Transform(pMatrix);
  }
  void Append(const CPDF_Path& other, const CFX_Matrix* pMatrix) {
    GetObject()->Append(other.GetObject(), pMatrix);
  }
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PATH_H_
