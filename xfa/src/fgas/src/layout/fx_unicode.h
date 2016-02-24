// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FGAS_SRC_LAYOUT_FX_UNICODE_H_
#define XFA_SRC_FGAS_SRC_LAYOUT_FX_UNICODE_H_

struct FX_TPO {
  int32_t index;
  int32_t pos;
};
typedef CFX_MassArrayTemplate<FX_TPO> CFX_TPOArray;

void FX_TEXTLAYOUT_PieceSort(CFX_TPOArray& tpos, int32_t iStart, int32_t iEnd);

#endif  // XFA_SRC_FGAS_SRC_LAYOUT_FX_UNICODE_H_
