// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_

#include <map>
#include <memory>
#include <set>

#include "core/include/fpdfapi/fpdf_objects.h"
#include "core/include/fxcrt/fx_system.h"

class CFX_Font;
class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_CryptoHandler;
class CPDF_DocPageData;
class CPDF_DocRenderData;
class CPDF_Font;
class CPDF_FontEncoding;
class CPDF_IccProfile;
class CPDF_Image;
class CPDF_Object;
class CPDF_Parser;
class CPDF_Pattern;
class CPDF_SecurityHandler;
class CPDF_StandardSecurityHandler;
class IFX_FileRead;

#define FPDFPERM_PRINT 0x0004
#define FPDFPERM_MODIFY 0x0008
#define FPDFPERM_EXTRACT 0x0010
#define FPDFPERM_ANNOT_FORM 0x0020
#define FPDFPERM_FILL_FORM 0x0100
#define FPDFPERM_EXTRACT_ACCESS 0x0200
#define FPDFPERM_ASSEMBLE 0x0400
#define FPDFPERM_PRINT_HIGH 0x0800
#define FPDF_PAGE_MAX_NUM 0xFFFFF

// Use the accessors below instead of directly accessing PDF_CharType.
extern const char PDF_CharType[256];

inline bool PDFCharIsWhitespace(uint8_t c) {
  return PDF_CharType[c] == 'W';
}
inline bool PDFCharIsNumeric(uint8_t c) {
  return PDF_CharType[c] == 'N';
}
inline bool PDFCharIsDelimiter(uint8_t c) {
  return PDF_CharType[c] == 'D';
}
inline bool PDFCharIsOther(uint8_t c) {
  return PDF_CharType[c] == 'R';
}

inline bool PDFCharIsLineEnding(uint8_t c) {
  return c == '\r' || c == '\n';
}

// Indexed by 8-bit char code, contains unicode code points.
extern const FX_WORD PDFDocEncoding[256];

class CPDF_Document : public CFX_PrivateData, public CPDF_IndirectObjectHolder {
 public:
  CPDF_Document();
  explicit CPDF_Document(CPDF_Parser* pParser);

  ~CPDF_Document();

  CPDF_Parser* GetParser() const { return m_pParser; }

  CPDF_Dictionary* GetRoot() const { return m_pRootDict; }

  CPDF_Dictionary* GetInfo() const { return m_pInfoDict; }

  void GetID(CFX_ByteString& id1, CFX_ByteString& id2) const {
    id1 = m_ID1;
    id2 = m_ID2;
  }

  int GetPageCount() const;

  CPDF_Dictionary* GetPage(int iPage);

  int GetPageIndex(FX_DWORD objnum);

  FX_DWORD GetUserPermissions(FX_BOOL bCheckRevision = FALSE) const;

  FX_BOOL IsOwner() const;

  CPDF_DocPageData* GetPageData() { return GetValidatePageData(); }

  void ClearPageData();

  void RemoveColorSpaceFromPageData(CPDF_Object* pObject);

  CPDF_DocRenderData* GetRenderData() { return GetValidateRenderData(); }

  void ClearRenderData();

  void ClearRenderFont();

  FX_BOOL IsFormStream(FX_DWORD objnum, FX_BOOL& bForm) const;

  // |pFontDict| must not be null.
  CPDF_Font* LoadFont(CPDF_Dictionary* pFontDict);

  CPDF_ColorSpace* LoadColorSpace(CPDF_Object* pCSObj,
                                  CPDF_Dictionary* pResources = NULL);

  CPDF_Pattern* LoadPattern(CPDF_Object* pObj,
                            FX_BOOL bShading,
                            const CFX_Matrix* matrix = NULL);

  CPDF_Image* LoadImageF(CPDF_Object* pObj);

  CPDF_StreamAcc* LoadFontFile(CPDF_Stream* pStream);

  CPDF_IccProfile* LoadIccProfile(CPDF_Stream* pStream);

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

  CPDF_Font* AddWindowsFont(LOGFONTA* pLogFont,
                            FX_BOOL bVert,
                            FX_BOOL bTranslateName = FALSE);
  CPDF_Font* AddWindowsFont(LOGFONTW* pLogFont,
                            FX_BOOL bVert,
                            FX_BOOL bTranslateName = FALSE);
#endif
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  CPDF_Font* AddMacFont(CTFontRef pFont,
                        FX_BOOL bVert,
                        FX_BOOL bTranslateName = FALSE);
#endif

