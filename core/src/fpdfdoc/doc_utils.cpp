// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
#include "doc_utils.h"

static const int FPDFDOC_UTILS_MAXRECURSION = 32;

CFX_WideString GetFullName(CPDF_Dictionary* pFieldDict)
{
    CFX_WideString full_name;
    CPDF_Dictionary* pLevel = pFieldDict;
    while (pLevel) {
        CFX_WideString short_name = pLevel->GetUnicodeText("T");
        if (short_name != L"") {
            if (full_name == L"") {
                full_name = short_name;
            } else {
                full_name = short_name + L"." + full_name;
            }
        }
        pLevel = pLevel->GetDict("Parent");
    }
    return full_name;
}
FX_BOOL CPDF_DefaultAppearance::HasFont()
{
    if (m_csDA.IsEmpty()) {
        return FALSE;
    }
    CPDF_SimpleParser syntax(m_csDA);
    return syntax.FindTagParam("Tf", 2);
}
CFX_ByteString CPDF_DefaultAppearance::GetFontString()
{
    CFX_ByteString csFont;
    if (m_csDA.IsEmpty()) {
        return csFont;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam("Tf", 2)) {
        csFont += (CFX_ByteString)syntax.GetWord();
        csFont += " ";
        csFont += (CFX_ByteString)syntax.GetWord();
        csFont += " ";
        csFont += (CFX_ByteString)syntax.GetWord();
    }
    return csFont;
}
void CPDF_DefaultAppearance::GetFont(CFX_ByteString& csFontNameTag, FX_FLOAT& fFontSize)
{
    csFontNameTag = "";
    fFontSize = 0;
    if (m_csDA.IsEmpty()) {
        return;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam("Tf", 2)) {
        csFontNameTag = (CFX_ByteString)syntax.GetWord();
        csFontNameTag.Delete(0, 1);
        fFontSize = FX_atof((CFX_ByteString)syntax.GetWord());
    }
    csFontNameTag = PDF_NameDecode(csFontNameTag);
}
FX_BOOL CPDF_DefaultAppearance::HasColor(FX_BOOL bStrokingOperation)
{
    if (m_csDA.IsEmpty()) {
        return FALSE;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam(bStrokingOperation ? "G" : "g", 1)) {
        return TRUE;
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam(bStrokingOperation ? "RG" : "rg", 3)) {
        return TRUE;
    }
    syntax.SetPos(0);
    return syntax.FindTagParam(bStrokingOperation ? "K" : "k", 4);
}
CFX_ByteString CPDF_DefaultAppearance::GetColorString(FX_BOOL bStrokingOperation)
{
    CFX_ByteString csColor;
    if (m_csDA.IsEmpty()) {
        return csColor;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam(bStrokingOperation ? "G" : "g", 1)) {
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
        return csColor;
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam(bStrokingOperation ? "RG" : "rg", 3)) {
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
        return csColor;
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam(bStrokingOperation ? "K" : "k", 4)) {
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
        csColor += " ";
        csColor += (CFX_ByteString)syntax.GetWord();
    }
    return csColor;
}
void CPDF_DefaultAppearance::GetColor(int& iColorType, FX_FLOAT fc[4], FX_BOOL bStrokingOperation)
{
    iColorType = COLORTYPE_TRANSPARENT;
    for (int c = 0; c < 4; c ++) {
        fc[c] = 0;
    }
    if (m_csDA.IsEmpty()) {
        return;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam(bStrokingOperation ? "G" : "g", 1)) {
        iColorType = COLORTYPE_GRAY;
        fc[0] = FX_atof((CFX_ByteString)syntax.GetWord());
        return;
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam(bStrokingOperation ? "RG" : "rg", 3)) {
        iColorType = COLORTYPE_RGB;
        fc[0] = FX_atof((CFX_ByteString)syntax.GetWord());
        fc[1] = FX_atof((CFX_ByteString)syntax.GetWord());
        fc[2] = FX_atof((CFX_ByteString)syntax.GetWord());
        return;
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam(bStrokingOperation ? "K" : "k", 4)) {
        iColorType = COLORTYPE_CMYK;
        fc[0] = FX_atof((CFX_ByteString)syntax.GetWord());
        fc[1] = FX_atof((CFX_ByteString)syntax.GetWord());
        fc[2] = FX_atof((CFX_ByteString)syntax.GetWord());
        fc[3] = FX_atof((CFX_ByteString)syntax.GetWord());
    }
}
void CPDF_DefaultAppearance::GetColor(FX_ARGB& color, int& iColorType, FX_BOOL bStrokingOperation)
{
    color = 0;
    iColorType = COLORTYPE_TRANSPARENT;
    if (m_csDA.IsEmpty()) {
        return;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam(bStrokingOperation ? "G" : "g", 1)) {
        iColorType = COLORTYPE_GRAY;
        FX_FLOAT g = FX_atof((CFX_ByteString)syntax.GetWord()) * 255 + 0.5f;
        color = ArgbEncode(255, (int)g, (int)g, (int)g);
        return;
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam(bStrokingOperation ? "RG" : "rg", 3)) {
        iColorType = COLORTYPE_RGB;
        FX_FLOAT r = FX_atof((CFX_ByteString)syntax.GetWord()) * 255 + 0.5f;
        FX_FLOAT g = FX_atof((CFX_ByteString)syntax.GetWord()) * 255 + 0.5f;
        FX_FLOAT b = FX_atof((CFX_ByteString)syntax.GetWord()) * 255 + 0.5f;
        color = ArgbEncode(255, (int)r, (int)g, (int)b);
        return;
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam(bStrokingOperation ? "K" : "k", 4)) {
        iColorType = COLORTYPE_CMYK;
        FX_FLOAT c = FX_atof((CFX_ByteString)syntax.GetWord());
        FX_FLOAT m = FX_atof((CFX_ByteString)syntax.GetWord());
        FX_FLOAT y = FX_atof((CFX_ByteString)syntax.GetWord());
        FX_FLOAT k = FX_atof((CFX_ByteString)syntax.GetWord());
        FX_FLOAT r = 1.0f - FX_MIN(1.0f, c + k);
        FX_FLOAT g = 1.0f - FX_MIN(1.0f, m + k);
        FX_FLOAT b = 1.0f - FX_MIN(1.0f, y + k);
        color = ArgbEncode(255, (int)(r * 255 + 0.5f), (int)(g * 255 + 0.5f), (int)(b * 255 + 0.5f));
    }
}
FX_BOOL CPDF_DefaultAppearance::HasTextMatrix()
{
    if (m_csDA.IsEmpty()) {
        return FALSE;
    }
    CPDF_SimpleParser syntax(m_csDA);
    return syntax.FindTagParam("Tm", 6);
}
CFX_ByteString CPDF_DefaultAppearance::GetTextMatrixString()
{
    CFX_ByteString csTM;
    if (m_csDA.IsEmpty()) {
        return csTM;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam("Tm", 6)) {
        for (int i = 0; i < 6; i ++) {
            csTM += (CFX_ByteString)syntax.GetWord();
            csTM += " ";
        }
        csTM += (CFX_ByteString)syntax.GetWord();
    }
    return csTM;
}
CFX_AffineMatrix CPDF_DefaultAppearance::GetTextMatrix()
{
    CFX_AffineMatrix tm;
    if (m_csDA.IsEmpty()) {
        return tm;
    }
    CPDF_SimpleParser syntax(m_csDA);
    if (syntax.FindTagParam("Tm", 6)) {
        FX_FLOAT f[6];
        for (int i = 0; i < 6; i ++) {
            f[i] = FX_atof((CFX_ByteString)syntax.GetWord());
        }
        tm.Set(f[0], f[1], f[2], f[3], f[4], f[5]);
    }
    return tm;
}
void InitInterFormDict(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument)
{
    if (pDocument == NULL) {
        return;
    }
    if (pFormDict == NULL) {
        pFormDict = CPDF_Dictionary::Create();
        if (pFormDict == NULL) {
            return;
        }
        FX_DWORD dwObjNum = pDocument->AddIndirectObject(pFormDict);
        CPDF_Dictionary* pRoot = pDocument->GetRoot();
        pRoot->SetAtReference("AcroForm", pDocument, dwObjNum);
    }
    CFX_ByteString csDA;
    if (!pFormDict->KeyExist("DR")) {
        CPDF_Font* pFont = NULL;
        CFX_ByteString csBaseName, csDefault;
        FX_BYTE charSet = CPDF_InterForm::GetNativeCharSet();
        pFont = CPDF_InterForm::AddStandardFont(pDocument, "Helvetica");
        if (pFont != NULL) {
            AddInterFormFont(pFormDict, pDocument, pFont, csBaseName);
            csDefault = csBaseName;
        }
        if (charSet != 0) {
            CFX_ByteString csFontName = CPDF_InterForm::GetNativeFont(charSet, NULL);
            if (pFont == NULL || csFontName != "Helvetica") {
                pFont = CPDF_InterForm::AddNativeFont(pDocument);
                if (pFont != NULL) {
                    csBaseName = "";
                    AddInterFormFont(pFormDict, pDocument, pFont, csBaseName);
                    csDefault = csBaseName;
                }
            }
        }
        if (pFont != NULL) {
            csDA = "/" + PDF_NameEncode(csDefault) + " 0 Tf";
        }
    }
    if (!csDA.IsEmpty()) {
        csDA += " ";
    }
    csDA += "0 g";
    if (!pFormDict->KeyExist("DA")) {
        pFormDict->SetAtString("DA", csDA);
    }
}
FX_DWORD CountInterFormFonts(CPDF_Dictionary* pFormDict)
{
    if (pFormDict == NULL) {
        return 0;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return 0;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return 0;
    }
    FX_DWORD dwCount = 0;
    FX_POSITION pos = pFonts->GetStartPos();
    while (pos) {
        CPDF_Object* pObj = NULL;
        CFX_ByteString csKey;
        pObj = pFonts->GetNextElement(pos, csKey);
        if (pObj == NULL) {
            continue;
        }
        CPDF_Object* pDirect = pObj->GetDirect();
        if (pDirect != NULL && pDirect->GetType() == PDFOBJ_DICTIONARY) {
            if (((CPDF_Dictionary*)pDirect)->GetString("Type") == "Font") {
                dwCount ++;
            }
        }
    }
    return dwCount;
}
CPDF_Font* GetInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, FX_DWORD index, CFX_ByteString& csNameTag)
{
    if (pFormDict == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return NULL;
    }
    FX_DWORD dwCount = 0;
    FX_POSITION pos = pFonts->GetStartPos();
    while (pos) {
        CPDF_Object* pObj = NULL;
        CFX_ByteString csKey;
        pObj = pFonts->GetNextElement(pos, csKey);
        if (pObj == NULL) {
            continue;
        }
        CPDF_Object* pDirect = pObj->GetDirect();
        if (pDirect == NULL || pDirect->GetType() != PDFOBJ_DICTIONARY) {
            continue;
        }
        CPDF_Dictionary* pElement = (CPDF_Dictionary*)pDirect;
        if (pElement->GetString("Type") != "Font") {
            continue;
        }
        if (dwCount == index) {
            csNameTag = csKey;
            return pDocument->LoadFont(pElement);
        }
        dwCount ++;
    }
    return NULL;
}
CPDF_Font* GetInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString csNameTag)
{
    CFX_ByteString csAlias = PDF_NameDecode(csNameTag);
    if (pFormDict == NULL || csAlias.IsEmpty()) {
        return NULL;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pElement = pFonts->GetDict(csAlias);
    if (pElement == NULL) {
        return NULL;
    }
    if (pElement->GetString("Type") == "Font") {
        return pDocument->LoadFont(pElement);
    }
    return NULL;
}
CPDF_Font* GetInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString csFontName, CFX_ByteString& csNameTag)
{
    if (pFormDict == NULL || csFontName.IsEmpty()) {
        return NULL;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return NULL;
    }
    FX_POSITION pos = pFonts->GetStartPos();
    while (pos) {
        CPDF_Object* pObj = NULL;
        CFX_ByteString csKey;
        pObj = pFonts->GetNextElement(pos, csKey);
        if (pObj == NULL) {
            continue;
        }
        CPDF_Object* pDirect = pObj->GetDirect();
        if (pDirect == NULL || pDirect->GetType() != PDFOBJ_DICTIONARY) {
            continue;
        }
        CPDF_Dictionary* pElement = (CPDF_Dictionary*)pDirect;
        if (pElement->GetString("Type") != "Font") {
            continue;
        }
        CPDF_Font* pFind = pDocument->LoadFont(pElement);
        if (pFind == NULL) {
            continue;
        }
        CFX_ByteString csBaseFont;
        csBaseFont = pFind->GetBaseFont();
        csBaseFont.Remove(' ');
        if (csBaseFont == csFontName) {
            csNameTag = csKey;
            return pFind;
        }
    }
    return NULL;
}
CPDF_Font* GetNativeInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, FX_BYTE charSet, CFX_ByteString& csNameTag)
{
    if (pFormDict == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return NULL;
    }
    FX_POSITION pos = pFonts->GetStartPos();
    while (pos) {
        CPDF_Object* pObj = NULL;
        CFX_ByteString csKey;
        pObj = pFonts->GetNextElement(pos, csKey);
        if (pObj == NULL) {
            continue;
        }
        CPDF_Object* pDirect = pObj->GetDirect();
        if (pDirect == NULL || pDirect->GetType() != PDFOBJ_DICTIONARY) {
            continue;
        }
        CPDF_Dictionary* pElement = (CPDF_Dictionary*)pDirect;
        if (pElement->GetString("Type") != "Font") {
            continue;
        }
        CPDF_Font* pFind = pDocument->LoadFont(pElement);
        if (pFind == NULL) {
            continue;
        }
        CFX_SubstFont* pSubst = (CFX_SubstFont*)pFind->GetSubstFont();
        if (pSubst == NULL) {
            continue;
        }
        if (pSubst->m_Charset == (int)charSet) {
            csNameTag = csKey;
            return pFind;
        }
    }
    return NULL;
}
CPDF_Font* GetNativeInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString& csNameTag)
{
    csNameTag = "";
    FX_BYTE charSet = CPDF_InterForm::GetNativeCharSet();
    CFX_SubstFont* pSubst;
    CPDF_Font* pFont = GetDefaultInterFormFont(pFormDict, pDocument);
    if (pFont != NULL) {
        pSubst = (CFX_SubstFont*)pFont->GetSubstFont();
        if (pSubst != NULL && pSubst->m_Charset == (int)charSet) {
            FindInterFormFont(pFormDict, pFont, csNameTag);
            return pFont;
        }
    }
    return GetNativeInterFormFont(pFormDict, pDocument, charSet, csNameTag);
}
FX_BOOL FindInterFormFont(CPDF_Dictionary* pFormDict, const CPDF_Font* pFont, CFX_ByteString& csNameTag)
{
    if (pFormDict == NULL || pFont == NULL) {
        return FALSE;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return FALSE;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return FALSE;
    }
    FX_POSITION pos = pFonts->GetStartPos();
    while (pos) {
        CPDF_Object* pObj = NULL;
        CFX_ByteString csKey;
        pObj = pFonts->GetNextElement(pos, csKey);
        if (pObj == NULL) {
            continue;
        }
        CPDF_Object* pDirect = pObj->GetDirect();
        if (pDirect == NULL || pDirect->GetType() != PDFOBJ_DICTIONARY) {
            continue;
        }
        CPDF_Dictionary* pElement = (CPDF_Dictionary*)pDirect;
        if (pElement->GetString("Type") != "Font") {
            continue;
        }
        if (pFont->GetFontDict() == pElement) {
            csNameTag = csKey;
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL FindInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument, CFX_ByteString csFontName, CPDF_Font*& pFont, CFX_ByteString& csNameTag)
{
    if (pFormDict == NULL) {
        return FALSE;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return FALSE;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return FALSE;
    }
    if (csFontName.GetLength() > 0) {
        csFontName.Remove(' ');
    }
    FX_POSITION pos = pFonts->GetStartPos();
    while (pos) {
        CPDF_Object* pObj = NULL;
        CFX_ByteString csKey, csTmp;
        pObj = pFonts->GetNextElement(pos, csKey);
        if (pObj == NULL) {
            continue;
        }
        CPDF_Object* pDirect = pObj->GetDirect();
        if (pDirect == NULL || pDirect->GetType() != PDFOBJ_DICTIONARY) {
            continue;
        }
        CPDF_Dictionary* pElement = (CPDF_Dictionary*)pDirect;
        if (pElement->GetString("Type") != "Font") {
            continue;
        }
        pFont = pDocument->LoadFont(pElement);
        if (pFont == NULL) {
            continue;
        }
        CFX_ByteString csBaseFont;
        csBaseFont = pFont->GetBaseFont();
        csBaseFont.Remove(' ');
        if (csBaseFont == csFontName) {
            csNameTag = csKey;
            return TRUE;
        }
    }
    return FALSE;
}
void AddInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, const CPDF_Font* pFont, CFX_ByteString& csNameTag)
{
    if (pFont == NULL) {
        return;
    }
    if (pFormDict == NULL) {
        InitInterFormDict(pFormDict, pDocument);
    }
    CFX_ByteString csTag;
    if (FindInterFormFont(pFormDict, pFont, csTag)) {
        csNameTag = csTag;
        return;
    }
    if (pFormDict == NULL) {
        InitInterFormDict(pFormDict, pDocument);
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        pDR = CPDF_Dictionary::Create();
        if (pDR == NULL) {
            return;
        }
        pFormDict->SetAt("DR", pDR);
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        pFonts = CPDF_Dictionary::Create();
        pDR->SetAt("Font", pFonts);
    }
    if (csNameTag.IsEmpty()) {
        csNameTag = pFont->GetBaseFont();
    }
    csNameTag.Remove(' ');
    csNameTag = CPDF_InterForm::GenerateNewResourceName(pDR, "Font", 4, csNameTag);
    pFonts->SetAtReference(csNameTag, pDocument, pFont->GetFontDict());
}
CPDF_Font* AddNativeInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, FX_BYTE charSet, CFX_ByteString& csNameTag)
{
    if (pFormDict == NULL) {
        InitInterFormDict(pFormDict, pDocument);
    }
    CFX_ByteString csTemp;
    CPDF_Font* pFont = GetNativeInterFormFont(pFormDict, pDocument, charSet, csTemp);
    if (pFont != NULL) {
        csNameTag = csTemp;
        return pFont;
    }
    CFX_ByteString csFontName = CPDF_InterForm::GetNativeFont(charSet);
    if (!csFontName.IsEmpty()) {
        if (FindInterFormFont(pFormDict, pDocument, csFontName, pFont, csNameTag)) {
            return pFont;
        }
    }
    pFont = CPDF_InterForm::AddNativeFont(charSet, pDocument);
    if (pFont != NULL) {
        AddInterFormFont(pFormDict, pDocument, pFont, csNameTag);
    }
    return pFont;
}
CPDF_Font* AddNativeInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, CFX_ByteString& csNameTag)
{
    FX_BYTE charSet = CPDF_InterForm::GetNativeCharSet();
    return AddNativeInterFormFont(pFormDict, pDocument, charSet, csNameTag);
}
void RemoveInterFormFont(CPDF_Dictionary* pFormDict, const CPDF_Font* pFont)
{
    if (pFormDict == NULL || pFont == NULL) {
        return;
    }
    CFX_ByteString csTag;
    if (!FindInterFormFont(pFormDict, pFont, csTag)) {
        return;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    pFonts->RemoveAt(csTag);
}
void RemoveInterFormFont(CPDF_Dictionary* pFormDict, CFX_ByteString csNameTag)
{
    if (pFormDict == NULL || csNameTag.IsEmpty()) {
        return;
    }
    CPDF_Dictionary* pDR = pFormDict->GetDict("DR");
    if (pDR == NULL) {
        return;
    }
    CPDF_Dictionary* pFonts = pDR->GetDict("Font");
    if (pFonts == NULL) {
        return;
    }
    pFonts->RemoveAt(csNameTag);
}
CPDF_Font* GetDefaultInterFormFont(CPDF_Dictionary* pFormDict, CPDF_Document* pDocument)
{
    if (pFormDict == NULL) {
        return NULL;
    }
    CPDF_DefaultAppearance cDA = pFormDict->GetString("DA");
    CFX_ByteString csFontNameTag;
    FX_FLOAT fFontSize;
    cDA.GetFont(csFontNameTag, fFontSize);
    return GetInterFormFont(pFormDict, pDocument, csFontNameTag);
}
CPDF_IconFit::ScaleMethod CPDF_IconFit::GetScaleMethod()
{
    if (m_pDict == NULL) {
        return Always;
    }
    CFX_ByteString csSW = m_pDict->GetString("SW", "A");
    if (csSW == "B") {
        return Bigger;
    } else if (csSW == "S") {
        return Smaller;
    } else if (csSW == "N") {
        return Never;
    }
    return Always;
}
FX_BOOL CPDF_IconFit::IsProportionalScale()
{
    if (m_pDict == NULL) {
        return TRUE;
    }
    return m_pDict->GetString("S", "P") != "A";
}
void CPDF_IconFit::GetIconPosition(FX_FLOAT& fLeft, FX_FLOAT& fBottom)
{
    fLeft = fBottom = 0.5;
    if (m_pDict == NULL) {
        return;
    }
    CPDF_Array* pA = m_pDict->GetArray("A");
    if (pA != NULL) {
        FX_DWORD dwCount = pA->GetCount();
        if (dwCount > 0) {
            fLeft = pA->GetNumber(0);
        }
        if (dwCount > 1) {
            fBottom = pA->GetNumber(1);
        }
    }
}
FX_BOOL CPDF_IconFit::GetFittingBounds()
{
    if (m_pDict == NULL) {
        return FALSE;
    }
    return m_pDict->GetBoolean("FB");
}
void SaveCheckedFieldStatus(CPDF_FormField* pField, CFX_ByteArray& statusArray)
{
    int iCount = pField->CountControls();
    for (int i = 0; i < iCount; i ++) {
        CPDF_FormControl* pControl = pField->GetControl(i);
        if (pControl == NULL) {
            continue;
        }
        statusArray.Add(pControl->IsChecked() ? 1 : 0);
    }
}
CPDF_Object* FPDF_GetFieldAttr(CPDF_Dictionary* pFieldDict, const FX_CHAR* name, int nLevel)
{
    if (nLevel > FPDFDOC_UTILS_MAXRECURSION) {
        return NULL;
    }
    if (pFieldDict == NULL) {
        return NULL;
    }
    CPDF_Object* pAttr = pFieldDict->GetElementValue(name);
    if (pAttr) {
        return pAttr;
    }
    CPDF_Dictionary* pParent = pFieldDict->GetDict("Parent");
    if (pParent == NULL) {
        return NULL;
    }
    return FPDF_GetFieldAttr(pParent, name, nLevel + 1);
}
