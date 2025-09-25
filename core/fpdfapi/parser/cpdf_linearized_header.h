// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_LINEARIZED_HEADER_H_
#define CORE_FPDFAPI_PARSER_CPDF_LINEARIZED_HEADER_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/fx_types.h"

class CPDF_Dictionary;
class CPDF_SyntaxParser;

class CPDF_LinearizedHeader {
 public:
  ~CPDF_LinearizedHeader();
  static std::unique_ptr<CPDF_LinearizedHeader> Parse(
      CPDF_SyntaxParser* parser);

  // Will only return values > 0.
  FX_FILESIZE GetFileSize() const { return file_size_; }
  uint32_t GetFirstPageNo() const { return first_page_no_; }
  // Will only return values > 0.
  FX_FILESIZE GetMainXRefTableFirstEntryOffset() const {
    return main_xref_table_first_entry_offset_;
  }
  uint32_t GetPageCount() const { return page_count_; }
  // Will only return values > 0.
  FX_FILESIZE GetFirstPageEndOffset() const { return first_page_end_offset_; }
  // Will only return values in the range [1, `CPDF_Parser::kMaxObjectNumber`].
  uint32_t GetFirstPageObjNum() const { return first_page_obj_num_; }
  // Will only return values > 0.
  FX_FILESIZE GetLastXRefOffset() const { return last_xref_offset_; }

  bool HasHintTable() const;
  // Will only return values > 0.
  FX_FILESIZE GetHintStart() const { return hint_start_; }
  uint32_t GetHintLength() const { return hint_length_; }

 protected:
  CPDF_LinearizedHeader(const CPDF_Dictionary* dict,
                        FX_FILESIZE szLastXRefOffset);

 private:
  const FX_FILESIZE file_size_;
  const uint32_t first_page_no_;
  const FX_FILESIZE main_xref_table_first_entry_offset_;
  const uint32_t page_count_;
  const FX_FILESIZE first_page_end_offset_;
  const uint32_t first_page_obj_num_;
  const FX_FILESIZE last_xref_offset_;
  FX_FILESIZE hint_start_ = 0;
  uint32_t hint_length_ = 0;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_LINEARIZED_HEADER_H_
