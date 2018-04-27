// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_HUFFMANTABLE_H_
#define CORE_FXCODEC_JBIG2_JBIG2_HUFFMANTABLE_H_

#include <vector>

#include "core/fxcodec/jbig2/JBig2_Define.h"
#include "core/fxcrt/fx_system.h"

class CJBig2_BitStream;
struct JBig2TableLine;

class CJBig2_HuffmanTable {
 public:
  CJBig2_HuffmanTable(const JBig2TableLine* pTable,
                      uint32_t nLines,
                      bool bHTOOB);
  explicit CJBig2_HuffmanTable(CJBig2_BitStream* pStream);
  ~CJBig2_HuffmanTable();

  bool IsHTOOB() const { return HTOOB; }
  uint32_t Size() const { return NTEMP; }
  const std::vector<JBig2HuffmanCode>& GetCODES() const { return CODES; }
  const std::vector<int>& GetRANGELEN() const { return RANGELEN; }
  const std::vector<int>& GetRANGELOW() const { return RANGELOW; }
  bool IsOK() const { return m_bOK; }

 private:
  bool ParseFromStandardTable(const JBig2TableLine* pTable);
  bool ParseFromCodedBuffer(CJBig2_BitStream* pStream);
  void ExtendBuffers(bool increment);

  bool m_bOK;
  bool HTOOB;
  uint32_t NTEMP;
  std::vector<JBig2HuffmanCode> CODES;
  std::vector<int> RANGELEN;
  std::vector<int> RANGELOW;
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_HUFFMANTABLE_H_
