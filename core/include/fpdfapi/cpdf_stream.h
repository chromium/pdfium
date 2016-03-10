// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_CPDF_STREAM_H_
#define CORE_INCLUDE_FPDFAPI_CPDF_STREAM_H_

#include "core/include/fpdfapi/cpdf_object.h"
#include "core/include/fxcrt/fx_stream.h"

class CPDF_Stream : public CPDF_Object {
 public:
  CPDF_Stream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict);

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CPDF_Dictionary* GetDict() const override;
  CFX_WideString GetUnicodeText() const override;
  bool IsStream() const override;
  CPDF_Stream* AsStream() override;
  const CPDF_Stream* AsStream() const override;

  FX_DWORD GetRawSize() const { return m_dwSize; }
  uint8_t* GetRawData() const { return m_pDataBuf; }

  void SetData(const uint8_t* pData,
               FX_DWORD size,
               FX_BOOL bCompressed,
               FX_BOOL bKeepBuf);

  void InitStream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict);
  void InitStreamFromFile(IFX_FileRead* pFile, CPDF_Dictionary* pDict);

  FX_BOOL ReadRawData(FX_FILESIZE start_pos,
                      uint8_t* pBuf,
                      FX_DWORD buf_size) const;

  bool IsMemoryBased() const { return m_GenNum == kMemoryBasedGenNum; }

 protected:
  static const FX_DWORD kMemoryBasedGenNum = (FX_DWORD)-1;

  ~CPDF_Stream() override;

  void InitStreamInternal(CPDF_Dictionary* pDict);

  CPDF_Dictionary* m_pDict;
  FX_DWORD m_dwSize;
  FX_DWORD m_GenNum;

  union {
    uint8_t* m_pDataBuf;
    IFX_FileRead* m_pFile;
  };
};

inline CPDF_Stream* ToStream(CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}

inline const CPDF_Stream* ToStream(const CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}

class CPDF_StreamAcc {
 public:
  CPDF_StreamAcc();
  ~CPDF_StreamAcc();

  void LoadAllData(const CPDF_Stream* pStream,
                   FX_BOOL bRawAccess = FALSE,
                   FX_DWORD estimated_size = 0,
                   FX_BOOL bImageAcc = FALSE);

  const CPDF_Stream* GetStream() const { return m_pStream; }
  CPDF_Dictionary* GetDict() const {
    return m_pStream ? m_pStream->GetDict() : nullptr;
  }
  const uint8_t* GetData() const;
  FX_DWORD GetSize() const;
  const CFX_ByteString& GetImageDecoder() const { return m_ImageDecoder; }
  const CPDF_Dictionary* GetImageParam() const { return m_pImageParam; }

  uint8_t* DetachData();

 protected:
  uint8_t* m_pData;
  FX_DWORD m_dwSize;
  FX_BOOL m_bNewBuf;
  CFX_ByteString m_ImageDecoder;
  CPDF_Dictionary* m_pImageParam;
  const CPDF_Stream* m_pStream;
  uint8_t* m_pSrcData;
};

#endif  // CORE_INCLUDE_FPDFAPI_CPDF_STREAM_H_
