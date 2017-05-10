// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_xrefstream.h"

#include "core/fpdfapi/edit/cpdf_creator.h"
#include "core/fpdfapi/edit/cpdf_flateencoder.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"

namespace {

const int32_t kObjectStreamMaxSize = 200;

int32_t WriteTrailer(CPDF_Document* pDocument,
                     CFX_FileBufferArchive* pFile,
                     CPDF_Array* pIDArray) {
  FX_FILESIZE offset = 0;
  int32_t len = 0;
  CPDF_Parser* pParser = pDocument->GetParser();
  if (pParser) {
    CPDF_Dictionary* p = pParser->GetTrailer();
    for (const auto& it : *p) {
      const CFX_ByteString& key = it.first;
      CPDF_Object* pValue = it.second.get();
      if (key == "Encrypt" || key == "Size" || key == "Filter" ||
          key == "Index" || key == "Length" || key == "Prev" || key == "W" ||
          key == "XRefStm" || key == "Type" || key == "ID") {
        continue;
      }
      if (key == "DecodeParms")
        continue;
      if (pFile->AppendString(("/")) < 0)
        return -1;

      len = pFile->AppendString(PDF_NameEncode(key).AsStringC());
      if (len < 0)
        return -1;
      offset += len + 1;

      if (!pValue->IsInline()) {
        if (pFile->AppendString(" ") < 0)
          return -1;

        len = pFile->AppendDWord(pValue->GetObjNum());
        if (len < 0 || pFile->AppendString(" 0 R ") < 0)
          return -1;
        offset += len + 6;
      } else if (!pValue->WriteTo(pFile, &offset)) {
        return -1;
      }
    }
    if (pIDArray) {
      if (pFile->AppendString(("/ID")) < 0)
        return -1;
      offset += 3;

      if (!pIDArray->WriteTo(pFile, &offset))
        return -1;
    }
    return offset;
  }
  if (pFile->AppendString("\r\n/Root ") < 0)
    return -1;

  len = pFile->AppendDWord(pDocument->GetRoot()->GetObjNum());
  if (len < 0 || pFile->AppendString(" 0 R\r\n") < 0)
    return -1;
  offset += len + 14;

  if (pDocument->GetInfo()) {
    if (pFile->AppendString("/Info ") < 0)
      return -1;

    len = pFile->AppendDWord(pDocument->GetInfo()->GetObjNum());
    if (len < 0 || pFile->AppendString(" 0 R\r\n") < 0)
      return -1;
    offset += len + 12;
  }
  if (pIDArray) {
    if (pFile->AppendString(("/ID")) < 0)
      return -1;
    offset += 3;

    if (!pIDArray->WriteTo(pFile, &offset))
      return -1;
  }
  return offset;
}

int32_t WriteEncryptDictObjectReference(uint32_t dwObjNum,
                                        CFX_FileBufferArchive* pFile) {
  ASSERT(pFile);

  FX_FILESIZE offset = 0;
  int32_t len = 0;
  if (pFile->AppendString("/Encrypt") < 0)
    return -1;
  offset += 8;

  if (pFile->AppendString(" ") < 0)
    return -1;

  len = pFile->AppendDWord(dwObjNum);
  if (len < 0 || pFile->AppendString(" 0 R ") < 0)
    return -1;
  offset += len + 6;

  return offset;
}

void AppendIndex0(CFX_ByteTextBuf& buffer, bool bFirstObject) {
  buffer.AppendByte(0);
  buffer.AppendByte(0);
  buffer.AppendByte(0);
  buffer.AppendByte(0);
  buffer.AppendByte(0);

  const uint8_t byte = bFirstObject ? 0xFF : 0;
  buffer.AppendByte(byte);
  buffer.AppendByte(byte);
}

void AppendIndex1(CFX_ByteTextBuf& buffer, FX_FILESIZE offset) {
  buffer.AppendByte(1);
  buffer.AppendByte(static_cast<uint8_t>(offset >> 24));
  buffer.AppendByte(static_cast<uint8_t>(offset >> 16));
  buffer.AppendByte(static_cast<uint8_t>(offset >> 8));
  buffer.AppendByte(static_cast<uint8_t>(offset));
  buffer.AppendByte(0);
  buffer.AppendByte(0);
}

void AppendIndex2(CFX_ByteTextBuf& buffer, uint32_t objnum, int32_t index) {
  buffer.AppendByte(2);
  buffer.AppendByte(static_cast<uint8_t>(objnum >> 24));
  buffer.AppendByte(static_cast<uint8_t>(objnum >> 16));
  buffer.AppendByte(static_cast<uint8_t>(objnum >> 8));
  buffer.AppendByte(static_cast<uint8_t>(objnum));
  buffer.AppendByte(static_cast<uint8_t>(index >> 8));
  buffer.AppendByte(static_cast<uint8_t>(index));
}

}  // namespace

