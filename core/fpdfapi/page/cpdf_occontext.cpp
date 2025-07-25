// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_occontext.h"

#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/check.h"

namespace {

bool HasIntent(const CPDF_Dictionary* dict,
               ByteStringView csElement,
               ByteStringView csDef) {
  RetainPtr<const CPDF_Object> pIntent = dict->GetDirectObjectFor("Intent");
  if (!pIntent) {
    return csElement == csDef;
  }

  ByteString bsIntent;
  if (const CPDF_Array* pArray = pIntent->AsArray()) {
    for (size_t i = 0; i < pArray->size(); i++) {
      bsIntent = pArray->GetByteStringAt(i);
      if (bsIntent == "All" || bsIntent == csElement) {
        return true;
      }
    }
    return false;
  }
  bsIntent = pIntent->GetString();
  return bsIntent == "All" || bsIntent == csElement;
}

RetainPtr<const CPDF_Dictionary> GetConfig(CPDF_Document* doc,
                                           const CPDF_Dictionary* pOCGDict) {
  DCHECK(pOCGDict);
  RetainPtr<const CPDF_Dictionary> pOCProperties =
      doc->GetRoot()->GetDictFor("OCProperties");
  if (!pOCProperties) {
    return nullptr;
  }

  RetainPtr<const CPDF_Array> pOCGs = pOCProperties->GetArrayFor("OCGs");
  if (!pOCGs) {
    return nullptr;
  }

  if (!pOCGs->Contains(pOCGDict)) {
    return nullptr;
  }

  RetainPtr<const CPDF_Dictionary> pConfig = pOCProperties->GetDictFor("D");
  RetainPtr<const CPDF_Array> pConfigArray =
      pOCProperties->GetArrayFor("Configs");
  if (!pConfigArray) {
    return pConfig;
  }

  for (size_t i = 0; i < pConfigArray->size(); i++) {
    RetainPtr<const CPDF_Dictionary> pFind = pConfigArray->GetDictAt(i);
    if (pFind && HasIntent(pFind.Get(), "View", "")) {
      return pFind;
    }
  }
  return pConfig;
}

ByteStringView GetUsageTypeString(CPDF_OCContext::UsageType eType) {
  switch (eType) {
    case CPDF_OCContext::kDesign:
      return "Design";
    case CPDF_OCContext::kPrint:
      return "Print";
    case CPDF_OCContext::kExport:
      return "Export";
    default:
      return "View";
  }
}

}  // namespace

CPDF_OCContext::CPDF_OCContext(CPDF_Document* doc, UsageType eUsageType)
    : document_(doc), usage_type_(eUsageType) {
  DCHECK(doc);
}

CPDF_OCContext::~CPDF_OCContext() = default;

bool CPDF_OCContext::LoadOCGStateFromConfig(
    ByteStringView config,
    const CPDF_Dictionary* pOCGDict) const {
  RetainPtr<const CPDF_Dictionary> pConfig = GetConfig(document_, pOCGDict);
  if (!pConfig) {
    return true;
  }

  bool bState = pConfig->GetByteStringFor("BaseState", "ON") != "OFF";
  RetainPtr<const CPDF_Array> pArray = pConfig->GetArrayFor("ON");
  if (pArray && pArray->Contains(pOCGDict)) {
    bState = true;
  }

  pArray = pConfig->GetArrayFor("OFF");
  if (pArray && pArray->Contains(pOCGDict)) {
    bState = false;
  }

  pArray = pConfig->GetArrayFor("AS");
  if (!pArray) {
    return bState;
  }

  ByteString csFind({config, "State"});
  for (size_t i = 0; i < pArray->size(); i++) {
    RetainPtr<const CPDF_Dictionary> pUsage = pArray->GetDictAt(i);
    if (!pUsage) {
      continue;
    }

    if (pUsage->GetByteStringFor("Event", "View") != config) {
      continue;
    }

    RetainPtr<const CPDF_Array> pOCGs = pUsage->GetArrayFor("OCGs");
    if (!pOCGs) {
      continue;
    }

    if (!pOCGs->Contains(pOCGDict)) {
      continue;
    }

    RetainPtr<const CPDF_Dictionary> pState = pUsage->GetDictFor(config);
    if (!pState) {
      continue;
    }

    bState = pState->GetByteStringFor(csFind.AsStringView()) != "OFF";
  }
  return bState;
}

bool CPDF_OCContext::LoadOCGState(const CPDF_Dictionary* pOCGDict) const {
  if (!HasIntent(pOCGDict, "View", "View")) {
    return true;
  }

  ByteStringView state = GetUsageTypeString(usage_type_);
  RetainPtr<const CPDF_Dictionary> pUsage = pOCGDict->GetDictFor("Usage");
  if (pUsage) {
    RetainPtr<const CPDF_Dictionary> pState = pUsage->GetDictFor(state);
    if (pState) {
      ByteString csFind({state, "State"});
      if (pState->KeyExist(csFind.AsStringView())) {
        return pState->GetByteStringFor(csFind.AsStringView()) != "OFF";
      }
    }
    if (state != "View") {
      pState = pUsage->GetDictFor("View");
      if (pState && pState->KeyExist("ViewState")) {
        return pState->GetByteStringFor("ViewState") != "OFF";
      }
    }
  }
  return LoadOCGStateFromConfig(state, pOCGDict);
}