  CPDF_Font* AddStandardFont(const FX_CHAR* font, CPDF_FontEncoding* pEncoding);

  CPDF_Font* AddFont(CFX_Font* pFont, int charset, FX_BOOL bVert);

  void CreateNewDoc();

  CPDF_Dictionary* CreateNewPage(int iPage);

  void DeletePage(int iPage);

  void LoadDoc();
  void LoadAsynDoc(CPDF_Dictionary* pLinearized);
  void LoadPages();

 protected:
  CPDF_Dictionary* m_pRootDict;

  CPDF_Dictionary* m_pInfoDict;

  CFX_ByteString m_ID1;

  CFX_ByteString m_ID2;

  FX_BOOL m_bLinearized;

  FX_DWORD m_dwFirstPageNo;

  FX_DWORD m_dwFirstPageObjNum;

  CFX_DWordArray m_PageList;

  int _GetPageCount() const;
  CPDF_Dictionary* _FindPDFPage(CPDF_Dictionary* pPages,
                                int iPage,
                                int nPagesToGo,
                                int level);
  int _FindPageIndex(CPDF_Dictionary* pNode,
                     FX_DWORD& skip_count,
                     FX_DWORD objnum,
                     int& index,
                     int level = 0);
  FX_BOOL IsContentUsedElsewhere(FX_DWORD objnum, CPDF_Dictionary* pPageDict);
  FX_BOOL CheckOCGVisible(CPDF_Dictionary* pOCG, FX_BOOL bPrinting);
  CPDF_DocPageData* GetValidatePageData();
  CPDF_DocRenderData* GetValidateRenderData();
  friend class CPDF_Creator;
  friend class CPDF_Parser;
  friend class CPDF_DataAvail;
  friend class CPDF_OCContext;

  CPDF_DocPageData* m_pDocPage;

  CPDF_DocRenderData* m_pDocRender;
};

#define PDFWORD_EOF 0
#define PDFWORD_NUMBER 1
#define PDFWORD_TEXT 2
#define PDFWORD_DELIMITER 3
#define PDFWORD_NAME 4
class CPDF_SimpleParser {
 public:
  CPDF_SimpleParser(const uint8_t* pData, FX_DWORD dwSize);

  CPDF_SimpleParser(const CFX_ByteStringC& str);

  CFX_ByteStringC GetWord();

  FX_BOOL SearchToken(const CFX_ByteStringC& token);

  FX_BOOL SkipWord(const CFX_ByteStringC& token);

  FX_BOOL FindTagPair(const CFX_ByteStringC& start_token,
                      const CFX_ByteStringC& end_token,
                      FX_DWORD& start_pos,
                      FX_DWORD& end_pos);

  FX_BOOL FindTagParam(const CFX_ByteStringC& token, int nParams);

  FX_DWORD GetPos() { return m_dwCurPos; }

  void SetPos(FX_DWORD pos) {
    ASSERT(pos <= m_dwSize);
    m_dwCurPos = pos;
  }

 private:
  void ParseWord(const uint8_t*& pStart, FX_DWORD& dwSize, int& type);

  const uint8_t* m_pData;

  FX_DWORD m_dwSize;

  FX_DWORD m_dwCurPos;
};
class CPDF_SyntaxParser {
 public:
  CPDF_SyntaxParser();
  virtual ~CPDF_SyntaxParser();

  void InitParser(IFX_FileRead* pFileAccess, FX_DWORD HeaderOffset);

  FX_FILESIZE SavePos() { return m_Pos; }

  void RestorePos(FX_FILESIZE pos) { m_Pos = pos; }

  CPDF_Object* GetObject(CPDF_IndirectObjectHolder* pObjList,
                         FX_DWORD objnum,
                         FX_DWORD gennum,
                         PARSE_CONTEXT* pContext,
                         FX_BOOL bDecrypt);

  CPDF_Object* GetObjectByStrict(CPDF_IndirectObjectHolder* pObjList,
                                 FX_DWORD objnum,
                                 FX_DWORD gennum,
                                 PARSE_CONTEXT* pContext);

  int GetDirectNum();

  CFX_ByteString GetString(FX_DWORD objnum, FX_DWORD gennum);

  CFX_ByteString GetName();

  CFX_ByteString GetKeyword();

  void GetBinary(uint8_t* buffer, FX_DWORD size);

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

