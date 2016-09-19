// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream_acc.h"
#include "core/fpdfapi/fpdf_parser/include/fpdf_parser_decode.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/stl_util.h"

CPDF_Stream::CPDF_Stream() {}

CPDF_Stream::CPDF_Stream(uint8_t* pData, uint32_t size, CPDF_Dictionary* pDict)
    : m_pDict(pDict),
      m_dwSize(size),
      m_pDataBuf(pData) {}

CPDF_Stream::~CPDF_Stream() {}

CPDF_Object::Type CPDF_Stream::GetType() const {
  return STREAM;
}

CPDF_Dictionary* CPDF_Stream::GetDict() const {
  return m_pDict.get();
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

void CPDF_Stream::InitStream(const uint8_t* pData,
                             uint32_t size,
                             CPDF_Dictionary* pDict) {
  m_pDict.reset(pDict);
  m_bMemoryBased = true;
  m_pFile = nullptr;
  m_pDataBuf.reset(FX_Alloc(uint8_t, size));
  if (pData)
    FXSYS_memcpy(m_pDataBuf.get(), pData, size);
  m_dwSize = size;
  if (m_pDict)
    m_pDict->SetIntegerFor("Length", m_dwSize);
}

void CPDF_Stream::InitStreamFromFile(IFX_FileRead* pFile,
                                     CPDF_Dictionary* pDict) {
  m_pDict.reset(pDict);
  m_bMemoryBased = false;
  m_pDataBuf.reset();
  m_pFile = pFile;
  m_dwSize = pdfium::base::checked_cast<uint32_t>(pFile->GetSize());
  if (m_pDict)
    m_pDict->SetIntegerFor("Length", m_dwSize);
}

CPDF_Object* CPDF_Stream::Clone() const {
  return CloneObjectNonCyclic(false);
}

CPDF_Object* CPDF_Stream::CloneNonCyclic(
    bool bDirect,
    std::set<const CPDF_Object*>* pVisited) const {
  pVisited->insert(this);
  CPDF_StreamAcc acc;
  acc.LoadAllData(this, TRUE);
  uint32_t streamSize = acc.GetSize();
  CPDF_Dictionary* pDict = GetDict();
  if (pDict && !pdfium::ContainsKey(*pVisited, pDict)) {
    pDict = ToDictionary(
        static_cast<CPDF_Object*>(pDict)->CloneNonCyclic(bDirect, pVisited));
  }

  return new CPDF_Stream(acc.DetachData(), streamSize, pDict);
}

void CPDF_Stream::SetData(const uint8_t* pData, uint32_t size) {
  m_bMemoryBased = true;
  m_pDataBuf.reset(FX_Alloc(uint8_t, size));
  if (pData)
    FXSYS_memcpy(m_pDataBuf.get(), pData, size);
  m_dwSize = size;
  if (!m_pDict)
    m_pDict.reset(new CPDF_Dictionary);
  m_pDict->SetIntegerFor("Length", size);
  m_pDict->RemoveFor("Filter");
  m_pDict->RemoveFor("DecodeParms");
}

FX_BOOL CPDF_Stream::ReadRawData(FX_FILESIZE offset,
                                 uint8_t* buf,
                                 uint32_t size) const {
  if (m_bMemoryBased && m_pFile)
    return m_pFile->ReadBlock(buf, offset, size);

  if (m_pDataBuf)
    FXSYS_memcpy(buf, m_pDataBuf.get() + offset, size);

  return TRUE;
}

CFX_WideString CPDF_Stream::GetUnicodeText() const {
  CPDF_StreamAcc stream;
  stream.LoadAllData(this, FALSE);
  return PDF_DecodeText(stream.GetData(), stream.GetSize());
}

