// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/cpdf_clippathdata.h"

#include "core/fpdfapi/fpdf_page/include/cpdf_path.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_textobject.h"

CPDF_ClipPathData::CPDF_ClipPathData()
    : m_PathCount(0),
      m_pPathList(nullptr),
      m_pTypeList(nullptr),
      m_TextCount(0),
      m_pTextList(nullptr) {}

CPDF_ClipPathData::~CPDF_ClipPathData() {
  delete[] m_pPathList;
  FX_Free(m_pTypeList);

  for (int i = m_TextCount - 1; i > -1; i--)
    delete m_pTextList[i];
  FX_Free(m_pTextList);
}

CPDF_ClipPathData::CPDF_ClipPathData(const CPDF_ClipPathData& src) {
  m_pPathList = nullptr;
  m_pPathList = nullptr;
  m_pTextList = nullptr;

  m_PathCount = src.m_PathCount;
  if (m_PathCount) {
    int alloc_size = m_PathCount;
    if (alloc_size % 8)
      alloc_size += 8 - (alloc_size % 8);

    m_pPathList = new CPDF_Path[alloc_size];
    for (int i = 0; i < m_PathCount; i++)
      m_pPathList[i] = src.m_pPathList[i];

    m_pTypeList = FX_Alloc(uint8_t, alloc_size);
    FXSYS_memcpy(m_pTypeList, src.m_pTypeList, m_PathCount);
  } else {
    m_pPathList = nullptr;
    m_pTypeList = nullptr;
  }

  m_TextCount = src.m_TextCount;
  if (m_TextCount) {
    m_pTextList = FX_Alloc(CPDF_TextObject*, m_TextCount);
    for (int i = 0; i < m_TextCount; i++) {
      if (src.m_pTextList[i])
        m_pTextList[i] = src.m_pTextList[i]->Clone();
      else
        m_pTextList[i] = nullptr;
    }
  } else {
    m_pTextList = nullptr;
  }
}

void CPDF_ClipPathData::SetCount(int path_count, int text_count) {
  ASSERT(m_TextCount == 0 && m_PathCount == 0);
  if (path_count) {
    m_PathCount = path_count;
    int alloc_size = (path_count + 7) / 8 * 8;
    m_pPathList = new CPDF_Path[alloc_size];
    m_pTypeList = FX_Alloc(uint8_t, alloc_size);
  }

  if (text_count) {
    m_TextCount = text_count;
    m_pTextList = FX_Alloc(CPDF_TextObject*, text_count);
  }
}