  void SetEncrypt(CPDF_CryptoHandler* pCryptoHandler) {
    m_pCryptoHandler.reset(pCryptoHandler);
  }

  FX_BOOL IsEncrypted() { return m_pCryptoHandler != NULL; }

  FX_BOOL GetCharAt(FX_FILESIZE pos, uint8_t& ch);

  FX_BOOL ReadBlock(uint8_t* pBuf, FX_DWORD size);

  CFX_ByteString GetNextWord(bool* bIsNumber);

 protected:
  friend class CPDF_Parser;
  friend class CPDF_DataAvail;
  friend class fpdf_parser_parser_ReadHexString_Test;

  static const int kParserMaxRecursionDepth = 64;
  static int s_CurrentRecursionDepth;

  virtual FX_BOOL GetNextChar(uint8_t& ch);

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
                          PARSE_CONTEXT* pContext,
                          FX_DWORD objnum,
                          FX_DWORD gennum);

  FX_FILESIZE m_Pos;

  FX_BOOL m_bFileStream;

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

  FX_FILESIZE m_dwWordPos;
};

#define PDFPARSE_TYPEONLY 1
#define PDFPARSE_NOSTREAM 2
struct PARSE_CONTEXT {
  uint32_t m_Flags;
  FX_FILESIZE m_DictStart;
  FX_FILESIZE m_DictEnd;
  FX_FILESIZE m_DataStart;
  FX_FILESIZE m_DataEnd;
};

#define PDFPARSE_ERROR_SUCCESS 0
#define PDFPARSE_ERROR_FILE 1
#define PDFPARSE_ERROR_FORMAT 2
#define PDFPARSE_ERROR_PASSWORD 3
#define PDFPARSE_ERROR_HANDLER 4
#define PDFPARSE_ERROR_CERT 5

class CPDF_Parser {
 public:
  CPDF_Parser();
  ~CPDF_Parser();

  FX_DWORD StartParse(IFX_FileRead* pFile,
                      FX_BOOL bReParse = FALSE,
                      FX_BOOL bOwnFileRead = TRUE);

  void CloseParser(FX_BOOL bReParse = FALSE);

  FX_DWORD GetPermissions(FX_BOOL bCheckRevision = FALSE);

  FX_BOOL IsOwner();

  void SetPassword(const FX_CHAR* password) { m_Password = password; }

  CFX_ByteString GetPassword() { return m_Password; }

  CPDF_CryptoHandler* GetCryptoHandler() {
    return m_Syntax.m_pCryptoHandler.get();
  }

  void SetSecurityHandler(CPDF_SecurityHandler* pSecurityHandler,
                          FX_BOOL bForced = FALSE);

  CPDF_Dictionary* GetTrailer() { return m_pTrailer; }

  FX_FILESIZE GetLastXRefOffset() { return m_LastXRefOffset; }

  CPDF_Document* GetDocument() { return m_pDocument; }

  FX_DWORD GetRootObjNum();
  FX_DWORD GetInfoObjNum();
  CPDF_Array* GetIDArray();

  CPDF_Dictionary* GetEncryptDict() { return m_pEncryptDict; }

  CPDF_Object* ParseIndirectObject(CPDF_IndirectObjectHolder* pObjList,
                                   FX_DWORD objnum,
                                   PARSE_CONTEXT* pContext = NULL);
  FX_DWORD GetLastObjNum() const;
  bool IsValidObjectNumber(FX_DWORD objnum) const;
  FX_BOOL IsFormStream(FX_DWORD objnum, FX_BOOL& bForm);

  FX_FILESIZE GetObjectOffset(FX_DWORD objnum) const;

  FX_FILESIZE GetObjectSize(FX_DWORD objnum) const;

  void GetIndirectBinary(FX_DWORD objnum, uint8_t*& pBuffer, FX_DWORD& size);

  IFX_FileRead* GetFileAccess() const { return m_Syntax.m_pFileAccess; }

  int GetFileVersion() const { return m_FileVersion; }

  FX_BOOL IsXRefStream() const { return m_bXRefStream; }
  CPDF_Object* ParseIndirectObjectAt(CPDF_IndirectObjectHolder* pObjList,
                                     FX_FILESIZE pos,
                                     FX_DWORD objnum,
                                     PARSE_CONTEXT* pContext);

  CPDF_Object* ParseIndirectObjectAtByStrict(
      CPDF_IndirectObjectHolder* pObjList,
      FX_FILESIZE pos,
      FX_DWORD objnum,
      PARSE_CONTEXT* pContext,
      FX_FILESIZE* pResultPos);

