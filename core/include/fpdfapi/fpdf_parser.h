// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_

#include <map>
#include <memory>
#include <set>

#include "core/include/fpdfapi/fpdf_objects.h"
#include "core/include/fxcrt/fx_basic.h"

class CFX_Font;
class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_CryptoHandler;
class CPDF_Document;
class CPDF_DocPageData;
class CPDF_DocRenderData;
class CPDF_Font;
class CPDF_FontEncoding;
class CPDF_IccProfile;
class CPDF_Image;
class CPDF_Object;
class CPDF_Parser;
class CPDF_Pattern;
class CPDF_StandardSecurityHandler;
class CPDF_SyntaxParser;
class IPDF_SecurityHandler;

#define FPDFPERM_PRINT 0x0004
#define FPDFPERM_MODIFY 0x0008
#define FPDFPERM_EXTRACT 0x0010
#define FPDFPERM_ANNOT_FORM 0x0020
#define FPDFPERM_FILL_FORM 0x0100
#define FPDFPERM_EXTRACT_ACCESS 0x0200
#define FPDFPERM_ASSEMBLE 0x0400
#define FPDFPERM_PRINT_HIGH 0x0800
#define FPDF_PAGE_MAX_NUM 0xFFFFF

// TODO(thestig) Using unique_ptr with ReleaseDeleter is still not ideal.
// Come up or wait for something better.
using ScopedFileStream =
    std::unique_ptr<IFX_FileStream, ReleaseDeleter<IFX_FileStream>>;

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

// Indexed by 8-bit char code, contains unicode code points.
extern const FX_WORD PDFDocEncoding[256];

class CPDF_CryptoHandler {
 public:
  virtual ~CPDF_CryptoHandler() {}

  virtual FX_BOOL Init(CPDF_Dictionary* pEncryptDict,
                       IPDF_SecurityHandler* pSecurityHandler) = 0;

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
               IPDF_SecurityHandler* pSecurityHandler) override;
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

CFX_ByteString PDF_NameDecode(const CFX_ByteStringC& orig);
CFX_ByteString PDF_NameDecode(const CFX_ByteString& orig);
CFX_ByteString PDF_NameEncode(const CFX_ByteString& orig);
CFX_ByteString PDF_EncodeString(const CFX_ByteString& src,
                                FX_BOOL bHex = FALSE);
CFX_WideString PDF_DecodeText(const uint8_t* pData, FX_DWORD size);
CFX_WideString PDF_DecodeText(const CFX_ByteString& bstr);
CFX_ByteString PDF_EncodeText(const FX_WCHAR* pString, int len = -1);
CFX_ByteString PDF_EncodeText(const CFX_WideString& str);

class CFDF_Document : public CPDF_IndirectObjectHolder {
 public:
  static CFDF_Document* CreateNewDoc();
  static CFDF_Document* ParseFile(IFX_FileRead* pFile,
                                  FX_BOOL bOwnFile = FALSE);
  static CFDF_Document* ParseMemory(const uint8_t* pData, FX_DWORD size);
  ~CFDF_Document();

  FX_BOOL WriteBuf(CFX_ByteTextBuf& buf) const;
  CPDF_Dictionary* GetRoot() const { return m_pRootDict; }

 protected:
  CFDF_Document();
  void ParseStream(IFX_FileRead* pFile, FX_BOOL bOwnFile);

  CPDF_Dictionary* m_pRootDict;
  IFX_FileRead* m_pFile;
  FX_BOOL m_bOwnFile;
};

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
  PDF_PAGENODE_UNKNOWN = 0,
  PDF_PAGENODE_PAGE,
  PDF_PAGENODE_PAGES,
  PDF_PAGENODE_ARRAY,
};

class CPDF_PageNode {
 public:
  CPDF_PageNode();
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
