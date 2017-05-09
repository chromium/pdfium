// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_creator.h"

#include "core/fpdfapi/edit/cpdf_encryptor.h"
#include "core/fpdfapi/edit/cpdf_flateencoder.h"
#include "core/fpdfapi/edit/cpdf_objectstream.h"
#include "core/fpdfapi/edit/cpdf_xrefstream.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_crypto_handler.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_security_handler.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/fx_extension.h"

namespace {

const uint32_t kXRefStreamMaxSize = 10000;

std::vector<uint8_t> GenerateFileID(uint32_t dwSeed1, uint32_t dwSeed2) {
  std::vector<uint8_t> buffer(sizeof(uint32_t) * 4);
  uint32_t* pBuffer = reinterpret_cast<uint32_t*>(buffer.data());
  void* pContext = FX_Random_MT_Start(dwSeed1);
  for (int i = 0; i < 2; ++i)
    *pBuffer++ = FX_Random_MT_Generate(pContext);

  FX_Random_MT_Close(pContext);
  pContext = FX_Random_MT_Start(dwSeed2);
  for (int i = 0; i < 2; ++i)
    *pBuffer++ = FX_Random_MT_Generate(pContext);

  FX_Random_MT_Close(pContext);
  return buffer;
}

int32_t OutputIndex(CFX_FileBufferArchive* pFile, FX_FILESIZE offset) {
  if (pFile->AppendByte(static_cast<uint8_t>(offset >> 24)) < 0)
    return -1;
  if (pFile->AppendByte(static_cast<uint8_t>(offset >> 16)) < 0)
    return -1;
  if (pFile->AppendByte(static_cast<uint8_t>(offset >> 8)) < 0)
    return -1;
  if (pFile->AppendByte(static_cast<uint8_t>(offset)) < 0)
    return -1;
  if (pFile->AppendByte(0) < 0)
    return -1;
  return 0;
}

}  // namespace

CPDF_Creator::CPDF_Creator(CPDF_Document* pDoc,
                           const CFX_RetainPtr<IFX_WriteStream>& pFile)
    : m_pDocument(pDoc),
      m_pParser(pDoc->GetParser()),
      m_bSecurityChanged(false),
      m_pEncryptDict(m_pParser ? m_pParser->GetEncryptDict() : nullptr),
      m_dwEncryptObjNum(0),
      m_pCryptoHandler(m_pParser ? m_pParser->GetCryptoHandler() : nullptr),
      m_pMetadata(nullptr),
      m_ObjectStreamSize(200),
      m_dwLastObjNum(m_pDocument->GetLastObjNum()),
      m_Offset(0),
      m_SavedOffset(0),
      m_iStage(-1),
      m_dwFlags(0),
      m_CurObjNum(0),
      m_XrefStart(0),
      m_pIDArray(nullptr),
      m_FileVersion(0) {
  m_File.AttachFile(pFile);
}

CPDF_Creator::~CPDF_Creator() {
  Clear();
}

bool CPDF_Creator::IsXRefNeedEnd() {
  if (!IsIncremental())
    return false;
  return m_pXRefStream->CountIndexArrayItems() >= kXRefStreamMaxSize;
}

int32_t CPDF_Creator::WriteIndirectObjectToStream(const CPDF_Object* pObj) {
  if (!m_pXRefStream)
    return 1;

  uint32_t objnum = pObj->GetObjNum();
  if (m_pParser && m_pParser->GetObjectGenNum(objnum) > 0)
    return 1;
  if (pObj->IsNumber())
    return 1;

  CPDF_Dictionary* pDict = pObj->GetDict();
  if (pObj->IsStream()) {
    if (pDict && pDict->GetStringFor("Type") == "XRef")
      return 0;
    return 1;
  }

  if (pDict) {
    if (pDict == m_pDocument->GetRoot() || pDict == m_pEncryptDict)
      return 1;
    if (pDict->IsSignatureDict())
      return 1;
    if (pDict->GetStringFor("Type") == "Page")
      return 1;
  }

  m_pXRefStream->AddObjectNumberToIndexArray(objnum);
  if (m_pXRefStream->CompressIndirectObject(objnum, pObj, this) < 0)
    return -1;
  if (!IsXRefNeedEnd())
    return 0;
  if (!m_pXRefStream->End(this, false))
    return -1;
  if (!m_pXRefStream->Start())
    return -1;
  return 0;
}
int32_t CPDF_Creator::WriteIndirectObjectToStream(uint32_t objnum,
                                                  const uint8_t* pBuffer,
                                                  uint32_t dwSize) {
  if (!m_pXRefStream)
    return 1;

  m_pXRefStream->AddObjectNumberToIndexArray(objnum);
  int32_t iRet =
      m_pXRefStream->CompressIndirectObject(objnum, pBuffer, dwSize, this);
  if (iRet < 1)
    return iRet;
  if (!IsXRefNeedEnd())
    return 0;
  if (!m_pXRefStream->End(this, false))
    return -1;
  if (!m_pXRefStream->Start())
    return -1;
  return 0;
}

