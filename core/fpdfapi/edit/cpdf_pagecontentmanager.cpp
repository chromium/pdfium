// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_pagecontentmanager.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"

CPDF_PageContentManager::CPDF_PageContentManager(
    CPDF_PageObjectHolder* obj_holder)
    : obj_holder_(obj_holder), doc_(obj_holder_->GetDocument()) {
  CPDF_Dictionary* page_dict = obj_holder_->GetDict();
  CPDF_Object* contents_obj = page_dict->GetObjectFor("Contents");
  CPDF_Array* contents_array = ToArray(contents_obj);
  if (contents_array) {
    contents_array_ = contents_array;
    return;
  }

  CPDF_Reference* contents_reference = ToReference(contents_obj);
  if (contents_reference) {
    CPDF_Object* indirect_obj = contents_reference->GetDirect();
    if (!indirect_obj)
      return;

    contents_array = indirect_obj->AsArray();
    if (contents_array)
      contents_array_ = contents_array;
    else if (indirect_obj->IsStream())
      contents_stream_ = indirect_obj->AsStream();
  }
}

CPDF_PageContentManager::~CPDF_PageContentManager() = default;

CPDF_Stream* CPDF_PageContentManager::GetStreamByIndex(size_t stream_index) {
  if (contents_stream_)
    return stream_index == 0 ? contents_stream_.Get() : nullptr;

  if (contents_array_) {
    CPDF_Reference* stream_reference =
        ToReference(contents_array_->GetObjectAt(stream_index));
    if (!stream_reference)
      return nullptr;

    return stream_reference->GetDirect()->AsStream();
  }

  return nullptr;
}

size_t CPDF_PageContentManager::AddStream(std::ostringstream* buf) {
  CPDF_Stream* new_stream = doc_->NewIndirect<CPDF_Stream>();
  new_stream->SetData(buf);

  // If there is one Content stream (not in an array), now there will be two, so
  // create an array with the old and the new one. The new one's index is 1.
  if (contents_stream_) {
    CPDF_Array* new_contents_array = doc_->NewIndirect<CPDF_Array>();
    new_contents_array->Add(contents_stream_->MakeReference(doc_.Get()));
    new_contents_array->Add(new_stream->MakeReference(doc_.Get()));

    CPDF_Dictionary* page_dict = obj_holder_->GetDict();
    page_dict->SetFor("Contents",
                      new_contents_array->MakeReference(doc_.Get()));
    contents_array_ = new_contents_array;
    contents_stream_ = nullptr;
    return 1;
  }

  // If there is an array, just add the new stream to it, at the last position.
  if (contents_array_) {
    contents_array_->Add(new_stream->MakeReference(doc_.Get()));
    return contents_array_->GetCount() - 1;
  }

  // There were no Contents, so add the new stream as the single Content stream.
  // Its index is 0.
  CPDF_Dictionary* page_dict = obj_holder_->GetDict();
  page_dict->SetFor("Contents", new_stream->MakeReference(doc_.Get()));
  contents_stream_ = new_stream;
  return 0;
}
