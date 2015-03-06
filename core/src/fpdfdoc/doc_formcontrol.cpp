// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
CPDF_FormControl::CPDF_FormControl(CPDF_FormField* pField, CPDF_Dictionary* pWidgetDict)
{
    m_pField = pField;
    m_pWidgetDict = pWidgetDict;
    m_pForm = m_pField->m_pForm;
}
CFX_FloatRect CPDF_FormControl::GetRect()
{
    return m_pWidgetDict->GetRect("Rect");
}
CFX_ByteString CPDF_FormControl::GetOnStateName()
{
    ASSERT(GetType() == CPDF_FormField::CheckBox || GetType() == CPDF_FormField::RadioButton);
    CFX_ByteString csOn;
    CPDF_Dictionary* pAP = m_pWidgetDict->GetDict("AP");
    if (pAP == NULL) {
        return csOn;
    }
    CPDF_Dictionary* pN = pAP->GetDict("N");
    if (pN == NULL) {
        return csOn;
    }
    FX_POSITION pos = pN->GetStartPos();
    while (pos) {
        pN->GetNextElement(pos, csOn);
        if (csOn != "Off") {
            return csOn;
        }
    }
    return CFX_ByteString();
}
void CPDF_FormControl::SetOnStateName(const CFX_ByteString& csOn)
{
    ASSERT(GetType() == CPDF_FormField::CheckBox || GetType() == CPDF_FormField::RadioButton);
    CFX_ByteString csValue = csOn;
    if (csValue.IsEmpty()) {
        csValue = "Yes";
    }
    if (csValue == "Off") {
        csValue = "Yes";
    }
    CFX_ByteString csAS = m_pWidgetDict->GetString("AS", "Off");
    if (csAS != "Off") {
        m_pWidgetDict->SetAtName("AS", csValue);
    }
    CPDF_Dictionary* pAP = m_pWidgetDict->GetDict("AP");
    if (pAP == NULL) {
        return;
    }
    FX_POSITION pos1 = pAP->GetStartPos();
    while (pos1) {
        CFX_ByteString csKey1;
        CPDF_Object* pObj1 = pAP->GetNextElement(pos1, csKey1);
        if (pObj1 == NULL) {
            continue;
        }
        CPDF_Object* pObjDirect1 = pObj1->GetDirect();
        if (pObjDirect1->GetType() != PDFOBJ_DICTIONARY) {
            continue;
        }
        CPDF_Dictionary* pSubDict = (CPDF_Dictionary*)pObjDirect1;
        FX_POSITION pos2 = pSubDict->GetStartPos();
        while (pos2) {
            CFX_ByteString csKey2;
            CPDF_Object* pObj2 = pSubDict->GetNextElement(pos2, csKey2);
            if (pObj2 == NULL) {
                continue;
            }
            if (csKey2 != "Off") {
                pSubDict->ReplaceKey(csKey2, csValue);
                break;
            }
        }
    }
}
CFX_ByteString CPDF_FormControl::GetCheckedAPState()
{
    ASSERT(GetType() == CPDF_FormField::CheckBox || GetType() == CPDF_FormField::RadioButton);
    CFX_ByteString csOn = GetOnStateName();
    if (GetType() == CPDF_FormField::RadioButton || GetType() == CPDF_FormField::CheckBox) {
        CPDF_Object* pOpt = FPDF_GetFieldAttr(m_pField->m_pDict, "Opt");
        if (pOpt != NULL && pOpt->GetType() == PDFOBJ_ARRAY) {
            int iIndex = m_pField->GetControlIndex(this);
            csOn.Format("%d", iIndex);
        }
    }
    if (csOn.IsEmpty()) {
        csOn = "Yes";
    }
    return csOn;
}
CFX_WideString CPDF_FormControl::GetExportValue()
{
    ASSERT(GetType() == CPDF_FormField::CheckBox || GetType() == CPDF_FormField::RadioButton);
    CFX_ByteString csOn = GetOnStateName();
    if (GetType() == CPDF_FormField::RadioButton || GetType() == CPDF_FormField::CheckBox) {
        CPDF_Object* pOpt = FPDF_GetFieldAttr(m_pField->m_pDict, "Opt");
        if (pOpt != NULL && pOpt->GetType() == PDFOBJ_ARRAY) {
            int iIndex = m_pField->GetControlIndex(this);
            csOn = ((CPDF_Array*)pOpt)->GetString(iIndex);
        }
    }
    if (csOn.IsEmpty()) {
        csOn = "Yes";
    }
    CFX_WideString csWOn = PDF_DecodeText(csOn);
    return csWOn;
}
FX_BOOL CPDF_FormControl::IsChecked()
{
    ASSERT(GetType() == CPDF_FormField::CheckBox || GetType() == CPDF_FormField::RadioButton);
    CFX_ByteString csOn = GetOnStateName();
    CFX_ByteString csAS = m_pWidgetDict->GetString("AS");
    return csAS == csOn;
}
FX_BOOL CPDF_FormControl::IsDefaultChecked()
{
    ASSERT(GetType() == CPDF_FormField::CheckBox || GetType() == CPDF_FormField::RadioButton);
    CPDF_Object* pDV = FPDF_GetFieldAttr(m_pField->m_pDict, "DV");
    if (pDV == NULL) {
        return FALSE;
    }
    CFX_ByteString csDV = pDV->GetString();
    CFX_ByteString csOn = GetOnStateName();
    return (csDV == csOn);
}
void CPDF_FormControl::CheckControl(FX_BOOL bChecked)
{
    ASSERT(GetType() == CPDF_FormField::CheckBox || GetType() == CPDF_FormField::RadioButton);
    CFX_ByteString csOn = GetOnStateName();
    CFX_ByteString csOldAS = m_pWidgetDict->GetString("AS", "Off");
    CFX_ByteString csAS = "Off";
    if (bChecked) {
        csAS = csOn;
    }
    if (csOldAS == csAS) {
        return;
    }
    m_pWidgetDict->SetAtName("AS", csAS);
    m_pForm->m_bUpdated = TRUE;
}
CPDF_Stream* FPDFDOC_GetAnnotAP(CPDF_Dictionary* pAnnotDict, CPDF_Annot::AppearanceMode mode);
void CPDF_FormControl::DrawControl(CFX_RenderDevice* pDevice, CFX_AffineMatrix* pMatrix, CPDF_Page* pPage,
                                   CPDF_Annot::AppearanceMode mode, const CPDF_RenderOptions* pOptions)
{
    if (m_pWidgetDict->GetInteger("F") & ANNOTFLAG_HIDDEN) {
        return;
    }
    CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(m_pWidgetDict, mode);
    if (pStream == NULL) {
        return;
    }
    CFX_FloatRect form_bbox = pStream->GetDict()->GetRect("BBox");
    CFX_AffineMatrix form_matrix = pStream->GetDict()->GetMatrix("Matrix");
    form_matrix.TransformRect(form_bbox);
    CFX_FloatRect arect = m_pWidgetDict->GetRect("Rect");
    CFX_AffineMatrix matrix;
    matrix.MatchRect(arect, form_bbox);
    matrix.Concat(*pMatrix);
    CPDF_Form form(m_pField->m_pForm->m_pDocument, m_pField->m_pForm->m_pFormDict->GetDict("DR"), pStream);
    form.ParseContent(NULL, NULL, NULL, NULL);
    CPDF_RenderContext context;
    context.Create(pPage);
    context.DrawObjectList(pDevice, &form, &matrix, pOptions);
}
const FX_CHAR* g_sHighlightingMode[] = {"N", "I", "O", "P", "T", ""};
CPDF_FormControl::HighlightingMode CPDF_FormControl::GetHighlightingMode()
{
    if (m_pWidgetDict == NULL) {
        return Invert;
    }
    CFX_ByteString csH = m_pWidgetDict->GetString("H", "I");
    int i = 0;
    while (g_sHighlightingMode[i][0] != '\0') {
        if (csH.Equal(g_sHighlightingMode[i])) {
            return (HighlightingMode)i;
        }
        i ++;
    }
    return Invert;
}
CPDF_ApSettings CPDF_FormControl::GetMK(FX_BOOL bCreate)
{
    if (!m_pWidgetDict) {
        return NULL;
    }
    CPDF_ApSettings mk = m_pWidgetDict->GetDict(FX_BSTRC("MK"));
    if (!mk && bCreate) {
        mk = CPDF_Dictionary::Create();
        if (mk == NULL) {
            return NULL;
        }
        m_pWidgetDict->SetAt(FX_BSTRC("MK"), mk);
    }
    return mk;
}
FX_BOOL CPDF_FormControl::HasMKEntry(CFX_ByteString csEntry)
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.HasMKEntry(csEntry);
}
int CPDF_FormControl::GetRotation()
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.GetRotation();
}
FX_ARGB CPDF_FormControl::GetColor(int& iColorType, CFX_ByteString csEntry)
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.GetColor(iColorType, csEntry);
}
FX_FLOAT CPDF_FormControl::GetOriginalColor(int index, CFX_ByteString csEntry)
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.GetOriginalColor(index, csEntry);
}
void CPDF_FormControl::GetOriginalColor(int& iColorType, FX_FLOAT fc[4], CFX_ByteString csEntry)
{
    CPDF_ApSettings mk = GetMK(FALSE);
    mk.GetOriginalColor(iColorType, fc, csEntry);
}
CFX_WideString CPDF_FormControl::GetCaption(CFX_ByteString csEntry)
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.GetCaption(csEntry);
}
CPDF_Stream* CPDF_FormControl::GetIcon(CFX_ByteString csEntry)
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.GetIcon(csEntry);
}
CPDF_IconFit CPDF_FormControl::GetIconFit()
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.GetIconFit();
}
int CPDF_FormControl::GetTextPosition()
{
    CPDF_ApSettings mk = GetMK(FALSE);
    return mk.GetTextPosition();
}
CPDF_Action CPDF_FormControl::GetAction()
{
    if (!m_pWidgetDict) {
        return CPDF_Action();
    }
    if (m_pWidgetDict->KeyExist("A")) {
        return CPDF_Action(m_pWidgetDict->GetDict("A"));
    }
    CPDF_Object* pObj = FPDF_GetFieldAttr(m_pField->m_pDict, "A");
    if (!pObj) {
        return CPDF_Action();
    }
    return CPDF_Action(pObj->GetDict());
}
CPDF_AAction CPDF_FormControl::GetAdditionalAction()
{
    if (m_pWidgetDict == NULL) {
        return NULL;
    }
    if (m_pWidgetDict->KeyExist("AA")) {
        return m_pWidgetDict->GetDict("AA");
    } else {
        return m_pField->GetAdditionalAction();
    }
}
CPDF_DefaultAppearance CPDF_FormControl::GetDefaultAppearance()
{
    if (m_pWidgetDict == NULL) {
        return CFX_ByteString();
    }
    if (m_pWidgetDict->KeyExist("DA")) {
        return m_pWidgetDict->GetString("DA");
    } else {
        CPDF_Object* pObj = FPDF_GetFieldAttr(m_pField->m_pDict, "DA");
        if (pObj == NULL) {
            return m_pField->m_pForm->GetDefaultAppearance();
        }
        return pObj->GetString();
    }
}
CPDF_Font* CPDF_FormControl::GetDefaultControlFont()
{
    CPDF_DefaultAppearance cDA = GetDefaultAppearance();
    CFX_ByteString csFontNameTag;
    FX_FLOAT fFontSize;
    cDA.GetFont(csFontNameTag, fFontSize);
    if (csFontNameTag.IsEmpty()) {
        return NULL;
    }
    CPDF_Object* pObj = FPDF_GetFieldAttr(m_pWidgetDict, "DR");
    if (pObj != NULL && pObj->GetType() == PDFOBJ_DICTIONARY) {
        CPDF_Dictionary* pFonts = ((CPDF_Dictionary*)pObj)->GetDict("Font");
        if (pFonts != NULL) {
            CPDF_Dictionary *pElement = pFonts->GetDict(csFontNameTag);
            CPDF_Font *pFont = m_pField->m_pForm->m_pDocument->LoadFont(pElement);
            if (pFont != NULL) {
                return pFont;
            }
        }
    }
    CPDF_Font *pFont = m_pField->m_pForm->GetFormFont(csFontNameTag);
    if (pFont != NULL) {
        return pFont;
    }
    CPDF_Dictionary *pPageDict = m_pWidgetDict->GetDict("P");
    pObj = FPDF_GetFieldAttr(pPageDict, "Resources");
    if (pObj != NULL && pObj->GetType() == PDFOBJ_DICTIONARY) {
        CPDF_Dictionary* pFonts = ((CPDF_Dictionary*)pObj)->GetDict("Font");
        if (pFonts != NULL) {
            CPDF_Dictionary *pElement = pFonts->GetDict(csFontNameTag);
            CPDF_Font *pFont = m_pField->m_pForm->m_pDocument->LoadFont(pElement);
            if (pFont != NULL) {
                return pFont;
            }
        }
    }
    return NULL;
}
int CPDF_FormControl::GetControlAlignment()
{
    if (m_pWidgetDict == NULL) {
        return 0;
    }
    if (m_pWidgetDict->KeyExist("Q")) {
        return m_pWidgetDict->GetInteger("Q", 0);
    } else {
        CPDF_Object* pObj = FPDF_GetFieldAttr(m_pField->m_pDict, "Q");
        if (pObj == NULL) {
            return m_pField->m_pForm->GetFormAlignment();
        }
        return pObj->GetInteger();
    }
}
FX_BOOL CPDF_ApSettings::HasMKEntry(FX_BSTR csEntry)
{
    if (m_pDict == NULL) {
        return FALSE;
    }
    return m_pDict->KeyExist(csEntry);
}
int CPDF_ApSettings::GetRotation()
{
    if (m_pDict == NULL) {
        return 0;
    }
    return m_pDict->GetInteger(FX_BSTRC("R"));
}
FX_ARGB CPDF_ApSettings::GetColor(int& iColorType, FX_BSTR csEntry)
{
    iColorType = COLORTYPE_TRANSPARENT;
    if (m_pDict == NULL) {
        return 0;
    }
    FX_ARGB color = 0;
    CPDF_Array* pEntry = m_pDict->GetArray(csEntry);
    if (pEntry == NULL) {
        return color;
    }
    FX_DWORD dwCount = pEntry->GetCount();
    if (dwCount == 1) {
        iColorType = COLORTYPE_GRAY;
        FX_FLOAT g = pEntry->GetNumber(0) * 255;
        color = ArgbEncode(255, (int)g, (int)g, (int)g);
    } else if (dwCount == 3) {
        iColorType = COLORTYPE_RGB;
        FX_FLOAT r = pEntry->GetNumber(0) * 255;
        FX_FLOAT g = pEntry->GetNumber(1) * 255;
        FX_FLOAT b = pEntry->GetNumber(2) * 255;
        color = ArgbEncode(255, (int)r, (int)g, (int)b);
    } else if (dwCount == 4) {
        iColorType = COLORTYPE_CMYK;
        FX_FLOAT c = pEntry->GetNumber(0);
        FX_FLOAT m = pEntry->GetNumber(1);
        FX_FLOAT y = pEntry->GetNumber(2);
        FX_FLOAT k = pEntry->GetNumber(3);
        FX_FLOAT r = 1.0f - FX_MIN(1.0f, c + k);
        FX_FLOAT g = 1.0f - FX_MIN(1.0f, m + k);
        FX_FLOAT b = 1.0f - FX_MIN(1.0f, y + k);
        color = ArgbEncode(255, (int)(r * 255), (int)(g * 255), (int)(b * 255));
    }
    return color;
}
FX_FLOAT CPDF_ApSettings::GetOriginalColor(int index, FX_BSTR csEntry)
{
    if (m_pDict == NULL) {
        return 0;
    }
    CPDF_Array* pEntry = m_pDict->GetArray(csEntry);
    if (pEntry != NULL) {
        return pEntry->GetNumber(index);
    }
    return 0;
}
void CPDF_ApSettings::GetOriginalColor(int& iColorType, FX_FLOAT fc[4], FX_BSTR csEntry)
{
    iColorType = COLORTYPE_TRANSPARENT;
    for (int i = 0; i < 4; i ++) {
        fc[i] = 0;
    }
    if (m_pDict == NULL) {
        return;
    }
    CPDF_Array* pEntry = m_pDict->GetArray(csEntry);
    if (pEntry == NULL) {
        return;
    }
    FX_DWORD dwCount = pEntry->GetCount();
    if (dwCount == 1) {
        iColorType = COLORTYPE_GRAY;
        fc[0] = pEntry->GetNumber(0);
    } else if (dwCount == 3) {
        iColorType = COLORTYPE_RGB;
        fc[0] = pEntry->GetNumber(0);
        fc[1] = pEntry->GetNumber(1);
        fc[2] = pEntry->GetNumber(2);
    } else if (dwCount == 4) {
        iColorType = COLORTYPE_CMYK;
        fc[0] = pEntry->GetNumber(0);
        fc[1] = pEntry->GetNumber(1);
        fc[2] = pEntry->GetNumber(2);
        fc[3] = pEntry->GetNumber(3);
    }
}
CFX_WideString CPDF_ApSettings::GetCaption(FX_BSTR csEntry)
{
    CFX_WideString csCaption;
    if (m_pDict == NULL) {
        return csCaption;
    }
    return m_pDict->GetUnicodeText(csEntry);
}
CPDF_Stream* CPDF_ApSettings::GetIcon(FX_BSTR csEntry)
{
    if (m_pDict == NULL) {
        return NULL;
    }
    return m_pDict->GetStream(csEntry);
}
CPDF_IconFit CPDF_ApSettings::GetIconFit()
{
    if (m_pDict == NULL) {
        return NULL;
    }
    return m_pDict->GetDict(FX_BSTRC("IF"));
}
int CPDF_ApSettings::GetTextPosition()
{
    if (m_pDict == NULL) {
        return TEXTPOS_CAPTION;
    }
    return m_pDict->GetInteger(FX_BSTRC("TP"), TEXTPOS_CAPTION);
}
