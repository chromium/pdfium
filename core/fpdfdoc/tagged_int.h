// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_TAGGED_INT_H_
#define CORE_FPDFDOC_TAGGED_INT_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fpdfdoc/fpdf_tagged.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "third_party/base/stl_util.h"

class CPDF_StructElement;

struct CPDF_StructKid {
  enum { Invalid, Element, PageContent, StreamContent, Object } m_Type;

  union {
    struct {
      CPDF_StructElement* m_pElement;
      CPDF_Dictionary* m_pDict;
    } m_Element;
    struct {
      uint32_t m_PageObjNum;
      uint32_t m_ContentId;
    } m_PageContent;
    struct {
      uint32_t m_PageObjNum;
      uint32_t m_ContentId;
      uint32_t m_RefObjNum;
    } m_StreamContent;
    struct {
      uint32_t m_PageObjNum;
      uint32_t m_RefObjNum;
    } m_Object;
  };
};

class CPDF_StructTree final : public IPDF_StructTree {
 public:
  explicit CPDF_StructTree(const CPDF_Document* pDoc);
  ~CPDF_StructTree() override;

  // IPDF_StructTree:
  int CountTopElements() const override;
  IPDF_StructElement* GetTopElement(int i) const override;

  void LoadPageTree(const CPDF_Dictionary* pPageDict);
  CPDF_StructElement* AddPageNode(
      CPDF_Dictionary* pElement,
      std::map<CPDF_Dictionary*, CPDF_StructElement*>& map,
      int nLevel = 0);
  bool AddTopLevelNode(CPDF_Dictionary* pDict, CPDF_StructElement* pElement);

 protected:
  const CPDF_Dictionary* const m_pTreeRoot;
  const CPDF_Dictionary* const m_pRoleMap;
  const CPDF_Dictionary* m_pPage;
  std::vector<CFX_RetainPtr<CPDF_StructElement>> m_Kids;

  friend class CPDF_StructElement;
};

class CPDF_StructElement final : public IPDF_StructElement {
 public:
  CPDF_StructElement(CPDF_StructTree* pTree,
                     CPDF_StructElement* pParent,
                     CPDF_Dictionary* pDict);

  // IPDF_StructElement
  IPDF_StructTree* GetTree() const override;
  const CFX_ByteString& GetType() const override;
  IPDF_StructElement* GetParent() const override;
  CPDF_Dictionary* GetDict() const override;
  int CountKids() const override;
  IPDF_StructElement* GetKidIfElement(int index) const override;
  CPDF_Object* GetAttr(const CFX_ByteStringC& owner,
                       const CFX_ByteStringC& name,
                       bool bInheritable = false,
                       FX_FLOAT fLevel = 0.0F) override;
  CFX_ByteString GetName(const CFX_ByteStringC& owner,
                         const CFX_ByteStringC& name,
                         const CFX_ByteStringC& default_value,
                         bool bInheritable = false,
                         int subindex = -1) override;
  FX_ARGB GetColor(const CFX_ByteStringC& owner,
                   const CFX_ByteStringC& name,
                   FX_ARGB default_value,
                   bool bInheritable = false,
                   int subindex = -1) override;
  FX_FLOAT GetNumber(const CFX_ByteStringC& owner,
                     const CFX_ByteStringC& name,
                     FX_FLOAT default_value,
                     bool bInheritable = false,
                     int subindex = -1) override;
  int GetInteger(const CFX_ByteStringC& owner,
                 const CFX_ByteStringC& name,
                 int default_value,
                 bool bInheritable = false,
                 int subindex = -1) override;

  std::vector<CPDF_StructKid>* GetKids() { return &m_Kids; }
  void LoadKids(CPDF_Dictionary* pDict);
  void LoadKid(uint32_t PageObjNum, CPDF_Object* pObj, CPDF_StructKid* pKid);
  CPDF_Object* GetAttr(const CFX_ByteStringC& owner,
                       const CFX_ByteStringC& name,
                       bool bInheritable,
                       int subindex);

  CPDF_StructElement* Retain();
  void Release();

 protected:
  ~CPDF_StructElement() override;

  int m_RefCount;
  CPDF_StructTree* const m_pTree;
  CPDF_StructElement* const m_pParent;
  CPDF_Dictionary* const m_pDict;
  CFX_ByteString m_Type;
  std::vector<CPDF_StructKid> m_Kids;
};

#endif  // CORE_FPDFDOC_TAGGED_INT_H_
