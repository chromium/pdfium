// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_FGAS_ALGORITHM_H_
#define XFA_FGAS_CRT_FGAS_ALGORITHM_H_

#include <cstdint>

#include "core/fxcrt/include/fx_basic.h"

template <class baseType>
class CFX_DSPATemplate {
 public:
  int32_t Lookup(const baseType& find, const baseType* pArray, int32_t iCount) {
    ASSERT(pArray);
    if (iCount < 1)
      return -1;

    int32_t iStart = 0, iEnd = iCount - 1, iMid;
    do {
      iMid = (iStart + iEnd) / 2;
      const baseType& v = pArray[iMid];
      if (find == v)
        return iMid;
      if (find < v)
        iEnd = iMid - 1;
      else
        iStart = iMid + 1;
    } while (iStart <= iEnd);
    return -1;
  }
};

#endif  // XFA_FGAS_CRT_FGAS_ALGORITHM_H_
