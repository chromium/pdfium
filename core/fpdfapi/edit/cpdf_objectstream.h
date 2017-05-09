// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_EDIT_CPDF_OBJECTSTREAM_H_
#define CORE_FPDFAPI_EDIT_CPDF_OBJECTSTREAM_H_

#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "third_party/base/stl_util.h"

class CPDF_Creator;
class CPDF_Object;

class CPDF_ObjectStream {
 public:
  struct Item {
    uint32_t objnum;
    FX_STRSIZE offset;
  };

  CPDF_ObjectStream();
  ~CPDF_ObjectStream();

  void Start();
  FX_FILESIZE End(CPDF_Creator* pCreator);
  void CompressIndirectObject(uint32_t dwObjNum, const CPDF_Object* pObj);
  void CompressIndirectObject(uint32_t dwObjNum,
                              const uint8_t* pBuffer,
                              uint32_t dwSize);

  bool IsNotFull() const;
  int32_t ItemCount() const { return pdfium::CollectionSize<int32_t>(m_Items); }
  void SetObjectNumber(uint32_t num) { m_dwObjNum = num; }
  uint32_t GetObjectNumber() const { return m_dwObjNum; }
  int32_t GetIndex() const { return m_index; }
  void IncrementIndex() { m_index++; }

  uint32_t GetObjectNumberForItem(int index) const {
    return m_Items[index].objnum;
  }

 private:
  std::vector<Item> m_Items;
  CFX_ByteTextBuf m_Buffer;
  uint32_t m_dwObjNum;
  int32_t m_index;
};

#endif  // CORE_FPDFAPI_EDIT_CPDF_OBJECTSTREAM_H_
