// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_document_datamerger_imp.h"
#include "xfa_document_layout_imp.h"
#include "xfa_layout_itemlayout.h"
#include "xfa_layout_pagemgr_new.h"
#include "xfa_layout_appadapter.h"
CXFA_LayoutPageMgr::CXFA_LayoutPageMgr(CXFA_LayoutProcessor* pLayoutProcessor)
    : m_pLayoutProcessor(pLayoutProcessor),
      m_pTemplatePageSetRoot(nullptr),
      m_pPageSetLayoutItemRoot(nullptr),
      m_pPageSetCurRoot(nullptr),
      m_pCurrentContainerRecord(nullptr),
      m_pCurPageArea(nullptr),
      m_nAvailPages(0),
      m_nCurPageCount(0),
      m_ePageSetMode(XFA_ATTRIBUTEENUM_OrderedOccurrence),
      m_bCreateOverFlowPage(FALSE) {
}
CXFA_LayoutPageMgr::~CXFA_LayoutPageMgr() {
  ClearData();
  CXFA_LayoutItem* pLayoutItem = GetRootLayoutItem();
  CXFA_LayoutItem* pNextLayout = NULL;
  for (; pLayoutItem; pLayoutItem = pNextLayout) {
    pNextLayout = pLayoutItem->m_pNextSibling;
    XFA_ReleaseLayoutItem(pLayoutItem);
  }
}
FX_BOOL CXFA_LayoutPageMgr::InitLayoutPage(CXFA_Node* pFormNode) {
  PrepareLayout();
  CXFA_Node* pTemplateNode = pFormNode->GetTemplateNode();
  if (!pTemplateNode) {
    return FALSE;
  }
  m_pTemplatePageSetRoot = pTemplateNode->GetProperty(0, XFA_ELEMENT_PageSet);
  ASSERT(m_pTemplatePageSetRoot);
  if (m_pPageSetLayoutItemRoot) {
    m_pPageSetLayoutItemRoot->m_pParent = NULL;
    m_pPageSetLayoutItemRoot->m_pFirstChild = NULL;
    m_pPageSetLayoutItemRoot->m_pNextSibling = NULL;
    m_pPageSetLayoutItemRoot->m_pFormNode = m_pTemplatePageSetRoot;
  } else {
    m_pPageSetLayoutItemRoot =
        new CXFA_ContainerLayoutItem(m_pTemplatePageSetRoot);
  }
  m_pPageSetCurRoot = m_pPageSetLayoutItemRoot;
  m_pTemplatePageSetRoot->SetUserData(XFA_LAYOUTITEMKEY,
                                      (void*)m_pPageSetLayoutItemRoot);
  XFA_ATTRIBUTEENUM eRelation =
      m_pTemplatePageSetRoot->GetEnum(XFA_ATTRIBUTE_Relation);
  if (eRelation != XFA_ATTRIBUTEENUM_Unknown) {
    m_ePageSetMode = eRelation;
  }
  InitPageSetMap();
  CXFA_Node* pPageArea = NULL;
  int32_t iCount = 0;
  for (pPageArea = m_pTemplatePageSetRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
       pPageArea;
       pPageArea = pPageArea->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pPageArea->GetClassID() == XFA_ELEMENT_PageArea) {
      iCount++;
      if (pPageArea->GetFirstChildByClass(XFA_ELEMENT_ContentArea)) {
        return TRUE;
      }
    }
  }
  if (iCount > 0) {
    return FALSE;
  }
  CXFA_Document* pDocument = pTemplateNode->GetDocument();
  IXFA_ObjFactory* pObjFactory = pDocument->GetParser()->GetFactory();
  pPageArea = m_pTemplatePageSetRoot->GetChild(0, XFA_ELEMENT_PageArea);
  if (!pPageArea) {
    pPageArea = pObjFactory->CreateNode(m_pTemplatePageSetRoot->GetPacketID(),
                                        XFA_ELEMENT_PageArea);
    if (!pPageArea) {
      return FALSE;
    }
    m_pTemplatePageSetRoot->InsertChild(pPageArea, NULL);
    pPageArea->SetFlag(XFA_NODEFLAG_Initialized);
  }
  CXFA_Node* pContentArea = pPageArea->GetChild(0, XFA_ELEMENT_ContentArea);
  if (!pContentArea) {
    pContentArea = pObjFactory->CreateNode(pPageArea->GetPacketID(),
                                           XFA_ELEMENT_ContentArea);
    if (!pContentArea) {
      return FALSE;
    }
    pPageArea->InsertChild(pContentArea, NULL);
    pContentArea->SetFlag(XFA_NODEFLAG_Initialized);
    pContentArea->SetMeasure(XFA_ATTRIBUTE_X,
                             CXFA_Measurement(0.25f, XFA_UNIT_In));
    pContentArea->SetMeasure(XFA_ATTRIBUTE_Y,
                             CXFA_Measurement(0.25f, XFA_UNIT_In));
    pContentArea->SetMeasure(XFA_ATTRIBUTE_W,
                             CXFA_Measurement(8.0f, XFA_UNIT_In));
    pContentArea->SetMeasure(XFA_ATTRIBUTE_H,
                             CXFA_Measurement(10.5f, XFA_UNIT_In));
  }
  CXFA_Node* pMedium = pPageArea->GetChild(0, XFA_ELEMENT_Medium);
  if (!pMedium) {
    pMedium =
        pObjFactory->CreateNode(pPageArea->GetPacketID(), XFA_ELEMENT_Medium);
    if (!pContentArea) {
      return FALSE;
    }
    pPageArea->InsertChild(pMedium, NULL);
    pMedium->SetFlag(XFA_NODEFLAG_Initialized);
    pMedium->SetMeasure(XFA_ATTRIBUTE_Short,
                        CXFA_Measurement(8.5f, XFA_UNIT_In));
    pMedium->SetMeasure(XFA_ATTRIBUTE_Long,
                        CXFA_Measurement(11.0f, XFA_UNIT_In));
  }
  return TRUE;
}
FX_BOOL CXFA_LayoutPageMgr::PrepareFirstPage(CXFA_Node* pRootSubform) {
  FX_BOOL bProBreakBefore = FALSE;
  CXFA_Node* pBreakBeforeNode = NULL;
  while (pRootSubform) {
    for (CXFA_Node* pBreakNode =
             pRootSubform->GetNodeItem(XFA_NODEITEM_FirstChild);
         pBreakNode;
         pBreakNode = pBreakNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      XFA_ELEMENT eType = pBreakNode->GetClassID();
      if (eType == XFA_ELEMENT_BreakBefore ||
          (eType == XFA_ELEMENT_Break &&
           pBreakNode->GetEnum(XFA_ATTRIBUTE_Before) !=
               XFA_ATTRIBUTEENUM_Auto)) {
        bProBreakBefore = TRUE;
        pBreakBeforeNode = pBreakNode;
        break;
      }
    }
    if (bProBreakBefore) {
      break;
    }
    bProBreakBefore = TRUE;
    pRootSubform = pRootSubform->GetFirstChildByClass(XFA_ELEMENT_Subform);
    while (pRootSubform &&
           !XFA_ItemLayoutProcessor_IsTakingSpace(pRootSubform)) {
      pRootSubform = pRootSubform->GetNextSameClassSibling(XFA_ELEMENT_Subform);
    }
  }
  CXFA_Node *pLeader, *pTrailer;
  if (pBreakBeforeNode &&
      ExecuteBreakBeforeOrAfter(pBreakBeforeNode, TRUE, pLeader, pTrailer)) {
    m_pCurrentContainerRecord = m_rgProposedContainerRecord.GetHeadPosition();
    return TRUE;
  }
  return AppendNewPage(TRUE);
}
FX_BOOL CXFA_LayoutPageMgr::AppendNewPage(FX_BOOL bFirstTemPage) {
  if (m_pCurrentContainerRecord !=
      m_rgProposedContainerRecord.GetTailPosition()) {
    return TRUE;
  }
  CXFA_Node* pPageNode = GetNextAvailPageArea(NULL);
  if (!pPageNode) {
    return FALSE;
  }
  if (bFirstTemPage && m_pCurrentContainerRecord == NULL) {
    m_pCurrentContainerRecord = m_rgProposedContainerRecord.GetHeadPosition();
  }
  return !bFirstTemPage || m_pCurrentContainerRecord != NULL;
}
static void XFA_LayoutItemMgr_ReorderLayoutItemToTail(
    CXFA_ContainerLayoutItem* pLayoutItem) {
  CXFA_ContainerLayoutItem* pParentLayoutItem =
      (CXFA_ContainerLayoutItem*)pLayoutItem->m_pParent;
  if (!pParentLayoutItem) {
    return;
  }
  pParentLayoutItem->RemoveChild(pLayoutItem);
  pParentLayoutItem->AddChild(pLayoutItem);
}
static void XFA_LayoutItemMgr_RemoveLayoutItem(
    CXFA_ContainerLayoutItem* pLayoutItem) {
  CXFA_ContainerLayoutItem* pParentLayoutItem =
      (CXFA_ContainerLayoutItem*)pLayoutItem->m_pParent;
  if (!pParentLayoutItem) {
    return;
  }
  pParentLayoutItem->RemoveChild(pLayoutItem);
}
void CXFA_LayoutPageMgr::RemoveLayoutRecord(CXFA_ContainerRecord* pNewRecord,
                                            CXFA_ContainerRecord* pPrevRecord) {
  if (!pNewRecord || !pPrevRecord) {
    return;
  }
  if (pNewRecord->pCurPageSet != pPrevRecord->pCurPageSet) {
    XFA_LayoutItemMgr_RemoveLayoutItem(pNewRecord->pCurPageSet);
    return;
  }
  if (pNewRecord->pCurPageArea != pPrevRecord->pCurPageArea) {
    XFA_LayoutItemMgr_RemoveLayoutItem(pNewRecord->pCurPageArea);
    return;
  }
  if (pNewRecord->pCurContentArea != pPrevRecord->pCurContentArea) {
    XFA_LayoutItemMgr_RemoveLayoutItem(pNewRecord->pCurContentArea);
    return;
  }
}
void CXFA_LayoutPageMgr::ReorderPendingLayoutRecordToTail(
    CXFA_ContainerRecord* pNewRecord,
    CXFA_ContainerRecord* pPrevRecord) {
  if (!pNewRecord || !pPrevRecord) {
    return;
  }
  if (pNewRecord->pCurPageSet != pPrevRecord->pCurPageSet) {
    XFA_LayoutItemMgr_ReorderLayoutItemToTail(pNewRecord->pCurPageSet);
    return;
  }
  if (pNewRecord->pCurPageArea != pPrevRecord->pCurPageArea) {
    XFA_LayoutItemMgr_ReorderLayoutItemToTail(pNewRecord->pCurPageArea);
    return;
  }
  if (pNewRecord->pCurContentArea != pPrevRecord->pCurContentArea) {
    XFA_LayoutItemMgr_ReorderLayoutItemToTail(pNewRecord->pCurContentArea);
    return;
  }
}
void CXFA_LayoutPageMgr::SubmitContentItem(
    CXFA_ContentLayoutItem* pContentLayoutItem,
    XFA_ItemLayoutProcessorResult eStatus) {
  if (pContentLayoutItem) {
    GetCurrentContainerRecord()->pCurContentArea->AddChild(pContentLayoutItem);
    m_bCreateOverFlowPage = FALSE;
  }
  if (eStatus != XFA_ItemLayoutProcessorResult_Done) {
    if (eStatus == XFA_ItemLayoutProcessorResult_PageFullBreak &&
        m_pCurrentContainerRecord ==
            m_rgProposedContainerRecord.GetTailPosition()) {
      AppendNewPage();
    }
    m_pCurrentContainerRecord = m_rgProposedContainerRecord.GetTailPosition();
    m_pCurPageArea = GetCurrentContainerRecord()->pCurPageArea->m_pFormNode;
  }
}
FX_FLOAT CXFA_LayoutPageMgr::GetAvailHeight() {
  FX_FLOAT fAvailHeight =
      GetCurrentContainerRecord()
          ->pCurContentArea->m_pFormNode->GetMeasure(XFA_ATTRIBUTE_H)
          .ToUnit(XFA_UNIT_Pt);
  if (fAvailHeight < XFA_LAYOUT_FLOAT_PERCISION) {
    if (m_pCurrentContainerRecord ==
        m_rgProposedContainerRecord.GetHeadPosition()) {
      fAvailHeight = 0;
    } else {
      fAvailHeight = XFA_LAYOUT_FLOAT_MAX;
    }
  }
  return fAvailHeight;
}
static CXFA_Node* XFA_ResolveBreakTarget(CXFA_Node* pPageSetRoot,
                                         FX_BOOL bNewExprStyle,
                                         CFX_WideStringC& wsTargetExpr) {
  CXFA_Document* pDocument = pPageSetRoot->GetDocument();
  if (wsTargetExpr.IsEmpty()) {
    return NULL;
  }
  CFX_WideString wsTargetAll = wsTargetExpr;
  wsTargetAll.TrimLeft();
  wsTargetAll.TrimRight();
  int32_t iSpliteIndex = 0;
  FX_BOOL bTargetAllFind = TRUE;
  while (iSpliteIndex != -1) {
    CFX_WideString wsTargetExpr;
    int32_t iSpliteNextIndex = 0;
    if (!bTargetAllFind) {
      iSpliteNextIndex = wsTargetAll.Find(' ', iSpliteIndex);
      wsTargetExpr =
          wsTargetAll.Mid(iSpliteIndex, iSpliteNextIndex - iSpliteIndex);
    } else {
      wsTargetExpr = wsTargetAll;
    }
    if (wsTargetExpr.IsEmpty()) {
      return NULL;
    }
    bTargetAllFind = FALSE;
    if (wsTargetExpr.GetAt(0) == '#') {
      CXFA_Node* pNode = pDocument->GetNodeByID(
          (CXFA_Node*)pDocument->GetXFANode(XFA_HASHCODE_Template),
          wsTargetExpr.Mid(1));
      if (pNode) {
        return pNode;
      }
    } else if (bNewExprStyle) {
      CFX_WideString wsProcessedTarget = wsTargetExpr;
      if (wsTargetExpr.Left(4) == FX_WSTRC(L"som(") &&
          wsTargetExpr.Right(1) == FX_WSTRC(L")")) {
        wsProcessedTarget = wsTargetExpr.Mid(4, wsTargetExpr.GetLength() - 5);
      }
      XFA_RESOLVENODE_RS rs;
      int32_t iCount = pDocument->GetScriptContext()->ResolveObjects(
          pPageSetRoot, wsProcessedTarget, rs,
          XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
              XFA_RESOLVENODE_Attributes | XFA_RESOLVENODE_Siblings |
              XFA_RESOLVENODE_Parent);
      if (iCount > 0 && rs.nodes[0]->IsNode()) {
        return (CXFA_Node*)rs.nodes[0];
      }
    }
    iSpliteIndex = iSpliteNextIndex;
  }
  return NULL;
}

