// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_CPDF_STANDARD_CRYPTO_HANDLER_H_
#define CORE_FPDFAPI_FPDF_PARSER_CPDF_STANDARD_CRYPTO_HANDLER_H_

#include "core/fpdfapi/fpdf_parser/ipdf_crypto_handler.h"

class CPDF_StandardCryptoHandler : public IPDF_CryptoHandler {
 public:
  CPDF_StandardCryptoHandler();
  ~CPDF_StandardCryptoHandler() override;

  // IPDF_CryptoHandler
  FX_BOOL Init(CPDF_Dictionary* pEncryptDict,
               IPDF_SecurityHandler* pSecurityHandler) override;
  uint32_t DecryptGetSize(uint32_t src_size) override;
  void* DecryptStart(uint32_t objnum, uint32_t gennum) override;
  FX_BOOL DecryptStream(void* context,
                        const uint8_t* src_buf,
                        uint32_t src_size,
                        CFX_BinaryBuf& dest_buf) override;
  FX_BOOL DecryptFinish(void* context, CFX_BinaryBuf& dest_buf) override;
  uint32_t EncryptGetSize(uint32_t objnum,
                          uint32_t version,
                          const uint8_t* src_buf,
                          uint32_t src_size) override;
  FX_BOOL EncryptContent(uint32_t objnum,
                         uint32_t version,
                         const uint8_t* src_buf,
                         uint32_t src_size,
                         uint8_t* dest_buf,
                         uint32_t& dest_size) override;

  FX_BOOL Init(int cipher, const uint8_t* key, int keylen);

 protected:
  virtual void CryptBlock(FX_BOOL bEncrypt,
                          uint32_t objnum,
                          uint32_t gennum,
                          const uint8_t* src_buf,
                          uint32_t src_size,
                          uint8_t* dest_buf,
                          uint32_t& dest_size);
  virtual void* CryptStart(uint32_t objnum, uint32_t gennum, FX_BOOL bEncrypt);
  virtual FX_BOOL CryptStream(void* context,
                              const uint8_t* src_buf,
                              uint32_t src_size,
                              CFX_BinaryBuf& dest_buf,
                              FX_BOOL bEncrypt);
  virtual FX_BOOL CryptFinish(void* context,
                              CFX_BinaryBuf& dest_buf,
                              FX_BOOL bEncrypt);

  uint8_t m_EncryptKey[32];
  int m_KeyLen;
  int m_Cipher;
  uint8_t* m_pAESContext;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_CPDF_STANDARD_CRYPTO_HANDLER_H_
