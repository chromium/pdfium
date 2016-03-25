// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_IPDF_SECURITY_HANDLER_H_
#define CORE_FPDFAPI_FPDF_PARSER_IPDF_SECURITY_HANDLER_H_

#include "core/fxcrt/include/fx_system.h"

class CPDF_Parser;
class CPDF_Dictionary;
class IPDF_CryptoHandler;

#define FXCIPHER_NONE 0
#define FXCIPHER_RC4 1
#define FXCIPHER_AES 2
#define FXCIPHER_AES2 3

class IPDF_SecurityHandler {
 public:
  virtual ~IPDF_SecurityHandler();
  virtual FX_BOOL OnInit(CPDF_Parser* pParser,
                         CPDF_Dictionary* pEncryptDict) = 0;

  virtual uint32_t GetPermissions() = 0;
  virtual FX_BOOL GetCryptInfo(int& cipher,
                               const uint8_t*& buffer,
                               int& keylen) = 0;

  virtual FX_BOOL IsMetadataEncrypted() = 0;
  virtual IPDF_CryptoHandler* CreateCryptoHandler() = 0;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_IPDF_SECURITY_HANDLER_H_
