// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_UTILS
#define _FX_UTILS

#include "fx_mem.h"
#include "core/include/fxcrt/fx_coordinates.h"  // For CFX_Rect.

class CFX_ThreadLock;
class CFX_BaseArray;
template <class baseType>
class CFX_BaseArrayTemplate;
template <class baseType>
class CFX_ObjectBaseArrayTemplate;
class CFX_BaseMassArray;
class CFX_BaseMassArrayImp;
template <class baseType>
class CFX_MassArrayTemplate;
template <class baseType>
class CFX_ObjectMassArrayTemplate;
class CFX_BaseDiscreteArray;
template <class baseType>
class CFX_DiscreteArrayTemplate;
class CFX_BaseStack;
template <class baseType>
class CFX_StackTemplate;
template <class baseType>
class CFX_ObjectStackTemplate;
template <class baseType>
class CFX_CPLTreeNode;
template <class baseType>
class CFX_CPLTree;
class FX_BASEARRAYDATA;

class CFX_ThreadLock {
 public:
  CFX_ThreadLock();
  virtual ~CFX_ThreadLock();
  void Lock();
  void Unlock();
};
class CFX_BaseArray : public CFX_Target {
 protected:
  CFX_BaseArray(int32_t iGrowSize, int32_t iBlockSize);
  ~CFX_BaseArray();
  int32_t GetSize() const;
  int32_t GetBlockSize() const;
  uint8_t* AddSpaceTo(int32_t index);
  uint8_t* GetAt(int32_t index) const;
  uint8_t* GetBuffer() const;
  int32_t Append(const CFX_BaseArray& src,
                 int32_t iStart = 0,
                 int32_t iCount = -1);
  int32_t Copy(const CFX_BaseArray& src,
               int32_t iStart = 0,
               int32_t iCount = -1);
  int32_t RemoveLast(int32_t iCount = -1);
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE);

  FX_BASEARRAYDATA* m_pData;
};
template <class baseType>
class CFX_BaseArrayTemplate : public CFX_BaseArray {
 public:
  CFX_BaseArrayTemplate(int32_t iGrowSize = 100)
      : CFX_BaseArray(iGrowSize, sizeof(baseType)) {}
  CFX_BaseArrayTemplate(int32_t iGrowSize, int32_t iBlockSize)
      : CFX_BaseArray(iGrowSize, iBlockSize) {}
  int32_t GetSize() const { return CFX_BaseArray::GetSize(); }
  int32_t GetBlockSize() const { return CFX_BaseArray::GetBlockSize(); }
  baseType* AddSpace() {
    return (baseType*)CFX_BaseArray::AddSpaceTo(CFX_BaseArray::GetSize());
  }
  int32_t Add(const baseType& element) {
    int32_t index = CFX_BaseArray::GetSize();
    *(baseType*)CFX_BaseArray::AddSpaceTo(index) = element;
    return index;
  }
  baseType* GetBuffer() const { return (baseType*)CFX_BaseArray::GetBuffer(); }
  baseType& GetAt(int32_t index) const {
    return *(baseType*)CFX_BaseArray::GetAt(index);
  }
  baseType* GetPtrAt(int32_t index) const {
    return (baseType*)CFX_BaseArray::GetAt(index);
  }
  void SetAt(int32_t index, const baseType& element) {
    *(baseType*)CFX_BaseArray::GetAt(index) = element;
  }
  void SetAtGrow(int32_t index, const baseType& element) {
    *(baseType*)CFX_BaseArray::AddSpaceTo(index) = element;
  }
  int32_t Append(const CFX_BaseArrayTemplate& src,
                 int32_t iStart = 0,
                 int32_t iCount = -1) {
    return CFX_BaseArray::Append(src, iStart, iCount);
  }
  int32_t Copy(const CFX_BaseArrayTemplate& src,
               int32_t iStart = 0,
               int32_t iCount = -1) {
    return CFX_BaseArray::Copy(src, iStart, iCount);
  }
  int32_t RemoveLast(int32_t iCount = -1) {
    return CFX_BaseArray::RemoveLast(iCount);
  }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    CFX_BaseArray::RemoveAll(bLeaveMemory);
  }
};
typedef CFX_BaseArrayTemplate<void*> CFDE_PtrArray;
typedef CFX_BaseArrayTemplate<FX_DWORD> CFDE_DWordArray;
typedef CFX_BaseArrayTemplate<FX_WORD> CFDE_WordArray;
template <class baseType>
class CFX_ObjectBaseArrayTemplate : public CFX_BaseArray {
 public:
  CFX_ObjectBaseArrayTemplate(int32_t iGrowSize = 100)
      : CFX_BaseArray(iGrowSize, sizeof(baseType)) {}
  ~CFX_ObjectBaseArrayTemplate() { RemoveAll(FALSE); }
  int32_t GetSize() const { return CFX_BaseArray::GetSize(); }
  int32_t GetBlockSize() const { return CFX_BaseArray::GetBlockSize(); }
  int32_t Add(const baseType& element) {
    int32_t index = CFX_BaseArray::GetSize();
    baseType* p = (baseType*)CFX_BaseArray::AddSpaceTo(index);
    new ((void*)p) baseType(element);
    return index;
  }
  baseType& GetAt(int32_t index) const {
    return *(baseType*)CFX_BaseArray::GetAt(index);
  }
  baseType* GetPtrAt(int32_t index) const {
    return (baseType*)CFX_BaseArray::GetAt(index);
  }
  int32_t Append(const CFX_ObjectBaseArrayTemplate& src,
                 int32_t iStart = 0,
                 int32_t iCount = -1) {
    FXSYS_assert(GetBlockSize() == src.GetBlockSize());
    if (iCount == 0) {
      return 0;
    }
    int32_t iSize = src.GetSize();
    FXSYS_assert(iStart > -1 && iStart < iSize);
    if (iCount < 0) {
      iCount = iSize;
    }
    if (iStart + iCount > iSize) {
      iCount = iSize - iStart;
    }
    if (iCount < 1) {
      return 0;
    }
    iSize = CFX_BaseArray::GetSize();
    CFX_BaseArray::AddSpaceTo(iSize + iCount - 1);
    uint8_t** pStart = CFX_BaseArray::GetAt(iSize);
    int32_t iBlockSize = CFX_BaseArray::GetBlockSize();
    iSize = iStart + iCount;
    for (int32_t i = iStart; i < iSize; i++) {
      FXTARGET_NewWith((void*)pStart) baseType(src.GetAt(i));
      pStart += iBlockSize;
    }
    return iCount;
  }
  int32_t Copy(const CFX_ObjectBaseArrayTemplate& src,
               int32_t iStart = 0,
               int32_t iCount = -1) {
    FXSYS_assert(GetBlockSize() == src.GetBlockSize());
    if (iCount == 0) {
      return 0;
    }
    int32_t iSize = src.GetSize();
    FXSYS_assert(iStart > -1 && iStart < iSize);
    if (iCount < 0) {
      iCount = iSize;
    }
    if (iStart + iCount > iSize) {
      iCount = iSize - iStart;
    }
    if (iCount < 1) {
      return 0;
    }
    RemoveAll(TRUE);
    CFX_BaseArray::AddSpaceTo(iCount - 1);
    uint8_t** pStart = CFX_BaseArray::GetAt(0);
    int32_t iBlockSize = CFX_BaseArray::GetBlockSize();
    iSize = iStart + iCount;
    for (int32_t i = iStart; i < iSize; i++) {
      new ((void*)pStart) baseType(src.GetAt(i));
      pStart += iBlockSize;
    }
    return iCount;
  }
  int32_t RemoveLast(int32_t iCount = -1) {
    int32_t iSize = CFX_BaseArray::GetSize();
    if (iCount < 0 || iCount > iSize) {
      iCount = iSize;
    }
    if (iCount == 0) {
      return iSize;
    }
    for (int32_t i = iSize - iCount; i < iSize; i++) {
      ((baseType*)GetPtrAt(i))->~baseType();
    }
    return CFX_BaseArray::RemoveLast(iCount);
  }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    int32_t iSize = CFX_BaseArray::GetSize();
    for (int32_t i = 0; i < iSize; i++) {
      ((baseType*)GetPtrAt(i))->~baseType();
    }
    CFX_BaseArray::RemoveAll(bLeaveMemory);
  }
};
class CFX_BaseMassArray : public CFX_Target {
 protected:
  CFX_BaseMassArray(int32_t iChunkSize, int32_t iBlockSize);
  ~CFX_BaseMassArray();
  int32_t GetSize() const;
  uint8_t* AddSpaceTo(int32_t index);
  uint8_t* GetAt(int32_t index) const;
  int32_t Append(const CFX_BaseMassArray& src,
                 int32_t iStart = 0,
                 int32_t iCount = -1);
  int32_t Copy(const CFX_BaseMassArray& src,
               int32_t iStart = 0,
               int32_t iCount = -1);
  int32_t RemoveLast(int32_t iCount = -1);
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE);
  CFX_BaseMassArrayImp* m_pData;
};
template <class baseType>
class CFX_MassArrayTemplate : public CFX_BaseMassArray {
 public:
  CFX_MassArrayTemplate(int32_t iChunkSize = 100)
      : CFX_BaseMassArray(iChunkSize, sizeof(baseType)) {}
  CFX_MassArrayTemplate(int32_t iChunkSize, int32_t iBlockSize)
      : CFX_BaseMassArray(iChunkSize, iBlockSize) {}
  int32_t GetSize() const { return CFX_BaseMassArray::GetSize(); }
  baseType* AddSpace() {
    return (baseType*)CFX_BaseMassArray::AddSpaceTo(
        CFX_BaseMassArray::GetSize());
  }
  int32_t Add(const baseType& element) {
    int32_t index = CFX_BaseMassArray::GetSize();
    *(baseType*)CFX_BaseMassArray::AddSpaceTo(index) = element;
    return index;
  }
  baseType& GetAt(int32_t index) const {
    return *(baseType*)CFX_BaseMassArray::GetAt(index);
  }
  baseType* GetPtrAt(int32_t index) const {
    return (baseType*)CFX_BaseMassArray::GetAt(index);
  }
  void SetAt(int32_t index, const baseType& element) {
    *(baseType*)CFX_BaseMassArray::GetAt(index) = element;
  }
  void SetAtGrow(int32_t index, const baseType& element) {
    *(baseType*)CFX_BaseMassArray::AddSpaceTo(index) = element;
  }
  int32_t Append(const CFX_MassArrayTemplate& src,
                 int32_t iStart = 0,
                 int32_t iCount = -1) {
    return CFX_BaseMassArray::Append(src, iStart, iCount);
  }
  int32_t Copy(const CFX_MassArrayTemplate& src,
               int32_t iStart = 0,
               int32_t iCount = -1) {
    return CFX_BaseMassArray::Copy(src, iStart, iCount);
  }
  int32_t RemoveLast(int32_t iCount = -1) {
    return CFX_BaseMassArray::RemoveLast(iCount);
  }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    CFX_BaseMassArray::RemoveAll(bLeaveMemory);
  }
};
typedef CFX_MassArrayTemplate<void*> CFX_PtrMassArray;
typedef CFX_MassArrayTemplate<int32_t> CFX_Int32MassArray;
typedef CFX_MassArrayTemplate<FX_DWORD> CFX_DWordMassArray;
typedef CFX_MassArrayTemplate<FX_WORD> CFX_WordMassArray;
typedef CFX_MassArrayTemplate<CFX_Rect> CFX_RectMassArray;
typedef CFX_MassArrayTemplate<CFX_RectF> CFX_RectFMassArray;
template <class baseType>
class CFX_ObjectMassArrayTemplate : public CFX_BaseMassArray {
 public:
  CFX_ObjectMassArrayTemplate(int32_t iChunkSize = 100)
      : CFX_BaseMassArray(iChunkSize, sizeof(baseType)) {}
  ~CFX_ObjectMassArrayTemplate() { RemoveAll(FALSE); }
  int32_t GetSize() const { return CFX_BaseMassArray::GetSize(); }
  int32_t Add(const baseType& element) {
    int32_t index = CFX_BaseMassArray::GetSize();
    baseType* p = (baseType*)CFX_BaseMassArray::AddSpaceTo(index);
    new ((void*)p) baseType(element);
    return index;
  }
  baseType& GetAt(int32_t index) const {
    return *(baseType*)CFX_BaseMassArray::GetAt(index);
  }
  baseType* GetPtrAt(int32_t index) const {
    return (baseType*)CFX_BaseMassArray::GetAt(index);
  }
  int32_t Append(const CFX_ObjectMassArrayTemplate& src,
                 int32_t iStart = 0,
                 int32_t iCount = -1) {
    if (iCount == 0) {
      return CFX_BaseMassArray::GetSize();
    }
    int32_t iSize = src.GetSize();
    FXSYS_assert(iStart > -1 && iStart < iSize);
    if (iCount < 0) {
      iCount = iSize;
    }
    int32_t iEnd = iStart + iCount;
    if (iEnd > iSize) {
      iEnd = iSize;
    }
    for (int32_t i = iStart; i < iEnd; i++) {
      Add(src.GetAt(i));
    }
    return CFX_BaseMassArray::GetSize();
  }
  int32_t Copy(const CFX_ObjectMassArrayTemplate& src,
               int32_t iStart = 0,
               int32_t iCount = -1) {
    if (iCount == 0) {
      return CFX_BaseMassArray::GetSize();
    }
    int32_t iSize = src.GetSize();
    FXSYS_assert(iStart > -1 && iStart < iSize);
    if (iCount < 0) {
      iCount = iSize;
    }
    int32_t iEnd = iStart + iCount;
    if (iEnd > iSize) {
      iEnd = iSize;
    }
    RemoveAll(TRUE);
    for (int32_t i = iStart; i < iEnd; i++) {
      Add(src.GetAt(i));
    }
    return CFX_BaseMassArray::GetSize();
  }
  int32_t RemoveLast(int32_t iCount = -1) {
    int32_t iSize = CFX_BaseMassArray::GetSize();
    if (iCount < 0 || iCount > iSize) {
      iCount = iSize;
    }
    if (iCount == 0) {
      return iSize;
    }
    for (int32_t i = iSize - iCount; i < iSize; i++) {
      ((baseType*)GetPtrAt(i))->~baseType();
    }
    return CFX_BaseMassArray::RemoveLast(iCount);
  }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    int32_t iSize = CFX_BaseMassArray::GetSize();
    for (int32_t i = 0; i < iSize; i++) {
      ((baseType*)GetPtrAt(i))->~baseType();
    }
    CFX_BaseMassArray::RemoveAll(bLeaveMemory);
  }
};
class CFX_BaseDiscreteArray : public CFX_Target {
 protected:
  CFX_BaseDiscreteArray(int32_t iChunkSize, int32_t iBlockSize);
  ~CFX_BaseDiscreteArray();
  uint8_t* AddSpaceTo(int32_t index);
  uint8_t* GetAt(int32_t index) const;
  void RemoveAll();
  void* m_pData;
};
template <class baseType>
class CFX_DiscreteArrayTemplate : public CFX_BaseDiscreteArray {
 public:
  CFX_DiscreteArrayTemplate(int32_t iChunkSize = 100)
      : CFX_BaseDiscreteArray(iChunkSize, sizeof(baseType)) {}
  baseType& GetAt(int32_t index, const baseType& defValue) const {
    baseType* p = (baseType*)CFX_BaseDiscreteArray::GetAt(index);
    return p == NULL ? (baseType&)defValue : *p;
  }
  baseType* GetPtrAt(int32_t index) const {
    return (baseType*)CFX_BaseDiscreteArray::GetAt(index);
  }
  void SetAtGrow(int32_t index, const baseType& element) {
    *(baseType*)CFX_BaseDiscreteArray::AddSpaceTo(index) = element;
  }
  void RemoveAll() { CFX_BaseDiscreteArray::RemoveAll(); }
};
typedef CFX_DiscreteArrayTemplate<void*> CFX_PtrDiscreteArray;
typedef CFX_DiscreteArrayTemplate<FX_DWORD> CFX_DWordDiscreteArray;
typedef CFX_DiscreteArrayTemplate<FX_WORD> CFX_WordDiscreteArray;
class CFX_BaseStack : public CFX_Target {
 protected:
  CFX_BaseStack(int32_t iChunkSize, int32_t iBlockSize);
  ~CFX_BaseStack();
  uint8_t* Push();
  void Pop();
  uint8_t* GetTopElement() const;
  int32_t GetSize() const;
  uint8_t* GetAt(int32_t index) const;
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE);
  CFX_BaseMassArrayImp* m_pData;
};
template <class baseType>
class CFX_StackTemplate : public CFX_BaseStack {
 public:
  CFX_StackTemplate(int32_t iChunkSize = 100)
      : CFX_BaseStack(iChunkSize, sizeof(baseType)) {}
  int32_t Push(const baseType& element) {
    int32_t index = CFX_BaseStack::GetSize();
    *(baseType*)CFX_BaseStack::Push() = element;
    return index;
  }
  void Pop() { CFX_BaseStack::Pop(); }
  baseType* GetTopElement() const {
    return (baseType*)CFX_BaseStack::GetTopElement();
  }
  int32_t GetSize() const { return CFX_BaseStack::GetSize(); }
  baseType* GetAt(int32_t index) const {
    return (baseType*)CFX_BaseStack::GetAt(index);
  }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    CFX_BaseStack::RemoveAll(bLeaveMemory);
  }
};
typedef CFX_StackTemplate<void*> CFX_PtrStack;
typedef CFX_StackTemplate<FX_DWORD> CFX_DWordStack;
typedef CFX_StackTemplate<FX_WORD> CFX_WordStack;
typedef CFX_StackTemplate<int32_t> CFX_Int32Stack;
template <class baseType>
class CFX_ObjectStackTemplate : public CFX_BaseStack {
 public:
  CFX_ObjectStackTemplate(int32_t iChunkSize = 100)
      : CFX_BaseStack(iChunkSize, sizeof(baseType)) {}
  ~CFX_ObjectStackTemplate() { RemoveAll(); }
  int32_t Push(const baseType& element) {
    int32_t index = CFX_BaseStack::GetSize();
    baseType* p = (baseType*)CFX_BaseStack::Push();
    new ((void*)p) baseType(element);
    return index;
  }
  void Pop() {
    baseType* p = (baseType*)CFX_BaseStack::GetTopElement();
    if (p != NULL) {
      p->~baseType();
    }
    CFX_BaseStack::Pop();
  }
  baseType* GetTopElement() const {
    return (baseType*)CFX_BaseStack::GetTopElement();
  }
  int32_t GetSize() const { return CFX_BaseStack::GetSize(); }
  baseType* GetAt(int32_t index) const {
    return (baseType*)CFX_BaseStack::GetAt(index);
  }
  void RemoveAll(FX_BOOL bLeaveMemory = FALSE) {
    int32_t iSize = CFX_BaseStack::GetSize();
    for (int32_t i = 0; i < iSize; i++) {
      ((baseType*)CFX_BaseStack::GetAt(i))->~baseType();
    }
    CFX_BaseStack::RemoveAll(bLeaveMemory);
  }
  int32_t Copy(const CFX_ObjectStackTemplate& src,
               int32_t iStart = 0,
               int32_t iCount = -1) {
    if (iCount == 0) {
      return CFX_BaseStack::GetSize();
    }
    int32_t iSize = src.GetSize();
    FXSYS_assert(iStart > -1 && iStart < iSize);
    if (iCount < 0) {
      iCount = iSize;
    }
    int32_t iEnd = iStart + iCount;
    if (iEnd > iSize) {
      iEnd = iSize;
    }
    RemoveAll(TRUE);
    for (int32_t i = iStart; i < iEnd; i++) {
      Push(*src.GetAt(i));
    }
    return CFX_BaseStack::GetSize();
  }
};
template <class baseType>
class CFX_CPLTreeNode : public CFX_Target {
 public:
  typedef CFX_CPLTreeNode<baseType> CPLTreeNode;
  CFX_CPLTreeNode()
      : m_pParentNode(NULL),
        m_pChildNode(NULL),
        m_pPrevNode(NULL),
        m_pNextNode(NULL),
        m_Data() {}
  enum TreeNode {
    Root = 0,
    Parent,
    FirstSibling,
    PreviousSibling,
    NextSibling,
    LastSibling,
    FirstNeighbor,
    PreviousNeighbor,
    NextNeighbor,
    LastNeighbor,
    FirstChild,
    LastChild
  };
  CPLTreeNode* GetNode(TreeNode eNode) const {
    switch (eNode) {
      case Root: {
        CPLTreeNode* pParent = (CPLTreeNode*)this;
        CPLTreeNode* pTemp;
        while ((pTemp = pParent->m_pParentNode) != NULL) {
          pParent = pTemp;
        }
        return pParent;
      }
      case Parent:
        return m_pParentNode;
      case FirstSibling: {
        CPLTreeNode* pNode = (CPLTreeNode*)this;
        CPLTreeNode* pTemp;
        while ((pTemp = pNode->m_pPrevNode) != NULL) {
          pNode = pTemp;
        }
        return pNode == (CPLTreeNode*)this ? NULL : pNode;
      }
      case PreviousSibling:
        return m_pPrevNode;
      case NextSibling:
        return m_pNextNode;
      case LastSibling: {
        CPLTreeNode* pNode = (CPLTreeNode*)this;
        CPLTreeNode* pTemp;
        while ((pTemp = pNode->m_pNextNode) != NULL) {
          pNode = pTemp;
        }
        return pNode == (CPLTreeNode*)this ? NULL : pNode;
      }
      case FirstNeighbor: {
        CPLTreeNode* pParent = (CPLTreeNode*)this;
        CPLTreeNode* pTemp;
        while ((pTemp = pParent->m_pParentNode) != NULL) {
          pParent = pTemp;
        }
        return pParent == (CPLTreeNode*)this ? NULL : pParent;
      }
      case PreviousNeighbor: {
        if (m_pPrevNode == NULL) {
          return m_pParentNode;
        }
        CPLTreeNode* pNode = m_pPrevNode;
        CPLTreeNode* pTemp;
        while ((pTemp = pNode->m_pChildNode) != NULL) {
          pNode = pTemp;
          while ((pTemp = pNode->m_pNextNode) != NULL) {
            pNode = pTemp;
          }
        }
        return pNode;
      }
      case NextNeighbor: {
        if (m_pChildNode != NULL) {
          return m_pChildNode;
        }
        if (m_pNextNode != NULL) {
          return m_pNextNode;
        }
        CPLTreeNode* pNode = m_pParentNode;
        while (pNode != NULL) {
          if (pNode->m_pNextNode != NULL) {
            return pNode->m_pNextNode;
          }
          pNode = pNode->m_pParentNode;
        }
        return NULL;
      }
      case LastNeighbor: {
        CPLTreeNode* pNode = (CPLTreeNode*)this;
        CPLTreeNode* pTemp;
        while ((pTemp = pNode->m_pParentNode) != NULL) {
          pNode = pTemp;
        }
        while (TRUE) {
          CPLTreeNode* pTemp;
          while ((pTemp = pNode->m_pNextNode) != NULL) {
            pNode = pTemp;
          }
          if (pNode->m_pChildNode == NULL) {
            break;
          }
          pNode = pNode->m_pChildNode;
        }
        return pNode == (CPLTreeNode*)this ? NULL : pNode;
      }
      case FirstChild:
        return m_pChildNode;
      case LastChild: {
        if (m_pChildNode == NULL) {
          return NULL;
        }
        CPLTreeNode* pChild = m_pChildNode;
        CPLTreeNode* pTemp;
        while ((pTemp = pChild->m_pNextNode) != NULL) {
          pChild = pTemp;
        }
        return pChild;
      }
      default:
        break;
    }
    return NULL;
  }
  void SetParentNode(CPLTreeNode* pNode) { m_pParentNode = pNode; }
  int32_t CountChildNodes() const {
    int32_t iCount = 0;
    CPLTreeNode* pNode = m_pChildNode;
    while (pNode) {
      iCount++;
      pNode = pNode->m_pNextNode;
    }
    return iCount;
  }
  CPLTreeNode* GetChildNode(int32_t iIndex) const {
    int32_t iCount = 0;
    CPLTreeNode* pNode = m_pChildNode;
    while (pNode) {
      if (iIndex == iCount) {
        return pNode;
      }
      iCount++;
      pNode = pNode->m_pNextNode;
    }
    return NULL;
  }
  int32_t GetNodeIndex() const {
    int32_t index = 0;
    CPLTreeNode* pNode = m_pPrevNode;
    while (pNode != NULL) {
      index++;
      pNode = pNode->m_pPrevNode;
    }
    return index;
  }
  FX_BOOL IsParentNode(const CPLTreeNode* pNode) const {
    CPLTreeNode* pParent = m_pParentNode;
    while (pParent != NULL) {
      if (pParent == pNode) {
        return TRUE;
      }
      pParent = pParent->GetTreeNode(Parent);
    }
    return FALSE;
  }
  FX_BOOL IsChildNode(const CPLTreeNode* pNode) const {
    if (pNode == NULL) {
      return FALSE;
    }
    return pNode->IsParentNode((const CPLTreeNode*)this);
  }
  void SetChildNode(CPLTreeNode* pNode) { m_pChildNode = pNode; }
  void SetPrevNode(CPLTreeNode* pNode) { m_pPrevNode = pNode; }
  void SetNextNode(CPLTreeNode* pNode) { m_pNextNode = pNode; }
  int32_t GetNodeLevel() const {
    int32_t iLevel = 0;
    CPLTreeNode* pNode = (CPLTreeNode*)this;
    while ((pNode = pNode->m_pParentNode) != NULL) {
      iLevel++;
    }
    return iLevel;
  }
  FX_BOOL IsRootNode() const { return m_pParentNode == NULL; }
  baseType GetData() const { return m_Data; }
  void SetData(baseType data) { m_Data = data; }

