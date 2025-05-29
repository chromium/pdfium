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
#include "core/fxcrt/check.h"

CPDF_StructElement::Kid::Kid() = default;

CPDF_StructElement::Kid::Kid(const Kid& that) = default;

CPDF_StructElement::Kid::~Kid() = default;

CPDF_StructElement::CPDF_StructElement(const CPDF_StructTree* pTree,
                                       RetainPtr<const CPDF_Dictionary> dict)
    : tree_(pTree),
      dict_(std::move(dict)),
      type_(tree_->GetRoleMapNameFor(dict_->GetNameFor("S").AsStringView())) {
  LoadKids();
}

CPDF_StructElement::~CPDF_StructElement() {
  for (auto& kid : kids_) {
    if (kid.type_ == Kid::kElement && kid.element_) {
      kid.element_->SetParent(nullptr);
    }
  }
}

ByteString CPDF_StructElement::GetObjType() const {
  return dict_->GetByteStringFor("Type");
}

WideString CPDF_StructElement::GetAltText() const {
  return dict_->GetUnicodeTextFor("Alt");
}

WideString CPDF_StructElement::GetActualText() const {
  return dict_->GetUnicodeTextFor("ActualText");
}

WideString CPDF_StructElement::GetTitle() const {
  return dict_->GetUnicodeTextFor("T");
}

std::optional<WideString> CPDF_StructElement::GetID() const {
  RetainPtr<const CPDF_Object> obj = dict_->GetObjectFor("ID");
  if (!obj || !obj->IsString()) {
    return std::nullopt;
  }
  return obj->GetUnicodeText();
}

std::optional<WideString> CPDF_StructElement::GetLang() const {
  RetainPtr<const CPDF_Object> obj = dict_->GetObjectFor("Lang");
  if (!obj || !obj->IsString()) {
    return std::nullopt;
  }
  return obj->GetUnicodeText();
}

RetainPtr<const CPDF_Object> CPDF_StructElement::GetA() const {
  return dict_->GetObjectFor("A");
}

RetainPtr<const CPDF_Object> CPDF_StructElement::GetK() const {
  return dict_->GetObjectFor("K");
}

size_t CPDF_StructElement::CountKids() const {
  return kids_.size();
}

CPDF_StructElement* CPDF_StructElement::GetKidIfElement(size_t index) const {
  return kids_[index].type_ == Kid::kElement ? kids_[index].element_.Get()
                                             : nullptr;
}

int CPDF_StructElement::GetKidContentId(size_t index) const {
  return kids_[index].type_ == Kid::kStreamContent ||
                 kids_[index].type_ == Kid::kPageContent
             ? kids_[index].content_id_
             : -1;
}

bool CPDF_StructElement::UpdateKidIfElement(const CPDF_Dictionary* dict,
                                            CPDF_StructElement* pElement) {
  bool bSave = false;
  for (auto& kid : kids_) {
    if (kid.type_ == Kid::kElement && kid.dict_ == dict) {
      kid.element_.Reset(pElement);
      bSave = true;
    }
  }
  return bSave;
}

void CPDF_StructElement::LoadKids() {
  RetainPtr<const CPDF_Object> pObj = dict_->GetObjectFor("Pg");
  const CPDF_Reference* pRef = ToReference(pObj.Get());
  const uint32_t page_obj_num = pRef ? pRef->GetRefObjNum() : 0;
  RetainPtr<const CPDF_Object> pKids = dict_->GetDirectObjectFor("K");
  if (!pKids) {
    return;
  }

  DCHECK(kids_.empty());
  if (const CPDF_Array* pArray = pKids->AsArray()) {
    kids_.resize(pArray->size());
    for (size_t i = 0; i < pArray->size(); ++i) {
      LoadKid(page_obj_num, pArray->GetDirectObjectAt(i), kids_[i]);
    }
    return;
  }

  kids_.resize(1);
  LoadKid(page_obj_num, std::move(pKids), kids_[0]);
}

void CPDF_StructElement::LoadKid(uint32_t page_obj_num,
                                 RetainPtr<const CPDF_Object> pKidObj,
                                 Kid& kid) {
  if (!pKidObj) {
    return;
  }

  if (pKidObj->IsNumber()) {
    if (tree_->GetPageObjNum() != page_obj_num) {
      return;
    }

    kid.type_ = Kid::kPageContent;
    kid.content_id_ = pKidObj->GetInteger();
    kid.page_obj_num_ = page_obj_num;
    return;
  }

  const CPDF_Dictionary* pKidDict = pKidObj->AsDictionary();
  if (!pKidDict) {
    return;
  }

  if (RetainPtr<const CPDF_Reference> pRef =
          ToReference(pKidDict->GetObjectFor("Pg"))) {
    page_obj_num = pRef->GetRefObjNum();
  }
  ByteString type = pKidDict->GetNameFor("Type");
  if ((type == "MCR" || type == "OBJR") &&
      tree_->GetPageObjNum() != page_obj_num) {
    return;
  }

  if (type == "MCR") {
    kid.type_ = Kid::kStreamContent;
    RetainPtr<const CPDF_Reference> pRef =
        ToReference(pKidDict->GetObjectFor("Stm"));
    kid.ref_obj_num_ = pRef ? pRef->GetRefObjNum() : 0;
    kid.page_obj_num_ = page_obj_num;
    kid.content_id_ = pKidDict->GetIntegerFor("MCID");
    return;
  }

  if (type == "OBJR") {
    kid.type_ = Kid::kObject;
    RetainPtr<const CPDF_Reference> pObj =
        ToReference(pKidDict->GetObjectFor("Obj"));
    kid.ref_obj_num_ = pObj ? pObj->GetRefObjNum() : 0;
    kid.page_obj_num_ = page_obj_num;
    return;
  }

  kid.type_ = Kid::kElement;
  kid.dict_.Reset(pKidDict);
}
