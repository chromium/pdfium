// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_STREAMPARSER_H_
#define CORE_FPDFAPI_PAGE_CPDF_STREAMPARSER_H_

#include <stdint.h>

#include <array>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/string_pool_template.h"
#include "core/fxcrt/weak_ptr.h"

class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Object;
class CPDF_Stream;

class CPDF_StreamParser {
 public:
  enum ElementType { kEndOfData, kNumber, kKeyword, kName, kOther };

  explicit CPDF_StreamParser(pdfium::span<const uint8_t> span);
  CPDF_StreamParser(pdfium::span<const uint8_t> span,
                    const WeakPtr<ByteStringPool>& pPool);
  ~CPDF_StreamParser();

  ElementType ParseNextElement();
  ByteStringView GetWord() const {
    return ByteStringView(word_buffer_).First(word_size_);
  }
  uint32_t GetPos() const { return pos_; }
  void SetPos(uint32_t pos) { pos_ = pos; }
  const RetainPtr<CPDF_Object>& GetObject() const { return last_obj_; }
  RetainPtr<CPDF_Object> ReadNextObject(bool bAllowNestedArray,
                                        bool bInArray,
                                        uint32_t dwRecursionLevel);
  RetainPtr<CPDF_Stream> ReadInlineStream(CPDF_Document* pDoc,
                                          RetainPtr<CPDF_Dictionary> pDict,
                                          const CPDF_Object* pCSObj);

 private:
  friend class CPDFStreamParserTest_ReadHexString_Test;
  static constexpr uint32_t kMaxWordLength = 255;

  void GetNextWord(bool& bIsNumber);
  ByteString ReadString();
  DataVector<uint8_t> ReadHexString();
  bool PositionIsInBounds() const;

  uint32_t pos_ = 0;        // Current byte position within |buf_|.
  uint32_t word_size_ = 0;  // Current byte position within |word_buffer_|.
  WeakPtr<ByteStringPool> pool_;
  RetainPtr<CPDF_Object> last_obj_;
  pdfium::raw_span<const uint8_t> buf_;
  // Include space for NUL.
  std::array<uint8_t, kMaxWordLength + 1> word_buffer_ = {};
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_STREAMPARSER_H_
