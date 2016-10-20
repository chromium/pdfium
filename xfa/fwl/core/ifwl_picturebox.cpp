// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_picturebox.h"

#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/lightwidget/cfwl_picturebox.h"

// static
IFWL_PictureBox* IFWL_PictureBox::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  return new IFWL_PictureBox(properties, pOuter);
}

IFWL_PictureBox::IFWL_PictureBox(const CFWL_WidgetImpProperties& properties,
                                 IFWL_Widget* pOuter)
    : IFWL_Widget(properties, pOuter),
      m_bTop(FALSE),
      m_bVCenter(FALSE),
      m_bButton(FALSE) {
  m_rtClient.Reset();
  m_rtImage.Reset();
  m_matrix.SetIdentity();
}

IFWL_PictureBox::~IFWL_PictureBox() {}

FWL_Error IFWL_PictureBox::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_PictureBox;
  return FWL_Error::Succeeded;
}

FWL_Type IFWL_PictureBox::GetClassID() const {
  return FWL_Type::PictureBox;
}

FWL_Error IFWL_PictureBox::Initialize() {
  if (IFWL_Widget::Initialize() != FWL_Error::Succeeded)
    return FWL_Error::Indefinite;

  m_pDelegate = new CFWL_PictureBoxImpDelegate(this);
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_PictureBox::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return IFWL_Widget::Finalize();
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

FX_BOOL IFWL_PictureBox::VStyle(FX_BOOL dwStyle) {
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

CFWL_PictureBoxImpDelegate::CFWL_PictureBoxImpDelegate(IFWL_PictureBox* pOwner)
    : m_pOwner(pOwner) {}

void CFWL_PictureBoxImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                              const CFX_Matrix* pMatrix) {
  m_pOwner->DrawWidget(pGraphics, pMatrix);
}
