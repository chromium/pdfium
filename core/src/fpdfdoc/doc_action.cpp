// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfdoc/fpdf_doc.h"

CPDF_Dest CPDF_Action::GetDest(CPDF_Document* pDoc) const {
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
  if (pDest->IsString() || pDest->IsName()) {
    CPDF_NameTree name_tree(pDoc, "Dests");
    CFX_ByteStringC name = pDest->GetString();
    return CPDF_Dest(name_tree.LookupNamedDest(pDoc, name));
  }
  if (CPDF_Array* pArray = pDest->AsArray())
    return CPDF_Dest(pArray);
  return CPDF_Dest();
}
const FX_CHAR* g_sATypes[] = {
    "Unknown",     "GoTo",       "GoToR",     "GoToE",      "Launch",
    "Thread",      "URI",        "Sound",     "Movie",      "Hide",
    "Named",       "SubmitForm", "ResetForm", "ImportData", "JavaScript",
    "SetOCGState", "Rendition",  "Trans",     "GoTo3DView", ""};
CPDF_Action::ActionType CPDF_Action::GetType() const {
  ActionType eType = Unknown;
  if (m_pDict) {
    CFX_ByteString csType = m_pDict->GetString("S");
    if (!csType.IsEmpty()) {
      int i = 0;
      while (g_sATypes[i][0] != '\0') {
        if (csType == g_sATypes[i]) {
          return (ActionType)i;
        }
        i++;
      }
    }
  }
  return eType;
}
CFX_WideString CPDF_Action::GetFilePath() const {
  CFX_ByteString type = m_pDict->GetString("S");
  if (type != "GoToR" && type != "Launch" && type != "SubmitForm" &&
      type != "ImportData") {
    return CFX_WideString();
  }
  CPDF_Object* pFile = m_pDict->GetElementValue("F");
  CFX_WideString path;
  if (!pFile) {
    if (type == "Launch") {
      CPDF_Dictionary* pWinDict = m_pDict->GetDict("Win");
      if (pWinDict) {
        return CFX_WideString::FromLocal(pWinDict->GetString("F"));
      }
    }
    return path;
  }
  CPDF_FileSpec filespec(pFile);
  filespec.GetFileName(path);
  return path;
}
CFX_ByteString CPDF_Action::GetURI(CPDF_Document* pDoc) const {
  CFX_ByteString csURI;
  if (!m_pDict) {
    return csURI;
  }
  if (m_pDict->GetString("S") != "URI") {
    return csURI;
  }
  csURI = m_pDict->GetString("URI");
  CPDF_Dictionary* pRoot = pDoc->GetRoot();
  CPDF_Dictionary* pURI = pRoot->GetDict("URI");
  if (pURI) {
    if (csURI.Find(":", 0) < 1) {
      csURI = pURI->GetString("Base") + csURI;
    }
  }
  return csURI;
}
FX_DWORD CPDF_ActionFields::GetFieldsCount() const {
  if (!m_pAction) {
    return 0;
  }
  CPDF_Dictionary* pDict = m_pAction->GetDict();
  if (!pDict) {
    return 0;
  }
  CFX_ByteString csType = pDict->GetString("S");
  CPDF_Object* pFields = NULL;
  if (csType == "Hide") {
    pFields = pDict->GetElementValue("T");
  } else {
    pFields = pDict->GetArray("Fields");
  }
  if (!pFields)
    return 0;
  if (pFields->IsDictionary())
    return 1;
  if (pFields->IsString())
    return 1;
  if (CPDF_Array* pArray = pFields->AsArray())
    return pArray->GetCount();
  return 0;
}

std::vector<CPDF_Object*> CPDF_ActionFields::GetAllFields() const {
  std::vector<CPDF_Object*> fields;
  if (!m_pAction)
    return fields;

  CPDF_Dictionary* pDict = m_pAction->GetDict();
  if (!pDict)
    return fields;

  CFX_ByteString csType = pDict->GetString("S");
  CPDF_Object* pFields;
  if (csType == "Hide")
    pFields = pDict->GetElementValue("T");
  else
    pFields = pDict->GetArray("Fields");
  if (!pFields)
    return fields;

  if (pFields->IsDictionary() || pFields->IsString()) {
    fields.push_back(pFields);
  } else if (CPDF_Array* pArray = pFields->AsArray()) {
    FX_DWORD iCount = pArray->GetCount();
    for (FX_DWORD i = 0; i < iCount; ++i) {
      CPDF_Object* pObj = pArray->GetElementValue(i);
      if (pObj) {
        fields.push_back(pObj);
      }
    }
  }
  return fields;
}

