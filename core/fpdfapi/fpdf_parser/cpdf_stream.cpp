// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/cpdf_stream.h"

#include "core/include/fpdfapi/cpdf_dictionary.h"
#include "core/include/fpdfapi/fpdf_parser_decode.h"

CPDF_Stream::CPDF_Stream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict)
    : m_pDict(pDict),
      m_dwSize(size),
      m_GenNum(kMemoryBasedGenNum),
      m_pDataBuf(pData) {}

CPDF_Stream::~CPDF_Stream() {
  if (IsMemoryBased())
    FX_Free(m_pDataBuf);

  if (m_pDict)
    m_pDict->Release();
}

CPDF_Object::Type CPDF_Stream::GetType() const {
  return STREAM;
}

CPDF_Dictionary* CPDF_Stream::GetDict() const {
  return m_pDict;
}

bool CPDF_Stream::IsStream() const {
  return true;
}

CPDF_Stream* CPDF_Stream::AsStream() {
  return this;
}

const CPDF_Stream* CPDF_Stream::AsStream() const {
  return this;
}

void CPDF_Stream::InitStreamInternal(CPDF_Dictionary* pDict) {
  if (pDict) {
    if (m_pDict)
      m_pDict->Release();
    m_pDict = pDict;
  }
  if (IsMemoryBased())
    FX_Free(m_pDataBuf);

  m_GenNum = 0;
  m_pFile = nullptr;
}

void CPDF_Stream::InitStream(uint8_t* pData,
                             FX_DWORD size,
                             CPDF_Dictionary* pDict) {
  InitStreamInternal(pDict);
  m_GenNum = kMemoryBasedGenNum;
  m_pDataBuf = FX_Alloc(uint8_t, size);
  if (pData)
    FXSYS_memcpy(m_pDataBuf, pData, size);

  m_dwSize = size;
  if (m_pDict)
    m_pDict->SetAtInteger("Length", size);
}

CPDF_Object* CPDF_Stream::Clone(FX_BOOL bDirect) const {
  CPDF_StreamAcc acc;
  acc.LoadAllData(this, TRUE);
  FX_DWORD streamSize = acc.GetSize();
  CPDF_Dictionary* pDict = GetDict();
  if (pDict)
    pDict = ToDictionary(pDict->Clone(bDirect));

  return new CPDF_Stream(acc.DetachData(), streamSize, pDict);
}

void CPDF_Stream::SetData(const uint8_t* pData,
                          FX_DWORD size,
                          FX_BOOL bCompressed,
                          FX_BOOL bKeepBuf) {
  if (IsMemoryBased())
    FX_Free(m_pDataBuf);
  m_GenNum = kMemoryBasedGenNum;

  if (bKeepBuf) {
    m_pDataBuf = const_cast<uint8_t*>(pData);
  } else {
    m_pDataBuf = FX_Alloc(uint8_t, size);
    if (pData) {
      FXSYS_memcpy(m_pDataBuf, pData, size);
    }
  }
  m_dwSize = size;
  if (!m_pDict)
    m_pDict = new CPDF_Dictionary;
  m_pDict->SetAtInteger("Length", size);
  if (!bCompressed) {
    m_pDict->RemoveAt("Filter");
    m_pDict->RemoveAt("DecodeParms");
  }
}

FX_BOOL CPDF_Stream::ReadRawData(FX_FILESIZE offset,
                                 uint8_t* buf,
                                 FX_DWORD size) const {
  if (!IsMemoryBased() && m_pFile)
    return m_pFile->ReadBlock(buf, offset, size);

  if (m_pDataBuf)
    FXSYS_memcpy(buf, m_pDataBuf + offset, size);

  return TRUE;
}

void CPDF_Stream::InitStreamFromFile(IFX_FileRead* pFile,
                                     CPDF_Dictionary* pDict) {
  InitStreamInternal(pDict);
  m_pFile = pFile;
  m_dwSize = (FX_DWORD)pFile->GetSize();
  if (m_pDict)
    m_pDict->SetAtInteger("Length", m_dwSize);
}

CFX_WideString CPDF_Stream::GetUnicodeText() const {
  CPDF_StreamAcc stream;
  stream.LoadAllData(this, FALSE);
  return PDF_DecodeText(stream.GetData(), stream.GetSize());
}

CPDF_StreamAcc::CPDF_StreamAcc()
    : m_pData(nullptr),
      m_dwSize(0),
      m_bNewBuf(FALSE),
      m_pImageParam(nullptr),
      m_pStream(nullptr),
      m_pSrcData(nullptr) {}

void CPDF_StreamAcc::LoadAllData(const CPDF_Stream* pStream,
                                 FX_BOOL bRawAccess,
                                 FX_DWORD estimated_size,
                                 FX_BOOL bImageAcc) {
  if (!pStream)
    return;

  m_pStream = pStream;
  if (pStream->IsMemoryBased() &&
      (!pStream->GetDict()->KeyExist("Filter") || bRawAccess)) {
    m_dwSize = pStream->GetRawSize();
    m_pData = pStream->GetRawData();
    return;
  }
  uint8_t* pSrcData;
  FX_DWORD dwSrcSize = pStream->GetRawSize();
  if (dwSrcSize == 0)
    return;

  if (!pStream->IsMemoryBased()) {
    pSrcData = m_pSrcData = FX_Alloc(uint8_t, dwSrcSize);
    if (!pStream->ReadRawData(0, pSrcData, dwSrcSize))
      return;
  } else {
    pSrcData = pStream->GetRawData();
  }
  uint8_t* pDecryptedData = pSrcData;
  FX_DWORD dwDecryptedSize = dwSrcSize;
  if (!pStream->GetDict()->KeyExist("Filter") || bRawAccess) {
    m_pData = pDecryptedData;
    m_dwSize = dwDecryptedSize;
  } else {
    FX_BOOL bRet = PDF_DataDecode(
        pDecryptedData, dwDecryptedSize, m_pStream->GetDict(), m_pData,
        m_dwSize, m_ImageDecoder, m_pImageParam, estimated_size, bImageAcc);
    if (!bRet) {
      m_pData = pDecryptedData;
      m_dwSize = dwDecryptedSize;
    }
  }
  if (pSrcData != pStream->GetRawData() && pSrcData != m_pData) {
    FX_Free(pSrcData);
  }
  if (pDecryptedData != pSrcData && pDecryptedData != m_pData) {
    FX_Free(pDecryptedData);
  }
  m_pSrcData = nullptr;
  m_bNewBuf = m_pData != pStream->GetRawData();
}

CPDF_StreamAcc::~CPDF_StreamAcc() {
  if (m_bNewBuf) {
    FX_Free(m_pData);
  }
  FX_Free(m_pSrcData);
}

const uint8_t* CPDF_StreamAcc::GetData() const {
  if (m_bNewBuf) {
    return m_pData;
  }
  if (!m_pStream) {
    return nullptr;
  }
  return m_pStream->GetRawData();
}

FX_DWORD CPDF_StreamAcc::GetSize() const {
  if (m_bNewBuf) {
    return m_dwSize;
  }
  if (!m_pStream) {
    return 0;
  }
  return m_pStream->GetRawSize();
}

uint8_t* CPDF_StreamAcc::DetachData() {
  if (m_bNewBuf) {
    uint8_t* p = m_pData;
    m_pData = nullptr;
    m_dwSize = 0;
    return p;
  }
  uint8_t* p = FX_Alloc(uint8_t, m_dwSize);
  FXSYS_memcpy(p, m_pData, m_dwSize);
  return p;
}
