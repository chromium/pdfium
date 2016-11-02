// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_picturebox.h"

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/cfwl_picturebox.h"
#include "xfa/fwl/core/fwl_noteimp.h"

IFWL_PictureBox::IFWL_PictureBox(const IFWL_App* app,
                                 const CFWL_WidgetImpProperties& properties)
    : IFWL_Widget(app, properties, nullptr),
      m_bTop(FALSE),
      m_bVCenter(FALSE),
      m_bButton(FALSE) {
  m_rtClient.Reset();
  m_rtImage.Reset();
  m_matrix.SetIdentity();
}

IFWL_PictureBox::~IFWL_PictureBox() {}

FWL_Type IFWL_PictureBox::GetClassID() const {
  return FWL_Type::PictureBox;
}

FWL_Error IFWL_PictureBox::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, 0, 0);
    if (!m_pProperties->m_pDataProvider)
      return FWL_Error::Indefinite;
    CFX_DIBitmap* pBitmap =
        static_cast<IFWL_PictureBoxDP*>(m_pProperties->m_pDataProvider)
            ->GetPicture(this);
    if (pBitmap) {
      rect.Set(0, 0, (FX_FLOAT)pBitmap->GetWidth(),
               (FX_FLOAT)pBitmap->GetHeight());
    }
    IFWL_Widget::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_PictureBox::Update() {
  if (IsLocked()) {
    return FWL_Error::Succeeded;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  GetClientRect(m_rtClient);
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_PictureBox::DrawWidget(CFX_Graphics* pGraphics,
                                      const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_Error::Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_Error::Indefinite;
  IFWL_ThemeProvider* pTheme = GetAvailableTheme();
  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, CFWL_Part::Edge, pTheme, pMatrix);
  }
  DrawBkground(pGraphics, pTheme, pMatrix);
  return FWL_Error::Succeeded;
}

void IFWL_PictureBox::DrawBkground(CFX_Graphics* pGraphics,
                                   IFWL_ThemeProvider* pTheme,
                                   const CFX_Matrix* pMatrix) {
  IFWL_PictureBoxDP* pPictureDP =
      static_cast<IFWL_PictureBoxDP*>(m_pProperties->m_pDataProvider);
  if (!pPictureDP)
    return;

  CFX_DIBitmap* pPicture = pPictureDP->GetPicture(this);
  CFX_Matrix matrix;
  pPictureDP->GetMatrix(this, matrix);
  if (!pPicture)
    return;

  matrix.Concat(*pMatrix);
  FX_FLOAT fx = (FX_FLOAT)pPicture->GetWidth();
  FX_FLOAT fy = (FX_FLOAT)pPicture->GetHeight();
  if (fx > m_rtClient.width) {
    fx = m_rtClient.width;
  }
  if (fy > m_rtClient.height) {
    fy = m_rtClient.height;
  }
  pGraphics->DrawImage(pPicture, CFX_PointF((m_rtClient.width - fx) / 2,
                                            (m_rtClient.height - fy) / 2),
                       &matrix);
}

bool IFWL_PictureBox::VStyle(uint32_t dwStyle) {
  switch (dwStyle & FWL_STYLEEXT_PTB_VAlignMask) {
    case FWL_STYLEEXT_PTB_Top:
      m_bTop = TRUE;
      return true;
    case FWL_STYLEEXT_PTB_Vcenter:
      m_bVCenter = TRUE;
      return true;
    case FWL_STYLEEXT_PTB_Bottom:
      m_bButton = TRUE;
      return true;
  }
  return false;
}

void IFWL_PictureBox::OnDrawWidget(CFX_Graphics* pGraphics,
                                   const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}
