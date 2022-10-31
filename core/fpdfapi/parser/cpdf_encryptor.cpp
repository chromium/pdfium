// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_encryptor.h"

#include <stdint.h>

#include "core/fpdfapi/parser/cpdf_crypto_handler.h"
#include "core/fxcrt/data_vector.h"
#include "third_party/base/check.h"

CPDF_Encryptor::CPDF_Encryptor(const CPDF_CryptoHandler* pHandler, int objnum)
    : m_pHandler(pHandler), m_ObjNum(objnum) {
  DCHECK(m_pHandler);
}

DataVector<uint8_t> CPDF_Encryptor::Encrypt(
    pdfium::span<const uint8_t> src_data) const {
  if (src_data.empty())
    return DataVector<uint8_t>();

  DataVector<uint8_t> result;
  size_t buf_size = m_pHandler->EncryptGetSize(src_data);
  result.resize(buf_size);
  m_pHandler->EncryptContent(m_ObjNum, 0, src_data, result.data(),
                             buf_size);  // Updates |buf_size| with actual.
  result.resize(buf_size);
  return result;
}

CPDF_Encryptor::~CPDF_Encryptor() = default;
