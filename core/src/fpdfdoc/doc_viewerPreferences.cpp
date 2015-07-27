// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
CPDF_ViewerPreferences::CPDF_ViewerPreferences(CPDF_Document *pDoc): m_pDoc(pDoc)
{
}
CPDF_ViewerPreferences::~CPDF_ViewerPreferences()
{
}
FX_BOOL CPDF_ViewerPreferences::IsDirectionR2L() const
{
    CPDF_Dictionary *pDict = m_pDoc->GetRoot();
    pDict = pDict->GetDict(FX_BSTRC("ViewerPreferences"));
    if (!pDict)	{
        return FALSE;
    }
    return FX_BSTRC("R2L") == pDict->GetString(FX_BSTRC("Direction"));
}
FX_BOOL CPDF_ViewerPreferences::PrintScaling() const
{
    CPDF_Dictionary *pDict = m_pDoc->GetRoot();
    pDict = pDict->GetDict(FX_BSTRC("ViewerPreferences"));
    if (!pDict)	{
        return TRUE;
    }
    return FX_BSTRC("None") != pDict->GetString(FX_BSTRC("PrintScaling"));
}
int32_t CPDF_ViewerPreferences::NumCopies() const
{
    CPDF_Dictionary *pDict = m_pDoc->GetRoot();
    pDict = pDict->GetDict(FX_BSTRC("ViewerPreferences"));
    if (!pDict) {
        return 1;
    }
    return pDict->GetInteger(FX_BSTRC("NumCopies"));
}
CPDF_Array* CPDF_ViewerPreferences::PrintPageRange() const
{
    CPDF_Dictionary *pDict = m_pDoc->GetRoot();
    CPDF_Array *pRange = NULL;
    pDict = pDict->GetDict(FX_BSTRC("ViewerPreferences"));
    if (!pDict) {
        return pRange;
    }
    pRange = pDict->GetArray(FX_BSTRC("PrintPageRange"));
    return pRange;
}
CFX_ByteString CPDF_ViewerPreferences::Duplex() const
{
    CPDF_Dictionary *pDict = m_pDoc->GetRoot();
    pDict = pDict->GetDict(FX_BSTRC("ViewerPreferences"));
    if (!pDict) {
        return FX_BSTRC("None");
    }
    return pDict->GetString(FX_BSTRC("Duplex"));
}
