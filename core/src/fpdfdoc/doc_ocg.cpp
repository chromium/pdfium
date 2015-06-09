// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
static int32_t FPDFDOC_OCG_FindGroup(const CPDF_Object *pObject, const CPDF_Dictionary *pGroupDict)
{
    if (pObject == NULL || pGroupDict == NULL) {
        return -1;
    }
    int32_t iType = pObject->GetType();
    if (iType == PDFOBJ_ARRAY) {
        FX_DWORD dwCount = ((CPDF_Array*)pObject)->GetCount();
        for (FX_DWORD i = 0; i < dwCount; i++) {
            if (((CPDF_Array*)pObject)->GetDict(i) == pGroupDict) {
                return i;
            }
        }
        return -1;
    }
    if (pObject->GetDict() == pGroupDict) {
        return 0;
    }
    return -1;
}
static FX_BOOL FPDFDOC_OCG_HasIntent(const CPDF_Dictionary *pDict, FX_BSTR csElement, FX_BSTR csDef = FX_BSTRC(""))
{
    FXSYS_assert(pDict != NULL);
    CPDF_Object *pIntent = pDict->GetElementValue(FX_BSTRC("Intent"));
    if (pIntent == NULL) {
        return csElement == csDef;
    }
    CFX_ByteString bsIntent;
    if (pIntent->GetType() == PDFOBJ_ARRAY) {
        FX_DWORD dwCount = ((CPDF_Array*)pIntent)->GetCount();
        for (FX_DWORD i = 0; i < dwCount; i++) {
            bsIntent = ((CPDF_Array*)pIntent)->GetString(i);
            if (bsIntent == FX_BSTRC("All") || bsIntent == csElement) {
                return TRUE;
            }
        }
        return FALSE;
    }
    bsIntent = pIntent->GetString();
    return bsIntent == FX_BSTRC("All") || bsIntent == csElement;
}
static CPDF_Dictionary* FPDFDOC_OCG_GetConfig(CPDF_Document *pDoc, const CPDF_Dictionary *pOCGDict, FX_BSTR bsState)
{
    FXSYS_assert(pDoc && pOCGDict);
    CPDF_Dictionary *pOCProperties = pDoc->GetRoot()->GetDict(FX_BSTRC("OCProperties"));
    if (!pOCProperties) {
        return NULL;
    }
    CPDF_Array *pOCGs = pOCProperties->GetArray(FX_BSTRC("OCGs"));
    if (!pOCGs) {
        return NULL;
    }
    if (FPDFDOC_OCG_FindGroup(pOCGs, pOCGDict) < 0) {
        return NULL;
    }
    CPDF_Dictionary *pConfig = pOCProperties->GetDict(FX_BSTRC("D"));
    CPDF_Array *pConfigs = pOCProperties->GetArray(FX_BSTRC("Configs"));
    if (pConfigs) {
        CPDF_Dictionary *pFind;
        int32_t iCount = pConfigs->GetCount();
        for (int32_t i = 0; i < iCount; i ++) {
            pFind = pConfigs->GetDict(i);
            if (!pFind) {
                continue;
            }
            if (!FPDFDOC_OCG_HasIntent(pFind, FX_BSTRC("View"), FX_BSTRC("View"))) {
                continue;
            }
            pConfig = pFind;
            break;
        }
    }
    return pConfig;
}
static CFX_ByteString FPDFDOC_OCG_GetUsageTypeString(CPDF_OCContext::UsageType eType)
{
    CFX_ByteString csState = FX_BSTRC("View");
    if (eType == CPDF_OCContext::Design) {
        csState = FX_BSTRC("Design");
    } else if (eType == CPDF_OCContext::Print) {
        csState = FX_BSTRC("Print");
    } else if (eType == CPDF_OCContext::Export) {
        csState = FX_BSTRC("Export");
    }
    return csState;
}
CPDF_OCContext::CPDF_OCContext(CPDF_Document *pDoc, UsageType eUsageType)
{
    FXSYS_assert(pDoc != NULL);
    m_pDocument = pDoc;
    m_eUsageType = eUsageType;
}
CPDF_OCContext::~CPDF_OCContext()
{
    m_OCGStates.RemoveAll();
}
FX_BOOL CPDF_OCContext::LoadOCGStateFromConfig(FX_BSTR csConfig, const CPDF_Dictionary *pOCGDict, FX_BOOL &bValidConfig) const
{
    CPDF_Dictionary *pConfig = FPDFDOC_OCG_GetConfig(m_pDocument, pOCGDict, csConfig);
    if (!pConfig) {
        return TRUE;
    }
    bValidConfig = TRUE;
    FX_BOOL bState = pConfig->GetString(FX_BSTRC("BaseState"), FX_BSTRC("ON")) != FX_BSTRC("OFF");
    CPDF_Array *pArray = pConfig->GetArray(FX_BSTRC("ON"));
    if (pArray) {
        if (FPDFDOC_OCG_FindGroup(pArray, pOCGDict) >= 0) {
            bState = TRUE;
        }
    }
    pArray = pConfig->GetArray(FX_BSTRC("OFF"));
    if (pArray) {
        if (FPDFDOC_OCG_FindGroup(pArray, pOCGDict) >= 0) {
            bState = FALSE;
        }
    }
    pArray = pConfig->GetArray(FX_BSTRC("AS"));
    if (pArray) {
        CFX_ByteString csFind = csConfig + FX_BSTRC("State");
        int32_t iCount = pArray->GetCount();
        for (int32_t i = 0; i < iCount; i ++) {
            CPDF_Dictionary *pUsage = pArray->GetDict(i);
            if (!pUsage) {
                continue;
            }
            if (pUsage->GetString(FX_BSTRC("Event"), FX_BSTRC("View")) != csConfig) {
                continue;
            }
            CPDF_Array *pOCGs = pUsage->GetArray(FX_BSTRC("OCGs"));
            if (!pOCGs) {
                continue;
            }
            if (FPDFDOC_OCG_FindGroup(pOCGs, pOCGDict) < 0) {
                continue;
            }
            CPDF_Dictionary *pState = pUsage->GetDict(csConfig);
            if (!pState) {
                continue;
            }
            bState = pState->GetString(csFind) != FX_BSTRC("OFF");
        }
    }
    return bState;
}
FX_BOOL CPDF_OCContext::LoadOCGState(const CPDF_Dictionary *pOCGDict) const
{
    if (!FPDFDOC_OCG_HasIntent(pOCGDict, FX_BSTRC("View"), FX_BSTRC("View"))) {
        return TRUE;
    }
    CFX_ByteString csState = FPDFDOC_OCG_GetUsageTypeString(m_eUsageType);
    CPDF_Dictionary *pUsage = pOCGDict->GetDict(FX_BSTRC("Usage"));
    if (pUsage) {
        CPDF_Dictionary *pState = pUsage->GetDict(csState);
        if (pState) {
            CFX_ByteString csFind = csState + FX_BSTRC("State");
            if (pState->KeyExist(csFind)) {
                return pState->GetString(csFind) != FX_BSTRC("OFF");
            }
        }
        if (csState != FX_BSTRC("View")) {
            pState = pUsage->GetDict(FX_BSTRC("View"));
            if (pState && pState->KeyExist(FX_BSTRC("ViewState"))) {
                return pState->GetString(FX_BSTRC("ViewState")) != FX_BSTRC("OFF");
            }
        }
    }
    FX_BOOL bDefValid = FALSE;
    return LoadOCGStateFromConfig(csState, pOCGDict, bDefValid);
}
FX_BOOL CPDF_OCContext::GetOCGVisible(const CPDF_Dictionary *pOCGDict)
{
    if (!pOCGDict) {
        return FALSE;
    }
    FX_LPVOID bState = NULL;
    if (m_OCGStates.Lookup(pOCGDict, bState)) {
        return (uintptr_t)bState != 0;
    }
    bState = (FX_LPVOID)(uintptr_t)LoadOCGState(pOCGDict);
    m_OCGStates.SetAt(pOCGDict, bState);
    return (uintptr_t)bState != 0;
}
FX_BOOL CPDF_OCContext::GetOCGVE(CPDF_Array *pExpression, FX_BOOL bFromConfig, int nLevel)
{
    if (nLevel > 32) {
        return FALSE;
    }
    if (pExpression == NULL) {
        return FALSE;
    }
    int32_t iCount = pExpression->GetCount();
    CPDF_Object *pOCGObj;
    CFX_ByteString csOperator = pExpression->GetString(0);
    if (csOperator == FX_BSTRC("Not")) {
        pOCGObj = pExpression->GetElementValue(1);
        if (pOCGObj == NULL) {
            return FALSE;
        }
        if (pOCGObj->GetType() == PDFOBJ_DICTIONARY) {
            return !(bFromConfig ? LoadOCGState((CPDF_Dictionary*)pOCGObj) : GetOCGVisible((CPDF_Dictionary*)pOCGObj));
        } else if (pOCGObj->GetType() == PDFOBJ_ARRAY) {
            return !GetOCGVE((CPDF_Array*)pOCGObj, bFromConfig, nLevel + 1);
        } else {
            return FALSE;
        }
    }
    if (csOperator == FX_BSTRC("Or") || csOperator == FX_BSTRC("And")) {
        FX_BOOL bValue = FALSE;
        for (int32_t i = 1; i < iCount; i ++) {
            pOCGObj = pExpression->GetElementValue(1);
            if (pOCGObj == NULL) {
                continue;
            }
            FX_BOOL bItem = FALSE;
            if (pOCGObj->GetType() == PDFOBJ_DICTIONARY) {
                bItem = bFromConfig ? LoadOCGState((CPDF_Dictionary*)pOCGObj) : GetOCGVisible((CPDF_Dictionary*)pOCGObj);
            } else if (pOCGObj->GetType() == PDFOBJ_ARRAY) {
                bItem = GetOCGVE((CPDF_Array*)pOCGObj, bFromConfig, nLevel + 1);
            }
            if (i == 1) {
                bValue = bItem;
            } else {
                if (csOperator == FX_BSTRC("Or")) {
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
FX_BOOL CPDF_OCContext::LoadOCMDState(const CPDF_Dictionary *pOCMDDict, FX_BOOL bFromConfig)
{
    FXSYS_assert(pOCMDDict != NULL);
    CPDF_Array *pVE = pOCMDDict->GetArray(FX_BSTRC("VE"));
    if (pVE != NULL) {
        return GetOCGVE(pVE, bFromConfig);
    }
    CFX_ByteString csP = pOCMDDict->GetString(FX_BSTRC("P"), FX_BSTRC("AnyOn"));
    CPDF_Object *pOCGObj = pOCMDDict->GetElementValue(FX_BSTRC("OCGs"));
    if (pOCGObj == NULL) {
        return TRUE;
    }
    if (pOCGObj->GetType() == PDFOBJ_DICTIONARY) {
        return bFromConfig ? LoadOCGState((CPDF_Dictionary*)pOCGObj) : GetOCGVisible((CPDF_Dictionary*)pOCGObj);
    }
    if (pOCGObj->GetType() != PDFOBJ_ARRAY) {
        return TRUE;
    }
    FX_BOOL bState = FALSE;
    if (csP == FX_BSTRC("AllOn") || csP == FX_BSTRC("AllOff")) {
        bState = TRUE;
    }
    int32_t iCount = ((CPDF_Array*)pOCGObj)->GetCount();
    for (int32_t i = 0; i < iCount; i ++) {
        FX_BOOL bItem = TRUE;
        CPDF_Dictionary* pItemDict = ((CPDF_Array*)pOCGObj)->GetDict(i);
        if (pItemDict) {
            bItem = bFromConfig ? LoadOCGState(pItemDict) : GetOCGVisible(pItemDict);
        }
        if (csP == FX_BSTRC("AnyOn") && bItem) {
            return TRUE;
        }
        if (csP == FX_BSTRC("AnyOff") && !bItem) {
            return TRUE;
        }
        if (csP == FX_BSTRC("AllOn") && !bItem) {
            return FALSE;
        }
        if (csP == FX_BSTRC("AllOff") && bItem) {
            return FALSE;
        }
    }
    return bState;
}
FX_BOOL CPDF_OCContext::CheckOCGVisible(const CPDF_Dictionary *pOCGDict)
{
    if (pOCGDict == NULL) {
        return TRUE;
    }
    CFX_ByteString csType = pOCGDict->GetString(FX_BSTRC("Type"), FX_BSTRC("OCG"));
    if (csType == FX_BSTRC("OCG")) {
        return GetOCGVisible(pOCGDict);
    } else {
        return LoadOCMDState(pOCGDict, FALSE);
    }
}
void CPDF_OCContext::ResetOCContext()
{
    m_OCGStates.RemoveAll();
}
