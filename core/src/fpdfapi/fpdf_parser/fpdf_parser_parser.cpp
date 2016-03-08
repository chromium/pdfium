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
#include "core/src/fpdfapi/fpdf_parser/cpdf_syntax_parser.h"
#include "core/src/fpdfapi/fpdf_parser/parser_int.h"
#include "third_party/base/stl_util.h"

namespace {

// A limit on the size of the xref table. Theoretical limits are higher, but
// this may be large enough in practice.
const int32_t kMaxXRefSize = 1048576;

// A limit on the maximum object number in the xref table. Theoretical limits
// are higher, but this may be large enough in practice.
const FX_DWORD kMaxObjectNumber = 1048576;

int32_t GetHeaderOffset(IFX_FileRead* pFile) {
  // TODO(dsinclair): This is a complicated way of saying %PDF, simplify?
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
  return pObjStream->GetDict()->GetIntegerBy("N");
}

int32_t GetStreamFirst(CPDF_StreamAcc* pObjStream) {
  return pObjStream->GetDict()->GetIntegerBy("First");
}

bool CanReadFromBitStream(const CFX_BitStream* hStream,
                          const FX_SAFE_DWORD& num_bits) {
  return (num_bits.IsValid() &&
          hStream->BitsRemaining() >= num_bits.ValueOrDie());
}

}  // namespace

bool IsSignatureDict(const CPDF_Dictionary* pDict) {
  CPDF_Object* pType = pDict->GetElementValue("Type");
  if (!pType)
    pType = pDict->GetElementValue("FT");
  return pType && pType->GetString() == "Sig";
}

CPDF_Parser::CPDF_Parser()
    : m_pDocument(nullptr),
      m_bOwnFileRead(true),
      m_FileVersion(0),
      m_pTrailer(nullptr),
      m_pEncryptDict(nullptr),
      m_pLinearized(nullptr),
      m_dwFirstPageNo(0),
      m_dwXrefStartObjNum(0) {
  m_pSyntax.reset(new CPDF_SyntaxParser);
}

CPDF_Parser::~CPDF_Parser() {
  CloseParser();
}

FX_DWORD CPDF_Parser::GetLastObjNum() const {
  return m_ObjectInfo.empty() ? 0 : m_ObjectInfo.rbegin()->first;
}

bool CPDF_Parser::IsValidObjectNumber(FX_DWORD objnum) const {
  return !m_ObjectInfo.empty() && objnum <= m_ObjectInfo.rbegin()->first;
}

FX_FILESIZE CPDF_Parser::GetObjectPositionOrZero(FX_DWORD objnum) const {
  auto it = m_ObjectInfo.find(objnum);
  return it != m_ObjectInfo.end() ? it->second.pos : 0;
}

uint8_t CPDF_Parser::GetObjectType(FX_DWORD objnum) const {
  ASSERT(IsValidObjectNumber(objnum));
  auto it = m_ObjectInfo.find(objnum);
  return it != m_ObjectInfo.end() ? it->second.type : 0;
}

uint16_t CPDF_Parser::GetObjectGenNum(FX_DWORD objnum) const {
  ASSERT(IsValidObjectNumber(objnum));
  auto it = m_ObjectInfo.find(objnum);
  return it != m_ObjectInfo.end() ? it->second.gennum : 0;
}

bool CPDF_Parser::IsObjectFreeOrNull(FX_DWORD objnum) const {
  uint8_t type = GetObjectType(objnum);
  return type == 0 || type == 255;
}

void CPDF_Parser::SetEncryptDictionary(CPDF_Dictionary* pDict) {
  m_pEncryptDict = pDict;
}

CPDF_CryptoHandler* CPDF_Parser::GetCryptoHandler() {
  return m_pSyntax->m_pCryptoHandler.get();
}

IFX_FileRead* CPDF_Parser::GetFileAccess() const {
  return m_pSyntax->m_pFileAccess;
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

void CPDF_Parser::CloseParser() {
  m_bVersionUpdated = FALSE;
  delete m_pDocument;
  m_pDocument = nullptr;

  if (m_pTrailer) {
    m_pTrailer->Release();
    m_pTrailer = nullptr;
  }
  ReleaseEncryptHandler();
  SetEncryptDictionary(nullptr);

  if (m_bOwnFileRead && m_pSyntax->m_pFileAccess) {
    m_pSyntax->m_pFileAccess->Release();
    m_pSyntax->m_pFileAccess = nullptr;
  }

  m_ObjectStreamMap.clear();
  m_ObjCache.clear();
  m_SortedOffset.clear();
  m_ObjectInfo.clear();

  int32_t iLen = m_Trailers.GetSize();
  for (int32_t i = 0; i < iLen; ++i) {
    if (CPDF_Dictionary* trailer = m_Trailers.GetAt(i))
      trailer->Release();
  }
  m_Trailers.RemoveAll();

  if (m_pLinearized) {
    m_pLinearized->Release();
    m_pLinearized = nullptr;
  }
}

CPDF_Parser::Error CPDF_Parser::StartParse(IFX_FileRead* pFileAccess) {
  CloseParser();

  m_bXRefStream = FALSE;
  m_LastXRefOffset = 0;
  m_bOwnFileRead = true;

  int32_t offset = GetHeaderOffset(pFileAccess);
  if (offset == -1) {
    if (pFileAccess)
      pFileAccess->Release();
    return FORMAT_ERROR;
  }
  m_pSyntax->InitParser(pFileAccess, offset);

  uint8_t ch;
  if (!m_pSyntax->GetCharAt(5, ch))
    return FORMAT_ERROR;
  if (std::isdigit(ch))
    m_FileVersion = FXSYS_toDecimalDigit(static_cast<FX_WCHAR>(ch)) * 10;

  if (!m_pSyntax->GetCharAt(7, ch))
    return FORMAT_ERROR;
  if (std::isdigit(ch))
    m_FileVersion += FXSYS_toDecimalDigit(static_cast<FX_WCHAR>(ch));

  if (m_pSyntax->m_FileLen < m_pSyntax->m_HeaderOffset + 9)
    return FORMAT_ERROR;

  m_pSyntax->RestorePos(m_pSyntax->m_FileLen - m_pSyntax->m_HeaderOffset - 9);
  m_pDocument = new CPDF_Document(this);

  FX_BOOL bXRefRebuilt = FALSE;
  if (m_pSyntax->SearchWord("startxref", TRUE, FALSE, 4096)) {
    m_SortedOffset.insert(m_pSyntax->SavePos());
    m_pSyntax->GetKeyword();

    bool bNumber;
    CFX_ByteString xrefpos_str = m_pSyntax->GetNextWord(&bNumber);
    if (!bNumber)
      return FORMAT_ERROR;

    m_LastXRefOffset = (FX_FILESIZE)FXSYS_atoi64(xrefpos_str);
    if (!LoadAllCrossRefV4(m_LastXRefOffset) &&
        !LoadAllCrossRefV5(m_LastXRefOffset)) {
      if (!RebuildCrossRef())
        return FORMAT_ERROR;

      bXRefRebuilt = TRUE;
      m_LastXRefOffset = 0;
    }
  } else {
    if (!RebuildCrossRef())
      return FORMAT_ERROR;

    bXRefRebuilt = TRUE;
  }
  Error eRet = SetEncryptHandler();
  if (eRet != SUCCESS)
    return eRet;

  m_pDocument->LoadDoc();
  if (!m_pDocument->GetRoot() || m_pDocument->GetPageCount() == 0) {
    if (bXRefRebuilt)
      return FORMAT_ERROR;

    ReleaseEncryptHandler();
    if (!RebuildCrossRef())
      return FORMAT_ERROR;

    eRet = SetEncryptHandler();
    if (eRet != SUCCESS)
      return eRet;

    m_pDocument->LoadDoc();
    if (!m_pDocument->GetRoot())
      return FORMAT_ERROR;
  }
  if (GetRootObjNum() == 0) {
    ReleaseEncryptHandler();
    if (!RebuildCrossRef() || GetRootObjNum() == 0)
      return FORMAT_ERROR;

    eRet = SetEncryptHandler();
    if (eRet != SUCCESS)
      return eRet;
  }
  if (m_pSecurityHandler && !m_pSecurityHandler->IsMetadataEncrypted()) {
    CPDF_Reference* pMetadata =
        ToReference(m_pDocument->GetRoot()->GetElement("Metadata"));
    if (pMetadata)
      m_pSyntax->m_MetadataObjnum = pMetadata->GetRefObjNum();
  }
  return SUCCESS;
}
CPDF_Parser::Error CPDF_Parser::SetEncryptHandler() {
  ReleaseEncryptHandler();
  SetEncryptDictionary(nullptr);

  if (!m_pTrailer)
    return FORMAT_ERROR;

  CPDF_Object* pEncryptObj = m_pTrailer->GetElement("Encrypt");
  if (pEncryptObj) {
    if (CPDF_Dictionary* pEncryptDict = pEncryptObj->AsDictionary()) {
      SetEncryptDictionary(pEncryptDict);
    } else if (CPDF_Reference* pRef = pEncryptObj->AsReference()) {
      pEncryptObj = m_pDocument->GetIndirectObject(pRef->GetRefObjNum());
      if (pEncryptObj)
        SetEncryptDictionary(pEncryptObj->GetDict());
    }
  }

  if (m_pEncryptDict) {
    CFX_ByteString filter = m_pEncryptDict->GetStringBy("Filter");
    std::unique_ptr<IPDF_SecurityHandler> pSecurityHandler;
    Error err = HANDLER_ERROR;
    if (filter == "Standard") {
      pSecurityHandler.reset(new CPDF_StandardSecurityHandler);
      err = PASSWORD_ERROR;
    }
    if (!pSecurityHandler)
      return HANDLER_ERROR;

    if (!pSecurityHandler->OnInit(this, m_pEncryptDict))
      return err;

    m_pSecurityHandler = std::move(pSecurityHandler);
    std::unique_ptr<CPDF_CryptoHandler> pCryptoHandler(
        m_pSecurityHandler->CreateCryptoHandler());
    if (!pCryptoHandler->Init(m_pEncryptDict, m_pSecurityHandler.get()))
      return HANDLER_ERROR;
    m_pSyntax->SetEncrypt(std::move(pCryptoHandler));
  }
  return SUCCESS;
}

void CPDF_Parser::ReleaseEncryptHandler() {
  m_pSyntax->m_pCryptoHandler.reset();
  m_pSecurityHandler.reset();
}

FX_FILESIZE CPDF_Parser::GetObjectOffset(FX_DWORD objnum) const {
  if (!IsValidObjectNumber(objnum))
    return 0;

  if (GetObjectType(objnum) == 1)
    return GetObjectPositionOrZero(objnum);

  if (GetObjectType(objnum) == 2) {
    FX_FILESIZE pos = GetObjectPositionOrZero(objnum);
    return GetObjectPositionOrZero(pos);
  }
  return 0;
}

FX_BOOL CPDF_Parser::LoadAllCrossRefV4(FX_FILESIZE xrefpos) {
  if (!LoadCrossRefV4(xrefpos, 0, TRUE))
    return FALSE;

  m_pTrailer = LoadTrailerV4();
  if (!m_pTrailer)
    return FALSE;

  int32_t xrefsize = GetDirectInteger(m_pTrailer, "Size");
  if (xrefsize > 0 && xrefsize <= kMaxXRefSize)
    ShrinkObjectMap(xrefsize);

  std::vector<FX_FILESIZE> CrossRefList;
  std::vector<FX_FILESIZE> XRefStreamList;
  std::set<FX_FILESIZE> seen_xrefpos;

  CrossRefList.push_back(xrefpos);
  XRefStreamList.push_back(GetDirectInteger(m_pTrailer, "XRefStm"));
  seen_xrefpos.insert(xrefpos);

  // When |m_pTrailer| doesn't have Prev entry or Prev entry value is not
  // numerical, GetDirectInteger() returns 0. Loading will end.
  xrefpos = GetDirectInteger(m_pTrailer, "Prev");
  while (xrefpos) {
    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos))
      return FALSE;

    seen_xrefpos.insert(xrefpos);

    // SLOW ...
    CrossRefList.insert(CrossRefList.begin(), xrefpos);
    LoadCrossRefV4(xrefpos, 0, TRUE);

    std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>> pDict(
        LoadTrailerV4());
    if (!pDict)
      return FALSE;

    xrefpos = GetDirectInteger(pDict.get(), "Prev");

    // SLOW ...
    XRefStreamList.insert(XRefStreamList.begin(),
                          pDict->GetIntegerBy("XRefStm"));
    m_Trailers.Add(pDict.release());
  }

  for (size_t i = 0; i < CrossRefList.size(); ++i) {
    if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], FALSE))
      return FALSE;
  }
  return TRUE;
}

