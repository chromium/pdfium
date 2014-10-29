// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../core/include/fwl_targetimp.h"
#include "../core/include/fwl_noteimp.h"
#include "../core/include/fwl_widgetimp.h"
#include "../core/include/fwl_panelimp.h"
#include "../core/include/fwl_formimp.h"
#include "include/fwl_formproxyimp.h"
CFWL_FormProxyImp::CFWL_FormProxyImp(IFWL_Widget *pOuter )
    : CFWL_FormImp(pOuter)
{
}
CFWL_FormProxyImp::CFWL_FormProxyImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : CFWL_FormImp(properties, pOuter)
{
}
CFWL_FormProxyImp::~CFWL_FormProxyImp()
{
}
FWL_ERR CFWL_FormProxyImp::GetClassName(CFX_WideString &wsClass) const
{
    wsClass = FWL_CLASS_FormProxy;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_FormProxyImp::GetClassID() const
{
    return FWL_CLASSHASH_FormProxy;
}
FX_BOOL CFWL_FormProxyImp::IsInstance(FX_WSTR wsClass) const
{
    if (wsClass == CFX_WideStringC(FWL_CLASS_FormProxy)) {
        return TRUE;
    }
    return CFWL_FormImp::IsInstance(wsClass);
}
FWL_ERR CFWL_FormProxyImp::Initialize()
{
    _FWL_ERR_CHECK_RETURN_VALUE_IF_FAIL(CFWL_WidgetImp::Initialize(), FWL_ERR_Indefinite);
    m_pDelegate = (IFWL_WidgetDelegate *)FX_NEW CFWL_FormProxyImpDelegate(this);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_FormProxyImp::Finalize()
{
    if ( m_pDelegate) {
        delete (CFWL_FormProxyImpDelegate*)m_pDelegate;
        m_pDelegate = NULL;
    }
    return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_FormProxyImp::Update()
{
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_FormProxyImp::DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    return FWL_ERR_Succeeded;
}
CFWL_FormProxyImpDelegate::CFWL_FormProxyImpDelegate(CFWL_FormProxyImp *pOwner)
    : m_pOwner(pOwner)
{
}
FX_INT32 CFWL_FormProxyImpDelegate::OnProcessMessage(CFWL_Message *pMessage)
{
    IFWL_WidgetDelegate *pDelegate = m_pOwner->m_pOuter->SetDelegate(NULL);
    return pDelegate->OnProcessMessage(pMessage);
}
