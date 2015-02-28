// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
const int nMaxRecursion = 32;
int CPDF_Dest::GetPageIndex(CPDF_Document* pDoc)
{
    if (m_pObj == NULL || m_pObj->GetType() != PDFOBJ_ARRAY) {
        return 0;
    }
    CPDF_Object* pPage = ((CPDF_Array*)m_pObj)->GetElementValue(0);
    if (pPage == NULL) {
        return 0;
    }
    if (pPage->GetType() == PDFOBJ_NUMBER) {
        return pPage->GetInteger();
    }
    if (pPage->GetType() != PDFOBJ_DICTIONARY) {
        return 0;
    }
    return pDoc->GetPageIndex(pPage->GetObjNum());
}
FX_DWORD CPDF_Dest::GetPageObjNum()
{
    if (m_pObj == NULL || m_pObj->GetType() != PDFOBJ_ARRAY) {
        return 0;
    }
    CPDF_Object* pPage = ((CPDF_Array*)m_pObj)->GetElementValue(0);
    if (pPage == NULL) {
        return 0;
    }
    if (pPage->GetType() == PDFOBJ_NUMBER) {
        return pPage->GetInteger();
    }
    if (pPage->GetType() == PDFOBJ_DICTIONARY) {
        return pPage->GetObjNum();
    }
    return 0;
}
const FX_CHAR* g_sZoomModes[] = {"XYZ", "Fit", "FitH", "FitV", "FitR", "FitB", "FitBH", "FitBV", ""};
int CPDF_Dest::GetZoomMode()
{
    if (m_pObj == NULL || m_pObj->GetType() != PDFOBJ_ARRAY) {
        return 0;
    }
    CFX_ByteString mode;
    CPDF_Object* pObj = ((CPDF_Array*)m_pObj)->GetElementValue(1);
    mode = pObj ? pObj->GetString() : CFX_ByteString();
    int i = 0;
    while (g_sZoomModes[i][0] != '\0') {
        if (mode == g_sZoomModes[i]) {
            return i + 1;
        }
        i ++;
    }
    return 0;
}
FX_FLOAT CPDF_Dest::GetParam(int index)
{
    if (m_pObj == NULL || m_pObj->GetType() != PDFOBJ_ARRAY) {
        return 0;
    }
    return ((CPDF_Array*)m_pObj)->GetNumber(2 + index);
}
CFX_ByteString CPDF_Dest::GetRemoteName()
{
    if (m_pObj == NULL) {
        return CFX_ByteString();
    }
    return m_pObj->GetString();
}
CPDF_NameTree::CPDF_NameTree(CPDF_Document* pDoc, FX_BSTR category)
{
    if (pDoc->GetRoot() && pDoc->GetRoot()->GetDict(FX_BSTRC("Names")))
        m_pRoot = pDoc->GetRoot()->GetDict(FX_BSTRC("Names"))->GetDict(category);
    else
        m_pRoot = NULL;
}
static CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode, const CFX_ByteString& csName,
                                   int& nIndex, CPDF_Array** ppFind, int nLevel = 0)
{
    if (nLevel > nMaxRecursion) {
        return NULL;
    }
    CPDF_Array* pLimits = pNode->GetArray(FX_BSTRC("Limits"));
    if (pLimits != NULL) {
        CFX_ByteString csLeft = pLimits->GetString(0);
        CFX_ByteString csRight = pLimits->GetString(1);
        if (csLeft.Compare(csRight) > 0) {
            CFX_ByteString csTmp = csRight;
            csRight = csLeft;
            csLeft = csTmp;
        }
        if (csName.Compare(csLeft) < 0 || csName.Compare(csRight) > 0) {
            return NULL;
        }
    }
    CPDF_Array* pNames = pNode->GetArray(FX_BSTRC("Names"));
    if (pNames) {
        FX_DWORD dwCount = pNames->GetCount() / 2;
        for (FX_DWORD i = 0; i < dwCount; i ++) {
            CFX_ByteString csValue = pNames->GetString(i * 2);
            FX_INT32 iCompare = csValue.Compare(csName);
            if (iCompare <= 0) {
                if (ppFind != NULL) {
                    *ppFind = pNames;
                }
                if (iCompare < 0) {
                    continue;
                }
            } else {
                break;
            }
            nIndex += i;
            return pNames->GetElementValue(i * 2 + 1);
        }
        nIndex += dwCount;
        return NULL;
    }
    CPDF_Array* pKids = pNode->GetArray(FX_BSTRC("Kids"));
    if (pKids == NULL) {
        return NULL;
    }
    for (FX_DWORD i = 0; i < pKids->GetCount(); i ++) {
        CPDF_Dictionary* pKid = pKids->GetDict(i);
        if (pKid == NULL) {
            continue;
        }
        CPDF_Object* pFound = SearchNameNode(pKid, csName, nIndex, ppFind, nLevel + 1);
        if (pFound) {
            return pFound;
        }
    }
    return NULL;
}
static CPDF_Object* SearchNameNode(CPDF_Dictionary* pNode, int nIndex, int& nCurIndex,
                                   CFX_ByteString& csName, CPDF_Array** ppFind, int nLevel = 0)
{
    if (nLevel > nMaxRecursion) {
        return NULL;
    }
    CPDF_Array* pNames = pNode->GetArray(FX_BSTRC("Names"));
    if (pNames) {
        int nCount = pNames->GetCount() / 2;
        if (nIndex >= nCurIndex + nCount) {
            nCurIndex += nCount;
            return NULL;
        } else {
            if (ppFind != NULL) {
                *ppFind = pNames;
            }
            csName = pNames->GetString((nIndex - nCurIndex) * 2);
            return pNames->GetElementValue((nIndex - nCurIndex) * 2 + 1);
        }
    }
    CPDF_Array* pKids = pNode->GetArray(FX_BSTRC("Kids"));
    if (pKids == NULL) {
        return NULL;
    }
    for (FX_DWORD i = 0; i < pKids->GetCount(); i ++) {
        CPDF_Dictionary* pKid = pKids->GetDict(i);
        if (pKid == NULL) {
            continue;
        }
        CPDF_Object* pFound = SearchNameNode(pKid, nIndex, nCurIndex, csName, ppFind, nLevel + 1);
        if (pFound) {
            return pFound;
        }
    }
    return NULL;
}
static int CountNames(CPDF_Dictionary* pNode, int nLevel = 0)
{
    if (nLevel > nMaxRecursion) {
        return 0;
    }
    CPDF_Array* pNames = pNode->GetArray(FX_BSTRC("Names"));
    if (pNames) {
        return pNames->GetCount() / 2;
    }
    CPDF_Array* pKids = pNode->GetArray(FX_BSTRC("Kids"));
    if (pKids == NULL) {
        return 0;
    }
    int nCount = 0;
    for (FX_DWORD i = 0; i < pKids->GetCount(); i ++) {
        CPDF_Dictionary* pKid = pKids->GetDict(i);
        if (pKid == NULL) {
            continue;
        }
        nCount += CountNames(pKid, nLevel + 1);
    }
    return nCount;
}
int CPDF_NameTree::GetCount() const
{
    if (m_pRoot == NULL) {
        return 0;
    }
    return ::CountNames(m_pRoot);
}
int CPDF_NameTree::GetIndex(const CFX_ByteString& csName) const
{
    if (m_pRoot == NULL) {
        return -1;
    }
    int nIndex = 0;
    if (SearchNameNode(m_pRoot, csName, nIndex, NULL) == NULL) {
        return -1;
    }
    return nIndex;
}
CPDF_Object* CPDF_NameTree::LookupValue(int nIndex, CFX_ByteString& csName) const
{
    if (m_pRoot == NULL) {
        return NULL;
    }
    int nCurIndex = 0;
    return SearchNameNode(m_pRoot, nIndex, nCurIndex, csName, NULL);
}
CPDF_Object* CPDF_NameTree::LookupValue(const CFX_ByteString& csName) const
{
    if (m_pRoot == NULL) {
        return NULL;
    }
    int nIndex = 0;
    return SearchNameNode(m_pRoot, csName, nIndex, NULL);
}
CPDF_Array*	CPDF_NameTree::LookupNamedDest(CPDF_Document* pDoc, FX_BSTR sName)
{
    CPDF_Object* pValue = LookupValue(sName);
    if (pValue == NULL) {
        CPDF_Dictionary* pDests = pDoc->GetRoot()->GetDict(FX_BSTRC("Dests"));
        if (pDests == NULL) {
            return NULL;
        }
        pValue = pDests->GetElementValue(sName);
    }
    if (pValue == NULL) {
        return NULL;
    }
    if (pValue->GetType() == PDFOBJ_ARRAY) {
        return (CPDF_Array*)pValue;
    }
    if (pValue->GetType() == PDFOBJ_DICTIONARY) {
        return ((CPDF_Dictionary*)pValue)->GetArray(FX_BSTRC("D"));
    }
    return NULL;
}
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_ || _FXM_PLATFORM_  == _FXM_PLATFORM_WINDOWS_
static CFX_WideString ChangeSlashToPlatform(FX_LPCWSTR str)
{
    CFX_WideString result;
    while (*str) {
        if (*str == '/') {
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
            result += ':';
#else
            result += '\\';
#endif
        } else {
            result += *str;
        }
        str++;
    }
    return result;
}
static CFX_WideString ChangeSlashToPDF(FX_LPCWSTR str)
{
    CFX_WideString result;
    while (*str) {
        if (*str == '\\' || *str == ':') {
            result += '/';
        } else {
            result += *str;
        }
        str++;
    }
    return result;
}
#endif
static CFX_WideString FILESPEC_DecodeFileName(FX_WSTR filepath)
{
    if (filepath.GetLength() <= 1) {
        return CFX_WideString();
    }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    if (filepath.Left(sizeof("/Mac") - 1) == CFX_WideStringC(L"/Mac")) {
        return ChangeSlashToPlatform(filepath.GetPtr() + 1);
    }
    return ChangeSlashToPlatform(filepath.GetPtr());
#elif _FXM_PLATFORM_  == _FXM_PLATFORM_WINDOWS_
    if (filepath.GetAt(0) != '/') {
        return ChangeSlashToPlatform(filepath.GetPtr());
    }
    if (filepath.GetAt(1) == '/') {
        return ChangeSlashToPlatform(filepath.GetPtr() + 1);
    }
    if (filepath.GetAt(2) == '/') {
        CFX_WideString result;
        result += filepath.GetAt(1);
        result += ':';
        result += ChangeSlashToPlatform(filepath.GetPtr() + 2);
        return result;
    }
    CFX_WideString result;
    result += '\\';
    result += ChangeSlashToPlatform(filepath.GetPtr());
    return result;
#else
    return filepath;
#endif
}
FX_BOOL CPDF_FileSpec::GetFileName(CFX_WideString &csFileName) const
{
    if (m_pObj == NULL) {
        return FALSE;
    }
    if (m_pObj->GetType() == PDFOBJ_DICTIONARY) {
        CPDF_Dictionary* pDict = (CPDF_Dictionary*)m_pObj;
        csFileName = pDict->GetUnicodeText(FX_BSTRC("UF"));
        if (csFileName.IsEmpty()) {
            csFileName = CFX_WideString::FromLocal(pDict->GetString(FX_BSTRC("F")));
        }
        if (pDict->GetString(FX_BSTRC("FS")) == FX_BSTRC("URL")) {
            return TRUE;
        }
        if (csFileName.IsEmpty()) {
            if (pDict->KeyExist(FX_BSTRC("DOS"))) {
                csFileName = CFX_WideString::FromLocal(pDict->GetString(FX_BSTRC("DOS")));
            } else if (pDict->KeyExist(FX_BSTRC("Mac"))) {
                csFileName = CFX_WideString::FromLocal(pDict->GetString(FX_BSTRC("Mac")));
            } else if (pDict->KeyExist(FX_BSTRC("Unix"))) {
                csFileName = CFX_WideString::FromLocal(pDict->GetString(FX_BSTRC("Unix")));
            } else {
                return FALSE;
            }
        }
    } else {
        csFileName = CFX_WideString::FromLocal(m_pObj->GetString());
    }
    csFileName = FILESPEC_DecodeFileName(csFileName);
    return TRUE;
}
CPDF_FileSpec::CPDF_FileSpec()
{
    m_pObj = CPDF_Dictionary::Create();
    if (m_pObj != NULL) {
        ((CPDF_Dictionary*)m_pObj)->SetAtName(FX_BSTRC("Type"), FX_BSTRC("Filespec"));
    }
}
FX_BOOL CPDF_FileSpec::IsURL() const
{
    if (m_pObj == NULL) {
        return FALSE;
    }
    if (m_pObj->GetType() != PDFOBJ_DICTIONARY) {
        return FALSE;
    }
    return ((CPDF_Dictionary*)m_pObj)->GetString(FX_BSTRC("FS")) == FX_BSTRC("URL");
}
CFX_WideString FILESPEC_EncodeFileName(FX_WSTR filepath)
{
    if (filepath.GetLength() <= 1) {
        return CFX_WideString();
    }
#if _FXM_PLATFORM_  == _FXM_PLATFORM_WINDOWS_
    if (filepath.GetAt(1) == ':') {
        CFX_WideString result;
        result = '/';
        result += filepath.GetAt(0);
        if (filepath.GetAt(2) != '\\') {
            result += '/';
        }
        result += ChangeSlashToPDF(filepath.GetPtr() + 2);
        return result;
    }
    if (filepath.GetAt(0) == '\\' && filepath.GetAt(1) == '\\') {
        return ChangeSlashToPDF(filepath.GetPtr() + 1);
    }
    if (filepath.GetAt(0) == '\\') {
        CFX_WideString result;
        result = '/';
        result += ChangeSlashToPDF(filepath.GetPtr());
        return result;
    }
    return ChangeSlashToPDF(filepath.GetPtr());
#elif _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
    if (filepath.Left(sizeof("Mac") - 1) == FX_WSTRC(L"Mac")) {
        CFX_WideString result;
        result = '/';
        result += ChangeSlashToPDF(filepath.GetPtr());
        return result;
    }
    return ChangeSlashToPDF(filepath.GetPtr());
#else
    return filepath;
#endif
}
CPDF_Stream* CPDF_FileSpec::GetFileStream() const
{
    if (m_pObj == NULL) {
        return NULL;
    }
    FX_INT32 iType = m_pObj->GetType();
    if (iType == PDFOBJ_STREAM) {
        return (CPDF_Stream*)m_pObj;
    } else if (iType == PDFOBJ_DICTIONARY) {
        CPDF_Dictionary *pEF = ((CPDF_Dictionary*)m_pObj)->GetDict(FX_BSTRC("EF"));
        if (pEF == NULL) {
            return NULL;
        }
        return pEF->GetStream(FX_BSTRC("F"));
    }
    return NULL;
}
static void FPDFDOC_FILESPEC_SetFileName(CPDF_Object *pObj, FX_WSTR wsFileName, FX_BOOL bURL)
{
    ASSERT(pObj != NULL);
    CFX_WideString wsStr;
    if (bURL) {
        wsStr = wsFileName;
    } else {
        wsStr = FILESPEC_EncodeFileName(wsFileName);
    }
    FX_INT32 iType = pObj->GetType();
    if (iType == PDFOBJ_STRING) {
        pObj->SetString(CFX_ByteString::FromUnicode(wsStr));
    } else if (iType == PDFOBJ_DICTIONARY) {
        CPDF_Dictionary* pDict = (CPDF_Dictionary*)pObj;
        pDict->SetAtString(FX_BSTRC("F"), CFX_ByteString::FromUnicode(wsStr));
        pDict->SetAtString(FX_BSTRC("UF"), PDF_EncodeText(wsStr));
    }
}
void CPDF_FileSpec::SetFileName(FX_WSTR wsFileName, FX_BOOL bURL)
{
    ASSERT(m_pObj != NULL);
    if (m_pObj->GetType() == PDFOBJ_DICTIONARY && bURL) {
        ((CPDF_Dictionary*)m_pObj)->SetAtName(FX_BSTRC("FS"), "URL");
    }
    FPDFDOC_FILESPEC_SetFileName(m_pObj, wsFileName, bURL);
}
static CFX_WideString _MakeRoman(int num)
{
    const int arabic[] = {
        1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1
    };
    const CFX_WideString roman[] = {
        L"m", L"cm", L"d", L"cd", L"c", L"xc", L"l", L"xl", L"x", L"ix", L"v", L"iv", L"i"
    };
    const int nMaxNum = 1000000;
    num %= nMaxNum;
    int i = 0;
    CFX_WideString wsRomanNumber;
    while (num > 0) {
        while (num >= arabic[i]) {
            num = num - arabic[i];
            wsRomanNumber += roman[i];
        }
        i = i + 1;
    }
    return wsRomanNumber;
}
static CFX_WideString _MakeLetters(int num)
{
    if (num == 0) {
        return CFX_WideString();
    }
    CFX_WideString wsLetters;
    const int nMaxCount = 1000;
    const int nLetterCount = 26;
    num -= 1;
    int count = num / nLetterCount + 1;
    count %= nMaxCount;
    FX_WCHAR ch = L'a' + num % nLetterCount;
    for (int i = 0; i < count; i++) {
        wsLetters += ch;
    }
    return wsLetters;
}
static CFX_WideString _GetLabelNumPortion(int num, const CFX_ByteString& bsStyle)
{
    CFX_WideString wsNumPortion;
    if		(bsStyle.IsEmpty()) {
        return wsNumPortion;
    }
    if (bsStyle == "D") {
        wsNumPortion.Format(L"%d", num);
    } else if (bsStyle == "R") {
        wsNumPortion = _MakeRoman(num);
        wsNumPortion.MakeUpper();
    } else if (bsStyle == "r") {
        wsNumPortion = _MakeRoman(num);
    } else if (bsStyle == "A") {
        wsNumPortion = _MakeLetters(num);
        wsNumPortion.MakeUpper();
    } else if (bsStyle == "a") {
        wsNumPortion = _MakeLetters(num);
    }
    return wsNumPortion;
}
CFX_WideString CPDF_PageLabel::GetLabel(int nPage) const
{
    CFX_WideString wsLabel;
    if (m_pDocument == NULL) {
        return wsLabel;
    }
    CPDF_Dictionary* pPDFRoot = m_pDocument->GetRoot();
    if (pPDFRoot == NULL) {
        return wsLabel;
    }
    CPDF_Dictionary* pLabels = pPDFRoot->GetDict(FX_BSTRC("PageLabels"));
    CPDF_NumberTree numberTree(pLabels);
    CPDF_Object* pValue = NULL;
    int n = nPage;
    while (n >= 0) {
        pValue = numberTree.LookupValue(n);
        if (pValue != NULL) {
            break;
        }
        n--;
    }
    if (pValue != NULL) {
        pValue = pValue->GetDirect();
        if (pValue->GetType() == PDFOBJ_DICTIONARY) {
            CPDF_Dictionary* pLabel = (CPDF_Dictionary*)pValue;
            if (pLabel->KeyExist(FX_BSTRC("P"))) {
                wsLabel += pLabel->GetUnicodeText(FX_BSTRC("P"));
            }
            CFX_ByteString bsNumberingStyle = pLabel->GetString(FX_BSTRC("S"), NULL);
            int nLabelNum = nPage - n + pLabel->GetInteger(FX_BSTRC("St"), 1);
            CFX_WideString wsNumPortion = _GetLabelNumPortion(nLabelNum, bsNumberingStyle);
            wsLabel += wsNumPortion;
            return wsLabel;
        }
    }
    wsLabel.Format(L"%d", nPage + 1);
    return wsLabel;
}
FX_INT32 CPDF_PageLabel::GetPageByLabel(FX_BSTR bsLabel) const
{
    if (m_pDocument == NULL) {
        return -1;
    }
    CPDF_Dictionary* pPDFRoot = m_pDocument->GetRoot();
    if (pPDFRoot == NULL) {
        return -1;
    }
    int nPages = m_pDocument->GetPageCount();
    CFX_ByteString bsLbl;
    CFX_ByteString bsOrig = bsLabel;
    for (int i = 0; i < nPages; i++) {
        bsLbl = PDF_EncodeText(GetLabel(i));
        if (!bsLbl.Compare(bsOrig)) {
            return i;
        }
    }
    bsLbl = bsOrig;
    int nPage = FXSYS_atoi(bsLbl);
    if (nPage > 0 && nPage <= nPages) {
        return nPage;
    }
    return -1;
}
FX_INT32 CPDF_PageLabel::GetPageByLabel(FX_WSTR wsLabel) const
{
    CFX_ByteString bsLabel = PDF_EncodeText((CFX_WideString)wsLabel);
    return GetPageByLabel(bsLabel);
}
