// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <map>

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_reference.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfdoc/doc_utils.h"
#include "core/fpdfdoc/tagged_int.h"
#include "core/include/fpdfdoc/fpdf_tagged.h"

const int nMaxRecursion = 32;
static FX_BOOL IsTagged(const CPDF_Document* pDoc) {
  CPDF_Dictionary* pCatalog = pDoc->GetRoot();
  CPDF_Dictionary* pMarkInfo = pCatalog->GetDictBy("MarkInfo");
  return pMarkInfo && pMarkInfo->GetIntegerBy("Marked");
}
CPDF_StructTree* CPDF_StructTree::LoadPage(const CPDF_Document* pDoc,
                                           const CPDF_Dictionary* pPageDict) {
  if (!IsTagged(pDoc)) {
    return NULL;
  }
  CPDF_StructTreeImpl* pTree = new CPDF_StructTreeImpl(pDoc);
  pTree->LoadPageTree(pPageDict);
  return pTree;
}
CPDF_StructTree* CPDF_StructTree::LoadDoc(const CPDF_Document* pDoc) {
  if (!IsTagged(pDoc)) {
    return NULL;
  }
  CPDF_StructTreeImpl* pTree = new CPDF_StructTreeImpl(pDoc);
  pTree->LoadDocTree();
  return pTree;
}
CPDF_StructTreeImpl::CPDF_StructTreeImpl(const CPDF_Document* pDoc) {
  CPDF_Dictionary* pCatalog = pDoc->GetRoot();
  m_pTreeRoot = pCatalog->GetDictBy("StructTreeRoot");
  if (!m_pTreeRoot) {
    return;
  }
  m_pRoleMap = m_pTreeRoot->GetDictBy("RoleMap");
}
CPDF_StructTreeImpl::~CPDF_StructTreeImpl() {
  for (int i = 0; i < m_Kids.GetSize(); i++)
    if (m_Kids[i]) {
      m_Kids[i]->Release();
    }
}
void CPDF_StructTreeImpl::LoadDocTree() {
  m_pPage = nullptr;
  if (!m_pTreeRoot)
    return;

  CPDF_Object* pKids = m_pTreeRoot->GetDirectObjectBy("K");
  if (!pKids)
    return;
  if (CPDF_Dictionary* pDict = pKids->AsDictionary()) {
    CPDF_StructElementImpl* pStructElementImpl =
        new CPDF_StructElementImpl(this, nullptr, pDict);
    m_Kids.Add(pStructElementImpl);
    return;
  }
  CPDF_Array* pArray = pKids->AsArray();
  if (!pArray)
    return;

  for (uint32_t i = 0; i < pArray->GetCount(); i++) {
    CPDF_Dictionary* pKid = pArray->GetDictAt(i);
    CPDF_StructElementImpl* pStructElementImpl =
        new CPDF_StructElementImpl(this, nullptr, pKid);
    m_Kids.Add(pStructElementImpl);
  }
}
void CPDF_StructTreeImpl::LoadPageTree(const CPDF_Dictionary* pPageDict) {
  m_pPage = pPageDict;
  if (!m_pTreeRoot)
    return;

  CPDF_Object* pKids = m_pTreeRoot->GetDirectObjectBy("K");
  if (!pKids)
    return;

  uint32_t dwKids = 0;
  if (pKids->IsDictionary())
    dwKids = 1;
  else if (CPDF_Array* pArray = pKids->AsArray())
    dwKids = pArray->GetCount();
  else
    return;

  uint32_t i;
  m_Kids.SetSize(dwKids);
  for (i = 0; i < dwKids; i++) {
    m_Kids[i] = NULL;
  }
  std::map<CPDF_Dictionary*, CPDF_StructElementImpl*> element_map;
  CPDF_Dictionary* pParentTree = m_pTreeRoot->GetDictBy("ParentTree");
  if (!pParentTree) {
    return;
  }
  CPDF_NumberTree parent_tree(pParentTree);
  int parents_id = pPageDict->GetIntegerBy("StructParents", -1);
  if (parents_id >= 0) {
    CPDF_Array* pParentArray = ToArray(parent_tree.LookupValue(parents_id));
    if (!pParentArray)
      return;

    for (i = 0; i < pParentArray->GetCount(); i++) {
      CPDF_Dictionary* pParent = pParentArray->GetDictAt(i);
      if (!pParent) {
        continue;
      }
      AddPageNode(pParent, element_map);
    }
  }
}
CPDF_StructElementImpl* CPDF_StructTreeImpl::AddPageNode(
    CPDF_Dictionary* pDict,
    std::map<CPDF_Dictionary*, CPDF_StructElementImpl*>& map,
    int nLevel) {
  if (nLevel > nMaxRecursion)
    return NULL;

  auto it = map.find(pDict);
  if (it != map.end())
    return it->second;

  CPDF_StructElementImpl* pElement =
      new CPDF_StructElementImpl(this, NULL, pDict);
  map[pDict] = pElement;
  CPDF_Dictionary* pParent = pDict->GetDictBy("P");
  if (!pParent || pParent->GetStringBy("Type") == "StructTreeRoot") {
    if (!AddTopLevelNode(pDict, pElement)) {
      pElement->Release();
      map.erase(pDict);
    }
  } else {
    CPDF_StructElementImpl* pParentElement =
        AddPageNode(pParent, map, nLevel + 1);
    FX_BOOL bSave = FALSE;
    for (int i = 0; i < pParentElement->m_Kids.GetSize(); i++) {
      if (pParentElement->m_Kids[i].m_Type != CPDF_StructKid::Element) {
        continue;
      }
      if (pParentElement->m_Kids[i].m_Element.m_pDict != pDict) {
        continue;
      }
      pParentElement->m_Kids[i].m_Element.m_pElement = pElement->Retain();
      bSave = TRUE;
    }
    if (!bSave) {
      pElement->Release();
      map.erase(pDict);
    }
  }
  return pElement;
}
FX_BOOL CPDF_StructTreeImpl::AddTopLevelNode(CPDF_Dictionary* pDict,
                                             CPDF_StructElementImpl* pElement) {
  CPDF_Object* pObj = m_pTreeRoot->GetDirectObjectBy("K");
  if (!pObj) {
    return FALSE;
  }
  if (pObj->IsDictionary()) {
    if (pObj->GetObjNum() == pDict->GetObjNum()) {
      if (m_Kids[0]) {
        m_Kids[0]->Release();
      }
      m_Kids[0] = pElement->Retain();
    } else {
      return FALSE;
    }
  }
  if (CPDF_Array* pTopKids = pObj->AsArray()) {
    uint32_t i;
    FX_BOOL bSave = FALSE;
    for (i = 0; i < pTopKids->GetCount(); i++) {
      CPDF_Reference* pKidRef = ToReference(pTopKids->GetObjectAt(i));
      if (!pKidRef)
        continue;
      if (pKidRef->GetRefObjNum() != pDict->GetObjNum())
        continue;

      if (m_Kids[i])
        m_Kids[i]->Release();
      m_Kids[i] = pElement->Retain();
      bSave = TRUE;
    }
    if (!bSave)
      return FALSE;
  }
  return TRUE;
}
CPDF_StructElementImpl::CPDF_StructElementImpl(CPDF_StructTreeImpl* pTree,
                                               CPDF_StructElementImpl* pParent,
                                               CPDF_Dictionary* pDict)
    : m_RefCount(0) {
  m_pTree = pTree;
  m_pDict = pDict;
  m_Type = pDict->GetStringBy("S");
  if (pTree->m_pRoleMap) {
    CFX_ByteString mapped =
        pTree->m_pRoleMap->GetStringBy(m_Type.AsByteStringC());
    if (!mapped.IsEmpty()) {
      m_Type = mapped;
    }
  }
  m_pParent = pParent;
  LoadKids(pDict);
}
CPDF_StructElementImpl::~CPDF_StructElementImpl() {
  for (int i = 0; i < m_Kids.GetSize(); i++) {
    if (m_Kids[i].m_Type == CPDF_StructKid::Element &&
        m_Kids[i].m_Element.m_pElement) {
      ((CPDF_StructElementImpl*)m_Kids[i].m_Element.m_pElement)->Release();
    }
  }
}
CPDF_StructElementImpl* CPDF_StructElementImpl::Retain() {
  m_RefCount++;
  return this;
}
void CPDF_StructElementImpl::Release() {
  if (--m_RefCount < 1) {
    delete this;
  }
}
void CPDF_StructElementImpl::LoadKids(CPDF_Dictionary* pDict) {
  CPDF_Object* pObj = pDict->GetObjectBy("Pg");
  uint32_t PageObjNum = 0;
  if (CPDF_Reference* pRef = ToReference(pObj))
    PageObjNum = pRef->GetRefObjNum();

  CPDF_Object* pKids = pDict->GetDirectObjectBy("K");
  if (!pKids)
    return;

  if (CPDF_Array* pArray = pKids->AsArray()) {
    m_Kids.SetSize(pArray->GetCount());
    for (uint32_t i = 0; i < pArray->GetCount(); i++) {
      CPDF_Object* pKid = pArray->GetDirectObjectAt(i);
      LoadKid(PageObjNum, pKid, &m_Kids[i]);
    }
  } else {
    m_Kids.SetSize(1);
    LoadKid(PageObjNum, pKids, &m_Kids[0]);
  }
}
void CPDF_StructElementImpl::LoadKid(uint32_t PageObjNum,
                                     CPDF_Object* pKidObj,
                                     CPDF_StructKid* pKid) {
  pKid->m_Type = CPDF_StructKid::Invalid;
  if (!pKidObj)
    return;

  if (pKidObj->IsNumber()) {
    if (m_pTree->m_pPage && m_pTree->m_pPage->GetObjNum() != PageObjNum) {
      return;
    }
    pKid->m_Type = CPDF_StructKid::PageContent;
    pKid->m_PageContent.m_ContentId = pKidObj->GetInteger();
    pKid->m_PageContent.m_PageObjNum = PageObjNum;
    return;
  }

  CPDF_Dictionary* pKidDict = pKidObj->AsDictionary();
  if (!pKidDict)
    return;

  if (CPDF_Reference* pRef = ToReference(pKidDict->GetObjectBy("Pg")))
    PageObjNum = pRef->GetRefObjNum();

  CFX_ByteString type = pKidDict->GetStringBy("Type");
  if (type == "MCR") {
    if (m_pTree->m_pPage && m_pTree->m_pPage->GetObjNum() != PageObjNum) {
      return;
    }
    pKid->m_Type = CPDF_StructKid::StreamContent;
    if (CPDF_Reference* pRef = ToReference(pKidDict->GetObjectBy("Stm"))) {
      pKid->m_StreamContent.m_RefObjNum = pRef->GetRefObjNum();
    } else {
      pKid->m_StreamContent.m_RefObjNum = 0;
    }
    pKid->m_StreamContent.m_PageObjNum = PageObjNum;
    pKid->m_StreamContent.m_ContentId = pKidDict->GetIntegerBy("MCID");
  } else if (type == "OBJR") {
    if (m_pTree->m_pPage && m_pTree->m_pPage->GetObjNum() != PageObjNum) {
      return;
    }
    pKid->m_Type = CPDF_StructKid::Object;
    if (CPDF_Reference* pObj = ToReference(pKidDict->GetObjectBy("Obj"))) {
      pKid->m_Object.m_RefObjNum = pObj->GetRefObjNum();
    } else {
      pKid->m_Object.m_RefObjNum = 0;
    }
    pKid->m_Object.m_PageObjNum = PageObjNum;
  } else {
    pKid->m_Type = CPDF_StructKid::Element;
    pKid->m_Element.m_pDict = pKidDict;
    if (!m_pTree->m_pPage) {
      pKid->m_Element.m_pElement =
          new CPDF_StructElementImpl(m_pTree, this, pKidDict);
    } else {
      pKid->m_Element.m_pElement = NULL;
    }
  }
}
static CPDF_Dictionary* FindAttrDict(CPDF_Object* pAttrs,
                                     const CFX_ByteStringC& owner,
                                     FX_FLOAT nLevel = 0.0F) {
  if (nLevel > nMaxRecursion)
    return nullptr;
  if (!pAttrs)
    return nullptr;

  CPDF_Dictionary* pDict = nullptr;
  if (pAttrs->IsDictionary()) {
    pDict = pAttrs->AsDictionary();
  } else if (CPDF_Stream* pStream = pAttrs->AsStream()) {
    pDict = pStream->GetDict();
  } else if (CPDF_Array* pArray = pAttrs->AsArray()) {
    for (uint32_t i = 0; i < pArray->GetCount(); i++) {
      CPDF_Object* pElement = pArray->GetDirectObjectAt(i);
      pDict = FindAttrDict(pElement, owner, nLevel + 1);
      if (pDict)
        return pDict;
    }
  }
  if (pDict && pDict->GetStringBy("O") == owner)
    return pDict;
  return nullptr;
}
CPDF_Object* CPDF_StructElementImpl::GetAttr(const CFX_ByteStringC& owner,
                                             const CFX_ByteStringC& name,
                                             FX_BOOL bInheritable,
                                             FX_FLOAT fLevel) {
  if (fLevel > nMaxRecursion) {
    return NULL;
  }
  if (bInheritable) {
    CPDF_Object* pAttr = GetAttr(owner, name, FALSE);
    if (pAttr) {
      return pAttr;
    }
    if (!m_pParent) {
      return NULL;
    }
    return m_pParent->GetAttr(owner, name, TRUE, fLevel + 1);
  }
  CPDF_Object* pA = m_pDict->GetDirectObjectBy("A");
  if (pA) {
    CPDF_Dictionary* pAttrDict = FindAttrDict(pA, owner);
    if (pAttrDict) {
      CPDF_Object* pAttr = pAttrDict->GetDirectObjectBy(name);
      if (pAttr) {
        return pAttr;
      }
    }
  }
  CPDF_Object* pC = m_pDict->GetDirectObjectBy("C");
  if (!pC)
    return nullptr;

  CPDF_Dictionary* pClassMap = m_pTree->m_pTreeRoot->GetDictBy("ClassMap");
  if (!pClassMap)
    return nullptr;

  if (CPDF_Array* pArray = pC->AsArray()) {
    for (uint32_t i = 0; i < pArray->GetCount(); i++) {
      CFX_ByteString class_name = pArray->GetStringAt(i);
      CPDF_Dictionary* pClassDict =
          pClassMap->GetDictBy(class_name.AsByteStringC());
      if (pClassDict && pClassDict->GetStringBy("O") == owner)
        return pClassDict->GetDirectObjectBy(name);
    }
    return nullptr;
  }
  CFX_ByteString class_name = pC->GetString();
  CPDF_Dictionary* pClassDict =
      pClassMap->GetDictBy(class_name.AsByteStringC());
  if (pClassDict && pClassDict->GetStringBy("O") == owner)
    return pClassDict->GetDirectObjectBy(name);
  return nullptr;
}
CPDF_Object* CPDF_StructElementImpl::GetAttr(const CFX_ByteStringC& owner,
                                             const CFX_ByteStringC& name,
                                             FX_BOOL bInheritable,
                                             int subindex) {
  CPDF_Object* pAttr = GetAttr(owner, name, bInheritable);
  CPDF_Array* pArray = ToArray(pAttr);
  if (!pArray || subindex == -1)
    return pAttr;

  if (subindex >= static_cast<int>(pArray->GetCount()))
    return pAttr;
  return pArray->GetDirectObjectAt(subindex);
}
CFX_ByteString CPDF_StructElementImpl::GetName(
    const CFX_ByteStringC& owner,
    const CFX_ByteStringC& name,
    const CFX_ByteStringC& default_value,
    FX_BOOL bInheritable,
    int subindex) {
  CPDF_Object* pAttr = GetAttr(owner, name, bInheritable, subindex);
  if (ToName(pAttr))
    return pAttr->GetString();
  return default_value;
}

