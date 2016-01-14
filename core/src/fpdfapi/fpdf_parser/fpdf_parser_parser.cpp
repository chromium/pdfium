// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/fpdf_parser.h"

#include <algorithm>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fxcrt/fx_ext.h"
#include "core/include/fxcrt/fx_safe_types.h"
#include "core/src/fpdfapi/fpdf_page/pageint.h"
#include "core/src/fpdfapi/fpdf_parser/parser_int.h"
#include "third_party/base/stl_util.h"

namespace {

// A limit on the size of the xref table. Theoretical limits are higher, but
// this may be large enough in practice.
const int32_t kMaxXRefSize = 1048576;

// A limit on the maximum object number in the xref table. Theoretical limits
// are higher, but this may be large enough in practice.
const FX_DWORD kMaxObjectNumber = 1048576;

struct SearchTagRecord {
  const char* m_pTag;
  FX_DWORD m_Len;
  FX_DWORD m_Offset;
};

template <typename T>
class ScopedSetInsertion {
 public:
  ScopedSetInsertion(std::set<T>* org_set, T elem)
      : m_Set(org_set), m_Entry(elem) {
    m_Set->insert(m_Entry);
  }
  ~ScopedSetInsertion() { m_Set->erase(m_Entry); }

 private:
  std::set<T>* const m_Set;
  const T m_Entry;
};

int CompareFileSize(const void* p1, const void* p2) {
  return *(FX_FILESIZE*)p1 - *(FX_FILESIZE*)p2;
}

int32_t GetHeaderOffset(IFX_FileRead* pFile) {
  const FX_DWORD tag = FXDWORD_FROM_LSBFIRST(0x46445025);
  const size_t kBufSize = 4;
  uint8_t buf[kBufSize];
  int32_t offset = 0;
  while (offset <= 1024) {
    if (!pFile->ReadBlock(buf, offset, kBufSize))
      return -1;

    if (*(FX_DWORD*)buf == tag)
      return offset;

    ++offset;
  }
  return -1;
}

int32_t GetDirectInteger(CPDF_Dictionary* pDict, const CFX_ByteStringC& key) {
  CPDF_Number* pObj = ToNumber(pDict->GetElement(key));
  return pObj ? pObj->GetInteger() : 0;
}

FX_DWORD GetVarInt(const uint8_t* p, int32_t n) {
  FX_DWORD result = 0;
  for (int32_t i = 0; i < n; ++i)
    result = result * 256 + p[i];
  return result;
}

int32_t GetStreamNCount(CPDF_StreamAcc* pObjStream) {
  return pObjStream->GetDict()->GetInteger("N");
}

int32_t GetStreamFirst(CPDF_StreamAcc* pObjStream) {
  return pObjStream->GetDict()->GetInteger("First");
}

bool CanReadFromBitStream(const CFX_BitStream* hStream,
                          const FX_SAFE_DWORD& num_bits) {
  return (num_bits.IsValid() &&
          hStream->BitsRemaining() >= num_bits.ValueOrDie());
}

}  // namespace

// TODO(thestig) Using unique_ptr with ReleaseDeleter is still not ideal.
// Come up or wait for something better.
using ScopedFileStream =
    std::unique_ptr<IFX_FileStream, ReleaseDeleter<IFX_FileStream>>;

bool IsSignatureDict(const CPDF_Dictionary* pDict) {
  CPDF_Object* pType = pDict->GetElementValue("Type");
  if (!pType)
    pType = pDict->GetElementValue("FT");
  return pType && pType->GetString() == "Sig";
}

CPDF_Parser::CPDF_Parser() {
  m_pDocument = NULL;
  m_pTrailer = NULL;
  m_pEncryptDict = NULL;
  m_pLinearized = NULL;
  m_dwFirstPageNo = 0;
  m_dwXrefStartObjNum = 0;
  m_bOwnFileRead = TRUE;
  m_FileVersion = 0;
  m_bForceUseSecurityHandler = FALSE;
}
CPDF_Parser::~CPDF_Parser() {
  CloseParser(FALSE);
}

FX_DWORD CPDF_Parser::GetLastObjNum() const {
  return m_ObjectInfo.empty() ? 0 : m_ObjectInfo.rbegin()->first;
}

bool CPDF_Parser::IsValidObjectNumber(FX_DWORD objnum) const {
  return !m_ObjectInfo.empty() && objnum <= m_ObjectInfo.rbegin()->first;
}

void CPDF_Parser::SetEncryptDictionary(CPDF_Dictionary* pDict) {
  m_pEncryptDict = pDict;
}

FX_FILESIZE CPDF_Parser::GetObjectPositionOrZero(FX_DWORD objnum) const {
  auto it = m_ObjectInfo.find(objnum);
  return it != m_ObjectInfo.end() ? it->second.pos : 0;
}

void CPDF_Parser::ShrinkObjectMap(FX_DWORD objnum) {
  if (objnum == 0) {
    m_ObjectInfo.clear();
    return;
  }

  auto it = m_ObjectInfo.lower_bound(objnum);
  while (it != m_ObjectInfo.end()) {
    auto saved_it = it++;
    m_ObjectInfo.erase(saved_it);
  }

  if (!pdfium::ContainsKey(m_ObjectInfo, objnum - 1))
    m_ObjectInfo[objnum - 1].pos = 0;
}

void CPDF_Parser::CloseParser(FX_BOOL bReParse) {
  m_bVersionUpdated = FALSE;
  if (!bReParse) {
    delete m_pDocument;
    m_pDocument = NULL;
  }
  if (m_pTrailer) {
    m_pTrailer->Release();
    m_pTrailer = NULL;
  }
  ReleaseEncryptHandler();
  SetEncryptDictionary(NULL);
  if (m_bOwnFileRead && m_Syntax.m_pFileAccess) {
    m_Syntax.m_pFileAccess->Release();
    m_Syntax.m_pFileAccess = NULL;
  }
  FX_POSITION pos = m_ObjectStreamMap.GetStartPosition();
  while (pos) {
    void* objnum;
    CPDF_StreamAcc* pStream;
    m_ObjectStreamMap.GetNextAssoc(pos, objnum, (void*&)pStream);
    delete pStream;
  }
  m_ObjectStreamMap.RemoveAll();
  m_ObjCache.clear();

  m_SortedOffset.RemoveAll();
  m_ObjectInfo.clear();
  m_V5Type.RemoveAll();
  m_ObjVersion.RemoveAll();
  int32_t iLen = m_Trailers.GetSize();
  for (int32_t i = 0; i < iLen; ++i) {
    if (CPDF_Dictionary* trailer = m_Trailers.GetAt(i))
      trailer->Release();
  }
  m_Trailers.RemoveAll();
  if (m_pLinearized) {
    m_pLinearized->Release();
    m_pLinearized = NULL;
  }
}
CPDF_SecurityHandler* FPDF_CreateStandardSecurityHandler();
CPDF_SecurityHandler* FPDF_CreatePubKeyHandler(void*);
FX_DWORD CPDF_Parser::StartParse(IFX_FileRead* pFileAccess,
                                 FX_BOOL bReParse,
                                 FX_BOOL bOwnFileRead) {
  CloseParser(bReParse);
  m_bXRefStream = FALSE;
  m_LastXRefOffset = 0;
  m_bOwnFileRead = bOwnFileRead;

  int32_t offset = GetHeaderOffset(pFileAccess);
  if (offset == -1) {
    if (bOwnFileRead && pFileAccess)
      pFileAccess->Release();
    return PDFPARSE_ERROR_FORMAT;
  }
  m_Syntax.InitParser(pFileAccess, offset);

  uint8_t ch;
  if (!m_Syntax.GetCharAt(5, ch))
    return PDFPARSE_ERROR_FORMAT;
  if (std::isdigit(ch))
    m_FileVersion = FXSYS_toDecimalDigit(ch) * 10;

  if (!m_Syntax.GetCharAt(7, ch))
    return PDFPARSE_ERROR_FORMAT;
  if (std::isdigit(ch))
    m_FileVersion += FXSYS_toDecimalDigit(ch);

  if (m_Syntax.m_FileLen < m_Syntax.m_HeaderOffset + 9)
    return PDFPARSE_ERROR_FORMAT;

  m_Syntax.RestorePos(m_Syntax.m_FileLen - m_Syntax.m_HeaderOffset - 9);
  if (!bReParse)
    m_pDocument = new CPDF_Document(this);

  FX_BOOL bXRefRebuilt = FALSE;
  if (m_Syntax.SearchWord("startxref", TRUE, FALSE, 4096)) {
    FX_FILESIZE startxref_offset = m_Syntax.SavePos();
    void* pResult = FXSYS_bsearch(&startxref_offset, m_SortedOffset.GetData(),
                                  m_SortedOffset.GetSize(), sizeof(FX_FILESIZE),
                                  CompareFileSize);
    if (!pResult)
      m_SortedOffset.Add(startxref_offset);

    m_Syntax.GetKeyword();
    bool bNumber;
    CFX_ByteString xrefpos_str = m_Syntax.GetNextWord(&bNumber);
    if (!bNumber)
      return PDFPARSE_ERROR_FORMAT;

    m_LastXRefOffset = (FX_FILESIZE)FXSYS_atoi64(xrefpos_str);
    if (!LoadAllCrossRefV4(m_LastXRefOffset) &&
        !LoadAllCrossRefV5(m_LastXRefOffset)) {
      if (!RebuildCrossRef())
        return PDFPARSE_ERROR_FORMAT;

      bXRefRebuilt = TRUE;
      m_LastXRefOffset = 0;
    }
  } else {
    if (!RebuildCrossRef())
      return PDFPARSE_ERROR_FORMAT;

    bXRefRebuilt = TRUE;
  }
  FX_DWORD dwRet = SetEncryptHandler();
  if (dwRet != PDFPARSE_ERROR_SUCCESS)
    return dwRet;

  m_pDocument->LoadDoc();
  if (!m_pDocument->GetRoot() || m_pDocument->GetPageCount() == 0) {
    if (bXRefRebuilt)
      return PDFPARSE_ERROR_FORMAT;

    ReleaseEncryptHandler();
    if (!RebuildCrossRef())
      return PDFPARSE_ERROR_FORMAT;

    dwRet = SetEncryptHandler();
    if (dwRet != PDFPARSE_ERROR_SUCCESS)
      return dwRet;

    m_pDocument->LoadDoc();
    if (!m_pDocument->GetRoot())
      return PDFPARSE_ERROR_FORMAT;
  }
  FXSYS_qsort(m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
              sizeof(FX_FILESIZE), CompareFileSize);
  if (GetRootObjNum() == 0) {
    ReleaseEncryptHandler();
    if (!RebuildCrossRef() || GetRootObjNum() == 0)
      return PDFPARSE_ERROR_FORMAT;

    dwRet = SetEncryptHandler();
    if (dwRet != PDFPARSE_ERROR_SUCCESS)
      return dwRet;
  }
  if (m_pSecurityHandler && !m_pSecurityHandler->IsMetadataEncrypted()) {
    CPDF_Reference* pMetadata =
        ToReference(m_pDocument->GetRoot()->GetElement("Metadata"));
    if (pMetadata)
      m_Syntax.m_MetadataObjnum = pMetadata->GetRefObjNum();
  }
  return PDFPARSE_ERROR_SUCCESS;
}
FX_DWORD CPDF_Parser::SetEncryptHandler() {
  ReleaseEncryptHandler();
  SetEncryptDictionary(NULL);
  if (!m_pTrailer) {
    return PDFPARSE_ERROR_FORMAT;
  }
  CPDF_Object* pEncryptObj = m_pTrailer->GetElement("Encrypt");
  if (pEncryptObj) {
    if (CPDF_Dictionary* pEncryptDict = pEncryptObj->AsDictionary()) {
      SetEncryptDictionary(pEncryptDict);
    } else if (CPDF_Reference* pRef = pEncryptObj->AsReference()) {
      pEncryptObj =
          m_pDocument->GetIndirectObject(pRef->GetRefObjNum(), nullptr);
      if (pEncryptObj)
        SetEncryptDictionary(pEncryptObj->GetDict());
    }
  }
  if (m_bForceUseSecurityHandler) {
    FX_DWORD err = PDFPARSE_ERROR_HANDLER;
    if (!m_pSecurityHandler) {
      return PDFPARSE_ERROR_HANDLER;
    }
    if (!m_pSecurityHandler->OnInit(this, m_pEncryptDict)) {
      return err;
    }
    std::unique_ptr<CPDF_CryptoHandler> pCryptoHandler(
        m_pSecurityHandler->CreateCryptoHandler());
    if (!pCryptoHandler->Init(m_pEncryptDict, m_pSecurityHandler.get())) {
      return PDFPARSE_ERROR_HANDLER;
    }
    m_Syntax.SetEncrypt(pCryptoHandler.release());
  } else if (m_pEncryptDict) {
    CFX_ByteString filter = m_pEncryptDict->GetString("Filter");
    std::unique_ptr<CPDF_SecurityHandler> pSecurityHandler;
    FX_DWORD err = PDFPARSE_ERROR_HANDLER;
    if (filter == "Standard") {
      pSecurityHandler.reset(FPDF_CreateStandardSecurityHandler());
      err = PDFPARSE_ERROR_PASSWORD;
    }
    if (!pSecurityHandler) {
      return PDFPARSE_ERROR_HANDLER;
    }
    if (!pSecurityHandler->OnInit(this, m_pEncryptDict)) {
      return err;
    }
    m_pSecurityHandler = std::move(pSecurityHandler);
    std::unique_ptr<CPDF_CryptoHandler> pCryptoHandler(
        m_pSecurityHandler->CreateCryptoHandler());
    if (!pCryptoHandler->Init(m_pEncryptDict, m_pSecurityHandler.get())) {
      return PDFPARSE_ERROR_HANDLER;
    }
    m_Syntax.SetEncrypt(pCryptoHandler.release());
  }
  return PDFPARSE_ERROR_SUCCESS;
}
void CPDF_Parser::ReleaseEncryptHandler() {
  m_Syntax.m_pCryptoHandler.reset();
  if (!m_bForceUseSecurityHandler) {
    m_pSecurityHandler.reset();
  }
}

FX_FILESIZE CPDF_Parser::GetObjectOffset(FX_DWORD objnum) const {
  if (!IsValidObjectNumber(objnum))
    return 0;

  if (m_V5Type[objnum] == 1)
    return GetObjectPositionOrZero(objnum);

  if (m_V5Type[objnum] == 2) {
    FX_FILESIZE pos = GetObjectPositionOrZero(objnum);
    return GetObjectPositionOrZero(pos);
  }
  return 0;
}

FX_BOOL CPDF_Parser::LoadAllCrossRefV4(FX_FILESIZE xrefpos) {
  if (!LoadCrossRefV4(xrefpos, 0, TRUE)) {
    return FALSE;
  }
  m_pTrailer = LoadTrailerV4();
  if (!m_pTrailer) {
    return FALSE;
  }

  int32_t xrefsize = GetDirectInteger(m_pTrailer, "Size");
  if (xrefsize <= 0 || xrefsize > kMaxXRefSize) {
    return FALSE;
  }
  ShrinkObjectMap(xrefsize);
  m_V5Type.SetSize(xrefsize);
  CFX_FileSizeArray CrossRefList;
  CFX_FileSizeArray XRefStreamList;
  CrossRefList.Add(xrefpos);
  XRefStreamList.Add(GetDirectInteger(m_pTrailer, "XRefStm"));

  std::set<FX_FILESIZE> seen_xrefpos;
  seen_xrefpos.insert(xrefpos);
  // When |m_pTrailer| doesn't have Prev entry or Prev entry value is not
  // numerical, GetDirectInteger() returns 0. Loading will end.
  xrefpos = GetDirectInteger(m_pTrailer, "Prev");
  while (xrefpos) {
    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos))
      return FALSE;
    seen_xrefpos.insert(xrefpos);
    CrossRefList.InsertAt(0, xrefpos);
    LoadCrossRefV4(xrefpos, 0, TRUE);
    std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>> pDict(
        LoadTrailerV4());
    if (!pDict)
      return FALSE;
    xrefpos = GetDirectInteger(pDict.get(), "Prev");

    XRefStreamList.InsertAt(0, pDict->GetInteger("XRefStm"));
    m_Trailers.Add(pDict.release());
  }
  for (int32_t i = 0; i < CrossRefList.GetSize(); i++) {
    if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], FALSE))
      return FALSE;
  }
  return TRUE;
}
FX_BOOL CPDF_Parser::LoadLinearizedAllCrossRefV4(FX_FILESIZE xrefpos,
                                                 FX_DWORD dwObjCount) {
  if (!LoadLinearizedCrossRefV4(xrefpos, dwObjCount)) {
    return FALSE;
  }
  m_pTrailer = LoadTrailerV4();
  if (!m_pTrailer) {
    return FALSE;
  }
  int32_t xrefsize = GetDirectInteger(m_pTrailer, "Size");
  if (xrefsize == 0) {
    return FALSE;
  }
  CFX_FileSizeArray CrossRefList, XRefStreamList;
  CrossRefList.Add(xrefpos);
  XRefStreamList.Add(GetDirectInteger(m_pTrailer, "XRefStm"));

  std::set<FX_FILESIZE> seen_xrefpos;
  seen_xrefpos.insert(xrefpos);
  xrefpos = GetDirectInteger(m_pTrailer, "Prev");
  while (xrefpos) {
    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos))
      return FALSE;
    seen_xrefpos.insert(xrefpos);
    CrossRefList.InsertAt(0, xrefpos);
    LoadCrossRefV4(xrefpos, 0, TRUE);
    std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>> pDict(
        LoadTrailerV4());
    if (!pDict) {
      return FALSE;
    }
    xrefpos = GetDirectInteger(pDict.get(), "Prev");

    XRefStreamList.InsertAt(0, pDict->GetInteger("XRefStm"));
    m_Trailers.Add(pDict.release());
  }
  for (int32_t i = 1; i < CrossRefList.GetSize(); i++)
    if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], FALSE)) {
      return FALSE;
    }
  return TRUE;
}
FX_BOOL CPDF_Parser::LoadLinearizedCrossRefV4(FX_FILESIZE pos,
                                              FX_DWORD dwObjCount) {
  FX_FILESIZE dwStartPos = pos - m_Syntax.m_HeaderOffset;
  m_Syntax.RestorePos(dwStartPos);
  void* pResult =
      FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
                    sizeof(FX_FILESIZE), CompareFileSize);
  if (!pResult) {
    m_SortedOffset.Add(pos);
  }
  FX_DWORD start_objnum = 0;
  FX_DWORD count = dwObjCount;
  FX_FILESIZE SavedPos = m_Syntax.SavePos();
  const int32_t recordsize = 20;
  std::vector<char> buf(1024 * recordsize + 1);
  buf[1024 * recordsize] = '\0';
  int32_t nBlocks = count / 1024 + 1;
  for (int32_t block = 0; block < nBlocks; block++) {
    int32_t block_size = block == nBlocks - 1 ? count % 1024 : 1024;
    FX_DWORD dwReadSize = block_size * recordsize;
    if ((FX_FILESIZE)(dwStartPos + dwReadSize) > m_Syntax.m_FileLen) {
      return FALSE;
    }
    if (!m_Syntax.ReadBlock(reinterpret_cast<uint8_t*>(buf.data()),
                            dwReadSize)) {
      return FALSE;
    }
    for (int32_t i = 0; i < block_size; i++) {
      FX_DWORD objnum = start_objnum + block * 1024 + i;
      char* pEntry = &buf[i * recordsize];
      if (pEntry[17] == 'f') {
        m_ObjectInfo[objnum].pos = 0;
        m_V5Type.SetAtGrow(objnum, 0);
      } else {
        int32_t offset = FXSYS_atoi(pEntry);
        if (offset == 0) {
          for (int32_t c = 0; c < 10; c++) {
            if (!std::isdigit(pEntry[c]))
              return FALSE;
          }
        }
        m_ObjectInfo[objnum].pos = offset;
        int32_t version = FXSYS_atoi(pEntry + 11);
        if (version >= 1) {
          m_bVersionUpdated = TRUE;
        }
        m_ObjVersion.SetAtGrow(objnum, version);
        if (m_ObjectInfo[objnum].pos < m_Syntax.m_FileLen) {
          void* pResult = FXSYS_bsearch(
              &m_ObjectInfo[objnum].pos, m_SortedOffset.GetData(),
              m_SortedOffset.GetSize(), sizeof(FX_FILESIZE), CompareFileSize);
          if (!pResult) {
            m_SortedOffset.Add(m_ObjectInfo[objnum].pos);
          }
        }
        m_V5Type.SetAtGrow(objnum, 1);
      }
    }
  }
  m_Syntax.RestorePos(SavedPos + count * recordsize);
  return TRUE;
}

bool CPDF_Parser::FindPosInOffsets(FX_FILESIZE pos) const {
  return FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
                       sizeof(FX_FILESIZE), CompareFileSize);
}

bool CPDF_Parser::LoadCrossRefV4(FX_FILESIZE pos,
                                 FX_FILESIZE streampos,
                                 FX_BOOL bSkip) {
  m_Syntax.RestorePos(pos);
  if (m_Syntax.GetKeyword() != "xref")
    return false;

  if (!FindPosInOffsets(pos))
    m_SortedOffset.Add(pos);

  if (streampos && !FindPosInOffsets(streampos))
      m_SortedOffset.Add(streampos);

  while (1) {
    FX_FILESIZE SavedPos = m_Syntax.SavePos();
    bool bIsNumber;
    CFX_ByteString word = m_Syntax.GetNextWord(&bIsNumber);
    if (word.IsEmpty())
      return false;

    if (!bIsNumber) {
      m_Syntax.RestorePos(SavedPos);
      break;
    }
    FX_DWORD start_objnum = FXSYS_atoi(word);
    if (start_objnum >= kMaxObjectNumber)
      return false;

    FX_DWORD count = m_Syntax.GetDirectNum();
    m_Syntax.ToNextWord();
    SavedPos = m_Syntax.SavePos();
    const int32_t recordsize = 20;
    m_dwXrefStartObjNum = start_objnum;
    if (!bSkip) {
      std::vector<char> buf(1024 * recordsize + 1);
      buf[1024 * recordsize] = '\0';
      int32_t nBlocks = count / 1024 + 1;
      for (int32_t block = 0; block < nBlocks; block++) {
        int32_t block_size = block == nBlocks - 1 ? count % 1024 : 1024;
        m_Syntax.ReadBlock(reinterpret_cast<uint8_t*>(buf.data()),
                           block_size * recordsize);
        for (int32_t i = 0; i < block_size; i++) {
          FX_DWORD objnum = start_objnum + block * 1024 + i;
          char* pEntry = &buf[i * recordsize];
          if (pEntry[17] == 'f') {
            m_ObjectInfo[objnum].pos = 0;
            m_V5Type.SetAtGrow(objnum, 0);
          } else {
            FX_FILESIZE offset = (FX_FILESIZE)FXSYS_atoi64(pEntry);
            if (offset == 0) {
              for (int32_t c = 0; c < 10; c++) {
                if (!std::isdigit(pEntry[c]))
                  return false;
              }
            }
            m_ObjectInfo[objnum].pos = offset;
            int32_t version = FXSYS_atoi(pEntry + 11);
            if (version >= 1) {
              m_bVersionUpdated = TRUE;
            }
            m_ObjVersion.SetAtGrow(objnum, version);
            if (m_ObjectInfo[objnum].pos < m_Syntax.m_FileLen &&
                !FindPosInOffsets(m_ObjectInfo[objnum].pos)) {
              m_SortedOffset.Add(m_ObjectInfo[objnum].pos);
            }
            m_V5Type.SetAtGrow(objnum, 1);
          }
        }
      }
    }
    m_Syntax.RestorePos(SavedPos + count * recordsize);
  }
  return !streampos || LoadCrossRefV5(&streampos, FALSE);
}