int32_t CPDF_Creator::AppendObjectNumberToXRef(uint32_t objnum) {
  if (!m_pXRefStream)
    return 1;

  m_pXRefStream->AddObjectNumberToIndexArray(objnum);
  if (!IsXRefNeedEnd())
    return 0;
  if (!m_pXRefStream->End(this, false))
    return -1;
  if (!m_pXRefStream->Start())
    return -1;
  return 0;
}

int32_t CPDF_Creator::WriteStream(const CPDF_Object* pStream,
                                  uint32_t objnum,
                                  CPDF_CryptoHandler* pCrypto) {
  CPDF_FlateEncoder encoder(const_cast<CPDF_Stream*>(pStream->AsStream()),
                            pStream != m_pMetadata);
  CPDF_Encryptor encryptor(pCrypto, objnum, encoder.GetData(),
                           encoder.GetSize());
  if (static_cast<uint32_t>(encoder.GetDict()->GetIntegerFor("Length")) !=
      encryptor.GetSize()) {
    encoder.CloneDict();
    encoder.GetDict()->SetNewFor<CPDF_Number>(
        "Length", static_cast<int>(encryptor.GetSize()));
  }
  if (WriteDirectObj(objnum, encoder.GetDict()) < 0)
    return -1;

  int len = m_File.AppendString("stream\r\n");
  if (len < 0)
    return -1;

  m_Offset += len;
  if (m_File.AppendBlock(encryptor.GetData(), encryptor.GetSize()) < 0)
    return -1;

  m_Offset += encryptor.GetSize();
  if ((len = m_File.AppendString("\r\nendstream")) < 0)
    return -1;

  m_Offset += len;
  return 1;
}

int32_t CPDF_Creator::WriteIndirectObj(uint32_t objnum,
                                       const CPDF_Object* pObj) {
  int32_t len = m_File.AppendDWord(objnum);
  if (len < 0)
    return -1;

  m_Offset += len;
  if ((len = m_File.AppendString(" 0 obj\r\n")) < 0)
    return -1;

  m_Offset += len;
  if (pObj->IsStream()) {
    CPDF_CryptoHandler* pHandler =
        pObj != m_pMetadata ? m_pCryptoHandler.Get() : nullptr;
    if (WriteStream(pObj, objnum, pHandler) < 0)
      return -1;
  } else {
    if (WriteDirectObj(objnum, pObj) < 0)
      return -1;
  }
  if ((len = m_File.AppendString("\r\nendobj\r\n")) < 0)
    return -1;

  m_Offset += len;
  if (AppendObjectNumberToXRef(objnum) < 0)
    return -1;
  return 0;
}

int32_t CPDF_Creator::WriteIndirectObj(const CPDF_Object* pObj) {
  int32_t iRet = WriteIndirectObjectToStream(pObj);
  if (iRet < 1)
    return iRet;
  return WriteIndirectObj(pObj->GetObjNum(), pObj);
}

