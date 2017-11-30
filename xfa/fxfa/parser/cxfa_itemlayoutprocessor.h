// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_ITEMLAYOUTPROCESSOR_H_
#define XFA_FXFA_PARSER_CXFA_ITEMLAYOUTPROCESSOR_H_

#include <float.h>

#include <list>
#include <map>
#include <tuple>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fxfa/fxfa_basic.h"

#define XFA_LAYOUT_INVALIDNODE ((CXFA_Node*)(intptr_t)-1)
#define XFA_LAYOUT_FLOAT_PERCISION (0.0005f)

class CXFA_ContainerLayoutItem;
class CXFA_ContentLayoutItem;
class CXFA_ItemLayoutProcessor;
class CXFA_LayoutContext;
class CXFA_LayoutPageMgr;
class CXFA_LayoutProcessor;
class CXFA_Node;

enum class XFA_ItemLayoutProcessorResult {
  Done,
  PageFullBreak,
  RowFullBreak,
  ManualBreak,
};

enum class XFA_ItemLayoutProcessorStages {
  None,
  BookendLeader,
  BreakBefore,
  Keep,
  Container,
  BreakAfter,
  BookendTrailer,
  Done,
};

bool XFA_ItemLayoutProcessor_IsTakingSpace(CXFA_Node* pNode);

class CXFA_ItemLayoutProcessor {
 public:
  static bool IncrementRelayoutNode(CXFA_LayoutProcessor* pLayoutProcessor,
                                    CXFA_Node* pNode,
                                    CXFA_Node* pParentNode);

  CXFA_ItemLayoutProcessor(CXFA_Node* pNode, CXFA_LayoutPageMgr* pPageMgr);
  ~CXFA_ItemLayoutProcessor();

  XFA_ItemLayoutProcessorResult DoLayout(bool bUseBreakControl,
                                         float fHeightLimit,
                                         float fRealHeight,
                                         CXFA_LayoutContext* pContext);
  void DoLayoutPageArea(CXFA_ContainerLayoutItem* pPageAreaLayoutItem);

  CFX_SizeF GetCurrentComponentSize();
  CXFA_Node* GetFormNode() { return m_pFormNode; }
  bool HasLayoutItem() const { return !!m_pLayoutItem; }
  CXFA_ContentLayoutItem* ExtractLayoutItem();
  void SplitLayoutItem(float fSplitPos);

  float FindSplitPos(float fProposedSplitPos);

  bool ProcessKeepForSplit(
      CXFA_ItemLayoutProcessor* pParentProcessor,
      CXFA_ItemLayoutProcessor* pChildProcessor,
      XFA_ItemLayoutProcessorResult eRetValue,
      std::vector<CXFA_ContentLayoutItem*>* rgCurLineLayoutItem,
      float* fContentCurRowAvailWidth,
      float* fContentCurRowHeight,
      float* fContentCurRowY,
      bool* bAddedItemInRow,
      bool* bForceEndPage,
      XFA_ItemLayoutProcessorResult* result);
  void ProcessUnUseOverFlow(CXFA_Node* pLeaderNode,
                            CXFA_Node* pTrailerNode,
                            CXFA_ContentLayoutItem* pTrailerItem,
                            CXFA_Node* pFormNode);
  bool IsAddNewRowForTrailer(CXFA_ContentLayoutItem* pTrailerItem);
  bool JudgeLeaderOrTrailerForOccur(CXFA_Node* pFormNode);

  CXFA_ContentLayoutItem* CreateContentLayoutItem(CXFA_Node* pFormNode);

  CXFA_Node* m_pFormNode;
  CXFA_ContentLayoutItem* m_pLayoutItem;
  CXFA_Node* m_pCurChildNode;
  float m_fUsedSize;
  CXFA_LayoutPageMgr* m_pPageMgr;
  std::list<CXFA_Node*> m_PendingNodes;
  bool m_bBreakPending;
  std::vector<float> m_rgSpecifiedColumnWidths;
  std::vector<CXFA_ContentLayoutItem*> m_arrayKeepItems;
  float m_fLastRowWidth;
  float m_fLastRowY;
  bool m_bUseInheriated;
  XFA_ItemLayoutProcessorResult m_ePreProcessRs;

 private:
  void SetCurrentComponentPos(const CFX_PointF& pos);
  void SetCurrentComponentSize(const CFX_SizeF& size);

  void SplitLayoutItem(CXFA_ContentLayoutItem* pLayoutItem,
                       CXFA_ContentLayoutItem* pSecondParent,
                       float fSplitPos);
  float InsertKeepLayoutItems();
  bool CalculateRowChildPosition(
      std::vector<CXFA_ContentLayoutItem*> (&rgCurLineLayoutItems)[3],
      XFA_AttributeEnum eFlowStrategy,
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

  void DoLayoutPositionedContainer(CXFA_LayoutContext* pContext);
  void DoLayoutTableContainer(CXFA_Node* pLayoutNode);
  XFA_ItemLayoutProcessorResult DoLayoutFlowedContainer(
      bool bUseBreakControl,
      XFA_AttributeEnum eFlowStrategy,
      float fHeightLimit,
      float fRealHeight,
      CXFA_LayoutContext* pContext,
      bool bRootForceTb);
  void DoLayoutField();

  void GotoNextContainerNode(CXFA_Node*& pCurActionNode,
                             XFA_ItemLayoutProcessorStages& nCurStage,
                             CXFA_Node* pParentContainer,
                             bool bUsePageBreak);

  bool ProcessKeepNodesForCheckNext(CXFA_Node*& pCurActionNode,
                                    XFA_ItemLayoutProcessorStages& nCurStage,
                                    CXFA_Node*& pNextContainer,
                                    bool& bLastKeepNode);

  bool ProcessKeepNodesForBreakBefore(CXFA_Node*& pCurActionNode,
                                      XFA_ItemLayoutProcessorStages& nCurStage,
                                      CXFA_Node* pContainerNode);

  CXFA_Node* GetSubformSetParent(CXFA_Node* pSubformSet);

  bool m_bKeepBreakFinish;
  bool m_bIsProcessKeep;
  CXFA_Node* m_pKeepHeadNode;
  CXFA_Node* m_pKeepTailNode;
  CXFA_ContentLayoutItem* m_pOldLayoutItem;
  CXFA_ItemLayoutProcessor* m_pCurChildPreprocessor;
  XFA_ItemLayoutProcessorStages m_nCurChildNodeStage;
  std::map<CXFA_Node*, int32_t> m_PendingNodesCount;
  float m_fWidthLimite;
  bool m_bHasAvailHeight;
};

#endif  // XFA_FXFA_PARSER_CXFA_ITEMLAYOUTPROCESSOR_H_
