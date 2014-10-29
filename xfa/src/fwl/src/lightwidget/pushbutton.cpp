// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_PushButton* CFWL_PushButton::Create()
{
    return FX_NEW CFWL_PushButton;
}
FWL_ERR CFWL_PushButton::Initialize(const CFWL_WidgetProperties *pProperties )
{
    _FWL_RETURN_VALUE_IF_FAIL(!m_pImp, FWL_ERR_Indefinite);
    if (pProperties) {
        *m_pProperties = *pProperties;
    }
    CFWL_WidgetImpProperties prop;
    prop.m_dwStyles = m_pProperties->m_dwStyles;
    prop.m_dwStyleExes = m_pProperties->m_dwStyleExes;
    prop.m_dwStates = m_pProperties->m_dwStates;
    prop.m_ctmOnParent = m_pProperties->m_ctmOnParent;
    prop.m_pDataProvider = &m_buttonData;
    if (m_pProperties->m_pParent) {
        prop.m_pParent = m_pProperties->m_pParent->GetWidget();
    }
    if (m_pProperties->m_pOwner) {
        prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
    }
    prop.m_rtWidget = m_pProperties->m_rtWidget;
    m_pImp = IFWL_PushButton::Create();
    FWL_ERR ret = ((IFWL_PushButton*)m_pImp)->Initialize(prop);
    if (ret == FWL_ERR_Succeeded) {
        CFWL_Widget::Initialize();
    }
    return ret;
}
FWL_ERR CFWL_PushButton::GetCaption(CFX_WideString &wsCaption)
{
    wsCaption = m_buttonData.m_wsCaption;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PushButton::SetCaption(FX_WSTR wsCaption)
{
    m_buttonData.m_wsCaption = wsCaption;
    return FWL_ERR_Succeeded;
}
CFX_DIBitmap* CFWL_PushButton::GetPicture()
{
    return m_buttonData.m_pBitmap;
}
FWL_ERR CFWL_PushButton::SetPicture(CFX_DIBitmap *pBitmap)
{
    m_buttonData.m_pBitmap = pBitmap;
    return FWL_ERR_Succeeded;
}
CFWL_PushButton::CFWL_PushButton()
{
}
CFWL_PushButton::~CFWL_PushButton()
{
}
FWL_ERR CFWL_PushButton::CFWL_PushButtonDP::GetCaption(IFWL_Widget *pWidget, CFX_WideString &wsCaption)
{
    wsCaption = m_wsCaption;
    return FWL_ERR_Succeeded;
}
CFX_DIBitmap* CFWL_PushButton::CFWL_PushButtonDP::GetPicture(IFWL_Widget *pWidget)
{
    return m_pBitmap;
}
