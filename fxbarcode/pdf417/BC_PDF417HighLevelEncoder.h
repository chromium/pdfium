// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_PDF417_BC_PDF417HIGHLEVELENCODER_H_
#define FXBARCODE_PDF417_BC_PDF417HIGHLEVELENCODER_H_

#include <vector>

#include "core/fxcrt/fx_string.h"
#include "fxbarcode/pdf417/BC_PDF417Compaction.h"

class CBC_PDF417HighLevelEncoder {
 public:
  static WideString EncodeHighLevel(WideString msg,
                                    Compaction compaction,
                                    int32_t& e);
  static void Inverse();
  static void Initialize();
  static void Finalize();

 private:
  static constexpr int32_t TEXT_COMPACTION = 0;
  static constexpr int32_t BYTE_COMPACTION = 1;
  static constexpr int32_t NUMERIC_COMPACTION = 2;
  static constexpr int32_t SUBMODE_PUNCTUATION = 3;

  static int32_t EncodeText(const WideString& msg,
                            size_t startpos,
                            size_t count,
                            int32_t initialSubmode,
                            WideString* sb);
  static void EncodeBinary(const std::vector<uint8_t>& bytes,
                           size_t startpos,
                           size_t count,
                           int32_t startmode,
                           WideString* sb);
  static void EncodeNumeric(const WideString& msg,
                            size_t startpos,
                            size_t count,
                            WideString* sb);
  static size_t DetermineConsecutiveDigitCount(WideString msg, size_t startpos);
  static size_t DetermineConsecutiveTextCount(WideString msg, size_t startpos);
  static Optional<size_t> DetermineConsecutiveBinaryCount(
      WideString msg,
      std::vector<uint8_t>* bytes,
      size_t startpos);

  friend class PDF417HighLevelEncoder_EncodeNumeric_Test;
  friend class PDF417HighLevelEncoder_EncodeBinary_Test;
  friend class PDF417HighLevelEncoder_EncodeText_Test;
  friend class PDF417HighLevelEncoder_ConsecutiveDigitCount_Test;
  friend class PDF417HighLevelEncoder_ConsecutiveTextCount_Test;
  friend class PDF417HighLevelEncoder_ConsecutiveBinaryCount_Test;
};

#endif  // FXBARCODE_PDF417_BC_PDF417HIGHLEVELENCODER_H_
