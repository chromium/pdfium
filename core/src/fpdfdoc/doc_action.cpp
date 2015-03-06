// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
CPDF_Dest CPDF_Action::GetDest(CPDF_Document* pDoc) const
{
    if (!m_pDict) {
        return CPDF_Dest();
    }
    CFX_ByteString type = m_pDict->GetString("S");
    if (type != "GoTo" && type != "GoToR") {
        return CPDF_Dest();
    }
    CPDF_Object* pDest = m_pDict->GetElementValue("D");
    if (!pDest) {
        return CPDF_Dest();
    }
    if (pDest->GetType() == PDFOBJ_STRING || pDest->GetType() == PDFOBJ_NAME) {
        CPDF_NameTree name_tree(pDoc, FX_BSTRC("Dests"));
        CFX_ByteStringC name = pDest->GetString();
        return CPDF_Dest(name_tree.LookupNamedDest(pDoc, name));
    }
    if (pDest->GetType() == PDFOBJ_ARRAY) {
        return CPDF_Dest((CPDF_Array*)pDest);
    }
    return CPDF_Dest();
}
const FX_CHAR* g_sATypes[] = {"Unknown", "GoTo", "GoToR", "GoToE", "Launch", "Thread", "URI", "Sound", "Movie",
                              "Hide",	"Named", "SubmitForm", "ResetForm", "ImportData", "JavaScript", "SetOCGState",
                              "Rendition", "Trans", "GoTo3DView", ""
                             };
