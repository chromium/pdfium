// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_contentimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetmgrimp.h"
FWL_ERR IFWL_Content::InsertWidget(IFWL_Widget* pChild, int32_t nIndex) {
  return static_cast<CFWL_ContentImp*>(GetImpl())->InsertWidget(pChild, nIndex);
}
FWL_ERR IFWL_Content::RemoveWidget(IFWL_Widget* pWidget) {
  return static_cast<CFWL_ContentImp*>(GetImpl())->RemoveWidget(pWidget);
}
FWL_ERR IFWL_Content::RemoveAllWidgets() {
  return static_cast<CFWL_ContentImp*>(GetImpl())->RemoveAllWidgets();
}
FWL_ERR IFWL_Content::GetMinSize(FX_FLOAT& fWidth, FX_FLOAT& fHeight) {
  return static_cast<CFWL_ContentImp*>(GetImpl())->GetMinSize(fWidth, fHeight);
}
FWL_ERR IFWL_Content::SetMinSize(FX_FLOAT fWidth, FX_FLOAT fHeight) {
  return static_cast<CFWL_ContentImp*>(GetImpl())->SetMinSize(fWidth, fHeight);
}
FWL_ERR IFWL_Content::GetMaxSize(FX_FLOAT& fWidth, FX_FLOAT& fHeight) {
  return static_cast<CFWL_ContentImp*>(GetImpl())->GetMaxSize(fWidth, fHeight);
}
FWL_ERR IFWL_Content::SetMaxSize(FX_FLOAT fWidth, FX_FLOAT fHeight) {
  return static_cast<CFWL_ContentImp*>(GetImpl())->SetMaxSize(fWidth, fHeight);
}
IFWL_Content::IFWL_Content() {
}
CFWL_ContentImp::CFWL_ContentImp(const CFWL_WidgetImpProperties& properties,
                                 IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_fWidthMin(0),
      m_fWidthMax(10000),
      m_fHeightMin(0),
      m_fHeightMax(10000) {}
CFWL_ContentImp::~CFWL_ContentImp() {}
FWL_ERR CFWL_ContentImp::InsertWidget(IFWL_Widget* pChild, int32_t nIndex) {
  if (!pChild)
    return FWL_ERR_Indefinite;
  pChild->SetParent(m_pInterface);
  if (nIndex == -1) {
    return FWL_ERR_Succeeded;
  }
  CFWL_WidgetMgr* pMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pMgr)
    return FWL_ERR_Indefinite;
  pMgr->SetWidgetIndex(pChild, nIndex);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::RemoveWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FWL_ERR_Indefinite;
  pWidget->SetParent(NULL);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::RemoveAllWidgets() {
  CFWL_WidgetMgr* pMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pMgr)
    return FWL_ERR_Indefinite;
  while (IFWL_Widget* widget =
             pMgr->GetWidget(m_pInterface, FWL_WGTRELATION_FirstChild)) {
    pMgr->SetParent(NULL, widget);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::GetMinSize(FX_FLOAT& fWidth, FX_FLOAT& fHeight) {
  fWidth = m_fWidthMin;
  fHeight = m_fHeightMin;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::SetMinSize(FX_FLOAT fWidth, FX_FLOAT fHeight) {
  m_fWidthMin = fWidth;
  m_fHeightMin = fHeight;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::GetMaxSize(FX_FLOAT& fWidth, FX_FLOAT& fHeight) {
  fWidth = m_fWidthMax;
  fHeight = m_fHeightMax;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::SetMaxSize(FX_FLOAT fWidth, FX_FLOAT fHeight) {
  m_fWidthMax = fWidth;
  m_fHeightMax = fHeight;
  return FWL_ERR_Succeeded;
}
