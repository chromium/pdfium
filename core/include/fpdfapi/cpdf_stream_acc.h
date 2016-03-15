// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_CPDF_STREAM_ACC_H_
#define CORE_INCLUDE_FPDFAPI_CPDF_STREAM_ACC_H_

#include "core/include/fpdfapi/cpdf_stream.h"
#include "core/include/fpdfapi/cpdf_dictionary.h"
#include "core/include/fxcrt/fx_string.h"
#include "core/include/fxcrt/fx_system.h"

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

#endif  // CORE_INCLUDE_FPDFAPI_CPDF_STREAM_ACC_H_
