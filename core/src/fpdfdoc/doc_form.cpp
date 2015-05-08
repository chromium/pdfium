// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
#include "../../include/fxcrt/fx_xml.h"
#include "doc_utils.h"

const int nMaxRecursion = 32;

class _CFieldNameExtractor 
{
public:
    _CFieldNameExtractor(const CFX_WideString& full_name)
    {
        m_pStart = full_name.c_str();
        m_pEnd = m_pStart + full_name.GetLength();
        m_pCur = m_pStart;
    }
    void GetNext(FX_LPCWSTR &pSubName, FX_STRSIZE& size)
    {
        pSubName = m_pCur;
        while (m_pCur < m_pEnd && m_pCur[0] != L'.') {
            m_pCur++;
        }
        size = (FX_STRSIZE)(m_pCur - pSubName);
        if (m_pCur < m_pEnd && m_pCur[0] == L'.') {
            m_pCur++;
        }
    }
protected:
    FX_LPCWSTR m_pStart;
    FX_LPCWSTR m_pEnd;
    FX_LPCWSTR m_pCur;
};
class CFieldTree 
{
public:
    struct _Node  {
        _Node *parent;
        CFX_PtrArray children;
        CFX_WideString short_name;
        CPDF_FormField *field_ptr;
        int CountFields(int nLevel = 0)
        {
            if (nLevel > nMaxRecursion) {
                return 0;
            }
            if (field_ptr) {
                return 1;
            }
            int count = 0;
            for (int i = 0; i < children.GetSize(); i ++) {
                count += ((_Node *)children.GetAt(i))->CountFields(nLevel + 1);
            }
            return count;
        }
        CPDF_FormField* GetField(int* fields_to_go)
        {
            if (field_ptr) {
                if (*fields_to_go == 0) {
                    return field_ptr;
                }
                --*fields_to_go;
                return NULL;
            }
            for (int i = 0; i < children.GetSize(); i++) {
                _Node *pNode = (_Node *)children.GetAt(i);
                CPDF_FormField* pField = pNode->GetField(fields_to_go);
                if (pField) {
                    return pField;
                }
            }
            return NULL;
        }
        CPDF_FormField* GetField(int index)
        {
            int fields_to_go = index;
            return GetField(&fields_to_go);
        }
    };
    CFieldTree();
    ~CFieldTree();
    void SetField(const CFX_WideString &full_name, CPDF_FormField *field_ptr);
    CPDF_FormField *GetField(const CFX_WideString &full_name);
    CPDF_FormField *RemoveField(const CFX_WideString &full_name);
    void RemoveAll();
    _Node *FindNode(const CFX_WideString &full_name);
    _Node * AddChild(_Node *pParent, const CFX_WideString &short_name, CPDF_FormField *field_ptr);
    void RemoveNode(_Node *pNode, int nLevel = 0);
    _Node *_Lookup(_Node *pParent, const CFX_WideString &short_name);
    _Node m_Root;
};
CFieldTree::CFieldTree()
{
    m_Root.parent = NULL;
    m_Root.field_ptr = NULL;
}
CFieldTree::~CFieldTree()
{
    RemoveAll();
}
CFieldTree::_Node *CFieldTree::AddChild(_Node *pParent, const CFX_WideString &short_name, CPDF_FormField *field_ptr)
{
    if (pParent == NULL) {
        return NULL;
    }
    _Node* pNode = new _Node;
    pNode->parent = pParent;
    pNode->short_name = short_name;
    pNode->field_ptr = field_ptr;
    pParent->children.Add(pNode);
    return pNode;
}
void CFieldTree::RemoveNode(_Node *pNode, int nLevel)
{
    if (pNode == NULL) {
        return ;
    }
    if (nLevel > nMaxRecursion) {
        delete pNode;
        return ;
    }
    CFX_PtrArray& ptr_array = pNode->children;
    for (int i = 0; i < ptr_array.GetSize(); i ++) {
        _Node *pChild = (_Node *)ptr_array[i];
        RemoveNode(pChild, nLevel + 1);
    }
    delete pNode;
}
CFieldTree::_Node *CFieldTree::_Lookup(_Node *pParent, const CFX_WideString &short_name)
{
    if (pParent == NULL) {
        return NULL;
    }
    CFX_PtrArray& ptr_array = pParent->children;
    for (int i = 0; i < ptr_array.GetSize(); i ++) {
        _Node *pNode = (_Node *)ptr_array[i];
        if (pNode->short_name.GetLength() == short_name.GetLength() &&
                FXSYS_memcmp32(pNode->short_name.c_str(), short_name.c_str(), short_name.GetLength()*sizeof(FX_WCHAR)) == 0) {
            return pNode;
        }
    }
    return NULL;
}
void CFieldTree::RemoveAll()
{
    CFX_PtrArray& ptr_array = m_Root.children;
    for (int i = 0; i < ptr_array.GetSize(); i ++) {
        _Node *pNode = (_Node *)ptr_array[i];
        RemoveNode(pNode);
    }
}
void CFieldTree::SetField(const CFX_WideString &full_name, CPDF_FormField *field_ptr)
{
    if (full_name == L"") {
        return;
    }
    _CFieldNameExtractor name_extractor(full_name);
    FX_LPCWSTR pName;
    FX_STRSIZE nLength;
    name_extractor.GetNext(pName, nLength);
    _Node *pNode = &m_Root, *pLast = NULL;
    while (nLength > 0) {
        pLast = pNode;
        CFX_WideString name = CFX_WideString(pName, nLength);
        pNode = _Lookup(pLast, name);
        if (pNode == NULL) {
            pNode = AddChild(pLast, name, NULL);
        }
        name_extractor.GetNext(pName, nLength);
    }
    if (pNode != &m_Root) {
        pNode->field_ptr = field_ptr;
    }
}
CPDF_FormField *CFieldTree::GetField(const CFX_WideString &full_name)
{
    if (full_name == L"") {
        return NULL;
    }
    _CFieldNameExtractor name_extractor(full_name);
    FX_LPCWSTR pName;
    FX_STRSIZE nLength;
    name_extractor.GetNext(pName, nLength);
    _Node *pNode = &m_Root, *pLast = NULL;
    while (nLength > 0 && pNode) {
        pLast = pNode;
        CFX_WideString name = CFX_WideString(pName, nLength);
        pNode = _Lookup(pLast, name);
        name_extractor.GetNext(pName, nLength);
    }
    return pNode ? pNode->field_ptr : NULL;
}
CPDF_FormField *CFieldTree::RemoveField(const CFX_WideString & full_name)
{
    if (full_name == L"") {
        return NULL;
    }
    _CFieldNameExtractor name_extractor(full_name);
    FX_LPCWSTR pName;
    FX_STRSIZE nLength;
    name_extractor.GetNext(pName, nLength);
    _Node *pNode = &m_Root, *pLast = NULL;
    while (nLength > 0 && pNode) {
        pLast = pNode;
        CFX_WideString name = CFX_WideString(pName, nLength);
        pNode = _Lookup(pLast, name);
        name_extractor.GetNext(pName, nLength);
    }
    if (pNode && pNode != &m_Root) {
        CFX_PtrArray& ptr_array = pLast->children;
        for (int i = 0; i < ptr_array.GetSize(); i ++) {
            if (pNode == (_Node *)ptr_array[i]) {
                ptr_array.RemoveAt(i);
                break;
            }
        }
        CPDF_FormField *pField = pNode->field_ptr;
        RemoveNode(pNode);
        return pField;
    }
    return NULL;
}
CFieldTree::_Node *CFieldTree::FindNode(const CFX_WideString& full_name)
{
    if (full_name == L"") {
        return NULL;
    }
    _CFieldNameExtractor name_extractor(full_name);
    FX_LPCWSTR pName;
    FX_STRSIZE nLength;
    name_extractor.GetNext(pName, nLength);
    _Node *pNode = &m_Root, *pLast = NULL;
    while (nLength > 0 && pNode) {
        pLast = pNode;
        CFX_WideString name = CFX_WideString(pName, nLength);
        pNode = _Lookup(pLast, name);
        name_extractor.GetNext(pName, nLength);
    }
    return pNode;
}
CPDF_InterForm::CPDF_InterForm(CPDF_Document* pDocument, FX_BOOL bGenerateAP) : CFX_PrivateData()
{
    m_pDocument = pDocument;
    m_bGenerateAP = bGenerateAP;
    m_pFormNotify = NULL;
    m_bUpdated = FALSE;
    m_pFieldTree = new CFieldTree;
    CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
    m_pFormDict = pRoot->GetDict("AcroForm");
    if (m_pFormDict == NULL) {
        return;
    }
    CPDF_Array* pFields = m_pFormDict->GetArray("Fields");
    if (pFields == NULL) {
        return;
    }
    int count = pFields->GetCount();
    for (int i = 0; i < count; i ++) {
        LoadField(pFields->GetDict(i));
    }
}
CPDF_InterForm::~CPDF_InterForm()
{
    FX_POSITION pos = m_ControlMap.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_ControlMap.GetNextAssoc(pos, key, value);
        delete (CPDF_FormControl*)value;
    }
    if (m_pFieldTree != NULL) {
        int nCount = m_pFieldTree->m_Root.CountFields();
        for (int i = 0; i < nCount; i++) {
            CPDF_FormField *pField = m_pFieldTree->m_Root.GetField(i);
            delete pField;
        }
        delete m_pFieldTree;
    }
}
FX_BOOL	CPDF_InterForm::m_bUpdateAP = TRUE;
FX_BOOL CPDF_InterForm::UpdatingAPEnabled()
{
    return m_bUpdateAP;
}
void CPDF_InterForm::EnableUpdateAP(FX_BOOL bUpdateAP)
{
    m_bUpdateAP = bUpdateAP;
}
CFX_ByteString CPDF_InterForm::GenerateNewResourceName(const CPDF_Dictionary* pResDict, FX_LPCSTR csType, int iMinLen, FX_LPCSTR csPrefix)
{
    CFX_ByteString csStr = csPrefix;
    CFX_ByteString csBType = csType;
    if (csStr.IsEmpty()) {
        if (csBType == "ExtGState") {
            csStr = "GS";
        } else if (csBType == "ColorSpace") {
            csStr = "CS";
        } else if (csBType == "Font") {
            csStr = "ZiTi";
        } else {
            csStr = "Res";
        }
    }
    CFX_ByteString csTmp = csStr;
    int iCount = csStr.GetLength();
    int m = 0;
    if (iMinLen > 0) {
        csTmp = "";
        while (m < iMinLen && m < iCount) {
            csTmp += csStr[m ++];
        }
        while (m < iMinLen) {
            csTmp += '0' + m % 10;
            m ++;
        }
    } else {
        m = iCount;
    }
    if (pResDict == NULL) {
        return csTmp;
    }
    CPDF_Dictionary* pDict = pResDict->GetDict(csType);
    if (pDict == NULL) {
        return csTmp;
    }
    int num = 0;
    CFX_ByteString bsNum;
    while (TRUE) {
        if (!pDict->KeyExist(csTmp + bsNum)) {
            return csTmp + bsNum;
        }
        if (m < iCount) {
            csTmp += csStr[m ++];
        } else {
            bsNum.Format("%d", num++);
        }
        m ++;
    }
    return csTmp;
}
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
typedef struct _PDF_FONTDATA {
    FX_BOOL		bFind;
    LOGFONTA	lf;
} PDF_FONTDATA, FAR* LPDF_FONTDATA;
static int CALLBACK EnumFontFamExProc(	ENUMLOGFONTEXA *lpelfe,
                                        NEWTEXTMETRICEX *lpntme,
                                        DWORD FontType,
                                        LPARAM lParam
                                     )
{
    if (FontType != 0x004 || strchr(lpelfe->elfLogFont.lfFaceName, '@') != NULL) {
        return 1;
    } else {
        LPDF_FONTDATA pData = (LPDF_FONTDATA)lParam;
        memcpy(&pData->lf, &lpelfe->elfLogFont, sizeof(LOGFONTA));
        pData->bFind = TRUE;
        return 0;
    }
}
static FX_BOOL RetrieveSpecificFont(LOGFONTA& lf)
{
    PDF_FONTDATA fd;
    memset(&fd, 0, sizeof(PDF_FONTDATA));
    HDC hDC = ::GetDC(NULL);
    EnumFontFamiliesExA(hDC, &lf, (FONTENUMPROCA)EnumFontFamExProc, (LPARAM)&fd, 0);
    ::ReleaseDC(NULL, hDC);
    if (fd.bFind) {
        memcpy(&lf, &fd.lf, sizeof(LOGFONTA));
    }
    return fd.bFind;
}
static FX_BOOL RetrieveSpecificFont(FX_BYTE charSet, FX_BYTE pitchAndFamily, LPCSTR pcsFontName, LOGFONTA& lf)
{
    memset(&lf, 0, sizeof(LOGFONTA));
    lf.lfCharSet = charSet;
    lf.lfPitchAndFamily = pitchAndFamily;
    if (pcsFontName != NULL) {
        strcpy(lf.lfFaceName, pcsFontName);
    }
    return RetrieveSpecificFont(lf);
}
static FX_BOOL RetrieveStockFont(int iFontObject, FX_BYTE charSet, LOGFONTA& lf)
{
    HFONT hFont = (HFONT)::GetStockObject(iFontObject);
    if (hFont != NULL) {
        memset(&lf, 0, sizeof(LOGFONTA));
        int iRet = ::GetObject(hFont, sizeof(LOGFONTA), &lf);
        if (iRet > 0 && (lf.lfCharSet == charSet || charSet == 255)) {
            return RetrieveSpecificFont(lf);
        }
    }
    return FALSE;
}
#endif
CPDF_Font* CPDF_InterForm::AddSystemDefaultFont(const CPDF_Document* pDocument)
{
    if (pDocument == NULL) {
        return NULL;
    }
    CPDF_Font* pFont = NULL;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    LOGFONTA lf;
    FX_BOOL bRet;
    bRet = RetrieveStockFont(DEFAULT_GUI_FONT, 255, lf);
    if (!bRet) {
        bRet = RetrieveStockFont(SYSTEM_FONT, 255, lf);
    }
    if (bRet) {
        pFont = ((CPDF_Document*)pDocument)->AddWindowsFont(&lf, FALSE, TRUE);
    }
#endif
    return pFont;
}
CPDF_Font* CPDF_InterForm::AddSystemFont(const CPDF_Document* pDocument, CFX_ByteString csFontName, FX_BYTE iCharSet)
{
    if (pDocument == NULL || csFontName.IsEmpty()) {
        return NULL;
    }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    if (iCharSet == 1) {
        iCharSet = GetNativeCharSet();
    }
    HFONT hFont = ::CreateFontA(0, 0, 0, 0, 0, 0, 0, 0, iCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, csFontName.c_str());
    if (hFont != NULL) {
        LOGFONTA lf;
        memset(&lf, 0, sizeof(LOGFONTA));
        ::GetObjectA(hFont, sizeof(LOGFONTA), &lf);
        ::DeleteObject(hFont);
        if (strlen(lf.lfFaceName) > 0) {
            return ((CPDF_Document*)pDocument)->AddWindowsFont(&lf, FALSE, TRUE);
        }
    }
#endif
    return NULL;
}
CPDF_Font* CPDF_InterForm::AddSystemFont(const CPDF_Document* pDocument, CFX_WideString csFontName, FX_BYTE iCharSet)
{
    if (pDocument == NULL || csFontName.IsEmpty()) {
        return NULL;
    }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    if (iCharSet == 1) {
        iCharSet = GetNativeCharSet();
    }
    HFONT hFont = ::CreateFontW(0, 0, 0, 0, 0, 0, 0, 0, iCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, csFontName.c_str());
    if (hFont != NULL) {
        LOGFONTA lf;
        memset(&lf, 0, sizeof(LOGFONTA));
        ::GetObject(hFont, sizeof(LOGFONTA), &lf);
        ::DeleteObject(hFont);
        if (strlen(lf.lfFaceName) > 0) {
            return ((CPDF_Document*)pDocument)->AddWindowsFont(&lf, FALSE, TRUE);
        }
    }
#endif
    return NULL;
}
CPDF_Font* CPDF_InterForm::AddStandardFont(const CPDF_Document* pDocument, CFX_ByteString csFontName)
{
    if (pDocument == NULL || csFontName.IsEmpty()) {
        return NULL;
    }
    CPDF_Font* pFont = NULL;
    if (csFontName == "ZapfDingbats") {
        pFont = ((CPDF_Document*)pDocument)->AddStandardFont(csFontName, NULL);
    } else {
        CPDF_FontEncoding encoding(PDFFONT_ENCODING_WINANSI);
        pFont = ((CPDF_Document*)pDocument)->AddStandardFont(csFontName, &encoding);
    }
    return pFont;
}
CFX_ByteString CPDF_InterForm::GetNativeFont(FX_BYTE charSet, FX_LPVOID pLogFont)
{
    CFX_ByteString csFontName;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    LOGFONTA lf;
    FX_BOOL bRet;
    if (charSet == ANSI_CHARSET) {
        csFontName = "Helvetica";
        return csFontName;
    }
    bRet = FALSE;
    if (charSet == SHIFTJIS_CHARSET) {
        bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "MS Mincho", lf);
    } else if (charSet == GB2312_CHARSET) {
        bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "SimSun", lf);
    } else if (charSet == CHINESEBIG5_CHARSET) {
        bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "MingLiU", lf);
    }
    if (!bRet) {
        bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "Arial Unicode MS", lf);
    }
    if (!bRet) {
        bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, "Microsoft Sans Serif", lf);
    }
    if (!bRet) {
        bRet = RetrieveSpecificFont(charSet, DEFAULT_PITCH | FF_DONTCARE, NULL, lf);
    }
    if (bRet) {
        if (pLogFont != NULL) {
            memcpy(pLogFont, &lf, sizeof(LOGFONTA));
        }
        csFontName = lf.lfFaceName;
        return csFontName;
    }