int32_t CPDF_Creator::WriteDirectObj(uint32_t objnum,
                                     const CPDF_Object* pObj,
                                     bool bEncrypt) {
  switch (pObj->GetType()) {
    case CPDF_Object::BOOLEAN:
    case CPDF_Object::NAME:
    case CPDF_Object::NULLOBJ:
    case CPDF_Object::NUMBER:
    case CPDF_Object::REFERENCE:
      if (!pObj->WriteTo(&m_File, &m_Offset))
        return -1;
      break;

    case CPDF_Object::STRING: {
      CFX_ByteString str = pObj->GetString();
      bool bHex = pObj->AsString()->IsHex();
      if (!m_pCryptoHandler || !bEncrypt) {
        if (!pObj->WriteTo(&m_File, &m_Offset))
          return -1;
        break;
      }
      CPDF_Encryptor encryptor(m_pCryptoHandler.Get(), objnum,
                               (uint8_t*)str.c_str(), str.GetLength());
      CFX_ByteString content = PDF_EncodeString(
          CFX_ByteString(encryptor.GetData(), encryptor.GetSize()), bHex);
      int32_t len = m_File.AppendString(content.AsStringC());
      if (len < 0)
        return -1;

      m_Offset += len;
      break;
    }
    case CPDF_Object::STREAM: {
      CPDF_FlateEncoder encoder(const_cast<CPDF_Stream*>(pObj->AsStream()),
                                true);
      CPDF_Encryptor encryptor(m_pCryptoHandler.Get(), objnum,
                               encoder.GetData(), encoder.GetSize());
      if (static_cast<uint32_t>(encoder.GetDict()->GetIntegerFor("Length")) !=
          encryptor.GetSize()) {
        encoder.CloneDict();
        encoder.GetDict()->SetNewFor<CPDF_Number>(
            "Length", static_cast<int>(encryptor.GetSize()));
      }
      if (WriteDirectObj(objnum, encoder.GetDict()) < 0)
        return -1;

      int32_t len = m_File.AppendString("stream\r\n");
      if (len < 0)
        return -1;

      m_Offset += len;
      if (m_File.AppendBlock(encryptor.GetData(), encryptor.GetSize()) < 0)
        return -1;

      m_Offset += encryptor.GetSize();
      len = m_File.AppendString("\r\nendstream");
      if (len < 0)
        return -1;

      m_Offset += len;
      break;
    }
    case CPDF_Object::ARRAY: {
      if (m_File.AppendString("[") < 0)
        return -1;

      m_Offset += 1;
      const CPDF_Array* p = pObj->AsArray();
      for (size_t i = 0; i < p->GetCount(); i++) {
        CPDF_Object* pElement = p->GetObjectAt(i);
        if (!pElement->IsInline()) {
          if (m_File.AppendString(" ") < 0)
            return -1;

          int32_t len = m_File.AppendDWord(pElement->GetObjNum());
          if (len < 0)
            return -1;
          if (m_File.AppendString(" 0 R") < 0)
            return -1;

          m_Offset += len + 5;
        } else {
          if (WriteDirectObj(objnum, pElement) < 0)
            return -1;
        }
      }
      if (m_File.AppendString("]") < 0)
        return -1;

      m_Offset += 1;
      break;
    }
    case CPDF_Object::DICTIONARY: {
      if (!m_pCryptoHandler || pObj == m_pEncryptDict) {
        if (!pObj->WriteTo(&m_File, &m_Offset))
          return -1;
        break;
      }

      if (m_File.AppendString("<<") < 0)
        return -1;

      m_Offset += 2;
      const CPDF_Dictionary* p = pObj->AsDictionary();
      bool bSignDict = p->IsSignatureDict();
      for (const auto& it : *p) {
        bool bSignValue = false;
        const CFX_ByteString& key = it.first;
        CPDF_Object* pValue = it.second.get();
        if (m_File.AppendString("/") < 0)
          return -1;

        int32_t len = m_File.AppendString(PDF_NameEncode(key).AsStringC());
        if (len < 0)
          return -1;

        m_Offset += len + 1;
        if (bSignDict && key == "Contents")
          bSignValue = true;

        if (!pValue->IsInline()) {
          if (m_File.AppendString(" ") < 0)
            return -1;

          len = m_File.AppendDWord(pValue->GetObjNum());
          if (len < 0)
            return -1;
          if (m_File.AppendString(" 0 R ") < 0)
            return -1;

          m_Offset += len + 6;
        } else {
          if (WriteDirectObj(objnum, pValue, !bSignValue) < 0)
            return -1;
        }
      }
      if (m_File.AppendString(">>") < 0)
        return -1;

      m_Offset += 2;
      break;
    }
  }
  return 1;
}

