// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_CMAPS_FPDF_CMAPS_H_
#define CORE_FPDFAPI_CMAPS_FPDF_CMAPS_H_

#include <stdint.h>

#include "core/fxcrt/unowned_ptr_exclusion.h"

namespace fxcmap {

struct DWordCIDMap {
  uint16_t hi_word_;
  uint16_t lo_word_low_;
  uint16_t lo_word_high_;
  uint16_t cid_;
};

struct CMap {
  enum class Type : bool { kSingle, kRange };

  UNOWNED_PTR_EXCLUSION const char* name_;              // POD struct.
  UNOWNED_PTR_EXCLUSION const uint16_t* word_map_;      // POD struct.
  UNOWNED_PTR_EXCLUSION const DWordCIDMap* dword_map_;  // POD struct.
  uint16_t word_count_;
  uint16_t dword_count_;
  Type word_map_type_;
  int8_t use_offset_;
};

uint16_t CIDFromCharCode(const CMap* cmap, uint32_t charcode);
uint32_t CharCodeFromCID(const CMap* cmap, uint16_t cid);

}  // namespace fxcmap

#endif  // CORE_FPDFAPI_CMAPS_FPDF_CMAPS_H_
