// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_read_validator.h"

#include <algorithm>

#include "third_party/base/logging.h"

namespace {

constexpr FX_FILESIZE kAlignBlockValue = 512;

FX_FILESIZE AlignDown(FX_FILESIZE offset) {
  return offset > 0 ? (offset - offset % kAlignBlockValue) : 0;
}

FX_FILESIZE AlignUp(FX_FILESIZE offset) {
  FX_SAFE_FILESIZE safe_result = AlignDown(offset);
  safe_result += kAlignBlockValue;
  if (safe_result.IsValid())
    return safe_result.ValueOrDie();
  return offset;
}

}  // namespace

CPDF_ReadValidator::Session::Session(CPDF_ReadValidator* validator)
    : validator_(validator) {
  ASSERT(validator_);
  saved_read_error_ = validator_->read_error_;
  saved_has_unavailable_data_ = validator_->has_unavailable_data_;
  validator_->ResetErrors();
}

CPDF_ReadValidator::Session::~Session() {
  validator_->read_error_ |= saved_read_error_;
  validator_->has_unavailable_data_ |= saved_has_unavailable_data_;
}

CPDF_ReadValidator::CPDF_ReadValidator(
    const CFX_RetainPtr<IFX_SeekableReadStream>& file_read,
    CPDF_DataAvail::FileAvail* file_avail)
    : file_read_(file_read),
      file_avail_(file_avail),
      read_error_(false),
      has_unavailable_data_(false) {
  ASSERT(file_read_);
}

CPDF_ReadValidator::~CPDF_ReadValidator() {}

void CPDF_ReadValidator::ResetErrors() {
  read_error_ = false;
  has_unavailable_data_ = false;
}

bool CPDF_ReadValidator::ReadBlock(void* buffer,
                                   FX_FILESIZE offset,
                                   size_t size) {
  // correct values checks:
  if (!pdfium::base::IsValueInRangeForNumericType<uint32_t>(size))
    return false;

  FX_SAFE_FILESIZE end_offset = offset;
  end_offset += size;
  if (!end_offset.IsValid() || end_offset.ValueOrDie() > GetSize())
    return false;

  if (!file_avail_ ||
      file_avail_->IsDataAvail(offset, static_cast<uint32_t>(size))) {
    if (file_read_->ReadBlock(buffer, offset, size))
      return true;
    read_error_ = true;
  }
  has_unavailable_data_ = true;
  ScheduleDownload(offset, size);
  return false;
}

FX_FILESIZE CPDF_ReadValidator::GetSize() {
  return file_read_->GetSize();
}

void CPDF_ReadValidator::ScheduleDownload(FX_FILESIZE offset, size_t size) {
  const FX_SAFE_UINT32 safe_size = size;
  if (safe_size.IsValid())
    ScheduleDataDownload(offset, safe_size.ValueOrDie());
}

void CPDF_ReadValidator::ScheduleDataDownload(FX_FILESIZE offset,
                                              uint32_t size) {
  if (!hints_ || size == 0)
    return;

  const FX_FILESIZE start_segment_offset = AlignDown(offset);
  FX_SAFE_FILESIZE end_segment_offset = offset;
  end_segment_offset += size;
  if (!end_segment_offset.IsValid()) {
    NOTREACHED();
    return;
  }
  end_segment_offset =
      std::min(GetSize(), AlignUp(end_segment_offset.ValueOrDie()));

  FX_SAFE_UINT32 segment_size = end_segment_offset;
  segment_size -= start_segment_offset;
  if (!segment_size.IsValid()) {
    NOTREACHED();
    return;
  }
  hints_->AddSegment(start_segment_offset, segment_size.ValueOrDie());
}

void CPDF_ReadValidator::ScheduleDownloadWholeFile() {
  const FX_SAFE_UINT32 safe_size = GetSize();
  if (safe_size.IsValid())
    ScheduleDataDownload(0, safe_size.ValueOrDie());
}

bool CPDF_ReadValidator::IsDataRangeAvailable(FX_FILESIZE offset,
                                              uint32_t size) const {
  return !file_avail_ || file_avail_->IsDataAvail(offset, size);
}

bool CPDF_ReadValidator::IsWholeFileAvailable() {
  const FX_SAFE_UINT32 safe_size = GetSize();
  return safe_size.IsValid() ? IsDataRangeAvailable(0, safe_size.ValueOrDie())
                             : false;
}
