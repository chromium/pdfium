// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _IFDE_TXTEDTBUF_H
#define _IFDE_TXTEDTBUF_H
#define FDE_DEFCHUNKLENGTH (1024)

class IFDE_TxtEdtBuf {
 public:
  virtual ~IFDE_TxtEdtBuf() {}
  virtual void Release() = 0;

  virtual FX_BOOL SetChunkSize(int32_t nChunkSize) = 0;
  virtual int32_t GetChunkSize() const = 0;
  virtual int32_t GetTextLength() const = 0;
  virtual void SetText(const CFX_WideString& wsText) = 0;
  virtual void GetText(CFX_WideString& wsText) const = 0;
  virtual FX_WCHAR GetCharByIndex(int32_t nIndex) const = 0;
  virtual void GetRange(CFX_WideString& wsText,
                        int32_t nBegin,
                        int32_t nCount = -1) const = 0;

  virtual void Insert(int32_t nPos,
                      const FX_WCHAR* lpText,
                      int32_t nLength = 1) = 0;
  virtual void Delete(int32_t nIndex, int32_t nLength = 1) = 0;

  virtual void Clear(FX_BOOL bRelease = TRUE) = 0;

  virtual FX_BOOL Optimize(IFX_Pause* pPause = NULL) = 0;
};
#endif
