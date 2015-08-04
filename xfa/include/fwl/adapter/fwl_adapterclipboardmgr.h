// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_CLIPBOARDMGR_H
#define _FWL_ADAPTER_CLIPBOARDMGR_H

enum FWL_CLIPBOARDFORMAT {
  FWL_CLIPBOARDFORMAT_Dib,
  FWL_CLIPBOARDFORMAT_Text,
  FWL_CLIPBOARDFORMAT_UncodeText,
};
typedef struct _FWL_HCLIPBOARDDATA { void* pData; } * FWL_HCLIPBOARDDATA;

class IFWL_AdapterClipboardMgr {
 public:
  virtual ~IFWL_AdapterClipboardMgr() {}
  virtual FWL_ERR Empty() = 0;
  virtual FX_BOOL IsDataAvailable(FX_DWORD dwFormat) = 0;
  virtual FWL_HCLIPBOARDDATA GetData(FX_DWORD dwFormat) = 0;
  virtual FWL_ERR SetData(FX_DWORD dwFormat, uint8_t* pBuf, int32_t iSize) = 0;
  virtual int32_t GetDataSize(FWL_HCLIPBOARDDATA hData) = 0;
  virtual void* LockDataBuffer(FWL_HCLIPBOARDDATA hData) = 0;
  virtual FX_BOOL UnLockDataBuffer(FWL_HCLIPBOARDDATA hData) = 0;
  virtual FWL_ERR SetStringData(const CFX_WideStringC& ws) = 0;
  virtual FWL_ERR SetStringData(const CFX_ByteStringC& bs) = 0;
  virtual FWL_ERR GetStringData(CFX_WideString& ws) = 0;
  virtual FWL_ERR GetStringData(CFX_ByteString& bs) = 0;
  virtual FWL_ERR EnumFormats(CFX_DWordArray& formats) = 0;
  virtual FX_DWORD RegisterFormat(const CFX_WideStringC& wsFormat) = 0;
};
#endif
