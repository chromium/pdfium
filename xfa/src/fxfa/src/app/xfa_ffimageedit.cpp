// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_ffwidget.h"
#include "xfa_fffield.h"
#include "xfa_ffpageview.h"
#include "xfa_ffimageedit.h"
#include "xfa_ffdocview.h"
#include "xfa_ffdoc.h"
CXFA_FFImageEdit::CXFA_FFImageEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pPageView, pDataAcc)
    , m_pOldDelegate(NULL)
{
}
CXFA_FFImageEdit::~CXFA_FFImageEdit()
{
    CXFA_FFImageEdit::UnloadWidget();
}
FX_BOOL CXFA_FFImageEdit::LoadWidget()
{
    CFWL_PictureBox* pPictureBox = FX_NEW CFWL_PictureBox;
    if (pPictureBox) {
        pPictureBox->Initialize();
    }
    m_pNormalWidget = (CFWL_Widget*)pPictureBox;
    IFWL_Widget* pWidget = m_pNormalWidget->GetWidget();
    m_pNormalWidget->SetPrivateData(pWidget, this, NULL);
    IFWL_NoteDriver *pNoteDriver = FWL_GetApp()->GetNoteDriver();
    pNoteDriver->RegisterEventTarget(pWidget, pWidget);
    m_pOldDelegate = pPictureBox->SetDelegate(this);
    CXFA_FFField::LoadWidget();
    if (m_pDataAcc->GetImageEditImage() != NULL) {
        return TRUE;
    }
    UpdateFWLData();
    return TRUE;
}
void CXFA_FFImageEdit::UnloadWidget()
{
    m_pDataAcc->SetImageEditImage(NULL);
    CXFA_FFField::UnloadWidget();
}
void CXFA_FFImageEdit::RenderWidget(CFX_Graphics* pGS, CFX_Matrix* pMatrix , FX_DWORD dwStatus , FX_INT32 iRotate )
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
    CXFA_Border borderUI = m_pDataAcc->GetUIBorder();
    DrawBorder(pGS, borderUI, m_rtUI, &mtRotate);
    RenderCaption(pGS, &mtRotate);
    if (CFX_DIBitmap* pDIBitmap = m_pDataAcc->GetImageEditImage()) {
        CFX_RectF rtImage;
        m_pNormalWidget->GetWidgetRect(rtImage);
        FX_INT32 iHorzAlign = XFA_ATTRIBUTEENUM_Left;
        FX_INT32 iVertAlign = XFA_ATTRIBUTEENUM_Top;
        if (CXFA_Para para = m_pDataAcc->GetPara()) {
            iHorzAlign = para.GetHorizontalAlign();
            iVertAlign = para.GetVerticalAlign();
        }
        FX_INT32 iAspect = XFA_ATTRIBUTEENUM_Fit;
        if (CXFA_Value value = m_pDataAcc->GetFormValue()) {
            if (CXFA_Image imageObj = value.GetImage()) {
                iAspect = imageObj.GetAspect();
            }
        }
        FX_INT32 iImageXDpi = 0;
        FX_INT32 iImageYDpi = 0;
        m_pDataAcc->GetImageEditDpi(iImageXDpi, iImageYDpi);
        XFA_DrawImage(pGS, rtImage, &mtRotate, pDIBitmap, iAspect, iImageXDpi, iImageYDpi, iHorzAlign, iVertAlign);
    }
}
FX_BOOL CXFA_FFImageEdit::OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy)
{
    if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open) {
        return FALSE;
    }
    if (!PtInActiveRect(fx, fy)) {
        return FALSE;
    }
    SetButtonDown(TRUE);
    CFWL_MsgMouse ms;
    ms.m_dwCmd = FWL_MSGMOUSECMD_LButtonDown;
    ms.m_dwFlags = dwFlags;
    ms.m_fx = fx;
    ms.m_fy = fy;
    ms.m_pDstTarget = m_pNormalWidget->m_pImp;
    FWLToClient(ms.m_fx, ms.m_fy);
    TranslateFWLMessage(&ms);
    IXFA_AppProvider* pAppProvider = GetAppProvider();
    if (!pAppProvider) {
        return TRUE;
    }
    CFX_WideString wsTitle;
    CFX_WideString wsFilter;
    pAppProvider->LoadString(XFA_IDS_ImageFilter, wsFilter);
    CFX_WideStringArray wsPathArray;
    pAppProvider->ShowFileDialog(wsTitle, wsFilter, wsPathArray);
    FX_INT32 iSize = wsPathArray.GetSize();
    if (iSize < 1) {
        return TRUE;
    }
    CFX_WideString wsFilePath = wsPathArray[0];
    FX_STRSIZE nLen = wsFilePath.GetLength();
    FX_STRSIZE nIndex = nLen - 1;
    while (nIndex > 0 && wsFilePath[nIndex] != '.') {
        nIndex--;
    }
    if (nIndex <= 0) {
        return TRUE;
    }
    CFX_WideString wsContentType(L"image/");
    wsContentType += wsFilePath.Right(nLen - nIndex - 1);
    wsContentType.MakeLower();
    FXCODEC_IMAGE_TYPE eImageType = XFA_GetImageType(wsContentType);
    if (eImageType == FXCODEC_IMAGE_UNKNOWN) {
        return TRUE;
    }
    CFX_WideString wsImage;
    IFX_FileRead* pFileRead = FX_CreateFileRead(wsFilePath);
    if (pFileRead != NULL) {
        FX_INT32 nDataSize = pFileRead->GetSize();
        if (nDataSize > 0) {
            CFX_ByteString bsBuf;
            FX_LPSTR pImageBuffer = bsBuf.GetBuffer(nDataSize);
            pFileRead->ReadBlock(pImageBuffer, 0, nDataSize);
            bsBuf.ReleaseBuffer();
            if (!bsBuf.IsEmpty()) {
                FX_LPSTR pData = XFA_Base64Encode(bsBuf, nDataSize);
                wsImage = CFX_WideString::FromLocal(pData);
                if (pData != NULL) {
                    FX_Free(pData);
                }
            }
        }
        m_pDataAcc->SetImageEditImage(NULL);
        pFileRead->Release();
    }
    m_pDataAcc->SetImageEdit(wsContentType, CFX_WideStringC(), wsImage);
    m_pDataAcc->LoadImageEditImage();
    AddInvalidateRect();
    m_pDocView->SetChangeMark();
    return TRUE;
}
void CXFA_FFImageEdit::SetFWLRect()
{
    if (!m_pNormalWidget) {
        return;
    }
    CFX_RectF rtUIMargin;
    m_pDataAcc->GetUIMargin(rtUIMargin);
    CFX_RectF rtImage(m_rtUI);
    rtImage.Deflate(rtUIMargin.left, rtUIMargin.top, rtUIMargin.width, rtUIMargin.height);
    m_pNormalWidget->SetWidgetRect(rtImage);
}
FX_BOOL	CXFA_FFImageEdit::CommitData()
{
    return TRUE;
}
FX_BOOL CXFA_FFImageEdit::UpdateFWLData()
{
    m_pDataAcc->SetImageEditImage(NULL);
    m_pDataAcc->LoadImageEditImage();
    return TRUE;
}
FX_INT32 CXFA_FFImageEdit::OnProcessMessage(CFWL_Message *pMessage)
{
    return m_pOldDelegate->OnProcessMessage(pMessage);
}
FWL_ERR	CXFA_FFImageEdit::OnProcessEvent(CFWL_Event *pEvent)
{
    CXFA_FFField::OnProcessEvent(pEvent);
    return m_pOldDelegate->OnProcessEvent(pEvent);
}
FWL_ERR	CXFA_FFImageEdit::OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    return m_pOldDelegate->OnDrawWidget(pGraphics, pMatrix);
}
