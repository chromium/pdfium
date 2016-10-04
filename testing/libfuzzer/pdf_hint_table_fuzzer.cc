// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_hint_tables.h"

struct DummyLinearizedDictionary {
  int end_of_first_page_offset;
  int number_of_pages;
  int first_page_object_number;
  int first_page_number;
  int primary_hint_stream_offset;
  int primary_hint_stream_length;
  int shared_hint_table_offset;
};

int32_t GetData(const int32_t** data32, const uint8_t** data, size_t* size) {
  const int32_t* ret = *data32;
  ++(*data32);
  *data += 4;
  *size -= 4;
  return *ret;
}

class HintTableForFuzzing : public CPDF_HintTables {
 public:
  HintTableForFuzzing(DummyLinearizedDictionary* dict,
                      CPDF_Dictionary* linearized_dict)
      : CPDF_HintTables(nullptr, linearized_dict), dict_(dict) {}
  ~HintTableForFuzzing() {}

  void Fuzz(const uint8_t* data, size_t size) {
    if (dict_->shared_hint_table_offset <= 0)
      return;

    if (size < static_cast<size_t>(dict_->shared_hint_table_offset))
      return;

    CFX_BitStream bs;
    bs.Init(data, size);
    if (!ReadPageHintTable(&bs))
      return;
    ReadSharedObjHintTable(&bs, dict_->shared_hint_table_offset);
  }

 private:
  int GetEndOfFirstPageOffset() const override {
    return dict_->end_of_first_page_offset;
  }
  int GetNumberOfPages() const override { return dict_->number_of_pages; }
  int GetFirstPageObjectNumber() const override {
    return dict_->first_page_object_number;
  }
  int GetFirstPageNumber() const override { return dict_->first_page_number; }
  int ReadPrimaryHintStreamOffset() const override {
    return dict_->primary_hint_stream_offset;
  }
  int ReadPrimaryHintStreamLength() const override {
    return dict_->primary_hint_stream_length;
  }

  DummyLinearizedDictionary* const dict_;
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Need 28 bytes for |dummy_dict|.
  // The header section of page offset hint table is 36 bytes.
  // The header section of shared object hint table is 24 bytes.
  if (size < 28 + 36 + 24)
    return 0;

  const int32_t* data32 = reinterpret_cast<const int32_t*>(data);
  DummyLinearizedDictionary dummy_dict;
  dummy_dict.end_of_first_page_offset = GetData(&data32, &data, &size);
  dummy_dict.number_of_pages = GetData(&data32, &data, &size);
  dummy_dict.first_page_object_number = GetData(&data32, &data, &size);
  dummy_dict.first_page_number = GetData(&data32, &data, &size);
  dummy_dict.primary_hint_stream_offset = GetData(&data32, &data, &size);
  dummy_dict.primary_hint_stream_length = GetData(&data32, &data, &size);
  dummy_dict.shared_hint_table_offset = GetData(&data32, &data, &size);

  CPDF_Dictionary* dummy_linearized_dict = new CPDF_Dictionary;

  {
    HintTableForFuzzing hint_table(&dummy_dict, dummy_linearized_dict);
    hint_table.Fuzz(data, size);
  }

  dummy_linearized_dict->Release();
  return 0;
}
