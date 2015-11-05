// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXFA_SRC_APP_XFA_FFDOCVIEW_H_
#define FXFA_SRC_APP_XFA_FFDOCVIEW_H_

#include "xfa_ffdoc.h"

class CXFA_FFPageView;
class CXFA_FFWidgetHandler;
class CXFA_FFDoc;
class CXFA_FFWidget;
extern const XFA_ATTRIBUTEENUM gs_EventActivity[];
enum XFA_DOCVIEW_LAYOUTSTATUS {
  XFA_DOCVIEW_LAYOUTSTATUS_None,
  XFA_DOCVIEW_LAYOUTSTATUS_Start,
  XFA_DOCVIEW_LAYOUTSTATUS_FormInitialize,
  XFA_DOCVIEW_LAYOUTSTATUS_FormInitCalculate,
  XFA_DOCVIEW_LAYOUTSTATUS_FormInitValidate,
  XFA_DOCVIEW_LAYOUTSTATUS_FormFormReady,
  XFA_DOCVIEW_LAYOUTSTATUS_Doing,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetInitialize,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetInitCalculate,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetInitValidate,
  XFA_DOCVIEW_LAYOUTSTATUS_PagesetFormReady,
  XFA_DOCVIEW_LAYOUTSTATUS_LayoutReady,
  XFA_DOCVIEW_LAYOUTSTATUS_DocReady,
  XFA_DOCVIEW_LAYOUTSTATUS_End,
  XFA_DOCVIEW_LAYOUTSTATUS_Next,
};
class CXFA_FFDocView : public IXFA_DocView {
 public:
  CXFA_FFDocView(CXFA_FFDoc* pDoc);
  ~CXFA_FFDocView();

  virtual IXFA_Doc* GetDoc() { return m_pDoc; }
  virtual int32_t StartLayout(int32_t iStartPage = 0);
  virtual int32_t DoLayout(IFX_Pause* pPause = NULL);
  virtual void StopLayout();
  virtual int32_t GetLayoutStatus();
  virtual void UpdateDocView();
  virtual int32_t CountPageViews();
  virtual IXFA_PageView* GetPageView(int32_t nIndex);
  virtual IXFA_Widget* GetWidgetByName(const CFX_WideStringC& wsName);
  virtual CXFA_WidgetAcc* GetWidgetAccByName(const CFX_WideStringC& wsName);
  virtual void ResetWidgetData(CXFA_WidgetAcc* pWidgetAcc = NULL);
  virtual int32_t ProcessWidgetEvent(CXFA_EventParam* pParam,
                                     CXFA_WidgetAcc* pWidgetAcc = NULL);
  virtual IXFA_WidgetHandler* GetWidgetHandler();
  virtual IXFA_WidgetIterator* CreateWidgetIterator();
  virtual IXFA_WidgetAccIterator* CreateWidgetAccIterator(
      XFA_WIDGETORDER eOrder = XFA_WIDGETORDER_PreOrder);
  virtual IXFA_Widget* GetFocusWidget();
  virtual void KillFocus();
  virtual FX_BOOL SetFocus(IXFA_Widget* hWidget);
  CXFA_FFWidget* GetWidgetByName(const CFX_WideStringC& wsName,
                                 CXFA_FFWidget* pRefWidget = NULL);
  CXFA_WidgetAcc* GetWidgetAccByName(const CFX_WideStringC& wsName,
                                     CXFA_WidgetAcc* pRefWidgetAcc = NULL);
  IXFA_DocLayout* GetXFALayout() const;
  void OnPageEvent(IXFA_LayoutPage* pSender,
                   XFA_PAGEEVENT eEvent,
                   int32_t iPageIndex);
  void LockUpdate();
  void UnlockUpdate();
  FX_BOOL IsUpdateLocked();
  void ClearInvalidateList();
  void AddInvalidateRect(CXFA_FFWidget* pWidget, const CFX_RectF& rtInvalidate);
  void AddInvalidateRect(IXFA_PageView* pPageView,
                         const CFX_RectF& rtInvalidate);
  void RunInvalidate();
  void RunDocClose();
  void DestroyDocView();

  FX_BOOL InitValidate(CXFA_Node* pNode);
  FX_BOOL RunValidate();

  void SetChangeMark();