FX_BOOL CPDF_Parser::LoadAllCrossRefV5(FX_FILESIZE xrefpos) {
  if (!LoadCrossRefV5(&xrefpos, TRUE)) {
    return FALSE;
  }
  std::set<FX_FILESIZE> seen_xrefpos;
  while (xrefpos) {
    seen_xrefpos.insert(xrefpos);
    if (!LoadCrossRefV5(&xrefpos, FALSE)) {
      return FALSE;
    }
    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos)) {
      return FALSE;
    }
  }
  m_ObjectStreamMap.InitHashTable(101, FALSE);
  m_bXRefStream = TRUE;
  return TRUE;
}

FX_BOOL CPDF_Parser::RebuildCrossRef() {
  m_ObjectInfo.clear();
  m_V5Type.RemoveAll();
  m_SortedOffset.RemoveAll();
  m_ObjVersion.RemoveAll();
  if (m_pTrailer) {
    m_pTrailer->Release();
    m_pTrailer = NULL;
  }
  int32_t status = 0;
  int32_t inside_index = 0;
  FX_DWORD objnum = 0;
  FX_DWORD gennum = 0;
  int32_t depth = 0;
  const FX_DWORD kBufferSize = 4096;
  std::vector<uint8_t> buffer(kBufferSize);
  FX_FILESIZE pos = m_Syntax.m_HeaderOffset;
  FX_FILESIZE start_pos = 0;
  FX_FILESIZE start_pos1 = 0;
  FX_FILESIZE last_obj = -1;
  FX_FILESIZE last_xref = -1;
  FX_FILESIZE last_trailer = -1;
  while (pos < m_Syntax.m_FileLen) {
    const FX_FILESIZE saved_pos = pos;
    bool bOverFlow = false;
    FX_DWORD size = std::min((FX_DWORD)(m_Syntax.m_FileLen - pos), kBufferSize);
    if (!m_Syntax.m_pFileAccess->ReadBlock(buffer.data(), pos, size))
      break;

    for (FX_DWORD i = 0; i < size; i++) {
      uint8_t byte = buffer[i];
      switch (status) {
        case 0:
          if (PDFCharIsWhitespace(byte))
            status = 1;

          if (std::isdigit(byte)) {
            --i;
            status = 1;
          }

          if (byte == '%') {
            inside_index = 0;
            status = 9;
          }

          if (byte == '(') {
            status = 10;
            depth = 1;
          }

          if (byte == '<') {
            inside_index = 1;
            status = 11;
          }

          if (byte == '\\')
            status = 13;

          if (byte == 't') {
            status = 7;
            inside_index = 1;
          }
          break;
        case 1:
          if (PDFCharIsWhitespace(byte)) {
            break;
          } else if (std::isdigit(byte)) {
            start_pos = pos + i;
            status = 2;
            objnum = FXSYS_toDecimalDigit(byte);
          } else if (byte == 't') {
            status = 7;
            inside_index = 1;
          } else if (byte == 'x') {
            status = 8;
            inside_index = 1;
          } else {
            --i;
            status = 0;
          }
          break;
        case 2:
          if (std::isdigit(byte)) {
            objnum = objnum * 10 + FXSYS_toDecimalDigit(byte);
            break;
          } else if (PDFCharIsWhitespace(byte)) {
            status = 3;
          } else {
            --i;
            status = 14;
            inside_index = 0;
          }
          break;
        case 3:
          if (std::isdigit(byte)) {
            start_pos1 = pos + i;
            status = 4;
            gennum = FXSYS_toDecimalDigit(byte);
          } else if (PDFCharIsWhitespace(byte)) {
            break;
          } else if (byte == 't') {
            status = 7;
            inside_index = 1;
          } else {
            --i;
            status = 0;
          }
          break;
        case 4:
          if (std::isdigit(byte)) {
            gennum = gennum * 10 + FXSYS_toDecimalDigit(byte);
            break;
          } else if (PDFCharIsWhitespace(byte)) {
            status = 5;
          } else {
            --i;
            status = 0;
          }
          break;
        case 5:
          if (byte == 'o') {
            status = 6;
            inside_index = 1;
          } else if (PDFCharIsWhitespace(byte)) {
            break;
          } else if (std::isdigit(byte)) {
            objnum = gennum;
            gennum = FXSYS_toDecimalDigit(byte);
            start_pos = start_pos1;
            start_pos1 = pos + i;
            status = 4;
          } else if (byte == 't') {
            status = 7;
            inside_index = 1;
          } else {
            --i;
            status = 0;
          }
          break;
        case 6:
          switch (inside_index) {
            case 1:
              if (byte != 'b') {
                --i;
                status = 0;
              } else {
                inside_index++;
              }
              break;
            case 2:
              if (byte != 'j') {
                --i;
                status = 0;
              } else {
                inside_index++;
              }
              break;
            case 3:
              if (PDFCharIsWhitespace(byte) || PDFCharIsDelimiter(byte)) {
                if (objnum > 0x1000000) {
                  status = 0;
                  break;
                }
                FX_FILESIZE obj_pos = start_pos - m_Syntax.m_HeaderOffset;
                last_obj = start_pos;
                void* pResult =
                    FXSYS_bsearch(&obj_pos, m_SortedOffset.GetData(),
                                  m_SortedOffset.GetSize(), sizeof(FX_FILESIZE),
                                  CompareFileSize);
                if (!pResult) {
                  m_SortedOffset.Add(obj_pos);
                }
                FX_FILESIZE obj_end = 0;
                CPDF_Object* pObject = ParseIndirectObjectAtByStrict(
                    m_pDocument, obj_pos, objnum, NULL, &obj_end);
                if (CPDF_Stream* pStream = ToStream(pObject)) {
                  if (CPDF_Dictionary* pDict = pStream->GetDict()) {
                    if ((pDict->KeyExist("Type")) &&
                        (pDict->GetString("Type") == "XRef" &&
                         pDict->KeyExist("Size"))) {
                      CPDF_Object* pRoot = pDict->GetElement("Root");
                      if (pRoot && pRoot->GetDict() &&
                          pRoot->GetDict()->GetElement("Pages")) {
                        if (m_pTrailer)
                          m_pTrailer->Release();
                        m_pTrailer = ToDictionary(pDict->Clone());
                        }
                      }
                  }
                }
                FX_FILESIZE offset = 0;
                m_Syntax.RestorePos(obj_pos);
                offset = m_Syntax.FindTag("obj", 0);
                if (offset == -1) {
                  offset = 0;
                } else {
                  offset += 3;
                }
                FX_FILESIZE nLen = obj_end - obj_pos - offset;
                if ((FX_DWORD)nLen > size - i) {
                  pos = obj_end + m_Syntax.m_HeaderOffset;
                  bOverFlow = true;
                } else {
                  i += (FX_DWORD)nLen;
                }
                if (!m_ObjectInfo.empty() && IsValidObjectNumber(objnum) &&
                    m_ObjectInfo[objnum].pos) {
                  if (pObject) {
                    FX_DWORD oldgen = m_ObjVersion.GetAt(objnum);
                    m_ObjectInfo[objnum].pos = obj_pos;
                    m_ObjVersion.SetAt(objnum, (int16_t)gennum);
                    if (oldgen != gennum) {
                      m_bVersionUpdated = TRUE;
                    }
                  }
                } else {
                  m_ObjectInfo[objnum].pos = obj_pos;
                  m_V5Type.SetAtGrow(objnum, 1);
                  m_ObjVersion.SetAtGrow(objnum, (int16_t)gennum);
                }
                if (pObject) {
                  pObject->Release();
                }
              }
              --i;
              status = 0;
              break;
          }
          break;
        case 7:
          if (inside_index == 7) {
            if (PDFCharIsWhitespace(byte) || PDFCharIsDelimiter(byte)) {
              last_trailer = pos + i - 7;
              m_Syntax.RestorePos(pos + i - m_Syntax.m_HeaderOffset);
              CPDF_Object* pObj =
                  m_Syntax.GetObject(m_pDocument, 0, 0, nullptr, true);
              if (pObj) {
                if (!pObj->IsDictionary() && !pObj->AsStream()) {
                  pObj->Release();
                } else {
                  CPDF_Stream* pStream = pObj->AsStream();
                  if (CPDF_Dictionary* pTrailer =
                          pStream ? pStream->GetDict() : pObj->AsDictionary()) {
                    if (m_pTrailer) {
                      CPDF_Object* pRoot = pTrailer->GetElement("Root");
                      CPDF_Reference* pRef = ToReference(pRoot);
                      if (!pRoot ||
                          (pRef && IsValidObjectNumber(pRef->GetRefObjNum()) &&
                           m_ObjectInfo[pRef->GetRefObjNum()].pos != 0)) {
                        auto it = pTrailer->begin();
                        while (it != pTrailer->end()) {
                          const CFX_ByteString& key = it->first;
                          CPDF_Object* pElement = it->second;
                          ++it;
                          FX_DWORD dwObjNum =
                              pElement ? pElement->GetObjNum() : 0;
                          if (dwObjNum) {
                            m_pTrailer->SetAtReference(key, m_pDocument,
                                                       dwObjNum);
                          } else {
                            m_pTrailer->SetAt(key, pElement->Clone());
                          }
                        }
                        pObj->Release();
                      } else {
                        pObj->Release();
                      }
                    } else {
                      if (pObj->IsStream()) {
                        m_pTrailer = ToDictionary(pTrailer->Clone());
                        pObj->Release();
                      } else {
                        m_pTrailer = pTrailer;
                      }
                      FX_FILESIZE dwSavePos = m_Syntax.SavePos();
                      CFX_ByteString strWord = m_Syntax.GetKeyword();
                      if (!strWord.Compare("startxref")) {
                        bool bNumber;
                        CFX_ByteString bsOffset =
                            m_Syntax.GetNextWord(&bNumber);
                        if (bNumber) {
                          m_LastXRefOffset = FXSYS_atoi(bsOffset);
                        }
                      }
                      m_Syntax.RestorePos(dwSavePos);
                    }
                  } else {
                    pObj->Release();
                  }
                }
              }
            }
            --i;
            status = 0;
          } else if (byte == "trailer"[inside_index]) {
            inside_index++;
          } else {
            --i;
            status = 0;
          }
          break;
        case 8:
          if (inside_index == 4) {
            last_xref = pos + i - 4;
            status = 1;
          } else if (byte == "xref"[inside_index]) {
            inside_index++;
          } else {
            --i;
            status = 0;
          }
          break;
        case 9:
          if (byte == '\r' || byte == '\n') {
            status = 0;
          }
          break;
        case 10:
          if (byte == ')') {
            if (depth > 0) {
              depth--;
            }
          } else if (byte == '(') {
            depth++;
          }
          if (!depth) {
            status = 0;
          }
          break;
        case 11:
          if (byte == '>' || (byte == '<' && inside_index == 1))
            status = 0;
          inside_index = 0;
          break;
        case 13:
          if (PDFCharIsDelimiter(byte) || PDFCharIsWhitespace(byte)) {
            --i;
            status = 0;
          }
          break;
        case 14:
          if (PDFCharIsWhitespace(byte)) {
            status = 0;
          } else if (byte == '%' || byte == '(' || byte == '<' ||
                     byte == '\\') {
            status = 0;
            --i;
          } else if (inside_index == 6) {
            status = 0;
            --i;
          } else if (byte == "endobj"[inside_index]) {
            inside_index++;
          }
          break;
      }
      if (bOverFlow) {
        size = 0;
        break;
      }
    }
    pos += size;

    // If the position has not changed at all in a loop iteration, then break
    // out to prevent infinite looping.
    if (pos == saved_pos)
      break;
  }
  if (last_xref != -1 && last_xref > last_obj) {
    last_trailer = last_xref;
  } else if (last_trailer == -1 || last_xref < last_obj) {
    last_trailer = m_Syntax.m_FileLen;
  }
  FX_FILESIZE offset = last_trailer - m_Syntax.m_HeaderOffset;
  void* pResult =
      FXSYS_bsearch(&offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
                    sizeof(FX_FILESIZE), CompareFileSize);
  if (!pResult) {
    m_SortedOffset.Add(offset);
  }
  return m_pTrailer && !m_ObjectInfo.empty();
}

FX_BOOL CPDF_Parser::LoadCrossRefV5(FX_FILESIZE* pos, FX_BOOL bMainXRef) {
  CPDF_Object* pObject = ParseIndirectObjectAt(m_pDocument, *pos, 0, nullptr);
  if (!pObject)
    return FALSE;
  if (m_pDocument) {
    FX_BOOL bInserted = FALSE;
    CPDF_Dictionary* pDict = m_pDocument->GetRoot();
    if (!pDict || pDict->GetObjNum() != pObject->m_ObjNum) {
      bInserted = m_pDocument->InsertIndirectObject(pObject->m_ObjNum, pObject);
    } else {
      if (pObject->IsStream())
        pObject->Release();
    }
    if (!bInserted)
      return FALSE;
  }

  CPDF_Stream* pStream = pObject->AsStream();
  if (!pStream)
    return FALSE;

  *pos = pStream->GetDict()->GetInteger("Prev");
  int32_t size = pStream->GetDict()->GetInteger("Size");
  if (size < 0) {
    pStream->Release();
    return FALSE;
  }
  if (bMainXRef) {
    m_pTrailer = ToDictionary(pStream->GetDict()->Clone());
    ShrinkObjectMap(size);
    if (m_V5Type.SetSize(size)) {
      FXSYS_memset(m_V5Type.GetData(), 0, size);
    }
  } else {
    m_Trailers.Add(ToDictionary(pStream->GetDict()->Clone()));
  }
  std::vector<std::pair<int32_t, int32_t> > arrIndex;
  CPDF_Array* pArray = pStream->GetDict()->GetArray("Index");
  if (pArray) {
    FX_DWORD nPairSize = pArray->GetCount() / 2;
    for (FX_DWORD i = 0; i < nPairSize; i++) {
      CPDF_Object* pStartNumObj = pArray->GetElement(i * 2);
      CPDF_Object* pCountObj = pArray->GetElement(i * 2 + 1);
      if (ToNumber(pStartNumObj) && ToNumber(pCountObj)) {
        int nStartNum = pStartNumObj->GetInteger();
        int nCount = pCountObj->GetInteger();
        if (nStartNum >= 0 && nCount > 0) {
          arrIndex.push_back(std::make_pair(nStartNum, nCount));
        }
      }
    }
  }
  if (arrIndex.size() == 0) {
    arrIndex.push_back(std::make_pair(0, size));
  }
  pArray = pStream->GetDict()->GetArray("W");
  if (!pArray) {
    pStream->Release();
    return FALSE;
  }
  CFX_DWordArray WidthArray;
  FX_SAFE_DWORD dwAccWidth = 0;
  for (FX_DWORD i = 0; i < pArray->GetCount(); i++) {
    WidthArray.Add(pArray->GetInteger(i));
    dwAccWidth += WidthArray[i];
  }
  if (!dwAccWidth.IsValid() || WidthArray.GetSize() < 3) {
    pStream->Release();
    return FALSE;
  }
  FX_DWORD totalWidth = dwAccWidth.ValueOrDie();
  CPDF_StreamAcc acc;
  acc.LoadAllData(pStream);
  const uint8_t* pData = acc.GetData();
  FX_DWORD dwTotalSize = acc.GetSize();
  FX_DWORD segindex = 0;
  for (FX_DWORD i = 0; i < arrIndex.size(); i++) {
    int32_t startnum = arrIndex[i].first;
    if (startnum < 0) {
      continue;
    }
    m_dwXrefStartObjNum =
        pdfium::base::checked_cast<FX_DWORD, int32_t>(startnum);
    FX_DWORD count =
        pdfium::base::checked_cast<FX_DWORD, int32_t>(arrIndex[i].second);
    FX_SAFE_DWORD dwCaculatedSize = segindex;
    dwCaculatedSize += count;
    dwCaculatedSize *= totalWidth;
    if (!dwCaculatedSize.IsValid() ||
        dwCaculatedSize.ValueOrDie() > dwTotalSize) {
      continue;
    }
    const uint8_t* segstart = pData + segindex * totalWidth;
    FX_SAFE_DWORD dwMaxObjNum = startnum;
    dwMaxObjNum += count;
    FX_DWORD dwV5Size =
        pdfium::base::checked_cast<FX_DWORD, int32_t>(m_V5Type.GetSize());
    if (!dwMaxObjNum.IsValid() || dwMaxObjNum.ValueOrDie() > dwV5Size) {
      continue;
    }
    for (FX_DWORD j = 0; j < count; j++) {
      int32_t type = 1;
      const uint8_t* entrystart = segstart + j * totalWidth;
      if (WidthArray[0]) {
        type = GetVarInt(entrystart, WidthArray[0]);
      }
      if (m_V5Type[startnum + j] == 255) {
        FX_FILESIZE offset =
            GetVarInt(entrystart + WidthArray[0], WidthArray[1]);
        m_ObjectInfo[startnum + j].pos = offset;
        void* pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(),
                                      m_SortedOffset.GetSize(),
                                      sizeof(FX_FILESIZE), CompareFileSize);
        if (!pResult) {
          m_SortedOffset.Add(offset);
        }
        continue;
      }
      if (m_V5Type[startnum + j]) {
        continue;
      }
      m_V5Type[startnum + j] = type;
      if (type == 0) {
        m_ObjectInfo[startnum + j].pos = 0;
      } else {
        FX_FILESIZE offset =
            GetVarInt(entrystart + WidthArray[0], WidthArray[1]);
        m_ObjectInfo[startnum + j].pos = offset;
        if (type == 1) {
          void* pResult = FXSYS_bsearch(&offset, m_SortedOffset.GetData(),
                                        m_SortedOffset.GetSize(),
                                        sizeof(FX_FILESIZE), CompareFileSize);
          if (!pResult) {
            m_SortedOffset.Add(offset);
          }
        } else {
          if (offset < 0 || offset >= m_V5Type.GetSize()) {
            pStream->Release();
            return FALSE;
          }
          m_V5Type[offset] = 255;
        }
      }
    }
    segindex += count;
  }
  pStream->Release();
  return TRUE;
}
CPDF_Array* CPDF_Parser::GetIDArray() {
  CPDF_Object* pID = m_pTrailer ? m_pTrailer->GetElement("ID") : NULL;
  if (!pID)
    return nullptr;

  if (CPDF_Reference* pRef = pID->AsReference()) {
    pID = ParseIndirectObject(nullptr, pRef->GetRefObjNum());
    m_pTrailer->SetAt("ID", pID);
  }
  return ToArray(pID);
}
FX_DWORD CPDF_Parser::GetRootObjNum() {
  CPDF_Reference* pRef =
      ToReference(m_pTrailer ? m_pTrailer->GetElement("Root") : nullptr);
  return pRef ? pRef->GetRefObjNum() : 0;
}
FX_DWORD CPDF_Parser::GetInfoObjNum() {
  CPDF_Reference* pRef =
      ToReference(m_pTrailer ? m_pTrailer->GetElement("Info") : nullptr);
  return pRef ? pRef->GetRefObjNum() : 0;
}
FX_BOOL CPDF_Parser::IsFormStream(FX_DWORD objnum, FX_BOOL& bForm) {
  bForm = FALSE;
  if (!IsValidObjectNumber(objnum))
    return TRUE;
  if (m_V5Type[objnum] == 0)
    return TRUE;
  if (m_V5Type[objnum] == 2)
    return TRUE;
  FX_FILESIZE pos = m_ObjectInfo[objnum].pos;
  void* pResult =
      FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
                    sizeof(FX_FILESIZE), CompareFileSize);
  if (!pResult) {
    return TRUE;
  }
  if ((FX_FILESIZE*)pResult - (FX_FILESIZE*)m_SortedOffset.GetData() ==
      m_SortedOffset.GetSize() - 1) {
    return FALSE;
  }
  FX_FILESIZE size = ((FX_FILESIZE*)pResult)[1] - pos;
  FX_FILESIZE SavedPos = m_Syntax.SavePos();
  m_Syntax.RestorePos(pos);
  const char kFormStream[] = "/Form\0stream";
  const CFX_ByteStringC kFormStreamStr(kFormStream, sizeof(kFormStream) - 1);
  bForm = m_Syntax.SearchMultiWord(kFormStreamStr, TRUE, size) == 0;
  m_Syntax.RestorePos(SavedPos);
  return TRUE;
}

CPDF_Object* CPDF_Parser::ParseIndirectObject(
    CPDF_IndirectObjectHolder* pObjList,
    FX_DWORD objnum,
    PARSE_CONTEXT* pContext) {
  if (!IsValidObjectNumber(objnum))
    return nullptr;

  // Prevent circular parsing the same object.
  if (pdfium::ContainsKey(m_ParsingObjNums, objnum))
    return nullptr;
  ScopedSetInsertion<FX_DWORD> local_insert(&m_ParsingObjNums, objnum);

  if (m_V5Type[objnum] == 1 || m_V5Type[objnum] == 255) {
    FX_FILESIZE pos = m_ObjectInfo[objnum].pos;
    if (pos <= 0)
      return nullptr;
    return ParseIndirectObjectAt(pObjList, pos, objnum, pContext);
  }
  if (m_V5Type[objnum] != 2)
    return nullptr;

  CPDF_StreamAcc* pObjStream = GetObjectStream(m_ObjectInfo[objnum].pos);
  if (!pObjStream)
    return nullptr;

  ScopedFileStream file(FX_CreateMemoryStream(
      (uint8_t*)pObjStream->GetData(), (size_t)pObjStream->GetSize(), FALSE));
  CPDF_SyntaxParser syntax;
  syntax.InitParser(file.get(), 0);
  const int32_t offset = GetStreamFirst(pObjStream);

  // Read object numbers from |pObjStream| into a cache.
  if (!pdfium::ContainsKey(m_ObjCache, pObjStream)) {
    for (int32_t i = GetStreamNCount(pObjStream); i > 0; --i) {
      FX_DWORD thisnum = syntax.GetDirectNum();
      FX_DWORD thisoff = syntax.GetDirectNum();
      m_ObjCache[pObjStream][thisnum] = thisoff;
    }
  }

  const auto it = m_ObjCache[pObjStream].find(objnum);
  if (it == m_ObjCache[pObjStream].end())
    return nullptr;

  syntax.RestorePos(offset + it->second);
  return syntax.GetObject(pObjList, 0, 0, pContext, true);
}

