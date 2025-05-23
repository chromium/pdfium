// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_contentparser.h"

#include <utility>
#include <variant>

#include "constants/page_object.h"
#include "core/fpdfapi/font/cpdf_type3char.h"
#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/pauseindicator_iface.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_fillrenderoptions.h"

CPDF_ContentParser::CPDF_ContentParser(CPDF_Page* pPage)
    : current_stage_(Stage::kGetContent), page_object_holder_(pPage) {
  DCHECK(pPage);
  if (!pPage->GetDocument()) {
    current_stage_ = Stage::kComplete;
    return;
  }

  RetainPtr<CPDF_Object> pContent =
      pPage->GetMutableDict()->GetMutableDirectObjectFor(
          pdfium::page_object::kContents);
  if (!pContent) {
    HandlePageContentFailure();
    return;
  }

  const CPDF_Stream* pStream = pContent->AsStream();
  if (pStream) {
    HandlePageContentStream(pStream);
    return;
  }

  const CPDF_Array* pArray = pContent->AsArray();
  if (pArray && HandlePageContentArray(pArray)) {
    return;
  }

  HandlePageContentFailure();
}

CPDF_ContentParser::CPDF_ContentParser(
    RetainPtr<const CPDF_Stream> pStream,
    CPDF_PageObjectHolder* pPageObjectHolder,
    const CPDF_AllStates* pGraphicStates,
    const CFX_Matrix* pParentMatrix,
    CPDF_Type3Char* pType3Char,
    CPDF_Form::RecursionState* recursion_state)
    : current_stage_(Stage::kParse),
      page_object_holder_(pPageObjectHolder),
      type3_char_(pType3Char) {
  DCHECK(page_object_holder_);
  CFX_Matrix form_matrix =
      page_object_holder_->GetDict()->GetMatrixFor("Matrix");
  if (pGraphicStates) {
    form_matrix.Concat(pGraphicStates->current_transformation_matrix());
  }

  RetainPtr<const CPDF_Array> pBBox =
      page_object_holder_->GetDict()->GetArrayFor("BBox");
  CFX_FloatRect form_bbox;
  CPDF_Path ClipPath;
  if (pBBox) {
    form_bbox = pBBox->GetRect();
    ClipPath.Emplace();
    ClipPath.AppendFloatRect(form_bbox);
    ClipPath.Transform(form_matrix);
    if (pParentMatrix) {
      ClipPath.Transform(*pParentMatrix);
    }

    form_bbox = form_matrix.TransformRect(form_bbox);
    if (pParentMatrix) {
      form_bbox = pParentMatrix->TransformRect(form_bbox);
    }
  }

  RetainPtr<CPDF_Dictionary> pResources =
      page_object_holder_->GetMutableDict()->GetMutableDictFor("Resources");
  parser_ = std::make_unique<CPDF_StreamContentParser>(
      page_object_holder_->GetDocument(),
      page_object_holder_->GetMutablePageResources(),
      page_object_holder_->GetMutableResources(), pParentMatrix,
      page_object_holder_, std::move(pResources), form_bbox, pGraphicStates,
      recursion_state);
  parser_->GetCurStates()->set_current_transformation_matrix(form_matrix);
  parser_->GetCurStates()->set_parent_matrix(form_matrix);
  if (ClipPath.HasRef()) {
    parser_->GetCurStates()->mutable_clip_path().AppendPathWithAutoMerge(
        ClipPath, CFX_FillRenderOptions::FillType::kWinding);
  }
  if (page_object_holder_->GetTransparency().IsGroup()) {
    CPDF_GeneralState& state = parser_->GetCurStates()->mutable_general_state();
    state.SetBlendType(BlendMode::kNormal);
    state.SetStrokeAlpha(1.0f);
    state.SetFillAlpha(1.0f);
    state.SetSoftMask(nullptr);
  }
  single_stream_ = pdfium::MakeRetain<CPDF_StreamAcc>(std::move(pStream));
  single_stream_->LoadAllDataFiltered();
  data_ = single_stream_->GetSpan();
}

CPDF_ContentParser::~CPDF_ContentParser() = default;

CPDF_PageObjectHolder::CTMMap CPDF_ContentParser::TakeAllCTMs() {
  return parser_ ? parser_->TakeAllCTMs() : CPDF_PageObjectHolder::CTMMap();
}

// Returning |true| means that there is more content to be processed and
// Continue() should be called again. Returning |false| means that we've
// completed the parse and Continue() is complete.
bool CPDF_ContentParser::Continue(PauseIndicatorIface* pPause) {
  while (current_stage_ == Stage::kGetContent) {
    current_stage_ = GetContent();
    if (pPause && pPause->NeedToPauseNow()) {
      return true;
    }
  }

  if (current_stage_ == Stage::kPrepareContent) {
    current_stage_ = PrepareContent();
  }

  while (current_stage_ == Stage::kParse) {
    current_stage_ = Parse();
    if (pPause && pPause->NeedToPauseNow()) {
      return true;
    }
  }

  if (current_stage_ == Stage::kCheckClip) {
    current_stage_ = CheckClip();
  }

  DCHECK_EQ(current_stage_, Stage::kComplete);
  return false;
}

