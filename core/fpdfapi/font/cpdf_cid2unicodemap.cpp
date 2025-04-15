// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_cid2unicodemap.h"

#include "core/fpdfapi/font/cpdf_fontglobals.h"

CPDF_CID2UnicodeMap::CPDF_CID2UnicodeMap(CIDSet charset)
    : charset_(charset),
      embedded_map_(
          CPDF_FontGlobals::GetInstance()->GetEmbeddedToUnicode(charset_)) {}

CPDF_CID2UnicodeMap::~CPDF_CID2UnicodeMap() = default;

bool CPDF_CID2UnicodeMap::IsLoaded() const {
  return !embedded_map_.empty();
}

wchar_t CPDF_CID2UnicodeMap::UnicodeFromCID(uint16_t cid) const {
  if (charset_ == CIDSET_UNICODE) {
    return cid;
  }
  return cid < embedded_map_.size() ? embedded_map_[cid] : 0;
}