  FX_DWORD StartAsynParse(IFX_FileRead* pFile,
                          FX_BOOL bReParse = FALSE,
                          FX_BOOL bOwnFileRead = TRUE);

  FX_DWORD GetFirstPageNo() { return m_dwFirstPageNo; }

 protected:
  CPDF_Object* ParseDirect(CPDF_Object* pObj);

  FX_BOOL LoadAllCrossRefV4(FX_FILESIZE pos);

  FX_BOOL LoadAllCrossRefV5(FX_FILESIZE pos);

  bool LoadCrossRefV4(FX_FILESIZE pos, FX_FILESIZE streampos, FX_BOOL bSkip);

  FX_BOOL LoadCrossRefV5(FX_FILESIZE* pos, FX_BOOL bMainXRef);

  CPDF_Dictionary* LoadTrailerV4();

  FX_BOOL RebuildCrossRef();

  FX_DWORD SetEncryptHandler();

  void ReleaseEncryptHandler();

  FX_BOOL LoadLinearizedAllCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount);

  FX_BOOL LoadLinearizedCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount);

  FX_BOOL LoadLinearizedAllCrossRefV5(FX_FILESIZE pos);

  FX_DWORD LoadLinearizedMainXRefTable();

  CPDF_StreamAcc* GetObjectStream(FX_DWORD number);

  FX_BOOL IsLinearizedFile(IFX_FileRead* pFileAccess, FX_DWORD offset);

  bool FindPosInOffsets(FX_FILESIZE pos) const;

  void SetEncryptDictionary(CPDF_Dictionary* pDict);

  FX_FILESIZE GetObjectPositionOrZero(FX_DWORD objnum) const;
  void ShrinkObjectMap(FX_DWORD size);

  CPDF_Document* m_pDocument;

  CPDF_SyntaxParser m_Syntax;
  FX_BOOL m_bOwnFileRead;
  int m_FileVersion;

  CPDF_Dictionary* m_pTrailer;

  CPDF_Dictionary* m_pEncryptDict;

  FX_FILESIZE m_LastXRefOffset;

  FX_BOOL m_bXRefStream;

  std::unique_ptr<CPDF_SecurityHandler> m_pSecurityHandler;

  FX_BOOL m_bForceUseSecurityHandler;

  CFX_ByteString m_bsRecipient;

  CFX_ByteString m_FilePath;

  CFX_ByteString m_Password;

  struct ObjectInfo {
    ObjectInfo() : pos(0) {}

    FX_FILESIZE pos;
// TODO(thestig): Use fields below in place of |m_V5Type| and |m_ObjVersion|
#if 0
    uint8_t type;
    uint16_t gennum;
#endif
  };
  std::map<FX_DWORD, ObjectInfo> m_ObjectInfo;

  CFX_ByteArray m_V5Type;
  CFX_WordArray m_ObjVersion;

  CFX_FileSizeArray m_SortedOffset;

  CFX_ArrayTemplate<CPDF_Dictionary*> m_Trailers;

  FX_BOOL m_bVersionUpdated;

  CPDF_Object* m_pLinearized;

  FX_DWORD m_dwFirstPageNo;

  FX_DWORD m_dwXrefStartObjNum;

  // A map of object numbers to indirect streams. Map owns the streams.
  CFX_MapPtrToPtr m_ObjectStreamMap;

  // Mapping of object numbers to offsets. The offsets are relative to the first
  // object in the stream.
  using StreamObjectCache = std::map<FX_DWORD, FX_DWORD>;

  // Mapping of streams to their object caches. This is valid as long as the
  // streams in |m_ObjectStreamMap| are valid.
  std::map<CPDF_StreamAcc*, StreamObjectCache> m_ObjCache;

  // All indirect object numbers that are being parsed.
  std::set<FX_DWORD> m_ParsingObjNums;

  friend class CPDF_Creator;
  friend class CPDF_DataAvail;
};

#define FXCIPHER_NONE 0
#define FXCIPHER_RC4 1
#define FXCIPHER_AES 2
#define FXCIPHER_AES2 3
class CPDF_SecurityHandler {
 public:
  virtual ~CPDF_SecurityHandler() {}

  virtual FX_BOOL OnInit(CPDF_Parser* pParser,
                         CPDF_Dictionary* pEncryptDict) = 0;

