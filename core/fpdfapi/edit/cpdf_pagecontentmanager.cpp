// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_pagecontentmanager.h"

#include <map>
#include <numeric>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/adapters.h"
#include "third_party/base/numerics/safe_conversions.h"

CPDF_PageContentManager::CPDF_PageContentManager(
    CPDF_PageObjectHolder* page_obj_holder,
    CPDF_IndirectObjectHolder* indirect_obj_holder)
    : page_obj_holder_(page_obj_holder),
      indirect_obj_holder_(indirect_obj_holder) {
  RetainPtr<CPDF_Dictionary> page_dict = page_obj_holder_->GetMutableDict();
  RetainPtr<CPDF_Object> contents_obj =
      page_dict->GetMutableObjectFor("Contents");
  RetainPtr<CPDF_Array> contents_array = ToArray(contents_obj);
  if (contents_array) {
    contents_array_ = std::move(contents_array);
    return;
  }

  RetainPtr<CPDF_Reference> contents_reference = ToReference(contents_obj);
  if (contents_reference) {
    RetainPtr<CPDF_Object> indirect_obj =
        contents_reference->GetMutableDirect();
    if (!indirect_obj)
      return;

    contents_array.Reset(indirect_obj->AsMutableArray());
    if (contents_array)
      contents_array_ = std::move(contents_array);
    else if (indirect_obj->IsStream())
      contents_stream_.Reset(indirect_obj->AsMutableStream());
  }
}

CPDF_PageContentManager::~CPDF_PageContentManager() = default;

RetainPtr<CPDF_Stream> CPDF_PageContentManager::GetStreamByIndex(
    size_t stream_index) {
  if (contents_stream_)
    return stream_index == 0 ? contents_stream_ : nullptr;

  if (!contents_array_)
    return nullptr;

  RetainPtr<CPDF_Reference> stream_reference =
      ToReference(contents_array_->GetMutableObjectAt(stream_index));
  if (!stream_reference)
    return nullptr;

  return ToStream(stream_reference->GetMutableDirect());
}

size_t CPDF_PageContentManager::AddStream(fxcrt::ostringstream* buf) {
  auto new_stream = indirect_obj_holder_->NewIndirect<CPDF_Stream>();
  new_stream->SetDataFromStringstream(buf);

  // If there is one Content stream (not in an array), now there will be two, so
  // create an array with the old and the new one. The new one's index is 1.
  if (contents_stream_) {
    auto new_contents_array = indirect_obj_holder_->NewIndirect<CPDF_Array>();
    new_contents_array->AppendNew<CPDF_Reference>(
        indirect_obj_holder_, contents_stream_->GetObjNum());
    new_contents_array->AppendNew<CPDF_Reference>(indirect_obj_holder_,
                                                  new_stream->GetObjNum());

    RetainPtr<CPDF_Dictionary> page_dict = page_obj_holder_->GetMutableDict();
    page_dict->SetNewFor<CPDF_Reference>("Contents", indirect_obj_holder_,
                                         new_contents_array->GetObjNum());
    contents_array_ = std::move(new_contents_array);
    contents_stream_ = nullptr;
    return 1;
  }

  // If there is an array, just add the new stream to it, at the last position.
  if (contents_array_) {
    contents_array_->AppendNew<CPDF_Reference>(indirect_obj_holder_,
                                               new_stream->GetObjNum());
    return contents_array_->size() - 1;
  }

  // There were no Contents, so add the new stream as the single Content stream.
  // Its index is 0.
  RetainPtr<CPDF_Dictionary> page_dict = page_obj_holder_->GetMutableDict();
  page_dict->SetNewFor<CPDF_Reference>("Contents", indirect_obj_holder_,
                                       new_stream->GetObjNum());
  contents_stream_ = std::move(new_stream);
  return 0;
}

void CPDF_PageContentManager::ScheduleRemoveStreamByIndex(size_t stream_index) {
  streams_to_remove_.insert(stream_index);
}

void CPDF_PageContentManager::ExecuteScheduledRemovals() {
  // This method assumes there are no dirty streams in the
  // CPDF_PageObjectHolder. If there were any, their indexes would need to be
  // updated.
  // Since this is only called by CPDF_PageContentGenerator::GenerateContent(),
  // which cleans up the dirty streams first, this should always be true.
  DCHECK(!page_obj_holder_->HasDirtyStreams());

  if (contents_stream_) {
    // Only stream that can be removed is 0.
    if (streams_to_remove_.find(0) != streams_to_remove_.end()) {
      RetainPtr<CPDF_Dictionary> page_dict = page_obj_holder_->GetMutableDict();
      page_dict->RemoveFor("Contents");
      contents_stream_ = nullptr;
    }
  } else if (contents_array_) {
    // Initialize a vector with the old stream indexes. This will be used to
    // build a map from the old to the new indexes.
    std::vector<size_t> streams_left(contents_array_->size());
    std::iota(streams_left.begin(), streams_left.end(), 0);

    // In reverse order so as to not change the indexes in the middle of the
    // loop, remove the streams.
    for (size_t stream_index : pdfium::base::Reversed(streams_to_remove_)) {
      contents_array_->RemoveAt(stream_index);
      streams_left.erase(streams_left.begin() + stream_index);
    }

    // Create a mapping from the old to the new stream indexes, shifted due to
    // the deletion of the |streams_to_remove_|.
    std::map<size_t, size_t> stream_index_mapping;
    for (size_t i = 0; i < streams_left.size(); ++i)
      stream_index_mapping[streams_left[i]] = i;

    // Update the page objects' content stream indexes.
    for (const auto& obj : *page_obj_holder_) {
      int32_t old_stream_index = obj->GetContentStream();
      int32_t new_stream_index = pdfium::base::checked_cast<int32_t>(
          stream_index_mapping[old_stream_index]);
      obj->SetContentStream(new_stream_index);
    }

    // Even if there is a single content stream now, keep the array with a
    // single element. It's valid, a second stream might be added soon, and the
    // complexity of removing it is not worth it.
  }

  streams_to_remove_.clear();
}
