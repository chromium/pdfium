// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_action.h"

#include "constants/stream_dict_common.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfdoc/cpdf_filespec.h"
#include "core/fpdfdoc/cpdf_nametree.h"

namespace {

const char* const g_sATypes[] = {
    "Unknown",     "GoTo",       "GoToR",     "GoToE",      "Launch",
    "Thread",      "URI",        "Sound",     "Movie",      "Hide",
    "Named",       "SubmitForm", "ResetForm", "ImportData", "JavaScript",
    "SetOCGState", "Rendition",  "Trans",     "GoTo3DView", nullptr};

}  // namespace

CPDF_Action::CPDF_Action(const CPDF_Dictionary* pDict) : m_pDict(pDict) {}

CPDF_Action::CPDF_Action(const CPDF_Action& that) = default;

CPDF_Action::~CPDF_Action() = default;

CPDF_Action::ActionType CPDF_Action::GetType() const {
  if (!m_pDict)
    return Unknown;

  // Validate |m_pDict|. Type is optional, but must be valid if present.
  const CPDF_Object* pType = m_pDict->GetObjectFor("Type");
  if (pType) {
    const CPDF_Name* pName = pType->AsName();
    if (!pName || pName->GetString() != "Action")
      return Unknown;
  }

  ByteString csType = m_pDict->GetStringFor("S");
  if (csType.IsEmpty())
    return Unknown;

  for (int i = 0; g_sATypes[i]; ++i) {
    if (csType == g_sATypes[i])
      return static_cast<ActionType>(i);
  }
  return Unknown;
}

CPDF_Dest CPDF_Action::GetDest(CPDF_Document* pDoc) const {
  ActionType type = GetType();
  if (type != GoTo && type != GoToR)
    return CPDF_Dest();

  const CPDF_Object* pDest = m_pDict->GetDirectObjectFor("D");
  if (!pDest)
    return CPDF_Dest();
  if (pDest->IsString() || pDest->IsName()) {
    CPDF_NameTree name_tree(pDoc, "Dests");
    return CPDF_Dest(name_tree.LookupNamedDest(pDoc, pDest->GetUnicodeText()));
  }
  if (const CPDF_Array* pArray = pDest->AsArray())
    return CPDF_Dest(pArray);

  return CPDF_Dest();
}

WideString CPDF_Action::GetFilePath() const {
  ActionType type = GetType();
  if (type != GoToR && type != Launch && type != SubmitForm &&
      type != ImportData) {
    return WideString();
  }

  const CPDF_Object* pFile = m_pDict->GetDirectObjectFor(pdfium::stream::kF);
  if (pFile)
    return CPDF_FileSpec(pFile).GetFileName();

  if (type != Launch)
    return WideString();

  const CPDF_Dictionary* pWinDict = m_pDict->GetDictFor("Win");
  if (!pWinDict)
    return WideString();

  return WideString::FromDefANSI(
      pWinDict->GetStringFor(pdfium::stream::kF).AsStringView());
}

ByteString CPDF_Action::GetURI(const CPDF_Document* pDoc) const {
  ActionType type = GetType();
  if (type != URI)
    return ByteString();

  ByteString csURI = m_pDict->GetStringFor("URI");
  const CPDF_Dictionary* pRoot = pDoc->GetRoot();
  const CPDF_Dictionary* pURI = pRoot->GetDictFor("URI");
  if (pURI) {
    auto result = csURI.Find(":");
    if (!result.has_value() || result.value() == 0) {
      auto* pBase = pURI->GetDirectObjectFor("Base");
      if (pBase && (pBase->IsString() || pBase->IsStream()))
        csURI = pBase->GetString() + csURI;
    }
  }
  return csURI;
}

bool CPDF_Action::GetHideStatus() const {
  return m_pDict->GetBooleanFor("H", true);
}

ByteString CPDF_Action::GetNamedAction() const {
  return m_pDict->GetStringFor("N");
}

uint32_t CPDF_Action::GetFlags() const {
  return m_pDict->GetIntegerFor("Flags");
}

Optional<WideString> CPDF_Action::MaybeGetJavaScript() const {
  const CPDF_Object* pObject = GetJavaScriptObject();
  if (!pObject)
    return pdfium::nullopt;
  return pObject->GetUnicodeText();
}

WideString CPDF_Action::GetJavaScript() const {
  const CPDF_Object* pObject = GetJavaScriptObject();
  return pObject ? pObject->GetUnicodeText() : WideString();
}

size_t CPDF_Action::GetSubActionsCount() const {
  if (!m_pDict || !m_pDict->KeyExist("Next"))
    return 0;

  const CPDF_Object* pNext = m_pDict->GetDirectObjectFor("Next");
  if (!pNext)
    return 0;
  if (pNext->IsDictionary())
    return 1;
  const CPDF_Array* pArray = pNext->AsArray();
  return pArray ? pArray->size() : 0;
}

CPDF_Action CPDF_Action::GetSubAction(size_t iIndex) const {
  if (!m_pDict || !m_pDict->KeyExist("Next"))
    return CPDF_Action(nullptr);

  const CPDF_Object* pNext = m_pDict->GetDirectObjectFor("Next");
  if (const CPDF_Array* pArray = ToArray(pNext))
    return CPDF_Action(pArray->GetDictAt(iIndex));
  if (const CPDF_Dictionary* pDict = ToDictionary(pNext)) {
    if (iIndex == 0)
      return CPDF_Action(pDict);
  }
  return CPDF_Action(nullptr);
}

const CPDF_Object* CPDF_Action::GetJavaScriptObject() const {
  if (!m_pDict)
    return nullptr;

  const CPDF_Object* pJS = m_pDict->GetDirectObjectFor("JS");
  return (pJS && (pJS->IsString() || pJS->IsStream())) ? pJS : nullptr;
}
