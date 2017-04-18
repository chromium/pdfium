// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cxml_databufacc.h"

CXML_DataBufAcc::CXML_DataBufAcc(const uint8_t* pBuffer, size_t size)
    : m_pBuffer(pBuffer), m_dwSize(size), m_dwCurPos(0) {}

CXML_DataBufAcc::~CXML_DataBufAcc() {}

bool CXML_DataBufAcc::ReadNextBlock() {
  if (m_dwCurPos >= m_dwSize)
    return false;

  m_dwCurPos = m_dwSize;
  return true;
}
