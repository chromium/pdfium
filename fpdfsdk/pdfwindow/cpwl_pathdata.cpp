// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_pathdata.h"

CPWL_PathData::CPWL_PathData() : point(), type(PWLPT_UNKNOWN) {}

CPWL_PathData::CPWL_PathData(const CFX_PointF& pt, PWL_PATHDATA_TYPE tp)
    : point(pt), type(tp) {}

CPWL_PathData::CPWL_PathData(const CPWL_PathData&) = default;

CPWL_PathData::~CPWL_PathData() = default;
