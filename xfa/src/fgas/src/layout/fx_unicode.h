// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FGAS_UNICODE_IMP
#define _FGAS_UNICODE_IMP
typedef struct _FX_TPO {
    FX_INT32 index;
    FX_INT32 pos;
} FX_TPO;
typedef CFX_MassArrayTemplate<FX_TPO>	CFX_TPOArray;
void FX_TEXTLAYOUT_PieceSort(CFX_TPOArray &tpos, FX_INT32 iStart, FX_INT32 iEnd);
#endif
