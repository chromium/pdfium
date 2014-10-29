// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_CLIPBOARDMGR_H
#define _FWL_ADAPTER_CLIPBOARDMGR_H
class IFWL_Widget;
class IFWL_AdapterClipboardMgr;
enum FWL_CLIPBOARDFORMAT {
    FWL_CLIPBOARDFORMAT_Dib,
    FWL_CLIPBOARDFORMAT_Text,
    FWL_CLIPBOARDFORMAT_UncodeText,
};
typedef struct _FWL_HCLIPBOARDDATA {
    FX_LPVOID pData;
} * FWL_HCLIPBOARDDATA;
class IFWL_AdapterClipboardMgr
{
public:
    virtual FWL_ERR		Empty() = 0;
    virtual FX_BOOL		IsDataAvailable(FX_DWORD dwFormat) = 0;
    virtual FWL_HCLIPBOARDDATA	GetData(FX_DWORD dwFormat) = 0;
    virtual FWL_ERR		SetData(FX_DWORD dwFormat, FX_BYTE *pBuf, FX_INT32 iSize) = 0;
    virtual FX_INT32	GetDataSize(FWL_HCLIPBOARDDATA hData) = 0;
    virtual FX_LPVOID	LockDataBuffer(FWL_HCLIPBOARDDATA hData) = 0;
    virtual FX_BOOL		UnLockDataBuffer(FWL_HCLIPBOARDDATA hData) = 0;
    virtual FWL_ERR		SetStringData(FX_WSTR ws) = 0;
    virtual FWL_ERR		SetStringData(FX_BSTR bs) = 0;
    virtual FWL_ERR		GetStringData(CFX_WideString &ws) = 0;
    virtual FWL_ERR		GetStringData(CFX_ByteString &bs) = 0;
    virtual FWL_ERR		EnumFormats(CFX_DWordArray &formats) = 0;
    virtual FX_DWORD	RegisterFormat(FX_WSTR wsFormat) = 0;
};
#endif