int32_t CPDF_Creator::WriteOldIndirectObject(uint32_t objnum) {
  if (m_pParser->IsObjectFreeOrNull(objnum))
    return 0;

  m_ObjectOffsets[objnum] = m_Offset;
  bool bExistInMap = !!m_pDocument->GetIndirectObject(objnum);
  const uint8_t object_type = m_pParser->GetObjectType(objnum);
  bool bObjStm = (object_type == 2) && m_pEncryptDict && !m_pXRefStream;
  if (m_pParser->IsVersionUpdated() || m_bSecurityChanged || bExistInMap ||
      bObjStm) {
    CPDF_Object* pObj = m_pDocument->GetOrParseIndirectObject(objnum);
    if (!pObj) {
      m_ObjectOffsets.erase(objnum);
      return 0;
    }
    if (WriteIndirectObj(pObj))
      return -1;
    if (!bExistInMap)
      m_pDocument->DeleteIndirectObject(objnum);
  } else {
    uint8_t* pBuffer;
    uint32_t size;
    m_pParser->GetIndirectBinary(objnum, pBuffer, size);
    if (!pBuffer)
      return 0;
    if (object_type == 2) {
      if (m_pXRefStream) {
        if (WriteIndirectObjectToStream(objnum, pBuffer, size) < 0) {
          FX_Free(pBuffer);
          return -1;
        }
      } else {
        int32_t len = m_File.AppendDWord(objnum);
        if (len < 0)
          return -1;
        if (m_File.AppendString(" 0 obj ") < 0)
          return -1;

        m_Offset += len + 7;
        if (m_File.AppendBlock(pBuffer, size) < 0)
          return -1;

        m_Offset += size;
        if (m_File.AppendString("\r\nendobj\r\n") < 0)
          return -1;

        m_Offset += 10;
      }
    } else {
      if (m_File.AppendBlock(pBuffer, size) < 0)
        return -1;

      m_Offset += size;
      if (AppendObjectNumberToXRef(objnum) < 0)
        return -1;
    }
    FX_Free(pBuffer);
  }
  return 1;
}

int32_t CPDF_Creator::WriteOldObjs() {
  uint32_t nLastObjNum = m_pParser->GetLastObjNum();
  if (!m_pParser->IsValidObjectNumber(nLastObjNum))
    return 0;

  for (uint32_t objnum = m_CurObjNum; objnum <= nLastObjNum; ++objnum) {
    int32_t iRet = WriteOldIndirectObject(objnum);
    if (iRet < 0)
      return iRet;
    if (!iRet)
      continue;
  }
  return 0;
}

int32_t CPDF_Creator::WriteNewObjs(bool bIncremental) {
  uint32_t iCount = pdfium::CollectionSize<uint32_t>(m_NewObjNumArray);
  uint32_t index = m_CurObjNum;
  while (index < iCount) {
    uint32_t objnum = m_NewObjNumArray[index];
    CPDF_Object* pObj = m_pDocument->GetIndirectObject(objnum);
    if (!pObj) {
      ++index;
      continue;
    }
    m_ObjectOffsets[objnum] = m_Offset;
    if (WriteIndirectObj(pObj))
      return -1;

    index++;
  }
  return 0;
}

void CPDF_Creator::InitOldObjNumOffsets() {
  if (!m_pParser)
    return;

  uint32_t j = 0;
  uint32_t dwStart = 0;
  uint32_t dwEnd = m_pParser->GetLastObjNum();
  while (dwStart <= dwEnd) {
    while (dwStart <= dwEnd && m_pParser->IsObjectFreeOrNull(dwStart))
      dwStart++;

    if (dwStart > dwEnd)
      break;

    j = dwStart;
    while (j <= dwEnd && !m_pParser->IsObjectFreeOrNull(j))
      j++;

    dwStart = j;
  }
}

void CPDF_Creator::InitNewObjNumOffsets() {
  for (const auto& pair : *m_pDocument) {
    const uint32_t objnum = pair.first;
    const CPDF_Object* pObj = pair.second.get();
    if (IsIncremental() || pObj->GetObjNum() == CPDF_Object::kInvalidObjNum)
      continue;
    if (m_pParser && m_pParser->IsValidObjectNumber(objnum) &&
        m_pParser->GetObjectType(objnum)) {
      continue;
    }
    m_NewObjNumArray.insert(std::lower_bound(m_NewObjNumArray.begin(),
                                             m_NewObjNumArray.end(), objnum),
                            objnum);
  }
}

