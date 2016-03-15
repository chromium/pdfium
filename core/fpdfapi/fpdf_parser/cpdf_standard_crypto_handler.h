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
  FX_DWORD DecryptGetSize(FX_DWORD src_size) override;
  void* DecryptStart(FX_DWORD objnum, FX_DWORD gennum) override;
  FX_BOOL DecryptStream(void* context,
                        const uint8_t* src_buf,
                        FX_DWORD src_size,
                        CFX_BinaryBuf& dest_buf) override;
  FX_BOOL DecryptFinish(void* context, CFX_BinaryBuf& dest_buf) override;
  FX_DWORD EncryptGetSize(FX_DWORD objnum,
                          FX_DWORD version,
                          const uint8_t* src_buf,
                          FX_DWORD src_size) override;
  FX_BOOL EncryptContent(FX_DWORD objnum,
                         FX_DWORD version,
                         const uint8_t* src_buf,
                         FX_DWORD src_size,
                         uint8_t* dest_buf,
                         FX_DWORD& dest_size) override;

  FX_BOOL Init(int cipher, const uint8_t* key, int keylen);

 protected:
  virtual void CryptBlock(FX_BOOL bEncrypt,
                          FX_DWORD objnum,
                          FX_DWORD gennum,
                          const uint8_t* src_buf,
                          FX_DWORD src_size,
                          uint8_t* dest_buf,
                          FX_DWORD& dest_size);
  virtual void* CryptStart(FX_DWORD objnum, FX_DWORD gennum, FX_BOOL bEncrypt);
  virtual FX_BOOL CryptStream(void* context,
                              const uint8_t* src_buf,
                              FX_DWORD src_size,
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
