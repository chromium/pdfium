// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFNOTIFY_H_
#define XFA_FXFA_CXFA_FFNOTIFY_H_

#include "core/fxcrt/mask.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_LayoutItem;
class CXFA_LayoutProcessor;
class CXFA_Script;
class CXFA_ViewLayoutItem;

class CXFA_FFNotify : public cppgc::GarbageCollected<CXFA_FFNotify> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFNotify();

  void Trace(cppgc::Visitor* visitor) const;

  void OnPageViewEvent(CXFA_ViewLayoutItem* pSender,
                       CXFA_FFDoc::PageViewEvent eEvent);

  void OnWidgetListItemAdded(CXFA_Node* pSender,
                             const WideString& wsLabel,
                             int32_t iIndex);
  void OnWidgetListItemRemoved(CXFA_Node* pSender, int32_t iIndex);

  // Node events
  void OnNodeReady(CXFA_Node* pNode);
  void OnValueChanging(CXFA_Node* pSender, XFA_Attribute eAttr);
  void OnValueChanged(CXFA_Node* pSender,
                      XFA_Attribute eAttr,
                      CXFA_Node* pParentNode,
                      CXFA_Node* pWidgetNode);
  void OnContainerChanged();
  void OnChildAdded(CXFA_Node* pSender);
  void OnChildRemoved();

  // These two return new views/widgets from cppgc heap.
  CXFA_FFPageView* OnCreateViewLayoutItem(CXFA_Node* pNode);
  CXFA_FFWidget* OnCreateContentLayoutItem(CXFA_Node* pNode);

  void OnLayoutItemAdded(CXFA_LayoutProcessor* pLayout,
                         CXFA_LayoutItem* pSender,
                         int32_t iPageIdx,
                         Mask<XFA_WidgetStatus> dwStatus);
  void OnLayoutItemRemoving(CXFA_LayoutProcessor* pLayout,
                            CXFA_LayoutItem* pSender);
  void StartFieldDrawLayout(CXFA_Node* pItem,
                            float* pCalcWidth,
                            float* pCalcHeight);
  bool RunScript(CXFA_Script* pScript, CXFA_Node* pFormItem);
  XFA_EventError ExecEventByDeepFirst(CXFA_Node* pFormNode,
                                      XFA_EVENTTYPE eEventType,
                                      bool bIsFormReady,
                                      bool bRecursive);
  void AddCalcValidate(CXFA_Node* pNode);
  CXFA_FFDoc* GetFFDoc() const { return m_pDoc; }
  CXFA_FFApp::CallbackIface* GetAppProvider();
  void HandleWidgetEvent(CXFA_Node* pNode, CXFA_EventParam* pParam);
  void OpenDropDownList(CXFA_Node* pNode);
  void ResetData(CXFA_Node* pNode);
  CXFA_FFDocView::LayoutStatus GetLayoutStatus();
  void RunNodeInitialize(CXFA_Node* pNode);
  void RunSubformIndexChange(CXFA_Subform* pSubformNode);
  CXFA_Node* GetFocusWidgetNode();
  void SetFocusWidgetNode(CXFA_Node* pNode);

 private:
  explicit CXFA_FFNotify(CXFA_FFDoc* pDoc);

  cppgc::Member<CXFA_FFDoc> const m_pDoc;
};

#endif  // XFA_FXFA_CXFA_FFNOTIFY_H_
