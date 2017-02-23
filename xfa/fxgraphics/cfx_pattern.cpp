// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cfx_pattern.h"

CFX_Pattern::CFX_Pattern(FX_HatchStyle hatchStyle,
                         const FX_ARGB foreArgb,
                         const FX_ARGB backArgb,
                         CFX_Matrix* matrix)
    : m_hatchStyle(hatchStyle), m_foreArgb(foreArgb), m_backArgb(backArgb) {
  if (matrix)
    m_matrix = *matrix;
  else
    m_matrix.SetIdentity();
}

CFX_Pattern::~CFX_Pattern() {}