CPDF_ContentParser::Stage CPDF_ContentParser::GetContent() {
  DCHECK_EQ(current_stage_, Stage::kGetContent);
  DCHECK(page_object_holder_->IsPage());
  RetainPtr<const CPDF_Array> pContent =
      page_object_holder_->GetDict()->GetArrayFor(
          pdfium::page_object::kContents);
  RetainPtr<const CPDF_Stream> pStreamObj = ToStream(
      pContent ? pContent->GetDirectObjectAt(current_offset_) : nullptr);
  stream_array_[current_offset_] =
      pdfium::MakeRetain<CPDF_StreamAcc>(std::move(pStreamObj));
  stream_array_[current_offset_]->LoadAllDataFiltered();
  current_offset_++;

  return current_offset_ == streams_ ? Stage::kPrepareContent
                                     : Stage::kGetContent;
}

CPDF_ContentParser::Stage CPDF_ContentParser::PrepareContent() {
  current_offset_ = 0;

  if (stream_array_.empty()) {
    data_ = single_stream_->GetSpan();
    return Stage::kParse;
  }

  FX_SAFE_UINT32 safe_size = 0;
  for (const auto& stream : stream_array_) {
    stream_segment_offsets_.push_back(safe_size.ValueOrDie());
    safe_size += stream->GetSize();
    safe_size += 1;
    if (!safe_size.IsValid()) {
      return Stage::kComplete;
    }
  }

  const size_t buffer_size = safe_size.ValueOrDie();
  auto buffer = FixedSizeDataVector<uint8_t>::TryZeroed(buffer_size);
  if (buffer.empty()) {
    data_.emplace<pdfium::raw_span<const uint8_t>>();
    return Stage::kComplete;
  }

  auto data_span = buffer.span();
  for (const auto& stream : stream_array_) {
    data_span = fxcrt::spancpy(data_span, stream->GetSpan());
    data_span.front() = ' ';
    data_span = data_span.subspan<1u>();
  }
  stream_array_.clear();
  data_ = std::move(buffer);
  return Stage::kParse;
}

CPDF_ContentParser::Stage CPDF_ContentParser::Parse() {
  if (!parser_) {
    recursion_state_.parsed_set.clear();
    parser_ = std::make_unique<CPDF_StreamContentParser>(
        page_object_holder_->GetDocument(),
        page_object_holder_->GetMutablePageResources(), nullptr, nullptr,
        page_object_holder_, page_object_holder_->GetMutableResources(),
        page_object_holder_->GetBBox(), nullptr, &recursion_state_);
    parser_->GetCurStates()->mutable_color_state().SetDefault();
  }
  if (current_offset_ >= GetData().size()) {
    return Stage::kCheckClip;
  }

  if (stream_segment_offsets_.empty()) {
    stream_segment_offsets_.push_back(0);
  }

  static constexpr uint32_t kParseStepLimit = 100;
  current_offset_ += parser_->Parse(GetData(), current_offset_, kParseStepLimit,
                                    stream_segment_offsets_);
  return Stage::kParse;
}

CPDF_ContentParser::Stage CPDF_ContentParser::CheckClip() {
  if (type3_char_) {
    type3_char_->InitializeFromStreamData(parser_->IsColored(),
                                          parser_->GetType3Data());
  }

  for (auto& pObj : *page_object_holder_) {
    if (!pObj->IsActive()) {
      continue;
    }
    CPDF_ClipPath& clip_path = pObj->mutable_clip_path();
    if (!clip_path.HasRef()) {
      continue;
    }
    if (clip_path.GetPathCount() != 1) {
      continue;
    }
    if (clip_path.GetTextCount() > 0) {
      continue;
    }

    CPDF_Path path = clip_path.GetPath(0);
    if (!path.IsRect() || pObj->IsShading()) {
      continue;
    }

    CFX_PointF point0 = path.GetPoint(0);
    CFX_PointF point2 = path.GetPoint(2);
    CFX_FloatRect old_rect(point0.x, point0.y, point2.x, point2.y);
    if (old_rect.Contains(pObj->GetRect())) {
      clip_path.SetNull();
    }
  }
  return Stage::kComplete;
}

void CPDF_ContentParser::HandlePageContentStream(const CPDF_Stream* pStream) {
  single_stream_ =
      pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(pStream));
  single_stream_->LoadAllDataFiltered();
  current_stage_ = Stage::kPrepareContent;
}

bool CPDF_ContentParser::HandlePageContentArray(const CPDF_Array* pArray) {
  streams_ = fxcrt::CollectionSize<uint32_t>(*pArray);
  if (streams_ == 0) {
    return false;
  }

  stream_array_.resize(streams_);
  return true;
}

void CPDF_ContentParser::HandlePageContentFailure() {
  current_stage_ = Stage::kComplete;
}

pdfium::span<const uint8_t> CPDF_ContentParser::GetData() const {
  if (is_owned()) {
    return std::get<FixedSizeDataVector<uint8_t>>(data_).span();
  }
  return std::get<pdfium::raw_span<const uint8_t>>(data_);
}
