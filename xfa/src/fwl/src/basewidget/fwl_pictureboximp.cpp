// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../core/include/fwl_targetimp.h"
#include "../core/include/fwl_noteimp.h"
#include "../core/include/fwl_widgetimp.h"
#include "include/fwl_pictureboximp.h"
IFWL_PictureBox* IFWL_PictureBox::Create()
{
    return new IFWL_PictureBox;
}
IFWL_PictureBox::IFWL_PictureBox()
{
    m_pData = NULL;
}
IFWL_PictureBox::~IFWL_PictureBox()
{
    if (m_pData) {
        delete (CFWL_PictureBoxImp*)m_pData;
        m_pData = NULL;
    }
}
FWL_ERR IFWL_PictureBox::Initialize(IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_PictureBoxImp(pOuter);
    ((CFWL_PictureBoxImp*)m_pData)->SetInterface(this);
    return ((CFWL_PictureBoxImp*)m_pData)->Initialize();
}
FWL_ERR	IFWL_PictureBox::Initialize(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter)
{
    m_pData = FX_NEW CFWL_PictureBoxImp(properties, pOuter);
    ((CFWL_PictureBoxImp*)m_pData)->SetInterface(this);
    return ((CFWL_PictureBoxImp*)m_pData)->Initialize();
}
CFWL_PictureBoxImp::CFWL_PictureBoxImp(IFWL_Widget *pOuter )
    : CFWL_WidgetImp(pOuter)
    , m_bTop(FALSE)
    , m_bVCenter(FALSE)
    , m_bButton(FALSE)
{
    m_rtClient.Reset();
    m_rtImage.Reset();
    m_matrix.Reset();
}
CFWL_PictureBoxImp::CFWL_PictureBoxImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : CFWL_WidgetImp(properties, pOuter)
    , m_bTop(FALSE)
    , m_bVCenter(FALSE)
    , m_bButton(FALSE)
{
    m_rtClient.Reset();
    m_rtImage.Reset();
    m_matrix.Reset();
}
CFWL_PictureBoxImp::~CFWL_PictureBoxImp()
{
}
FWL_ERR CFWL_PictureBoxImp::GetClassName(CFX_WideString &wsClass) const
{
    wsClass = FWL_CLASS_PictureBox;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_PictureBoxImp::GetClassID() const
{
    return FWL_CLASSHASH_PictureBox;
}
FWL_ERR	CFWL_PictureBoxImp::Initialize()
{
    _FWL_ERR_CHECK_RETURN_VALUE_IF_FAIL(CFWL_WidgetImp::Initialize(), FWL_ERR_Indefinite);
    m_pDelegate = (IFWL_WidgetDelegate*) FX_NEW CFWL_PictureBoxImpDelegate(this);
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_PictureBoxImp::Finalize()
{
    if ( m_pDelegate) {
        delete (CFWL_PictureBoxImpDelegate*)m_pDelegate;
        m_pDelegate = NULL;
    }
    return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_PictureBoxImp::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    if (bAutoSize) {
        rect.Set(0, 0, 0, 0);
        _FWL_RETURN_VALUE_IF_FAIL(m_pProperties->m_pDataProvider, FWL_ERR_Indefinite);
        CFX_DIBitmap *pBitmap = ((IFWL_PictureBoxDP*)m_pProperties->m_pDataProvider)->GetPicture(m_pInterface);
        if (pBitmap) {
            rect.Set(0, 0, (FX_FLOAT)pBitmap->GetWidth(), (FX_FLOAT)pBitmap->GetHeight());
        }
        CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
    } else {
        rect = m_pProperties->m_rtWidget;
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBoxImp::Update()
{
    if (IsLocked()) {
        return FWL_ERR_Succeeded;
    }
    if (!m_pProperties->m_pThemeProvider) {
        m_pProperties->m_pThemeProvider = GetAvailableTheme();
    }
    GetClientRect(m_rtClient);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBoxImp::DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    _FWL_RETURN_VALUE_IF_FAIL(pGraphics, FWL_ERR_Indefinite);
    _FWL_RETURN_VALUE_IF_FAIL(m_pProperties->m_pThemeProvider, FWL_ERR_Indefinite);
    IFWL_ThemeProvider *pTheme = GetAvailableTheme();
    if (HasBorder()) {
        DrawBorder(pGraphics, FWL_PART_PTB_Border, pTheme, pMatrix );
    }
    if (HasEdge()) {
        DrawEdge(pGraphics, FWL_PART_PTB_Edge, pTheme, pMatrix);
    }
    DrawBkground(pGraphics, pTheme, pMatrix);
    return FWL_ERR_Succeeded;
}
void CFWL_PictureBoxImp::DrawBkground(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix)
{
    _FWL_RETURN_IF_FAIL(m_pProperties->m_pDataProvider);
    CFX_DIBitmap *pPicture = ((IFWL_PictureBoxDP*)m_pProperties->m_pDataProvider)->GetPicture(m_pInterface);
    CFX_Matrix matrix;
    ((IFWL_PictureBoxDP*)m_pProperties->m_pDataProvider)->GetMatrix(m_pInterface, matrix);
    if (!pPicture) {
        return;
    }
    matrix.Concat(*pMatrix);
    FX_FLOAT fx = (FX_FLOAT)pPicture->GetWidth();
    FX_FLOAT fy = (FX_FLOAT)pPicture->GetHeight();
    if (fx > m_rtClient.width) {
        fx = m_rtClient.width;
    }
    if (fy > m_rtClient.height) {
        fy = m_rtClient.height;
    }
    CFX_PointF pt;
    pt.Set((m_rtClient.width - fx) / 2, (m_rtClient.height - fy) / 2);
    pGraphics->DrawImage(pPicture, pt, &matrix);
}
FX_BOOL CFWL_PictureBoxImp::VStyle(FX_BOOL dwStyle)
{
    switch(dwStyle & FWL_STYLEEXT_PTB_VAlignMask) {
        case FWL_STYLEEXT_PTB_Top: {
                return m_bTop = TRUE;
                break;
            }
        case FWL_STYLEEXT_PTB_Vcenter: {
                return m_bVCenter = TRUE;
                break;
            }
        case FWL_STYLEEXT_PTB_Bottom: {
                return m_bButton = TRUE;
                break;
            }
    }
    return FALSE;
}
CFWL_PictureBoxImpDelegate::CFWL_PictureBoxImpDelegate(CFWL_PictureBoxImp *pOwner)
    : m_pOwner(pOwner)
{
}
FWL_ERR	CFWL_PictureBoxImpDelegate::OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix)
{
    return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