CPDF_StreamAcc* CPDF_Parser::GetObjectStream(FX_DWORD objnum) {
  CPDF_StreamAcc* pStreamAcc = nullptr;
  if (m_ObjectStreamMap.Lookup((void*)(uintptr_t)objnum, (void*&)pStreamAcc))
    return pStreamAcc;

  const CPDF_Stream* pStream = ToStream(
      m_pDocument ? m_pDocument->GetIndirectObject(objnum, nullptr) : nullptr);
  if (!pStream)
    return nullptr;

  pStreamAcc = new CPDF_StreamAcc;
  pStreamAcc->LoadAllData(pStream);
  m_ObjectStreamMap.SetAt((void*)(uintptr_t)objnum, pStreamAcc);
  return pStreamAcc;
}

FX_FILESIZE CPDF_Parser::GetObjectSize(FX_DWORD objnum) const {
  if (!IsValidObjectNumber(objnum))
    return 0;

  if (m_V5Type[objnum] == 2)
    objnum = GetObjectPositionOrZero(objnum);

  if (m_V5Type[objnum] == 1 || m_V5Type[objnum] == 255) {
    FX_FILESIZE offset = GetObjectPositionOrZero(objnum);
    if (offset == 0)
      return 0;

    FX_FILESIZE* pResult = static_cast<FX_FILESIZE*>(FXSYS_bsearch(
        &offset, m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
        sizeof(FX_FILESIZE), CompareFileSize));
    if (!pResult)
      return 0;

    if (pResult - m_SortedOffset.GetData() == m_SortedOffset.GetSize() - 1)
      return 0;

    return pResult[1] - offset;
  }
  return 0;
}

void CPDF_Parser::GetIndirectBinary(FX_DWORD objnum,
                                    uint8_t*& pBuffer,
                                    FX_DWORD& size) {
  pBuffer = NULL;
  size = 0;
  if (!IsValidObjectNumber(objnum))
    return;

  if (m_V5Type[objnum] == 2) {
    CPDF_StreamAcc* pObjStream = GetObjectStream(m_ObjectInfo[objnum].pos);
    if (!pObjStream)
      return;

    int32_t offset = GetStreamFirst(pObjStream);
    const uint8_t* pData = pObjStream->GetData();
    FX_DWORD totalsize = pObjStream->GetSize();
    ScopedFileStream file(
        FX_CreateMemoryStream((uint8_t*)pData, (size_t)totalsize, FALSE));
    CPDF_SyntaxParser syntax;
    syntax.InitParser(file.get(), 0);
    for (int i = GetStreamNCount(pObjStream); i > 0; --i) {
      FX_DWORD thisnum = syntax.GetDirectNum();
      FX_DWORD thisoff = syntax.GetDirectNum();
      if (thisnum != objnum)
        continue;

      if (i == 1) {
        size = totalsize - (thisoff + offset);
      } else {
        syntax.GetDirectNum();  // Skip nextnum.
        FX_DWORD nextoff = syntax.GetDirectNum();
        size = nextoff - thisoff;
      }
      pBuffer = FX_Alloc(uint8_t, size);
      FXSYS_memcpy(pBuffer, pData + thisoff + offset, size);
      return;
    }
    return;
  }

  if (m_V5Type[objnum] != 1)
    return;

  FX_FILESIZE pos = m_ObjectInfo[objnum].pos;
  if (pos == 0) {
    return;
  }
  FX_FILESIZE SavedPos = m_Syntax.SavePos();
  m_Syntax.RestorePos(pos);
  bool bIsNumber;
  CFX_ByteString word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_Syntax.RestorePos(SavedPos);
    return;
  }
  FX_DWORD parser_objnum = FXSYS_atoi(word);
  if (parser_objnum && parser_objnum != objnum) {
    m_Syntax.RestorePos(SavedPos);
    return;
  }
  word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_Syntax.RestorePos(SavedPos);
    return;
  }
  if (m_Syntax.GetKeyword() != "obj") {
    m_Syntax.RestorePos(SavedPos);
    return;
  }
  void* pResult =
      FXSYS_bsearch(&pos, m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
                    sizeof(FX_FILESIZE), CompareFileSize);
  if (!pResult) {
    m_Syntax.RestorePos(SavedPos);
    return;
  }
  FX_FILESIZE nextoff = ((FX_FILESIZE*)pResult)[1];
  FX_BOOL bNextOffValid = FALSE;
  if (nextoff != pos) {
    m_Syntax.RestorePos(nextoff);
    word = m_Syntax.GetNextWord(&bIsNumber);
    if (word == "xref") {
      bNextOffValid = TRUE;
    } else if (bIsNumber) {
      word = m_Syntax.GetNextWord(&bIsNumber);
      if (bIsNumber && m_Syntax.GetKeyword() == "obj") {
        bNextOffValid = TRUE;
      }
    }
  }
  if (!bNextOffValid) {
    m_Syntax.RestorePos(pos);
    while (1) {
      if (m_Syntax.GetKeyword() == "endobj") {
        break;
      }
      if (m_Syntax.SavePos() == m_Syntax.m_FileLen) {
        break;
      }
    }
    nextoff = m_Syntax.SavePos();
  }
  size = (FX_DWORD)(nextoff - pos);
  pBuffer = FX_Alloc(uint8_t, size);
  m_Syntax.RestorePos(pos);
  m_Syntax.ReadBlock(pBuffer, size);
  m_Syntax.RestorePos(SavedPos);
}

CPDF_Object* CPDF_Parser::ParseIndirectObjectAt(
    CPDF_IndirectObjectHolder* pObjList,
    FX_FILESIZE pos,
    FX_DWORD objnum,
    PARSE_CONTEXT* pContext) {
  FX_FILESIZE SavedPos = m_Syntax.SavePos();
  m_Syntax.RestorePos(pos);
  bool bIsNumber;
  CFX_ByteString word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  FX_FILESIZE objOffset = m_Syntax.SavePos();
  objOffset -= word.GetLength();
  FX_DWORD parser_objnum = FXSYS_atoi(word);
  if (objnum && parser_objnum != objnum) {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  FX_DWORD parser_gennum = FXSYS_atoi(word);
  if (m_Syntax.GetKeyword() != "obj") {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  CPDF_Object* pObj =
      m_Syntax.GetObject(pObjList, objnum, parser_gennum, pContext, true);
  m_Syntax.SavePos();
  CFX_ByteString bsWord = m_Syntax.GetKeyword();
  if (bsWord == "endobj") {
    m_Syntax.SavePos();
  }
  m_Syntax.RestorePos(SavedPos);
  if (pObj) {
    if (!objnum)
      pObj->m_ObjNum = parser_objnum;
    pObj->m_GenNum = parser_gennum;
  }
  return pObj;
}
CPDF_Object* CPDF_Parser::ParseIndirectObjectAtByStrict(
    CPDF_IndirectObjectHolder* pObjList,
    FX_FILESIZE pos,
    FX_DWORD objnum,
    PARSE_CONTEXT* pContext,
    FX_FILESIZE* pResultPos) {
  FX_FILESIZE SavedPos = m_Syntax.SavePos();
  m_Syntax.RestorePos(pos);
  bool bIsNumber;
  CFX_ByteString word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  FX_DWORD parser_objnum = FXSYS_atoi(word);
  if (objnum && parser_objnum != objnum) {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  FX_DWORD gennum = FXSYS_atoi(word);
  if (m_Syntax.GetKeyword() != "obj") {
    m_Syntax.RestorePos(SavedPos);
    return NULL;
  }
  CPDF_Object* pObj =
      m_Syntax.GetObjectByStrict(pObjList, objnum, gennum, pContext);
  if (pResultPos) {
    *pResultPos = m_Syntax.m_Pos;
  }
  m_Syntax.RestorePos(SavedPos);
  return pObj;
}

CPDF_Dictionary* CPDF_Parser::LoadTrailerV4() {
  if (m_Syntax.GetKeyword() != "trailer")
    return nullptr;

  std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>> pObj(
      m_Syntax.GetObject(m_pDocument, 0, 0, nullptr, true));
  if (!ToDictionary(pObj.get()))
    return nullptr;
  return pObj.release()->AsDictionary();
}

FX_DWORD CPDF_Parser::GetPermissions(FX_BOOL bCheckRevision) {
  if (!m_pSecurityHandler) {
    return (FX_DWORD)-1;
  }
  FX_DWORD dwPermission = m_pSecurityHandler->GetPermissions();
  if (m_pEncryptDict && m_pEncryptDict->GetString("Filter") == "Standard") {
    dwPermission &= 0xFFFFFFFC;
    dwPermission |= 0xFFFFF0C0;
    if (bCheckRevision && m_pEncryptDict->GetInteger("R") == 2) {
      dwPermission &= 0xFFFFF0FF;
    }
  }
  return dwPermission;
}
FX_BOOL CPDF_Parser::IsOwner() {
  return !m_pSecurityHandler || m_pSecurityHandler->IsOwner();
}
void CPDF_Parser::SetSecurityHandler(CPDF_SecurityHandler* pSecurityHandler,
                                     FX_BOOL bForced) {
  m_bForceUseSecurityHandler = bForced;
  m_pSecurityHandler.reset(pSecurityHandler);
  if (m_bForceUseSecurityHandler) {
    return;
  }
  m_Syntax.m_pCryptoHandler.reset(pSecurityHandler->CreateCryptoHandler());
  m_Syntax.m_pCryptoHandler->Init(NULL, pSecurityHandler);
}
FX_BOOL CPDF_Parser::IsLinearizedFile(IFX_FileRead* pFileAccess,
                                      FX_DWORD offset) {
  m_Syntax.InitParser(pFileAccess, offset);
  m_Syntax.RestorePos(m_Syntax.m_HeaderOffset + 9);
  FX_FILESIZE SavedPos = m_Syntax.SavePos();
  bool bIsNumber;
  CFX_ByteString word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    return FALSE;
  }
  FX_DWORD objnum = FXSYS_atoi(word);
  word = m_Syntax.GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    return FALSE;
  }
  FX_DWORD gennum = FXSYS_atoi(word);
  if (m_Syntax.GetKeyword() != "obj") {
    m_Syntax.RestorePos(SavedPos);
    return FALSE;
  }
  m_pLinearized = m_Syntax.GetObject(nullptr, objnum, gennum, nullptr, true);
  if (!m_pLinearized) {
    return FALSE;
  }

  CPDF_Dictionary* pDict = m_pLinearized->GetDict();
  if (pDict && pDict->GetElement("Linearized")) {
    m_Syntax.GetNextWord(nullptr);

    CPDF_Object* pLen = pDict->GetElement("L");
    if (!pLen) {
      m_pLinearized->Release();
      m_pLinearized = NULL;
      return FALSE;
    }
    if (pLen->GetInteger() != (int)pFileAccess->GetSize()) {
      return FALSE;
    }

    if (CPDF_Number* pNo = ToNumber(pDict->GetElement("P")))
      m_dwFirstPageNo = pNo->GetInteger();

    if (CPDF_Number* pTable = ToNumber(pDict->GetElement("T")))
      m_LastXRefOffset = pTable->GetInteger();

    return TRUE;
  }
  m_pLinearized->Release();
  m_pLinearized = NULL;
  return FALSE;
}
FX_DWORD CPDF_Parser::StartAsynParse(IFX_FileRead* pFileAccess,
                                     FX_BOOL bReParse,
                                     FX_BOOL bOwnFileRead) {
  CloseParser(bReParse);
  m_bXRefStream = FALSE;
  m_LastXRefOffset = 0;
  m_bOwnFileRead = bOwnFileRead;
  int32_t offset = GetHeaderOffset(pFileAccess);
  if (offset == -1) {
    return PDFPARSE_ERROR_FORMAT;
  }
  if (!IsLinearizedFile(pFileAccess, offset)) {
    m_Syntax.m_pFileAccess = NULL;
    return StartParse(pFileAccess, bReParse, bOwnFileRead);
  }
  if (!bReParse) {
    m_pDocument = new CPDF_Document(this);
  }
  FX_FILESIZE dwFirstXRefOffset = m_Syntax.SavePos();
  FX_BOOL bXRefRebuilt = FALSE;
  FX_BOOL bLoadV4 = FALSE;
  if (!(bLoadV4 = LoadCrossRefV4(dwFirstXRefOffset, 0, FALSE)) &&
      !LoadCrossRefV5(&dwFirstXRefOffset, TRUE)) {
    if (!RebuildCrossRef()) {
      return PDFPARSE_ERROR_FORMAT;
    }
    bXRefRebuilt = TRUE;
    m_LastXRefOffset = 0;
  }
  if (bLoadV4) {
    m_pTrailer = LoadTrailerV4();
    if (!m_pTrailer) {
      return PDFPARSE_ERROR_SUCCESS;
    }

    int32_t xrefsize = GetDirectInteger(m_pTrailer, "Size");
    if (xrefsize > 0) {
      ShrinkObjectMap(xrefsize);
      m_V5Type.SetSize(xrefsize);
    }
  }
  FX_DWORD dwRet = SetEncryptHandler();
  if (dwRet != PDFPARSE_ERROR_SUCCESS) {
    return dwRet;
  }
  m_pDocument->LoadAsynDoc(m_pLinearized->GetDict());
  if (!m_pDocument->GetRoot() || m_pDocument->GetPageCount() == 0) {
    if (bXRefRebuilt) {
      return PDFPARSE_ERROR_FORMAT;
    }
    ReleaseEncryptHandler();
    if (!RebuildCrossRef()) {
      return PDFPARSE_ERROR_FORMAT;
    }
    dwRet = SetEncryptHandler();
    if (dwRet != PDFPARSE_ERROR_SUCCESS) {
      return dwRet;
    }
    m_pDocument->LoadAsynDoc(m_pLinearized->GetDict());
    if (!m_pDocument->GetRoot()) {
      return PDFPARSE_ERROR_FORMAT;
    }
  }
  FXSYS_qsort(m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
              sizeof(FX_FILESIZE), CompareFileSize);
  if (GetRootObjNum() == 0) {
    ReleaseEncryptHandler();
    if (!RebuildCrossRef() || GetRootObjNum() == 0)
      return PDFPARSE_ERROR_FORMAT;

    dwRet = SetEncryptHandler();
    if (dwRet != PDFPARSE_ERROR_SUCCESS) {
      return dwRet;
    }
  }
  if (m_pSecurityHandler && m_pSecurityHandler->IsMetadataEncrypted()) {
    if (CPDF_Reference* pMetadata =
            ToReference(m_pDocument->GetRoot()->GetElement("Metadata")))
      m_Syntax.m_MetadataObjnum = pMetadata->GetRefObjNum();
  }
  return PDFPARSE_ERROR_SUCCESS;
}
FX_BOOL CPDF_Parser::LoadLinearizedAllCrossRefV5(FX_FILESIZE xrefpos) {
  if (!LoadCrossRefV5(&xrefpos, FALSE)) {
    return FALSE;
  }
  std::set<FX_FILESIZE> seen_xrefpos;
  while (xrefpos) {
    seen_xrefpos.insert(xrefpos);
    if (!LoadCrossRefV5(&xrefpos, FALSE)) {
      return FALSE;
    }
    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos)) {
      return FALSE;
    }
  }
  m_ObjectStreamMap.InitHashTable(101, FALSE);
  m_bXRefStream = TRUE;
  return TRUE;
}
FX_DWORD CPDF_Parser::LoadLinearizedMainXRefTable() {
  FX_DWORD dwSaveMetadataObjnum = m_Syntax.m_MetadataObjnum;
  m_Syntax.m_MetadataObjnum = 0;
  if (m_pTrailer) {
    m_pTrailer->Release();
    m_pTrailer = NULL;
  }
  m_Syntax.RestorePos(m_LastXRefOffset - m_Syntax.m_HeaderOffset);
  uint8_t ch = 0;
  FX_DWORD dwCount = 0;
  m_Syntax.GetNextChar(ch);
  while (PDFCharIsWhitespace(ch)) {
    ++dwCount;
    if (m_Syntax.m_FileLen >=
        (FX_FILESIZE)(m_Syntax.SavePos() + m_Syntax.m_HeaderOffset)) {
      break;
    }
    m_Syntax.GetNextChar(ch);
  }
  m_LastXRefOffset += dwCount;
  FX_POSITION pos = m_ObjectStreamMap.GetStartPosition();
  while (pos) {
    void* objnum;
    CPDF_StreamAcc* pStream;
    m_ObjectStreamMap.GetNextAssoc(pos, objnum, (void*&)pStream);
    delete pStream;
  }
  m_ObjectStreamMap.RemoveAll();
  m_ObjCache.clear();

  if (!LoadLinearizedAllCrossRefV4(m_LastXRefOffset, m_dwXrefStartObjNum) &&
      !LoadLinearizedAllCrossRefV5(m_LastXRefOffset)) {
    m_LastXRefOffset = 0;
    m_Syntax.m_MetadataObjnum = dwSaveMetadataObjnum;
    return PDFPARSE_ERROR_FORMAT;
  }
  FXSYS_qsort(m_SortedOffset.GetData(), m_SortedOffset.GetSize(),
              sizeof(FX_FILESIZE), CompareFileSize);
  m_Syntax.m_MetadataObjnum = dwSaveMetadataObjnum;
  return PDFPARSE_ERROR_SUCCESS;
}

// static
int CPDF_SyntaxParser::s_CurrentRecursionDepth = 0;

CPDF_SyntaxParser::CPDF_SyntaxParser() {
  m_pFileAccess = NULL;
  m_pFileBuf = NULL;
  m_BufSize = CPDF_ModuleMgr::kFileBufSize;
  m_pFileBuf = NULL;
  m_MetadataObjnum = 0;
  m_dwWordPos = 0;
  m_bFileStream = FALSE;
}

CPDF_SyntaxParser::~CPDF_SyntaxParser() {
  FX_Free(m_pFileBuf);
}

FX_BOOL CPDF_SyntaxParser::GetCharAt(FX_FILESIZE pos, uint8_t& ch) {
  CFX_AutoRestorer<FX_FILESIZE> save_pos(&m_Pos);
  m_Pos = pos;
  return GetNextChar(ch);
}

FX_BOOL CPDF_SyntaxParser::GetNextChar(uint8_t& ch) {
  FX_FILESIZE pos = m_Pos + m_HeaderOffset;
  if (pos >= m_FileLen) {
    return FALSE;
  }
  if (m_BufOffset >= pos || (FX_FILESIZE)(m_BufOffset + m_BufSize) <= pos) {
    FX_FILESIZE read_pos = pos;
    FX_DWORD read_size = m_BufSize;
    if ((FX_FILESIZE)read_size > m_FileLen) {
      read_size = (FX_DWORD)m_FileLen;
    }
    if ((FX_FILESIZE)(read_pos + read_size) > m_FileLen) {
      if (m_FileLen < (FX_FILESIZE)read_size) {
        read_pos = 0;
        read_size = (FX_DWORD)m_FileLen;
      } else {
        read_pos = m_FileLen - read_size;
      }
    }
    if (!m_pFileAccess->ReadBlock(m_pFileBuf, read_pos, read_size)) {
      return FALSE;
    }
    m_BufOffset = read_pos;
  }
  ch = m_pFileBuf[pos - m_BufOffset];
  m_Pos++;
  return TRUE;
}
FX_BOOL CPDF_SyntaxParser::GetCharAtBackward(FX_FILESIZE pos, uint8_t& ch) {
  pos += m_HeaderOffset;
  if (pos >= m_FileLen) {
    return FALSE;
  }
  if (m_BufOffset >= pos || (FX_FILESIZE)(m_BufOffset + m_BufSize) <= pos) {
    FX_FILESIZE read_pos;
    if (pos < (FX_FILESIZE)m_BufSize) {
      read_pos = 0;
    } else {
      read_pos = pos - m_BufSize + 1;
    }
    FX_DWORD read_size = m_BufSize;
    if ((FX_FILESIZE)(read_pos + read_size) > m_FileLen) {
      if (m_FileLen < (FX_FILESIZE)read_size) {
        read_pos = 0;
        read_size = (FX_DWORD)m_FileLen;
      } else {
        read_pos = m_FileLen - read_size;
      }
    }
    if (!m_pFileAccess->ReadBlock(m_pFileBuf, read_pos, read_size)) {
      return FALSE;
    }
    m_BufOffset = read_pos;
  }
  ch = m_pFileBuf[pos - m_BufOffset];
  return TRUE;
}
FX_BOOL CPDF_SyntaxParser::ReadBlock(uint8_t* pBuf, FX_DWORD size) {
  if (!m_pFileAccess->ReadBlock(pBuf, m_Pos + m_HeaderOffset, size)) {
    return FALSE;
  }
  m_Pos += size;
  return TRUE;
}

void CPDF_SyntaxParser::GetNextWordInternal(bool* bIsNumber) {
  m_WordSize = 0;
  if (bIsNumber)
    *bIsNumber = true;
  uint8_t ch;
  if (!GetNextChar(ch)) {
    return;
  }
  while (1) {
    while (PDFCharIsWhitespace(ch)) {
      if (!GetNextChar(ch))
        return;
    }
    if (ch != '%')
      break;

    while (1) {
      if (!GetNextChar(ch))
        return;
      if (PDFCharIsLineEnding(ch))
        break;
    }
  }

  if (PDFCharIsDelimiter(ch)) {
    if (bIsNumber)
      *bIsNumber = false;
    m_WordBuffer[m_WordSize++] = ch;
    if (ch == '/') {
      while (1) {
        if (!GetNextChar(ch))
          return;

        if (!PDFCharIsOther(ch) && !PDFCharIsNumeric(ch)) {
          m_Pos--;
          return;
        }

        if (m_WordSize < sizeof(m_WordBuffer) - 1)
          m_WordBuffer[m_WordSize++] = ch;
      }
    } else if (ch == '<') {
      if (!GetNextChar(ch))
        return;
      if (ch == '<')
        m_WordBuffer[m_WordSize++] = ch;
      else
        m_Pos--;
    } else if (ch == '>') {
      if (!GetNextChar(ch))
        return;
      if (ch == '>')
        m_WordBuffer[m_WordSize++] = ch;
      else
        m_Pos--;
    }
    return;
  }

  while (1) {
    if (m_WordSize < sizeof(m_WordBuffer) - 1)
      m_WordBuffer[m_WordSize++] = ch;

    if (!PDFCharIsNumeric(ch))
      if (bIsNumber)
        *bIsNumber = false;
    if (!GetNextChar(ch))
      return;

    if (PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch)) {
      m_Pos--;
      break;
    }
  }
}

