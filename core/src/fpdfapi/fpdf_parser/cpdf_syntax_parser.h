// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_PARSER_CPDF_SYNTAX_PARSER_H_
#define CORE_SRC_FPDFAPI_FPDF_PARSER_CPDF_SYNTAX_PARSER_H_

#include <memory>

#include "core/include/fxcrt/fx_basic.h"

class CPDF_CryptoHandler;
class CPDF_IndirectObjectHolder;
class CPDF_Object;
class CPDF_Dictionary;
class CPDF_Stream;
class IFX_FileRead;

class CPDF_SyntaxParser {
 public:
  CPDF_SyntaxParser();
  ~CPDF_SyntaxParser();

  void InitParser(IFX_FileRead* pFileAccess, FX_DWORD HeaderOffset);

  FX_FILESIZE SavePos() const { return m_Pos; }
  void RestorePos(FX_FILESIZE pos) { m_Pos = pos; }

  CPDF_Object* GetObject(CPDF_IndirectObjectHolder* pObjList,
                         FX_DWORD objnum,
                         FX_DWORD gennum,
                         FX_BOOL bDecrypt);
  CPDF_Object* GetObjectByStrict(CPDF_IndirectObjectHolder* pObjList,
                                 FX_DWORD objnum,
                                 FX_DWORD gennum);
  CFX_ByteString GetKeyword();

  void ToNextLine();
  void ToNextWord();

  FX_BOOL SearchWord(const CFX_ByteStringC& word,
                     FX_BOOL bWholeWord,
                     FX_BOOL bForward,
                     FX_FILESIZE limit);
  int SearchMultiWord(const CFX_ByteStringC& words,
                      FX_BOOL bWholeWord,
                      FX_FILESIZE limit);
  FX_FILESIZE FindTag(const CFX_ByteStringC& tag, FX_FILESIZE limit);

  void SetEncrypt(std::unique_ptr<CPDF_CryptoHandler> pCryptoHandler);

  FX_BOOL ReadBlock(uint8_t* pBuf, FX_DWORD size);
  FX_BOOL GetCharAt(FX_FILESIZE pos, uint8_t& ch);
  CFX_ByteString GetNextWord(bool* bIsNumber);

 private:
  friend class CPDF_Parser;
  friend class CPDF_DataAvail;
  friend class fpdf_parser_parser_ReadHexString_Test;

  static const int kParserMaxRecursionDepth = 64;
  static int s_CurrentRecursionDepth;

  uint32_t GetDirectNum();

  FX_BOOL GetNextChar(uint8_t& ch);
  FX_BOOL GetCharAtBackward(FX_FILESIZE pos, uint8_t& ch);
  void GetNextWordInternal(bool* bIsNumber);
  bool IsWholeWord(FX_FILESIZE startpos,
                   FX_FILESIZE limit,
                   const CFX_ByteStringC& tag,
                   FX_BOOL checkKeyword);

  CFX_ByteString ReadString();
  CFX_ByteString ReadHexString();
  unsigned int ReadEOLMarkers(FX_FILESIZE pos);
  CPDF_Stream* ReadStream(CPDF_Dictionary* pDict,
                          FX_DWORD objnum,
                          FX_DWORD gennum);

  FX_FILESIZE m_Pos;
  int m_MetadataObjnum;
  IFX_FileRead* m_pFileAccess;
  FX_DWORD m_HeaderOffset;
  FX_FILESIZE m_FileLen;
  uint8_t* m_pFileBuf;
  FX_DWORD m_BufSize;
  FX_FILESIZE m_BufOffset;
  std::unique_ptr<CPDF_CryptoHandler> m_pCryptoHandler;
  uint8_t m_WordBuffer[257];
  FX_DWORD m_WordSize;
};

#endif  // CORE_SRC_FPDFAPI_FPDF_PARSER_CPDF_SYNTAX_PARSER_H_