 protected:
  CPLTreeNode* m_pParentNode;
  CPLTreeNode* m_pChildNode;
  CPLTreeNode* m_pPrevNode;
  CPLTreeNode* m_pNextNode;
  baseType m_Data;
  friend class CFX_CPLTree<baseType>;
};
template <class baseType>
class CFX_CPLTree {
 public:
  typedef CFX_CPLTreeNode<baseType> CPLTreeNode;
  CFX_CPLTree() : m_Root() {}
  ~CFX_CPLTree() {
    CPLTreeNode* pNode = m_Root.GetNode(CPLTreeNode::LastNeighbor);
    while (pNode != NULL) {
      if (pNode->IsRootNode()) {
        break;
      }
      CPLTreeNode* pTemp = pNode->GetNode(CPLTreeNode::PreviousNeighbor);
      delete pNode;
      pNode = pTemp;
    }
  }
  CPLTreeNode* GetRoot() { return &m_Root; }
  CPLTreeNode* AddChild(baseType data, CPLTreeNode* pParent = NULL) {
    if (pParent == NULL) {
      pParent = &m_Root;
    }
    CPLTreeNode* pChild = new CPLTreeNode;
    pChild->SetParentNode(pParent);
    pChild->SetData(data);
    if (pParent->m_pChildNode == NULL) {
      pParent->m_pChildNode = pChild;
    } else {
      CPLTreeNode* pLast = pParent->GetNode(CPLTreeNode::LastChild);
      pChild->SetPrevNode(pLast);
      pLast->SetNextNode(pChild);
    }
    return pChild;
  }

 protected:
  CPLTreeNode m_Root;
};
#endif