CFX_ByteString CPDF_SyntaxParser::ReadString() {
  uint8_t ch;
  if (!GetNextChar(ch)) {
    return CFX_ByteString();
  }
  CFX_ByteTextBuf buf;
  int32_t parlevel = 0;
  int32_t status = 0, iEscCode = 0;
  while (1) {
    switch (status) {
      case 0:
        if (ch == ')') {
          if (parlevel == 0) {
            return buf.GetByteString();
          }
          parlevel--;
          buf.AppendChar(')');
        } else if (ch == '(') {
          parlevel++;
          buf.AppendChar('(');
        } else if (ch == '\\') {
          status = 1;
        } else {
          buf.AppendChar(ch);
        }
        break;
      case 1:
        if (ch >= '0' && ch <= '7') {
          iEscCode = FXSYS_toDecimalDigit(ch);
          status = 2;
          break;
        }
        if (ch == 'n') {
          buf.AppendChar('\n');
        } else if (ch == 'r') {
          buf.AppendChar('\r');
        } else if (ch == 't') {
          buf.AppendChar('\t');
        } else if (ch == 'b') {
          buf.AppendChar('\b');
        } else if (ch == 'f') {
          buf.AppendChar('\f');
        } else if (ch == '\r') {
          status = 4;
          break;
        } else if (ch == '\n') {
        } else {
          buf.AppendChar(ch);
        }
        status = 0;
        break;
      case 2:
        if (ch >= '0' && ch <= '7') {
          iEscCode = iEscCode * 8 + FXSYS_toDecimalDigit(ch);
          status = 3;
        } else {
          buf.AppendChar(iEscCode);
          status = 0;
          continue;
        }
        break;
      case 3:
        if (ch >= '0' && ch <= '7') {
          iEscCode = iEscCode * 8 + FXSYS_toDecimalDigit(ch);
          buf.AppendChar(iEscCode);
          status = 0;
        } else {
          buf.AppendChar(iEscCode);
          status = 0;
          continue;
        }
        break;
      case 4:
        status = 0;
        if (ch != '\n') {
          continue;
        }
        break;
    }
    if (!GetNextChar(ch)) {
      break;
    }
  }
  GetNextChar(ch);
  return buf.GetByteString();
}
CFX_ByteString CPDF_SyntaxParser::ReadHexString() {
  uint8_t ch;
  if (!GetNextChar(ch))
    return CFX_ByteString();

  CFX_BinaryBuf buf;
  bool bFirst = true;
  uint8_t code = 0;
  while (1) {
    if (ch == '>')
      break;

    if (std::isxdigit(ch)) {
      int val = FXSYS_toHexDigit(ch);
      if (bFirst) {
        code = val * 16;
      } else {
        code += val;
        buf.AppendByte((uint8_t)code);
      }
      bFirst = !bFirst;
    }

    if (!GetNextChar(ch))
      break;
  }
  if (!bFirst)
    buf.AppendByte((uint8_t)code);

  return buf.GetByteString();
}
void CPDF_SyntaxParser::ToNextLine() {
  uint8_t ch;
  while (GetNextChar(ch)) {
    if (ch == '\n') {
      break;
    }
    if (ch == '\r') {
      GetNextChar(ch);
      if (ch != '\n') {
        --m_Pos;
      }
      break;
    }
  }
}
void CPDF_SyntaxParser::ToNextWord() {
  uint8_t ch;
  if (!GetNextChar(ch))
    return;

  while (1) {
    while (PDFCharIsWhitespace(ch)) {
      m_dwWordPos = m_Pos;
      if (!GetNextChar(ch))
        return;
    }

    if (ch != '%')
      break;

    while (1) {
      if (!GetNextChar(ch))
        return;
      if (PDFCharIsLineEnding(ch))
        break;
    }
  }
  m_Pos--;
}

CFX_ByteString CPDF_SyntaxParser::GetNextWord(bool* bIsNumber) {
  GetNextWordInternal(bIsNumber);
  return CFX_ByteString((const FX_CHAR*)m_WordBuffer, m_WordSize);
}

CFX_ByteString CPDF_SyntaxParser::GetKeyword() {
  return GetNextWord(nullptr);
}

CPDF_Object* CPDF_SyntaxParser::GetObject(CPDF_IndirectObjectHolder* pObjList,
                                          FX_DWORD objnum,
                                          FX_DWORD gennum,
                                          PARSE_CONTEXT* pContext,
                                          FX_BOOL bDecrypt) {
  CFX_AutoRestorer<int> restorer(&s_CurrentRecursionDepth);
  if (++s_CurrentRecursionDepth > kParserMaxRecursionDepth) {
    return NULL;
  }
  FX_FILESIZE SavedPos = m_Pos;
  FX_BOOL bTypeOnly = pContext && (pContext->m_Flags & PDFPARSE_TYPEONLY);
  bool bIsNumber;
  CFX_ByteString word = GetNextWord(&bIsNumber);
  if (word.GetLength() == 0) {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_INVALID;
    return NULL;
  }
  if (bIsNumber) {
    FX_FILESIZE SavedPos = m_Pos;
    CFX_ByteString nextword = GetNextWord(&bIsNumber);
    if (bIsNumber) {
      CFX_ByteString nextword2 = GetNextWord(nullptr);
      if (nextword2 == "R") {
        FX_DWORD objnum = FXSYS_atoi(word);
        if (bTypeOnly)
          return (CPDF_Object*)PDFOBJ_REFERENCE;
        return new CPDF_Reference(pObjList, objnum);
      }
    }
    m_Pos = SavedPos;
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_NUMBER;
    return new CPDF_Number(word);
  }
  if (word == "true" || word == "false") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_BOOLEAN;
    return new CPDF_Boolean(word == "true");
  }
  if (word == "null") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_NULL;
    return new CPDF_Null;
  }
  if (word == "(") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_STRING;
    CFX_ByteString str = ReadString();
    if (m_pCryptoHandler && bDecrypt) {
      m_pCryptoHandler->Decrypt(objnum, gennum, str);
    }
    return new CPDF_String(str, FALSE);
  }
  if (word == "<") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_STRING;
    CFX_ByteString str = ReadHexString();
    if (m_pCryptoHandler && bDecrypt) {
      m_pCryptoHandler->Decrypt(objnum, gennum, str);
    }
    return new CPDF_String(str, TRUE);
  }
  if (word == "[") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_ARRAY;
    CPDF_Array* pArray = new CPDF_Array;
    while (CPDF_Object* pObj =
               GetObject(pObjList, objnum, gennum, nullptr, true)) {
      pArray->Add(pObj);
    }
    return pArray;
  }
  if (word[0] == '/') {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_NAME;
    return new CPDF_Name(
        PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1)));
  }
  if (word == "<<") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_DICTIONARY;

    if (pContext)
      pContext->m_DictStart = SavedPos;

    int32_t nKeys = 0;
    FX_FILESIZE dwSignValuePos = 0;
    std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>> pDict(
        new CPDF_Dictionary);
    while (1) {
      CFX_ByteString key = GetNextWord(nullptr);
      if (key.IsEmpty())
        return nullptr;

      FX_FILESIZE SavedPos = m_Pos - key.GetLength();
      if (key == ">>")
        break;

      if (key == "endobj") {
        m_Pos = SavedPos;
        break;
      }
      if (key[0] != '/')
        continue;

      ++nKeys;
      key = PDF_NameDecode(key);
      if (key.IsEmpty())
        continue;

      if (key == "/Contents")
        dwSignValuePos = m_Pos;

      CPDF_Object* pObj = GetObject(pObjList, objnum, gennum, nullptr, true);
      if (!pObj)
        continue;

      CFX_ByteStringC keyNoSlash(key.c_str() + 1, key.GetLength() - 1);
      pDict->SetAt(keyNoSlash, pObj);
    }

    // Only when this is a signature dictionary and has contents, we reset the
    // contents to the un-decrypted form.
    if (IsSignatureDict(pDict.get()) && dwSignValuePos) {
      CFX_AutoRestorer<FX_FILESIZE> save_pos(&m_Pos);
      m_Pos = dwSignValuePos;
      CPDF_Object* pObj = GetObject(pObjList, objnum, gennum, nullptr, FALSE);
      pDict->SetAt("Contents", pObj);
    }
    if (pContext) {
      pContext->m_DictEnd = m_Pos;
      if (pContext->m_Flags & PDFPARSE_NOSTREAM) {
        return pDict.release();
      }
    }
    FX_FILESIZE SavedPos = m_Pos;
    CFX_ByteString nextword = GetNextWord(nullptr);
    if (nextword != "stream") {
      m_Pos = SavedPos;
      return pDict.release();
    }

    return ReadStream(pDict.release(), pContext, objnum, gennum);
  }
  if (word == ">>") {
    m_Pos = SavedPos;
    return nullptr;
  }
  if (bTypeOnly)
    return (CPDF_Object*)PDFOBJ_INVALID;

  return nullptr;
}

CPDF_Object* CPDF_SyntaxParser::GetObjectByStrict(
    CPDF_IndirectObjectHolder* pObjList,
    FX_DWORD objnum,
    FX_DWORD gennum,
    PARSE_CONTEXT* pContext) {
  CFX_AutoRestorer<int> restorer(&s_CurrentRecursionDepth);
  if (++s_CurrentRecursionDepth > kParserMaxRecursionDepth) {
    return NULL;
  }
  FX_FILESIZE SavedPos = m_Pos;
  FX_BOOL bTypeOnly = pContext && (pContext->m_Flags & PDFPARSE_TYPEONLY);
  bool bIsNumber;
  CFX_ByteString word = GetNextWord(&bIsNumber);
  if (word.GetLength() == 0) {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_INVALID;
    return nullptr;
  }
  if (bIsNumber) {
    FX_FILESIZE SavedPos = m_Pos;
    CFX_ByteString nextword = GetNextWord(&bIsNumber);
    if (bIsNumber) {
      CFX_ByteString nextword2 = GetNextWord(nullptr);
      if (nextword2 == "R") {
        if (bTypeOnly)
          return (CPDF_Object*)PDFOBJ_REFERENCE;
        FX_DWORD objnum = FXSYS_atoi(word);
        return new CPDF_Reference(pObjList, objnum);
      }
    }
    m_Pos = SavedPos;
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_NUMBER;
    return new CPDF_Number(word);
  }
  if (word == "true" || word == "false") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_BOOLEAN;
    return new CPDF_Boolean(word == "true");
  }
  if (word == "null") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_NULL;
    return new CPDF_Null;
  }
  if (word == "(") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_STRING;
    CFX_ByteString str = ReadString();
    if (m_pCryptoHandler)
      m_pCryptoHandler->Decrypt(objnum, gennum, str);
    return new CPDF_String(str, FALSE);
  }
  if (word == "<") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_STRING;
    CFX_ByteString str = ReadHexString();
    if (m_pCryptoHandler)
      m_pCryptoHandler->Decrypt(objnum, gennum, str);
    return new CPDF_String(str, TRUE);
  }
  if (word == "[") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_ARRAY;
    std::unique_ptr<CPDF_Array, ReleaseDeleter<CPDF_Array>> pArray(
        new CPDF_Array);
    while (CPDF_Object* pObj =
               GetObject(pObjList, objnum, gennum, nullptr, true)) {
      pArray->Add(pObj);
    }
    return m_WordBuffer[0] == ']' ? pArray.release() : nullptr;
  }
  if (word[0] == '/') {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_NAME;
    return new CPDF_Name(
        PDF_NameDecode(CFX_ByteStringC(m_WordBuffer + 1, m_WordSize - 1)));
  }
  if (word == "<<") {
    if (bTypeOnly)
      return (CPDF_Object*)PDFOBJ_DICTIONARY;
    if (pContext)
      pContext->m_DictStart = SavedPos;

    std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>> pDict(
        new CPDF_Dictionary);
    while (1) {
      FX_FILESIZE SavedPos = m_Pos;
      CFX_ByteString key = GetNextWord(nullptr);
      if (key.IsEmpty())
        return nullptr;

      if (key == ">>")
        break;

      if (key == "endobj") {
        m_Pos = SavedPos;
        break;
      }
      if (key[0] != '/')
        continue;

      key = PDF_NameDecode(key);
      std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>> obj(
          GetObject(pObjList, objnum, gennum, nullptr, true));
      if (!obj) {
        uint8_t ch;
        while (GetNextChar(ch) && ch != 0x0A && ch != 0x0D) {
        }
        return nullptr;
      }
      if (key.GetLength() > 1) {
        pDict->SetAt(CFX_ByteStringC(key.c_str() + 1, key.GetLength() - 1),
                     obj.release());
      }
    }
    if (pContext) {
      pContext->m_DictEnd = m_Pos;
      if (pContext->m_Flags & PDFPARSE_NOSTREAM) {
        return pDict.release();
      }
    }
    FX_FILESIZE SavedPos = m_Pos;
    CFX_ByteString nextword = GetNextWord(nullptr);
    if (nextword != "stream") {
      m_Pos = SavedPos;
      return pDict.release();
    }

    return ReadStream(pDict.release(), pContext, objnum, gennum);
  }
  if (word == ">>") {
    m_Pos = SavedPos;
    return nullptr;
  }
  if (bTypeOnly)
    return (CPDF_Object*)PDFOBJ_INVALID;
  return nullptr;
}

unsigned int CPDF_SyntaxParser::ReadEOLMarkers(FX_FILESIZE pos) {
  unsigned char byte1 = 0;
  unsigned char byte2 = 0;
  GetCharAt(pos, byte1);
  GetCharAt(pos + 1, byte2);
  unsigned int markers = 0;
  if (byte1 == '\r' && byte2 == '\n') {
    markers = 2;
  } else if (byte1 == '\r' || byte1 == '\n') {
    markers = 1;
  }
  return markers;
}
CPDF_Stream* CPDF_SyntaxParser::ReadStream(CPDF_Dictionary* pDict,
                                           PARSE_CONTEXT* pContext,
                                           FX_DWORD objnum,
                                           FX_DWORD gennum) {
  CPDF_Object* pLenObj = pDict->GetElement("Length");
  FX_FILESIZE len = -1;
  CPDF_Reference* pLenObjRef = ToReference(pLenObj);

  bool differingObjNum = !pLenObjRef || (pLenObjRef->GetObjList() &&
                                         pLenObjRef->GetRefObjNum() != objnum);
  if (pLenObj && differingObjNum)
    len = pLenObj->GetInteger();

  // Locate the start of stream.
  ToNextLine();
  FX_FILESIZE streamStartPos = m_Pos;
  if (pContext) {
    pContext->m_DataStart = streamStartPos;
  }

  const CFX_ByteStringC kEndStreamStr("endstream");
  const CFX_ByteStringC kEndObjStr("endobj");
  CPDF_CryptoHandler* pCryptoHandler =
      objnum == (FX_DWORD)m_MetadataObjnum ? nullptr : m_pCryptoHandler.get();
  if (!pCryptoHandler) {
    FX_BOOL bSearchForKeyword = TRUE;
    if (len >= 0) {
      pdfium::base::CheckedNumeric<FX_FILESIZE> pos = m_Pos;
      pos += len;
      if (pos.IsValid() && pos.ValueOrDie() < m_FileLen) {
        m_Pos = pos.ValueOrDie();
      }
      m_Pos += ReadEOLMarkers(m_Pos);
      FXSYS_memset(m_WordBuffer, 0, kEndStreamStr.GetLength() + 1);
      GetNextWordInternal(nullptr);
      // Earlier version of PDF specification doesn't require EOL marker before
      // 'endstream' keyword. If keyword 'endstream' follows the bytes in
      // specified length, it signals the end of stream.
      if (FXSYS_memcmp(m_WordBuffer, kEndStreamStr.GetPtr(),
                       kEndStreamStr.GetLength()) == 0) {
        bSearchForKeyword = FALSE;
      }
    }
    if (bSearchForKeyword) {
      // If len is not available, len needs to be calculated
      // by searching the keywords "endstream" or "endobj".
      m_Pos = streamStartPos;
      FX_FILESIZE endStreamOffset = 0;
      while (endStreamOffset >= 0) {
        endStreamOffset = FindTag(kEndStreamStr, 0);
        if (endStreamOffset < 0) {
          // Can't find any "endstream".
          break;
        }
        if (IsWholeWord(m_Pos - kEndStreamStr.GetLength(), m_FileLen,
                        kEndStreamStr, TRUE)) {
          // Stop searching when the keyword "endstream" is found.
          endStreamOffset = m_Pos - streamStartPos - kEndStreamStr.GetLength();
          break;
        }
      }
      m_Pos = streamStartPos;
      FX_FILESIZE endObjOffset = 0;
      while (endObjOffset >= 0) {
        endObjOffset = FindTag(kEndObjStr, 0);
        if (endObjOffset < 0) {
          // Can't find any "endobj".
          break;
        }
        if (IsWholeWord(m_Pos - kEndObjStr.GetLength(), m_FileLen, kEndObjStr,
                        TRUE)) {
          // Stop searching when the keyword "endobj" is found.
          endObjOffset = m_Pos - streamStartPos - kEndObjStr.GetLength();
          break;
        }
      }
      if (endStreamOffset < 0 && endObjOffset < 0) {
        // Can't find "endstream" or "endobj".
        pDict->Release();
        return nullptr;
      }
      if (endStreamOffset < 0 && endObjOffset >= 0) {
        // Correct the position of end stream.
        endStreamOffset = endObjOffset;
      } else if (endStreamOffset >= 0 && endObjOffset < 0) {
        // Correct the position of end obj.
        endObjOffset = endStreamOffset;
      } else if (endStreamOffset > endObjOffset) {
        endStreamOffset = endObjOffset;
      }
      len = endStreamOffset;
      int numMarkers = ReadEOLMarkers(streamStartPos + endStreamOffset - 2);
      if (numMarkers == 2) {
        len -= 2;
      } else {
        numMarkers = ReadEOLMarkers(streamStartPos + endStreamOffset - 1);
        if (numMarkers == 1) {
          len -= 1;
        }
      }
      if (len < 0) {
        pDict->Release();
        return nullptr;
      }
      pDict->SetAtInteger("Length", len);
    }
    m_Pos = streamStartPos;
  }
  if (len < 0) {
    pDict->Release();
    return nullptr;
  }
  uint8_t* pData = nullptr;
  if (len > 0) {
    pData = FX_Alloc(uint8_t, len);
    ReadBlock(pData, len);
    if (pCryptoHandler) {
      CFX_BinaryBuf dest_buf;
      dest_buf.EstimateSize(pCryptoHandler->DecryptGetSize(len));
      void* context = pCryptoHandler->DecryptStart(objnum, gennum);
      pCryptoHandler->DecryptStream(context, pData, len, dest_buf);
      pCryptoHandler->DecryptFinish(context, dest_buf);
      FX_Free(pData);
      pData = dest_buf.GetBuffer();
      len = dest_buf.GetSize();
      dest_buf.DetachBuffer();
    }
  }
  CPDF_Stream* pStream = new CPDF_Stream(pData, len, pDict);
  if (pContext) {
    pContext->m_DataEnd = pContext->m_DataStart + len;
  }
  streamStartPos = m_Pos;
  FXSYS_memset(m_WordBuffer, 0, kEndObjStr.GetLength() + 1);
  GetNextWordInternal(nullptr);
  int numMarkers = ReadEOLMarkers(m_Pos);
  if (m_WordSize == kEndObjStr.GetLength() && numMarkers != 0 &&
      FXSYS_memcmp(m_WordBuffer, kEndObjStr.GetPtr(), kEndObjStr.GetLength()) ==
          0) {
    m_Pos = streamStartPos;
  }
  return pStream;
}
void CPDF_SyntaxParser::InitParser(IFX_FileRead* pFileAccess,
                                   FX_DWORD HeaderOffset) {
  FX_Free(m_pFileBuf);
  m_pFileBuf = FX_Alloc(uint8_t, m_BufSize);
  m_HeaderOffset = HeaderOffset;
  m_FileLen = pFileAccess->GetSize();
  m_Pos = 0;
  m_pFileAccess = pFileAccess;
  m_BufOffset = 0;
  pFileAccess->ReadBlock(
      m_pFileBuf, 0,
      (size_t)((FX_FILESIZE)m_BufSize > m_FileLen ? m_FileLen : m_BufSize));
}
int32_t CPDF_SyntaxParser::GetDirectNum() {
  bool bIsNumber;
  GetNextWordInternal(&bIsNumber);
  if (!bIsNumber)
    return 0;

  m_WordBuffer[m_WordSize] = 0;
  return FXSYS_atoi(reinterpret_cast<const FX_CHAR*>(m_WordBuffer));
}

bool CPDF_SyntaxParser::IsWholeWord(FX_FILESIZE startpos,
                                    FX_FILESIZE limit,
                                    const CFX_ByteStringC& tag,
                                    FX_BOOL checkKeyword) {
  const FX_DWORD taglen = tag.GetLength();
  bool bCheckLeft = !PDFCharIsDelimiter(tag[0]) && !PDFCharIsWhitespace(tag[0]);
  bool bCheckRight = !PDFCharIsDelimiter(tag[taglen - 1]) &&
                     !PDFCharIsWhitespace(tag[taglen - 1]);
  uint8_t ch;
  if (bCheckRight && startpos + (int32_t)taglen <= limit &&
      GetCharAt(startpos + (int32_t)taglen, ch)) {
    if (PDFCharIsNumeric(ch) || PDFCharIsOther(ch) ||
        (checkKeyword && PDFCharIsDelimiter(ch))) {
      return false;
    }
  }

  if (bCheckLeft && startpos > 0 && GetCharAt(startpos - 1, ch)) {
    if (PDFCharIsNumeric(ch) || PDFCharIsOther(ch) ||
        (checkKeyword && PDFCharIsDelimiter(ch))) {
      return false;
    }
  }
  return true;
}

