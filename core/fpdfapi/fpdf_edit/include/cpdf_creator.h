// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_EDIT_INCLUDE_CPDF_CREATOR_H_
#define CORE_FPDFAPI_FPDF_EDIT_INCLUDE_CPDF_CREATOR_H_

#include "core/fxcrt/include/fx_basic.h"

class CPDF_Array;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Object;
class CPDF_Parser;
class CPDF_XRefStream;
class IPDF_CryptoHandler;

#define FPDFCREATE_INCREMENTAL 1
#define FPDFCREATE_NO_ORIGINAL 2
#define FPDFCREATE_PROGRESSIVE 4
#define FPDFCREATE_OBJECTSTREAM 8

CFX_ByteTextBuf& operator<<(CFX_ByteTextBuf& buf, const CPDF_Object* pObj);

class CPDF_Creator {
 public:
  CPDF_Creator(CPDF_Document* pDoc);
  ~CPDF_Creator();

  void RemoveSecurity();
  bool Create(IFX_StreamWrite* pFile, FX_DWORD flags = 0);
  int32_t Continue(IFX_Pause* pPause = NULL);
  FX_BOOL SetFileVersion(int32_t fileVersion = 17);

 private:
  friend class CPDF_ObjectStream;
  friend class CPDF_XRefStream;

  bool Create(FX_DWORD flags);
  void ResetStandardSecurity();
  void Clear();

  void InitOldObjNumOffsets();
  void InitNewObjNumOffsets();
  void InitID(FX_BOOL bDefault = TRUE);

  void AppendNewObjNum(FX_DWORD objbum);
  int32_t AppendObjectNumberToXRef(FX_DWORD objnum);

  int32_t WriteDoc_Stage1(IFX_Pause* pPause);
  int32_t WriteDoc_Stage2(IFX_Pause* pPause);
  int32_t WriteDoc_Stage3(IFX_Pause* pPause);
  int32_t WriteDoc_Stage4(IFX_Pause* pPause);

  int32_t WriteOldIndirectObject(FX_DWORD objnum);
  int32_t WriteOldObjs(IFX_Pause* pPause);
  int32_t WriteNewObjs(FX_BOOL bIncremental, IFX_Pause* pPause);
  int32_t WriteIndirectObj(const CPDF_Object* pObj);
  int32_t WriteDirectObj(FX_DWORD objnum,
                         const CPDF_Object* pObj,
                         FX_BOOL bEncrypt = TRUE);
  int32_t WriteIndirectObjectToStream(const CPDF_Object* pObj);
  int32_t WriteIndirectObj(FX_DWORD objnum, const CPDF_Object* pObj);
  int32_t WriteIndirectObjectToStream(FX_DWORD objnum,
                                      const uint8_t* pBuffer,
                                      FX_DWORD dwSize);

  int32_t WriteStream(const CPDF_Object* pStream,
                      FX_DWORD objnum,
                      IPDF_CryptoHandler* pCrypto);

  CPDF_Document* m_pDocument;
  CPDF_Parser* m_pParser;
  FX_BOOL m_bCompress;
  FX_BOOL m_bSecurityChanged;
  CPDF_Dictionary* m_pEncryptDict;
  FX_DWORD m_dwEnryptObjNum;
  FX_BOOL m_bEncryptCloned;
  FX_BOOL m_bStandardSecurity;
  IPDF_CryptoHandler* m_pCryptoHandler;
  FX_BOOL m_bNewCrypto;
  FX_BOOL m_bEncryptMetadata;
  CPDF_Object* m_pMetadata;
  CPDF_XRefStream* m_pXRefStream;
  int32_t m_ObjectStreamSize;
  FX_DWORD m_dwLastObjNum;
  CFX_FileBufferArchive m_File;
  FX_FILESIZE m_Offset;
  int32_t m_iStage;
  FX_DWORD m_dwFlags;
  FX_POSITION m_Pos;
  FX_FILESIZE m_XrefStart;
  CFX_FileSizeListArray m_ObjectOffset;
  CFX_DWordArray m_NewObjNumArray;
  CPDF_Array* m_pIDArray;
  int32_t m_FileVersion;
};

#endif  // CORE_FPDFAPI_FPDF_EDIT_INCLUDE_CPDF_CREATOR_H_
