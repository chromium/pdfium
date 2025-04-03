// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTPROCESSOR_H_
#define XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTPROCESSOR_H_

#include <iterator>
#include <list>
#include <map>
#include <optional>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/unowned_ptr_exclusion.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/prefinalizer.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutprocessor.h"

class CXFA_LayoutItem;
class CXFA_LayoutProcessor;
class CXFA_Node;

class CXFA_ViewLayoutProcessor
    : public cppgc::GarbageCollected<CXFA_ViewLayoutProcessor> {
  CPPGC_USING_PRE_FINALIZER(CXFA_ViewLayoutProcessor, PreFinalize);

 public:
  struct BreakData {
    CPPGC_STACK_ALLOCATED();  // Raw/Unowned pointers allowed.
   public:
    UNOWNED_PTR_EXCLUSION CXFA_Node* pLeader;   // POD struct.
    UNOWNED_PTR_EXCLUSION CXFA_Node* pTrailer;  // POD struct.
    bool bCreatePage;
  };

  struct OverflowData {
    CPPGC_STACK_ALLOCATED();  // Raw/Unowned pointers allowed.
   public:
    UNOWNED_PTR_EXCLUSION CXFA_Node* pLeader;   // POD struct.
    UNOWNED_PTR_EXCLUSION CXFA_Node* pTrailer;  // POD struct.
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ViewLayoutProcessor();

  void PreFinalize();
  void Trace(cppgc::Visitor* visitor) const;
  cppgc::Heap* GetHeap() const { return heap_; }

  bool InitLayoutPage(CXFA_Node* pFormNode);
  bool PrepareFirstPage(CXFA_Node* pRootSubform);
  float GetAvailHeight();
  bool GetNextAvailContentHeight(float fChildHeight);
  void SubmitContentItem(CXFA_ContentLayoutItem* pContentLayoutItem,
                         CXFA_ContentLayoutProcessor::Result eStatus);
  void FinishPaginatedPageSets();
  void SyncLayoutData();
  int32_t GetPageCount() const;
  CXFA_ViewLayoutItem* GetPage(int32_t index) const;
  int32_t GetPageIndex(const CXFA_ViewLayoutItem* pPage) const;
  CXFA_ViewLayoutItem* GetRootLayoutItem() const {
    return page_set_root_layout_item_;
  }
  std::optional<BreakData> ProcessBreakBefore(const CXFA_Node* pBreakNode);
  std::optional<BreakData> ProcessBreakAfter(const CXFA_Node* pBreakNode);
  std::optional<OverflowData> ProcessOverflow(CXFA_Node* pFormNode,
                                              bool bCreatePage);
  CXFA_Node* QueryOverflow(CXFA_Node* pFormNode);
  CXFA_Node* ProcessBookendLeader(const CXFA_Node* pBookendNode);
  CXFA_Node* ProcessBookendTrailer(const CXFA_Node* pBookendNode);

 private:
  class CXFA_ViewRecord : public cppgc::GarbageCollected<CXFA_ViewRecord> {
   public:
    CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
    ~CXFA_ViewRecord();

    void Trace(cppgc::Visitor* visitor) const;

    cppgc::Member<CXFA_ViewLayoutItem> pCurPageSet;
    cppgc::Member<CXFA_ViewLayoutItem> pCurPageArea;
    cppgc::Member<CXFA_ViewLayoutItem> pCurContentArea;

   private:
    CXFA_ViewRecord();
  };

  CXFA_ViewLayoutProcessor(cppgc::Heap* pHeap,
                           CXFA_LayoutProcessor* pLayoutProcessor);

  bool AppendNewPage(bool bFirstTemPage);
  void RemoveLayoutRecord(CXFA_ViewRecord* pNewRecord,
                          CXFA_ViewRecord* pPrevRecord);
  bool HasCurrentViewRecord() const {
    return current_view_record_iter_ != proposed_view_records_.end();
  }
  CXFA_ViewRecord* GetCurrentViewRecord() {
    return HasCurrentViewRecord() ? current_view_record_iter_->Get() : nullptr;
  }
  const CXFA_ViewRecord* GetCurrentViewRecord() const {
    return HasCurrentViewRecord() ? current_view_record_iter_->Get() : nullptr;
  }
  void ResetToFirstViewRecord() {
    current_view_record_iter_ = proposed_view_records_.begin();
  }
  std::list<cppgc::Member<CXFA_ViewRecord>>::iterator GetTailPosition() {
    auto iter = proposed_view_records_.end();
    return !proposed_view_records_.empty() ? std::prev(iter) : iter;
  }
  void AppendNewRecord(CXFA_ViewRecord* pNewRecord);
  CXFA_ViewRecord* CreateViewRecord(CXFA_Node* pPageNode, bool bCreateNew);
  CXFA_ViewRecord* CreateViewRecordSimple();
  void AddPageAreaLayoutItem(CXFA_ViewRecord* pNewRecord,
                             CXFA_Node* pNewPageArea);
  void AddContentAreaLayoutItem(CXFA_ViewRecord* pNewRecord,
                                CXFA_Node* pContentArea);
  bool RunBreak(XFA_Element eBreakType,
                XFA_AttributeValue eTargetType,
                CXFA_Node* pTarget,
                bool bStartNew);
  bool ShouldGetNextPageArea(CXFA_Node* pTarget, bool bStartNew) const;
  bool BreakOverflow(const CXFA_Node* pOverflowNode,
                     bool bCreatePage,
                     CXFA_Node** pLeaderTemplate,
                     CXFA_Node** pTrailerTemplate);
  CXFA_Node* ProcessBookendLeaderOrTrailer(const CXFA_Node* pBookendNode,
                                           bool bLeader);
  CXFA_Node* ResolveBookendLeaderOrTrailer(const CXFA_Node* pBookendNode,
                                           bool bLeader);
  std::optional<BreakData> ProcessBreakBeforeOrAfter(
      const CXFA_Node* pBreakNode,
      bool bBefore);
  BreakData ExecuteBreakBeforeOrAfter(const CXFA_Node* pCurNode, bool bBefore);

  int32_t CreateMinPageRecord(CXFA_Node* pPageArea,
                              bool bTargetPageArea,
                              bool bCreateLast);
  void CreateMinPageSetRecord(CXFA_Node* pPageSet, bool bCreateAll);
  void CreateNextMinRecord(CXFA_Node* pRecordNode);
  bool FindPageAreaFromPageSet(CXFA_Node* pPageSet,
                               CXFA_Node* pStartChild,
                               CXFA_Node* pTargetPageArea,
                               CXFA_Node* pTargetContentArea,
                               bool bNewPage,
                               bool bQuery);
  bool FindPageAreaFromPageSet_Ordered(CXFA_Node* pPageSet,
                                       CXFA_Node* pStartChild,
                                       CXFA_Node* pTargetPageArea,
                                       CXFA_Node* pTargetContentArea,
                                       bool bNewPage,
                                       bool bQuery);
  bool FindPageAreaFromPageSet_SimplexDuplex(
      CXFA_Node* pPageSet,
      CXFA_Node* pStartChild,
      CXFA_Node* pTargetPageArea,
      CXFA_Node* pTargetContentArea,
      bool bNewPage,
      bool bQuery,
      XFA_AttributeValue ePreferredPosition);
  bool MatchPageAreaOddOrEven(CXFA_Node* pPageArea);
  CXFA_Node* GetNextAvailPageArea(CXFA_Node* pTargetPageArea,
                                  CXFA_Node* pTargetContentArea,
                                  bool bNewPage,
                                  bool bQuery);
  bool GetNextContentArea(CXFA_Node* pTargetContentArea);
  void InitPageSetMap();
  void ProcessLastPageSet();
  bool IsPageSetRootOrderedOccurrence() const {
    return page_set_mode_ == XFA_AttributeValue::OrderedOccurrence;
  }
  void ClearData();
  void MergePageSetContents();
  void LayoutPageSetContents();
  void PrepareLayout();
  void SaveLayoutItemChildren(CXFA_LayoutItem* pParentLayoutItem);
  void ProcessSimplexOrDuplexPageSets(CXFA_ViewLayoutItem* pPageSetLayoutItem,
                                      bool bIsSimplex);

  UnownedPtr<cppgc::Heap> heap_;
  cppgc::Member<CXFA_LayoutProcessor> layout_processor_;
  cppgc::Member<CXFA_Node> page_set_node_;
  cppgc::Member<CXFA_Node> cur_page_area_;
  cppgc::Member<CXFA_ViewLayoutItem> page_set_root_layout_item_;
  cppgc::Member<CXFA_ViewLayoutItem> page_set_cur_layout_item_;
  std::list<cppgc::Member<CXFA_ViewRecord>> proposed_view_records_;
  std::list<cppgc::Member<CXFA_ViewRecord>>::iterator current_view_record_iter_;
  int32_t avail_pages_ = 0;
  int32_t cur_page_count_ = 0;
  XFA_AttributeValue page_set_mode_ = XFA_AttributeValue::OrderedOccurrence;
  bool create_over_flow_page_ = false;
  std::map<cppgc::Member<CXFA_Node>, int32_t> page_set_map_;
  std::vector<cppgc::Member<CXFA_ViewLayoutItem>> page_array_;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTPROCESSOR_H_
