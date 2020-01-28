// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_cmapmanager.h"

#include <utility>

#include "core/fpdfapi/font/cpdf_cid2unicodemap.h"
#include "core/fpdfapi/font/cpdf_cmap.h"
#include "third_party/base/ptr_util.h"

namespace {

RetainPtr<const CPDF_CMap> LoadPredefinedCMap(ByteStringView name) {
  if (!name.IsEmpty() && name[0] == '/')
    name = name.Last(name.GetLength() - 1);
  return pdfium::MakeRetain<CPDF_CMap>(name);
}

}  // namespace

CPDF_CMapManager::CPDF_CMapManager() = default;

CPDF_CMapManager::~CPDF_CMapManager() = default;

RetainPtr<const CPDF_CMap> CPDF_CMapManager::GetPredefinedCMap(
    const ByteString& name) {
  auto it = m_CMaps.find(name);
  if (it != m_CMaps.end())
    return it->second;

  RetainPtr<const CPDF_CMap> pCMap = LoadPredefinedCMap(name.AsStringView());
  if (!name.IsEmpty())
    m_CMaps[name] = pCMap;

  return pCMap;
}

CPDF_CID2UnicodeMap* CPDF_CMapManager::GetCID2UnicodeMap(CIDSet charset) {
  if (!m_CID2UnicodeMaps[charset]) {
    m_CID2UnicodeMaps[charset] =
        pdfium::MakeUnique<CPDF_CID2UnicodeMap>(charset);
  }
  return m_CID2UnicodeMaps[charset].get();
}
