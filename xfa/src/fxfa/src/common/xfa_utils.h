// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_UTILS_H_
#define _XFA_UTILS_H_
FX_BOOL XFA_FDEExtension_ResolveNamespaceQualifier(
    IFDE_XMLElement* pNode,
    const CFX_WideStringC& wsQualifier,
    CFX_WideString& wsNamespaceURI);
template <class NodeType, class TraverseStrategy>
class CXFA_NodeIteratorTemplate {
 public:
  CXFA_NodeIteratorTemplate(NodeType* pRootNode = NULL) : m_pRoot(pRootNode) {
    if (pRootNode) {
      m_NodeStack.Push(pRootNode);
    }
  }
  FX_BOOL Init(NodeType* pRootNode) {
    if (!pRootNode) {
      return FALSE;
    }
    m_pRoot = pRootNode;
    m_NodeStack.RemoveAll();
    m_NodeStack.Push(pRootNode);
    return TRUE;
  }
  void Clear() { m_NodeStack.RemoveAll(); }
  void Reset() {
    Clear();
    if (m_pRoot) {
      m_NodeStack.Push(m_pRoot);
    }
  }
  FX_BOOL SetCurrent(NodeType* pCurNode) {
    m_NodeStack.RemoveAll();
    if (pCurNode) {
      CFX_StackTemplate<NodeType*> revStack;
      NodeType* pNode;
      for (pNode = pCurNode; pNode && pNode != m_pRoot;
           pNode = TraverseStrategy::GetParent(pNode)) {
        revStack.Push(pNode);
      }
      if (!pNode) {
        return FALSE;
      }
      revStack.Push(m_pRoot);
      while (revStack.GetSize()) {
        m_NodeStack.Push(*revStack.GetTopElement());
        revStack.Pop();
      }
    }
    return TRUE;
  }
  NodeType* GetCurrent() const {
    return m_NodeStack.GetSize() ? *m_NodeStack.GetTopElement() : NULL;
  }
  NodeType* GetRoot() const { return m_pRoot; }
  NodeType* MoveToPrev() {
    int32_t nStackLength = m_NodeStack.GetSize();
    if (nStackLength == 1) {
      return NULL;
    } else if (nStackLength > 1) {
      NodeType* pCurItem = *m_NodeStack.GetTopElement();
      m_NodeStack.Pop();
      NodeType* pParentItem = *m_NodeStack.GetTopElement();
      NodeType* pParentFirstChildItem =
          TraverseStrategy::GetFirstChild(pParentItem);
      if (pCurItem == pParentFirstChildItem) {
        return pParentItem;
      }
      NodeType *pPrevItem = pParentFirstChildItem, *pPrevItemNext = NULL;
      for (; pPrevItem; pPrevItem = pPrevItemNext) {
        pPrevItemNext = TraverseStrategy::GetNextSibling(pPrevItem);
        if (!pPrevItemNext || pPrevItemNext == pCurItem) {
          break;
        }
      }
      m_NodeStack.Push(pPrevItem);
    } else {
      m_NodeStack.RemoveAll();
      if (m_pRoot) {
        m_NodeStack.Push(m_pRoot);
      }
    }
    if (m_NodeStack.GetSize() > 0) {
      NodeType* pChildItem = *m_NodeStack.GetTopElement();
      while ((pChildItem = TraverseStrategy::GetFirstChild(pChildItem)) !=
             NULL) {
        while (NodeType* pNextItem =
                   TraverseStrategy::GetNextSibling(pChildItem)) {
          pChildItem = pNextItem;
        }
        m_NodeStack.Push(pChildItem);
      }
      return *m_NodeStack.GetTopElement();
    }
    return NULL;
  }
  NodeType* MoveToNext() {
    NodeType** ppNode = NULL;
    NodeType* pCurrent = GetCurrent();
    while (m_NodeStack.GetSize() > 0) {
      while ((ppNode = m_NodeStack.GetTopElement()) != NULL) {
        if (pCurrent != *ppNode) {
          return *ppNode;
        }
        NodeType* pChild = TraverseStrategy::GetFirstChild(*ppNode);
        if (pChild == NULL) {
          break;
        }
        m_NodeStack.Push(pChild);
      }
      while ((ppNode = m_NodeStack.GetTopElement()) != NULL) {
        NodeType* pNext = TraverseStrategy::GetNextSibling(*ppNode);
        m_NodeStack.Pop();
        if (m_NodeStack.GetSize() == 0) {
          break;
        }
        if (pNext) {
          m_NodeStack.Push(pNext);
          break;
        }
      }
    }
    return NULL;
  }
  NodeType* SkipChildrenAndMoveToNext() {
    NodeType** ppNode = NULL;
    while ((ppNode = m_NodeStack.GetTopElement()) != NULL) {
      NodeType* pNext = TraverseStrategy::GetNextSibling(*ppNode);
      m_NodeStack.Pop();
      if (m_NodeStack.GetSize() == 0) {
        break;
      }
      if (pNext) {
        m_NodeStack.Push(pNext);
        break;
      }
    }
    return GetCurrent();
  }