CPDF_XRefStream::CPDF_XRefStream()
    : m_PrevOffset(0), m_dwTempObjNum(0), m_iSeg(0) {}

CPDF_XRefStream::~CPDF_XRefStream() {}

bool CPDF_XRefStream::Start() {
  m_IndexArray.clear();
  m_Buffer.Clear();
  m_iSeg = 0;
  return true;
}

bool CPDF_XRefStream::CompressIndirectObject(uint32_t dwObjNum,
                                             const CPDF_Object* pObj,
                                             CPDF_Creator* pCreator) {
  ASSERT(pCreator);

  m_ObjStream.CompressIndirectObject(dwObjNum, pObj);
  if (m_ObjStream.ItemCount() < kObjectStreamMaxSize &&
      m_ObjStream.IsNotFull()) {
    return true;
  }
  return EndObjectStream(pCreator, true);
}

bool CPDF_XRefStream::CompressIndirectObject(uint32_t dwObjNum,
                                             const uint8_t* pBuffer,
                                             uint32_t dwSize,
                                             CPDF_Creator* pCreator) {
  ASSERT(pCreator);

  m_ObjStream.CompressIndirectObject(dwObjNum, pBuffer, dwSize);
  if (m_ObjStream.ItemCount() < kObjectStreamMaxSize &&
      m_ObjStream.IsNotFull()) {
    return true;
  }
  return EndObjectStream(pCreator, true);
}

bool CPDF_XRefStream::EndObjectStream(CPDF_Creator* pCreator, bool bEOF) {
  FX_FILESIZE objOffset = 0;
  if (bEOF) {
    objOffset = m_ObjStream.End(pCreator);
    if (objOffset < 0)
      return false;
  }

  if (!m_ObjStream.GetObjectNumber())
    m_ObjStream.SetObjectNumber(pCreator->GetNextObjectNumber());

  int32_t iSize = m_ObjStream.ItemCount();
  size_t iSeg = m_IndexArray.size();
  if (!pCreator->IsIncremental()) {
    if (m_dwTempObjNum == 0) {
      AppendIndex0(m_Buffer, true);
      m_dwTempObjNum++;
    }
    uint32_t end_num = m_IndexArray.back().objnum + m_IndexArray.back().count;
    int index = 0;
    for (; m_dwTempObjNum < end_num; m_dwTempObjNum++) {
      if (pCreator->HasObjectNumber(m_dwTempObjNum)) {
        if (index >= iSize ||
            m_dwTempObjNum != m_ObjStream.GetObjectNumberForItem(index)) {
          AppendIndex1(m_Buffer, pCreator->GetObjectOffset(m_dwTempObjNum));
        } else {
          AppendIndex2(m_Buffer, m_ObjStream.GetObjectNumber(), index++);
        }
      } else {
        AppendIndex0(m_Buffer, false);
      }
    }
    if (iSize > 0 && bEOF)
      pCreator->SetObjectOffset(m_ObjStream.GetObjectNumber(), objOffset);

    m_iSeg = iSeg;
    if (bEOF)
      m_ObjStream.Start();

    return true;
  }

  for (auto it = m_IndexArray.begin() + m_iSeg; it != m_IndexArray.end();
       ++it) {
    for (uint32_t m = it->objnum; m < it->objnum + it->count; ++m) {
      if (m_ObjStream.GetIndex() >= iSize ||
          m != m_ObjStream.GetObjectNumberForItem(it - m_IndexArray.begin())) {
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(m));
      } else {
        AppendIndex2(m_Buffer, m_ObjStream.GetObjectNumber(),
                     m_ObjStream.GetIndex());
        m_ObjStream.IncrementIndex();
      }
    }
  }
  if (iSize > 0 && bEOF) {
    AppendIndex1(m_Buffer, objOffset);
    m_IndexArray.push_back({m_ObjStream.GetObjectNumber(), 1});
    iSeg += 1;
  }
  m_iSeg = iSeg;
  if (bEOF)
    m_ObjStream.Start();

  return true;
}

