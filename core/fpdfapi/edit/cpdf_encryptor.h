// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_EDIT_CPDF_ENCRYPTOR_H_
#define CORE_FPDFAPI_EDIT_CPDF_ENCRYPTOR_H_

#include <stdint.h>

class CPDF_CryptoHandler;

class CPDF_Encryptor {
 public:
  CPDF_Encryptor(CPDF_CryptoHandler* pHandler,
                 int objnum,
                 uint8_t* src_data,
                 uint32_t src_size);
  ~CPDF_Encryptor();

  uint32_t GetSize() const { return m_dwSize; }
  uint8_t* GetData() const { return m_pData; }

 private:
  uint8_t* m_pData;
  uint32_t m_dwSize;
  bool m_bNewBuf;
};

#endif  // CORE_FPDFAPI_EDIT_CPDF_ENCRYPTOR_H_
