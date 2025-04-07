// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_SEGMENT_H_
#define CORE_FXCODEC_JBIG2_JBIG2_SEGMENT_H_

#include <memory>
#include <vector>

#include "core/fxcodec/jbig2/JBig2_Define.h"
#include "core/fxcodec/jbig2/JBig2_HuffmanTable.h"
#include "core/fxcodec/jbig2/JBig2_PatternDict.h"
#include "core/fxcodec/jbig2/JBig2_SymbolDict.h"

enum JBig2_SegmentState {
  JBIG2_SEGMENT_HEADER_UNPARSED,
  JBIG2_SEGMENT_DATA_UNPARSED,
  JBIG2_SEGMENT_PARSE_COMPLETE,
  JBIG2_SEGMENT_PAUSED,
  JBIG2_SEGMENT_ERROR
};

enum JBig2_ResultType {
  JBIG2_VOID_POINTER = 0,
  JBIG2_IMAGE_POINTER,
  JBIG2_SYMBOL_DICT_POINTER,
  JBIG2_PATTERN_DICT_POINTER,
  JBIG2_HUFFMAN_TABLE_POINTER
};

class CJBig2_Segment {
 public:
  CJBig2_Segment();
  ~CJBig2_Segment();

  uint32_t number_ = 0;
  union {
    struct {
      uint8_t type : 6;
      uint8_t page_association_size : 1;
      uint8_t deferred_non_retain : 1;
    } s;
    uint8_t c = 0;
  } flags_;
  int32_t referred_to_segment_count_ = 0;
  std::vector<uint32_t> referred_to_segment_numbers_;
  uint32_t page_association_ = 0;
  uint32_t data_length_ = 0;
  uint32_t header_length_ = 0;
  uint32_t data_offset_ = 0;
  uint64_t key_ = 0;
  JBig2_SegmentState state_ = JBIG2_SEGMENT_HEADER_UNPARSED;
  JBig2_ResultType result_type_ = JBIG2_VOID_POINTER;
  std::unique_ptr<CJBig2_SymbolDict> symbol_dict_;
  std::unique_ptr<CJBig2_PatternDict> pattern_dict_;
  std::unique_ptr<CJBig2_Image> image_;
  std::unique_ptr<CJBig2_HuffmanTable> huffman_table_;
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_SEGMENT_H_
