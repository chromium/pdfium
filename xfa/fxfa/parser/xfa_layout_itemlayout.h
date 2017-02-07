// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_LAYOUT_ITEMLAYOUT_H_
#define XFA_FXFA_PARSER_XFA_LAYOUT_ITEMLAYOUT_H_

#include <float.h>

#include <list>
#include <map>
#include <tuple>
#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fxfa/fxfa_basic.h"

#define XFA_LAYOUT_INVALIDNODE ((CXFA_Node*)(intptr_t)-1)
#define XFA_LAYOUT_FLOAT_PERCISION (0.0005f)

class CXFA_ContainerLayoutItem;
class CXFA_ContentLayoutItem;
class CXFA_ItemLayoutProcessor;
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

class CXFA_LayoutContext {
 public:
  CXFA_LayoutContext()
      : m_prgSpecifiedColumnWidths(nullptr),
        m_fCurColumnWidth(0),
        m_bCurColumnWidthAvaiable(false),
        m_pOverflowProcessor(nullptr),
        m_pOverflowNode(nullptr) {}
  ~CXFA_LayoutContext() {}

  CFX_ArrayTemplate<FX_FLOAT>* m_prgSpecifiedColumnWidths;
  FX_FLOAT m_fCurColumnWidth;
  bool m_bCurColumnWidthAvaiable;
  CXFA_ItemLayoutProcessor* m_pOverflowProcessor;
  CXFA_Node* m_pOverflowNode;
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
                                         FX_FLOAT fHeightLimit,
                                         FX_FLOAT fRealHeight,
                                         CXFA_LayoutContext* pContext);
  void DoLayoutPageArea(CXFA_ContainerLayoutItem* pPageAreaLayoutItem);

  CFX_SizeF GetCurrentComponentSize();
  CXFA_Node* GetFormNode() { return m_pFormNode; }
  bool HasLayoutItem() const { return !!m_pLayoutItem; }
  CXFA_ContentLayoutItem* ExtractLayoutItem();
  void SplitLayoutItem(FX_FLOAT fSplitPos);

  FX_FLOAT FindSplitPos(FX_FLOAT fProposedSplitPos);

  bool ProcessKeepForSplit(
      CXFA_ItemLayoutProcessor* pParentProcessor,
      CXFA_ItemLayoutProcessor* pChildProcessor,
      XFA_ItemLayoutProcessorResult eRetValue,
      CFX_ArrayTemplate<CXFA_ContentLayoutItem*>* rgCurLineLayoutItem,
      FX_FLOAT* fContentCurRowAvailWidth,
      FX_FLOAT* fContentCurRowHeight,
      FX_FLOAT* fContentCurRowY,
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
  FX_FLOAT m_fUsedSize;
  CXFA_LayoutPageMgr* m_pPageMgr;
  std::list<CXFA_Node*> m_PendingNodes;
  bool m_bBreakPending;
  CFX_ArrayTemplate<FX_FLOAT> m_rgSpecifiedColumnWidths;
  std::vector<CXFA_ContentLayoutItem*> m_arrayKeepItems;
  FX_FLOAT m_fLastRowWidth;
  FX_FLOAT m_fLastRowY;
  bool m_bUseInheriated;
  XFA_ItemLayoutProcessorResult m_ePreProcessRs;

 private:
  void SetCurrentComponentPos(const CFX_PointF& pos);
  void SetCurrentComponentSize(const CFX_SizeF& size);

  void SplitLayoutItem(CXFA_ContentLayoutItem* pLayoutItem,
                       CXFA_ContentLayoutItem* pSecondParent,
                       FX_FLOAT fSplitPos);
  FX_FLOAT InsertKeepLayoutItems();
  bool CalculateRowChildPosition(
      CFX_ArrayTemplate<CXFA_ContentLayoutItem*> (&rgCurLineLayoutItems)[3],
      XFA_ATTRIBUTEENUM eFlowStrategy,
      bool bContainerHeightAutoSize,
      bool bContainerWidthAutoSize,
      FX_FLOAT* fContentCalculatedWidth,
      FX_FLOAT* fContentCalculatedHeight,
      FX_FLOAT* fContentCurRowY,
      FX_FLOAT fContentCurRowHeight,
      FX_FLOAT fContentWidthLimit,
      bool bRootForceTb);
  void ProcessUnUseBinds(CXFA_Node* pFormNode);
  bool JudgePutNextPage(CXFA_ContentLayoutItem* pParentLayoutItem,
                        FX_FLOAT fChildHeight,
                        std::vector<CXFA_ContentLayoutItem*>* pKeepItems);

  void DoLayoutPositionedContainer(CXFA_LayoutContext* pContext);
  void DoLayoutTableContainer(CXFA_Node* pLayoutNode);
  XFA_ItemLayoutProcessorResult DoLayoutFlowedContainer(
      bool bUseBreakControl,
      XFA_ATTRIBUTEENUM eFlowStrategy,
      FX_FLOAT fHeightLimit,
      FX_FLOAT fRealHeight,
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
  FX_FLOAT m_fWidthLimite;
  bool m_bHasAvailHeight;
};

#endif  // XFA_FXFA_PARSER_XFA_LAYOUT_ITEMLAYOUT_H_
