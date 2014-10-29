// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_fwladapter.h"
#include "xfa_ffdocview.h"
#include "xfa_ffpageview.h"
#include "xfa_ffwidgethandler.h"
#include "xfa_ffdoc.h"
#include "xfa_ffwidget.h"
#include "xfa_fffield.h"
#include "xfa_ffpushbutton.h"
#include "xfa_ffcheckbutton.h"
#include "xfa_ffchoicelist.h"
#include "xfa_ffimageedit.h"
#include "xfa_fftextedit.h"
#include "xfa_ffbarcode.h"
#include "xfa_ffdraw.h"
#include "xfa_fftext.h"
#include "xfa_ffpath.h"
#include "xfa_ffimage.h"
#include "xfa_ffexclgroup.h"
#include "xfa_ffsubform.h"
#include "xfa_ffsignature.h"
#include "xfa_ffapp.h"
#include "xfa_textlayout.h"
#include "xfa_ffwidgetacc.h"
#include "xfa_ffnotify.h"
static void XFA_FFDeleteWidgetAcc(FX_LPVOID pData)
{
    if (pData) {
        delete (CXFA_WidgetAcc*)pData;
    }
}
static XFA_MAPDATABLOCKCALLBACKINFO gs_XFADeleteWidgetAcc = {XFA_FFDeleteWidgetAcc, NULL};
CXFA_FFNotify::CXFA_FFNotify(CXFA_FFDoc* pDoc)
    : m_pDoc(pDoc)
{
}
CXFA_FFNotify::~CXFA_FFNotify()
{
}
void CXFA_FFNotify::OnPageEvent(IXFA_LayoutPage *pSender, XFA_PAGEEVENT eEvent, FX_LPVOID pParam)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView(pSender->GetLayout());
    if (!pDocView) {
        return;
    }
    pDocView->OnPageEvent(pSender, eEvent, (FX_INT32)(FX_UINTPTR)pParam);
}
void CXFA_FFNotify::OnNodeEvent(CXFA_Node *pSender, XFA_NODEEVENT eEvent, FX_LPVOID pParam , FX_LPVOID pParam2 , FX_LPVOID pParam3, FX_LPVOID pParam4)
{
    switch (eEvent) {
        case XFA_NODEEVENT_Ready:
            OnNodeReady(pSender);
            break;
        case XFA_NODEEVENT_ValueChanging:
            OnValueChanging(pSender, pParam, pParam2);
            break;
        case XFA_NODEEVENT_ValueChanged:
            OnValueChanged(pSender, pParam, pParam2, pParam3, pParam4);
            break;
        case XFA_NODEEVENT_ChildAdded:
            OnChildAdded(pSender, pParam, pParam2);
            break;
        case XFA_NODEEVENT_ChildRemoved:
            OnChildRemoved(pSender, pParam, pParam2);
            break;
    }
}
void CXFA_FFNotify::OnWidgetDataEvent(CXFA_WidgetData* pSender, FX_DWORD dwEvent, FX_LPVOID pParam , FX_LPVOID pAdditional , FX_LPVOID pAdditional2 )
{
    CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pSender;
    switch (dwEvent) {
        case XFA_WIDGETEVENT_ListItemAdded: {
                if (pWidgetAcc->GetUIType() != XFA_ELEMENT_ChoiceList) {
                    return;
                }
                FX_BOOL bStaticNotify = pWidgetAcc->GetDocView()->IsStaticNotify();
                CXFA_FFWidget* pWidget = pWidgetAcc->GetNextWidget(NULL);
                if (!pWidget) {
                    if (bStaticNotify) {
                        pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent((XFA_HWIDGET)pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemAdded, pParam, pAdditional);
                    }
                    return;
                }
                while (pWidget) {
                    if (pWidget->IsLoaded()) {
                        if (pWidgetAcc->IsListBox()) {
                            ((CXFA_FFListBox*)pWidget)->InsertItem((FX_WSTR)(FX_LPCWSTR)pParam, (FX_INT32)(FX_UINTPTR)pAdditional2);
                        } else {
                            ((CXFA_FFComboBox*)pWidget)->InsertItem((FX_WSTR)(FX_LPCWSTR)pParam, (FX_INT32)(FX_UINTPTR)pAdditional2);
                        }
                    }
                    if (bStaticNotify) {
                        pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent((XFA_HWIDGET)pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemAdded, pParam, pAdditional);
                    }
                    pWidget = pWidgetAcc->GetNextWidget(pWidget);
                }
            }
            break;
        case XFA_WIDGETEVENT_ListItemRemoved: {
                if (pWidgetAcc->GetUIType() != XFA_ELEMENT_ChoiceList) {
                    return;
                }
                FX_BOOL bStaticNotify = pWidgetAcc->GetDocView()->IsStaticNotify();
                CXFA_FFWidget* pWidget = pWidgetAcc->GetNextWidget(NULL);
                if (!pWidget) {
                    if (bStaticNotify) {
                        pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent((XFA_HWIDGET)pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemRemoved, pParam, pAdditional);
                    }
                    return;
                }
                while (pWidget) {
                    if (pWidget->IsLoaded()) {
                        if (pWidgetAcc->IsListBox()) {
                            ((CXFA_FFListBox*)pWidget)->DeleteItem((FX_INT32)(FX_UINTPTR)pParam);
                        } else {
                            ((CXFA_FFComboBox*)pWidget)->DeleteItem((FX_INT32)(FX_UINTPTR)pParam);
                        }
                    }
                    if (bStaticNotify) {
                        pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent((XFA_HWIDGET)pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemRemoved, pParam, pAdditional);
                    }
                    pWidget = pWidgetAcc->GetNextWidget(pWidget);
                }
            }
            break;
    }
}
CXFA_LayoutItem* CXFA_FFNotify::OnCreateLayoutItem(CXFA_Node* pNode)
{
    IXFA_DocLayout* pLayout = m_pDoc->GetXFADoc()->GetDocLayout();
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView(pLayout);
    XFA_ELEMENT eType = pNode->GetClassID();
    if (eType == XFA_ELEMENT_PageArea) {
        return (CXFA_LayoutItem*)(FX_NEW CXFA_FFPageView(pDocView, pNode));
    } else if (eType == XFA_ELEMENT_ContentArea) {
        return (CXFA_LayoutItem*)(FX_NEW CXFA_ContainerLayoutItemImpl(pNode));
    }
    CXFA_WidgetAcc* pAcc = (CXFA_WidgetAcc*)pNode->GetWidgetData();
    if (!pAcc) {
        return (CXFA_LayoutItem*)(FX_NEW CXFA_ContentLayoutItemImpl(pNode));
    }
    CXFA_FFPageView* pPageView = NULL;
    CXFA_FFWidget* pWidget = NULL;
    switch(pAcc->GetUIType()) {
        case XFA_ELEMENT_Barcode:
            pWidget = FX_NEW CXFA_FFBarcode(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Button:
            pWidget = FX_NEW CXFA_FFPushButton(pPageView, pAcc);
            break;
        case XFA_ELEMENT_CheckButton:
            pWidget = FX_NEW CXFA_FFCheckButton(pPageView, pAcc);
            break;
        case XFA_ELEMENT_ChoiceList: {
                if (pAcc->IsListBox()) {
                    pWidget = FX_NEW CXFA_FFListBox(pPageView, pAcc);
                } else {
                    pWidget = FX_NEW CXFA_FFComboBox(pPageView, pAcc);
                }
            }
            break;
        case XFA_ELEMENT_DateTimeEdit:
            pWidget = FX_NEW CXFA_FFDateTimeEdit(pPageView, pAcc);
            break;
        case XFA_ELEMENT_ImageEdit:
            pWidget = FX_NEW CXFA_FFImageEdit(pPageView, pAcc);
            break;
        case XFA_ELEMENT_NumericEdit:
            pWidget = FX_NEW CXFA_FFNumericEdit(pPageView, pAcc);
            break;
        case XFA_ELEMENT_PasswordEdit:
            pWidget = FX_NEW CXFA_FFPasswordEdit(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Signature:
            pWidget = FX_NEW CXFA_FFSignature(pPageView, pAcc);
            break;
        case XFA_ELEMENT_TextEdit:
            pWidget = FX_NEW CXFA_FFTextEdit(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Arc:
            pWidget = FX_NEW CXFA_FFArc(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Line:
            pWidget = FX_NEW CXFA_FFLine(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Rectangle:
            pWidget = FX_NEW CXFA_FFRectangle(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Text:
            pWidget = FX_NEW CXFA_FFText(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Image:
            pWidget = FX_NEW CXFA_FFImage(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Draw:
            pWidget = FX_NEW CXFA_FFDraw(pPageView, pAcc);
            break;
        case XFA_ELEMENT_Subform:
            pWidget = FX_NEW CXFA_FFSubForm(pPageView, pAcc);
            break;
        case XFA_ELEMENT_ExclGroup:
            pWidget = FX_NEW CXFA_FFExclGroup(pPageView, pAcc);
            break;
        case XFA_ELEMENT_DefaultUi:
        default:
            pWidget = NULL;
            break;
    }
    if (!pWidget) {
        return NULL;
    }
    pWidget->SetDocView(pDocView);
    return pWidget->GetLayoutItem();
}
void CXFA_FFNotify::OnLayoutEvent(IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, XFA_LAYOUTEVENT eEvent, FX_LPVOID pParam , FX_LPVOID pParam2 )
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView(pLayout);
    if (!pDocView || !XFA_GetWidgetFromLayoutItem(pSender)) {
        return;
    }
    switch (eEvent) {
        case XFA_LAYOUTEVENT_ItemAdded:
            OnLayoutItemAdd(pDocView, pLayout, pSender, pParam, pParam2);
            break;
        case XFA_LAYOUTEVENT_ItemRemoving:
            OnLayoutItemRemoving(pDocView, pLayout, pSender, pParam, pParam2);
            break;
        case XFA_LAYOUTEVENT_RectChanged:
            OnLayoutItemRectChanged(pDocView, pLayout, pSender, pParam, pParam2);
            break;
        case XFA_LAYOUTEVENT_StatusChanged:
            OnLayoutItemStatustChanged(pDocView, pLayout, pSender, pParam, pParam2);
            break;
    }
}
void CXFA_FFNotify::StartFieldDrawLayout(CXFA_Node *pItem, FX_FLOAT &fCalcWidth, FX_FLOAT &fCalcHeight)
{
    CXFA_WidgetAcc *pAcc = (CXFA_WidgetAcc*)pItem->GetWidgetData();
    if (!pAcc) {
        return;
    }
    pAcc->StartWidgetLayout(fCalcWidth, fCalcHeight);
}
FX_BOOL CXFA_FFNotify::FindSplitPos(CXFA_Node *pItem, FX_INT32 iBlockIndex, FX_FLOAT &fCalcHeightPos)
{
    CXFA_WidgetAcc *pAcc = (CXFA_WidgetAcc*)pItem->GetWidgetData();
    if (!pAcc) {
        return FALSE;
    }
    return (XFA_LAYOUTRESULT)pAcc->FindSplitPos(iBlockIndex, fCalcHeightPos);
}
FX_BOOL CXFA_FFNotify::RunScript(CXFA_Node* pScript, CXFA_Node* pFormItem)
{
    FX_BOOL bRet = FALSE;
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return bRet;
    }
    CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pFormItem->GetWidgetData();
    if (!pWidgetAcc) {
        return bRet;
    }
    CXFA_EventParam EventParam;
    EventParam.m_eType = XFA_EVENT_Unknown;
    FXJSE_HVALUE pRetValue = NULL;
    FX_INT32 iRet = pWidgetAcc->ExecuteScript(CXFA_Script(pScript), &EventParam, &pRetValue);
    if (iRet == XFA_EVENTERROR_Sucess && pRetValue) {
        bRet = FXJSE_Value_ToBoolean(pRetValue);
        FXJSE_Value_Release(pRetValue);
    }
    return bRet;
}
FX_INT32 CXFA_FFNotify::ExecEventByDeepFirst(CXFA_Node* pFormNode, XFA_EVENTTYPE eEventType, FX_BOOL bIsFormReady, FX_BOOL bRecursive, CXFA_WidgetAcc* pExclude)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return XFA_EVENTERROR_NotExist;
    }
    return pDocView->ExecEventActivityByDeepFirst(pFormNode, eEventType, bIsFormReady, bRecursive, pExclude ? pExclude->GetNode() : NULL);
}
void CXFA_FFNotify::AddCalcValidate(CXFA_Node* pNode)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pNode->GetWidgetData();
    if (!pWidgetAcc) {
        return;
    }
    pDocView->AddCalculateWidgetAcc(pWidgetAcc);
    pDocView->AddValidateWidget(pWidgetAcc);
}
XFA_HDOC CXFA_FFNotify::GetHDOC()
{
    return (XFA_HDOC)m_pDoc;
}
IXFA_DocProvider* CXFA_FFNotify::GetDocProvider()
{
    return m_pDoc->GetDocProvider();
}
IXFA_AppProvider* CXFA_FFNotify::GetAppProvider()
{
    return m_pDoc->GetApp()->GetAppProvider();
}
IXFA_WidgetHandler* CXFA_FFNotify::GetWidgetHandler()
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    return pDocView ? pDocView->GetWidgetHandler() : NULL;
}
XFA_HWIDGET	CXFA_FFNotify::GetHWidget(CXFA_LayoutItem* pLayoutItem)
{
    return (XFA_HWIDGET)XFA_GetWidgetFromLayoutItem(pLayoutItem);
}
void CXFA_FFNotify::OpenDropDownList(XFA_HWIDGET hWidget)
{
    CXFA_FFWidget* pWidget = (CXFA_FFWidget*)hWidget;
    if (pWidget->GetDataAcc()->GetUIType() != XFA_ELEMENT_ChoiceList) {
        return;
    }
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    pDocView->LockUpdate();
    ((CXFA_FFComboBox*)pWidget)->OpenDropDownList();
    pDocView->UnlockUpdate();
    pDocView->UpdateDocView();
}
CFX_WideString CXFA_FFNotify::GetCurrentDateTime()
{
    CFX_Unitime dataTime;
    dataTime.Now();
    CFX_WideString wsDateTime;
    wsDateTime.Format((FX_LPCWSTR)L"%d%02d%02dT%02d%02d%02d", dataTime.GetYear(), dataTime.GetMonth(), dataTime.GetDay(), dataTime.GetHour(), dataTime.GetMinute(), dataTime.GetSecond());
    return wsDateTime;
}
void CXFA_FFNotify::ResetData(CXFA_WidgetData* pWidgetData)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    pDocView->ResetWidgetData((CXFA_WidgetAcc*)pWidgetData);
}
FX_INT32 CXFA_FFNotify::GetLayoutStatus()
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    return pDocView ? pDocView->GetLayoutStatus() : 0;
}
void CXFA_FFNotify::RunNodeInitialize(CXFA_Node* pNode)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    pDocView->AddNewFormNode(pNode);
}
void CXFA_FFNotify::RunSubformIndexChange(CXFA_Node* pSubformNode)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    pDocView->AddIndexChangedSubform(pSubformNode);
}
CXFA_Node* CXFA_FFNotify::GetFocusWidgetNode()
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return NULL;
    }
    CXFA_WidgetAcc* pAcc = pDocView->GetFocusWidgetAcc();
    return pAcc ? pAcc->GetNode() : NULL;
}
void CXFA_FFNotify::SetFocusWidgetNode(CXFA_Node* pNode)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    CXFA_WidgetAcc* pAcc = pNode ? (CXFA_WidgetAcc*)pNode->GetWidgetData() : NULL;
    pDocView->SetFocusWidgetAcc(pAcc);
}
void CXFA_FFNotify::OnNodeReady(CXFA_Node *pNode)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    XFA_ELEMENT iType = pNode->GetClassID();
    if (XFA_IsCreateWidget(iType)) {
        CXFA_WidgetAcc* pAcc = FX_NEW CXFA_WidgetAcc(pDocView, (CXFA_Node*)pNode);
        pNode->SetObject(XFA_ATTRIBUTE_WidgetData, pAcc, &gs_XFADeleteWidgetAcc);
        return;
    }
    switch (iType) {
        case XFA_ELEMENT_BindItems:
            pDocView->m_bindItems.Add(pNode);
            break;
        case XFA_ELEMENT_Validate: {
                pNode->SetFlag(XFA_NODEFLAG_NeedsInitApp, TRUE, FALSE);
            }
            break;
        default:
            break;
    }
}
void CXFA_FFNotify::OnValueChanging(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    if (pDocView->GetLayoutStatus() < XFA_DOCVIEW_LAYOUTSTATUS_End) {
        return;
    }
    FX_DWORD dwPacket = pSender->GetPacketID();
    if (dwPacket & XFA_XDPPACKET_Datasets) {
    } else if (pSender->IsFormContainer()) {
        XFA_ATTRIBUTE eAttr = (XFA_ATTRIBUTE)(FX_UINTPTR)pParam;
        if (eAttr == XFA_ATTRIBUTE_Presence) {
            CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pSender->GetWidgetData();
            if (!pWidgetAcc) {
                return;
            }
            CXFA_FFWidget* pWidget = NULL;
            while ((pWidget = pWidgetAcc->GetNextWidget(pWidget)) != NULL) {
                if (pWidget->IsLoaded()) {
                    pWidget->AddInvalidateRect();
                }
            }
        }
    }
}
void CXFA_FFNotify::OnValueChanged(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2, FX_LPVOID pParam3, FX_LPVOID pParam4)
{
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    FX_DWORD dwPacket = pSender->GetPacketID();
    XFA_ATTRIBUTE eAttr = (XFA_ATTRIBUTE)(FX_UINTPTR)pParam;
    if (dwPacket & XFA_XDPPACKET_Form) {
        CXFA_Node* pParentNode = (CXFA_Node*)pParam3;
        CXFA_Node* pWidgetNode = (CXFA_Node*)pParam4;
        XFA_ELEMENT ePType = pParentNode->GetClassID();
        FX_BOOL bIsContainerNode = pParentNode->IsContainerNode();
        CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pWidgetNode->GetWidgetData();
        if (!pWidgetAcc) {
            return;
        }
        FX_BOOL bUpdateProperty = FALSE;
        pDocView->SetChangeMark();
        switch (ePType) {
            case XFA_ELEMENT_Caption: {
                    CXFA_TextLayout* pCapOut = pWidgetAcc->GetCaptionTextLayout();
                    if (!pCapOut) {
                        return;
                    }
                    pCapOut->Unload();
                }
                break;
            case XFA_ELEMENT_Ui:
            case XFA_ELEMENT_Para:
                bUpdateProperty = TRUE;
                break;
            case XFA_ELEMENT_Font:
            case XFA_ELEMENT_Margin:
            case XFA_ELEMENT_Value:
            case XFA_ELEMENT_Items:
                break;
            default:
                break;
        }
        if (bIsContainerNode && eAttr == XFA_ATTRIBUTE_Access) {
            bUpdateProperty = TRUE;
            FX_BOOL bNotify = pDocView->IsStaticNotify();
            if (bNotify) {
                pWidgetAcc->NotifyEvent(XFA_WIDGETEVENT_AccessChanged, NULL, pParam2, NULL);
            }
        }
        if (eAttr == XFA_ATTRIBUTE_Value) {
            pDocView->AddCalculateNodeNotify(pSender);
            if (ePType == XFA_ELEMENT_Value || bIsContainerNode) {
                FX_BOOL bNotify = pDocView->IsStaticNotify();
                if (bIsContainerNode) {
                    pWidgetAcc->UpdateUIDisplay();
                    pDocView->AddCalculateWidgetAcc(pWidgetAcc);
                    pDocView->AddValidateWidget(pWidgetAcc);
                } else if (pWidgetNode->GetNodeItem(XFA_NODEITEM_Parent)->GetClassID() == XFA_ELEMENT_ExclGroup) {
                    pWidgetAcc->UpdateUIDisplay();
                }
                if (bNotify) {
                    pWidgetAcc->NotifyEvent(XFA_WIDGETEVENT_PostContentChanged, NULL, NULL, NULL);
                }
                return;
            }
        }
        CXFA_FFWidget* pWidget = NULL;
        while ((pWidget = pWidgetAcc->GetNextWidget(pWidget)) != NULL) {
            if (!pWidget->IsLoaded()) {
                continue;
            }
            if (bUpdateProperty) {
                pWidget->UpdateWidgetProperty();
            }
            pWidget->LayoutWidget();
            pWidget->AddInvalidateRect();
        }
    } else {
        if (eAttr == XFA_ATTRIBUTE_Value) {
            pDocView->AddCalculateNodeNotify(pSender);
        }
    }
}
void CXFA_FFNotify::OnChildAdded(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2)
{
    if (!pSender->IsFormContainer()) {
        return;
    }
    CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
    if (!pDocView) {
        return;
    }
    FX_BOOL bLayoutReady = !(pDocView->m_bInLayoutStatus) && (pDocView->GetLayoutStatus() >= XFA_DOCVIEW_LAYOUTSTATUS_End);
    if (bLayoutReady) {
        m_pDoc->GetDocProvider()->SetChangeMark((XFA_HDOC)m_pDoc);
    }
}
void CXFA_FFNotify::OnChildRemoved(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2)
{
    if (CXFA_FFDocView* pDocView = m_pDoc->GetDocView()) {
        FX_BOOL bLayoutReady = !(pDocView->m_bInLayoutStatus) && (pDocView->GetLayoutStatus() >= XFA_DOCVIEW_LAYOUTSTATUS_End);
        if (bLayoutReady) {
            m_pDoc->GetDocProvider()->SetChangeMark((XFA_HDOC)m_pDoc);
        }
    }
}
void CXFA_FFNotify::OnLayoutItemAdd(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2)
{
    CXFA_FFWidget* pWidget = (CXFA_FFWidget*)(CXFA_ContentLayoutItemImpl*)pSender;
    FX_INT32 iPageIdx = (FX_INT32)(FX_UINTPTR)pParam;
    IXFA_PageView* pNewPageView = pDocView->GetPageView(iPageIdx);
    FX_DWORD dwStatus = (FX_DWORD)(FX_UINTPTR)pParam2;
    FX_DWORD dwFilter = XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable | XFA_WIDGETSTATUS_Printable;
    pWidget->ModifyStatus(dwStatus, dwFilter);
    if (pDocView->GetLayoutStatus() >= XFA_DOCVIEW_LAYOUTSTATUS_End) {
        IXFA_PageView* pPrePageView = pWidget->GetPageView();
        if (pPrePageView != pNewPageView || (dwStatus & (XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable)) == (XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable)) {
            pWidget->SetPageView(pNewPageView);
            m_pDoc->GetDocProvider()->WidgetEvent((XFA_HWIDGET)pWidget, pWidget->GetDataAcc(), XFA_WIDGETEVENT_PostAdded, pNewPageView, pPrePageView);
        }
        if ((dwStatus & XFA_WIDGETSTATUS_Visible) == 0) {
            return;
        }
        if (pWidget->IsLoaded()) {
            CFX_RectF rtOld;
            pWidget->GetWidgetRect(rtOld);
            CFX_RectF rtNew = pWidget->ReCacheWidgetRect();
            if(rtOld != rtNew) {
                pWidget->LayoutWidget();
            }
        } else {
            pWidget->LoadWidget();
        }
        pWidget->AddInvalidateRect(NULL);
    } else {
        pWidget->SetPageView(pNewPageView);
    }
}
void CXFA_FFNotify::OnLayoutItemRemoving(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2)
{
    CXFA_FFWidget* pWidget = (CXFA_FFWidget*)pSender;
    pDocView->DeleteLayoutItem(pWidget);
    if (pDocView->GetLayoutStatus() < XFA_DOCVIEW_LAYOUTSTATUS_End) {
        return;
    }
    m_pDoc->GetDocProvider()->WidgetEvent((XFA_HWIDGET)pWidget, pWidget->GetDataAcc(), XFA_WIDGETEVENT_PreRemoved, NULL, NULL);
    pWidget->AddInvalidateRect(NULL);
}
void CXFA_FFNotify::OnLayoutItemRectChanged(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2)
{
}
void CXFA_FFNotify::OnLayoutItemStatustChanged(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2)
{
    CXFA_FFWidget* pWidget = (CXFA_FFWidget*)(CXFA_ContentLayoutItemImpl*)pSender;
    if (!pWidget) {
        return;
    }
    FX_DWORD dwStatus = (FX_DWORD)(FX_UINTPTR)pParam;
    if (dwStatus == 0) {
        CXFA_LayoutItem* pPreItem = pSender->GetPrev();
        if (pPreItem) {
            CXFA_FFWidget* pPreWidget = (CXFA_FFWidget*)(CXFA_ContentLayoutItemImpl*)pPreItem;
            if (pPreWidget) {
                dwStatus = pPreWidget->GetStatus();
            }
        }
    }
    FX_DWORD dwOldStatus = pWidget->GetStatus();
    FX_DWORD dwFilter = XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable | XFA_WIDGETSTATUS_Printable;
    if ((dwOldStatus & dwFilter) == dwStatus) {
        return;
    }
    pWidget->ModifyStatus(dwStatus, dwFilter);
}