FX_BOOL CPDF_SyntaxParser::SearchWord(const CFX_ByteStringC& tag,
                                      FX_BOOL bWholeWord,
                                      FX_BOOL bForward,
                                      FX_FILESIZE limit) {
  int32_t taglen = tag.GetLength();
  if (taglen == 0) {
    return FALSE;
  }
  FX_FILESIZE pos = m_Pos;
  int32_t offset = 0;
  if (!bForward) {
    offset = taglen - 1;
  }
  const uint8_t* tag_data = tag.GetPtr();
  uint8_t byte;
  while (1) {
    if (bForward) {
      if (limit) {
        if (pos >= m_Pos + limit) {
          return FALSE;
        }
      }
      if (!GetCharAt(pos, byte)) {
        return FALSE;
      }
    } else {
      if (limit) {
        if (pos <= m_Pos - limit) {
          return FALSE;
        }
      }
      if (!GetCharAtBackward(pos, byte)) {
        return FALSE;
      }
    }
    if (byte == tag_data[offset]) {
      if (bForward) {
        offset++;
        if (offset < taglen) {
          pos++;
          continue;
        }
      } else {
        offset--;
        if (offset >= 0) {
          pos--;
          continue;
        }
      }
      FX_FILESIZE startpos = bForward ? pos - taglen + 1 : pos;
      if (!bWholeWord || IsWholeWord(startpos, limit, tag, FALSE)) {
        m_Pos = startpos;
        return TRUE;
      }
    }
    if (bForward) {
      offset = byte == tag_data[0] ? 1 : 0;
      pos++;
    } else {
      offset = byte == tag_data[taglen - 1] ? taglen - 2 : taglen - 1;
      pos--;
    }
    if (pos < 0) {
      return FALSE;
    }
  }
  return FALSE;
}

int32_t CPDF_SyntaxParser::SearchMultiWord(const CFX_ByteStringC& tags,
                                           FX_BOOL bWholeWord,
                                           FX_FILESIZE limit) {
  int32_t ntags = 1;
  for (int i = 0; i < tags.GetLength(); ++i) {
    if (tags[i] == 0) {
      ++ntags;
    }
  }

  std::vector<SearchTagRecord> patterns(ntags);
  FX_DWORD start = 0;
  FX_DWORD itag = 0;
  FX_DWORD max_len = 0;
  for (int i = 0; i <= tags.GetLength(); ++i) {
    if (tags[i] == 0) {
      FX_DWORD len = i - start;
      max_len = std::max(len, max_len);
      patterns[itag].m_pTag = tags.GetCStr() + start;
      patterns[itag].m_Len = len;
      patterns[itag].m_Offset = 0;
      start = i + 1;
      ++itag;
    }
  }

  const FX_FILESIZE pos_limit = m_Pos + limit;
  for (FX_FILESIZE pos = m_Pos; !limit || pos < pos_limit; ++pos) {
    uint8_t byte;
    if (!GetCharAt(pos, byte))
      break;

    for (int i = 0; i < ntags; ++i) {
      SearchTagRecord& pat = patterns[i];
      if (pat.m_pTag[pat.m_Offset] != byte) {
        pat.m_Offset = (pat.m_pTag[0] == byte) ? 1 : 0;
        continue;
      }

      ++pat.m_Offset;
      if (pat.m_Offset != pat.m_Len)
        continue;

      if (!bWholeWord ||
          IsWholeWord(pos - pat.m_Len, limit,
                      CFX_ByteStringC(pat.m_pTag, pat.m_Len), FALSE)) {
        return i;
      }

      pat.m_Offset = (pat.m_pTag[0] == byte) ? 1 : 0;
    }
  }
  return -1;
}

FX_FILESIZE CPDF_SyntaxParser::FindTag(const CFX_ByteStringC& tag,
                                       FX_FILESIZE limit) {
  int32_t taglen = tag.GetLength();
  int32_t match = 0;
  limit += m_Pos;
  FX_FILESIZE startpos = m_Pos;
  while (1) {
    uint8_t ch;
    if (!GetNextChar(ch)) {
      return -1;
    }
    if (ch == tag[match]) {
      match++;
      if (match == taglen) {
        return m_Pos - startpos - taglen;
      }
    } else {
      match = ch == tag[0] ? 1 : 0;
    }
    if (limit && m_Pos == limit) {
      return -1;
    }
  }
  return -1;
}
void CPDF_SyntaxParser::GetBinary(uint8_t* buffer, FX_DWORD size) {
  FX_DWORD offset = 0;
  uint8_t ch;
  while (1) {
    if (!GetNextChar(ch)) {
      return;
    }
    buffer[offset++] = ch;
    if (offset == size) {
      break;
    }
  }
}

class CPDF_DataAvail final : public IPDF_DataAvail {
 public:
  CPDF_DataAvail(IFX_FileAvail* pFileAvail,
                 IFX_FileRead* pFileRead,
                 FX_BOOL bSupportHintTable);
  ~CPDF_DataAvail() override;

  // IPDF_DataAvail:
  DocAvailStatus IsDocAvail(IFX_DownloadHints* pHints) override;
  void SetDocument(CPDF_Document* pDoc) override;
  DocAvailStatus IsPageAvail(int iPage, IFX_DownloadHints* pHints) override;
  DocFormStatus IsFormAvail(IFX_DownloadHints* pHints) override;
  DocLinearizationStatus IsLinearizedPDF() override;
  FX_BOOL IsLinearized() override { return m_bLinearized; }
  void GetLinearizedMainXRefInfo(FX_FILESIZE* pPos, FX_DWORD* pSize) override;

  int GetPageCount() const;
  CPDF_Dictionary* GetPage(int index);

  friend class CPDF_HintTables;

 protected:
  static const int kMaxDataAvailRecursionDepth = 64;
  static int s_CurrentDataAvailRecursionDepth;
  static const int kMaxPageRecursionDepth = 1024;

  FX_DWORD GetObjectSize(FX_DWORD objnum, FX_FILESIZE& offset);
  FX_BOOL IsObjectsAvail(CFX_ArrayTemplate<CPDF_Object*>& obj_array,
                         FX_BOOL bParsePage,
                         IFX_DownloadHints* pHints,
                         CFX_ArrayTemplate<CPDF_Object*>& ret_array);
  FX_BOOL CheckDocStatus(IFX_DownloadHints* pHints);
  FX_BOOL CheckHeader(IFX_DownloadHints* pHints);
  FX_BOOL CheckFirstPage(IFX_DownloadHints* pHints);
  FX_BOOL CheckHintTables(IFX_DownloadHints* pHints);
  FX_BOOL CheckEnd(IFX_DownloadHints* pHints);
  FX_BOOL CheckCrossRef(IFX_DownloadHints* pHints);
  FX_BOOL CheckCrossRefItem(IFX_DownloadHints* pHints);
  FX_BOOL CheckTrailer(IFX_DownloadHints* pHints);
  FX_BOOL CheckRoot(IFX_DownloadHints* pHints);
  FX_BOOL CheckInfo(IFX_DownloadHints* pHints);
  FX_BOOL CheckPages(IFX_DownloadHints* pHints);
  FX_BOOL CheckPage(IFX_DownloadHints* pHints);
  FX_BOOL CheckResources(IFX_DownloadHints* pHints);
  FX_BOOL CheckAnnots(IFX_DownloadHints* pHints);
  FX_BOOL CheckAcroForm(IFX_DownloadHints* pHints);
  FX_BOOL CheckAcroFormSubObject(IFX_DownloadHints* pHints);
  FX_BOOL CheckTrailerAppend(IFX_DownloadHints* pHints);
  FX_BOOL CheckPageStatus(IFX_DownloadHints* pHints);
  FX_BOOL CheckAllCrossRefStream(IFX_DownloadHints* pHints);

  int32_t CheckCrossRefStream(IFX_DownloadHints* pHints,
                              FX_FILESIZE& xref_offset);
  FX_BOOL IsLinearizedFile(uint8_t* pData, FX_DWORD dwLen);
  void SetStartOffset(FX_FILESIZE dwOffset);
  FX_BOOL GetNextToken(CFX_ByteString& token);
  FX_BOOL GetNextChar(uint8_t& ch);
  CPDF_Object* ParseIndirectObjectAt(
      FX_FILESIZE pos,
      FX_DWORD objnum,
      CPDF_IndirectObjectHolder* pObjList = NULL);
  CPDF_Object* GetObject(FX_DWORD objnum,
                         IFX_DownloadHints* pHints,
                         FX_BOOL* pExistInFile);
  FX_BOOL GetPageKids(CPDF_Parser* pParser, CPDF_Object* pPages);
  FX_BOOL PreparePageItem();
  FX_BOOL LoadPages(IFX_DownloadHints* pHints);
  FX_BOOL LoadAllXref(IFX_DownloadHints* pHints);
  FX_BOOL LoadAllFile(IFX_DownloadHints* pHints);
  DocAvailStatus CheckLinearizedData(IFX_DownloadHints* pHints);
  FX_BOOL CheckPageAnnots(int iPage, IFX_DownloadHints* pHints);

  DocAvailStatus CheckLinearizedFirstPage(int iPage, IFX_DownloadHints* pHints);
  FX_BOOL HaveResourceAncestor(CPDF_Dictionary* pDict);
  FX_BOOL CheckPage(int32_t iPage, IFX_DownloadHints* pHints);
  FX_BOOL LoadDocPages(IFX_DownloadHints* pHints);
  FX_BOOL LoadDocPage(int32_t iPage, IFX_DownloadHints* pHints);
  FX_BOOL CheckPageNode(CPDF_PageNode& pageNodes,
                        int32_t iPage,
                        int32_t& iCount,
                        IFX_DownloadHints* pHints,
                        int level);
  FX_BOOL CheckUnkownPageNode(FX_DWORD dwPageNo,
                              CPDF_PageNode* pPageNode,
                              IFX_DownloadHints* pHints);
  FX_BOOL CheckArrayPageNode(FX_DWORD dwPageNo,
                             CPDF_PageNode* pPageNode,
                             IFX_DownloadHints* pHints);
  FX_BOOL CheckPageCount(IFX_DownloadHints* pHints);
  bool IsFirstCheck(int iPage);
  void ResetFirstCheck(int iPage);
  FX_BOOL IsDataAvail(FX_FILESIZE offset,
                      FX_DWORD size,
                      IFX_DownloadHints* pHints);

  CPDF_Parser m_parser;

  CPDF_SyntaxParser m_syntaxParser;

  CPDF_Object* m_pRoot;

  FX_DWORD m_dwRootObjNum;

  FX_DWORD m_dwInfoObjNum;

  CPDF_Object* m_pLinearized;

  CPDF_Object* m_pTrailer;

  FX_BOOL m_bDocAvail;

  FX_FILESIZE m_dwHeaderOffset;

  FX_FILESIZE m_dwLastXRefOffset;

  FX_FILESIZE m_dwXRefOffset;

  FX_FILESIZE m_dwTrailerOffset;

  FX_FILESIZE m_dwCurrentOffset;

  PDF_DATAAVAIL_STATUS m_docStatus;

  FX_FILESIZE m_dwFileLen;

  CPDF_Document* m_pDocument;

  std::set<FX_DWORD> m_ObjectSet;

  CFX_ArrayTemplate<CPDF_Object*> m_objs_array;

  FX_FILESIZE m_Pos;

  FX_FILESIZE m_bufferOffset;

  FX_DWORD m_bufferSize;

  CFX_ByteString m_WordBuf;

  uint8_t m_bufferData[512];

  CFX_FileSizeArray m_CrossOffset;

  CFX_DWordArray m_XRefStreamList;

  CFX_DWordArray m_PageObjList;

  FX_DWORD m_PagesObjNum;

  FX_BOOL m_bLinearized;

  FX_DWORD m_dwFirstPageNo;

  FX_BOOL m_bLinearedDataOK;

  FX_BOOL m_bMainXRefLoadTried;

  FX_BOOL m_bMainXRefLoadedOK;

  FX_BOOL m_bPagesTreeLoad;

  FX_BOOL m_bPagesLoad;

  CPDF_Parser* m_pCurrentParser;

  FX_FILESIZE m_dwCurrentXRefSteam;

  FX_BOOL m_bAnnotsLoad;

  FX_BOOL m_bHaveAcroForm;

  FX_DWORD m_dwAcroFormObjNum;

  FX_BOOL m_bAcroFormLoad;

  CPDF_Object* m_pAcroForm;

  CFX_ArrayTemplate<CPDF_Object*> m_arrayAcroforms;

  CPDF_Dictionary* m_pPageDict;

  CPDF_Object* m_pPageResource;

  FX_BOOL m_bNeedDownLoadResource;

  FX_BOOL m_bPageLoadedOK;

  FX_BOOL m_bLinearizedFormParamLoad;

  CFX_ArrayTemplate<CPDF_Object*> m_PagesArray;

  FX_DWORD m_dwEncryptObjNum;

  FX_FILESIZE m_dwPrevXRefOffset;

  FX_BOOL m_bTotalLoadPageTree;

  FX_BOOL m_bCurPageDictLoadOK;

  CPDF_PageNode m_pageNodes;

  std::set<FX_DWORD> m_pageMapCheckState;
  std::set<FX_DWORD> m_pagesLoadState;

  std::unique_ptr<CPDF_HintTables> m_pHintTables;
  FX_BOOL m_bSupportHintTable;
};

IPDF_DataAvail::IPDF_DataAvail(IFX_FileAvail* pFileAvail,
                               IFX_FileRead* pFileRead)
    : m_pFileAvail(pFileAvail), m_pFileRead(pFileRead) {}

// static
IPDF_DataAvail* IPDF_DataAvail::Create(IFX_FileAvail* pFileAvail,
                                       IFX_FileRead* pFileRead) {
  return new CPDF_DataAvail(pFileAvail, pFileRead, TRUE);
}

// static
int CPDF_DataAvail::s_CurrentDataAvailRecursionDepth = 0;

CPDF_DataAvail::CPDF_DataAvail(IFX_FileAvail* pFileAvail,
                               IFX_FileRead* pFileRead,
                               FX_BOOL bSupportHintTable)
    : IPDF_DataAvail(pFileAvail, pFileRead) {
  m_Pos = 0;
  m_dwFileLen = 0;
  if (m_pFileRead) {
    m_dwFileLen = (FX_DWORD)m_pFileRead->GetSize();
  }
  m_dwCurrentOffset = 0;
  m_dwXRefOffset = 0;
  m_bufferOffset = 0;
  m_dwFirstPageNo = 0;
  m_bufferSize = 0;
  m_PagesObjNum = 0;
  m_dwCurrentXRefSteam = 0;
  m_dwAcroFormObjNum = 0;
  m_dwInfoObjNum = 0;
  m_pDocument = 0;
  m_dwEncryptObjNum = 0;
  m_dwPrevXRefOffset = 0;
  m_dwLastXRefOffset = 0;
  m_bDocAvail = FALSE;
  m_bMainXRefLoadTried = FALSE;
  m_bDocAvail = FALSE;
  m_bLinearized = FALSE;
  m_bPagesLoad = FALSE;
  m_bPagesTreeLoad = FALSE;
  m_bMainXRefLoadedOK = FALSE;
  m_bAnnotsLoad = FALSE;
  m_bHaveAcroForm = FALSE;
  m_bAcroFormLoad = FALSE;
  m_bPageLoadedOK = FALSE;
  m_bNeedDownLoadResource = FALSE;
  m_bLinearizedFormParamLoad = FALSE;
  m_pLinearized = NULL;
  m_pRoot = NULL;
  m_pTrailer = NULL;
  m_pCurrentParser = NULL;
  m_pAcroForm = NULL;
  m_pPageDict = NULL;
  m_pPageResource = NULL;
  m_docStatus = PDF_DATAAVAIL_HEADER;
  m_parser.m_bOwnFileRead = FALSE;
  m_bTotalLoadPageTree = FALSE;
  m_bCurPageDictLoadOK = FALSE;
  m_bLinearedDataOK = FALSE;
  m_bSupportHintTable = bSupportHintTable;
}
CPDF_DataAvail::~CPDF_DataAvail() {
  if (m_pLinearized) {
    m_pLinearized->Release();
  }
  if (m_pRoot) {
    m_pRoot->Release();
  }
  if (m_pTrailer) {
    m_pTrailer->Release();
  }

  int iSize = m_arrayAcroforms.GetSize();
  for (int i = 0; i < iSize; ++i) {
    m_arrayAcroforms.GetAt(i)->Release();
  }
}
void CPDF_DataAvail::SetDocument(CPDF_Document* pDoc) {
  m_pDocument = pDoc;
}
FX_DWORD CPDF_DataAvail::GetObjectSize(FX_DWORD objnum, FX_FILESIZE& offset) {
  CPDF_Parser* pParser = (CPDF_Parser*)(m_pDocument->GetParser());
  if (!pParser || !pParser->IsValidObjectNumber(objnum))
    return 0;

  if (pParser->m_V5Type[objnum] == 2)
    objnum = pParser->m_ObjectInfo[objnum].pos;

  if (pParser->m_V5Type[objnum] == 1 || pParser->m_V5Type[objnum] == 255) {
    offset = pParser->m_ObjectInfo[objnum].pos;
    if (offset == 0) {
      return 0;
    }
    void* pResult = FXSYS_bsearch(&offset, pParser->m_SortedOffset.GetData(),
                                  pParser->m_SortedOffset.GetSize(),
                                  sizeof(FX_FILESIZE), CompareFileSize);
    if (!pResult) {
      return 0;
    }
    if ((FX_FILESIZE*)pResult -
            (FX_FILESIZE*)pParser->m_SortedOffset.GetData() ==
        pParser->m_SortedOffset.GetSize() - 1) {
      return 0;
    }
    return (FX_DWORD)(((FX_FILESIZE*)pResult)[1] - offset);
  }
  return 0;
}
FX_BOOL CPDF_DataAvail::IsObjectsAvail(
    CFX_ArrayTemplate<CPDF_Object*>& obj_array,
    FX_BOOL bParsePage,
    IFX_DownloadHints* pHints,
    CFX_ArrayTemplate<CPDF_Object*>& ret_array) {
  if (!obj_array.GetSize()) {
    return TRUE;
  }
  FX_DWORD count = 0;
  CFX_ArrayTemplate<CPDF_Object*> new_obj_array;
  int32_t i = 0;
  for (i = 0; i < obj_array.GetSize(); i++) {
    CPDF_Object* pObj = obj_array[i];
    if (!pObj)
      continue;

    int32_t type = pObj->GetType();
    switch (type) {
      case PDFOBJ_ARRAY: {
        CPDF_Array* pArray = pObj->GetArray();
        for (FX_DWORD k = 0; k < pArray->GetCount(); k++) {
          new_obj_array.Add(pArray->GetElement(k));
        }
      } break;
      case PDFOBJ_STREAM:
        pObj = pObj->GetDict();
      case PDFOBJ_DICTIONARY: {
        CPDF_Dictionary* pDict = pObj->GetDict();
        if (pDict && pDict->GetString("Type") == "Page" && !bParsePage) {
          continue;
        }
        for (const auto& it : *pDict) {
          const CFX_ByteString& key = it.first;
          CPDF_Object* value = it.second;
          if (key != "Parent") {
            new_obj_array.Add(value);
          }
        }
      } break;
      case PDFOBJ_REFERENCE: {
        CPDF_Reference* pRef = pObj->AsReference();
        FX_DWORD dwNum = pRef->GetRefObjNum();
        FX_FILESIZE offset;
        FX_DWORD size = GetObjectSize(dwNum, offset);
        if (size == 0 || offset < 0 || offset >= m_dwFileLen) {
          break;
        }
        if (!IsDataAvail(offset, size, pHints)) {
          ret_array.Add(pObj);
          count++;
        } else if (!pdfium::ContainsKey(m_ObjectSet, dwNum)) {
          m_ObjectSet.insert(dwNum);
          CPDF_Object* pReferred =
              m_pDocument->GetIndirectObject(pRef->GetRefObjNum(), nullptr);
          if (pReferred) {
            new_obj_array.Add(pReferred);
          }
        }
      } break;
    }
  }
  if (count > 0) {
    int32_t iSize = new_obj_array.GetSize();
    for (i = 0; i < iSize; ++i) {
      CPDF_Object* pObj = new_obj_array[i];
      if (CPDF_Reference* pRef = pObj->AsReference()) {
        FX_DWORD dwNum = pRef->GetRefObjNum();
        if (!pdfium::ContainsKey(m_ObjectSet, dwNum))
          ret_array.Add(pObj);
      } else {
        ret_array.Add(pObj);
      }
    }
    return FALSE;
  }
  obj_array.RemoveAll();
  obj_array.Append(new_obj_array);
  return IsObjectsAvail(obj_array, FALSE, pHints, ret_array);
}

IPDF_DataAvail::DocAvailStatus CPDF_DataAvail::IsDocAvail(
    IFX_DownloadHints* pHints) {
  if (!m_dwFileLen && m_pFileRead) {
    m_dwFileLen = (FX_DWORD)m_pFileRead->GetSize();
    if (!m_dwFileLen) {
      return DataError;
    }
  }
  while (!m_bDocAvail) {
    if (!CheckDocStatus(pHints)) {
      return DataNotAvailable;
    }
  }
  return DataAvailable;
}

