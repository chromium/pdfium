// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTPROCESSOR_H_
#define XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTPROCESSOR_H_

#include <float.h>

#include <list>
#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/optional.h"
#include "xfa/fxfa/fxfa_basic.h"

constexpr float kXFALayoutPrecision = 0.0005f;

class CXFA_ContentLayoutItem;
class CXFA_ContentLayoutProcessor;
class CXFA_LayoutProcessor;
class CXFA_Node;
class CXFA_ViewLayoutItem;
class CXFA_ViewLayoutProcessor;

class CXFA_ContentLayoutProcessor {
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

  CXFA_ContentLayoutProcessor(CXFA_Node* pNode,
                              CXFA_ViewLayoutProcessor* pViewLayoutProcessor);
  ~CXFA_ContentLayoutProcessor();

  Result DoLayout(bool bUseBreakControl, float fHeightLimit, float fRealHeight);
  void DoLayoutPageArea(CXFA_ViewLayoutItem* pPageAreaLayoutItem);

  CXFA_Node* GetFormNode() { return m_pFormNode; }
  RetainPtr<CXFA_ContentLayoutItem> ExtractLayoutItem();

 private:
  class Context {
   public:
    Context();
    ~Context();

    Optional<float> m_fCurColumnWidth;
    UnownedPtr<std::vector<float>> m_prgSpecifiedColumnWidths;
    UnownedPtr<CXFA_ContentLayoutProcessor> m_pOverflowProcessor;
    UnownedPtr<CXFA_Node> m_pOverflowNode;
  };

  Result DoLayoutInternal(bool bUseBreakControl,
                          float fHeightLimit,
                          float fRealHeight,
                          Context* pContext);

  CFX_SizeF GetCurrentComponentSize();
  bool HasLayoutItem() const { return !!m_pLayoutItem; }
  void SplitLayoutItem(float fSplitPos);
  float FindSplitPos(float fProposedSplitPos);
  bool ProcessKeepForSplit(
      CXFA_ContentLayoutProcessor* pChildProcessor,
      Result eRetValue,
      std::vector<RetainPtr<CXFA_ContentLayoutItem>>* rgCurLineLayoutItem,
      float* fContentCurRowAvailWidth,
      float* fContentCurRowHeight,
      float* fContentCurRowY,
      bool* bAddedItemInRow,
      bool* bForceEndPage,
      Result* result);
  void ProcessUnUseOverFlow(
      CXFA_Node* pLeaderNode,
      CXFA_Node* pTrailerNode,
      const RetainPtr<CXFA_ContentLayoutItem>& pTrailerItem,
      CXFA_Node* pFormNode);
  bool IsAddNewRowForTrailer(CXFA_ContentLayoutItem* pTrailerItem);
  bool JudgeLeaderOrTrailerForOccur(CXFA_Node* pFormNode);

  RetainPtr<CXFA_ContentLayoutItem> CreateContentLayoutItem(
      CXFA_Node* pFormNode);

  void SetCurrentComponentPos(const CFX_PointF& pos);
  void SetCurrentComponentSize(const CFX_SizeF& size);

