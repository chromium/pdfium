// Copyright 2017 The PDFium Authors
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
#include "third_party/base/check.h"

CPDF_StructElement::Kid::Kid() = default;

CPDF_StructElement::Kid::Kid(const Kid& that) = default;

CPDF_StructElement::Kid::~Kid() = default;

CPDF_StructElement::CPDF_StructElement(const CPDF_StructTree* pTree,
                                       RetainPtr<const CPDF_Dictionary> pDict)
    : m_pTree(pTree),
      m_pDict(std::move(pDict)),
      m_Type(m_pTree->GetRoleMapNameFor(m_pDict->GetNameFor("S"))) {
  LoadKids(m_pDict);
}

CPDF_StructElement::~CPDF_StructElement() {
  for (auto& kid : m_Kids) {
    if (kid.m_Type == Kid::kElement && kid.m_pElement) {
      kid.m_pElement->SetParent(nullptr);
    }
  }
}

ByteString CPDF_StructElement::GetObjType() const {
  return m_pDict->GetByteStringFor("Type");
}

WideString CPDF_StructElement::GetAltText() const {
  return m_pDict->GetUnicodeTextFor("Alt");
}

WideString CPDF_StructElement::GetActualText() const {
  return m_pDict->GetUnicodeTextFor("ActualText");
}

WideString CPDF_StructElement::GetTitle() const {
  return m_pDict->GetUnicodeTextFor("T");
}

absl::optional<WideString> CPDF_StructElement::GetID() const {
  RetainPtr<const CPDF_Object> obj = m_pDict->GetObjectFor("ID");
  if (!obj || !obj->IsString())
    return absl::nullopt;
  return obj->GetUnicodeText();
}

absl::optional<WideString> CPDF_StructElement::GetLang() const {
  RetainPtr<const CPDF_Object> obj = m_pDict->GetObjectFor("Lang");
  if (!obj || !obj->IsString())
    return absl::nullopt;
  return obj->GetUnicodeText();
}

RetainPtr<const CPDF_Object> CPDF_StructElement::GetA() const {
  return m_pDict->GetObjectFor("A");
}

RetainPtr<const CPDF_Object> CPDF_StructElement::GetK() const {
  return m_pDict->GetObjectFor("K");
}

size_t CPDF_StructElement::CountKids() const {
  return m_Kids.size();
}

CPDF_StructElement* CPDF_StructElement::GetKidIfElement(size_t index) const {
  return m_Kids[index].m_Type == Kid::kElement ? m_Kids[index].m_pElement.Get()
                                               : nullptr;
}

bool CPDF_StructElement::UpdateKidIfElement(const CPDF_Dictionary* pDict,
                                            CPDF_StructElement* pElement) {
  bool bSave = false;
  for (auto& kid : m_Kids) {
    if (kid.m_Type == Kid::kElement && kid.m_pDict == pDict) {
      kid.m_pElement.Reset(pElement);
      bSave = true;
    }
  }
  return bSave;
}

void CPDF_StructElement::LoadKids(RetainPtr<const CPDF_Dictionary> pDict) {
  RetainPtr<const CPDF_Object> pObj = pDict->GetObjectFor("Pg");
  const CPDF_Reference* pRef = ToReference(pObj.Get());
  const uint32_t PageObjNum = pRef ? pRef->GetRefObjNum() : 0;
  RetainPtr<const CPDF_Object> pKids = pDict->GetDirectObjectFor("K");
  if (!pKids)
    return;

  DCHECK(m_Kids.empty());
  if (const CPDF_Array* pArray = pKids->AsArray()) {
    m_Kids.resize(pArray->size());
    for (size_t i = 0; i < pArray->size(); ++i) {
      LoadKid(PageObjNum, pArray->GetDirectObjectAt(i), &m_Kids[i]);
    }
    return;
  }

  m_Kids.resize(1);
  LoadKid(PageObjNum, std::move(pKids), &m_Kids[0]);
}

void CPDF_StructElement::LoadKid(uint32_t PageObjNum,
                                 RetainPtr<const CPDF_Object> pKidObj,
                                 Kid* pKid) {
  if (!pKidObj)
    return;

  if (pKidObj->IsNumber()) {
    if (m_pTree->GetPageObjNum() != PageObjNum)
      return;

    pKid->m_Type = Kid::kPageContent;
    pKid->m_ContentId = pKidObj->GetInteger();
    pKid->m_PageObjNum = PageObjNum;
    return;
  }

  const CPDF_Dictionary* pKidDict = pKidObj->AsDictionary();
  if (!pKidDict)
    return;

  if (RetainPtr<const CPDF_Reference> pRef =
          ToReference(pKidDict->GetObjectFor("Pg"))) {
    PageObjNum = pRef->GetRefObjNum();
  }
  ByteString type = pKidDict->GetNameFor("Type");
  if ((type == "MCR" || type == "OBJR") &&
      m_pTree->GetPageObjNum() != PageObjNum) {
    return;
  }

  if (type == "MCR") {
    pKid->m_Type = Kid::kStreamContent;
    RetainPtr<const CPDF_Reference> pRef =
        ToReference(pKidDict->GetObjectFor("Stm"));
    pKid->m_RefObjNum = pRef ? pRef->GetRefObjNum() : 0;
    pKid->m_PageObjNum = PageObjNum;
    pKid->m_ContentId = pKidDict->GetIntegerFor("MCID");
    return;
  }

  if (type == "OBJR") {
    pKid->m_Type = Kid::kObject;
    RetainPtr<const CPDF_Reference> pObj =
        ToReference(pKidDict->GetObjectFor("Obj"));
    pKid->m_RefObjNum = pObj ? pObj->GetRefObjNum() : 0;
    pKid->m_PageObjNum = PageObjNum;
    return;
  }

  pKid->m_Type = Kid::kElement;
  pKid->m_pDict.Reset(pKidDict);
}
