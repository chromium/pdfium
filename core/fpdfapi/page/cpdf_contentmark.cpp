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
  if (m_Ref)
    result->m_Ref.Reset(new MarkData(*m_Ref));

  return result;
}

size_t CPDF_ContentMark::CountItems() const {
  return m_Ref->CountItems();
}

const CPDF_ContentMarkItem& CPDF_ContentMark::GetItem(size_t i) const {
  ASSERT(i < CountItems());
  return m_Ref->GetItem(i);
}

int CPDF_ContentMark::GetMarkedContentID() const {
  if (!m_Ref)
    return -1;

  return m_Ref->GetMarkedContentID();
}

void CPDF_ContentMark::AddMark(ByteString name,
                               const CPDF_Dictionary* pDict,
                               bool bDirect) {
  if (!m_Ref)
    m_Ref.Reset(new MarkData());

  m_Ref->AddMark(std::move(name), pDict, bDirect);
}

void CPDF_ContentMark::DeleteLastMark() {
  if (!m_Ref)
    return;

  m_Ref->DeleteLastMark();
  if (CountItems() == 0)
    m_Ref.Reset();
}

CPDF_ContentMark::MarkData::MarkData() {}

CPDF_ContentMark::MarkData::MarkData(const MarkData& src)
    : m_Marks(src.m_Marks) {}

CPDF_ContentMark::MarkData::~MarkData() {}

size_t CPDF_ContentMark::MarkData::CountItems() const {
  return m_Marks.size();
}

const CPDF_ContentMarkItem& CPDF_ContentMark::MarkData::GetItem(
    size_t index) const {
  return m_Marks[index];
}

int CPDF_ContentMark::MarkData::GetMarkedContentID() const {
  for (const auto& mark : m_Marks) {
    const CPDF_Dictionary* pDict = mark.GetParam();
    if (pDict && pDict->KeyExist("MCID"))
      return pDict->GetIntegerFor("MCID");
  }
  return -1;
}

void CPDF_ContentMark::MarkData::AddMark(ByteString name,
                                         const CPDF_Dictionary* pDict,
                                         bool bDirect) {
  CPDF_ContentMarkItem item;
  item.SetName(std::move(name));
  if (pDict) {
    if (bDirect)
      item.SetDirectDict(ToDictionary(pDict->Clone()));
    else
      item.SetPropertiesDict(pDict);
  }
  m_Marks.push_back(std::move(item));
}

void CPDF_ContentMark::MarkData::DeleteLastMark() {
  if (!m_Marks.empty())
    m_Marks.pop_back();
}
