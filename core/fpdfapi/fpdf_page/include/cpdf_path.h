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
  int GetPointCount() const { return GetObject()->GetPointCount(); }
  int GetFlag(int index) const { return GetObject()->GetFlag(index); }
  FX_FLOAT GetPointX(int index) const { return GetObject()->GetPointX(index); }
  FX_FLOAT GetPointY(int index) const { return GetObject()->GetPointY(index); }
  FX_PATHPOINT* GetPoints() const { return GetObject()->GetPoints(); }
  CFX_FloatRect GetBoundingBox() const { return GetObject()->GetBoundingBox(); }
  CFX_FloatRect GetBoundingBox(FX_FLOAT line_width,
                               FX_FLOAT miter_limit) const {
    return GetObject()->GetBoundingBox(line_width, miter_limit);
  }

  FX_BOOL IsRect() const { return GetObject()->IsRect(); }
  void Transform(const CFX_Matrix* pMatrix) {
    GetPrivateCopy()->Transform(pMatrix);
  }
  void Append(const CPDF_Path& other, const CFX_Matrix* pMatrix) {
    GetObject()->Append(other.GetObject(), pMatrix);
  }

  void AppendRect(FX_FLOAT left,
                  FX_FLOAT bottom,
                  FX_FLOAT right,
                  FX_FLOAT top) {
    GetObject()->AppendRect(left, bottom, right, top);
  }
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PATH_H_
