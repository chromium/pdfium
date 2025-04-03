// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTPROCESSOR_H_
#define XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTPROCESSOR_H_

#include <float.h>

#include <array>
#include <list>
#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/macros.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/persistent.h"
#include "xfa/fxfa/fxfa_basic.h"

constexpr float kXFALayoutPrecision = 0.0005f;

class CXFA_ContentLayoutItem;
class CXFA_Node;
class CXFA_ViewLayoutItem;
class CXFA_ViewLayoutProcessor;

class CXFA_ContentLayoutProcessor
    : public cppgc::GarbageCollected<CXFA_ContentLayoutProcessor> {
 public:
  enum class Result : uint8_t {
    kDone,
    kPageFullBreak,
    kRowFullBreak,
    kManualBreak,
  };

  enum class Stage : uint8_t {
    kNone,
    kBookendLeader,
    kBreakBefore,
    kKeep,
    kContainer,
    kBreakAfter,
    kBookendTrailer,
    kDone,
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ContentLayoutProcessor();

  void Trace(cppgc::Visitor* visitor) const;
  cppgc::Heap* GetHeap() const { return heap_; }

  Result DoLayout(bool bUseBreakControl, float fHeightLimit, float fRealHeight);
  void DoLayoutPageArea(CXFA_ViewLayoutItem* pPageAreaLayoutItem);

  CXFA_Node* GetFormNode() { return form_node_; }
  CXFA_ContentLayoutItem* ExtractLayoutItem();

 private:
  class Context {
    CPPGC_STACK_ALLOCATED();  // Allows Raw/Unowned pointers.

   public:
    Context();
    ~Context();

    std::optional<float> cur_column_width_;
    UnownedPtr<std::vector<float>> prg_specified_column_widths_;
    UnownedPtr<CXFA_ContentLayoutProcessor> overflow_processor_;  // OK, stack
    UnownedPtr<CXFA_Node> overflow_node_;                         // Ok, stack
  };

  using ContentLayoutItemVector =
      std::vector<cppgc::Persistent<CXFA_ContentLayoutItem>>;

  CXFA_ContentLayoutProcessor(cppgc::Heap* pHeap,
                              CXFA_Node* pNode,
                              CXFA_ViewLayoutProcessor* pViewLayoutProcessor);

  Result DoLayoutInternal(bool bUseBreakControl,
                          float fHeightLimit,
                          float fRealHeight,
                          Context* pContext);

  CFX_SizeF GetCurrentComponentSize();
  bool HasLayoutItem() const { return !!layout_item_; }
  void SplitLayoutItem(float fSplitPos);
  float FindSplitPos(float fProposedSplitPos);
  bool ProcessKeepForSplit(CXFA_ContentLayoutProcessor* pChildProcessor,
                           Result eRetValue,
                           ContentLayoutItemVector& rgCurLineLayoutItem,
                           float* fContentCurRowAvailWidth,
                           float* fContentCurRowHeight,
                           float* fContentCurRowY,
                           bool* bAddedItemInRow,
                           bool* bForceEndPage,
                           Result* result);
  void ProcessUnUseOverFlow(CXFA_Node* pLeaderNode,
                            CXFA_Node* pTrailerNode,
                            CXFA_ContentLayoutItem* pTrailerItem,
                            CXFA_Node* pFormNode);
  bool IsAddNewRowForTrailer(CXFA_ContentLayoutItem* pTrailerItem);
  bool JudgeLeaderOrTrailerForOccur(CXFA_Node* pFormNode);

  // Object comes from GCed heap.
  CXFA_ContentLayoutItem* CreateContentLayoutItem(CXFA_Node* pFormNode);

  void SetCurrentComponentPos(const CFX_PointF& pos);
  void SetCurrentComponentSize(const CFX_SizeF& size);

  void SplitLayoutItem(CXFA_ContentLayoutItem* pLayoutItem,
                       CXFA_ContentLayoutItem* pSecondParent,
                       float fSplitPos);
  float InsertKeepLayoutItems();
  bool CalculateRowChildPosition(
      std::array<ContentLayoutItemVector, 3>& rgCurLineLayoutItems,
      XFA_AttributeValue eFlowStrategy,
      bool bContainerHeightAutoSize,
      bool bContainerWidthAutoSize,
      float* fContentCalculatedWidth,
      float* fContentCalculatedHeight,
      float* fContentCurRowY,
      float fContentCurRowHeight,
      float fContentWidthLimit,
      bool bRootForceTb);
  void ProcessUnUseBinds(CXFA_Node* pFormNode);
  bool JudgePutNextPage(CXFA_ContentLayoutItem* pParentLayoutItem,
                        float fChildHeight,
                        std::vector<CXFA_ContentLayoutItem*>* pKeepItems);

  void DoLayoutPositionedContainer(Context* pContext);
  void DoLayoutTableContainer(CXFA_Node* pLayoutNode);
  Result DoLayoutFlowedContainer(bool bUseBreakControl,
                                 XFA_AttributeValue eFlowStrategy,
                                 float fHeightLimit,
                                 float fRealHeight,
                                 Context* pContext,
                                 bool bRootForceTb);
  void DoLayoutField();

  void GotoNextContainerNodeSimple();

  // Return new stage and new action node.
  std::pair<Stage, CXFA_Node*> GotoNextContainerNode(
      Stage nCurStage,
      CXFA_Node* pParentContainer,
      CXFA_Node* pCurActionNode);

  std::optional<Stage> ProcessKeepNodesForCheckNext(CXFA_Node** pCurActionNode,
                                                    CXFA_Node** pNextContainer,
                                                    bool* pLastKeepNode);

  std::optional<Stage> ProcessKeepNodesForBreakBefore(
      CXFA_Node** pCurActionNode,
      CXFA_Node* pContainerNode);

  CXFA_Node* GetSubformSetParent(CXFA_Node* pSubformSet);

  void UpdatePendingItemLayout(CXFA_ContentLayoutItem* pLayoutItem);
  void AddTrailerBeforeSplit(float fSplitPos,
                             CXFA_ContentLayoutItem* pTrailerLayoutItem,
                             bool bUseInherited);
  void AddLeaderAfterSplit(CXFA_ContentLayoutItem* pLeaderLayoutItem);
  void AddPendingNode(CXFA_Node* pPendingNode, bool bBreakPending);
  float InsertPendingItems(CXFA_Node* pCurChildNode);
  Result InsertFlowedItem(
      CXFA_ContentLayoutProcessor* pProcessor,
      bool bContainerWidthAutoSize,
      bool bContainerHeightAutoSize,
      float fContainerHeight,
      XFA_AttributeValue eFlowStrategy,
      uint8_t* uCurHAlignState,
      std::array<ContentLayoutItemVector, 3>& rgCurLineLayoutItems,
      bool bUseBreakControl,
      float fAvailHeight,
      float fRealHeight,
      float fContentWidthLimit,
      float* fContentCurRowY,
      float* fContentCurRowAvailWidth,
      float* fContentCurRowHeight,
      bool* bAddedItemInRow,
      bool* bForceEndPage,
      Context* pLayoutContext,
      bool bNewRow);

  std::optional<Stage> HandleKeep(CXFA_Node* pBreakAfterNode,
                                  CXFA_Node** pCurActionNode);
  std::optional<Stage> HandleBookendLeader(CXFA_Node* pParentContainer,
                                           CXFA_Node** pCurActionNode);
  std::optional<Stage> HandleBreakBefore(CXFA_Node* pChildContainer,
                                         CXFA_Node** pCurActionNode);
  std::optional<Stage> HandleBreakAfter(CXFA_Node* pChildContainer,
                                        CXFA_Node** pCurActionNode);
  std::optional<Stage> HandleCheckNextChildContainer(
      CXFA_Node* pParentContainer,
      CXFA_Node* pChildContainer,
      CXFA_Node** pCurActionNode);
  std::optional<Stage> HandleBookendTrailer(CXFA_Node* pParentContainer,
                                            CXFA_Node** pCurActionNode);
  void ProcessKeepNodesEnd();
  void AdjustContainerSpecifiedSize(Context* pContext,
                                    CFX_SizeF* pSize,
                                    bool* pContainerWidthAutoSize,
                                    bool* pContainerHeightAutoSize);
  CXFA_ContentLayoutItem* FindLastContentLayoutItem(
      XFA_AttributeValue eFlowStrategy);
  CFX_SizeF CalculateLayoutItemSize(const CXFA_ContentLayoutItem* pLayoutChild);

  Stage cur_child_node_stage_ = Stage::kNone;
  Result pre_process_rs_ = Result::kDone;
  bool break_pending_ = true;
  bool use_inherited_ = false;
  bool keep_break_finish_ = false;
  bool is_process_keep_ = false;
  bool has_avail_height_ = true;
  float used_size_ = 0;
  float last_row_width_ = 0;
  float last_row_y_ = 0;
  float width_limit_ = 0;
  UnownedPtr<cppgc::Heap> heap_;
  cppgc::Member<CXFA_Node> const form_node_;
  cppgc::Member<CXFA_Node> cur_child_node_;
  cppgc::Member<CXFA_Node> keep_head_node_;
  cppgc::Member<CXFA_Node> keep_tail_node_;
  cppgc::Member<CXFA_ContentLayoutItem> layout_item_;
  cppgc::Member<CXFA_ContentLayoutItem> old_layout_item_;
  cppgc::Member<CXFA_ViewLayoutProcessor> view_layout_processor_;
  cppgc::Member<CXFA_ContentLayoutProcessor> cur_child_preprocessor_;
  std::vector<float> rg_specified_column_widths_;
  std::vector<cppgc::Member<CXFA_ContentLayoutItem>> array_keep_items_;
  std::list<cppgc::Member<CXFA_Node>> pending_nodes_;
  std::map<cppgc::Member<CXFA_Node>, int32_t> pending_nodes_count_;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTPROCESSOR_H_
