// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFNOTIFY_H_
#define XFA_FXFA_CXFA_FFNOTIFY_H_

#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_FFWidgetHandler;
class CXFA_ContainerLayoutItem;
class CXFA_ContentLayoutItem;

class CXFA_FFNotify {
 public:
  explicit CXFA_FFNotify(CXFA_FFDoc* pDoc);
  ~CXFA_FFNotify();

  void OnPageEvent(CXFA_ContainerLayoutItem* pSender, uint32_t dwEvent);

  void OnWidgetListItemAdded(CXFA_WidgetAcc* pSender,
                             const wchar_t* pLabel,
                             const wchar_t* pValue,
                             int32_t iIndex);
  void OnWidgetListItemRemoved(CXFA_WidgetAcc* pSender, int32_t iIndex);

  // Node events
  void OnNodeReady(CXFA_Node* pNode);
  void OnValueChanging(CXFA_Node* pSender, XFA_Attribute eAttr);
  void OnValueChanged(CXFA_Node* pSender,
                      XFA_Attribute eAttr,
                      CXFA_Node* pParentNode,
                      CXFA_Node* pWidgetNode);
  void OnChildAdded(CXFA_Node* pSender);
  void OnChildRemoved();

  CXFA_ContainerLayoutItem* OnCreateContainerLayoutItem(CXFA_Node* pNode);
  CXFA_ContentLayoutItem* OnCreateContentLayoutItem(CXFA_Node* pNode);

  void OnLayoutItemAdded(CXFA_LayoutProcessor* pLayout,
                         CXFA_LayoutItem* pSender,
                         int32_t iPageIdx,
                         uint32_t dwStatus);
  void OnLayoutItemRemoving(CXFA_LayoutProcessor* pLayout,
                            CXFA_LayoutItem* pSender);

  void StartFieldDrawLayout(CXFA_Node* pItem,
                            float& fCalcWidth,
                            float& fCalcHeight);
  bool FindSplitPos(CXFA_Node* pItem,
                    int32_t iBlockIndex,
                    float& fCalcHeightPos);
  bool RunScript(CXFA_Script* pScript, CXFA_Node* pFormItem);
  int32_t ExecEventByDeepFirst(CXFA_Node* pFormNode,
                               XFA_EVENTTYPE eEventType,
                               bool bIsFormReady = false,
                               bool bRecursive = true,
                               CXFA_WidgetAcc* pExclude = nullptr);
  void AddCalcValidate(CXFA_Node* pNode);
  CXFA_FFDoc* GetHDOC();
  IXFA_DocEnvironment* GetDocEnvironment() const;
  IXFA_AppProvider* GetAppProvider();
  CXFA_FFWidgetHandler* GetWidgetHandler();
  CXFA_FFWidget* GetHWidget(CXFA_LayoutItem* pLayoutItem);
  void OpenDropDownList(CXFA_FFWidget* hWidget);
  WideString GetCurrentDateTime();
  void ResetData(CXFA_WidgetAcc* pWidgetAcc = nullptr);
  int32_t GetLayoutStatus();
  void RunNodeInitialize(CXFA_Node* pNode);
  void RunSubformIndexChange(CXFA_Node* pSubformNode);
  CXFA_Node* GetFocusWidgetNode();
  void SetFocusWidgetNode(CXFA_Node* pNode);

 private:
  UnownedPtr<CXFA_FFDoc> const m_pDoc;
};

#endif  // XFA_FXFA_CXFA_FFNOTIFY_H_