bool CPDF_OCContext::GetOCGVisible(const CPDF_Dictionary* pOCGDict) const {
  if (!pOCGDict) {
    return false;
  }

  const auto it = ogcstate_cache_.find(pOCGDict);
  if (it != ogcstate_cache_.end()) {
    return it->second;
  }

  bool bState = LoadOCGState(pOCGDict);
  ogcstate_cache_[pdfium::WrapRetain(pOCGDict)] = bState;
  return bState;
}

bool CPDF_OCContext::CheckPageObjectVisible(const CPDF_PageObject* pObj) const {
  const CPDF_ContentMarks* pMarks = pObj->GetContentMarks();
  for (size_t i = 0; i < pMarks->CountItems(); ++i) {
    const CPDF_ContentMarkItem* item = pMarks->GetItem(i);
    if (item->GetName() == "OC" &&
        item->GetParamType() == CPDF_ContentMarkItem::kPropertiesDict &&
        !CheckOCGDictVisible(item->GetParam().Get())) {
      return false;
    }
  }
  return true;
}

bool CPDF_OCContext::GetOCGVE(const CPDF_Array* pExpression, int nLevel) const {
  if (nLevel > 32 || !pExpression) {
    return false;
  }

  ByteString csOperator = pExpression->GetByteStringAt(0);
  if (csOperator == "Not") {
    RetainPtr<const CPDF_Object> pOCGObj = pExpression->GetDirectObjectAt(1);
    if (!pOCGObj) {
      return false;
    }
    if (const CPDF_Dictionary* dict = pOCGObj->AsDictionary()) {
      return !GetOCGVisible(dict);
    }
    if (const CPDF_Array* pArray = pOCGObj->AsArray()) {
      return !GetOCGVE(pArray, nLevel + 1);
    }
    return false;
  }

  if (csOperator != "Or" && csOperator != "And") {
    return false;
  }

  bool bValue = false;
  for (size_t i = 1; i < pExpression->size(); i++) {
    RetainPtr<const CPDF_Object> pOCGObj = pExpression->GetDirectObjectAt(i);
    if (!pOCGObj) {
      continue;
    }

    bool bItem = false;
    if (const CPDF_Dictionary* dict = pOCGObj->AsDictionary()) {
      bItem = GetOCGVisible(dict);
    } else if (const CPDF_Array* pArray = pOCGObj->AsArray()) {
      bItem = GetOCGVE(pArray, nLevel + 1);
    }

    if (i == 1) {
      bValue = bItem;
    } else {
      if (csOperator == "Or") {
        bValue = bValue || bItem;
      } else {
        bValue = bValue && bItem;
      }
    }
  }
  return bValue;
}

bool CPDF_OCContext::LoadOCMDState(const CPDF_Dictionary* pOCMDDict) const {
  RetainPtr<const CPDF_Array> pVE = pOCMDDict->GetArrayFor("VE");
  if (pVE) {
    return GetOCGVE(pVE.Get(), 0);
  }

  ByteString csP = pOCMDDict->GetByteStringFor("P", "AnyOn");
  RetainPtr<const CPDF_Object> pOCGObj = pOCMDDict->GetDirectObjectFor("OCGs");
  if (!pOCGObj) {
    return true;
  }

  if (const CPDF_Dictionary* dict = pOCGObj->AsDictionary()) {
    return GetOCGVisible(dict);
  }

  const CPDF_Array* pArray = pOCGObj->AsArray();
  if (!pArray) {
    return true;
  }

  bool bState = (csP == "AllOn" || csP == "AllOff");
  // At least one entry of OCGs needs to be a valid dictionary for it to be
  // considered present. See "OCGs" in table 4.49 in the PDF 1.7 spec.
  bool bValidEntrySeen = false;
  for (size_t i = 0; i < pArray->size(); i++) {
    bool bItem = true;
    RetainPtr<const CPDF_Dictionary> pItemDict = pArray->GetDictAt(i);
    if (!pItemDict) {
      continue;
    }

    bValidEntrySeen = true;
    bItem = GetOCGVisible(pItemDict.Get());

    if ((csP == "AnyOn" && bItem) || (csP == "AnyOff" && !bItem)) {
      return true;
    }
    if ((csP == "AllOn" && !bItem) || (csP == "AllOff" && bItem)) {
      return false;
    }
  }

  return !bValidEntrySeen || bState;
}

bool CPDF_OCContext::CheckOCGDictVisible(
    const CPDF_Dictionary* pOCGDict) const {
  if (!pOCGDict) {
    return true;
  }

  ByteString csType = pOCGDict->GetByteStringFor("Type", "OCG");
  if (csType == "OCG") {
    return GetOCGVisible(pOCGDict);
  }
  return LoadOCMDState(pOCGDict);
}
