// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_contentmark.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"

CPDF_ContentMark::CPDF_ContentMark() {}

CPDF_ContentMark::~CPDF_ContentMark() {}

std::unique_ptr<CPDF_ContentMark> CPDF_ContentMark::Clone() {
  auto result = pdfium::MakeUnique<CPDF_ContentMark>();
  if (m_pMarkData)
    result->m_pMarkData = pdfium::MakeRetain<MarkData>(*m_pMarkData);
  return result;
}

size_t CPDF_ContentMark::CountItems() const {
  return m_pMarkData ? m_pMarkData->CountItems() : 0;
}

bool CPDF_ContentMark::ContainsItem(const CPDF_ContentMarkItem* pItem) const {
  return m_pMarkData && m_pMarkData->ContainsItem(pItem);
}

CPDF_ContentMarkItem* CPDF_ContentMark::GetItem(size_t index) {
  return const_cast<CPDF_ContentMarkItem*>(
      static_cast<const CPDF_ContentMark*>(this)->GetItem(index));
}

const CPDF_ContentMarkItem* CPDF_ContentMark::GetItem(size_t index) const {
  ASSERT(index < CountItems());
  return m_pMarkData->GetItem(index);
}

int CPDF_ContentMark::GetMarkedContentID() const {
  return m_pMarkData ? m_pMarkData->GetMarkedContentID() : -1;
}

void CPDF_ContentMark::AddMark(ByteString name) {
  EnsureMarkDataExists();
  m_pMarkData->AddMark(std::move(name));
}

void CPDF_ContentMark::AddMarkWithDirectDict(ByteString name,
                                             CPDF_Dictionary* pDict) {
  EnsureMarkDataExists();
  m_pMarkData->AddMarkWithDirectDict(std::move(name), pDict);
}

void CPDF_ContentMark::AddMarkWithPropertiesDict(
    ByteString name,
    CPDF_Dictionary* pDict,
    const ByteString& property_name) {
  EnsureMarkDataExists();
  m_pMarkData->AddMarkWithPropertiesDict(std::move(name), pDict, property_name);
}

bool CPDF_ContentMark::RemoveMark(CPDF_ContentMarkItem* pMarkItem) {
  return m_pMarkData && m_pMarkData->RemoveMark(pMarkItem);
}

void CPDF_ContentMark::EnsureMarkDataExists() {
  if (!m_pMarkData)
    m_pMarkData = pdfium::MakeRetain<MarkData>();
}

void CPDF_ContentMark::DeleteLastMark() {
  if (!m_pMarkData)
    return;

  m_pMarkData->DeleteLastMark();
  if (CountItems() == 0)
    m_pMarkData.Reset();
}

size_t CPDF_ContentMark::FindFirstDifference(
    const CPDF_ContentMark* other) const {
  if (m_pMarkData == other->m_pMarkData)
    return CountItems();

  size_t min_len = std::min(CountItems(), other->CountItems());

  for (size_t i = 0; i < min_len; ++i) {
    if (GetItem(i) != other->GetItem(i))
      return i;
  }
  return min_len;
}

CPDF_ContentMark::MarkData::MarkData() {}

CPDF_ContentMark::MarkData::MarkData(const MarkData& src)
    : m_Marks(src.m_Marks) {}

CPDF_ContentMark::MarkData::~MarkData() {}

size_t CPDF_ContentMark::MarkData::CountItems() const {
  return m_Marks.size();
}

bool CPDF_ContentMark::MarkData::ContainsItem(
    const CPDF_ContentMarkItem* pItem) const {
  for (const auto pMark : m_Marks) {
    if (pMark.Get() == pItem)
      return true;
  }
  return false;
}

CPDF_ContentMarkItem* CPDF_ContentMark::MarkData::GetItem(size_t index) {
  return m_Marks[index].Get();
}

const CPDF_ContentMarkItem* CPDF_ContentMark::MarkData::GetItem(
    size_t index) const {
  return m_Marks[index].Get();
}

int CPDF_ContentMark::MarkData::GetMarkedContentID() const {
  for (const auto pMark : m_Marks) {
    const CPDF_Dictionary* pDict = pMark->GetParam();
    if (pDict && pDict->KeyExist("MCID"))
      return pDict->GetIntegerFor("MCID");
  }
  return -1;
}

void CPDF_ContentMark::MarkData::AddMark(ByteString name) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>(std::move(name));
  m_Marks.push_back(pItem);
}

void CPDF_ContentMark::MarkData::AddMarkWithDirectDict(ByteString name,
                                                       CPDF_Dictionary* pDict) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>(std::move(name));
  pItem->SetDirectDict(ToDictionary(pDict->Clone()));
  m_Marks.push_back(pItem);
}

void CPDF_ContentMark::MarkData::AddMarkWithPropertiesDict(
    ByteString name,
    CPDF_Dictionary* pDict,
    const ByteString& property_name) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>(std::move(name));
  pItem->SetPropertiesDict(pDict, property_name);
  m_Marks.push_back(pItem);
}

bool CPDF_ContentMark::MarkData::RemoveMark(CPDF_ContentMarkItem* pMarkItem) {
  for (auto it = m_Marks.begin(); it != m_Marks.end(); ++it) {
    if (it->Get() == pMarkItem) {
      m_Marks.erase(it);
      return true;
    }
  }
  return false;
}

void CPDF_ContentMark::MarkData::DeleteLastMark() {
  if (!m_Marks.empty())
    m_Marks.pop_back();
}
