// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_hint_tables.h"

#include <limits>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_data_avail.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_linearized_header.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_read_validator.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_syntax_parser.h"
#include "core/fxcrt/cfx_bitstream.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span.h"

namespace {

bool CanReadFromBitStream(const CFX_BitStream* hStream,
                          const FX_SAFE_UINT32& bits) {
  return bits.IsValid() && hStream->BitsRemaining() >= bits.ValueOrDie();
}

// Sanity check values from the page table header. The note in the PDF 1.7
// reference for Table F.3 says the valid range is only 0 through 32. Though 0
// is not useful either.
bool IsValidPageOffsetHintTableBitCount(uint32_t bits) {
  return bits > 0 && bits <= 32;
}

}  // namespace

CPDF_HintTables::PageInfo::PageInfo() = default;
CPDF_HintTables::PageInfo::~PageInfo() = default;

//  static
std::unique_ptr<CPDF_HintTables> CPDF_HintTables::Parse(
    CPDF_SyntaxParser* parser,
    const CPDF_LinearizedHeader* pLinearized) {
  DCHECK(parser);
  if (!pLinearized || pLinearized->GetPageCount() <= 1 ||
      !pLinearized->HasHintTable()) {
    return nullptr;
  }

  const FX_FILESIZE szHintStart = pLinearized->GetHintStart();
  const uint32_t szHintLength = pLinearized->GetHintLength();

  if (!parser->GetValidator()->CheckDataRangeAndRequestIfUnavailable(
          szHintStart, szHintLength)) {
    return nullptr;
  }

  parser->SetPos(szHintStart);
  RetainPtr<CPDF_Stream> hints_stream = ToStream(
      parser->GetIndirectObject(nullptr, CPDF_SyntaxParser::ParseType::kLoose));

  if (!hints_stream) {
    return nullptr;
  }

  auto pHintTables = std::make_unique<CPDF_HintTables>(
      parser->GetValidator().Get(), pLinearized);
  if (!pHintTables->LoadHintStream(hints_stream.Get())) {
    return nullptr;
  }

  return pHintTables;
}

CPDF_HintTables::CPDF_HintTables(CPDF_ReadValidator* pValidator,
                                 const CPDF_LinearizedHeader* pLinearized)
    : validator_(pValidator), linearized_(pLinearized) {
  DCHECK(linearized_);
}

CPDF_HintTables::~CPDF_HintTables() = default;

