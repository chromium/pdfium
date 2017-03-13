// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_FGAS_UTILS_H_
#define XFA_FGAS_CRT_FGAS_UTILS_H_

#include "core/fxcrt/fx_coordinates.h"

class CFX_BaseDiscreteArray {
 protected:
  CFX_BaseDiscreteArray(int32_t iChunkSize, int32_t iBlockSize);
  ~CFX_BaseDiscreteArray();

  uint8_t* AddSpaceTo(int32_t index);
  uint8_t* GetAt(int32_t index) const;
  void RemoveAll();
  void* m_pData;
};

template <class baseType>
class CFX_DiscreteArrayTemplate : public CFX_BaseDiscreteArray {
 public:
  explicit CFX_DiscreteArrayTemplate(int32_t iChunkSize)
      : CFX_BaseDiscreteArray(iChunkSize, sizeof(baseType)) {}

  baseType& GetAt(int32_t index, const baseType& defValue) const {
    baseType* p = (baseType*)CFX_BaseDiscreteArray::GetAt(index);
    return p ? *p : (baseType&)defValue;
  }
  baseType* GetPtrAt(int32_t index) const {
    return (baseType*)CFX_BaseDiscreteArray::GetAt(index);
  }
  void SetAtGrow(int32_t index, const baseType& element) {
    *(baseType*)CFX_BaseDiscreteArray::AddSpaceTo(index) = element;
  }
  void RemoveAll() { CFX_BaseDiscreteArray::RemoveAll(); }
};

#endif  // XFA_FGAS_CRT_FGAS_UTILS_H_