FX_BOOL CPDF_DataAvail::CheckAcroFormSubObject(IFX_DownloadHints* pHints) {
  if (!m_objs_array.GetSize()) {
    m_objs_array.RemoveAll();
    m_ObjectSet.clear();
    CFX_ArrayTemplate<CPDF_Object*> obj_array;
    obj_array.Append(m_arrayAcroforms);
    FX_BOOL bRet = IsObjectsAvail(obj_array, FALSE, pHints, m_objs_array);
    if (bRet) {
      m_objs_array.RemoveAll();
    }
    return bRet;
  }
  CFX_ArrayTemplate<CPDF_Object*> new_objs_array;
  FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
  if (bRet) {
    int32_t iSize = m_arrayAcroforms.GetSize();
    for (int32_t i = 0; i < iSize; ++i) {
      m_arrayAcroforms.GetAt(i)->Release();
    }
    m_arrayAcroforms.RemoveAll();
  } else {
    m_objs_array.RemoveAll();
    m_objs_array.Append(new_objs_array);
  }
  return bRet;
}
FX_BOOL CPDF_DataAvail::CheckAcroForm(IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  m_pAcroForm = GetObject(m_dwAcroFormObjNum, pHints, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_PAGETREE;
    return TRUE;
  }
  if (!m_pAcroForm) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return TRUE;
    }
    return FALSE;
  }
  m_arrayAcroforms.Add(m_pAcroForm);
  m_docStatus = PDF_DATAAVAIL_PAGETREE;
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckDocStatus(IFX_DownloadHints* pHints) {
  switch (m_docStatus) {
    case PDF_DATAAVAIL_HEADER:
      return CheckHeader(pHints);
    case PDF_DATAAVAIL_FIRSTPAGE:
    case PDF_DATAAVAIL_FIRSTPAGE_PREPARE:
      return CheckFirstPage(pHints);
    case PDF_DATAAVAIL_HINTTABLE:
      return CheckHintTables(pHints);
    case PDF_DATAAVAIL_END:
      return CheckEnd(pHints);
    case PDF_DATAAVAIL_CROSSREF:
      return CheckCrossRef(pHints);
    case PDF_DATAAVAIL_CROSSREF_ITEM:
      return CheckCrossRefItem(pHints);
    case PDF_DATAAVAIL_CROSSREF_STREAM:
      return CheckAllCrossRefStream(pHints);
    case PDF_DATAAVAIL_TRAILER:
      return CheckTrailer(pHints);
    case PDF_DATAAVAIL_TRAILER_APPEND:
      return CheckTrailerAppend(pHints);
    case PDF_DATAAVAIL_LOADALLCROSSREF:
      return LoadAllXref(pHints);
    case PDF_DATAAVAIL_LOADALLFILE:
      return LoadAllFile(pHints);
    case PDF_DATAAVAIL_ROOT:
      return CheckRoot(pHints);
    case PDF_DATAAVAIL_INFO:
      return CheckInfo(pHints);
    case PDF_DATAAVAIL_ACROFORM:
      return CheckAcroForm(pHints);
    case PDF_DATAAVAIL_PAGETREE:
      if (m_bTotalLoadPageTree) {
        return CheckPages(pHints);
      }
      return LoadDocPages(pHints);
    case PDF_DATAAVAIL_PAGE:
      if (m_bTotalLoadPageTree) {
        return CheckPage(pHints);
      }
      m_docStatus = PDF_DATAAVAIL_PAGE_LATERLOAD;
      return TRUE;
    case PDF_DATAAVAIL_ERROR:
      return LoadAllFile(pHints);
    case PDF_DATAAVAIL_PAGE_LATERLOAD:
      m_docStatus = PDF_DATAAVAIL_PAGE;
    default:
      m_bDocAvail = TRUE;
      return TRUE;
  }
}
FX_BOOL CPDF_DataAvail::CheckPageStatus(IFX_DownloadHints* pHints) {
  switch (m_docStatus) {
    case PDF_DATAAVAIL_PAGETREE:
      return CheckPages(pHints);
    case PDF_DATAAVAIL_PAGE:
      return CheckPage(pHints);
    case PDF_DATAAVAIL_ERROR:
      return LoadAllFile(pHints);
    default:
      m_bPagesTreeLoad = TRUE;
      m_bPagesLoad = TRUE;
      return TRUE;
  }
}
FX_BOOL CPDF_DataAvail::LoadAllFile(IFX_DownloadHints* pHints) {
  if (m_pFileAvail->IsDataAvail(0, (FX_DWORD)m_dwFileLen)) {
    m_docStatus = PDF_DATAAVAIL_DONE;
    return TRUE;
  }
  pHints->AddSegment(0, (FX_DWORD)m_dwFileLen);
  return FALSE;
}
FX_BOOL CPDF_DataAvail::LoadAllXref(IFX_DownloadHints* pHints) {
  m_parser.m_Syntax.InitParser(m_pFileRead, (FX_DWORD)m_dwHeaderOffset);
  m_parser.m_bOwnFileRead = FALSE;
  if (!m_parser.LoadAllCrossRefV4(m_dwLastXRefOffset) &&
      !m_parser.LoadAllCrossRefV5(m_dwLastXRefOffset)) {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return FALSE;
  }
  FXSYS_qsort(m_parser.m_SortedOffset.GetData(),
              m_parser.m_SortedOffset.GetSize(), sizeof(FX_FILESIZE),
              CompareFileSize);
  m_dwRootObjNum = m_parser.GetRootObjNum();
  m_dwInfoObjNum = m_parser.GetInfoObjNum();
  m_pCurrentParser = &m_parser;
  m_docStatus = PDF_DATAAVAIL_ROOT;
  return TRUE;
}
CPDF_Object* CPDF_DataAvail::GetObject(FX_DWORD objnum,
                                       IFX_DownloadHints* pHints,
                                       FX_BOOL* pExistInFile) {
  CPDF_Object* pRet = nullptr;
  FX_DWORD size = 0;
  FX_FILESIZE offset = 0;
  CPDF_Parser* pParser = nullptr;
  if (pExistInFile)
    *pExistInFile = TRUE;

  if (m_pDocument) {
    size = GetObjectSize(objnum, offset);
    pParser = (CPDF_Parser*)(m_pDocument->GetParser());
  } else {
    size = (FX_DWORD)m_parser.GetObjectSize(objnum);
    offset = m_parser.GetObjectOffset(objnum);
    pParser = &m_parser;
  }
  if (!IsDataAvail(offset, size, pHints)) {
    return nullptr;
  }
  if (pParser) {
    pRet = pParser->ParseIndirectObject(NULL, objnum, NULL);
  }

  if (!pRet && pExistInFile) {
    *pExistInFile = FALSE;
  }

  return pRet;
}