bool CPDF_HintTables::ReadPageHintTable(CFX_BitStream* hStream) {
  const uint32_t nPages = linearized_->GetPageCount();
  if (nPages < 1 || nPages >= CPDF_Document::kPageMaxNum) {
    return false;
  }

  const uint32_t nFirstPageNum = linearized_->GetFirstPageNo();
  if (nFirstPageNum >= nPages) {
    return false;
  }

  if (!hStream || hStream->IsEOF()) {
    return false;
  }

  const uint32_t kHeaderSize = 288;
  if (hStream->BitsRemaining() < kHeaderSize) {
    return false;
  }

  // Item 1: The least number of objects in a page.
  const uint32_t dwObjLeastNum = hStream->GetBits(32);
  if (!dwObjLeastNum || dwObjLeastNum > CPDF_Parser::kMaxObjectNumber) {
    return false;
  }

  // Item 2: The location of the first page's page object.
  const FX_FILESIZE szFirstObjLoc =
      HintsOffsetToFileOffset(hStream->GetBits(32));
  if (!szFirstObjLoc) {
    return false;
  }

  first_page_obj_offset_ = szFirstObjLoc;

  // Item 3: The number of bits needed to represent the difference
  // between the greatest and least number of objects in a page.
  const uint32_t dwDeltaObjectsBits = hStream->GetBits(16);
  if (!IsValidPageOffsetHintTableBitCount(dwDeltaObjectsBits)) {
    return false;
  }

  // Item 4: The least length of a page in bytes.
  const uint32_t dwPageLeastLen = hStream->GetBits(32);
  if (!dwPageLeastLen) {
    return false;
  }

  // Item 5: The number of bits needed to represent the difference
  // between the greatest and least length of a page, in bytes.
  const uint32_t dwDeltaPageLenBits = hStream->GetBits(16);
  if (!IsValidPageOffsetHintTableBitCount(dwDeltaPageLenBits)) {
    return false;
  }

  // Skip Item 6, 7, 8, 9 total 96 bits.
  hStream->SkipBits(96);

  // Item 10: The number of bits needed to represent the greatest
  // number of shared object references.
  const uint32_t dwSharedObjBits = hStream->GetBits(16);
  if (!IsValidPageOffsetHintTableBitCount(dwSharedObjBits)) {
    return false;
  }

  // Item 11: The number of bits needed to represent the numerically
  // greatest shared object identifier used by the pages.
  const uint32_t dwSharedIdBits = hStream->GetBits(16);
  if (!IsValidPageOffsetHintTableBitCount(dwSharedIdBits)) {
    return false;
  }

  // Item 12: The number of bits needed to represent the numerator of
  // the fractional position for each shared object reference. For each
  // shared object referenced from a page, there is an indication of
  // where in the page's content stream the object is first referenced.
  const uint32_t dwSharedNumeratorBits = hStream->GetBits(16);
  if (dwSharedNumeratorBits > 32) {
    return false;
  }

  // Item 13: Skip Item 13 which has 16 bits.
  hStream->SkipBits(16);

  FX_SAFE_UINT32 required_bits = dwDeltaObjectsBits;
  required_bits *= nPages;
  if (!CanReadFromBitStream(hStream, required_bits)) {
    return false;
  }

  page_infos_ = std::vector<PageInfo>(nPages);
  page_infos_[nFirstPageNum].set_start_obj_num(
      linearized_->GetFirstPageObjNum());
  // The object number of remaining pages starts from 1.
  FX_SAFE_UINT32 dwStartObjNum = 1;
  for (uint32_t i = 0; i < nPages; ++i) {
    FX_SAFE_UINT32 safeDeltaObj = hStream->GetBits(dwDeltaObjectsBits);
    safeDeltaObj += dwObjLeastNum;
    if (!safeDeltaObj.IsValid()) {
      return false;
    }
    page_infos_[i].set_objects_count(safeDeltaObj.ValueOrDie());
    if (i == nFirstPageNum) {
      continue;
    }
    page_infos_[i].set_start_obj_num(dwStartObjNum.ValueOrDie());
    dwStartObjNum += page_infos_[i].objects_count();
    if (!dwStartObjNum.IsValid() ||
        dwStartObjNum.ValueOrDie() > CPDF_Parser::kMaxObjectNumber) {
      return false;
    }
  }
  hStream->ByteAlign();

  required_bits = dwDeltaPageLenBits;
  required_bits *= nPages;
  if (!CanReadFromBitStream(hStream, required_bits)) {
    return false;
  }

  for (uint32_t i = 0; i < nPages; ++i) {
    FX_SAFE_UINT32 safePageLen = hStream->GetBits(dwDeltaPageLenBits);
    safePageLen += dwPageLeastLen;
    if (!safePageLen.IsValid()) {
      return false;
    }
    page_infos_[i].set_page_length(safePageLen.ValueOrDie());
  }

  DCHECK(first_page_obj_offset_);
  page_infos_[nFirstPageNum].set_page_offset(first_page_obj_offset_);
  FX_FILESIZE prev_page_end = linearized_->GetFirstPageEndOffset();
  for (uint32_t i = 0; i < nPages; ++i) {
    if (i == nFirstPageNum) {
      continue;
    }
    page_infos_[i].set_page_offset(prev_page_end);
    prev_page_end += page_infos_[i].page_length();
  }
  hStream->ByteAlign();

  // Number of shared objects.
  required_bits = dwSharedObjBits;
  required_bits *= nPages;
  if (!CanReadFromBitStream(hStream, required_bits)) {
    return false;
  }

  std::vector<uint32_t> dwNSharedObjsArray(nPages);
  for (uint32_t i = 0; i < nPages; i++) {
    dwNSharedObjsArray[i] = hStream->GetBits(dwSharedObjBits);
  }
  hStream->ByteAlign();

  // Array of identifiers, size = nshared_objects.
  for (uint32_t i = 0; i < nPages; i++) {
    required_bits = dwSharedIdBits;
    required_bits *= dwNSharedObjsArray[i];
    if (!CanReadFromBitStream(hStream, required_bits)) {
      return false;
    }

    for (uint32_t j = 0; j < dwNSharedObjsArray[i]; j++) {
      page_infos_[i].AddIdentifier(hStream->GetBits(dwSharedIdBits));
    }
  }
  hStream->ByteAlign();

  if (dwSharedNumeratorBits) {
    for (uint32_t i = 0; i < nPages; i++) {
      FX_SAFE_UINT32 safeSize = dwNSharedObjsArray[i];
      safeSize *= dwSharedNumeratorBits;
      if (!CanReadFromBitStream(hStream, safeSize)) {
        return false;
      }

      hStream->SkipBits(safeSize.ValueOrDie());
    }
    hStream->ByteAlign();
  }

  FX_SAFE_UINT32 safeTotalPageLen = nPages;
  safeTotalPageLen *= dwDeltaPageLenBits;
  if (!CanReadFromBitStream(hStream, safeTotalPageLen)) {
    return false;
  }

  hStream->SkipBits(safeTotalPageLen.ValueOrDie());
  hStream->ByteAlign();
  return true;
}

