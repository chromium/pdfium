// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_DATABUFACC_H_
#define CORE_FXCRT_XML_CXML_DATABUFACC_H_

#include "core/fxcrt/fx_system.h"

class CXML_DataBufAcc {
 public:
  CXML_DataBufAcc(const uint8_t* pBuffer, size_t size);
  ~CXML_DataBufAcc();

  bool IsEOF() const { return m_dwCurPos >= m_dwSize; }
  FX_FILESIZE GetPosition() const {
    return static_cast<FX_FILESIZE>(m_dwCurPos);
  }
  bool ReadNextBlock();
  const uint8_t* GetBlockBuffer() const { return m_pBuffer; }
  size_t GetBlockSize() const { return m_dwSize; }

 private:
  const uint8_t* m_pBuffer;
  size_t m_dwSize;
  size_t m_dwCurPos;
};

#endif  // CORE_FXCRT_XML_CXML_DATABUFACC_H_
