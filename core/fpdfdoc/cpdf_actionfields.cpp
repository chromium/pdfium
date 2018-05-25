// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_actionfields.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfdoc/cpdf_action.h"

CPDF_ActionFields::CPDF_ActionFields(const CPDF_Action* pAction)
    : m_pAction(pAction) {}

CPDF_ActionFields::~CPDF_ActionFields() {}

size_t CPDF_ActionFields::GetFieldsCount() const {
  if (!m_pAction)
    return 0;

  const CPDF_Dictionary* pDict = m_pAction->GetDict();
  if (!pDict)
    return 0;

  ByteString csType = pDict->GetStringFor("S");
  const CPDF_Object* pFields;
  if (csType == "Hide")
    pFields = pDict->GetDirectObjectFor("T");
  else
    pFields = pDict->GetArrayFor("Fields");

  if (!pFields)
    return 0;
  if (pFields->IsDictionary())
    return 1;
  if (pFields->IsString())
    return 1;
  const CPDF_Array* pArray = pFields->AsArray();
  return pArray ? pArray->GetCount() : 0;
}

std::vector<const CPDF_Object*> CPDF_ActionFields::GetAllFields() const {
  std::vector<const CPDF_Object*> fields;
  if (!m_pAction)
    return fields;

  const CPDF_Dictionary* pDict = m_pAction->GetDict();
  if (!pDict)
    return fields;

  ByteString csType = pDict->GetStringFor("S");
  const CPDF_Object* pFields;
  if (csType == "Hide")
    pFields = pDict->GetDirectObjectFor("T");
  else
    pFields = pDict->GetArrayFor("Fields");

  if (!pFields)
    return fields;

  if (pFields->IsDictionary() || pFields->IsString()) {
    fields.push_back(pFields);
  } else if (const CPDF_Array* pArray = pFields->AsArray()) {
    for (size_t i = 0; i < pArray->GetCount(); ++i) {
      const CPDF_Object* pObj = pArray->GetDirectObjectAt(i);
      if (pObj)
        fields.push_back(pObj);
    }
  }
  return fields;
}

const CPDF_Object* CPDF_ActionFields::GetField(size_t iIndex) const {
  if (!m_pAction)
    return nullptr;

  const CPDF_Dictionary* pDict = m_pAction->GetDict();
  if (!pDict)
    return nullptr;

  ByteString csType = pDict->GetStringFor("S");
  const CPDF_Object* pFields;
  if (csType == "Hide")
    pFields = pDict->GetDirectObjectFor("T");
  else
    pFields = pDict->GetArrayFor("Fields");

  if (!pFields)
    return nullptr;

  const CPDF_Object* pFindObj = nullptr;
  if (pFields->IsDictionary() || pFields->IsString()) {
    if (iIndex == 0)
      pFindObj = pFields;
  } else if (const CPDF_Array* pArray = pFields->AsArray()) {
    pFindObj = pArray->GetDirectObjectAt(iIndex);
  }
  return pFindObj;
}
