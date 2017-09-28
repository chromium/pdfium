// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_SYNTAX_PARSER_H_
#define CORE_FPDFAPI_PARSER_CPDF_SYNTAX_PARSER_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fxcrt/string_pool_template.h"
#include "core/fxcrt/weak_ptr.h"

class CPDF_CryptoHandler;
class CPDF_Dictionary;
class CPDF_IndirectObjectHolder;
class CPDF_Object;
class CPDF_ReadValidator;
class CPDF_Stream;
class IFX_SeekableReadStream;

class CPDF_SyntaxParser {
 public:
  enum class ParseType { kStrict, kLoose };

  CPDF_SyntaxParser();
  explicit CPDF_SyntaxParser(const WeakPtr<ByteStringPool>& pPool);
  ~CPDF_SyntaxParser();

  void InitParser(const RetainPtr<IFX_SeekableReadStream>& pFileAccess,
                  uint32_t HeaderOffset);

  void InitParserWithValidator(const RetainPtr<CPDF_ReadValidator>& pValidator,
                               uint32_t HeaderOffset);

  FX_FILESIZE GetPos() const { return m_Pos; }
  void SetPos(FX_FILESIZE pos) { m_Pos = std::min(pos, m_FileLen); }

  std::unique_ptr<CPDF_Object> GetObjectBody(
      CPDF_IndirectObjectHolder* pObjList,
      uint32_t objnum,
      uint32_t gennum,
      bool bDecrypt);

  std::unique_ptr<CPDF_Object> GetObjectBodyForStrict(
      CPDF_IndirectObjectHolder* pObjList,
      uint32_t objnum,
      uint32_t gennum,
      bool bDecrypt);

  std::unique_ptr<CPDF_Object> GetIndirectObject(
      CPDF_IndirectObjectHolder* pObjList,
      uint32_t objnum,
      bool bDecrypt,
      ParseType parse_type);

  ByteString GetKeyword();
  void ToNextLine();
  void ToNextWord();
  bool BackwardsSearchToWord(const ByteStringView& word, FX_FILESIZE limit);
  FX_FILESIZE FindTag(const ByteStringView& tag, FX_FILESIZE limit);
  void SetEncrypt(const RetainPtr<CPDF_CryptoHandler>& pCryptoHandler);
  bool ReadBlock(uint8_t* pBuf, uint32_t size);
  bool GetCharAt(FX_FILESIZE pos, uint8_t& ch);
  ByteString GetNextWord(bool* bIsNumber);
  ByteString PeekNextWord(bool* bIsNumber);

  RetainPtr<IFX_SeekableReadStream> GetFileAccess() const;

  const RetainPtr<CPDF_ReadValidator>& GetValidator() const {
    return m_pFileAccess;
  }

 private:
  friend class CPDF_Parser;
  friend class CPDF_DataAvail;
  friend class cpdf_syntax_parser_ReadHexString_Test;

  static const int kParserMaxRecursionDepth = 64;
  static int s_CurrentRecursionDepth;

  uint32_t GetDirectNum();
  bool ReadBlockAt(FX_FILESIZE read_pos);
  bool GetNextChar(uint8_t& ch);
  bool GetCharAtBackward(FX_FILESIZE pos, uint8_t* ch);
  void GetNextWordInternal(bool* bIsNumber);
  bool IsWholeWord(FX_FILESIZE startpos,
                   FX_FILESIZE limit,
                   const ByteStringView& tag,
                   bool checkKeyword);

  ByteString ReadString();
  ByteString ReadHexString();
  unsigned int ReadEOLMarkers(FX_FILESIZE pos);
  std::unique_ptr<CPDF_Stream> ReadStream(
      std::unique_ptr<CPDF_Dictionary> pDict,
      uint32_t objnum,
      uint32_t gennum);

  bool IsPositionRead(FX_FILESIZE pos) const;

  std::unique_ptr<CPDF_Object> GetObjectBodyInternal(
      CPDF_IndirectObjectHolder* pObjList,
      uint32_t objnum,
      uint32_t gennum,
      bool bDecrypt,
      ParseType parse_type);

  FX_FILESIZE m_Pos;
  uint32_t m_MetadataObjnum;
  RetainPtr<CPDF_ReadValidator> m_pFileAccess;
  FX_FILESIZE m_HeaderOffset;
  FX_FILESIZE m_FileLen;
  std::vector<uint8_t> m_pFileBuf;
  FX_FILESIZE m_BufOffset;
  RetainPtr<CPDF_CryptoHandler> m_pCryptoHandler;
  uint8_t m_WordBuffer[257];
  uint32_t m_WordSize;
  WeakPtr<ByteStringPool> m_pPool;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_SYNTAX_PARSER_H_