  void AddValidateWidget(CXFA_WidgetAcc* pWidget);
  void AddCalculateNodeNotify(CXFA_Node* pNodeChange);
  void AddCalculateWidgetAcc(CXFA_WidgetAcc* pWidgetAcc);
  int32_t RunCalculateWidgets();
  FX_BOOL IsStaticNotify();
  FX_BOOL RunLayout();
  void RunSubformIndexChange();
  void AddNewFormNode(CXFA_Node* pNode);
  void AddIndexChangedSubform(CXFA_Node* pNode);
  CXFA_WidgetAcc* GetFocusWidgetAcc();
  void SetFocusWidgetAcc(CXFA_WidgetAcc* pWidgetAcc);
  void DeleteLayoutItem(CXFA_FFWidget* pWidget);
  int32_t ExecEventActivityByDeepFirst(CXFA_Node* pFormNode,
                                       XFA_EVENTTYPE eEventType,
                                       FX_BOOL bIsFormReady = FALSE,
                                       FX_BOOL bRecursive = TRUE,
                                       CXFA_Node* pExclude = NULL);
  FX_BOOL m_bLayoutEvent;
  CFX_WideStringArray m_arrNullTestMsg;
  CXFA_FFWidget* m_pListFocusWidget;
  FX_BOOL m_bInLayoutStatus;

 protected:
  FX_BOOL RunEventLayoutReady();
  void RunBindItems();
  FX_BOOL InitCalculate(CXFA_Node* pNode);
  void InitLayout(CXFA_Node* pNode);
  void RunCalculateRecursive(int32_t& iIndex);
  void ShowNullTestMsg();
  FX_BOOL ResetSingleWidgetAccData(CXFA_WidgetAcc* pWidgetAcc);
  CXFA_Node* GetRootSubform();

  CXFA_FFDoc* m_pDoc;
  CXFA_FFWidgetHandler* m_pWidgetHandler;
  IXFA_DocLayout* m_pXFADocLayout;
  CXFA_WidgetAcc* m_pFocusAcc;
  CXFA_FFWidget* m_pFocusWidget;
  CXFA_FFWidget* m_pOldFocusWidget;
  CFX_MapPtrToPtr m_mapPageInvalidate;
  CFX_PtrArray m_ValidateAccs;
  CFX_PtrArray m_bindItems;
  CFX_PtrArray m_CalculateAccs;

  CFX_PtrArray m_NewAddedNodes;
  CFX_PtrArray m_IndexChangedSubforms;
  XFA_DOCVIEW_LAYOUTSTATUS m_iStatus;
  int32_t m_iLock;
  friend class CXFA_FFNotify;
};
class CXFA_FFDocWidgetIterator : public IXFA_WidgetIterator {
 public:
  CXFA_FFDocWidgetIterator(CXFA_FFDocView* pDocView, CXFA_Node* pTravelRoot);
  virtual ~CXFA_FFDocWidgetIterator();

  virtual void Release() { delete this; }

  virtual void Reset();
  virtual IXFA_Widget* MoveToFirst();
  virtual IXFA_Widget* MoveToLast();
  virtual IXFA_Widget* MoveToNext();
  virtual IXFA_Widget* MoveToPrevious();
  virtual IXFA_Widget* GetCurrentWidget();
  virtual FX_BOOL SetCurrentWidget(IXFA_Widget* hWidget);

 protected:
  CXFA_ContainerIterator m_ContentIterator;
  CXFA_FFDocView* m_pDocView;
  CXFA_FFWidget* m_pCurWidget;
};
class CXFA_WidgetAccIterator : public IXFA_WidgetAccIterator {
 public:
  CXFA_WidgetAccIterator(CXFA_FFDocView* pDocView, CXFA_Node* pTravelRoot);
  virtual ~CXFA_WidgetAccIterator();
  virtual void Release() { delete this; }
  virtual void Reset();
  virtual CXFA_WidgetAcc* MoveToFirst();
  virtual CXFA_WidgetAcc* MoveToLast();
  virtual CXFA_WidgetAcc* MoveToNext();
  virtual CXFA_WidgetAcc* MoveToPrevious();
  virtual CXFA_WidgetAcc* GetCurrentWidgetAcc();
  virtual FX_BOOL SetCurrentWidgetAcc(CXFA_WidgetAcc* hWidget);
  virtual void SkipTree();

 protected:
  CXFA_ContainerIterator m_ContentIterator;
  CXFA_FFDocView* m_pDocView;
  CXFA_WidgetAcc* m_pCurWidgetAcc;
};

#endif  // FXFA_SRC_APP_XFA_FFDOCVIEW_H_