int32_t CPDF_Creator::WriteDoc_Stage1() {
  ASSERT(m_iStage > -1 || m_iStage < 20);
  if (m_iStage == 0) {
    if (!m_pParser)
      m_dwFlags &= ~FPDFCREATE_INCREMENTAL;
    if (m_bSecurityChanged && (m_dwFlags & FPDFCREATE_NO_ORIGINAL) == 0)
      m_dwFlags &= ~FPDFCREATE_INCREMENTAL;

    CPDF_Dictionary* pDict = m_pDocument->GetRoot();
    m_pMetadata = pDict ? pDict->GetDirectObjectFor("Metadata") : nullptr;
    if (m_dwFlags & FPDFCREATE_OBJECTSTREAM) {
      m_pXRefStream = pdfium::MakeUnique<CPDF_XRefStream>();
      m_pXRefStream->Start();
      if (IsIncremental() && m_pParser)
        m_pXRefStream->SetPreviousOffset(m_pParser->GetLastXRefOffset());
    }
    m_iStage = 10;
  }
  if (m_iStage == 10) {
    if (!IsIncremental()) {
      if (m_File.AppendString("%PDF-1.") < 0)
        return -1;

      m_Offset += 7;
      int32_t version = 7;
      if (m_FileVersion)
        version = m_FileVersion;
      else if (m_pParser)
        version = m_pParser->GetFileVersion();

      int32_t len = m_File.AppendDWord(version % 10);
      if (len < 0)
        return -1;

      m_Offset += len;
      if ((len = m_File.AppendString("\r\n%\xA1\xB3\xC5\xD7\r\n")) < 0)
        return -1;

      m_Offset += len;
      InitOldObjNumOffsets();
      m_iStage = 20;
    } else {
      CFX_RetainPtr<IFX_SeekableReadStream> pSrcFile =
          m_pParser->GetFileAccess();
      m_Offset = pSrcFile->GetSize();
      m_SavedOffset = m_Offset;
      m_iStage = 15;
    }
  }
  if (m_iStage == 15) {
    if ((m_dwFlags & FPDFCREATE_NO_ORIGINAL) == 0 && m_SavedOffset > 0) {
      CFX_RetainPtr<IFX_SeekableReadStream> pSrcFile =
          m_pParser->GetFileAccess();
      std::vector<uint8_t> buffer(4096);
      FX_FILESIZE src_size = m_SavedOffset;
      while (src_size) {
        uint32_t block_size = src_size > 4096 ? 4096 : src_size;
        if (!pSrcFile->ReadBlock(buffer.data(), m_Offset - src_size,
                                 block_size)) {
          return -1;
        }
        if (m_File.AppendBlock(buffer.data(), block_size) < 0)
          return -1;

        src_size -= block_size;
      }
    }
    if ((m_dwFlags & FPDFCREATE_NO_ORIGINAL) == 0 &&
        m_pParser->GetLastXRefOffset() == 0) {
      InitOldObjNumOffsets();
      uint32_t dwEnd = m_pParser->GetLastObjNum();
      bool bObjStm = (m_dwFlags & FPDFCREATE_OBJECTSTREAM) != 0;
      for (uint32_t objnum = 0; objnum <= dwEnd; objnum++) {
        if (m_pParser->IsObjectFreeOrNull(objnum))
          continue;

        m_ObjectOffsets[objnum] = m_pParser->GetObjectPositionOrZero(objnum);
        if (bObjStm)
          m_pXRefStream->AddObjectNumberToIndexArray(objnum);
      }
      if (bObjStm) {
        m_pXRefStream->EndXRefStream(this);
        m_pXRefStream->Start();
      }
    }
    m_iStage = 20;
  }
  InitNewObjNumOffsets();
  return m_iStage;
}

int32_t CPDF_Creator::WriteDoc_Stage2() {
  ASSERT(m_iStage >= 20 || m_iStage < 30);
  if (m_iStage == 20) {
    if (!IsIncremental() && m_pParser) {
      m_CurObjNum = 0;
      m_iStage = 21;
    } else {
      m_iStage = 25;
    }
  }
  if (m_iStage == 21) {
    int32_t iRet = WriteOldObjs();
    if (iRet)
      return iRet;

    m_iStage = 25;
  }
  if (m_iStage == 25) {
    m_CurObjNum = 0;
    m_iStage = 26;
  }
  if (m_iStage == 26) {
    int32_t iRet = WriteNewObjs(IsIncremental());
    if (iRet)
      return iRet;

    m_iStage = 27;
  }
  if (m_iStage == 27) {
    if (m_pEncryptDict && m_pEncryptDict->IsInline()) {
      m_dwLastObjNum += 1;
      FX_FILESIZE saveOffset = m_Offset;
      if (WriteIndirectObj(m_dwLastObjNum, m_pEncryptDict) < 0)
        return -1;

      m_ObjectOffsets[m_dwLastObjNum] = saveOffset;
      m_dwEncryptObjNum = m_dwLastObjNum;
      if (IsIncremental())
        m_NewObjNumArray.push_back(m_dwLastObjNum);
    }
    m_iStage = 80;
  }
  return m_iStage;
}

