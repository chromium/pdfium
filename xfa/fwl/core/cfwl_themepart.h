// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_THEMEPART_H_
#define XFA_FWL_CORE_CFWL_THEMEPART_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/ifwl_widget.h"

class CFWL_ThemePart {
 public:
  CFWL_ThemePart()
      : m_pWidget(NULL), m_iPart(0), m_dwStates(0), m_dwData(0), m_pData(NULL) {
    m_rtPart.Reset();
    m_matrix.SetIdentity();
  }

  CFX_Matrix m_matrix;
  CFX_RectF m_rtPart;
  IFWL_Widget* m_pWidget;
  int32_t m_iPart;
  uint32_t m_dwStates;
  uint32_t m_dwData;
  void* m_pData;
};

#endif  // XFA_FWL_CORE_CFWL_THEMEPART_H_