bool CPDF_HintTables::ReadSharedObjHintTable(CFX_BitStream* hStream,
                                             uint32_t offset) {
  if (!hStream || hStream->IsEOF()) {
    return false;
  }

  FX_SAFE_UINT32 bit_offset = offset;
  bit_offset *= 8;
  if (!bit_offset.IsValid() || hStream->GetPos() > bit_offset.ValueOrDie()) {
    return false;
  }
  hStream->SkipBits((bit_offset - hStream->GetPos()).ValueOrDie());

  const uint32_t kHeaderSize = 192;
  if (hStream->BitsRemaining() < kHeaderSize) {
    return false;
  }

  // Item 1: The object number of the first object in the shared objects
  // section.
  uint32_t dwFirstSharedObjNum = hStream->GetBits(32);
  if (!dwFirstSharedObjNum) {
    return false;
  }

  // Item 2: The location of the first object in the shared objects section.
  const FX_FILESIZE szFirstSharedObjLoc =
      HintsOffsetToFileOffset(hStream->GetBits(32));
  if (!szFirstSharedObjLoc) {
    return false;
  }

  // Item 3: The number of shared object entries for the first page.
  first_page_shared_objs_ = hStream->GetBits(32);

  // Item 4: The number of shared object entries for the shared objects
  // section, including the number of shared object entries for the first page.
  uint32_t dwSharedObjTotal = hStream->GetBits(32);

  // Item 5: The number of bits needed to represent the greatest number of
  // objects in a shared object group.
  uint32_t dwSharedObjNumBits = hStream->GetBits(16);
  if (dwSharedObjNumBits > 32) {
    return false;
  }

  // Item 6: The least length of a shared object group in bytes.
  uint32_t dwGroupLeastLen = hStream->GetBits(32);

  // Item 7: The number of bits needed to represent the difference between the
  // greatest and least length of a shared object group, in bytes.
  uint32_t dwDeltaGroupLen = hStream->GetBits(16);

  // Trying to decode more than 32 bits isn't going to work when we write into
  // a uint32_t. Decoding 0 bits also makes no sense.
  if (!IsValidPageOffsetHintTableBitCount(dwDeltaGroupLen)) {
    return false;
  }

  if (dwFirstSharedObjNum > CPDF_Parser::kMaxObjectNumber ||
      first_page_shared_objs_ > CPDF_Parser::kMaxObjectNumber ||
      dwSharedObjTotal > CPDF_Parser::kMaxObjectNumber) {
    return false;
  }

  FX_SAFE_UINT32 required_bits = dwSharedObjTotal;
  required_bits *= dwDeltaGroupLen;
  if (!CanReadFromBitStream(hStream, required_bits)) {
    return false;
  }

  if (dwSharedObjTotal > 0) {
    uint32_t dwLastSharedObj = dwSharedObjTotal - 1;
    if (dwLastSharedObj > first_page_shared_objs_) {
      FX_SAFE_UINT32 safeObjNum = dwFirstSharedObjNum;
      safeObjNum += dwLastSharedObj - first_page_shared_objs_;
      if (!safeObjNum.IsValid()) {
        return false;
      }
    }
  }

  shared_obj_group_infos_.resize(dwSharedObjTotal);
  // Table F.6 - Shared object hint table, shared object group entries:
  // Item 1: A number that, when added to the least shared object
  // group length.
  FX_SAFE_FILESIZE prev_shared_group_end_offset = first_page_obj_offset_;
  for (uint32_t i = 0; i < dwSharedObjTotal; ++i) {
    if (i == first_page_shared_objs_) {
      prev_shared_group_end_offset = szFirstSharedObjLoc;
    }

    FX_SAFE_UINT32 safeObjLen = hStream->GetBits(dwDeltaGroupLen);
    safeObjLen += dwGroupLeastLen;
    if (!safeObjLen.IsValid()) {
      return false;
    }

    shared_obj_group_infos_[i].length_ = safeObjLen.ValueOrDie();
    shared_obj_group_infos_[i].offset_ =
        prev_shared_group_end_offset.ValueOrDie();
    prev_shared_group_end_offset += shared_obj_group_infos_[i].length_;
    if (!prev_shared_group_end_offset.IsValid()) {
      return false;
    }
  }

  hStream->ByteAlign();
  {
    // Item 2: A flag indicating whether the shared object signature (item 3) is
    // present.
    uint32_t signature_count = 0;
    for (uint32_t i = 0; i < dwSharedObjTotal; ++i) {
      signature_count += hStream->GetBits(1);
    }
    hStream->ByteAlign();
    // Item 3: (Only if item 2 is 1) The shared object signature, a 16-byte MD5
    // hash that uniquely identifies the resource that the group of objects
    // represents.
    if (signature_count) {
      required_bits = signature_count;
      required_bits *= 128;
      if (!CanReadFromBitStream(hStream, required_bits)) {
        return false;
      }

      hStream->SkipBits(required_bits.ValueOrDie());
      hStream->ByteAlign();
    }
  }
  // Item 4: A number equal to 1 less than the number of objects in the group.
  FX_SAFE_UINT32 cur_obj_num = linearized_->GetFirstPageObjNum();
  for (uint32_t i = 0; i < dwSharedObjTotal; ++i) {
    if (i == first_page_shared_objs_) {
      cur_obj_num = dwFirstSharedObjNum;
    }

    FX_SAFE_UINT32 obj_count =
        dwSharedObjNumBits ? hStream->GetBits(dwSharedObjNumBits) : 0;
    obj_count += 1;
    if (!obj_count.IsValid()) {
      return false;
    }

    uint32_t obj_num = cur_obj_num.ValueOrDie();
    cur_obj_num += obj_count.ValueOrDie();
    if (!cur_obj_num.IsValid()) {
      return false;
    }

    shared_obj_group_infos_[i].start_obj_num_ = obj_num;
    shared_obj_group_infos_[i].objects_count_ = obj_count.ValueOrDie();
  }

  hStream->ByteAlign();
  return true;
}

