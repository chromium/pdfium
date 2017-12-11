// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_flateencoder.h"

#include <memory>

#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"

CPDF_FlateEncoder::CPDF_FlateEncoder(CPDF_Stream* pStream, bool bFlateEncode)
    : m_dwSize(0), m_pAcc(pdfium::MakeRetain<CPDF_StreamAcc>(pStream)) {
  m_pAcc->LoadAllDataRaw();

  bool bHasFilter = pStream && pStream->HasFilter();
  if (bHasFilter && !bFlateEncode) {
    auto pDestAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
    pDestAcc->LoadAllDataFiltered();

    m_dwSize = pDestAcc->GetSize();
    m_pData = pDestAcc->DetachData();
    m_pDict = ToDictionary(pStream->GetDict()->Clone());
    m_pDict->RemoveFor("Filter");
    return;
  }
  if (bHasFilter || !bFlateEncode) {
    m_pData = const_cast<uint8_t*>(m_pAcc->GetData());
    m_dwSize = m_pAcc->GetSize();
    m_pDict = pStream->GetDict();
    return;
  }

  // TODO(thestig): Move to Init() and check return value.
  uint8_t* buffer = nullptr;
  ::FlateEncode(m_pAcc->GetData(), m_pAcc->GetSize(), &buffer, &m_dwSize);

  m_pData = std::unique_ptr<uint8_t, FxFreeDeleter>(buffer);
  m_pDict = ToDictionary(pStream->GetDict()->Clone());
  m_pDict->SetNewFor<CPDF_Number>("Length", static_cast<int>(m_dwSize));
  m_pDict->SetNewFor<CPDF_Name>("Filter", "FlateDecode");
  m_pDict->RemoveFor("DecodeParms");
}

CPDF_FlateEncoder::~CPDF_FlateEncoder() {}

void CPDF_FlateEncoder::CloneDict() {
  if (m_pDict.IsOwned())
    return;

  m_pDict = ToDictionary(m_pDict->Clone());
  ASSERT(m_pDict.IsOwned());
}