  virtual FX_DWORD GetPermissions() = 0;

  virtual FX_BOOL IsOwner() = 0;

  virtual FX_BOOL GetCryptInfo(int& cipher,
                               const uint8_t*& buffer,
                               int& keylen) = 0;

  virtual FX_BOOL IsMetadataEncrypted() { return TRUE; }

  virtual CPDF_CryptoHandler* CreateCryptoHandler() = 0;

  virtual CPDF_StandardSecurityHandler* GetStandardHandler() { return NULL; }
};
#define PDF_ENCRYPT_CONTENT 0
class CPDF_StandardSecurityHandler : public CPDF_SecurityHandler {
 public:
  CPDF_StandardSecurityHandler();
  ~CPDF_StandardSecurityHandler() override;

  // CPDF_SecurityHandler
  FX_BOOL OnInit(CPDF_Parser* pParser, CPDF_Dictionary* pEncryptDict) override;
  FX_DWORD GetPermissions() override;
  FX_BOOL IsOwner() override { return m_bOwner; }
  FX_BOOL GetCryptInfo(int& cipher,
                       const uint8_t*& buffer,
                       int& keylen) override;
  FX_BOOL IsMetadataEncrypted() override;
  CPDF_CryptoHandler* CreateCryptoHandler() override;
  CPDF_StandardSecurityHandler* GetStandardHandler() override { return this; }

  void OnCreate(CPDF_Dictionary* pEncryptDict,
                CPDF_Array* pIdArray,
                const uint8_t* user_pass,
                FX_DWORD user_size,
                const uint8_t* owner_pass,
                FX_DWORD owner_size,
                FX_DWORD type = PDF_ENCRYPT_CONTENT);

  void OnCreate(CPDF_Dictionary* pEncryptDict,
                CPDF_Array* pIdArray,
                const uint8_t* user_pass,
                FX_DWORD user_size,
                FX_DWORD type = PDF_ENCRYPT_CONTENT);

  CFX_ByteString GetUserPassword(const uint8_t* owner_pass, FX_DWORD pass_size);
  CFX_ByteString GetUserPassword(const uint8_t* owner_pass,
                                 FX_DWORD pass_size,
                                 int32_t key_len);
  int GetVersion() { return m_Version; }
  int GetRevision() { return m_Revision; }

  int CheckPassword(const uint8_t* password,
                    FX_DWORD pass_size,
                    FX_BOOL bOwner,
                    uint8_t* key);
  int CheckPassword(const uint8_t* password,
                    FX_DWORD pass_size,
                    FX_BOOL bOwner,
                    uint8_t* key,
                    int key_len);

 private:
  int m_Version;

  int m_Revision;

  CPDF_Parser* m_pParser;

  CPDF_Dictionary* m_pEncryptDict;

  FX_BOOL LoadDict(CPDF_Dictionary* pEncryptDict);
  FX_BOOL LoadDict(CPDF_Dictionary* pEncryptDict,
                   FX_DWORD type,
                   int& cipher,
                   int& key_len);

  FX_BOOL CheckUserPassword(const uint8_t* password,
                            FX_DWORD pass_size,
                            FX_BOOL bIgnoreEncryptMeta,
                            uint8_t* key,
                            int32_t key_len);

  FX_BOOL CheckOwnerPassword(const uint8_t* password,
                             FX_DWORD pass_size,
                             uint8_t* key,
                             int32_t key_len);
  FX_BOOL AES256_CheckPassword(const uint8_t* password,
                               FX_DWORD size,
                               FX_BOOL bOwner,
                               uint8_t* key);
  void AES256_SetPassword(CPDF_Dictionary* pEncryptDict,
                          const uint8_t* password,
                          FX_DWORD size,
                          FX_BOOL bOwner,
                          const uint8_t* key);
  void AES256_SetPerms(CPDF_Dictionary* pEncryptDict,
                       FX_DWORD permission,
                       FX_BOOL bEncryptMetadata,
                       const uint8_t* key);
  void OnCreate(CPDF_Dictionary* pEncryptDict,
                CPDF_Array* pIdArray,
                const uint8_t* user_pass,
                FX_DWORD user_size,
                const uint8_t* owner_pass,
                FX_DWORD owner_size,
                FX_BOOL bDefault,
                FX_DWORD type);
  FX_BOOL CheckSecurity(int32_t key_len);

  FX_BOOL m_bOwner;

  FX_DWORD m_Permissions;

