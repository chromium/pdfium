// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_panelimp.h"

// static
IFWL_Panel* IFWL_Panel::Create(CFWL_WidgetImpProperties& properties,
                               IFWL_Widget* pOuter) {
  IFWL_Panel* pPanel = new IFWL_Panel;
  CFWL_PanelImp* pPanelImpl = new CFWL_PanelImp(properties, pOuter);
  pPanel->SetImpl(pPanelImpl);
  pPanelImpl->SetInterface(pPanel);
  return pPanel;
}
IFWL_Panel::IFWL_Panel() {}
IFWL_Content* IFWL_Panel::GetContent() {
  return static_cast<CFWL_PanelImp*>(GetImpl())->GetContent();
}
FWL_ERR IFWL_Panel::SetContent(IFWL_Content* pContent) {
  return static_cast<CFWL_PanelImp*>(GetImpl())->SetContent(pContent);
}

CFWL_PanelImp::CFWL_PanelImp(const CFWL_WidgetImpProperties& properties,
                             IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter), m_pContent(nullptr) {}
CFWL_PanelImp::~CFWL_PanelImp() {}
FWL_ERR CFWL_PanelImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_Panel;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_PanelImp::GetClassID() const {
  return FWL_CLASSHASH_Panel;
}
FWL_ERR CFWL_PanelImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    if (m_pContent) {
      m_pContent->GetWidgetRect(rect, TRUE);
    }
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PanelImp::Update() {
  if (m_pContent) {
    CFX_RectF rtClient;
    GetClientRect(rtClient);
    FWL_GRIDUNIT eWidth = FWL_GRIDUNIT_Fixed, eHeight = FWL_GRIDUNIT_Fixed;
    IFWL_WidgetMgr* pWidgetMgr = FWL_GetWidgetMgr();
    if (!pWidgetMgr)
      return FWL_ERR_Indefinite;
    IFWL_Widget* pParent =
        pWidgetMgr->GetWidget(GetInterface(), FWL_WGTRELATION_Parent);
    if (pParent && pParent->GetClassID() == FWL_CLASSHASH_Grid) {
      IFWL_Grid* pGrid = static_cast<IFWL_Grid*>(pParent);
      pGrid->GetWidgetSize(GetInterface(), FWL_GRIDSIZE_Width, eWidth);
      pGrid->GetWidgetSize(GetInterface(), FWL_GRIDSIZE_Height, eHeight);
    }
    m_pContent->SetWidgetRect(rtClient);
    m_pContent->Update();
  }
  return FWL_ERR_Succeeded;
}
IFWL_Content* CFWL_PanelImp::GetContent() {
  return m_pContent;
}
FWL_ERR CFWL_PanelImp::SetContent(IFWL_Content* pContent) {
  if (!pContent)
    return FWL_ERR_Indefinite;
  m_pContent = pContent;
  return pContent->SetParent(m_pInterface);
}
class CFWL_CustomPanelImp : public CFWL_WidgetImp {
 public:
  CFWL_CustomPanelImp(const CFWL_WidgetImpProperties& properties,
                      IFWL_Widget* pOuter);
  virtual ~CFWL_CustomPanelImp();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual IFWL_Content* GetContent();
  virtual FWL_ERR SetContent(IFWL_Content* pContent);
  FWL_ERR SetProxy(IFWL_Proxy* pProxy);

 protected:
  IFWL_Content* m_pContent;
  IFWL_Proxy* m_pProxy;
};
CFWL_CustomPanelImp::CFWL_CustomPanelImp(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_pContent(nullptr),
      m_pProxy(nullptr) {}
CFWL_CustomPanelImp::~CFWL_CustomPanelImp() {}
FWL_ERR CFWL_CustomPanelImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize && m_pProxy &&
      (m_pProxy->GetWidgetRect(rect, bAutoSize) == FWL_ERR_Succeeded)) {
    return FWL_ERR_Succeeded;
  }
  return CFWL_WidgetImp::GetWidgetRect(rect, bAutoSize);
}
FWL_ERR CFWL_CustomPanelImp::Update() {
  if (m_pProxy) {
    return m_pProxy->Update();
  }
  return CFWL_WidgetImp::Update();
}
IFWL_Content* CFWL_CustomPanelImp::GetContent() {
  return m_pContent;
}
FWL_ERR CFWL_CustomPanelImp::SetContent(IFWL_Content* pContent) {
  if (!pContent)
    return FWL_ERR_Indefinite;
  m_pContent = pContent;
  return pContent->SetParent(m_pInterface);
}
FWL_ERR CFWL_CustomPanelImp::SetProxy(IFWL_Proxy* pProxy) {
  m_pProxy = pProxy;
  return FWL_ERR_Succeeded;
}

// statuc
IFWL_CustomPanel* IFWL_CustomPanel::Create(CFWL_WidgetImpProperties& properties,
                                           IFWL_Widget* pOuter) {
  IFWL_CustomPanel* pCustomPanel = new IFWL_CustomPanel;
  CFWL_CustomPanelImp* pCustomPanelImpl =
      new CFWL_CustomPanelImp(properties, pOuter);
  pCustomPanel->SetImpl(pCustomPanelImpl);
  pCustomPanelImpl->SetInterface(pCustomPanel);
  return pCustomPanel;
}
IFWL_CustomPanel::IFWL_CustomPanel() {}
IFWL_Content* IFWL_CustomPanel::GetContent() {
  return static_cast<CFWL_CustomPanelImp*>(GetImpl())->GetContent();
}
FWL_ERR IFWL_CustomPanel::SetContent(IFWL_Content* pContent) {
  return static_cast<CFWL_CustomPanelImp*>(GetImpl())->SetContent(pContent);
}
FWL_ERR IFWL_CustomPanel::SetProxy(IFWL_Proxy* pProxy) {
  return static_cast<CFWL_CustomPanelImp*>(GetImpl())->SetProxy(pProxy);
}
