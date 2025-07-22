// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDOCVIEW_H_
#define XFA_FXFA_CXFA_FFDOCVIEW_H_

#include <list>
#include <vector>

#include "core/fxcrt/span.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/fxfa.h"

class CXFA_BindItems;
class CXFA_FFDoc;
class CXFA_FFWidgetHandler;
class CXFA_Node;
class CXFA_Subform;
class CXFA_ViewLayoutItem;

extern const pdfium::span<const XFA_AttributeValue> kXFAEventActivity;

class CXFA_FFDocView : public cppgc::GarbageCollected<CXFA_FFDocView> {
 public:
  enum class LayoutStatus : uint8_t { kNone, kStart, kDoing, kEnd };

  class UpdateScope {
    CPPGC_STACK_ALLOCATED();

   public:
    explicit UpdateScope(CXFA_FFDocView* pDocView);
    ~UpdateScope();

   private:
    UnownedPtr<CXFA_FFDocView> const doc_view_;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFDocView();

  void Trace(cppgc::Visitor* visitor) const;

  CXFA_FFDoc* GetDoc() const { return doc_; }
  int32_t StartLayout();
  int32_t DoLayout();
  void StopLayout();

  void SetLayoutEvent() { layout_event_ = true; }
  bool InLayoutStatus() const { return in_layout_status_; }
  LayoutStatus GetLayoutStatus() const { return status_; }

  void UpdateDocView();
  void UpdateUIDisplay(CXFA_Node* pNode, CXFA_FFWidget* pExcept);

  int32_t CountPageViews() const;
  CXFA_FFPageView* GetPageView(int32_t nIndex) const;

  void ResetNode(CXFA_Node* pNode);
  CXFA_Node* GetRootSubform();
  CXFA_FFWidgetHandler* GetWidgetHandler();
  CXFA_FFWidget* GetFocusWidget() const { return focus_widget_; }
  bool SetFocus(CXFA_FFWidget* pNewFocus);
  CXFA_FFWidget* GetWidgetForNode(CXFA_Node* node);
  CXFA_FFWidget* GetWidgetByName(const WideString& wsName,
                                 CXFA_FFWidget* pRefWidget);
  CXFA_LayoutProcessor* GetLayoutProcessor() const;
  void OnPageViewEvent(CXFA_ViewLayoutItem* pSender,
                       CXFA_FFDoc::PageViewEvent eEvent);
  void LockUpdate() { lock_++; }
  void UnlockUpdate() { lock_--; }
  void InvalidateRect(CXFA_FFPageView* pPageView,
                      const CFX_RectF& rtInvalidate);
  void RunDocClose();

  void ProcessValueChanged(CXFA_Node* node);
  void SetChangeMark();

  void AddValidateNode(CXFA_Node* node);
  void AddCalculateNodeNotify(CXFA_Node* pNodeChange);
  void AddCalculateNode(CXFA_Node* node);

  bool RunLayout();
  void AddNewFormNode(CXFA_Node* pNode);
  void AddIndexChangedSubform(CXFA_Subform* pNode);
  CXFA_Node* GetFocusNode() const { return focus_node_; }
  void SetFocusNode(CXFA_Node* pNode);
  void DeleteLayoutItem(CXFA_FFWidget* pWidget);
  XFA_EventError ExecEventActivityByDeepFirst(CXFA_Node* pFormNode,
                                              XFA_EVENTTYPE eEventType,
                                              bool bIsFormReady,
                                              bool bRecursive);

  void AddBindItem(CXFA_BindItems* item) { bind_items_.push_back(item); }
  void AddNullTestMsg(const WideString& msg);

 private:
  explicit CXFA_FFDocView(CXFA_FFDoc* doc);

  bool RunEventLayoutReady();
  void RunBindItems();
  void InitCalculate(CXFA_Node* pNode);
  void InitLayout(CXFA_Node* pNode);
  size_t RunCalculateRecursive(size_t index);
  void ShowNullTestMsg();
  bool ResetSingleNodeData(CXFA_Node* pNode);

  bool IsUpdateLocked() const { return lock_ > 0; }
  void InitValidate(CXFA_Node* pNode);
  void RunValidate();
  XFA_EventError RunCalculateWidgets();
  void RunSubformIndexChange();

  cppgc::Member<CXFA_FFDoc> const doc_;
  cppgc::Member<CXFA_FFWidgetHandler> widget_handler_;
  cppgc::Member<CXFA_Node> focus_node_;
  cppgc::Member<CXFA_FFWidget> focus_widget_;
  std::list<cppgc::Member<CXFA_Node>> validate_nodes_;
  std::vector<cppgc::Member<CXFA_Node>> calculate_nodes_;
  std::list<cppgc::Member<CXFA_BindItems>> bind_items_;
  std::list<cppgc::Member<CXFA_Node>> new_added_nodes_;
  std::list<cppgc::Member<CXFA_Subform>> index_changed_subforms_;
  std::vector<WideString> null_test_msg_array_;
  bool layout_event_ = false;
  bool in_layout_status_ = false;
  LayoutStatus status_ = LayoutStatus::kNone;
  int32_t lock_ = 0;
};

#endif  // XFA_FXFA_CXFA_FFDOCVIEW_H_
