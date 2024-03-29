// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_string.h"

#include <stdint.h>

#include <utility>

#include "core/fpdfapi/parser/cpdf_encryptor.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_stream.h"

CPDF_String::CPDF_String() = default;

CPDF_String::CPDF_String(WeakPtr<ByteStringPool> pPool,
                         const ByteString& str,
                         bool bHex)
    : m_String(str), m_bHex(bHex) {
  if (pPool)
    m_String = pPool->Intern(m_String);
}

CPDF_String::CPDF_String(WeakPtr<ByteStringPool> pPool, WideStringView str)
    : m_String(PDF_EncodeText(str)) {
  if (pPool)
    m_String = pPool->Intern(m_String);
}

CPDF_String::~CPDF_String() = default;

CPDF_Object::Type CPDF_String::GetType() const {
  return kString;
}

RetainPtr<CPDF_Object> CPDF_String::Clone() const {
  auto pRet = pdfium::MakeRetain<CPDF_String>();
  pRet->m_String = m_String;
  pRet->m_bHex = m_bHex;
  return pRet;
}

ByteString CPDF_String::GetString() const {
  return m_String;
}

void CPDF_String::SetString(const ByteString& str) {
  m_String = str;
}

CPDF_String* CPDF_String::AsMutableString() {
  return this;
}

WideString CPDF_String::GetUnicodeText() const {
  return PDF_DecodeText(m_String.unsigned_span());
}

bool CPDF_String::WriteTo(IFX_ArchiveStream* archive,
                          const CPDF_Encryptor* encryptor) const {
  DataVector<uint8_t> encrypted_data;
  pdfium::span<const uint8_t> data = m_String.unsigned_span();
  if (encryptor) {
    encrypted_data = encryptor->Encrypt(data);
    data = encrypted_data;
  }
  ByteStringView raw(data);
  ByteString content =
      m_bHex ? PDF_HexEncodeString(raw) : PDF_EncodeString(raw);
  return archive->WriteString(content.AsStringView());
}

ByteString CPDF_String::EncodeString() const {
  return m_bHex ? PDF_HexEncodeString(m_String.AsStringView())
                : PDF_EncodeString(m_String.AsStringView());
}
