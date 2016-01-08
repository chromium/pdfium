// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_pictureboximp.h"

// static
IFWL_PictureBox* IFWL_PictureBox::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_PictureBox* pPictureBox = new IFWL_PictureBox;
  CFWL_PictureBoxImp* pPictureBoxImpl =
      new CFWL_PictureBoxImp(properties, pOuter);
  pPictureBox->SetImpl(pPictureBoxImpl);
  pPictureBoxImpl->SetInterface(pPictureBox);
  return pPictureBox;
}
IFWL_PictureBox::IFWL_PictureBox() {}

CFWL_PictureBoxImp::CFWL_PictureBoxImp(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_bTop(FALSE),
      m_bVCenter(FALSE),
      m_bButton(FALSE) {
  m_rtClient.Reset();
  m_rtImage.Reset();
  m_matrix.SetIdentity();
}
CFWL_PictureBoxImp::~CFWL_PictureBoxImp() {}
FWL_ERR CFWL_PictureBoxImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_PictureBox;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_PictureBoxImp::GetClassID() const {
  return FWL_CLASSHASH_PictureBox;
}
FWL_ERR CFWL_PictureBoxImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_PictureBoxImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBoxImp::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_PictureBoxImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.Set(0, 0, 0, 0);
    if (!m_pProperties->m_pDataProvider)
      return FWL_ERR_Indefinite;
    CFX_DIBitmap* pBitmap =
        static_cast<IFWL_PictureBoxDP*>(m_pProperties->m_pDataProvider)
            ->GetPicture(m_pInterface);
    if (pBitmap) {
      rect.Set(0, 0, (FX_FLOAT)pBitmap->GetWidth(),
               (FX_FLOAT)pBitmap->GetHeight());
    }
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBoxImp::Update() {
  if (IsLocked()) {
    return FWL_ERR_Succeeded;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  GetClientRect(m_rtClient);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBoxImp::DrawWidget(CFX_Graphics* pGraphics,
                                       const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  IFWL_ThemeProvider* pTheme = GetAvailableTheme();
  if (HasBorder()) {
    DrawBorder(pGraphics, FWL_PART_PTB_Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, FWL_PART_PTB_Edge, pTheme, pMatrix);
  }
  DrawBkground(pGraphics, pTheme, pMatrix);
  return FWL_ERR_Succeeded;
}
void CFWL_PictureBoxImp::DrawBkground(CFX_Graphics* pGraphics,
                                      IFWL_ThemeProvider* pTheme,
                                      const CFX_Matrix* pMatrix) {
  IFWL_PictureBoxDP* pPictureDP =
      static_cast<IFWL_PictureBoxDP*>(m_pProperties->m_pDataProvider);
  if (!pPictureDP)
    return;

  CFX_DIBitmap* pPicture = pPictureDP->GetPicture(m_pInterface);
  CFX_Matrix matrix;
  pPictureDP->GetMatrix(m_pInterface, matrix);
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
  CFX_PointF pt;
  pt.Set((m_rtClient.width - fx) / 2, (m_rtClient.height - fy) / 2);
  pGraphics->DrawImage(pPicture, pt, &matrix);
}
FX_BOOL CFWL_PictureBoxImp::VStyle(FX_BOOL dwStyle) {
  switch (dwStyle & FWL_STYLEEXT_PTB_VAlignMask) {
    case FWL_STYLEEXT_PTB_Top: {
      return m_bTop = TRUE;
      break;
    }
    case FWL_STYLEEXT_PTB_Vcenter: {
      return m_bVCenter = TRUE;
      break;
    }
    case FWL_STYLEEXT_PTB_Bottom: {
      return m_bButton = TRUE;
      break;
    }
  }
  return FALSE;
}
CFWL_PictureBoxImpDelegate::CFWL_PictureBoxImpDelegate(
    CFWL_PictureBoxImp* pOwner)
    : m_pOwner(pOwner) {}
FWL_ERR CFWL_PictureBoxImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                                 const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