bool CPDF_XRefStream::GenerateXRefStream(CPDF_Creator* pCreator, bool bEOF) {
  FX_FILESIZE offset_tmp = pCreator->GetOffset();
  uint32_t objnum = pCreator->GetNextObjectNumber();
  CFX_FileBufferArchive* pFile = pCreator->GetFile();
  if (pCreator->IsIncremental()) {
    AddObjectNumberToIndexArray(objnum);
  } else {
    for (; m_dwTempObjNum < pCreator->GetLastObjectNumber(); m_dwTempObjNum++) {
      if (pCreator->HasObjectNumber(m_dwTempObjNum))
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(m_dwTempObjNum));
      else
        AppendIndex0(m_Buffer, false);
    }
  }

  AppendIndex1(m_Buffer, offset_tmp);

  int32_t len = pFile->AppendDWord(objnum);
  if (len < 0)
    return false;
  pCreator->IncrementOffset(len);

  len = pFile->AppendString(" 0 obj\r\n<</Type /XRef/W[1 4 2]/Index[");
  if (len < 0)
    return false;
  pCreator->IncrementOffset(len);

  if (!pCreator->IsIncremental()) {
    if (pFile->AppendDWord(0) < 0)
      return false;

    len = pFile->AppendString(" ");
    if (len < 0)
      return false;
    pCreator->IncrementOffset(len + 1);

    len = pFile->AppendDWord(objnum + 1);
    if (len < 0)
      return false;
    pCreator->IncrementOffset(len);
  } else {
    for (const auto& pair : m_IndexArray) {
      len = pFile->AppendDWord(pair.objnum);
      if (len < 0 || pFile->AppendString(" ") < 0)
        return false;
      pCreator->IncrementOffset(len + 1);

      len = pFile->AppendDWord(pair.count);
      if (len < 0 || pFile->AppendString(" ") < 0)
        return false;
      pCreator->IncrementOffset(len + 1);
    }
  }
  if (pFile->AppendString("]/Size ") < 0)
    return false;

  len = pFile->AppendDWord(objnum + 1);
  if (len < 0)
    return false;
  pCreator->IncrementOffset(len + 7);

  if (m_PrevOffset > 0) {
    if (pFile->AppendString("/Prev ") < 0)
      return false;

    char offset_buf[20];
    memset(offset_buf, 0, sizeof(offset_buf));
    FXSYS_i64toa(m_PrevOffset, offset_buf, 10);
    int32_t offset_len = (int32_t)FXSYS_strlen(offset_buf);
    if (pFile->AppendBlock(offset_buf, offset_len) < 0)
      return false;

    pCreator->IncrementOffset(offset_len + 6);
  }

  CPDF_FlateEncoder encoder(m_Buffer.GetBuffer(), m_Buffer.GetLength(), true,
                            true);
  if (pFile->AppendString("/Filter /FlateDecode") < 0)
    return false;
  pCreator->IncrementOffset(20);

  len = pFile->AppendString("/DecodeParms<</Columns 7/Predictor 12>>");
  if (len < 0)
    return false;
  pCreator->IncrementOffset(len);

  if (pFile->AppendString("/Length ") < 0)
    return false;

  len = pFile->AppendDWord(encoder.GetSize());
  if (len < 0)
    return false;
  pCreator->IncrementOffset(len + 8);

  if (bEOF) {
    len = WriteTrailer(pCreator->GetDocument(), pFile, pCreator->GetIDArray());
    if (len < 0)
      return false;
    pCreator->IncrementOffset(len);

    if (CPDF_Dictionary* encryptDict = pCreator->GetEncryptDict()) {
      uint32_t dwEncryptObjNum = encryptDict->GetObjNum();
      if (dwEncryptObjNum == 0)
        dwEncryptObjNum = pCreator->GetEncryptObjectNumber();

      len = WriteEncryptDictObjectReference(dwEncryptObjNum, pFile);
      if (len < 0)
        return false;
      pCreator->IncrementOffset(len);
    }
  }

  len = pFile->AppendString(">>stream\r\n");
  if (len < 0)
    return false;
  pCreator->IncrementOffset(len);

  if (pFile->AppendBlock(encoder.GetData(), encoder.GetSize()) < 0)
    return false;

  len = pFile->AppendString("\r\nendstream\r\nendobj\r\n");
  if (len < 0)
    return false;
  pCreator->IncrementOffset(encoder.GetSize() + len);

  m_PrevOffset = offset_tmp;
  return true;
}

bool CPDF_XRefStream::End(CPDF_Creator* pCreator, bool bEOF) {
  if (!EndObjectStream(pCreator, bEOF))
    return false;
  return GenerateXRefStream(pCreator, bEOF);
}

bool CPDF_XRefStream::EndXRefStream(CPDF_Creator* pCreator) {
  if (!pCreator->IsIncremental()) {
    AppendIndex0(m_Buffer, true);
    for (uint32_t i = 1; i < pCreator->GetLastObjectNumber() + 1; i++) {
      if (pCreator->HasObjectNumber(i))
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(i));
      else
        AppendIndex0(m_Buffer, false);
    }
  } else {
    for (const auto& pair : m_IndexArray) {
      for (uint32_t j = pair.objnum; j < pair.objnum + pair.count; ++j)
        AppendIndex1(m_Buffer, pCreator->GetObjectOffset(j));
    }
  }
  return GenerateXRefStream(pCreator, false);
}

void CPDF_XRefStream::AddObjectNumberToIndexArray(uint32_t objnum) {
  if (m_IndexArray.empty()) {
    m_IndexArray.push_back({objnum, 1});
    return;
  }

  uint32_t next_objnum = m_IndexArray.back().objnum + m_IndexArray.back().count;
  if (objnum == next_objnum)
    m_IndexArray.back().count += 1;
  else
    m_IndexArray.push_back({objnum, 1});
}