CPDF_Action::ActionType CPDF_Action::GetType() const
{
    ActionType eType = Unknown;
    if (m_pDict != NULL) {
        CFX_ByteString csType = m_pDict->GetString("S");
        if (!csType.IsEmpty()) {
            int i = 0;
            while (g_sATypes[i][0] != '\0') {
                if (csType == g_sATypes[i]) {
                    return (ActionType)i;
                }
                i ++;
            }
        }
    }
    return eType;
}
CFX_WideString CPDF_Action::GetFilePath() const
{
    CFX_ByteString type = m_pDict->GetString("S");
    if (type != "GoToR" && type != "Launch" &&
            type != "SubmitForm" && type != "ImportData") {
        return CFX_WideString();
    }
    CPDF_Object* pFile = m_pDict->GetElementValue("F");
    CFX_WideString path;
    if (pFile == NULL) {
        if (type == "Launch") {
            CPDF_Dictionary* pWinDict = m_pDict->GetDict(FX_BSTRC("Win"));
            if (pWinDict) {
                return CFX_WideString::FromLocal(pWinDict->GetString(FX_BSTRC("F")));
            }
        }
        return path;
    }
    CPDF_FileSpec filespec(pFile);
    filespec.GetFileName(path);
    return path;
}
CFX_ByteString CPDF_Action::GetURI(CPDF_Document* pDoc) const
{
    CFX_ByteString csURI;
    if (m_pDict == NULL) {
        return csURI;
    }
    if (m_pDict->GetString("S") != "URI") {
        return csURI;
    }
    csURI = m_pDict->GetString("URI");
    CPDF_Dictionary* pRoot = pDoc->GetRoot();
    CPDF_Dictionary* pURI = pRoot->GetDict("URI");
    if (pURI != NULL) {
        if (csURI.Find(FX_BSTRC(":"), 0) < 1) {
            csURI = pURI->GetString("Base") + csURI;
        }
    }
    return csURI;
}
FX_DWORD CPDF_ActionFields::GetFieldsCount() const
{
    if (m_pAction == NULL) {
        return 0;
    }
    CPDF_Dictionary* pDict = m_pAction->GetDict();
    if (pDict == NULL) {
        return 0;
    }
    CFX_ByteString csType = pDict->GetString("S");
    CPDF_Object* pFields = NULL;
    if (csType == "Hide") {
        pFields = pDict->GetElementValue("T");
    } else {
        pFields = pDict->GetArray("Fields");
    }
    if (pFields == NULL) {
        return 0;
    }
    int iType = pFields->GetType();
    if (iType == PDFOBJ_DICTIONARY) {
        return 1;
    } else if (iType == PDFOBJ_STRING) {
        return 1;
    } else if (iType == PDFOBJ_ARRAY) {
        return ((CPDF_Array*)pFields)->GetCount();
    }
    return 0;
}
void CPDF_ActionFields::GetAllFields(CFX_PtrArray& fieldObjects) const
{
    fieldObjects.RemoveAll();
    if (m_pAction == NULL) {
        return;
    }
    CPDF_Dictionary* pDict = m_pAction->GetDict();
    if (pDict == NULL) {
        return;
    }
    CFX_ByteString csType = pDict->GetString("S");
    CPDF_Object* pFields = NULL;
    if (csType == "Hide") {
        pFields = pDict->GetElementValue("T");
    } else {
        pFields = pDict->GetArray("Fields");
    }
    if (pFields == NULL) {
        return;
    }
    int iType = pFields->GetType();
    if (iType == PDFOBJ_DICTIONARY || iType == PDFOBJ_STRING) {
        fieldObjects.Add(pFields);
    } else if (iType == PDFOBJ_ARRAY) {
        CPDF_Array* pArray = (CPDF_Array*)pFields;
        FX_DWORD iCount = pArray->GetCount();
        for (FX_DWORD i = 0; i < iCount; i ++) {
            CPDF_Object* pObj = pArray->GetElementValue(i);
            if (pObj != NULL) {
                fieldObjects.Add(pObj);
            }
        }
    }
}
CPDF_Object* CPDF_ActionFields::GetField(FX_DWORD iIndex) const
{
    if (m_pAction == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pDict = m_pAction->GetDict();
    if (pDict == NULL) {
        return NULL;
    }
    CFX_ByteString csType = pDict->GetString("S");
    CPDF_Object* pFields = NULL;
    if (csType == "Hide") {
        pFields = pDict->GetElementValue("T");
    } else {
        pFields = pDict->GetArray("Fields");
    }
    if (pFields == NULL) {
        return NULL;
    }
    CPDF_Object* pFindObj = NULL;
    int iType = pFields->GetType();
    if (iType == PDFOBJ_DICTIONARY || iType == PDFOBJ_STRING) {
        if (iIndex == 0) {
            pFindObj = pFields;
        }
    } else if (iType == PDFOBJ_ARRAY) {
        pFindObj = ((CPDF_Array*)pFields)->GetElementValue(iIndex);
    }
    return pFindObj;
}
CPDF_LWinParam CPDF_Action::GetWinParam() const
{
    if (m_pDict == NULL) {
        return NULL;
    }
    if (m_pDict->GetString("S") != "Launch") {
        return NULL;
    }
    return m_pDict->GetDict("Win");
}
CFX_WideString CPDF_Action::GetJavaScript() const
{
    CFX_WideString csJS;
    if (m_pDict == NULL) {
        return csJS;
    }
    CPDF_Object* pJS = m_pDict->GetElementValue("JS");
    if (pJS != NULL) {
        return pJS->GetUnicodeText();
    }
    return csJS;
}
CPDF_Dictionary* CPDF_Action::GetAnnot() const
{
    if (m_pDict == NULL) {
        return NULL;
    }
    CFX_ByteString csType = m_pDict->GetString("S");
    if (csType == FX_BSTRC("Rendition")) {
        return m_pDict->GetDict("AN");
    } else if (csType == FX_BSTRC("Movie")) {
        return m_pDict->GetDict("Annotation");
    }
    return NULL;
}
FX_INT32 CPDF_Action::GetOperationType() const
{
    if (m_pDict == NULL) {
        return 0;
    }
    CFX_ByteString csType = m_pDict->GetString("S");
    if (csType == FX_BSTRC("Rendition")) {
        return m_pDict->GetInteger("OP");
    } else if (csType == FX_BSTRC("Movie")) {
        CFX_ByteString csOP = m_pDict->GetString("Operation");
        if (csOP == FX_BSTRC("Play")) {
            return 0;
        } else if (csOP == FX_BSTRC("Stop")) {
            return 1;
        } else if (csOP == FX_BSTRC("Pause")) {
            return 2;
        } else if (csOP == FX_BSTRC("Resume")) {
            return 3;
        }
    }
    return 0;
}
FX_DWORD CPDF_Action::GetSubActionsCount() const
{
    if (m_pDict == NULL || !m_pDict->KeyExist("Next")) {
        return 0;
    }
    CPDF_Object* pNext = m_pDict->GetElementValue("Next");
    if (!pNext) {
        return 0;
    }
    int iObjType = pNext->GetType();
    if (iObjType == PDFOBJ_DICTIONARY) {
        return 1;
    }
    if (iObjType == PDFOBJ_ARRAY) {
        return ((CPDF_Array*)pNext)->GetCount();
    }
    return 0;
}
CPDF_Action CPDF_Action::GetSubAction(FX_DWORD iIndex) const
{
    if (m_pDict == NULL || !m_pDict->KeyExist("Next")) {
        return CPDF_Action();
    }
    CPDF_Object* pNext = m_pDict->GetElementValue("Next");
    int iObjType = pNext->GetType();
    if (iObjType == PDFOBJ_DICTIONARY) {
        CPDF_Dictionary *pDict = static_cast<CPDF_Dictionary*>(pNext);
        if (iIndex == 0) {
            return CPDF_Action(pDict);
        }
    } else if (iObjType == PDFOBJ_ARRAY) {
        CPDF_Array* pArray = static_cast<CPDF_Array*>(pNext);
        return CPDF_Action(pArray->GetDict(iIndex));
    }
    return CPDF_Action();
}
const FX_CHAR* g_sAATypes[] = {"E", "X", "D", "U", "Fo", "Bl", "PO", "PC", "PV", "PI",
                               "O", "C",
                               "K", "F", "V", "C",
                               "WC", "WS", "DS", "WP", "DP",
                               ""
                              };
FX_BOOL CPDF_AAction::ActionExist(AActionType eType) const
{
    if (m_pDict == NULL) {
        return FALSE;
    }
    return m_pDict->KeyExist(g_sAATypes[(int)eType]);
}
CPDF_Action CPDF_AAction::GetAction(AActionType eType) const
{
    if (!m_pDict) {
        return CPDF_Action();
    }
    return CPDF_Action(m_pDict->GetDict(g_sAATypes[(int)eType]));
}
FX_POSITION CPDF_AAction::GetStartPos() const
{
    if (m_pDict == NULL) {
        return NULL;
    }
    return m_pDict->GetStartPos();
}
CPDF_Action CPDF_AAction::GetNextAction(FX_POSITION& pos, AActionType& eType) const
{
    if (m_pDict == NULL) {
        return CPDF_Action();
    }
    CFX_ByteString csKey;
    CPDF_Object* pObj = m_pDict->GetNextElement(pos, csKey);
    if (!pObj) {
        return CPDF_Action();
    }
    CPDF_Object* pDirect = pObj->GetDirect();
    if (!pDirect || pDirect->GetType() != PDFOBJ_DICTIONARY) {
        return CPDF_Action();
    }
    int i = 0;
    while (g_sAATypes[i][0] != '\0') {
        if (csKey == g_sAATypes[i]) {
            break;
        }
        i++;
    }
    eType = (AActionType)i;
    return CPDF_Action(static_cast<CPDF_Dictionary*>(pDirect));
}
CPDF_DocJSActions::CPDF_DocJSActions(CPDF_Document* pDoc)
{
    m_pDocument = pDoc;
}
int CPDF_DocJSActions::CountJSActions() const
{
    ASSERT(m_pDocument != NULL);
    CPDF_NameTree name_tree(m_pDocument, FX_BSTRC("JavaScript"));
    return name_tree.GetCount();
}
CPDF_Action CPDF_DocJSActions::GetJSAction(int index, CFX_ByteString& csName) const
{
    ASSERT(m_pDocument != NULL);
    CPDF_NameTree name_tree(m_pDocument, FX_BSTRC("JavaScript"));
    CPDF_Object *pAction = name_tree.LookupValue(index, csName);
    if (pAction == NULL || pAction->GetType() != PDFOBJ_DICTIONARY) {
        return CPDF_Action();
    }
    return CPDF_Action(pAction->GetDict());
}
CPDF_Action CPDF_DocJSActions::GetJSAction(const CFX_ByteString& csName) const
{
    ASSERT(m_pDocument != NULL);
    CPDF_NameTree name_tree(m_pDocument, FX_BSTRC("JavaScript"));
    CPDF_Object *pAction = name_tree.LookupValue(csName);
    if (pAction == NULL || pAction->GetType() != PDFOBJ_DICTIONARY) {
        return CPDF_Action();
    }
    return CPDF_Action(pAction->GetDict());
}
int CPDF_DocJSActions::FindJSAction(const CFX_ByteString& csName) const
{
    ASSERT(m_pDocument != NULL);
    CPDF_NameTree name_tree(m_pDocument, FX_BSTRC("JavaScript"));
    return name_tree.GetIndex(csName);
}
