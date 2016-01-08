// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
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

static void XFA_FFDeleteWidgetAcc(void* pData) {
  delete static_cast<CXFA_WidgetAcc*>(pData);
}
static XFA_MAPDATABLOCKCALLBACKINFO gs_XFADeleteWidgetAcc = {
    XFA_FFDeleteWidgetAcc, NULL};
CXFA_FFNotify::CXFA_FFNotify(CXFA_FFDoc* pDoc) : m_pDoc(pDoc) {}
CXFA_FFNotify::~CXFA_FFNotify() {}
void CXFA_FFNotify::OnPageEvent(IXFA_LayoutPage* pSender,
                                XFA_PAGEEVENT eEvent,
                                void* pParam) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView(pSender->GetLayout());
  if (!pDocView) {
    return;
  }
  pDocView->OnPageEvent(pSender, eEvent, (int32_t)(uintptr_t)pParam);
}
void CXFA_FFNotify::OnNodeEvent(CXFA_Node* pSender,
                                XFA_NODEEVENT eEvent,
                                void* pParam,
                                void* pParam2,
                                void* pParam3,
                                void* pParam4) {
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
void CXFA_FFNotify::OnWidgetDataEvent(CXFA_WidgetData* pSender,
                                      FX_DWORD dwEvent,
                                      void* pParam,
                                      void* pAdditional,
                                      void* pAdditional2) {
  CXFA_WidgetAcc* pWidgetAcc = static_cast<CXFA_WidgetAcc*>(pSender);
  switch (dwEvent) {
    case XFA_WIDGETEVENT_ListItemAdded: {
      if (pWidgetAcc->GetUIType() != XFA_ELEMENT_ChoiceList) {
        return;
      }
      FX_BOOL bStaticNotify = pWidgetAcc->GetDocView()->IsStaticNotify();
      CXFA_FFWidget* pWidget = pWidgetAcc->GetNextWidget(NULL);
      if (!pWidget) {
        if (bStaticNotify) {
          pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent(
              pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemAdded, pParam,
              pAdditional);
        }
        return;
      }
      while (pWidget) {
        if (pWidget->IsLoaded()) {
          if (pWidgetAcc->IsListBox()) {
            static_cast<CXFA_FFListBox*>(pWidget)
                ->InsertItem((const CFX_WideStringC&)(const FX_WCHAR*)pParam,
                             (int32_t)(uintptr_t)pAdditional2);
          } else {
            static_cast<CXFA_FFComboBox*>(pWidget)
                ->InsertItem((const CFX_WideStringC&)(const FX_WCHAR*)pParam,
                             (int32_t)(uintptr_t)pAdditional2);
          }
        }
        if (bStaticNotify) {
          pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent(
              pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemAdded, pParam,
              pAdditional);
        }
        pWidget = pWidgetAcc->GetNextWidget(pWidget);
      }
    } break;
    case XFA_WIDGETEVENT_ListItemRemoved: {
      if (pWidgetAcc->GetUIType() != XFA_ELEMENT_ChoiceList) {
        return;
      }
      FX_BOOL bStaticNotify = pWidgetAcc->GetDocView()->IsStaticNotify();
      CXFA_FFWidget* pWidget = pWidgetAcc->GetNextWidget(NULL);
      if (!pWidget) {
        if (bStaticNotify) {
          pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent(
              pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemRemoved, pParam,
              pAdditional);
        }
        return;
      }
      while (pWidget) {
        if (pWidget->IsLoaded()) {
          if (pWidgetAcc->IsListBox()) {
            static_cast<CXFA_FFListBox*>(pWidget)
                ->DeleteItem((int32_t)(uintptr_t)pParam);
          } else {
            static_cast<CXFA_FFComboBox*>(pWidget)
                ->DeleteItem((int32_t)(uintptr_t)pParam);
          }
        }
        if (bStaticNotify) {
          pWidgetAcc->GetDoc()->GetDocProvider()->WidgetEvent(
              pWidget, pWidgetAcc, XFA_WIDGETEVENT_ListItemRemoved, pParam,
              pAdditional);
        }
        pWidget = pWidgetAcc->GetNextWidget(pWidget);
      }
    } break;
  }
}
CXFA_LayoutItem* CXFA_FFNotify::OnCreateLayoutItem(CXFA_Node* pNode) {
  IXFA_DocLayout* pLayout = m_pDoc->GetXFADoc()->GetDocLayout();
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView(pLayout);
  XFA_ELEMENT eType = pNode->GetClassID();
  if (eType == XFA_ELEMENT_PageArea) {
    return new CXFA_FFPageView(pDocView, pNode);
  }
  if (eType == XFA_ELEMENT_ContentArea) {
    return new CXFA_ContainerLayoutItem(pNode);
  }
  CXFA_WidgetAcc* pAcc = static_cast<CXFA_WidgetAcc*>(pNode->GetWidgetData());
  if (!pAcc) {
    return new CXFA_ContentLayoutItem(pNode);
  }
  CXFA_FFPageView* pPageView = NULL;
  CXFA_FFWidget* pWidget = NULL;
  switch (pAcc->GetUIType()) {
    case XFA_ELEMENT_Barcode:
      pWidget = new CXFA_FFBarcode(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Button:
      pWidget = new CXFA_FFPushButton(pPageView, pAcc);
      break;
    case XFA_ELEMENT_CheckButton:
      pWidget = new CXFA_FFCheckButton(pPageView, pAcc);
      break;
    case XFA_ELEMENT_ChoiceList: {
      if (pAcc->IsListBox()) {
        pWidget = new CXFA_FFListBox(pPageView, pAcc);
      } else {
        pWidget = new CXFA_FFComboBox(pPageView, pAcc);
      }
    } break;
    case XFA_ELEMENT_DateTimeEdit:
      pWidget = new CXFA_FFDateTimeEdit(pPageView, pAcc);
      break;
    case XFA_ELEMENT_ImageEdit:
      pWidget = new CXFA_FFImageEdit(pPageView, pAcc);
      break;
    case XFA_ELEMENT_NumericEdit:
      pWidget = new CXFA_FFNumericEdit(pPageView, pAcc);
      break;
    case XFA_ELEMENT_PasswordEdit:
      pWidget = new CXFA_FFPasswordEdit(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Signature:
      pWidget = new CXFA_FFSignature(pPageView, pAcc);
      break;
    case XFA_ELEMENT_TextEdit:
      pWidget = new CXFA_FFTextEdit(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Arc:
      pWidget = new CXFA_FFArc(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Line:
      pWidget = new CXFA_FFLine(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Rectangle:
      pWidget = new CXFA_FFRectangle(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Text:
      pWidget = new CXFA_FFText(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Image:
      pWidget = new CXFA_FFImage(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Draw:
      pWidget = new CXFA_FFDraw(pPageView, pAcc);
      break;
    case XFA_ELEMENT_Subform:
      pWidget = new CXFA_FFSubForm(pPageView, pAcc);
      break;
    case XFA_ELEMENT_ExclGroup:
      pWidget = new CXFA_FFExclGroup(pPageView, pAcc);
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
  return pWidget;
}
void CXFA_FFNotify::OnLayoutEvent(IXFA_DocLayout* pLayout,
                                  CXFA_LayoutItem* pSender,
                                  XFA_LAYOUTEVENT eEvent,
                                  void* pParam,
                                  void* pParam2) {
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
void CXFA_FFNotify::StartFieldDrawLayout(CXFA_Node* pItem,
                                         FX_FLOAT& fCalcWidth,
                                         FX_FLOAT& fCalcHeight) {
  CXFA_WidgetAcc* pAcc = static_cast<CXFA_WidgetAcc*>(pItem->GetWidgetData());
  if (!pAcc) {
    return;
  }
  pAcc->StartWidgetLayout(fCalcWidth, fCalcHeight);
}
FX_BOOL CXFA_FFNotify::FindSplitPos(CXFA_Node* pItem,
                                    int32_t iBlockIndex,
                                    FX_FLOAT& fCalcHeightPos) {
  CXFA_WidgetAcc* pAcc = static_cast<CXFA_WidgetAcc*>(pItem->GetWidgetData());
  if (!pAcc) {
    return FALSE;
  }
  return (XFA_LAYOUTRESULT)pAcc->FindSplitPos(iBlockIndex, fCalcHeightPos);
}
FX_BOOL CXFA_FFNotify::RunScript(CXFA_Node* pScript, CXFA_Node* pFormItem) {
  FX_BOOL bRet = FALSE;
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return bRet;
  }
  CXFA_WidgetAcc* pWidgetAcc =
      static_cast<CXFA_WidgetAcc*>(pFormItem->GetWidgetData());
  if (!pWidgetAcc) {
    return bRet;
  }
  CXFA_EventParam EventParam;
  EventParam.m_eType = XFA_EVENT_Unknown;
  FXJSE_HVALUE pRetValue = NULL;
  int32_t iRet =
      pWidgetAcc->ExecuteScript(CXFA_Script(pScript), &EventParam, &pRetValue);
  if (iRet == XFA_EVENTERROR_Sucess && pRetValue) {
    bRet = FXJSE_Value_ToBoolean(pRetValue);
    FXJSE_Value_Release(pRetValue);
  }
  return bRet;
}
int32_t CXFA_FFNotify::ExecEventByDeepFirst(CXFA_Node* pFormNode,
                                            XFA_EVENTTYPE eEventType,
                                            FX_BOOL bIsFormReady,
                                            FX_BOOL bRecursive,
                                            CXFA_WidgetAcc* pExclude) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return XFA_EVENTERROR_NotExist;
  }
  return pDocView->ExecEventActivityByDeepFirst(
      pFormNode, eEventType, bIsFormReady, bRecursive,
      pExclude ? pExclude->GetNode() : NULL);
}
void CXFA_FFNotify::AddCalcValidate(CXFA_Node* pNode) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  CXFA_WidgetAcc* pWidgetAcc =
      static_cast<CXFA_WidgetAcc*>(pNode->GetWidgetData());
  if (!pWidgetAcc) {
    return;
  }
  pDocView->AddCalculateWidgetAcc(pWidgetAcc);
  pDocView->AddValidateWidget(pWidgetAcc);
}
IXFA_Doc* CXFA_FFNotify::GetHDOC() {
  return m_pDoc;
}
IXFA_DocProvider* CXFA_FFNotify::GetDocProvider() {
  return m_pDoc->GetDocProvider();
}
IXFA_AppProvider* CXFA_FFNotify::GetAppProvider() {
  return m_pDoc->GetApp()->GetAppProvider();
}
IXFA_WidgetHandler* CXFA_FFNotify::GetWidgetHandler() {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  return pDocView ? pDocView->GetWidgetHandler() : NULL;
}
IXFA_Widget* CXFA_FFNotify::GetHWidget(CXFA_LayoutItem* pLayoutItem) {
  return XFA_GetWidgetFromLayoutItem(pLayoutItem);
}
void CXFA_FFNotify::OpenDropDownList(IXFA_Widget* hWidget) {
  CXFA_FFWidget* pWidget = static_cast<CXFA_FFWidget*>(hWidget);
  if (pWidget->GetDataAcc()->GetUIType() != XFA_ELEMENT_ChoiceList) {
    return;
  }
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  pDocView->LockUpdate();
  static_cast<CXFA_FFComboBox*>(pWidget)->OpenDropDownList();
  pDocView->UnlockUpdate();
  pDocView->UpdateDocView();
}
CFX_WideString CXFA_FFNotify::GetCurrentDateTime() {
  CFX_Unitime dataTime;
  dataTime.Now();
  CFX_WideString wsDateTime;
  wsDateTime.Format(L"%d%02d%02dT%02d%02d%02d", dataTime.GetYear(),
                    dataTime.GetMonth(), dataTime.GetDay(), dataTime.GetHour(),
                    dataTime.GetMinute(), dataTime.GetSecond());
  return wsDateTime;
}
void CXFA_FFNotify::ResetData(CXFA_WidgetData* pWidgetData) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  pDocView->ResetWidgetData(static_cast<CXFA_WidgetAcc*>(pWidgetData));
}
int32_t CXFA_FFNotify::GetLayoutStatus() {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  return pDocView ? pDocView->GetLayoutStatus() : 0;
}
void CXFA_FFNotify::RunNodeInitialize(CXFA_Node* pNode) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  pDocView->AddNewFormNode(pNode);
}
void CXFA_FFNotify::RunSubformIndexChange(CXFA_Node* pSubformNode) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  pDocView->AddIndexChangedSubform(pSubformNode);
}
CXFA_Node* CXFA_FFNotify::GetFocusWidgetNode() {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return NULL;
  }
  CXFA_WidgetAcc* pAcc = pDocView->GetFocusWidgetAcc();
  return pAcc ? pAcc->GetNode() : NULL;
}
void CXFA_FFNotify::SetFocusWidgetNode(CXFA_Node* pNode) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  CXFA_WidgetAcc* pAcc =
      pNode ? static_cast<CXFA_WidgetAcc*>(pNode->GetWidgetData()) : nullptr;
  pDocView->SetFocusWidgetAcc(pAcc);
}
void CXFA_FFNotify::OnNodeReady(CXFA_Node* pNode) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  XFA_ELEMENT iType = pNode->GetClassID();
  if (XFA_IsCreateWidget(iType)) {
    CXFA_WidgetAcc* pAcc =
        new CXFA_WidgetAcc(pDocView, static_cast<CXFA_Node*>(pNode));
    pNode->SetObject(XFA_ATTRIBUTE_WidgetData, pAcc, &gs_XFADeleteWidgetAcc);
    return;
  }
  switch (iType) {
    case XFA_ELEMENT_BindItems:
      pDocView->m_bindItems.Add(pNode);
      break;
    case XFA_ELEMENT_Validate: {
      pNode->SetFlag(XFA_NODEFLAG_NeedsInitApp, TRUE, FALSE);
    } break;
    default:
      break;
  }
}
void CXFA_FFNotify::OnValueChanging(CXFA_Node* pSender,
                                    void* pParam,
                                    void* pParam2) {
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
    XFA_ATTRIBUTE eAttr = (XFA_ATTRIBUTE)(uintptr_t)pParam;
    if (eAttr == XFA_ATTRIBUTE_Presence) {
      CXFA_WidgetAcc* pWidgetAcc =
          static_cast<CXFA_WidgetAcc*>(pSender->GetWidgetData());
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
void CXFA_FFNotify::OnValueChanged(CXFA_Node* pSender,
                                   void* pParam,
                                   void* pParam2,
                                   void* pParam3,
                                   void* pParam4) {
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  FX_DWORD dwPacket = pSender->GetPacketID();
  XFA_ATTRIBUTE eAttr = (XFA_ATTRIBUTE)(uintptr_t)pParam;
  if (dwPacket & XFA_XDPPACKET_Form) {
    CXFA_Node* pParentNode = static_cast<CXFA_Node*>(pParam3);
    CXFA_Node* pWidgetNode = static_cast<CXFA_Node*>(pParam4);
    XFA_ELEMENT ePType = pParentNode->GetClassID();
    FX_BOOL bIsContainerNode = pParentNode->IsContainerNode();
    CXFA_WidgetAcc* pWidgetAcc =
        static_cast<CXFA_WidgetAcc*>(pWidgetNode->GetWidgetData());
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
      } break;
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
        pWidgetAcc->NotifyEvent(XFA_WIDGETEVENT_AccessChanged, NULL, pParam2,
                                NULL);
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
        } else if (pWidgetNode->GetNodeItem(XFA_NODEITEM_Parent)
                       ->GetClassID() == XFA_ELEMENT_ExclGroup) {
          pWidgetAcc->UpdateUIDisplay();
        }
        if (bNotify) {
          pWidgetAcc->NotifyEvent(XFA_WIDGETEVENT_PostContentChanged, NULL,
                                  NULL, NULL);
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
      pWidget->PerformLayout();
      pWidget->AddInvalidateRect();
    }
  } else {
    if (eAttr == XFA_ATTRIBUTE_Value) {
      pDocView->AddCalculateNodeNotify(pSender);
    }
  }
}
void CXFA_FFNotify::OnChildAdded(CXFA_Node* pSender,
                                 void* pParam,
                                 void* pParam2) {
  if (!pSender->IsFormContainer()) {
    return;
  }
  CXFA_FFDocView* pDocView = m_pDoc->GetDocView();
  if (!pDocView) {
    return;
  }
  FX_BOOL bLayoutReady =
      !(pDocView->m_bInLayoutStatus) &&
      (pDocView->GetLayoutStatus() >= XFA_DOCVIEW_LAYOUTSTATUS_End);
  if (bLayoutReady) {
    m_pDoc->GetDocProvider()->SetChangeMark(m_pDoc);
  }
}
void CXFA_FFNotify::OnChildRemoved(CXFA_Node* pSender,
                                   void* pParam,
                                   void* pParam2) {
  if (CXFA_FFDocView* pDocView = m_pDoc->GetDocView()) {
    FX_BOOL bLayoutReady =
        !(pDocView->m_bInLayoutStatus) &&
        (pDocView->GetLayoutStatus() >= XFA_DOCVIEW_LAYOUTSTATUS_End);
    if (bLayoutReady) {
      m_pDoc->GetDocProvider()->SetChangeMark(m_pDoc);
    }
  }
}
void CXFA_FFNotify::OnLayoutItemAdd(CXFA_FFDocView* pDocView,
                                    IXFA_DocLayout* pLayout,
                                    CXFA_LayoutItem* pSender,
                                    void* pParam,
                                    void* pParam2) {
  CXFA_FFWidget* pWidget = static_cast<CXFA_FFWidget*>(pSender);
  int32_t iPageIdx = (int32_t)(uintptr_t)pParam;
  IXFA_PageView* pNewPageView = pDocView->GetPageView(iPageIdx);
  FX_DWORD dwStatus = (FX_DWORD)(uintptr_t)pParam2;
  FX_DWORD dwFilter = XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable |
                      XFA_WIDGETSTATUS_Printable;
  pWidget->ModifyStatus(dwStatus, dwFilter);
  if (pDocView->GetLayoutStatus() >= XFA_DOCVIEW_LAYOUTSTATUS_End) {
    IXFA_PageView* pPrePageView = pWidget->GetPageView();
    if (pPrePageView != pNewPageView ||
        (dwStatus & (XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable)) ==
            (XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable)) {
      pWidget->SetPageView(pNewPageView);
      m_pDoc->GetDocProvider()->WidgetEvent(pWidget, pWidget->GetDataAcc(),
                                            XFA_WIDGETEVENT_PostAdded,
                                            pNewPageView, pPrePageView);
    }
    if ((dwStatus & XFA_WIDGETSTATUS_Visible) == 0) {
      return;
    }
    if (pWidget->IsLoaded()) {
      CFX_RectF rtOld;
      pWidget->GetWidgetRect(rtOld);
      CFX_RectF rtNew = pWidget->ReCacheWidgetRect();
      if (rtOld != rtNew) {
        pWidget->PerformLayout();
      }
    } else {
      pWidget->LoadWidget();
    }
    pWidget->AddInvalidateRect(NULL);
  } else {
    pWidget->SetPageView(pNewPageView);
  }
}
void CXFA_FFNotify::OnLayoutItemRemoving(CXFA_FFDocView* pDocView,
                                         IXFA_DocLayout* pLayout,
                                         CXFA_LayoutItem* pSender,
                                         void* pParam,
                                         void* pParam2) {
  CXFA_FFWidget* pWidget = static_cast<CXFA_FFWidget*>(pSender);
  pDocView->DeleteLayoutItem(pWidget);
  if (pDocView->GetLayoutStatus() < XFA_DOCVIEW_LAYOUTSTATUS_End) {
    return;
  }
  m_pDoc->GetDocProvider()->WidgetEvent(pWidget, pWidget->GetDataAcc(),
                                        XFA_WIDGETEVENT_PreRemoved, NULL, NULL);
  pWidget->AddInvalidateRect(NULL);
}
void CXFA_FFNotify::OnLayoutItemRectChanged(CXFA_FFDocView* pDocView,
                                            IXFA_DocLayout* pLayout,
                                            CXFA_LayoutItem* pSender,
                                            void* pParam,
                                            void* pParam2) {
}
void CXFA_FFNotify::OnLayoutItemStatustChanged(CXFA_FFDocView* pDocView,
                                               IXFA_DocLayout* pLayout,
                                               CXFA_LayoutItem* pSender,
                                               void* pParam,
                                               void* pParam2) {
  CXFA_FFWidget* pWidget = static_cast<CXFA_FFWidget*>(pSender);
  if (!pWidget) {
    return;
  }
  FX_DWORD dwStatus = (FX_DWORD)(uintptr_t)pParam;
  if (dwStatus == 0) {
    CXFA_LayoutItem* pPreItem = pSender->GetPrev();
    if (pPreItem) {
      CXFA_FFWidget* pPreWidget = static_cast<CXFA_FFWidget*>(pPreItem);
      if (pPreWidget) {
        dwStatus = pPreWidget->GetStatus();
      }
    }
  }
  FX_DWORD dwOldStatus = pWidget->GetStatus();
  FX_DWORD dwFilter = XFA_WIDGETSTATUS_Visible | XFA_WIDGETSTATUS_Viewable |
                      XFA_WIDGETSTATUS_Printable;
  if ((dwOldStatus & dwFilter) == dwStatus) {
    return;
  }
  pWidget->ModifyStatus(dwStatus, dwFilter);
}
