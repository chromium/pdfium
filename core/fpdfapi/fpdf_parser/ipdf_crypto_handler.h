// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_IPDF_CRYPTO_HANDLER_H_
#define CORE_FPDFAPI_FPDF_PARSER_IPDF_CRYPTO_HANDLER_H_

#include "core/fxcrt/include/fx_basic.h"

class CPDF_Dictionary;
class IPDF_SecurityHandler;

class IPDF_CryptoHandler {
 public:
  virtual ~IPDF_CryptoHandler();

  virtual FX_BOOL Init(CPDF_Dictionary* pEncryptDict,
                       IPDF_SecurityHandler* pSecurityHandler) = 0;

  virtual FX_DWORD DecryptGetSize(FX_DWORD src_size) = 0;
  virtual void* DecryptStart(FX_DWORD objnum, FX_DWORD gennum) = 0;
  virtual FX_BOOL DecryptStream(void* context,
                                const uint8_t* src_buf,
                                FX_DWORD src_size,
                                CFX_BinaryBuf& dest_buf) = 0;

  virtual FX_BOOL DecryptFinish(void* context, CFX_BinaryBuf& dest_buf) = 0;
  virtual FX_DWORD EncryptGetSize(FX_DWORD objnum,
                                  FX_DWORD version,
                                  const uint8_t* src_buf,
                                  FX_DWORD src_size) = 0;

  virtual FX_BOOL EncryptContent(FX_DWORD objnum,
                                 FX_DWORD version,
                                 const uint8_t* src_buf,
                                 FX_DWORD src_size,
                                 uint8_t* dest_buf,
                                 FX_DWORD& dest_size) = 0;

  void Decrypt(FX_DWORD objnum, FX_DWORD version, CFX_ByteString& str);
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_IPDF_CRYPTO_HANDLER_H_