  int m_Cipher;

  uint8_t m_EncryptKey[32];

  int m_KeyLen;
};
class CPDF_CryptoHandler {
 public:
  virtual ~CPDF_CryptoHandler() {}

  virtual FX_BOOL Init(CPDF_Dictionary* pEncryptDict,
                       CPDF_SecurityHandler* pSecurityHandler) = 0;

  virtual FX_DWORD DecryptGetSize(FX_DWORD src_size) = 0;

  virtual void* DecryptStart(FX_DWORD objnum, FX_DWORD gennum) = 0;

  virtual FX_BOOL DecryptStream(void* context,
                                const uint8_t* src_buf,
                                FX_DWORD src_size,
                                CFX_BinaryBuf& dest_buf) = 0;

  virtual FX_BOOL DecryptFinish(void* context, CFX_BinaryBuf& dest_buf) = 0;

  virtual FX_DWORD EncryptGetSize(FX_DWORD objnum,
                                  FX_DWORD version,
                                  const uint8_t* src_buf,
                                  FX_DWORD src_size) = 0;

  virtual FX_BOOL EncryptContent(FX_DWORD objnum,
                                 FX_DWORD version,
                                 const uint8_t* src_buf,
                                 FX_DWORD src_size,
                                 uint8_t* dest_buf,
                                 FX_DWORD& dest_size) = 0;

  void Decrypt(FX_DWORD objnum, FX_DWORD version, CFX_ByteString& str);
};
class CPDF_StandardCryptoHandler : public CPDF_CryptoHandler {
 public:
  CPDF_StandardCryptoHandler();
  ~CPDF_StandardCryptoHandler() override;

  // CPDF_CryptoHandler
  FX_BOOL Init(CPDF_Dictionary* pEncryptDict,
               CPDF_SecurityHandler* pSecurityHandler) override;
  FX_DWORD DecryptGetSize(FX_DWORD src_size) override;
  void* DecryptStart(FX_DWORD objnum, FX_DWORD gennum) override;
  FX_BOOL DecryptStream(void* context,
                        const uint8_t* src_buf,
                        FX_DWORD src_size,
                        CFX_BinaryBuf& dest_buf) override;
  FX_BOOL DecryptFinish(void* context, CFX_BinaryBuf& dest_buf) override;
  FX_DWORD EncryptGetSize(FX_DWORD objnum,
                          FX_DWORD version,
                          const uint8_t* src_buf,
                          FX_DWORD src_size) override;
  FX_BOOL EncryptContent(FX_DWORD objnum,
                         FX_DWORD version,
                         const uint8_t* src_buf,
                         FX_DWORD src_size,
                         uint8_t* dest_buf,
                         FX_DWORD& dest_size) override;

  FX_BOOL Init(int cipher, const uint8_t* key, int keylen);

 protected:
  virtual void CryptBlock(FX_BOOL bEncrypt,
                          FX_DWORD objnum,
                          FX_DWORD gennum,
                          const uint8_t* src_buf,
                          FX_DWORD src_size,
                          uint8_t* dest_buf,
                          FX_DWORD& dest_size);
  virtual void* CryptStart(FX_DWORD objnum, FX_DWORD gennum, FX_BOOL bEncrypt);
  virtual FX_BOOL CryptStream(void* context,
                              const uint8_t* src_buf,
                              FX_DWORD src_size,
                              CFX_BinaryBuf& dest_buf,
                              FX_BOOL bEncrypt);
  virtual FX_BOOL CryptFinish(void* context,
                              CFX_BinaryBuf& dest_buf,
                              FX_BOOL bEncrypt);

  uint8_t m_EncryptKey[32];

  int m_KeyLen;

  int m_Cipher;

  uint8_t* m_pAESContext;
};
class CPDF_Point {
 public:
  CPDF_Point(FX_FLOAT xx, FX_FLOAT yy) {
    x = xx;
    y = yy;
  }

  FX_FLOAT x;

  FX_FLOAT y;
};

#define CPDF_Rect CFX_FloatRect
CFX_ByteString PDF_NameDecode(const CFX_ByteStringC& orig);
CFX_ByteString PDF_NameDecode(const CFX_ByteString& orig);
CFX_ByteString PDF_NameEncode(const CFX_ByteString& orig);
CFX_ByteString PDF_EncodeString(const CFX_ByteString& src,
                                FX_BOOL bHex = FALSE);
