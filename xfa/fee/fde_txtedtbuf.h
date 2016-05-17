// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FEE_FDE_TXTEDTBUF_H_
#define XFA_FEE_FDE_TXTEDTBUF_H_

#include "xfa/fee/ifde_txtedtengine.h"
#include "xfa/fgas/crt/fgas_memory.h"

class CFDE_TxtEdtBuf;

class CFDE_TxtEdtBufIter : public IFX_CharIter {
 public:
  CFDE_TxtEdtBufIter(CFDE_TxtEdtBuf* pBuf, FX_WCHAR wcAlias = 0);
  ~CFDE_TxtEdtBufIter() override;

  FX_BOOL Next(FX_BOOL bPrev = FALSE) override;
  FX_WCHAR GetChar() override;
  void SetAt(int32_t nIndex) override;
  int32_t GetAt() const override;
  FX_BOOL IsEOF(FX_BOOL bTail = TRUE) const override;
  IFX_CharIter* Clone() override;

 private:
  CFDE_TxtEdtBuf* m_pBuf;
  int32_t m_nCurChunk;
  int32_t m_nCurIndex;
  int32_t m_nIndex;
  FX_WCHAR m_Alias;
};

class CFDE_TxtEdtBuf {
 public:
  CFDE_TxtEdtBuf();

  void Release();
  FX_BOOL SetChunkSize(int32_t nChunkSize);
  int32_t GetChunkSize() const;
  int32_t GetTextLength() const;
  void SetText(const CFX_WideString& wsText);
  void GetText(CFX_WideString& wsText) const;
  FX_WCHAR GetCharByIndex(int32_t nIndex) const;
  void GetRange(CFX_WideString& wsText,
                int32_t nBegin,
                int32_t nCount = -1) const;

  void Insert(int32_t nPos, const FX_WCHAR* lpText, int32_t nLength = 1);
  void Delete(int32_t nIndex, int32_t nLength = 1);
  void Clear(FX_BOOL bRelease = TRUE);

  FX_BOOL Optimize(IFX_Pause* pPause = nullptr);

 protected:
  ~CFDE_TxtEdtBuf();

 private:
  friend class CFDE_TxtEdtBufIter;

  struct FDE_CHUNKHEADER {
    int32_t nUsed;
    FX_WCHAR wChars[1];
  };

  struct FDE_CHUNKPLACE {
    int32_t nChunkIndex;
    int32_t nCharIndex;
  };

  void ResetChunkBuffer(int32_t nDefChunkCount, int32_t nChunkSize);
  int32_t CP2Index(const FDE_CHUNKPLACE& cp) const;
  void Index2CP(int32_t nIndex, FDE_CHUNKPLACE& cp) const;

  int32_t m_nChunkSize;

  int32_t m_nTotal;
  FX_BOOL m_bChanged;
  CFX_ArrayTemplate<FDE_CHUNKHEADER*> m_Chunks;
  IFX_MemoryAllocator* m_pAllocator;
};

#endif  // XFA_FEE_FDE_TXTEDTBUF_H_
