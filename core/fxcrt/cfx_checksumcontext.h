// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_CHECKSUMCONTEXT_H_
#define CORE_FXCRT_CFX_CHECKSUMCONTEXT_H_

#include <memory>

#include "core/fdrm/crypto/fx_crypt.h"
#include "core/fxcrt/xml/cfx_saxreader.h"

class CFX_ChecksumContext {
 public:
  CFX_ChecksumContext();
  ~CFX_ChecksumContext();

  void StartChecksum();
  void Update(const CFX_ByteStringC& bsText);
  bool UpdateChecksum(const CFX_RetainPtr<IFX_SeekableReadStream>& pSrcFile,
                      FX_FILESIZE offset = 0,
                      size_t size = 0);
  void FinishChecksum();
  CFX_ByteString GetChecksum() const;

 private:
  std::unique_ptr<CFX_SAXReader> m_pSAXReader;
  std::unique_ptr<CRYPT_sha1_context> m_pByteContext;
  CFX_ByteString m_bsChecksum;
};

#endif  // CORE_FXCRT_CFX_CHECKSUMCONTEXT_H_
