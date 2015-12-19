// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_JBIG2_JBIG2_HUFFMANTABLE_H_
#define CORE_SRC_FXCODEC_JBIG2_JBIG2_HUFFMANTABLE_H_

#include <vector>

#include "core/include/fxcrt/fx_system.h"

class CJBig2_BitStream;
struct JBig2TableLine;

class CJBig2_HuffmanTable {
 public:
  CJBig2_HuffmanTable(const JBig2TableLine* pTable,
                      FX_DWORD nLines,
                      bool bHTOOB);

  explicit CJBig2_HuffmanTable(CJBig2_BitStream* pStream);

  ~CJBig2_HuffmanTable();

  bool IsHTOOB() const { return HTOOB; }
  FX_DWORD Size() const { return NTEMP; }
  const std::vector<int>& GetCODES() const { return CODES; }
  const std::vector<int>& GetPREFLEN() const { return PREFLEN; }
  const std::vector<int>& GetRANGELEN() const { return RANGELEN; }
  const std::vector<int>& GetRANGELOW() const { return RANGELOW; }
  bool IsOK() const { return m_bOK; }

 private:
  void ParseFromStandardTable(const JBig2TableLine* pTable);
  bool ParseFromCodedBuffer(CJBig2_BitStream* pStream);
  void InitCodes();
  void ExtendBuffers(bool increment);

  bool m_bOK;
  bool HTOOB;
  FX_DWORD NTEMP;
  std::vector<int> CODES;
  std::vector<int> PREFLEN;
  std::vector<int> RANGELEN;
  std::vector<int> RANGELOW;
};

#endif  // CORE_SRC_FXCODEC_JBIG2_JBIG2_HUFFMANTABLE_H_
