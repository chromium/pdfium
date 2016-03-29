// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_pictureboxtp.h"

#include "xfa/fwl/basewidget/ifwl_picturebox.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/ifwl_widget.h"

CFWL_PictureBoxTP::CFWL_PictureBoxTP() {}

CFWL_PictureBoxTP::~CFWL_PictureBoxTP() {}

FX_BOOL CFWL_PictureBoxTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_PictureBox;
}
FX_BOOL CFWL_PictureBoxTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (!pParams)
    return FALSE;
  switch (pParams->m_iPart) {
    case FWL_PART_PTB_Border: {
      DrawBorder(pParams->m_pGraphics, &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_PTB_Edge: {
      DrawEdge(pParams->m_pGraphics, pParams->m_pWidget->GetStyles(),
               &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    default: {}
  }
  return TRUE;
}
