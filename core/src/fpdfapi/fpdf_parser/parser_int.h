// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_PARSER_PARSER_INT_H_
#define CORE_SRC_FPDFAPI_FPDF_PARSER_PARSER_INT_H_

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_stream.h"

class CFX_BitStream;
class CPDF_DataAvail;
class CPDF_Dictionary;
class CPDF_Stream;
class IFX_DownloadHints;

class CPDF_HintTables {
 public:
  CPDF_HintTables(CPDF_DataAvail* pDataAvail, CPDF_Dictionary* pLinearized)
      : m_pLinearizedDict(pLinearized),
        m_pDataAvail(pDataAvail),
        m_nFirstPageSharedObjs(0),
        m_szFirstPageObjOffset(0) {}
  ~CPDF_HintTables();

  FX_BOOL GetPagePos(int index,
                     FX_FILESIZE& szPageStartPos,
                     FX_FILESIZE& szPageLength,
                     FX_DWORD& dwObjNum);
  IPDF_DataAvail::DocAvailStatus CheckPage(int index,
                                           IFX_DownloadHints* pHints);
  FX_BOOL LoadHintStream(CPDF_Stream* pHintStream);

 protected:
  FX_BOOL ReadPageHintTable(CFX_BitStream* hStream);
  FX_BOOL ReadSharedObjHintTable(CFX_BitStream* hStream, FX_DWORD offset);
  FX_DWORD GetItemLength(int index, const CFX_FileSizeArray& szArray);

 private:
  int ReadPrimaryHintStreamOffset() const;
  int ReadPrimaryHintStreamLength() const;

  CPDF_Dictionary* m_pLinearizedDict;
  CPDF_DataAvail* m_pDataAvail;
  FX_DWORD m_nFirstPageSharedObjs;
  FX_FILESIZE m_szFirstPageObjOffset;
  CFX_DWordArray m_dwDeltaNObjsArray;
  CFX_DWordArray m_dwNSharedObjsArray;
  CFX_DWordArray m_dwSharedObjNumArray;
  CFX_DWordArray m_dwIdentifierArray;
  CFX_FileSizeArray m_szPageOffsetArray;
  CFX_FileSizeArray m_szSharedObjOffsetArray;
};

#endif  // CORE_SRC_FPDFAPI_FPDF_PARSER_PARSER_INT_H_