 protected:
  NodeType* m_pRoot;
  CFX_StackTemplate<NodeType*> m_NodeStack;
};
template <class KeyType>
class CXFA_PtrSetTemplate : private CFX_MapPtrToPtr {
 public:
  CXFA_PtrSetTemplate() : CFX_MapPtrToPtr(10) {}

  int GetCount() const { return CFX_MapPtrToPtr::GetCount(); }

  FX_BOOL IsEmpty() const { return CFX_MapPtrToPtr::IsEmpty(); }

  FX_BOOL Lookup(KeyType key) const {
    void* pValue = NULL;
    return CFX_MapPtrToPtr::Lookup((void*)key, pValue);
  }

  FX_BOOL operator[](KeyType key) { return Lookup(key); }

  void Add(KeyType key) { CFX_MapPtrToPtr::SetAt((void*)key, (void*)key); }

  FX_BOOL RemoveKey(KeyType key) {
    return CFX_MapPtrToPtr::RemoveKey((void*)key);
  }

  void RemoveAll() { CFX_MapPtrToPtr::RemoveAll(); }

  FX_POSITION GetStartPosition() const {
    return CFX_MapPtrToPtr::GetStartPosition();
  }

  void GetNextAssoc(FX_POSITION& rNextPosition, KeyType& rKey) const {
    void* pKey = NULL;
    void* pValue = NULL;
    CFX_MapPtrToPtr::GetNextAssoc(rNextPosition, pKey, pValue);
    rKey = (KeyType)(uintptr_t)pKey;
  }
};
class CXFA_Node;
class CXFA_WidgetData;
#include "fxfa_localevalue.h"
CXFA_Node* XFA_CreateUIChild(CXFA_Node* pNode, XFA_ELEMENT& eWidgetType);
CXFA_LocaleValue XFA_GetLocaleValue(CXFA_WidgetData* pWidgetData);
CFX_WideString XFA_NumericLimit(const CFX_WideString& wsValue,
                                int32_t iLead,
                                int32_t iTread);
FX_DOUBLE XFA_WideStringToDouble(const CFX_WideString& wsStringVal);
FX_DOUBLE XFA_ByteStringToDouble(const CFX_ByteStringC& szStringVal);
int32_t XFA_MapRotation(int32_t nRotation);
#ifndef XFA_PARSE_HAS_LINEIDENTIFIER
#define XFA_PARSE_HAS_LINEIDENTIFIER
#endif
FX_BOOL XFA_RecognizeRichText(IFDE_XMLElement* pRichTextXMLNode);
void XFA_GetPlainTextFromRichText(IFDE_XMLNode* pXMLNode,
                                  CFX_WideString& wsPlainText);
FX_BOOL XFA_FieldIsMultiListBox(CXFA_Node* pFieldNode);
IFX_Stream* XFA_CreateWideTextRead(const CFX_WideString& wsBuffer);
FX_BOOL XFA_IsLayoutElement(XFA_ELEMENT eElement,
                            FX_BOOL bLayoutContainer = FALSE);
FX_BOOL XFA_IsTakingupSpace(XFA_ATTRIBUTEENUM ePresence);
FX_BOOL XFA_IsFlowingLayout(XFA_ATTRIBUTEENUM eLayout);
FX_BOOL XFA_IsHorizontalFlow(XFA_ATTRIBUTEENUM eLayout);
void XFA_DataExporter_DealWithDataGroupNode(CXFA_Node* pDataNode);
void XFA_DataExporter_RegenerateFormFile(CXFA_Node* pNode,
                                         IFX_Stream* pStream,
                                         const FX_CHAR* pChecksum = NULL,
                                         FX_BOOL bSaveXML = FALSE);
#endif
