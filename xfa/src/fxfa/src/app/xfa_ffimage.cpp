// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_ffwidget.h"
#include "xfa_ffdraw.h"
#include "xfa_ffimage.h"
#include "xfa_ffpageview.h"
#include "xfa_ffdoc.h"
#include "xfa_ffapp.h"
CXFA_FFImage::CXFA_FFImage(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFDraw(pPageView, pDataAcc)
{
}
CXFA_FFImage::~CXFA_FFImage()
{
    CXFA_FFImage::UnloadWidget();
}
FX_BOOL CXFA_FFImage::IsLoaded()
{
    return GetDataAcc()->GetImageImage() != NULL;
}
FX_BOOL CXFA_FFImage::LoadWidget()
{
    if (GetDataAcc()->GetImageImage()) {
        return TRUE;
    }
    GetDataAcc()->LoadImageImage();
    return CXFA_FFDraw::LoadWidget();
}
void CXFA_FFImage::UnloadWidget()
{
    GetDataAcc()->SetImageImage(NULL);
}
void CXFA_FFImage::RenderWidget(CFX_Graphics* pGS, CFX_Matrix* pMatrix , FX_DWORD dwStatus , FX_INT32 iRotate )
{
    if (!IsMatchVisibleStatus(dwStatus)) {
        return;
    }
    CFX_Matrix mtRotate;
    GetRotateMatrix(mtRotate);
    if (pMatrix) {
        mtRotate.Concat(*pMatrix);
    }
    CXFA_FFWidget::RenderWidget(pGS, &mtRotate, dwStatus);
    if (CFX_DIBitmap* pDIBitmap = GetDataAcc()->GetImageImage()) {
        CFX_RectF rtImage;
        GetRectWithoutRotate(rtImage);
        if (CXFA_Margin mgWidget = m_pDataAcc->GetMargin()) {
            XFA_RectWidthoutMargin(rtImage, mgWidget);
        }
        FX_INT32 iHorzAlign = XFA_ATTRIBUTEENUM_Left;
        FX_INT32 iVertAlign = XFA_ATTRIBUTEENUM_Top;
        if (CXFA_Para para = m_pDataAcc->GetPara()) {
            iHorzAlign = para.GetHorizontalAlign();
            iVertAlign = para.GetVerticalAlign();
        }
        CXFA_Value value = m_pDataAcc->GetFormValue();
        CXFA_Image imageObj = value.GetImage();
        FX_INT32 iAspect = imageObj.GetAspect();
        FX_INT32 iImageXDpi = 0;
        FX_INT32 iImageYDpi = 0;
        m_pDataAcc->GetImageDpi(iImageXDpi, iImageYDpi);
        XFA_DrawImage(pGS, rtImage, &mtRotate, pDIBitmap, iAspect, iImageXDpi, iImageYDpi, iHorzAlign, iVertAlign);
    }
}
