// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_PATHDATA_H_
#define FPDFSDK_PDFWINDOW_CPWL_PATHDATA_H_

#include "core/fxcrt/fx_coordinates.h"

enum PWL_PATHDATA_TYPE {
  PWLPT_MOVETO,
  PWLPT_LINETO,
  PWLPT_BEZIERTO,
  PWLPT_UNKNOWN
};

class CPWL_PathData {
 public:
  CPWL_PathData();
  CPWL_PathData(const CFX_PointF& pt, PWL_PATHDATA_TYPE tp);
  CPWL_PathData(const CPWL_PathData&);
  ~CPWL_PathData();

  CFX_PointF point;
  PWL_PATHDATA_TYPE type;
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_PATHDATA_H_
