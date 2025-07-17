// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"

#include <algorithm>
#include <utility>

#include "constants/transparency.h"
#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_contentparser.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/containers/unique_ptr_adapters.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/stl_util.h"

bool GraphicsData::operator<(const GraphicsData& other) const {
  if (!FXSYS_SafeEQ(fillAlpha, other.fillAlpha)) {
    return FXSYS_SafeLT(fillAlpha, other.fillAlpha);
  }
  if (!FXSYS_SafeEQ(strokeAlpha, other.strokeAlpha)) {
    return FXSYS_SafeLT(strokeAlpha, other.strokeAlpha);
  }
  return blendType < other.blendType;
}

bool FontData::operator<(const FontData& other) const {
  if (baseFont != other.baseFont) {
    return baseFont < other.baseFont;
  }
  return type < other.type;
}

CPDF_PageObjectHolder::CPDF_PageObjectHolder(
    CPDF_Document* pDoc,
    RetainPtr<CPDF_Dictionary> dict,
    RetainPtr<CPDF_Dictionary> pPageResources,
    RetainPtr<CPDF_Dictionary> pResources)
    : page_resources_(std::move(pPageResources)),
      resources_(std::move(pResources)),
      dict_(std::move(dict)),
      document_(pDoc) {
  DCHECK(dict_);
}

CPDF_PageObjectHolder::~CPDF_PageObjectHolder() = default;

bool CPDF_PageObjectHolder::IsPage() const {
  return false;
}

void CPDF_PageObjectHolder::StartParse(
    std::unique_ptr<CPDF_ContentParser> pParser) {
  DCHECK_EQ(parse_state_, ParseState::kNotParsed);
  parser_ = std::move(pParser);
  parse_state_ = ParseState::kParsing;
}

void CPDF_PageObjectHolder::ContinueParse(PauseIndicatorIface* pPause) {
  if (parse_state_ == ParseState::kParsed) {
    return;
  }

  DCHECK_EQ(parse_state_, ParseState::kParsing);
  if (parser_->Continue(pPause)) {
    return;
  }

  parse_state_ = ParseState::kParsed;
  document_->IncrementParsedPageCount();
  all_ctms_ = parser_->TakeAllCTMs();

  parser_.reset();
}

void CPDF_PageObjectHolder::AddImageMaskBoundingBox(const CFX_FloatRect& box) {
  mask_bounding_boxes_.push_back(box);
}

std::set<int32_t> CPDF_PageObjectHolder::TakeDirtyStreams() {
  auto dirty_streams = std::move(dirty_streams_);
  dirty_streams_.clear();
  return dirty_streams;
}

std::optional<ByteString> CPDF_PageObjectHolder::GraphicsMapSearch(
    const GraphicsData& gd) {
  auto it = graphics_map_.find(gd);
  if (it == graphics_map_.end()) {
    return std::nullopt;
  }

  return it->second;
}

void CPDF_PageObjectHolder::GraphicsMapInsert(const GraphicsData& gd,
                                              const ByteString& str) {
  graphics_map_[gd] = str;
}

std::optional<ByteString> CPDF_PageObjectHolder::FontsMapSearch(
    const FontData& fd) {
  auto it = fonts_map_.find(fd);
  if (it == fonts_map_.end()) {
    return std::nullopt;
  }

  return it->second;
}

void CPDF_PageObjectHolder::FontsMapInsert(const FontData& fd,
                                           const ByteString& str) {
  fonts_map_[fd] = str;
}

CFX_Matrix CPDF_PageObjectHolder::GetCTMAtBeginningOfStream(int32_t stream) {
  CHECK(stream >= 0 || stream == CPDF_PageObject::kNoContentStream);

  if (stream == 0 || all_ctms_.empty()) {
    return CFX_Matrix();
  }

  if (stream == CPDF_PageObject::kNoContentStream) {
    return all_ctms_.rbegin()->second;
  }

  // For all other cases, CTM at beginning of `stream` is the same value as CTM
  // at the end of the previous stream.
  return GetCTMAtEndOfStream(stream - 1);
}

CFX_Matrix CPDF_PageObjectHolder::GetCTMAtEndOfStream(int32_t stream) {
  // This code should never need to calculate the CTM for the end of
  // `CPDF_PageObject::kNoContentStream`, which uses a negative sentinel value.
  // All other streams have a non-negative index.
  CHECK_GE(stream, 0);

  if (all_ctms_.empty()) {
    return CFX_Matrix();
  }

  const auto it = all_ctms_.lower_bound(stream);
  return it != all_ctms_.end() ? it->second : all_ctms_.rbegin()->second;
}

void CPDF_PageObjectHolder::LoadTransparencyInfo() {
  RetainPtr<const CPDF_Dictionary> pGroup = dict_->GetDictFor("Group");
  if (!pGroup) {
    return;
  }

  if (pGroup->GetByteStringFor(pdfium::transparency::kGroupSubType) !=
      pdfium::transparency::kTransparency) {
    return;
  }
  transparency_.SetGroup();
  if (pGroup->GetIntegerFor(pdfium::transparency::kI)) {
    transparency_.SetIsolated();
  }
}

size_t CPDF_PageObjectHolder::GetActivePageObjectCount() const {
  size_t count = 0;
  for (const auto& page_object : page_object_list_) {
    if (page_object->IsActive()) {
      ++count;
    }
  }
  return count;
}

CPDF_PageObject* CPDF_PageObjectHolder::GetPageObjectByIndex(
    size_t index) const {
  return fxcrt::IndexInBounds(page_object_list_, index)
             ? page_object_list_[index].get()
             : nullptr;
}

void CPDF_PageObjectHolder::AppendPageObject(
    std::unique_ptr<CPDF_PageObject> pPageObj) {
  CHECK(pPageObj);
  page_object_list_.push_back(std::move(pPageObj));
}

bool CPDF_PageObjectHolder::InsertPageObjectAtIndex(
    size_t index,
    std::unique_ptr<CPDF_PageObject> page_obj) {
  CHECK(page_obj);
  if (index > page_object_list_.size()) {
    return false;
  }

  // Unsafe, but the compiler will not complain, because
  // std::deque::iterator::operator++() has not been marked as unsafe yet.
  page_object_list_.insert(UNSAFE_TODO(page_object_list_.begin() + index),
                           std::move(page_obj));
  return true;
}

std::unique_ptr<CPDF_PageObject> CPDF_PageObjectHolder::RemovePageObject(
    CPDF_PageObject* pPageObj) {
  auto it = std::ranges::find_if(page_object_list_,
                                 pdfium::MatchesUniquePtr(pPageObj));
  if (it == std::end(page_object_list_)) {
    return nullptr;
  }

  std::unique_ptr<CPDF_PageObject> result = std::move(*it);
  page_object_list_.erase(it);

  int32_t content_stream = pPageObj->GetContentStream();
  if (content_stream >= 0) {
    dirty_streams_.insert(content_stream);
  }

  return result;
}

bool CPDF_PageObjectHolder::ErasePageObjectAtIndex(size_t index) {
  if (index >= page_object_list_.size()) {
    return false;
  }

  // Unsafe, but the compiler will not complain, because
  // std::deque::iterator::operator++() has not been marked as unsafe yet.
  page_object_list_.erase(UNSAFE_TODO(page_object_list_.begin() + index));
  return true;
}