CFX_WideString PDF_DecodeText(const uint8_t* pData,
                              FX_DWORD size,
                              CFX_CharMap* pCharMap = NULL);
inline CFX_WideString PDF_DecodeText(const CFX_ByteString& bstr,
                                     CFX_CharMap* pCharMap = NULL) {
  return PDF_DecodeText((const uint8_t*)bstr.c_str(), bstr.GetLength(),
                        pCharMap);
}
CFX_ByteString PDF_EncodeText(const FX_WCHAR* pString,
                              int len = -1,
                              CFX_CharMap* pCharMap = NULL);
inline CFX_ByteString PDF_EncodeText(const CFX_WideString& str,
                                     CFX_CharMap* pCharMap = NULL) {
  return PDF_EncodeText(str.c_str(), str.GetLength(), pCharMap);
}
FX_FLOAT PDF_ClipFloat(FX_FLOAT f);
class CFDF_Document : public CPDF_IndirectObjectHolder {
 public:
  static CFDF_Document* CreateNewDoc();
  static CFDF_Document* ParseFile(IFX_FileRead* pFile,
                                  FX_BOOL bOwnFile = FALSE);
  static CFDF_Document* ParseMemory(const uint8_t* pData, FX_DWORD size);

  ~CFDF_Document();

  FX_BOOL WriteBuf(CFX_ByteTextBuf& buf) const;

  CPDF_Dictionary* GetRoot() const { return m_pRootDict; }

  CFX_WideString GetWin32Path() const;

 protected:
  CFDF_Document();
  void ParseStream(IFX_FileRead* pFile, FX_BOOL bOwnFile);
  CPDF_Dictionary* m_pRootDict;
  IFX_FileRead* m_pFile;
  FX_BOOL m_bOwnFile;
};

CFX_WideString FPDF_FileSpec_GetWin32Path(const CPDF_Object* pFileSpec);
void FPDF_FileSpec_SetWin32Path(CPDF_Object* pFileSpec,
                                const CFX_WideString& fullpath);

void FlateEncode(const uint8_t* src_buf,
                 FX_DWORD src_size,
                 uint8_t*& dest_buf,
                 FX_DWORD& dest_size);
void FlateEncode(const uint8_t* src_buf,
                 FX_DWORD src_size,
                 int predictor,
                 int Colors,
                 int BitsPerComponent,
                 int Columns,
                 uint8_t*& dest_buf,
                 FX_DWORD& dest_size);
FX_DWORD FlateDecode(const uint8_t* src_buf,
                     FX_DWORD src_size,
                     uint8_t*& dest_buf,
                     FX_DWORD& dest_size);
FX_DWORD RunLengthDecode(const uint8_t* src_buf,
                         FX_DWORD src_size,
                         uint8_t*& dest_buf,
                         FX_DWORD& dest_size);
bool IsSignatureDict(const CPDF_Dictionary* pDict);

class CPDF_NumberTree {
 public:
  CPDF_NumberTree(CPDF_Dictionary* pRoot) { m_pRoot = pRoot; }

  CPDF_Object* LookupValue(int num);

 protected:
  CPDF_Dictionary* m_pRoot;
};

class IFX_FileAvail {
 public:
  virtual ~IFX_FileAvail() {}
  virtual FX_BOOL IsDataAvail(FX_FILESIZE offset, FX_DWORD size) = 0;
};
class IFX_DownloadHints {
 public:
  virtual ~IFX_DownloadHints() {}
  virtual void AddSegment(FX_FILESIZE offset, FX_DWORD size) = 0;
};

