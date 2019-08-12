// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_cid2unicodemap.h"

#include "core/fpdfapi/font/cpdf_fontglobals.h"

CPDF_CID2UnicodeMap::CPDF_CID2UnicodeMap() = default;

CPDF_CID2UnicodeMap::~CPDF_CID2UnicodeMap() = default;

bool CPDF_CID2UnicodeMap::IsLoaded() const {
  return !m_pEmbeddedMap.empty();
}

wchar_t CPDF_CID2UnicodeMap::UnicodeFromCID(uint16_t CID) const {
  if (m_Charset == CIDSET_UNICODE)
    return CID;

  if (CID < m_pEmbeddedMap.size())
    return m_pEmbeddedMap[CID];

  return 0;
}

void CPDF_CID2UnicodeMap::Load(CIDSet charset) {
  m_Charset = charset;
  m_pEmbeddedMap =
      CPDF_FontGlobals::GetInstance()->GetEmbeddedToUnicode(charset);
}