int32_t CPDF_Creator::WriteDoc_Stage3() {
  ASSERT(m_iStage >= 80 || m_iStage < 90);
  uint32_t dwLastObjNum = m_dwLastObjNum;
  if (m_iStage == 80) {
    m_XrefStart = m_Offset;
    if (m_dwFlags & FPDFCREATE_OBJECTSTREAM) {
      m_pXRefStream->End(this, true);
      m_XrefStart = m_pXRefStream->GetPreviousOffset();
      m_iStage = 90;
    } else if (!IsIncremental() || !m_pParser->IsXRefStream()) {
      if (!IsIncremental() || m_pParser->GetLastXRefOffset() == 0) {
        CFX_ByteString str;
        str = pdfium::ContainsKey(m_ObjectOffsets, 1)
                  ? "xref\r\n"
                  : "xref\r\n0 1\r\n0000000000 65535 f\r\n";
        if (m_File.AppendString(str.AsStringC()) < 0)
          return -1;

        m_CurObjNum = 1;
        m_iStage = 81;
      } else {
        if (m_File.AppendString("xref\r\n") < 0)
          return -1;

        m_CurObjNum = 0;
        m_iStage = 82;
      }
    } else {
      m_iStage = 90;
    }
  }
  if (m_iStage == 81) {
    CFX_ByteString str;
    uint32_t i = m_CurObjNum;
    uint32_t j;
    while (i <= dwLastObjNum) {
      while (i <= dwLastObjNum && !pdfium::ContainsKey(m_ObjectOffsets, i))
        i++;

      if (i > dwLastObjNum)
        break;

      j = i;
      while (j <= dwLastObjNum && pdfium::ContainsKey(m_ObjectOffsets, j))
        j++;

      if (i == 1)
        str.Format("0 %d\r\n0000000000 65535 f\r\n", j);
      else
        str.Format("%d %d\r\n", i, j - i);

      if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0)
        return -1;

      while (i < j) {
        str.Format("%010d 00000 n\r\n", m_ObjectOffsets[i++]);
        if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0)
          return -1;
      }
      if (i > dwLastObjNum)
        break;
    }
    m_iStage = 90;
  }
  if (m_iStage == 82) {
    CFX_ByteString str;
    uint32_t iCount = pdfium::CollectionSize<uint32_t>(m_NewObjNumArray);
    uint32_t i = m_CurObjNum;
    while (i < iCount) {
      size_t j = i;
      uint32_t objnum = m_NewObjNumArray[i];
      while (j < iCount) {
        if (++j == iCount)
          break;
        uint32_t dwCurrent = m_NewObjNumArray[j];
        if (dwCurrent - objnum > 1)
          break;
        objnum = dwCurrent;
      }
      objnum = m_NewObjNumArray[i];
      if (objnum == 1)
        str.Format("0 %d\r\n0000000000 65535 f\r\n", j - i + 1);
      else
        str.Format("%d %d\r\n", objnum, j - i);

      if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0)
        return -1;

      while (i < j) {
        objnum = m_NewObjNumArray[i++];
        str.Format("%010d 00000 n\r\n", m_ObjectOffsets[objnum]);
        if (m_File.AppendBlock(str.c_str(), str.GetLength()) < 0)
          return -1;
      }
    }
    m_iStage = 90;
  }
  return m_iStage;
}

