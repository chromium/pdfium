// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_stream.h"

#include <stdint.h>

#include <sstream>
#include <utility>

#include "constants/stream_dict_common.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_encryptor.h"
#include "core/fpdfapi/parser/cpdf_flateencoder.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/span_util.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/numerics/safe_conversions.h"

namespace {

bool IsMetaDataStreamDictionary(const CPDF_Dictionary* dict) {
  // See ISO 32000-1:2008 spec, table 315.
  return ValidateDictType(dict, "Metadata") &&
         dict->GetNameFor("Subtype") == "XML";
}

}  // namespace

CPDF_Stream::CPDF_Stream() = default;

CPDF_Stream::CPDF_Stream(RetainPtr<CPDF_Dictionary> pDict)
    : CPDF_Stream(DataVector<uint8_t>(), std::move(pDict)) {}

CPDF_Stream::CPDF_Stream(DataVector<uint8_t> pData,
                         RetainPtr<CPDF_Dictionary> pDict)
    : data_(std::move(pData)), dict_(std::move(pDict)) {
  SetLengthInDict(pdfium::base::checked_cast<int>(
      absl::get<DataVector<uint8_t>>(data_).size()));
}

CPDF_Stream::~CPDF_Stream() {
  m_ObjNum = kInvalidObjNum;
  if (dict_ && dict_->GetObjNum() == kInvalidObjNum)
    dict_.Leak();  // lowercase release, release ownership.
}

CPDF_Object::Type CPDF_Stream::GetType() const {
  return kStream;
}

const CPDF_Dictionary* CPDF_Stream::GetDictInternal() const {
  return dict_.Get();
}

CPDF_Stream* CPDF_Stream::AsMutableStream() {
  return this;
}

void CPDF_Stream::InitStreamWithEmptyData(RetainPtr<CPDF_Dictionary> pDict) {
  dict_ = std::move(pDict);
  TakeData({});
}

void CPDF_Stream::InitStreamFromFile(RetainPtr<IFX_SeekableReadStream> pFile,
                                     RetainPtr<CPDF_Dictionary> pDict) {
  data_ = pFile;
  dict_ = std::move(pDict);
  SetLengthInDict(pdfium::base::checked_cast<int>(pFile->GetSize()));
}

RetainPtr<CPDF_Object> CPDF_Stream::Clone() const {
  return CloneObjectNonCyclic(false);
}

RetainPtr<CPDF_Object> CPDF_Stream::CloneNonCyclic(
    bool bDirect,
    std::set<const CPDF_Object*>* pVisited) const {
  pVisited->insert(this);
  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(this));
  pAcc->LoadAllDataRaw();

  RetainPtr<const CPDF_Dictionary> pDict = GetDict();
  RetainPtr<CPDF_Dictionary> pNewDict;
  if (pDict && !pdfium::Contains(*pVisited, pDict.Get())) {
    pNewDict = ToDictionary(static_cast<const CPDF_Object*>(pDict.Get())
                                ->CloneNonCyclic(bDirect, pVisited));
  }
  return pdfium::MakeRetain<CPDF_Stream>(pAcc->DetachData(),
                                         std::move(pNewDict));
}

void CPDF_Stream::SetDataAndRemoveFilter(pdfium::span<const uint8_t> pData) {
  SetData(pData);
  dict_->RemoveFor("Filter");
  dict_->RemoveFor(pdfium::stream::kDecodeParms);
}

void CPDF_Stream::SetDataFromStringstreamAndRemoveFilter(
    fxcrt::ostringstream* stream) {
  if (stream->tellp() <= 0) {
    SetDataAndRemoveFilter({});
    return;
  }

  SetDataAndRemoveFilter(
      {reinterpret_cast<const uint8_t*>(stream->str().c_str()),
       static_cast<size_t>(stream->tellp())});
}

void CPDF_Stream::SetData(pdfium::span<const uint8_t> pData) {
  DataVector<uint8_t> data_copy(pData.begin(), pData.end());
  TakeData(std::move(data_copy));
}

void CPDF_Stream::TakeData(DataVector<uint8_t> data) {
  const size_t size = data.size();
  data_ = std::move(data);
  SetLengthInDict(pdfium::base::checked_cast<int>(size));
}

void CPDF_Stream::SetDataFromStringstream(fxcrt::ostringstream* stream) {
  if (stream->tellp() <= 0) {
    SetData({});
    return;
  }
  SetData({reinterpret_cast<const uint8_t*>(stream->str().c_str()),
           static_cast<size_t>(stream->tellp())});
}

DataVector<uint8_t> CPDF_Stream::ReadAllRawData() const {
  CHECK(IsFileBased());

  DataVector<uint8_t> result(GetRawSize());
  DCHECK(!result.empty());

  auto underlying_stream = absl::get<RetainPtr<IFX_SeekableReadStream>>(data_);
  if (!underlying_stream->ReadBlockAtOffset(result, 0))
    return DataVector<uint8_t>();

  return result;
}

bool CPDF_Stream::HasFilter() const {
  return dict_ && dict_->KeyExist("Filter");
}

WideString CPDF_Stream::GetUnicodeText() const {
  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(this));
  pAcc->LoadAllDataFiltered();
  return PDF_DecodeText(pAcc->GetSpan());
}

bool CPDF_Stream::WriteTo(IFX_ArchiveStream* archive,
                          const CPDF_Encryptor* encryptor) const {
  const bool is_metadata = IsMetaDataStreamDictionary(GetDict().Get());
  CPDF_FlateEncoder encoder(pdfium::WrapRetain(this), !is_metadata);

  DataVector<uint8_t> encrypted_data;
  pdfium::span<const uint8_t> data = encoder.GetSpan();
  if (encryptor && !is_metadata) {
    encrypted_data = encryptor->Encrypt(data);
    data = encrypted_data;
  }

  encoder.UpdateLength(data.size());
  if (!encoder.WriteDictTo(archive, encryptor))
    return false;

  if (!archive->WriteString("stream\r\n"))
    return false;

  if (!archive->WriteBlock(data))
    return false;

  return archive->WriteString("\r\nendstream");
}

size_t CPDF_Stream::GetRawSize() const {
  if (IsFileBased()) {
    return pdfium::base::checked_cast<size_t>(
        absl::get<RetainPtr<IFX_SeekableReadStream>>(data_)->GetSize());
  }
  if (IsMemoryBased())
    return absl::get<DataVector<uint8_t>>(data_).size();
  DCHECK(IsUninitialized());
  return 0;
}

pdfium::span<const uint8_t> CPDF_Stream::GetInMemoryRawData() const {
  DCHECK(IsMemoryBased());
  return absl::get<DataVector<uint8_t>>(data_);
}

void CPDF_Stream::SetLengthInDict(int length) {
  if (!dict_)
    dict_ = pdfium::MakeRetain<CPDF_Dictionary>();
  dict_->SetNewFor<CPDF_Number>("Length", length);
}
