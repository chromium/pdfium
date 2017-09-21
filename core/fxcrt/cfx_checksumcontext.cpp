// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_checksumcontext.h"

#include "core/fdrm/crypto/fx_crypt.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/xml/cfx_saxreaderhandler.h"
#include "third_party/base/ptr_util.h"

namespace {

struct FX_BASE64DATA {
  uint32_t data1 : 2;
  uint32_t data2 : 6;
  uint32_t data3 : 4;
  uint32_t data4 : 4;
  uint32_t data5 : 6;
  uint32_t data6 : 2;
  uint32_t data7 : 8;
};

const char g_FXBase64EncoderMap[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};

void Base64EncodePiece(const FX_BASE64DATA& src, int32_t iBytes, char dst[4]) {
  dst[0] = g_FXBase64EncoderMap[src.data2];
  uint32_t b = src.data1 << 4;
  if (iBytes > 1) {
    b |= src.data4;
  }
  dst[1] = g_FXBase64EncoderMap[b];
  if (iBytes > 1) {
    b = src.data3 << 2;
    if (iBytes > 2) {
      b |= src.data6;
    }
    dst[2] = g_FXBase64EncoderMap[b];
    if (iBytes > 2) {
      dst[3] = g_FXBase64EncoderMap[src.data5];
    } else {
      dst[3] = '=';
    }
  } else {
    dst[2] = dst[3] = '=';
  }
}

int32_t Base64EncodeA(const uint8_t* pSrc, int32_t iSrcLen, char* pDst) {
  ASSERT(pSrc);
  if (iSrcLen < 1) {
    return 0;
  }
  if (!pDst) {
    int32_t iDstLen = iSrcLen / 3 * 4;
    if ((iSrcLen % 3) != 0) {
      iDstLen += 4;
    }
    return iDstLen;
  }
  FX_BASE64DATA srcData;
  int32_t iBytes = 3;
  char* pDstEnd = pDst;
  while (iSrcLen > 0) {
    if (iSrcLen > 2) {
      ((uint8_t*)&srcData)[0] = *pSrc++;
      ((uint8_t*)&srcData)[1] = *pSrc++;
      ((uint8_t*)&srcData)[2] = *pSrc++;
      iSrcLen -= 3;
    } else {
      *((uint32_t*)&srcData) = 0;
      ((uint8_t*)&srcData)[0] = *pSrc++;
      if (iSrcLen > 1) {
        ((uint8_t*)&srcData)[1] = *pSrc++;
      }
      iBytes = iSrcLen;
      iSrcLen = 0;
    }
    Base64EncodePiece(srcData, iBytes, pDstEnd);
    pDstEnd += 4;
  }
  return pDstEnd - pDst;
}

}  // namespace

CFX_ChecksumContext::CFX_ChecksumContext() {}

CFX_ChecksumContext::~CFX_ChecksumContext() {}

void CFX_ChecksumContext::StartChecksum() {
  FinishChecksum();
  m_pByteContext = pdfium::MakeUnique<CRYPT_sha1_context>();
  CRYPT_SHA1Start(m_pByteContext.get());
  m_bsChecksum.clear();
  m_pSAXReader = pdfium::MakeUnique<CFX_SAXReader>();
}

bool CFX_ChecksumContext::UpdateChecksum(
    const RetainPtr<IFX_SeekableReadStream>& pSrcFile,
    FX_FILESIZE offset,
    size_t size) {
  if (!m_pSAXReader || !pSrcFile)
    return false;

  if (size < 1)
    size = pSrcFile->GetSize();

  CFX_SAXReaderHandler handler(this);
  m_pSAXReader->SetHandler(&handler);
  if (m_pSAXReader->StartParse(
          pSrcFile, (uint32_t)offset, (uint32_t)size,
          CFX_SaxParseMode_NotSkipSpace | CFX_SaxParseMode_NotConvert_amp |
              CFX_SaxParseMode_NotConvert_lt | CFX_SaxParseMode_NotConvert_gt |
              CFX_SaxParseMode_NotConvert_sharp) < 0) {
    return false;
  }
  return m_pSAXReader->ContinueParse() > 99;
}

void CFX_ChecksumContext::FinishChecksum() {
  m_pSAXReader.reset();
  if (m_pByteContext) {
    uint8_t digest[20];
    memset(digest, 0, 20);
    CRYPT_SHA1Finish(m_pByteContext.get(), digest);
    int32_t nLen = Base64EncodeA(digest, 20, nullptr);
    char* pBuffer = m_bsChecksum.GetBuffer(nLen);
    Base64EncodeA(digest, 20, pBuffer);
    m_bsChecksum.ReleaseBuffer(nLen);
    m_pByteContext.reset();
  }
}

ByteString CFX_ChecksumContext::GetChecksum() const {
  return m_bsChecksum;
}

void CFX_ChecksumContext::Update(const ByteStringView& bsText) {
  if (!m_pByteContext)
    return;

  CRYPT_SHA1Update(m_pByteContext.get(), bsText.raw_str(), bsText.GetLength());
}
