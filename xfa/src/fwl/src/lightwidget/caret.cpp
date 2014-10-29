// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_Caret* CFWL_Caret::Create()
{
    return FX_NEW CFWL_Caret;
}
FWL_ERR	CFWL_Caret::Initialize(const CFWL_WidgetProperties *pProperties )
{
    _FWL_RETURN_VALUE_IF_FAIL(!m_pImp, FWL_ERR_Indefinite);
    if (pProperties) {
        *m_pProperties = *pProperties;
    }
    CFWL_WidgetImpProperties prop;
    prop.m_ctmOnParent = m_pProperties->m_ctmOnParent;
    prop.m_rtWidget = m_pProperties->m_rtWidget;
    prop.m_dwStyles = m_pProperties->m_dwStyles;
    prop.m_dwStyleExes = m_pProperties->m_dwStyleExes;
    prop.m_dwStates = m_pProperties->m_dwStates;
    if (m_pProperties->m_pParent) {
        prop.m_pParent = m_pProperties->m_pParent->GetWidget();
    }
    if (m_pProperties->m_pOwner) {
        prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
    }
    m_pImp = IFWL_Caret::Create();
    FWL_ERR ret = ((IFWL_Caret*)m_pImp)->Initialize(prop);
    if (ret == FWL_ERR_Succeeded) {
        CFWL_Widget::Initialize();
    }
    return ret;
}
FWL_ERR CFWL_Caret::ShowCaret(FX_BOOL bFlag)
{
    return ((IFWL_Caret*)m_pImp)->ShowCaret(bFlag);
}
FWL_ERR CFWL_Caret::GetFrequency(FX_DWORD &elapse)
{
    return ((IFWL_Caret*)m_pImp)->GetFrequency(elapse);
}
FWL_ERR CFWL_Caret::SetFrequency(FX_DWORD elapse)
{
    return ((IFWL_Caret*)m_pImp)->SetFrequency(elapse);
}
FWL_ERR	CFWL_Caret::SetColor(CFX_Color crFill)
{
    return ((IFWL_Caret*)m_pImp)->SetColor(crFill);
}
CFWL_Caret::CFWL_Caret()
{
}
CFWL_Caret::~CFWL_Caret()
{
}
