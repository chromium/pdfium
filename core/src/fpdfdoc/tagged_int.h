// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFDOC_TAGGED_INT_H_
#define CORE_SRC_FPDFDOC_TAGGED_INT_H_

#include "core/include/fpdfdoc/fpdf_tagged.h"

class CPDF_StructElementImpl;

class CPDF_StructTreeImpl : public CPDF_StructTree {
 public:
  explicit CPDF_StructTreeImpl(const CPDF_Document* pDoc);
  ~CPDF_StructTreeImpl() override;

  // CPDF_StructTree
  int CountTopElements() const override { return m_Kids.GetSize(); }
  CPDF_StructElement* GetTopElement(int i) const override {
    return (CPDF_StructElement*)m_Kids.GetAt(i);
  }

  void LoadDocTree();
  void LoadPageTree(const CPDF_Dictionary* pPageDict);
  CPDF_StructElementImpl* AddPageNode(CPDF_Dictionary* pElement,
                                      CFX_MapPtrToPtr& map,
                                      int nLevel = 0);
  FX_BOOL AddTopLevelNode(CPDF_Dictionary* pDict,
                          CPDF_StructElementImpl* pElement);

 protected:
  const CPDF_Dictionary* m_pTreeRoot;
  const CPDF_Dictionary* m_pRoleMap;
  const CPDF_Dictionary* m_pPage;
  CFX_ArrayTemplate<CPDF_StructElementImpl*> m_Kids;
  friend class CPDF_StructElementImpl;
};

class CPDF_StructElementImpl final : public CPDF_StructElement {
 public:
  CPDF_StructElementImpl(CPDF_StructTreeImpl* pTree,
                         CPDF_StructElementImpl* pParent,
                         CPDF_Dictionary* pDict);

  // CPDF_StructTreeImpl
  CPDF_StructTree* GetTree() const override { return m_pTree; }
  const CFX_ByteString& GetType() const override { return m_Type; }
  CPDF_StructElement* GetParent() const override { return m_pParent; }
  CPDF_Dictionary* GetDict() const override { return m_pDict; }
  int CountKids() const override { return m_Kids.GetSize(); }
  const CPDF_StructKid& GetKid(int index) const override {
    return m_Kids.GetData()[index];
  }
  CPDF_Object* GetAttr(const CFX_ByteStringC& owner,
                       const CFX_ByteStringC& name,
                       FX_BOOL bInheritable = FALSE,
                       FX_FLOAT fLevel = 0.0F) override;
  CFX_ByteString GetName(const CFX_ByteStringC& owner,
                         const CFX_ByteStringC& name,
                         const CFX_ByteStringC& default_value,
                         FX_BOOL bInheritable = FALSE,
                         int subindex = -1) override;
  FX_ARGB GetColor(const CFX_ByteStringC& owner,
                   const CFX_ByteStringC& name,
                   FX_ARGB default_value,
                   FX_BOOL bInheritable = FALSE,
                   int subindex = -1) override;
  FX_FLOAT GetNumber(const CFX_ByteStringC& owner,
                     const CFX_ByteStringC& name,
                     FX_FLOAT default_value,
                     FX_BOOL bInheritable = FALSE,
                     int subindex = -1) override;
  int GetInteger(const CFX_ByteStringC& owner,
                 const CFX_ByteStringC& name,
                 int default_value,
                 FX_BOOL bInheritable = FALSE,
                 int subindex = -1) override;

  void LoadKids(CPDF_Dictionary* pDict);
  void LoadKid(FX_DWORD PageObjNum, CPDF_Object* pObj, CPDF_StructKid* pKid);
  CPDF_Object* GetAttr(const CFX_ByteStringC& owner,
                       const CFX_ByteStringC& name,
                       FX_BOOL bInheritable,
                       int subindex);
  CPDF_StructElementImpl* Retain();
  void Release();

 protected:
  ~CPDF_StructElementImpl() override;

  CPDF_StructTreeImpl* m_pTree;
  CFX_ByteString m_Type;
  CPDF_StructElementImpl* m_pParent;
  CPDF_Dictionary* m_pDict;
  CFX_ArrayTemplate<CPDF_StructKid> m_Kids;
  int m_RefCount;

  friend class CPDF_StructTreeImpl;
};

#endif  // CORE_SRC_FPDFDOC_TAGGED_INT_H_
