// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_basic.h"
#include "plex.h"
CFX_Plex* CFX_Plex::Create(CFX_Plex*& pHead, FX_DWORD nMax, FX_DWORD cbElement)
{
    CFX_Plex* p = (CFX_Plex*)FX_Alloc(FX_BYTE, sizeof(CFX_Plex) + nMax * cbElement);
    p->pNext = pHead;
    pHead = p;
    return p;
}
void CFX_Plex::FreeDataChain()
{
    CFX_Plex* p = this;
    while (p != NULL) {
        FX_BYTE* bytes = (FX_BYTE*)p;
        CFX_Plex* pNext = p->pNext;
        FX_Free(bytes);
        p = pNext;
    }
}
