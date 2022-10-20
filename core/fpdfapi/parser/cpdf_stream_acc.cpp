// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_stream_acc.h"

#include <utility>
#include <vector>

#include "core/fdrm/fx_crypt.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/data_vector.h"
#include "third_party/base/check_op.h"

CPDF_StreamAcc::CPDF_StreamAcc(RetainPtr<const CPDF_Stream> pStream)
    : m_pStream(std::move(pStream)) {}

CPDF_StreamAcc::~CPDF_StreamAcc() = default;

void CPDF_StreamAcc::LoadAllData(bool bRawAccess,
                                 uint32_t estimated_size,
                                 bool bImageAcc) {
  if (bRawAccess) {
    DCHECK(!estimated_size);
    DCHECK(!bImageAcc);
  }

  if (!m_pStream)
    return;

  bool bProcessRawData = bRawAccess || !m_pStream->HasFilter();
  if (bProcessRawData)
    ProcessRawData();
  else
    ProcessFilteredData(estimated_size, bImageAcc);
}

void CPDF_StreamAcc::LoadAllDataFiltered() {
  LoadAllData(false, 0, false);
}

void CPDF_StreamAcc::LoadAllDataFilteredWithEstimatedSize(
    uint32_t estimated_size) {
  LoadAllData(false, estimated_size, false);
}

void CPDF_StreamAcc::LoadAllDataImageAcc(uint32_t estimated_size) {
  LoadAllData(false, estimated_size, true);
}

void CPDF_StreamAcc::LoadAllDataRaw() {
  LoadAllData(true, 0, false);
}

RetainPtr<const CPDF_Stream> CPDF_StreamAcc::GetStream() const {
  return m_pStream;
}

int CPDF_StreamAcc::GetLength1ForTest() const {
  return m_pStream->GetDict()->GetIntegerFor("Length1");
}

RetainPtr<const CPDF_Dictionary> CPDF_StreamAcc::GetImageParam() const {
  return m_pImageParam;
}

const uint8_t* CPDF_StreamAcc::GetData() const {
  if (is_owned())
    return absl::get<DataVector<uint8_t>>(m_Data).data();
  return (m_pStream && m_pStream->IsMemoryBased())
             ? m_pStream->GetInMemoryRawData().data()
             : nullptr;
}

uint32_t CPDF_StreamAcc::GetSize() const {
  if (is_owned())
    return absl::get<DataVector<uint8_t>>(m_Data).size();
  return (m_pStream && m_pStream->IsMemoryBased()) ? m_pStream->GetRawSize()
                                                   : 0;
}

pdfium::span<const uint8_t> CPDF_StreamAcc::GetSpan() const {
  return {GetData(), GetSize()};
}

uint64_t CPDF_StreamAcc::KeyForCache() const {
  return m_pStream ? m_pStream->KeyForCache() : 0;
}

ByteString CPDF_StreamAcc::ComputeDigest() const {
  uint8_t digest[20];
  CRYPT_SHA1Generate(GetData(), GetSize(), digest);
  return ByteString(digest, 20);
}

DataVector<uint8_t> CPDF_StreamAcc::DetachData() {
  if (is_owned())
    return std::move(absl::get<DataVector<uint8_t>>(m_Data));

  auto span = absl::get<pdfium::span<const uint8_t>>(m_Data);
  return DataVector<uint8_t>(span.begin(), span.end());
}

void CPDF_StreamAcc::ProcessRawData() {
  if (m_pStream->IsUninitialized())
    return;

  uint32_t dwSrcSize = m_pStream->GetRawSize();
  if (dwSrcSize == 0)
    return;

  if (m_pStream->IsMemoryBased()) {
    m_Data = m_pStream->GetInMemoryRawData();
    return;
  }

  DataVector<uint8_t> data = ReadRawStream();
  if (data.empty())
    return;

  m_Data = std::move(data);
}

void CPDF_StreamAcc::ProcessFilteredData(uint32_t estimated_size,
                                         bool bImageAcc) {
  if (m_pStream->IsUninitialized())
    return;

  uint32_t dwSrcSize = m_pStream->GetRawSize();
  if (dwSrcSize == 0)
    return;

  absl::variant<pdfium::span<const uint8_t>, DataVector<uint8_t>> src_data;
  pdfium::span<const uint8_t> src_span;
  if (m_pStream->IsMemoryBased()) {
    src_span = m_pStream->GetInMemoryRawData();
    src_data = src_span;
  } else {
    DataVector<uint8_t> temp_src_data = ReadRawStream();
    if (temp_src_data.empty())
      return;

    src_span = pdfium::make_span(temp_src_data);
    src_data = std::move(temp_src_data);
  }

  std::unique_ptr<uint8_t, FxFreeDeleter> pDecodedData;
  uint32_t dwDecodedSize = 0;

  absl::optional<DecoderArray> decoder_array =
      GetDecoderArray(m_pStream->GetDict());
  if (!decoder_array.has_value() || decoder_array.value().empty() ||
      !PDF_DataDecode(src_span, estimated_size, bImageAcc,
                      decoder_array.value(), &pDecodedData, &dwDecodedSize,
                      &m_ImageDecoder, &m_pImageParam)) {
    m_Data = std::move(src_data);
    return;
  }

  if (pDecodedData) {
    DCHECK_NE(pDecodedData.get(), src_span.data());
    // TODO(crbug.com/pdfium/1872): Avoid copying.
    m_Data = DataVector<uint8_t>(pDecodedData.get(),
                                 pDecodedData.get() + dwDecodedSize);
  } else {
    m_Data = std::move(src_data);
  }
}

DataVector<uint8_t> CPDF_StreamAcc::ReadRawStream() const {
  DCHECK(m_pStream);
  DCHECK(m_pStream->IsFileBased());

  DataVector<uint8_t> result(m_pStream->GetRawSize());
  DCHECK(!result.empty());
  if (!m_pStream->ReadRawData(0, result.data(), result.size()))
    return DataVector<uint8_t>();
  return result;
}