int32_t CPDF_Creator::WriteDoc_Stage4() {
  ASSERT(m_iStage >= 90);
  if ((m_dwFlags & FPDFCREATE_OBJECTSTREAM) == 0) {
    bool bXRefStream = IsIncremental() && m_pParser->IsXRefStream();
    if (!bXRefStream) {
      if (m_File.AppendString("trailer\r\n<<") < 0)
        return -1;
    } else {
      if (m_File.AppendDWord(m_pDocument->GetLastObjNum() + 1) < 0)
        return -1;
      if (m_File.AppendString(" 0 obj <<") < 0)
        return -1;
    }

    if (m_pParser) {
      CPDF_Dictionary* p = m_pParser->GetTrailer();
      for (const auto& it : *p) {
        const CFX_ByteString& key = it.first;
        CPDF_Object* pValue = it.second.get();
        // TODO(ochang): Consolidate with similar check in
        // CPDF_XRefStream::WriteTrailer.
        if (key == "Encrypt" || key == "Size" || key == "Filter" ||
            key == "Index" || key == "Length" || key == "Prev" || key == "W" ||
            key == "XRefStm" || key == "ID") {
          continue;
        }
        if (m_File.AppendString(("/")) < 0)
          return -1;
        if (m_File.AppendString(PDF_NameEncode(key).AsStringC()) < 0)
          return -1;
        if (!pValue->IsInline()) {
          if (m_File.AppendString(" ") < 0)
            return -1;
          if (m_File.AppendDWord(pValue->GetObjNum()) < 0)
            return -1;
          if (m_File.AppendString(" 0 R ") < 0)
            return -1;
        } else {
          FX_FILESIZE offset = 0;
          if (!pValue->WriteTo(&m_File, &offset))
            return -1;
        }
      }
    } else {
      if (m_File.AppendString("\r\n/Root ") < 0)
        return -1;
      if (m_File.AppendDWord(m_pDocument->GetRoot()->GetObjNum()) < 0)
        return -1;
      if (m_File.AppendString(" 0 R\r\n") < 0)
        return -1;
      if (m_pDocument->GetInfo()) {
        if (m_File.AppendString("/Info ") < 0)
          return -1;
        if (m_File.AppendDWord(m_pDocument->GetInfo()->GetObjNum()) < 0)
          return -1;
        if (m_File.AppendString(" 0 R\r\n") < 0)
          return -1;
      }
    }
    if (m_pEncryptDict) {
      if (m_File.AppendString("/Encrypt") < 0)
        return -1;

      uint32_t dwObjNum = m_pEncryptDict->GetObjNum();
      if (dwObjNum == 0)
        dwObjNum = m_pDocument->GetLastObjNum() + 1;
      if (m_File.AppendString(" ") < 0)
        return -1;
      if (m_File.AppendDWord(dwObjNum) < 0)
        return -1;
      if (m_File.AppendString(" 0 R ") < 0)
        return -1;
    }

    if (m_File.AppendString("/Size ") < 0)
      return -1;
    if (m_File.AppendDWord(m_dwLastObjNum + (bXRefStream ? 2 : 1)) < 0)
      return -1;
    if (IsIncremental()) {
      FX_FILESIZE prev = m_pParser->GetLastXRefOffset();
      if (prev) {
        if (m_File.AppendString("/Prev ") < 0)
          return -1;

        char offset_buf[20];
        memset(offset_buf, 0, sizeof(offset_buf));
        FXSYS_i64toa(prev, offset_buf, 10);
        if (m_File.AppendBlock(offset_buf, FXSYS_strlen(offset_buf)) < 0)
          return -1;
      }
    }
    if (m_pIDArray) {
      if (m_File.AppendString(("/ID")) < 0)
        return -1;

      FX_FILESIZE offset = 0;
      if (!m_pIDArray->WriteTo(&m_File, &offset))
        return -1;
    }
    if (!bXRefStream) {
      if (m_File.AppendString(">>") < 0)
        return -1;
    } else {
      if (m_File.AppendString("/W[0 4 1]/Index[") < 0)
        return -1;
      if (IsIncremental() && m_pParser && m_pParser->GetLastXRefOffset() == 0) {
        uint32_t i = 0;
        for (i = 0; i < m_dwLastObjNum; i++) {
          if (!pdfium::ContainsKey(m_ObjectOffsets, i))
            continue;
          if (m_File.AppendDWord(i) < 0)
            return -1;
          if (m_File.AppendString(" 1 ") < 0)
            return -1;
        }
        if (m_File.AppendString("]/Length ") < 0)
          return -1;
        if (m_File.AppendDWord(m_dwLastObjNum * 5) < 0)
          return -1;
        if (m_File.AppendString(">>stream\r\n") < 0)
          return -1;
        for (i = 0; i < m_dwLastObjNum; i++) {
          auto it = m_ObjectOffsets.find(i);
          if (it == m_ObjectOffsets.end())
            continue;
          OutputIndex(&m_File, it->second);
        }
      } else {
        size_t count = m_NewObjNumArray.size();
        size_t i = 0;
        for (i = 0; i < count; i++) {
          if (m_File.AppendDWord(m_NewObjNumArray[i]) < 0)
            return -1;
          if (m_File.AppendString(" 1 ") < 0)
            return -1;
        }
        if (m_File.AppendString("]/Length ") < 0)
          return -1;
        if (m_File.AppendDWord(count * 5) < 0)
          return -1;
        if (m_File.AppendString(">>stream\r\n") < 0)
          return -1;
        for (i = 0; i < count; i++)
          OutputIndex(&m_File, m_ObjectOffsets[m_NewObjNumArray[i]]);
      }
      if (m_File.AppendString("\r\nendstream") < 0)
        return -1;
    }
  }

  if (m_File.AppendString("\r\nstartxref\r\n") < 0)
    return -1;

  char offset_buf[20];
  memset(offset_buf, 0, sizeof(offset_buf));
  FXSYS_i64toa(m_XrefStart, offset_buf, 10);
  if (m_File.AppendBlock(offset_buf, FXSYS_strlen(offset_buf)) < 0)
    return -1;
  if (m_File.AppendString("\r\n%%EOF\r\n") < 0)
    return -1;

  m_File.Flush();
  return m_iStage = 100;
}