CPDF_Object* CPDF_ActionFields::GetField(FX_DWORD iIndex) const {
  if (!m_pAction) {
    return NULL;
  }
  CPDF_Dictionary* pDict = m_pAction->GetDict();
  if (!pDict) {
    return NULL;
  }
  CFX_ByteString csType = pDict->GetString("S");
  CPDF_Object* pFields = NULL;
  if (csType == "Hide") {
    pFields = pDict->GetElementValue("T");
  } else {
    pFields = pDict->GetArray("Fields");
  }
  if (!pFields) {
    return NULL;
  }
  CPDF_Object* pFindObj = NULL;
  if (pFields->IsDictionary() || pFields->IsString()) {
    if (iIndex == 0)
      pFindObj = pFields;
  } else if (CPDF_Array* pArray = pFields->AsArray()) {
    pFindObj = pArray->GetElementValue(iIndex);
  }
  return pFindObj;
}

CFX_WideString CPDF_Action::GetJavaScript() const {
  CFX_WideString csJS;
  if (!m_pDict) {
    return csJS;
  }
  CPDF_Object* pJS = m_pDict->GetElementValue("JS");
  return pJS ? pJS->GetUnicodeText() : csJS;
}
CPDF_Dictionary* CPDF_Action::GetAnnot() const {
  if (!m_pDict) {
    return nullptr;
  }
  CFX_ByteString csType = m_pDict->GetString("S");
  if (csType == "Rendition") {
    return m_pDict->GetDict("AN");
  }
  if (csType == "Movie") {
    return m_pDict->GetDict("Annotation");
  }
  return nullptr;
}
int32_t CPDF_Action::GetOperationType() const {
  if (!m_pDict) {
    return 0;
  }
  CFX_ByteString csType = m_pDict->GetString("S");
  if (csType == "Rendition") {
    return m_pDict->GetInteger("OP");
  }
  if (csType == "Movie") {
    CFX_ByteString csOP = m_pDict->GetString("Operation");
    if (csOP == "Play") {
      return 0;
    }
    if (csOP == "Stop") {
      return 1;
    }
    if (csOP == "Pause") {
      return 2;
    }
    if (csOP == "Resume") {
      return 3;
    }
  }
  return 0;
}
FX_DWORD CPDF_Action::GetSubActionsCount() const {
  if (!m_pDict || !m_pDict->KeyExist("Next"))
    return 0;

  CPDF_Object* pNext = m_pDict->GetElementValue("Next");
  if (!pNext)
    return 0;
  if (pNext->IsDictionary())
    return 1;
  if (CPDF_Array* pArray = pNext->AsArray())
    return pArray->GetCount();
  return 0;
}
CPDF_Action CPDF_Action::GetSubAction(FX_DWORD iIndex) const {
  if (!m_pDict || !m_pDict->KeyExist("Next")) {
    return CPDF_Action();
  }
  CPDF_Object* pNext = m_pDict->GetElementValue("Next");
  if (CPDF_Dictionary* pDict = ToDictionary(pNext)) {
    if (iIndex == 0)
      return CPDF_Action(pDict);
  } else if (CPDF_Array* pArray = ToArray(pNext)) {
    return CPDF_Action(pArray->GetDict(iIndex));
  }
  return CPDF_Action();
}
const FX_CHAR* g_sAATypes[] = {"E",  "X",  "D",  "U",  "Fo", "Bl", "PO", "PC",
                               "PV", "PI", "O",  "C",  "K",  "F",  "V",  "C",
                               "WC", "WS", "DS", "WP", "DP", ""};
FX_BOOL CPDF_AAction::ActionExist(AActionType eType) const {
  return m_pDict && m_pDict->KeyExist(g_sAATypes[(int)eType]);
}
CPDF_Action CPDF_AAction::GetAction(AActionType eType) const {
  if (!m_pDict) {
    return CPDF_Action();
  }
  return CPDF_Action(m_pDict->GetDict(g_sAATypes[(int)eType]));
}

CPDF_DocJSActions::CPDF_DocJSActions(CPDF_Document* pDoc) : m_pDocument(pDoc) {}

int CPDF_DocJSActions::CountJSActions() const {
  ASSERT(m_pDocument);
  CPDF_NameTree name_tree(m_pDocument, "JavaScript");
  return name_tree.GetCount();
}
CPDF_Action CPDF_DocJSActions::GetJSAction(int index,
                                           CFX_ByteString& csName) const {
  ASSERT(m_pDocument);
  CPDF_NameTree name_tree(m_pDocument, "JavaScript");
  CPDF_Object* pAction = name_tree.LookupValue(index, csName);
  if (!ToDictionary(pAction)) {
    return CPDF_Action();
  }
  return CPDF_Action(pAction->GetDict());
}
CPDF_Action CPDF_DocJSActions::GetJSAction(const CFX_ByteString& csName) const {
  ASSERT(m_pDocument);
  CPDF_NameTree name_tree(m_pDocument, "JavaScript");
  CPDF_Object* pAction = name_tree.LookupValue(csName);
  if (!ToDictionary(pAction)) {
    return CPDF_Action();
  }
  return CPDF_Action(pAction->GetDict());
}
int CPDF_DocJSActions::FindJSAction(const CFX_ByteString& csName) const {
  ASSERT(m_pDocument);
  CPDF_NameTree name_tree(m_pDocument, "JavaScript");
  return name_tree.GetIndex(csName);
}
