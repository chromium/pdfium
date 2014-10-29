// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
#include "include/fwl_threadimp.h"
#include "include/fwl_noteimp.h"
#include "include/fwl_widgetimp.h"
#include "include/fwl_contentimp.h"
#include "include/fwl_widgetmgrimp.h"
IFWL_Content* IFWL_Content::Create()
{
    return new IFWL_Content;
}
FWL_ERR IFWL_Content::Initialize()
{
    m_pData = FX_NEW CFWL_ContentImp;
    ((CFWL_ContentImp*)m_pData)->SetInterface(this);
    return ((CFWL_ContentImp*)m_pData)->Initialize();
}
FWL_ERR IFWL_Content::InsertWidget(IFWL_Widget *pChild, FX_INT32 nIndex )
{
    return ((CFWL_ContentImp*)m_pData)->InsertWidget(pChild, nIndex);
}
FWL_ERR IFWL_Content::RemoveWidget(IFWL_Widget *pWidget)
{
    return ((CFWL_ContentImp*)m_pData)->RemoveWidget(pWidget);
}
FWL_ERR IFWL_Content::RemoveAllWidgets()
{
    return ((CFWL_ContentImp*)m_pData)->RemoveAllWidgets();
}
FWL_ERR	IFWL_Content::GetMinSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight)
{
    return ((CFWL_ContentImp*)m_pData)->GetMinSize(fWidth, fHeight);
}
FWL_ERR	IFWL_Content::SetMinSize(FX_FLOAT fWidth, FX_FLOAT fHeight)
{
    return ((CFWL_ContentImp*)m_pData)->SetMinSize(fWidth, fHeight);
}
FWL_ERR	IFWL_Content::GetMaxSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight)
{
    return ((CFWL_ContentImp*)m_pData)->GetMaxSize(fWidth, fHeight);
}
FWL_ERR IFWL_Content::SetMaxSize(FX_FLOAT fWidth, FX_FLOAT fHeight)
{
    return ((CFWL_ContentImp*)m_pData)->SetMaxSize(fWidth, fHeight);
}
IFWL_Content::IFWL_Content()
{
    m_pData = NULL;
}
IFWL_Content::~IFWL_Content()
{
    if (m_pData) {
        delete (CFWL_ContentImp*)m_pData;
        m_pData = NULL;
    }
}
CFWL_ContentImp::CFWL_ContentImp()
{
    m_fWidthMin = 0;
    m_fWidthMax = 10000;
    m_fHeightMin = 0;
    m_fHeightMax = 10000;
}
CFWL_ContentImp::CFWL_ContentImp(const CFWL_WidgetImpProperties &properties)
    : CFWL_WidgetImp(properties)
{
    m_fWidthMin = 0;
    m_fWidthMax = 10000;
    m_fHeightMin = 0;
    m_fHeightMax = 10000;
}
CFWL_ContentImp::~CFWL_ContentImp()
{
}
FWL_ERR CFWL_ContentImp::InsertWidget(IFWL_Widget *pChild, FX_INT32 nIndex )
{
    _FWL_RETURN_VALUE_IF_FAIL(pChild, FWL_ERR_Indefinite);
    pChild->SetParent(m_pInterface);
    if (nIndex == -1) {
        return FWL_ERR_Succeeded;
    }
    CFWL_WidgetMgr *pMgr = (CFWL_WidgetMgr*)FWL_GetWidgetMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pMgr, FWL_ERR_Indefinite);
    pMgr->SetWidgetIndex(pChild, nIndex);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::RemoveWidget(IFWL_Widget *pWidget)
{
    _FWL_RETURN_VALUE_IF_FAIL(pWidget, FWL_ERR_Indefinite);
    pWidget->SetParent(NULL);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_ContentImp::RemoveAllWidgets()
{
    CFWL_WidgetMgr *pMgr = (CFWL_WidgetMgr*)FWL_GetWidgetMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pMgr, FWL_ERR_Indefinite);
    while (IFWL_Widget * widget = pMgr->GetWidget(m_pInterface, FWL_WGTRELATION_FirstChild)) {
        pMgr->SetParent(NULL, widget);
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_ContentImp::GetMinSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight)
{
    fWidth = m_fWidthMin;
    fHeight = m_fHeightMin;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_ContentImp::SetMinSize(FX_FLOAT fWidth, FX_FLOAT fHeight)
{
    m_fWidthMin = fWidth;
    m_fHeightMin = fHeight;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_ContentImp::GetMaxSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight)
{
    fWidth = m_fWidthMax;
    fHeight = m_fHeightMax;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_ContentImp::SetMaxSize(FX_FLOAT fWidth, FX_FLOAT fHeight)
{
    m_fWidthMax = fWidth;
    m_fHeightMax = fHeight;
    return FWL_ERR_Succeeded;
}
