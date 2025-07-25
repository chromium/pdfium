// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_reference.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_stream.h"

CPDF_Reference::CPDF_Reference(CPDF_IndirectObjectHolder* doc, uint32_t objnum)
    : obj_list_(doc), ref_obj_num_(objnum) {}

CPDF_Reference::~CPDF_Reference() = default;

CPDF_Object::Type CPDF_Reference::GetType() const {
  return kReference;
}

ByteString CPDF_Reference::GetString() const {
  const CPDF_Object* obj = FastGetDirect();
  return obj ? obj->GetString() : ByteString();
}

float CPDF_Reference::GetNumber() const {
  const CPDF_Object* obj = FastGetDirect();
  return obj ? obj->GetNumber() : 0;
}

int CPDF_Reference::GetInteger() const {
  const CPDF_Object* obj = FastGetDirect();
  return obj ? obj->GetInteger() : 0;
}

const CPDF_Dictionary* CPDF_Reference::GetDictInternal() const {
  const CPDF_Object* obj = FastGetDirect();
  return obj ? obj->GetDictInternal() : nullptr;
}

CPDF_Reference* CPDF_Reference::AsMutableReference() {
  return this;
}

RetainPtr<CPDF_Object> CPDF_Reference::Clone() const {
  return CloneObjectNonCyclic(false);
}

RetainPtr<CPDF_Object> CPDF_Reference::CloneNonCyclic(
    bool bDirect,
    std::set<const CPDF_Object*>* pVisited) const {
  pVisited->insert(this);
  if (!bDirect) {
    return pdfium::MakeRetain<CPDF_Reference>(obj_list_, ref_obj_num_);
  }
  RetainPtr<const CPDF_Object> pDirect = GetDirect();
  return pDirect && !pdfium::Contains(*pVisited, pDirect.Get())
             ? pDirect->CloneNonCyclic(true, pVisited)
             : nullptr;
}

const CPDF_Object* CPDF_Reference::FastGetDirect() const {
  if (!obj_list_) {
    return nullptr;
  }
  const CPDF_Object* obj =
      obj_list_->GetOrParseIndirectObjectInternal(ref_obj_num_);
  return (obj && !obj->IsReference()) ? obj : nullptr;
}

void CPDF_Reference::SetRef(CPDF_IndirectObjectHolder* doc, uint32_t objnum) {
  obj_list_ = doc;
  ref_obj_num_ = objnum;
}

const CPDF_Object* CPDF_Reference::GetDirectInternal() const {
  return obj_list_ ? obj_list_->GetOrParseIndirectObjectInternal(ref_obj_num_)
                   : nullptr;
}

bool CPDF_Reference::WriteTo(IFX_ArchiveStream* archive,
                             const CPDF_Encryptor* encryptor) const {
  return archive->WriteString(" ") && archive->WriteDWord(GetRefObjNum()) &&
         archive->WriteString(" 0 R ");
}

RetainPtr<CPDF_Reference> CPDF_Reference::MakeReference(
    CPDF_IndirectObjectHolder* holder) const {
  DCHECK_EQ(holder, obj_list_);
  // Do not allow reference to reference, just create other reference for same
  // object.
  return pdfium::MakeRetain<CPDF_Reference>(holder, GetRefObjNum());
}
