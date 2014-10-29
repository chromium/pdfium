// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_UTILS
#define _FX_UTILS
class CFX_ThreadLock;
class CFX_BaseArray;
template<class baseType> class CFX_BaseArrayTemplate;
template<class baseType> class CFX_ObjectBaseArrayTemplate;
class CFX_BaseMassArray;
template<class baseType> class CFX_MassArrayTemplate;
template<class baseType> class CFX_ObjectMassArrayTemplate;
class CFX_BaseDiscreteArray;
template<class baseType> class CFX_DiscreteArrayTemplate;
class CFX_BaseStack;
template<class baseType> class CFX_StackTemplate;
template<class baseType> class CFX_ObjectStackTemplate;
template<class baseType> class CFX_CPLTreeNode;
template<class baseType> class CFX_CPLTree;
class CFX_ThreadLock
{
public:
    CFX_ThreadLock();
    virtual ~CFX_ThreadLock();
    void	Lock();
    void	Unlock();
private:
    FX_LPVOID	m_pData;
};
class CFX_BaseArray : public CFX_Target
{
protected:
    CFX_BaseArray(FX_INT32 iGrowSize, FX_INT32 iBlockSize);
    ~CFX_BaseArray();
    FX_INT32	GetSize() const;
    FX_INT32	GetBlockSize() const;
    FX_LPBYTE	AddSpaceTo(FX_INT32 index);
    FX_LPBYTE	GetAt(FX_INT32 index) const;
    FX_LPBYTE	GetBuffer() const;
    FX_INT32	Append(const CFX_BaseArray &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1);
    FX_INT32	Copy(const CFX_BaseArray &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1);
    FX_INT32	RemoveLast(FX_INT32 iCount = -1);
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE);
    FX_LPVOID	m_pData;
};
template<class baseType>
class CFX_BaseArrayTemplate : public CFX_BaseArray
{
public:
    CFX_BaseArrayTemplate(FX_INT32 iGrowSize = 100) : CFX_BaseArray(iGrowSize, sizeof(baseType)) {}
    CFX_BaseArrayTemplate(FX_INT32 iGrowSize, FX_INT32 iBlockSize) : CFX_BaseArray(iGrowSize, iBlockSize) {}
    FX_INT32	GetSize() const
    {
        return CFX_BaseArray::GetSize();
    }
    FX_INT32	GetBlockSize() const
    {
        return CFX_BaseArray::GetBlockSize();
    }
    baseType*	AddSpace()
    {
        return (baseType*)CFX_BaseArray::AddSpaceTo(CFX_BaseArray::GetSize());
    }
    FX_INT32	Add(const baseType &element)
    {
        FX_INT32 index = CFX_BaseArray::GetSize();
        *(baseType*)CFX_BaseArray::AddSpaceTo(index) = element;
        return index;
    }
    baseType*	GetBuffer() const
    {
        return (baseType*)CFX_BaseArray::GetBuffer();
    }
    baseType&	GetAt(FX_INT32 index) const
    {
        return *(baseType*)CFX_BaseArray::GetAt(index);
    }
    baseType*	GetPtrAt(FX_INT32 index) const
    {
        return (baseType*)CFX_BaseArray::GetAt(index);
    }
    void		SetAt(FX_INT32 index, const baseType &element)
    {
        *(baseType*)CFX_BaseArray::GetAt(index) = element;
    }
    void		SetAtGrow(FX_INT32 index, const baseType &element)
    {
        *(baseType*)CFX_BaseArray::AddSpaceTo(index) = element;
    }
    FX_INT32	Append(const CFX_BaseArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        return CFX_BaseArray::Append(src, iStart, iCount);
    }
    FX_INT32	Copy(const CFX_BaseArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        return CFX_BaseArray::Copy(src, iStart, iCount);
    }
    FX_INT32	RemoveLast(FX_INT32 iCount = -1)
    {
        return CFX_BaseArray::RemoveLast(iCount);
    }
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        CFX_BaseArray::RemoveAll(bLeaveMemory);
    }
};
typedef CFX_BaseArrayTemplate<FX_LPVOID>	CFDE_PtrArray;
typedef CFX_BaseArrayTemplate<FX_DWORD>		CFDE_DWordArray;
typedef CFX_BaseArrayTemplate<FX_WORD>		CFDE_WordArray;
template<class baseType>
class CFX_ObjectBaseArrayTemplate : public CFX_BaseArray
{
public:
    CFX_ObjectBaseArrayTemplate(FX_INT32 iGrowSize = 100) : CFX_BaseArray(iGrowSize, sizeof(baseType)) {}
    ~CFX_ObjectBaseArrayTemplate()
    {
        RemoveAll(FALSE);
    }
    FX_INT32	GetSize() const
    {
        return CFX_BaseArray::GetSize();
    }
    FX_INT32	GetBlockSize() const
    {
        return CFX_BaseArray::GetBlockSize();
    }
    FX_INT32	Add(const baseType &element)
    {
        FX_INT32 index = CFX_BaseArray::GetSize();
        baseType *p = (baseType*)CFX_BaseArray::AddSpaceTo(index);
        FXTARGET_New ((void*)p)baseType(element);
        return index;
    }
    baseType&	GetAt(FX_INT32 index) const
    {
        return *(baseType*)CFX_BaseArray::GetAt(index);
    }
    baseType*	GetPtrAt(FX_INT32 index) const
    {
        return (baseType*)CFX_BaseArray::GetAt(index);
    }
    FX_INT32	Append(const CFX_ObjectBaseArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        FXSYS_assert(GetBlockSize() == src.GetBlockSize());
        if (iCount == 0) {
            return 0;
        }
        FX_INT32 iSize = src.GetSize();
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
        FX_LPBYTE *pStart = CFX_BaseArray::GetAt(iSize);
        FX_INT32 iBlockSize = CFX_BaseArray::GetBlockSize();
        iSize = iStart + iCount;
        for (FX_INT32 i = iStart; i < iSize; i ++) {
            FXTARGET_NewWith ((void*)pStart)baseType(src.GetAt(i));
            pStart += iBlockSize;
        }
        return iCount;
    }
    FX_INT32	Copy(const CFX_ObjectBaseArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        FXSYS_assert(GetBlockSize() == src.GetBlockSize());
        if (iCount == 0) {
            return 0;
        }
        FX_INT32 iSize = src.GetSize();
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
        FX_LPBYTE *pStart = CFX_BaseArray::GetAt(0);
        FX_INT32 iBlockSize = CFX_BaseArray::GetBlockSize();
        iSize = iStart + iCount;
        for (FX_INT32 i = iStart; i < iSize; i ++) {
            FXTARGET_New ((void*)pStart)baseType(src.GetAt(i));
            pStart += iBlockSize;
        }
        return iCount;
    }
    FX_INT32	RemoveLast(FX_INT32 iCount = -1)
    {
        FX_INT32 iSize = CFX_BaseArray::GetSize();
        if (iCount < 0 || iCount > iSize) {
            iCount = iSize;
        }
        if (iCount == 0) {
            return iSize;
        }
        for (FX_INT32 i = iSize - iCount; i < iSize; i ++) {
            ((baseType*)GetPtrAt(i))->~baseType();
        }
        return CFX_BaseArray::RemoveLast(iCount);
    }
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        FX_INT32 iSize = CFX_BaseArray::GetSize();
        for (FX_INT32 i = 0; i < iSize; i ++) {
            ((baseType*)GetPtrAt(i))->~baseType();
        }
        CFX_BaseArray::RemoveAll(bLeaveMemory);
    }
};
class CFX_BaseMassArray : public CFX_Target
{
protected:
    CFX_BaseMassArray(FX_INT32 iChunkSize, FX_INT32 iBlockSize);
    ~CFX_BaseMassArray();
    FX_INT32	GetSize() const;
    FX_LPBYTE	AddSpaceTo(FX_INT32 index);
    FX_LPBYTE	GetAt(FX_INT32 index) const;
    FX_INT32	Append(const CFX_BaseMassArray &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1);
    FX_INT32	Copy(const CFX_BaseMassArray &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1);
    FX_INT32	RemoveLast(FX_INT32 iCount = -1);
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE);
    FX_LPVOID	m_pData;
};
template<class baseType>
class CFX_MassArrayTemplate : public CFX_BaseMassArray
{
public:
    CFX_MassArrayTemplate(FX_INT32 iChunkSize = 100) : CFX_BaseMassArray(iChunkSize, sizeof(baseType)) {}
    CFX_MassArrayTemplate(FX_INT32 iChunkSize, FX_INT32 iBlockSize) : CFX_BaseMassArray(iChunkSize, iBlockSize) {}
    FX_INT32	GetSize() const
    {
        return CFX_BaseMassArray::GetSize();
    }
    baseType*	AddSpace()
    {
        return (baseType*)CFX_BaseMassArray::AddSpaceTo(CFX_BaseMassArray::GetSize());
    }
    FX_INT32	Add(const baseType &element)
    {
        FX_INT32 index = CFX_BaseMassArray::GetSize();
        *(baseType*)CFX_BaseMassArray::AddSpaceTo(index) = element;
        return index;
    }
    baseType&	GetAt(FX_INT32 index) const
    {
        return *(baseType*)CFX_BaseMassArray::GetAt(index);
    }
    baseType*	GetPtrAt(FX_INT32 index) const
    {
        return (baseType*)CFX_BaseMassArray::GetAt(index);
    }
    void		SetAt(FX_INT32 index, const baseType &element)
    {
        *(baseType*)CFX_BaseMassArray::GetAt(index) = element;
    }
    void		SetAtGrow(FX_INT32 index, const baseType &element)
    {
        *(baseType*)CFX_BaseMassArray::AddSpaceTo(index) = element;
    }
    FX_INT32	Append(const CFX_MassArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        return CFX_BaseMassArray::Append(src, iStart, iCount);
    }
    FX_INT32	Copy(const CFX_MassArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        return CFX_BaseMassArray::Copy(src, iStart, iCount);
    }
    FX_INT32	RemoveLast(FX_INT32 iCount = -1)
    {
        return CFX_BaseMassArray::RemoveLast(iCount);
    }
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        CFX_BaseMassArray::RemoveAll(bLeaveMemory);
    }
};
typedef CFX_MassArrayTemplate<FX_LPVOID>	CFX_PtrMassArray;
typedef CFX_MassArrayTemplate<FX_INT32>		CFX_Int32MassArray;
typedef CFX_MassArrayTemplate<FX_DWORD>		CFX_DWordMassArray;
typedef CFX_MassArrayTemplate<FX_WORD>		CFX_WordMassArray;
typedef CFX_MassArrayTemplate<CFX_Rect>		CFX_RectMassArray;
typedef CFX_MassArrayTemplate<CFX_RectF>	CFX_RectFMassArray;
template<class baseType>
class CFX_ObjectMassArrayTemplate : public CFX_BaseMassArray
{
public:
    CFX_ObjectMassArrayTemplate(FX_INT32 iChunkSize = 100) : CFX_BaseMassArray(iChunkSize, sizeof(baseType)) {}
    ~CFX_ObjectMassArrayTemplate()
    {
        RemoveAll(FALSE);
    }
    FX_INT32	GetSize() const
    {
        return CFX_BaseMassArray::GetSize();
    }
    FX_INT32	Add(const baseType &element)
    {
        FX_INT32 index = CFX_BaseMassArray::GetSize();
        baseType *p = (baseType*)CFX_BaseMassArray::AddSpaceTo(index);
        FXTARGET_New ((void*)p)baseType(element);
        return index;
    }
    baseType&	GetAt(FX_INT32 index) const
    {
        return *(baseType*)CFX_BaseMassArray::GetAt(index);
    }
    baseType*	GetPtrAt(FX_INT32 index) const
    {
        return (baseType*)CFX_BaseMassArray::GetAt(index);
    }
    FX_INT32	Append(const CFX_ObjectMassArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        if (iCount == 0) {
            return CFX_BaseMassArray::GetSize();
        }
        FX_INT32 iSize = src.GetSize();
        FXSYS_assert(iStart > -1 && iStart < iSize);
        if (iCount < 0) {
            iCount = iSize;
        }
        FX_INT32 iEnd = iStart + iCount;
        if (iEnd > iSize) {
            iEnd = iSize;
        }
        for (FX_INT32 i = iStart; i < iEnd; i ++) {
            Add(src.GetAt(i));
        }
        return CFX_BaseMassArray::GetSize();
    }
    FX_INT32	Copy(const CFX_ObjectMassArrayTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        if (iCount == 0) {
            return CFX_BaseMassArray::GetSize();
        }
        FX_INT32 iSize = src.GetSize();
        FXSYS_assert(iStart > -1 && iStart < iSize);
        if (iCount < 0) {
            iCount = iSize;
        }
        FX_INT32 iEnd = iStart + iCount;
        if (iEnd > iSize) {
            iEnd = iSize;
        }
        RemoveAll(TRUE);
        for (FX_INT32 i = iStart; i < iEnd; i ++) {
            Add(src.GetAt(i));
        }
        return CFX_BaseMassArray::GetSize();
    }
    FX_INT32	RemoveLast(FX_INT32 iCount = -1)
    {
        FX_INT32 iSize = CFX_BaseMassArray::GetSize();
        if (iCount < 0 || iCount > iSize) {
            iCount = iSize;
        }
        if (iCount == 0) {
            return iSize;
        }
        for (FX_INT32 i = iSize - iCount; i < iSize; i ++) {
            ((baseType*)GetPtrAt(i))->~baseType();
        }
        return CFX_BaseMassArray::RemoveLast(iCount);
    }
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        FX_INT32 iSize = CFX_BaseMassArray::GetSize();
        for (FX_INT32 i = 0; i < iSize; i ++) {
            ((baseType*)GetPtrAt(i))->~baseType();
        }
        CFX_BaseMassArray::RemoveAll(bLeaveMemory);
    }
};
class CFX_BaseDiscreteArray : public CFX_Target
{
protected:
    CFX_BaseDiscreteArray(FX_INT32 iChunkSize, FX_INT32 iBlockSize);
    ~CFX_BaseDiscreteArray();
    FX_LPBYTE	AddSpaceTo(FX_INT32 index);
    FX_LPBYTE	GetAt(FX_INT32 index) const;
    void		RemoveAll();
    FX_LPVOID	m_pData;
};
template<class baseType>
class CFX_DiscreteArrayTemplate : public CFX_BaseDiscreteArray
{
public:
    CFX_DiscreteArrayTemplate(FX_INT32 iChunkSize = 100) : CFX_BaseDiscreteArray(iChunkSize, sizeof(baseType)) {}
    baseType&	GetAt(FX_INT32 index, const baseType &defValue) const
    {
        baseType *p = (baseType*)CFX_BaseDiscreteArray::GetAt(index);
        return p == NULL ? (baseType&)defValue : *p;
    }
    baseType*	GetPtrAt(FX_INT32 index) const
    {
        return (baseType*)CFX_BaseDiscreteArray::GetAt(index);
    }
    void		SetAtGrow(FX_INT32 index, const baseType &element)
    {
        *(baseType*)CFX_BaseDiscreteArray::AddSpaceTo(index) = element;
    }
    void		RemoveAll()
    {
        CFX_BaseDiscreteArray::RemoveAll();
    }
};
typedef CFX_DiscreteArrayTemplate<FX_LPVOID>	CFX_PtrDiscreteArray;
typedef CFX_DiscreteArrayTemplate<FX_DWORD>		CFX_DWordDiscreteArray;
typedef CFX_DiscreteArrayTemplate<FX_WORD>		CFX_WordDiscreteArray;
class CFX_BaseStack : public CFX_Target
{
protected:
    CFX_BaseStack(FX_INT32 iChunkSize, FX_INT32 iBlockSize);
    ~CFX_BaseStack();
    FX_LPBYTE	Push();
    void		Pop();
    FX_LPBYTE	GetTopElement() const;
    FX_INT32	GetSize() const;
    FX_LPBYTE	GetAt(FX_INT32 index) const;
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE);
    FX_LPVOID	m_pData;
};
template<class baseType>
class CFX_StackTemplate : public CFX_BaseStack
{
public:
    CFX_StackTemplate(FX_INT32 iChunkSize = 100) : CFX_BaseStack(iChunkSize, sizeof(baseType)) {}
    FX_INT32	Push(const baseType &element)
    {
        FX_INT32 index = CFX_BaseStack::GetSize();
        *(baseType*)CFX_BaseStack::Push() = element;
        return index;
    }
    void		Pop()
    {
        CFX_BaseStack::Pop();
    }
    baseType*	GetTopElement() const
    {
        return (baseType*)CFX_BaseStack::GetTopElement();
    }
    FX_INT32	GetSize() const
    {
        return CFX_BaseStack::GetSize();
    }
    baseType*	GetAt(FX_INT32 index) const
    {
        return (baseType*)CFX_BaseStack::GetAt(index);
    }
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        CFX_BaseStack::RemoveAll(bLeaveMemory);
    }
};
typedef CFX_StackTemplate<FX_LPVOID>	CFX_PtrStack;
typedef CFX_StackTemplate<FX_DWORD>	CFX_DWordStack;
typedef CFX_StackTemplate<FX_WORD>		CFX_WordStack;
typedef CFX_StackTemplate<FX_INT32>	CFX_Int32Stack;
template<class baseType>
class CFX_ObjectStackTemplate : public CFX_BaseStack
{
public:
    CFX_ObjectStackTemplate(FX_INT32 iChunkSize = 100) : CFX_BaseStack(iChunkSize, sizeof(baseType)) {}
    ~CFX_ObjectStackTemplate()
    {
        RemoveAll();
    }
    FX_INT32	Push(const baseType &element)
    {
        FX_INT32 index = CFX_BaseStack::GetSize();
        baseType *p = (baseType*)CFX_BaseStack::Push();
        FXTARGET_New ((void*)p)baseType(element);
        return index;
    }
    void		Pop()
    {
        baseType *p = (baseType*)CFX_BaseStack::GetTopElement();
        if (p != NULL) {
            p->~baseType();
        }
        CFX_BaseStack::Pop();
    }
    baseType*	GetTopElement() const
    {
        return (baseType*)CFX_BaseStack::GetTopElement();
    }
    FX_INT32	GetSize() const
    {
        return CFX_BaseStack::GetSize();
    }
    baseType*	GetAt(FX_INT32 index) const
    {
        return (baseType*)CFX_BaseStack::GetAt(index);
    }
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE)
    {
        FX_INT32 iSize = CFX_BaseStack::GetSize();
        for (FX_INT32 i = 0; i < iSize; i ++) {
            ((baseType*)CFX_BaseStack::GetAt(i))->~baseType();
        }
        CFX_BaseStack::RemoveAll(bLeaveMemory);
    }
    FX_INT32	Copy(const CFX_ObjectStackTemplate &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1)
    {
        if (iCount == 0) {
            return CFX_BaseStack::GetSize();
        }
        FX_INT32 iSize = src.GetSize();
        FXSYS_assert(iStart > -1 && iStart < iSize);
        if (iCount < 0) {
            iCount = iSize;
        }
        FX_INT32 iEnd = iStart + iCount;
        if (iEnd > iSize) {
            iEnd = iSize;
        }
        RemoveAll(TRUE);
        for (FX_INT32 i = iStart; i < iEnd; i ++) {
            Push(*src.GetAt(i));
        }
        return CFX_BaseStack::GetSize();
    }
};
template<class baseType>
class CFX_CPLTreeNode : public CFX_Target
{
public:
    typedef CFX_CPLTreeNode<baseType>	CPLTreeNode;
    CFX_CPLTreeNode() : m_pParentNode(NULL)
        , m_pChildNode(NULL)
        , m_pPrevNode(NULL)
        , m_pNextNode(NULL)
        , m_Data()
    {
    }
    enum TreeNode {Root = 0, Parent, FirstSibling, PreviousSibling, NextSibling, LastSibling, FirstNeighbor, PreviousNeighbor, NextNeighbor, LastNeighbor, FirstChild, LastChild};
    CPLTreeNode* GetNode(TreeNode eNode) const
    {
        switch (eNode) {
            case Root: {
                    CPLTreeNode *pParent = (CPLTreeNode*)this;
                    CPLTreeNode *pTemp;
                    while ((pTemp = pParent->m_pParentNode) != NULL) {
                        pParent = pTemp;
                    }
                    return pParent;
                }
            case Parent:
                return m_pParentNode;
            case FirstSibling: {
                    CPLTreeNode *pNode = (CPLTreeNode*)this;
                    CPLTreeNode *pTemp;
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
                    CPLTreeNode *pNode = (CPLTreeNode*)this;
                    CPLTreeNode *pTemp;
                    while ((pTemp = pNode->m_pNextNode) != NULL) {
                        pNode = pTemp;
                    }
                    return pNode == (CPLTreeNode*)this ? NULL : pNode;
                }
            case FirstNeighbor: {
                    CPLTreeNode *pParent = (CPLTreeNode*)this;
                    CPLTreeNode *pTemp;
                    while ((pTemp = pParent->m_pParentNode) != NULL) {
                        pParent = pTemp;
                    }
                    return pParent == (CPLTreeNode*)this ? NULL : pParent;
                }
            case PreviousNeighbor: {
                    if (m_pPrevNode == NULL) {
                        return m_pParentNode;
                    }
                    CPLTreeNode *pNode = m_pPrevNode;
                    CPLTreeNode *pTemp;
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
                    CPLTreeNode *pNode = m_pParentNode;
                    while (pNode != NULL) {
                        if (pNode->m_pNextNode != NULL) {
                            return pNode->m_pNextNode;
                        }
                        pNode = pNode->m_pParentNode;
                    }
                    return NULL;
                }
            case LastNeighbor: {
                    CPLTreeNode *pNode = (CPLTreeNode*)this;
                    CPLTreeNode *pTemp;
                    while ((pTemp = pNode->m_pParentNode) != NULL) {
                        pNode = pTemp;
                    }
                    while (TRUE) {
                        CPLTreeNode *pTemp;
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
                    CPLTreeNode *pChild = m_pChildNode;
                    CPLTreeNode *pTemp;
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
    void SetParentNode(CPLTreeNode *pNode)
    {
        m_pParentNode = pNode;
    }
    FX_INT32 CountChildNodes() const
    {
        FX_INT32 iCount = 0;
        CPLTreeNode *pNode = m_pChildNode;
        while (pNode) {
            iCount ++;
            pNode = pNode->m_pNextNode;
        }
        return iCount;
    }
    CPLTreeNode* GetChildNode(FX_INT32 iIndex) const
    {
        FX_INT32 iCount = 0;
        CPLTreeNode *pNode = m_pChildNode;
        while (pNode) {
            if (iIndex == iCount) {
                return pNode;
            }
            iCount ++;
            pNode = pNode->m_pNextNode;
        }
        return NULL;
    }
    FX_INT32 GetNodeIndex() const
    {
        FX_INT32 index = 0;
        CPLTreeNode *pNode = m_pPrevNode;
        while (pNode != NULL) {
            index ++;
            pNode = pNode->m_pPrevNode;
        }
        return index;
    }
    FX_BOOL IsParentNode(const CPLTreeNode *pNode) const
    {
        CPLTreeNode *pParent = m_pParentNode;
        while (pParent != NULL) {
            if (pParent == pNode) {
                return TRUE;
            }
            pParent = pParent->GetTreeNode(Parent);
        }
        return FALSE;
    }
    FX_BOOL IsChildNode(const CPLTreeNode *pNode) const
    {
        if (pNode == NULL) {
            return FALSE;
        }
        return pNode->IsParentNode((const CPLTreeNode*)this);
    }
    void SetChildNode(CPLTreeNode *pNode)
    {
        m_pChildNode = pNode;
    }
    void SetPrevNode(CPLTreeNode *pNode)
    {
        m_pPrevNode = pNode;
    }
    void SetNextNode(CPLTreeNode *pNode)
    {
        m_pNextNode = pNode;
    }
    FX_INT32 GetNodeLevel() const
    {
        FX_INT32 iLevel = 0;
        CPLTreeNode *pNode = (CPLTreeNode*)this;
        while ((pNode = pNode->m_pParentNode) != NULL) {
            iLevel ++;
        }
        return iLevel;
    }
    FX_BOOL IsRootNode() const
    {
        return m_pParentNode == NULL;
    }
    baseType GetData() const
    {
        return m_Data;
    }
    void SetData(baseType data)
    {
        m_Data = data;
    }
protected:
    CPLTreeNode		*m_pParentNode;
    CPLTreeNode		*m_pChildNode;
    CPLTreeNode		*m_pPrevNode;
    CPLTreeNode		*m_pNextNode;
    baseType		m_Data;
    friend class CFX_CPLTree<baseType>;
};
template<class baseType>
class CFX_CPLTree : public CFX_Object
{
public:
    typedef CFX_CPLTreeNode<baseType>	CPLTreeNode;
    CFX_CPLTree() : m_Root()
    {
    }
    ~CFX_CPLTree()
    {
        CPLTreeNode *pNode = m_Root.GetNode(CPLTreeNode::LastNeighbor);
        while (pNode != NULL) {
            if (pNode->IsRootNode()) {
                break;
            }
            CPLTreeNode *pTemp = pNode->GetNode(CPLTreeNode::PreviousNeighbor);
            delete pNode;
            pNode = pTemp;
        }
    }
    CPLTreeNode* GetRoot()
    {
        return &m_Root;
    }
    CPLTreeNode* AddChild(baseType data, CPLTreeNode *pParent = NULL)
    {
        if (pParent == NULL) {
            pParent = &m_Root;
        }
        CPLTreeNode *pChild = FXTARGET_New CPLTreeNode;
        pChild->SetParentNode(pParent);
        pChild->SetData(data);
        if (pParent->m_pChildNode == NULL) {
            pParent->m_pChildNode = pChild;
        } else {
            CPLTreeNode *pLast = pParent->GetNode(CPLTreeNode::LastChild);
            pChild->SetPrevNode(pLast);
            pLast->SetNextNode(pChild);
        }
        return pChild;
    }
protected:
    CPLTreeNode		m_Root;
};
#endif