FX_BOOL CPDF_Parser::LoadLinearizedAllCrossRefV4(FX_FILESIZE xrefpos,
                                                 FX_DWORD dwObjCount) {
  if (!LoadLinearizedCrossRefV4(xrefpos, dwObjCount))
    return FALSE;

  m_pTrailer = LoadTrailerV4();
  if (!m_pTrailer)
    return FALSE;

  int32_t xrefsize = GetDirectInteger(m_pTrailer, "Size");
  if (xrefsize == 0)
    return FALSE;

  std::vector<FX_FILESIZE> CrossRefList;
  std::vector<FX_FILESIZE> XRefStreamList;
  std::set<FX_FILESIZE> seen_xrefpos;

  CrossRefList.push_back(xrefpos);
  XRefStreamList.push_back(GetDirectInteger(m_pTrailer, "XRefStm"));
  seen_xrefpos.insert(xrefpos);

  xrefpos = GetDirectInteger(m_pTrailer, "Prev");
  while (xrefpos) {
    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos))
      return FALSE;

    seen_xrefpos.insert(xrefpos);

    // SLOW ...
    CrossRefList.insert(CrossRefList.begin(), xrefpos);
    LoadCrossRefV4(xrefpos, 0, TRUE);

    std::unique_ptr<CPDF_Dictionary, ReleaseDeleter<CPDF_Dictionary>> pDict(
        LoadTrailerV4());
    if (!pDict)
      return FALSE;

    xrefpos = GetDirectInteger(pDict.get(), "Prev");

    // SLOW ...
    XRefStreamList.insert(XRefStreamList.begin(),
                          pDict->GetIntegerBy("XRefStm"));
    m_Trailers.Add(pDict.release());
  }

  for (size_t i = 1; i < CrossRefList.size(); ++i) {
    if (!LoadCrossRefV4(CrossRefList[i], XRefStreamList[i], FALSE))
      return FALSE;
  }
  return TRUE;
}

FX_BOOL CPDF_Parser::LoadLinearizedCrossRefV4(FX_FILESIZE pos,
                                              FX_DWORD dwObjCount) {
  FX_FILESIZE dwStartPos = pos - m_pSyntax->m_HeaderOffset;

  m_pSyntax->RestorePos(dwStartPos);
  m_SortedOffset.insert(pos);

  FX_DWORD start_objnum = 0;
  FX_DWORD count = dwObjCount;
  FX_FILESIZE SavedPos = m_pSyntax->SavePos();

  const int32_t recordsize = 20;
  std::vector<char> buf(1024 * recordsize + 1);
  buf[1024 * recordsize] = '\0';

  int32_t nBlocks = count / 1024 + 1;
  for (int32_t block = 0; block < nBlocks; block++) {
    int32_t block_size = block == nBlocks - 1 ? count % 1024 : 1024;
    FX_DWORD dwReadSize = block_size * recordsize;
    if ((FX_FILESIZE)(dwStartPos + dwReadSize) > m_pSyntax->m_FileLen)
      return FALSE;

    if (!m_pSyntax->ReadBlock(reinterpret_cast<uint8_t*>(buf.data()),
                              dwReadSize)) {
      return FALSE;
    }

    for (int32_t i = 0; i < block_size; i++) {
      FX_DWORD objnum = start_objnum + block * 1024 + i;
      char* pEntry = &buf[i * recordsize];
      if (pEntry[17] == 'f') {
        m_ObjectInfo[objnum].pos = 0;
        m_ObjectInfo[objnum].type = 0;
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
        if (version >= 1)
          m_bVersionUpdated = TRUE;

        m_ObjectInfo[objnum].gennum = version;
        if (m_ObjectInfo[objnum].pos < m_pSyntax->m_FileLen)
          m_SortedOffset.insert(m_ObjectInfo[objnum].pos);

        m_ObjectInfo[objnum].type = 1;
      }
    }
  }
  m_pSyntax->RestorePos(SavedPos + count * recordsize);
  return TRUE;
}

bool CPDF_Parser::LoadCrossRefV4(FX_FILESIZE pos,
                                 FX_FILESIZE streampos,
                                 FX_BOOL bSkip) {
  m_pSyntax->RestorePos(pos);
  if (m_pSyntax->GetKeyword() != "xref")
    return false;

  m_SortedOffset.insert(pos);
  if (streampos)
    m_SortedOffset.insert(streampos);

  while (1) {
    FX_FILESIZE SavedPos = m_pSyntax->SavePos();
    bool bIsNumber;
    CFX_ByteString word = m_pSyntax->GetNextWord(&bIsNumber);
    if (word.IsEmpty())
      return false;

    if (!bIsNumber) {
      m_pSyntax->RestorePos(SavedPos);
      break;
    }

    FX_DWORD start_objnum = FXSYS_atoui(word);
    if (start_objnum >= kMaxObjectNumber)
      return false;

    FX_DWORD count = m_pSyntax->GetDirectNum();
    m_pSyntax->ToNextWord();
    SavedPos = m_pSyntax->SavePos();
    const int32_t recordsize = 20;

    m_dwXrefStartObjNum = start_objnum;
    if (!bSkip) {
      std::vector<char> buf(1024 * recordsize + 1);
      buf[1024 * recordsize] = '\0';

      int32_t nBlocks = count / 1024 + 1;
      for (int32_t block = 0; block < nBlocks; block++) {
        int32_t block_size = block == nBlocks - 1 ? count % 1024 : 1024;
        m_pSyntax->ReadBlock(reinterpret_cast<uint8_t*>(buf.data()),
                             block_size * recordsize);

        for (int32_t i = 0; i < block_size; i++) {
          FX_DWORD objnum = start_objnum + block * 1024 + i;
          char* pEntry = &buf[i * recordsize];
          if (pEntry[17] == 'f') {
            m_ObjectInfo[objnum].pos = 0;
            m_ObjectInfo[objnum].type = 0;
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
            if (version >= 1)
              m_bVersionUpdated = TRUE;

            m_ObjectInfo[objnum].gennum = version;
            if (m_ObjectInfo[objnum].pos < m_pSyntax->m_FileLen)
              m_SortedOffset.insert(m_ObjectInfo[objnum].pos);

            m_ObjectInfo[objnum].type = 1;
          }
        }
      }
    }
    m_pSyntax->RestorePos(SavedPos + count * recordsize);
  }
  return !streampos || LoadCrossRefV5(&streampos, FALSE);
}

