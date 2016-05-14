// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_contentmarkdata.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_pageobject.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfdoc/include/fpdf_doc.h"

static int32_t FPDFDOC_OCG_FindGroup(const CPDF_Object* pObject,
                                     const CPDF_Dictionary* pGroupDict) {
  if (!pObject || !pGroupDict)
    return -1;

  if (const CPDF_Array* pArray = pObject->AsArray()) {
    for (size_t i = 0; i < pArray->GetCount(); i++) {
      if (pArray->GetDictAt(i) == pGroupDict)
        return i;
    }
    return -1;
  }
  return pObject->GetDict() == pGroupDict ? 0 : -1;
}
static FX_BOOL FPDFDOC_OCG_HasIntent(const CPDF_Dictionary* pDict,
                                     const CFX_ByteStringC& csElement,
                                     const CFX_ByteStringC& csDef = "") {
  CPDF_Object* pIntent = pDict->GetDirectObjectBy("Intent");
  if (!pIntent) {
    return csElement == csDef;
  }
  CFX_ByteString bsIntent;
  if (CPDF_Array* pArray = pIntent->AsArray()) {
    for (size_t i = 0; i < pArray->GetCount(); i++) {
      bsIntent = pArray->GetStringAt(i);
      if (bsIntent == "All" || bsIntent == csElement)
        return TRUE;
    }
    return FALSE;
  }
  bsIntent = pIntent->GetString();
  return bsIntent == "All" || bsIntent == csElement;
}

static CPDF_Dictionary* FPDFDOC_OCG_GetConfig(CPDF_Document* pDoc,
                                              const CPDF_Dictionary* pOCGDict) {
  ASSERT(pOCGDict);
  CPDF_Dictionary* pOCProperties = pDoc->GetRoot()->GetDictBy("OCProperties");
  if (!pOCProperties)
    return nullptr;

  CPDF_Array* pOCGs = pOCProperties->GetArrayBy("OCGs");
  if (!pOCGs)
    return nullptr;

  if (FPDFDOC_OCG_FindGroup(pOCGs, pOCGDict) < 0)
    return nullptr;

  CPDF_Dictionary* pConfig = pOCProperties->GetDictBy("D");
  CPDF_Array* pConfigs = pOCProperties->GetArrayBy("Configs");
  if (pConfigs) {
    CPDF_Dictionary* pFind;
    for (size_t i = 0; i < pConfigs->GetCount(); i++) {
      pFind = pConfigs->GetDictAt(i);
      if (!pFind) {
        continue;
      }
      if (!FPDFDOC_OCG_HasIntent(pFind, "View", "View")) {
        continue;
      }
      pConfig = pFind;
      break;
    }
  }
  return pConfig;
}
static CFX_ByteString FPDFDOC_OCG_GetUsageTypeString(
    CPDF_OCContext::UsageType eType) {
  CFX_ByteString csState = "View";
  if (eType == CPDF_OCContext::Design) {
    csState = "Design";
  } else if (eType == CPDF_OCContext::Print) {
    csState = "Print";
  } else if (eType == CPDF_OCContext::Export) {
    csState = "Export";
  }
  return csState;
}
CPDF_OCContext::CPDF_OCContext(CPDF_Document* pDoc, UsageType eUsageType) {
  ASSERT(pDoc);
  m_pDocument = pDoc;
  m_eUsageType = eUsageType;
}
CPDF_OCContext::~CPDF_OCContext() {
  m_OCGStates.clear();
}

FX_BOOL CPDF_OCContext::LoadOCGStateFromConfig(const CFX_ByteString& csConfig,
                                               const CPDF_Dictionary* pOCGDict,
                                               FX_BOOL& bValidConfig) const {
  CPDF_Dictionary* pConfig = FPDFDOC_OCG_GetConfig(m_pDocument, pOCGDict);
  if (!pConfig)
    return TRUE;

  bValidConfig = TRUE;
  FX_BOOL bState = pConfig->GetStringBy("BaseState", "ON") != "OFF";
  CPDF_Array* pArray = pConfig->GetArrayBy("ON");
  if (pArray) {
    if (FPDFDOC_OCG_FindGroup(pArray, pOCGDict) >= 0) {
      bState = TRUE;
    }
  }
  pArray = pConfig->GetArrayBy("OFF");
  if (pArray) {
    if (FPDFDOC_OCG_FindGroup(pArray, pOCGDict) >= 0) {
      bState = FALSE;
    }
  }
  pArray = pConfig->GetArrayBy("AS");
  if (pArray) {
    CFX_ByteString csFind = csConfig + "State";
    for (size_t i = 0; i < pArray->GetCount(); i++) {
      CPDF_Dictionary* pUsage = pArray->GetDictAt(i);
      if (!pUsage) {
        continue;
      }
      if (pUsage->GetStringBy("Event", "View") != csConfig) {
        continue;
      }
      CPDF_Array* pOCGs = pUsage->GetArrayBy("OCGs");
      if (!pOCGs) {
        continue;
      }
      if (FPDFDOC_OCG_FindGroup(pOCGs, pOCGDict) < 0) {
        continue;
      }
      CPDF_Dictionary* pState = pUsage->GetDictBy(csConfig);
      if (!pState) {
        continue;
      }
      bState = pState->GetStringBy(csFind) != "OFF";
    }
  }
  return bState;
}
FX_BOOL CPDF_OCContext::LoadOCGState(const CPDF_Dictionary* pOCGDict) const {
  if (!FPDFDOC_OCG_HasIntent(pOCGDict, "View", "View")) {
    return TRUE;
  }
  CFX_ByteString csState = FPDFDOC_OCG_GetUsageTypeString(m_eUsageType);
  CPDF_Dictionary* pUsage = pOCGDict->GetDictBy("Usage");
  if (pUsage) {
    CPDF_Dictionary* pState = pUsage->GetDictBy(csState);
    if (pState) {
      CFX_ByteString csFind = csState + "State";
      if (pState->KeyExist(csFind)) {
        return pState->GetStringBy(csFind) != "OFF";
      }
    }
    if (csState != "View") {
      pState = pUsage->GetDictBy("View");
      if (pState && pState->KeyExist("ViewState")) {
        return pState->GetStringBy("ViewState") != "OFF";
      }
    }
  }
  FX_BOOL bDefValid = FALSE;
  return LoadOCGStateFromConfig(csState, pOCGDict, bDefValid);
}

