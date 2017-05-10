// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_objectstream.h"

#include "core/fpdfapi/edit/cpdf_creator.h"
#include "core/fpdfapi/edit/cpdf_encryptor.h"
#include "core/fpdfapi/edit/cpdf_flateencoder.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"

namespace {

const int kObjectStreamMaxLength = 256 * 1024;

}  // namespace

CPDF_ObjectStream::CPDF_ObjectStream() : m_dwObjNum(0), m_index(0) {}

CPDF_ObjectStream::~CPDF_ObjectStream() {}

bool CPDF_ObjectStream::IsNotFull() const {
  return m_Buffer.GetLength() < kObjectStreamMaxLength;
}

void CPDF_ObjectStream::Start() {
  m_Items.clear();
  m_Buffer.Clear();
  m_dwObjNum = 0;
  m_index = 0;
}

void CPDF_ObjectStream::CompressIndirectObject(uint32_t dwObjNum,
                                               const CPDF_Object* pObj) {
  m_Items.push_back({dwObjNum, m_Buffer.GetLength()});
  m_Buffer << pObj;
}

void CPDF_ObjectStream::CompressIndirectObject(uint32_t dwObjNum,
                                               const uint8_t* pBuffer,
                                               uint32_t dwSize) {
  m_Items.push_back({dwObjNum, m_Buffer.GetLength()});
  m_Buffer.AppendBlock(pBuffer, dwSize);
}

FX_FILESIZE CPDF_ObjectStream::End(CPDF_Creator* pCreator) {
  ASSERT(pCreator);

  if (m_Items.empty())
    return 0;

  IFX_ArchiveStream* archive = pCreator->GetArchive();
  FX_FILESIZE ObjOffset = archive->CurrentOffset();
  if (!m_dwObjNum)
    m_dwObjNum = pCreator->GetNextObjectNumber();

  CFX_ByteTextBuf tempBuffer;
  for (const auto& pair : m_Items)
    tempBuffer << pair.objnum << " " << pair.offset << " ";

  if (!archive->WriteDWord(m_dwObjNum) ||
      !archive->WriteString(" 0 obj\r\n<</Type /ObjStm /N ") ||
      !archive->WriteDWord(pdfium::CollectionSize<uint32_t>(m_Items)) ||
      !archive->WriteString("/First ") ||
      !archive->WriteDWord(static_cast<uint32_t>(tempBuffer.GetLength())) ||
      !archive->WriteString("/Length ")) {
    return -1;
  }
  tempBuffer << m_Buffer;

  CPDF_FlateEncoder encoder(tempBuffer.GetBuffer(), tempBuffer.GetLength(),
                            true, false);
  CPDF_Encryptor encryptor(pCreator->GetCryptoHandler(), m_dwObjNum,
                           encoder.GetData(), encoder.GetSize());

  if (!archive->WriteDWord(encryptor.GetSize()) ||
      !archive->WriteString("/Filter /FlateDecode") ||
      !archive->WriteString(">>stream\r\n") ||
      !archive->WriteBlock(encryptor.GetData(), encryptor.GetSize()) ||
      !archive->WriteString("\r\nendstream\r\nendobj\r\n")) {
    return -1;
  }

  return ObjOffset;
}