class IPDF_DataAvail {
 public:
  // Must match PDF_DATA_* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/src/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocAvailStatus {
    DataError = -1,        // PDF_DATA_ERROR
    DataNotAvailable = 0,  // PDF_DATA_NOTAVAIL
    DataAvailable = 1,     // PDF_DATA_AVAIL
  };

  // Must match PDF_*LINEAR* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/src/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocLinearizationStatus {
    LinearizationUnknown = -1,  // PDF_LINEARIZATION_UNKNOWN
    NotLinearized = 0,          // PDF_NOT_LINEARIZED
    Linearized = 1,             // PDF_LINEARIZED
  };

  // Must match PDF_FORM_* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/src/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocFormStatus {
    FormError = -1,        // PDF_FORM_ERROR
    FormNotAvailable = 0,  // PDF_FORM_NOTAVAIL
    FormAvailable = 1,     // PDF_FORM_AVAIL
    FormNotExist = 2,      // PDF_FORM_NOTEXIST
  };

  static IPDF_DataAvail* Create(IFX_FileAvail* pFileAvail,
                                IFX_FileRead* pFileRead);
  virtual ~IPDF_DataAvail() {}

  IFX_FileAvail* GetFileAvail() const { return m_pFileAvail; }
  IFX_FileRead* GetFileRead() const { return m_pFileRead; }

  virtual DocAvailStatus IsDocAvail(IFX_DownloadHints* pHints) = 0;
  virtual void SetDocument(CPDF_Document* pDoc) = 0;
  virtual DocAvailStatus IsPageAvail(int iPage, IFX_DownloadHints* pHints) = 0;
  virtual FX_BOOL IsLinearized() = 0;
  virtual DocFormStatus IsFormAvail(IFX_DownloadHints* pHints) = 0;
  virtual DocLinearizationStatus IsLinearizedPDF() = 0;
  virtual void GetLinearizedMainXRefInfo(FX_FILESIZE* pPos,
                                         FX_DWORD* pSize) = 0;

 protected:
  IPDF_DataAvail(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead);

  IFX_FileAvail* m_pFileAvail;
  IFX_FileRead* m_pFileRead;
};

enum PDF_PAGENODE_TYPE {
  PDF_PAGENODE_UNKOWN = 0,
  PDF_PAGENODE_PAGE,
  PDF_PAGENODE_PAGES,
  PDF_PAGENODE_ARRAY,
};
class CPDF_PageNode {
 public:
  CPDF_PageNode() : m_type(PDF_PAGENODE_UNKOWN) {}
  ~CPDF_PageNode();
  PDF_PAGENODE_TYPE m_type;
  FX_DWORD m_dwPageNo;
  CFX_ArrayTemplate<CPDF_PageNode*> m_childNode;
};
enum PDF_DATAAVAIL_STATUS {
  PDF_DATAAVAIL_HEADER = 0,
  PDF_DATAAVAIL_FIRSTPAGE,
  PDF_DATAAVAIL_FIRSTPAGE_PREPARE,
  PDF_DATAAVAIL_HINTTABLE,
  PDF_DATAAVAIL_END,
  PDF_DATAAVAIL_CROSSREF,
  PDF_DATAAVAIL_CROSSREF_ITEM,
  PDF_DATAAVAIL_CROSSREF_STREAM,
  PDF_DATAAVAIL_TRAILER,
  PDF_DATAAVAIL_LOADALLCROSSREF,
  PDF_DATAAVAIL_ROOT,
  PDF_DATAAVAIL_INFO,
  PDF_DATAAVAIL_ACROFORM,
  PDF_DATAAVAIL_ACROFORM_SUBOBJECT,
  PDF_DATAAVAIL_PAGETREE,
  PDF_DATAAVAIL_PAGE,
  PDF_DATAAVAIL_PAGE_LATERLOAD,
  PDF_DATAAVAIL_RESOURCES,
  PDF_DATAAVAIL_DONE,
  PDF_DATAAVAIL_ERROR,
  PDF_DATAAVAIL_LOADALLFILE,
  PDF_DATAAVAIL_TRAILER_APPEND
};

// Public for testing.
FX_DWORD A85Decode(const uint8_t* src_buf,
                   FX_DWORD src_size,
                   uint8_t*& dest_buf,
                   FX_DWORD& dest_size);
// Public for testing.
FX_DWORD HexDecode(const uint8_t* src_buf,
                   FX_DWORD src_size,
                   uint8_t*& dest_buf,
                   FX_DWORD& dest_size);
// Public for testing.
FX_DWORD FPDFAPI_FlateOrLZWDecode(FX_BOOL bLZW,
                                  const uint8_t* src_buf,
                                  FX_DWORD src_size,
                                  CPDF_Dictionary* pParams,
                                  FX_DWORD estimated_size,
                                  uint8_t*& dest_buf,
                                  FX_DWORD& dest_size);
FX_BOOL PDF_DataDecode(const uint8_t* src_buf,
                       FX_DWORD src_size,
                       const CPDF_Dictionary* pDict,
                       uint8_t*& dest_buf,
                       FX_DWORD& dest_size,
                       CFX_ByteString& ImageEncoding,
                       CPDF_Dictionary*& pImageParms,
                       FX_DWORD estimated_size,
                       FX_BOOL bImageAcc);

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_
