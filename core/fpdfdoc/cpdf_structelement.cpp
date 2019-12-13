// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_structelement.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfdoc/cpdf_structtree.h"

namespace {

ByteString GetStructElementType(CPDF_StructTree* pTree,
                                const CPDF_Dictionary* pDict) {
  ByteString type = pDict->GetStringFor("S");
  if (pTree->GetRoleMap()) {
    ByteString mapped = pTree->GetRoleMap()->GetStringFor(type);
    if (!mapped.IsEmpty())
      type = std::move(mapped);
  }
  return type;
}

}  // namespace

CPDF_StructKid::CPDF_StructKid() = default;

CPDF_StructKid::CPDF_StructKid(const CPDF_StructKid& that) = default;

CPDF_StructKid::~CPDF_StructKid() = default;

CPDF_StructElement::CPDF_StructElement(CPDF_StructTree* pTree,
                                       CPDF_StructElement* pParent,
                                       const CPDF_Dictionary* pDict)
    : m_pTree(pTree),
      m_pParent(pParent),
      m_pDict(pDict),
      m_Type(GetStructElementType(m_pTree.Get(), m_pDict.Get())) {
  LoadKids(m_pDict.Get());
}

CPDF_StructElement::~CPDF_StructElement() = default;

WideString CPDF_StructElement::GetAltText() const {
  return GetDict()->GetUnicodeTextFor("Alt");
}

WideString CPDF_StructElement::GetTitle() const {
  return GetDict()->GetUnicodeTextFor("T");
}

size_t CPDF_StructElement::CountKids() const {
  return m_Kids.size();
}

CPDF_StructElement* CPDF_StructElement::GetKidIfElement(size_t index) const {
  return m_Kids[index].m_Type == CPDF_StructKid::kElement
             ? m_Kids[index].m_pElement.Get()
             : nullptr;
}

void CPDF_StructElement::LoadKids(const CPDF_Dictionary* pDict) {
  const CPDF_Object* pObj = pDict->GetObjectFor("Pg");
  uint32_t PageObjNum = 0;
  if (const CPDF_Reference* pRef = ToReference(pObj))
    PageObjNum = pRef->GetRefObjNum();

  const CPDF_Object* pKids = pDict->GetDirectObjectFor("K");
  if (!pKids)
    return;

  m_Kids.clear();
  if (const CPDF_Array* pArray = pKids->AsArray()) {
    m_Kids.resize(pArray->size());
    for (uint32_t i = 0; i < pArray->size(); i++) {
      const CPDF_Object* pKid = pArray->GetDirectObjectAt(i);
      LoadKid(PageObjNum, pKid, &m_Kids[i]);
    }
    return;
  }

  m_Kids.resize(1);
  LoadKid(PageObjNum, pKids, &m_Kids[0]);
}

void CPDF_StructElement::LoadKid(uint32_t PageObjNum,
                                 const CPDF_Object* pKidObj,
                                 CPDF_StructKid* pKid) {
  pKid->m_Type = CPDF_StructKid::kInvalid;
  if (!pKidObj)
    return;

  if (pKidObj->IsNumber()) {
    if (m_pTree->GetPage()->GetObjNum() != PageObjNum)
      return;

    pKid->m_Type = CPDF_StructKid::kPageContent;
    pKid->m_ContentId = pKidObj->GetInteger();
    pKid->m_PageObjNum = PageObjNum;
    return;
  }

  const CPDF_Dictionary* pKidDict = pKidObj->AsDictionary();
  if (!pKidDict)
    return;
  if (const CPDF_Reference* pRef = ToReference(pKidDict->GetObjectFor("Pg")))
    PageObjNum = pRef->GetRefObjNum();

  ByteString type = pKidDict->GetStringFor("Type");
  if ((type == "MCR" || type == "OBJR") &&
      m_pTree->GetPage()->GetObjNum() != PageObjNum) {
    return;
  }

  if (type == "MCR") {
    pKid->m_Type = CPDF_StructKid::kStreamContent;
    const CPDF_Reference* pRef = ToReference(pKidDict->GetObjectFor("Stm"));
    pKid->m_RefObjNum = pRef ? pRef->GetRefObjNum() : 0;
    pKid->m_PageObjNum = PageObjNum;
    pKid->m_ContentId = pKidDict->GetIntegerFor("MCID");
    return;
  }

  if (type == "OBJR") {
    pKid->m_Type = CPDF_StructKid::kObject;
    const CPDF_Reference* pObj = ToReference(pKidDict->GetObjectFor("Obj"));
    pKid->m_RefObjNum = pObj ? pObj->GetRefObjNum() : 0;
    pKid->m_PageObjNum = PageObjNum;
    return;
  }

  pKid->m_Type = CPDF_StructKid::kElement;
  pKid->m_pDict.Reset(pKidDict);
  pKid->m_pElement = nullptr;
}