FX_BOOL CPDF_DataAvail::CheckInfo(IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  CPDF_Object* pInfo = GetObject(m_dwInfoObjNum, pHints, &bExist);
  if (!bExist) {
    if (m_bHaveAcroForm) {
      m_docStatus = PDF_DATAAVAIL_ACROFORM;
    } else {
      m_docStatus = PDF_DATAAVAIL_PAGETREE;
    }
    return TRUE;
  }
  if (!pInfo) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return TRUE;
    }
    if (m_Pos == m_dwFileLen) {
      m_docStatus = PDF_DATAAVAIL_ERROR;
    }
    return FALSE;
  }
  if (pInfo) {
    pInfo->Release();
  }
  if (m_bHaveAcroForm) {
    m_docStatus = PDF_DATAAVAIL_ACROFORM;
  } else {
    m_docStatus = PDF_DATAAVAIL_PAGETREE;
  }
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckRoot(IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  m_pRoot = GetObject(m_dwRootObjNum, pHints, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return TRUE;
  }
  if (!m_pRoot) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return TRUE;
    }
    return FALSE;
  }
  CPDF_Dictionary* pDict = m_pRoot->GetDict();
  if (!pDict) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  CPDF_Reference* pRef = ToReference(pDict->GetElement("Pages"));
  if (!pRef) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }

  m_PagesObjNum = pRef->GetRefObjNum();
  CPDF_Reference* pAcroFormRef =
      ToReference(m_pRoot->GetDict()->GetElement("AcroForm"));
  if (pAcroFormRef) {
    m_bHaveAcroForm = TRUE;
    m_dwAcroFormObjNum = pAcroFormRef->GetRefObjNum();
  }

  if (m_dwInfoObjNum) {
    m_docStatus = PDF_DATAAVAIL_INFO;
  } else {
    m_docStatus =
        m_bHaveAcroForm ? PDF_DATAAVAIL_ACROFORM : PDF_DATAAVAIL_PAGETREE;
  }
  return TRUE;
}
FX_BOOL CPDF_DataAvail::PreparePageItem() {
  CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  CPDF_Reference* pRef =
      ToReference(pRoot ? pRoot->GetElement("Pages") : nullptr);
  if (!pRef) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }

  m_PagesObjNum = pRef->GetRefObjNum();
  m_pCurrentParser = (CPDF_Parser*)m_pDocument->GetParser();
  m_docStatus = PDF_DATAAVAIL_PAGETREE;
  return TRUE;
}
bool CPDF_DataAvail::IsFirstCheck(int iPage) {
  return m_pageMapCheckState.insert(iPage).second;
}
void CPDF_DataAvail::ResetFirstCheck(int iPage) {
  m_pageMapCheckState.erase(iPage);
}
FX_BOOL CPDF_DataAvail::CheckPage(IFX_DownloadHints* pHints) {
  FX_DWORD iPageObjs = m_PageObjList.GetSize();
  CFX_DWordArray UnavailObjList;
  for (FX_DWORD i = 0; i < iPageObjs; ++i) {
    FX_DWORD dwPageObjNum = m_PageObjList.GetAt(i);
    FX_BOOL bExist = FALSE;
    CPDF_Object* pObj = GetObject(dwPageObjNum, pHints, &bExist);
    if (!pObj) {
      if (bExist) {
        UnavailObjList.Add(dwPageObjNum);
      }
      continue;
    }
    if (pObj->IsArray()) {
      CPDF_Array* pArray = pObj->GetArray();
      if (pArray) {
        int32_t iSize = pArray->GetCount();
        for (int32_t j = 0; j < iSize; ++j) {
          if (CPDF_Reference* pRef = ToReference(pArray->GetElement(j)))
            UnavailObjList.Add(pRef->GetRefObjNum());
        }
      }
    }
    if (!pObj->IsDictionary()) {
      pObj->Release();
      continue;
    }
    CFX_ByteString type = pObj->GetDict()->GetString("Type");
    if (type == "Pages") {
      m_PagesArray.Add(pObj);
      continue;
    }
    pObj->Release();
  }
  m_PageObjList.RemoveAll();
  if (UnavailObjList.GetSize()) {
    m_PageObjList.Append(UnavailObjList);
    return FALSE;
  }
  FX_DWORD iPages = m_PagesArray.GetSize();
  for (FX_DWORD i = 0; i < iPages; i++) {
    CPDF_Object* pPages = m_PagesArray.GetAt(i);
    if (!pPages)
      continue;

    if (!GetPageKids(m_pCurrentParser, pPages)) {
      pPages->Release();
      while (++i < iPages) {
        pPages = m_PagesArray.GetAt(i);
        pPages->Release();
      }
      m_PagesArray.RemoveAll();
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return FALSE;
    }
    pPages->Release();
  }
  m_PagesArray.RemoveAll();
  if (!m_PageObjList.GetSize()) {
    m_docStatus = PDF_DATAAVAIL_DONE;
  }
  return TRUE;
}
FX_BOOL CPDF_DataAvail::GetPageKids(CPDF_Parser* pParser, CPDF_Object* pPages) {
  if (!pParser) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  CPDF_Dictionary* pDict = pPages->GetDict();
  CPDF_Object* pKids = pDict ? pDict->GetElement("Kids") : NULL;
  if (!pKids) {
    return TRUE;
  }
  switch (pKids->GetType()) {
    case PDFOBJ_REFERENCE:
      m_PageObjList.Add(pKids->AsReference()->GetRefObjNum());
      break;
    case PDFOBJ_ARRAY: {
      CPDF_Array* pKidsArray = pKids->AsArray();
      for (FX_DWORD i = 0; i < pKidsArray->GetCount(); ++i) {
        if (CPDF_Reference* pRef = ToReference(pKidsArray->GetElement(i)))
          m_PageObjList.Add(pRef->GetRefObjNum());
      }
    } break;
    default:
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return FALSE;
  }
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckPages(IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  CPDF_Object* pPages = GetObject(m_PagesObjNum, pHints, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return TRUE;
  }
  if (!pPages) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return TRUE;
    }
    return FALSE;
  }
  if (!GetPageKids(m_pCurrentParser, pPages)) {
    pPages->Release();
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  pPages->Release();
  m_docStatus = PDF_DATAAVAIL_PAGE;
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckHeader(IFX_DownloadHints* pHints) {
  FX_DWORD req_size = 1024;
  if ((FX_FILESIZE)req_size > m_dwFileLen) {
    req_size = (FX_DWORD)m_dwFileLen;
  }
  if (m_pFileAvail->IsDataAvail(0, req_size)) {
    uint8_t buffer[1024];
    m_pFileRead->ReadBlock(buffer, 0, req_size);
    if (IsLinearizedFile(buffer, req_size)) {
      m_docStatus = PDF_DATAAVAIL_FIRSTPAGE;
    } else {
      if (m_docStatus == PDF_DATAAVAIL_ERROR) {
        return FALSE;
      }
      m_docStatus = PDF_DATAAVAIL_END;
    }
    return TRUE;
  }
  pHints->AddSegment(0, req_size);
  return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckFirstPage(IFX_DownloadHints* pHints) {
  CPDF_Dictionary* pDict = m_pLinearized->GetDict();
  CPDF_Object* pEndOffSet = pDict ? pDict->GetElement("E") : NULL;
  if (!pEndOffSet) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  CPDF_Object* pXRefOffset = pDict ? pDict->GetElement("T") : NULL;
  if (!pXRefOffset) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  CPDF_Object* pFileLen = pDict ? pDict->GetElement("L") : NULL;
  if (!pFileLen) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  FX_BOOL bNeedDownLoad = FALSE;
  if (pEndOffSet->IsNumber()) {
    FX_DWORD dwEnd = pEndOffSet->GetInteger();
    dwEnd += 512;
    if ((FX_FILESIZE)dwEnd > m_dwFileLen) {
      dwEnd = (FX_DWORD)m_dwFileLen;
    }
    int32_t iStartPos = (int32_t)(m_dwFileLen > 1024 ? 1024 : m_dwFileLen);
    int32_t iSize = dwEnd > 1024 ? dwEnd - 1024 : 0;
    if (!m_pFileAvail->IsDataAvail(iStartPos, iSize)) {
      pHints->AddSegment(iStartPos, iSize);
      bNeedDownLoad = TRUE;
    }
  }
  m_dwLastXRefOffset = 0;
  FX_FILESIZE dwFileLen = 0;
  if (pXRefOffset->IsNumber())
    m_dwLastXRefOffset = pXRefOffset->GetInteger();

  if (pFileLen->IsNumber())
    dwFileLen = pFileLen->GetInteger();

  if (!m_pFileAvail->IsDataAvail(m_dwLastXRefOffset,
                                 (FX_DWORD)(dwFileLen - m_dwLastXRefOffset))) {
    if (m_docStatus == PDF_DATAAVAIL_FIRSTPAGE) {
      FX_DWORD dwSize = (FX_DWORD)(dwFileLen - m_dwLastXRefOffset);
      FX_FILESIZE offset = m_dwLastXRefOffset;
      if (dwSize < 512 && dwFileLen > 512) {
        dwSize = 512;
        offset = dwFileLen - 512;
      }
      pHints->AddSegment(offset, dwSize);
    }
  } else {
    m_docStatus = PDF_DATAAVAIL_FIRSTPAGE_PREPARE;
  }
  if (bNeedDownLoad || m_docStatus != PDF_DATAAVAIL_FIRSTPAGE_PREPARE) {
    m_docStatus = PDF_DATAAVAIL_FIRSTPAGE_PREPARE;
    return FALSE;
  }
  m_docStatus =
      m_bSupportHintTable ? PDF_DATAAVAIL_HINTTABLE : PDF_DATAAVAIL_DONE;
  return TRUE;
}
FX_BOOL CPDF_DataAvail::IsDataAvail(FX_FILESIZE offset,
                                    FX_DWORD size,
                                    IFX_DownloadHints* pHints) {
  if (offset > m_dwFileLen)
    return TRUE;
  FX_SAFE_DWORD safeSize = pdfium::base::checked_cast<FX_DWORD>(offset);
  safeSize += size;
  safeSize += 512;
  if (!safeSize.IsValid() || safeSize.ValueOrDie() > m_dwFileLen)
    size = m_dwFileLen - offset;
  else
    size += 512;
  if (!m_pFileAvail->IsDataAvail(offset, size)) {
    pHints->AddSegment(offset, size);
    return FALSE;
  }
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckHintTables(IFX_DownloadHints* pHints) {
  CPDF_Dictionary* pDict = m_pLinearized->GetDict();
  if (!pDict) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  if (!pDict->KeyExist("H") || !pDict->KeyExist("O") || !pDict->KeyExist("N")) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  int nPageCount = pDict->GetElementValue("N")->GetInteger();
  if (nPageCount <= 1) {
    m_docStatus = PDF_DATAAVAIL_DONE;
    return TRUE;
  }
  CPDF_Array* pHintStreamRange = pDict->GetArray("H");
  FX_FILESIZE szHSStart =
      pHintStreamRange->GetElementValue(0)
          ? pHintStreamRange->GetElementValue(0)->GetInteger()
          : 0;
  FX_FILESIZE szHSLength =
      pHintStreamRange->GetElementValue(1)
          ? pHintStreamRange->GetElementValue(1)->GetInteger()
          : 0;
  if (szHSStart < 0 || szHSLength <= 0) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  if (!IsDataAvail(szHSStart, szHSLength, pHints)) {
    return FALSE;
  }
  m_syntaxParser.InitParser(m_pFileRead, m_dwHeaderOffset);
  std::unique_ptr<CPDF_HintTables> pHintTables(
      new CPDF_HintTables(this, pDict));
  std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>> pHintStream(
      ParseIndirectObjectAt(szHSStart, 0));
  CPDF_Stream* pStream = ToStream(pHintStream.get());
  if (pStream && pHintTables->LoadHintStream(pStream))
    m_pHintTables = std::move(pHintTables);

  m_docStatus = PDF_DATAAVAIL_DONE;
  return TRUE;
}
CPDF_Object* CPDF_DataAvail::ParseIndirectObjectAt(
    FX_FILESIZE pos,
    FX_DWORD objnum,
    CPDF_IndirectObjectHolder* pObjList) {
  FX_FILESIZE SavedPos = m_syntaxParser.SavePos();
  m_syntaxParser.RestorePos(pos);
  bool bIsNumber;
  CFX_ByteString word = m_syntaxParser.GetNextWord(&bIsNumber);
  if (!bIsNumber)
    return nullptr;

  FX_DWORD parser_objnum = FXSYS_atoi(word);
  if (objnum && parser_objnum != objnum)
    return nullptr;

  word = m_syntaxParser.GetNextWord(&bIsNumber);
  if (!bIsNumber)
    return nullptr;

  FX_DWORD gennum = FXSYS_atoi(word);
  if (m_syntaxParser.GetKeyword() != "obj") {
    m_syntaxParser.RestorePos(SavedPos);
    return nullptr;
  }
  CPDF_Object* pObj =
      m_syntaxParser.GetObject(pObjList, parser_objnum, gennum, nullptr, true);
  m_syntaxParser.RestorePos(SavedPos);
  return pObj;
}
IPDF_DataAvail::DocLinearizationStatus CPDF_DataAvail::IsLinearizedPDF() {
  FX_DWORD req_size = 1024;
  if (!m_pFileAvail->IsDataAvail(0, req_size)) {
    return LinearizationUnknown;
  }
  if (!m_pFileRead) {
    return NotLinearized;
  }
  FX_FILESIZE dwSize = m_pFileRead->GetSize();
  if (dwSize < (FX_FILESIZE)req_size) {
    return LinearizationUnknown;
  }
  uint8_t buffer[1024];
  m_pFileRead->ReadBlock(buffer, 0, req_size);
  if (IsLinearizedFile(buffer, req_size)) {
    return Linearized;
  }
  return NotLinearized;
}
FX_BOOL CPDF_DataAvail::IsLinearizedFile(uint8_t* pData, FX_DWORD dwLen) {
  ScopedFileStream file(FX_CreateMemoryStream(pData, (size_t)dwLen, FALSE));
  int32_t offset = GetHeaderOffset(file.get());
  if (offset == -1) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  m_dwHeaderOffset = offset;
  m_syntaxParser.InitParser(file.get(), offset);
  m_syntaxParser.RestorePos(m_syntaxParser.m_HeaderOffset + 9);
  bool bNumber;
  CFX_ByteString wordObjNum = m_syntaxParser.GetNextWord(&bNumber);
  if (!bNumber)
    return FALSE;

  FX_DWORD objnum = FXSYS_atoi(wordObjNum);
  if (m_pLinearized) {
    m_pLinearized->Release();
    m_pLinearized = NULL;
  }
  m_pLinearized =
      ParseIndirectObjectAt(m_syntaxParser.m_HeaderOffset + 9, objnum);
  if (!m_pLinearized) {
    return FALSE;
  }

  CPDF_Dictionary* pDict = m_pLinearized->GetDict();
  if (pDict && pDict->GetElement("Linearized")) {
    CPDF_Object* pLen = pDict->GetElement("L");
    if (!pLen) {
      return FALSE;
    }
    if ((FX_FILESIZE)pLen->GetInteger() != m_pFileRead->GetSize()) {
      return FALSE;
    }
    m_bLinearized = TRUE;

    if (CPDF_Number* pNo = ToNumber(pDict->GetElement("P")))
      m_dwFirstPageNo = pNo->GetInteger();

    return TRUE;
  }
  return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckEnd(IFX_DownloadHints* pHints) {
  FX_DWORD req_pos = (FX_DWORD)(m_dwFileLen > 1024 ? m_dwFileLen - 1024 : 0);
  FX_DWORD dwSize = (FX_DWORD)(m_dwFileLen - req_pos);
  if (m_pFileAvail->IsDataAvail(req_pos, dwSize)) {
    uint8_t buffer[1024];
    m_pFileRead->ReadBlock(buffer, req_pos, dwSize);
    ScopedFileStream file(FX_CreateMemoryStream(buffer, (size_t)dwSize, FALSE));
    m_syntaxParser.InitParser(file.get(), 0);
    m_syntaxParser.RestorePos(dwSize - 1);
    if (m_syntaxParser.SearchWord("startxref", TRUE, FALSE, dwSize)) {
      m_syntaxParser.GetNextWord(nullptr);
      bool bNumber;
      CFX_ByteString xrefpos_str = m_syntaxParser.GetNextWord(&bNumber);
      if (!bNumber) {
        m_docStatus = PDF_DATAAVAIL_ERROR;
        return FALSE;
      }
      m_dwXRefOffset = (FX_FILESIZE)FXSYS_atoi64(xrefpos_str);
      if (!m_dwXRefOffset || m_dwXRefOffset > m_dwFileLen) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        return TRUE;
      }
      m_dwLastXRefOffset = m_dwXRefOffset;
      SetStartOffset(m_dwXRefOffset);
      m_docStatus = PDF_DATAAVAIL_CROSSREF;
      return TRUE;
    }
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return TRUE;
  }
  pHints->AddSegment(req_pos, dwSize);
  return FALSE;
}
int32_t CPDF_DataAvail::CheckCrossRefStream(IFX_DownloadHints* pHints,
                                            FX_FILESIZE& xref_offset) {
  xref_offset = 0;
  FX_DWORD req_size =
      (FX_DWORD)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
  if (m_pFileAvail->IsDataAvail(m_Pos, req_size)) {
    int32_t iSize = (int32_t)(m_Pos + req_size - m_dwCurrentXRefSteam);
    CFX_BinaryBuf buf(iSize);
    uint8_t* pBuf = buf.GetBuffer();
    m_pFileRead->ReadBlock(pBuf, m_dwCurrentXRefSteam, iSize);
    ScopedFileStream file(FX_CreateMemoryStream(pBuf, (size_t)iSize, FALSE));
    m_parser.m_Syntax.InitParser(file.get(), 0);
    bool bNumber;
    CFX_ByteString objnum = m_parser.m_Syntax.GetNextWord(&bNumber);
    if (!bNumber)
      return -1;

    FX_DWORD objNum = FXSYS_atoi(objnum);
    CPDF_Object* pObj = m_parser.ParseIndirectObjectAt(NULL, 0, objNum, NULL);
    if (!pObj) {
      m_Pos += m_parser.m_Syntax.SavePos();
      return 0;
    }
    CPDF_Dictionary* pDict = pObj->GetDict();
    CPDF_Name* pName = ToName(pDict ? pDict->GetElement("Type") : nullptr);
    if (pName) {
      if (pName->GetString() == "XRef") {
        m_Pos += m_parser.m_Syntax.SavePos();
        xref_offset = pObj->GetDict()->GetInteger("Prev");
        pObj->Release();
        return 1;
      }
    }
    pObj->Release();
    return -1;
  }
  pHints->AddSegment(m_Pos, req_size);
  return 0;
}
inline void CPDF_DataAvail::SetStartOffset(FX_FILESIZE dwOffset) {
  m_Pos = dwOffset;
}

FX_BOOL CPDF_DataAvail::GetNextToken(CFX_ByteString& token) {
  uint8_t ch;
  if (!GetNextChar(ch))
    return FALSE;

  while (1) {
    while (PDFCharIsWhitespace(ch)) {
      if (!GetNextChar(ch))
        return FALSE;
    }

    if (ch != '%')
      break;

    while (1) {
      if (!GetNextChar(ch))
        return FALSE;
      if (PDFCharIsLineEnding(ch))
        break;
    }
  }

  uint8_t buffer[256];
  FX_DWORD index = 0;
  if (PDFCharIsDelimiter(ch)) {
    buffer[index++] = ch;
    if (ch == '/') {
      while (1) {
        if (!GetNextChar(ch))
          return FALSE;

        if (!PDFCharIsOther(ch) && !PDFCharIsNumeric(ch)) {
          m_Pos--;
          CFX_ByteString ret(buffer, index);
          token = ret;
          return TRUE;
        }

        if (index < sizeof(buffer))
          buffer[index++] = ch;
      }
    } else if (ch == '<') {
      if (!GetNextChar(ch))
        return FALSE;

      if (ch == '<')
        buffer[index++] = ch;
      else
        m_Pos--;
    } else if (ch == '>') {
      if (!GetNextChar(ch))
        return FALSE;

      if (ch == '>')
        buffer[index++] = ch;
      else
        m_Pos--;
    }

    CFX_ByteString ret(buffer, index);
    token = ret;
    return TRUE;
  }

  while (1) {
    if (index < sizeof(buffer))
      buffer[index++] = ch;

    if (!GetNextChar(ch))
      return FALSE;

    if (PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch)) {
      m_Pos--;
      break;
    }
  }

  token = CFX_ByteString(buffer, index);
  return TRUE;
}

FX_BOOL CPDF_DataAvail::GetNextChar(uint8_t& ch) {
  FX_FILESIZE pos = m_Pos;
  if (pos >= m_dwFileLen) {
    return FALSE;
  }
  if (m_bufferOffset >= pos ||
      (FX_FILESIZE)(m_bufferOffset + m_bufferSize) <= pos) {
    FX_FILESIZE read_pos = pos;
    FX_DWORD read_size = 512;
    if ((FX_FILESIZE)read_size > m_dwFileLen) {
      read_size = (FX_DWORD)m_dwFileLen;
    }
    if ((FX_FILESIZE)(read_pos + read_size) > m_dwFileLen) {
      read_pos = m_dwFileLen - read_size;
    }
    if (!m_pFileRead->ReadBlock(m_bufferData, read_pos, read_size)) {
      return FALSE;
    }
    m_bufferOffset = read_pos;
    m_bufferSize = read_size;
  }
  ch = m_bufferData[pos - m_bufferOffset];
  m_Pos++;
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckCrossRefItem(IFX_DownloadHints* pHints) {
  int32_t iSize = 0;
  CFX_ByteString token;
  while (1) {
    if (!GetNextToken(token)) {
      iSize = (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
      pHints->AddSegment(m_Pos, iSize);
      return FALSE;
    }
    if (token == "trailer") {
      m_dwTrailerOffset = m_Pos;
      m_docStatus = PDF_DATAAVAIL_TRAILER;
      return TRUE;
    }
  }
}
FX_BOOL CPDF_DataAvail::CheckAllCrossRefStream(IFX_DownloadHints* pHints) {
  FX_FILESIZE xref_offset = 0;
  int32_t nRet = CheckCrossRefStream(pHints, xref_offset);
  if (nRet == 1) {
    if (!xref_offset) {
      m_docStatus = PDF_DATAAVAIL_LOADALLCROSSREF;
    } else {
      m_dwCurrentXRefSteam = xref_offset;
      m_Pos = xref_offset;
    }
    return TRUE;
  }
  if (nRet == -1) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
  }
  return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckCrossRef(IFX_DownloadHints* pHints) {
  int32_t iSize = 0;
  CFX_ByteString token;
  if (!GetNextToken(token)) {
    iSize = (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
    pHints->AddSegment(m_Pos, iSize);
    return FALSE;
  }
  if (token == "xref") {
    m_CrossOffset.InsertAt(0, m_dwXRefOffset);
    while (1) {
      if (!GetNextToken(token)) {
        iSize =
            (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
        pHints->AddSegment(m_Pos, iSize);
        m_docStatus = PDF_DATAAVAIL_CROSSREF_ITEM;
        return FALSE;
      }
      if (token == "trailer") {
        m_dwTrailerOffset = m_Pos;
        m_docStatus = PDF_DATAAVAIL_TRAILER;
        return TRUE;
      }
    }
  } else {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CPDF_DataAvail::CheckTrailerAppend(IFX_DownloadHints* pHints) {
  if (m_Pos < m_dwFileLen) {
    FX_FILESIZE dwAppendPos = m_Pos + m_syntaxParser.SavePos();
    int32_t iSize = (int32_t)(
        dwAppendPos + 512 > m_dwFileLen ? m_dwFileLen - dwAppendPos : 512);
    if (!m_pFileAvail->IsDataAvail(dwAppendPos, iSize)) {
      pHints->AddSegment(dwAppendPos, iSize);
      return FALSE;
    }
  }
  if (m_dwPrevXRefOffset) {
    SetStartOffset(m_dwPrevXRefOffset);
    m_docStatus = PDF_DATAAVAIL_CROSSREF;
  } else {
    m_docStatus = PDF_DATAAVAIL_LOADALLCROSSREF;
  }
  return TRUE;
}

FX_BOOL CPDF_DataAvail::CheckTrailer(IFX_DownloadHints* pHints) {
  int32_t iTrailerSize =
      (int32_t)(m_Pos + 512 > m_dwFileLen ? m_dwFileLen - m_Pos : 512);
  if (m_pFileAvail->IsDataAvail(m_Pos, iTrailerSize)) {
    int32_t iSize = (int32_t)(m_Pos + iTrailerSize - m_dwTrailerOffset);
    CFX_BinaryBuf buf(iSize);
    uint8_t* pBuf = buf.GetBuffer();
    if (!pBuf) {
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return FALSE;
    }
    if (!m_pFileRead->ReadBlock(pBuf, m_dwTrailerOffset, iSize)) {
      return FALSE;
    }
    ScopedFileStream file(FX_CreateMemoryStream(pBuf, (size_t)iSize, FALSE));
    m_syntaxParser.InitParser(file.get(), 0);
    std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>> pTrailer(
        m_syntaxParser.GetObject(nullptr, 0, 0, nullptr, true));
    if (!pTrailer) {
      m_Pos += m_syntaxParser.SavePos();
      pHints->AddSegment(m_Pos, iTrailerSize);
      return FALSE;
    }
    if (!pTrailer->IsDictionary())
      return FALSE;

    CPDF_Dictionary* pTrailerDict = pTrailer->GetDict();
    CPDF_Object* pEncrypt = pTrailerDict->GetElement("Encrypt");
    if (ToReference(pEncrypt)) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return TRUE;
    }

    FX_DWORD xrefpos = GetDirectInteger(pTrailerDict, "Prev");
    if (xrefpos) {
      m_dwPrevXRefOffset = GetDirectInteger(pTrailerDict, "XRefStm");
      if (m_dwPrevXRefOffset) {
        m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      } else {
        m_dwPrevXRefOffset = xrefpos;
        if (m_dwPrevXRefOffset >= m_dwFileLen) {
          m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
        } else {
          SetStartOffset(m_dwPrevXRefOffset);
          m_docStatus = PDF_DATAAVAIL_TRAILER_APPEND;
        }
      }
      return TRUE;
    }
    m_dwPrevXRefOffset = 0;
    m_docStatus = PDF_DATAAVAIL_TRAILER_APPEND;
    return TRUE;
  }
  pHints->AddSegment(m_Pos, iTrailerSize);
  return FALSE;
}

FX_BOOL CPDF_DataAvail::CheckPage(int32_t iPage, IFX_DownloadHints* pHints) {
  while (TRUE) {
    switch (m_docStatus) {
      case PDF_DATAAVAIL_PAGETREE:
        if (!LoadDocPages(pHints)) {
          return FALSE;
        }
        break;
      case PDF_DATAAVAIL_PAGE:
        if (!LoadDocPage(iPage, pHints)) {
          return FALSE;
        }
        break;
      case PDF_DATAAVAIL_ERROR:
        return LoadAllFile(pHints);
      default:
        m_bPagesTreeLoad = TRUE;
        m_bPagesLoad = TRUE;
        m_bCurPageDictLoadOK = TRUE;
        m_docStatus = PDF_DATAAVAIL_PAGE;
        return TRUE;
    }
  }
}
FX_BOOL CPDF_DataAvail::CheckArrayPageNode(FX_DWORD dwPageNo,
                                           CPDF_PageNode* pPageNode,
                                           IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  CPDF_Object* pPages = GetObject(dwPageNo, pHints, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  if (!pPages) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return FALSE;
    }
    return FALSE;
  }

  CPDF_Array* pArray = pPages->AsArray();
  if (!pArray) {
    pPages->Release();
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }

  pPageNode->m_type = PDF_PAGENODE_PAGES;
  for (FX_DWORD i = 0; i < pArray->GetCount(); ++i) {
    CPDF_Reference* pKid = ToReference(pArray->GetElement(i));
    if (!pKid)
      continue;

    CPDF_PageNode* pNode = new CPDF_PageNode();
    pPageNode->m_childNode.Add(pNode);
    pNode->m_dwPageNo = pKid->GetRefObjNum();
  }
  pPages->Release();
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckUnkownPageNode(FX_DWORD dwPageNo,
                                            CPDF_PageNode* pPageNode,
                                            IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  CPDF_Object* pPage = GetObject(dwPageNo, pHints, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  if (!pPage) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return FALSE;
    }
    return FALSE;
  }
  if (pPage->IsArray()) {
    pPageNode->m_dwPageNo = dwPageNo;
    pPageNode->m_type = PDF_PAGENODE_ARRAY;
    pPage->Release();
    return TRUE;
  }
  if (!pPage->IsDictionary()) {
    pPage->Release();
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  pPageNode->m_dwPageNo = dwPageNo;
  CPDF_Dictionary* pDict = pPage->GetDict();
  CFX_ByteString type = pDict->GetString("Type");
  if (type == "Pages") {
    pPageNode->m_type = PDF_PAGENODE_PAGES;
    CPDF_Object* pKids = pDict->GetElement("Kids");
    if (!pKids) {
      m_docStatus = PDF_DATAAVAIL_PAGE;
      return TRUE;
    }
    switch (pKids->GetType()) {
      case PDFOBJ_REFERENCE: {
        CPDF_Reference* pKid = pKids->AsReference();
        CPDF_PageNode* pNode = new CPDF_PageNode();
        pPageNode->m_childNode.Add(pNode);
        pNode->m_dwPageNo = pKid->GetRefObjNum();
      } break;
      case PDFOBJ_ARRAY: {
        CPDF_Array* pKidsArray = pKids->AsArray();
        for (FX_DWORD i = 0; i < pKidsArray->GetCount(); ++i) {
          CPDF_Reference* pKid = ToReference(pKidsArray->GetElement(i));
          if (!pKid)
            continue;

          CPDF_PageNode* pNode = new CPDF_PageNode();
          pPageNode->m_childNode.Add(pNode);
          pNode->m_dwPageNo = pKid->GetRefObjNum();
        }
      } break;
      default:
        break;
    }
  } else if (type == "Page") {
    pPageNode->m_type = PDF_PAGENODE_PAGE;
  } else {
    pPage->Release();
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  pPage->Release();
  return TRUE;
}
FX_BOOL CPDF_DataAvail::CheckPageNode(CPDF_PageNode& pageNodes,
                                      int32_t iPage,
                                      int32_t& iCount,
                                      IFX_DownloadHints* pHints,
                                      int level) {
  if (level >= kMaxPageRecursionDepth) {
    return FALSE;
  }
  int32_t iSize = pageNodes.m_childNode.GetSize();
  if (iSize <= 0 || iPage >= iSize) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  for (int32_t i = 0; i < iSize; ++i) {
    CPDF_PageNode* pNode = pageNodes.m_childNode.GetAt(i);
    if (!pNode) {
      continue;
    }
    switch (pNode->m_type) {
      case PDF_PAGENODE_UNKOWN:
        if (!CheckUnkownPageNode(pNode->m_dwPageNo, pNode, pHints)) {
          return FALSE;
        }
        --i;
        break;
      case PDF_PAGENODE_PAGE:
        iCount++;
        if (iPage == iCount && m_pDocument) {
          m_pDocument->m_PageList.SetAt(iPage, pNode->m_dwPageNo);
        }
        break;
      case PDF_PAGENODE_PAGES:
        if (!CheckPageNode(*pNode, iPage, iCount, pHints, level + 1)) {
          return FALSE;
        }
        break;
      case PDF_PAGENODE_ARRAY:
        if (!CheckArrayPageNode(pNode->m_dwPageNo, pNode, pHints)) {
          return FALSE;
        }
        --i;
        break;
    }
    if (iPage == iCount) {
      m_docStatus = PDF_DATAAVAIL_DONE;
      return TRUE;
    }
  }
  return TRUE;
}
FX_BOOL CPDF_DataAvail::LoadDocPage(int32_t iPage, IFX_DownloadHints* pHints) {
  if (m_pDocument->GetPageCount() <= iPage ||
      m_pDocument->m_PageList.GetAt(iPage)) {
    m_docStatus = PDF_DATAAVAIL_DONE;
    return TRUE;
  }
  if (m_pageNodes.m_type == PDF_PAGENODE_PAGE) {
    if (iPage == 0) {
      m_docStatus = PDF_DATAAVAIL_DONE;
      return TRUE;
    }
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return TRUE;
  }
  int32_t iCount = -1;
  return CheckPageNode(m_pageNodes, iPage, iCount, pHints, 0);
}
FX_BOOL CPDF_DataAvail::CheckPageCount(IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  CPDF_Object* pPages = GetObject(m_PagesObjNum, pHints, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  if (!pPages) {
    return FALSE;
  }
  CPDF_Dictionary* pPagesDict = pPages->GetDict();
  if (!pPagesDict) {
    pPages->Release();
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }
  if (!pPagesDict->KeyExist("Kids")) {
    pPages->Release();
    return TRUE;
  }
  int count = pPagesDict->GetInteger("Count");
  if (count > 0) {
    pPages->Release();
    return TRUE;
  }
  pPages->Release();
  return FALSE;
}
FX_BOOL CPDF_DataAvail::LoadDocPages(IFX_DownloadHints* pHints) {
  if (!CheckUnkownPageNode(m_PagesObjNum, &m_pageNodes, pHints)) {
    return FALSE;
  }
  if (CheckPageCount(pHints)) {
    m_docStatus = PDF_DATAAVAIL_PAGE;
    return TRUE;
  }
  m_bTotalLoadPageTree = TRUE;
  return FALSE;
}
FX_BOOL CPDF_DataAvail::LoadPages(IFX_DownloadHints* pHints) {
  while (!m_bPagesTreeLoad) {
    if (!CheckPageStatus(pHints)) {
      return FALSE;
    }
  }
  if (m_bPagesLoad) {
    return TRUE;
  }
  m_pDocument->LoadPages();
  return FALSE;
}
IPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckLinearizedData(
    IFX_DownloadHints* pHints) {
  if (m_bLinearedDataOK) {
    return DataAvailable;
  }

  if (!m_bMainXRefLoadTried) {
    FX_SAFE_DWORD data_size = m_dwFileLen;
    data_size -= m_dwLastXRefOffset;
    if (!data_size.IsValid()) {
      return DataError;
    }
    if (!m_pFileAvail->IsDataAvail(m_dwLastXRefOffset,
                                   data_size.ValueOrDie())) {
      pHints->AddSegment(m_dwLastXRefOffset, data_size.ValueOrDie());
      return DataNotAvailable;
    }
    FX_DWORD dwRet = m_pDocument->GetParser()->LoadLinearizedMainXRefTable();
    m_bMainXRefLoadTried = TRUE;
    if (dwRet != PDFPARSE_ERROR_SUCCESS) {
      return DataError;
    }
    if (!PreparePageItem()) {
      return DataNotAvailable;
    }
    m_bMainXRefLoadedOK = TRUE;
    m_bLinearedDataOK = TRUE;
  }

  return m_bLinearedDataOK ? DataAvailable : DataNotAvailable;
}
FX_BOOL CPDF_DataAvail::CheckPageAnnots(int32_t iPage,
                                        IFX_DownloadHints* pHints) {
  if (!m_objs_array.GetSize()) {
    m_objs_array.RemoveAll();
    m_ObjectSet.clear();
    CPDF_Dictionary* pPageDict = m_pDocument->GetPage(iPage);
    if (!pPageDict) {
      return TRUE;
    }
    CPDF_Object* pAnnots = pPageDict->GetElement("Annots");
    if (!pAnnots) {
      return TRUE;
    }
    CFX_ArrayTemplate<CPDF_Object*> obj_array;
    obj_array.Add(pAnnots);
    FX_BOOL bRet = IsObjectsAvail(obj_array, FALSE, pHints, m_objs_array);
    if (bRet) {
      m_objs_array.RemoveAll();
    }
    return bRet;
  }
  CFX_ArrayTemplate<CPDF_Object*> new_objs_array;
  FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
  m_objs_array.RemoveAll();
  if (!bRet) {
    m_objs_array.Append(new_objs_array);
  }
  return bRet;
}
IPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckLinearizedFirstPage(
    int32_t iPage,
    IFX_DownloadHints* pHints) {
  if (!m_bAnnotsLoad) {
    if (!CheckPageAnnots(iPage, pHints)) {
      return DataNotAvailable;
    }
    m_bAnnotsLoad = TRUE;
  }

  DocAvailStatus nRet = CheckLinearizedData(pHints);
  if (nRet == DataAvailable)
    m_bPageLoadedOK = FALSE;
  return nRet;
}
FX_BOOL CPDF_DataAvail::HaveResourceAncestor(CPDF_Dictionary* pDict) {
  CFX_AutoRestorer<int> restorer(&s_CurrentDataAvailRecursionDepth);
  if (++s_CurrentDataAvailRecursionDepth > kMaxDataAvailRecursionDepth) {
    return FALSE;
  }
  CPDF_Object* pParent = pDict->GetElement("Parent");
  if (!pParent) {
    return FALSE;
  }
  CPDF_Dictionary* pParentDict = pParent->GetDict();
  if (!pParentDict) {
    return FALSE;
  }
  CPDF_Object* pRet = pParentDict->GetElement("Resources");
  if (pRet) {
    m_pPageResource = pRet;
    return TRUE;
  }
  return HaveResourceAncestor(pParentDict);
}
IPDF_DataAvail::DocAvailStatus CPDF_DataAvail::IsPageAvail(
    int32_t iPage,
    IFX_DownloadHints* pHints) {
  if (!m_pDocument) {
    return DataError;
  }
  if (IsFirstCheck(iPage)) {
    m_bCurPageDictLoadOK = FALSE;
    m_bPageLoadedOK = FALSE;
    m_bAnnotsLoad = FALSE;
    m_bNeedDownLoadResource = FALSE;
    m_objs_array.RemoveAll();
    m_ObjectSet.clear();
  }
  if (pdfium::ContainsKey(m_pagesLoadState, iPage))
    return DataAvailable;

  if (m_bLinearized) {
    if ((FX_DWORD)iPage == m_dwFirstPageNo) {
      DocAvailStatus nRet = CheckLinearizedFirstPage(iPage, pHints);
      if (nRet == DataAvailable)
        m_pagesLoadState.insert(iPage);
      return nRet;
    }
    DocAvailStatus nResult = CheckLinearizedData(pHints);
    if (nResult != DataAvailable) {
      return nResult;
    }
    if (m_pHintTables) {
      nResult = m_pHintTables->CheckPage(iPage, pHints);
      if (nResult != DataAvailable)
        return nResult;
      m_pagesLoadState.insert(iPage);
      return DataAvailable;
    }
    if (m_bMainXRefLoadedOK) {
      if (m_bTotalLoadPageTree) {
        if (!LoadPages(pHints)) {
          return DataNotAvailable;
        }
      } else {
        if (!m_bCurPageDictLoadOK && !CheckPage(iPage, pHints)) {
          return DataNotAvailable;
        }
      }
    } else {
      if (!LoadAllFile(pHints)) {
        return DataNotAvailable;
      }
      ((CPDF_Parser*)m_pDocument->GetParser())->RebuildCrossRef();
      ResetFirstCheck(iPage);
      return DataAvailable;
    }
  } else {
    if (!m_bTotalLoadPageTree) {
      if (!m_bCurPageDictLoadOK && !CheckPage(iPage, pHints)) {
        return DataNotAvailable;
      }
    }
  }
  if (m_bHaveAcroForm && !m_bAcroFormLoad) {
    if (!CheckAcroFormSubObject(pHints)) {
      return DataNotAvailable;
    }
    m_bAcroFormLoad = TRUE;
  }
  if (!m_bPageLoadedOK) {
    if (!m_objs_array.GetSize()) {
      m_objs_array.RemoveAll();
      m_ObjectSet.clear();
      m_pPageDict = m_pDocument->GetPage(iPage);
      if (!m_pPageDict) {
        ResetFirstCheck(iPage);
        return DataAvailable;
      }
      CFX_ArrayTemplate<CPDF_Object*> obj_array;
      obj_array.Add(m_pPageDict);
      FX_BOOL bRet = IsObjectsAvail(obj_array, TRUE, pHints, m_objs_array);
      if (!bRet)
        return DataNotAvailable;

      m_objs_array.RemoveAll();
    } else {
      CFX_ArrayTemplate<CPDF_Object*> new_objs_array;
      FX_BOOL bRet =
          IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
      m_objs_array.RemoveAll();
      if (!bRet) {
        m_objs_array.Append(new_objs_array);
        return DataNotAvailable;
      }
    }
    m_bPageLoadedOK = TRUE;
  }

  if (!m_bAnnotsLoad) {
    if (!CheckPageAnnots(iPage, pHints)) {
      return DataNotAvailable;
    }
    m_bAnnotsLoad = TRUE;
  }

  if (m_pPageDict && !m_bNeedDownLoadResource) {
    m_pPageResource = m_pPageDict->GetElement("Resources");
    if (!m_pPageResource) {
      m_bNeedDownLoadResource = HaveResourceAncestor(m_pPageDict);
    } else {
      m_bNeedDownLoadResource = TRUE;
    }
  }
  if (m_bNeedDownLoadResource) {
    FX_BOOL bRet = CheckResources(pHints);
    if (!bRet) {
      return DataNotAvailable;
    }
    m_bNeedDownLoadResource = FALSE;
  }
  m_bPageLoadedOK = FALSE;
  m_bAnnotsLoad = FALSE;
  m_bCurPageDictLoadOK = FALSE;
  ResetFirstCheck(iPage);
  m_pagesLoadState.insert(iPage);
  return DataAvailable;
}
FX_BOOL CPDF_DataAvail::CheckResources(IFX_DownloadHints* pHints) {
  if (!m_objs_array.GetSize()) {
    m_objs_array.RemoveAll();
    CFX_ArrayTemplate<CPDF_Object*> obj_array;
    obj_array.Add(m_pPageResource);
    FX_BOOL bRet = IsObjectsAvail(obj_array, TRUE, pHints, m_objs_array);
    if (bRet) {
      m_objs_array.RemoveAll();
    }
    return bRet;
  }
  CFX_ArrayTemplate<CPDF_Object*> new_objs_array;
  FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
  m_objs_array.RemoveAll();
  if (!bRet) {
    m_objs_array.Append(new_objs_array);
  }
  return bRet;
}
void CPDF_DataAvail::GetLinearizedMainXRefInfo(FX_FILESIZE* pPos,
                                               FX_DWORD* pSize) {
  if (pPos) {
    *pPos = m_dwLastXRefOffset;
  }
  if (pSize) {
    *pSize = (FX_DWORD)(m_dwFileLen - m_dwLastXRefOffset);
  }
}
int CPDF_DataAvail::GetPageCount() const {
  if (m_pLinearized) {
    CPDF_Dictionary* pDict = m_pLinearized->GetDict();
    CPDF_Object* pObj = pDict ? pDict->GetElementValue("N") : nullptr;
    return pObj ? pObj->GetInteger() : 0;
  }
  return m_pDocument ? m_pDocument->GetPageCount() : 0;
}
CPDF_Dictionary* CPDF_DataAvail::GetPage(int index) {
  if (!m_pDocument || index < 0 || index >= this->GetPageCount()) {
    return nullptr;
  }
  if (m_pLinearized) {
    CPDF_Dictionary* pDict = m_pLinearized->GetDict();
    CPDF_Object* pObj = pDict ? pDict->GetElementValue("P") : nullptr;
    int pageNum = pObj ? pObj->GetInteger() : 0;
    if (m_pHintTables && index != pageNum) {
      FX_FILESIZE szPageStartPos = 0;
      FX_FILESIZE szPageLength = 0;
      FX_DWORD dwObjNum = 0;
      FX_BOOL bPagePosGot = m_pHintTables->GetPagePos(index, szPageStartPos,
                                                      szPageLength, dwObjNum);
      if (!bPagePosGot) {
        return nullptr;
      }
      m_syntaxParser.InitParser(m_pFileRead, (FX_DWORD)szPageStartPos);
      CPDF_Object* pPageDict = ParseIndirectObjectAt(0, dwObjNum, m_pDocument);
      if (!pPageDict) {
        return nullptr;
      }
      if (!m_pDocument->InsertIndirectObject(dwObjNum, pPageDict))
        return nullptr;
      return pPageDict->GetDict();
    }
  }
  return m_pDocument->GetPage(index);
}
IPDF_DataAvail::DocFormStatus CPDF_DataAvail::IsFormAvail(
    IFX_DownloadHints* pHints) {
  if (!m_pDocument) {
    return FormAvailable;
  }
  if (!m_bLinearizedFormParamLoad) {
    CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
    if (!pRoot) {
      return FormAvailable;
    }
    CPDF_Object* pAcroForm = pRoot->GetElement("AcroForm");
    if (!pAcroForm) {
      return FormNotExist;
    }
    DocAvailStatus nDocStatus = CheckLinearizedData(pHints);
    if (nDocStatus == DataError)
      return FormError;
    if (nDocStatus == DataNotAvailable)
      return FormNotAvailable;

    if (!m_objs_array.GetSize()) {
      m_objs_array.Add(pAcroForm->GetDict());
    }
    m_bLinearizedFormParamLoad = TRUE;
  }
  CFX_ArrayTemplate<CPDF_Object*> new_objs_array;
  FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
  m_objs_array.RemoveAll();
  if (!bRet) {
    m_objs_array.Append(new_objs_array);
    return FormNotAvailable;
  }
  return FormAvailable;
}

CPDF_PageNode::~CPDF_PageNode() {
  for (int32_t i = 0; i < m_childNode.GetSize(); ++i) {
    delete m_childNode[i];
  }
  m_childNode.RemoveAll();
}
CPDF_HintTables::~CPDF_HintTables() {
  m_dwDeltaNObjsArray.RemoveAll();
  m_dwNSharedObjsArray.RemoveAll();
  m_dwSharedObjNumArray.RemoveAll();
  m_dwIdentifierArray.RemoveAll();
  m_szPageOffsetArray.RemoveAll();
  m_szSharedObjOffsetArray.RemoveAll();
}
FX_DWORD CPDF_HintTables::GetItemLength(int index,
                                        const CFX_FileSizeArray& szArray) {
  if (index < 0 || szArray.GetSize() < 2 || index > szArray.GetSize() - 2 ||
      szArray[index] > szArray[index + 1])
    return 0;
  return szArray[index + 1] - szArray[index];
}
FX_BOOL CPDF_HintTables::ReadPageHintTable(CFX_BitStream* hStream) {
  if (!hStream || hStream->IsEOF())
    return FALSE;
  int nStreamOffset = ReadPrimaryHintStreamOffset();
  int nStreamLen = ReadPrimaryHintStreamLength();
  if (nStreamOffset < 0 || nStreamLen < 1)
    return FALSE;

  const FX_DWORD kHeaderSize = 288;
  if (hStream->BitsRemaining() < kHeaderSize)
    return FALSE;
  // Item 1: The least number of objects in a page.
  FX_DWORD dwObjLeastNum = hStream->GetBits(32);
  // Item 2: The location of the first page's page object.
  FX_DWORD dwFirstObjLoc = hStream->GetBits(32);
  if (dwFirstObjLoc > nStreamOffset) {
    FX_SAFE_DWORD safeLoc = pdfium::base::checked_cast<FX_DWORD>(nStreamLen);
    safeLoc += dwFirstObjLoc;
    if (!safeLoc.IsValid())
      return FALSE;
    m_szFirstPageObjOffset =
        pdfium::base::checked_cast<FX_FILESIZE>(safeLoc.ValueOrDie());
  } else {
    m_szFirstPageObjOffset =
        pdfium::base::checked_cast<FX_FILESIZE>(dwFirstObjLoc);
  }
  // Item 3: The number of bits needed to represent the difference
  // between the greatest and least number of objects in a page.
  FX_DWORD dwDeltaObjectsBits = hStream->GetBits(16);
  // Item 4: The least length of a page in bytes.
  FX_DWORD dwPageLeastLen = hStream->GetBits(32);
  // Item 5: The number of bits needed to represent the difference
  // between the greatest and least length of a page, in bytes.
  FX_DWORD dwDeltaPageLenBits = hStream->GetBits(16);
  // Skip Item 6, 7, 8, 9 total 96 bits.
  hStream->SkipBits(96);
  // Item 10: The number of bits needed to represent the greatest
  // number of shared object references.
  FX_DWORD dwSharedObjBits = hStream->GetBits(16);
  // Item 11: The number of bits needed to represent the numerically
  // greatest shared object identifier used by the pages.
  FX_DWORD dwSharedIdBits = hStream->GetBits(16);
  // Item 12: The number of bits needed to represent the numerator of
  // the fractional position for each shared object reference. For each
  // shared object referenced from a page, there is an indication of
  // where in the page's content stream the object is first referenced.
  FX_DWORD dwSharedNumeratorBits = hStream->GetBits(16);
  // Item 13: Skip Item 13 which has 16 bits.
  hStream->SkipBits(16);
  CPDF_Object* pPageNum = m_pLinearizedDict->GetElementValue("N");
  int nPages = pPageNum ? pPageNum->GetInteger() : 0;
  if (nPages < 1)
    return FALSE;

  FX_SAFE_DWORD required_bits = dwDeltaObjectsBits;
  required_bits *= pdfium::base::checked_cast<FX_DWORD>(nPages);
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;
  for (int i = 0; i < nPages; ++i) {
    FX_SAFE_DWORD safeDeltaObj = hStream->GetBits(dwDeltaObjectsBits);
    safeDeltaObj += dwObjLeastNum;
    if (!safeDeltaObj.IsValid())
      return FALSE;
    m_dwDeltaNObjsArray.Add(safeDeltaObj.ValueOrDie());
  }
  hStream->ByteAlign();

  required_bits = dwDeltaPageLenBits;
  required_bits *= pdfium::base::checked_cast<FX_DWORD>(nPages);
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;
  CFX_DWordArray dwPageLenArray;
  for (int i = 0; i < nPages; ++i) {
    FX_SAFE_DWORD safePageLen = hStream->GetBits(dwDeltaPageLenBits);
    safePageLen += dwPageLeastLen;
    if (!safePageLen.IsValid())
      return FALSE;
    dwPageLenArray.Add(safePageLen.ValueOrDie());
  }
  CPDF_Object* pOffsetE = m_pLinearizedDict->GetElementValue("E");
  int nOffsetE = pOffsetE ? pOffsetE->GetInteger() : -1;
  if (nOffsetE < 0)
    return FALSE;
  CPDF_Object* pFirstPageNum = m_pLinearizedDict->GetElementValue("P");
  int nFirstPageNum = pFirstPageNum ? pFirstPageNum->GetInteger() : 0;
  for (int i = 0; i < nPages; ++i) {
    if (i == nFirstPageNum) {
      m_szPageOffsetArray.Add(m_szFirstPageObjOffset);
    } else if (i == nFirstPageNum + 1) {
      if (i == 1) {
        m_szPageOffsetArray.Add(nOffsetE);
      } else {
        m_szPageOffsetArray.Add(m_szPageOffsetArray[i - 2] +
                                dwPageLenArray[i - 2]);
      }
    } else {
      if (i == 0) {
        m_szPageOffsetArray.Add(nOffsetE);
      } else {
        m_szPageOffsetArray.Add(m_szPageOffsetArray[i - 1] +
                                dwPageLenArray[i - 1]);
      }
    }
  }
  if (nPages > 0) {
    m_szPageOffsetArray.Add(m_szPageOffsetArray[nPages - 1] +
                            dwPageLenArray[nPages - 1]);
  }
  hStream->ByteAlign();

  // number of shared objects
  required_bits = dwSharedObjBits;
  required_bits *= pdfium::base::checked_cast<FX_DWORD>(nPages);
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;
  for (int i = 0; i < nPages; i++) {
    m_dwNSharedObjsArray.Add(hStream->GetBits(dwSharedObjBits));
  }
  hStream->ByteAlign();

  // array of identifier, sizes = nshared_objects
  for (int i = 0; i < nPages; i++) {
    required_bits = dwSharedIdBits;
    required_bits *= m_dwNSharedObjsArray[i];
    if (!CanReadFromBitStream(hStream, required_bits))
      return FALSE;
    for (int j = 0; j < m_dwNSharedObjsArray[i]; j++) {
      m_dwIdentifierArray.Add(hStream->GetBits(dwSharedIdBits));
    }
  }
  hStream->ByteAlign();

  for (int i = 0; i < nPages; i++) {
    FX_SAFE_DWORD safeSize = m_dwNSharedObjsArray[i];
    safeSize *= dwSharedNumeratorBits;
    if (!CanReadFromBitStream(hStream, safeSize))
      return FALSE;
    hStream->SkipBits(safeSize.ValueOrDie());
  }
  hStream->ByteAlign();

  FX_SAFE_DWORD safeTotalPageLen = pdfium::base::checked_cast<FX_DWORD>(nPages);
  safeTotalPageLen *= dwDeltaPageLenBits;
  if (!CanReadFromBitStream(hStream, safeTotalPageLen))
    return FALSE;
  hStream->SkipBits(safeTotalPageLen.ValueOrDie());
  hStream->ByteAlign();
  return TRUE;
}
FX_BOOL CPDF_HintTables::ReadSharedObjHintTable(CFX_BitStream* hStream,
                                                FX_DWORD offset) {
  if (!hStream || hStream->IsEOF())
    return FALSE;
  int nStreamOffset = ReadPrimaryHintStreamOffset();
  int nStreamLen = ReadPrimaryHintStreamLength();
  if (nStreamOffset < 0 || nStreamLen < 1)
    return FALSE;

  FX_SAFE_DWORD bit_offset = offset;
  bit_offset *= 8;
  if (!bit_offset.IsValid() || hStream->GetPos() > bit_offset.ValueOrDie())
    return FALSE;
  hStream->SkipBits(bit_offset.ValueOrDie() - hStream->GetPos());

  const FX_DWORD kHeaderSize = 192;
  if (hStream->BitsRemaining() < kHeaderSize)
    return FALSE;
  // Item 1: The object number of the first object in the shared objects
  // section.
  FX_DWORD dwFirstSharedObjNum = hStream->GetBits(32);
  // Item 2: The location of the first object in the shared objects section.
  FX_DWORD dwFirstSharedObjLoc = hStream->GetBits(32);
  if (dwFirstSharedObjLoc > nStreamOffset)
    dwFirstSharedObjLoc += nStreamLen;
  // Item 3: The number of shared object entries for the first page.
  m_nFirstPageSharedObjs = hStream->GetBits(32);
  // Item 4: The number of shared object entries for the shared objects
  // section, including the number of shared object entries for the first page.
  FX_DWORD dwSharedObjTotal = hStream->GetBits(32);
  // Item 5: The number of bits needed to represent the greatest number of
  // objects in a shared object group. Skipped.
  hStream->SkipBits(16);
  // Item 6: The least length of a shared object group in bytes.
  FX_DWORD dwGroupLeastLen = hStream->GetBits(32);
  // Item 7: The number of bits needed to represent the difference between the
  // greatest and least length of a shared object group, in bytes.
  FX_DWORD dwDeltaGroupLen = hStream->GetBits(16);
  CPDF_Object* pFirstPageObj = m_pLinearizedDict->GetElementValue("O");
  int nFirstPageObjNum = pFirstPageObj ? pFirstPageObj->GetInteger() : -1;
  if (nFirstPageObjNum < 0)
    return FALSE;
  FX_DWORD dwPrevObjLen = 0;
  FX_DWORD dwCurObjLen = 0;
  FX_SAFE_DWORD required_bits = dwSharedObjTotal;
  required_bits *= dwDeltaGroupLen;
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;

  for (int i = 0; i < dwSharedObjTotal; ++i) {
    dwPrevObjLen = dwCurObjLen;
    FX_SAFE_DWORD safeObjLen = hStream->GetBits(dwDeltaGroupLen);
    safeObjLen += dwGroupLeastLen;
    if (!safeObjLen.IsValid())
      return FALSE;
    dwCurObjLen = safeObjLen.ValueOrDie();
    if (i < m_nFirstPageSharedObjs) {
      m_dwSharedObjNumArray.Add(nFirstPageObjNum + i);
      if (i == 0)
        m_szSharedObjOffsetArray.Add(m_szFirstPageObjOffset);
    } else {
      FX_SAFE_DWORD safeObjNum = dwFirstSharedObjNum;
      safeObjNum += i - m_nFirstPageSharedObjs;
      if (!safeObjNum.IsValid())
        return FALSE;
      m_dwSharedObjNumArray.Add(safeObjNum.ValueOrDie());
      if (i == m_nFirstPageSharedObjs)
        m_szSharedObjOffsetArray.Add(
            pdfium::base::checked_cast<int32_t>(dwFirstSharedObjLoc));
    }
    if (i != 0 && i != m_nFirstPageSharedObjs) {
      FX_SAFE_INT32 safeLoc = pdfium::base::checked_cast<int32_t>(dwPrevObjLen);
      safeLoc += m_szSharedObjOffsetArray[i - 1];
      if (!safeLoc.IsValid())
        return FALSE;
      m_szSharedObjOffsetArray.Add(safeLoc.ValueOrDie());
    }
  }
  if (dwSharedObjTotal > 0) {
    FX_SAFE_INT32 safeLoc = pdfium::base::checked_cast<int32_t>(dwCurObjLen);
    safeLoc += m_szSharedObjOffsetArray[dwSharedObjTotal - 1];
    if (!safeLoc.IsValid())
      return FALSE;
    m_szSharedObjOffsetArray.Add(safeLoc.ValueOrDie());
  }
  hStream->ByteAlign();
  if (hStream->BitsRemaining() < dwSharedObjTotal)
    return FALSE;
  hStream->SkipBits(dwSharedObjTotal);
  hStream->ByteAlign();
  return TRUE;
}
FX_BOOL CPDF_HintTables::GetPagePos(int index,
                                    FX_FILESIZE& szPageStartPos,
                                    FX_FILESIZE& szPageLength,
                                    FX_DWORD& dwObjNum) {
  if (!m_pLinearizedDict)
    return FALSE;
  szPageStartPos = m_szPageOffsetArray[index];
  szPageLength = GetItemLength(index, m_szPageOffsetArray);
  CPDF_Object* pFirstPageNum = m_pLinearizedDict->GetElementValue("P");
  int nFirstPageNum = pFirstPageNum ? pFirstPageNum->GetInteger() : 0;
  CPDF_Object* pFirstPageObjNum = m_pLinearizedDict->GetElementValue("O");
  if (!pFirstPageObjNum)
    return FALSE;
  int nFirstPageObjNum = pFirstPageObjNum->GetInteger();
  if (index == nFirstPageNum) {
    dwObjNum = nFirstPageObjNum;
    return TRUE;
  }
  // The object number of remaining pages starts from 1.
  dwObjNum = 1;
  for (int i = 0; i < index; ++i) {
    if (i == nFirstPageNum)
      continue;
    dwObjNum += m_dwDeltaNObjsArray[i];
  }
  return TRUE;
}
IPDF_DataAvail::DocAvailStatus CPDF_HintTables::CheckPage(
    int index,
    IFX_DownloadHints* pHints) {
  if (!m_pLinearizedDict || !pHints)
    return IPDF_DataAvail::DataError;
  CPDF_Object* pFirstAvailPage = m_pLinearizedDict->GetElementValue("P");
  int nFirstAvailPage = pFirstAvailPage ? pFirstAvailPage->GetInteger() : 0;
  if (index == nFirstAvailPage)
    return IPDF_DataAvail::DataAvailable;
  FX_DWORD dwLength = GetItemLength(index, m_szPageOffsetArray);
  // If two pages have the same offset, it should be treated as an error.
  if (!dwLength)
    return IPDF_DataAvail::DataError;
  if (!m_pDataAvail->IsDataAvail(m_szPageOffsetArray[index], dwLength, pHints))
    return IPDF_DataAvail::DataNotAvailable;
  // Download data of shared objects in the page.
  FX_DWORD offset = 0;
  for (int i = 0; i < index; ++i) {
    offset += m_dwNSharedObjsArray[i];
  }
  CPDF_Object* pFirstPageObj = m_pLinearizedDict->GetElementValue("O");
  int nFirstPageObjNum = pFirstPageObj ? pFirstPageObj->GetInteger() : -1;
  if (nFirstPageObjNum < 0)
    return IPDF_DataAvail::DataError;
  FX_DWORD dwIndex = 0;
  FX_DWORD dwObjNum = 0;
  for (int j = 0; j < m_dwNSharedObjsArray[index]; ++j) {
    dwIndex = m_dwIdentifierArray[offset + j];
    if (dwIndex >= m_dwSharedObjNumArray.GetSize())
      return IPDF_DataAvail::DataNotAvailable;
    dwObjNum = m_dwSharedObjNumArray[dwIndex];
    if (dwObjNum >= nFirstPageObjNum &&
        dwObjNum < nFirstPageObjNum + m_nFirstPageSharedObjs) {
      continue;
    }
    dwLength = GetItemLength(dwIndex, m_szSharedObjOffsetArray);
    // If two objects have the same offset, it should be treated as an error.
    if (!dwLength)
      return IPDF_DataAvail::DataError;
    if (!m_pDataAvail->IsDataAvail(m_szSharedObjOffsetArray[dwIndex], dwLength,
                                   pHints)) {
      return IPDF_DataAvail::DataNotAvailable;
    }
  }
  return IPDF_DataAvail::DataAvailable;
}

FX_BOOL CPDF_HintTables::LoadHintStream(CPDF_Stream* pHintStream) {
  if (!pHintStream || !m_pLinearizedDict)
    return FALSE;
  CPDF_Dictionary* pDict = pHintStream->GetDict();
  CPDF_Object* pOffset = pDict ? pDict->GetElement("S") : nullptr;
  if (!pOffset || pOffset->GetType() != PDFOBJ_NUMBER)
    return FALSE;
  int shared_hint_table_offset = pOffset->GetInteger();
  CPDF_StreamAcc acc;
  acc.LoadAllData(pHintStream);
  FX_DWORD size = acc.GetSize();
  // The header section of page offset hint table is 36 bytes.
  // The header section of shared object hint table is 24 bytes.
  // Hint table has at least 60 bytes.
  const FX_DWORD MIN_STREAM_LEN = 60;
  if (size < MIN_STREAM_LEN || shared_hint_table_offset <= 0 ||
      size < shared_hint_table_offset) {
    return FALSE;
  }
  CFX_BitStream bs;
  bs.Init(acc.GetData(), size);
  return ReadPageHintTable(&bs) &&
         ReadSharedObjHintTable(&bs, pdfium::base::checked_cast<FX_DWORD>(
                                         shared_hint_table_offset));
}

int CPDF_HintTables::ReadPrimaryHintStreamOffset() const {
  if (!m_pLinearizedDict)
    return -1;
  CPDF_Array* pRange = m_pLinearizedDict->GetArray("H");
  if (!pRange)
    return -1;
  CPDF_Object* pStreamOffset = pRange->GetElementValue(0);
  if (!pStreamOffset)
    return -1;
  return pStreamOffset->GetInteger();
}
int CPDF_HintTables::ReadPrimaryHintStreamLength() const {
  if (!m_pLinearizedDict)
    return -1;
  CPDF_Array* pRange = m_pLinearizedDict->GetArray("H");
  if (!pRange)
    return -1;
  CPDF_Object* pStreamLen = pRange->GetElementValue(1);
  if (!pStreamLen)
    return -1;
  return pStreamLen->GetInteger();
}
