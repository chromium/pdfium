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
extern const XFA_ATTRIBUTEENUM gs_EventActivity[] = {
    XFA_ATTRIBUTEENUM_Click,      XFA_ATTRIBUTEENUM_Change,
    XFA_ATTRIBUTEENUM_DocClose,   XFA_ATTRIBUTEENUM_DocReady,
    XFA_ATTRIBUTEENUM_Enter,      XFA_ATTRIBUTEENUM_Exit,
    XFA_ATTRIBUTEENUM_Full,       XFA_ATTRIBUTEENUM_IndexChange,
    XFA_ATTRIBUTEENUM_Initialize, XFA_ATTRIBUTEENUM_MouseDown,
    XFA_ATTRIBUTEENUM_MouseEnter, XFA_ATTRIBUTEENUM_MouseExit,
    XFA_ATTRIBUTEENUM_MouseUp,    XFA_ATTRIBUTEENUM_PostExecute,
    XFA_ATTRIBUTEENUM_PostOpen,   XFA_ATTRIBUTEENUM_PostPrint,
    XFA_ATTRIBUTEENUM_PostSave,   XFA_ATTRIBUTEENUM_PostSign,
    XFA_ATTRIBUTEENUM_PostSubmit, XFA_ATTRIBUTEENUM_PreExecute,
    XFA_ATTRIBUTEENUM_PreOpen,    XFA_ATTRIBUTEENUM_PrePrint,
    XFA_ATTRIBUTEENUM_PreSave,    XFA_ATTRIBUTEENUM_PreSign,
    XFA_ATTRIBUTEENUM_PreSubmit,  XFA_ATTRIBUTEENUM_Ready,
    XFA_ATTRIBUTEENUM_Unknown,
};
CXFA_FFDocView::CXFA_FFDocView(CXFA_FFDoc* pDoc)
    : m_bLayoutEvent(FALSE),
      m_pListFocusWidget(nullptr),
      m_bInLayoutStatus(FALSE),
      m_pDoc(pDoc),
      m_pWidgetHandler(nullptr),
      m_pXFADocLayout(nullptr),
      m_pFocusAcc(nullptr),
      m_pFocusWidget(nullptr),
      m_pOldFocusWidget(nullptr),
      m_iStatus(XFA_DOCVIEW_LAYOUTSTATUS_None),
      m_iLock(0) {
}
CXFA_FFDocView::~CXFA_FFDocView() {
  DestroyDocView();
  if (m_pWidgetHandler) {
    delete m_pWidgetHandler;
  }
  m_pWidgetHandler = NULL;
}
void CXFA_FFDocView::InitLayout(CXFA_Node* pNode) {
  RunBindItems();
  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_Initialize);
  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_IndexChange);
}
int32_t CXFA_FFDocView::StartLayout(int32_t iStartPage) {
  m_iStatus = XFA_DOCVIEW_LAYOUTSTATUS_Start;
  m_pDoc->GetXFADoc()->DoProtoMerge();
  m_pDoc->GetXFADoc()->DoDataMerge();
  m_pXFADocLayout = GetXFALayout();
  int32_t iStatus = m_pXFADocLayout->StartLayout();
  if (iStatus < 0) {
    return iStatus;
  }
  CXFA_Node* pRootItem =
      (CXFA_Node*)m_pDoc->GetXFADoc()->GetXFANode(XFA_HASHCODE_Form);
  if (!pRootItem) {
    return iStatus;
  }
  InitLayout(pRootItem);
  InitCalculate(pRootItem);
  InitValidate(pRootItem);
  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready, TRUE);
  m_iStatus = XFA_DOCVIEW_LAYOUTSTATUS_Start;
  return iStatus;
}
int32_t CXFA_FFDocView::DoLayout(IFX_Pause* pPause) {
  int32_t iStatus = 100;
  iStatus = m_pXFADocLayout->DoLayout(pPause);
  if (iStatus != 100) {
    return iStatus;
  }
  m_iStatus = XFA_DOCVIEW_LAYOUTSTATUS_Doing;
  return iStatus;
}
void CXFA_FFDocView::StopLayout() {
  CXFA_Node* pRootItem =
      (CXFA_Node*)m_pDoc->GetXFADoc()->GetXFANode(XFA_HASHCODE_Form);
  if (!pRootItem) {
    return;
  }
  CXFA_Node* pSubformNode = pRootItem->GetChild(0, XFA_ELEMENT_Subform);
  if (!pSubformNode) {
    return;
  }
  CXFA_Node* pPageSetNode =
      pSubformNode->GetFirstChildByClass(XFA_ELEMENT_PageSet);
  if (!pPageSetNode) {
    return;
  }
  RunCalculateWidgets();
  RunValidate();
  InitLayout(pPageSetNode);
  InitCalculate(pPageSetNode);
  InitValidate(pPageSetNode);
  ExecEventActivityByDeepFirst(pPageSetNode, XFA_EVENT_Ready, TRUE);
  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready);
  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_DocReady);
  RunCalculateWidgets();
  RunValidate();
  if (RunLayout()) {
    ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready);
  }
  m_CalculateAccs.RemoveAll();
  if (m_pFocusAcc && !m_pFocusWidget) {
    SetFocusWidgetAcc(m_pFocusAcc);
  }
  m_iStatus = XFA_DOCVIEW_LAYOUTSTATUS_End;
}
int32_t CXFA_FFDocView::GetLayoutStatus() {
  return m_iStatus;
}
void CXFA_FFDocView::ShowNullTestMsg() {
  int32_t iCount = m_arrNullTestMsg.GetSize();
  CXFA_FFApp* pApp = m_pDoc->GetApp();
  IXFA_AppProvider* pAppProvider = pApp->GetAppProvider();
  if (pAppProvider && iCount) {
    int32_t iRemain = iCount > 7 ? iCount - 7 : 0;
    iCount -= iRemain;
    CFX_WideString wsMsg;
    for (int32_t i = 0; i < iCount; i++) {
      wsMsg += m_arrNullTestMsg[i] + FX_WSTRC(L"\n");
    }
    if (iRemain > 0) {
      CFX_WideString wsLimit;
      pAppProvider->LoadString(XFA_IDS_ValidateLimit, wsLimit);
      if (!wsLimit.IsEmpty()) {
        CFX_WideString wsTemp;
        wsTemp.Format((const FX_WCHAR*)wsLimit, iRemain);
        wsMsg += FX_WSTRC(L"\n") + wsTemp;
      }
    }
    CFX_WideString wsTitle;
    pAppProvider->LoadString(XFA_IDS_AppName, wsTitle);
    pAppProvider->MsgBox(wsMsg, wsTitle, XFA_MBICON_Status, XFA_MB_OK);
  }
  m_arrNullTestMsg.RemoveAll();
}
void CXFA_FFDocView::UpdateDocView() {
  if (IsUpdateLocked()) {
    return;
  }
  LockUpdate();
  int32_t iNewAdds = m_NewAddedNodes.GetSize();
  for (int32_t i = 0; i < iNewAdds; i++) {
    CXFA_Node* pNode = (CXFA_Node*)m_NewAddedNodes[i];
    InitCalculate(pNode);
    InitValidate(pNode);
    ExecEventActivityByDeepFirst(pNode, XFA_EVENT_Ready, TRUE);
  }
  m_NewAddedNodes.RemoveAll();
  this->RunSubformIndexChange();
  this->RunCalculateWidgets();
  this->RunValidate();
  ShowNullTestMsg();
  m_iStatus = XFA_DOCVIEW_LAYOUTSTATUS_Next;
  if (RunLayout() && m_bLayoutEvent) {
    RunEventLayoutReady();
  }
  m_bLayoutEvent = FALSE;
  m_CalculateAccs.RemoveAll();
  this->RunInvalidate();
  UnlockUpdate();
}
int32_t CXFA_FFDocView::CountPageViews() {
  if (!m_pXFADocLayout) {
    return 0;
  }
  return m_pXFADocLayout->CountPages();
}
IXFA_PageView* CXFA_FFDocView::GetPageView(int32_t nIndex) {
  if (!m_pXFADocLayout) {
    return NULL;
  }
  return static_cast<CXFA_FFPageView*>(m_pXFADocLayout->GetPage(nIndex));
}
IXFA_Widget* CXFA_FFDocView::GetWidgetByName(const CFX_WideStringC& wsName) {
  return GetWidgetByName(wsName, NULL);
}
CXFA_WidgetAcc* CXFA_FFDocView::GetWidgetAccByName(
    const CFX_WideStringC& wsName) {
  return GetWidgetAccByName(wsName, NULL);
}
IXFA_DocLayout* CXFA_FFDocView::GetXFALayout() const {
  return m_pDoc->GetXFADoc()->GetDocLayout();
}
FX_BOOL CXFA_FFDocView::ResetSingleWidgetAccData(CXFA_WidgetAcc* pWidgetAcc) {
  CXFA_Node* pNode = pWidgetAcc->GetNode();
  XFA_ELEMENT eType = pNode->GetClassID();
  if (eType != XFA_ELEMENT_Field && eType != XFA_ELEMENT_ExclGroup) {
    return FALSE;
  }
  FX_BOOL bNotify = IsStaticNotify();
  pWidgetAcc->ResetData();
  pWidgetAcc->UpdateUIDisplay();
  if (bNotify) {
    pWidgetAcc->NotifyEvent(XFA_WIDGETEVENT_PostContentChanged, NULL, NULL,
                            NULL);
  }
  if (CXFA_Validate validate = pWidgetAcc->GetValidate()) {
    AddValidateWidget(pWidgetAcc);
    ((CXFA_Node*)validate)->SetFlag(XFA_NODEFLAG_NeedsInitApp, TRUE, FALSE);
  }
  return TRUE;
}
void CXFA_FFDocView::ResetWidgetData(CXFA_WidgetAcc* pWidgetAcc) {
  m_bLayoutEvent = TRUE;
  FX_BOOL bChanged = FALSE;
  CXFA_Node* pFormNode = NULL;
  if (pWidgetAcc) {
    bChanged = ResetSingleWidgetAccData(pWidgetAcc);
    pFormNode = pWidgetAcc->GetNode();
  } else {
    pFormNode = GetRootSubform();
  }
  if (!pFormNode) {
    return;
  }
  if (pFormNode->GetClassID() != XFA_ELEMENT_Field &&
      pFormNode->GetClassID() != XFA_ELEMENT_ExclGroup) {
    CXFA_WidgetAccIterator Iterator(this, pFormNode);
    while (CXFA_WidgetAcc* pAcc = Iterator.MoveToNext()) {
      bChanged |= ResetSingleWidgetAccData(pAcc);
      if (pAcc->GetNode()->GetClassID() == XFA_ELEMENT_ExclGroup) {
        Iterator.SkipTree();
      }
    }
  }
  if (bChanged) {
    m_pDoc->GetDocProvider()->SetChangeMark(m_pDoc);
  }
}
int32_t CXFA_FFDocView::ProcessWidgetEvent(CXFA_EventParam* pParam,
                                           CXFA_WidgetAcc* pWidgetAcc) {
  if (pParam == NULL) {
    return XFA_EVENTERROR_Error;
  }
  if (pParam->m_eType == XFA_EVENT_Validate) {
    CFX_WideString wsValidateStr = FX_WSTRC(L"preSubmit");
    CXFA_Node* pConfigItem =
        (CXFA_Node*)m_pDoc->GetXFADoc()->GetXFANode(XFA_HASHCODE_Config);
    if (pConfigItem) {
      CXFA_Node* pValidateNode = NULL;
      CXFA_Node* pAcrobatNode = pConfigItem->GetChild(0, XFA_ELEMENT_Acrobat);
      pValidateNode =
          pAcrobatNode ? pAcrobatNode->GetChild(0, XFA_ELEMENT_Validate) : NULL;
      if (!pValidateNode) {
        CXFA_Node* pPresentNode = pConfigItem->GetChild(0, XFA_ELEMENT_Present);
        pValidateNode = pPresentNode
                            ? pPresentNode->GetChild(0, XFA_ELEMENT_Validate)
                            : NULL;
      }
      if (pValidateNode) {
        wsValidateStr = pValidateNode->GetContent();
      }
    }
    FX_BOOL bValidate = FALSE;
    switch (pParam->m_iValidateActivities) {
      case XFA_VALIDATE_preSubmit:
        bValidate = wsValidateStr.Find(L"preSubmit") != -1;
        break;
      case XFA_VALIDATE_prePrint:
        bValidate = wsValidateStr.Find(L"prePrint") != -1;
        break;
      case XFA_VALIDATE_preExecute:
        bValidate = wsValidateStr.Find(L"preExecute") != -1;
        break;
      case XFA_VALIDATE_preSave:
        bValidate = wsValidateStr.Find(L"preSave") != -1;
        break;
    }
    if (!bValidate) {
      return XFA_EVENTERROR_Sucess;
    }
  }
  CXFA_Node* pNode = pWidgetAcc ? pWidgetAcc->GetNode() : NULL;
  if (!pNode) {
    CXFA_Node* pRootItem =
        (CXFA_Node*)m_pDoc->GetXFADoc()->GetXFANode(XFA_HASHCODE_Form);
    if (!pRootItem) {
      return XFA_EVENTERROR_Error;
    }
    pNode = pRootItem->GetChild(0, XFA_ELEMENT_Subform);
  }
  ExecEventActivityByDeepFirst(pNode, pParam->m_eType, pParam->m_bIsFormReady);
  return XFA_EVENTERROR_Sucess;
}
IXFA_WidgetHandler* CXFA_FFDocView::GetWidgetHandler() {
  if (!m_pWidgetHandler) {
    m_pWidgetHandler = new CXFA_FFWidgetHandler(this);
  }
  return m_pWidgetHandler;
}
IXFA_WidgetIterator* CXFA_FFDocView::CreateWidgetIterator() {
  CXFA_Node* pFormRoot = GetRootSubform();
  if (!pFormRoot) {
    return NULL;
  }
  return new CXFA_FFDocWidgetIterator(this, pFormRoot);
}
IXFA_WidgetAccIterator* CXFA_FFDocView::CreateWidgetAccIterator(
    XFA_WIDGETORDER eOrder) {
  CXFA_Node* pFormRoot = GetRootSubform();
  if (!pFormRoot) {
    return NULL;
  }
  return new CXFA_WidgetAccIterator(this, pFormRoot);
}
IXFA_Widget* CXFA_FFDocView::GetFocusWidget() {
  return m_pFocusWidget;
}
void CXFA_FFDocView::KillFocus() {
  if (m_pFocusWidget &&
      (m_pFocusWidget->GetStatus() & XFA_WIDGETSTATUS_Focused)) {
    (m_pFocusWidget)->OnKillFocus(NULL);
  }
  m_pFocusAcc = NULL;
  m_pFocusWidget = NULL;
  m_pOldFocusWidget = NULL;
}
FX_BOOL CXFA_FFDocView::SetFocus(IXFA_Widget* hWidget) {
  CXFA_FFWidget* pNewFocus = (CXFA_FFWidget*)hWidget;
  if (m_pOldFocusWidget == pNewFocus) {
    return FALSE;
  }
  CXFA_FFWidget* pOldFocus = m_pOldFocusWidget;
  m_pOldFocusWidget = pNewFocus;
  if (pOldFocus) {
    if (m_pFocusWidget != m_pOldFocusWidget &&
        (pOldFocus->GetStatus() & XFA_WIDGETSTATUS_Focused)) {
      m_pFocusWidget = pOldFocus;
      pOldFocus->OnKillFocus(pNewFocus);
    } else if ((pOldFocus->GetStatus() & XFA_WIDGETSTATUS_Visible)) {
      if (!pOldFocus->IsLoaded()) {
        pOldFocus->LoadWidget();
      }
      pOldFocus->OnSetFocus(m_pFocusWidget);
      m_pFocusWidget = pOldFocus;
      pOldFocus->OnKillFocus(pNewFocus);
    }
  }
  if (m_pFocusWidget == m_pOldFocusWidget) {
    return FALSE;
  }
  pNewFocus = m_pOldFocusWidget;
  if (m_pListFocusWidget && pNewFocus == m_pListFocusWidget) {
    m_pFocusAcc = NULL;
    m_pFocusWidget = NULL;
    m_pListFocusWidget = NULL;
    m_pOldFocusWidget = NULL;
    return FALSE;
  }
  if (pNewFocus && (pNewFocus->GetStatus() & XFA_WIDGETSTATUS_Visible)) {
    if (!pNewFocus->IsLoaded()) {
      pNewFocus->LoadWidget();
    }
    pNewFocus->OnSetFocus(m_pFocusWidget);
  }
  m_pFocusAcc = pNewFocus ? pNewFocus->GetDataAcc() : NULL;
  m_pFocusWidget = pNewFocus;
  m_pOldFocusWidget = m_pFocusWidget;
  return TRUE;
}
CXFA_WidgetAcc* CXFA_FFDocView::GetFocusWidgetAcc() {
  return m_pFocusAcc;
}
void CXFA_FFDocView::SetFocusWidgetAcc(CXFA_WidgetAcc* pWidgetAcc) {
  CXFA_FFWidget* pNewFocus =
      pWidgetAcc ? pWidgetAcc->GetNextWidget(NULL) : NULL;
  if (SetFocus(pNewFocus)) {
    m_pFocusAcc = pWidgetAcc;
    if (m_iStatus >= XFA_DOCVIEW_LAYOUTSTATUS_End) {
      m_pDoc->GetDocProvider()->SetFocusWidget(m_pDoc, m_pFocusWidget);
    }
  }
}
void CXFA_FFDocView::DeleteLayoutItem(CXFA_FFWidget* pWidget) {
  if (m_pFocusAcc == pWidget->GetDataAcc()) {
    m_pFocusAcc = NULL;
    m_pFocusWidget = NULL;
    m_pOldFocusWidget = NULL;
  }
}
static int32_t XFA_ProcessEvent(CXFA_FFDocView* pDocView,
                                CXFA_WidgetAcc* pWidgetAcc,
                                CXFA_EventParam* pParam) {
  if (!pParam || pParam->m_eType == XFA_EVENT_Unknown) {
    return XFA_EVENTERROR_NotExist;
  }
  if (!pWidgetAcc || pWidgetAcc->GetClassID() == XFA_ELEMENT_Draw) {
    return XFA_EVENTERROR_NotExist;
  }
  switch (pParam->m_eType) {
    case XFA_EVENT_Calculate:
      return pWidgetAcc->ProcessCalculate();
    case XFA_EVENT_Validate:
      if (((CXFA_FFDoc*)pDocView->GetDoc())
              ->GetDocProvider()
              ->IsValidationsEnabled(pDocView->GetDoc())) {
        return pWidgetAcc->ProcessValidate(0x01);
      }
      return XFA_EVENTERROR_Disabled;
    case XFA_EVENT_InitCalculate: {
      CXFA_Calculate calc = pWidgetAcc->GetCalculate();
      if (!calc) {
        return XFA_EVENTERROR_NotExist;
      }
      if (pWidgetAcc->GetNode()->HasFlag(XFA_NODEFLAG_UserInteractive)) {
        return XFA_EVENTERROR_Disabled;
      }
      CXFA_Script script = calc.GetScript();
      return pWidgetAcc->ExecuteScript(script, pParam);
    }
    default:
      break;
  }
  int32_t iRet =
      pWidgetAcc->ProcessEvent(gs_EventActivity[pParam->m_eType], pParam);
  return iRet;
}
int32_t CXFA_FFDocView::ExecEventActivityByDeepFirst(CXFA_Node* pFormNode,
                                                     XFA_EVENTTYPE eEventType,
                                                     FX_BOOL bIsFormReady,
                                                     FX_BOOL bRecursive,
                                                     CXFA_Node* pExclude) {
  int32_t iRet = XFA_EVENTERROR_NotExist;
  if (pFormNode == pExclude) {
    return iRet;
  }
  XFA_ELEMENT elementType = pFormNode->GetClassID();
  if (elementType == XFA_ELEMENT_Field) {
    if (eEventType == XFA_EVENT_IndexChange) {
      return iRet;
    }
    CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pFormNode->GetWidgetData();
    if (pWidgetAcc == NULL) {
      return iRet;
    }
    CXFA_EventParam eParam;
    eParam.m_eType = eEventType;
    eParam.m_pTarget = pWidgetAcc;
    eParam.m_bIsFormReady = bIsFormReady;
    return XFA_ProcessEvent(this, pWidgetAcc, &eParam);
  }
  if (bRecursive) {
    for (CXFA_Node* pNode = pFormNode->GetNodeItem(
             XFA_NODEITEM_FirstChild, XFA_OBJECTTYPE_ContainerNode);
         pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling,
                                           XFA_OBJECTTYPE_ContainerNode)) {
      elementType = pNode->GetClassID();
      if (elementType != XFA_ELEMENT_Variables &&
          elementType != XFA_ELEMENT_Draw) {
        iRet |= ExecEventActivityByDeepFirst(pNode, eEventType, bIsFormReady,
                                             bRecursive, pExclude);
      }
    }
  }
  CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pFormNode->GetWidgetData();
  if (pWidgetAcc == NULL) {
    return iRet;
  }
  CXFA_EventParam eParam;
  eParam.m_eType = eEventType;
  eParam.m_pTarget = pWidgetAcc;
  eParam.m_bIsFormReady = bIsFormReady;
  iRet |= XFA_ProcessEvent(this, pWidgetAcc, &eParam);
  return iRet;
}
CXFA_FFWidget* CXFA_FFDocView::GetWidgetByName(const CFX_WideStringC& wsName,
                                               CXFA_FFWidget* pRefWidget) {
  CXFA_WidgetAcc* pRefAcc = pRefWidget ? pRefWidget->GetDataAcc() : NULL;
  if (CXFA_WidgetAcc* pAcc = GetWidgetAccByName(wsName, pRefAcc)) {
    return pAcc->GetNextWidget(NULL);
  }
  return NULL;
}
CXFA_WidgetAcc* CXFA_FFDocView::GetWidgetAccByName(
    const CFX_WideStringC& wsName,
    CXFA_WidgetAcc* pRefWidgetAcc) {
  CFX_WideString wsExpression;
  FX_DWORD dwStyle = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                     XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent;
  IXFA_ScriptContext* pScriptContext = m_pDoc->GetXFADoc()->GetScriptContext();
  if (!pScriptContext) {
    return NULL;
  }
  CXFA_Node* refNode = NULL;
  if (pRefWidgetAcc != NULL) {
    refNode = pRefWidgetAcc->GetNode();
    wsExpression = wsName;
  } else {
    wsExpression = L"$form." + wsName;
  }
  XFA_RESOLVENODE_RS resoveNodeRS;
  int32_t iRet = pScriptContext->ResolveObjects(refNode, wsExpression,
                                                resoveNodeRS, dwStyle);
  if (iRet < 1) {
    return NULL;
  }
  if (resoveNodeRS.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    CXFA_Object* pNode = resoveNodeRS.nodes[0];
    if (pNode->IsNode()) {
      return (CXFA_WidgetAcc*)((CXFA_Node*)pNode)->GetWidgetData();
    }
  }
  return NULL;
}
void CXFA_FFDocView::OnPageEvent(IXFA_LayoutPage* pSender,
                                 XFA_PAGEEVENT eEvent,
                                 int32_t iPageIndex) {
  FX_BOOL bNofify = m_iStatus >= XFA_DOCVIEW_LAYOUTSTATUS_End;
  CXFA_FFPageView* pFFPageView = static_cast<CXFA_FFPageView*>(pSender);
  if (eEvent == XFA_PAGEEVENT_PageRemoved) {
    if (bNofify) {
      m_pDoc->GetDocProvider()->PageViewEvent(pFFPageView,
                                              XFA_PAGEVIEWEVENT_PostRemoved);
    }
  } else if (eEvent == XFA_PAGEEVENT_PageAdded) {
    if (bNofify) {
      m_pDoc->GetDocProvider()->PageViewEvent(pFFPageView,
                                              XFA_PAGEVIEWEVENT_PostAdded);
      pFFPageView->LoadPageView();
    }
  }
}
void CXFA_FFDocView::LockUpdate() {
  m_iLock++;
}
void CXFA_FFDocView::UnlockUpdate() {
  m_iLock--;
}
FX_BOOL CXFA_FFDocView::IsUpdateLocked() {
  return m_iLock;
}
void CXFA_FFDocView::ClearInvalidateList() {
  FX_POSITION ps = m_mapPageInvalidate.GetStartPosition();
  while (ps) {
    void* pPageView = NULL;
    CFX_RectF* pRect = NULL;
    m_mapPageInvalidate.GetNextAssoc(ps, pPageView, (void*&)pRect);
    delete pRect;
  }
  m_mapPageInvalidate.RemoveAll();
}
void CXFA_FFDocView::AddInvalidateRect(CXFA_FFWidget* pWidget,
                                       const CFX_RectF& rtInvalidate) {
  AddInvalidateRect(pWidget->GetPageView(), rtInvalidate);
}
void CXFA_FFDocView::AddInvalidateRect(IXFA_PageView* pPageView,
                                       const CFX_RectF& rtInvalidate) {
  CFX_RectF* pRect = (CFX_RectF*)m_mapPageInvalidate.GetValueAt(pPageView);
  if (!pRect) {
    pRect = new CFX_RectF;
    pRect->Set(rtInvalidate.left, rtInvalidate.top, rtInvalidate.width,
               rtInvalidate.height);
    m_mapPageInvalidate.SetAt(pPageView, pRect);
  } else {
    pRect->Union(rtInvalidate);
  }
}
void CXFA_FFDocView::RunInvalidate() {
  FX_POSITION ps = m_mapPageInvalidate.GetStartPosition();
  while (ps) {
    IXFA_PageView* pPageView = NULL;
    CFX_RectF* pRect = NULL;
    m_mapPageInvalidate.GetNextAssoc(ps, (void*&)pPageView, (void*&)pRect);
    m_pDoc->GetDocProvider()->InvalidateRect(pPageView, *pRect);
    delete pRect;
  }
  m_mapPageInvalidate.RemoveAll();
}
FX_BOOL CXFA_FFDocView::RunLayout() {
  LockUpdate();
  m_bInLayoutStatus = TRUE;
  if (!m_pXFADocLayout->IncrementLayout() &&
      m_pXFADocLayout->StartLayout() < 100) {
    m_pXFADocLayout->DoLayout();
    UnlockUpdate();
    m_bInLayoutStatus = FALSE;
    return TRUE;
  }
  m_bInLayoutStatus = FALSE;
  UnlockUpdate();
  return FALSE;
}
void CXFA_FFDocView::RunSubformIndexChange() {
  int32_t iSubforms = m_IndexChangedSubforms.GetSize();
  for (int32_t i = 0; i < iSubforms; i++) {
    CXFA_Node* pSubformNode = (CXFA_Node*)m_IndexChangedSubforms[i];
    CXFA_WidgetAcc* pWidgetAcc = (CXFA_WidgetAcc*)pSubformNode->GetWidgetData();
    if (!pWidgetAcc) {
      continue;
    }
    CXFA_EventParam eParam;
    eParam.m_eType = XFA_EVENT_IndexChange;
    eParam.m_pTarget = pWidgetAcc;
    pWidgetAcc->ProcessEvent(XFA_ATTRIBUTEENUM_IndexChange, &eParam);
  }
  m_IndexChangedSubforms.RemoveAll();
}
void CXFA_FFDocView::AddNewFormNode(CXFA_Node* pNode) {
  m_NewAddedNodes.Add(pNode);
  this->InitLayout(pNode);
}
void CXFA_FFDocView::AddIndexChangedSubform(CXFA_Node* pNode) {
  FXSYS_assert(pNode->GetClassID() == XFA_ELEMENT_Subform);
  m_IndexChangedSubforms.Add(pNode);
}
void CXFA_FFDocView::RunDocClose() {
  CXFA_Node* pRootItem =
      (CXFA_Node*)m_pDoc->GetXFADoc()->GetXFANode(XFA_HASHCODE_Form);
  if (!pRootItem) {
    return;
  }
  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_DocClose);
}
void CXFA_FFDocView::DestroyDocView() {
  ClearInvalidateList();
  m_iStatus = XFA_DOCVIEW_LAYOUTSTATUS_None;
  m_iLock = 0;
  m_ValidateAccs.RemoveAll();
  m_bindItems.RemoveAll();
  m_CalculateAccs.RemoveAll();
}
FX_BOOL CXFA_FFDocView::IsStaticNotify() {
  return m_pDoc->GetDocType() == XFA_DOCTYPE_Static;
}
void CXFA_FFDocView::AddCalculateWidgetAcc(CXFA_WidgetAcc* pWidgetAcc) {
  int32_t iAccs = m_CalculateAccs.GetSize();
  CXFA_WidgetAcc* pCurrentAcc =
      (iAccs < 1) ? (CXFA_WidgetAcc*)NULL
                  : (CXFA_WidgetAcc*)m_CalculateAccs[iAccs - 1];
  if (pCurrentAcc != pWidgetAcc) {
    m_CalculateAccs.Add(pWidgetAcc);
  }
}
void CXFA_FFDocView::AddCalculateNodeNotify(CXFA_Node* pNodeChange) {
  CXFA_CalcData* pGlobalData =
      (CXFA_CalcData*)pNodeChange->GetUserData(XFA_CalcData);
  int32_t iCount = pGlobalData ? pGlobalData->m_Globals.GetSize() : 0;
  for (int32_t i = 0; i < iCount; i++) {
    CXFA_WidgetAcc* pResultAcc = (CXFA_WidgetAcc*)pGlobalData->m_Globals[i];
    if (pResultAcc->GetNode()->HasFlag(XFA_NODEFLAG_HasRemoved)) {
      continue;
    }
    int32_t iAccs = m_CalculateAccs.GetSize();
    CXFA_WidgetAcc* pCurrentAcc =
        (iAccs < 1) ? (CXFA_WidgetAcc*)NULL
                    : (CXFA_WidgetAcc*)m_CalculateAccs[iAccs - 1];
    if (pCurrentAcc != pResultAcc) {
      m_CalculateAccs.Add(pResultAcc);
    }
  }
}
void CXFA_FFDocView::RunCalculateRecursive(int32_t& iIndex) {
  while (iIndex < m_CalculateAccs.GetSize()) {
    CXFA_WidgetAcc* pCurAcc = (CXFA_WidgetAcc*)m_CalculateAccs[iIndex];
    AddCalculateNodeNotify(pCurAcc->GetNode());
    int32_t iRefCount =
        (int32_t)(uintptr_t)pCurAcc->GetNode()->GetUserData(XFA_CalcRefCount);
    iRefCount++;
    pCurAcc->GetNode()->SetUserData(XFA_CalcRefCount,
                                    (void*)(uintptr_t)iRefCount);
    if (iRefCount > 11) {
      break;
    }
    if ((pCurAcc->ProcessCalculate()) == XFA_EVENTERROR_Sucess) {
      AddValidateWidget(pCurAcc);
    }
    iIndex++;
    RunCalculateRecursive(iIndex);
  }
}
int32_t CXFA_FFDocView::RunCalculateWidgets() {
  if (!m_pDoc->GetDocProvider()->IsCalculationsEnabled(m_pDoc)) {
    return XFA_EVENTERROR_Disabled;
  }
  int32_t iCounts = m_CalculateAccs.GetSize();
  int32_t iIndex = 0;
  if (iCounts > 0) {
    RunCalculateRecursive(iIndex);
  }
  for (int32_t i = 0; i < m_CalculateAccs.GetSize(); i++) {
    CXFA_WidgetAcc* pCurAcc = (CXFA_WidgetAcc*)m_CalculateAccs[i];
    pCurAcc->GetNode()->SetUserData(XFA_CalcRefCount, (void*)(uintptr_t)0);
  }
  m_CalculateAccs.RemoveAll();
  return XFA_EVENTERROR_Sucess;
}
void CXFA_FFDocView::AddValidateWidget(CXFA_WidgetAcc* pWidget) {
  if (m_ValidateAccs.Find(pWidget) < 0) {
    m_ValidateAccs.Add(pWidget);
  }
}
FX_BOOL CXFA_FFDocView::InitCalculate(CXFA_Node* pNode) {
  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_InitCalculate);
  return TRUE;
}
FX_BOOL CXFA_FFDocView::InitValidate(CXFA_Node* pNode) {
  if (!m_pDoc->GetDocProvider()->IsValidationsEnabled(m_pDoc)) {
    return FALSE;
  }
  ExecEventActivityByDeepFirst(pNode, XFA_EVENT_Validate);
  m_ValidateAccs.RemoveAll();
  return TRUE;
}
FX_BOOL CXFA_FFDocView::RunValidate() {
  if (!m_pDoc->GetDocProvider()->IsValidationsEnabled(m_pDoc)) {
    return FALSE;
  }
  int32_t iCounts = m_ValidateAccs.GetSize();
  for (int32_t i = 0; i < iCounts; i++) {
    CXFA_WidgetAcc* pAcc = (CXFA_WidgetAcc*)m_ValidateAccs[i];
    if (pAcc->GetNode()->HasFlag(XFA_NODEFLAG_HasRemoved)) {
      continue;
    }
    pAcc->ProcessValidate();
  }
  m_ValidateAccs.RemoveAll();
  return TRUE;
}
FX_BOOL CXFA_FFDocView::RunEventLayoutReady() {
  CXFA_Node* pRootItem =
      (CXFA_Node*)m_pDoc->GetXFADoc()->GetXFANode(XFA_HASHCODE_Form);
  if (!pRootItem) {
    return FALSE;
  }
  ExecEventActivityByDeepFirst(pRootItem, XFA_EVENT_Ready);
  RunLayout();
  return TRUE;
}
void CXFA_FFDocView::RunBindItems() {
  int32_t iCount = m_bindItems.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    if (((CXFA_Node*)m_bindItems[i])->HasFlag(XFA_NODEFLAG_HasRemoved)) {
      continue;
    }
    CXFA_Node* pWidgetNode =
        ((CXFA_Node*)m_bindItems[i])->GetNodeItem(XFA_NODEITEM_Parent);
    CXFA_WidgetAcc* pAcc = (CXFA_WidgetAcc*)pWidgetNode->GetWidgetData();
    if (!pAcc) {
      continue;
    }
    CXFA_BindItems binditems((CXFA_Node*)m_bindItems[i]);
    IXFA_ScriptContext* pScriptContext =
        pWidgetNode->GetDocument()->GetScriptContext();
    CFX_WideStringC wsRef;
    binditems.GetRef(wsRef);
    FX_DWORD dwStyle = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                       XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent |
                       XFA_RESOLVENODE_ALL;
    XFA_RESOLVENODE_RS rs;
    pScriptContext->ResolveObjects(pWidgetNode, wsRef, rs, dwStyle);
    int32_t iCount = rs.nodes.GetSize();
    pAcc->DeleteItem(-1);
    if (rs.dwFlags != XFA_RESOVENODE_RSTYPE_Nodes || iCount < 1) {
      continue;
    }
    CFX_WideStringC wsValueRef, wsLabelRef;
    binditems.GetValueRef(wsValueRef);
    binditems.GetLabelRef(wsLabelRef);
    FX_BOOL bUseValue = wsLabelRef.IsEmpty() || wsLabelRef == wsValueRef;
    FX_BOOL bLabelUseContent =
        wsLabelRef.IsEmpty() || wsLabelRef == FX_WSTRC(L"$");
    FX_BOOL bValueUseContent =
        wsValueRef.IsEmpty() || wsValueRef == FX_WSTRC(L"$");
    CFX_WideString wsValue, wsLabel;
    FX_DWORD uValueHash = FX_HashCode_String_GetW(CFX_WideString(wsValueRef),
                                                  wsValueRef.GetLength());
    for (int32_t i = 0; i < iCount; i++) {
      CXFA_Object* refObj = rs.nodes[i];
      if (!refObj->IsNode()) {
        continue;
      }
      CXFA_Node* refNode = (CXFA_Node*)refObj;
      if (bValueUseContent) {
        wsValue = refNode->GetContent();
      } else {
        CXFA_Node* nodeValue = refNode->GetFirstChildByName(uValueHash);
        if (nodeValue == NULL) {
          wsValue = refNode->GetContent();
        } else {
          wsValue = nodeValue->GetContent();
        }
      }
      if (!bUseValue) {
        if (bLabelUseContent) {
          wsLabel = refNode->GetContent();
        } else {
          CXFA_Node* nodeLabel = refNode->GetFirstChildByName(wsLabelRef);
          if (nodeLabel != NULL) {
            wsLabel = nodeLabel->GetContent();
          }
        }
      } else {
        wsLabel = wsValue;
      }
      pAcc->InsertItem(wsLabel, wsValue);
    }
  }
  m_bindItems.RemoveAll();
}
void CXFA_FFDocView::SetChangeMark() {
  if (m_iStatus < XFA_DOCVIEW_LAYOUTSTATUS_End) {
    return;
  }
  m_pDoc->GetDocProvider()->SetChangeMark(m_pDoc);
}
CXFA_Node* CXFA_FFDocView::GetRootSubform() {
  CXFA_Node* pFormPacketNode =
      (CXFA_Node*)m_pDoc->GetXFADoc()->GetXFANode(XFA_HASHCODE_Form);
  if (!pFormPacketNode) {
    return NULL;
  }
  return pFormPacketNode->GetFirstChildByClass(XFA_ELEMENT_Subform);
}
CXFA_FFDocWidgetIterator::CXFA_FFDocWidgetIterator(CXFA_FFDocView* pDocView,
                                                   CXFA_Node* pTravelRoot)
    : m_ContentIterator(pTravelRoot) {
  m_pDocView = pDocView;
  m_pCurWidget = NULL;
}
CXFA_FFDocWidgetIterator::~CXFA_FFDocWidgetIterator() {}
void CXFA_FFDocWidgetIterator::Reset() {
  m_ContentIterator.Reset();
  m_pCurWidget = NULL;
}
IXFA_Widget* CXFA_FFDocWidgetIterator::MoveToFirst() {
  return NULL;
}
IXFA_Widget* CXFA_FFDocWidgetIterator::MoveToLast() {
  return NULL;
}
IXFA_Widget* CXFA_FFDocWidgetIterator::MoveToNext() {
  CXFA_Node* pItem = m_pCurWidget ? m_ContentIterator.MoveToNext()
                                  : m_ContentIterator.GetCurrent();
  while (pItem) {
    if (CXFA_WidgetAcc* pAcc = (CXFA_WidgetAcc*)pItem->GetWidgetData()) {
      while ((m_pCurWidget = pAcc->GetNextWidget(NULL)) != NULL) {
        if (!m_pCurWidget->IsLoaded() &&
            (m_pCurWidget->GetStatus() & XFA_WIDGETSTATUS_Visible)) {
          m_pCurWidget->LoadWidget();
        }
        return m_pCurWidget;
      }
    }
    pItem = m_ContentIterator.MoveToNext();
  }
  return NULL;
}
IXFA_Widget* CXFA_FFDocWidgetIterator::MoveToPrevious() {
  return NULL;
}
IXFA_Widget* CXFA_FFDocWidgetIterator::GetCurrentWidget() {
  return NULL;
}
FX_BOOL CXFA_FFDocWidgetIterator::SetCurrentWidget(IXFA_Widget* hWidget) {
  return FALSE;
}
IXFA_WidgetAccIterator* XFA_WidgetAccIterator_Create(
    CXFA_WidgetAcc* pTravelRoot,
    XFA_WIDGETORDER eOrder) {
  if (!pTravelRoot) {
    return NULL;
  }
  return new CXFA_WidgetAccIterator(pTravelRoot->GetDocView(),
                                    pTravelRoot->GetNode());
}
CXFA_WidgetAccIterator::CXFA_WidgetAccIterator(CXFA_FFDocView* pDocView,
                                               CXFA_Node* pTravelRoot)
    : m_ContentIterator(pTravelRoot) {
  m_pDocView = pDocView;
  m_pCurWidgetAcc = NULL;
}
CXFA_WidgetAccIterator::~CXFA_WidgetAccIterator() {}
void CXFA_WidgetAccIterator::Reset() {
  m_pCurWidgetAcc = NULL;
  m_ContentIterator.Reset();
}
CXFA_WidgetAcc* CXFA_WidgetAccIterator::MoveToFirst() {
  return NULL;
}
CXFA_WidgetAcc* CXFA_WidgetAccIterator::MoveToLast() {
  return NULL;
}
CXFA_WidgetAcc* CXFA_WidgetAccIterator::MoveToNext() {
  CXFA_Node* pItem = m_pCurWidgetAcc ? m_ContentIterator.MoveToNext()
                                     : m_ContentIterator.GetCurrent();
  while (pItem) {
    if ((m_pCurWidgetAcc = (CXFA_WidgetAcc*)pItem->GetWidgetData()) != NULL) {
      return m_pCurWidgetAcc;
    }
    pItem = m_ContentIterator.MoveToNext();
  }
  return NULL;
}
CXFA_WidgetAcc* CXFA_WidgetAccIterator::MoveToPrevious() {
  return NULL;
}
CXFA_WidgetAcc* CXFA_WidgetAccIterator::GetCurrentWidgetAcc() {
  return NULL;
}
FX_BOOL CXFA_WidgetAccIterator::SetCurrentWidgetAcc(CXFA_WidgetAcc* hWidget) {
  return FALSE;
}
void CXFA_WidgetAccIterator::SkipTree() {
  m_ContentIterator.SkipChildrenAndMoveToNext();
  m_pCurWidgetAcc = NULL;
}
