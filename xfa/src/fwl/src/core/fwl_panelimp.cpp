// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
#include "include/fwl_noteimp.h"
#include "include/fwl_widgetimp.h"
#include "include/fwl_panelimp.h"
IFWL_Panel* IFWL_Panel::Create()
{
    return new IFWL_Panel;
}
FWL_ERR	IFWL_Panel::Initialize(IFWL_Widget *pOuter )
{
    m_pData = FX_NEW CFWL_PanelImp(pOuter);
    ((CFWL_PanelImp*)m_pData)->SetInterface(this);
    return ((CFWL_PanelImp*)m_pData)->Initialize();
}
FWL_ERR	IFWL_Panel::Initialize(CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
{
    m_pData = FX_NEW CFWL_PanelImp(properties, pOuter);
    ((CFWL_PanelImp*)m_pData)->SetInterface(this);
    return ((CFWL_PanelImp*)m_pData)->Initialize();
}
IFWL_Content* IFWL_Panel::GetContent()
{
    return ((CFWL_PanelImp*)m_pData)->GetContent();
}
FWL_ERR	IFWL_Panel::SetContent(IFWL_Content *pContent)
{
    return ((CFWL_PanelImp*)m_pData)->SetContent(pContent);
}
IFWL_Panel::IFWL_Panel()
{
    m_pData = NULL;
}
IFWL_Panel::~IFWL_Panel()
{
    if (m_pData) {
        delete (CFWL_PanelImp*)m_pData;
        m_pData = NULL;
    }
}
CFWL_PanelImp::CFWL_PanelImp(IFWL_Widget* pOuter )
    : CFWL_WidgetImp(pOuter)
    , m_pContent(NULL)
{
}
CFWL_PanelImp::CFWL_PanelImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : CFWL_WidgetImp(properties, pOuter)
    , m_pContent(NULL)
{
}
CFWL_PanelImp::~CFWL_PanelImp()
{
}
FWL_ERR CFWL_PanelImp::GetClassName(CFX_WideString &wsClass) const
{
    wsClass = FWL_CLASS_Panel;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_PanelImp::GetClassID() const
{
    return FWL_CLASSHASH_Panel;
}
FWL_ERR CFWL_PanelImp::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    if (bAutoSize) {
        if (m_pContent) {
            m_pContent->GetWidgetRect(rect, TRUE);
        }
    } else {
        rect = m_pProperties->m_rtWidget;
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PanelImp::Update()
{
    if (m_pContent) {
        CFX_RectF rtClient;
        GetClientRect(rtClient);
        FWL_GRIDUNIT eWidth = FWL_GRIDUNIT_Fixed, eHeight = FWL_GRIDUNIT_Fixed;
        IFWL_WidgetMgr *pWidgetMgr = FWL_GetWidgetMgr();
        _FWL_RETURN_VALUE_IF_FAIL(pWidgetMgr, FWL_ERR_Indefinite);
        IFWL_Widget *pParent = pWidgetMgr->GetWidget((IFWL_Widget*)this, FWL_WGTRELATION_Parent);
        if (pParent && pParent->GetClassID() == FWL_CLASSHASH_Grid) {
            IFWL_Grid* pGrid = (IFWL_Grid*)pParent;
            pGrid->GetWidgetSize((IFWL_Widget*)this, FWL_GRIDSIZE_Width, eWidth);
            pGrid->GetWidgetSize((IFWL_Widget*)this, FWL_GRIDSIZE_Height, eHeight);
        }
        if (eWidth != FWL_GRIDUNIT_Auto || eHeight != FWL_GRIDUNIT_Auto) {
        }
        m_pContent->SetWidgetRect(rtClient);
        m_pContent->Update();
    }
    return FWL_ERR_Succeeded;
}
IFWL_Content* CFWL_PanelImp::GetContent()
{
    return m_pContent;
}
FWL_ERR CFWL_PanelImp::SetContent(IFWL_Content *pContent)
{
    _FWL_RETURN_VALUE_IF_FAIL(pContent, FWL_ERR_Indefinite);
    m_pContent = pContent;
    return pContent->SetParent(m_pInterface);
}
class CFWL_CustomPanelImp : public CFWL_WidgetImp
{
public:
    CFWL_CustomPanelImp(IFWL_Widget *pOuter = NULL);
    CFWL_CustomPanelImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter = NULL);
    virtual ~CFWL_CustomPanelImp();
    virtual FWL_ERR			GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize = FALSE);
    virtual	FWL_ERR			Update();
    virtual IFWL_Content*	GetContent();
    virtual FWL_ERR			SetContent(IFWL_Content *pContent);
    FWL_ERR					SetProxy(IFWL_Proxy *pProxy);
protected:
    IFWL_Content			*m_pContent;
    IFWL_Proxy				*m_pProxy;
};
CFWL_CustomPanelImp::CFWL_CustomPanelImp(IFWL_Widget* pOuter )
    : CFWL_WidgetImp(pOuter)
    , m_pContent(NULL)
    , m_pProxy(NULL)
{
}
CFWL_CustomPanelImp::CFWL_CustomPanelImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : CFWL_WidgetImp(properties, pOuter)
    , m_pContent(NULL)
    , m_pProxy(NULL)
{
}
CFWL_CustomPanelImp::~CFWL_CustomPanelImp()
{
}
FWL_ERR CFWL_CustomPanelImp::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    if (bAutoSize && m_pProxy && (m_pProxy->GetWidgetRect(rect, bAutoSize) == FWL_ERR_Succeeded)) {
        return FWL_ERR_Succeeded;
    }
    return CFWL_WidgetImp::GetWidgetRect(rect, bAutoSize);
}
FWL_ERR CFWL_CustomPanelImp::Update()
{
    if (m_pProxy) {
        return m_pProxy->Update();
    }
    return CFWL_WidgetImp::Update();
}
IFWL_Content* CFWL_CustomPanelImp::GetContent()
{
    return m_pContent;
}
FWL_ERR CFWL_CustomPanelImp::SetContent(IFWL_Content *pContent)
{
    _FWL_RETURN_VALUE_IF_FAIL(pContent, FWL_ERR_Indefinite);
    m_pContent = pContent;
    return pContent->SetParent(m_pInterface);
}
FWL_ERR CFWL_CustomPanelImp::SetProxy(IFWL_Proxy *pProxy)
{
    m_pProxy = pProxy;
    return FWL_ERR_Succeeded;
}
IFWL_CustomPanel* IFWL_CustomPanel::Create()
{
    return new IFWL_CustomPanel;
}
FWL_ERR	IFWL_CustomPanel::Initialize(IFWL_Widget *pOuter )
{
    m_pData = FX_NEW CFWL_CustomPanelImp(pOuter);
    ((CFWL_CustomPanelImp*)m_pData)->SetInterface(this);
    return ((CFWL_CustomPanelImp*)m_pData)->Initialize();
}
FWL_ERR	IFWL_CustomPanel::Initialize(CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
{
    m_pData = FX_NEW CFWL_CustomPanelImp(properties, pOuter);
    ((CFWL_CustomPanelImp*)m_pData)->SetInterface(this);
    return ((CFWL_CustomPanelImp*)m_pData)->Initialize();
}
IFWL_Content* IFWL_CustomPanel::GetContent()
{
    return ((CFWL_CustomPanelImp*)m_pData)->GetContent();
}
FWL_ERR	IFWL_CustomPanel::SetContent(IFWL_Content *pContent)
{
    return ((CFWL_CustomPanelImp*)m_pData)->SetContent(pContent);
}
FWL_ERR	IFWL_CustomPanel::SetProxy(IFWL_Proxy *pProxy)
{
    return ((CFWL_CustomPanelImp*)m_pData)->SetProxy(pProxy);
}
IFWL_CustomPanel::IFWL_CustomPanel()
{
    m_pData = NULL;
}
IFWL_CustomPanel::~IFWL_CustomPanel()
{
    if (m_pData) {
        delete (CFWL_CustomPanelImp*)m_pData;
        m_pData = NULL;
    }
}
