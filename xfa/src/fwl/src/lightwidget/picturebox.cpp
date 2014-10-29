// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
CFWL_PictureBox* CFWL_PictureBox::Create()
{
    return FX_NEW CFWL_PictureBox;
}
FWL_ERR CFWL_PictureBox::Initialize(const CFWL_WidgetProperties *pProperties )
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
    prop.m_pDataProvider = &m_PictureBoxDP;
    if (m_pProperties->m_pParent) {
        prop.m_pParent = m_pProperties->m_pParent->GetWidget();
    }
    if (m_pProperties->m_pOwner) {
        prop.m_pOwner = m_pProperties->m_pOwner->GetWidget();
    }
    prop.m_rtWidget = m_pProperties->m_rtWidget;
    m_pImp = IFWL_PictureBox::Create();
    FWL_ERR ret = ((IFWL_PictureBox*)m_pImp)->Initialize(prop);
    if (ret == FWL_ERR_Succeeded) {
        CFWL_Widget::Initialize();
    }
    return ret;
}
CFX_DIBitmap* CFWL_PictureBox::GetPicture()
{
    return m_PictureBoxDP.m_pBitmap;
}
FWL_ERR CFWL_PictureBox::SetPicture(CFX_DIBitmap *pBitmap)
{
    m_PictureBoxDP.m_pBitmap = pBitmap;
    return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_PictureBox::GetRotation()
{
    return m_PictureBoxDP.m_fRotation;
}
FWL_ERR CFWL_PictureBox::SetRotation(FX_FLOAT fRotation)
{
    m_PictureBoxDP.m_fRotation = fRotation;
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_PictureBox::GetFlipMode()
{
    return m_PictureBoxDP.GetFlipMode((IFWL_Widget*)this);
}
FWL_ERR CFWL_PictureBox::SetFlipMode(FX_INT32 iFlipMode)
{
    m_PictureBoxDP.m_iFlipMode = iFlipMode;
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_PictureBox::GetOpacity()
{
    return m_PictureBoxDP.GetOpacity((IFWL_Widget*)this);
}
FWL_ERR CFWL_PictureBox::SetOpacity(FX_INT32 iOpacity)
{
    m_PictureBoxDP.m_iOpacity = iOpacity;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::GetScale(FX_FLOAT &fScaleX, FX_FLOAT &fScaleY)
{
    CFX_Matrix matrix;
    m_PictureBoxDP.GetMatrix((IFWL_Widget*)this, matrix);
    matrix.Scale(fScaleX, fScaleY);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::SetScale(FX_FLOAT fScaleX, FX_FLOAT fScaleY)
{
    m_PictureBoxDP.m_fScaleX = fScaleX;
    m_PictureBoxDP.m_fScaleY = fScaleY;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::GetOffset(FX_FLOAT &fx, FX_FLOAT &fy)
{
    CFX_Matrix matrix;
    m_PictureBoxDP.GetMatrix((IFWL_Widget*)this, matrix);
    fx = matrix.e;
    fy = matrix.f;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_PictureBox::SetOffset(FX_FLOAT fx, FX_FLOAT fy)
{
    m_PictureBoxDP.m_fOffSetX = fx;
    m_PictureBoxDP.m_fOffSetY = fy;
    return FWL_ERR_Succeeded;
}
CFWL_PictureBox::CFWL_PictureBox()
{
}
CFWL_PictureBox::~CFWL_PictureBox()
{
}
FWL_ERR CFWL_PictureBox::CFWL_PictureBoxDP::GetCaption(IFWL_Widget *pWidget, CFX_WideString &wsCaption)
{
    return FWL_ERR_Succeeded;
}
CFX_DIBitmap* CFWL_PictureBox::CFWL_PictureBoxDP::GetPicture(IFWL_Widget *pWidget)
{
    return m_pBitmap;
}
CFX_DIBitmap* CFWL_PictureBox::CFWL_PictureBoxDP::GetErrorPicture(IFWL_Widget *pWidget)
{
    return m_pBitmap;
}
CFX_DIBitmap* CFWL_PictureBox::CFWL_PictureBoxDP::GetInitialPicture(IFWL_Widget *pWidget)
{
    return m_pBitmap;
}
FX_INT32 CFWL_PictureBox::CFWL_PictureBoxDP::GetOpacity(IFWL_Widget *pWidget)
{
    return m_iOpacity;
}
FWL_ERR CFWL_PictureBox::CFWL_PictureBoxDP::GetMatrix(IFWL_Widget *pWidget, CFX_Matrix &matrix)
{
    CFX_RectF rect;
    pWidget->GetClientRect(rect);
    FX_FLOAT fLen = rect.width / 2;
    FX_FLOAT fWid = rect.height / 2;
    matrix.Reset();
    matrix.Translate(-fLen, -fWid);
    matrix.Rotate(m_fRotation);
    matrix.Translate(fLen, fWid);
    matrix.Scale(m_fScaleX, m_fScaleY);
    matrix.Translate(m_fOffSetX, m_fOffSetY);
    return FWL_ERR_Succeeded;
}
FX_INT32 CFWL_PictureBox::CFWL_PictureBoxDP::GetFlipMode(IFWL_Widget *pWidget)
{
    return m_iFlipMode;
}
