// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_Barcode* CFWL_Barcode::Create()
{
    return FX_NEW CFWL_Barcode;
}
FWL_ERR	CFWL_Barcode::Initialize(const CFWL_WidgetProperties *pProperties )
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
    prop.m_pDataProvider = &m_barcodeData;
    if (m_pProperties->m_pParent) {
        prop.m_pParent = m_pProperties->m_pParent->GetWidget();
    }
    if (m_pProperties->m_pOwner) {
        prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
    }
    m_pImp = IFWL_Barcode::Create();
    FWL_ERR ret = ((IFWL_Barcode*)m_pImp)->Initialize(prop);
    if (ret == FWL_ERR_Succeeded) {
        CFWL_Widget::Initialize();
    }
    return ret;
}
CFWL_Barcode::CFWL_Barcode()
{
}
CFWL_Barcode::~CFWL_Barcode()
{
}
void CFWL_Barcode::SetType(BC_TYPE type)
{
    _FWL_RETURN_IF_FAIL(m_pImp);
    ((IFWL_Barcode*)m_pImp)->SetType(type);
}
FX_BOOL	CFWL_Barcode::IsProtectedType()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return ((IFWL_Barcode*)m_pImp)->IsProtectedType();
}
FWL_ERR CFWL_Barcode::CFWL_BarcodeDP::GetCaption(IFWL_Widget *pWidget, CFX_WideString &wsCaption)
{
    return FWL_ERR_Succeeded;
}
