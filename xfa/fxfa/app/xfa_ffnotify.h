// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFNOTIFY_H_
#define XFA_FXFA_APP_XFA_FFNOTIFY_H_

#include "xfa/fxfa/parser/xfa_document.h"

class CXFA_FFWidgetHandler;

class CXFA_FFNotify {
 public:
  CXFA_FFNotify(CXFA_FFDoc* pDoc);
  ~CXFA_FFNotify();

  void OnPageEvent(CXFA_ContainerLayoutItem* pSender,
                   XFA_PAGEEVENT eEvent,
                   void* pParam = NULL);

  void OnNodeEvent(CXFA_Node* pSender,
                   XFA_NODEEVENT eEvent,
                   void* pParam = NULL,
                   void* pParam2 = NULL,
                   void* pParam3 = NULL,
                   void* pParam4 = NULL);
  void OnWidgetDataEvent(CXFA_WidgetData* pSender,
                         uint32_t dwEvent,
                         void* pParam = NULL,
                         void* pAdditional = NULL,
                         void* pAdditional2 = NULL);
  CXFA_LayoutItem* OnCreateLayoutItem(CXFA_Node* pNode);
  void OnLayoutEvent(CXFA_LayoutProcessor* pLayout,
                     CXFA_LayoutItem* pSender,
                     XFA_LAYOUTEVENT eEvent,
                     void* pParam = NULL,
                     void* pParam2 = NULL);

  void StartFieldDrawLayout(CXFA_Node* pItem,
                            FX_FLOAT& fCalcWidth,
                            FX_FLOAT& fCalcHeight);
  FX_BOOL FindSplitPos(CXFA_Node* pItem,
                       int32_t iBlockIndex,
                       FX_FLOAT& fCalcHeightPos);
  FX_BOOL RunScript(CXFA_Node* pScript, CXFA_Node* pFormItem);
  int32_t ExecEventByDeepFirst(CXFA_Node* pFormNode,
                               XFA_EVENTTYPE eEventType,
                               FX_BOOL bIsFormReady = FALSE,
                               FX_BOOL bRecursive = TRUE,
                               CXFA_WidgetAcc* pExclude = NULL);
  void AddCalcValidate(CXFA_Node* pNode);
  CXFA_FFDoc* GetHDOC();
  IXFA_DocProvider* GetDocProvider();
  IXFA_AppProvider* GetAppProvider();
  CXFA_FFWidgetHandler* GetWidgetHandler();
  CXFA_FFWidget* GetHWidget(CXFA_LayoutItem* pLayoutItem);
  void OpenDropDownList(CXFA_FFWidget* hWidget);
  CFX_WideString GetCurrentDateTime();
  void ResetData(CXFA_WidgetData* pWidgetData = NULL);
  int32_t GetLayoutStatus();
  void RunNodeInitialize(CXFA_Node* pNode);
  void RunSubformIndexChange(CXFA_Node* pSubformNode);
  CXFA_Node* GetFocusWidgetNode();
  void SetFocusWidgetNode(CXFA_Node* pNode);

 protected:
  void OnNodeReady(CXFA_Node* pNode);
  void OnValueChanging(CXFA_Node* pSender, void* pParam, void* pParam2);
  void OnValueChanged(CXFA_Node* pSender,
                      void* pParam,
                      void* pParam2,
                      void* pParam3,
                      void* pParam4);
  void OnChildAdded(CXFA_Node* pSender, void* pParam, void* pParam2);
  void OnChildRemoved(CXFA_Node* pSender, void* pParam, void* pParam2);
  void OnLayoutItemAdd(CXFA_FFDocView* pDocView,
                       CXFA_LayoutProcessor* pLayout,
                       CXFA_LayoutItem* pSender,
                       void* pParam,
                       void* pParam2);
  void OnLayoutItemRemoving(CXFA_FFDocView* pDocView,
                            CXFA_LayoutProcessor* pLayout,
                            CXFA_LayoutItem* pSender,
                            void* pParam,
                            void* pParam2);
  void OnLayoutItemRectChanged(CXFA_FFDocView* pDocView,
                               CXFA_LayoutProcessor* pLayout,
                               CXFA_LayoutItem* pSender,
                               void* pParam,
                               void* pParam2);
  void OnLayoutItemStatustChanged(CXFA_FFDocView* pDocView,
                                  CXFA_LayoutProcessor* pLayout,
                                  CXFA_LayoutItem* pSender,
                                  void* pParam,
                                  void* pParam2);
  CXFA_FFDoc* m_pDoc;
};

#endif  // XFA_FXFA_APP_XFA_FFNOTIFY_H_
