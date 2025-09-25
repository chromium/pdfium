// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_linearized_header.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_syntax_parser.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/ptr_util.h"

namespace {

constexpr FX_FILESIZE kLinearizedHeaderOffset = 9;
constexpr size_t kMaxInt = static_cast<size_t>(std::numeric_limits<int>::max());

template <class T>
bool IsValidNumericDictionaryValue(const CPDF_Dictionary* dict,
                                   ByteStringView key,
                                   T min_value,
                                   bool must_exist = true) {
  if (!dict->KeyExist(key)) {
    return !must_exist;
  }
  RetainPtr<const CPDF_Number> pNum = dict->GetNumberFor(key);
  if (!pNum || !pNum->IsInteger()) {
    return false;
  }
  const int raw_value = pNum->GetInteger();
  if (!pdfium::IsValueInRangeForNumericType<T>(raw_value)) {
    return false;
  }
  return static_cast<T>(raw_value) >= min_value;
}

bool IsLinearizedHeaderValid(const CPDF_LinearizedHeader* header,
                             FX_FILESIZE document_size) {
  DCHECK(header);
  return header->GetFileSize() == document_size &&
         header->GetFirstPageNo() < kMaxInt &&
         header->GetFirstPageNo() < header->GetPageCount() &&
         header->GetMainXRefTableFirstEntryOffset() < document_size &&
         header->GetFirstPageEndOffset() < document_size &&
         header->GetFirstPageObjNum() <= CPDF_Parser::kMaxObjectNumber &&
         header->GetLastXRefOffset() < document_size &&
         header->GetHintStart() < document_size;
}

}  // namespace

// static
std::unique_ptr<CPDF_LinearizedHeader> CPDF_LinearizedHeader::Parse(
    CPDF_SyntaxParser* parser) {
  parser->SetPos(kLinearizedHeaderOffset);

  const auto dict = ToDictionary(
      parser->GetIndirectObject(nullptr, CPDF_SyntaxParser::ParseType::kLoose));

  if (!dict || !dict->KeyExist("Linearized") ||
      !IsValidNumericDictionaryValue<FX_FILESIZE>(dict.Get(), "L", 1) ||
      !IsValidNumericDictionaryValue<uint32_t>(dict.Get(), "P", 0, false) ||
      !IsValidNumericDictionaryValue<FX_FILESIZE>(dict.Get(), "T", 1) ||
      !IsValidNumericDictionaryValue<uint32_t>(dict.Get(), "N", 1) ||
      !IsValidNumericDictionaryValue<FX_FILESIZE>(dict.Get(), "E", 1) ||
      !IsValidNumericDictionaryValue<uint32_t>(dict.Get(), "O", 1)) {
    return nullptr;
  }
  // Move parser to the start of the xref table for the documents first page.
  // (skpping endobj keyword)
  if (parser->GetNextWord().word != "endobj") {
    return nullptr;
  }

  auto result = pdfium::WrapUnique(
      new CPDF_LinearizedHeader(dict.Get(), parser->GetPos()));

  if (!IsLinearizedHeaderValid(result.get(), parser->GetDocumentSize())) {
    return nullptr;
  }

  return result;
}

CPDF_LinearizedHeader::CPDF_LinearizedHeader(const CPDF_Dictionary* dict,
                                             FX_FILESIZE szLastXRefOffset)
    : file_size_(dict->GetIntegerFor("L")),
      first_page_no_(dict->GetIntegerFor("P")),
      main_xref_table_first_entry_offset_(dict->GetIntegerFor("T")),
      page_count_(dict->GetIntegerFor("N")),
      first_page_end_offset_(dict->GetIntegerFor("E")),
      first_page_obj_num_(dict->GetIntegerFor("O")),
      last_xref_offset_(szLastXRefOffset) {
  RetainPtr<const CPDF_Array> pHintStreamRange = dict->GetArrayFor("H");
  const size_t nHintStreamSize =
      pHintStreamRange ? pHintStreamRange->size() : 0;
  if (nHintStreamSize == 2 || nHintStreamSize == 4) {
    hint_start_ = std::max(pHintStreamRange->GetIntegerAt(0), 0);
    const FX_SAFE_UINT32 safe_hint_length = pHintStreamRange->GetIntegerAt(1);
    if (safe_hint_length.IsValid()) {
      hint_length_ = safe_hint_length.ValueOrDie();
    }
  }
}

CPDF_LinearizedHeader::~CPDF_LinearizedHeader() = default;

bool CPDF_LinearizedHeader::HasHintTable() const {
  return GetPageCount() > 1 && GetHintStart() > 0 && GetHintLength() > 0;
}