  void SplitLayoutItem(CXFA_ContentLayoutItem* pLayoutItem,
                       CXFA_ContentLayoutItem* pSecondParent,
                       float fSplitPos);
  float InsertKeepLayoutItems();
  bool CalculateRowChildPosition(
      std::vector<RetainPtr<CXFA_ContentLayoutItem>> (&rgCurLineLayoutItems)[3],
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
  bool JudgePutNextPage(
      CXFA_ContentLayoutItem* pParentLayoutItem,
      float fChildHeight,
      std::vector<RetainPtr<CXFA_ContentLayoutItem>>* pKeepItems);

  void DoLayoutPositionedContainer(Context* pContext);
  void DoLayoutTableContainer(CXFA_Node* pLayoutNode);
  Result DoLayoutFlowedContainer(bool bUseBreakControl,
                                 XFA_AttributeValue eFlowStrategy,
                                 float fHeightLimit,
                                 float fRealHeight,
                                 Context* pContext,
                                 bool bRootForceTb);
  void DoLayoutField();

  void GotoNextContainerNodeSimple(bool bUsePageBreak);
  Stage GotoNextContainerNode(Stage nCurStage,
                              bool bUsePageBreak,
                              CXFA_Node* pParentContainer,
                              CXFA_Node** pCurActionNode);

  Optional<Stage> ProcessKeepNodesForCheckNext(CXFA_Node** pCurActionNode,
                                               CXFA_Node** pNextContainer,
                                               bool* pLastKeepNode);

  Optional<Stage> ProcessKeepNodesForBreakBefore(CXFA_Node** pCurActionNode,
                                                 CXFA_Node* pContainerNode);

  CXFA_Node* GetSubformSetParent(CXFA_Node* pSubformSet);

  void UpdatePendingItemLayout(
      const RetainPtr<CXFA_ContentLayoutItem>& pLayoutItem);
  void AddTrailerBeforeSplit(
      float fSplitPos,
      const RetainPtr<CXFA_ContentLayoutItem>& pTrailerLayoutItem,
      bool bUseInherited);
  void AddLeaderAfterSplit(
      const RetainPtr<CXFA_ContentLayoutItem>& pLeaderLayoutItem);
  void AddPendingNode(CXFA_Node* pPendingNode, bool bBreakPending);
  float InsertPendingItems(CXFA_Node* pCurChildNode);
  Result InsertFlowedItem(
      CXFA_ContentLayoutProcessor* pProcessor,
      bool bContainerWidthAutoSize,
      bool bContainerHeightAutoSize,
      float fContainerHeight,
      XFA_AttributeValue eFlowStrategy,
      uint8_t* uCurHAlignState,
      std::vector<RetainPtr<CXFA_ContentLayoutItem>> (&rgCurLineLayoutItems)[3],
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

  Optional<Stage> HandleKeep(CXFA_Node* pBreakAfterNode,
                             CXFA_Node** pCurActionNode);
  Optional<Stage> HandleBookendLeader(CXFA_Node* pParentContainer,
                                      CXFA_Node** pCurActionNode);
  Optional<Stage> HandleBreakBefore(CXFA_Node* pChildContainer,
                                    CXFA_Node** pCurActionNode);
  Optional<Stage> HandleBreakAfter(CXFA_Node* pChildContainer,
                                   CXFA_Node** pCurActionNode);
  Optional<Stage> HandleCheckNextChildContainer(CXFA_Node* pParentContainer,
                                                CXFA_Node* pChildContainer,
                                                CXFA_Node** pCurActionNode);
  Optional<Stage> HandleBookendTrailer(CXFA_Node* pParentContainer,
                                       CXFA_Node** pCurActionNode);
  void ProcessKeepNodesEnd();
  void AdjustContainerSpecifiedSize(Context* pContext,
                                    CFX_SizeF* pSize,
                                    bool* pContainerWidthAutoSize,
                                    bool* pContainerHeightAutoSize);
  CXFA_ContentLayoutItem* FindLastContentLayoutItem(
      XFA_AttributeValue eFlowStrategy);
  CFX_SizeF CalculateLayoutItemSize(const CXFA_ContentLayoutItem* pLayoutChild);

  Stage m_nCurChildNodeStage = Stage::kNone;
  Result m_ePreProcessRs = Result::kDone;
  bool m_bBreakPending = true;
  bool m_bUseInherited = false;
  bool m_bKeepBreakFinish = false;
  bool m_bIsProcessKeep = false;
  bool m_bHasAvailHeight = true;
  float m_fUsedSize = 0;
  float m_fLastRowWidth = 0;
  float m_fLastRowY = 0;
  float m_fWidthLimit = 0;
  CXFA_Node* const m_pFormNode;
  CXFA_Node* m_pCurChildNode = nullptr;
  CXFA_Node* m_pKeepHeadNode = nullptr;
  CXFA_Node* m_pKeepTailNode = nullptr;
  RetainPtr<CXFA_ContentLayoutItem> m_pLayoutItem;
  RetainPtr<CXFA_ContentLayoutItem> m_pOldLayoutItem;
  UnownedPtr<CXFA_ViewLayoutProcessor> m_pViewLayoutProcessor;
  std::vector<float> m_rgSpecifiedColumnWidths;
  std::vector<RetainPtr<CXFA_ContentLayoutItem>> m_ArrayKeepItems;
  std::list<CXFA_Node*> m_PendingNodes;
  std::map<CXFA_Node*, int32_t> m_PendingNodesCount;
  std::unique_ptr<CXFA_ContentLayoutProcessor> m_pCurChildPreprocessor;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTPROCESSOR_H_