bool CPDF_HintTables::GetPagePos(uint32_t index,
                                 FX_FILESIZE* szPageStartPos,
                                 FX_FILESIZE* szPageLength,
                                 uint32_t* dwObjNum) const {
  if (index >= linearized_->GetPageCount()) {
    return false;
  }

  *szPageStartPos = page_infos_[index].page_offset();
  *szPageLength = page_infos_[index].page_length();
  *dwObjNum = page_infos_[index].start_obj_num();
  return true;
}

CPDF_DataAvail::DocAvailStatus CPDF_HintTables::CheckPage(uint32_t index) {
  if (index == linearized_->GetFirstPageNo()) {
    return CPDF_DataAvail::kDataAvailable;
  }

  if (index >= linearized_->GetPageCount()) {
    return CPDF_DataAvail::kDataError;
  }

  const uint32_t dwLength = page_infos_[index].page_length();
  if (!dwLength) {
    return CPDF_DataAvail::kDataError;
  }

  if (!validator_->CheckDataRangeAndRequestIfUnavailable(
          page_infos_[index].page_offset(), dwLength)) {
    return CPDF_DataAvail::kDataNotAvailable;
  }

  // Download data of shared objects in the page.
  for (const uint32_t dwIndex : page_infos_[index].Identifiers()) {
    if (dwIndex >= shared_obj_group_infos_.size()) {
      continue;
    }
    const SharedObjGroupInfo& shared_group_info =
        shared_obj_group_infos_[dwIndex];

    if (!shared_group_info.offset_ || !shared_group_info.length_) {
      return CPDF_DataAvail::kDataError;
    }

    if (!validator_->CheckDataRangeAndRequestIfUnavailable(
            shared_group_info.offset_, shared_group_info.length_)) {
      return CPDF_DataAvail::kDataNotAvailable;
    }
  }
  return CPDF_DataAvail::kDataAvailable;
}