#endif
    return csFontName;
}
CFX_ByteString CPDF_InterForm::GetNativeFont(FX_LPVOID pLogFont)
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    FX_BYTE charSet = GetNativeCharSet();
    return GetNativeFont(charSet, pLogFont);
#else
    return CFX_ByteString();
#endif
}
FX_BYTE CPDF_InterForm::GetNativeCharSet()
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    FX_BYTE charSet = ANSI_CHARSET;
    UINT iCodePage = ::GetACP();
    switch (iCodePage) {
        case 932:
            charSet = SHIFTJIS_CHARSET;
            break;
        case 936:
            charSet = GB2312_CHARSET;
            break;
        case 950:
            charSet = CHINESEBIG5_CHARSET;
            break;
        case 1252:
            charSet = ANSI_CHARSET;
            break;
        case 874:
            charSet = THAI_CHARSET;
            break;
        case 949:
            charSet = HANGUL_CHARSET;
            break;
        case 1200:
            charSet = ANSI_CHARSET;
            break;
        case 1250:
            charSet = EASTEUROPE_CHARSET;
            break;
        case 1251:
            charSet = RUSSIAN_CHARSET;
            break;
        case 1253:
            charSet = GREEK_CHARSET;
            break;
        case 1254:
            charSet = TURKISH_CHARSET;
            break;
        case 1255:
            charSet = HEBREW_CHARSET;
            break;
        case 1256:
            charSet = ARABIC_CHARSET;
            break;
        case 1257:
            charSet = BALTIC_CHARSET;
            break;
        case 1258:
            charSet = VIETNAMESE_CHARSET;
            break;
        case 1361:
            charSet = JOHAB_CHARSET;
            break;
    }
    return charSet;
