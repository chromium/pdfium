// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_encryptor.h"
#include "core/fpdfapi/parser/cpdf_crypto_handler.h"

CPDF_Encryptor::CPDF_Encryptor(CPDF_CryptoHandler* pHandler,
                               int objnum,
                               uint8_t* src_data,
                               uint32_t src_size)
    : m_pData(nullptr), m_dwSize(0), m_bNewBuf(false) {
  if (src_size == 0)
    return;

  if (!pHandler) {
    m_pData = (uint8_t*)src_data;
    m_dwSize = src_size;
    return;
  }
  m_dwSize = pHandler->EncryptGetSize(objnum, 0, src_data, src_size);
  m_pData = FX_Alloc(uint8_t, m_dwSize);
  pHandler->EncryptContent(objnum, 0, src_data, src_size, m_pData, m_dwSize);
  m_bNewBuf = true;
}

CPDF_Encryptor::~CPDF_Encryptor() {
  if (m_bNewBuf)
    FX_Free(m_pData);
}