FX_BOOL XFA_LayoutPageMgr_RunBreakTestScript(CXFA_Node* pTestScript) {
  CFX_WideString wsExpression;
  pTestScript->TryContent(wsExpression);
  if (wsExpression.IsEmpty()) {
    return TRUE;
  }
  return pTestScript->GetDocument()->GetParser()->GetNotify()->RunScript(
      pTestScript, pTestScript->GetNodeItem(XFA_NODEITEM_Parent,
                                            XFA_OBJECTTYPE_ContainerNode));
}
CXFA_ContainerRecord* CXFA_LayoutPageMgr::CreateContainerRecord(
    CXFA_Node* pPageNode,
    FX_BOOL bCreateNew) {
  CXFA_ContainerRecord* pNewRecord = new CXFA_ContainerRecord();
  if (m_pCurrentContainerRecord) {
    if (!IsPageSetRootOrderedOccurrence() || pPageNode == NULL) {
      *pNewRecord = *GetCurrentContainerRecord();
      m_rgProposedContainerRecord.AddTail(pNewRecord);
      return pNewRecord;
    }
    CXFA_Node* pPageSet = pPageNode->GetNodeItem(XFA_NODEITEM_Parent);
    if (!bCreateNew) {
      if (pPageSet == m_pTemplatePageSetRoot) {
        pNewRecord->pCurPageSet = m_pPageSetCurRoot;
      } else {
        CXFA_ContainerLayoutItem* pParentLayoutItem =
            (CXFA_ContainerLayoutItem*)pPageSet->GetUserData(XFA_LAYOUTITEMKEY);
        if (pParentLayoutItem == NULL) {
          pParentLayoutItem = m_pPageSetCurRoot;
        }
        pNewRecord->pCurPageSet = pParentLayoutItem;
      }
    } else {
      CXFA_ContainerLayoutItem* pParentPageSetLayout = NULL;
      if (pPageSet == GetCurrentContainerRecord()->pCurPageSet->m_pFormNode) {
        pParentPageSetLayout =
            (CXFA_ContainerLayoutItem*)
                GetCurrentContainerRecord()->pCurPageSet->m_pParent;
      } else {
        pParentPageSetLayout =
            (CXFA_ContainerLayoutItem*)pPageSet->GetNodeItem(
                                                     XFA_NODEITEM_Parent)
                ->GetUserData(XFA_LAYOUTITEMKEY);
      }
      CXFA_ContainerLayoutItem* pPageSetLayoutItem =
          new CXFA_ContainerLayoutItem(pPageSet);
      pPageSet->SetUserData(XFA_LAYOUTITEMKEY, (void*)pPageSetLayoutItem);
      if (pParentPageSetLayout == NULL) {
        CXFA_ContainerLayoutItem* pPrePageSet = m_pPageSetLayoutItemRoot;
        while (pPrePageSet->m_pNextSibling) {
          pPrePageSet = (CXFA_ContainerLayoutItem*)pPrePageSet->m_pNextSibling;
        }
        pPrePageSet->m_pNextSibling = pPageSetLayoutItem;
        m_pPageSetCurRoot = pPageSetLayoutItem;
      } else {
        pParentPageSetLayout->AddChild(pPageSetLayoutItem);
      }
      pNewRecord->pCurPageSet = pPageSetLayoutItem;
    }
  } else {
    if (pPageNode) {
      CXFA_Node* pPageSet = pPageNode->GetNodeItem(XFA_NODEITEM_Parent);
      if (pPageSet == m_pTemplatePageSetRoot) {
        pNewRecord->pCurPageSet = m_pPageSetLayoutItemRoot;
      } else {
        CXFA_ContainerLayoutItem* pPageSetLayoutItem =
            new CXFA_ContainerLayoutItem(pPageSet);
        pPageSet->SetUserData(XFA_LAYOUTITEMKEY, (void*)pPageSetLayoutItem);
        m_pPageSetLayoutItemRoot->AddChild(pPageSetLayoutItem);
        pNewRecord->pCurPageSet = pPageSetLayoutItem;
      }
    } else {
      pNewRecord->pCurPageSet = m_pPageSetLayoutItemRoot;
    }
  }
  m_rgProposedContainerRecord.AddTail(pNewRecord);
  return pNewRecord;
}
void CXFA_LayoutPageMgr::AddPageAreaLayoutItem(CXFA_ContainerRecord* pNewRecord,
                                               CXFA_Node* pNewPageArea) {
  CXFA_ContainerLayoutItem* pNewPageAreaLayoutItem = NULL;
  if (m_PageArray.GetSize() > m_nAvailPages) {
    CXFA_ContainerLayoutItem* pContainerItem = m_PageArray[m_nAvailPages];
    pContainerItem->m_pFormNode = pNewPageArea;
    m_nAvailPages++;
    pNewPageAreaLayoutItem = pContainerItem;
  } else {
    IXFA_Notify* pNotify =
        pNewPageArea->GetDocument()->GetParser()->GetNotify();
    CXFA_ContainerLayoutItem* pContainerItem =
        (CXFA_ContainerLayoutItem*)pNotify->OnCreateLayoutItem(pNewPageArea);
    m_PageArray.Add(pContainerItem);
    m_nAvailPages++;
    pNotify->OnPageEvent(pContainerItem, XFA_PAGEEVENT_PageAdded,
                         (void*)(uintptr_t)m_nAvailPages);
    pNewPageAreaLayoutItem = pContainerItem;
  }
  pNewRecord->pCurPageSet->AddChild(pNewPageAreaLayoutItem);
  pNewRecord->pCurPageArea = pNewPageAreaLayoutItem;
  pNewRecord->pCurContentArea = NULL;
}
void CXFA_LayoutPageMgr::AddContentAreaLayoutItem(
    CXFA_ContainerRecord* pNewRecord,
    CXFA_Node* pContentArea) {
  if (pContentArea == NULL) {
    pNewRecord->pCurContentArea = NULL;
    return;
  }
  CXFA_ContainerLayoutItem* pNewContentAreaLayoutItem =
      new CXFA_ContainerLayoutItem(pContentArea);
  ASSERT(pNewRecord->pCurPageArea);
  pNewRecord->pCurPageArea->AddChild(pNewContentAreaLayoutItem);
  pNewRecord->pCurContentArea = pNewContentAreaLayoutItem;
}
class CXFA_TraverseStrategy_PageSetContainerLayoutItem {
 public:
  static inline CXFA_ContainerLayoutItem* GetFirstChild(
      CXFA_ContainerLayoutItem* pLayoutItem) {
    if (pLayoutItem->m_pFormNode->GetClassID() == XFA_ELEMENT_PageSet) {
      CXFA_ContainerLayoutItem* pChildItem =
          (CXFA_ContainerLayoutItem*)pLayoutItem->m_pFirstChild;
      while (pChildItem &&
             pChildItem->m_pFormNode->GetClassID() != XFA_ELEMENT_PageSet) {
        pChildItem = (CXFA_ContainerLayoutItem*)pChildItem->m_pNextSibling;
      }
      return pChildItem;
    }
    return NULL;
  }
  static inline CXFA_ContainerLayoutItem* GetNextSibling(
      CXFA_ContainerLayoutItem* pLayoutItem) {
    CXFA_ContainerLayoutItem* pChildItem =
        (CXFA_ContainerLayoutItem*)pLayoutItem->m_pNextSibling;
    while (pChildItem &&
           pChildItem->m_pFormNode->GetClassID() != XFA_ELEMENT_PageSet) {
      pChildItem = (CXFA_ContainerLayoutItem*)pChildItem->m_pNextSibling;
    }
    return pChildItem;
  }
  static inline CXFA_ContainerLayoutItem* GetParent(
      CXFA_ContainerLayoutItem* pLayoutItem) {
    return (CXFA_ContainerLayoutItem*)pLayoutItem->m_pParent;
  }
};
void CXFA_LayoutPageMgr::FinishPaginatedPageSets() {
  CXFA_ContainerLayoutItem* pRootPageSetLayoutItem = m_pPageSetLayoutItemRoot;
  for (; pRootPageSetLayoutItem;
       pRootPageSetLayoutItem =
           (CXFA_ContainerLayoutItem*)pRootPageSetLayoutItem->m_pNextSibling) {
    CXFA_NodeIteratorTemplate<CXFA_ContainerLayoutItem,
                              CXFA_TraverseStrategy_PageSetContainerLayoutItem>
        sIterator(pRootPageSetLayoutItem);
    for (CXFA_ContainerLayoutItem* pPageSetLayoutItem = sIterator.GetCurrent();
         pPageSetLayoutItem; pPageSetLayoutItem = sIterator.MoveToNext()) {
      XFA_ATTRIBUTEENUM ePageRelation =
          pPageSetLayoutItem->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Relation);
      switch (ePageRelation) {
        case XFA_ATTRIBUTEENUM_OrderedOccurrence:
        default: { ProcessLastPageSet(); } break;
        case XFA_ATTRIBUTEENUM_SimplexPaginated:
        case XFA_ATTRIBUTEENUM_DuplexPaginated: {
          CXFA_LayoutItem* pLastPageAreaLayoutItem = NULL;
          int32_t nPageAreaCount = 0;
          for (CXFA_LayoutItem* pPageAreaLayoutItem =
                   pPageSetLayoutItem->m_pFirstChild;
               pPageAreaLayoutItem;
               pPageAreaLayoutItem = pPageAreaLayoutItem->m_pNextSibling) {
            if (pPageAreaLayoutItem->m_pFormNode->GetClassID() !=
                XFA_ELEMENT_PageArea) {
              continue;
            }
            nPageAreaCount++;
            pLastPageAreaLayoutItem = pPageAreaLayoutItem;
          }
          if (!pLastPageAreaLayoutItem) {
            break;
          }
          if (!FindPageAreaFromPageSet_SimplexDuplex(
                  pPageSetLayoutItem->m_pFormNode, NULL, NULL, NULL, TRUE, TRUE,
                  nPageAreaCount == 1 ? XFA_ATTRIBUTEENUM_Only
                                      : XFA_ATTRIBUTEENUM_Last) &&
              (nPageAreaCount == 1 &&
               !FindPageAreaFromPageSet_SimplexDuplex(
                   pPageSetLayoutItem->m_pFormNode, NULL, NULL, NULL, TRUE,
                   TRUE, XFA_ATTRIBUTEENUM_Last))) {
            break;
          }
          CXFA_Node* pNode = m_pCurPageArea;
          XFA_ATTRIBUTEENUM eCurChoice =
              pNode->GetEnum(XFA_ATTRIBUTE_PagePosition);
          if (eCurChoice == XFA_ATTRIBUTEENUM_Last) {
            XFA_ATTRIBUTEENUM eOddOrEven = XFA_ATTRIBUTEENUM_Any;
            pNode->TryEnum(XFA_ATTRIBUTE_OddOrEven, eOddOrEven);
            XFA_ATTRIBUTEENUM eLastChoice =
                pLastPageAreaLayoutItem->m_pFormNode->GetEnum(
                    XFA_ATTRIBUTE_PagePosition);
            if (eLastChoice == XFA_ATTRIBUTEENUM_First &&
                (ePageRelation == XFA_ATTRIBUTEENUM_SimplexPaginated ||
                 eOddOrEven != XFA_ATTRIBUTEENUM_Odd)) {
              CXFA_ContainerRecord* pRecord = CreateContainerRecord();
              AddPageAreaLayoutItem(pRecord, pNode);
              break;
              ;
            }
          }
          FX_BOOL bUsable = TRUE;
          CFX_ArrayTemplate<FX_FLOAT> rgUsedHeights;
          for (CXFA_LayoutItem* pChildLayoutItem =
                   pLastPageAreaLayoutItem->m_pFirstChild;
               pChildLayoutItem;
               pChildLayoutItem = pChildLayoutItem->m_pNextSibling) {
            if (pChildLayoutItem->m_pFormNode->GetClassID() !=
                XFA_ELEMENT_ContentArea) {
              continue;
            }
            FX_FLOAT fUsedHeight = 0;
            for (CXFA_LayoutItem* pContentChildLayoutItem =
                     pChildLayoutItem->m_pFirstChild;
                 pContentChildLayoutItem;
                 pContentChildLayoutItem =
                     pContentChildLayoutItem->m_pNextSibling) {
              if (CXFA_ContentLayoutItem* pContent =
                      pContentChildLayoutItem->AsContentLayoutItem()) {
                fUsedHeight += pContent->m_sSize.y;
              }
            }
            rgUsedHeights.Add(fUsedHeight);
          }
          int32_t iCurContentAreaIndex = -1;
          for (CXFA_Node* pContentAreaNode =
                   pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
               pContentAreaNode;
               pContentAreaNode =
                   pContentAreaNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
            if (pContentAreaNode->GetClassID() != XFA_ELEMENT_ContentArea) {
              continue;
            }
            iCurContentAreaIndex++;
            if (rgUsedHeights[iCurContentAreaIndex] >
                pContentAreaNode->GetMeasure(XFA_ATTRIBUTE_H)
                        .ToUnit(XFA_UNIT_Pt) +
                    XFA_LAYOUT_FLOAT_PERCISION) {
              bUsable = FALSE;
              break;
            }
          }
          if (bUsable) {
            CXFA_LayoutItem* pChildLayoutItem =
                pLastPageAreaLayoutItem->m_pFirstChild;
            CXFA_Node* pContentAreaNode =
                pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
            pLastPageAreaLayoutItem->m_pFormNode = pNode;
            while (pChildLayoutItem && pContentAreaNode) {
              if (pChildLayoutItem->m_pFormNode->GetClassID() !=
                  XFA_ELEMENT_ContentArea) {
                pChildLayoutItem = pChildLayoutItem->m_pNextSibling;
                continue;
              }
              if (pContentAreaNode->GetClassID() != XFA_ELEMENT_ContentArea) {
                pContentAreaNode =
                    pContentAreaNode->GetNodeItem(XFA_NODEITEM_NextSibling);
                continue;
              }
              pChildLayoutItem->m_pFormNode = pContentAreaNode;
              pChildLayoutItem = pChildLayoutItem->m_pNextSibling;
              pContentAreaNode =
                  pContentAreaNode->GetNodeItem(XFA_NODEITEM_NextSibling);
            }
          } else if (pNode->GetEnum(XFA_ATTRIBUTE_PagePosition) ==
                     XFA_ATTRIBUTEENUM_Last) {
            CXFA_ContainerRecord* pRecord = CreateContainerRecord();
            AddPageAreaLayoutItem(pRecord, pNode);
          }
        } break;
      }
    }
  }
}
int32_t CXFA_LayoutPageMgr::GetPageCount() const {
  return m_PageArray.GetSize();
}
IXFA_LayoutPage* CXFA_LayoutPageMgr::GetPage(int32_t index) const {
  if (index < 0 || index >= m_PageArray.GetSize())
    return nullptr;
  return m_PageArray[index];
}
int32_t CXFA_LayoutPageMgr::GetPageIndex(const IXFA_LayoutPage* pPage) const {
  // FIXME: Find() method should take const.
  return m_PageArray.Find(static_cast<CXFA_ContainerLayoutItem*>(
      const_cast<IXFA_LayoutPage*>(pPage)));
}
FX_BOOL CXFA_LayoutPageMgr::RunBreak(XFA_ELEMENT eBreakType,
                                     XFA_ATTRIBUTEENUM eTargetType,
                                     CXFA_Node* pTarget,
                                     FX_BOOL bStartNew) {
  FX_BOOL bRet = FALSE;
  switch (eTargetType) {
    case XFA_ATTRIBUTEENUM_ContentArea:
      if (pTarget && pTarget->GetClassID() != XFA_ELEMENT_ContentArea) {
        pTarget = NULL;
      }
      if (!pTarget || !m_pCurrentContainerRecord ||
          pTarget !=
              GetCurrentContainerRecord()->pCurContentArea->m_pFormNode ||
          bStartNew) {
        CXFA_Node* pPageArea = NULL;
        if (pTarget) {
          pPageArea = pTarget->GetNodeItem(XFA_NODEITEM_Parent);
        }
        pPageArea = GetNextAvailPageArea(pPageArea, pTarget);
        bRet = pPageArea != NULL;
      }
      break;
    case XFA_ATTRIBUTEENUM_PageArea:
      if (pTarget && pTarget->GetClassID() != XFA_ELEMENT_PageArea) {
        pTarget = NULL;
      }
      if (!pTarget || !m_pCurrentContainerRecord ||
          pTarget != GetCurrentContainerRecord()->pCurPageArea->m_pFormNode ||
          bStartNew) {
        CXFA_Node* pPageArea = GetNextAvailPageArea(pTarget, NULL, TRUE);
        bRet = pPageArea != NULL;
      }
      break;
    case XFA_ATTRIBUTEENUM_PageOdd:
      if (pTarget && pTarget->GetClassID() != XFA_ELEMENT_PageArea) {
        pTarget = NULL;
      }
      if (m_nAvailPages % 2 != 1 || !m_pCurrentContainerRecord ||
          (pTarget &&
           pTarget != GetCurrentContainerRecord()->pCurPageArea->m_pFormNode) ||
          bStartNew) {
        if (m_nAvailPages % 2 == 1) {
        }
      }
      break;
    case XFA_ATTRIBUTEENUM_PageEven:
      if (pTarget && pTarget->GetClassID() != XFA_ELEMENT_PageArea) {
        pTarget = NULL;
      }
      if (m_nAvailPages % 2 != 0 || !m_pCurrentContainerRecord ||
          (pTarget &&
           pTarget != GetCurrentContainerRecord()->pCurPageArea->m_pFormNode) ||
          bStartNew) {
        if (m_nAvailPages % 2 == 0) {
        }
      }
      break;
    case XFA_ATTRIBUTEENUM_Auto:
    default:
      break;
      ;
  }
  return bRet;
}
FX_BOOL CXFA_LayoutPageMgr::ExecuteBreakBeforeOrAfter(
    CXFA_Node* pCurNode,
    FX_BOOL bBefore,
    CXFA_Node*& pBreakLeaderTemplate,
    CXFA_Node*& pBreakTrailerTemplate) {
  XFA_ELEMENT eType = pCurNode->GetClassID();
  switch (eType) {
    case XFA_ELEMENT_BreakBefore:
    case XFA_ELEMENT_BreakAfter: {
      CFX_WideStringC wsBreakLeader, wsBreakTrailer;
      CXFA_Node* pFormNode = pCurNode->GetNodeItem(
          XFA_NODEITEM_Parent, XFA_OBJECTTYPE_ContainerNode);
      CXFA_Node* pContainer = pFormNode->GetTemplateNode();
      FX_BOOL bStartNew = pCurNode->GetInteger(XFA_ATTRIBUTE_StartNew) != 0;
      CXFA_Node* pScript = pCurNode->GetFirstChildByClass(XFA_ELEMENT_Script);
      if (pScript && !XFA_LayoutPageMgr_RunBreakTestScript(pScript)) {
        return FALSE;
      }
      CFX_WideStringC wsTarget = pCurNode->GetCData(XFA_ATTRIBUTE_Target);
      CXFA_Node* pTarget =
          XFA_ResolveBreakTarget(m_pTemplatePageSetRoot, TRUE, wsTarget);
      wsBreakTrailer = pCurNode->GetCData(XFA_ATTRIBUTE_Trailer);
      wsBreakLeader = pCurNode->GetCData(XFA_ATTRIBUTE_Leader);
      pBreakLeaderTemplate =
          XFA_ResolveBreakTarget(pContainer, TRUE, wsBreakLeader);
      pBreakTrailerTemplate =
          XFA_ResolveBreakTarget(pContainer, TRUE, wsBreakTrailer);
      if (RunBreak(eType, pCurNode->GetEnum(XFA_ATTRIBUTE_TargetType), pTarget,
                   bStartNew)) {
        return TRUE;
      } else {
        if (m_rgProposedContainerRecord.GetCount() > 0 &&
            m_pCurrentContainerRecord ==
                m_rgProposedContainerRecord.GetHeadPosition() &&
            eType == XFA_ELEMENT_BreakBefore) {
          CXFA_Node* pParentNode = pFormNode->GetNodeItem(
              XFA_NODEITEM_Parent, XFA_OBJECTTYPE_ContainerNode);
          if (!pParentNode ||
              pFormNode !=
                  pParentNode->GetNodeItem(XFA_NODEITEM_FirstChild,
                                           XFA_OBJECTTYPE_ContainerNode)) {
            break;
          }
          pParentNode = pParentNode->GetNodeItem(XFA_NODEITEM_Parent);
          if (!pParentNode || pParentNode->GetClassID() != XFA_ELEMENT_Form) {
            break;
          }
          return TRUE;
        }
      }
    } break;
    case XFA_ELEMENT_Break: {
      FX_BOOL bStartNew = pCurNode->GetInteger(XFA_ATTRIBUTE_StartNew) != 0;
      CFX_WideStringC wsTarget = pCurNode->GetCData(
          bBefore ? XFA_ATTRIBUTE_BeforeTarget : XFA_ATTRIBUTE_AfterTarget);
      CXFA_Node* pTarget =
          XFA_ResolveBreakTarget(m_pTemplatePageSetRoot, TRUE, wsTarget);
      if (RunBreak(bBefore ? XFA_ELEMENT_BreakBefore : XFA_ELEMENT_BreakAfter,
                   pCurNode->GetEnum(bBefore ? XFA_ATTRIBUTE_Before
                                             : XFA_ATTRIBUTE_After),
                   pTarget, bStartNew)) {
        return TRUE;
      }
    } break;
    default:
      break;
  }
  return FALSE;
}
static void XFA_SetLayoutGeneratedNodeFlag(CXFA_Node* pNode) {
  pNode->SetFlag(XFA_NODEFLAG_LayoutGeneratedNode, TRUE, FALSE);
  pNode->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE, FALSE);
}
FX_BOOL CXFA_LayoutPageMgr::ProcessBreakBeforeOrAfter(
    CXFA_Node* pBreakNode,
    FX_BOOL bBefore,
    CXFA_Node*& pBreakLeaderNode,
    CXFA_Node*& pBreakTrailerNode,
    FX_BOOL& bCreatePage) {
  CXFA_Node *pLeaderTemplate = NULL, *pTrailerTemplate = NULL;
  CXFA_Node* pFormNode = pBreakNode->GetNodeItem(XFA_NODEITEM_Parent,
                                                 XFA_OBJECTTYPE_ContainerNode);
  if (XFA_ItemLayoutProcessor_IsTakingSpace(pFormNode)) {
    bCreatePage = ExecuteBreakBeforeOrAfter(pBreakNode, bBefore,
                                            pLeaderTemplate, pTrailerTemplate);
    CXFA_Document* pDocument = pBreakNode->GetDocument();
    CXFA_Node* pDataScope = NULL;
    pFormNode = pFormNode->GetNodeItem(XFA_NODEITEM_Parent,
                                       XFA_OBJECTTYPE_ContainerNode);
    if (pLeaderTemplate) {
      if (!pDataScope) {
        pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
      }
      pBreakLeaderNode = pDocument->DataMerge_CopyContainer(
          pLeaderTemplate, pFormNode, pDataScope, TRUE);
      pDocument->DataMerge_UpdateBindingRelations(pBreakLeaderNode);
      XFA_SetLayoutGeneratedNodeFlag(pBreakLeaderNode);
    }
    if (pTrailerTemplate) {
      if (!pDataScope) {
        pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
      }
      pBreakTrailerNode = pDocument->DataMerge_CopyContainer(
          pTrailerTemplate, pFormNode, pDataScope, TRUE);
      pDocument->DataMerge_UpdateBindingRelations(pBreakTrailerNode);
      XFA_SetLayoutGeneratedNodeFlag(pBreakTrailerNode);
    }
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_LayoutPageMgr::ProcessBookendLeaderOrTrailer(
    CXFA_Node* pBookendNode,
    FX_BOOL bLeader,
    CXFA_Node*& pBookendAppendNode) {
  CXFA_Node* pLeaderTemplate = NULL;
  CXFA_Node* pFormNode = pBookendNode->GetNodeItem(
      XFA_NODEITEM_Parent, XFA_OBJECTTYPE_ContainerNode);
  if (ResolveBookendLeaderOrTrailer(pBookendNode, bLeader, pLeaderTemplate)) {
    CXFA_Document* pDocument = pBookendNode->GetDocument();
    CXFA_Node* pDataScope = NULL;
    if (pLeaderTemplate) {
      if (!pDataScope) {
        pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
      }
      pBookendAppendNode = pDocument->DataMerge_CopyContainer(
          pLeaderTemplate, pFormNode, pDataScope, TRUE);
      pDocument->DataMerge_UpdateBindingRelations(pBookendAppendNode);
      XFA_SetLayoutGeneratedNodeFlag(pBookendAppendNode);
      return TRUE;
    }
  }
  return FALSE;
}
CXFA_Node* CXFA_LayoutPageMgr::BreakOverflow(CXFA_Node* pOverflowNode,
                                             CXFA_Node*& pLeaderTemplate,
                                             CXFA_Node*& pTrailerTemplate,
                                             FX_BOOL bCreatePage) {
  CFX_WideStringC wsOverflowLeader, wsOverflowTrailer;
  CXFA_Node* pContainer =
      pOverflowNode->GetNodeItem(XFA_NODEITEM_Parent,
                                 XFA_OBJECTTYPE_ContainerNode)
          ->GetTemplateNode();
  if (pOverflowNode->GetClassID() == XFA_ELEMENT_Break) {
    CFX_WideStringC wsOverflowLeader;
    CFX_WideStringC wsOverflowTarget;
    CFX_WideStringC wsOverflowTrailer;
    pOverflowNode->TryCData(XFA_ATTRIBUTE_OverflowLeader, wsOverflowLeader);
    pOverflowNode->TryCData(XFA_ATTRIBUTE_OverflowTrailer, wsOverflowTrailer);
    pOverflowNode->TryCData(XFA_ATTRIBUTE_OverflowTarget, wsOverflowTarget);
    if (!wsOverflowLeader.IsEmpty() || !wsOverflowTrailer.IsEmpty() ||
        !wsOverflowTarget.IsEmpty()) {
      if (!wsOverflowTarget.IsEmpty() && bCreatePage &&
          !m_bCreateOverFlowPage) {
        CXFA_Node* pTarget = XFA_ResolveBreakTarget(
            this->m_pTemplatePageSetRoot, TRUE, wsOverflowTarget);
        if (pTarget) {
          m_bCreateOverFlowPage = TRUE;
          switch (pTarget->GetClassID()) {
            case XFA_ELEMENT_PageArea:
              RunBreak(XFA_ELEMENT_Overflow, XFA_ATTRIBUTEENUM_PageArea,
                       pTarget, TRUE);
              break;
            case XFA_ELEMENT_ContentArea:
              RunBreak(XFA_ELEMENT_Overflow, XFA_ATTRIBUTEENUM_ContentArea,
                       pTarget, TRUE);
              break;
            default:
              break;
          }
        }
      }
      if (!bCreatePage) {
        pLeaderTemplate =
            XFA_ResolveBreakTarget(pContainer, TRUE, wsOverflowLeader);
        pTrailerTemplate =
            XFA_ResolveBreakTarget(pContainer, TRUE, wsOverflowTrailer);
      }
      return pOverflowNode;
    }
    return NULL;
  } else if (pOverflowNode->GetClassID() == XFA_ELEMENT_Overflow) {
    CFX_WideStringC wsOverflowTarget;
    pOverflowNode->TryCData(XFA_ATTRIBUTE_Leader, wsOverflowLeader);
    pOverflowNode->TryCData(XFA_ATTRIBUTE_Trailer, wsOverflowTrailer);
    pOverflowNode->TryCData(XFA_ATTRIBUTE_Target, wsOverflowTarget);
    if (!wsOverflowTarget.IsEmpty() && bCreatePage && !m_bCreateOverFlowPage) {
      CXFA_Node* pTarget = XFA_ResolveBreakTarget(this->m_pTemplatePageSetRoot,
                                                  TRUE, wsOverflowTarget);
      if (pTarget) {
        m_bCreateOverFlowPage = TRUE;
        switch (pTarget->GetClassID()) {
          case XFA_ELEMENT_PageArea:
            RunBreak(XFA_ELEMENT_Overflow, XFA_ATTRIBUTEENUM_PageArea, pTarget,
                     TRUE);
            break;
          case XFA_ELEMENT_ContentArea:
            RunBreak(XFA_ELEMENT_Overflow, XFA_ATTRIBUTEENUM_ContentArea,
                     pTarget, TRUE);
            break;
          default:
            break;
        }
      }
    }
    if (!bCreatePage) {
      pLeaderTemplate =
          XFA_ResolveBreakTarget(pContainer, TRUE, wsOverflowLeader);
      pTrailerTemplate =
          XFA_ResolveBreakTarget(pContainer, TRUE, wsOverflowTrailer);
    }
    return pOverflowNode;
  }
  return NULL;
}
FX_BOOL CXFA_LayoutPageMgr::ProcessOverflow(CXFA_Node* pFormNode,
                                            CXFA_Node*& pLeaderNode,
                                            CXFA_Node*& pTrailerNode,
                                            FX_BOOL bDataMerge,
                                            FX_BOOL bCreatePage) {
  if (pFormNode == NULL) {
    return FALSE;
  }
  CXFA_Node *pLeaderTemplate = NULL, *pTrailerTemplate = NULL;
  FX_BOOL bIsOverflowNode = FALSE;
  if (pFormNode->GetClassID() == XFA_ELEMENT_Overflow ||
      pFormNode->GetClassID() == XFA_ELEMENT_Break) {
    bIsOverflowNode = TRUE;
  }
  for (CXFA_Node* pCurNode =
           bIsOverflowNode ? pFormNode
                           : pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pCurNode; pCurNode = pCurNode->GetNodeItem((XFA_NODEITEM_NextSibling))) {
    if (BreakOverflow(pCurNode, pLeaderTemplate, pTrailerTemplate,
                      bCreatePage)) {
      if (bIsOverflowNode) {
        pFormNode = pCurNode->GetNodeItem(XFA_NODEITEM_Parent);
      }
      CXFA_Document* pDocument = pCurNode->GetDocument();
      CXFA_Node* pDataScope = NULL;
      if (pLeaderTemplate) {
        if (!pDataScope) {
          pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
        }
        pLeaderNode = pDocument->DataMerge_CopyContainer(
            pLeaderTemplate, pFormNode, pDataScope, TRUE);
        pDocument->DataMerge_UpdateBindingRelations(pLeaderNode);
        XFA_SetLayoutGeneratedNodeFlag(pLeaderNode);
      }
      if (pTrailerTemplate) {
        if (!pDataScope) {
          pDataScope = XFA_DataMerge_FindDataScope(pFormNode);
        }
        pTrailerNode = pDocument->DataMerge_CopyContainer(
            pTrailerTemplate, pFormNode, pDataScope, TRUE);
        pDocument->DataMerge_UpdateBindingRelations(pTrailerNode);
        XFA_SetLayoutGeneratedNodeFlag(pTrailerNode);
      }
      return TRUE;
    }
    if (bIsOverflowNode) {
      break;
    }
  }
  return FALSE;
}
FX_BOOL CXFA_LayoutPageMgr::ResolveBookendLeaderOrTrailer(
    CXFA_Node* pBookendNode,
    FX_BOOL bLeader,
    CXFA_Node*& pBookendAppendTemplate) {
  CFX_WideStringC wsBookendLeader;
  CXFA_Node* pContainer =
      pBookendNode->GetNodeItem(XFA_NODEITEM_Parent,
                                XFA_OBJECTTYPE_ContainerNode)
          ->GetTemplateNode();
  if (pBookendNode->GetClassID() == XFA_ELEMENT_Break) {
    pBookendNode->TryCData(
        bLeader ? XFA_ATTRIBUTE_BookendLeader : XFA_ATTRIBUTE_BookendTrailer,
        wsBookendLeader);
    if (!wsBookendLeader.IsEmpty()) {
      pBookendAppendTemplate =
          XFA_ResolveBreakTarget(pContainer, FALSE, wsBookendLeader);
      return TRUE;
    }
    return FALSE;
  } else if (pBookendNode->GetClassID() == XFA_ELEMENT_Bookend) {
    pBookendNode->TryCData(
        bLeader ? XFA_ATTRIBUTE_Leader : XFA_ATTRIBUTE_Trailer,
        wsBookendLeader);
    pBookendAppendTemplate =
        XFA_ResolveBreakTarget(pContainer, TRUE, wsBookendLeader);
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_LayoutPageMgr::FindPageAreaFromPageSet(
    CXFA_Node* pPageSet,
    CXFA_Node* pStartChild,
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    FX_BOOL bNewPage,
    FX_BOOL bQuery) {
  if (pPageSet == NULL && pStartChild == NULL) {
    return FALSE;
  }
  if (IsPageSetRootOrderedOccurrence()) {
    return FindPageAreaFromPageSet_Ordered(pPageSet, pStartChild,
                                           pTargetPageArea, pTargetContentArea,
                                           bNewPage, bQuery);
  }
  XFA_ATTRIBUTEENUM ePreferredPosition = m_pCurrentContainerRecord
                                             ? XFA_ATTRIBUTEENUM_Rest
                                             : XFA_ATTRIBUTEENUM_First;
  return FindPageAreaFromPageSet_SimplexDuplex(
      pPageSet, pStartChild, pTargetPageArea, pTargetContentArea, bNewPage,
      bQuery, ePreferredPosition);
}
FX_BOOL CXFA_LayoutPageMgr::FindPageAreaFromPageSet_Ordered(
    CXFA_Node* pPageSet,
    CXFA_Node* pStartChild,
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    FX_BOOL bNewPage,
    FX_BOOL bQuery) {
  int32_t iPageSetCount = 0;
  if (!pStartChild && !bQuery) {
    m_pPageSetMap.Lookup(pPageSet, iPageSetCount);
    int32_t iMax = -1;
    CXFA_Node* pOccurNode = pPageSet->GetFirstChildByClass(XFA_ELEMENT_Occur);
    if (pOccurNode) {
      pOccurNode->TryInteger(XFA_ATTRIBUTE_Max, iMax, FALSE);
    }
    if (iMax >= 0 && iMax <= iPageSetCount) {
      return FALSE;
    }
  }
  FX_BOOL bRes = FALSE;
  CXFA_Node* pCurrentNode =
      pStartChild ? pStartChild->GetNodeItem(XFA_NODEITEM_NextSibling)
                  : pPageSet->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pCurrentNode;
       pCurrentNode = pCurrentNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pCurrentNode->GetClassID() == XFA_ELEMENT_PageArea) {
      if ((pTargetPageArea == pCurrentNode || pTargetPageArea == NULL)) {
        if (pCurrentNode->GetFirstChildByClass(XFA_ELEMENT_ContentArea) ==
            NULL) {
          if (pTargetPageArea == pCurrentNode) {
            CreateMinPageRecord(pCurrentNode, TRUE);
            pTargetPageArea = NULL;
          }
          continue;
        }
        if (!bQuery) {
          CXFA_ContainerRecord* pNewRecord =
              CreateContainerRecord(pCurrentNode, pStartChild == NULL);
          AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
          if (pTargetContentArea == NULL) {
            pTargetContentArea =
                pCurrentNode->GetFirstChildByClass(XFA_ELEMENT_ContentArea);
          }
          AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
        }
        m_pCurPageArea = pCurrentNode;
        m_nCurPageCount = 1;
        bRes = TRUE;
        break;
      }
      if (!bQuery) {
        CreateMinPageRecord(pCurrentNode, FALSE);
      }
    } else if (pCurrentNode->GetClassID() == XFA_ELEMENT_PageSet) {
      if (FindPageAreaFromPageSet_Ordered(pCurrentNode, NULL, pTargetPageArea,
                                          pTargetContentArea, bNewPage,
                                          bQuery)) {
        bRes = TRUE;
        break;
      }
      if (!bQuery) {
        CreateMinPageSetRecord(pCurrentNode, TRUE);
      }
    }
  }
  if (!pStartChild && bRes && !bQuery) {
    m_pPageSetMap.SetAt(pPageSet, ++iPageSetCount);
  }
  return bRes;
}
FX_BOOL CXFA_LayoutPageMgr::FindPageAreaFromPageSet_SimplexDuplex(
    CXFA_Node* pPageSet,
    CXFA_Node* pStartChild,
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    FX_BOOL bNewPage,
    FX_BOOL bQuery,
    XFA_ATTRIBUTEENUM ePreferredPosition) {
  const XFA_ATTRIBUTEENUM eFallbackPosition = XFA_ATTRIBUTEENUM_Any;
  CXFA_Node *pPreferredPageArea = NULL, *pFallbackPageArea = NULL;
  CXFA_Node* pCurrentNode = NULL;
  if (!pStartChild || pStartChild->GetClassID() == XFA_ELEMENT_PageArea) {
    pCurrentNode = pPageSet->GetNodeItem(XFA_NODEITEM_FirstChild);
  } else {
    pCurrentNode = pStartChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  for (; pCurrentNode;
       pCurrentNode = pCurrentNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pCurrentNode->GetClassID() == XFA_ELEMENT_PageArea) {
      if (!MatchPageAreaOddOrEven(pCurrentNode, FALSE)) {
        continue;
      }
      XFA_ATTRIBUTEENUM eCurPagePosition =
          pCurrentNode->GetEnum(XFA_ATTRIBUTE_PagePosition);
      if (ePreferredPosition == XFA_ATTRIBUTEENUM_Last) {
        if (eCurPagePosition != ePreferredPosition) {
          continue;
        }
        if (m_ePageSetMode == XFA_ATTRIBUTEENUM_SimplexPaginated ||
            pCurrentNode->GetEnum(XFA_ATTRIBUTE_OddOrEven) ==
                XFA_ATTRIBUTEENUM_Any) {
          pPreferredPageArea = pCurrentNode;
          break;
        }
        CXFA_ContainerRecord* pNewRecord = CreateContainerRecord();
        AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
        AddContentAreaLayoutItem(pNewRecord, pCurrentNode->GetFirstChildByClass(
                                                 XFA_ELEMENT_ContentArea));
        pPreferredPageArea = pCurrentNode;
        return FALSE;
      } else if (ePreferredPosition == XFA_ATTRIBUTEENUM_Only) {
        if (eCurPagePosition != ePreferredPosition) {
          continue;
        }
        if (m_ePageSetMode != XFA_ATTRIBUTEENUM_DuplexPaginated ||
            pCurrentNode->GetEnum(XFA_ATTRIBUTE_OddOrEven) ==
                XFA_ATTRIBUTEENUM_Any) {
          pPreferredPageArea = pCurrentNode;
          break;
        }
        return FALSE;
      }
      if ((pTargetPageArea == pCurrentNode || pTargetPageArea == NULL)) {
        if (pCurrentNode->GetFirstChildByClass(XFA_ELEMENT_ContentArea) ==
            NULL) {
          if (pTargetPageArea == pCurrentNode) {
            CXFA_ContainerRecord* pNewRecord = CreateContainerRecord();
            AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
            pTargetPageArea = NULL;
          }
          continue;
        }
        if ((ePreferredPosition == XFA_ATTRIBUTEENUM_Rest &&
             eCurPagePosition == XFA_ATTRIBUTEENUM_Any) ||
            eCurPagePosition == ePreferredPosition) {
          pPreferredPageArea = pCurrentNode;
          break;
        } else if (eCurPagePosition == eFallbackPosition &&
                   !pFallbackPageArea) {
          pFallbackPageArea = pCurrentNode;
        }
      } else if (pTargetPageArea &&
                 !MatchPageAreaOddOrEven(pTargetPageArea, FALSE)) {
        CXFA_ContainerRecord* pNewRecord = CreateContainerRecord();
        AddPageAreaLayoutItem(pNewRecord, pCurrentNode);
        AddContentAreaLayoutItem(pNewRecord, pCurrentNode->GetFirstChildByClass(
                                                 XFA_ELEMENT_ContentArea));
      }
    } else if (pCurrentNode->GetClassID() == XFA_ELEMENT_PageSet) {
      if (FindPageAreaFromPageSet_SimplexDuplex(
              pCurrentNode, NULL, pTargetPageArea, pTargetContentArea, bNewPage,
              bQuery, ePreferredPosition)) {
        break;
      }
    }
  }
  CXFA_Node* pCurPageArea = NULL;
  if (pPreferredPageArea) {
    pCurPageArea = pPreferredPageArea;
  } else if (pFallbackPageArea) {
    pCurPageArea = pFallbackPageArea;
  }
  if (!pCurPageArea) {
    return FALSE;
  }
  if (!bQuery) {
    CXFA_ContainerRecord* pNewRecord = CreateContainerRecord();
    AddPageAreaLayoutItem(pNewRecord, pCurPageArea);
    if (pTargetContentArea == NULL) {
      pTargetContentArea =
          pCurPageArea->GetFirstChildByClass(XFA_ELEMENT_ContentArea);
    }
    AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
  }
  m_pCurPageArea = pCurPageArea;
  return TRUE;
}
FX_BOOL CXFA_LayoutPageMgr::MatchPageAreaOddOrEven(CXFA_Node* pPageArea,
                                                   FX_BOOL bLastMatch) {
  if (m_ePageSetMode != XFA_ATTRIBUTEENUM_DuplexPaginated) {
    return TRUE;
  }
  XFA_ATTRIBUTEENUM eOddOrEven = XFA_ATTRIBUTEENUM_Any;
  pPageArea->TryEnum(XFA_ATTRIBUTE_OddOrEven, eOddOrEven);
  if (eOddOrEven != XFA_ATTRIBUTEENUM_Any) {
    int32_t iPageCount = GetPageCount();
    if (bLastMatch) {
      return eOddOrEven == XFA_ATTRIBUTEENUM_Odd ? iPageCount % 2 == 1
                                                 : iPageCount % 2 == 0;
    }
    return eOddOrEven == XFA_ATTRIBUTEENUM_Odd ? iPageCount % 2 == 0
                                               : iPageCount % 2 == 1;
  }
  return TRUE;
}
CXFA_Node* CXFA_LayoutPageMgr::GetNextAvailPageArea(
    CXFA_Node* pTargetPageArea,
    CXFA_Node* pTargetContentArea,
    FX_BOOL bNewPage,
    FX_BOOL bQuery) {
  if (m_pCurPageArea == NULL) {
    FindPageAreaFromPageSet(m_pTemplatePageSetRoot, NULL, pTargetPageArea,
                            pTargetContentArea, bNewPage, bQuery);
    ASSERT(m_pCurPageArea);
    return m_pCurPageArea;
  }
  if (pTargetPageArea == NULL || pTargetPageArea == m_pCurPageArea) {
    if (!bNewPage && GetNextContentArea(pTargetContentArea)) {
      return m_pCurPageArea;
    }
    if (IsPageSetRootOrderedOccurrence()) {
      int32_t iMax = -1;
      CXFA_Node* pOccurNode =
          m_pCurPageArea->GetFirstChildByClass(XFA_ELEMENT_Occur);
      if (pOccurNode) {
        pOccurNode->TryInteger(XFA_ATTRIBUTE_Max, iMax, FALSE);
      }
      if ((iMax < 0 || m_nCurPageCount < iMax)) {
        if (!bQuery) {
          CXFA_ContainerRecord* pNewRecord =
              CreateContainerRecord(m_pCurPageArea);
          AddPageAreaLayoutItem(pNewRecord, m_pCurPageArea);
          if (pTargetContentArea == NULL) {
            pTargetContentArea =
                m_pCurPageArea->GetFirstChildByClass(XFA_ELEMENT_ContentArea);
          }
          AddContentAreaLayoutItem(pNewRecord, pTargetContentArea);
        }
        m_nCurPageCount++;
        return m_pCurPageArea;
      }
    }
  }
  if (!bQuery && IsPageSetRootOrderedOccurrence()) {
    CreateMinPageRecord(m_pCurPageArea, FALSE, TRUE);
  }
  if (FindPageAreaFromPageSet(m_pCurPageArea->GetNodeItem(XFA_NODEITEM_Parent),
                              m_pCurPageArea, pTargetPageArea,
                              pTargetContentArea, bNewPage, bQuery)) {
    return m_pCurPageArea;
  }
  CXFA_Node* pPageSet = m_pCurPageArea->GetNodeItem(XFA_NODEITEM_Parent);
  while (TRUE) {
    if (FindPageAreaFromPageSet(pPageSet, NULL, pTargetPageArea,
                                pTargetContentArea, bNewPage, bQuery)) {
      return m_pCurPageArea;
    }
    if (!bQuery && IsPageSetRootOrderedOccurrence()) {
      CreateMinPageSetRecord(pPageSet);
    }
    if (FindPageAreaFromPageSet(NULL, pPageSet, pTargetPageArea,
                                pTargetContentArea, bNewPage, bQuery)) {
      return m_pCurPageArea;
    }
    if (pPageSet == m_pTemplatePageSetRoot) {
      break;
    }
    pPageSet = pPageSet->GetNodeItem(XFA_NODEITEM_Parent);
  }
  return NULL;
}
static FX_BOOL XFA_LayoutPageMgr_CheckContentAreaNotUsed(
    CXFA_ContainerLayoutItem* pPageAreaLayoutItem,
    CXFA_Node* pContentArea,
    CXFA_ContainerLayoutItem*& pContentAreaLayoutItem) {
  for (CXFA_ContainerLayoutItem* pLayoutItem =
           (CXFA_ContainerLayoutItem*)pPageAreaLayoutItem->m_pFirstChild;
       pLayoutItem;
       pLayoutItem = (CXFA_ContainerLayoutItem*)pLayoutItem->m_pNextSibling) {
    if (pLayoutItem->m_pFormNode == pContentArea) {
      if (pLayoutItem->m_pFirstChild == NULL) {
        pContentAreaLayoutItem = pLayoutItem;
        return TRUE;
      }
      return FALSE;
    }
  }
  return TRUE;
}
FX_BOOL CXFA_LayoutPageMgr::GetNextContentArea(CXFA_Node* pContentArea) {
  CXFA_Node* pCurContentNode =
      GetCurrentContainerRecord()->pCurContentArea->m_pFormNode;
  if (pContentArea == NULL) {
    pContentArea =
        pCurContentNode->GetNextSameClassSibling(XFA_ELEMENT_ContentArea);
    if (pContentArea == NULL) {
      return FALSE;
    }
  } else {
    if (pContentArea->GetNodeItem(XFA_NODEITEM_Parent) != m_pCurPageArea) {
      return FALSE;
    }
    CXFA_ContainerLayoutItem* pContentAreaLayout = NULL;
    if (!XFA_LayoutPageMgr_CheckContentAreaNotUsed(
            GetCurrentContainerRecord()->pCurPageArea, pContentArea,
            pContentAreaLayout)) {
      return FALSE;
    }
    if (pContentAreaLayout) {
      if (pContentAreaLayout->m_pFormNode != pCurContentNode) {
        CXFA_ContainerRecord* pNewRecord = CreateContainerRecord();
        pNewRecord->pCurContentArea = pContentAreaLayout;
        return TRUE;
      } else {
        return FALSE;
      }
    }
  }
  CXFA_ContainerRecord* pNewRecord = CreateContainerRecord();
  AddContentAreaLayoutItem(pNewRecord, pContentArea);
  return TRUE;
}
void CXFA_LayoutPageMgr::InitPageSetMap() {
  if (!IsPageSetRootOrderedOccurrence()) {
    return;
  }
  CXFA_NodeIterator sIterator(m_pTemplatePageSetRoot);
  for (CXFA_Node* pPageSetNode = sIterator.GetCurrent(); pPageSetNode;
       pPageSetNode = sIterator.MoveToNext()) {
    if (pPageSetNode->GetClassID() == XFA_ELEMENT_PageSet) {
      XFA_ATTRIBUTEENUM eRelation =
          pPageSetNode->GetEnum(XFA_ATTRIBUTE_Relation);
      if (eRelation == XFA_ATTRIBUTEENUM_OrderedOccurrence) {
        m_pPageSetMap.SetAt(pPageSetNode, 0);
      }
    }
  }
}
int32_t CXFA_LayoutPageMgr::CreateMinPageRecord(CXFA_Node* pPageArea,
                                                FX_BOOL bTargetPageArea,
                                                FX_BOOL bCreateLast) {
  if (pPageArea == NULL) {
    return 0;
  }
  CXFA_Node* pOccurNode = pPageArea->GetFirstChildByClass(XFA_ELEMENT_Occur);
  int32_t iMin = 0;
  if ((pOccurNode && pOccurNode->TryInteger(XFA_ATTRIBUTE_Min, iMin, FALSE)) ||
      bTargetPageArea) {
    CXFA_Node* pContentArea =
        pPageArea->GetFirstChildByClass(XFA_ELEMENT_ContentArea);
    if (iMin < 1 && bTargetPageArea && !pContentArea) {
      iMin = 1;
    }
    int32_t i = 0;
    if (bCreateLast) {
      i = m_nCurPageCount;
    }
    for (; i < iMin; i++) {
      CXFA_ContainerRecord* pNewRecord = CreateContainerRecord();
      AddPageAreaLayoutItem(pNewRecord, pPageArea);
      AddContentAreaLayoutItem(pNewRecord, pContentArea);
    }
  }
  return iMin;
}
void CXFA_LayoutPageMgr::CreateMinPageSetRecord(CXFA_Node* pPageSet,
                                                FX_BOOL bCreateAll) {
  if (pPageSet == NULL) {
    return;
  }
  int32_t iCurSetCount = 0;
  if (!m_pPageSetMap.Lookup(pPageSet, iCurSetCount)) {
    return;
  }
  if (bCreateAll) {
    iCurSetCount = 0;
  }
  CXFA_Node* pOccurNode = pPageSet->GetFirstChildByClass(XFA_ELEMENT_Occur);
  int32_t iMin = 0;
  if (pOccurNode && pOccurNode->TryInteger(XFA_ATTRIBUTE_Min, iMin, FALSE)) {
    if (iCurSetCount < iMin) {
      for (int32_t i = 0; i < iMin - iCurSetCount; i++) {
        for (CXFA_Node* pCurrentPageNode =
                 pPageSet->GetNodeItem(XFA_NODEITEM_FirstChild);
             pCurrentPageNode; pCurrentPageNode = pCurrentPageNode->GetNodeItem(
                                   XFA_NODEITEM_NextSibling)) {
          if (pCurrentPageNode->GetClassID() == XFA_ELEMENT_PageArea) {
            CreateMinPageRecord(pCurrentPageNode, FALSE);
          } else if (pCurrentPageNode->GetClassID() == XFA_ELEMENT_PageSet) {
            CreateMinPageSetRecord(pCurrentPageNode, TRUE);
          }
        }
      }
      m_pPageSetMap.SetAt(pPageSet, iMin);
    }
  }
}
void CXFA_LayoutPageMgr::CreateNextMinRecord(CXFA_Node* pRecordNode) {
  if (pRecordNode == NULL) {
    return;
  }
  for (CXFA_Node* pCurrentNode =
           pRecordNode->GetNodeItem(XFA_NODEITEM_NextSibling);
       pCurrentNode;
       pCurrentNode = pCurrentNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pCurrentNode->GetClassID() == XFA_ELEMENT_PageArea) {
      CreateMinPageRecord(pCurrentNode, FALSE);
    } else if (pCurrentNode->GetClassID() == XFA_ELEMENT_PageSet) {
      CreateMinPageSetRecord(pCurrentNode, TRUE);
    }
  }
}
void CXFA_LayoutPageMgr::ProcessLastPageSet() {
  CreateMinPageRecord(m_pCurPageArea, FALSE, TRUE);
  CreateNextMinRecord(m_pCurPageArea);
  CXFA_Node* pPageSet = m_pCurPageArea->GetNodeItem(XFA_NODEITEM_Parent);
  while (TRUE) {
    CreateMinPageSetRecord(pPageSet);
    if (pPageSet == m_pTemplatePageSetRoot) {
      break;
    }
    CreateNextMinRecord(pPageSet);
    pPageSet = pPageSet->GetNodeItem(XFA_NODEITEM_Parent);
  }
}
FX_BOOL CXFA_LayoutPageMgr::GetNextAvailContentHeight(FX_FLOAT fChildHeight) {
  CXFA_Node* pCurContentNode =
      GetCurrentContainerRecord()->pCurContentArea->m_pFormNode;
  if (pCurContentNode == NULL) {
    return FALSE;
  }
  pCurContentNode =
      pCurContentNode->GetNextSameClassSibling(XFA_ELEMENT_ContentArea);
  if (pCurContentNode) {
    FX_FLOAT fNextContentHeight =
        pCurContentNode->GetMeasure(XFA_ATTRIBUTE_H).ToUnit(XFA_UNIT_Pt);
    return fNextContentHeight > fChildHeight;
  }
  CXFA_Node* pPageNode = GetCurrentContainerRecord()->pCurPageArea->m_pFormNode;
  CXFA_Node* pOccurNode = pPageNode->GetFirstChildByClass(XFA_ELEMENT_Occur);
  int32_t iMax = 0;
  if (pOccurNode && pOccurNode->TryInteger(XFA_ATTRIBUTE_Max, iMax, FALSE)) {
    if (m_nCurPageCount == iMax) {
      CXFA_Node* pSrcPage = m_pCurPageArea;
      int32_t nSrcPageCount = m_nCurPageCount;
      FX_POSITION psSrcRecord = m_rgProposedContainerRecord.GetTailPosition();
      CXFA_Node* pNextPage = GetNextAvailPageArea(NULL, NULL, FALSE, TRUE);
      m_pCurPageArea = pSrcPage;
      m_nCurPageCount = nSrcPageCount;
      CXFA_ContainerRecord* pPrevRecord =
          (CXFA_ContainerRecord*)m_rgProposedContainerRecord.GetNext(
              psSrcRecord);
      while (psSrcRecord) {
        FX_POSITION psSaveRecord = psSrcRecord;
        CXFA_ContainerRecord* pInsertRecord =
            (CXFA_ContainerRecord*)m_rgProposedContainerRecord.GetNext(
                psSrcRecord);
        RemoveLayoutRecord(pInsertRecord, pPrevRecord);
        delete pInsertRecord;
        m_rgProposedContainerRecord.RemoveAt(psSaveRecord);
      }
      if (pNextPage) {
        CXFA_Node* pContentArea =
            pNextPage->GetFirstChildByClass(XFA_ELEMENT_ContentArea);
        if (pContentArea) {
          FX_FLOAT fNextContentHeight =
              pContentArea->GetMeasure(XFA_ATTRIBUTE_H).ToUnit(XFA_UNIT_Pt);
          if (fNextContentHeight > fChildHeight) {
            return TRUE;
          }
        }
      }
      return FALSE;
    }
  }
  CXFA_Node* pContentArea =
      pPageNode->GetFirstChildByClass(XFA_ELEMENT_ContentArea);
  FX_FLOAT fNextContentHeight =
      pContentArea->GetMeasure(XFA_ATTRIBUTE_H).ToUnit(XFA_UNIT_Pt);
  if (fNextContentHeight < XFA_LAYOUT_FLOAT_PERCISION) {
    return TRUE;
  }
  if (fNextContentHeight > fChildHeight) {
    return TRUE;
  }
  return FALSE;
}
void CXFA_LayoutPageMgr::ClearData() {
  ClearRecordList();
}
void CXFA_LayoutPageMgr::ClearRecordList() {
  if (!m_pTemplatePageSetRoot) {
    return;
  }
  if (m_rgProposedContainerRecord.GetCount() > 0) {
    FX_POSITION sPos;
    sPos = m_rgProposedContainerRecord.GetHeadPosition();
    while (sPos) {
      CXFA_ContainerRecord* pRecord =
          (CXFA_ContainerRecord*)m_rgProposedContainerRecord.GetNext(sPos);
      delete pRecord;
    }
    m_rgProposedContainerRecord.RemoveAll();
  }
  m_pCurrentContainerRecord = NULL;
  m_pCurPageArea = NULL;
  m_nCurPageCount = 0;
  m_bCreateOverFlowPage = FALSE;
  m_pPageSetMap.RemoveAll();
}
CXFA_LayoutItem* CXFA_LayoutPageMgr::FindOrCreateLayoutItem(
    CXFA_Node* pFormNode) {
#if defined(_XFA_LAYOUTITEM_MAPCACHE_)
  if (m_NodeToContent.GetCount() > 0) {
    CXFA_ContentLayoutItem* pLayoutItem = NULL;
    if (m_NodeToContent.Lookup(pFormNode, (void*&)pLayoutItem)) {
      if (pLayoutItem->m_pNext) {
        m_NodeToContent.SetAt(pFormNode, pLayoutItem->m_pNext);
        pLayoutItem->m_pNext->m_pPrev = NULL;
        pLayoutItem->m_pNext = NULL;
      } else {
        m_NodeToContent.RemoveKey(pFormNode);
      }
      pLayoutItem->m_pFormNode = pFormNode;
      return pLayoutItem;
    }
  }
#endif
  return (CXFA_LayoutItem*)pFormNode->GetDocument()
      ->GetParser()
      ->GetNotify()
      ->OnCreateLayoutItem(pFormNode);
}
#if defined(_XFA_LAYOUTITEM_MAPCACHE_)
void CXFA_LayoutPageMgr::SaveLayoutItem(CXFA_LayoutItem* pParentLayoutItem) {
  CXFA_LayoutItem* pNextLayoutItem,
      * pCurLayoutItem = pParentLayoutItem->m_pFirstChild;
  while (pCurLayoutItem) {
    pNextLayoutItem = pCurLayoutItem->m_pNextSibling;
    if (pCurLayoutItem->m_pFirstChild) {
      SaveLayoutItem(pCurLayoutItem);
    }
    if (pCurLayoutItem->IsContentLayoutItem()) {
      if (m_NodeToContent.GetValueAt(pCurLayoutItem->m_pFormNode) == NULL) {
        pCurLayoutItem->m_pFormNode->SetUserData(XFA_LAYOUTITEMKEY, NULL);
        m_NodeToContent.SetAt(pCurLayoutItem->m_pFormNode, pCurLayoutItem);
      }
    } else if (pCurLayoutItem->m_pFormNode->GetClassID() !=
               XFA_ELEMENT_PageArea) {
      delete pCurLayoutItem;
      pCurLayoutItem = NULL;
    }
    if (pCurLayoutItem) {
      pCurLayoutItem->m_pParent = NULL;
      pCurLayoutItem->m_pNextSibling = NULL;
      pCurLayoutItem->m_pFirstChild = NULL;
    }
    pCurLayoutItem = pNextLayoutItem;
  }
}
#elif defined(_XFA_LAYOUTITEM_ProcessCACHE_)
static void XFA_SyncRemoveLayoutItem(CXFA_LayoutItem* pParentLayoutItem,
                                     IXFA_Notify* pNotify,
                                     IXFA_DocLayout* pDocLayout) {
  CXFA_LayoutItem* pNextLayoutItem;
  CXFA_LayoutItem* pCurLayoutItem = pParentLayoutItem->m_pFirstChild;
  while (pCurLayoutItem) {
    pNextLayoutItem = pCurLayoutItem->m_pNextSibling;
    if (pCurLayoutItem->m_pFirstChild) {
      XFA_SyncRemoveLayoutItem(pCurLayoutItem, pNotify, pDocLayout);
    }
    pNotify->OnLayoutEvent(pDocLayout, pCurLayoutItem,
                           XFA_LAYOUTEVENT_ItemRemoving);
    delete pCurLayoutItem;
    pCurLayoutItem = pNextLayoutItem;
  }
}
void CXFA_LayoutPageMgr::SaveLayoutItem(CXFA_LayoutItem* pParentLayoutItem) {
  CXFA_LayoutItem* pNextLayoutItem;
  CXFA_LayoutItem* pCurLayoutItem = pParentLayoutItem->m_pFirstChild;
  while (pCurLayoutItem) {
    pNextLayoutItem = pCurLayoutItem->m_pNextSibling;
    if (pCurLayoutItem->IsContentLayoutItem()) {
      FX_DWORD dwFlag = pCurLayoutItem->m_pFormNode->GetFlag();
      if (dwFlag & (XFA_NODEFLAG_HasRemoved)) {
        IXFA_Notify* pNotify =
            m_pTemplatePageSetRoot->GetDocument()->GetParser()->GetNotify();
        IXFA_DocLayout* pDocLayout =
            m_pTemplatePageSetRoot->GetDocument()->GetDocLayout();
        if (pCurLayoutItem->m_pFirstChild) {
          XFA_SyncRemoveLayoutItem(pCurLayoutItem, pNotify, pDocLayout);
        }
        pNotify->OnLayoutEvent(pDocLayout, pCurLayoutItem,
                               XFA_LAYOUTEVENT_ItemRemoving);
        delete pCurLayoutItem;
        pCurLayoutItem = pNextLayoutItem;
        continue;
      }
      if (dwFlag & XFA_NODEFLAG_LayoutGeneratedNode) {
        CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
            sIterator(pCurLayoutItem->m_pFormNode);
        for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
             pNode = sIterator.MoveToNext()) {
          pNode->SetFlag(XFA_NODEFLAG_UnusedNode, TRUE, FALSE);
        }
      }
    }
    if (pCurLayoutItem->m_pFirstChild) {
      SaveLayoutItem(pCurLayoutItem);
    }
    pCurLayoutItem->m_pParent = NULL;
    pCurLayoutItem->m_pNextSibling = NULL;
    pCurLayoutItem->m_pFirstChild = NULL;
    if (!pCurLayoutItem->IsContentLayoutItem() &&
        pCurLayoutItem->m_pFormNode->GetClassID() != XFA_ELEMENT_PageArea) {
      delete pCurLayoutItem;
    }
    pCurLayoutItem = pNextLayoutItem;
  }
}
#endif
CXFA_Node* CXFA_LayoutPageMgr::QueryOverflow(
    CXFA_Node* pFormNode,
    CXFA_LayoutContext* pLayoutContext) {
  for (CXFA_Node* pCurNode = pFormNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pCurNode; pCurNode = pCurNode->GetNodeItem((XFA_NODEITEM_NextSibling))) {
    if (pCurNode->GetClassID() == XFA_ELEMENT_Break) {
      CFX_WideStringC wsOverflowLeader;
      CFX_WideStringC wsOverflowTarget;
      CFX_WideStringC wsOverflowTrailer;
      pCurNode->TryCData(XFA_ATTRIBUTE_OverflowLeader, wsOverflowLeader);
      pCurNode->TryCData(XFA_ATTRIBUTE_OverflowTrailer, wsOverflowTrailer);
      pCurNode->TryCData(XFA_ATTRIBUTE_OverflowTarget, wsOverflowTarget);
      if (!wsOverflowLeader.IsEmpty() || !wsOverflowTrailer.IsEmpty() ||
          !wsOverflowTarget.IsEmpty()) {
        return pCurNode;
      }
      return NULL;
    } else if (pCurNode->GetClassID() == XFA_ELEMENT_Overflow) {
      return pCurNode;
    }
  }
  return NULL;
}
void CXFA_LayoutPageMgr::MergePageSetContents() {
  CXFA_Document* pDocument = m_pTemplatePageSetRoot->GetDocument();
  IXFA_Notify* pNotify = pDocument->GetParser()->GetNotify();
  IXFA_DocLayout* pDocLayout = pDocument->GetDocLayout();
  CXFA_ContainerLayoutItem* pRootLayout = this->GetRootLayoutItem();
  {
    for (int32_t iIndex = 0; iIndex < pDocument->m_pPendingPageSet.GetSize();
         iIndex++) {
      CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
          sIterator(pDocument->m_pPendingPageSet.GetAt(iIndex));
      for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
           pNode = sIterator.MoveToNext()) {
        if (pNode->IsContainerNode()) {
          CXFA_Node* pBindNode = pNode->GetBindData();
          if (pBindNode) {
            pBindNode->RemoveBindItem(pNode);
            pNode->SetObject(XFA_ATTRIBUTE_BindingNode, NULL);
          }
        }
        pNode->SetFlag(XFA_NODEFLAG_UnusedNode);
      }
    }
  }
  int32_t iIndex = 0;
  CXFA_Node* pPendingPageSet = NULL;
  for (; pRootLayout;
       pRootLayout = (CXFA_ContainerLayoutItem*)pRootLayout->m_pNextSibling) {
    pPendingPageSet = NULL;
    CXFA_NodeIteratorTemplate<
        CXFA_ContainerLayoutItem,
        CXFA_TraverseStrategy_ContentAreaContainerLayoutItem>
        iterator(pRootLayout);
    CXFA_ContainerLayoutItem* pRootPageSetContainerItem = iterator.GetCurrent();
    ASSERT(pRootPageSetContainerItem->m_pFormNode->GetClassID() ==
           XFA_ELEMENT_PageSet);
    if (iIndex < pDocument->m_pPendingPageSet.GetSize()) {
      pPendingPageSet = pDocument->m_pPendingPageSet.GetAt(iIndex);
      iIndex++;
    }
    if (!pPendingPageSet) {
      if (pRootPageSetContainerItem->m_pFormNode->GetPacketID() ==
          XFA_XDPPACKET_Template) {
        pPendingPageSet =
            pRootPageSetContainerItem->m_pFormNode->CloneTemplateToForm(FALSE);
      } else {
        pPendingPageSet = pRootPageSetContainerItem->m_pFormNode;
      }
    }
    if (pRootPageSetContainerItem->m_pFormNode->GetUserData(
            XFA_LAYOUTITEMKEY) == pRootPageSetContainerItem) {
      pRootPageSetContainerItem->m_pFormNode->SetUserData(XFA_LAYOUTITEMKEY,
                                                          NULL);
    }
    pRootPageSetContainerItem->m_pFormNode = pPendingPageSet;
    pPendingPageSet->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
    for (CXFA_ContainerLayoutItem* pContainerItem = iterator.MoveToNext();
         pContainerItem; pContainerItem = iterator.MoveToNext()) {
      CXFA_Node* pNode = pContainerItem->m_pFormNode;
      if (pNode->GetPacketID() != XFA_XDPPACKET_Template) {
        continue;
      }
      switch (pNode->GetClassID()) {
        case XFA_ELEMENT_PageSet: {
          CXFA_Node* pParentNode = pContainerItem->m_pParent->m_pFormNode;
          pContainerItem->m_pFormNode = XFA_NodeMerge_CloneOrMergeContainer(
              pDocument, pParentNode, pContainerItem->m_pFormNode, TRUE);
        } break;
        case XFA_ELEMENT_PageArea: {
          CXFA_ContainerLayoutItem* pFormLayout = pContainerItem;
          CXFA_Node* pParentNode = pContainerItem->m_pParent->m_pFormNode;
          FX_BOOL bIsExistForm = TRUE;
          for (int32_t iLevel = 0; iLevel < 3; iLevel++) {
            pFormLayout = (CXFA_ContainerLayoutItem*)pFormLayout->m_pFirstChild;
            if (iLevel == 2) {
              while (pFormLayout &&
                     !XFA_ItemLayoutProcessor_IsTakingSpace(
                         pFormLayout->m_pFormNode)) {
                pFormLayout =
                    (CXFA_ContainerLayoutItem*)pFormLayout->m_pNextSibling;
              }
            }
            if (pFormLayout == NULL) {
              bIsExistForm = FALSE;
              break;
            }
          }
          if (bIsExistForm) {
            CXFA_Node* pNewSubform = pFormLayout->m_pFormNode;
            if (pContainerItem->m_pOldSubform != NULL &&
                pContainerItem->m_pOldSubform != pNewSubform) {
              CXFA_Node* pExistingNode = XFA_DataMerge_FindFormDOMInstance(
                  pDocument, pContainerItem->m_pFormNode->GetClassID(),
                  pContainerItem->m_pFormNode->GetNameHash(), pParentNode);
              CXFA_ContainerIterator sIterator(pExistingNode);
              for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
                   pNode = sIterator.MoveToNext()) {
                if (pNode->GetClassID() != XFA_ELEMENT_ContentArea) {
                  CXFA_LayoutItem* pLayoutItem = static_cast<CXFA_LayoutItem*>(
                      pNode->GetUserData(XFA_LAYOUTITEMKEY));
                  if (pLayoutItem) {
                    pNotify->OnLayoutEvent(pDocLayout, pLayoutItem,
                                           XFA_LAYOUTEVENT_ItemRemoving);
                    delete pLayoutItem;
                  }
                }
              }
              if (pExistingNode) {
                pParentNode->RemoveChild(pExistingNode);
              }
            }
            pContainerItem->m_pOldSubform = pNewSubform;
          }
          pContainerItem->m_pFormNode = pDocument->DataMerge_CopyContainer(
              pContainerItem->m_pFormNode, pParentNode,
              (CXFA_Node*)pDocument->GetXFANode(XFA_HASHCODE_Record), TRUE);
        } break;
        case XFA_ELEMENT_ContentArea: {
          CXFA_Node* pParentNode = pContainerItem->m_pParent->m_pFormNode;
          for (CXFA_Node* pChildNode =
                   pParentNode->GetNodeItem(XFA_NODEITEM_FirstChild);
               pChildNode;
               pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
            if (pChildNode->GetTemplateNode() != pContainerItem->m_pFormNode) {
              continue;
            }
            pContainerItem->m_pFormNode = pChildNode;
            break;
          }
        } break;
        default:
          break;
      }
    }
    if (!pPendingPageSet->GetNodeItem(XFA_NODEITEM_Parent)) {
      CXFA_Node* pFormToplevelSubform =
          ((CXFA_Node*)pDocument->GetXFANode(XFA_HASHCODE_Form))
              ->GetFirstChildByClass(XFA_ELEMENT_Subform);
      pFormToplevelSubform->InsertChild(pPendingPageSet);
    }
    pDocument->DataMerge_UpdateBindingRelations(pPendingPageSet);
    pPendingPageSet->SetFlag(XFA_NODEFLAG_Initialized);
  }
  pPendingPageSet = GetRootLayoutItem()->m_pFormNode;
  while (pPendingPageSet) {
    CXFA_Node* pNextPendingPageSet =
        pPendingPageSet->GetNextSameClassSibling(XFA_ELEMENT_PageSet);
    CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFANode>
        sIterator(pPendingPageSet);
    CXFA_Node* pNode = sIterator.GetCurrent();
    while (pNode) {
      if (pNode->HasFlag(XFA_NODEFLAG_UnusedNode)) {
        if (pNode->GetObjectType() == XFA_OBJECTTYPE_ContainerNode) {
          XFA_ELEMENT eCurId = pNode->GetClassID();
          if (eCurId == XFA_ELEMENT_PageArea || eCurId == XFA_ELEMENT_PageSet) {
            CXFA_ContainerIterator iteChild(pNode);
            CXFA_Node* pChildNode = iteChild.MoveToNext();
            for (; pChildNode; pChildNode = iteChild.MoveToNext()) {
              CXFA_LayoutItem* pLayoutItem = static_cast<CXFA_LayoutItem*>(
                  pChildNode->GetUserData(XFA_LAYOUTITEMKEY));
              if (pLayoutItem) {
                pNotify->OnLayoutEvent(pDocLayout, pLayoutItem,
                                       XFA_LAYOUTEVENT_ItemRemoving);
                delete pLayoutItem;
              }
            }
          } else if (eCurId != XFA_ELEMENT_ContentArea) {
            CXFA_LayoutItem* pLayoutItem = static_cast<CXFA_LayoutItem*>(
                pNode->GetUserData(XFA_LAYOUTITEMKEY));
            if (pLayoutItem) {
              pNotify->OnLayoutEvent(pDocLayout, pLayoutItem,
                                     XFA_LAYOUTEVENT_ItemRemoving);
              delete pLayoutItem;
            }
          }
          CXFA_Node* pNext = sIterator.SkipChildrenAndMoveToNext();
          pNode->GetNodeItem(XFA_NODEITEM_Parent)->RemoveChild(pNode);
          pNode = pNext;
        } else {
          pNode->SetFlag(XFA_NODEFLAG_UnusedNode, FALSE);
          pNode->SetFlag(XFA_NODEFLAG_Initialized);
          pNode = sIterator.MoveToNext();
        }
      } else {
        pNode->SetFlag(XFA_NODEFLAG_Initialized);
        pNode = sIterator.MoveToNext();
      }
    }
    pPendingPageSet = pNextPendingPageSet;
  }
}
void CXFA_LayoutPageMgr::LayoutPageSetContents() {
  CXFA_ContainerLayoutItem* pRootLayoutItem = this->GetRootLayoutItem();
  for (; pRootLayoutItem;
       pRootLayoutItem =
           (CXFA_ContainerLayoutItem*)pRootLayoutItem->m_pNextSibling) {
    CXFA_NodeIteratorTemplate<
        CXFA_ContainerLayoutItem,
        CXFA_TraverseStrategy_ContentAreaContainerLayoutItem>
        iterator(pRootLayoutItem);
    for (CXFA_ContainerLayoutItem* pContainerItem = iterator.GetCurrent();
         pContainerItem; pContainerItem = iterator.MoveToNext()) {
      CXFA_Node* pNode = pContainerItem->m_pFormNode;
      switch (pNode->GetClassID()) {
        case XFA_ELEMENT_PageArea:
          m_pLayoutProcessor->GetRootRootItemLayoutProcessor()
              ->DoLayoutPageArea(pContainerItem);
          break;
        default:
          break;
      }
    }
  }
}
void XFA_SyncContainer(IXFA_Notify* pNotify,
                       IXFA_DocLayout* pDocLayout,
                       CXFA_LayoutItem* pContainerItem,
                       FX_DWORD dwRelevant,
                       FX_BOOL bVisible,
                       int32_t nPageIndex) {
  FX_BOOL bVisibleItem = FALSE;
  FX_DWORD dwStatus = 0;
  FX_DWORD dwRelevantContainer = 0;
  if (bVisible) {
    XFA_ATTRIBUTEENUM eAttributeValue =
        pContainerItem->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Presence);
    if (eAttributeValue == XFA_ATTRIBUTEENUM_Visible ||
        eAttributeValue == XFA_ATTRIBUTEENUM_Unknown) {
      bVisibleItem = TRUE;
    }
    dwRelevantContainer =
        XFA_GetRelevant(pContainerItem->m_pFormNode, dwRelevant);
    dwStatus =
        (bVisibleItem ? XFA_LAYOUTSTATUS_Visible : 0) | dwRelevantContainer;
  }
  pNotify->OnLayoutEvent(pDocLayout, pContainerItem, XFA_LAYOUTEVENT_ItemAdded,
                         (void*)(uintptr_t)nPageIndex,
                         (void*)(uintptr_t)dwStatus);
  for (CXFA_LayoutItem* pChild = pContainerItem->m_pFirstChild; pChild;
       pChild = pChild->m_pNextSibling) {
    if (pChild->IsContentLayoutItem()) {
      XFA_SyncContainer(pNotify, pDocLayout, pChild, dwRelevantContainer,
                        bVisibleItem, nPageIndex);
    }
  }
}
void CXFA_LayoutPageMgr::SyncLayoutData() {
  MergePageSetContents();
  LayoutPageSetContents();
  IXFA_Notify* pNotify =
      m_pTemplatePageSetRoot->GetDocument()->GetParser()->GetNotify();
  int32_t nPageIdx = -1;
  CXFA_ContainerLayoutItem* pRootLayoutItem = this->GetRootLayoutItem();
  for (; pRootLayoutItem;
       pRootLayoutItem =
           (CXFA_ContainerLayoutItem*)pRootLayoutItem->m_pNextSibling) {
    CXFA_NodeIteratorTemplate<
        CXFA_ContainerLayoutItem,
        CXFA_TraverseStrategy_ContentAreaContainerLayoutItem>
        iteratorParent(pRootLayoutItem);
    for (CXFA_ContainerLayoutItem* pContainerItem = iteratorParent.GetCurrent();
         pContainerItem; pContainerItem = iteratorParent.MoveToNext()) {
      switch (pContainerItem->m_pFormNode->GetClassID()) {
        case XFA_ELEMENT_PageArea: {
          nPageIdx++;
          FX_DWORD dwRelevant =
              XFA_LAYOUTSTATUS_Viewable | XFA_LAYOUTSTATUS_Printable;
          CXFA_NodeIteratorTemplate<CXFA_LayoutItem,
                                    CXFA_TraverseStrategy_LayoutItem>
              iterator(pContainerItem);
          CXFA_LayoutItem* pChildLayoutItem = iterator.GetCurrent();
          while (pChildLayoutItem) {
            CXFA_ContentLayoutItem* pContentItem =
                pChildLayoutItem->AsContentLayoutItem();
            if (!pContentItem) {
              pChildLayoutItem = iterator.MoveToNext();
              continue;
            }
            FX_BOOL bVisible =
                (pContentItem->m_pFormNode->GetEnum(XFA_ATTRIBUTE_Presence) ==
                 XFA_ATTRIBUTEENUM_Visible);
            FX_DWORD dwRelevantChild =
                XFA_GetRelevant(pContentItem->m_pFormNode, dwRelevant);
            XFA_SyncContainer(pNotify, m_pLayoutProcessor, pContentItem,
                              dwRelevantChild, bVisible, nPageIdx);
            pChildLayoutItem = iterator.SkipChildrenAndMoveToNext();
          }
        } break;
        default:
          break;
      }
    }
  }
  int32_t nPage = m_PageArray.GetSize();
  for (int32_t i = nPage - 1; i >= m_nAvailPages; i--) {
    CXFA_ContainerLayoutItem* pPage = m_PageArray[i];
    m_PageArray.RemoveAt(i);
    pNotify->OnPageEvent(pPage, XFA_PAGEEVENT_PageRemoved);
    delete pPage;
  }
  ClearRecordList();
}
void XFA_ReleaseLayoutItem_NoPageArea(CXFA_LayoutItem* pLayoutItem) {
  CXFA_LayoutItem* pNext, * pNode = pLayoutItem->m_pFirstChild;
  while (pNode) {
    pNext = pNode->m_pNextSibling;
    pNode->m_pParent = NULL;
    XFA_ReleaseLayoutItem_NoPageArea(pNode);
    pNode = pNext;
  }
  if (pLayoutItem->m_pFormNode->GetClassID() != XFA_ELEMENT_PageArea) {
    delete pLayoutItem;
  }
}
void CXFA_LayoutPageMgr::PrepareLayout() {
  m_pPageSetCurRoot = NULL;
  m_ePageSetMode = XFA_ATTRIBUTEENUM_OrderedOccurrence;
  m_nAvailPages = 0;
  ClearRecordList();
  if (!m_pPageSetLayoutItemRoot) {
    return;
  }
  CXFA_ContainerLayoutItem* pRootLayoutItem = m_pPageSetLayoutItemRoot;
  if (pRootLayoutItem &&
      pRootLayoutItem->m_pFormNode->GetPacketID() == XFA_XDPPACKET_Form) {
    CXFA_Node* pPageSetFormNode = pRootLayoutItem->m_pFormNode;
    pRootLayoutItem->m_pFormNode->GetDocument()->m_pPendingPageSet.RemoveAll();
    if (pPageSetFormNode->HasFlag(XFA_NODEFLAG_HasRemoved)) {
      XFA_ReleaseLayoutItem(pRootLayoutItem);
      m_pPageSetLayoutItemRoot = NULL;
      pRootLayoutItem = NULL;
      pPageSetFormNode = NULL;
      m_PageArray.RemoveAll();
    }
    while (pPageSetFormNode) {
      CXFA_Node* pNextPageSet =
          pPageSetFormNode->GetNextSameClassSibling(XFA_ELEMENT_PageSet);
      pPageSetFormNode->GetNodeItem(XFA_NODEITEM_Parent)
          ->RemoveChild(pPageSetFormNode, FALSE);
      pRootLayoutItem->m_pFormNode->GetDocument()->m_pPendingPageSet.Add(
          pPageSetFormNode);
      pPageSetFormNode = pNextPageSet;
    }
  }
#if defined(_XFA_LAYOUTITEM_MAPCACHE_) || defined(_XFA_LAYOUTITEM_ProcessCACHE_)
  pRootLayoutItem = m_pPageSetLayoutItemRoot;
  CXFA_ContainerLayoutItem* pNextLayout = NULL;
  for (; pRootLayoutItem; pRootLayoutItem = pNextLayout) {
    pNextLayout = (CXFA_ContainerLayoutItem*)pRootLayoutItem->m_pNextSibling;
    SaveLayoutItem(pRootLayoutItem);
    delete pRootLayoutItem;
  }
  m_pPageSetLayoutItemRoot = NULL;
#else
  IXFA_Notify* pNotify =
      m_pLayoutProcessor->GetDocument()->GetParser()->GetNotify();
  pRootLayoutItem = m_pPageSetLayoutItemRoot;
  for (; pRootLayoutItem;
       pRootLayoutItem =
           (CXFA_ContainerLayoutItem*)pRootLayoutItem->m_pNextSibling) {
    CXFA_NodeIteratorTemplate<CXFA_ContainerLayoutItem,
                              CXFA_TraverseStrategy_PageAreaContainerLayoutItem>
        iterator(pRootLayoutItem);
    for (CXFA_ContainerLayoutItem* pContainerItem = iterator.GetCurrent();
         pContainerItem; pContainerItem = iterator.MoveToNext()) {
      if (pContainerItem->m_pFormNode->GetClassID() != XFA_ELEMENT_PageArea) {
        continue;
      }
      CXFA_NodeIteratorTemplate<CXFA_LayoutItem,
                                CXFA_TraverseStrategy_LayoutItem>
          iterator(pContainerItem);
      for (CXFA_LayoutItem* pLayoutItem = iterator.GetCurrent(); pLayoutItem;
           pLayoutItem = iterator.MoveToNext()) {
        if (!pLayoutItem->IsContentLayoutItem()) {
          continue;
        }
        pNotify->OnLayoutEvent(m_pLayoutProcessor, pLayoutItem,
                               XFA_LAYOUTEVENT_ItemRemoving);
      }
      pNotify->OnPageEvent(pContainerItem, XFA_PAGEEVENT_PageRemoved);
    }
  }
  pRootLayoutItem = m_pPageSetLayoutItemRoot;
  CXFA_ContainerLayoutItem* pNextLayout = NULL;
  for (; pRootLayoutItem; pRootLayoutItem = pNextLayout) {
    pNextLayout = (CXFA_ContainerLayoutItem*)pRootLayoutItem->m_pNextSibling;
    XFA_ReleaseLayoutItem_NoPageArea(pRootLayoutItem);
  }
  m_pPageSetLayoutItemRoot = NULL;
#endif
}
