// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_contentmarks.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/check_op.h"

CPDF_ContentMarks::CPDF_ContentMarks() = default;

CPDF_ContentMarks::~CPDF_ContentMarks() = default;

std::unique_ptr<CPDF_ContentMarks> CPDF_ContentMarks::Clone() {
  auto result = std::make_unique<CPDF_ContentMarks>();
  if (mark_data_) {
    result->mark_data_ = pdfium::MakeRetain<MarkData>(*mark_data_);
  }
  return result;
}

size_t CPDF_ContentMarks::CountItems() const {
  return mark_data_ ? mark_data_->CountItems() : 0;
}

bool CPDF_ContentMarks::ContainsItem(const CPDF_ContentMarkItem* pItem) const {
  return mark_data_ && mark_data_->ContainsItem(pItem);
}

CPDF_ContentMarkItem* CPDF_ContentMarks::GetItem(size_t index) {
  return mark_data_->GetItem(index);
}

const CPDF_ContentMarkItem* CPDF_ContentMarks::GetItem(size_t index) const {
  return mark_data_->GetItem(index);
}

int CPDF_ContentMarks::GetMarkedContentID() const {
  return mark_data_ ? mark_data_->GetMarkedContentID() : -1;
}

void CPDF_ContentMarks::AddMark(ByteString name) {
  EnsureMarkDataExists();
  mark_data_->AddMark(std::move(name));
}

void CPDF_ContentMarks::AddMarkWithDirectDict(ByteString name,
                                              RetainPtr<CPDF_Dictionary> dict) {
  EnsureMarkDataExists();
  mark_data_->AddMarkWithDirectDict(std::move(name), std::move(dict));
}

void CPDF_ContentMarks::AddMarkWithPropertiesHolder(
    const ByteString& name,
    RetainPtr<CPDF_Dictionary> dict,
    const ByteString& property_name) {
  EnsureMarkDataExists();
  mark_data_->AddMarkWithPropertiesHolder(name, std::move(dict), property_name);
}

bool CPDF_ContentMarks::RemoveMark(CPDF_ContentMarkItem* pMarkItem) {
  return mark_data_ && mark_data_->RemoveMark(pMarkItem);
}

void CPDF_ContentMarks::EnsureMarkDataExists() {
  if (!mark_data_) {
    mark_data_ = pdfium::MakeRetain<MarkData>();
  }
}

size_t CPDF_ContentMarks::FindFirstDifference(
    const CPDF_ContentMarks* other) const {
  if (mark_data_ == other->mark_data_) {
    return CountItems();
  }

  size_t min_len = std::min(CountItems(), other->CountItems());

  for (size_t i = 0; i < min_len; ++i) {
    if (GetItem(i) != other->GetItem(i)) {
      return i;
    }
  }
  return min_len;
}

CPDF_ContentMarks::MarkData::MarkData() = default;

CPDF_ContentMarks::MarkData::MarkData(const MarkData& src)
    : marks_(src.marks_) {}

CPDF_ContentMarks::MarkData::~MarkData() = default;

size_t CPDF_ContentMarks::MarkData::CountItems() const {
  return marks_.size();
}

bool CPDF_ContentMarks::MarkData::ContainsItem(
    const CPDF_ContentMarkItem* pItem) const {
  for (const auto& pMark : marks_) {
    if (pMark == pItem) {
      return true;
    }
  }
  return false;
}

CPDF_ContentMarkItem* CPDF_ContentMarks::MarkData::GetItem(size_t index) {
  CHECK_LT(index, marks_.size());
  return marks_[index].Get();
}

const CPDF_ContentMarkItem* CPDF_ContentMarks::MarkData::GetItem(
    size_t index) const {
  CHECK_LT(index, marks_.size());
  return marks_[index].Get();
}

int CPDF_ContentMarks::MarkData::GetMarkedContentID() const {
  for (const auto& pMark : marks_) {
    RetainPtr<const CPDF_Dictionary> dict = pMark->GetParam();
    if (dict && dict->KeyExist("MCID")) {
      return dict->GetIntegerFor("MCID");
    }
  }
  return -1;
}

void CPDF_ContentMarks::MarkData::AddMark(ByteString name) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>(std::move(name));
  marks_.push_back(pItem);
}

void CPDF_ContentMarks::MarkData::AddMarkWithDirectDict(
    ByteString name,
    RetainPtr<CPDF_Dictionary> dict) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>(std::move(name));
  pItem->SetDirectDict(ToDictionary(dict->Clone()));
  marks_.push_back(pItem);
}

void CPDF_ContentMarks::MarkData::AddMarkWithPropertiesHolder(
    const ByteString& name,
    RetainPtr<CPDF_Dictionary> dict,
    const ByteString& property_name) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>(name);
  pItem->SetPropertiesHolder(std::move(dict), property_name);
  marks_.push_back(std::move(pItem));
}

bool CPDF_ContentMarks::MarkData::RemoveMark(CPDF_ContentMarkItem* pMarkItem) {
  for (auto it = marks_.begin(); it != marks_.end(); ++it) {
    if (*it == pMarkItem) {
      marks_.erase(it);
      return true;
    }
  }
  return false;
}
