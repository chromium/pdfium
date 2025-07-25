// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_structtree.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfdoc/cpdf_numbertree.h"
#include "core/fpdfdoc/cpdf_structelement.h"

namespace {

bool IsTagged(const CPDF_Document* doc) {
  RetainPtr<const CPDF_Dictionary> pMarkInfo =
      doc->GetRoot()->GetDictFor("MarkInfo");
  return pMarkInfo && pMarkInfo->GetIntegerFor("Marked");
}

}  // namespace

// static
std::unique_ptr<CPDF_StructTree> CPDF_StructTree::LoadPage(
    const CPDF_Document* doc,
    RetainPtr<const CPDF_Dictionary> pPageDict) {
  if (!IsTagged(doc)) {
    return nullptr;
  }

  auto pTree = std::make_unique<CPDF_StructTree>(doc);
  pTree->LoadPageTree(std::move(pPageDict));
  return pTree;
}

CPDF_StructTree::CPDF_StructTree(const CPDF_Document* doc)
    : tree_root_(doc->GetRoot()->GetDictFor("StructTreeRoot")),
      role_map_(tree_root_ ? tree_root_->GetDictFor("RoleMap") : nullptr) {}

CPDF_StructTree::~CPDF_StructTree() = default;

ByteString CPDF_StructTree::GetRoleMapNameFor(ByteStringView type) const {
  if (role_map_) {
    ByteString mapped = role_map_->GetNameFor(type);
    if (!mapped.IsEmpty()) {
      return mapped;
    }
  }
  return ByteString(type);
}

void CPDF_StructTree::LoadPageTree(RetainPtr<const CPDF_Dictionary> pPageDict) {
  page_ = std::move(pPageDict);
  if (!tree_root_) {
    return;
  }

  RetainPtr<const CPDF_Object> pKids = tree_root_->GetDirectObjectFor("K");
  if (!pKids) {
    return;
  }

  size_t kids_count;
  if (pKids->IsDictionary()) {
    kids_count = 1;
  } else if (const CPDF_Array* pArray = pKids->AsArray()) {
    kids_count = pArray->size();
  } else {
    return;
  }

  kids_.clear();
  kids_.resize(kids_count);

  RetainPtr<const CPDF_Dictionary> pParentTree =
      tree_root_->GetDictFor("ParentTree");
  if (!pParentTree) {
    return;
  }

  CPDF_NumberTree parent_tree(std::move(pParentTree));
  int parents_id = page_->GetIntegerFor("StructParents", -1);
  if (parents_id < 0) {
    return;
  }

  RetainPtr<const CPDF_Array> pParentArray =
      ToArray(parent_tree.LookupValue(parents_id));
  if (!pParentArray) {
    return;
  }

  StructElementMap element_map;
  for (size_t i = 0; i < pParentArray->size(); i++) {
    RetainPtr<const CPDF_Dictionary> pParent = pParentArray->GetDictAt(i);
    if (pParent) {
      AddPageNode(std::move(pParent), &element_map, 0);
    }
  }
}

RetainPtr<CPDF_StructElement> CPDF_StructTree::AddPageNode(
    RetainPtr<const CPDF_Dictionary> dict,
    StructElementMap* map,
    int nLevel) {
  static constexpr int kStructTreeMaxRecursion = 32;
  if (nLevel > kStructTreeMaxRecursion) {
    return nullptr;
  }

  auto it = map->find(dict);
  if (it != map->end()) {
    return it->second;
  }

  RetainPtr<const CPDF_Dictionary> key(dict);
  auto pElement = pdfium::MakeRetain<CPDF_StructElement>(this, dict);
  (*map)[key] = pElement;
  RetainPtr<const CPDF_Dictionary> pParent = dict->GetDictFor("P");
  if (!pParent || pParent->GetNameFor("Type") == "StructTreeRoot") {
    if (!AddTopLevelNode(dict, pElement)) {
      map->erase(key);
    }
    return pElement;
  }

  RetainPtr<CPDF_StructElement> pParentElement =
      AddPageNode(std::move(pParent), map, nLevel + 1);
  if (!pParentElement) {
    return pElement;
  }

  if (!pParentElement->UpdateKidIfElement(dict, pElement.Get())) {
    map->erase(key);
    return pElement;
  }

  pElement->SetParent(pParentElement.Get());
  return pElement;
}

bool CPDF_StructTree::AddTopLevelNode(
    const CPDF_Dictionary* dict,
    const RetainPtr<CPDF_StructElement>& pElement) {
  RetainPtr<const CPDF_Object> pObj = tree_root_->GetDirectObjectFor("K");
  if (!pObj) {
    return false;
  }

  if (pObj->IsDictionary()) {
    if (pObj->GetObjNum() != dict->GetObjNum()) {
      return false;
    }
    kids_[0] = pElement;
  }

  const CPDF_Array* pTopKids = pObj->AsArray();
  if (!pTopKids) {
    return true;
  }

  bool bSave = false;
  for (size_t i = 0; i < pTopKids->size(); i++) {
    RetainPtr<const CPDF_Reference> pKidRef =
        ToReference(pTopKids->GetObjectAt(i));
    if (pKidRef && pKidRef->GetRefObjNum() == dict->GetObjNum()) {
      kids_[i] = pElement;
      bSave = true;
    }
  }
  return bSave;
}
