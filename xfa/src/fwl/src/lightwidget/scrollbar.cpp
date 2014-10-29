// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_ScrollBar* CFWL_ScrollBar::Create()
{
    return FX_NEW CFWL_ScrollBar;
}
FWL_ERR CFWL_ScrollBar::Initialize(const CFWL_WidgetProperties *pProperties )
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
    if (m_pProperties->m_pParent) {
        prop.m_pParent = m_pProperties->m_pParent->GetWidget();
    }
    if (m_pProperties->m_pOwner) {
        prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
    }
    prop.m_rtWidget = m_pProperties->m_rtWidget;
    m_pImp = IFWL_ScrollBar::Create();
    FWL_ERR ret = ((IFWL_ScrollBar*)m_pImp)->Initialize(prop);
    if (ret == FWL_ERR_Succeeded) {
        CFWL_Widget::Initialize();
    }
    return ret;
}
FX_BOOL CFWL_ScrollBar::IsVertical()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ScrollBar*)m_pImp)->IsVertical();
}
FWL_ERR CFWL_ScrollBar::GetRange(FX_FLOAT &fMin, FX_FLOAT &fMax)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ScrollBar*)m_pImp)->GetRange(fMin, fMax);
}
FWL_ERR CFWL_ScrollBar::SetRange(FX_FLOAT fMin, FX_FLOAT fMax)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ScrollBar*)m_pImp)->SetRange(fMin, fMax);
}
FX_FLOAT CFWL_ScrollBar::GetPageSize()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_ScrollBar*)m_pImp)->GetPageSize();
}
FWL_ERR CFWL_ScrollBar::SetPageSize(FX_FLOAT fPageSize)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ScrollBar*)m_pImp)->SetPageSize(fPageSize);
}
FX_FLOAT CFWL_ScrollBar::GetStepSize()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_ScrollBar*)m_pImp)->GetStepSize();
}
FWL_ERR CFWL_ScrollBar::SetStepSize(FX_FLOAT fStepSize)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ScrollBar*)m_pImp)->SetStepSize(fStepSize);
}
FX_FLOAT CFWL_ScrollBar::GetPos()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, -1);
    return ((IFWL_ScrollBar*)m_pImp)->GetPos();
}
FWL_ERR CFWL_ScrollBar::SetPos(FX_FLOAT fPos)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ScrollBar*)m_pImp)->SetPos(fPos);
}
FX_FLOAT CFWL_ScrollBar::GetTrackPos()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, -1);
    return ((IFWL_ScrollBar*)m_pImp)->GetTrackPos();
}
FWL_ERR CFWL_ScrollBar::SetTrackPos(FX_FLOAT fTrackPos)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return ((IFWL_ScrollBar*)m_pImp)->SetTrackPos(fTrackPos);
}
FX_BOOL CFWL_ScrollBar::DoScroll(FX_DWORD dwCode, FX_FLOAT fPos)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return ((IFWL_ScrollBar*)m_pImp)->DoScroll(dwCode, fPos);
}
CFWL_ScrollBar::CFWL_ScrollBar()
{
}
CFWL_ScrollBar::~CFWL_ScrollBar()
{
}
