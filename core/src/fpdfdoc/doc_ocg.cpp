// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfdoc/fpdf_doc.h"

static int32_t FPDFDOC_OCG_FindGroup(const CPDF_Object* pObject,
                                     const CPDF_Dictionary* pGroupDict) {
  if (!pObject || !pGroupDict)
    return -1;

  if (const CPDF_Array* pArray = pObject->AsArray()) {
    FX_DWORD dwCount = pArray->GetCount();
    for (FX_DWORD i = 0; i < dwCount; i++) {
      if (pArray->GetDict(i) == pGroupDict)
        return i;
    }
    return -1;
  }
  return pObject->GetDict() == pGroupDict ? 0 : -1;
}
static FX_BOOL FPDFDOC_OCG_HasIntent(const CPDF_Dictionary* pDict,
                                     const CFX_ByteStringC& csElement,
                                     const CFX_ByteStringC& csDef = "") {
  CPDF_Object* pIntent = pDict->GetElementValue("Intent");
  if (!pIntent) {
    return csElement == csDef;
  }
  CFX_ByteString bsIntent;
  if (CPDF_Array* pArray = pIntent->AsArray()) {
    FX_DWORD dwCount = pArray->GetCount();
    for (FX_DWORD i = 0; i < dwCount; i++) {
      bsIntent = pArray->GetString(i);
      if (bsIntent == "All" || bsIntent == csElement)
        return TRUE;
    }
    return FALSE;
  }
  bsIntent = pIntent->GetString();
  return bsIntent == "All" || bsIntent == csElement;
}
static CPDF_Dictionary* FPDFDOC_OCG_GetConfig(CPDF_Document* pDoc,
                                              const CPDF_Dictionary* pOCGDict,
                                              const CFX_ByteStringC& bsState) {
  FXSYS_assert(pDoc && pOCGDict);
  CPDF_Dictionary* pOCProperties = pDoc->GetRoot()->GetDict("OCProperties");
  if (!pOCProperties) {
    return NULL;
  }
  CPDF_Array* pOCGs = pOCProperties->GetArray("OCGs");
  if (!pOCGs) {
    return NULL;
  }
  if (FPDFDOC_OCG_FindGroup(pOCGs, pOCGDict) < 0) {
    return NULL;
  }
  CPDF_Dictionary* pConfig = pOCProperties->GetDict("D");
  CPDF_Array* pConfigs = pOCProperties->GetArray("Configs");
  if (pConfigs) {
    CPDF_Dictionary* pFind;
    int32_t iCount = pConfigs->GetCount();
    for (int32_t i = 0; i < iCount; i++) {
      pFind = pConfigs->GetDict(i);
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
  FXSYS_assert(pDoc);
  m_pDocument = pDoc;
  m_eUsageType = eUsageType;
}
CPDF_OCContext::~CPDF_OCContext() {
  m_OCGStates.clear();
}
FX_BOOL CPDF_OCContext::LoadOCGStateFromConfig(const CFX_ByteStringC& csConfig,
                                               const CPDF_Dictionary* pOCGDict,
                                               FX_BOOL& bValidConfig) const {
  CPDF_Dictionary* pConfig =
      FPDFDOC_OCG_GetConfig(m_pDocument, pOCGDict, csConfig);
  if (!pConfig) {
    return TRUE;
  }
  bValidConfig = TRUE;
  FX_BOOL bState = pConfig->GetString("BaseState", "ON") != "OFF";
  CPDF_Array* pArray = pConfig->GetArray("ON");
  if (pArray) {
    if (FPDFDOC_OCG_FindGroup(pArray, pOCGDict) >= 0) {
      bState = TRUE;
    }
  }
  pArray = pConfig->GetArray("OFF");
  if (pArray) {
    if (FPDFDOC_OCG_FindGroup(pArray, pOCGDict) >= 0) {
      bState = FALSE;
    }
  }
  pArray = pConfig->GetArray("AS");
  if (pArray) {
    CFX_ByteString csFind = csConfig + "State";
    int32_t iCount = pArray->GetCount();
    for (int32_t i = 0; i < iCount; i++) {
      CPDF_Dictionary* pUsage = pArray->GetDict(i);
      if (!pUsage) {
        continue;
      }
      if (pUsage->GetString("Event", "View") != csConfig) {
        continue;
      }
      CPDF_Array* pOCGs = pUsage->GetArray("OCGs");
      if (!pOCGs) {
        continue;
      }
      if (FPDFDOC_OCG_FindGroup(pOCGs, pOCGDict) < 0) {
        continue;
      }
      CPDF_Dictionary* pState = pUsage->GetDict(csConfig);
      if (!pState) {
        continue;
      }
      bState = pState->GetString(csFind) != "OFF";
    }
  }
  return bState;
}
FX_BOOL CPDF_OCContext::LoadOCGState(const CPDF_Dictionary* pOCGDict) const {
  if (!FPDFDOC_OCG_HasIntent(pOCGDict, "View", "View")) {
    return TRUE;
  }
  CFX_ByteString csState = FPDFDOC_OCG_GetUsageTypeString(m_eUsageType);
  CPDF_Dictionary* pUsage = pOCGDict->GetDict("Usage");
  if (pUsage) {
    CPDF_Dictionary* pState = pUsage->GetDict(csState);
    if (pState) {
      CFX_ByteString csFind = csState + "State";
      if (pState->KeyExist(csFind)) {
        return pState->GetString(csFind) != "OFF";
      }
    }
    if (csState != "View") {
      pState = pUsage->GetDict("View");
      if (pState && pState->KeyExist("ViewState")) {
        return pState->GetString("ViewState") != "OFF";
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

FX_BOOL CPDF_OCContext::GetOCGVE(CPDF_Array* pExpression,
                                 FX_BOOL bFromConfig,
                                 int nLevel) {
  if (nLevel > 32) {
    return FALSE;
  }
  if (!pExpression) {
    return FALSE;
  }
  int32_t iCount = pExpression->GetCount();
  CPDF_Object* pOCGObj;
  CFX_ByteString csOperator = pExpression->GetString(0);
  if (csOperator == "Not") {
    pOCGObj = pExpression->GetElementValue(1);
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
    for (int32_t i = 1; i < iCount; i++) {
      pOCGObj = pExpression->GetElementValue(1);
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
  CPDF_Array* pVE = pOCMDDict->GetArray("VE");
  if (pVE) {
    return GetOCGVE(pVE, bFromConfig);
  }
  CFX_ByteString csP = pOCMDDict->GetString("P", "AnyOn");
  CPDF_Object* pOCGObj = pOCMDDict->GetElementValue("OCGs");
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
  int32_t iCount = pArray->GetCount();
  for (int32_t i = 0; i < iCount; i++) {
    FX_BOOL bItem = TRUE;
    CPDF_Dictionary* pItemDict = pArray->GetDict(i);
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
  CFX_ByteString csType = pOCGDict->GetString("Type", "OCG");
  if (csType == "OCG") {
    return GetOCGVisible(pOCGDict);
  }
  return LoadOCMDState(pOCGDict, FALSE);
}
void CPDF_OCContext::ResetOCContext() {
  m_OCGStates.clear();
}
