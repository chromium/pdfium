// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_ScrollBar* CFWL_ScrollBar::Create() {
  return new CFWL_ScrollBar;
}
FWL_ERR CFWL_ScrollBar::Initialize(const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_ERR_Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  m_pIface = new IFWL_ScrollBar;
  FWL_ERR ret =
      ((IFWL_ScrollBar*)m_pIface)
          ->Initialize(m_pProperties->MakeWidgetImpProperties(nullptr),
                       nullptr);
  if (ret == FWL_ERR_Succeeded) {
    CFWL_Widget::Initialize();
  }
  return ret;
}
FX_BOOL CFWL_ScrollBar::IsVertical() {
  if (!m_pIface)
    return FALSE;
  return ((IFWL_ScrollBar*)m_pIface)->IsVertical();
}
FWL_ERR CFWL_ScrollBar::GetRange(FX_FLOAT& fMin, FX_FLOAT& fMax) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return ((IFWL_ScrollBar*)m_pIface)->GetRange(fMin, fMax);
}
FWL_ERR CFWL_ScrollBar::SetRange(FX_FLOAT fMin, FX_FLOAT fMax) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return ((IFWL_ScrollBar*)m_pIface)->SetRange(fMin, fMax);
}
FX_FLOAT CFWL_ScrollBar::GetPageSize() {
  if (!m_pIface)
    return 0;
  return ((IFWL_ScrollBar*)m_pIface)->GetPageSize();
}
FWL_ERR CFWL_ScrollBar::SetPageSize(FX_FLOAT fPageSize) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return ((IFWL_ScrollBar*)m_pIface)->SetPageSize(fPageSize);
}
FX_FLOAT CFWL_ScrollBar::GetStepSize() {
  if (!m_pIface)
    return 0;
  return ((IFWL_ScrollBar*)m_pIface)->GetStepSize();
}
FWL_ERR CFWL_ScrollBar::SetStepSize(FX_FLOAT fStepSize) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return ((IFWL_ScrollBar*)m_pIface)->SetStepSize(fStepSize);
}
FX_FLOAT CFWL_ScrollBar::GetPos() {
  if (!m_pIface)
    return -1;
  return ((IFWL_ScrollBar*)m_pIface)->GetPos();
}
FWL_ERR CFWL_ScrollBar::SetPos(FX_FLOAT fPos) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return ((IFWL_ScrollBar*)m_pIface)->SetPos(fPos);
}
FX_FLOAT CFWL_ScrollBar::GetTrackPos() {
  if (!m_pIface)
    return -1;
  return ((IFWL_ScrollBar*)m_pIface)->GetTrackPos();
}
FWL_ERR CFWL_ScrollBar::SetTrackPos(FX_FLOAT fTrackPos) {
  if (!m_pIface)
    return FWL_ERR_Indefinite;
  return ((IFWL_ScrollBar*)m_pIface)->SetTrackPos(fTrackPos);
}
FX_BOOL CFWL_ScrollBar::DoScroll(FX_DWORD dwCode, FX_FLOAT fPos) {
  if (!m_pIface)
    return FALSE;
  return ((IFWL_ScrollBar*)m_pIface)->DoScroll(dwCode, fPos);
}
CFWL_ScrollBar::CFWL_ScrollBar() {}
CFWL_ScrollBar::~CFWL_ScrollBar() {}
