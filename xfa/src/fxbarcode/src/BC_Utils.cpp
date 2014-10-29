// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
FX_BOOL BC_FX_ByteString_Replace(CFX_ByteString &dst, FX_DWORD first, FX_DWORD last, FX_INT32 count, FX_CHAR c)
{
    if (first > last || count <= 0) {
        return FALSE;
    }
    dst.Delete(first, last - first);
    for (FX_INT32 i = 0; i < count; i ++) {
        dst.Insert(0, c);
    }
    return TRUE;
}
void BC_FX_ByteString_Append(CFX_ByteString &dst, FX_INT32 count, FX_CHAR c)
{
    for (FX_INT32 i = 0; i < count; i ++) {
        dst += c;
    }
}
void BC_FX_ByteString_Append(CFX_ByteString &dst, const CFX_ByteArray &ba)
{
    for (FX_INT32 i = 0; i < ba.GetSize(); i ++) {
        dst += ba[i];
    }
}
void BC_FX_PtrArray_Sort(CFX_PtrArray &src, BC_PtrArrayCompareCallback fun)
{
    FX_INT32 nLength = src.GetSize();
    FX_BOOL changed = true;
    do {
        changed = false;
        for (FX_INT32 i = 0; i < nLength - 1; i++) {
            if ( fun(src[i + 1], src[i]) ) {
                void *temp = src[i];
                src.SetAt(i, src[i + 1]);
                src.SetAt(i + 1, temp);
                changed = true;
            }
        }
    } while (changed);
}
