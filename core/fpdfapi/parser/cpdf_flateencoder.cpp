// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_flateencoder.h"

#include <memory>
#include <utility>

#include "constants/stream_dict_common.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "third_party/base/check.h"
#include "third_party/base/numerics/safe_conversions.h"

CPDF_FlateEncoder::OwnedData::OwnedData(
    std::unique_ptr<uint8_t, FxFreeDeleter> buffer,
    uint32_t size)
    : buffer(std::move(buffer)), size(size) {}

CPDF_FlateEncoder::OwnedData::~OwnedData() = default;

CPDF_FlateEncoder::CPDF_FlateEncoder(const CPDF_Stream* pStream,
                                     bool bFlateEncode)
    : m_pAcc(pdfium::MakeRetain<CPDF_StreamAcc>(pStream)) {
  m_pAcc->LoadAllDataRaw();

  bool bHasFilter = pStream->HasFilter();
  if (bHasFilter && !bFlateEncode) {
    auto pDestAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
    pDestAcc->LoadAllDataFiltered();

    m_Data.emplace<OwnedData>(pDestAcc->DetachData(), pDestAcc->GetSize());
    m_pClonedDict = ToDictionary(pStream->GetDict()->Clone());
    m_pClonedDict->RemoveFor("Filter");
    DCHECK(!m_pDict);
    return;
  }
  if (bHasFilter || !bFlateEncode) {
    m_Data = m_pAcc->GetSpan();
    m_pDict.Reset(pStream->GetDict());
    DCHECK(!m_pClonedDict);
    return;
  }

  // TODO(thestig): Move to Init() and check return value.
  std::unique_ptr<uint8_t, FxFreeDeleter> buffer;
  uint32_t buffer_size;
  ::FlateEncode(m_pAcc->GetSpan(), &buffer, &buffer_size);

  m_Data.emplace<OwnedData>(std::move(buffer), buffer_size);
  m_pClonedDict = ToDictionary(pStream->GetDict()->Clone());
  m_pClonedDict->SetNewFor<CPDF_Number>(
      "Length", pdfium::base::checked_cast<int>(buffer_size));
  m_pClonedDict->SetNewFor<CPDF_Name>("Filter", "FlateDecode");
  m_pClonedDict->RemoveFor(pdfium::stream::kDecodeParms);
  DCHECK(!m_pDict);
}

CPDF_FlateEncoder::~CPDF_FlateEncoder() = default;

void CPDF_FlateEncoder::CloneDict() {
  if (m_pClonedDict) {
    DCHECK(!m_pDict);
    return;
  }

  m_pClonedDict = ToDictionary(m_pDict->Clone());
  DCHECK(m_pClonedDict);
  m_pDict.Reset();
}

CPDF_Dictionary* CPDF_FlateEncoder::GetClonedDict() {
  DCHECK(!m_pDict);
  return m_pClonedDict.Get();
}

const CPDF_Dictionary* CPDF_FlateEncoder::GetDict() const {
  if (m_pClonedDict) {
    DCHECK(!m_pDict);
    return m_pClonedDict.Get();
  }

  return m_pDict.Get();
}

pdfium::span<const uint8_t> CPDF_FlateEncoder::GetSpan() const {
  if (is_owned()) {
    const OwnedData& data = absl::get<OwnedData>(m_Data);
    return pdfium::make_span(data.buffer.get(), data.size);
  }
  return absl::get<pdfium::span<const uint8_t>>(m_Data);
}