FX_BOOL CPDF_OCContext::GetOCGVisible(const CPDF_Dictionary* pOCGDict) {
  if (!pOCGDict)
    return FALSE;

  const auto it = m_OCGStates.find(pOCGDict);
  if (it != m_OCGStates.end())
    return it->second;

  FX_BOOL bState = LoadOCGState(pOCGDict);
  m_OCGStates[pOCGDict] = bState;
  return bState;
}

FX_BOOL CPDF_OCContext::CheckObjectVisible(const CPDF_PageObject* pObj) {
  const CPDF_ContentMarkData* pData = pObj->m_ContentMark;
  for (int i = 0; i < pData->CountItems(); i++) {
    const CPDF_ContentMarkItem& item = pData->GetItem(i);
    if (item.GetName() == "OC" &&
        item.GetParamType() == CPDF_ContentMarkItem::PropertiesDict &&
        !CheckOCGVisible(item.GetParam())) {
      return FALSE;
    }
  }
  return TRUE;
}

FX_BOOL CPDF_OCContext::GetOCGVE(CPDF_Array* pExpression,
                                 FX_BOOL bFromConfig,
                                 int nLevel) {
  if (nLevel > 32) {
    return FALSE;
  }
  if (!pExpression) {
    return FALSE;
  }
  CPDF_Object* pOCGObj;
  CFX_ByteString csOperator = pExpression->GetStringAt(0);
  if (csOperator == "Not") {
    pOCGObj = pExpression->GetDirectObjectAt(1);
    if (!pOCGObj)
      return FALSE;
    if (CPDF_Dictionary* pDict = pOCGObj->AsDictionary())
      return !(bFromConfig ? LoadOCGState(pDict) : GetOCGVisible(pDict));
    if (CPDF_Array* pArray = pOCGObj->AsArray())
      return !GetOCGVE(pArray, bFromConfig, nLevel + 1);
    return FALSE;
  }
  if (csOperator == "Or" || csOperator == "And") {
    FX_BOOL bValue = FALSE;
    for (size_t i = 1; i < pExpression->GetCount(); i++) {
      pOCGObj = pExpression->GetDirectObjectAt(1);
      if (!pOCGObj) {
        continue;
      }
      FX_BOOL bItem = FALSE;
      if (CPDF_Dictionary* pDict = pOCGObj->AsDictionary())
        bItem = bFromConfig ? LoadOCGState(pDict) : GetOCGVisible(pDict);
      else if (CPDF_Array* pArray = pOCGObj->AsArray())
        bItem = GetOCGVE(pArray, bFromConfig, nLevel + 1);

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
  return FALSE;
}
FX_BOOL CPDF_OCContext::LoadOCMDState(const CPDF_Dictionary* pOCMDDict,
                                      FX_BOOL bFromConfig) {
  CPDF_Array* pVE = pOCMDDict->GetArrayBy("VE");
  if (pVE) {
    return GetOCGVE(pVE, bFromConfig);
  }
  CFX_ByteString csP = pOCMDDict->GetStringBy("P", "AnyOn");
  CPDF_Object* pOCGObj = pOCMDDict->GetDirectObjectBy("OCGs");
  if (!pOCGObj)
    return TRUE;
  if (const CPDF_Dictionary* pDict = pOCGObj->AsDictionary())
    return bFromConfig ? LoadOCGState(pDict) : GetOCGVisible(pDict);

  CPDF_Array* pArray = pOCGObj->AsArray();
  if (!pArray)
    return TRUE;

  FX_BOOL bState = FALSE;
  if (csP == "AllOn" || csP == "AllOff") {
    bState = TRUE;
  }
  for (size_t i = 0; i < pArray->GetCount(); i++) {
    FX_BOOL bItem = TRUE;
    CPDF_Dictionary* pItemDict = pArray->GetDictAt(i);
    if (pItemDict)
      bItem = bFromConfig ? LoadOCGState(pItemDict) : GetOCGVisible(pItemDict);

    if ((csP == "AnyOn" && bItem) || (csP == "AnyOff" && !bItem))
      return TRUE;
    if ((csP == "AllOn" && !bItem) || (csP == "AllOff" && bItem))
      return FALSE;
  }
  return bState;
}
FX_BOOL CPDF_OCContext::CheckOCGVisible(const CPDF_Dictionary* pOCGDict) {
  if (!pOCGDict) {
    return TRUE;
  }
  CFX_ByteString csType = pOCGDict->GetStringBy("Type", "OCG");
  if (csType == "OCG") {
    return GetOCGVisible(pOCGDict);
  }
  return LoadOCMDState(pOCGDict, FALSE);
}
void CPDF_OCContext::ResetOCContext() {
  m_OCGStates.clear();
}
