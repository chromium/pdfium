// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_filespec.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/fx_system.h"

namespace {

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_ || \
    _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
CFX_WideString ChangeSlashToPlatform(const wchar_t* str) {
  CFX_WideString result;
  while (*str) {
    if (*str == '/') {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      result += L':';
#else
      result += L'\\';
#endif
    } else {
      result += *str;
    }
    str++;
  }
  return result;
}

CFX_WideString ChangeSlashToPDF(const wchar_t* str) {
  CFX_WideString result;
  while (*str) {
    if (*str == '\\' || *str == ':')
      result += L'/';
    else
      result += *str;

    str++;
  }
  return result;
}
#endif  // _FXM_PLATFORM_APPLE_ || _FXM_PLATFORM_WINDOWS_

}  // namespace

CFX_WideString CPDF_FileSpec::DecodeFileName(const CFX_WideString& filepath) {
  if (filepath.GetLength() <= 1)
    return CFX_WideString();

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  if (filepath.Left(sizeof("/Mac") - 1) == CFX_WideStringC(L"/Mac"))
    return ChangeSlashToPlatform(filepath.c_str() + 1);
  return ChangeSlashToPlatform(filepath.c_str());
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

  if (filepath.GetAt(0) != L'/')
    return ChangeSlashToPlatform(filepath.c_str());
  if (filepath.GetAt(1) == L'/')
    return ChangeSlashToPlatform(filepath.c_str() + 1);
  if (filepath.GetAt(2) == L'/') {
    CFX_WideString result;
    result += filepath.GetAt(1);
    result += L':';
    result += ChangeSlashToPlatform(filepath.c_str() + 2);
    return result;
  }
  CFX_WideString result;
  result += L'\\';
  result += ChangeSlashToPlatform(filepath.c_str());
  return result;
#else
  return CFX_WideString(filepath);
#endif
}

bool CPDF_FileSpec::GetFileName(CFX_WideString* csFileName) const {
  if (CPDF_Dictionary* pDict = m_pObj->AsDictionary()) {
    *csFileName = pDict->GetUnicodeTextFor("UF");
    if (csFileName->IsEmpty()) {
      *csFileName =
          CFX_WideString::FromLocal(pDict->GetStringFor("F").AsStringC());
    }
    if (pDict->GetStringFor("FS") == "URL")
      return true;
    if (csFileName->IsEmpty()) {
      if (pDict->KeyExist("DOS")) {
        *csFileName =
            CFX_WideString::FromLocal(pDict->GetStringFor("DOS").AsStringC());
      } else if (pDict->KeyExist("Mac")) {
        *csFileName =
            CFX_WideString::FromLocal(pDict->GetStringFor("Mac").AsStringC());
      } else if (pDict->KeyExist("Unix")) {
        *csFileName =
            CFX_WideString::FromLocal(pDict->GetStringFor("Unix").AsStringC());
      } else {
        return false;
      }
    }
  } else if (m_pObj->IsString()) {
    *csFileName = CFX_WideString::FromLocal(m_pObj->GetString().AsStringC());
  } else {
    return false;
  }
  *csFileName = DecodeFileName(*csFileName);
  return true;
}

CPDF_FileSpec::CPDF_FileSpec(CPDF_Object* pObj) : m_pObj(pObj) {}

CPDF_FileSpec::~CPDF_FileSpec() {}

CFX_WideString CPDF_FileSpec::EncodeFileName(const CFX_WideString& filepath) {
  if (filepath.GetLength() <= 1)
    return CFX_WideString();

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  if (filepath.GetAt(1) == L':') {
    CFX_WideString result(L'/');
    result += filepath.GetAt(0);
    if (filepath.GetAt(2) != L'\\')
      result += L'/';

    result += ChangeSlashToPDF(filepath.c_str() + 2);
    return result;
  }
  if (filepath.GetAt(0) == L'\\' && filepath.GetAt(1) == L'\\')
    return ChangeSlashToPDF(filepath.c_str() + 1);

  if (filepath.GetAt(0) == L'\\')
    return L'/' + ChangeSlashToPDF(filepath.c_str());
  return ChangeSlashToPDF(filepath.c_str());
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  if (filepath.Left(sizeof("Mac") - 1) == L"Mac")
    return L'/' + ChangeSlashToPDF(filepath.c_str());
  return ChangeSlashToPDF(filepath.c_str());
#else
  return CFX_WideString(filepath);
#endif
}

void CPDF_FileSpec::SetFileName(const CFX_WideString& wsFileName) {
  if (!m_pObj)
    return;

  CFX_WideString wsStr = EncodeFileName(wsFileName);
  if (m_pObj->IsString()) {
    m_pObj->SetString(CFX_ByteString::FromUnicode(wsStr));
  } else if (CPDF_Dictionary* pDict = m_pObj->AsDictionary()) {
    pDict->SetNewFor<CPDF_String>("F", CFX_ByteString::FromUnicode(wsStr),
                                  false);
    pDict->SetNewFor<CPDF_String>("UF", PDF_EncodeText(wsStr), false);
  }
}
