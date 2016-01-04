// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_SERIAL_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_SERIAL_H_

#include "fpdf_page.h"
#include "fpdf_pageobj.h"

class CPDF_ObjectStream;
class CPDF_XRefStream;
CFX_ByteTextBuf& operator<<(CFX_ByteTextBuf& buf, const CPDF_Object* pObj);
#define FPDFCREATE_INCREMENTAL 1
#define FPDFCREATE_NO_ORIGINAL 2
#define FPDFCREATE_PROGRESSIVE 4
#define FPDFCREATE_OBJECTSTREAM 8

class CPDF_Creator {
 public:
  CPDF_Creator(CPDF_Document* pDoc);
  ~CPDF_Creator();

  void RemoveSecurity();
  FX_BOOL Create(IFX_StreamWrite* pFile, FX_DWORD flags = 0);
  int32_t Continue(IFX_Pause* pPause = NULL);
  FX_BOOL SetFileVersion(int32_t fileVersion = 17);

 protected:
  CPDF_Document* m_pDocument;

  CPDF_Parser* m_pParser;

  FX_BOOL m_bCompress;

  FX_BOOL m_bSecurityChanged;

  CPDF_Dictionary* m_pEncryptDict;
  FX_DWORD m_dwEnryptObjNum;
  FX_BOOL m_bEncryptCloned;

  FX_BOOL m_bStandardSecurity;

  CPDF_CryptoHandler* m_pCryptoHandler;
  FX_BOOL m_bNewCrypto;

  FX_BOOL m_bEncryptMetadata;

  CPDF_Object* m_pMetadata;

  CPDF_XRefStream* m_pXRefStream;

  int32_t m_ObjectStreamSize;

  FX_DWORD m_dwLastObjNum;
  FX_BOOL Create(FX_DWORD flags);
  void ResetStandardSecurity();
  void Clear();
  int32_t WriteDoc_Stage1(IFX_Pause* pPause);
  int32_t WriteDoc_Stage2(IFX_Pause* pPause);
  int32_t WriteDoc_Stage3(IFX_Pause* pPause);
  int32_t WriteDoc_Stage4(IFX_Pause* pPause);

  CFX_FileBufferArchive m_File;

  FX_FILESIZE m_Offset;
  void InitOldObjNumOffsets();
  void InitNewObjNumOffsets();
  void AppendNewObjNum(FX_DWORD objbum);
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
  int32_t AppendObjectNumberToXRef(FX_DWORD objnum);
  void InitID(FX_BOOL bDefault = TRUE);
  int32_t WriteStream(const CPDF_Object* pStream,
                      FX_DWORD objnum,
                      CPDF_CryptoHandler* pCrypto);

  int32_t m_iStage;
  FX_DWORD m_dwFlags;
  FX_POSITION m_Pos;
  FX_FILESIZE m_XrefStart;
  CFX_FileSizeListArray m_ObjectOffset;
  CFX_DWordArray m_NewObjNumArray;
  CPDF_Array* m_pIDArray;
  int32_t m_FileVersion;

  friend class CPDF_ObjectStream;
  friend class CPDF_XRefStream;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_SERIAL_H_