void CPDF_Creator::Clear() {
  m_pXRefStream.reset();
  m_File.Clear();
  m_NewObjNumArray.clear();
  m_pIDArray.reset();
}

bool CPDF_Creator::Create(uint32_t flags) {
  m_dwFlags = flags;
  m_iStage = 0;
  m_Offset = 0;
  m_dwLastObjNum = m_pDocument->GetLastObjNum();
  m_ObjectOffsets.clear();
  m_NewObjNumArray.clear();
  InitID();
  if (flags & FPDFCREATE_PROGRESSIVE)
    return true;
  return Continue() > -1;
}

void CPDF_Creator::InitID(bool bDefault) {
  CPDF_Array* pOldIDArray = m_pParser ? m_pParser->GetIDArray() : nullptr;
  bool bNewId = !m_pIDArray;
  if (bNewId) {
    m_pIDArray = pdfium::MakeUnique<CPDF_Array>();
    CPDF_Object* pID1 = pOldIDArray ? pOldIDArray->GetObjectAt(0) : nullptr;
    if (pID1) {
      m_pIDArray->Add(pID1->Clone());
    } else {
      std::vector<uint8_t> buffer =
          GenerateFileID((uint32_t)(uintptr_t)this, m_dwLastObjNum);
      CFX_ByteString bsBuffer(buffer.data(), buffer.size());
      m_pIDArray->AddNew<CPDF_String>(bsBuffer, true);
    }
  }
  if (!bDefault)
    return;

  if (pOldIDArray) {
    CPDF_Object* pID2 = pOldIDArray->GetObjectAt(1);
    if (IsIncremental() && m_pEncryptDict && pID2) {
      m_pIDArray->Add(pID2->Clone());
      return;
    }
    std::vector<uint8_t> buffer =
        GenerateFileID((uint32_t)(uintptr_t)this, m_dwLastObjNum);
    CFX_ByteString bsBuffer(buffer.data(), buffer.size());
    m_pIDArray->AddNew<CPDF_String>(bsBuffer, true);
    return;
  }

  m_pIDArray->Add(m_pIDArray->GetObjectAt(0)->Clone());
  if (m_pEncryptDict && !pOldIDArray && m_pParser && bNewId) {
    if (m_pEncryptDict->GetStringFor("Filter") == "Standard") {
      CFX_ByteString user_pass = m_pParser->GetPassword();
      uint32_t flag = PDF_ENCRYPT_CONTENT;
      CPDF_SecurityHandler handler;
      handler.OnCreate(m_pEncryptDict, m_pIDArray.get(), user_pass.raw_str(),
                       user_pass.GetLength(), flag);
      m_pCryptoHandler = pdfium::MakeRetain<CPDF_CryptoHandler>();
      m_pCryptoHandler->Init(m_pEncryptDict, &handler);
      m_bSecurityChanged = true;
    }
  }
}

int32_t CPDF_Creator::Continue() {
  if (m_iStage < 0)
    return m_iStage;

  int32_t iRet = 0;
  while (m_iStage < 100) {
    if (m_iStage < 20)
      iRet = WriteDoc_Stage1();
    else if (m_iStage < 30)
      iRet = WriteDoc_Stage2();
    else if (m_iStage < 90)
      iRet = WriteDoc_Stage3();
    else
      iRet = WriteDoc_Stage4();

    if (iRet < m_iStage)
      break;
  }

  if (iRet < 1 || m_iStage == 100) {
    m_iStage = -1;
    Clear();
    return iRet > 99 ? 0 : (iRet < 1 ? -1 : iRet);
  }
  return m_iStage;
}

bool CPDF_Creator::SetFileVersion(int32_t fileVersion) {
  if (fileVersion < 10 || fileVersion > 17)
    return false;

  m_FileVersion = fileVersion;
  return true;
}

void CPDF_Creator::IncrementOffset(FX_FILESIZE inc) {
  pdfium::base::CheckedNumeric<FX_FILESIZE> size = m_Offset;
  size += inc;
  m_Offset = size.ValueOrDie();
}

void CPDF_Creator::RemoveSecurity() {
  m_pCryptoHandler.Reset();
  m_bSecurityChanged = true;
  m_pEncryptDict = nullptr;
}