FX_BOOL CPDF_Parser::LoadAllCrossRefV5(FX_FILESIZE xrefpos) {
  if (!LoadCrossRefV5(&xrefpos, TRUE))
    return FALSE;

  std::set<FX_FILESIZE> seen_xrefpos;
  while (xrefpos) {
    seen_xrefpos.insert(xrefpos);
    if (!LoadCrossRefV5(&xrefpos, FALSE))
      return FALSE;

    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos))
      return FALSE;
  }
  m_ObjectStreamMap.clear();
  m_bXRefStream = TRUE;
  return TRUE;
}

FX_BOOL CPDF_Parser::RebuildCrossRef() {
  m_ObjectInfo.clear();
  m_SortedOffset.clear();
  if (m_pTrailer) {
    m_pTrailer->Release();
    m_pTrailer = nullptr;
  }

  ParserState state = ParserState::kDefault;

  int32_t inside_index = 0;
  FX_DWORD objnum = 0;
  FX_DWORD gennum = 0;
  int32_t depth = 0;

  const FX_DWORD kBufferSize = 4096;
  std::vector<uint8_t> buffer(kBufferSize);

  FX_FILESIZE pos = m_pSyntax->m_HeaderOffset;
  FX_FILESIZE start_pos = 0;
  FX_FILESIZE start_pos1 = 0;
  FX_FILESIZE last_obj = -1;
  FX_FILESIZE last_xref = -1;
  FX_FILESIZE last_trailer = -1;

  while (pos < m_pSyntax->m_FileLen) {
    const FX_FILESIZE saved_pos = pos;
    bool bOverFlow = false;
    FX_DWORD size =
        std::min((FX_DWORD)(m_pSyntax->m_FileLen - pos), kBufferSize);
    if (!m_pSyntax->m_pFileAccess->ReadBlock(buffer.data(), pos, size))
      break;

    for (FX_DWORD i = 0; i < size; i++) {
      uint8_t byte = buffer[i];
      switch (state) {
        case ParserState::kDefault:
          if (PDFCharIsWhitespace(byte)) {
            state = ParserState::kWhitespace;
          } else if (std::isdigit(byte)) {
            --i;
            state = ParserState::kWhitespace;
          } else if (byte == '%') {
            inside_index = 0;
            state = ParserState::kComment;
          } else if (byte == '(') {
            state = ParserState::kString;
            depth = 1;
          } else if (byte == '<') {
            inside_index = 1;
            state = ParserState::kHexString;
          } else if (byte == '\\') {
            state = ParserState::kEscapedString;
          } else if (byte == 't') {
            state = ParserState::kTrailer;
            inside_index = 1;
          }
          break;

        case ParserState::kWhitespace:
          if (std::isdigit(byte)) {
            start_pos = pos + i;
            state = ParserState::kObjNum;
            objnum = FXSYS_toDecimalDigit(static_cast<FX_WCHAR>(byte));
          } else if (byte == 't') {
            state = ParserState::kTrailer;
            inside_index = 1;
          } else if (byte == 'x') {
            state = ParserState::kXref;
            inside_index = 1;
          } else if (!PDFCharIsWhitespace(byte)) {
            --i;
            state = ParserState::kDefault;
          }
          break;

        case ParserState::kObjNum:
          if (std::isdigit(byte)) {
            objnum =
                objnum * 10 + FXSYS_toDecimalDigit(static_cast<FX_WCHAR>(byte));
          } else if (PDFCharIsWhitespace(byte)) {
            state = ParserState::kPostObjNum;
          } else {
            --i;
            state = ParserState::kEndObj;
            inside_index = 0;
          }
          break;

        case ParserState::kPostObjNum:
          if (std::isdigit(byte)) {
            start_pos1 = pos + i;
            state = ParserState::kGenNum;
            gennum = FXSYS_toDecimalDigit(static_cast<FX_WCHAR>(byte));
          } else if (byte == 't') {
            state = ParserState::kTrailer;
            inside_index = 1;
          } else if (!PDFCharIsWhitespace(byte)) {
            --i;
            state = ParserState::kDefault;
          }
          break;

        case ParserState::kGenNum:
          if (std::isdigit(byte)) {
            gennum =
                gennum * 10 + FXSYS_toDecimalDigit(static_cast<FX_WCHAR>(byte));
          } else if (PDFCharIsWhitespace(byte)) {
            state = ParserState::kPostGenNum;
          } else {
            --i;
            state = ParserState::kDefault;
          }
          break;

        case ParserState::kPostGenNum:
          if (byte == 'o') {
            state = ParserState::kBeginObj;
            inside_index = 1;
          } else if (std::isdigit(byte)) {
            objnum = gennum;
            gennum = FXSYS_toDecimalDigit(static_cast<FX_WCHAR>(byte));
            start_pos = start_pos1;
            start_pos1 = pos + i;
            state = ParserState::kGenNum;
          } else if (byte == 't') {
            state = ParserState::kTrailer;
            inside_index = 1;
          } else if (!PDFCharIsWhitespace(byte)) {
            --i;
            state = ParserState::kDefault;
          }
          break;

        case ParserState::kBeginObj:
          switch (inside_index) {
            case 1:
              if (byte != 'b') {
                --i;
                state = ParserState::kDefault;
              } else {
                inside_index++;
              }
              break;
            case 2:
              if (byte != 'j') {
                --i;
                state = ParserState::kDefault;
              } else {
                inside_index++;
              }
              break;
            case 3:
              if (PDFCharIsWhitespace(byte) || PDFCharIsDelimiter(byte)) {
                FX_FILESIZE obj_pos = start_pos - m_pSyntax->m_HeaderOffset;
                m_SortedOffset.insert(obj_pos);
                last_obj = start_pos;
                FX_FILESIZE obj_end = 0;
                CPDF_Object* pObject = ParseIndirectObjectAtByStrict(
                    m_pDocument, obj_pos, objnum, &obj_end);
                if (CPDF_Stream* pStream = ToStream(pObject)) {
                  if (CPDF_Dictionary* pDict = pStream->GetDict()) {
                    if ((pDict->KeyExist("Type")) &&
                        (pDict->GetStringBy("Type") == "XRef" &&
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
                m_pSyntax->RestorePos(obj_pos);
                offset = m_pSyntax->FindTag("obj", 0);
                if (offset == -1)
                  offset = 0;
                else
                  offset += 3;

                FX_FILESIZE nLen = obj_end - obj_pos - offset;
                if ((FX_DWORD)nLen > size - i) {
                  pos = obj_end + m_pSyntax->m_HeaderOffset;
                  bOverFlow = true;
                } else {
                  i += (FX_DWORD)nLen;
                }

                if (!m_ObjectInfo.empty() && IsValidObjectNumber(objnum) &&
                    m_ObjectInfo[objnum].pos) {
                  if (pObject) {
                    FX_DWORD oldgen = GetObjectGenNum(objnum);
                    m_ObjectInfo[objnum].pos = obj_pos;
                    m_ObjectInfo[objnum].gennum = gennum;
                    if (oldgen != gennum)
                      m_bVersionUpdated = TRUE;
                  }
                } else {
                  m_ObjectInfo[objnum].pos = obj_pos;
                  m_ObjectInfo[objnum].type = 1;
                  m_ObjectInfo[objnum].gennum = gennum;
                }

                if (pObject)
                  pObject->Release();
              }
              --i;
              state = ParserState::kDefault;
              break;
          }
          break;

        case ParserState::kTrailer:
          if (inside_index == 7) {
            if (PDFCharIsWhitespace(byte) || PDFCharIsDelimiter(byte)) {
              last_trailer = pos + i - 7;
              m_pSyntax->RestorePos(pos + i - m_pSyntax->m_HeaderOffset);

              CPDF_Object* pObj = m_pSyntax->GetObject(m_pDocument, 0, 0, true);
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
                      }
                      pObj->Release();
                    } else {
                      if (pObj->IsStream()) {
                        m_pTrailer = ToDictionary(pTrailer->Clone());
                        pObj->Release();
                      } else {
                        m_pTrailer = pTrailer;
                      }

                      FX_FILESIZE dwSavePos = m_pSyntax->SavePos();
                      CFX_ByteString strWord = m_pSyntax->GetKeyword();
                      if (!strWord.Compare("startxref")) {
                        bool bNumber;
                        CFX_ByteString bsOffset =
                            m_pSyntax->GetNextWord(&bNumber);
                        if (bNumber)
                          m_LastXRefOffset = FXSYS_atoi(bsOffset);
                      }
                      m_pSyntax->RestorePos(dwSavePos);
                    }
                  } else {
                    pObj->Release();
                  }
                }
              }
            }
            --i;
            state = ParserState::kDefault;
          } else if (byte == "trailer"[inside_index]) {
            inside_index++;
          } else {
            --i;
            state = ParserState::kDefault;
          }
          break;

        case ParserState::kXref:
          if (inside_index == 4) {
            last_xref = pos + i - 4;
            state = ParserState::kWhitespace;
          } else if (byte == "xref"[inside_index]) {
            inside_index++;
          } else {
            --i;
            state = ParserState::kDefault;
          }
          break;

        case ParserState::kComment:
          if (byte == '\r' || byte == '\n')
            state = ParserState::kDefault;
          break;

        case ParserState::kString:
          if (byte == ')') {
            if (depth > 0)
              depth--;
          } else if (byte == '(') {
            depth++;
          }

          if (!depth)
            state = ParserState::kDefault;
          break;

        case ParserState::kHexString:
          if (byte == '>' || (byte == '<' && inside_index == 1))
            state = ParserState::kDefault;
          inside_index = 0;
          break;

        case ParserState::kEscapedString:
          if (PDFCharIsDelimiter(byte) || PDFCharIsWhitespace(byte)) {
            --i;
            state = ParserState::kDefault;
          }
          break;

        case ParserState::kEndObj:
          if (PDFCharIsWhitespace(byte)) {
            state = ParserState::kDefault;
          } else if (byte == '%' || byte == '(' || byte == '<' ||
                     byte == '\\') {
            state = ParserState::kDefault;
            --i;
          } else if (inside_index == 6) {
            state = ParserState::kDefault;
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

  if (last_xref != -1 && last_xref > last_obj)
    last_trailer = last_xref;
  else if (last_trailer == -1 || last_xref < last_obj)
    last_trailer = m_pSyntax->m_FileLen;

  m_SortedOffset.insert(last_trailer - m_pSyntax->m_HeaderOffset);
  return m_pTrailer && !m_ObjectInfo.empty();
}

FX_BOOL CPDF_Parser::LoadCrossRefV5(FX_FILESIZE* pos, FX_BOOL bMainXRef) {
  CPDF_Object* pObject = ParseIndirectObjectAt(m_pDocument, *pos, 0);
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

  *pos = pStream->GetDict()->GetIntegerBy("Prev");
  int32_t size = pStream->GetDict()->GetIntegerBy("Size");
  if (size < 0) {
    pStream->Release();
    return FALSE;
  }

  if (bMainXRef) {
    m_pTrailer = ToDictionary(pStream->GetDict()->Clone());
    ShrinkObjectMap(size);
    for (auto& it : m_ObjectInfo)
      it.second.type = 0;
  } else {
    m_Trailers.Add(ToDictionary(pStream->GetDict()->Clone()));
  }

  std::vector<std::pair<int32_t, int32_t> > arrIndex;
  CPDF_Array* pArray = pStream->GetDict()->GetArrayBy("Index");
  if (pArray) {
    FX_DWORD nPairSize = pArray->GetCount() / 2;
    for (FX_DWORD i = 0; i < nPairSize; i++) {
      CPDF_Object* pStartNumObj = pArray->GetElement(i * 2);
      CPDF_Object* pCountObj = pArray->GetElement(i * 2 + 1);

      if (ToNumber(pStartNumObj) && ToNumber(pCountObj)) {
        int nStartNum = pStartNumObj->GetInteger();
        int nCount = pCountObj->GetInteger();
        if (nStartNum >= 0 && nCount > 0)
          arrIndex.push_back(std::make_pair(nStartNum, nCount));
      }
    }
  }

  if (arrIndex.size() == 0)
    arrIndex.push_back(std::make_pair(0, size));

  pArray = pStream->GetDict()->GetArrayBy("W");
  if (!pArray) {
    pStream->Release();
    return FALSE;
  }

  CFX_DWordArray WidthArray;
  FX_SAFE_DWORD dwAccWidth = 0;
  for (FX_DWORD i = 0; i < pArray->GetCount(); i++) {
    WidthArray.Add(pArray->GetIntegerAt(i));
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
    if (startnum < 0)
      continue;

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
    FX_DWORD dwV5Size = m_ObjectInfo.empty() ? 0 : GetLastObjNum() + 1;
    if (!dwMaxObjNum.IsValid() || dwMaxObjNum.ValueOrDie() > dwV5Size)
      continue;

    for (FX_DWORD j = 0; j < count; j++) {
      int32_t type = 1;
      const uint8_t* entrystart = segstart + j * totalWidth;
      if (WidthArray[0])
        type = GetVarInt(entrystart, WidthArray[0]);

      if (GetObjectType(startnum + j) == 255) {
        FX_FILESIZE offset =
            GetVarInt(entrystart + WidthArray[0], WidthArray[1]);
        m_ObjectInfo[startnum + j].pos = offset;
        m_SortedOffset.insert(offset);
        continue;
      }

      if (GetObjectType(startnum + j))
        continue;

      m_ObjectInfo[startnum + j].type = type;
      if (type == 0) {
        m_ObjectInfo[startnum + j].pos = 0;
      } else {
        FX_FILESIZE offset =
            GetVarInt(entrystart + WidthArray[0], WidthArray[1]);
        m_ObjectInfo[startnum + j].pos = offset;
        if (type == 1) {
          m_SortedOffset.insert(offset);
        } else {
          if (offset < 0 || !IsValidObjectNumber(offset)) {
            pStream->Release();
            return FALSE;
          }
          m_ObjectInfo[offset].type = 255;
        }
      }
    }
    segindex += count;
  }
  pStream->Release();
  return TRUE;
}

CPDF_Array* CPDF_Parser::GetIDArray() {
  CPDF_Object* pID = m_pTrailer ? m_pTrailer->GetElement("ID") : nullptr;
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

  if (GetObjectType(objnum) == 0)
    return TRUE;

  if (GetObjectType(objnum) == 2)
    return TRUE;

  FX_FILESIZE pos = m_ObjectInfo[objnum].pos;
  auto it = m_SortedOffset.find(pos);
  if (it == m_SortedOffset.end())
    return TRUE;

  if (++it == m_SortedOffset.end())
    return FALSE;

  FX_FILESIZE size = *it - pos;
  FX_FILESIZE SavedPos = m_pSyntax->SavePos();
  m_pSyntax->RestorePos(pos);

  const char kFormStream[] = "/Form\0stream";
  const CFX_ByteStringC kFormStreamStr(kFormStream, sizeof(kFormStream) - 1);
  bForm = m_pSyntax->SearchMultiWord(kFormStreamStr, TRUE, size) == 0;
  m_pSyntax->RestorePos(SavedPos);
  return TRUE;
}

CPDF_Object* CPDF_Parser::ParseIndirectObject(
    CPDF_IndirectObjectHolder* pObjList,
    FX_DWORD objnum) {
  if (!IsValidObjectNumber(objnum))
    return nullptr;

  // Prevent circular parsing the same object.
  if (pdfium::ContainsKey(m_ParsingObjNums, objnum))
    return nullptr;
  ScopedSetInsertion<FX_DWORD> local_insert(&m_ParsingObjNums, objnum);

  if (GetObjectType(objnum) == 1 || GetObjectType(objnum) == 255) {
    FX_FILESIZE pos = m_ObjectInfo[objnum].pos;
    if (pos <= 0)
      return nullptr;
    return ParseIndirectObjectAt(pObjList, pos, objnum);
  }
  if (GetObjectType(objnum) != 2)
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
  return syntax.GetObject(pObjList, 0, 0, true);
}

CPDF_StreamAcc* CPDF_Parser::GetObjectStream(FX_DWORD objnum) {
  auto it = m_ObjectStreamMap.find(objnum);
  if (it != m_ObjectStreamMap.end())
    return it->second.get();

  if (!m_pDocument)
    return nullptr;

  const CPDF_Stream* pStream = ToStream(m_pDocument->GetIndirectObject(objnum));
  if (!pStream)
    return nullptr;

  CPDF_StreamAcc* pStreamAcc = new CPDF_StreamAcc;
  pStreamAcc->LoadAllData(pStream);
  m_ObjectStreamMap[objnum].reset(pStreamAcc);
  return pStreamAcc;
}

FX_FILESIZE CPDF_Parser::GetObjectSize(FX_DWORD objnum) const {
  if (!IsValidObjectNumber(objnum))
    return 0;

  if (GetObjectType(objnum) == 2)
    objnum = GetObjectPositionOrZero(objnum);

  if (GetObjectType(objnum) != 1 && GetObjectType(objnum) != 255)
    return 0;

  FX_FILESIZE offset = GetObjectPositionOrZero(objnum);
  if (offset == 0)
    return 0;

  auto it = m_SortedOffset.find(offset);
  if (it == m_SortedOffset.end() || ++it == m_SortedOffset.end())
    return 0;

  return *it - offset;
}

void CPDF_Parser::GetIndirectBinary(FX_DWORD objnum,
                                    uint8_t*& pBuffer,
                                    FX_DWORD& size) {
  pBuffer = nullptr;
  size = 0;
  if (!IsValidObjectNumber(objnum))
    return;

  if (GetObjectType(objnum) == 2) {
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

  if (GetObjectType(objnum) != 1)
    return;

  FX_FILESIZE pos = m_ObjectInfo[objnum].pos;
  if (pos == 0)
    return;

  FX_FILESIZE SavedPos = m_pSyntax->SavePos();
  m_pSyntax->RestorePos(pos);

  bool bIsNumber;
  CFX_ByteString word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_pSyntax->RestorePos(SavedPos);
    return;
  }

  FX_DWORD parser_objnum = FXSYS_atoui(word);
  if (parser_objnum && parser_objnum != objnum) {
    m_pSyntax->RestorePos(SavedPos);
    return;
  }

  word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_pSyntax->RestorePos(SavedPos);
    return;
  }

  if (m_pSyntax->GetKeyword() != "obj") {
    m_pSyntax->RestorePos(SavedPos);
    return;
  }

  auto it = m_SortedOffset.find(pos);
  if (it == m_SortedOffset.end() || ++it == m_SortedOffset.end()) {
    m_pSyntax->RestorePos(SavedPos);
    return;
  }

  FX_FILESIZE nextoff = *it;
  FX_BOOL bNextOffValid = FALSE;
  if (nextoff != pos) {
    m_pSyntax->RestorePos(nextoff);
    word = m_pSyntax->GetNextWord(&bIsNumber);
    if (word == "xref") {
      bNextOffValid = TRUE;
    } else if (bIsNumber) {
      word = m_pSyntax->GetNextWord(&bIsNumber);
      if (bIsNumber && m_pSyntax->GetKeyword() == "obj") {
        bNextOffValid = TRUE;
      }
    }
  }

  if (!bNextOffValid) {
    m_pSyntax->RestorePos(pos);
    while (1) {
      if (m_pSyntax->GetKeyword() == "endobj")
        break;

      if (m_pSyntax->SavePos() == m_pSyntax->m_FileLen)
        break;
    }
    nextoff = m_pSyntax->SavePos();
  }

  size = (FX_DWORD)(nextoff - pos);
  pBuffer = FX_Alloc(uint8_t, size);
  m_pSyntax->RestorePos(pos);
  m_pSyntax->ReadBlock(pBuffer, size);
  m_pSyntax->RestorePos(SavedPos);
}

CPDF_Object* CPDF_Parser::ParseIndirectObjectAt(
    CPDF_IndirectObjectHolder* pObjList,
    FX_FILESIZE pos,
    FX_DWORD objnum) {
  FX_FILESIZE SavedPos = m_pSyntax->SavePos();
  m_pSyntax->RestorePos(pos);
  bool bIsNumber;
  CFX_ByteString word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  FX_FILESIZE objOffset = m_pSyntax->SavePos();
  objOffset -= word.GetLength();
  FX_DWORD parser_objnum = FXSYS_atoui(word);
  if (objnum && parser_objnum != objnum) {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  FX_DWORD parser_gennum = FXSYS_atoui(word);
  if (m_pSyntax->GetKeyword() != "obj") {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  CPDF_Object* pObj =
      m_pSyntax->GetObject(pObjList, objnum, parser_gennum, true);
  m_pSyntax->SavePos();

  CFX_ByteString bsWord = m_pSyntax->GetKeyword();
  if (bsWord == "endobj")
    m_pSyntax->SavePos();

  m_pSyntax->RestorePos(SavedPos);
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
    FX_FILESIZE* pResultPos) {
  FX_FILESIZE SavedPos = m_pSyntax->SavePos();
  m_pSyntax->RestorePos(pos);

  bool bIsNumber;
  CFX_ByteString word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  FX_DWORD parser_objnum = FXSYS_atoui(word);
  if (objnum && parser_objnum != objnum) {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber) {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  FX_DWORD gennum = FXSYS_atoui(word);
  if (m_pSyntax->GetKeyword() != "obj") {
    m_pSyntax->RestorePos(SavedPos);
    return nullptr;
  }

  CPDF_Object* pObj = m_pSyntax->GetObjectByStrict(pObjList, objnum, gennum);
  if (pResultPos)
    *pResultPos = m_pSyntax->m_Pos;

  m_pSyntax->RestorePos(SavedPos);
  return pObj;
}

CPDF_Dictionary* CPDF_Parser::LoadTrailerV4() {
  if (m_pSyntax->GetKeyword() != "trailer")
    return nullptr;

  std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>> pObj(
      m_pSyntax->GetObject(m_pDocument, 0, 0, true));
  if (!ToDictionary(pObj.get()))
    return nullptr;
  return pObj.release()->AsDictionary();
}

FX_DWORD CPDF_Parser::GetPermissions(FX_BOOL bCheckRevision) {
  if (!m_pSecurityHandler)
    return (FX_DWORD)-1;

  FX_DWORD dwPermission = m_pSecurityHandler->GetPermissions();
  if (m_pEncryptDict && m_pEncryptDict->GetStringBy("Filter") == "Standard") {
    dwPermission &= 0xFFFFFFFC;
    dwPermission |= 0xFFFFF0C0;
    if (bCheckRevision && m_pEncryptDict->GetIntegerBy("R") == 2)
      dwPermission &= 0xFFFFF0FF;
  }
  return dwPermission;
}

FX_BOOL CPDF_Parser::IsLinearizedFile(IFX_FileRead* pFileAccess,
                                      FX_DWORD offset) {
  m_pSyntax->InitParser(pFileAccess, offset);
  m_pSyntax->RestorePos(m_pSyntax->m_HeaderOffset + 9);

  FX_FILESIZE SavedPos = m_pSyntax->SavePos();
  bool bIsNumber;
  CFX_ByteString word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber)
    return FALSE;

  FX_DWORD objnum = FXSYS_atoui(word);
  word = m_pSyntax->GetNextWord(&bIsNumber);
  if (!bIsNumber)
    return FALSE;

  FX_DWORD gennum = FXSYS_atoui(word);
  if (m_pSyntax->GetKeyword() != "obj") {
    m_pSyntax->RestorePos(SavedPos);
    return FALSE;
  }

  m_pLinearized = m_pSyntax->GetObject(nullptr, objnum, gennum, true);
  if (!m_pLinearized)
    return FALSE;

  CPDF_Dictionary* pDict = m_pLinearized->GetDict();
  if (pDict && pDict->GetElement("Linearized")) {
    m_pSyntax->GetNextWord(nullptr);

    CPDF_Object* pLen = pDict->GetElement("L");
    if (!pLen) {
      m_pLinearized->Release();
      m_pLinearized = nullptr;
      return FALSE;
    }

    if (pLen->GetInteger() != (int)pFileAccess->GetSize())
      return FALSE;

    if (CPDF_Number* pNo = ToNumber(pDict->GetElement("P")))
      m_dwFirstPageNo = pNo->GetInteger();

    if (CPDF_Number* pTable = ToNumber(pDict->GetElement("T")))
      m_LastXRefOffset = pTable->GetInteger();

    return TRUE;
  }
  m_pLinearized->Release();
  m_pLinearized = nullptr;
  return FALSE;
}

CPDF_Parser::Error CPDF_Parser::StartAsyncParse(IFX_FileRead* pFileAccess) {
  CloseParser();
  m_bXRefStream = FALSE;
  m_LastXRefOffset = 0;
  m_bOwnFileRead = true;

  int32_t offset = GetHeaderOffset(pFileAccess);
  if (offset == -1)
    return FORMAT_ERROR;

  if (!IsLinearizedFile(pFileAccess, offset)) {
    m_pSyntax->m_pFileAccess = nullptr;
    return StartParse(pFileAccess);
  }

  m_pDocument = new CPDF_Document(this);
  FX_FILESIZE dwFirstXRefOffset = m_pSyntax->SavePos();

  FX_BOOL bXRefRebuilt = FALSE;
  FX_BOOL bLoadV4 = FALSE;
  if (!(bLoadV4 = LoadCrossRefV4(dwFirstXRefOffset, 0, FALSE)) &&
      !LoadCrossRefV5(&dwFirstXRefOffset, TRUE)) {
    if (!RebuildCrossRef())
      return FORMAT_ERROR;

    bXRefRebuilt = TRUE;
    m_LastXRefOffset = 0;
  }

  if (bLoadV4) {
    m_pTrailer = LoadTrailerV4();
    if (!m_pTrailer)
      return SUCCESS;

    int32_t xrefsize = GetDirectInteger(m_pTrailer, "Size");
    if (xrefsize > 0)
      ShrinkObjectMap(xrefsize);
  }

  Error eRet = SetEncryptHandler();
  if (eRet != SUCCESS)
    return eRet;

  m_pDocument->LoadAsynDoc(m_pLinearized->GetDict());
  if (!m_pDocument->GetRoot() || m_pDocument->GetPageCount() == 0) {
    if (bXRefRebuilt)
      return FORMAT_ERROR;

    ReleaseEncryptHandler();
    if (!RebuildCrossRef())
      return FORMAT_ERROR;

    eRet = SetEncryptHandler();
    if (eRet != SUCCESS)
      return eRet;

    m_pDocument->LoadAsynDoc(m_pLinearized->GetDict());
    if (!m_pDocument->GetRoot())
      return FORMAT_ERROR;
  }

  if (GetRootObjNum() == 0) {
    ReleaseEncryptHandler();
    if (!RebuildCrossRef() || GetRootObjNum() == 0)
      return FORMAT_ERROR;

    eRet = SetEncryptHandler();
    if (eRet != SUCCESS)
      return eRet;
  }

  if (m_pSecurityHandler && m_pSecurityHandler->IsMetadataEncrypted()) {
    if (CPDF_Reference* pMetadata =
            ToReference(m_pDocument->GetRoot()->GetElement("Metadata")))
      m_pSyntax->m_MetadataObjnum = pMetadata->GetRefObjNum();
  }
  return SUCCESS;
}

FX_BOOL CPDF_Parser::LoadLinearizedAllCrossRefV5(FX_FILESIZE xrefpos) {
  if (!LoadCrossRefV5(&xrefpos, FALSE))
    return FALSE;

  std::set<FX_FILESIZE> seen_xrefpos;
  while (xrefpos) {
    seen_xrefpos.insert(xrefpos);
    if (!LoadCrossRefV5(&xrefpos, FALSE))
      return FALSE;

    // Check for circular references.
    if (pdfium::ContainsKey(seen_xrefpos, xrefpos))
      return FALSE;
  }
  m_ObjectStreamMap.clear();
  m_bXRefStream = TRUE;
  return TRUE;
}

CPDF_Parser::Error CPDF_Parser::LoadLinearizedMainXRefTable() {
  FX_DWORD dwSaveMetadataObjnum = m_pSyntax->m_MetadataObjnum;
  m_pSyntax->m_MetadataObjnum = 0;
  if (m_pTrailer) {
    m_pTrailer->Release();
    m_pTrailer = nullptr;
  }

  m_pSyntax->RestorePos(m_LastXRefOffset - m_pSyntax->m_HeaderOffset);
  uint8_t ch = 0;
  FX_DWORD dwCount = 0;
  m_pSyntax->GetNextChar(ch);
  while (PDFCharIsWhitespace(ch)) {
    ++dwCount;
    if (m_pSyntax->m_FileLen >=
        (FX_FILESIZE)(m_pSyntax->SavePos() + m_pSyntax->m_HeaderOffset)) {
      break;
    }
    m_pSyntax->GetNextChar(ch);
  }
  m_LastXRefOffset += dwCount;
  m_ObjectStreamMap.clear();
  m_ObjCache.clear();

  if (!LoadLinearizedAllCrossRefV4(m_LastXRefOffset, m_dwXrefStartObjNum) &&
      !LoadLinearizedAllCrossRefV5(m_LastXRefOffset)) {
    m_LastXRefOffset = 0;
    m_pSyntax->m_MetadataObjnum = dwSaveMetadataObjnum;
    return FORMAT_ERROR;
  }

  m_pSyntax->m_MetadataObjnum = dwSaveMetadataObjnum;
  return SUCCESS;
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
      CPDF_IndirectObjectHolder* pObjList = nullptr);
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
  m_parser.m_bOwnFileRead = false;
  m_bTotalLoadPageTree = FALSE;
  m_bCurPageDictLoadOK = FALSE;
  m_bLinearedDataOK = FALSE;
  m_bSupportHintTable = bSupportHintTable;
}
CPDF_DataAvail::~CPDF_DataAvail() {
  if (m_pLinearized)
    m_pLinearized->Release();

  if (m_pRoot)
    m_pRoot->Release();

  if (m_pTrailer)
    m_pTrailer->Release();

  int iSize = m_arrayAcroforms.GetSize();
  for (int i = 0; i < iSize; ++i)
    m_arrayAcroforms.GetAt(i)->Release();
}

void CPDF_DataAvail::SetDocument(CPDF_Document* pDoc) {
  m_pDocument = pDoc;
}

FX_DWORD CPDF_DataAvail::GetObjectSize(FX_DWORD objnum, FX_FILESIZE& offset) {
  CPDF_Parser* pParser = m_pDocument->GetParser();
  if (!pParser || !pParser->IsValidObjectNumber(objnum))
    return 0;

  if (pParser->GetObjectType(objnum) == 2)
    objnum = pParser->GetObjectPositionOrZero(objnum);

  if (pParser->GetObjectType(objnum) != 1 &&
      pParser->GetObjectType(objnum) != 255) {
    return 0;
  }

  offset = pParser->GetObjectPositionOrZero(objnum);
  if (offset == 0)
    return 0;

  auto it = pParser->m_SortedOffset.find(offset);
  if (it == pParser->m_SortedOffset.end() ||
      ++it == pParser->m_SortedOffset.end()) {
    return 0;
  }
  return *it - offset;
}

FX_BOOL CPDF_DataAvail::IsObjectsAvail(
    CFX_ArrayTemplate<CPDF_Object*>& obj_array,
    FX_BOOL bParsePage,
    IFX_DownloadHints* pHints,
    CFX_ArrayTemplate<CPDF_Object*>& ret_array) {
  if (!obj_array.GetSize())
    return TRUE;

  FX_DWORD count = 0;
  CFX_ArrayTemplate<CPDF_Object*> new_obj_array;
  int32_t i = 0;
  for (i = 0; i < obj_array.GetSize(); i++) {
    CPDF_Object* pObj = obj_array[i];
    if (!pObj)
      continue;

    int32_t type = pObj->GetType();
    switch (type) {
      case CPDF_Object::ARRAY: {
        CPDF_Array* pArray = pObj->GetArray();
        for (FX_DWORD k = 0; k < pArray->GetCount(); ++k)
          new_obj_array.Add(pArray->GetElement(k));
      } break;
      case CPDF_Object::STREAM:
        pObj = pObj->GetDict();
      case CPDF_Object::DICTIONARY: {
        CPDF_Dictionary* pDict = pObj->GetDict();
        if (pDict && pDict->GetStringBy("Type") == "Page" && !bParsePage)
          continue;

        for (const auto& it : *pDict) {
          const CFX_ByteString& key = it.first;
          CPDF_Object* value = it.second;
          if (key != "Parent")
            new_obj_array.Add(value);
        }
      } break;
      case CPDF_Object::REFERENCE: {
        CPDF_Reference* pRef = pObj->AsReference();
        FX_DWORD dwNum = pRef->GetRefObjNum();

        FX_FILESIZE offset;
        FX_DWORD size = GetObjectSize(dwNum, offset);
        if (size == 0 || offset < 0 || offset >= m_dwFileLen)
          break;

        if (!IsDataAvail(offset, size, pHints)) {
          ret_array.Add(pObj);
          count++;
        } else if (!pdfium::ContainsKey(m_ObjectSet, dwNum)) {
          m_ObjectSet.insert(dwNum);
          CPDF_Object* pReferred =
              m_pDocument->GetIndirectObject(pRef->GetRefObjNum());
          if (pReferred)
            new_obj_array.Add(pReferred);
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
    if (!m_dwFileLen)
      return DataError;
  }

  while (!m_bDocAvail) {
    if (!CheckDocStatus(pHints))
      return DataNotAvailable;
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
    if (bRet)
      m_objs_array.RemoveAll();
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
      if (m_bTotalLoadPageTree)
        return CheckPages(pHints);
      return LoadDocPages(pHints);
    case PDF_DATAAVAIL_PAGE:
      if (m_bTotalLoadPageTree)
        return CheckPage(pHints);
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
  m_parser.m_pSyntax->InitParser(m_pFileRead, (FX_DWORD)m_dwHeaderOffset);
  m_parser.m_bOwnFileRead = false;
  if (!m_parser.LoadAllCrossRefV4(m_dwLastXRefOffset) &&
      !m_parser.LoadAllCrossRefV5(m_dwLastXRefOffset)) {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return FALSE;
  }

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
    pParser = m_pDocument->GetParser();
  } else {
    size = (FX_DWORD)m_parser.GetObjectSize(objnum);
    offset = m_parser.GetObjectOffset(objnum);
    pParser = &m_parser;
  }

  if (!IsDataAvail(offset, size, pHints))
    return nullptr;

  if (pParser)
    pRet = pParser->ParseIndirectObject(nullptr, objnum);

  if (!pRet && pExistInFile)
    *pExistInFile = FALSE;

  return pRet;
}

FX_BOOL CPDF_DataAvail::CheckInfo(IFX_DownloadHints* pHints) {
  FX_BOOL bExist = FALSE;
  CPDF_Object* pInfo = GetObject(m_dwInfoObjNum, pHints, &bExist);
  if (!bExist) {
    m_docStatus =
        (m_bHaveAcroForm ? PDF_DATAAVAIL_ACROFORM : PDF_DATAAVAIL_PAGETREE);
    return TRUE;
  }

  if (!pInfo) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return TRUE;
    }

    if (m_Pos == m_dwFileLen)
      m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }

  if (pInfo)
    pInfo->Release();

  m_docStatus =
      (m_bHaveAcroForm ? PDF_DATAAVAIL_ACROFORM : PDF_DATAAVAIL_PAGETREE);

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
  m_pCurrentParser = m_pDocument->GetParser();
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
      if (bExist)
        UnavailObjList.Add(dwPageObjNum);
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

    CFX_ByteString type = pObj->GetDict()->GetStringBy("Type");
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
  if (!m_PageObjList.GetSize())
    m_docStatus = PDF_DATAAVAIL_DONE;
  return TRUE;
}

FX_BOOL CPDF_DataAvail::GetPageKids(CPDF_Parser* pParser, CPDF_Object* pPages) {
  if (!pParser) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }

  CPDF_Dictionary* pDict = pPages->GetDict();
  CPDF_Object* pKids = pDict ? pDict->GetElement("Kids") : NULL;
  if (!pKids)
    return TRUE;

  switch (pKids->GetType()) {
    case CPDF_Object::REFERENCE:
      m_PageObjList.Add(pKids->AsReference()->GetRefObjNum());
      break;
    case CPDF_Object::ARRAY: {
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
  if ((FX_FILESIZE)req_size > m_dwFileLen)
    req_size = (FX_DWORD)m_dwFileLen;

  if (m_pFileAvail->IsDataAvail(0, req_size)) {
    uint8_t buffer[1024];
    m_pFileRead->ReadBlock(buffer, 0, req_size);

    if (IsLinearizedFile(buffer, req_size)) {
      m_docStatus = PDF_DATAAVAIL_FIRSTPAGE;
    } else {
      if (m_docStatus == PDF_DATAAVAIL_ERROR)
        return FALSE;
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
    if ((FX_FILESIZE)dwEnd > m_dwFileLen)
      dwEnd = (FX_DWORD)m_dwFileLen;

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

  CPDF_Array* pHintStreamRange = pDict->GetArrayBy("H");
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

  if (!IsDataAvail(szHSStart, szHSLength, pHints))
    return FALSE;

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

  FX_DWORD parser_objnum = FXSYS_atoui(word);
  if (objnum && parser_objnum != objnum)
    return nullptr;

  word = m_syntaxParser.GetNextWord(&bIsNumber);
  if (!bIsNumber)
    return nullptr;

  FX_DWORD gennum = FXSYS_atoui(word);
  if (m_syntaxParser.GetKeyword() != "obj") {
    m_syntaxParser.RestorePos(SavedPos);
    return nullptr;
  }

  CPDF_Object* pObj =
      m_syntaxParser.GetObject(pObjList, parser_objnum, gennum, true);
  m_syntaxParser.RestorePos(SavedPos);
  return pObj;
}

IPDF_DataAvail::DocLinearizationStatus CPDF_DataAvail::IsLinearizedPDF() {
  FX_DWORD req_size = 1024;
  if (!m_pFileAvail->IsDataAvail(0, req_size))
    return LinearizationUnknown;

  if (!m_pFileRead)
    return NotLinearized;

  FX_FILESIZE dwSize = m_pFileRead->GetSize();
  if (dwSize < (FX_FILESIZE)req_size)
    return LinearizationUnknown;

  uint8_t buffer[1024];
  m_pFileRead->ReadBlock(buffer, 0, req_size);
  if (IsLinearizedFile(buffer, req_size))
    return Linearized;

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

  FX_DWORD objnum = FXSYS_atoui(wordObjNum);
  if (m_pLinearized) {
    m_pLinearized->Release();
    m_pLinearized = nullptr;
  }

  m_pLinearized =
      ParseIndirectObjectAt(m_syntaxParser.m_HeaderOffset + 9, objnum);
  if (!m_pLinearized)
    return FALSE;

  CPDF_Dictionary* pDict = m_pLinearized->GetDict();
  if (pDict && pDict->GetElement("Linearized")) {
    CPDF_Object* pLen = pDict->GetElement("L");
    if (!pLen)
      return FALSE;

    if ((FX_FILESIZE)pLen->GetInteger() != m_pFileRead->GetSize())
      return FALSE;

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
    m_parser.m_pSyntax->InitParser(file.get(), 0);

    bool bNumber;
    CFX_ByteString objnum = m_parser.m_pSyntax->GetNextWord(&bNumber);
    if (!bNumber)
      return -1;

    FX_DWORD objNum = FXSYS_atoui(objnum);
    CPDF_Object* pObj = m_parser.ParseIndirectObjectAt(nullptr, 0, objNum);
    if (!pObj) {
      m_Pos += m_parser.m_pSyntax->SavePos();
      return 0;
    }

    CPDF_Dictionary* pDict = pObj->GetDict();
    CPDF_Name* pName = ToName(pDict ? pDict->GetElement("Type") : nullptr);
    if (pName) {
      if (pName->GetString() == "XRef") {
        m_Pos += m_parser.m_pSyntax->SavePos();
        xref_offset = pObj->GetDict()->GetIntegerBy("Prev");
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
  if (pos >= m_dwFileLen)
    return FALSE;

  if (m_bufferOffset >= pos ||
      (FX_FILESIZE)(m_bufferOffset + m_bufferSize) <= pos) {
    FX_FILESIZE read_pos = pos;
    FX_DWORD read_size = 512;
    if ((FX_FILESIZE)read_size > m_dwFileLen)
      read_size = (FX_DWORD)m_dwFileLen;

    if ((FX_FILESIZE)(read_pos + read_size) > m_dwFileLen)
      read_pos = m_dwFileLen - read_size;

    if (!m_pFileRead->ReadBlock(m_bufferData, read_pos, read_size))
      return FALSE;

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

  if (nRet == -1)
    m_docStatus = PDF_DATAAVAIL_ERROR;
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

    if (!m_pFileRead->ReadBlock(pBuf, m_dwTrailerOffset, iSize))
      return FALSE;

    ScopedFileStream file(FX_CreateMemoryStream(pBuf, (size_t)iSize, FALSE));
    m_syntaxParser.InitParser(file.get(), 0);

    std::unique_ptr<CPDF_Object, ReleaseDeleter<CPDF_Object>> pTrailer(
        m_syntaxParser.GetObject(nullptr, 0, 0, true));
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
        if (!LoadDocPages(pHints))
          return FALSE;
        break;
      case PDF_DATAAVAIL_PAGE:
        if (!LoadDocPage(iPage, pHints))
          return FALSE;
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
    if (m_docStatus == PDF_DATAAVAIL_ERROR)
      m_docStatus = PDF_DATAAVAIL_ERROR;
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
  CFX_ByteString type = pDict->GetStringBy("Type");
  if (type == "Pages") {
    pPageNode->m_type = PDF_PAGENODE_PAGES;
    CPDF_Object* pKids = pDict->GetElement("Kids");
    if (!pKids) {
      m_docStatus = PDF_DATAAVAIL_PAGE;
      return TRUE;
    }

    switch (pKids->GetType()) {
      case CPDF_Object::REFERENCE: {
        CPDF_Reference* pKid = pKids->AsReference();
        CPDF_PageNode* pNode = new CPDF_PageNode();
        pPageNode->m_childNode.Add(pNode);
        pNode->m_dwPageNo = pKid->GetRefObjNum();
      } break;
      case CPDF_Object::ARRAY: {
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
  if (level >= kMaxPageRecursionDepth)
    return FALSE;

  int32_t iSize = pageNodes.m_childNode.GetSize();
  if (iSize <= 0 || iPage >= iSize) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return FALSE;
  }

  for (int32_t i = 0; i < iSize; ++i) {
    CPDF_PageNode* pNode = pageNodes.m_childNode.GetAt(i);
    if (!pNode)
      continue;

    switch (pNode->m_type) {
      case PDF_PAGENODE_UNKNOWN:
        if (!CheckUnkownPageNode(pNode->m_dwPageNo, pNode, pHints)) {
          return FALSE;
        }
        --i;
        break;
      case PDF_PAGENODE_PAGE:
        iCount++;
        if (iPage == iCount && m_pDocument)
          m_pDocument->m_PageList.SetAt(iPage, pNode->m_dwPageNo);
        break;
      case PDF_PAGENODE_PAGES:
        if (!CheckPageNode(*pNode, iPage, iCount, pHints, level + 1))
          return FALSE;
        break;
      case PDF_PAGENODE_ARRAY:
        if (!CheckArrayPageNode(pNode->m_dwPageNo, pNode, pHints))
          return FALSE;
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

  if (!pPages)
    return FALSE;

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

  int count = pPagesDict->GetIntegerBy("Count");
  if (count > 0) {
    pPages->Release();
    return TRUE;
  }

  pPages->Release();
  return FALSE;
}

FX_BOOL CPDF_DataAvail::LoadDocPages(IFX_DownloadHints* pHints) {
  if (!CheckUnkownPageNode(m_PagesObjNum, &m_pageNodes, pHints))
    return FALSE;

  if (CheckPageCount(pHints)) {
    m_docStatus = PDF_DATAAVAIL_PAGE;
    return TRUE;
  }

  m_bTotalLoadPageTree = TRUE;
  return FALSE;
}

FX_BOOL CPDF_DataAvail::LoadPages(IFX_DownloadHints* pHints) {
  while (!m_bPagesTreeLoad) {
    if (!CheckPageStatus(pHints))
      return FALSE;
  }

  if (m_bPagesLoad)
    return TRUE;

  m_pDocument->LoadPages();
  return FALSE;
}

IPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckLinearizedData(
    IFX_DownloadHints* pHints) {
  if (m_bLinearedDataOK)
    return DataAvailable;

  if (!m_bMainXRefLoadTried) {
    FX_SAFE_DWORD data_size = m_dwFileLen;
    data_size -= m_dwLastXRefOffset;
    if (!data_size.IsValid())
      return DataError;

    if (!m_pFileAvail->IsDataAvail(m_dwLastXRefOffset,
                                   data_size.ValueOrDie())) {
      pHints->AddSegment(m_dwLastXRefOffset, data_size.ValueOrDie());
      return DataNotAvailable;
    }

    CPDF_Parser::Error eRet =
        m_pDocument->GetParser()->LoadLinearizedMainXRefTable();
    m_bMainXRefLoadTried = TRUE;
    if (eRet != CPDF_Parser::SUCCESS)
      return DataError;

    if (!PreparePageItem())
      return DataNotAvailable;

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
    if (!pPageDict)
      return TRUE;

    CPDF_Object* pAnnots = pPageDict->GetElement("Annots");
    if (!pAnnots)
      return TRUE;

    CFX_ArrayTemplate<CPDF_Object*> obj_array;
    obj_array.Add(pAnnots);

    FX_BOOL bRet = IsObjectsAvail(obj_array, FALSE, pHints, m_objs_array);
    if (bRet)
      m_objs_array.RemoveAll();

    return bRet;
  }

  CFX_ArrayTemplate<CPDF_Object*> new_objs_array;
  FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
  m_objs_array.RemoveAll();
  if (!bRet)
    m_objs_array.Append(new_objs_array);

  return bRet;
}

IPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckLinearizedFirstPage(
    int32_t iPage,
    IFX_DownloadHints* pHints) {
  if (!m_bAnnotsLoad) {
    if (!CheckPageAnnots(iPage, pHints))
      return DataNotAvailable;
    m_bAnnotsLoad = TRUE;
  }

  DocAvailStatus nRet = CheckLinearizedData(pHints);
  if (nRet == DataAvailable)
    m_bPageLoadedOK = FALSE;
  return nRet;
}

FX_BOOL CPDF_DataAvail::HaveResourceAncestor(CPDF_Dictionary* pDict) {
  CFX_AutoRestorer<int> restorer(&s_CurrentDataAvailRecursionDepth);
  if (++s_CurrentDataAvailRecursionDepth > kMaxDataAvailRecursionDepth)
    return FALSE;

  CPDF_Object* pParent = pDict->GetElement("Parent");
  if (!pParent)
    return FALSE;

  CPDF_Dictionary* pParentDict = pParent->GetDict();
  if (!pParentDict)
    return FALSE;

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
  if (!m_pDocument)
    return DataError;

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
    if (nResult != DataAvailable)
      return nResult;

    if (m_pHintTables) {
      nResult = m_pHintTables->CheckPage(iPage, pHints);
      if (nResult != DataAvailable)
        return nResult;
      m_pagesLoadState.insert(iPage);
      return DataAvailable;
    }

    if (m_bMainXRefLoadedOK) {
      if (m_bTotalLoadPageTree) {
        if (!LoadPages(pHints))
          return DataNotAvailable;
      } else {
        if (!m_bCurPageDictLoadOK && !CheckPage(iPage, pHints))
          return DataNotAvailable;
      }
    } else {
      if (!LoadAllFile(pHints))
        return DataNotAvailable;
      m_pDocument->GetParser()->RebuildCrossRef();
      ResetFirstCheck(iPage);
      return DataAvailable;
    }
  } else {
    if (!m_bTotalLoadPageTree && !m_bCurPageDictLoadOK &&
        !CheckPage(iPage, pHints)) {
      return DataNotAvailable;
    }
  }

  if (m_bHaveAcroForm && !m_bAcroFormLoad) {
    if (!CheckAcroFormSubObject(pHints))
      return DataNotAvailable;
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
    if (!CheckPageAnnots(iPage, pHints))
      return DataNotAvailable;
    m_bAnnotsLoad = TRUE;
  }

  if (m_pPageDict && !m_bNeedDownLoadResource) {
    m_pPageResource = m_pPageDict->GetElement("Resources");
    if (!m_pPageResource)
      m_bNeedDownLoadResource = HaveResourceAncestor(m_pPageDict);
    else
      m_bNeedDownLoadResource = TRUE;
  }

  if (m_bNeedDownLoadResource) {
    FX_BOOL bRet = CheckResources(pHints);
    if (!bRet)
      return DataNotAvailable;
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
    if (bRet)
      m_objs_array.RemoveAll();
    return bRet;
  }

  CFX_ArrayTemplate<CPDF_Object*> new_objs_array;
  FX_BOOL bRet = IsObjectsAvail(m_objs_array, FALSE, pHints, new_objs_array);
  m_objs_array.RemoveAll();
  if (!bRet)
    m_objs_array.Append(new_objs_array);
  return bRet;
}

void CPDF_DataAvail::GetLinearizedMainXRefInfo(FX_FILESIZE* pPos,
                                               FX_DWORD* pSize) {
  if (pPos)
    *pPos = m_dwLastXRefOffset;
  if (pSize)
    *pSize = (FX_DWORD)(m_dwFileLen - m_dwLastXRefOffset);
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
  if (!m_pDocument || index < 0 || index >= GetPageCount())
    return nullptr;

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
      if (!bPagePosGot)
        return nullptr;

      m_syntaxParser.InitParser(m_pFileRead, (FX_DWORD)szPageStartPos);
      CPDF_Object* pPageDict = ParseIndirectObjectAt(0, dwObjNum, m_pDocument);
      if (!pPageDict)
        return nullptr;

      if (!m_pDocument->InsertIndirectObject(dwObjNum, pPageDict))
        return nullptr;
      return pPageDict->GetDict();
    }
  }
  return m_pDocument->GetPage(index);
}

IPDF_DataAvail::DocFormStatus CPDF_DataAvail::IsFormAvail(
    IFX_DownloadHints* pHints) {
  if (!m_pDocument)
    return FormAvailable;

  if (!m_bLinearizedFormParamLoad) {
    CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
    if (!pRoot)
      return FormAvailable;

    CPDF_Object* pAcroForm = pRoot->GetElement("AcroForm");
    if (!pAcroForm)
      return FormNotExist;

    DocAvailStatus nDocStatus = CheckLinearizedData(pHints);
    if (nDocStatus == DataError)
      return FormError;
    if (nDocStatus == DataNotAvailable)
      return FormNotAvailable;

    if (!m_objs_array.GetSize())
      m_objs_array.Add(pAcroForm->GetDict());
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

CPDF_PageNode::CPDF_PageNode() : m_type(PDF_PAGENODE_UNKNOWN) {}

CPDF_PageNode::~CPDF_PageNode() {
  for (int32_t i = 0; i < m_childNode.GetSize(); ++i)
    delete m_childNode[i];
  m_childNode.RemoveAll();
}

CPDF_HintTables::~CPDF_HintTables() {
  m_dwDeltaNObjsArray.RemoveAll();
  m_dwNSharedObjsArray.RemoveAll();
  m_dwSharedObjNumArray.RemoveAll();
  m_dwIdentifierArray.RemoveAll();
}

FX_DWORD CPDF_HintTables::GetItemLength(
    int index,
    const std::vector<FX_FILESIZE>& szArray) {
  if (index < 0 || szArray.size() < 2 ||
      static_cast<size_t>(index) > szArray.size() - 2 ||
      szArray[index] > szArray[index + 1]) {
    return 0;
  }
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
      m_szPageOffsetArray.push_back(m_szFirstPageObjOffset);
    } else if (i == nFirstPageNum + 1) {
      if (i == 1) {
        m_szPageOffsetArray.push_back(nOffsetE);
      } else {
        m_szPageOffsetArray.push_back(m_szPageOffsetArray[i - 2] +
                                      dwPageLenArray[i - 2]);
      }
    } else {
      if (i == 0) {
        m_szPageOffsetArray.push_back(nOffsetE);
      } else {
        m_szPageOffsetArray.push_back(m_szPageOffsetArray[i - 1] +
                                      dwPageLenArray[i - 1]);
      }
    }
  }

  if (nPages > 0) {
    m_szPageOffsetArray.push_back(m_szPageOffsetArray[nPages - 1] +
                                  dwPageLenArray[nPages - 1]);
  }
  hStream->ByteAlign();

  // Number of shared objects.
  required_bits = dwSharedObjBits;
  required_bits *= pdfium::base::checked_cast<FX_DWORD>(nPages);
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;

  for (int i = 0; i < nPages; i++)
    m_dwNSharedObjsArray.Add(hStream->GetBits(dwSharedObjBits));
  hStream->ByteAlign();

  // Array of identifiers, size = nshared_objects.
  for (int i = 0; i < nPages; i++) {
    required_bits = dwSharedIdBits;
    required_bits *= m_dwNSharedObjsArray[i];
    if (!CanReadFromBitStream(hStream, required_bits))
      return FALSE;

    for (int j = 0; j < m_dwNSharedObjsArray[i]; j++)
      m_dwIdentifierArray.Add(hStream->GetBits(dwSharedIdBits));
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
        m_szSharedObjOffsetArray.push_back(m_szFirstPageObjOffset);
    } else {
      FX_SAFE_DWORD safeObjNum = dwFirstSharedObjNum;
      safeObjNum += i - m_nFirstPageSharedObjs;
      if (!safeObjNum.IsValid())
        return FALSE;

      m_dwSharedObjNumArray.Add(safeObjNum.ValueOrDie());
      if (i == m_nFirstPageSharedObjs) {
        m_szSharedObjOffsetArray.push_back(
            pdfium::base::checked_cast<int32_t>(dwFirstSharedObjLoc));
      }
    }

    if (i != 0 && i != m_nFirstPageSharedObjs) {
      FX_SAFE_INT32 safeLoc = pdfium::base::checked_cast<int32_t>(dwPrevObjLen);
      safeLoc += m_szSharedObjOffsetArray[i - 1];
      if (!safeLoc.IsValid())
        return FALSE;

      m_szSharedObjOffsetArray.push_back(safeLoc.ValueOrDie());
    }
  }

  if (dwSharedObjTotal > 0) {
    FX_SAFE_INT32 safeLoc = pdfium::base::checked_cast<int32_t>(dwCurObjLen);
    safeLoc += m_szSharedObjOffsetArray[dwSharedObjTotal - 1];
    if (!safeLoc.IsValid())
      return FALSE;

    m_szSharedObjOffsetArray.push_back(safeLoc.ValueOrDie());
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
  for (int i = 0; i < index; ++i)
    offset += m_dwNSharedObjsArray[i];

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
  if (!pOffset || !pOffset->IsNumber())
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

  CPDF_Array* pRange = m_pLinearizedDict->GetArrayBy("H");
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

  CPDF_Array* pRange = m_pLinearizedDict->GetArrayBy("H");
  if (!pRange)
    return -1;

  CPDF_Object* pStreamLen = pRange->GetElementValue(1);
  if (!pStreamLen)
    return -1;

  return pStreamLen->GetInteger();
}
