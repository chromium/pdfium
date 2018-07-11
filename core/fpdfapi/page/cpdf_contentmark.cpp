// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_contentmark.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"

CPDF_ContentMark::CPDF_ContentMark() {}

CPDF_ContentMark::~CPDF_ContentMark() {}

std::unique_ptr<CPDF_ContentMark> CPDF_ContentMark::Clone() {
  auto result = pdfium::MakeUnique<CPDF_ContentMark>();
  if (m_pMarkData)
    result->m_pMarkData.Reset(new MarkData(*m_pMarkData));

  return result;
}

size_t CPDF_ContentMark::CountItems() const {
  if (!m_pMarkData)
    return 0;

  return m_pMarkData->CountItems();
}

CPDF_ContentMarkItem* CPDF_ContentMark::GetItem(size_t i) {
  ASSERT(i < CountItems());
  return m_pMarkData->GetItem(i);
}

const CPDF_ContentMarkItem* CPDF_ContentMark::GetItem(size_t i) const {
  ASSERT(i < CountItems());
  return m_pMarkData->GetItem(i);
}

int CPDF_ContentMark::GetMarkedContentID() const {
  if (!m_pMarkData)
    return -1;

  return m_pMarkData->GetMarkedContentID();
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

void CPDF_ContentMark::AddMarkWithPropertiesDict(ByteString name,
                                                 CPDF_Dictionary* pDict) {
  EnsureMarkDataExists();
  m_pMarkData->AddMarkWithPropertiesDict(std::move(name), pDict);
}

void CPDF_ContentMark::EnsureMarkDataExists() {
  if (!m_pMarkData)
    m_pMarkData.Reset(new MarkData());
}

void CPDF_ContentMark::DeleteLastMark() {
  if (!m_pMarkData)
    return;

  m_pMarkData->DeleteLastMark();
  if (CountItems() == 0)
    m_pMarkData.Reset();
}

CPDF_ContentMark::MarkData::MarkData() {}

CPDF_ContentMark::MarkData::MarkData(const MarkData& src)
    : m_Marks(src.m_Marks) {}

CPDF_ContentMark::MarkData::~MarkData() {}

size_t CPDF_ContentMark::MarkData::CountItems() const {
  return m_Marks.size();
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
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>();
  pItem->SetName(std::move(name));
  m_Marks.push_back(pItem);
}

void CPDF_ContentMark::MarkData::AddMarkWithDirectDict(ByteString name,
                                                       CPDF_Dictionary* pDict) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>();
  pItem->SetName(std::move(name));
  pItem->SetDirectDict(ToDictionary(pDict->Clone()));
  m_Marks.push_back(pItem);
}

void CPDF_ContentMark::MarkData::AddMarkWithPropertiesDict(
    ByteString name,
    CPDF_Dictionary* pDict) {
  auto pItem = pdfium::MakeRetain<CPDF_ContentMarkItem>();
  pItem->SetName(std::move(name));
  pItem->SetPropertiesDict(pDict);
  m_Marks.push_back(pItem);
}

void CPDF_ContentMark::MarkData::DeleteLastMark() {
  if (!m_Marks.empty())
    m_Marks.pop_back();
}