FX_ARGB CPDF_StructElementImpl::GetColor(const CFX_ByteStringC& owner,
                                         const CFX_ByteStringC& name,
                                         FX_ARGB default_value,
                                         FX_BOOL bInheritable,
                                         int subindex) {
  CPDF_Array* pArray = ToArray(GetAttr(owner, name, bInheritable, subindex));
  if (!pArray)
    return default_value;
  return 0xff000000 | ((int)(pArray->GetNumberAt(0) * 255) << 16) |
         ((int)(pArray->GetNumberAt(1) * 255) << 8) |
         (int)(pArray->GetNumberAt(2) * 255);
}
FX_FLOAT CPDF_StructElementImpl::GetNumber(const CFX_ByteStringC& owner,
                                           const CFX_ByteStringC& name,
                                           FX_FLOAT default_value,
                                           FX_BOOL bInheritable,
                                           int subindex) {
  CPDF_Object* pAttr = GetAttr(owner, name, bInheritable, subindex);
  return ToNumber(pAttr) ? pAttr->GetNumber() : default_value;
}
int CPDF_StructElementImpl::GetInteger(const CFX_ByteStringC& owner,
                                       const CFX_ByteStringC& name,
                                       int default_value,
                                       FX_BOOL bInheritable,
                                       int subindex) {
  CPDF_Object* pAttr = GetAttr(owner, name, bInheritable, subindex);
  return ToNumber(pAttr) ? pAttr->GetInteger() : default_value;
}