#else
    return 0;
#endif
}
CPDF_Font* CPDF_InterForm::AddNativeFont(FX_BYTE charSet, const CPDF_Document* pDocument)
{
    if (pDocument == NULL) {
        return NULL;
    }
    CPDF_Font* pFont = NULL;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    LOGFONTA lf;
    CFX_ByteString csFontName = GetNativeFont(charSet, &lf);
    if (!csFontName.IsEmpty()) {
        if (csFontName == "Helvetica") {
            pFont = AddStandardFont(pDocument, csFontName);
        } else {
            pFont = ((CPDF_Document*)pDocument)->AddWindowsFont(&lf, FALSE, TRUE);
        }
    }
#endif
    return pFont;
}
CPDF_Font* CPDF_InterForm::AddNativeFont(const CPDF_Document* pDocument)
{
    if (pDocument == NULL) {
        return NULL;
    }
    CPDF_Font* pFont = NULL;
    FX_BYTE charSet = GetNativeCharSet();
    pFont = AddNativeFont(charSet, pDocument);
    return pFont;
}
FX_BOOL CPDF_InterForm::ValidateFieldName(CFX_WideString& csNewFieldName, int iType, const CPDF_FormField* pExcludedField, const CPDF_FormControl* pExcludedControl)
{
    if (csNewFieldName.IsEmpty()) {
        return FALSE;
    }
    int iPos = 0;
    int iLength = csNewFieldName.GetLength();
    CFX_WideString csSub;
    while (TRUE) {
        while (iPos < iLength && (csNewFieldName[iPos] == L'.' || csNewFieldName[iPos] == L' ')) {
            iPos ++;
        }
        if (iPos < iLength && !csSub.IsEmpty()) {
            csSub += L'.';
        }
        while (iPos < iLength && csNewFieldName[iPos] != L'.') {
            csSub += csNewFieldName[iPos ++];
        }
        for (int i = csSub.GetLength() - 1; i > -1; i --) {
            if (csSub[i] == L' ' || csSub[i] == L'.') {
                csSub.SetAt(i, L'\0');
            } else {
                break;
            }
        }
        FX_DWORD dwCount = m_pFieldTree->m_Root.CountFields();
        for (FX_DWORD m = 0; m < dwCount; m ++) {
            CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(m);
            if (pField == NULL) {
                continue;
            }
            if (pField == pExcludedField) {
                if (pExcludedControl != NULL) {
                    if (pField->CountControls() < 2) {
                        continue;
                    }
                } else {
                    continue;
                }
            }
            CFX_WideString csFullName = pField->GetFullName();
            int iRet = CompareFieldName(csSub, csFullName);
            if (iRet == 1) {
                if (pField->GetFieldType() != iType) {
                    return FALSE;
                }
            } else if (iRet == 2 && csSub == csNewFieldName) {
                if (csFullName[iPos] == L'.') {
                    return FALSE;
                }
            } else if (iRet == 3 && csSub == csNewFieldName) {
                if (csNewFieldName[csFullName.GetLength()] == L'.') {
                    return FALSE;
                }
            }
        }
        if (iPos >= iLength) {
            break;
        }
    }
    if (csSub.IsEmpty()) {
        return FALSE;
    }
    csNewFieldName = csSub;
    return TRUE;
}
FX_BOOL CPDF_InterForm::ValidateFieldName(CFX_WideString& csNewFieldName, int iType)
{
    return ValidateFieldName(csNewFieldName, iType, NULL, NULL);
}
FX_BOOL CPDF_InterForm::ValidateFieldName(const CPDF_FormField* pField, CFX_WideString& csNewFieldName)
{
    if (pField == NULL || csNewFieldName.IsEmpty()) {
        return FALSE;
    }
    return ValidateFieldName(csNewFieldName, ((CPDF_FormField*)pField)->GetFieldType(), pField, NULL);
}
FX_BOOL CPDF_InterForm::ValidateFieldName(const CPDF_FormControl* pControl, CFX_WideString& csNewFieldName)
{
    if (pControl == NULL || csNewFieldName.IsEmpty()) {
        return FALSE;
    }
    CPDF_FormField* pField = ((CPDF_FormControl*)pControl)->GetField();
    return ValidateFieldName(csNewFieldName, pField->GetFieldType(), pField, pControl);
}
int CPDF_InterForm::CompareFieldName(const CFX_ByteString& name1, const CFX_ByteString& name2)
{
    FX_LPCSTR ptr1 = name1, ptr2 = name2;
    if (name1.GetLength() != name2.GetLength()) {
        int i = 0;
        while (ptr1[i] == ptr2[i]) {
            i ++;
        }
        if (i == name1.GetLength()) {
            return 2;
        }
        if (i == name2.GetLength()) {
            return 3;
        }
        return 0;
    } else {
        return name1 == name2 ? 1 : 0;
    }
}
int CPDF_InterForm::CompareFieldName(const CFX_WideString& name1, const CFX_WideString& name2)
{
    FX_LPCWSTR ptr1 = name1.c_str();
    FX_LPCWSTR ptr2 = name2.c_str();
    if (name1.GetLength() != name2.GetLength()) {
        int i = 0;
        while (ptr1[i] == ptr2[i]) {
            i ++;
        }
        if (i == name1.GetLength()) {
            return 2;
        }
        if (i == name2.GetLength()) {
            return 3;
        }
        return 0;
    } else {
        return name1 == name2 ? 1 : 0;
    }
}
FX_DWORD CPDF_InterForm::CountFields(const CFX_WideString &csFieldName)
{
    if (csFieldName.IsEmpty()) {
        return (FX_DWORD)m_pFieldTree->m_Root.CountFields();
    }
    CFieldTree::_Node *pNode = m_pFieldTree->FindNode(csFieldName);
    if (pNode == NULL) {
        return 0;
    }
    return pNode->CountFields();
}
CPDF_FormField* CPDF_InterForm::GetField(FX_DWORD index, const CFX_WideString &csFieldName)
{
    if (csFieldName == L"") {
        return m_pFieldTree->m_Root.GetField(index);
    }
    CFieldTree::_Node *pNode = m_pFieldTree->FindNode(csFieldName);
    if (pNode == NULL) {
        return NULL;
    }
    return pNode->GetField(index);
}
void CPDF_InterForm::GetAllFieldNames(CFX_WideStringArray& allFieldNames)
{
    allFieldNames.RemoveAll();
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int i = 0; i < nCount; i ++) {
        CPDF_FormField *pField = m_pFieldTree->m_Root.GetField(i);
        if (pField) {
            CFX_WideString full_name = GetFullName(pField->GetFieldDict());
            allFieldNames.Add(full_name);
        }
    }
}
FX_BOOL CPDF_InterForm::IsValidFormField(const void* pField)
{
    if (pField == NULL) {
        return FALSE;
    }
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int i = 0; i < nCount; i++) {
        CPDF_FormField *pFormField = m_pFieldTree->m_Root.GetField(i);
        if (pField == pFormField) {
            return TRUE;
        }
    }
    return FALSE;
}
CPDF_FormField* CPDF_InterForm::GetFieldByDict(CPDF_Dictionary* pFieldDict) const
{
    if (pFieldDict == NULL) {
        return NULL;
    }
    CFX_WideString csWName = GetFullName(pFieldDict);
    return m_pFieldTree->GetField(csWName);
}
FX_DWORD CPDF_InterForm::CountControls(CFX_WideString csFieldName)
{
    if (csFieldName.IsEmpty()) {
        return (FX_DWORD)m_ControlMap.GetCount();
    }
    CPDF_FormField *pField = m_pFieldTree->GetField(csFieldName);
    if (pField == NULL) {
        return 0;
    }
    return pField->m_ControlList.GetSize();
}
CPDF_FormControl* CPDF_InterForm::GetControl(FX_DWORD index, CFX_WideString csFieldName)
{
    CPDF_FormField *pField = m_pFieldTree->GetField(csFieldName);
    if (pField == NULL) {
        return NULL;
    }
    if (index < (FX_DWORD)pField->m_ControlList.GetSize()) {
        return (CPDF_FormControl *)pField->m_ControlList.GetAt(index);
    }
    return NULL;
}
FX_BOOL CPDF_InterForm::IsValidFormControl(const void* pControl)
{
    if (pControl == NULL) {
        return FALSE;
    }
    FX_POSITION pos = m_ControlMap.GetStartPosition();
    while (pos) {
        CPDF_Dictionary* pWidgetDict = NULL;
        void* pFormControl = NULL;
        m_ControlMap.GetNextAssoc(pos, (FX_LPVOID&)pWidgetDict, pFormControl);
        if (pControl == pFormControl) {
            return TRUE;
        }
    }
    return FALSE;
}
int CPDF_InterForm::CountPageControls(CPDF_Page* pPage) const
{
    CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArray("Annots");
    if (pAnnotList == NULL) {
        return 0;
    }
    int count = 0;
    for (FX_DWORD i = 0; i < pAnnotList->GetCount(); i ++) {
        CPDF_Dictionary* pAnnot = pAnnotList->GetDict(i);
        if (pAnnot == NULL) {
            continue;
        }
        CPDF_FormControl* pControl;
        if (!m_ControlMap.Lookup(pAnnot, (FX_LPVOID&)pControl)) {
            continue;
        }
        count ++;
    }
    return count;
}
CPDF_FormControl* CPDF_InterForm::GetPageControl(CPDF_Page* pPage, int index) const
{
    CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArray("Annots");
    if (pAnnotList == NULL) {
        return NULL;
    }
    int count = 0;
    for (FX_DWORD i = 0; i < pAnnotList->GetCount(); i ++) {
        CPDF_Dictionary* pAnnot = pAnnotList->GetDict(i);
        if (pAnnot == NULL) {
            continue;
        }
        CPDF_FormControl* pControl;
        if (!m_ControlMap.Lookup(pAnnot, (FX_LPVOID&)pControl)) {
            continue;
        }
        if (index == count) {
            return pControl;
        }
        count ++;
    }
    return NULL;
}
CPDF_FormControl* CPDF_InterForm::GetControlAtPoint(CPDF_Page* pPage, FX_FLOAT pdf_x, FX_FLOAT pdf_y) const
{
    CPDF_Array* pAnnotList = pPage->m_pFormDict->GetArray("Annots");
    if (pAnnotList == NULL) {
        return NULL;
    }
    for (FX_DWORD i = pAnnotList->GetCount(); i > 0; i --) {
        CPDF_Dictionary* pAnnot = pAnnotList->GetDict(i - 1);
        if (pAnnot == NULL) {
            continue;
        }
        CPDF_FormControl* pControl;
        if (!m_ControlMap.Lookup(pAnnot, (FX_LPVOID&)pControl)) {
            continue;
        }
        CFX_FloatRect rect = pControl->GetRect();
        if (rect.Contains(pdf_x, pdf_y)) {
            return pControl;
        }
    }
    return NULL;
}
CPDF_FormControl* CPDF_InterForm::GetControlByDict(CPDF_Dictionary* pWidgetDict) const
{
    CPDF_FormControl* pControl = NULL;
    m_ControlMap.Lookup(pWidgetDict, (FX_LPVOID&)pControl);
    return pControl;
}
FX_DWORD CPDF_InterForm::CountInternalFields(const CFX_WideString& csFieldName) const
{
    if (m_pFormDict == NULL) {
        return 0;
    }
    CPDF_Array* pArray = m_pFormDict->GetArray("Fields");
    if (pArray == NULL) {
        return 0;
    }
    if (csFieldName.IsEmpty()) {
        return pArray->GetCount();
    } else {
        int iLength = csFieldName.GetLength();
        int iPos = 0;
        CPDF_Dictionary* pDict = NULL;
        while (pArray != NULL) {
            CFX_WideString csSub;
            if (iPos < iLength && csFieldName[iPos] == L'.') {
                iPos ++;
            }
            while (iPos < iLength && csFieldName[iPos] != L'.') {
                csSub += csFieldName[iPos ++];
            }
            int iCount = pArray->GetCount();
            FX_BOOL bFind = FALSE;
            for (int i = 0; i < iCount; i ++) {
                pDict = pArray->GetDict(i);
                if (pDict == NULL) {
                    continue;
                }
                CFX_WideString csT = pDict->GetUnicodeText("T");
                if (csT == csSub) {
                    bFind = TRUE;
                    break;
                }
            }
            if (!bFind) {
                return 0;
            }
            if (iPos >= iLength) {
                break;
            }
            pArray = pDict->GetArray("Kids");
        }
        if (pDict == NULL) {
            return 0;
        } else {
            pArray = pDict->GetArray("Kids");
            if (pArray == NULL) {
                return 1;
            } else {
                return pArray->GetCount();
            }
        }
    }
}
CPDF_Dictionary* CPDF_InterForm::GetInternalField(FX_DWORD index, const CFX_WideString& csFieldName) const
{
    if (m_pFormDict == NULL) {
        return NULL;
    }
    CPDF_Array* pArray = m_pFormDict->GetArray("Fields");
    if (pArray == NULL) {
        return 0;
    }
    if (csFieldName.IsEmpty()) {
        return pArray->GetDict(index);
    } else {
        int iLength = csFieldName.GetLength();
        int iPos = 0;
        CPDF_Dictionary* pDict = NULL;
        while (pArray != NULL) {
            CFX_WideString csSub;
            if (iPos < iLength && csFieldName[iPos] == L'.') {
                iPos ++;
            }
            while (iPos < iLength && csFieldName[iPos] != L'.') {
                csSub += csFieldName[iPos ++];
            }
            int iCount = pArray->GetCount();
            FX_BOOL bFind = FALSE;
            for (int i = 0; i < iCount; i ++) {
                pDict = pArray->GetDict(i);
                if (pDict == NULL) {
                    continue;
                }
                CFX_WideString csT = pDict->GetUnicodeText("T");
                if (csT == csSub) {
                    bFind = TRUE;
                    break;
                }
            }
            if (!bFind) {
                return NULL;
            }
            if (iPos >= iLength) {
                break;
            }
            pArray = pDict->GetArray("Kids");
        }
        if (pDict == NULL) {
            return NULL;
        } else {
            pArray = pDict->GetArray("Kids");
            if (pArray == NULL) {
                return pDict;
            } else {
                return pArray->GetDict(index);
            }
        }
    }
}
FX_BOOL CPDF_InterForm::NeedConstructAP()
{
    if (m_pFormDict == NULL) {
        return FALSE;
    }
    return m_pFormDict->GetBoolean("NeedAppearances");
}
void CPDF_InterForm::NeedConstructAP(FX_BOOL bNeedAP)
{
    if (m_pFormDict == NULL) {
        InitInterFormDict(m_pFormDict, m_pDocument);
    }
    m_pFormDict->SetAtBoolean("NeedAppearances", bNeedAP);
    m_bGenerateAP = bNeedAP;
}
int CPDF_InterForm::CountFieldsInCalculationOrder()
{
    if (m_pFormDict == NULL) {
        return 0;
    }
    CPDF_Array* pArray = m_pFormDict->GetArray("CO");
    if (pArray == NULL) {
        return 0;
    }
    return pArray->GetCount();
}
CPDF_FormField* CPDF_InterForm::GetFieldInCalculationOrder(int index)
{
    if (m_pFormDict == NULL || index < 0) {
        return NULL;
    }
    CPDF_Array* pArray = m_pFormDict->GetArray("CO");
    if (pArray == NULL) {
        return NULL;
    }
    CPDF_Object* pElement = pArray->GetElementValue(index);
    if (pElement != NULL && pElement->GetType() == PDFOBJ_DICTIONARY) {
        return GetFieldByDict((CPDF_Dictionary*)pElement);
    }
    return NULL;
}
int CPDF_InterForm::FindFieldInCalculationOrder(const CPDF_FormField* pField)
{
    if (m_pFormDict == NULL || pField == NULL) {
        return -1;
    }
    CPDF_Array* pArray = m_pFormDict->GetArray("CO");
    if (pArray == NULL) {
        return -1;
    }
    for (FX_DWORD i = 0; i < pArray->GetCount(); i ++) {
        CPDF_Object* pElement = pArray->GetElementValue(i);
        if (pElement == pField->m_pDict) {
            return i;
        }
    }
    return -1;
}
FX_DWORD CPDF_InterForm::CountFormFonts()
{
    return CountInterFormFonts(m_pFormDict);
}
CPDF_Font* CPDF_InterForm::GetFormFont(FX_DWORD index, CFX_ByteString& csNameTag)
{
    return GetInterFormFont(m_pFormDict, m_pDocument, index, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetFormFont(CFX_ByteString csNameTag)
{
    return GetInterFormFont(m_pFormDict, m_pDocument, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetFormFont(CFX_ByteString csFontName, CFX_ByteString& csNameTag)
{
    return GetInterFormFont(m_pFormDict, m_pDocument, csFontName, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetNativeFormFont(FX_BYTE charSet, CFX_ByteString& csNameTag)
{
    return GetNativeInterFormFont(m_pFormDict, m_pDocument, charSet, csNameTag);
}
CPDF_Font* CPDF_InterForm::GetNativeFormFont(CFX_ByteString& csNameTag)
{
    return GetNativeInterFormFont(m_pFormDict, m_pDocument, csNameTag);
}
FX_BOOL CPDF_InterForm::FindFormFont(const CPDF_Font* pFont, CFX_ByteString& csNameTag)
{
    return FindInterFormFont(m_pFormDict, pFont, csNameTag);
}
FX_BOOL CPDF_InterForm::FindFormFont(CFX_ByteString csFontName, CPDF_Font*& pFont, CFX_ByteString& csNameTag)
{
    return FindInterFormFont(m_pFormDict, m_pDocument, csFontName, pFont, csNameTag);
}
void CPDF_InterForm::AddFormFont(const CPDF_Font* pFont, CFX_ByteString& csNameTag)
{
    AddInterFormFont(m_pFormDict, m_pDocument, pFont, csNameTag);
    m_bUpdated = TRUE;
}
CPDF_Font* CPDF_InterForm::AddNativeFormFont(FX_BYTE charSet, CFX_ByteString& csNameTag)
{
    m_bUpdated = TRUE;
    return AddNativeInterFormFont(m_pFormDict, m_pDocument, charSet, csNameTag);
}
CPDF_Font* CPDF_InterForm::AddNativeFormFont(CFX_ByteString& csNameTag)
{
    m_bUpdated = TRUE;
    return AddNativeInterFormFont(m_pFormDict, m_pDocument, csNameTag);
}
void CPDF_InterForm::RemoveFormFont(const CPDF_Font* pFont)
{
    m_bUpdated = TRUE;
    RemoveInterFormFont(m_pFormDict, pFont);
}
void CPDF_InterForm::RemoveFormFont(CFX_ByteString csNameTag)
{
    m_bUpdated = TRUE;
    RemoveInterFormFont(m_pFormDict, csNameTag);
}
CPDF_DefaultAppearance CPDF_InterForm::GetDefaultAppearance()
{
    CFX_ByteString csDA;
    if (m_pFormDict == NULL) {
        return csDA;
    }
    csDA = m_pFormDict->GetString("DA");
    return csDA;
}
CPDF_Font* CPDF_InterForm::GetDefaultFormFont()
{
    return GetDefaultInterFormFont(m_pFormDict, m_pDocument);
}
int CPDF_InterForm::GetFormAlignment()
{
    if (m_pFormDict == NULL) {
        return 0;
    }
    return m_pFormDict->GetInteger("Q", 0);
}
FX_BOOL CPDF_InterForm::ResetForm(const CFX_PtrArray& fields, FX_BOOL bIncludeOrExclude, FX_BOOL bNotify)
{
    if (bNotify && m_pFormNotify != NULL) {
        int iRet = m_pFormNotify->BeforeFormReset(this);
        if (iRet < 0) {
            return FALSE;
        }
    }
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int i = 0; i < nCount; i++) {
        CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
        if (pField == NULL) {
            continue;
        }
        FX_BOOL bFind = FALSE;
        int iCount = fields.GetSize();
        for (int i = 0; i < iCount; i ++) {
            if (pField == (CPDF_FormField*)fields[i]) {
                bFind = TRUE;
                break;
            }
        }
        if ((bIncludeOrExclude && bFind) || (!bIncludeOrExclude && !bFind)) {
            pField->ResetField(bNotify);
        }
    }
    if (bNotify && m_pFormNotify != NULL) {
        m_pFormNotify->AfterFormReset(this);
    }
    return TRUE;
}
FX_BOOL CPDF_InterForm::ResetForm(FX_BOOL bNotify)
{
    if (bNotify && m_pFormNotify != NULL) {
        int iRet = m_pFormNotify->BeforeFormReset(this);
        if (iRet < 0) {
            return FALSE;
        }
    }
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int i = 0; i < nCount; i++) {
        CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
        if (pField == NULL) {
            continue;
        }
        pField->ResetField(bNotify);
    }
    if (bNotify && m_pFormNotify != NULL) {
        m_pFormNotify->AfterFormReset(this);
    }
    return TRUE;
}
void CPDF_InterForm::ReloadForm()
{
    FX_POSITION pos = m_ControlMap.GetStartPosition();
    while (pos) {
        CPDF_Dictionary* pWidgetDict;
        CPDF_FormControl* pControl;
        m_ControlMap.GetNextAssoc(pos, (FX_LPVOID&)pWidgetDict, (FX_LPVOID&)pControl);
        delete pControl;
    }
    m_ControlMap.RemoveAll();
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int k = 0; k < nCount; k ++) {
        CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(k);
        delete pField;
    }
    m_pFieldTree->RemoveAll();
    if (m_pFormDict == NULL) {
        return;
    }
    CPDF_Array* pFields = m_pFormDict->GetArray("Fields");
    if (pFields == NULL) {
        return;
    }
    int iCount = pFields->GetCount();
    for (int i = 0; i < iCount; i ++) {
        LoadField(pFields->GetDict(i));
    }
}
void CPDF_InterForm::LoadField(CPDF_Dictionary* pFieldDict, int nLevel)
{
    if (nLevel > nMaxRecursion) {
        return;
    }
    if (pFieldDict == NULL) {
        return;
    }
    FX_DWORD dwParentObjNum = pFieldDict->GetObjNum();
    CPDF_Array* pKids = pFieldDict->GetArray("Kids");
    if (!pKids) {
        AddTerminalField(pFieldDict);
        return;
    }
    CPDF_Dictionary* pFirstKid = pKids->GetDict(0);
    if (pFirstKid == NULL) {
        return;
    }
    if (pFirstKid->KeyExist("T") || pFirstKid->KeyExist("Kids")) {
        for (FX_DWORD i = 0; i < pKids->GetCount(); i ++) {
            CPDF_Dictionary * pChildDict = pKids->GetDict(i);
            if (pChildDict) {
                if (pChildDict->GetObjNum() != dwParentObjNum) {
                    LoadField(pChildDict, nLevel + 1);
                }
            }
        }
    } else {
        AddTerminalField(pFieldDict);
    }
}
FX_BOOL CPDF_InterForm::HasXFAForm() const
{
    return m_pFormDict && m_pFormDict->GetArray(FX_BSTRC("XFA")) != NULL;
}
void CPDF_InterForm::FixPageFields(const CPDF_Page* pPage)
{
    ASSERT(pPage != NULL);
    CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
    if (pPageDict == NULL) {
        return;
    }
    CPDF_Array* pAnnots = pPageDict->GetArray(FX_BSTRC("Annots"));
    if (pAnnots == NULL) {
        return;
    }
    int iAnnotCount = pAnnots->GetCount();
    for (int i = 0; i < iAnnotCount; i++) {
        CPDF_Dictionary* pAnnot = pAnnots->GetDict(i);
        if (pAnnot != NULL && pAnnot->GetString(FX_BSTRC("Subtype")) == "Widget") {
            LoadField(pAnnot);
        }
    }
}
CPDF_FormField* CPDF_InterForm::AddTerminalField(const CPDF_Dictionary* pFieldDict)
{
    if (!pFieldDict->KeyExist(FX_BSTRC("T"))) {
        return NULL;
    }
    CPDF_Dictionary* pDict = (CPDF_Dictionary*)pFieldDict;
    CFX_WideString csWName = GetFullName(pDict);
    if (csWName.IsEmpty()) {
        return NULL;
    }
    CPDF_FormField* pField = NULL;
    pField = m_pFieldTree->GetField(csWName);
    if (pField == NULL) {
        CPDF_Dictionary *pParent = (CPDF_Dictionary*)pFieldDict;
        if (!pFieldDict->KeyExist(FX_BSTR("T")) &&
                pFieldDict->GetString(FX_BSTRC("Subtype")) == FX_BSTRC("Widget")) {
            pParent = pFieldDict->GetDict(FX_BSTRC("Parent"));
            if (!pParent) {
                pParent = (CPDF_Dictionary*)pFieldDict;
            }
        }
        if (pParent && pParent != pFieldDict && !pParent->KeyExist(FX_BSTRC("FT"))) {
            if (pFieldDict->KeyExist(FX_BSTRC("FT"))) {
                CPDF_Object *pFTValue = pFieldDict->GetElementValue(FX_BSTRC("FT"));
                if (pFTValue) {
                    pParent->SetAt(FX_BSTRC("FT"), pFTValue->Clone());
                }
            }
            if (pFieldDict->KeyExist(FX_BSTRC("Ff"))) {
                CPDF_Object *pFfValue = pFieldDict->GetElementValue(FX_BSTRC("Ff"));
                if (pFfValue) {
                    pParent->SetAt(FX_BSTRC("Ff"), pFfValue->Clone());
                }
            }
        }
        pField = new CPDF_FormField(this, pParent);
        CPDF_Object* pTObj = pDict->GetElement("T");
        if (pTObj && pTObj->GetType() == PDFOBJ_REFERENCE) {
            CPDF_Object* pClone = pTObj->Clone(TRUE);
            if (pClone) {
                pDict->SetAt("T", pClone);
            } else {
                pDict->SetAtName("T", "");
            }
        }
        m_pFieldTree->SetField(csWName, pField);
    }
    CPDF_Array* pKids = pFieldDict->GetArray("Kids");
    if (pKids == NULL) {
        if (pFieldDict->GetString("Subtype") == "Widget") {
            AddControl(pField, pFieldDict);
        }
    } else {
        for (FX_DWORD i = 0; i < pKids->GetCount(); i ++) {
            CPDF_Dictionary* pKid = pKids->GetDict(i);
            if (pKid == NULL) {
                continue;
            }
            if (pKid->GetString("Subtype") != "Widget") {
                continue;
            }
            AddControl(pField, pKid);
        }
    }
    return pField;
}
CPDF_FormControl* CPDF_InterForm::AddControl(const CPDF_FormField* pField, const CPDF_Dictionary* pWidgetDict)
{
    void *rValue = NULL;
    if (m_ControlMap.Lookup((CPDF_Dictionary*)pWidgetDict, rValue)) {
        return (CPDF_FormControl*)rValue;
    }
    CPDF_FormControl* pControl = new CPDF_FormControl((CPDF_FormField*)pField, (CPDF_Dictionary*)pWidgetDict);
    m_ControlMap.SetAt((CPDF_Dictionary*)pWidgetDict, pControl);
    ((CPDF_FormField*)pField)->m_ControlList.Add(pControl);
    return pControl;
}
CPDF_FormField* CPDF_InterForm::CheckRequiredFields(const CFX_PtrArray *fields, FX_BOOL bIncludeOrExclude) const
{
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int i = 0; i < nCount; i++) {
        CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
        if (pField == NULL) {
            continue;
        }
        FX_INT32 iType = pField->GetType();
        if (iType == CPDF_FormField::PushButton || iType == CPDF_FormField::CheckBox || iType == CPDF_FormField::ListBox) {
            continue;
        }
        FX_DWORD dwFlags = pField->GetFieldFlags();
        if (dwFlags & 0x04) {
            continue;
        }
        FX_BOOL bFind = TRUE;
        if (fields != NULL) {
            bFind = fields->Find(pField, 0) >= 0;
        }
        if ((bIncludeOrExclude && bFind) || (!bIncludeOrExclude && !bFind)) {
            CPDF_Dictionary *pFieldDict = pField->m_pDict;
            if ((dwFlags & 0x02) != 0 && pFieldDict->GetString("V").IsEmpty()) {
                return pField;
            }
        }
    }
    return NULL;
}
CFDF_Document* CPDF_InterForm::ExportToFDF(FX_WSTR pdf_path, FX_BOOL bSimpleFileSpec) const
{
    CFX_PtrArray fields;
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int i = 0; i < nCount; i ++) {
        CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
        fields.Add(pField);
    }
    return ExportToFDF(pdf_path, fields, TRUE, bSimpleFileSpec);
}
CFX_WideString FILESPEC_EncodeFileName(FX_WSTR filepath);
CFDF_Document* CPDF_InterForm::ExportToFDF(FX_WSTR pdf_path, CFX_PtrArray& fields, FX_BOOL bIncludeOrExclude, FX_BOOL bSimpleFileSpec) const
{
    CFDF_Document* pDoc = CFDF_Document::CreateNewDoc();
    if (pDoc == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pMainDict = pDoc->GetRoot()->GetDict("FDF");
    if (!pdf_path.IsEmpty()) {
        if (bSimpleFileSpec) {
            CFX_WideString wsFilePath = FILESPEC_EncodeFileName(pdf_path);
            pMainDict->SetAtString(FX_BSTRC("F"), CFX_ByteString::FromUnicode(wsFilePath));
            pMainDict->SetAtString(FX_BSTRC("UF"), PDF_EncodeText(wsFilePath));
        } else {
            CPDF_FileSpec filespec;
            filespec.SetFileName(pdf_path);
            pMainDict->SetAt("F", (CPDF_Object*)filespec);
        }
    }
    CPDF_Array* pFields = CPDF_Array::Create();
    if (pFields == NULL) {
        return NULL;
    }
    pMainDict->SetAt("Fields", pFields);
    int nCount = m_pFieldTree->m_Root.CountFields();
    for (int i = 0; i < nCount; i ++) {
        CPDF_FormField* pField = m_pFieldTree->m_Root.GetField(i);
        if (pField == NULL || pField->GetType() == CPDF_FormField::PushButton) {
            continue;
        }
        FX_DWORD dwFlags = pField->GetFieldFlags();
        if (dwFlags & 0x04) {
            continue;
        }
        FX_BOOL bFind = fields.Find(pField, 0) >= 0;
        if ((bIncludeOrExclude && bFind) || (!bIncludeOrExclude && !bFind)) {
            if ((dwFlags & 0x02) != 0 && pField->m_pDict->GetString("V").IsEmpty()) {
                continue;
            }
            CFX_WideString fullname = GetFullName(pField->GetFieldDict());
            CPDF_Dictionary* pFieldDict = CPDF_Dictionary::Create();
            if (pFieldDict == NULL) {
                return NULL;
            }
            CPDF_String* pString = CPDF_String::Create(fullname);
            if (pString == NULL) {
                pFieldDict->Release();
                return NULL;
            }
            pFieldDict->SetAt("T", pString);
            if (pField->GetType() == CPDF_FormField::CheckBox || pField->GetType() == CPDF_FormField::RadioButton) {
                CFX_WideString csExport = pField->GetCheckValue(FALSE);
                CFX_ByteString csBExport = PDF_EncodeText(csExport);
                CPDF_Object* pOpt = FPDF_GetFieldAttr(pField->m_pDict, "Opt");
                if (pOpt == NULL) {
                    pFieldDict->SetAtName("V", csBExport);
                } else {
                    pFieldDict->SetAtString("V", csBExport);
                }
            } else {
                CPDF_Object* pV = FPDF_GetFieldAttr(pField->m_pDict, "V");
                if (pV != NULL) {
                    pFieldDict->SetAt("V", pV->Clone(TRUE));
                }
            }
            pFields->Add(pFieldDict);
        }
    }
    return pDoc;
}
const struct _SupportFieldEncoding {
    FX_LPCSTR m_name;
    FX_INT32 m_codePage;
} g_fieldEncoding[] = {
    { "BigFive", 950 },
    { "GBK", 936 },
    { "Shift-JIS", 932 },
    { "UHC", 949 },
};
static void FPDFDOC_FDF_GetFieldValue(CPDF_Dictionary *pFieldDict, CFX_WideString &csValue, CFX_ByteString &bsEncoding)
{
    ASSERT(pFieldDict != NULL);
    CFX_ByteString csBValue = pFieldDict->GetString("V");
    FX_INT32 iCount = sizeof(g_fieldEncoding) / sizeof(g_fieldEncoding[0]);
    FX_INT32 i = 0;
    for (; i < iCount; ++i)
        if (bsEncoding == g_fieldEncoding[i].m_name) {
            break;
        }
    if (i < iCount) {
        CFX_CharMap *pCharMap = CFX_CharMap::GetDefaultMapper(g_fieldEncoding[i].m_codePage);
        FXSYS_assert(pCharMap != NULL);
        csValue.ConvertFrom(csBValue, pCharMap);
        return;
    }
    CFX_ByteString csTemp = csBValue.Left(2);
    if (csTemp == "\xFF\xFE" || csTemp == "\xFE\xFF") {
        csValue = PDF_DecodeText(csBValue);
    } else {
        csValue = CFX_WideString::FromLocal(csBValue);
    }
}
void CPDF_InterForm::FDF_ImportField(CPDF_Dictionary* pFieldDict, const CFX_WideString& parent_name, FX_BOOL bNotify, int nLevel)
{
    CFX_WideString name;
    if (!parent_name.IsEmpty()) {
        name = parent_name + L".";
    }
    name += pFieldDict->GetUnicodeText("T");
    CPDF_Array* pKids = pFieldDict->GetArray("Kids");
    if (pKids) {
        for (FX_DWORD i = 0; i < pKids->GetCount(); i ++) {
            CPDF_Dictionary* pKid = pKids->GetDict(i);
            if (pKid == NULL) {
                continue;
            }
            if (nLevel <= nMaxRecursion) {
                FDF_ImportField(pKid, name, bNotify, nLevel + 1);
            }
        }
        return;
    }
    if (!pFieldDict->KeyExist("V")) {
        return;
    }
    CPDF_FormField* pField = m_pFieldTree->GetField(name);
    if (pField == NULL) {
        return;
    }
    CFX_WideString csWValue;
    FPDFDOC_FDF_GetFieldValue(pFieldDict, csWValue, m_bsEncoding);
    int iType = pField->GetFieldType();
    if (bNotify && m_pFormNotify != NULL) {
        int iRet = 0;
        if (iType == FIELDTYPE_LISTBOX) {
            iRet = m_pFormNotify->BeforeSelectionChange(pField, csWValue);
        } else if (iType == FIELDTYPE_COMBOBOX || iType == FIELDTYPE_TEXTFIELD) {
            iRet = m_pFormNotify->BeforeValueChange(pField, csWValue);
        }
        if (iRet < 0) {
            return;
        }
    }
    CFX_ByteArray statusArray;
    if (iType == FIELDTYPE_CHECKBOX || iType == FIELDTYPE_RADIOBUTTON) {
        SaveCheckedFieldStatus(pField, statusArray);
    }
    pField->SetValue(csWValue);
    CPDF_FormField::Type eType = pField->GetType();
    if ((eType == CPDF_FormField::ListBox || eType == CPDF_FormField::ComboBox) && pFieldDict->KeyExist("Opt")) {
        pField->m_pDict->SetAt("Opt", pFieldDict->GetElementValue("Opt")->Clone(TRUE));
    }
    if (bNotify && m_pFormNotify != NULL) {
        if (iType == FIELDTYPE_CHECKBOX || iType == FIELDTYPE_RADIOBUTTON) {
            m_pFormNotify->AfterCheckedStatusChange(pField, statusArray);
        } else if (iType == FIELDTYPE_LISTBOX) {
            m_pFormNotify->AfterSelectionChange(pField);
        } else if (iType == FIELDTYPE_COMBOBOX || iType == FIELDTYPE_TEXTFIELD) {
            m_pFormNotify->AfterValueChange(pField);
        }
    }
    if (CPDF_InterForm::m_bUpdateAP) {
        pField->UpdateAP(NULL);
    }
}
FX_BOOL CPDF_InterForm::ImportFromFDF(const CFDF_Document* pFDF, FX_BOOL bNotify)
{
    if (pFDF == NULL) {
        return FALSE;
    }
    CPDF_Dictionary* pMainDict = pFDF->GetRoot()->GetDict("FDF");
    if (pMainDict == NULL) {
        return FALSE;
    }
    CPDF_Array* pFields = pMainDict->GetArray("Fields");
    if (pFields == NULL) {
        return FALSE;
    }
    m_bsEncoding = pMainDict->GetString(FX_BSTRC("Encoding"));
    if (bNotify && m_pFormNotify != NULL) {
        int iRet = m_pFormNotify->BeforeFormImportData(this);
        if (iRet < 0) {
            return FALSE;
        }
    }
    for (FX_DWORD i = 0; i < pFields->GetCount(); i ++) {
        CPDF_Dictionary* pField = pFields->GetDict(i);
        if (pField == NULL) {
            continue;
        }
        FDF_ImportField(pField, L"", bNotify);
    }
    if (bNotify && m_pFormNotify != NULL) {
        m_pFormNotify->AfterFormImportData(this);
    }
    return TRUE;
}
void CPDF_InterForm::SetFormNotify(const CPDF_FormNotify* pNotify)
{
    m_pFormNotify = (CPDF_FormNotify*)pNotify;
}
int CPDF_InterForm::GetPageWithWidget(int iCurPage, FX_BOOL bNext)
{
    if (iCurPage < 0) {
        return -1;
    }
    int iPageCount = m_pDocument->GetPageCount();
    if (iCurPage >= iPageCount) {
        return -1;
    }
    int iNewPage = iCurPage;
    do {
        iNewPage += bNext ? 1 : -1;
        if (iNewPage >= iPageCount) {
            iNewPage = 0;
        }
        if (iNewPage < 0) {
            iNewPage = iPageCount - 1;
        }
        if (iNewPage == iCurPage) {
            break;
        }
        CPDF_Dictionary* pPageDict = m_pDocument->GetPage(iNewPage);
        if (pPageDict == NULL) {
            continue;
        }
        CPDF_Array* pAnnots = pPageDict->GetArray("Annots");
        if (pAnnots == NULL) {
            continue;
        }
        FX_DWORD dwCount = pAnnots->GetCount();
        for (FX_DWORD i = 0; i < dwCount; i ++) {
            CPDF_Object* pAnnotDict = pAnnots->GetElementValue(i);
            if (pAnnotDict == NULL) {
                continue;
            }
            CPDF_FormControl* pControl = NULL;
            if (m_ControlMap.Lookup(pAnnotDict, (void*&)pControl)) {
                return iNewPage;
            }
        }
    } while (TRUE);
    return -1;
}
