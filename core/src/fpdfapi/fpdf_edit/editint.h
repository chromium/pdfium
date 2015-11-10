// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_EDIT_EDITINT_H_
#define CORE_SRC_FPDFAPI_FPDF_EDIT_EDITINT_H_

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_stream.h"
#include "core/include/fxcrt/fx_system.h"

class CPDF_Creator;
class CPDF_Object;

class CPDF_ObjectStream {
 public:
  CPDF_ObjectStream();

  FX_BOOL Start();

  int32_t CompressIndirectObject(FX_DWORD dwObjNum, const CPDF_Object* pObj);
  int32_t CompressIndirectObject(FX_DWORD dwObjNum,
                                 const uint8_t* pBuffer,
                                 FX_DWORD dwSize);

  FX_FILESIZE End(CPDF_Creator* pCreator);

  CFX_DWordArray m_ObjNumArray;

  CFX_ByteTextBuf m_Buffer;
  FX_DWORD m_dwObjNum;
  int32_t m_index;

 protected:
  CFX_DWordArray m_OffsetArray;
};
class CPDF_XRefStream {
 public:
  CPDF_XRefStream();

  FX_BOOL Start();

  int32_t CompressIndirectObject(FX_DWORD dwObjNum,
                                 const CPDF_Object* pObj,
                                 CPDF_Creator* pCreator);

  int32_t CompressIndirectObject(FX_DWORD dwObjNum,
                                 const uint8_t* pBuffer,
                                 FX_DWORD dwSize,
                                 CPDF_Creator* pCreator);

  FX_BOOL End(CPDF_Creator* pCreator, FX_BOOL bEOF = FALSE);
  FX_BOOL AddObjectNumberToIndexArray(FX_DWORD objnum);
  FX_BOOL EndXRefStream(CPDF_Creator* pCreator);

  CFX_DWordArray m_IndexArray;

  FX_FILESIZE m_PrevOffset;
  FX_DWORD m_dwTempObjNum;

 protected:
  int32_t EndObjectStream(CPDF_Creator* pCreator, FX_BOOL bEOF = TRUE);
  FX_BOOL GenerateXRefStream(CPDF_Creator* pCreator, FX_BOOL bEOF);
  int32_t m_iSeg;
  CPDF_ObjectStream m_ObjStream;
  CFX_ByteTextBuf m_Buffer;
};

#endif  // CORE_SRC_FPDFAPI_FPDF_EDIT_EDITINT_H_