bool CPDF_HintTables::LoadHintStream(CPDF_Stream* pHintStream) {
  if (!pHintStream || !linearized_->HasHintTable()) {
    return false;
  }

  RetainPtr<const CPDF_Object> pOffset =
      pHintStream->GetDict()->GetObjectFor("S");
  if (!pOffset || !pOffset->IsNumber()) {
    return false;
  }

  int shared_hint_table_offset = pOffset->GetInteger();
  if (shared_hint_table_offset <= 0) {
    return false;
  }

  auto pAcc =
      pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(pHintStream));
  pAcc->LoadAllDataFiltered();

  uint32_t size = pAcc->GetSize();
  // The header section of page offset hint table is 36 bytes.
  // The header section of shared object hint table is 24 bytes.
  // Hint table has at least 60 bytes.
  const uint32_t kMinStreamLength = 60;
  if (size < kMinStreamLength) {
    return false;
  }

  FX_SAFE_UINT32 safe_shared_hint_table_offset = shared_hint_table_offset;
  if (!safe_shared_hint_table_offset.IsValid() ||
      size < safe_shared_hint_table_offset.ValueOrDie()) {
    return false;
  }

  CFX_BitStream bs(pAcc->GetSpan().first(size));
  return ReadPageHintTable(&bs) &&
         ReadSharedObjHintTable(&bs, shared_hint_table_offset);
}

FX_FILESIZE CPDF_HintTables::HintsOffsetToFileOffset(
    uint32_t hints_offset) const {
  FX_SAFE_FILESIZE file_offset = hints_offset;
  if (!file_offset.IsValid()) {
    return 0;
  }

  // The resulting positions shall be interpreted as if the primary hint stream
  // itself were not present. That is, a position greater than the hint stream
  // offset shall have the hint stream length added to it to determine the
  // actual offset relative to the beginning of the file.
  // See ISO 32000-1:2008 spec, annex F.4 (Hint tables).
  // Note: The PDF spec does not mention this, but positions equal to the hint
  // stream offset also need to have the hint stream length added to it. e.g.
  // There exists linearized PDFs generated by Adobe software that have this
  // property.
  if (file_offset.ValueOrDie() >= linearized_->GetHintStart()) {
    file_offset += linearized_->GetHintLength();
  }

  return file_offset.ValueOrDefault(0);
}
