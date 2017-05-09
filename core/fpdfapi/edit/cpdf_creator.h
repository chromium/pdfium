// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_EDIT_CPDF_CREATOR_H_
#define CORE_FPDFAPI_EDIT_CPDF_CREATOR_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"

class CPDF_Array;
class CPDF_CryptoHandler;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Object;
class CPDF_Parser;
class CPDF_XRefStream;

#define FPDFCREATE_INCREMENTAL 1
#define FPDFCREATE_NO_ORIGINAL 2
#define FPDFCREATE_PROGRESSIVE 4
#define FPDFCREATE_OBJECTSTREAM 8

class CPDF_Creator {
 public:
  explicit CPDF_Creator(CPDF_Document* pDoc,
                        const CFX_RetainPtr<IFX_WriteStream>& pFile);
  ~CPDF_Creator();

  void RemoveSecurity();
  bool Create(uint32_t flags);
  int32_t Continue();
  bool SetFileVersion(int32_t fileVersion);

  CFX_FileBufferArchive* GetFile() { return &m_File; }

  FX_FILESIZE GetOffset() const { return m_Offset; }
  void IncrementOffset(FX_FILESIZE inc);
  uint32_t GetNextObjectNumber() { return ++m_dwLastObjNum; }
  uint32_t GetLastObjectNumber() const { return m_dwLastObjNum; }
  CPDF_CryptoHandler* GetCryptoHandler() { return m_pCryptoHandler.Get(); }
  CPDF_Document* GetDocument() const { return m_pDocument; }
  CPDF_Array* GetIDArray() const { return m_pIDArray.get(); }
  CPDF_Dictionary* GetEncryptDict() const { return m_pEncryptDict; }
  uint32_t GetEncryptObjectNumber() const { return m_dwEncryptObjNum; }

  uint32_t GetObjectOffset(uint32_t objnum) { return m_ObjectOffsets[objnum]; }
  bool HasObjectNumber(uint32_t objnum) {
    return m_ObjectOffsets.find(objnum) != m_ObjectOffsets.end();
  }
  void SetObjectOffset(uint32_t objnum, FX_FILESIZE offset) {
    m_ObjectOffsets[objnum] = offset;
  }
  bool IsIncremental() const { return !!(m_dwFlags & FPDFCREATE_INCREMENTAL); }

 private:
  void Clear();

  void InitOldObjNumOffsets();
  void InitNewObjNumOffsets();
  void InitID();

  int32_t AppendObjectNumberToXRef(uint32_t objnum);

  int32_t WriteDoc_Stage1();
  int32_t WriteDoc_Stage2();
  int32_t WriteDoc_Stage3();
  int32_t WriteDoc_Stage4();

  int32_t WriteOldIndirectObject(uint32_t objnum);
  int32_t WriteOldObjs();
  int32_t WriteNewObjs();
  int32_t WriteIndirectObj(const CPDF_Object* pObj);
  int32_t WriteDirectObj(uint32_t objnum,
                         const CPDF_Object* pObj,
                         bool bEncrypt);
  int32_t WriteIndirectObjectToStream(const CPDF_Object* pObj);
  int32_t WriteIndirectObj(uint32_t objnum, const CPDF_Object* pObj);
  int32_t WriteIndirectObjectToStream(uint32_t objnum,
                                      const uint8_t* pBuffer,
                                      uint32_t dwSize);

  int32_t WriteStream(const CPDF_Object* pStream,
                      uint32_t objnum,
                      CPDF_CryptoHandler* pCrypto);

  bool IsXRefNeedEnd();

  CPDF_Document* const m_pDocument;
  CPDF_Parser* const m_pParser;
  bool m_bSecurityChanged;
  CPDF_Dictionary* m_pEncryptDict;
  uint32_t m_dwEncryptObjNum;
  CFX_RetainPtr<CPDF_CryptoHandler> m_pCryptoHandler;
  CPDF_Object* m_pMetadata;
  std::unique_ptr<CPDF_XRefStream> m_pXRefStream;
  uint32_t m_dwLastObjNum;
  CFX_FileBufferArchive m_File;
  FX_FILESIZE m_Offset;
  FX_FILESIZE m_SavedOffset;
  int32_t m_iStage;
  uint32_t m_dwFlags;
  uint32_t m_CurObjNum;
  FX_FILESIZE m_XrefStart;
  std::map<uint32_t, FX_FILESIZE> m_ObjectOffsets;
  std::vector<uint32_t> m_NewObjNumArray;  // Sorted, ascending.
  std::unique_ptr<CPDF_Array> m_pIDArray;
  int32_t m_FileVersion;
};

#endif  // CORE_FPDFAPI_EDIT_CPDF_CREATOR_H_
